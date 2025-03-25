#ifndef _KD_MEDIA_H
#define _KD_MEDIA_H

#include <stddef.h>
#include <stdint.h>
#include <memory>
#include <string>

extern "C" {
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
#include <libavcodec/avcodec.h>
#include <libavdevice/avdevice.h>
#include <libavutil/imgutils.h>
}

enum class KdMediaVideoType {
  kVideoTypeH264,
  kVideoTypeH265,
  kVideoTypeMjpeg,
  kVideoTypeButt
};

struct KdMediaInputConfig {
  KdMediaVideoType video_type = KdMediaVideoType::kVideoTypeH264; // Venc type
  int venc_width = 1280; // Video encoder width
  int venc_height = 720; // Video encoder height
  int bitrate_kbps = 2000; // Bitrate in kbps
  std::string camera_device = "/dev/video1";

};

class IOnVEncData {
  public:
  virtual ~IOnVEncData() {}
  virtual void OnVEncData( unsigned char *data, size_t size, bool bKeyFrame, uint64_t timestamp) = 0;
};

class IOnFrameData {
  public:
  virtual ~IOnFrameData() {}
  virtual void OnFrameData(unsigned char *data, size_t size, bool bKeyFrame, uint64_t timestamp) = 0;
};

struct KdMediaFeatureConfig {
  IOnVEncData *on_venc_data = nullptr;
  IOnFrameData *on_frame_data = nullptr;
};

/**
 * @class KdMedia
 * @brief Class to manage media features and operations.
 */
class KdMedia {
  public:
  /**
   * @brief Configure media features.
   * @param input_config Configuration for media input.
   * @param feature_config Configuration for media features.
   * @return Status of the configuration operation.
   */
  int configure_media_features(const KdMediaInputConfig &input_config, const KdMediaFeatureConfig &feature_config);

  /**
   * @brief Enable media features.
   * @return Status of the enable operation.
   */
  int enable_media_features();

  /**
   * @brief Disable media features.
   * @return Status of the disable operation.
   */
  int disable_media_features();

  /**
   * @brief Destroy media features.
   * @return Status of the destroy operation.
   */
  int destroy_media_features();

  protected:
  static void *camera_venc_stream_thread(void *arg);
  int _init_encoder(AVCodecContext *&codec_ctx, AVFrame *&frame);
  int _init_camera(AVFormatContext *&fmt_ctx);

  private:
  KdMediaInputConfig input_config_;
  KdMediaFeatureConfig feature_config_;

  AVFormatContext *fmt_ctx_ = nullptr;
  AVCodecContext *codec_ctx_ = nullptr;
  AVPacket *pkt_ = nullptr;
  AVFrame *frame_ = nullptr;

  pthread_t camera_venc_stream_tid_{0}; // Thread ID for video encoder stream
  bool start_camera_venc_stream_{false}; // Flag to start video encoder stream

};

#endif // _KD_MEDIA_H
