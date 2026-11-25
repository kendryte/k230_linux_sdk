/*
 * libvcodec - decoder
 *
 *   the OUTPUT queue  = bitstream going in
 *   the CAPTURE queue = frames coming out
 *
 * Mirror image of the encoder: the frame side is multi-planar and the
 * bitstream side single-planar, because the driver derives the plane count
 * from the pixel format. See the header of kd_enc.c.
 *
 * The big difference from the encoder is that the resolution is only known
 * once decoding starts, so the real output buffers cannot be attached up
 * front. The decoder section of vcodec.h spells out both ways to deal with
 * that; dec_probe_output() below explains why a placeholder buffer still has
 * to be attached before anything is decoded at all.
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

#define KD_DEC_IN_TYPE V4L2_BUF_TYPE_VIDEO_OUTPUT /* bitstream */
#define KD_DEC_OUT_TYPE V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE /* frames */

#define KD_DEC_DEFAULT_STREAM_BUFS 4
#define KD_DEC_DEFAULT_STREAM_SIZE (1024 * 1024)

/*
 * Geometry used to bring the frame queue up before the real resolution is
 * known, see dec_probe_output(). Only has to be non-zero and plausible; small
 * keeps the throwaway allocation cheap.
 */
#define KD_DEC_PROBE_WIDTH 320
#define KD_DEC_PROBE_HEIGHT 240

struct kd_dec {
	int fd;
	kd_dec_cfg cfg;

	kd_bufgroup *in; /* bitstream, internal */
	kd_bufgroup *out; /* frames, supplied by the application */
	bool out_owned; /* out was allocated by us, not by the application */

	bool started;
	bool out_streaming;
	bool probing; /* out is the throwaway group from dec_probe_output() */
	bool draining;
	bool got_last;

	bool info_valid;
	kd_dec_frameinfo info;
};

/* ========================================================================
 * Formats and events
 * ======================================================================== */

static int dec_subscribe(struct kd_dec *dec, uint32_t type)
{
	struct v4l2_event_subscription sub;

	memset(&sub, 0, sizeof(sub));
	sub.type = type;

	if (kd_ioctl(dec->fd, VIDIOC_SUBSCRIBE_EVENT, &sub) < 0) {
		KD_LOGE("subscribing to event %u failed: %s", type,
			strerror(errno));
		return kd_errno_to_err(errno);
	}

	return KD_OK;
}

static int dec_set_formats(struct kd_dec *dec)
{
	const kd_dec_cfg *cfg = &dec->cfg;
	struct v4l2_format f;
	unsigned w = cfg->width != 0 ? cfg->width : KD_DEC_PROBE_WIDTH;
	unsigned h = cfg->height != 0 ? cfg->height : KD_DEC_PROBE_HEIGHT;

	/*
	 * Coded format first. In this driver the input size decides the
	 * output size (end of mvx_session_set_format in mvx_session.c), so
	 * the width and height given here are only a hint: fill them in when
	 * the application knows them, leave them 0 otherwise, and the real
	 * size arrives with the SOURCE_CHANGE event once the firmware has
	 * parsed the stream.
	 *
	 * Zero is not passed on to the driver though. try_format() has no
	 * lower bound, so 0x0 would make the frame buffers come out 0 bytes
	 * long, and dec_probe_output() needs to allocate one before the
	 * resolution is known.
	 */
	memset(&f, 0, sizeof(f));
	f.type = KD_DEC_IN_TYPE;
	f.fmt.pix.width = w;
	f.fmt.pix.height = h;
	f.fmt.pix.pixelformat = kd_codec_fourcc(cfg->codec);
	f.fmt.pix.field = V4L2_FIELD_NONE;
	f.fmt.pix.sizeimage = (uint32_t)cfg->stream_buffer_size;

	if (kd_ioctl(dec->fd, VIDIOC_S_FMT, &f) < 0) {
		KD_LOGE("setting the coded format failed (fourcc=0x%08x): %s",
			kd_codec_fourcc(cfg->codec), strerror(errno));
		return kd_errno_to_err(errno);
	}

	if (f.fmt.pix.sizeimage > dec->cfg.stream_buffer_size)
		dec->cfg.stream_buffer_size = f.fmt.pix.sizeimage;

	/* Then pick the output frame format. The driver overrides width and
	 * height from the coded side, so this really only selects a fourcc.
	 * The frame side is multi-planar, see the file header. */
	memset(&f, 0, sizeof(f));
	f.type = KD_DEC_OUT_TYPE;
	f.fmt.pix_mp.width = w;
	f.fmt.pix_mp.height = h;
	f.fmt.pix_mp.pixelformat = kd_pixfmt_fourcc(cfg->pixfmt);
	f.fmt.pix_mp.field = V4L2_FIELD_NONE;
	f.fmt.pix_mp.num_planes = 1;

	if (kd_ioctl(dec->fd, VIDIOC_S_FMT, &f) < 0) {
		KD_LOGE("setting the output frame format failed "
			"(fourcc=0x%08x): %s",
			kd_pixfmt_fourcc(cfg->pixfmt), strerror(errno));
		return kd_errno_to_err(errno);
	}

	return KD_OK;
}

