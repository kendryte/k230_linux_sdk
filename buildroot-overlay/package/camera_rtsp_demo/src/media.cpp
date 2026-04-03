#include "media.h"
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

static constexpr size_t kMaxRawPacketQueueSize = 6;
int KdMedia::configure_media_features(const KdMediaInputConfig &input_config, const KdMediaFeatureConfig &feature_config)
{
    input_config_ = input_config;
    feature_config_ = feature_config;
    avdevice_register_all();
    return 0;
}

int KdMedia::enable_media_features()
{
    int ret;
    if (_init_camera(fmt_ctx_) < 0) {
        return -1;
    }

    if (_init_encoder(codec_ctx_, frame_) < 0) {
        avformat_close_input(&fmt_ctx_);
        return -1;
    }

    if (_init_osd() < 0) {
        return -1;
    }

    pkt_ = av_packet_alloc();
    if (!pkt_) {
        std::cerr << "Cannot allocate packet" << std::endl;
        avcodec_free_context(&codec_ctx_);
        avformat_close_input(&fmt_ctx_);
        return -1;
    }

    if (camera_capture_frame_tid_ != 0) {
        pthread_join(camera_capture_frame_tid_,nullptr);
        camera_capture_frame_tid_ = 0;
    }

    if (camera_venc_stream_tid_ != 0) {
        pthread_join(camera_venc_stream_tid_,nullptr);
        camera_venc_stream_tid_ = 0;
    }

    pthread_create(&camera_capture_frame_tid_, nullptr, camera_capture_frame_thread, this);
    pthread_create(&camera_venc_stream_tid_, nullptr, camera_venc_stream_thread, this);

    return 0;
}

int KdMedia::disable_media_features()
{
    stop_flag_ = true;
    if (camera_capture_frame_tid_ != 0) {
        pthread_join(camera_capture_frame_tid_, nullptr);
        camera_capture_frame_tid_ = 0;
    }

    if (camera_venc_stream_tid_ != 0) {
        pthread_join(camera_venc_stream_tid_, nullptr);
        camera_venc_stream_tid_ = 0;
    }

    {
        std::unique_lock<std::mutex> lock(list_mutex_);
        while (!packet_list_.empty()) {
            AVPacket *pkt = packet_list_.front();
            packet_list_.pop_front();
            av_packet_free(&pkt);
        }
    }


    if (frame_ != nullptr) {
        av_frame_free(&frame_);
    }

    if (pkt_ != nullptr) {
        av_packet_free(&pkt_);
    }

    if (codec_ctx_ != nullptr) {
        avcodec_free_context(&codec_ctx_);
    }

    if (fmt_ctx_ != nullptr) {
        avformat_close_input(&fmt_ctx_);
    }

    return 0;
}

int KdMedia::destroy_media_features()
{
    return 0;
}

int KdMedia::_init_camera(AVFormatContext *&fmt_ctx) {
    AVInputFormat *input_fmt = av_find_input_format("v4l2");
    if (!input_fmt) {
        std::cerr << "Cannot find input format" << std::endl;
        return -1;
    }

    AVDictionary *options = nullptr;
    av_dict_set(&options, "framerate", "30", 0);
    av_dict_set(&options, "pixel_format", "nv12", 0);
    av_dict_set(&options, "probesize", "10000000", 0); // Set probesize to 10MB
    char video_size[20];
    snprintf(video_size, sizeof(video_size), "%dx%d", input_config_.venc_width, input_config_.venc_height);
    av_dict_set(&options, "video_size", video_size, 0);

    av_dict_set_int(&options, "buffer_size", 1 * 4096, 0); // 缓存大小，通常为帧数*4096
    av_dict_set_int(&options, "input_queue_size", 1, 0); // 输入队列大小，即缓存帧数

    av_dict_set(&options, "probesize", "32", 0);  // 最小探测头 (32字节)
    av_dict_set(&options, "analyzeduration", "0", 0);  // 禁用格式分析延迟

    av_dict_set(&options, "fflags", "nobuffer", 0);      // 禁用内部缓存
    av_dict_set(&options, "flags", "low_delay", 0);      // 全局低延迟模式

    // av_dict_set(&options, "mem_type", "dmabuf", 0);
    // camera_dmabuf_ = true;

    if (avformat_open_input(&fmt_ctx,input_config_.camera_device.c_str(), input_fmt, &options) != 0) {
        std::cerr << "Cannot open input" << std::endl;
        return -1;
    }

    if (avformat_find_stream_info(fmt_ctx, nullptr) < 0) {
        std::cerr << "Cannot find stream info" << std::endl;
        return -1;
    }

    return 0;
}

