#include <functional>
#include <future>
#include <iostream>
#include <new>
#include <fstream>
#include <memory>
#include <atomic>
#include <thread>
#include <mutex>
#include <pthread.h>
#include <sched.h>
#include "rtsp_client.h"
#include "BasicUsageEnvironment.hh"
#include "GroupsockHelper.hh"
#include "liveMedia.hh"
#include "BasicUsageEnvironment.hh"
#include "g711LiveFrameSource.h"

// Forward function definitions:

struct RtspHooks {
  IOnVideoData *video = nullptr;
  IOnAudioData *audio = nullptr;
  IOnBackChannel *backchannel = nullptr;
  IRtspClientEvent *event = nullptr;
  FramedSource *backchannel_source = nullptr;
  bool require_backchannel = false;
  // Points at KdRtspClient::Impl::watchVariable_. Non-zero means the owner
  // asked the live555 thread to exit, so a dropped session must not reconnect.
  char volatile *watch = nullptr;
};

// Back-channel setup runs on the live555 thread before the RTSPClient pointer
// is stored on the subsession. Each KdRtspClient owns that thread.
static thread_local FramedSource *tls_backchannel_source = nullptr;

namespace backchannel_test {
// RTSP 'response handlers':
void continueAfterDESCRIBE(RTSPClient* rtspClient, int resultCode, char* resultString);
void continueAfterSETUP(RTSPClient* rtspClient, int resultCode, char* resultString);
void continueAfterPLAY(RTSPClient* rtspClient, int resultCode, char* resultString);

// Other event handler functions:
void subsessionAfterPlaying(void* clientData); // called when a stream's subsession (e.g., audio or video substream) ends
void subsessionByeHandler(void* clientData, char const* reason);
  // called when a RTCP "BYE" is received for a subsession
void streamTimerHandler(void* clientData);
  // called at the end of a stream's expected duration (if the stream has not already signaled its end using a RTCP "BYE")

// The main streaming routine (for each "rtsp://" URL):
void openURL(UsageEnvironment& env, char const* progName, char const* rtspURL, const RtspHooks &hooks);

// Used to iterate through each stream's 'subsessions', setting up each one:
void setupNextSubsession(RTSPClient* rtspClient);

// Used to shut down and close a stream (including its "RTSPClient" object).
// allow_reconnect is false only for a finite stream whose SDP duration elapsed.
// A dropped live session (TCP close, RTCP BYE, source closure) still reconnects.
void shutdownStream(RTSPClient* rtspClient, int exitCode = 1, bool allow_reconnect = true);

// Annex-B parameter sets from SDP. H.265 uses sprop-vps/sps/pps; some servers
// still put VPS,SPS,PPS in the H.264 sprop-parameter-sets list.
static unsigned annexb_from_sprop(char const *sprop, u_int8_t *dst, unsigned dst_size);
static unsigned annexb_from_h265_sprop(MediaSubsession &subsession, u_int8_t *dst, unsigned dst_size);

// A function that outputs a string that identifies each stream (for debugging output).  Modify this if you wish:
UsageEnvironment& operator<<(UsageEnvironment& env, const RTSPClient& rtspClient) {
  return env << "[URL:\"" << rtspClient.url() << "\"]: ";
}

// A function that outputs a string that identifies each subsession (for debugging output).  Modify this if you wish:
UsageEnvironment& operator<<(UsageEnvironment& env, const MediaSubsession& subsession) {
  return env << subsession.mediumName() << "/" << subsession.codecName();
}

class MediaSessionExt;
class MediaSubSessionExt : public MediaSubsession {
  public:
    explicit MediaSubSessionExt(MediaSessionExt& parent): MediaSubsession((MediaSession&)parent) {}
    virtual Boolean createBackChannelObjects(int useSpecialRTPoffset) override {
      do {
        fReadSource = tls_backchannel_source;
        printf("MediaSubSessionExt::createBackChannelObjects()  SimpleRTPSink::createNew  payloadFormat = %d\n",(int)fRTPPayloadFormat);
        fRTPSink = SimpleRTPSink::createNew(fParent.envir(), fRTPSocket, fRTPPayloadFormat, 8000, "audio", "PCMU", 1, false);
        if(!fRTPSink) {
          fprintf(stderr, "MediaSubSessionExt::createBackChannelObjects()  fRTPSink==NULL\n");
          break;
        }
        printf("MediaSubSessionExt::createBackChannelObjects()  sink = %p\n",fRTPSink);
        printf("MediaSubSessionExt::createBackChannelObjects()  source = %p\n",fReadSource);
        return True;
      } while (0);
      return False; // an error occurred
    }
};

class MediaSessionExt: public MediaSession {
  public:
    static MediaSession* createNew(UsageEnvironment& env, char const* sdpDescription);
  protected:
    explicit MediaSessionExt(UsageEnvironment& env) : MediaSession(env) {}
    MediaSubsession* createNewMediaSubsession() {
      return (MediaSubsession*) (new MediaSubSessionExt(*this));
    }
  private:
    FramedSource *source_{nullptr};
};

MediaSession* MediaSessionExt::createNew(UsageEnvironment& env, char const* sdpDescription) {
  MediaSessionExt* newSession = new MediaSessionExt(env);
  if (newSession != NULL) {
    if (!newSession->initializeWithSDP(sdpDescription)) {
      delete newSession;
      return NULL;
    }
  }

  return (MediaSessionExt*)newSession;
}

// Define a class to hold per-stream state that we maintain throughout each stream's lifetime:

class StreamClientState {
public:
  StreamClientState();
  virtual ~StreamClientState();

public:
  MediaSubsessionIterator* iter;
  MediaSession* session;
  MediaSubsession* subsession;
  TaskToken streamTimerTask;
  double duration;
};

// If you're streaming just a single stream (i.e., just from a single URL, once), then you can define and use just a single
// "StreamClientState" structure, as a global variable in your application.  However, because - in this demo application - we're
// showing how to play multiple streams, concurrently, we can't do that.  Instead, we have to have a separate "StreamClientState"
// structure for each "RTSPClient".  To do this, we subclass "RTSPClient", and add a "StreamClientState" field to the subclass:

class ourRTSPClient: public RTSPClient {
public:
  static ourRTSPClient* createNew(UsageEnvironment& env, char const* rtspURL,
				  int verbosityLevel = 0,
				  char const* applicationName = NULL,
				  portNumBits tunnelOverHTTPPortNum = 0);

protected:
  ourRTSPClient(UsageEnvironment& env, char const* rtspURL,
		int verbosityLevel, char const* applicationName, portNumBits tunnelOverHTTPPortNum);
    // called only by createNew();
  virtual ~ourRTSPClient();
  virtual void handleTCPConnectionClosed();

public:
  StreamClientState scs;
  RtspHooks hooks;
};

// Define a data sink (a subclass of "MediaSink") to receive the data for each subsession (i.e., each audio or video 'substream').
// In practice, this might be a class (or a chain of classes) that decodes and then renders the incoming audio or video.
// Or it might be a "FileSink", for outputting the received data into a file (as is done by the "openRTSP" application).
// In this example code, however, we define a simple 'dummy' sink that receives incoming data, but does nothing with it.

class DummySink: public MediaSink {
public:
  static DummySink* createNew(UsageEnvironment& env,
			      MediaSubsession& subsession, // identifies the kind of data that's being received
			      char const* streamId = NULL); // identifies the stream itself (optional)

private:
  DummySink(UsageEnvironment& env, MediaSubsession& subsession, char const* streamId);
    // called only by "createNew()"
  virtual ~DummySink();

