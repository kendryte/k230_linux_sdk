# live555 local patches

Buildroot ships live555 **2021.05.03**. The rt-smart tree carries its own
vendored copy at
`canmv_k230_dpu/src/rtsmart/mpp/middleware/src/live555/`, which is
**2023.01.19 plus Canaan changes**. These patches bring the Canaan changes
that are worth having over to the Linux build, rebased onto 2021.05.03.

`0001-*` is buildroot's own; everything from `0002-*` up is ours.

| patch | what |
|---|---|
| `0002` | ONVIF audio back channel (`Require:` header, `a=sendonly` parsing, send-side subsession). `package/librtsp_client` does not compile without it. |
| `0003` | Three real bug fixes: RTP-over-TCP no longer blocks the event loop, RTSP clients are told when the connection dies, IPv6 addresses compare correctly. |
| `0004` | RTP socket buffers sized for one I-frame. Tuning, not a fix -- separate so it can be dropped on its own. |
| `0005` | `SO_REUSEADDR` back on for the listening socket, so a server that had a client attached can be restarted without waiting 60s for TIME_WAIT. |

Each patch header explains its own change in full. Read those first.

## How the Canaan changes were separated from upstream drift

A plain 2-way diff between the two trees is useless, because 2021 -> 2023 is
itself about a thousand lines of upstream churn (mostly threading `TLSState`
through everything). So stock 2023.01.19 was downloaded from

    https://download.videolan.org/pub/contrib/live555/live.2023.01.19.tar.gz

and the rt-smart tree diffed against *that*. What is left is Canaan's work
and nothing else: 20 files. Do the same thing next time rather than diffing
rt-smart against whatever buildroot currently ships.

## Deliberately NOT carried over

Most of what rt-smart changed is specific to RT-Smart and its lwIP stack, or
is leftover debugging. Porting it to Linux would be a regression, not a fix:

| rt-smart change | why not |
|---|---|
| `GroupsockHelper.cpp`: `SO_REUSEPORT` and `IP_MULTICAST_LOOP` blocks commented out | lwIP does not implement them; Linux does, and live555 wants them |
| `GenericMediaServer.cpp`: `SO_LINGER {1,0}` added | forces a RST instead of a clean close, to dodge lwIP's stale TIME_WAIT PCBs. On Linux this would cut responses short. The other half of rt-smart's change here, dropping `NoReuse`, **is** carried over -- see `0005` |
| `RTSPServer.cpp`, `DynamicRTSPServer.cpp`: IPv6 listener disabled (`ourSocketIPv6 = -1`) | lwIP build has no IPv6 |
| `Groupsock.cpp`: per-packet TTL `setsockopt` bypassed | works around an lwIP limitation |
| `mediaServer/*`: RTSP-over-HTTP tunnelling `#if 0`'d out | that is the stand-alone demo server, which we do not ship as a product |
| `(socklen_t *)` casts on `getsockopt` | rt-smart's `SOCKLEN_T` differs; on Linux the cast is a no-op at best |
| `printf("=========createSocket:%d\n", ...)` and friends | debug noise |
| `StreamParser.cpp` `BANK_SIZE`, `DynamicRTSPServer.cpp` `OutPacketBuffer::maxSize` | neither code path is used by `librtsp_server` or `librtsp_client`; see the `0004` header |
| `H264or5VideoStreamDiscreteFramer.cpp`: `fDurationInMicroseconds` assigned before `nalUnitEndsAccessUnit()` | `librtsp_server` already handles this version-independently, by writing the field itself in `LiveServerMediaSession.cpp`. Porting it would be harmless but redundant |

## If you upgrade live555

All of `0002`..`0005` have to be rebased, and `0002` will never be accepted
upstream (it is ONVIF, not RFC). Each hunk that matters carries a
`Local patch; reapply when upgrading live555.` comment in the source, so
`grep -rn "Local patch" ` in the build tree finds them all.
