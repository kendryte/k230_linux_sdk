/*
 * libvcodec - one-shot JPEG encode
 *
 * A snapshot helper built on top of the public encoder API: open the VPU,
 * push one frame through a KD_CODEC_JPEG session, block for the result, close
 * everything again. Nothing in here is on a video fast path.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define _GNU_SOURCE

#include <string.h>

#include "kd_common.h"

/* Rough upper bound for one still frame on this hardware. Hitting it means
 * something is wrong, not that the picture was slow. */
#define KD_JPEG_TIMEOUT_MS 2000

/* The drain command is only sent once; after that only this many empty polls
 * are tolerated before giving up. */
#define KD_JPEG_MAX_ROUNDS 4

int kd_jpeg_encode(unsigned width, unsigned height, kd_pixfmt pixfmt,
		   const void *src, size_t src_size, unsigned quality,
		   void *dst, size_t dst_cap)
{
	kd_enc_cfg cfg;
	kd_enc *enc = NULL;
	size_t need;
	unsigned round;
	int ret;

	if (src == NULL || dst == NULL || dst_cap == 0 || width == 0 ||
	    height == 0)
		return KD_ERR_INVAL;

	if (kd_pixfmt_fourcc(pixfmt) == 0)
		return KD_ERR_INVAL;

	if (quality > 100)
		return KD_ERR_INVAL;

	need = kd_frame_size(pixfmt, kd_default_stride(pixfmt, width), height);
	if (src_size < need) {
		KD_LOGE("the source frame is %zu bytes, %ux%u needs %zu",
			src_size, width, height, need);
		return KD_ERR_INVAL;
	}

	kd_enc_cfg_default(&cfg);
	cfg.codec = KD_CODEC_JPEG;
	cfg.width = width;
	cfg.height = height;
	cfg.pixfmt = pixfmt;
	cfg.jpeg_quality = quality;
	cfg.bitstream_buffers = 2;

	ret = kd_enc_create(&enc, &cfg);
	if (ret != KD_OK)
		return ret;

	/* No input group attached, so start() allocates one internally and
	 * send_frame_copy() is the way in */
	ret = kd_enc_start(enc);
	if (ret != KD_OK)
		goto out;

	ret = kd_enc_send_frame_copy(enc, src, need, 0);
	if (ret != KD_OK)
		goto out;

	/* One frame in, one picture out: ask for the drain straight away so
	 * the firmware has no reason to hold anything back */
	kd_enc_stop(enc);

	for (round = 0; round < KD_JPEG_MAX_ROUNDS; round++) {
		ret = kd_enc_read_stream(enc, dst, dst_cap, NULL, NULL,
					 KD_JPEG_TIMEOUT_MS);
		if (ret > 0)
			goto out;

		if (ret == KD_ERR_EOS) {
			KD_LOGE("the encoder finished without producing a "
				"picture");
			ret = KD_ERR_IO;
			goto out;
		}

		if (ret != KD_ERR_AGAIN)
			goto out;
	}

	KD_LOGE("no JPEG after %d ms", KD_JPEG_MAX_ROUNDS * KD_JPEG_TIMEOUT_MS);
	ret = KD_ERR_IO;

out:
	kd_enc_destroy(enc);

	return ret;
}