  static void afterGettingFrame(void* clientData, unsigned frameSize,
                                unsigned numTruncatedBytes,
				struct timeval presentationTime,
                                unsigned durationInMicroseconds);
  void afterGettingFrame(unsigned frameSize, unsigned numTruncatedBytes,
			 struct timeval presentationTime, unsigned durationInMicroseconds);

private:
  // redefined virtual functions:
  virtual Boolean continuePlaying();

private:
  void ProcessH264(unsigned frameSize, uint64_t ms);
  void ProcessH265(unsigned frameSize, uint64_t ms);
  IOnVideoData *videoData() const;
  IOnAudioData *audioData() const;

private:
  u_int8_t* fReceiveBufferAlloc;
  u_int8_t* fReceiveBuffer;
  MediaSubsession& fSubsession;
  char* fStreamId;

  uint8_t * fExtraData;
  size_t fExtraDataSize;
  uint64_t fExtraTimestamp;
  enum class ExtraState : int  { INIT, VPS_OK, SPS_OK, OK};
  ExtraState fExtraState;
};

#define RTSP_CLIENT_VERBOSITY_LEVEL 1 // by default, print verbose output from each "RTSPClient"

static ourRTSPClient *as_our(RTSPClient *client) {
  return (ourRTSPClient *)client;
}

// Set once PLAY succeeds on this live555 thread. A later socket drop then
// reopens the same URL. The first DESCRIBE failure during init() does not.
static thread_local bool tls_had_play = false;
static thread_local unsigned tls_backoff_us = 1000000;
static thread_local int tls_reconnect_left = 3;
// The open RTSPClient on this live555 thread. runSession closes it when the
// owner stops the loop; otherwise its socket, and the pbufs queued on it, leak.
static thread_local ourRTSPClient *tls_client = nullptr;

// Used only on the live555 thread. Close() sets watch and joins that thread
// before Impl or the player is destroyed, so these pointers stay valid.
struct ReconnectArgs {
  UsageEnvironment *env;
  char *url;
  RtspHooks hooks;
  TaskToken token;
};

static thread_local ReconnectArgs *tls_reconnect = nullptr;

static void reconnectTask(void *clientData) {
  ReconnectArgs *args = (ReconnectArgs *)clientData;
  tls_reconnect = nullptr;
  if (args->hooks.watch != nullptr && *args->hooks.watch == 0 && args->url != nullptr) {
    printf("rtsp_client: reconnect %s\n", args->url);
    openURL(*args->env, "BackChannel RTSP Client", args->url, args->hooks);
  }
  delete[] args->url;
  delete args;
}

static void scheduleReconnect(UsageEnvironment &env, char *url, const RtspHooks &hooks) {
  if (url == nullptr) {
    return;
  }
  if (tls_reconnect != nullptr || hooks.watch == nullptr || *hooks.watch != 0) {
    delete[] url;
    return;
  }
  if (tls_reconnect_left <= 0) {
    printf("rtsp_client: reconnect gave up\n");
    delete[] url;
    if (hooks.event != nullptr) {
      hooks.event->OnRtspClientEvent(1);
    }
    return;
  }
  tls_reconnect_left--;
  ReconnectArgs *args = new ReconnectArgs;
  args->env = &env;
  args->url = url;
  args->hooks = hooks;
  args->token = nullptr;
  tls_reconnect = args;
  unsigned delay = tls_backoff_us;
  unsigned next = tls_backoff_us * 2;
  tls_backoff_us = next > 5000000 ? 5000000 : next;
  printf("rtsp_client: session closed, reconnect in %u s\n", delay / 1000000);
  args->token = env.taskScheduler().scheduleDelayedTask((int64_t)delay, reconnectTask, args);
}

void openURL(UsageEnvironment& env, char const* progName, char const* rtspURL, const RtspHooks &hooks) {
  // Begin by creating a "RTSPClient" object.  Note that there is a separate "RTSPClient" object for each stream that we wish
  // to receive (even if more than stream uses the same "rtsp://" URL).
  ourRTSPClient* rtspClient = ourRTSPClient::createNew(env, rtspURL, RTSP_CLIENT_VERBOSITY_LEVEL, progName);
  if (rtspClient == NULL) {
    env << "Failed to create a RTSP client for URL \"" << rtspURL << "\": " << env.getResultMsg() << "\n";
    if (tls_had_play) {
      scheduleReconnect(env, strDup(rtspURL), hooks);
    }
    return;
  }

  tls_client = rtspClient;
  rtspClient->hooks = hooks;
  rtspClient->bRequireBackChannel = hooks.require_backchannel;
  // Next, send a RTSP "DESCRIBE" command, to get a SDP description for the stream.
  // Note that this command - like all RTSP commands - is sent asynchronously; we do not block, waiting for a response.
  // Instead, the following function call returns immediately, and we handle the RTSP response later, from within the event loop:
  rtspClient->sendDescribeCommand(continueAfterDESCRIBE);
}


// Implementation of the RTSP 'response handlers':
void continueAfterDESCRIBE(RTSPClient* rtspClient, int resultCode, char* resultString) {
  do {
    UsageEnvironment& env = rtspClient->envir(); // alias
    StreamClientState& scs = ((ourRTSPClient*)rtspClient)->scs; // alias

    if (resultCode != 0) {
      env << *rtspClient << "Failed to get a SDP description: " << resultString << "\n";
      delete[] resultString;
      break;
    }

    char* const sdpDescription = resultString;
    env << *rtspClient << "Got a SDP description:\n" << sdpDescription << "\n";

    // Create a media session object from this SDP description:
    scs.session = MediaSessionExt::createNew(env, sdpDescription);
    delete[] sdpDescription; // because we don't need it anymore
    if (scs.session == NULL) {
      env << *rtspClient << "Failed to create a MediaSession object from the SDP description: " << env.getResultMsg() << "\n";
      break;
    } else if (!scs.session->hasSubsessions()) {
      env << *rtspClient << "This session has no media subsessions (i.e., no \"m=\" lines)\n";
      break;
    }

    // Then, create and set up our data source objects for the session.  We do this by iterating over the session's 'subsessions',
    // calling "MediaSubsession::initiate()", and then sending a RTSP "SETUP" command, on each one.
    // (Each 'subsession' will have its own data source.)
    scs.iter = new MediaSubsessionIterator(*scs.session);
    setupNextSubsession(rtspClient);
    return;
  } while (0);

  // An unrecoverable error occurred with this stream.
  shutdownStream(rtspClient);
}

// By default, we request that the server stream its data using RTP/UDP.
// If, instead, you want to request that the server stream via RTP-over-TCP, change the following to True:
#define REQUEST_STREAMING_OVER_TCP True

void setupNextSubsession(RTSPClient* rtspClient) {
  UsageEnvironment& env = rtspClient->envir(); // alias
  StreamClientState& scs = ((ourRTSPClient*)rtspClient)->scs; // alias

  scs.subsession = scs.iter->next();
  if (scs.subsession != NULL) {
    if (!scs.subsession->initiate()) {
      env << *rtspClient << "Failed to initiate the \"" << *scs.subsession << "\" subsession: " << env.getResultMsg() << "\n";
      setupNextSubsession(rtspClient); // give up on this subsession; go to the next one
    } else {
      env << *rtspClient << "Initiated the \"" << *scs.subsession << "\" subsession (";
      if (scs.subsession->rtcpIsMuxed()) {
	env << "client port " << scs.subsession->clientPortNum();
      } else {
	env << "client ports " << scs.subsession->clientPortNum() << "-" << scs.subsession->clientPortNum()+1;
      }
      env << ")\n";

      // Continue setting up this subsession, by sending a RTSP "SETUP" command:
      rtspClient->sendSetupCommand(*scs.subsession, continueAfterSETUP, False, REQUEST_STREAMING_OVER_TCP);
    }
    return;
  }

  // We've finished setting up all of the subsessions.  Now, send a RTSP "PLAY" command to start the streaming:
  if (scs.session->absStartTime() != NULL) {
    // Special case: The stream is indexed by 'absolute' time, so send an appropriate "PLAY" command:
    rtspClient->sendPlayCommand(*scs.session, continueAfterPLAY, scs.session->absStartTime(), scs.session->absEndTime());
  } else {
    scs.duration = scs.session->playEndTime() - scs.session->playStartTime();
    rtspClient->sendPlayCommand(*scs.session, continueAfterPLAY);
  }
}

void continueAfterSETUP(RTSPClient* rtspClient, int resultCode, char* resultString) {
  do {
    UsageEnvironment& env = rtspClient->envir(); // alias
    StreamClientState& scs = ((ourRTSPClient*)rtspClient)->scs; // alias

    if (resultCode != 0) {
      env << *rtspClient << "Failed to set up the \"" << *scs.subsession << "\" subsession: " << resultString << "\n";
      break;
    }

    env << *rtspClient << "Set up the \"" << *scs.subsession << "\" subsession (";
    if (scs.subsession->rtcpIsMuxed()) {
      env << "client port " << scs.subsession->clientPortNum();
    } else {
      env << "client ports " << scs.subsession->clientPortNum() << "-" << scs.subsession->clientPortNum()+1;
    }
    env << ")\n";

    // Having successfully setup the subsession, create a data sink for it, and call "startPlaying()" on it.
    // (This will prepare the data sink to receive data; the actual flow of data from the client won't start happening until later,
    // after we've sent a RTSP "PLAY" command.)

    if (scs.subsession->getFlag() == FLAG_RECVONLY) {
       scs.subsession->sink = DummySink::createNew(env, *scs.subsession, rtspClient->url());
          // perhaps use your own custom "MediaSink" subclass instead
       // scs.subsession->sink = FileSink::createNew(env, "back.pcm");

       if (scs.subsession->sink == NULL) {
          env << *rtspClient << "Failed to create a data sink for the \"" << *scs.subsession
	      << "\" subsession: " << env.getResultMsg() << "\n";
          break;
       }

       env << *rtspClient << "Created a data sink for the \"" << *scs.subsession << "\" subsession\n";
       scs.subsession->miscPtr = rtspClient; // a hack to let subsession handle functions get the "RTSPClient" from the subsession
       scs.subsession->sink->startPlaying(*(scs.subsession->readSource()),
                                               subsessionAfterPlaying, scs.subsession);

       if(!strcmp(scs.subsession->mediumName(), "video")) {
          IOnVideoData::VideoType type = IOnVideoData::VideoTypeInvalid;
          if(!strcmp(scs.subsession->codecName(),"H264")) {
              if (as_our(rtspClient)->hooks.video) {
                  // TODO
                  uint8_t *extra_data = nullptr;
                  size_t extra_data_size = 0;
                  unsigned int len = 0;
                  unsigned int numSpropRecords = 0;
                  SPropRecord *record = parseSPropParameterSets(scs.subsession->fmtp_spropparametersets(), numSpropRecords);
                  if (numSpropRecords == 2 && record) { // sps & pps
                    for (auto i = 0; i < numSpropRecords; i++) {
                      len += 4;
                      len += record[i].sPropLength;
                    }
                  }
	                  if (len) {
	                    uint8_t start_code[4] = {0x00, 0x00, 0x00, 0x01};
	                    extra_data = new uint8_t[len];
                    extra_data_size = 0;
                    for (auto i = 0; i < numSpropRecords; i++) {
                      memcpy(&extra_data[extra_data_size], start_code, 4);
                      extra_data_size += 4;
                      memcpy(&extra_data[extra_data_size], record[i].sPropBytes, record[i].sPropLength);
                      extra_data_size += record[i].sPropLength;
                    }
	                  }
	                  as_our(rtspClient)->hooks.video->OnVideoType(IOnVideoData::VideoTypeH264, extra_data, extra_data_size);

	                  if (extra_data) delete []extra_data;
	                  delete[] record;
	              }
	          } else if (!strcmp(scs.subsession->codecName(),"H265")) {
              if (as_our(rtspClient)->hooks.video) {
                  uint8_t *extra_data = nullptr;
                  size_t extra_data_size = 0;
                  unsigned const cap = 4096;
                  uint8_t *buf = new (std::nothrow) uint8_t[cap];
                  if (buf != nullptr) {
                    unsigned n = annexb_from_h265_sprop(*scs.subsession, buf, cap);
                    if (n == 0) {
                      n = annexb_from_sprop(scs.subsession->fmtp_spropparametersets(), buf, cap);
                    }
                    if (n > 0) {
                      extra_data = buf;
                      extra_data_size = n;
                    } else {
                      delete[] buf;
                    }
                  }
                  as_our(rtspClient)->hooks.video->OnVideoType(IOnVideoData::VideoTypeH265, extra_data, extra_data_size);
                  delete[] extra_data;
	              }
          } else {
              if (as_our(rtspClient)->hooks.video) {
                  as_our(rtspClient)->hooks.video->OnVideoType(IOnVideoData::VideoTypeInvalid, nullptr, 0);
              }
          }
      } else if(!strcmp(scs.subsession->mediumName(), "audio")) {
          if (as_our(rtspClient)->hooks.audio) {
            as_our(rtspClient)->hooks.audio->OnAudioStart();
          }
      }
    } else {
       //scs.subsession->sink is equal to scs.subsession->fRTPSink;

       env << *rtspClient << "Created a data sink for the \"" << *scs.subsession << "\" subsession\n";
       scs.subsession->miscPtr = rtspClient; // a hack to let subsession handle functions get the "RTSPClient" from the subsession

       // This sink is created by function MediaSubsession::createBackChannelObjects
       env << *rtspClient << "sink  = " << scs.subsession->sink << "\n";
       env << *rtspClient << "source  = " << scs.subsession->readSource() << "\n";

       scs.subsession->sink->startPlaying(*(scs.subsession->readSource()),
                                          subsessionAfterPlaying, scs.subsession);
       if (as_our(rtspClient)->hooks.backchannel) {
          as_our(rtspClient)->hooks.backchannel->OnBackChannelStart();
       }
    }

    // Also set a handler to be called if a RTCP "BYE" arrives for this subsession:
    if (scs.subsession->rtcpInstance() != NULL) {
      scs.subsession->rtcpInstance()->setByeWithReasonHandler(subsessionByeHandler, scs.subsession);
    }
  } while (0);
  delete[] resultString;

  // Set up the next subsession, if any:
  setupNextSubsession(rtspClient);
}

void continueAfterPLAY(RTSPClient* rtspClient, int resultCode, char* resultString) {
  Boolean success = False;

  rtspClient->envir() << *rtspClient << "continueAfterPLAY \n";
  do {
    UsageEnvironment& env = rtspClient->envir(); // alias
    StreamClientState& scs = ((ourRTSPClient*)rtspClient)->scs; // alias

    if (resultCode != 0) {
      env << *rtspClient << "Failed to start playing session: " << resultString << "\n";
      break;
    }

    // Set a timer to be handled at the end of the stream's expected duration (if the stream does not already signal its end
    // using a RTCP "BYE").  This is optional.  If, instead, you want to keep the stream active - e.g., so you can later
    // 'seek' back within it and do another RTSP "PLAY" - then you can omit this code.
    // (Alternatively, if you don't want to receive the entire stream, you could set this timer for some shorter value.)
    if (scs.duration > 0) {
      unsigned const delaySlop = 2; // number of seconds extra to delay, after the stream's expected duration.  (This is optional.)
      scs.duration += delaySlop;
      unsigned uSecsToDelay = (unsigned)(scs.duration*1000000);
      scs.streamTimerTask = env.taskScheduler().scheduleDelayedTask(uSecsToDelay, (TaskFunc*)streamTimerHandler, rtspClient);
    }

    env << *rtspClient << "Started playing session";
    if (scs.duration > 0) {
      env << " (for up to " << scs.duration << " seconds)";
    }
    env << "...\n";

    tls_had_play = true;
    tls_backoff_us = 1000000;
    tls_reconnect_left = 3;
    success = True;
  } while (0);
  delete[] resultString;

  rtspClient->envir() << *rtspClient << "continueAfterPLAY done\n";
  if (!success) {
    // An unrecoverable error occurred with this stream.
    rtspClient->envir() << *rtspClient << "continueAfterPLAY shutdownStream\n";
    shutdownStream(rtspClient);
  }
}


// Implementation of the other event handlers:

void subsessionAfterPlaying(void* clientData) {
  MediaSubsession* subsession = (MediaSubsession*)clientData;
  RTSPClient* rtspClient = (RTSPClient*)(subsession->miscPtr);

  rtspClient->envir() << *rtspClient << "subsessionAfterPlaying\n";

  // Begin by closing this subsession's stream:
  Medium::close(subsession->sink);
  subsession->sink = NULL;

  // Next, check whether *all* subsessions' streams have now been closed:
  MediaSession& session = subsession->parentSession();
  MediaSubsessionIterator iter(session);
  while ((subsession = iter.next()) != NULL) {
    if (subsession->sink != NULL) return; // this subsession is still active
  }

  // All subsessions' streams have now been closed, so shutdown the client:
  rtspClient->envir() << *rtspClient << "subsessionAfterPlaying shutdownStream\n";
  shutdownStream(rtspClient);
}

void subsessionByeHandler(void* clientData, char const* reason) {
  MediaSubsession* subsession = (MediaSubsession*)clientData;
  RTSPClient* rtspClient = (RTSPClient*)subsession->miscPtr;
  UsageEnvironment& env = rtspClient->envir(); // alias

  env << *rtspClient << "Received RTCP \"BYE\"";
  if (reason != NULL) {
    env << " (reason:\"" << reason << "\")";
    delete[] (char*)reason;
  }
  env << " on \"" << *subsession << "\" subsession\n";

  // Now act as if the subsession had closed:
  subsessionAfterPlaying(subsession);
}

void streamTimerHandler(void* clientData) {
  ourRTSPClient* rtspClient = (ourRTSPClient*)clientData;
  StreamClientState& scs = rtspClient->scs; // alias

  scs.streamTimerTask = NULL;

  // SDP duration elapsed. Live cameras advertise duration 0, so this does not
  // run for them. Reconnecting would replay a file that has already finished.
  rtspClient->envir() << *rtspClient << "streamTimerHandler shutdownStream\n";
  shutdownStream(rtspClient, 1, false);
}

void ourRTSPClient::handleTCPConnectionClosed() {
  shutdownStream(this);
}

void shutdownStream(RTSPClient* rtspClient, int exitCode, bool allow_reconnect) {
  UsageEnvironment& env = rtspClient->envir(); // alias
  StreamClientState& scs = ((ourRTSPClient*)rtspClient)->scs; // alias

  // First, check whether any subsessions have still to be closed:
  if (scs.session != NULL) {
    Boolean someSubsessionsWereActive = False;
    MediaSubsessionIterator iter(*scs.session);
    MediaSubsession* subsession;

    while ((subsession = iter.next()) != NULL) {
      if (subsession->sink != NULL) {
	Medium::close(subsession->sink);
	subsession->sink = NULL;

	if (subsession->rtcpInstance() != NULL) {
	  subsession->rtcpInstance()->setByeHandler(NULL, NULL); // in case the server sends a RTCP "BYE" while handling "TEARDOWN"
	}

	someSubsessionsWereActive = True;
      }
    }

    if (someSubsessionsWereActive) {
      // Send a RTSP "TEARDOWN" command, to tell the server to shutdown the stream.
      // Don't bother handling the response to the "TEARDOWN".
      rtspClient->sendTeardownCommand(*scs.session, NULL);
    }
  }

  env << *rtspClient << "Closing the stream.\n";
  ourRTSPClient *ours = as_our(rtspClient);
  RtspHooks hooks = ours->hooks;
  char *urlCopy = strDup(rtspClient->url());
  // PLAY already succeeded and the owner did not ask the thread to exit.
  // A normal end (SDP duration) passes allow_reconnect false.
  bool replay = allow_reconnect && tls_had_play && hooks.watch != nullptr && *hooks.watch == 0;
  // Close() may hold the owner's lock while it joins this thread, so the
  // owner-requested shutdown must not call back into it.
  bool owner_closing = hooks.watch != nullptr && *hooks.watch != 0;
  IRtspClientEvent *event = owner_closing ? nullptr : hooks.event;
  if (tls_client == ours) {
    tls_client = nullptr;
  }
  Medium::close(rtspClient);
    // Note that this will also cause this stream's "StreamClientState" structure to get reclaimed.

  if (event) {
    event->OnRtspClientEvent(0);
  }
  if (replay) {
    scheduleReconnect(env, urlCopy, hooks);
  } else {
    delete[] urlCopy;
  }
}


// Implementation of "ourRTSPClient":

ourRTSPClient* ourRTSPClient::createNew(UsageEnvironment& env, char const* rtspURL,
					int verbosityLevel, char const* applicationName, portNumBits tunnelOverHTTPPortNum) {
  return new ourRTSPClient(env, rtspURL, verbosityLevel, applicationName, tunnelOverHTTPPortNum);
}

ourRTSPClient::ourRTSPClient(UsageEnvironment& env, char const* rtspURL,
			     int verbosityLevel, char const* applicationName, portNumBits tunnelOverHTTPPortNum)
  : RTSPClient(env,rtspURL, verbosityLevel, applicationName, tunnelOverHTTPPortNum, -1) {
}

ourRTSPClient::~ourRTSPClient() {
}

IOnVideoData *DummySink::videoData() const {
  if (fSubsession.miscPtr == nullptr) {
    return nullptr;
  }
  return ((ourRTSPClient *)fSubsession.miscPtr)->hooks.video;
}

IOnAudioData *DummySink::audioData() const {
  if (fSubsession.miscPtr == nullptr) {
    return nullptr;
  }
  return ((ourRTSPClient *)fSubsession.miscPtr)->hooks.audio;
}


// Implementation of "StreamClientState":

StreamClientState::StreamClientState()
  : iter(NULL), session(NULL), subsession(NULL), streamTimerTask(NULL), duration(0.0) {
}

StreamClientState::~StreamClientState() {
  delete iter;
  if (session != NULL) {
    // We also need to delete "session", and unschedule "streamTimerTask" (if set)
    UsageEnvironment& env = session->envir(); // alias

    env.taskScheduler().unscheduleDelayedTask(streamTimerTask);
    Medium::close(session);
  }
}


// Implementation of "DummySink":

// Even though we're not going to be doing anything with the incoming data, we still need to receive it.
// Define the size of the buffer that we'll use:
#define DUMMY_SINK_RECEIVE_BUFFER_SIZE 1000000
#define DUMMY_SINK_EXTRA_BUFFER_SIZE (64 * 1024)

DummySink* DummySink::createNew(UsageEnvironment& env, MediaSubsession& subsession, char const* streamId) {
  return new DummySink(env, subsession, streamId);
}

DummySink::DummySink(UsageEnvironment& env, MediaSubsession& subsession, char const* streamId)
  : MediaSink(env),
    fSubsession(subsession) {
  fStreamId = strDup(streamId);
  fReceiveBufferAlloc = new u_int8_t[DUMMY_SINK_RECEIVE_BUFFER_SIZE + DUMMY_SINK_EXTRA_BUFFER_SIZE];
    // add bytes to merge vps/sps/pps and idr, and start-code as well
  fReceiveBuffer = fReceiveBufferAlloc + DUMMY_SINK_EXTRA_BUFFER_SIZE;
  fExtraData = new u_int8_t[DUMMY_SINK_EXTRA_BUFFER_SIZE];
  fExtraDataSize = 0;
  fExtraState = ExtraState::INIT;
}

DummySink::~DummySink() {
  delete[] fReceiveBufferAlloc;
  delete[] fExtraData;
  delete[] fStreamId;
}

void DummySink::afterGettingFrame(void* clientData, unsigned frameSize, unsigned numTruncatedBytes,
				  struct timeval presentationTime, unsigned durationInMicroseconds) {
  DummySink* sink = (DummySink*)clientData;
  sink->afterGettingFrame(frameSize, numTruncatedBytes, presentationTime, durationInMicroseconds);
}

static unsigned annexb_from_sprop(char const *sprop, u_int8_t *dst, unsigned dst_size) {
  if (sprop == nullptr || sprop[0] == '\0' || dst == nullptr || dst_size < 4) {
    return 0;
  }
  unsigned num = 0;
  SPropRecord *record = parseSPropParameterSets(sprop, num);
  if (record == nullptr || num == 0) {
    delete[] record;
    return 0;
  }
  unsigned off = 0;
  static const u_int8_t start_code[4] = {0x00, 0x00, 0x00, 0x01};
  for (unsigned i = 0; i < num; ++i) {
    if (record[i].sPropBytes == nullptr || record[i].sPropLength == 0) {
      continue;
    }
    if (off + 4 + record[i].sPropLength > dst_size) {
      delete[] record;
      return 0;
    }
    memcpy(dst + off, start_code, 4);
    off += 4;
    memcpy(dst + off, record[i].sPropBytes, record[i].sPropLength);
    off += record[i].sPropLength;
  }
  delete[] record;
  return off;
}

static unsigned annexb_from_h265_sprop(MediaSubsession &subsession, u_int8_t *dst, unsigned dst_size) {
  char const *parts[3] = {
    subsession.fmtp_spropvps(),
    subsession.fmtp_spropsps(),
    subsession.fmtp_sproppps(),
  };
  unsigned off = 0;
  for (unsigned i = 0; i < 3; ++i) {
    char const *sprop = parts[i];
    if (sprop == nullptr || sprop[0] == '\0') {
      continue;
    }
    if (off >= dst_size) {
      return 0;
    }
    unsigned n = annexb_from_sprop(sprop, dst + off, dst_size - off);
    if (n == 0) {
      return 0;
    }
    off += n;
  }
  return off;
}

void DummySink::ProcessH264(unsigned frameSize, uint64_t ms) {
  #define H264_NAL(v) (v & 0x1F)
  enum class H264_NAL_TYPE : int {
    NAL_IDR = 5,
    NAL_SEI = 6,
    NAL_SPS = 7,
    NAL_PPS = 8
  };

  u_int8_t *data = &fReceiveBuffer[-4];
  data[0] = 0x00; data[1] = 0x00;  data[2] = 0x00; data[3] = 0x01;
  unsigned frame_size = frameSize + 4;

  int nal_type = H264_NAL(fReceiveBuffer[0]);
  if (nal_type == (int)H264_NAL_TYPE::NAL_SPS) {
    if (frame_size > DUMMY_SINK_EXTRA_BUFFER_SIZE) {
      fExtraState = ExtraState::INIT;
      fExtraDataSize = 0;
      return;
    }
    fExtraDataSize = 0;
    memcpy(&fExtraData[fExtraDataSize], data, frame_size);
    fExtraDataSize = frame_size;
    fExtraState = ExtraState::SPS_OK;
    fExtraTimestamp = ms;
    // printf("SPS\n");
  } else if (nal_type == (int)H264_NAL_TYPE::NAL_PPS) {
    if (fExtraState == ExtraState::SPS_OK && fExtraTimestamp == ms) {
      if (fExtraDataSize + frame_size > DUMMY_SINK_EXTRA_BUFFER_SIZE) {
        fExtraState = ExtraState::INIT;
        fExtraDataSize = 0;
        return;
      }
      memcpy(&fExtraData[fExtraDataSize], data, frame_size);
      fExtraDataSize += frame_size;
      fExtraState = ExtraState::OK;
      // printf("PPS\n");
    } else {
      fExtraState = ExtraState::INIT;
      fExtraDataSize = 0;
    }
  } else if (nal_type == (int)H264_NAL_TYPE::NAL_IDR) {
    // printf("IDR\n");
    // SPS+PPS are a few dozen bytes. Keep the scratch on the stack small:
    // a 64KB buffer here overflows the live555 thread, and writing the
    // parameter sets in front of fReceiveBuffer walks off the allocation.
    unsigned extra_size = 0;
    u_int8_t extra_buf[512];
    const u_int8_t *extra = nullptr;
    if (fExtraState == ExtraState::OK && fExtraDataSize > 0 &&
        fExtraDataSize <= sizeof(extra_buf)) {
      extra = fExtraData;
      extra_size = fExtraDataSize;
    } else {
      extra_size = annexb_from_sprop(fSubsession.fmtp_spropparametersets(), extra_buf, sizeof(extra_buf));
      extra = extra_size > 0 ? extra_buf : nullptr;
    }
    IOnVideoData *video = videoData();
    if (video == nullptr) {
      return;
    }
    if (extra != nullptr && extra_size > 0) {
      unsigned total = extra_size + frame_size;
      u_int8_t *au = new (std::nothrow) u_int8_t[total];
      if (au == nullptr) {
        return;
      }
      memcpy(au, extra, extra_size);
      memcpy(au + extra_size, data, frame_size);
      video->OnVideoData(au, total, ms, true);
      delete[] au;
    } else {
      video->OnVideoData(data, frame_size, ms, true);
    }
  } else if (nal_type < (int)H264_NAL_TYPE::NAL_IDR) {
    // printf("nonIDR\n");
    if (IOnVideoData *video = videoData()) video->OnVideoData(data, frame_size, ms, false);
  } else if (nal_type == (int)H264_NAL_TYPE::NAL_SEI) {
    // discard SEI
  } else {
     // discard it， do nothing
     printf("discard h264 packets,  nal type = %d\n", nal_type);
  }
}

void DummySink::ProcessH265(unsigned frameSize, uint64_t ms) {
  #define H265_NAL(v)	 (((v)>> 1) & 0x3f)
  enum class H265_NAL_TYPE : int {
    NAL_TRAIL_N = 0,
    NAL_TRAIL_R = 1,
    NAL_RASL_N = 8,
    NAL_RASL_R = 9,
    NAL_BLA_W_LP = 16,
    NAL_BLA_W_RADL = 17,
    NAL_BLA_N_LP = 18,
    NAL_IDR_W_RADL = 19,
    NAL_IDR_N_LP = 20,
    NAL_CRA_NUT = 21,
    NAL_VPS = 32,
    NAL_SPS = 33,
    NAL_PPS = 34,
    NAL_SEI_PREFIX = 39
  };

  u_int8_t *data = &fReceiveBuffer[-4];
  data[0] = 0x00; data[1] = 0x00;  data[2] = 0x00; data[3] = 0x01;
  unsigned frame_size = frameSize + 4;

  int nal_type = H265_NAL(fReceiveBuffer[0]);
  // BLA, IDR and CRA can all start decoding. Many cameras refresh on CRA.
  const bool irap = nal_type >= (int)H265_NAL_TYPE::NAL_BLA_W_LP &&
                    nal_type <= (int)H265_NAL_TYPE::NAL_CRA_NUT;
  if (nal_type == (int)H265_NAL_TYPE::NAL_VPS) {
    if (frame_size > DUMMY_SINK_EXTRA_BUFFER_SIZE) {
      fExtraState = ExtraState::INIT;
      fExtraDataSize = 0;
      return;
    }
    fExtraDataSize = 0;
    memcpy(&fExtraData[fExtraDataSize], data, frame_size);
    fExtraDataSize = frame_size;
    fExtraState = ExtraState::VPS_OK;
    fExtraTimestamp = ms;
    // printf("VPS\n");
  }
  else if (nal_type == (int)H265_NAL_TYPE::NAL_SPS) {
    if (fExtraState == ExtraState::VPS_OK && fExtraTimestamp == ms) {
      if (fExtraDataSize + frame_size > DUMMY_SINK_EXTRA_BUFFER_SIZE) {
        fExtraState = ExtraState::INIT;
        fExtraDataSize = 0;
        return;
      }
      memcpy(&fExtraData[fExtraDataSize], data, frame_size);
      fExtraDataSize += frame_size;
      fExtraState = ExtraState::SPS_OK;
      // printf("SPS\n");
    } else {
      fExtraState = ExtraState::INIT;
      fExtraDataSize = 0;
    }
  } else if (nal_type == (int)H265_NAL_TYPE::NAL_PPS) {
    if (fExtraState == ExtraState::SPS_OK && fExtraTimestamp == ms) {
      if (fExtraDataSize + frame_size > DUMMY_SINK_EXTRA_BUFFER_SIZE) {
        fExtraState = ExtraState::INIT;
        fExtraDataSize = 0;
        return;
      }
      memcpy(&fExtraData[fExtraDataSize], data, frame_size);
      fExtraDataSize += frame_size;
      fExtraState = ExtraState::OK;
      // printf("PPS\n");
    } else {
      fExtraState = ExtraState::INIT;
      fExtraDataSize = 0;
    }
  } else if (irap) {
    // BLA, IDR and CRA can all start decoding. Parameter sets go into a heap
    // access unit. Writing them in front of fReceiveBuffer walks off the
    // allocation once the start code has already used 4 bytes of the prefix.
    unsigned extra_size = 0;
    u_int8_t extra_buf[512];
    const u_int8_t *extra = nullptr;
    if (fExtraState == ExtraState::OK && fExtraDataSize > 0 &&
        fExtraDataSize <= sizeof(extra_buf)) {
      extra = fExtraData;
      extra_size = fExtraDataSize;
    } else {
      extra_size = annexb_from_h265_sprop(fSubsession, extra_buf, sizeof(extra_buf));
      extra = extra_size > 0 ? extra_buf : nullptr;
    }
    IOnVideoData *video = videoData();
    if (video == nullptr) {
      return;
    }
    if (extra != nullptr && extra_size > 0) {
      unsigned total = extra_size + frame_size;
      u_int8_t *au = new (std::nothrow) u_int8_t[total];
      if (au == nullptr) {
        return;
      }
      memcpy(au, extra, extra_size);
      memcpy(au + extra_size, data, frame_size);
      video->OnVideoData(au, total, ms, true);
      delete[] au;
    } else {
      video->OnVideoData(data, frame_size, ms, true);
    }
  } else if (nal_type == (int)H265_NAL_TYPE::NAL_TRAIL_N || nal_type == (int)H265_NAL_TYPE::NAL_TRAIL_R
        || nal_type == (int)H265_NAL_TYPE::NAL_RASL_N || nal_type == (int)H265_NAL_TYPE::NAL_RASL_R) {
    // RASL after CRA/BLA refers to pictures before that RAP. VDEC skips those
    // NALs from the header type, so they are still forwarded. Drop them here
    // only if a decoder shows corruption on the first RAP.
    if (IOnVideoData *video = videoData()) video->OnVideoData(data, frame_size, ms, false);
  } else if( nal_type == (int)H265_NAL_TYPE::NAL_SEI_PREFIX){
    // discard SEI_PREFIX
  } else {
    // discard it， do nothing
    printf("discard h265 packets,  nal type = %d\n", nal_type);
  }
}

void DummySink::afterGettingFrame(unsigned frameSize, unsigned numTruncatedBytes,
				  struct timeval presentationTime, unsigned /*durationInMicroseconds*/) {