int KdMedia::_init_encoder(AVCodecContext *&codec_ctx, AVFrame *&frame) {
    const AVCodec *codec = nullptr;

    if (input_config_.video_type == KdMediaVideoType::kVideoTypeH264)
    {
        codec = avcodec_find_encoder_by_name("h264_v4l2m2m");
    }
    else if (input_config_.video_type == KdMediaVideoType::kVideoTypeH265)
    {
        codec = avcodec_find_encoder_by_name("hevc_v4l2m2m");
    }
    else if (input_config_.video_type == KdMediaVideoType::kVideoTypeMjpeg)
    {
        codec = avcodec_find_encoder_by_name("mjpeg_v4l2m2m");
    }

    if (!codec) {
        std::cerr << "Cannot find encoder" << std::endl;
        return -1;
    }

    codec_ctx = avcodec_alloc_context3(codec);
    if (!codec_ctx) {
        std::cerr << "Cannot allocate codec context" << std::endl;
        return -1;
    }

    //  // 启用每个关键帧包含SPS/PPS
    // codec_ctx->flags2 |= AV_CODEC_FLAG2_LOCAL_HEADER;
    // printf("@@@@@@@@@@@set AV_CODEC_FLAG2_LOCAL_HEADER\n");

    // //禁用 GLOBAL_HEADER 标志
    // codec_ctx->flags &= ~AV_CODEC_FLAG_GLOBAL_HEADER;
    // printf("@@@@@@@@@@@set AV_CODEC_FLAG_GLOBAL_HEADER\n");

    // 禁用延迟
    codec_ctx->flags |= AV_CODEC_FLAG_LOW_DELAY;

    codec_ctx->codec_id = codec->id;
    codec_ctx->codec_type = AVMEDIA_TYPE_VIDEO;
    codec_ctx->width = input_config_.venc_width;
    codec_ctx->height = input_config_.venc_height;
    codec_ctx->pix_fmt = AV_PIX_FMT_NV12;
    codec_ctx->time_base = (AVRational){1, 30}; // Set time_base to 1/30 for 30 fps
    codec_ctx->bit_rate = input_config_.bitrate_kbps * 1000;
    codec_ctx->gop_size = 30; // Set GOP size to 100
    codec_ctx->max_b_frames = 0;

    //cbr mode
    codec_ctx->rc_min_rate = input_config_.bitrate_kbps * 1000;
    codec_ctx->rc_max_rate = input_config_.bitrate_kbps * 1000;
    //codec_ctx->rc_buffer_size = input_config_.bitrate_kbps * 1000 / 2;
    codec_ctx->rc_buffer_size = input_config_.bitrate_kbps * 1000 / 2;
    codec_ctx->bit_rate = input_config_.bitrate_kbps * 1000;
    codec_ctx->rc_initial_buffer_occupancy = codec_ctx->rc_buffer_size * 3 / 4;

    // Additional parameters to optimize video quality
    codec_ctx->qmin = 20; // Minimum quantizer scale
    codec_ctx->qmax = 40; // Maximum quantizer scale
    codec_ctx->qcompress = 0.9; // Quantizer curve compression factor
    codec_ctx->refs = 0; // Number of reference frames
    codec_ctx->me_range = 8; // Limit motion estimation search range
    codec_ctx->max_qdiff = 6; // Maximum quantizer difference between frames

    // 禁用或简化高级编码功能
    codec_ctx->flags &= ~AV_CODEC_FLAG_LOOP_FILTER; // 禁用环路滤波
    codec_ctx->mb_decision = 0;                    // 使用更快的宏块决策算法
    codec_ctx->trellis = 0;                        // 禁用Trellis编码

     // 创建选项字典
    AVDictionary *codec_options = nullptr;
    // 设置输入缓冲区数量（原始帧缓冲区）
    av_dict_set_int(&codec_options, "num_output_buffers", 1, 0);
    // 设置输出缓冲区数量（编码帧缓冲区）
    av_dict_set_int(&codec_options, "num_capture_buffers", 1, 0);
    // 启用 dmabuf 模式
    if(input_config_.osd_region)
        av_dict_set(&codec_options, "in_mem_type", "dmabuf", 0);

    // 设置额外的低延迟参数
    av_dict_set(&codec_options, "tune", "zerolatency", 0);

    if (avcodec_open2(codec_ctx, codec, &codec_options) < 0) {
        std::cerr << "Cannot open codec" << std::endl;
        return -1;
    }

    frame = av_frame_alloc();
    frame->format = codec_ctx->pix_fmt;
    frame->width = codec_ctx->width;
    frame->height = codec_ctx->height;
    if (av_frame_get_buffer(frame, 32) < 0) {
        std::cerr << "Cannot allocate frame buffer" << std::endl;
        return -1;
    }

    return 0;
}

