#include <iostream>
#include <mutex>
#include "LiveFrameSource.h"

static struct timeval addUsToTimeval(const struct timeval &base, uint64_t delta_us) {
    struct timeval ref = base;
    ref.tv_sec += delta_us / 1000000;
    ref.tv_usec += delta_us % 1000000;
    if (ref.tv_usec >= 1000000) {
        ref.tv_sec++;
        ref.tv_usec -= 1000000;
    }
    return ref;
}

LiveFrameSource* LiveFrameSource::createNew(UsageEnvironment &env, size_t queue_size) {
    return new LiveFrameSource(env, queue_size);
}

LiveFrameSource::LiveFrameSource(UsageEnvironment &env, size_t queue_size) : FramedSource(env), fQueueSize(queue_size) {
    fEventTriggerId = envir().taskScheduler().createEventTrigger(deliverFrame0);
    fThread = std::thread([this](){
        while(this->fNeedReadFrame) {
            this->getFrame();
        }
    });
}

void LiveFrameSource::pushData(const uint8_t *data, size_t data_size, uint64_t timestamp) {
    if (data == nullptr || data_size == 0) {
        return;
    }
    if (data_size > maxFrameSize()) {
        std::cout << "LiveFrameSource::pushData() -- drop oversized frame " << data_size << std::endl;
        return;
    }
    std::shared_ptr<uint8_t> buf = make_shared_array<uint8_t>(data_size);
    memcpy(buf.get(), data, data_size); 
    RawData raw_data;
    raw_data.buffer_ = buf;
    raw_data.size_ = data_size;
    raw_data.timestamp_ = timestamp;
    
    std::unique_lock<std::mutex> lck(fMutexRaw);
    while (fRawDataQueue.size() >= fQueueSize) {
        fRawDataQueue.pop_front();
        fRawDropCount++;
    }
    fRawDataQueue.push_back(raw_data);
    const size_t depth = fRawDataQueue.size();
    const bool new_max = depth > fMaxRawDepth.load();
    if (new_max) {
        fMaxRawDepth.store(depth);
    }
    // Past half full, report a new high-water mark at once and a backlog that
    // stays there at most once a second, not on every push while the consumer
    // is behind (e.g. with throttled CPU clocks).
    if (depth >= fQueueSize / 2) {
        const auto now = std::chrono::steady_clock::now();
        if (new_max || now - fLastRawDepthLog >= std::chrono::seconds(1)) {
            fLastRawDepthLog = now;
            std::cout << "LiveFrameSource raw queue depth " << depth << "/"
                      << fQueueSize << ", raw_drop " << fRawDropCount.load()
                      << ", active_drop " << fPacketDropCount.load()
                      << ", idle_drop " << fIdleDropCount.load() << std::endl;
        }
    }
    lck.unlock();
    fCondRaw.notify_one();
}

int LiveFrameSource::getFrame() {
    RawData raw_data;
    std::unique_lock<std::mutex> lck(fMutexRaw);
    fCondRaw.wait(lck, [this]() {
        return !fNeedReadFrame || !fRawDataQueue.empty();
    });
    if (!fNeedReadFrame && fRawDataQueue.empty()) {
        return 0;
    }
    if (!fRawDataQueue.empty()) {
        raw_data = fRawDataQueue.front();
        fRawDataQueue.pop_front();
    }
    lck.unlock();

    int frameSize = raw_data.size_;
    if (!raw_data.buffer_ || raw_data.size_ == 0) {
        return 0;
    }
    processFrame(raw_data.buffer_, raw_data.size_, raw_data.timestamp_);
    return frameSize;
}

void LiveFrameSource::processFrame(std::shared_ptr<uint8_t> data, size_t size, uint64_t timestamp) {
    const struct timeval unset_ref{0, 0};
    std::list<FramePacket> packetList = this->parseFrame(data, size, unset_ref);
    if (packetList.empty()) {
        return;
    }

    bool has_vcl = false;
    for (const auto &packet : packetList) {
        if (packet.is_vcl_) {
            has_vcl = true;
            break;
        }
    }

    struct timeval ref;
    if (has_vcl || (GetEncodeType() != EncodeType::H264 && GetEncodeType() != EncodeType::H265)) {
        ref = presentationTimeFor(timestamp);
    } else if (fHaveTimestampBase) {
        ref = fLastPresentationTime;
    } else {
        gettimeofday(&ref, NULL);
    }

    uint64_t access_unit_id = ++fNextAccessUnitId;
    for (auto &packet : packetList) {
        packet.access_unit_id_ = access_unit_id;
        packet.timestamp_ = ref;
    }
    markAccessUnitEnd(packetList);
    queueFramePackets(packetList);
}

void LiveFrameSource::markAccessUnitEnd(std::list<FramePacket> &packets) {
    for (auto it = packets.rbegin(); it != packets.rend(); ++it) {
        if (it->is_vcl_) {
            it->ends_access_unit_ = true;
            return;
        }
    }
}

void LiveFrameSource::queueFramePacket(LiveFrameSource::FramePacket &packet) {
    std::list<FramePacket> packets;
    packets.push_back(packet);
    queueFramePackets(packets);
}

void LiveFrameSource::dropOldestAccessUnitLocked() {
    if (fFramePacketQueue.empty()) {
        return;
    }
    uint64_t drop_id = fFramePacketQueue.front().access_unit_id_;
    bool consumer_active = fConsumerActive.load();
    do {
        fFramePacketQueue.pop_front();
        if (consumer_active) {
            fPacketDropCount++;
        } else {
            fIdleDropCount++;
        }
    } while (!fFramePacketQueue.empty() && fFramePacketQueue.front().access_unit_id_ == drop_id);
}

