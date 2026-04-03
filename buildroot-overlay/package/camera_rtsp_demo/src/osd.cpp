#include "osd.h"
#include <fcntl.h>
#include <cstring>
#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <iostream>
#include <memory>
#include <mutex>
#include <atomic>
#include <fstream>
#include <linux/videodev2.h>
#include <stddef.h>
#include <errno.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/poll.h>
#include <stdint.h>
#include <ctype.h>
#include <sys/stat.h>

static AVFilterContext *FindNonai2dContext(AVFilterGraph *graph) {
    if (!graph) {
        return nullptr;
    }

    AVFilterContext *ctx = avfilter_graph_get_filter(graph, "nonai2d");
    if (ctx) {
        return ctx;
    }

    ctx = avfilter_graph_get_filter(graph, "Parsed_nonai2d_osd_0");
    if (ctx) {
        return ctx;
    }

    for (unsigned i = 0; i < graph->nb_filters; ++i) {
        AVFilterContext *it = graph->filters[i];
        if (!it || !it->filter || !it->filter->name) {
            continue;
        }
        if (strcmp(it->filter->name, "nonai2d_osd") == 0) {
            return it;
        }
    }

    return nullptr;
}

OsdManager::OsdManager() {
    graph_ = nullptr;
    main_src_ = nullptr;
    osd_ctx_ = nullptr;
    sink_ = nullptr;
    width_ = 0;
    height_ = 0;
    time_base_ = {0, 0};
    inited_ = false;
    frame_cnt_ = 0;
}

OsdManager::~OsdManager() {
    Deinit();
}

bool OsdManager::IsReady() const {
    return inited_;
}

void OsdManager::Deinit() {
    if (!inited_) {
        return;
    }

    if (graph_) {
        avfilter_graph_free(&graph_);
        graph_ = nullptr;
    }

    main_src_ = nullptr;
    osd_ctx_ = nullptr;
    sink_ = nullptr;
    inited_ = false;
}