/**
 * Ask the firmware to scale the frames down on their way out.
 *
 * Must happen after dec_set_formats() - the driver works the output size out
 * from the input port and would overwrite this - and before either queue
 * starts streaming, because that is when the firmware is loaded and the
 * driver stops accepting the option. See kd_mvx.h.
 */
static int dec_set_downscale(struct kd_dec *dec)
{
	struct kd_mvx_dsl_frame dsl;

	if (dec->cfg.scale_width == 0 || dec->cfg.scale_height == 0)
		return KD_OK;

	memset(&dsl, 0, sizeof(dsl));
	dsl.width = dec->cfg.scale_width;
	dsl.height = dec->cfg.scale_height;

	if (kd_ioctl(dec->fd, KD_VIDIOC_S_MVX_DSL_FRAME, &dsl) < 0) {
		KD_LOGE("asking the decoder to scale down to %ux%u failed: %s",
			dsl.width, dsl.height, strerror(errno));
		return kd_errno_to_err(errno);
	}

	KD_LOGI("decoder will scale its output down to %ux%u", dsl.width,
		dsl.height);

	return KD_OK;
}

static int dec_refresh_frameinfo(struct kd_dec *dec)
{
	struct v4l2_format f;
	struct v4l2_control ctrl;

	memset(&f, 0, sizeof(f));
	f.type = KD_DEC_OUT_TYPE;

	if (kd_ioctl(dec->fd, VIDIOC_G_FMT, &f) < 0) {
		KD_LOGE("reading back the output frame format failed: %s",
			strerror(errno));
		return kd_errno_to_err(errno);
	}

	dec->info.width = f.fmt.pix_mp.width;
	dec->info.height = f.fmt.pix_mp.height;
	dec->info.stride = f.fmt.pix_mp.plane_fmt[0].bytesperline;

	dec->info.buffer_size = 0;
	{
		unsigned p;

		dec->info.nplanes = f.fmt.pix_mp.num_planes;
		if (dec->info.nplanes > KD_FRAME_MAX_PLANES)
			dec->info.nplanes = KD_FRAME_MAX_PLANES;

		memset(dec->info.plane_size, 0, sizeof(dec->info.plane_size));
		memset(dec->info.plane_stride, 0,
		       sizeof(dec->info.plane_stride));

		for (p = 0; p < dec->info.nplanes; p++) {
			dec->info.plane_size[p] =
				f.fmt.pix_mp.plane_fmt[p].sizeimage;
			dec->info.plane_stride[p] =
				f.fmt.pix_mp.plane_fmt[p].bytesperline;
			dec->info.buffer_size += dec->info.plane_size[p];
		}
	}

	if (kd_fourcc_pixfmt(f.fmt.pix_mp.pixelformat, &dec->info.pixfmt) !=
	    KD_OK)
		dec->info.pixfmt = dec->cfg.pixfmt;

	if (dec->info.stride == 0)
		dec->info.stride =
			kd_default_stride(dec->info.pixfmt, dec->info.width);

	if (dec->info.buffer_size == 0)
		dec->info.buffer_size = kd_frame_size(
			dec->info.pixfmt, dec->info.stride, dec->info.height);

	/*
	 * A driver that answered G_FMT but left the plane sizes at zero would
	 * leave the application nothing to allocate from. Fall back to one
	 * plane holding the whole frame, which is at least self-consistent,
	 * and let attaching complain if the queue really wanted more.
	 */
	if (dec->info.nplanes == 0 || dec->info.plane_size[0] == 0) {
		dec->info.nplanes = 1;
		dec->info.plane_size[0] = dec->info.buffer_size;
		dec->info.plane_stride[0] = dec->info.stride;
	}

	/* Smallest number of output buffers the driver will work with */
	memset(&ctrl, 0, sizeof(ctrl));
	ctrl.id = V4L2_CID_MIN_BUFFERS_FOR_CAPTURE;
	dec->info.min_buffers =
		kd_ioctl(dec->fd, VIDIOC_G_CTRL, &ctrl) == 0 ?
			(unsigned)ctrl.value :
			4;

	dec->info_valid = true;

	KD_LOGI("decoder output: %ux%u stride=%u %zu bytes per frame, "
		"at least %u buffers needed",
		dec->info.width, dec->info.height, dec->info.stride,
		dec->info.buffer_size, dec->info.min_buffers);

	return KD_OK;
}

