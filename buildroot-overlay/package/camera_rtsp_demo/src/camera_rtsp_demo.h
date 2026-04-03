#ifndef _CAMERA_RTSP_DEMO_H
#define _CAMERA_RTSP_DEMO_H
#include <atomic>
#include <vector>
#include <string>
#include "rtsp_server.h"
#include "media.h"

class MyCameraRtspDemo:public IOnVEncData {
  public:
    MyCameraRtspDemo();
    int Init(const KdMediaInputConfig &config,const std::string &stream_url = "test", int port = 8554);
    int DeInit();
    int Start();
    int Stop();

    static void OnUpdateOsdRegions(OsdRegion* regions, int region_count, void* user_data);

  protected:
    virtual void OnVEncData(unsigned char *data, size_t size, bool bKeyFrame, uint64_t timestamp) override;

  private:
    KdRtspServer rtsp_server_;
    std::string stream_url_;
    std::atomic<bool> started_{false};
    KdMedia     media_;
    KdMediaFeatureConfig feature_config_;
    KdMediaInputConfig input_config_;
    char        sps_pps_[1024] = {0};
    size_t      sps_pps_size_ = 0;
};



#endif // _CAMERA_RTSP_DEMO_H