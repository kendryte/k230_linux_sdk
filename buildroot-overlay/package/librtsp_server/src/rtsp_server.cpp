#include <functional>
#include <signal.h>
#include <iostream>
#include <fstream>
#include <memory>
#include <atomic>
#include <thread>
#include <map>

#include "rtsp_server.h"
#include "liveMedia.hh"
#include "BasicUsageEnvironment.hh"
#include "GroupsockHelper.hh"
#include "LiveServerMediaSession.h"
#include "h264LiveFrameSource.h"
#include "h265LiveFrameSource.h"
#include "g711LiveFrameSource.h"
#include "mjpegLiveFrameSource.h"
#include "mjpegMediaSubSession.h"
#include "mjpegStreamReplicator.h"

// How many encoded frames a session will hold before it starts dropping the
// oldest. 8 was not enough: one I-frame plus the P-frames behind it already
// fills that, so a client that stalled for a moment lost picture instead of
// catching up.
static constexpr unsigned kVideoQueueSize = 64;
static constexpr unsigned kAudioQueueSize = 64;

// How long a client may go quiet before the server hangs up on it. live555
// counts from the last RTCP receiver report, and clients commonly send those
// every 5s or so under load, so 10s hung up on viewers that were perfectly
// alive. 65s matches what the RT-Smart side uses.
static constexpr unsigned kReclamationSeconds = 65;

struct SessionInfo {
    // video,  FIXME
    LiveFrameSource *h26x_source = nullptr;
    StreamReplicator *h26x_replicator = nullptr;
    MjpegLiveVideoSource *jpeg_source = nullptr;
    JpegStreamReplicator *jpeg_replicator = nullptr;
    // audio
    LiveFrameSource *g711_source = nullptr;
    StreamReplicator *g711_replicator = nullptr;
};

static std::mutex session_info_map_mutex_;
static std::map<std::string, SessionInfo> session_info_map_;
typedef std::map<std::string, std::string> SessionUrlMap;
static SessionUrlMap session_url_map_;

class KdRtspServer::Impl {
  public:
    Impl() {}
    ~Impl() { DeInit(); }

    int Init(Port port = 8554);
    void DeInit();

    int CreateSession(const std::string &session_name, const SessionAttr &session_attr);
    int DestroySession(const std::string &session_name);
    char* GetRtspUrl(const std::string &session_name);
    void Start();
    void Stop();

    int SendVideoData(const std::string &session_name, const uint8_t *data, size_t size, uint64_t timestamp);
    int SendAudioData(const std::string &session_name, const uint8_t *data, size_t size, uint64_t timestamp);

  private:
    Impl(const Impl &) = delete;
    Impl& operator=(const Impl &) = delete;

  private:
    void announceStream(ServerMediaSession* sms, char const* streamName);

  private:
    TaskScheduler *scheduler_{nullptr};
    UsageEnvironment* env_{nullptr};
    RTSPServer *rtspServer_{nullptr};
    volatile char watchVariable_{0};
    std::thread server_loop_;
};

int KdRtspServer::Impl::Init(Port port) {
    scheduler_ = BasicTaskScheduler::createNew();
    env_ = BasicUsageEnvironment::createNew(*scheduler_);
    UserAuthenticationDatabase* authDB = nullptr;
    rtspServer_ = RTSPServer::createNew(*env_, port, authDB, kReclamationSeconds);
    if (!rtspServer_) {
        *env_ << "create rtsp server failed." << env_->getResultMsg() << "\n";
        return -1;
    }
    return 0;
}

void KdRtspServer::Impl::DeInit() {
    if (rtspServer_) {
      Medium::close(rtspServer_);
      env_->reclaim();
      delete scheduler_;
      env_ = nullptr;
      scheduler_ = nullptr;
      rtspServer_ = nullptr;
    }
}

