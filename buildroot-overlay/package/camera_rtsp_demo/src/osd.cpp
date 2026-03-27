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
#include "OSD1_40x40_argb.c"

static constexpr char kNonai2dCommandTarget[] = "nonai2d_osd";

OsdManager::OsdManager() {
    graph_ = nullptr;
    main_src_ = nullptr;
    osd_src_ = nullptr;
    sink_ = nullptr;
    width_ = 0;
    height_ = 0;
    time_base_ = {0, 0};
    inited_ = false;
    frame_cnt_ = 0;

    for (int i = 0; i < NONAI2D_OSD_REGION_NUM; ++i) {
        osd_frame_[i] = nullptr;
    }
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

    for (int i = 0; i < NONAI2D_OSD_REGION_NUM; ++i) {
        if (osd_frame_[i]) {
            av_frame_free(&osd_frame_[i]);
            osd_frame_[i] = nullptr;
        }
    }

    if (graph_) {
        avfilter_graph_free(&graph_);
        graph_ = nullptr;
    }

    inited_ = false;
}

int OsdManager::TriWave(int t, int max_v) {
    int period;
    int p;

    if (max_v <= 0) {
        return 0;
    }

    period = max_v * 2;
    if (period <= 0) {
        return 0;
    }

    p = t % period;
    if (p <= max_v) {
        return p;
    }
    return period - p;
}

int OsdManager::SendNonai2dOsdCommand(const char *cmd, int value) {
    char arg[32];
    char res[128];

    if (!graph_ || !cmd) {
        return AVERROR(EINVAL);
    }

    snprintf(arg, sizeof(arg), "%d", value);
    return avfilter_graph_send_command(graph_,
                                       kNonai2dCommandTarget,
                                       cmd,
                                       arg,
                                       res,
                                       sizeof(res),
                                       0);
}

int OsdManager::ReconfigNonai2dOsdPosition(int base_x, int base_y) {
    int ret;
    int changed = 0;
    int max_x;
    int max_y;

    if (!graph_) {
        return AVERROR(EINVAL);
    }

    max_x = width_ > 40 ? width_ - 40 : 0;
    max_y = height_ > 40 ? height_ - 40 : 0;

    for (int i = 0; i < osd_config_.region_num; ++i) {
        char key[16];
        int x = base_x + i * 48;
        int y = base_y + (i % 2) * 36;

        if (x < 0) {
            x = 0;
        }
        if (x > max_x) {
            x = max_x;
        }
        if (y < 0) {
            y = 0;
        }
        if (y > max_y) {
            y = max_y;
        }

        if (x == osd_config_.x[i] && y == osd_config_.y[i]) {
            continue;
        }

        snprintf(key, sizeof(key), "x%d", i);
        ret = SendNonai2dOsdCommand(key, x);
        if (ret < 0) {
            return ret;
        }

        snprintf(key, sizeof(key), "y%d", i);
        ret = SendNonai2dOsdCommand(key, y);
        if (ret < 0) {
            return ret;
        }

        osd_config_.x[i] = x;
        osd_config_.y[i] = y;
        changed = 1;
    }

    return 0;
}

