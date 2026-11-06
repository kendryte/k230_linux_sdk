/*
 * Capture NV12 from several vvcam nodes in one process.
 *
 *   vvcam-libcamera-grab -d 1,4
 *   vvcam-libcamera-grab -d 1
 *
 * With two or more nodes, the first is stopped after each has 5 frames.
 * Every remaining node must then deliver another frame.
 */

#include <chrono>
#include <cstdio>
#include <condition_variable>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <mutex>
#include <optional>
#include <string>
#include <vector>

#include <fcntl.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

#include <linux/videodev2.h>

#include "vvcam_cid.h"

#include <libcamera/camera.h>
#include <libcamera/camera_manager.h>
#include <libcamera/control_ids.h>
#include <libcamera/controls.h>
#include <libcamera/formats.h>
#include <libcamera/framebuffer.h>
#include <libcamera/framebuffer_allocator.h>
#include <libcamera/request.h>
#include <libcamera/stream.h>

using namespace libcamera;

static constexpr unsigned kFrames = 5;
static int g_exposureUs = -1;
static float g_gain = 1.0f;
static bool g_gainSet = false;
static int g_awb = 1;
static bool g_awbSet = false;
static int g_bright = 0;
static bool g_brightSet = false;
static int g_contrast = 0;
static bool g_contrastSet = false;
static int g_saturation = 0;
static bool g_saturationSet = false;
static unsigned g_sensorWidth = 0;
static unsigned g_sensorHeight = 0;
static unsigned g_sensorFps = 0;
static bool g_sensorTarget = false;
static std::string g_calibDir;
static int g_hflip = 0;
static int g_vflip = 0;

static unsigned align_down(unsigned value, unsigned align)
{
	return value & ~(align - 1);
}

static void usage()
{
	std::cerr << "usage: vvcam-libcamera-grab -d video[,video...] [-e us] [-g gain] [-a 0|1] [-b bright] [-k contrast] [-u saturation] [-S WxH@fps] [-C dir] [--hflip 0|1] [--vflip 0|1] [width height]\n";
}

