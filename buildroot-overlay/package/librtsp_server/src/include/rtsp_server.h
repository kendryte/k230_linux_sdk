#ifndef _KD_RTSP_SERVER_H
#define _KD_RTSP_SERVER_H

#include <unistd.h>
#include <string>
#include <memory>

enum class VideoType {
    kVideoTypeH264,
    kVideoTypeH265,
    kVideoTypeMjpeg,
    kVideoTypeButt
};

/*
 * This server sends only. The audio back channel (RTSP clients talking back
 * to the board, ONVIF-style) used to live here, but it never worked on Linux:
 * it needed a modified live555 and buildroot ships the stock one, so the
 * calls it depended on had been commented out ever since the port. It has
 * been removed rather than left as dead weight. Two-way audio is going to be
 * WebRTC instead.
 */

struct SessionAttr {
    bool with_video {false};
    bool with_audio {false};   // G711U
    VideoType video_type; // valid when with_video is true
};

class KdRtspServer {
  public:
    KdRtspServer();
    ~KdRtspServer();

    int Init(int port = 8554);
    void DeInit();

    int CreateSession(const std::string &session_name, const SessionAttr &session_attr);
    int DestroySession(const std::string &session_name);
    char* GetRtspUrl(const std::string &session_name);
    void Start();
    void Stop();

    int SendVideoData(const std::string &session_name, const uint8_t *data, size_t size, uint64_t timestamp);
    int SendAudioData(const std::string &session_name, const uint8_t *data, size_t size, uint64_t timestamp);

  private:
    KdRtspServer(const KdRtspServer &) = delete;
    KdRtspServer& operator=(const KdRtspServer &) = delete;

  private:
    class Impl;
    std::unique_ptr<Impl> impl_{nullptr};
};

#endif // _RTSP_SERVER_H