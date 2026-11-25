/*
 * libvcodec - encoder
 *
 * V4L2 names its queues from the application's point of view, which reads
 * backwards here:
 *   the OUTPUT queue  = raw frames going in
 *   the CAPTURE queue = bitstream coming out
 * The public API only ever says input frame / output bitstream; this mapping
 * lives in this file alone.
 *
 * The frame side has to be multi-planar and the bitstream side single-planar,
 * the same split the ARM reference player uses (mvx_player.cpp, Encoder).
 * It is not a free choice: the driver derives the plane count from the pixel
 * format (NV12 is two planes), and a single-planar queue cannot describe
 * more than one, so QBUF fails on the plane it was never told about.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define _GNU_SOURCE

#include <errno.h>
#include <poll.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "kd_common.h"
#include "kd_mvx.h"

#define KD_ENC_IN_TYPE V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE /* raw frames */
#define KD_ENC_OUT_TYPE V4L2_BUF_TYPE_VIDEO_CAPTURE /* bitstream */

#define KD_ENC_DEFAULT_BITSTREAM_BUFS 4
#define KD_ENC_DEFAULT_INPUT_BUFS 4
#define KD_ENC_MIN_BITSTREAM_SIZE (512 * 1024)

struct kd_enc {
	int fd;
	kd_enc_cfg cfg;

	unsigned stride; /* stride in use, derived when cfg.stride is 0 */
	size_t frame_size; /* bytes in one raw frame */
	size_t bitstream_size; /* bytes in one bitstream buffer */

	kd_bufgroup *in; /* input frames */
	bool in_owned; /* in was created by the library, free it on destroy */
	kd_bufgroup *out; /* output bitstream, always internal */

	bool auto_requeue; /* hand finished frames back to the capture device */

	bool started;
	bool draining; /* ENC_CMD_STOP has been sent */
	bool got_last; /* the packet flagged LAST has been seen */

	bool force_idr; /* code the next frame as IDR */
};

/* ========================================================================
 * Controls
 * ======================================================================== */

static int enc_set_ctrl(struct kd_enc *enc, uint32_t id, int32_t value,
			const char *what, bool fatal)
{
	struct v4l2_control ctrl = { .id = id, .value = value };

	if (kd_ioctl(enc->fd, VIDIOC_S_CTRL, &ctrl) == 0)
		return KD_OK;

	if (fatal) {
		KD_LOGE("setting %s to %d failed: %s", what, value,
			strerror(errno));
		return kd_errno_to_err(errno);
	}

	KD_LOGW("setting %s to %d failed: %s (ignored, the driver default "
		"stays)",
		what, value, strerror(errno));

	return KD_OK;
}

static int enc_apply_profile(struct kd_enc *enc)
{
	const kd_enc_cfg *cfg = &enc->cfg;

	if (cfg->profile == KD_PROFILE_DEFAULT && cfg->level == 0)
		return KD_OK;

	if (cfg->codec == KD_CODEC_H264) {
		int32_t p;

		switch (cfg->profile) {
		case KD_PROFILE_BASELINE:
			p = V4L2_MPEG_VIDEO_H264_PROFILE_BASELINE;
			break;
		case KD_PROFILE_MAIN:
			p = V4L2_MPEG_VIDEO_H264_PROFILE_MAIN;
			break;
		case KD_PROFILE_HIGH:
			p = V4L2_MPEG_VIDEO_H264_PROFILE_HIGH;
			break;
		default:
			p = -1;
			break;
		}

		if (p >= 0)
			enc_set_ctrl(enc, V4L2_CID_MPEG_VIDEO_H264_PROFILE, p,
				     "H.264 profile", false);

		if (cfg->level != 0)
			enc_set_ctrl(enc, V4L2_CID_MPEG_VIDEO_H264_LEVEL,
				     (int32_t)cfg->level, "H.264 level", false);

		return KD_OK;
	}

	if (cfg->codec == KD_CODEC_H265) {
		/* The driver only accepts the private controls for H.265;
		 * the standard HEVC ones are not registered */
		int32_t p = KD_MVE_H265_PROFILE_MAIN;

		if (cfg->profile == KD_PROFILE_BASELINE ||
		    cfg->profile == KD_PROFILE_HIGH)
			KD_LOGW("H.265 has no baseline/high profile, using main");

		enc_set_ctrl(enc, KD_CID_MVE_H265_PROFILE, p, "H.265 profile",
			     false);

		if (cfg->level != 0)
			enc_set_ctrl(enc, KD_CID_MVE_H265_LEVEL,
				     (int32_t)cfg->level, "H.265 level", false);
	}

	return KD_OK;
}