/**
 * Drain the event queue.
 * Returns KD_ERR_SRC_CHANGE when the resolution changed (frameinfo has
 * already been refreshed), KD_OK when there is nothing to report.
 */
static int dec_drain_events(struct kd_dec *dec)
{
	int result = KD_OK;

	for (;;) {
		struct v4l2_event ev;

		memset(&ev, 0, sizeof(ev));

		if (kd_ioctl(dec->fd, VIDIOC_DQEVENT, &ev) < 0) {
			if (errno != ENOENT)
				KD_LOGW("DQEVENT failed: %s", strerror(errno));
			break;
		}

		switch (ev.type) {
		case V4L2_EVENT_SOURCE_CHANGE:
			KD_LOGI("SOURCE_CHANGE: the resolution is now known "
				"or has changed");
			dec_refresh_frameinfo(dec);
			result = KD_ERR_SRC_CHANGE;
			break;
		case V4L2_EVENT_EOS:
			KD_LOGI("EOS event received");
			break;
		default:
			break;
		}
	}

	return result;
}

/* ========================================================================
 * Create and destroy
 * ======================================================================== */

void kd_dec_cfg_default(kd_dec_cfg *cfg)
{
	if (cfg == NULL)
		return;

	memset(cfg, 0, sizeof(*cfg));
	cfg->codec = KD_CODEC_H264;
	cfg->pixfmt = KD_PIXFMT_NV12;
	cfg->stream_buffers = KD_DEC_DEFAULT_STREAM_BUFS;
	cfg->stream_buffer_size = KD_DEC_DEFAULT_STREAM_SIZE;
}

int kd_dec_create(kd_dec **out, const kd_dec_cfg *cfg)
{
	struct kd_dec *dec;
	int fd;

	if (out == NULL || cfg == NULL)
		return KD_ERR_INVAL;

	if (kd_codec_fourcc(cfg->codec) == 0 ||
	    kd_pixfmt_fourcc(cfg->pixfmt) == 0) {
		KD_LOGE("unknown codec or pixel format");
		return KD_ERR_INVAL;
	}

	fd = kd_open_device(cfg->dev);
	if (fd < 0)
		return fd;

	dec = calloc(1, sizeof(*dec));
	if (dec == NULL) {
		close(fd);
		return KD_ERR_NOMEM;
	}

	dec->fd = fd;
	dec->cfg = *cfg;

	if (dec->cfg.stream_buffers == 0)
		dec->cfg.stream_buffers = KD_DEC_DEFAULT_STREAM_BUFS;

	if (dec->cfg.stream_buffer_size == 0)
		dec->cfg.stream_buffer_size = KD_DEC_DEFAULT_STREAM_SIZE;

	*out = dec;

	return KD_OK;
}