  if (numTruncatedBytes > 0) {
    printf("DummySink::afterGettingFrame discard truncated frame, truncated bytes = %u\n", numTruncatedBytes);
    continuePlaying();
    return;
  }

  if (!strcmp(fSubsession.mediumName(), "audio") && !strcmp(fSubsession.codecName(),"PCMU")) {
    if (IOnAudioData *audio = audioData()) {
      uint64_t ms = presentationTime.tv_sec * 1000 + presentationTime.tv_usec/1000;
      audio->OnAudioData(fReceiveBuffer, frameSize, ms);
    }
  }

  if (!strcmp(fSubsession.mediumName(), "video")) {
      uint64_t ms = presentationTime.tv_sec * 1000 + presentationTime.tv_usec/1000;
      if(!strcmp(fSubsession.codecName(),"H264")) {
          ProcessH264(frameSize, ms);
      } else if (!strcmp(fSubsession.codecName(),"H265")) {
          ProcessH265(frameSize, ms);
      } else {
          // discard it, do nothing
      }
  }
  // Then continue, to request the next frame of data:
  continuePlaying();
}

Boolean DummySink::continuePlaying() {
  if (fSource == NULL) return False; // sanity check (should not happen)

  // Request the next frame of data from our input source.  "afterGettingFrame()" will get called later, when it arrives:
  fSource->getNextFrame(fReceiveBuffer, DUMMY_SINK_RECEIVE_BUFFER_SIZE,
                        afterGettingFrame, this,
                        onSourceClosure, this);
  return True;
}

void runSession(UsageEnvironment &env, const char *url, const RtspHooks &hooks, char volatile &watch) {
  tls_backchannel_source = hooks.backchannel_source;
  tls_had_play = false;
  tls_backoff_us = 1000000;
  tls_reconnect_left = 3;
  openURL(env, "BackChannel RTSP Client", url, hooks);
  env.taskScheduler().doEventLoop(&watch);
  if (tls_client != nullptr) {
    shutdownStream(tls_client, 0, false);
  }
  if (tls_reconnect != nullptr) {
    if (tls_reconnect->token != nullptr) {
      env.taskScheduler().unscheduleDelayedTask(tls_reconnect->token);
    }
    delete[] tls_reconnect->url;
    delete tls_reconnect;
    tls_reconnect = nullptr;
  }
  tls_backchannel_source = nullptr;
  tls_had_play = false;
  printf("KdRtspClient thread exit\n");
}

} //namespace