int KdRtspServer::Impl::CreateSession(const std::string &session_name, const SessionAttr &session_attr) {
    std::unique_lock<std::mutex> lck(session_info_map_mutex_);
    if (session_info_map_.count(session_name)) {
        *env_ << "stream session has already been created\n";
        return 0;
    }
    lck.unlock();
    if (!session_attr.with_video && !session_attr.with_audio)  {
        *env_ << "no subSessions\n";
        return -1;
    }

    char const* descriptionString = "Session streamed by \"KdRTSPServer\"";
    ServerMediaSession *sms = nullptr;

    // create live-sources and replicators
    SessionInfo info;
    if(session_attr.with_video) {
        if (session_attr.video_type == VideoType::kVideoTypeH264) {
            info.h26x_source = H264LiveFrameSource::createNew(*env_, kVideoQueueSize);
            info.h26x_replicator = StreamReplicator::createNew(*env_, info.h26x_source, false);
            if (!info.h26x_replicator) goto err_exit;
        } else if (session_attr.video_type == VideoType::kVideoTypeH265) {
            info.h26x_source = H265LiveFrameSource::createNew(*env_, kVideoQueueSize);
            info.h26x_replicator = StreamReplicator::createNew(*env_, info.h26x_source, false);
            if (!info.h26x_replicator) goto err_exit;
        } else if (session_attr.video_type == VideoType::kVideoTypeMjpeg) {
            info.jpeg_source = MjpegLiveVideoSource::createNew(*env_, kVideoQueueSize);
            info.jpeg_replicator = JpegStreamReplicator::createNew(*env_, info.jpeg_source, false);
            if (!info.jpeg_replicator) goto err_exit;
        } else {
            *env_ << "video type not supported yet\n";
            goto err_exit;
        }
        std::cout << "with_video" << std::endl;
    }

    if (session_attr.with_audio) {
        std::cout << "with_audio" << std::endl;
        info.g711_source = G711LiveFrameSource::createNew(*env_, kAudioQueueSize);
        info.g711_replicator = StreamReplicator::createNew(*env_, info.g711_source, false);
    }

    // create SMS and subsessions
    sms = ServerMediaSession::createNew(*env_, session_name.c_str(), session_name.c_str(), descriptionString);
    if (info.h26x_replicator) {
        LiveServerMediaSession *h26xliveSubSession = LiveServerMediaSession::createNew(*env_, info.h26x_replicator);
        sms->addSubsession(h26xliveSubSession);
    }

    if (info.jpeg_replicator) {
        MjpegMediaSubsession *jpegliveSubSession = MjpegMediaSubsession::createNew(*env_, info.jpeg_replicator);
        sms->addSubsession(jpegliveSubSession);
    }

    if (info.g711_replicator) {
        LiveServerMediaSession *g711liveSubSession = LiveServerMediaSession::createNew(*env_, info.g711_replicator);
        std::cout << "g711liveSubSession" << std::endl;
        sms->addSubsession(g711liveSubSession);
    }

    rtspServer_->addServerMediaSession(sms);
    announceStream(sms, session_name.c_str());

    lck.lock();
    session_info_map_[session_name] = info;
    return 0;

err_exit:
    if (info.h26x_replicator) Medium::close(info.h26x_replicator);
    if (info.jpeg_replicator) Medium::close(info.jpeg_replicator);
    if (info.g711_replicator) Medium::close(info.g711_replicator);
    return -1;
}


int KdRtspServer::Impl::DestroySession(const std::string &session_name) {
    std::unique_lock<std::mutex> lck(session_info_map_mutex_);
    auto iter = session_info_map_.find(session_name);
    if (iter == session_info_map_.end()) {
        // erase(end()) is undefined behaviour, so a name that is not there
        // has to return before it gets that far
        return -1;
    }

    // Viewers before the thing they are reading from, as in Stop()
    if (rtspServer_) rtspServer_->deleteServerMediaSession(session_name.c_str());
    if (iter->second.h26x_replicator) Medium::close(iter->second.h26x_replicator);
    if (iter->second.jpeg_replicator) Medium::close(iter->second.jpeg_replicator);
    if (iter->second.g711_replicator) Medium::close(iter->second.g711_replicator);

    session_info_map_.erase(iter);
    session_url_map_.erase(session_name);
    return 0;
}

char* KdRtspServer::Impl::GetRtspUrl(const std::string &session_name) {
    if (session_url_map_.find(session_name) != session_url_map_.end()) {
        return strdup(session_url_map_[session_name].c_str());
    }
    return nullptr;
}

