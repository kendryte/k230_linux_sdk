#ifndef _CAMERA_WEBRTC_DEMO_H
#define _CAMERA_WEBRTC_DEMO_H

#include <atomic>
#include <string>
#include <pthread.h>
#include "media.h"
#include "osd.h"

extern "C" {
#include "peer.h"
#include "http_server.h"
}

class MyCameraWebRtcDemo : public IOnVEncData {
public:
    MyCameraWebRtcDemo();
    ~MyCameraWebRtcDemo();

    int Init(const KdMediaInputConfig &config, int port = 8080);
    int DeInit();
    int Start();
    int Stop();

    // WebRTC callbacks (called from C by libpeer)
    static void OnIceConnectionStateChange(PeerConnectionState state, void *data);
    static void OnIceCandidate(char *sdp, void *userdata);

    // HTTP request handler (called from C by http_server)
    static void OnHttpRequest(const char *method, const char *path,
                              const char *body, int body_len,
                              http_response_t *response);

protected:
    virtual void OnVEncData(unsigned char *data, size_t size, bool bKeyFrame, uint64_t timestamp) override;

private:
    KdMedia media_;
    KdMediaFeatureConfig feature_config_;
    KdMediaInputConfig input_config_;
    std::atomic<bool> started_{false};

    // WebRTC
    PeerConnection *pc_ = nullptr;
    volatile PeerConnectionState state_ = PEER_CONNECTION_CLOSED;
    int port_ = 8080;

    // H.264 SPS/PPS cache (WebRTC requires SPS/PPS before each I-frame)
    uint8_t *sps_pps_buf_ = nullptr;
    size_t sps_pps_size_ = 0;

    static void OnUpdateOsdRegions(OsdRegion* regions, int region_count, void* user_data);
    OsdRegion osd_regions_[NONAI2D_OSD_REGION_NUM];

    // Peer connection thread
    pthread_t peer_thread_;
    std::atomic<int> interrupted_{0};
    volatile int peer_in_pc_ = 0;
    volatile int venc_in_pc_ = 0;

    // Offer synchronization (ICE gathering)
    pthread_mutex_t offer_mutex_ = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t offer_cond_ = PTHREAD_COND_INITIALIZER;
    char *offer_sdp_ = nullptr;
    int offer_ready_ = 0;

    static void *PeerConnectionTask(void *data);

    // Singleton instance for C callbacks
    static MyCameraWebRtcDemo *instance_;
};

#endif // _CAMERA_WEBRTC_DEMO_H
