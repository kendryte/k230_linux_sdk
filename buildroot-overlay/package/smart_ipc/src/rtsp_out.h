/*
 * RTSP output for the demos: a C door onto librtsp_server
 *
 * librtsp_server is C++ (KdRtspServer wrapping live555) while the demos are
 * C, so this is the whole of the glue. Open a server, hand it encoder
 * packets as they come out, close it.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef VCODEC_DEMO_RTSP_OUT_H
#define VCODEC_DEMO_RTSP_OUT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct rtsp_out;

/*
 * Start an RTSP server on @port publishing one stream at @path, so clients
 * play rtsp://<board ip>:<port>/<path>. @h265 picks H.265 over H.264; it has
 * to match what the encoder is actually producing, nothing checks.
 *
 * NULL on failure, with the reason already on stderr.
 */
struct rtsp_out *rtsp_out_open(int port, const char *path, int h265);

/* The stream's URL, owned by the handle. Never NULL. */
const char *rtsp_out_url(const struct rtsp_out *r);

/*
 * Hand over one encoder packet, Annex B with start codes, exactly as
 * kd_enc_read_stream() produced it - the server splits NAL units out itself
 * and picks the SPS/PPS up on the way past.
 *
 * The data is copied, so the caller can reuse its buffer straight away.
 * @ts_us is a capture timestamp in microseconds. Returns 0 on success.
 */
int rtsp_out_send(struct rtsp_out *r, const void *data, size_t size,
		  uint64_t ts_us);

void rtsp_out_close(struct rtsp_out *r);

#ifdef __cplusplus
}
#endif

#endif /* VCODEC_DEMO_RTSP_OUT_H */
