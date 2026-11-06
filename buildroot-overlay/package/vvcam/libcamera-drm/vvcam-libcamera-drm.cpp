/*
 * Open vvcam cameras through libcamera and show NV12 on the screen.
 *
 *   vvcam-libcamera-drm
 *   vvcam-libcamera-drm -d 1
 *   vvcam-libcamera-drm -d 1 480 800
 *   vvcam-libcamera-drm -d 1 -x 0 -y 0 -w 240 -h 800 -d 4 -x 240 -y 0 -w 240 -h 800
 *
 * Each -d starts a camera. -x/-y are the screen position of that camera,
 * and -w/-h are its capture and display size. Press q or Ctrl-C to stop.
 * Width is aligned down to 16 and height to 8.
 */

#include <atomic>
#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <mutex>
#include <fcntl.h>
#include <signal.h>
#include <termios.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/mman.h>
#include <vector>

#include <libcamera/camera.h>
#include <libcamera/camera_manager.h>
#include <libcamera/formats.h>
#include <libcamera/framebuffer.h>
#include <libcamera/framebuffer_allocator.h>
#include <libcamera/request.h>
#include <libcamera/stream.h>

#include <display.h>

using namespace libcamera;

static constexpr unsigned kMaxCameras = 3;

static volatile sig_atomic_t g_stop;

static void on_signal(int)
{
	g_stop = 1;
}

struct StdinKeys {
	bool active = false;
	bool termios_saved = false;
	struct termios saved {};
	int flags = -1;

	void enable()
	{
		flags = fcntl(STDIN_FILENO, F_GETFL, 0);
		if (flags >= 0)
			fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
		if (isatty(STDIN_FILENO) && tcgetattr(STDIN_FILENO, &saved) == 0) {
			struct termios raw = saved;
			raw.c_lflag &= ~(ICANON | ECHO);
			raw.c_cc[VMIN] = 0;
			raw.c_cc[VTIME] = 0;
			if (tcsetattr(STDIN_FILENO, TCSANOW, &raw) == 0)
				termios_saved = true;
		}
		active = true;
	}

	void restore()
	{
		if (!active)
			return;
		if (termios_saved)
			tcsetattr(STDIN_FILENO, TCSANOW, &saved);
		if (flags >= 0)
			fcntl(STDIN_FILENO, F_SETFL, flags);
		active = false;
	}

	bool quit_requested()
	{
		char key;
		while (read(STDIN_FILENO, &key, 1) > 0) {
			if (key == 'q' || key == 'Q')
				return true;
		}
		return false;
	}

	~StdinKeys()
	{
		restore();
	}
};

static unsigned align_down(unsigned value, unsigned align)
{
	return value & ~(align - 1);
}

static void usage()
{
	std::cerr << "usage: vvcam-libcamera-drm [-d video] [width height]\n"
		  << "       vvcam-libcamera-drm -d video -x X -y Y -w W -h H [-d video -x X -y Y -w W -h H ...]\n"
		  << "  -d  /dev/video 序号，默认 1\n"
		  << "  -x  画面左上角的横坐标，默认单路居中\n"
		  << "  -y  画面左上角的纵坐标，默认单路居中\n"
		  << "  -w  这一路的宽\n"
		  << "  -h  这一路的高\n";
}

static bool read_first_line(const std::string &path, std::string *text)
{
	std::ifstream input(path);
	if (!input)
		return false;
	std::getline(input, *text);
	while (!text->empty() && (text->back() == '\n' || text->back() == '\r'))
		text->pop_back();
	return !text->empty();
}

struct MappedFd {
	void *address = MAP_FAILED;
	size_t length = 0;
};