class KdRtspClient::Impl {
  public:
    Impl() {}
    ~Impl() { DeInit(); }

    int Init(const RtspClientInitParam &param);
    void DeInit();
    int Open(const char *url);
    void Close();

    int SendAudioData(const uint8_t *data, size_t size, uint64_t timestamp);

  private:
    Impl(const Impl &) = delete;
    Impl& operator=(const Impl &) = delete;

  private:
    std::mutex mutex_;
    bool initialized_ = false;
    bool started_ = false;
    TaskScheduler *scheduler_{nullptr};
    UsageEnvironment* env_{nullptr};
    bool enableBackchanel_{false};
    FramedSource *g711_source_{nullptr}; // source, to send data to rtsp-server
    std::thread thread_;
    volatile char watchVariable_{0};
    IOnAudioData *on_audio_data_{nullptr};
    IOnVideoData *on_video_data_{nullptr};
    IOnBackChannel *on_backchannel_{nullptr};
    IRtspClientEvent *on_event_{nullptr};
};

int KdRtspClient::Impl::Init(const RtspClientInitParam &param) {
  std::unique_lock<std::mutex> lck(mutex_);
  if (initialized_) return 0;
  scheduler_ = BasicTaskScheduler::createNew();
  env_ = BasicUsageEnvironment::createNew(*scheduler_);
  enableBackchanel_ = (param.on_backchannel != nullptr);
  if(enableBackchanel_) {
    g711_source_ = G711LiveFrameSource::createNew(*env_, 8);
    on_backchannel_ = param.on_backchannel;
  }
  on_audio_data_ = param.on_audio_data;
  on_video_data_ = param.on_video_data;
  on_event_ = param.on_event;
  initialized_ = true;
  started_ = false;
  return 0;
}