static int enc_apply_controls(struct kd_enc *enc)
{
	const kd_enc_cfg *cfg = &enc->cfg;

	/* The bitstream needs start codes or the file will not play. The
	 * driver already defaults to 0; set it anyway. */
	enc_set_ctrl(enc, KD_CID_MVE_NALU_FORMAT, KD_NALU_FORMAT_START_CODES,
		     "NALU format", false);

	/* The frame rate control is Q16 fixed point, not integer fps */
	enc_set_ctrl(enc, KD_CID_MVE_FRAME_RATE,
		     (int32_t)(cfg->framerate << 16), "frame rate", false);

	if (cfg->codec == KD_CODEC_JPEG) {
		if (cfg->jpeg_quality != 0)
			enc_set_ctrl(enc, V4L2_CID_JPEG_COMPRESSION_QUALITY,
				     (int32_t)cfg->jpeg_quality,
				     "JPEG quality", false);
		return KD_OK;
	}

	if (cfg->gop > 0)
		enc_set_ctrl(enc, V4L2_CID_MPEG_VIDEO_GOP_SIZE,
			     (int32_t)cfg->gop, "GOP size", false);

	if (cfg->bitrate > 0) {
		enc_set_ctrl(enc, V4L2_CID_MPEG_VIDEO_FRAME_RC_ENABLE, 1,
			     "rate control", false);
		enc_set_ctrl(enc, V4L2_CID_MPEG_VIDEO_BITRATE,
			     (int32_t)cfg->bitrate, "bitrate", false);
	}

	return enc_apply_profile(enc);
}

/* ========================================================================
 * Formats
 * ======================================================================== */

static int enc_set_formats(struct kd_enc *enc)
{
	const kd_enc_cfg *cfg = &enc->cfg;
	struct v4l2_format f;

	/*
	 * The order follows the kernel stateful encoder specification: coded
	 * format first, raw format second. When the raw format is set the
	 * driver copies the colorspace over to the coded side
	 * (mvx_v4l2_vidioc.c:976-979).
	 */
	memset(&f, 0, sizeof(f));
	f.type = KD_ENC_OUT_TYPE;
	f.fmt.pix.width = cfg->width;
	f.fmt.pix.height = cfg->height;
	f.fmt.pix.pixelformat = kd_codec_fourcc(cfg->codec);
	f.fmt.pix.field = V4L2_FIELD_NONE;
	f.fmt.pix.sizeimage = (uint32_t)enc->bitstream_size;

	if (kd_ioctl(enc->fd, VIDIOC_S_FMT, &f) < 0) {
		KD_LOGE("setting the coded format failed (%ux%u fourcc=0x%08x): %s",
			cfg->width, cfg->height, kd_codec_fourcc(cfg->codec),
			strerror(errno));
		return kd_errno_to_err(errno);
	}

	if (f.fmt.pix.sizeimage > enc->bitstream_size)
		enc->bitstream_size = f.fmt.pix.sizeimage;

	/*
	 * The frame side is multi-planar. num_planes is left at 1 on purpose:
	 * the driver overrides it from the pixel format anyway (get_stride()),
	 * and passing 1 means only plane 0's stride is taken as a request
	 * while the chroma strides are derived.
	 */
	memset(&f, 0, sizeof(f));
	f.type = KD_ENC_IN_TYPE;
	f.fmt.pix_mp.width = cfg->width;
	f.fmt.pix_mp.height = cfg->height;
	f.fmt.pix_mp.pixelformat = kd_pixfmt_fourcc(cfg->pixfmt);
	f.fmt.pix_mp.field = V4L2_FIELD_NONE;
	f.fmt.pix_mp.num_planes = 1;
	f.fmt.pix_mp.plane_fmt[0].bytesperline = enc->stride;
	f.fmt.pix_mp.plane_fmt[0].sizeimage = (uint32_t)enc->frame_size;

	if (kd_ioctl(enc->fd, VIDIOC_S_FMT, &f) < 0) {
		KD_LOGE("setting the input frame format failed "
			"(%ux%u stride=%u fourcc=0x%08x): %s",
			cfg->width, cfg->height, enc->stride,
			kd_pixfmt_fourcc(cfg->pixfmt), strerror(errno));
		return kd_errno_to_err(errno);
	}

	/*
	 * The driver may adjust stride and sizeimage. A larger value has to
	 * be adopted or external dmabufs will not match; a smaller one can be
	 * ignored.
	 */
	if (f.fmt.pix_mp.plane_fmt[0].bytesperline != enc->stride) {
		KD_LOGW("the driver changed the stride from %u to %u",
			enc->stride, f.fmt.pix_mp.plane_fmt[0].bytesperline);
		enc->stride = f.fmt.pix_mp.plane_fmt[0].bytesperline;
	}

	{
		size_t total = 0;
		unsigned p;

		for (p = 0; p < f.fmt.pix_mp.num_planes && p < KD_MAX_PLANES;
		     p++)
			total += f.fmt.pix_mp.plane_fmt[p].sizeimage;

		if (total > enc->frame_size)
			enc->frame_size = total;

		KD_LOGI("encoder ready: input %ux%u stride=%u frame=%zu bytes "
			"over %u plane(s), bitstream buffer=%zu bytes",
			cfg->width, cfg->height, enc->stride, enc->frame_size,
			f.fmt.pix_mp.num_planes, enc->bitstream_size);
	}

	return KD_OK;
}