static void *plane_address(std::map<int, MappedFd> &mapped, const FrameBuffer::Plane &plane)
{
	const int fd = plane.fd.get();
	auto it = mapped.find(fd);
	if (it == mapped.end()) {
		size_t length = static_cast<size_t>(plane.offset) + plane.length;
		const off_t end = lseek(fd, 0, SEEK_END);
		if (end > 0 && static_cast<size_t>(end) > length)
			length = static_cast<size_t>(end);
		void *address = mmap(nullptr, length, PROT_READ, MAP_SHARED, fd, 0);
		if (address == MAP_FAILED)
			return nullptr;
		mapped.emplace(fd, MappedFd{ address, length });
		it = mapped.find(fd);
	}
	if (plane.offset + plane.length > it->second.length)
		return nullptr;
	return static_cast<uint8_t *>(it->second.address) + plane.offset;
}

static void copy_nv12(uint8_t *dst, unsigned dst_stride,
		       const uint8_t *src_y, unsigned src_y_stride,
		       const uint8_t *src_uv, unsigned src_uv_stride,
		       unsigned width, unsigned height)
{
	for (unsigned y = 0; y < height; y++)
		memcpy(dst + y * dst_stride, src_y + y * src_y_stride, width);

	uint8_t *dst_uv = dst + static_cast<size_t>(dst_stride) * height;
	for (unsigned y = 0; y < height / 2; y++)
		memcpy(dst_uv + y * dst_stride, src_uv + y * src_uv_stride, width);
}

static int wait_flip(struct display *display)
{
	while (display->req && !g_stop) {
		fd_set fds;
		FD_ZERO(&fds);
		FD_SET(display->fd, &fds);
		timeval tv{ 0, 200000 };
		const int ret = select(display->fd + 1, &fds, nullptr, nullptr, &tv);
		if (ret < 0) {
			if (errno == EINTR)
				continue;
			return -1;
		}
		if (ret == 0)
			continue;
		drmHandleEvent(display->fd, &display->drm_event_ctx);
		drmModeAtomicFree(display->req);
		display->req = nullptr;
	}
	return 0;
}

struct View {
	int video = 1;
	int x = -1;
	int y = -1;
	unsigned width = 0;
	unsigned height = 0;
};

class Preview {
public:
	Preview(struct display_buffer *buffer, unsigned width, unsigned height, unsigned stride)
		: buffer_(buffer), width_(width), height_(height), stride_(stride)
	{
	}

	void onRequest(Request *request)
	{
		if (request->status() == Request::RequestComplete)
			show(request);
		if (g_stop)
			return;
		request->reuse(Request::ReuseBuffers);
		if (camera_)
			camera_->queueRequest(request);
	}

	Camera *camera_ = nullptr;
	std::map<int, MappedFd> mapped_;
	std::atomic<unsigned> copied{ 0 };
	bool seen_ = false;

	bool take()
	{
		std::lock_guard<std::mutex> lock(mutex_);
		if (!dirty_)
			return false;
		dirty_ = false;
		seen_ = true;
		return true;
	}

private:
	void show(Request *request)
	{
		FrameBuffer *fb = request->buffers().begin()->second;
		const auto planes = fb->planes();
		if (planes.empty())
			return;

		const uint8_t *y = static_cast<const uint8_t *>(plane_address(mapped_, planes[0]));
		const uint8_t *uv = nullptr;
		unsigned y_stride = stride_;
		unsigned uv_stride = stride_;
		if (!y)
			return;
		if (planes.size() >= 2) {
			uv = static_cast<const uint8_t *>(plane_address(mapped_, planes[1]));
			if (!uv)
				return;
		} else {
			uv = y + static_cast<size_t>(y_stride) * height_;
		}

		std::lock_guard<std::mutex> lock(mutex_);
		copy_nv12(static_cast<uint8_t *>(buffer_->map), buffer_->stride,
			  y, y_stride, uv, uv_stride, width_, height_);
		dirty_ = true;
		copied.fetch_add(1, std::memory_order_relaxed);
	}

	struct display_buffer *buffer_;
	unsigned width_;
	unsigned height_;
	unsigned stride_;
	std::mutex mutex_;
	bool dirty_ = false;
};

