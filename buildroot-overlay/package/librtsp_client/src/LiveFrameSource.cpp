#include <iostream>
#include <mutex>
#include "LiveFrameSource.h"

LiveFrameSource* LiveFrameSource::createNew(UsageEnvironment &env, size_t queue_size) {
    return new LiveFrameSource(env, queue_size);
}

LiveFrameSource::LiveFrameSource(UsageEnvironment &env, size_t queue_size) : FramedSource(env), fQueueSize(queue_size) {
    fEventTriggerId = envir().taskScheduler().createEventTrigger(deliverFrame0);
    fThread = std::thread([this](){
        while(this->fNeedReadFrame) {
            this->getFrame();
            usleep(1000 * 10); // FIXME
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
    }
    fRawDataQueue.push_back(raw_data);
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

    int frameSize = 0;
    if (raw_data.buffer_ && raw_data.size_) {
        struct timeval ref;
        if (raw_data.timestamp_) {
            ref.tv_sec = raw_data.timestamp_ / 1000;
            ref.tv_usec = (raw_data.timestamp_ % 1000) * 1000;
        } else {
            gettimeofday(&ref, NULL);
        }
        frameSize = raw_data.size_;
        processFrame(raw_data.buffer_, frameSize, ref);
    }
    return frameSize;
}

void LiveFrameSource::processFrame(std::shared_ptr<uint8_t> data, size_t size, const struct timeval &ref) {
    std::list<FramePacket> packetList = this->parseFrame(data, size, ref);
    while (!packetList.empty()) {
        auto packet = packetList.front();
        queueFramePacket(packet);
        packetList.pop_front();
    }
}

void LiveFrameSource::queueFramePacket(LiveFrameSource::FramePacket &packet) {
    std::unique_lock<std::mutex> lck(fMutex);
    while (fFramePacketQueue.size() >= fQueueSize) {
        fFramePacketQueue.pop_front();
    }
    fFramePacketQueue.push_back(packet);
    lck.unlock();

    // post an event to ask to deliver the frame
    envir().taskScheduler().triggerEvent(fEventTriggerId, this);
}

 void LiveFrameSource::doGetNextFrame() {
    deliverFrame();
 }

void LiveFrameSource::doStopGettingFrames() {
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


void LiveFrameSource::deliverFrame0(void *clientData) {
    ((LiveFrameSource*)clientData)->deliverFrame();
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
            memcpy(fTo, packet.buffer_.get() + packet.offset_, fFrameSize);
        }

        if (fFrameSize > 0) {
            FramedSource::afterGetting(this);
            // envir().taskScheduler().scheduleDelayedTask(0, (TaskFunc*)FramedSource::afterGetting, this);
        }
    }
}
