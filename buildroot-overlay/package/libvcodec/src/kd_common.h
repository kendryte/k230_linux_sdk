/*
 * libvcodec - internal shared definitions
 *
 * Not installed. Applications only ever see vcodec.h.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KD_COMMON_H
#define KD_COMMON_H

#include <linux/videodev2.h>
#include <stdint.h>

#include "vcodec.h"

/*
 * P010 only landed in videodev2.h in 5.11, so the kernel headers of a cross
 * build may not have it. The target runs 6.6 and the driver supports the
 * format (mvx_v4l2_vidioc.c:107); this is just a fallback definition.
 */
#ifndef V4L2_PIX_FMT_P010
#define V4L2_PIX_FMT_P010 v4l2_fourcc('P', '0', '1', '0')
#endif

/* ========================================================================
 * Logging
 * ======================================================================== */

void kd_log_impl(kd_log_level level, const char *fmt, ...)
	__attribute__((format(printf, 2, 3)));

extern kd_log_level kd_g_log_level;

#define KD_LOGE(...)                                       \
	do {                                               \
		if (kd_g_log_level >= KD_LOG_ERROR)        \
			kd_log_impl(KD_LOG_ERROR, __VA_ARGS__); \
	} while (0)
#define KD_LOGW(...)                                      \
	do {                                              \
		if (kd_g_log_level >= KD_LOG_WARN)        \
			kd_log_impl(KD_LOG_WARN, __VA_ARGS__); \
	} while (0)
#define KD_LOGI(...)                                      \
	do {                                              \
		if (kd_g_log_level >= KD_LOG_INFO)        \
			kd_log_impl(KD_LOG_INFO, __VA_ARGS__); \
	} while (0)
#define KD_LOGD(...)                                       \
	do {                                               \
		if (kd_g_log_level >= KD_LOG_DEBUG)        \
			kd_log_impl(KD_LOG_DEBUG, __VA_ARGS__); \
	} while (0)

/* ========================================================================
 * Small helpers
 * ======================================================================== */

/** ioctl wrapper that retries on EINTR. Returns -1 and keeps errno. */
int kd_ioctl(int fd, unsigned long req, void *arg);

/** errno -> KD_ERR_*. */
int kd_errno_to_err(int err);

/**
 * Open the VPU device.
 * @param dev  NULL probes for it (matching driver == "mvx")
 * Returns an fd with O_NONBLOCK set, or a negative KD_ERR_*.
 */
int kd_open_device(const char *dev);

/** Wait for events on an fd. @events is a POLLIN/POLLOUT/POLLPRI mask. */
int kd_poll(int fd, short events, int timeout_ms, short *revents);

/* ========================================================================
 * Formats
 * ======================================================================== */

uint32_t kd_pixfmt_fourcc(kd_pixfmt f);
uint32_t kd_codec_fourcc(kd_codec c);

/** Bytes per pixel of the luma plane, used to derive a stride. */
unsigned kd_pixfmt_bpp(kd_pixfmt f);

/** Packed-row stride, used when the caller passes 0. */
unsigned kd_default_stride(kd_pixfmt f, unsigned width);

/** Whole frame in bytes, chroma planes included. */
size_t kd_frame_size(kd_pixfmt f, unsigned stride, unsigned height);

/** V4L2 fourcc -> kd_pixfmt, KD_ERR_INVAL when unknown. */
int kd_fourcc_pixfmt(uint32_t fourcc, kd_pixfmt *out);

/** pts (microseconds) <-> struct timeval. The driver carries the timestamp
 *  through to the matching output buffer unchanged. */
void kd_pts_to_tv(int64_t pts, struct timeval *tv);
int64_t kd_tv_to_pts(const struct timeval *tv);

/* ========================================================================
 * Buffer group - the container shared by all four codec queues
 *
 *   encoder: input frames (may be external) + output bitstream (internal)
 *   decoder: input bitstream (internal)     + output frames (may be external)
 *
 * Application index and V4L2 index are 1:1: attaching does REQBUFS(count),
 * so index i always lands on V4L2 index i and no mapping table is needed.
 * ======================================================================== */

/*
 * The frame queues of this driver are multi-planar: NV12 and NV21 are two
 * planes, I420 is three (mvx_buffer.c, get_stride()). Only the bitstream
 * queues are single-planar. KD_MAX_PLANES covers the worst case.
 */
#define KD_MAX_PLANES 3

struct kd_bufplane {
	void *addr; /* mmap address of an internal buffer, else NULL */
	size_t size;
	/*
	 * External groups only: the dmabuf this plane is imported from, -1 for
	 * internal ones. One fd per plane, because that is how V4L2 imports
	 * them - but several planes may name the same fd and differ only in
	 * offset, which is how one camera buffer carries a whole NV12 frame.
	 */
	int dmabuf_fd;
	size_t offset; /* where this plane starts inside its dmabuf */
};