/**
 * Bring the frame queue up before the resolution is known.
 *
 * The firmware is only fetched once BOTH queues are streaming: see
 * mvx_session_streamon() in mvx_session.c, which does
 *
 *	if (!is_stream_on(session))
 *		return 0;
 *
 * and only reaches mvx_fw_cache_get() below that. So streaming the bitstream
 * queue alone parses nothing: no SOURCE_CHANGE is ever raised, no bitstream
 * buffer ever comes back, and the application waits forever.
 *
 * One small throwaway buffer is enough to get past that gate. It is never
 * filled: the firmware announces the real geometry with MVX_FW_CODE_ALLOC_PARAM
 * (handle_fw_message() in mvx_session.c) before producing any frame, and that
 * always raises SOURCE_CHANGE. kd_dec_attach_output() then swaps this group for
 * a properly sized one.
 */
static int dec_probe_output(struct kd_dec *dec)
{
	struct v4l2_format f;
	kd_bufgroup *grp = NULL;
	size_t size;
	int ret;

	memset(&f, 0, sizeof(f));
	f.type = KD_DEC_OUT_TYPE;

	if (kd_ioctl(dec->fd, VIDIOC_G_FMT, &f) < 0) {
		KD_LOGE("reading back the output frame format failed: %s",
			strerror(errno));
		return kd_errno_to_err(errno);
	}

	size = f.fmt.pix_mp.plane_fmt[0].sizeimage;
	if (size == 0)
		size = 4096;

	ret = kd_bufgroup_create_internal(&grp, 1);
	if (ret != KD_OK)
		return ret;

	ret = kd_bufgroup_attach(grp, dec->fd, KD_DEC_OUT_TYPE, size);
	if (ret != KD_OK) {
		KD_LOGE("allocating the probe frame buffer failed: %s",
			kd_strerror(ret));
		kd_bufgroup_destroy(grp);
		return ret;
	}

	ret = kd_bufgroup_queue_all(grp);
	if (ret != KD_OK)
		goto detach;

	ret = kd_streamon(dec->fd, KD_DEC_OUT_TYPE);
	if (ret != KD_OK)
		goto detach;

	dec->out = grp;
	dec->out_owned = true;
	dec->out_streaming = true;
	dec->probing = true;

	KD_LOGI("frame queue started with one %zu byte probe buffer, waiting "
		"for the firmware to report the real resolution",
		size);

	return KD_OK;

detach:
	kd_bufgroup_detach(grp);
	kd_bufgroup_destroy(grp);

	return ret;
}

int kd_dec_start(kd_dec *dec)
{
	int ret;

	if (dec == NULL)
		return KD_ERR_INVAL;

	if (dec->started)
		return KD_ERR_STATE;

	ret = dec_set_formats(dec);
	if (ret != KD_OK)
		return ret;

	ret = dec_set_downscale(dec);
	if (ret != KD_OK)
		return ret;

	/* Subscribing has to happen before STREAMON or the first
	 * SOURCE_CHANGE is lost */
	ret = dec_subscribe(dec, V4L2_EVENT_SOURCE_CHANGE);
	if (ret != KD_OK)
		return ret;

	dec_subscribe(dec, V4L2_EVENT_EOS);

	ret = kd_bufgroup_create_internal(&dec->in, dec->cfg.stream_buffers);
	if (ret != KD_OK)
		return ret;

	ret = kd_bufgroup_attach(dec->in, dec->fd, KD_DEC_IN_TYPE,
				 dec->cfg.stream_buffer_size);
	if (ret != KD_OK) {
		KD_LOGE("allocating the bitstream buffers failed: %s",
			kd_strerror(ret));
		goto free_in;
	}

	ret = kd_streamon(dec->fd, KD_DEC_IN_TYPE);
	if (ret != KD_OK)
		goto detach_in;

	dec->started = true;

	ret = dec_probe_output(dec);
	if (ret != KD_OK) {
		kd_streamoff(dec->fd, KD_DEC_IN_TYPE);
		dec->started = false;
		goto detach_in;
	}

	return KD_OK;

detach_in:
	kd_bufgroup_detach(dec->in);
free_in:
	kd_bufgroup_destroy(dec->in);
	dec->in = NULL;

	return ret;
}

