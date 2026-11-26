/*
 * WebRTC output for the demos: encoder packets straight into a browser
 *
 * Deliberately shaped like rtsp_out.h, because it is the other half of the
 * same choice: RTSP needs VLC or ffplay, this one needs only the address bar.
 * Open a server, hand it encoder packets as they come out, close it.
 *
 * Underneath are libpeer and the signaling server copied from the rt-smart
 * sample_webrtc example, see webrtc/README.md.
 *
 * No STUN or TURN is configured, by design: an IPC and its viewer are on the
 * same LAN, and the box should not depend on a public server to show a
 * picture. The flip side is that this will not traverse a NAT.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef VCODEC_DEMO_WEBRTC_OUT_H
#define VCODEC_DEMO_WEBRTC_OUT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct webrtc_out;

/*
 * Start the signaling server on @port and serve the viewer page at "/", so a
 * browser opens http://<board ip>:<port>/. @h265 picks H.265 over H.264; it
 * has to match what the encoder is actually producing, nothing checks.
 *
 * Up to four browsers at once; a fifth is turned away with 503.
 *
 * NULL on failure, with the reason already on stderr.
 */
struct webrtc_out *webrtc_out_open(int port, int h265);

/* The page's URL, owned by the handle. Never NULL. */
const char *webrtc_out_url(const struct webrtc_out *w);

/*
 * Hand over one encoder packet, Annex B with start codes, exactly as
 * kd_enc_read_stream() produced it.
 *
 * Unlike rtsp_out_send() this wants to know whether the packet is a keyframe:
 * every I-frame has to go out with the parameter sets in front of it or a
 * browser joining mid-stream has nothing to initialise its decoder with. The
 * parameter sets are picked up off the stream as it goes past and cached, so
 * the caller has nothing to do beyond passing @keyframe through.
 *
 * @ts_us is the encoder's own timestamp in microseconds (unlike the RTSP path,
 * this one really is used - nothing downstream re-stamps the frame).
 *
 * Returns immediately: the packet is copied into a queue that a sender thread
 * drains. Returns 0 on success, -1 if the queue was full and the oldest packet
 * had to be dropped.
 */
int webrtc_out_send(struct webrtc_out *w, const void *data, size_t size,
		    uint64_t ts_us, int keyframe);

/* How many packets had to be dropped because the queue was full. */
unsigned long webrtc_out_drops(const struct webrtc_out *w);

/*
 * Non-zero if a browser has asked for a keyframe since the last call, and
 * clears the request. A browser that joins mid-GOP sees nothing until the
 * next I-frame, so the caller should answer this with kd_enc_force_idr().
 */
int webrtc_out_take_keyframe_request(struct webrtc_out *w);

void webrtc_out_close(struct webrtc_out *w);

#ifdef __cplusplus
}
#endif

#endif /* VCODEC_DEMO_WEBRTC_OUT_H */