struct kd_bufslot {
	struct kd_bufplane plane[KD_MAX_PLANES];
	unsigned nplanes; /* external: how many planes were committed */
	size_t size; /* sum over the planes, counting a shared fd once */
	bool committed; /* external: registered by the application */
	bool queued; /* handed to the VPU with QBUF, not dequeued yet */
	/*
	 * Set when attaching split one committed dmabuf across the several
	 * planes the driver wants. Undone on detach so the slot goes back to
	 * what the application committed.
	 */
	bool split;
};

struct kd_bufgroup {
	bool external;
	unsigned count;
	unsigned ncommitted;
	struct kd_bufslot *slots;

	/* Set by kd_bufgroup_create_from_v4l2(): the capture device the
	 * buffers were exported from. src_fd >= 0 is what lets the encoder
	 * requeue finished frames on its own, and owns_fds says the exported
	 * fds are ours to close. */
	int src_fd;
	uint32_t src_type;
	bool owns_fds;

	/* valid after attaching */
	bool attached;
	int fd;
	uint32_t type; /* V4L2_BUF_TYPE_VIDEO_{OUTPUT,CAPTURE}[_MPLANE] */
	uint32_t memory; /* V4L2_MEMORY_DMABUF / _MMAP */
	bool mplane; /* type is one of the _MPLANE ones */
	unsigned nplanes; /* 1 unless mplane, read back from the driver */
	size_t plane_size[KD_MAX_PLANES]; /* the driver's sizeimage per plane */
};

/**
 * Bind the group to one V4L2 queue: REQBUFS, plus QUERYBUF and mmap for
 * internal groups.
 *
 * The plane count comes from a G_FMT on @type, so the format has to be set
 * before this is called.
 *
 * An external (dmabuf) group that was committed with one fd per slot can
 * still feed a multi-plane queue: the one buffer is cut up along the plane
 * sizes the driver just reported and every plane points at the same fd with
 * its own data_offset - see kd_bufgroup_split_slot(). That is how an NV12
 * camera buffer, which V4L2 can only ever export as a single fd, reaches a
 * VPU queue that wants two planes. The buffer has to be at least as big as
 * the plane sizes add up to, or this returns KD_ERR_INVAL with an
 * explanation rather than letting QBUF fail later.
 *
 * @param min_size  smallest acceptable buffer, used to sanity check the
 *                  length the driver reports
 */
int kd_bufgroup_attach(struct kd_bufgroup *grp, int fd, uint32_t type,
		       size_t min_size);

/** Plane @p of slot @index: CPU address and size, internal groups only. */
void *kd_bufgroup_plane(const struct kd_bufgroup *grp, unsigned index,
			unsigned p, size_t *size);

/** Unbind: munmap and REQBUFS(0). */
void kd_bufgroup_detach(struct kd_bufgroup *grp);

/** QBUF. bytesused may be 0 for raw frames (the driver sets
 *  allow_zero_bytesused). */
int kd_bufgroup_qbuf(struct kd_bufgroup *grp, unsigned index, size_t bytesused,
		     int64_t pts, uint32_t v4l2_flags);

/** Everything DQBUF brings back. */
struct kd_dqinfo {
	unsigned index;
	size_t bytesused;
	int64_t pts;
	uint32_t flags; /* v4l2_buffer.flags */
	uint32_t reserved2; /* carries the frame type on single plane output,
			     * see kd_mvx.h */
};

/**
 * DQBUF.
 * Returns KD_OK or KD_ERR_AGAIN (queue empty for now).
 * Note that this driver does not signal end of stream with -EPIPE: look for
 * V4L2_BUF_FLAG_LAST in info->flags instead.
 */
int kd_bufgroup_dqbuf(struct kd_bufgroup *grp, struct kd_dqinfo *info);

/** QBUF every slot that is not already queued, for the bitstream receive
 *  side and the decoder output side. */
int kd_bufgroup_queue_all(struct kd_bufgroup *grp);

/** Drop every queued flag, for use after STREAMOFF hands all buffers back. */
void kd_bufgroup_reset_queued(struct kd_bufgroup *grp);

/** QBUF a finished frame back to the capture device the group came from.
 *  Only valid for groups built by kd_bufgroup_create_from_v4l2(). */
int kd_bufgroup_requeue_source(struct kd_bufgroup *grp, unsigned index);

int kd_streamon(int fd, uint32_t type);
int kd_streamoff(int fd, uint32_t type);

#endif /* KD_COMMON_H */
