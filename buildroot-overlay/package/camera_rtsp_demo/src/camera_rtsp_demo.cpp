#include "camera_rtsp_demo.h"
#include <iostream>
#include <unistd.h>
#include <thread>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include "osd.h"
#include "OSD1_40x40_argb.c"

extern "C" {
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
#include <libavcodec/avcodec.h>
#include <libavdevice/avdevice.h>
#include <libavutil/imgutils.h>
}

static unsigned int g_fullscreen_osd_data[1280 * 720] __attribute__((aligned(0x1000)));
static unsigned int g_small_osd_data[40 * 40] __attribute__((aligned(0x1000)));
static int g_frame_count = 0;

static void GenerateRainbowPixel(unsigned int *data, int x, int y, int time_offset, int width) {
    int hue = (time_offset + x * 10 + y * 10) % 360;
    int red, green, blue;
    if (hue < 60) {
        red = 255; green = hue * 255 / 60; blue = 0;
    } else if (hue < 120) {
        red = 255 - (hue - 60) * 255 / 60; green = 255; blue = 0;
    } else if (hue < 180) {
        red = 0; green = 255; blue = (hue - 120) * 255 / 60;
    } else if (hue < 240) {
        red = 0; green = 255 - (hue - 180) * 255 / 60; blue = 255;
    } else if (hue < 300) {
        red = (hue - 240) * 255 / 60; green = 0; blue = 255;
    } else {
        red = 255; green = 0; blue = 255 - (hue - 300) * 255 / 60;
    }
    int alpha = 200;
    data[y * width + x] = (alpha << 24) | (red << 16) | (green << 8) | blue;
}

static void GenerateFullscreenRainbow() {
    if (g_frame_count > 0)
    {
        return;
    }

    g_frame_count++;
    int time_offset = (g_frame_count / 2) % 360;
    for (int y = 0; y < 720; y++) {
        for (int x = 0; x < 1280; x++) {
            GenerateRainbowPixel(g_fullscreen_osd_data, x, y, time_offset, 1280);
        }
    }
}

static void GenerateSmallRainbow() {
    g_frame_count++;
    int time_offset = (g_frame_count / 2) % 360;
    for (int y = 0; y < 40; y++) {
        for (int x = 0; x < 40; x++) {
            GenerateRainbowPixel(g_small_osd_data, x, y, time_offset, 40);
        }
    }
}

MyCameraRtspDemo::MyCameraRtspDemo() {
}

int MyCameraRtspDemo::Init(const KdMediaInputConfig &config,const std::string &stream_url, int port) {
    //init rtsp server
    input_config_ = config;

    if (rtsp_server_.Init(port, nullptr) < 0) {
        return -1;
    }
    // enable audio-track
    SessionAttr session_attr;
    session_attr.with_audio = true;
    session_attr.with_audio_backchannel = false;
    session_attr.with_video = true;

    if (config.video_type == KdMediaVideoType::kVideoTypeH264) {
        session_attr.video_type = VideoType::kVideoTypeH264;
    } else if (config.video_type == KdMediaVideoType::kVideoTypeH265) {
        session_attr.video_type = VideoType::kVideoTypeH265;
    } else {
        printf("video codec type not supported yet\n");
        return -1;
    }

    if (rtsp_server_.CreateSession(stream_url, session_attr) < 0) {
        return -1;
    }

    stream_url_ = stream_url;

    feature_config_.on_venc_data = this;

    if (input_config_.osd_region > 0) {
        input_config_.osd_callback = OnUpdateOsdRegions;
        input_config_.osd_user_data = this;
    }

    media_.configure_media_features(input_config_, feature_config_);
    return 0;
}

int MyCameraRtspDemo::DeInit() {
    Stop();
    media_.destroy_media_features();

    rtsp_server_.DeInit();
    
    return 0;
}

int MyCameraRtspDemo::Start() {
    if(started_) return 0;
    media_.enable_media_features();

    rtsp_server_.Start();

    started_ = true;

    printf("Play this stream using the URL:%s\n",rtsp_server_.GetRtspUrl(stream_url_));

    return 0;
}

int MyCameraRtspDemo::Stop() {
    if (!started_) return 0;

    rtsp_server_.Stop();

    started_ = false;
    media_.disable_media_features();
    return 0;
}

#include <time.h>
static uint64_t get_precise_timestamp_us() {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return (uint64_t)ts.tv_sec * 1000000ULL + ts.tv_nsec / 1000;  // 纳秒转微秒
}

void MyCameraRtspDemo::OnUpdateOsdRegions(OsdRegion* regions, int region_count, void* user_data) {
    (void)user_data;
    
    if (!regions || region_count <= 0) {
        return;
    }

#if 0
    // Full-screen rainbow (1280x720), single region
    GenerateFullscreenRainbow();
    regions[0].x = 0;
    regions[0].y = 0;
    regions[0].width = 1280;
    regions[0].height = 720;
    regions[0].osd_image_data = g_fullscreen_osd_data;
    regions[0].osd_image_size = sizeof(g_fullscreen_osd_data);
    regions[0].enabled = 1;
    for (int i = 1; i < region_count; i++) {
        regions[i].enabled = 0;
    }
#elif 1
    // Small rainbow (40x40), multiple regions, position changes dynamically (circular arrangement)
    GenerateSmallRainbow();
    int time_offset = (g_frame_count / 3) % 360;
    int radius = 250;
    
    int center_x = 640;
    int center_y = 360;
    
    for (int i = 0; i < region_count; i++) {
        int angle = (i * 360 / region_count + time_offset) % 360;
        float rad = angle * 3.14159f / 180.0f;
        
        int x = center_x + (int)(cos(rad) * radius) - 20;
        int y = center_y + (int)(sin(rad) * radius) - 20;
        
        if (x < 0) x = 0;
        if (y < 0) y = 0;
        if (x > 1240) x = 1240;
        if (y > 680) y = 680;
        
        regions[i].x = x;
        regions[i].y = y;
        regions[i].width = 40;
        regions[i].height = 40;
        regions[i].osd_image_data = g_small_osd_data;
        regions[i].osd_image_size = sizeof(g_small_osd_data);
        regions[i].enabled = 1;
    }
    
#else
    // Use osd_data (40x40), multiple regions
    for (int i = 0; i < region_count; i++) {
        regions[i].x = 10 + i * 50;
        regions[i].y = 10 + i * 30;
        regions[i].width = 40;
        regions[i].height = 40;
        regions[i].osd_image_data = osd_data;
        regions[i].osd_image_size = osd_data_size;
        regions[i].enabled = 1;
    }
#endif
}

void MyCameraRtspDemo::OnVEncData(unsigned char *data, size_t size, bool bKeyFrame, uint64_t timestamp)
{
    if (!started_) return ;

    // Process encoded packet here
    rtsp_server_.SendVideoData(stream_url_, data, size, timestamp);
    
}