void KdRtspServer::Impl::Start() {
#if 0
    if (rtspServer_->setUpTunnelingOverHTTP(80) || rtspServer_->setUpTunnelingOverHTTP(8000) || rtspServer_->setUpTunnelingOverHTTP(8080)) {
        *env_ << "\n(We use port " << rtspServer_->httpServerPortNum() << " for optional RTSP-over-HTTP tunneling.)\n";
    } else {
        *env_ << "\n(RTSP-over-HTTP tunneling is not available.)\n";
    }
#endif

    watchVariable_ = 0;
    server_loop_ = std::thread([this]() {
        env_->taskScheduler().doEventLoop(&watchVariable_);
    });
}

void KdRtspServer::Impl::Stop() {
    // live555 is single threaded: every object below belongs to the event
    // loop, so the loop has to be stopped before any of them is freed.
    // Closing them first left doEventLoop() walking memory that had already
    // gone back to the heap, which is a segfault on the way out.
    if(server_loop_.joinable()) {
      watchVariable_ = 1;
      server_loop_.join();
    }

    std::unique_lock<std::mutex> lck(session_info_map_mutex_);
    for(auto &it: session_info_map_) {
        // Anyone still watching holds a replica of the replicator, and
        // tearing that replica down reaches back into the replicator. So
        // the viewers go first, then the thing they were reading from.
        if (rtspServer_) rtspServer_->deleteServerMediaSession(it.first.c_str());
        if (it.second.h26x_replicator) Medium::close(it.second.h26x_replicator);
        if (it.second.jpeg_replicator) Medium::close(it.second.jpeg_replicator);
        if (it.second.g711_replicator) Medium::close(it.second.g711_replicator);
    }
    session_info_map_.clear();
    session_url_map_.clear();
}


void KdRtspServer::Impl::announceStream(ServerMediaSession* sms, char const* streamName) {
    char* url = rtspServer_->rtspURL(sms);
    session_url_map_[streamName] = url;
    UsageEnvironment& env = rtspServer_->envir();
    env << "\n\"" << streamName << "\" stream " << "\n";
    env << "Play this stream using the URL \"" << url << "\"\n";
    delete[] url;
}

int KdRtspServer::Impl::SendVideoData(const std::string &session_name, const uint8_t *data, size_t size, uint64_t timestamp) {
    std::unique_lock<std::mutex> lck(session_info_map_mutex_);
    if (session_info_map_.count(session_name)) {
        auto &info = session_info_map_[session_name];
        if (info.h26x_source) {
            // TODO， send when there is at least one client
            info.h26x_source->pushData(data, size, timestamp);
        }
        if (info.jpeg_source) {
            // TODO， send when there is at least one client
            info.jpeg_source->pushData(data, size, timestamp);
        }
        return 0;
    }
    return -1;
}

int KdRtspServer::Impl::SendAudioData(const std::string &session_name, const uint8_t *data, size_t size, uint64_t timestamp) {
    std::unique_lock<std::mutex> lck(session_info_map_mutex_);
    if (session_info_map_.count(session_name)) {
        auto &info = session_info_map_[session_name];
        if (info.g711_source) {
            // TODO， send when there is at least one client
            info.g711_source->pushData(data, size, timestamp);
        }
        return 0;
    }
    return -1;
}


KdRtspServer::KdRtspServer() : impl_(std::make_unique<Impl>()) {}
KdRtspServer::~KdRtspServer() {}

int KdRtspServer::Init(int port) {
    return impl_->Init((Port)port);
}

void KdRtspServer::DeInit() {
    impl_->DeInit();
}

int KdRtspServer::CreateSession(const std::string &session_name, const SessionAttr &session_attr) {
    return impl_->CreateSession(session_name, session_attr);
}

int KdRtspServer::DestroySession(const std::string &session_name) {
    session_url_map_.erase(session_name);
    return impl_->DestroySession(session_name);
}

char* KdRtspServer::GetRtspUrl(const std::string &session_name) {
    return impl_->GetRtspUrl(session_name);
}

void KdRtspServer::Start() {
    impl_->Start();
}

void KdRtspServer::Stop() {
    impl_->Stop();
}

int KdRtspServer::SendVideoData(const std::string &session_name, const uint8_t *data, size_t size, uint64_t timestamp) {
    return impl_->SendVideoData(session_name, data, size, timestamp);
}

int KdRtspServer::SendAudioData(const std::string &session_name, const uint8_t *data, size_t size, uint64_t timestamp) {
    return impl_->SendAudioData(session_name, data, size, timestamp);
}