struct Running {
	View view;
	std::string entity;
	std::shared_ptr<Camera> camera;
	std::unique_ptr<CameraConfiguration> config;
	std::unique_ptr<FrameBufferAllocator> allocator;
	std::vector<std::unique_ptr<Request>> requests;
	std::unique_ptr<Preview> preview;
	struct display_plane *plane = nullptr;
	struct display_buffer *buffer = nullptr;
	bool started = false;
};

static bool parse_int(const char *text, long *value)
{
	char *end = nullptr;
	*value = strtol(text, &end, 10);
	return end && !*end;
}

static int parse_args(int argc, char **argv, std::vector<View> *views)
{
	int current = -1;
	int opt;
	while ((opt = getopt(argc, argv, "d:x:y:w:h:")) != -1) {
		long value = 0;
		if (opt != 'd' && opt != 'x' && opt != 'y' && opt != 'w' && opt != 'h') {
			usage();
			return -1;
		}
		if (!parse_int(optarg, &value)) {
			usage();
			return -1;
		}
		if (opt == 'd') {
			if (views->size() >= kMaxCameras) {
				std::cerr << "vvcam-libcamera-drm: 最多 " << kMaxCameras << " 路\n";
				return -1;
			}
			views->push_back(View{});
			current = static_cast<int>(views->size()) - 1;
			if (value < 0 || value > 63) {
				usage();
				return -1;
			}
			(*views)[current].video = static_cast<int>(value);
			continue;
		}
		if (current < 0) {
			usage();
			return -1;
		}
		View &view = (*views)[current];
		switch (opt) {
		case 'x':
			view.x = static_cast<int>(value);
			break;
		case 'y':
			view.y = static_cast<int>(value);
			break;
		case 'w':
			if (value <= 0) {
				usage();
				return -1;
			}
			view.width = static_cast<unsigned>(value);
			break;
		case 'h':
			if (value <= 0) {
				usage();
				return -1;
			}
			view.height = static_cast<unsigned>(value);
			break;
		}
	}

	if (views->empty())
		views->push_back(View{});
	if (argc - optind == 2 && views->size() == 1 &&
	    (*views)[0].width == 0 && (*views)[0].height == 0) {
		(*views)[0].width = static_cast<unsigned>(atoi(argv[optind]));
		(*views)[0].height = static_cast<unsigned>(atoi(argv[optind + 1]));
	} else if (argc != optind) {
		usage();
		return -1;
	}
	return 0;
}

