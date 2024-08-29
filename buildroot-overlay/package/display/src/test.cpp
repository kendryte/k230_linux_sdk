#include <cstdio>
#include <drm/drm_fourcc.h>
#include <iostream>
#include <pipeline.hpp>

using namespace pipeline;

int main(void) {
    unsigned width = 480;
    unsigned height = 320;
    auto display = Display::create();
    if (display) {
        printf("fd: %d\n", display->fd_to_select());
        return 0;
    }
    /*
    if (auto display = Display::create()) {
        printf("fd: %d\n", display->fd_to_select());
        if (!display->createChannel(width, height, DRM_FORMAT_NV12)) {
            std::cerr << "create display channel error" << std::endl;
            return -1;
        }
        if (auto vicap = Vicap::create(1, width, height, V4L2_PIX_FMT_NV12)) {
            auto pipe = Pipeline();
            if (!pipe.link(vicap.value(), display.value())) {
                std::cerr << "link error" << std::endl;
                return -1;
            }
            pipe.run();
        }
    }*/
}
