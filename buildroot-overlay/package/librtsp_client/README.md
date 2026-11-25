# Where these files came from

`src/*.cpp`, `src/*.h` and `src/include/rtsp_client.h` are copied byte-for-byte
from the rt-smart library at
`canmv_k230_dpu/src/rtsmart/mpp/middleware/src/rtsp_client/`, commit `5cdf479`.

Nothing in them is rt-smart specific -- `rtsp_client.cpp` includes only standard
C++, `<pthread.h>`, `<sched.h>` and live555 -- so the port is packaging work,
not code work. `src/Makefile`, `src/exports.map`, `librtsp_client.mk` and
`Config.in` are the only files written here; the rt-smart `Makefile` was
dropped because it builds a `.a` the rt-smart way.

| file | what it is |
|---|---|
| `rtsp_client.cpp` | the whole client: DESCRIBE/SETUP/PLAY state machine, H.264/H.265 and G.711 depacketising, the live555 event loop thread, and the back channel sender |
| `include/rtsp_client.h` | the public API: `KdRtspClient` plus the four callback interfaces the application implements |
| `LiveFrameSource.{cpp,h}` | a live555 `FramedSource` fed from outside by `pushData()` -- here it feeds *outgoing* back channel audio |
| `g711LiveFrameSource.{cpp,h}` | the G.711 flavour of it |

## Local changes

None, and it is worth keeping it that way so the next sync stays a plain copy.
`diff -r` against the rt-smart directory should report only the Makefile.

## It needs a live555 patch

The client calls `createBackChannelObjects()`, `getFlag()`, `FLAG_RECVONLY` and
`RTSPClient::bRequireBackChannel`. None of those exist in stock live555 -- they
are an ONVIF audio back channel extension that rt-smart carries inside its
vendored live555 copy. The same extension is backported for the Linux build in

    package/live555/0002-liveMedia-add-ONVIF-audio-back-channel-support.patch

Without it this package does not compile. If live555 is ever upgraded, that
patch has to be reapplied (upstream will not take it; it is ONVIF, not RFC).

It also needs

    package/live555/0003-fix-RTP-over-TCP-stalls-connection-close-IPv6-compare.patch

for `ourRTSPClient::handleTCPConnectionClosed()` to be called at all. Stock
live555 has no such virtual, so that function compiles as a *new* virtual
that nothing ever dispatches to -- no warning, no error, just a client that
never notices the server hanging up and sits in its event loop forever.

## Why `exports.map`

`librtsp_server.so` has its own `LiveFrameSource` and `G711LiveFrameSource`
classes. Same names, different layout -- the server's grew access-unit tracking
and drop counters during its own port, the client's is the plain rt-smart one.
A program that links both libraries would get whichever copy the dynamic linker
resolved first for both, i.e. read one class through the other's vtable.

`exports.map` keeps everything except the `KdRtspClient` API local to this
library, so the two copies can no longer see or preempt each other. That is
also why the header files were left alone even though their include guards
collide (`_G711_LIVEFRAMESOURCE_H` in both): nothing includes both.

## Two small differences from `librtsp_server`

* The Kconfig symbol is `BR2_PACKAGE_LIBRTSP_CLIENT`, i.e. the name buildroot
  derives from the directory. The server's is `BR2_PACKAGE_LIB_RTSPSERVER`,
  which buildroot does *not* derive, so the server only ever gets built as a
  dependency of something else. Setting this one in a defconfig is enough.
* This library actually links the four live555 libraries, so its `DT_NEEDED`
  is complete. `librtsp_server.so` leaves them undefined, which is why every
  consumer of it has to repeat `-lliveMedia -lgroupsock ...` on its own link
  line.

## Using it

```c
#include <rtsp_client.h>   // from staging

class MyVideo : public IOnVideoData {
    void OnVideoType(VideoType t, uint8_t *extra, size_t n) override { /* SPS/PPS */ }
    void OnVideoData(const uint8_t *d, size_t n, uint64_t ts, bool key) override { /* feed decoder */ }
};

KdRtspClient c;
RtspClientInitParam p;
MyVideo v; p.on_video_data = &v;
c.Init(p);
c.Open("rtsp://192.168.1.64:554/stream1");
...
c.Close(); c.DeInit();
```

Link with `-lrtsp_client`. Frames arrive on the client's own thread, so do not
block in the callbacks. `SendAudioData()` is the back channel direction and
only works if the server answered the `Require:` header -- `OnBackChannelStart()`
tells you that it did.
