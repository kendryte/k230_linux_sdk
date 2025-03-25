#include "media.h"
#include <fcntl.h>
#include <cstring>
#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>
#include <iostream>
#include <memory>
#include <mutex>
#include <atomic>

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

    pkt_ = av_packet_alloc();
    if (!pkt_) {
        std::cerr << "Cannot allocate packet" << std::endl;
        avcodec_free_context(&codec_ctx_);
        avformat_close_input(&fmt_ctx_);
        return -1;
    }

    if (camera_venc_stream_tid_ != 0) {
        pthread_join(camera_venc_stream_tid_,nullptr);
        camera_venc_stream_tid_ = 0;
    }
    start_camera_venc_stream_ = true;
    pthread_create(&camera_venc_stream_tid_, nullptr, camera_venc_stream_thread, this);

    return 0;
}

int KdMedia::disable_media_features()
{
    start_camera_venc_stream_ = false;
    if (camera_venc_stream_tid_ != 0) {
        pthread_join(camera_venc_stream_tid_, nullptr);
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
    AVCodec *codec = nullptr;

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

    codec_ctx->codec_id = codec->id;
    codec_ctx->codec_type = AVMEDIA_TYPE_VIDEO;
    codec_ctx->width = input_config_.venc_width;
    codec_ctx->height = input_config_.venc_height;
    codec_ctx->pix_fmt = AV_PIX_FMT_NV12;
    codec_ctx->time_base = (AVRational){1, 30}; // Set time_base to 1/30 for 30 fps
    codec_ctx->bit_rate = input_config_.bitrate_kbps * 1000;
    codec_ctx->gop_size = 30; // Set GOP size to 30

    //cbr mode
    codec_ctx->rc_min_rate = input_config_.bitrate_kbps * 1000;
    codec_ctx->rc_max_rate = input_config_.bitrate_kbps * 1000;
    codec_ctx->rc_buffer_size = input_config_.bitrate_kbps * 1000;
    codec_ctx->bit_rate = input_config_.bitrate_kbps * 1000;
    codec_ctx->rc_initial_buffer_occupancy = codec_ctx->rc_buffer_size * 3 / 4;

    // Additional parameters to optimize video quality
    codec_ctx->qmin = 20; // Minimum quantizer scale
    codec_ctx->qmax = 40; // Maximum quantizer scale
    codec_ctx->qcompress = 0.6; // Quantizer curve compression factor
    codec_ctx->refs = 3; // Number of reference frames
    codec_ctx->flags |= AV_CODEC_FLAG_LOOP_FILTER; // Enable loop filter
    codec_ctx->me_range = 16; // Limit motion estimation search range
    codec_ctx->max_qdiff = 4; // Maximum quantizer difference between frames


    if (avcodec_open2(codec_ctx, codec, nullptr) < 0) {
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

void *KdMedia::camera_venc_stream_thread(void *arg)
{
    printf("camera_venc_stream_thread start\n");
    KdMedia *media = static_cast<KdMedia *>(arg);
    AVCodecContext *codec_ctx = media->codec_ctx_;
    AVFrame *frame = media->frame_;
    AVFormatContext *fmt_ctx = media->fmt_ctx_;
    AVPacket *pkt = media->pkt_;
    KdMediaFeatureConfig &feature_config = media->feature_config_;
    bool isIFrame = false;
    int ret;

    while (media->start_camera_venc_stream_) {
        ret = av_read_frame(fmt_ctx, pkt);
        if (ret < 0) {
            break;
        }

        if (pkt->stream_index == fmt_ctx->streams[0]->index) {
            // Copy packet data to frame
            av_image_fill_arrays(frame->data, frame->linesize, pkt->data, codec_ctx->pix_fmt, codec_ctx->width, codec_ctx->height, 32);
            frame->pts = pkt->pts;

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
                ret = avcodec_receive_packet(codec_ctx, pkt);
                if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                    break;
                } else if (ret < 0) {
                    std::cerr << "Error receiving packet from codec" << std::endl;
                    break;
                }

                // Process encoded packet here
                if (feature_config.on_venc_data) {
                    isIFrame = (pkt->flags & AV_PKT_FLAG_KEY) != 0;
                    feature_config.on_venc_data->OnVEncData(pkt->data, pkt->size, isIFrame, pkt->pts);
                }

                av_packet_unref(pkt);
            }
        }

        av_packet_unref(pkt);
    }

    return nullptr;
}

// int KdMedia::Test()
// {
//     AVFormatContext *fmt_ctx = nullptr;
//     AVPacket *pkt = nullptr;
//     int ret;

//     avdevice_register_all();
//     AVInputFormat *input_fmt = av_find_input_format("v4l2");
//     if (!input_fmt) {
//         std::cerr << "Cannot find input format" << std::endl;
//         return -1;
//     }

//     AVDictionary *options = nullptr;
//     av_dict_set(&options, "framerate", "30", 0);
//     av_dict_set(&options, "pixel_format", "nv12", 0);

//     av_dict_set(&options, "probesize", "10000000", 0); // Set probesize to 10MB
//     if (avformat_open_input(&fmt_ctx, "/dev/video1", input_fmt, &options) != 0) {
//         std::cerr << "Cannot open input" << std::endl;
//         return -1;
//     }

//     if (avformat_find_stream_info(fmt_ctx, nullptr) < 0) {
//         std::cerr << "Cannot find stream info" << std::endl;
//         return -1;
//     }

//     AVStream *video_stream = fmt_ctx->streams[0];

//     pkt = av_packet_alloc();
//     if (!pkt) {
//         std::cerr << "Cannot allocate packet" << std::endl;
//         return -1;
//     }

//     int read_frame_count = 0;
//     while (true) {
//         ret = av_read_frame(fmt_ctx, pkt);
//         if (ret < 0) {
//             break;
//         }

//         if (pkt->stream_index == video_stream->index) {
//             // Process raw packet here
//             read_frame_count++;
//             printf("[%d]Raw packet size: %d\n",read_frame_count, pkt->size);
//         }

//         av_packet_unref(pkt);
//     }

//     av_packet_free(&pkt);
//     avformat_close_input(&fmt_ctx);
//     return 0;
// }