void KdRtspClient::Impl::DeInit() {
  Close();
  std::unique_lock<std::mutex> lck(mutex_);
  if (!initialized_) return;
  if (g711_source_) {
    Medium::close(g711_source_),g711_source_ = nullptr;
  }
  env_->reclaim(), env_ = nullptr;
  delete scheduler_, scheduler_ = nullptr;
  initialized_ = false;
  started_ = false;
}

int KdRtspClient::Impl::Open(const char *url) {
	  std::unique_lock<std::mutex> lck(mutex_);
	  if (started_) return 0;
	  watchVariable_ = 0;
	  std::string rtsp_url = url ? url : "";
	  thread_ = std::thread([this, rtsp_url]() {
        // RT-Thread takes this number as-is, and a smaller number runs first.
        // MicroPython is 25. The old value 30 sits behind it, so a busy face
        // detector stops this thread from reading the socket.
        // 12 is just behind the lwIP tcpip thread (10).
        struct sched_param param;
        param.sched_priority = 12;
        int sched_ret = pthread_setschedparam(pthread_self(), SCHED_FIFO, &param);
        if (sched_ret != 0) {
          printf("rtsp_client: reader priority 12 failed: %d\n", sched_ret);
        }
        RtspHooks hooks;
        hooks.video = on_video_data_;
        hooks.audio = on_audio_data_;
        hooks.backchannel = on_backchannel_;
        hooks.event = on_event_;
        hooks.require_backchannel = enableBackchanel_;
        hooks.backchannel_source = enableBackchanel_ ? g711_source_ : nullptr;
        hooks.watch = &watchVariable_;
        backchannel_test::runSession(*env_, rtsp_url.c_str(), hooks, watchVariable_);
      });
  started_ = true;
  return 0;
}

