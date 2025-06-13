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

//#ifdef RTSP_SERVER_TYPE_SMOL
extern "C" {
#include "smolrtsp_server.h"
}
//#endif

MyCameraRtspDemo::MyCameraRtspDemo() {
}

int MyCameraRtspDemo::Init(const KdMediaInputConfig &config,const std::string &stream_url, int port) {
    //init rtsp server
    input_config_ = config;

//#ifndef RTSP_SERVER_TYPE_SMOL
if (config.rtsp_server_type == 0) {
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
//#endif
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
//#ifndef RTSP_SERVER_TYPE_SMOL
    if (input_config_.rtsp_server_type == 0) {
        rtsp_server_.DeInit();
    }

//#endif
    return 0;
}

int MyCameraRtspDemo::Start() {
    if(started_) return 0;
    media_.enable_media_features();
//#ifndef RTSP_SERVER_TYPE_SMOL
    if (input_config_.rtsp_server_type == 0) {
        rtsp_server_.Start();

    } else {
        if (smolrtsp_server_start() < 0) {
            std::cerr << "Failed to start smolrtsp server" << std::endl;
            return -1;
        }
    }
    started_ = true;

//#ifndef RTSP_SERVER_TYPE_SMOL
    if (input_config_.rtsp_server_type == 0)
        printf("Play this stream using the URL:%s\n",rtsp_server_.GetRtspUrl(stream_url_));
//#endif
    return 0;
}

int MyCameraRtspDemo::Stop() {
    if (!started_) return 0;
//#ifndef RTSP_SERVER_TYPE_SMOL
    if (input_config_.rtsp_server_type == 0)
        rtsp_server_.Stop();
    else
        smolrtsp_server_stop();

    started_ = false;
    media_.disable_media_features();
    return 0;
}

static int parse_nalu_type(const uint8_t* data, int size) {
    const uint8_t* nalu_header = NULL;

    // 检查输入有效性
    if (!data || size < 4) return -1; // 至少需要4字节（起始码+NAL头部）

    // 查找起始码 (0x00000001 或 0x000001)
    for (int i = 0; i < size - 3; i++) {
        if (data[i] == 0 && data[i+1] == 0) {
            if (data[i+2] == 0 && data[i+3] == 1) {
                nalu_header = data + i + 4; // 4字节起始码后
                break;
            } else if (data[i+2] == 1) {
                nalu_header = data + i + 3; // 3字节起始码后
                break;
            }
        }
    }

    if (!nalu_header || (nalu_header - data + 1 > size)) {
        return -1; // 未找到起始码或数据不足
    }

    // 提取NALU类型（头部字节的低5位）
    return nalu_header[0] & 0x1F;
}


#include <time.h>
static uint64_t get_precise_timestamp_us() {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return (uint64_t)ts.tv_sec * 1000000ULL + ts.tv_nsec / 1000;  // 纳秒转微秒
}

void MyCameraRtspDemo::OnVEncData(unsigned char *data, size_t size, bool bKeyFrame, uint64_t timestamp)
{
    if (!started_) return ;

    //printf("OnVEncData, size: %zu, bKeyFrame: %d, timestamp: %llu(%llu)\n", size, bKeyFrame, timestamp,get_precise_timestamp_us());

    // Process encoded packet here
    //#ifndef RTSP_SERVER_TYPE_SMOL
    if (input_config_.rtsp_server_type == 0)
    {
        rtsp_server_.SendVideoData(stream_url_, data, size, timestamp);
    }
    else
    {

        // printf("@@@@@@@@@0x%x_0x%x_0x%x_0x%x_0x%x_0x%x_0x%x_0x%x_0x%x_0x%x_0x%x_0x%x\n",
        //        data[0], data[1], data[2], data[3],
        //        data[4], data[5], data[6], data[7],
        //        data[8], data[9], data[10], data[11]);
        if (sps_pps_size_ == 0)
        {
            // Parse NALU type
            int nalu_type = parse_nalu_type(data, size);
            if (nalu_type == 0x7 || nalu_type == 0x8) {
                // SPS or PPS NALU, store it
                if (sps_pps_size_ + size < sizeof(sps_pps_)) {
                    memcpy(sps_pps_ + sps_pps_size_, data, size);
                    sps_pps_size_ += size;
                } else {
                    std::cerr << "SPS/PPS buffer overflow" << std::endl;
                }
            }
        }

        //printf("NALU type: %d, size: %zu, timestamp: %llu\n", nalu_type, size, timestamp);

        //timestamp = 0;
        if (bKeyFrame) {
            // If it's a keyframe, send SPS/PPS first
            if (sps_pps_size_ > 0) {
                if (smolrtsp_send_video_stream((const uint8_t*)sps_pps_, sps_pps_size_, timestamp) < 0) {
                    std::cerr << "Failed to send SPS/PPS" << std::endl;
                }
                //sps_pps_size_ = 0; // Reset after sending
            }
        }

        if (smolrtsp_send_video_stream(data, size,timestamp) < 0) {
            std::cerr << "Failed to send video stream" << std::endl;
        }
    }

}