void kd_dec_destroy(kd_dec *dec)
{
	if (dec == NULL)
		return;

	if (dec->out_streaming) {
		kd_streamoff(dec->fd, KD_DEC_OUT_TYPE);
		dec->out_streaming = false;
	}

	if (dec->started) {
		kd_streamoff(dec->fd, KD_DEC_IN_TYPE);
		dec->started = false;
	}

	if (dec->out != NULL) {
		kd_bufgroup_detach(dec->out);
		if (dec->out_owned)
			kd_bufgroup_destroy(dec->out);
		dec->out = NULL;
	}

	if (dec->in != NULL) {
		kd_bufgroup_detach(dec->in);
		kd_bufgroup_destroy(dec->in);
	}

	if (dec->fd >= 0)
		close(dec->fd);

	free(dec);
}

/* ========================================================================
 * Feeding the bitstream
 * ======================================================================== */

int kd_dec_send_stream(kd_dec *dec, const void *data, size_t size, int64_t pts,
		       unsigned flags)
{
	uint32_t v4l2_flags = 0;
	unsigned i;
	void *dst;

	if (dec == NULL || data == NULL || size == 0)
		return KD_ERR_INVAL;

	if (!dec->started)
		return KD_ERR_STATE;

	if (dec->draining) {
		KD_LOGE("the decoder is draining, no more data accepted");
		return KD_ERR_STATE;
	}

	if (size > dec->cfg.stream_buffer_size) {
		KD_LOGE("this packet is %zu bytes, the bitstream buffer holds "
			"%zu; raise stream_buffer_size when creating the "
			"decoder",
			size, dec->cfg.stream_buffer_size);
		return KD_ERR_INVAL;
	}

	for (i = 0; i < dec->in->count; i++)
		if (!dec->in->slots[i].queued)
			break;

	if (i == dec->in->count) {
		struct kd_dqinfo info;
		int ret = kd_bufgroup_dqbuf(dec->in, &info);

		if (ret != KD_OK)
			return ret;

		i = info.index;
	}

	/* The bitstream queue is single-planar, so plane 0 is all of it */
	dst = dec->in->slots[i].plane[0].addr;
	if (dst == NULL)
		return KD_ERR_STATE;

	memcpy(dst, data, size);

	/*
	 * On decoder input the driver maps V4L2_BUF_FLAG_KEYFRAME to
	 * MVX_BUFFER_EOF, meaning a complete frame ends with this packet.
	 * That is a different thing entirely from the forced IDR it means on
	 * encoder input (mvx_v4l2_buffer.c:187-238).
	 */
	if ((flags & KD_FLAG_END_OF_FRAME) != 0)
		v4l2_flags |= V4L2_BUF_FLAG_KEYFRAME;

	return kd_bufgroup_qbuf(dec->in, i, size, pts, v4l2_flags);
}

/* ========================================================================
 * Output frames
 * ======================================================================== */

int kd_dec_get_frameinfo(kd_dec *dec, kd_dec_frameinfo *info)
{
	if (dec == NULL || info == NULL)
		return KD_ERR_INVAL;

	if (!dec->info_valid) {
		int ret;

		/* Still the probe geometry, not the stream's: reporting it
		 * would have the caller allocate the wrong size */
		if (dec->probing)
			return KD_ERR_AGAIN;

		ret = dec_refresh_frameinfo(dec);
		if (ret != KD_OK)
			return ret;
	}

	*info = dec->info;

	return KD_OK;
}