int main(int argc, char **argv)
{
	struct sigaction action{};
	action.sa_handler = on_signal;
	sigaction(SIGINT, &action, nullptr);
	sigaction(SIGTERM, &action, nullptr);

	std::vector<View> specs;
	if (parse_args(argc, argv, &specs) < 0)
		return 1;

	struct display *display = display_init(0);
	if (!display) {
		std::cerr << "vvcam-libcamera-drm: 没有可用的显示\n";
		return 1;
	}

	for (View &view : specs) {
		if (view.width == 0)
			view.width = display->width;
		if (view.height == 0)
			view.height = display->height;
		if (view.width > 1920)
			view.width = 1920;
		if (view.height > 1080)
			view.height = 1080;
		view.width = align_down(view.width, 16);
		view.height = align_down(view.height, 8);
		if (view.x < 0 || view.y < 0) {
			if (specs.size() == 1) {
				view.x = display->width > view.width ?
					 static_cast<int>((display->width - view.width) / 2) : 0;
				view.y = display->height > view.height ?
					 static_cast<int>((display->height - view.height) / 2) : 0;
			} else {
				if (view.x < 0)
					view.x = 0;
				if (view.y < 0)
					view.y = 0;
			}
		}
		if (view.width < 32 || view.height < 16 ||
		    view.x + static_cast<int>(view.width) > static_cast<int>(display->width) ||
		    view.y + static_cast<int>(view.height) > static_cast<int>(display->height)) {
			std::cerr << "vvcam-libcamera-drm: /dev/video" << view.video
				  << " 的位置 " << view.x << "," << view.y
				  << " 尺寸 " << view.width << "x" << view.height
				  << " 超出屏幕 " << display->width << "x" << display->height << "\n";
			display_exit(display);
			return 1;
		}
	}

	CameraManager manager;
	if (manager.start() < 0) {
		std::cerr << "vvcam-libcamera-drm: CameraManager 启动失败\n";
		display_exit(display);
		return 1;
	}

	std::vector<Running> running;
	auto finish = [&]() {
		g_stop = 1;
		for (Running &slot : running) {
			if (slot.started && slot.camera)
				slot.camera->stop();
			slot.started = false;
			slot.requests.clear();
			slot.allocator.reset();
			slot.preview.reset();
			if (slot.camera) {
				slot.camera->release();
				slot.camera.reset();
			}
		}
		manager.stop();
	};

	for (const View &spec : specs) {
		Running slot;
		slot.view = spec;
		const std::string name_path = "/sys/class/video4linux/video" +
					      std::to_string(spec.video) + "/name";
		if (!read_first_line(name_path, &slot.entity) ||
		    !slot.entity.starts_with("vvcam-video.")) {
			std::cerr << "vvcam-libcamera-drm: /dev/video" << spec.video
				  << " 不是 vvcam 采集节点";
			if (!slot.entity.empty())
				std::cerr << "（" << slot.entity << "）";
			std::cerr << "\n";
			finish();
			display_exit(display);
			return 1;
		}
		slot.camera = manager.get(slot.entity);
		if (!slot.camera || slot.camera->acquire() < 0) {
			std::cerr << "vvcam-libcamera-drm: 打开 " << slot.entity << " 失败\n";
			finish();
			display_exit(display);
			return 1;
		}
		slot.config = slot.camera->generateConfiguration({ StreamRole::Viewfinder });
		if (!slot.config || slot.config->empty()) {
			std::cerr << "vvcam-libcamera-drm: " << slot.entity << " 没有可用的流配置\n";
			finish();
			display_exit(display);
			return 1;
		}
		StreamConfiguration &cfg = slot.config->at(0);
		cfg.pixelFormat = formats::NV12;
		cfg.size = Size(spec.width, spec.height);
		if (slot.config->validate() == CameraConfiguration::Invalid) {
			std::cerr << "vvcam-libcamera-drm: " << slot.entity << " 配置无效\n";
			finish();
			display_exit(display);
			return 1;
		}
		slot.view.width = cfg.size.width;
		slot.view.height = cfg.size.height;
		if (slot.view.x + static_cast<int>(slot.view.width) > static_cast<int>(display->width) ||
		    slot.view.y + static_cast<int>(slot.view.height) > static_cast<int>(display->height)) {
			std::cerr << "vvcam-libcamera-drm: " << slot.entity
				  << " 对齐后的尺寸超出屏幕\n";
			finish();
			display_exit(display);
			return 1;
		}
		slot.plane = display_get_plane(display, DRM_FORMAT_NV12);
		if (!slot.plane) {
			std::cerr << "vvcam-libcamera-drm: 没有足够的 NV12 图层\n";
			finish();
			display_exit(display);
			return 1;
		}
		slot.buffer = display_allocate_buffer(slot.plane, slot.view.width, slot.view.height);
		if (!slot.buffer) {
			std::cerr << "vvcam-libcamera-drm: 分配显示 buffer 失败\n";
			finish();
			display_exit(display);
			return 1;
		}
		if (slot.camera->configure(slot.config.get()) < 0) {
			std::cerr << "vvcam-libcamera-drm: " << slot.entity << " configure 失败\n";
			finish();
			display_exit(display);
			return 1;
		}
		Stream *stream = cfg.stream();
		slot.allocator = std::make_unique<FrameBufferAllocator>(slot.camera);
		if (slot.allocator->allocate(stream) < 0) {
			std::cerr << "vvcam-libcamera-drm: 分配采集 buffer 失败\n";
			finish();
			display_exit(display);
			return 1;
		}
		slot.preview = std::make_unique<Preview>(slot.buffer, slot.view.width,
							 slot.view.height, cfg.stride);
		slot.preview->camera_ = slot.camera.get();
		slot.camera->requestCompleted.connect(slot.preview.get(), &Preview::onRequest);
		for (const std::unique_ptr<FrameBuffer> &fb : slot.allocator->buffers(stream)) {
			std::unique_ptr<Request> request = slot.camera->createRequest();
			if (!request || request->addBuffer(stream, fb.get()) < 0) {
				std::cerr << "vvcam-libcamera-drm: 创建 request 失败\n";
				finish();
				display_exit(display);
				return 1;
			}
			slot.requests.push_back(std::move(request));
		}
		std::cout << "camera " << slot.entity
			  << " /dev/video" << slot.view.video
			  << " " << slot.view.width << "x" << slot.view.height
			  << " at " << slot.view.x << "," << slot.view.y << "\n";
		running.push_back(std::move(slot));
	}

	for (Running &slot : running) {
		if (slot.camera->start() < 0) {
			std::cerr << "vvcam-libcamera-drm: start " << slot.entity << " 失败\n";
			finish();
			for (Running &item : running) {
				if (item.buffer)
					display_free_buffer(item.buffer);
				if (item.plane)
					display_free_plane(item.plane);
			}
			display_exit(display);
			return 1;
		}
		slot.started = true;
	}
	for (Running &slot : running) {
		for (std::unique_ptr<Request> &request : slot.requests) {
			if (slot.camera->queueRequest(request.get()) < 0) {
				std::cerr << "vvcam-libcamera-drm: queueRequest 失败\n";
				finish();
				for (Running &item : running) {
					if (item.buffer)
						display_free_buffer(item.buffer);
					if (item.plane)
						display_free_plane(item.plane);
				}
				display_exit(display);
				return 1;
			}
		}
	}
	StdinKeys keys;
	keys.enable();
	std::cout << "按 q 或 Ctrl-C 退出\n";
	std::cout.flush();

	unsigned shown = 0;
	while (!g_stop) {
		if (keys.quit_requested())
			break;
		bool dirty = false;
		for (Running &slot : running) {
			if (slot.preview->take())
				dirty = true;
		}
		if (!dirty) {
			usleep(5000);
			continue;
		}
		bool modeset = false;
		bool commit_ok = true;
		for (Running &slot : running) {
			if (!slot.preview->seen_)
				continue;
			if (modeset)
				slot.plane->first = false;
			if (display_update_buffer(slot.buffer, slot.view.x, slot.view.y) != 0) {
				commit_ok = false;
				break;
			}
			modeset = true;
		}
		if (!commit_ok || display_commit(display) != 0) {
			std::cerr << "vvcam-libcamera-drm: 送显失败\n";
			break;
		}
		shown++;
		if (shown == 1 || shown % 30 == 0) {
			std::cout << "shown " << shown;
			for (const Running &slot : running) {
				std::cout << " video" << slot.view.video << "="
					  << slot.preview->copied.load(std::memory_order_relaxed);
			}
			std::cout << "\n";
			std::cout.flush();
		}
		if (wait_flip(display) < 0) {
			std::cerr << "vvcam-libcamera-drm: 等待 vsync 失败\n";
			break;
		}
	}

	finish();
	for (Running &slot : running) {
		if (slot.buffer)
			display_free_buffer(slot.buffer);
		if (slot.plane)
			display_free_plane(slot.plane);
	}
	display_exit(display);
	std::cout << "vvcam-libcamera-drm: 退出，已显示 " << shown << " 帧\n";
	std::cout.flush();
	return shown > 0 ? 0 : 1;
}