/* ========================================================================
 * Create and destroy
 * ======================================================================== */

void kd_enc_cfg_default(kd_enc_cfg *cfg)
{
	if (cfg == NULL)
		return;

	memset(cfg, 0, sizeof(*cfg));
	cfg->codec = KD_CODEC_H264;
	cfg->pixfmt = KD_PIXFMT_NV12;
	cfg->framerate = 30;
	cfg->bitstream_buffers = KD_ENC_DEFAULT_BITSTREAM_BUFS;
}

int kd_enc_create(kd_enc **out, const kd_enc_cfg *cfg)
{
	struct kd_enc *enc;
	int fd;

	if (out == NULL || cfg == NULL)
		return KD_ERR_INVAL;

	if (cfg->width == 0 || cfg->height == 0) {
		KD_LOGE("width and height must not be 0");
		return KD_ERR_INVAL;
	}

	if (kd_pixfmt_fourcc(cfg->pixfmt) == 0 ||
	    kd_codec_fourcc(cfg->codec) == 0) {
		KD_LOGE("unknown pixel format or codec");
		return KD_ERR_INVAL;
	}

	fd = kd_open_device(cfg->dev);
	if (fd < 0)
		return fd;

	enc = calloc(1, sizeof(*enc));
	if (enc == NULL) {
		close(fd);
		return KD_ERR_NOMEM;
	}

	enc->fd = fd;
	enc->cfg = *cfg;

	if (enc->cfg.framerate == 0)
		enc->cfg.framerate = 30;

	if (enc->cfg.gop == 0 && enc->cfg.codec != KD_CODEC_JPEG)
		enc->cfg.gop = enc->cfg.framerate * 2;

	if (enc->cfg.bitrate == 0 && enc->cfg.codec != KD_CODEC_JPEG) {
		/* No bitrate given: guess one from resolution and frame rate.
		 * Good enough to start with, not meant to be exact. */
		uint64_t bps = (uint64_t)cfg->width * cfg->height *
			       enc->cfg.framerate / 15;

		if (bps < 1000)
			bps = 1000;
		if (bps > 1000000000)
			bps = 1000000000;

		enc->cfg.bitrate = (unsigned)bps;
		KD_LOGI("no bitrate given, using %u bps for %ux%u@%ufps",
			enc->cfg.bitrate, cfg->width, cfg->height,
			enc->cfg.framerate);
	}

	if (enc->cfg.bitstream_buffers == 0)
		enc->cfg.bitstream_buffers = KD_ENC_DEFAULT_BITSTREAM_BUFS;

	enc->stride = cfg->stride != 0 ?
			      cfg->stride :
			      kd_default_stride(cfg->pixfmt, cfg->width);
	enc->frame_size = kd_frame_size(cfg->pixfmt, enc->stride, cfg->height);

	enc->bitstream_size = (size_t)cfg->width * cfg->height / 2;
	if (enc->bitstream_size < KD_ENC_MIN_BITSTREAM_SIZE)
		enc->bitstream_size = KD_ENC_MIN_BITSTREAM_SIZE;

	*out = enc;

	return KD_OK;
}

