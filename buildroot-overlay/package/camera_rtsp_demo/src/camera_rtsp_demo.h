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
    KdRtspServer rtsp_server_;//rtsp server
    std::string stream_url_;
    std::atomic<bool> started_{false};
    KdMedia     media_;
    KdMediaFeatureConfig feature_config_;
    KdMediaInputConfig input_config_;

};



#endif // _CAMERA_RTSP_DEMO_H