int kd_dec_attach_output(kd_dec *dec, kd_bufgroup *grp)
{
	size_t min_size;
	int ret;

	if (dec == NULL || grp == NULL)
		return KD_ERR_INVAL;

	if (!dec->started)
		return KD_ERR_STATE;

	if (!dec->info_valid) {
		/* See kd_dec_get_frameinfo() */
		if (dec->probing)
			return KD_ERR_AGAIN;

		ret = dec_refresh_frameinfo(dec);
		if (ret != KD_OK)
			return ret;
	}

	min_size = dec->info.buffer_size;

	if (grp->count < dec->info.min_buffers)
		KD_LOGW("%u output buffers given, the driver asks for at "
			"least %u; the pipeline may stall",
			grp->count, dec->info.min_buffers);

	/* Swapping groups: stop the queue and detach the old one first */
	if (dec->out_streaming) {
		kd_streamoff(dec->fd, KD_DEC_OUT_TYPE);
		dec->out_streaming = false;
	}

	if (dec->out != NULL) {
		kd_bufgroup_detach(dec->out);
		if (dec->out_owned)
			kd_bufgroup_destroy(dec->out);
		dec->out = NULL;
		dec->out_owned = false;
	}

	ret = kd_bufgroup_attach(grp, dec->fd, KD_DEC_OUT_TYPE, min_size);
	if (ret != KD_OK) {
		KD_LOGE("attaching the output buffers failed: %s",
			kd_strerror(ret));
		return ret;
	}

	ret = kd_bufgroup_queue_all(grp);
	if (ret != KD_OK)
		goto detach;

	ret = kd_streamon(dec->fd, KD_DEC_OUT_TYPE);
	if (ret != KD_OK)
		goto detach;

	dec->out = grp;
	dec->out_streaming = true;
	dec->probing = false;

	return KD_OK;

detach:
	kd_bufgroup_detach(grp);

	return ret;
}

/**
 * React to SOURCE_CHANGE on behalf of the application when
 * cfg.auto_output_buffers is set: allocate output frames from the VPU and
 * attach them, so KD_ERR_SRC_CHANGE never reaches the caller.
 */
static int dec_setup_auto_output(struct kd_dec *dec)
{
	kd_bufgroup *grp = NULL;
	unsigned count = dec->cfg.auto_output_buffers;
	int ret;

	if (count < dec->info.min_buffers)
		count = dec->info.min_buffers;

	ret = kd_bufgroup_create_internal(&grp, count);
	if (ret != KD_OK)
		return ret;

	ret = kd_dec_attach_output(dec, grp);
	if (ret != KD_OK) {
		kd_bufgroup_destroy(grp);
		return ret;
	}

	dec->out_owned = true;

	KD_LOGI("allocated %u output frames of %zu bytes", count,
		dec->info.buffer_size);

	return KD_OK;
}

