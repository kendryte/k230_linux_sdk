#include "camera_rtsp_demo.h"
#include <iostream>
#include <atomic>
#include <chrono>
#include <unistd.h>
#include <signal.h>
#include <thread>


std::atomic<bool> g_exit_flag{false};

static void sigHandler(int sig_no) {
    g_exit_flag.store(true);
    printf("exit_flag true\n");
}

static void Usage() {
    std::cout << "Usage: ./camera_rtsp_demo [-H] [-t <codec_type>] [-w <width>] [-h <height>] [-b <bitrate_kbps>] [-c <rtsp_type>] [-l <enable_log>]" << std::endl;
    std::cout << "-H: display this help message" << std::endl;
    std::cout << "-t: the video encoder type: h264/h265, default h264" << std::endl;
    std::cout << "-w: the video encoder width, default 1280" << std::endl;
    std::cout << "-h: the video encoder height, default 720" << std::endl;
    std::cout << "-b: the video encoder bitrate(kbps), default 2000" << std::endl;
    std::cout << "-c: the RTSP server type: 0=live555, 1=smolrtsp, default 0" << std::endl;
    std::cout << "-l: enable log, default 0" << std::endl;
    exit(-1);
}

int parse_config(int argc, char *argv[], KdMediaInputConfig &config) {
    int result;
    opterr = 0;
    while ((result = getopt(argc, argv, "H:t:w:h:b:c:l:")) != -1) {
        switch(result) {
        case 'H' : {
            Usage(); break;
        }
        case 't': {
            std::string s = optarg;
            if (s == "h264") config.video_type = KdMediaVideoType::kVideoTypeH264;
            else if (s == "h265") config.video_type = KdMediaVideoType::kVideoTypeH264;
            else Usage();
            break;
        }
        case 'w': {
            int n = atoi(optarg);
            if (n < 0) Usage();
            config.venc_width = n;
            break;
        }
        case 'h': {
            int n = atoi(optarg);
            if (n < 0) Usage();
            config.venc_height = n;
            break;
        }
        case 'b': {
            int n = atoi(optarg);
            if (n < 0) Usage();
            config.bitrate_kbps = n;
            break;
        }
        case 'c': {
            int n = atoi(optarg);
            if (n < 0 || n > 1) Usage();
            // 0=live555, 1=smolrtsp
            config.rtsp_server_type = n;
            break;
        }
        case 'l': {
            int n = atoi(optarg);
            if (n < 0 || n > 1) Usage();
            config.enable_log = n;
            break;
        }
        default: Usage(); break;
        }
    }

    printf("Config parameters:\n");
    printf("Video encoder type: %s\n", (config.video_type == KdMediaVideoType::kVideoTypeH264) ? "h264" : "h265");
    printf("Video encoder width: %d\n", config.venc_width);
    printf("Video encoder height: %d\n", config.venc_height);
    printf("Video encoder bitrate (kbps): %d\n", config.bitrate_kbps);
    printf("RTSP server type: %s\n", (config.rtsp_server_type == 0) ? "live555" : "smolrtsp");
    printf("Enable log: %s\n", (config.enable_log == 1) ? "true" : "false");
    printf("\n");
    return 0;
}

int main(int argc, char *argv[]) {
    signal(SIGINT, sigHandler);
    signal(SIGPIPE, SIG_IGN);
    g_exit_flag.store(false);

    KdMediaInputConfig config;
    parse_config(argc, argv, config);

    MyCameraRtspDemo *camera_rtsp_demo = new MyCameraRtspDemo();
    if (!camera_rtsp_demo || camera_rtsp_demo->Init(config) < 0) {
        std::cout << "KdRtspServer Init failed." << std::endl;
        return -1;
    }

    camera_rtsp_demo->Start();
    printf("camera_rtsp_demo started.\n");

    while (!g_exit_flag) {
        usleep(1000*100);
    }

    camera_rtsp_demo->Stop();
    camera_rtsp_demo->DeInit();
    delete camera_rtsp_demo;
    return 0;
}