int KdMedia::_init_osd() {

    int ret=0;
    if(input_config_.osd_region)
    {
        ret = osd_manager_.Init(
            input_config_.venc_width,
            input_config_.venc_height,
            (AVRational){1, 30},
            "mmap", "dmabuf");
        if (ret < 0) {
            char errbuf[AV_ERROR_MAX_STRING_SIZE];
            av_strerror(ret, errbuf, sizeof(errbuf));
            std::cerr << "nonai2d osd init failed: " << errbuf << std::endl;
        }

        for (int i = 0; i < NONAI2D_OSD_REGION_NUM; ++i) {
            regions_[i].enabled = 0;
            regions_[i].osd_image_data = nullptr;
        }
    }
    return ret;
}

#include <time.h>
static uint64_t get_precise_timestamp_us() {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return (uint64_t)ts.tv_sec * 1000000ULL + ts.tv_nsec / 1000;  // 纳秒转微秒
}

void *KdMedia::camera_capture_frame_thread(void *arg)
{
    printf("camera_capture_frame_thread start, TID: %d\n", syscall(SYS_gettid));
    KdMedia *media = static_cast<KdMedia *>(arg);
    AVCodecContext *codec_ctx = media->codec_ctx_;
    AVFrame *frame = media->frame_;
    AVFormatContext *fmt_ctx = media->fmt_ctx_;
    KdMediaFeatureConfig &feature_config = media->feature_config_;
    KdMediaInputConfig &input_config = media->input_config_;
    bool isIFrame = false;
    int ret;
    int timestamp = 0;
    while (!media->stop_flag_) {
        // 分配新的AVPacket
        AVPacket *new_pkt = av_packet_alloc();
        if (!new_pkt) {
            std::cerr << "Failed to allocate packet!" << std::endl;
            continue;
        }

        ret = av_read_frame(fmt_ctx, new_pkt);
        if (ret < 0) {
            std::cerr << "Error reading frame: " << ret << std::endl;
            av_packet_free(&new_pkt);
            break;
        }

        if (new_pkt->stream_index == fmt_ctx->streams[0]->index) {

            if (input_config.enable_log)
            {
                timestamp ++;
                new_pkt->pts = timestamp;
                printf("cur camera  timestamp:%lld,now time:%lld\n", new_pkt->pts,get_precise_timestamp_us());
            }

            // 使用智能锁进行线程安全操作
            std::unique_lock<std::mutex> lock(media->list_mutex_);
            while (media->packet_list_.size() >= kMaxRawPacketQueueSize) {
                AVPacket *old_pkt = media->packet_list_.front();
                media->packet_list_.pop_front();
                av_packet_free(&old_pkt);
            }
            // 将AVPacket加入链表（不进行深拷贝，仅转移所有权）
            media->packet_list_.push_back(new_pkt);
            lock.unlock();
        }
        else {
            // 非视频流数据包，释放
            av_packet_free(&new_pkt);
        }

    }

    return nullptr;
}