int kd_dec_get_frame(kd_dec *dec, kd_frame *frame, int timeout_ms)
{
	int remaining = timeout_ms;

	if (dec == NULL || frame == NULL)
		return KD_ERR_INVAL;

	if (!dec->started)
		return KD_ERR_STATE;

	if (dec->got_last)
		return KD_ERR_EOS;

	for (;;) {
		struct kd_dqinfo info;
		bool out_ready;
		short events;
		short revents = 0;
		int ret;

		/* Events come first: a resolution change has to be acted on
		 * before the next frame is taken, or that frame is wrong */
		ret = dec_drain_events(dec);
		if (ret == KD_ERR_SRC_CHANGE) {
			if (dec->cfg.auto_output_buffers == 0)
				return ret;

			ret = dec_setup_auto_output(dec);
			if (ret != KD_OK)
				return ret;

			continue;
		}

		/*
		 * While probing, the only buffer on the queue is the throwaway
		 * one from dec_probe_output(). The driver hands it straight
		 * back once the firmware knows the geometry, flagged
		 * V4L2_BUF_FLAG_MVX_BUFFER_NEED_REALLOC with bytesused 0
		 * (buf_queue() in mvx_v4l2_vidioc.c); it is not a frame, so
		 * leave it alone and wait for SOURCE_CHANGE instead.
		 */
		out_ready = dec->out_streaming && !dec->probing;

		if (out_ready) {
			ret = kd_bufgroup_dqbuf(dec->out, &info);

			if (ret == KD_OK) {
				bool last = (info.flags &
					     V4L2_BUF_FLAG_LAST) != 0;

				if (info.bytesused == 0 && last) {
					kd_bufgroup_qbuf(dec->out, info.index,
							 0, 0, 0);
					dec->got_last = true;
					return KD_ERR_EOS;
				}

				frame->index = info.index;

				{
					const struct kd_bufslot *s =
						&dec->out->slots[info.index];
					unsigned p;

					frame->nplanes = dec->out->nplanes;

					for (p = 0; p < KD_FRAME_MAX_PLANES;
					     p++) {
						frame->plane[p] =
							p < dec->out->nplanes ?
								s->plane[p].addr :
								NULL;
						frame->plane_size[p] =
							p < dec->out->nplanes ?
								s->plane[p].size :
								0;
					}

					frame->data = frame->plane[0];
				}

				frame->size = info.bytesused != 0 ?
						      info.bytesused :
						      dec->info.buffer_size;
				frame->pts = info.pts;
				frame->eos = last;
				frame->_slot = info.index;

				if (last)
					dec->got_last = true;

				return KD_OK;
			}

			if (ret != KD_ERR_AGAIN)
				return ret;
		}

		if (remaining == 0)
			return KD_ERR_AGAIN;

		/*
		 * POLLIN cannot be waited on before the output queue is
		 * streaming: vb2 answers POLLERR for a queue that is not
		 * running. Only events are worth waiting for at that point.
		 */
		events = POLLPRI;
		if (out_ready)
			events |= POLLIN;

		ret = kd_poll(dec->fd, events, remaining, &revents);
		if (ret != KD_OK)
			return ret;

		if ((revents & POLLERR) != 0 && !out_ready) {
			/* Frame queue not carrying frames yet: this POLLERR is
			 * expected, treat it as no event */
			revents &= (short)~POLLERR;
			if (revents == 0)
				return KD_ERR_AGAIN;
		}

		if ((revents & POLLERR) != 0) {
			KD_LOGE("the decoder reported an error (poll returned "
				"POLLERR), see dmesg | grep -i mvx");
			return KD_ERR_IO;
		}

		if (remaining > 0) {
			/*
			 * One poll round counts as the whole wait; the time
			 * left is not tracked precisely. If the next round
			 * still finds nothing the call times out instead of
			 * blocking on.
			 */
			remaining = 0;
		}
	}
}

int kd_dec_release_frame(kd_dec *dec, kd_frame *frame)
{
	if (dec == NULL || frame == NULL || dec->out == NULL)
		return KD_ERR_INVAL;

	if (frame->_slot >= dec->out->count)
		return KD_ERR_INVAL;

	return kd_bufgroup_qbuf(dec->out, frame->_slot, 0, 0, 0);
}

int kd_dec_get_fd(kd_dec *dec)
{
	return dec != NULL ? dec->fd : -1;
}

int kd_dec_stop(kd_dec *dec)
{
	struct v4l2_decoder_cmd cmd;

	if (dec == NULL)
		return KD_ERR_INVAL;

	if (!dec->started)
		return KD_ERR_STATE;

	if (dec->draining)
		return KD_OK;

	memset(&cmd, 0, sizeof(cmd));
	cmd.cmd = V4L2_DEC_CMD_STOP;

	if (kd_ioctl(dec->fd, VIDIOC_DECODER_CMD, &cmd) < 0) {
		KD_LOGE("DEC_CMD_STOP failed: %s", strerror(errno));
		return kd_errno_to_err(errno);
	}

	dec->draining = true;

	return KD_OK;
}
