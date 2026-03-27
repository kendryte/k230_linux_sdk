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
    std::string device;
    int region_num;
    int x[NONAI2D_OSD_REGION_NUM];
    int y[NONAI2D_OSD_REGION_NUM];
    int index[NONAI2D_OSD_REGION_NUM];
    int bg_alpha[NONAI2D_OSD_REGION_NUM];
    int osd_alpha[NONAI2D_OSD_REGION_NUM];
    int video_alpha[NONAI2D_OSD_REGION_NUM];
    int add_order[NONAI2D_OSD_REGION_NUM];
    uint32_t bg_color[NONAI2D_OSD_REGION_NUM];
    char *data[NONAI2D_OSD_REGION_NUM];
    int eof_action;
    int shortest;
    int repeatlast;
} Nonai2dOsdConfig;

class OsdManager {
public:
    OsdManager();
    ~OsdManager();

    int Init(int width, int height, AVRational time_base, int regions, const char* in_mem_type, const char* out_mem_type);
    int Apply(AVFrame *frame);
    void Deinit();
    bool IsReady() const;

private:
    int TriWave(int t, int max_v);
    int SendNonai2dOsdCommand(const char *cmd, int value);
    int ReconfigNonai2dOsdPosition(int base_x, int base_y);
    void DumpNonai2dOsdOutputOnce(const AVFrame *frame);

    AVFilterGraph *graph_;
    AVFilterContext *main_src_;
    AVFilterContext *osd_src_;
    AVFilterContext *sink_;
    AVFrame *osd_frame_[NONAI2D_OSD_REGION_NUM];
    int width_;
    int height_;
    AVRational time_base_;
    bool inited_;
    int frame_cnt_;
    Nonai2dOsdConfig osd_config_;
};

#endif // OSD_H