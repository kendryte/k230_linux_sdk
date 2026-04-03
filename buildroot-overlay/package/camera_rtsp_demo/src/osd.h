#ifndef OSD_H
#define OSD_H

#include <string>
#include <cstdint>

extern "C" {
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavutil/buffer.h>
#include <libavutil/rational.h>
#include <libavutil/frame.h>
}

#define NONAI2D_OSD_REGION_NUM 8

typedef struct {
    int x;
    int y;
    int width;
    int height;
    const unsigned int *osd_image_data;
    int osd_image_size;
    int enabled;
} OsdRegion;

typedef struct {
    std::string device;
    int x[NONAI2D_OSD_REGION_NUM];
    int y[NONAI2D_OSD_REGION_NUM];
    int index[NONAI2D_OSD_REGION_NUM];
    int bg_alpha[NONAI2D_OSD_REGION_NUM];
    int osd_alpha[NONAI2D_OSD_REGION_NUM];
    int video_alpha[NONAI2D_OSD_REGION_NUM];
    int add_order[NONAI2D_OSD_REGION_NUM];
    uint32_t bg_color[NONAI2D_OSD_REGION_NUM];
    char *data[NONAI2D_OSD_REGION_NUM];
    int width[NONAI2D_OSD_REGION_NUM];
    int height[NONAI2D_OSD_REGION_NUM];
    int pixfmt[NONAI2D_OSD_REGION_NUM];
    int valid[NONAI2D_OSD_REGION_NUM];
} Nonai2dOsdConfig;

class OsdManager {
public:
    OsdManager();
    ~OsdManager();

    int Init(int width, int height, AVRational time_base, const char* in_mem_type, const char* out_mem_type);
    int Apply(AVFrame *frame, OsdRegion* regions, int region_count);
    void Deinit();
    bool IsReady() const;

private:
    AVFilterGraph *graph_;
    AVFilterContext *main_src_;
    AVFilterContext *osd_ctx_;
    AVFilterContext *sink_;
    int width_;
    int height_;
    AVRational time_base_;
    bool inited_;
    int frame_cnt_;
    Nonai2dOsdConfig osd_config_;
};

#endif // OSD_H