int kd_enc_attach_input(kd_enc *enc, kd_bufgroup *grp)
{
	if (enc == NULL || grp == NULL)
		return KD_ERR_INVAL;

	if (enc->started) {
		KD_LOGE("the encoder is started, input buffers can no longer "
			"be changed");
		return KD_ERR_STATE;
	}

	if (enc->in != NULL) {
		KD_LOGE("input buffers can only be attached once");
		return KD_ERR_STATE;
	}

	enc->in = grp;
	enc->in_owned = false;

	/* The library can only requeue frames when it knows who owns them,
	 * which is the case for kd_bufgroup_create_from_v4l2() groups */
	enc->auto_requeue = grp->src_fd >= 0;

	return KD_OK;
}

int kd_enc_set_auto_requeue(kd_enc *enc, bool enable)
{
	if (enc == NULL)
		return KD_ERR_INVAL;

	if (enable && (enc->in == NULL || enc->in->src_fd < 0)) {
		KD_LOGE("auto-requeue needs an input group built by "
			"kd_bufgroup_create_from_v4l2()");
		return KD_ERR_STATE;
	}

	enc->auto_requeue = enable;

	return KD_OK;
}

int kd_enc_start(kd_enc *enc)
{
	int ret;

	if (enc == NULL)
		return KD_ERR_INVAL;

	if (enc->started)
		return KD_ERR_STATE;

	ret = enc_set_formats(enc);
	if (ret != KD_OK)
		return ret;

	ret = enc_apply_controls(enc);
	if (ret != KD_OK)
		return ret;

	if (enc->in == NULL) {
		/* Nothing attached, so allocate a group that only
		 * kd_enc_send_frame_copy() can use */
		ret = kd_bufgroup_create_internal(&enc->in,
						  KD_ENC_DEFAULT_INPUT_BUFS);
		if (ret != KD_OK)
			return ret;

		enc->in_owned = true;
	}

	ret = kd_bufgroup_attach(enc->in, enc->fd, KD_ENC_IN_TYPE,
				 enc->frame_size);
	if (ret != KD_OK) {
		KD_LOGE("attaching the input buffers failed: %s",
			kd_strerror(ret));
		return ret;
	}

	ret = kd_bufgroup_create_internal(&enc->out,
					  enc->cfg.bitstream_buffers);
	if (ret != KD_OK)
		goto detach_in;

	ret = kd_bufgroup_attach(enc->out, enc->fd, KD_ENC_OUT_TYPE,
				 enc->bitstream_size);
	if (ret != KD_OK) {
		KD_LOGE("allocating the bitstream buffers failed: %s",
			kd_strerror(ret));
		goto free_out;
	}

	/* Queue every bitstream buffer up front so the encoder has somewhere
	 * to write */
	ret = kd_bufgroup_queue_all(enc->out);
	if (ret != KD_OK)
		goto detach_out;

	ret = kd_streamon(enc->fd, KD_ENC_IN_TYPE);
	if (ret != KD_OK)
		goto detach_out;

	ret = kd_streamon(enc->fd, KD_ENC_OUT_TYPE);
	if (ret != KD_OK) {
		kd_streamoff(enc->fd, KD_ENC_IN_TYPE);
		goto detach_out;
	}

	enc->started = true;

	return KD_OK;

detach_out:
	kd_bufgroup_detach(enc->out);
free_out:
	kd_bufgroup_destroy(enc->out);
	enc->out = NULL;
detach_in:
	kd_bufgroup_detach(enc->in);

	return ret;
}

void kd_enc_destroy(kd_enc *enc)
{
	if (enc == NULL)
		return;

	if (enc->started) {
		kd_streamoff(enc->fd, KD_ENC_IN_TYPE);
		kd_streamoff(enc->fd, KD_ENC_OUT_TYPE);
		enc->started = false;
	}

	if (enc->out != NULL) {
		kd_bufgroup_detach(enc->out);
		kd_bufgroup_destroy(enc->out);
	}

	if (enc->in != NULL) {
		kd_bufgroup_detach(enc->in);
		if (enc->in_owned)
			kd_bufgroup_destroy(enc->in);
	}

	if (enc->fd >= 0)
		close(enc->fd);

	free(enc);
}

/* ========================================================================
 * Feeding frames
 * ======================================================================== */

/**
 * Collect the input frames the VPU is done with and hand them straight back
 * to the capture device. Runs on every send and every stream fetch, which is
 * often enough that the camera never starves.
 */
static void enc_requeue_finished(struct kd_enc *enc)
{
	if (!enc->auto_requeue || !enc->started)
		return;

	for (;;) {
		struct kd_dqinfo info;

		if (kd_bufgroup_dqbuf(enc->in, &info) != KD_OK)
			return;

		kd_bufgroup_requeue_source(enc->in, info.index);
	}
}