void LiveFrameSource::queueFramePackets(std::list<LiveFrameSource::FramePacket> &packets) {
    std::unique_lock<std::mutex> lck(fMutex);
    size_t queue_limit = fConsumerActive.load() ? fQueueSize : (fQueueSize < 16 ? fQueueSize : 16);
    while (!fFramePacketQueue.empty() && fFramePacketQueue.size() + packets.size() > queue_limit) {
        dropOldestAccessUnitLocked();
    }
    fFramePacketQueue.splice(fFramePacketQueue.end(), packets);
    lck.unlock();

    // post an event to ask to deliver the frame
    envir().taskScheduler().triggerEvent(fEventTriggerId, this);
}

void LiveFrameSource::doGetNextFrame() {
    fConsumerActive.store(true);
    // Do not deliver synchronously from the RTSP PLAY call stack.  A queued
    // source can otherwise fill the interleaved TCP window before live555 has
    // sent the PLAY response, while clients wait for that response before
    // reading RTP.  Deferring by one scheduler turn also prevents recursive
    // draining of the queue.
    if (nextTask() == nullptr) {
        nextTask() = envir().taskScheduler().scheduleDelayedTask(
            0, deliverFrameFromTask, this);
    }
}

void LiveFrameSource::doStopGettingFrames() {
    fConsumerActive.store(false);
    FramedSource::doStopGettingFrames();
}

LiveFrameSource::~LiveFrameSource() {
    fNeedReadFrame.store(false);
    fCondRaw.notify_all();
    if(fThread.joinable()) {
        fThread.join();
    }
    while (!fFramePacketQueue.empty()) {
        fFramePacketQueue.pop_front();
    }
    while (!fRawDataQueue.empty()) {
        fRawDataQueue.pop_front();
    }
    if(fEventTriggerId) {
        envir().taskScheduler().deleteEventTrigger(fEventTriggerId);
        fEventTriggerId = 0;
    }
}

std::list<LiveFrameSource::FramePacket>
LiveFrameSource::parseFrame(std::shared_ptr<uint8_t> frame_data, size_t size, const struct timeval &ref) {
    std::list<FramePacket> frameList;
    if (frame_data != NULL) {
        FramePacket packet(frame_data, 0, size, ref);
        frameList.push_back(packet);
    } else {
        std::cout << "LiveFrameSource::parseFrame  frame empty" << std::endl;
    }
    return frameList;
}

struct timeval LiveFrameSource::presentationTimeFor(uint64_t timestamp) {
    struct timeval now;
    gettimeofday(&now, NULL);

    if (timestamp == 0) {
        fLastPresentationTime = now;
        return now;
    }

    // Advance from the previous presentation time instead of mapping every PTS
    // against an absolute base. Some encoders repeat or decrease PTS values at
    // GOP boundaries; using the last valid interval keeps RTP monotonic without
    // making later timestamps jump backward to catch up.
    if (!fHaveTimestampBase) {
        fHaveTimestampBase = true;
        fLastInputTimestamp = timestamp;
        fTimestampScaleToUs = 0;
        fLastPresentationTime = now;
        return now;
    }

    uint64_t step_us = fLastFrameStepUs;
    if (timestamp > fLastInputTimestamp) {
        uint64_t step = timestamp - fLastInputTimestamp;
        if (fTimestampScaleToUs == 0) {
            fTimestampScaleToUs = (step < 10000) ? 1000 : 1;
        }
        if (fTimestampScaleToUs == 1 || step <= (~(uint64_t)0) / 1000) {
            uint64_t candidate_us = step * fTimestampScaleToUs;
            if (candidate_us > 0 && candidate_us <= 1000000) {
                step_us = candidate_us;
                fLastFrameStepUs = candidate_us;
            }
        }
    }
    fLastInputTimestamp = timestamp;
    fLastPresentationTime = addUsToTimeval(fLastPresentationTime, step_us);
    return fLastPresentationTime;
}


void LiveFrameSource::deliverFrame0(void *clientData) {
    LiveFrameSource *source = (LiveFrameSource*)clientData;
    if (source->nextTask() != nullptr) {
        source->envir().taskScheduler().unscheduleDelayedTask(source->nextTask());
    }
    source->deliverFrame();
}

void LiveFrameSource::deliverFrameFromTask(void *clientData) {
    LiveFrameSource *source = (LiveFrameSource*)clientData;
    source->nextTask() = nullptr;
    source->deliverFrame();
}

void LiveFrameSource::deliverFrame() {
    if (isCurrentlyAwaitingData()) {
        fDurationInMicroseconds = 0;
        fFrameSize = 0;

        FramePacket packet;
        std::unique_lock<std::mutex> lck(fMutex);
        if (!fFramePacketQueue.empty()) {
            packet = fFramePacketQueue.front();
            fFramePacketQueue.pop_front();
        }
        lck.unlock();

        if(packet.size_) {
            if (packet.size_ > fMaxSize) {
                fFrameSize = fMaxSize;
                fNumTruncatedBytes = packet.size_ - fMaxSize;
                std::cout << "LiveFrameSource::deliverFrame() -- truncate bytes " << fNumTruncatedBytes << std::endl;
            } else {
                fFrameSize = packet.size_;
            }
            fPresentationTime = packet.timestamp_;
            if (GetEncodeType() == EncodeType::H264 || GetEncodeType() == EncodeType::H265) {
                fDurationInMicroseconds = packet.ends_access_unit_
                    ? kAccessUnitEnds : kAccessUnitContinues;
            }
            memcpy(fTo, packet.buffer_.get() + packet.offset_, fFrameSize);
        }

        if (fFrameSize > 0) {
            FramedSource::afterGetting(this);
            // envir().taskScheduler().scheduleDelayedTask(0, (TaskFunc*)FramedSource::afterGetting, this);
        }
    }
}