int OsdManager::Init(int width, int height, AVRational time_base, 
                     const char* in_mem_type, const char* out_mem_type) {
    int ret;
    char args[256];
    char region_args[384];
    std::string filter_desc;
    const AVFilter *buffer = avfilter_get_by_name("buffer");
    const AVFilter *buffersink = avfilter_get_by_name("buffersink");
    AVFilterInOut *inputs = NULL;
    AVFilterInOut *outputs = NULL;
    std::string nonai2d_dev;

    osd_config_.device = "";
    auto detect_nonai2d_device = []() -> std::string {
        for (int i = 0; i < 64; ++i) {
            char name_path[128];
            char dev_path[32];
            snprintf(name_path, sizeof(name_path), "/sys/class/video4linux/video%d/name", i);
            snprintf(dev_path, sizeof(dev_path), "/dev/video%d", i);

            std::ifstream ifs(name_path);
            if (!ifs.is_open()) {
                continue;
            }

            std::string name;
            std::getline(ifs, name);
            if (name.find("non-ai-2d") != std::string::npos ||
                name.find("nonai") != std::string::npos ||
                name.find("canaan-non-ai-2d") != std::string::npos) {
                return std::string(dev_path);
            }
        }
        return std::string("/dev/video0");
    };

    graph_ = avfilter_graph_alloc();
    if (!graph_) {
        return AVERROR(ENOMEM);
    }

    width_ = width;
    height_ = height;
    time_base_ = time_base;

    snprintf(args, sizeof(args),
             "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=1/1",
             width, height, AV_PIX_FMT_NV12, time_base.num, time_base.den);
    ret = avfilter_graph_create_filter(&main_src_, buffer, "in", args, NULL, graph_);
    if (ret < 0) {
        goto osd_init_fail;
    }

    ret = avfilter_graph_create_filter(&sink_, buffersink, "out", NULL, NULL, graph_);
    if (ret < 0) {
        goto osd_init_fail;
    }

    if (!osd_config_.device.empty()) {
        nonai2d_dev = osd_config_.device;
    } else {
        nonai2d_dev = detect_nonai2d_device();
    }

    snprintf(region_args, sizeof(region_args),
            "[in]nonai2d_osd@nonai2d=device=%s:in_mem_type=%s:out_mem_type=%s[out]",
            nonai2d_dev.c_str(),
            in_mem_type,
            out_mem_type);
    filter_desc = region_args;

    std::cout << "nonai2d_osd filter desc: " << filter_desc << std::endl;
    std::cout << "nonai2d_osd use device: " << nonai2d_dev << std::endl;

    outputs = avfilter_inout_alloc();
    inputs = avfilter_inout_alloc();
    if (!outputs || !inputs) {
        ret = AVERROR(ENOMEM);
        goto osd_init_fail;
    }

    outputs->name = av_strdup("in");
    outputs->filter_ctx = main_src_;
    outputs->pad_idx = 0;
    outputs->next = NULL;

    inputs->name = av_strdup("out");
    inputs->filter_ctx = sink_;
    inputs->pad_idx = 0;
    inputs->next = NULL;

    ret = avfilter_graph_parse_ptr(graph_, filter_desc.c_str(), &inputs, &outputs, NULL);
    if (ret < 0) {
        goto osd_init_fail;
    }

    ret = avfilter_graph_config(graph_, NULL);
    if (ret < 0) {
        goto osd_init_fail;
    }

    osd_ctx_ = FindNonai2dContext(graph_);
    if (!osd_ctx_) {
        printf("%s>failed line %d\n", __func__, __LINE__);
        ret = AVERROR_FILTER_NOT_FOUND;
        goto osd_init_fail;
    }


    inited_ = true;
    avfilter_inout_free(&inputs);
    avfilter_inout_free(&outputs);
    return 0;

osd_init_fail:
    avfilter_inout_free(&inputs);
    avfilter_inout_free(&outputs);
    Deinit();
    return ret;
}

int OsdManager::Apply(AVFrame *frame, OsdRegion* regions, int region_count) {
    int ret;

    if (!inited_ || !frame) {
        printf("%s>failed line %d\n", __func__, __LINE__);
        return AVERROR(EINVAL);
    }

    if (!regions || region_count <= 0 || region_count > NONAI2D_OSD_REGION_NUM) {
        printf("%s>failed line %d, invalid regions\n", __func__, __LINE__);
        return AVERROR(EINVAL);
    }

    for (int i = 0; i < region_count; ++i) {
        OsdRegion* region = &regions[i];
        
        if (!region->enabled || !region->osd_image_data) {
            continue;
        }

        char args[512];
        char res[128];

        snprintf(args, sizeof(args),
               "%d:index:%d:x:%d:y:%d:width:%d:height:%d:valid:%d:data:%p",
               i, i, region->x, region->y, region->width, region->height, 1, region->osd_image_data);

        ret = avfilter_process_command(osd_ctx_,
                                       "config_region",
                                       args,
                                       res,
                                       sizeof(res),
                                       0);
        if (ret < 0) {
            printf("%s>failed to config region %d, ret=%d\n", __func__, i, ret);
            return ret;
        }

        osd_config_.x[i] = region->x;
        osd_config_.y[i] = region->y;
        osd_config_.width[i] = region->width;
        osd_config_.height[i] = region->height;
        osd_config_.valid[i] = 1;
    }

    ret = av_buffersrc_add_frame_flags(main_src_, frame, AV_BUFFERSRC_FLAG_KEEP_REF);
    if (ret < 0) {
        printf("%s>failed line %d\n", __func__, __LINE__);
        return ret;
    }

    av_frame_unref(frame);
    ret = av_buffersink_get_frame(sink_, frame);
    if (ret < 0) {
        printf("%s>failed line %d\n", __func__, __LINE__);
        return ret;
    }

    frame_cnt_++;

    return 0;
}
