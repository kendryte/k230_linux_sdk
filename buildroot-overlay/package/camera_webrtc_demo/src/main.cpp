#include "camera_webrtc_demo.h"
#include <iostream>
#include <atomic>
#include <unistd.h>
#include <signal.h>

std::atomic<bool> g_exit_flag{false};

static void sigHandler(int sig_no) {
    g_exit_flag.store(true);
    printf("exit_flag true\n");
}

static void Usage() {
    std::cout << "Usage: ./camera_webrtc_demo [-H] [-w <width>] [-h <height>] [-b <bitrate_kbps>] [-p <port>] [-l <enable_log>] [-r <osd_region>]" << std::endl;
    std::cout << "-H: display this help message" << std::endl;
    std::cout << "-w: video encoder width, default 1280" << std::endl;
    std::cout << "-h: video encoder height, default 720" << std::endl;
    std::cout << "-b: video encoder bitrate(kbps), default 2000" << std::endl;
    std::cout << "-p: http server port, default 8080" << std::endl;
    std::cout << "-l: enable log, default 0" << std::endl;
    std::cout << "-r: osd region count, default 0" << std::endl;
    exit(-1);
}

int main(int argc, char *argv[]) {
    signal(SIGINT, sigHandler);
    signal(SIGPIPE, SIG_IGN);
    g_exit_flag.store(false);

    KdMediaInputConfig config;
    int port = 8080;

    int result;
    opterr = 0;
    while ((result = getopt(argc, argv, "Hw:h:b:p:l:r:")) != -1) {
        switch (result) {
        case 'H': Usage(); break;
        case 'w': config.venc_width = atoi(optarg); break;
        case 'h': config.venc_height = atoi(optarg); break;
        case 'b': config.bitrate_kbps = atoi(optarg); break;
        case 'p': port = atoi(optarg); break;
        case 'l': config.enable_log = atoi(optarg); break;
        case 'r': config.osd_region = atoi(optarg); break;
        default: Usage(); break;
        }
    }

    printf("Config: %dx%d @ %dkbps, HTTP port %d, log=%d, osd=%d\n",
           config.venc_width, config.venc_height, config.bitrate_kbps, port, config.enable_log, config.osd_region);

    MyCameraWebRtcDemo *demo = new MyCameraWebRtcDemo();
    if (!demo || demo->Init(config, port) < 0) {
        std::cout << "Init failed." << std::endl;
        return -1;
    }

    demo->Start();

    while (!g_exit_flag) {
        usleep(1000 * 100);
    }

    demo->Stop();
    demo->DeInit();
    delete demo;
    return 0;
}
