#ifndef _CAMERA_RTSP_DEMO_H
#define _CAMERA_RTSP_DEMO_H
#include <atomic>
#include <vector>
#include <string>
#include "rtsp_server.h"
#include "media.h"

//#define RTSP_SERVER_TYPE_SMOL 1

class MyCameraRtspDemo:public IOnVEncData {
  public:
    MyCameraRtspDemo();
    //init
    int Init(const KdMediaInputConfig &config,const std::string &stream_url = "test", int port = 8554);
    //deinit
    int DeInit();
    //start
    int Start();
    //stop
    int Stop();

  protected:
    // IOnVEncData
    virtual void OnVEncData(unsigned char *data, size_t size, bool bKeyFrame, uint64_t timestamp) override;

  private:
  //#ifndef RTSP_SERVER_TYPE_SMOL
    KdRtspServer rtsp_server_;//rtsp server
  //#endif
    std::string stream_url_;
    std::atomic<bool> started_{false};
    KdMedia     media_;
    KdMediaFeatureConfig feature_config_;
    KdMediaInputConfig input_config_;
    char        sps_pps_[1024] = {0}; // sps pps data
    size_t      sps_pps_size_ = 0; // sps pps data size

};



#endif // _CAMERA_RTSP_DEMO_H