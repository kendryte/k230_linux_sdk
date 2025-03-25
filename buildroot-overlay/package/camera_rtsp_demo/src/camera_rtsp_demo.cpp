#include "camera_rtsp_demo.h"
#include <iostream>
#include <unistd.h>
#include <thread>

extern "C" {
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
#include <libavcodec/avcodec.h>
#include <libavdevice/avdevice.h>
#include <libavutil/imgutils.h>
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

    //init media
    feature_config_.on_venc_data = this;

    media_.configure_media_features(config, feature_config_);
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

void MyCameraRtspDemo::OnVEncData(unsigned char *data, size_t size, bool bKeyFrame, uint64_t timestamp)
{
    if (!started_) return ;

    // Process encoded packet here
    rtsp_server_.SendVideoData(stream_url_, data, size, timestamp);
}