void KdRtspClient::Impl::Close() {
  std::unique_lock<std::mutex> lck(mutex_);
  if (!started_) return;
  if(thread_.joinable()) {
    // watch is the live555 poll flag. join is the lifetime barrier: reconnect
    // tasks and RtspHooks (raw pointers into this Impl and the player) run
    // only on this thread, and runSession drops a pending reconnect before
    // returning. Do not destroy Impl, the scheduler, or the player until
    // join returns. Do not call Close() from the live555 thread.
    watchVariable_ = 1;
    thread_.join();
  }
  started_ = false;
}

int KdRtspClient::Impl::SendAudioData(const uint8_t *data, size_t size, uint64_t timestamp) {
  std::unique_lock<std::mutex> lck(mutex_);
  if (g711_source_) {
    ((G711LiveFrameSource*)g711_source_)->pushData(data, size, timestamp);
    return 0;
  }
  return -1;
}


KdRtspClient::KdRtspClient() : impl_(std::make_unique<Impl>()) {}
KdRtspClient::~KdRtspClient() {}

int KdRtspClient::Init(const RtspClientInitParam &param) {
  return impl_->Init(param);
}

void KdRtspClient::DeInit() {
  return impl_->DeInit();
}

int KdRtspClient::Open(const char *url) {
  return impl_->Open(url);
}

void KdRtspClient::Close() {
  impl_->Close();
}

int KdRtspClient::SendAudioData(const uint8_t *data, size_t size, uint64_t timestamp) {
  return impl_->SendAudioData(data, size, timestamp);
}