int OsdManager::Init(int width, int height, AVRational time_base, int regions,
                     const char* in_mem_type, const char* out_mem_type) {
    int ret;
    int region_num;
    char args[256];
    char region_args[384];
    std::string filter_desc;
    const AVFilter *buffer = avfilter_get_by_name("buffer");
    const AVFilter *buffersink = avfilter_get_by_name("buffersink");
    AVFilterInOut *inputs = NULL;
    AVFilterInOut *outputs = NULL;
    std::string nonai2d_dev;
    const char *eof_action_str = "repeat";
    Nonai2dOsdConfig osd_cfg;

    osd_cfg.device = "";

    osd_cfg.region_num = regions;
    for (int i = 0; i < regions; ++i) {
        osd_cfg.x[i] = 100 + i*50;
        osd_cfg.y[i] = 100;
        osd_cfg.index[i] = i;
        osd_cfg.bg_alpha[i] = 200;
        osd_cfg.osd_alpha[i] = 200;
        osd_cfg.video_alpha[i] = 200;
        osd_cfg.add_order[i] = 0;
        osd_cfg.bg_color[i] = 0xC88080;
        osd_cfg.data[i] = nullptr;
    }
    osd_cfg.eof_action = 0;
    osd_cfg.shortest = 0;
    osd_cfg.repeatlast = 1;

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
    osd_config_ = osd_cfg;

    snprintf(args, sizeof(args),
             "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=1/1",
             width, height, AV_PIX_FMT_NV12, time_base.num, time_base.den);
    ret = avfilter_graph_create_filter(&main_src_, buffer, "in", args, NULL, graph_);
    if (ret < 0) {
        goto osd_init_fail;
    }

    snprintf(args, sizeof(args),
             "video_size=40x40:pix_fmt=%d:time_base=%d/%d:pixel_aspect=1/1",
             AV_PIX_FMT_ARGB, time_base.num, time_base.den);
    ret = avfilter_graph_create_filter(&osd_src_, buffer, "osd", args, NULL, graph_);
    if (ret < 0) {
        goto osd_init_fail;
    }

    ret = avfilter_graph_create_filter(&sink_, buffersink, "out", NULL, NULL, graph_);
    if (ret < 0) {
        goto osd_init_fail;
    }

    if (!osd_cfg.device.empty()) {
        nonai2d_dev = osd_cfg.device;
    } else {
        nonai2d_dev = detect_nonai2d_device();
    }

    if (osd_cfg.eof_action == 1) {
        eof_action_str = "endall";
    } else if (osd_cfg.eof_action == 2) {
        eof_action_str = "pass";
    }

    region_num = osd_cfg.region_num;
    if (region_num < 1) {
        region_num = 1;
    }
    if (region_num > NONAI2D_OSD_REGION_NUM) {
        region_num = NONAI2D_OSD_REGION_NUM;
    }
    osd_config_.region_num = region_num;

    snprintf(region_args, sizeof(region_args),
             "[in][osd]nonai2d_osd=device=%s:region_num=%d:in_mem_type=%s:out_mem_type=%s:",
             nonai2d_dev.c_str(),
             region_num,
             in_mem_type,
             out_mem_type);
    filter_desc = region_args;

    for (int i = 0; i < region_num; ++i) {
        snprintf(region_args, sizeof(region_args),
                 "x%d=%d:y%d=%d:index%d=%d:bg_alpha%d=%d:osd_alpha%d=%d:video_alpha%d=%d:add_order%d=%d:bg_color%d=0x%08X:",
                 i, osd_cfg.x[i],
                 i, osd_cfg.y[i],
                 i, osd_cfg.index[i],
                 i, osd_cfg.bg_alpha[i],
                 i, osd_cfg.osd_alpha[i],
                 i, osd_cfg.video_alpha[i],
                 i, osd_cfg.add_order[i],
                 i, osd_cfg.bg_color[i]);
        filter_desc += region_args;
    }

    snprintf(region_args, sizeof(region_args),
             "eof_action=%s:shortest=%d:repeatlast=%d[out]",
             eof_action_str,
             osd_cfg.shortest,
             osd_cfg.repeatlast);
    filter_desc += region_args;

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
    outputs->next = avfilter_inout_alloc();
    if (!outputs->next) {
        ret = AVERROR(ENOMEM);
        goto osd_init_fail;
    }
    outputs->next->name = av_strdup("osd");
    outputs->next->filter_ctx = osd_src_;
    outputs->next->pad_idx = 0;
    outputs->next->next = NULL;

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

    for (int i = 0; i < region_num; ++i) {
        osd_frame_[i] = av_frame_alloc();
        if (!osd_frame_[i]) {
            ret = AVERROR(ENOMEM);
            goto osd_init_fail;
        }

        osd_frame_[i]->format = AV_PIX_FMT_ARGB;
        osd_frame_[i]->width = 40;
        osd_frame_[i]->height = 40;
        osd_frame_[i]->pts = 0;

        ret = av_frame_get_buffer(osd_frame_[i], 32);
        if (ret < 0) {
            goto osd_init_fail;
        }

        ret = av_frame_make_writable(osd_frame_[i]);
        if (ret < 0) {
            goto osd_init_fail;
        }

        for (int y = 0; y < 40; y++) {
            memcpy(osd_frame_[i]->data[0] + y * osd_frame_[i]->linesize[0],
                   ((const uint8_t *)osd_data) + y * 40 * 4,
                   40 * 4);
        }
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

int OsdManager::Apply(AVFrame *frame) {
    int ret;
    AVFrame *osd_frame;

    if (!inited_ || !frame) {
        printf("%s>failed line %d\n", __func__, __LINE__);
        return AVERROR(EINVAL);
    }

    if (frame_cnt_ % 90 == 0) {
        int step = frame_cnt_ / 15;
        int max_x = width_ > 40 ? width_ - 40 : 0;
        int max_y = height_ > 40 ? height_ - 40 : 0;
        int x = TriWave(step * 14, max_x);
        int y = TriWave(step * 9 + 37, max_y);

        ret = ReconfigNonai2dOsdPosition(x, y);
        if (ret < 0) {
            printf("%s>failed line %d, reconfig ret=%d\n", __func__, __LINE__, ret);
            return ret;
        }
    }

    ret = av_buffersrc_add_frame_flags(main_src_, frame, AV_BUFFERSRC_FLAG_KEEP_REF);
    if (ret < 0) {
        printf("%s>failed line %d\n", __func__, __LINE__);
        return ret;
    }

    osd_frame = osd_frame_[0];
    if (!osd_frame) {
        return AVERROR(EINVAL);
    }

    osd_frame->pts = frame->pts;
    ret = av_buffersrc_add_frame_flags(osd_src_, osd_frame, AV_BUFFERSRC_FLAG_KEEP_REF);
    if (ret < 0) {
        printf("%s>failed line %d\n", __func__, __LINE__);
        return ret;
    }

    // buffersink will move refs into frame; clear existing refs first to avoid leaks.
    av_frame_unref(frame);
    ret = av_buffersink_get_frame(sink_, frame);
    if (ret < 0) {
        printf("%s>failed line %d\n", __func__, __LINE__);
        return ret;
    }

    frame_cnt_++;

    return 0;
}