int kd_enc_send_frame(kd_enc *enc, unsigned index, int64_t pts)
{
	uint32_t flags = 0;
	int ret;

	if (enc == NULL)
		return KD_ERR_INVAL;

	if (!enc->started)
		return KD_ERR_STATE;

	if (enc->draining) {
		KD_LOGE("the encoder is draining, no more frames accepted");
		return KD_ERR_STATE;
	}

	enc_requeue_finished(enc);

	/*
	 * The driver has no FORCE_KEY_FRAME control; a forced IDR is
	 * requested with V4L2_BUF_FLAG_KEYFRAME on the input buffer, which
	 * mvx_v4l2_buffer.c maps to MVX_BUFFER_FRAME_FLAG_FORCE_IDR. The same
	 * flag means end-of-frame on decoder input, so do not mix the two up.
	 */
	if (enc->force_idr)
		flags |= V4L2_BUF_FLAG_KEYFRAME;

	ret = kd_bufgroup_qbuf(enc->in, index, enc->frame_size, pts, flags);
	if (ret == KD_OK)
		enc->force_idr = false;

	return ret;
}

int kd_enc_send_frame_copy(kd_enc *enc, const void *data, size_t size,
			   int64_t pts)
{
	const unsigned char *src;
	size_t left;
	unsigned i, p;
	void *dst;

	if (enc == NULL || data == NULL)
		return KD_ERR_INVAL;

	if (!enc->started)
		return KD_ERR_STATE;

	if (enc->in->external) {
		KD_LOGE("the input group holds external dmabufs, use "
			"kd_enc_send_frame()");
		return KD_ERR_STATE;
	}

	if (size > enc->frame_size) {
		KD_LOGE("got %zu bytes, a frame is at most %zu", size,
			enc->frame_size);
		return KD_ERR_INVAL;
	}

	/* Look for a free slot, reclaiming a finished one if there is none */
	for (i = 0; i < enc->in->count; i++)
		if (!enc->in->slots[i].queued)
			break;

	if (i == enc->in->count) {
		struct kd_dqinfo info;
		int ret = kd_bufgroup_dqbuf(enc->in, &info);

		if (ret != KD_OK)
			return ret;

		i = info.index;
	}

	/*
	 * The caller hands over one contiguous frame, but the driver maps each
	 * plane separately, so split it on the way in.
	 */
	src = data;
	left = size;

	for (p = 0; p < enc->in->nplanes && left > 0; p++) {
		size_t n = enc->in->slots[i].plane[p].size;

		dst = enc->in->slots[i].plane[p].addr;
		if (dst == NULL)
			return KD_ERR_STATE;

		if (n > left)
			n = left;

		memcpy(dst, src, n);
		src += n;
		left -= n;
	}

	return kd_enc_send_frame(enc, i, pts);
}

int kd_enc_reclaim_frame(kd_enc *enc, unsigned *index)
{
	struct kd_dqinfo info;
	int ret;

	if (enc == NULL || index == NULL)
		return KD_ERR_INVAL;

	if (!enc->started)
		return KD_ERR_STATE;

	if (enc->auto_requeue) {
		KD_LOGE("auto-requeue is on, the library already returns "
			"finished frames; call kd_enc_set_auto_requeue(enc, "
			"false) to take over");
		return KD_ERR_STATE;
	}

	ret = kd_bufgroup_dqbuf(enc->in, &info);
	if (ret != KD_OK)
		return ret;

	*index = info.index;

	return KD_OK;
}

/* ========================================================================
 * Collecting the bitstream
 * ======================================================================== */