static Orientation flip_orientation(int hflip, int vflip)
{
	if (hflip && vflip)
		return Orientation::Rotate180;
	if (hflip)
		return Orientation::Rotate0Mirror;
	if (vflip)
		return Orientation::Rotate180Mirror;
	return Orientation::Rotate0;
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

static bool parse_videos(const std::string &text, std::vector<int> *videos)
{
	videos->clear();
	std::string::size_type begin = 0;
	while (begin <= text.size()) {
		const std::string::size_type comma = text.find(',', begin);
		const std::string item = text.substr(begin, comma == std::string::npos ? std::string::npos : comma - begin);
		if (item.empty())
			return false;
		char *end = nullptr;
		const long value = strtol(item.c_str(), &end, 10);
		if (!end || *end || value < 0 || value > 63)
			return false;
		videos->push_back(static_cast<int>(value));
		if (comma == std::string::npos)
			break;
		begin = comma + 1;
	}
	return !videos->empty();
}

/* Scene dir is read when CREATE loads calib, so this runs before acquire(). */
static bool set_scene_dir(int video, const std::string &dir)
{
	char path[128];
	v4l2_ext_control ctrl[2] = {};
	v4l2_ext_controls ctrls = {};
	const std::string node = "/dev/video" + std::to_string(video);
	int fd;
	int ret;

	if (dir.size() >= sizeof(path))
		return false;
	memset(path, 0, sizeof(path));
	memcpy(path, dir.c_str(), dir.size());

	ctrl[0].id = VVCAM_CID_SCENE_CALIB_DIR;
	ctrl[0].size = static_cast<__u32>(strlen(path) + 1);
	ctrl[0].string = path;
	ctrl[1].id = VVCAM_CID_SCENE_MODE;
	ctrl[1].value = 0;

	ctrls.ctrl_class = V4L2_CTRL_CLASS_USER;
	ctrls.count = 2;
	ctrls.controls = ctrl;

	fd = open(node.c_str(), O_RDWR | O_CLOEXEC);
	if (fd < 0)
		return false;
	ret = ioctl(fd, VIDIOC_S_EXT_CTRLS, &ctrls);
	if (ret == 0) {
		v4l2_ext_control sensor[4] = {};
		v4l2_ext_controls sensorCtrls = {};

		/*
		 * CREATE loads calib for the sensor mode. That CREATE is
		 * ENUM_FMT inside acquire(), so the target has to be pending
		 * on this node before the fd is closed.
		 */
		sensor[0].id = VVCAM_CID_SENSOR_ENABLE;
		sensor[0].value = g_sensorTarget ? 1 : 0;
		sensor[1].id = VVCAM_CID_SENSOR_WIDTH;
		sensor[1].value = g_sensorTarget ? g_sensorWidth : 0;
		sensor[2].id = VVCAM_CID_SENSOR_HEIGHT;
		sensor[2].value = g_sensorTarget ? g_sensorHeight : 0;
		sensor[3].id = VVCAM_CID_SENSOR_FPS;
		sensor[3].value = g_sensorTarget ? g_sensorFps : 0;
		sensorCtrls.ctrl_class = V4L2_CTRL_CLASS_USER;
		sensorCtrls.count = 4;
		sensorCtrls.controls = sensor;
		ret = ioctl(fd, VIDIOC_S_EXT_CTRLS, &sensorCtrls);
	}
	close(fd);
	return ret == 0;
}

struct GrabCamera {
	std::shared_ptr<Camera> camera;
	int index = 0;
	std::string entity;
	std::unique_ptr<CameraConfiguration> config;
	std::unique_ptr<FrameBufferAllocator> allocator;
	std::vector<std::unique_ptr<Request>> requests;
	Stream *stream = nullptr;
	unsigned width = 0;
	unsigned height = 0;
	unsigned frameSize = 0;
	unsigned saved = 0;
	unsigned completed = 0;
	double firstMean = -1;
	double firstChromaU = -1;
	double firstChromaV = -1;
	bool requeue = true;
	bool manual = false;
	std::mutex mutex;
	std::condition_variable cv;
};

static void chroma_var(const uint8_t *uv, unsigned uvBytes, double *varU, double *varV)
{
	uint64_t sumU = 0;
	uint64_t sumV = 0;
	uint64_t sqU = 0;
	uint64_t sqV = 0;
	unsigned count = 0;

	for (unsigned i = 0; i + 1 < uvBytes; i += 2) {
		const unsigned u = uv[i];
		const unsigned v = uv[i + 1];
		sumU += u;
		sumV += v;
		sqU += static_cast<uint64_t>(u) * u;
		sqV += static_cast<uint64_t>(v) * v;
		count++;
	}
	if (!count) {
		*varU = 0;
		*varV = 0;
		return;
	}
	const double meanU = static_cast<double>(sumU) / count;
	const double meanV = static_cast<double>(sumV) / count;
	*varU = static_cast<double>(sqU) / count - meanU * meanU;
	*varV = static_cast<double>(sqV) / count - meanV * meanV;
}

static bool save_frame(GrabCamera *grab, FrameBuffer *buffer)
{
	if (buffer->planes().empty())
		return false;

	char path[64];
	snprintf(path, sizeof(path), "/tmp/grab-video%d-%06u.nv12",
		 grab->index, grab->saved);
	std::ofstream output(path, std::ios::binary);
	unsigned written = 0;
	bool meanDone = grab->saved != 0;
	bool chromaDone = grab->saved != 0;

	for (const FrameBuffer::Plane &plane : buffer->planes()) {
		const size_t mapLength = static_cast<size_t>(plane.offset) + plane.length;
		void *address = mmap(nullptr, mapLength, PROT_READ, MAP_SHARED,
				     plane.fd.get(), 0);
		if (address == MAP_FAILED)
			return false;
		const uint8_t *bytes = static_cast<const uint8_t *>(address) + plane.offset;
		output.write(reinterpret_cast<const char *>(bytes), plane.length);
		if (grab->width && grab->height && (!meanDone || !chromaDone)) {
			const unsigned yCount = grab->width * grab->height;
			const unsigned uvBytes = yCount / 2;
			if (!meanDone && plane.length >= yCount) {
				uint64_t sum = 0;
				const unsigned count = std::min(yCount, static_cast<unsigned>(plane.length));
				for (unsigned i = 0; i < count; i++)
					sum += bytes[i];
				grab->firstMean = count ? static_cast<double>(sum) / count : 0;
				meanDone = true;
			}
			if (!chromaDone && uvBytes) {
				const uint8_t *uv = nullptr;
				if (plane.length >= yCount + uvBytes)
					uv = bytes + yCount;
				else if (meanDone && plane.length >= uvBytes &&
					 static_cast<unsigned>(plane.length) < yCount)
					uv = bytes;
				if (uv) {
					chroma_var(uv, uvBytes, &grab->firstChromaU, &grab->firstChromaV);
					chromaDone = true;
				}
			}
		}
		written += plane.length;
		munmap(address, mapLength);
	}

	if (!output.good() || written != grab->frameSize) {
		std::cerr << "vvcam-libcamera-grab: " << path
			  << " 写入 " << written
			  << "，期望 " << grab->frameSize << "\n";
		return false;
	}
	grab->saved++;
	std::cout << "saved " << path << " " << written << " bytes\n";
	std::cout.flush();
	return true;
}

static void on_request(Request *request)
{
	GrabCamera *grab = reinterpret_cast<GrabCamera *>(request->cookie());
	if (request->status() == Request::RequestCancelled) {
		std::lock_guard<std::mutex> lock(grab->mutex);
		grab->cv.notify_all();
		return;
	}

	FrameBuffer *buffer = request->findBuffer(grab->stream);
	bool queueAgain = false;
	{
		std::lock_guard<std::mutex> lock(grab->mutex);
		grab->completed++;
		if (grab->saved == 0) {
			const std::optional<int32_t> exposure =
				request->metadata().get(controls::ExposureTime);
			const std::optional<float> gain =
				request->metadata().get(controls::AnalogueGain);
			if (exposure && gain) {
				char line[80];
				snprintf(line, sizeof(line),
					 "meta exposure %d gain %.3f\n",
					 *exposure, static_cast<double>(*gain));
				std::cout << line;
				std::cout.flush();
			} else {
				std::cerr << "vvcam-libcamera-grab: 第 1 帧没有曝光 metadata\n";
			}
		}
		if (grab->saved < kFrames && buffer)
			save_frame(grab, buffer);
		queueAgain = grab->requeue;
		grab->cv.notify_all();
	}
	if (!queueAgain)
		return;
	request->reuse(Request::ReuseBuffers);
	{
		std::lock_guard<std::mutex> lock(grab->mutex);
		if (!grab->requeue)
			return;
	}
	if (grab->manual)
		request->controls().set(controls::AeEnable, false);
	if (grab->camera->queueRequest(request) < 0) {
		std::lock_guard<std::mutex> lock(grab->mutex);
		grab->requeue = false;
		grab->cv.notify_all();
	}
}

static bool wait_until(GrabCamera *grab, unsigned saved, unsigned completed)
{
	std::unique_lock<std::mutex> lock(grab->mutex);
	return grab->cv.wait_for(lock, std::chrono::seconds(8), [&]() {
		return grab->saved >= saved && grab->completed >= completed;
	});
}

int main(int argc, char **argv)
{
	static const struct option longopts[] = {
		{ "hflip", required_argument, nullptr, 'M' },
		{ "vflip", required_argument, nullptr, 'F' },
		{ nullptr, 0, nullptr, 0 },
	};
	std::string list;
	int opt;
	while ((opt = getopt_long(argc, argv, "d:e:g:a:b:k:u:S:C:M:F:", longopts, nullptr)) != -1) {
		if (opt == 'd')
			list = optarg;
		else if (opt == 'M' || opt == 'F') {
			char *end = nullptr;
			const long flip = strtol(optarg, &end, 10);
			if (!end || *end || (flip != 0 && flip != 1)) {
				usage();
				return 1;
			}
			if (opt == 'M')
				g_hflip = static_cast<int>(flip);
			else
				g_vflip = static_cast<int>(flip);
		}
		else if (opt == 'e')
			g_exposureUs = atoi(optarg);
		else if (opt == 'g') {
			char *end = nullptr;
			const float gain = strtof(optarg, &end);
			if (!end || *end || !(gain > 0.0f)) {
				usage();
				return 1;
			}
			g_gain = gain;
			g_gainSet = true;
		} else if (opt == 'a') {
			char *end = nullptr;
			const long awb = strtol(optarg, &end, 10);
			if (!end || *end || (awb != 0 && awb != 1)) {
				usage();
				return 1;
			}
			g_awb = static_cast<int>(awb);
			g_awbSet = true;
		} else if (opt == 'b') {
			char *end = nullptr;
			const long bright = strtol(optarg, &end, 10);
			if (!end || *end || bright < -128 || bright > 127) {
				usage();
				return 1;
			}
			g_bright = static_cast<int>(bright);
			g_brightSet = true;
		} else if (opt == 'k') {
			char *end = nullptr;
			const long contrast = strtol(optarg, &end, 10);
			if (!end || *end || contrast < 0 || contrast > 255) {
				usage();
				return 1;
			}
			g_contrast = static_cast<int>(contrast);
			g_contrastSet = true;
		} else if (opt == 'u') {
			char *end = nullptr;
			const long saturation = strtol(optarg, &end, 10);
			if (!end || *end || saturation < 0 || saturation > 255) {
				usage();
				return 1;
			}
			g_saturation = static_cast<int>(saturation);
			g_saturationSet = true;
		} else if (opt == 'S') {
			unsigned width = 0, height = 0, fps = 0;
			if (sscanf(optarg, "%ux%u@%u", &width, &height, &fps) != 3 ||
			    !width || !height || !fps) {
				usage();
				return 1;
			}
			g_sensorWidth = width;
			g_sensorHeight = height;
			g_sensorFps = fps;
			g_sensorTarget = true;
		} else if (opt == 'C') {
			if (!optarg || strlen(optarg) >= 128) {
				usage();
				return 1;
			}
			g_calibDir = optarg;
		} else {
			usage();
			return 1;
		}
	}
	std::vector<int> videos;
	if (!parse_videos(list, &videos)) {
		usage();
		return 1;
	}

	unsigned width = 480;
	unsigned height = 800;
	if (argc - optind == 2) {
		width = static_cast<unsigned>(atoi(argv[optind]));
		height = static_cast<unsigned>(atoi(argv[optind + 1]));
	} else if (argc != optind) {
		usage();
		return 1;
	}
	width = align_down(width, 16);
	height = align_down(height, 8);

	CameraManager manager;
	if (manager.start() < 0) {
		std::cerr << "vvcam-libcamera-grab: CameraManager 启动失败\n";
		return 1;
	}

	std::vector<std::unique_ptr<GrabCamera>> grabs;
	auto release_all = [&]() {
		for (auto it = grabs.rbegin(); it != grabs.rend(); ++it) {
			GrabCamera *grab = it->get();
			if (!grab->camera)
				continue;
			{
				std::lock_guard<std::mutex> lock(grab->mutex);
				grab->requeue = false;
			}
			grab->camera->stop();
			grab->requests.clear();
			grab->allocator.reset();
			grab->camera->release();
			grab->camera.reset();
		}
		grabs.clear();
		manager.stop();
	};

	for (int index : videos) {
		auto grab = std::make_unique<GrabCamera>();
		grab->index = index;
		const std::string name_path = "/sys/class/video4linux/video" +
					      std::to_string(index) + "/name";
		if (!read_first_line(name_path, &grab->entity) ||
		    !grab->entity.starts_with("vvcam-video.")) {
			std::cerr << "vvcam-libcamera-grab: /dev/video" << index
				  << " 不是 vvcam 采集节点\n";
			release_all();
			return 1;
		}
		if (grabs.empty()) {
			if (!set_scene_dir(index, g_calibDir)) {
				std::cerr << "vvcam-libcamera-grab: scene 配置失败\n";
				release_all();
				return 1;
			}
			std::cout << "scene calib "
				  << (g_calibDir.empty() ? "/etc/vvcam" : g_calibDir)
				  << "\n";
			std::cout.flush();
		}
		grab->camera = manager.get(grab->entity);
		if (!grab->camera || grab->camera->acquire() < 0) {
			std::cerr << "vvcam-libcamera-grab: 打开 " << grab->entity << " 失败\n";
			release_all();
			return 1;
		}
		grab->config = grab->camera->generateConfiguration({ StreamRole::Viewfinder });
		if (!grab->config || grab->config->empty()) {
			std::cerr << "vvcam-libcamera-grab: 没有可用的流配置\n";
			release_all();
			return 1;
		}
		StreamConfiguration &cfg = grab->config->at(0);
		cfg.pixelFormat = formats::NV12;
		cfg.size = Size(width, height);
		if (grabs.empty() && g_sensorTarget) {
			SensorConfiguration sensor;
			sensor.outputSize = Size(g_sensorWidth, g_sensorHeight);
			sensor.bitDepth = g_sensorFps;
			grab->config->sensorConfig = sensor;
		}
		grab->config->orientation = flip_orientation(g_hflip, g_vflip);
		if (grabs.empty()) {
			std::cout << "flip hflip=" << g_hflip
				  << " vflip=" << g_vflip << "\n";
			std::cout.flush();
		}
		if (grab->config->validate() == CameraConfiguration::Invalid ||
		    grab->camera->configure(grab->config.get()) < 0) {
			std::cerr << "vvcam-libcamera-grab: " << grab->entity << " configure 失败\n";
			release_all();
			return 1;
		}
		const ControlInfo &exposure = grab->camera->controls().at(&controls::ExposureTime);
		std::cout << "exposure range " << exposure.min().get<int32_t>()
			  << ".." << exposure.max().get<int32_t>() << "\n";
		const ControlInfo &gain = grab->camera->controls().at(&controls::AnalogueGain);
		char gainRange[64];
		snprintf(gainRange, sizeof(gainRange), "gain range %.3f..%.3f\n",
			 gain.min().get<float>(), gain.max().get<float>());
		std::cout << gainRange;
		std::cout.flush();
		grab->stream = cfg.stream();
		grab->width = cfg.size.width;
		grab->height = cfg.size.height;
		grab->frameSize = cfg.frameSize;
		grab->allocator = std::make_unique<FrameBufferAllocator>(grab->camera);
		if (grab->allocator->allocate(grab->stream) < 0) {
			std::cerr << "vvcam-libcamera-grab: 分配 buffer 失败\n";
			release_all();
			return 1;
		}
		grab->manual = grabs.empty() && g_exposureUs >= 0;
		grab->camera->requestCompleted.connect(on_request);
		for (const std::unique_ptr<FrameBuffer> &fb : grab->allocator->buffers(grab->stream)) {
			std::unique_ptr<Request> request = grab->camera->createRequest(reinterpret_cast<uint64_t>(grab.get()));
			if (!request || request->addBuffer(grab->stream, fb.get()) < 0) {
				std::cerr << "vvcam-libcamera-grab: 创建 request 失败\n";
				release_all();
				return 1;
			}
			if (grab->manual) {
				request->controls().set(controls::AeEnable, false);
				request->controls().set(controls::ExposureTime, static_cast<int32_t>(g_exposureUs));
			}
			if (grabs.empty() && g_gainSet)
				request->controls().set(controls::AnalogueGain, g_gain);
			if (grabs.empty() && g_awbSet)
				request->controls().set(controls::AwbEnable, g_awb != 0);
			if (grabs.empty() && g_brightSet)
				request->controls().set(controls::Brightness,
							 static_cast<float>(g_bright) / 128.0f);
			if (grabs.empty() && g_contrastSet)
				request->controls().set(controls::Contrast,
							 static_cast<float>(g_contrast) / 128.0f);
			if (grabs.empty() && g_saturationSet)
				request->controls().set(controls::Saturation,
							 static_cast<float>(g_saturation) / 128.0f);
			grab->requests.push_back(std::move(request));
		}
		grabs.push_back(std::move(grab));
	}

	for (const std::unique_ptr<GrabCamera> &grab : grabs) {
		if (grab->camera->start() < 0) {
			std::cerr << "vvcam-libcamera-grab: start " << grab->entity << " 失败\n";
			release_all();
			return 1;
		}
	}
	for (const std::unique_ptr<GrabCamera> &grab : grabs) {
		for (const std::unique_ptr<Request> &request : grab->requests) {
			if (grab->camera->queueRequest(request.get()) < 0) {
				std::cerr << "vvcam-libcamera-grab: queue " << grab->entity << " 失败\n";
				release_all();
				return 1;
			}
		}
	}

	for (const std::unique_ptr<GrabCamera> &grab : grabs) {
		if (!wait_until(grab.get(), kFrames, 0)) {
			std::cerr << "vvcam-libcamera-grab: " << grab->entity
				  << " 没有凑满 " << kFrames << " 帧\n";
			release_all();
			return 1;
		}
		std::cout << grab->entity << " first-frame mean Y "
			  << grab->firstMean << "\n";
		if (grab->firstChromaU >= 0 && grab->firstChromaV >= 0) {
			char line[96];
			snprintf(line, sizeof(line),
				 "%s first-frame chroma var U %.3f V %.3f\n",
				 grab->entity.c_str(),
				 grab->firstChromaU, grab->firstChromaV);
			std::cout << line;
		}
	}

	if (grabs.size() >= 2) {
		GrabCamera *stopping = grabs[0].get();
		{
			std::lock_guard<std::mutex> lock(stopping->mutex);
			stopping->requeue = false;
		}
		stopping->camera->stop();
		for (size_t index = 1; index < grabs.size(); index++) {
			GrabCamera *rest = grabs[index].get();
			unsigned before = 0;
			{
				std::lock_guard<std::mutex> lock(rest->mutex);
				before = rest->completed;
			}
			if (!wait_until(rest, 0, before + 1)) {
				std::cerr << "vvcam-libcamera-grab: 停掉 /dev/video" << stopping->index
					  << " 后 /dev/video" << rest->index << " 不再出帧\n";
				release_all();
				return 1;
			}
			std::lock_guard<std::mutex> lock(rest->mutex);
			std::cout << "after stop /dev/video" << stopping->index
				  << ", /dev/video" << rest->index
				  << " completed " << rest->completed
				  << " (was " << before << ")\n";
		}
	}

	release_all();
	return 0;
}