void *KdMedia::camera_venc_stream_thread(void *arg)
{
    printf("camera_venc_stream_thread start, TID: %d\n", syscall(SYS_gettid));
    KdMedia *media = static_cast<KdMedia*>(arg);
    AVFrame *frame = media->frame_;
    AVFormatContext *fmt_ctx = media->fmt_ctx_;
    AVCodecContext *codec_ctx = media->codec_ctx_;
    AVPacket *pkt2 = media->pkt_;
    KdMediaFeatureConfig &feature_config = media->feature_config_;
    KdMediaInputConfig &input_config = media->input_config_;
    OsdManager &osd_manager = media->osd_manager_;
    int ret;
    bool bget_pkt = false;
    
    while(true){
        AVPacket *pkt = nullptr;
        if (media->stop_flag_) {
            break;
        }
        // 从队列中获取AVPacket
        {
            bget_pkt = false;
            std::unique_lock<std::mutex> lock(media->list_mutex_);
            if (!media->packet_list_.empty()) {
                pkt = media->packet_list_.front();
                media->packet_list_.pop_front();
                bget_pkt = true;
            }
            lock.unlock();

            // 如果没有数据包，等待一段时间
            if (!bget_pkt) {
                ret = avcodec_receive_packet(codec_ctx, pkt2);
                if (0 == ret)
                {
                    //printf("@@@@@@@@@@@@@@@@@@@@get venc packet again ok\n");
                    // Process encoded packet here
                    if (feature_config.on_venc_data) {
                        if (input_config.enable_log)
                            printf("cur venc timestamp:%lld,now time:%lld,venc size:%d,key:%d\n", pkt2->pts,get_precise_timestamp_us(),pkt2->size, pkt2->flags & AV_PKT_FLAG_KEY);
                        feature_config.on_venc_data->OnVEncData(pkt2->data, pkt2->size, (pkt2->flags & AV_PKT_FLAG_KEY),get_precise_timestamp_us());

                    }
                    av_packet_unref(pkt2);
                }


                usleep(1000); // 等待1毫秒
                continue; // 继续下一次循环
            }
        }

        if (pkt) {
            if(media->camera_dmabuf_) {
                frame->opaque = pkt->side_data;
                frame->pts = pkt->pts;
            } else {
                // Y平面
                frame->data[0] = pkt->data;
                frame->linesize[0] = fmt_ctx->streams[0]->codecpar->width;
                // UV平面 (NV12格式中，UV数据紧跟在Y数据后)
                frame->data[1] = pkt->data + fmt_ctx->streams[0]->codecpar->width * fmt_ctx->streams[0]->codecpar->height; // UV平面起始地址为Y平面后
                frame->linesize[1] = fmt_ctx->streams[0]->codecpar->width; // NV12的UV平面宽度等于Y平面宽度
                frame->pts = pkt->pts; // 设置帧的时间戳
            }

            if (input_config.osd_region > 0 && osd_manager.IsReady()) {
                if (input_config.osd_callback) {
                    input_config.osd_callback(media->regions_, input_config.osd_region, input_config.osd_user_data);
                }
                ret = osd_manager.Apply(frame, media->regions_, input_config.osd_region);
                if (ret < 0) {
                    std::cerr << "OSD apply failed: " << ret << std::endl;
                }
            }

            // Send frame to encoder
            ret = avcodec_send_frame(codec_ctx, frame);
            if (ret < 0) {
                char errbuf[AV_ERROR_MAX_STRING_SIZE];
                av_strerror(ret, errbuf, sizeof(errbuf));
                std::cerr << "Error sending frame to encoder: " << errbuf << std::endl;
                break;
            }

            // Receive encoded packet from encoder
            while (ret >= 0) {
            //while (true) {
                ret = avcodec_receive_packet(codec_ctx, pkt2);
                if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                    break;
                } else if (ret < 0) {
                    std::cerr << "Error receiving packet from codec" << std::endl;
                    break;
                }

                // Process encoded packet here
                if (feature_config.on_venc_data) {
                    if (input_config.enable_log)
                        printf("cur venc timestamp:%lld,now time:%lld,venc size:%d,key:%d\n", pkt2->pts,get_precise_timestamp_us(),pkt2->size, pkt2->flags & AV_PKT_FLAG_KEY);
                    feature_config.on_venc_data->OnVEncData(pkt2->data, pkt2->size, (pkt2->flags & AV_PKT_FLAG_KEY),get_precise_timestamp_us());
                }

                av_packet_unref(pkt2);
            }

            // 释放原始数据包
            av_packet_free(&pkt);
        }

    }

    return nullptr;
}