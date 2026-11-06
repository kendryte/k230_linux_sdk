/*
 * Acquire and configure a list of vvcam nodes, hold them open, then release.
 * Does not start streaming.
 *
 *   vvcam-libcamera-hold -d 1,2,3
 *   vvcam-libcamera-hold -d 1,4,7 480 800
 */

#include <chrono>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include <getopt.h>
#include <stdlib.h>

#include <libcamera/camera.h>
#include <libcamera/camera_manager.h>
#include <libcamera/formats.h>
#include <libcamera/stream.h>

using namespace libcamera;

static unsigned align_down(unsigned value, unsigned align)
{
	return value & ~(align - 1);
}

static void usage()
{
	std::cerr << "usage: vvcam-libcamera-hold -d video[,video...] [--hflip 0|1] [--vflip 0|1] [width height]\n";
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

int main(int argc, char **argv)
{
	static const struct option longopts[] = {
		{ "hflip", required_argument, nullptr, 'M' },
		{ "vflip", required_argument, nullptr, 'F' },
		{ nullptr, 0, nullptr, 0 },
	};
	std::string list;
	int hflip = 0;
	int vflip = 0;
	int opt;
	while ((opt = getopt_long(argc, argv, "d:M:F:", longopts, nullptr)) != -1) {
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
				hflip = static_cast<int>(flip);
			else
				vflip = static_cast<int>(flip);
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
	if (width < 32 || height < 16) {
		std::cerr << "vvcam-libcamera-hold: 分辨率太小\n";
		return 1;
	}

	CameraManager manager;
	if (manager.start() < 0) {
		std::cerr << "vvcam-libcamera-hold: CameraManager 启动失败\n";
		return 1;
	}

	struct Held {
		std::shared_ptr<Camera> camera;
		int index = 0;
	};
	std::vector<Held> held;

	auto release_all = [&]() {
		while (!held.empty()) {
			Held item = std::move(held.back());
			held.pop_back();
			if (item.camera) {
				item.camera->release();
				item.camera.reset();
			}
		}
		manager.stop();
	};

	for (int index : videos) {
		std::string entity;
		const std::string name_path = "/sys/class/video4linux/video" +
					      std::to_string(index) + "/name";
		if (!read_first_line(name_path, &entity) ||
		    !entity.starts_with("vvcam-video.")) {
			std::cerr << "vvcam-libcamera-hold: /dev/video" << index
				  << " 不是 vvcam 采集节点\n";
			release_all();
			return 1;
		}

		std::shared_ptr<Camera> camera = manager.get(entity);
		if (!camera) {
			std::cerr << "vvcam-libcamera-hold: libcamera 没有注册 /dev/video"
				  << index << "（" << entity << "）\n";
			release_all();
			return 1;
		}
		if (camera->acquire() < 0) {
			std::cerr << "vvcam-libcamera-hold: 打开 " << entity << " 失败\n";
			release_all();
			return 1;
		}

		std::unique_ptr<CameraConfiguration> config =
			camera->generateConfiguration({ StreamRole::Viewfinder });
		if (!config || config->empty()) {
			std::cerr << "vvcam-libcamera-hold: " << entity << " 没有可用的流配置\n";
			camera->release();
			release_all();
			return 1;
		}
		StreamConfiguration &cfg = config->at(0);
		cfg.pixelFormat = formats::NV12;
		cfg.size = Size(width, height);
		config->orientation = flip_orientation(hflip, vflip);
		if (config->validate() == CameraConfiguration::Invalid) {
			std::cerr << "vvcam-libcamera-hold: " << entity << " 配置无效\n";
			camera->release();
			release_all();
			return 1;
		}
		if (camera->configure(config.get()) < 0) {
			std::cerr << "vvcam-libcamera-hold: " << entity << " configure 失败\n";
			camera->release();
			release_all();
			return 1;
		}

		std::cout << "hold /dev/video" << index << " " << entity
			  << " " << cfg.size.width << "x" << cfg.size.height
			  << " NV12 hflip=" << hflip << " vflip=" << vflip << "\n";
		std::cout.flush();
		held.push_back(Held{ std::move(camera), index });
	}

	std::this_thread::sleep_for(std::chrono::seconds(1));
	std::cout << "release " << held.size() << " camera(s)\n";
	release_all();
	return 0;
}
