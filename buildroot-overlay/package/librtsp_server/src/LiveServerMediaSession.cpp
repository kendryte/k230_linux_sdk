#include <iostream>
#include <sstream>
#include "LiveServerMediaSession.h"
#include "LiveFrameSource.h"

namespace {

// Where the RTP marker bit comes from.
//
// A video frame reaches live555 as several NAL units (SPS, PPS, then one or
// more slices). The marker bit has to be set on the last one, so a decoder
// knows the picture is complete. The stock framer cannot tell: it only ever
// sees one NAL unit at a time, so it guesses "any VCL NAL unit ends the
// frame", which is wrong the moment a frame is coded as more than one slice.
//
// LiveFrameSource does know, because it split the frame up in the first
// place, and it says so by putting kAccessUnitEnds / kAccessUnitContinues in
// fDurationInMicroseconds. These framers read that back out. A duration of 1
// or 2 microseconds is not a plausible real duration, so there is no ambiguity.
//
// The catch is *when* the framer's own fDurationInMicroseconds gets written.
// live555 2023.01.19 assigns it before calling nalUnitEndsAccessUnit(); the
// 2021.05.03 that buildroot ships assigns it after, so the flag read there
// belongs to the previous NAL unit and every marker bit lands one NAL unit
// late. Rather than depend on which version is installed, these framers
// intercept the delivery callback and write the field themselves first. The
// base class then assigns the same value again, which is harmless.
template <typename Base>
class LiveDiscreteFramer : public Base {
  public:
    static LiveDiscreteFramer *createNew(UsageEnvironment &env, FramedSource *input_source) {
        return new LiveDiscreteFramer(env, input_source);
    }

  protected:
    LiveDiscreteFramer(UsageEnvironment &env, FramedSource *input_source)
        : Base(env, input_source, False, False) {}

    Boolean nalUnitEndsAccessUnit(u_int8_t nal_unit_type) override {
        if (this->fDurationInMicroseconds == LiveFrameSource::kAccessUnitEnds) return True;
        if (this->fDurationInMicroseconds == LiveFrameSource::kAccessUnitContinues) return False;
        return Base::nalUnitEndsAccessUnit(nal_unit_type);
    }

    void doGetNextFrame() override {
        this->fInputSource->getNextFrame(this->fTo, this->fMaxSize, afterGettingFrameFirst, this,
                                         FramedSource::handleClosure, this);
    }

  private:
    static void afterGettingFrameFirst(void *clientData, unsigned frameSize,
                                       unsigned numTruncatedBytes,
                                       struct timeval presentationTime,
                                       unsigned durationInMicroseconds) {
        auto *self = static_cast<LiveDiscreteFramer *>(clientData);
        self->fDurationInMicroseconds = durationInMicroseconds;
        Base::afterGettingFrame(clientData, frameSize, numTruncatedBytes,
                                presentationTime, durationInMicroseconds);
    }
};

using LiveH264Framer = LiveDiscreteFramer<H264VideoStreamDiscreteFramer>;
using LiveH265Framer = LiveDiscreteFramer<H265VideoStreamDiscreteFramer>;

}  // namespace

LiveServerMediaSession *LiveServerMediaSession::createNew(UsageEnvironment &env, StreamReplicator *replicator) {
    return new LiveServerMediaSession(env, replicator);
}

// For live streaming, we always set reuseFirstSource False, and use StreamReplicator,
//    so that createNewStreamSource() will be called when a new client comes.
//
LiveServerMediaSession::LiveServerMediaSession(UsageEnvironment &env, StreamReplicator *replicator)
    : OnDemandServerMediaSubsession(env, False/*reuseFirstSource*/), fReplicator(replicator)
    { }

LiveServerMediaSession::~LiveServerMediaSession() {
}

char const *LiveServerMediaSession::getAuxSDPLine(RTPSink *rtpSink, FramedSource *inputSource) {
    const char *auxLine = NULL;
    std::string sdpLine =  ((LiveFrameSource *)(fReplicator->inputSource()))->getAuxLine();
    if (!sdpLine.empty()) {
        std::ostringstream os;
        os << "a=fmtp:" << (int)rtpSink->rtpPayloadType() << " ";
        os << sdpLine;
        os << "\r\n";
        auxLine = strdup(os.str().c_str());

        // std::cout << auxLine << std::endl;
    }
    return auxLine;
}

FramedSource *LiveServerMediaSession::createNewStreamSource(unsigned clientSessionId, unsigned &estBitrate) {
    EncodeType type = ((LiveFrameSource *)fReplicator->inputSource())->GetEncodeType();
    // estBitrate is in kbps and only feeds the "b=AS:" SDP line, which some
    // clients use to size their receive buffer. 2048 was a 2 Mbps guess; the
    // encoder here is routinely configured well above that, and a client that
    // believes the low number can throw away the overflow.
    if (type == EncodeType::H264) {
        estBitrate = 20000;
        return LiveH264Framer::createNew(envir(), fReplicator->createStreamReplica());
    } else if (type == EncodeType::H265) {
        estBitrate = 20000;
        return LiveH265Framer::createNew(envir(), fReplicator->createStreamReplica());
    } else if (type == EncodeType::G711U) {
        estBitrate = 64;
        return fReplicator->createStreamReplica();
    } else {
        std::cout << "createNewStreamSource() -- type not supported yet" << (int) type << std::endl;
        return nullptr;
    }
}

RTPSink *LiveServerMediaSession::createNewRTPSink(Groupsock *rtpGroupsock, unsigned char rtpPayloadTypeIfDynamic, FramedSource* inputSource) {
    EncodeType type = ((LiveFrameSource *)fReplicator->inputSource())->GetEncodeType();
    // OutPacketBuffer::maxSize is a single global shared by every sink in the
    // process, so assigning it outright lets the last session created shrink
    // the buffer out from under the ones already running. increaseMaxSizeTo()
    // only ever raises it. 2 MB to match LiveFrameSource::maxFrameSize().
    if (type == EncodeType::H264) {
        OutPacketBuffer::increaseMaxSizeTo(2 * 1024 * 1024);
        return H264VideoRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic);
    } else if (type == EncodeType::H265) {
        OutPacketBuffer::increaseMaxSizeTo(2 * 1024 * 1024);
        return H265VideoRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic);
    } else if (type == EncodeType::G711U) {
        return SimpleRTPSink::createNew(envir(), rtpGroupsock, 0, 8000, "audio", "PCMU", 1, False /*allowMultipleFramesPerPacket*/);
    } else {
        std::cout << "createNewRTPSink() -- type not supported yet" << (int) type << std::endl;
        return nullptr;
    }
}
