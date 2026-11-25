#ifndef _LIVEFRAMESOURCE_H
#define _LIVEFRAMESOURCE_H

#include "FramedSource.hh"
#include <list>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <chrono>
#include <memory>
#include <string>

enum class EncodeType {
   INVALID = 0,
   H264 = 1,
   H265 = 2,
   G711U = 100,
   BOTTOM
};

template <typename T>
std::shared_ptr<T> make_shared_array(size_t size) {
    return std::shared_ptr<T>(new T[size], std::default_delete<T[]>());
}

class LiveFrameSource : public FramedSource {
  public:
    static LiveFrameSource *createNew(UsageEnvironment &env, size_t queue_size);
    void pushData(const uint8_t *data, size_t data_size, uint64_t timestamp);
    std::string getAuxLine() { return fAuxLine; };
    virtual EncodeType GetEncodeType() { return EncodeType::INVALID;}

  public:
    struct FramePacket {
      FramePacket() = default;
      FramePacket(std::shared_ptr<uint8_t> buffer, size_t offset, size_t size, struct timeval timestamp) :
        buffer_(buffer), offset_(offset), size_(size), timestamp_(timestamp) {}
      std::shared_ptr<uint8_t> buffer_{nullptr};
      size_t offset_{0};
      size_t size_{0};
      struct timeval timestamp_{0, 0};
      uint64_t access_unit_id_{0};
      bool is_vcl_{false};
      bool ends_access_unit_{false};
    };

    struct RawData {
        std::shared_ptr<uint8_t> buffer_{nullptr};
        size_t size_{0};
        uint64_t timestamp_{0};
    };

  protected:
    LiveFrameSource(UsageEnvironment &env, size_t queue_size);
    virtual ~LiveFrameSource();

    virtual void doGetNextFrame();
    virtual void doStopGettingFrames();

    virtual unsigned maxFrameSize() const {
      return 2 * 1024 * 1024;
    }
    static void deliverFrame0(void *clientData);
    static void deliverFrameFromTask(void *clientData);
    void deliverFrame();

    int getFrame();
    void processFrame(std::shared_ptr<uint8_t> data, size_t size, uint64_t timestamp);
    virtual std::list<FramePacket> parseFrame(std::shared_ptr<uint8_t> data, size_t size, const struct timeval &ref);
    void markAccessUnitEnd(std::list<FramePacket> &packets);
    void queueFramePacket(FramePacket &packet);
    void queueFramePackets(std::list<FramePacket> &packets);
    void dropOldestAccessUnitLocked();
    struct timeval presentationTimeFor(uint64_t timestamp);

  protected:
    std::list<FramePacket> fFramePacketQueue;
    std::list<RawData> fRawDataQueue;
    EventTriggerId fEventTriggerId = 0;
    size_t fQueueSize;

    std::thread fThread;
    std::mutex fMutex;
    std::mutex fMutexRaw;
    std::condition_variable fCondRaw;
    std::atomic<bool> fNeedReadFrame{true};
    std::atomic<unsigned long long> fRawDropCount{0};
    std::atomic<unsigned long long> fPacketDropCount{0};
    std::atomic<unsigned long long> fIdleDropCount{0};
    std::atomic<size_t> fMaxRawDepth{0};
    std::chrono::steady_clock::time_point fLastRawDepthLog{}; // guarded by fMutexRaw
    std::atomic<bool> fConsumerActive{false};
    uint64_t fNextAccessUnitId{0};
    bool fHaveTimestampBase{false};
    uint64_t fLastInputTimestamp{0};
    uint32_t fTimestampScaleToUs{0};
    struct timeval fLastPresentationTime{0, 0};
    uint64_t fLastFrameStepUs{33333};
    std::string fAuxLine;

  public:
    static constexpr unsigned kAccessUnitContinues = 1;
    static constexpr unsigned kAccessUnitEnds = 2;
};

#endif  // _LIVEFRAMESOURCE_H