int kd_enc_get_stream(kd_enc *enc, kd_packet *pkt, int timeout_ms)
{
	if (enc == NULL || pkt == NULL)
		return KD_ERR_INVAL;

	if (!enc->started)
		return KD_ERR_STATE;

	if (enc->got_last)
		return KD_ERR_EOS;

	enc_requeue_finished(enc);

	for (;;) {
		struct kd_dqinfo info;
		bool last;
		int ret;

		ret = kd_bufgroup_dqbuf(enc->out, &info);

		if (ret == KD_ERR_AGAIN) {
			short revents = 0;

			if (timeout_ms == 0)
				return KD_ERR_AGAIN;

			ret = kd_poll(enc->fd, POLLIN, timeout_ms, &revents);
			if (ret != KD_OK)
				return ret;

			if ((revents & POLLERR) != 0) {
				KD_LOGE("the encoder reported an error (poll "
					"returned POLLERR), see "
					"dmesg | grep -i mvx");
				return KD_ERR_IO;
			}

			if ((revents & POLLIN) == 0)
				return KD_ERR_AGAIN;

			continue;
		}

		if (ret != KD_OK)
			return ret;

		last = (info.flags & V4L2_BUF_FLAG_LAST) != 0;

		if (info.bytesused == 0) {
			/*
			 * Empty packet. The one closing a drain usually is, so
			 * treat that as end of stream; otherwise put the
			 * buffer back and keep waiting.
			 */
			kd_bufgroup_qbuf(enc->out, info.index, 0, 0, 0);

			if (last) {
				enc->got_last = true;
				return KD_ERR_EOS;
			}

			if (timeout_ms == 0)
				return KD_ERR_AGAIN;

			continue;
		}

		/* The bitstream queue is single-planar, so plane 0 is all of it */
		pkt->data = enc->out->slots[info.index].plane[0].addr;
		pkt->size = info.bytesused;
		pkt->pts = info.pts;
		/*
		 * V4L2_BUF_FLAG_KEYFRAME cannot be used here: on the output
		 * direction the driver uses it to mean end-of-frame. The real
		 * frame type is in the top byte of reserved2, 0 being I.
		 */
		pkt->keyframe =
			KD_RESERVED2_FRAME_TYPE(info.reserved2) ==
			KD_MVX_FRAME_TYPE_I;
		pkt->eos = last;
		pkt->_slot = info.index;

		if (last)
			enc->got_last = true;

		return KD_OK;
	}
}

int kd_enc_read_stream(kd_enc *enc, void *buf, size_t cap, int64_t *pts,
		       bool *keyframe, int timeout_ms)
{
	kd_packet pkt;
	int ret;

	if (enc == NULL || buf == NULL || cap == 0)
		return KD_ERR_INVAL;

	ret = kd_enc_get_stream(enc, &pkt, timeout_ms);
	if (ret != KD_OK)
		return ret;

	if (pkt.size > cap) {
		KD_LOGE("a %zu byte packet does not fit in %zu bytes and was "
			"dropped; size the buffer for the largest frame",
			pkt.size, cap);
		kd_enc_release_stream(enc, &pkt);
		return KD_ERR_NOMEM;
	}

	memcpy(buf, pkt.data, pkt.size);

	if (pts != NULL)
		*pts = pkt.pts;
	if (keyframe != NULL)
		*keyframe = pkt.keyframe;

	ret = (int)pkt.size;
	kd_enc_release_stream(enc, &pkt);

	return ret;
}

int kd_enc_release_stream(kd_enc *enc, kd_packet *pkt)
{
	if (enc == NULL || pkt == NULL)
		return KD_ERR_INVAL;

	if (pkt->_slot >= enc->out->count)
		return KD_ERR_INVAL;

	return kd_bufgroup_qbuf(enc->out, pkt->_slot, 0, 0, 0);
}

/* ========================================================================
 * Runtime control and shutdown
 * ======================================================================== */

int kd_enc_set_bitrate(kd_enc *enc, unsigned bps)
{
	if (enc == NULL || bps == 0)
		return KD_ERR_INVAL;

	enc->cfg.bitrate = bps;

	return enc_set_ctrl(enc, V4L2_CID_MPEG_VIDEO_BITRATE, (int32_t)bps,
			    "bitrate", true);
}

int kd_enc_force_idr(kd_enc *enc)
{
	if (enc == NULL)
		return KD_ERR_INVAL;

	enc->force_idr = true;

	return KD_OK;
}

int kd_enc_get_fd(kd_enc *enc)
{
	return enc != NULL ? enc->fd : -1;
}

int kd_enc_stop(kd_enc *enc)
{
	struct v4l2_encoder_cmd cmd;

	if (enc == NULL)
		return KD_ERR_INVAL;

	if (!enc->started)
		return KD_ERR_STATE;

	if (enc->draining)
		return KD_OK;

	memset(&cmd, 0, sizeof(cmd));
	cmd.cmd = V4L2_ENC_CMD_STOP;

	if (kd_ioctl(enc->fd, VIDIOC_ENCODER_CMD, &cmd) < 0) {
		KD_LOGE("ENC_CMD_STOP failed: %s", strerror(errno));
		return kd_errno_to_err(errno);
	}

	enc->draining = true;

	return KD_OK;
}
