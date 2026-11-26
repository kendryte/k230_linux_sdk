# Where these files came from

Copied byte-for-byte from the rt-smart example at
`canmv_k230_dpu/src/rtsmart/examples/mpp/sample_webrtc/`, commit `5cdf479`.

| file | what it is |
|---|---|
| `http_server.{c,h}` | the signaling server: binds every local IPv4 address, accepts one request at a time, hands it to a callback together with the client's address and the local address that accepted it |
| `web_page.h` | the viewer page as one big string literal, served at `/` |

## Local changes

None. Keep it that way: `diff` against the rt-smart tree should stay empty so
that the next sync is a plain copy. Anything Linux-specific belongs in
`../webrtc_out.c`, which is ours.

The one piece of `sample_webrtc/main.c` that could not be copied is the local
IP enumeration -- it goes through rt-smart's `netmgmt_utils_*`. `webrtc_out.c`
does the same job with `getifaddrs()`. Everything else in `webrtc_out.c`
(sessions, routes, `replace_mdns_candidates()`) is that `main.c` with the MPP
parts taken out.

## Why not reuse `package/camera_webrtc_demo`

It carries an older fork of `http_server.c`: a single listener, no local-IP
callback, no startup synchronisation. More to the point it has no
`replace_mdns_candidates()`, so Chrome's mDNS candidate hiding makes it fail
until the user turns off a browser flag. The rt-smart version fixes that
server-side. `camera_webrtc_demo` is deliberately left alone.
