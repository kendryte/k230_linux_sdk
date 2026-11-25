/*
 * libvcodec - hardware video codec library for K230 Linux
 *
 * A thin wrapper over the K230 VPU (ARM Linlon / MVX, exposed as a V4L2 M2M
 * device). Plain C, no dependencies, does not link ffmpeg.
 *
 * Three things shape this API:
 *
 *   1. Zero copy is a first class citizen. Encoder input frames and decoder
 *      output frames are both plugged in through a kd_bufgroup, which can
 *      either wrap external dmabufs (camera / DRM) or be allocated by the
 *      library.
 *
 *   2. The VPU never allocates YUV memory, it only imports. On K230 the VPU
 *      uses scattered pages (vb2_dma_sg) while the camera and DRM both
 *      require physically contiguous memory (vb2_dma_contig / drm_gem_dma).
 *      Contiguous memory can feed a scatter-gather importer, not the other
 *      way round. So in a zero copy pipeline the camera or DRM allocates and
 *      the VPU imports.
 *
 *   3. The fd is exposed. kd_enc_get_fd() / kd_dec_get_fd() let an
 *      application drive several channels from one epoll loop on a single
 *      thread instead of one blocking thread per channel.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef VCODEC_H
#define VCODEC_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define KD_VERSION_MAJOR 1
#define KD_VERSION_MINOR 0

/* ========================================================================
 * Error codes
 *
 * Every function returns int: KD_OK on success, negative on error.
 * KD_ERR_AGAIN and KD_ERR_SRC_CHANGE are part of normal operation, not
 * failures.
 * ======================================================================== */

#define KD_OK 0
#define KD_ERR_AGAIN (-1) /* no data / no free slot right now, retry later */
#define KD_ERR_EOS (-2) /* stream ended, no more data will come */
#define KD_ERR_SRC_CHANGE (-3) /* decoder: resolution known or changed, output buffers must be re-attached */
#define KD_ERR_INVAL (-4) /* bad argument */
#define KD_ERR_NOMEM (-5) /* out of memory */
#define KD_ERR_IO (-6) /* ioctl / driver error */
#define KD_ERR_NODEV (-7) /* no VPU device node found */
#define KD_ERR_STATE (-8) /* called at the wrong time, e.g. send before start */

/** Turn an error code into a readable string. Static storage, do not free. */
const char *kd_strerror(int err);

/* ========================================================================
 * Basic types
 * ======================================================================== */

typedef enum {
	KD_CODEC_H264 = 0,
	KD_CODEC_H265,
	KD_CODEC_JPEG,
} kd_codec;

typedef enum {
	KD_PIXFMT_NV12 = 0, /* Y/CbCr 4:2:0, one allocation, Y plane then interleaved UV */
	KD_PIXFMT_NV21, /* same, reversed chroma order */
	KD_PIXFMT_P010, /* 10 bit 4:2:0 */
	KD_PIXFMT_YUYV, /* packed 4:2:2 */
} kd_pixfmt;

typedef enum {
	KD_PROFILE_DEFAULT = 0,
	KD_PROFILE_BASELINE, /* H.264 only */
	KD_PROFILE_MAIN,
	KD_PROFILE_HIGH, /* H.264 only */
} kd_profile;

typedef enum {
	KD_LOG_NONE = 0,
	KD_LOG_ERROR,
	KD_LOG_WARN,
	KD_LOG_INFO,
	KD_LOG_DEBUG,
} kd_log_level;

/** Set the log level, default KD_LOG_ERROR. Logs go to stderr. */
void kd_set_log_level(kd_log_level level);

/**
 * Find the V4L2 device node of the VPU.
 *
 * The VPU node number is not fixed on K230 (a camera usually takes
 * /dev/video0), so never hard-code it. The library probes automatically by
 * default; this function is only here for applications that want to know.
 *
 * @param path  output buffer, e.g. "/dev/video9"
 * @param len   buffer length, 32 or more recommended
 */
int kd_probe_device(char *path, size_t len);

/* ========================================================================
 * Buffer group - where YUV frames come from and go to
 *
 * The encoder plugs one in on the input side (raw frames), the decoder on
 * the output side (decoded frames). The bitstream side is always allocated
 * by the library.
 *
 * Three ways to build one:
 *
 *   from_v4l2 - the library exports the dmabufs of another V4L2 device
 *               (a camera) and takes care of sizes and fd lifetime. This is
 *               the shortest path to a zero copy capture pipeline, and it is
 *               the only one that enables encoder auto-requeue.
 *
 *   external  - the application supplies dmabuf fds. Use this when the
 *               buffers come from something that is not a V4L2 device, DRM
 *               being the usual case.
 *
 *   internal  - the library asks the VPU for memory (V4L2 MMAP). Not zero
 *               copy; meant for file input/output and debugging.
 *               kd_bufgroup_map() hands out the CPU address.
 *
 * Indices are the application's own. If a camera DQBUF gives you index 3,
 * you commit 3 and you send 3; mapping that onto a VPU slot is the library's
 * job even when the two numbering schemes disagree.
 * ======================================================================== */

typedef struct kd_bufgroup kd_bufgroup;

/**
 * Build a buffer group from a streaming V4L2 capture device.
 *
 * The library reads the frame size with VIDIOC_G_FMT, exports every buffer
 * with VIDIOC_EXPBUF and keeps the resulting fds, closing them in
 * kd_bufgroup_destroy(). The group is ready to attach right away, no
 * kd_bufgroup_commit() calls needed.
 *
 * The device must have completed VIDIOC_REQBUFS already, and @count must be
 * the buffer count the driver actually granted.
 *
 * A group built this way remembers the source device, which is what lets
 * the encoder hand finished frames back to the camera on its own; see
 * kd_enc_set_auto_requeue().
 *
 * @param v4l2_fd  fd of the capture device. Borrowed, not closed.
 */
int kd_bufgroup_create_from_v4l2(kd_bufgroup **grp, int v4l2_fd,
				 unsigned count);

/**
 * Build an external buffer group. It starts out empty: commit every slot
 * with kd_bufgroup_commit() before attaching it.
 */
int kd_bufgroup_create_external(kd_bufgroup **grp, unsigned count);

/**
 * Build an internal buffer group. The memory is allocated when the group is
 * attached to a codec, because only then are the size and the target device
 * known.
 */
int kd_bufgroup_create_internal(kd_bufgroup **grp, unsigned count);

/**
 * Register one dmabuf in an external group.
 *
 * @param index      application index, must be < count and must be unique
 * @param dmabuf_fd  dmabuf file descriptor. The library only records it and
 *                   does not take ownership; it must stay valid until
 *                   kd_bufgroup_destroy()
 * @param size       size of the buffer in bytes. It must be right: length is
 *                   mandatory when V4L2 imports a dmabuf, and a zero makes
 *                   QBUF fail with -EINVAL
 *
 * One fd is enough even for a multi-plane format on the encoder input: when
 * the queue wants more planes than were committed, the library cuts this one
 * buffer up along the plane sizes and points every plane at the same fd with
 * its own data_offset. That is what makes an NV12 camera buffer - which V4L2
 * can only ever export as a single fd - usable without a copy. The buffer
 * must be at least as big as the plane sizes add up to; attaching fails with
 * an explanation if it is not. Use kd_bufgroup_commit_planes() instead when
 * the planes really do live in separate allocations.
 */
int kd_bufgroup_commit(kd_bufgroup *grp, unsigned index, int dmabuf_fd,
		       size_t size);

/**
 * Register one frame that is split across several dmabufs, one per plane.
 *
 * NV12 and NV21 are two planes on this driver and I420 is three, and the
 * number has to match what the queue reports or attaching fails - see
 * kd_dec_frameinfo.nplanes, which is where to get it from.
 *
 * This is for the decoder output, where the planes genuinely are separate
 * allocations: DRM is happy to build a framebuffer out of two buffers, so
 * showing a decoded frame costs nothing but the extra allocation. On the
 * encoder input one fd is enough - kd_bufgroup_commit() splits it.
 *
 * @param dmabuf_fds  one fd per plane, in V4L2 plane order. Borrowed, they
 *                    must outlive the group
 * @param sizes       bytes in each plane, each one the real byte count
 * @param nplanes     length of both arrays
 */
int kd_bufgroup_commit_planes(kd_bufgroup *grp, unsigned index,
			      const int *dmabuf_fds, const size_t *sizes,
			      unsigned nplanes);

/**
 * CPU address of an internal buffer, NULL for the other kinds.
 * Call it after attaching; nothing is allocated before that.
 */
void *kd_bufgroup_map(kd_bufgroup *grp, unsigned index, size_t *size);

/** Number of buffers in the group. */
unsigned kd_bufgroup_count(const kd_bufgroup *grp);

/**
 * Destroy the group. Fds handed over by kd_bufgroup_commit() are left alone;
 * fds the library exported itself in kd_bufgroup_create_from_v4l2() are
 * closed here.
 *
 * Destroy the codec using the group first.
 */
void kd_bufgroup_destroy(kd_bufgroup *grp);

/* ========================================================================
 * Encoder
 *
 * Short form, camera to H.264 with zero copy and nothing to book-keep:
 *
 *     kd_enc_cfg cfg;
 *     kd_enc_cfg_default(&cfg);
 *     cfg.codec = KD_CODEC_H264;
 *     cfg.width = 1920; cfg.height = 1080;
 *     cfg.framerate = 30; cfg.bitrate = 4000000;
 *     kd_enc_create(&enc, &cfg);
 *
 *     kd_bufgroup_create_from_v4l2(&grp, cam_fd, n);
 *     kd_enc_attach_input(enc, grp);
 *     kd_enc_start(enc);
 *
 *     while (running) {
 *         camera_dqbuf(&index, &pts);
 *         kd_enc_send_frame(enc, index, pts);   // returned to the camera
 *                                               // by the library when done
 *         while ((n = kd_enc_read_stream(enc, buf, sizeof buf,
 *                                        &pts, &key, 0)) > 0)
 *             write(fd, buf, n);
 *     }
 *
 *     kd_enc_stop(enc);
 *     kd_enc_destroy(enc);
 *
 * Long form, for code that must not copy the bitstream or wants to control
 * when frames go back upstream: kd_enc_get_stream() plus
 * kd_enc_release_stream(), and kd_enc_reclaim_frame() after turning
 * auto-requeue off.
 * ======================================================================== */

typedef struct kd_enc kd_enc;

typedef struct {
	kd_codec codec;
	unsigned width;
	unsigned height;
	kd_pixfmt pixfmt;

	/* Bytes per line of an input frame. 0 means packed rows
	 * (width x bytes per pixel). vvcam hands out packed frames, so 0 is
	 * right for the camera case; if the source has alignment rules, pass
	 * the bytesperline its G_FMT reported. */
	unsigned stride;

	unsigned framerate; /* fps, feeds rate control, 0 = 30 */
	unsigned bitrate; /* bps, 0 = derive one from the resolution */
	unsigned gop; /* I frame interval in frames, 0 = framerate x 2 */

	kd_profile profile;
	unsigned level; /* 0 = driver default, rarely needed */

	/* JPEG only, 1..100, 0 = default */
	unsigned jpeg_quality;

	/* Number of bitstream buffers, 0 = 4. Frames are dropped when the
	 * bitstream is not collected fast enough, so raise this for high
	 * bitrates or slow storage. */
	unsigned bitstream_buffers;

	/* VPU device node. NULL = probe automatically, which is recommended. */
	const char *dev;
} kd_enc_cfg;

typedef struct {
	void *data;
	size_t size;
	int64_t pts;
	bool keyframe;
	bool eos; /* true = last packet, nothing follows */
	unsigned _slot; /* library internal, do not touch */
} kd_packet;

/** Fill in a default config. Always call it before setting fields, or
 *  fields added in a later version will hold garbage. */
void kd_enc_cfg_default(kd_enc_cfg *cfg);

int kd_enc_create(kd_enc **enc, const kd_enc_cfg *cfg);

/**
 * Attach the source of input frames. Call it before kd_enc_start(), once.
 * Without it start() creates an internal group of 4 buffers, and then only
 * kd_enc_send_frame_copy() can be used.
 */
int kd_enc_attach_input(kd_enc *enc, kd_bufgroup *grp);

int kd_enc_start(kd_enc *enc);

/**
 * Control whether the library returns finished input frames to the capture
 * device itself. On by default whenever the input group came from
 * kd_bufgroup_create_from_v4l2(); it cannot be enabled for other groups
 * because the library has no way to reach the owner of the buffers.
 *
 * Turn it off to decide yourself when a frame goes back upstream, then
 * collect finished frames with kd_enc_reclaim_frame().
 */
int kd_enc_set_auto_requeue(kd_enc *enc, bool enable);

/**
 * Submit a frame for encoding, without copying it.
 *
 * @param index  index inside the input group
 * @param pts    timestamp in a unit of your choosing, carried to the output
 *               packet unchanged
 *
 * KD_ERR_AGAIN means the input queue is full: drain it with
 * kd_enc_get_stream() (or kd_enc_reclaim_frame()) and retry.
 *
 * With auto-requeue off, this index must not be handed back to the camera
 * until kd_enc_reclaim_frame() returns it - the VPU is still reading it.
 */
int kd_enc_send_frame(kd_enc *enc, unsigned index, int64_t pts);

/**
 * Submit a frame the library copies in. Only works with an internal group.
 * Meant for file input and debugging; use kd_enc_send_frame() for cameras.
 */
int kd_enc_send_frame_copy(kd_enc *enc, const void *data, size_t size,
			   int64_t pts);

/**
 * Copy out one bitstream packet and release it in one step.
 *
 * Returns the number of bytes written to @buf, KD_ERR_AGAIN when nothing is
 * ready, KD_ERR_EOS after a drain has finished, or KD_ERR_NOMEM when the
 * packet does not fit in @cap - in which case the packet is dropped, so size
 * @buf for the worst case frame.
 *
 * @param pts       may be NULL
 * @param keyframe  may be NULL
 */
int kd_enc_read_stream(kd_enc *enc, void *buf, size_t cap, int64_t *pts,
		       bool *keyframe, int timeout_ms);

/**
 * Borrow one bitstream packet without copying it.
 *
 * @param timeout_ms  0 = return immediately, -1 = wait forever,
 *                    >0 = wait at most that long
 *
 * On KD_OK the packet is valid and must be handed back with
 * kd_enc_release_stream() once consumed, or the pool runs dry within a few
 * frames. KD_ERR_AGAIN means nothing is ready yet.
 */
int kd_enc_get_stream(kd_enc *enc, kd_packet *pkt, int timeout_ms);

int kd_enc_release_stream(kd_enc *enc, kd_packet *pkt);

/**
 * Ask the encoder which input frame it is done with.
 *
 * On KD_OK *index is free to be handed back upstream. KD_ERR_AGAIN means no
 * frame has finished yet. Returns KD_ERR_STATE while auto-requeue is on,
 * since the library is already doing this.
 *
 * There is no reference counting and no automatic recycling here: the V4L2
 * DQBUF on the input queue already states exactly that the VPU is done with
 * a frame, and passing that through unchanged is the least error prone
 * thing to do.
 */
int kd_enc_reclaim_frame(kd_enc *enc, unsigned *index);

/** Change the bitrate while running. */
int kd_enc_set_bitrate(kd_enc *enc, unsigned bps);

/** Force the next frame to be encoded as an IDR. */
int kd_enc_force_idr(kd_enc *enc);

/**
 * The underlying fd, for epoll / poll.
 *
 * EPOLLIN  -> a bitstream packet is ready (kd_enc_get_stream)
 * EPOLLOUT -> an input slot opened up (kd_enc_reclaim_frame, or send again)
 *
 * Use it to wait for events only, never issue ioctls on it.
 */
int kd_enc_get_fd(kd_enc *enc);

/**
 * Finish up: send the drain command and flush the remaining bitstream.
 * Keep calling kd_enc_get_stream() afterwards until a packet comes back with
 * pkt.eos set, or until KD_ERR_EOS.
 */
int kd_enc_stop(kd_enc *enc);

void kd_enc_destroy(kd_enc *enc);

/* ========================================================================
 * One-shot JPEG
 * ======================================================================== */

/**
 * Encode a single raw frame to JPEG and block until it is done.
 *
 * Meant for snapshots. It opens the VPU, encodes, and closes again, so do
 * not call it per frame in a video loop - build a KD_CODEC_JPEG encoder for
 * that.
 *
 * @param src      raw frame, packed rows (no padding)
 * @param src_size bytes in @src
 * @param quality  1..100, 0 = default
 * @param dst      output buffer
 * @param dst_cap  size of @dst; roughly width x height is plenty
 *
 * Returns the JPEG size in bytes, or a negative error code.
 */
int kd_jpeg_encode(unsigned width, unsigned height, kd_pixfmt pixfmt,
		   const void *src, size_t src_size, unsigned quality,
		   void *dst, size_t dst_cap);

/* ========================================================================
 * Decoder
 *
 * The big difference from the encoder: the resolution is only known once
 * decoding has started. So output buffers cannot be attached up front.
 *
 * Easy path - let the library allocate the output frames:
 *
 *     kd_dec_cfg_default(&cfg);
 *     cfg.codec = KD_CODEC_H264;
 *     cfg.auto_output_buffers = 8;   // the library handles SOURCE_CHANGE
 *     kd_dec_create(&dec, &cfg);
 *     kd_dec_start(dec);
 *
 *     kd_dec_send_stream(dec, au, len, pts, KD_FLAG_END_OF_FRAME);
 *     if (kd_dec_get_frame(dec, &frame, 100) == KD_OK) {
 *         // frame.data points at the decoded frame
 *         kd_dec_release_frame(dec, &frame);
 *     }
 *
 * Zero copy path - DRM owns the frames, so leave auto_output_buffers at 0
 * and react to KD_ERR_SRC_CHANGE:
 *
 *     ret = kd_dec_get_frame(dec, &frame, 100);
 *     if (ret == KD_ERR_SRC_CHANGE) {
 *         kd_dec_frameinfo info;
 *         kd_dec_get_frameinfo(dec, &info);
 *
 *         // allocate n frames from DRM, one buffer per plane, each exported
 *         // with DRM_IOCTL_PRIME_HANDLE_TO_FD. info.nplanes says how many
 *         // the driver wants - 2 for NV12 - and committing a different
 *         // number is what attach refuses.
 *         kd_bufgroup_create_external(&grp, n);
 *         for (i...)
 *             kd_bufgroup_commit_planes(grp, i, fd[i], info.plane_size,
 *                                       info.nplanes);
 *         kd_dec_attach_output(dec, grp);
 *     }
 *
 * The library will not reallocate external output buffers for you: that
 * memory belongs to the application and may well be scanned out by a DRM
 * plane at that moment.
 * ======================================================================== */

typedef struct kd_dec kd_dec;

/** This packet ends a complete frame (access unit). Packets demuxed from an
 *  mp4 should always carry it. */
#define KD_FLAG_END_OF_FRAME (1u << 0)

typedef struct {
	kd_codec codec;
	kd_pixfmt pixfmt; /* wanted output format, NV12 by default */

	/* Resolution hint. Fill it in when a container such as mp4 tells you,
	 * it saves one SOURCE_CHANGE round trip; leave 0 when unknown.
	 * It really is only a hint: the firmware decides the final size, and
	 * getting the hint wrong does not break anything. */
	unsigned width;
	unsigned height;

	unsigned stream_buffers; /* number of bitstream buffers, 0 = 4 */
	size_t stream_buffer_size; /* largest packet in bytes, 0 = 1MB */

	/* Non-zero means the library allocates this many output frames itself
	 * when the resolution becomes known, and kd_dec_get_frame() never
	 * reports KD_ERR_SRC_CHANGE. The frames are then ordinary mapped
	 * memory reachable through kd_frame.data, which rules out zero copy
	 * to DRM - leave this at 0 for that. The driver's minimum is honoured
	 * if it is larger than the value given here. */
	unsigned auto_output_buffers;

	/*
	 * Have the decoder shrink the picture on its way out, to this size.
	 * 0 x 0, the default, decodes at the stream's own size.
	 *
	 * This is the only scaler in the chip: the display controller shows a
	 * frame at whatever size it arrives, so a 1080p stream on an 800x480
	 * panel is either scaled here or not seen whole at all.
	 *
	 * Three things to know:
	 *   - down only, there is no upscaler;
	 *   - the aspect ratio is not kept. Work out a size that preserves it
	 *     and put that here;
	 *   - it has to be set before kd_dec_start(). The driver refuses to
	 *     change it once the firmware is loaded, and the stream's own
	 *     size is only known after that - so sizing this from the stream
	 *     means decoding once to find out and then starting over with a
	 *     second decoder.
	 */
	unsigned scale_width;
	unsigned scale_height;

	const char *dev; /* NULL = probe automatically */
} kd_dec_cfg;

/* NV12 and NV21 are two planes, I420 three; YUYV is one. See kd_frame. */
#define KD_FRAME_MAX_PLANES 3

typedef struct {
	unsigned width;
	unsigned height;
	unsigned stride; /* bytes per line */
	kd_pixfmt pixfmt;
	unsigned min_buffers; /* driver minimum, allocating a few more is safer */
	size_t buffer_size; /* bytes per frame, use it when asking DRM */

	/*
	 * How the driver splits a frame across V4L2 planes. This is not a
	 * property of the pixel format alone - it is what this driver reports
	 * for it - and the zero copy path has to match it exactly: one dmabuf
	 * per plane, committed with kd_bufgroup_commit_planes(). NV12 comes
	 * back as 2 here, which is why a single allocation is not enough.
	 */
	unsigned nplanes;
	size_t plane_size[KD_FRAME_MAX_PLANES]; /* bytes in each plane */
	unsigned plane_stride[KD_FRAME_MAX_PLANES]; /* bytes per line of each */
} kd_dec_frameinfo;

typedef struct {
	unsigned index; /* index inside the output group */

	/*
	 * The decoder hands back one mapping per plane, and they are NOT
	 * contiguous: NV12 gives a Y plane and a separate UV plane. Walk
	 * plane[0..nplanes-1] rather than assuming data + something.
	 *
	 * data is plane[0] and size is the sum, kept so that single-plane
	 * formats such as YUYV read naturally.
	 *
	 * All of these are NULL for external groups, where the content lives
	 * in your own dmabuf.
	 */
	unsigned nplanes;
	void *plane[KD_FRAME_MAX_PLANES];
	size_t plane_size[KD_FRAME_MAX_PLANES];

	void *data;
	size_t size;

	int64_t pts;
	bool eos;
	unsigned _slot; /* library internal */
} kd_frame;

void kd_dec_cfg_default(kd_dec_cfg *cfg);

int kd_dec_create(kd_dec **dec, const kd_dec_cfg *cfg);
int kd_dec_start(kd_dec *dec);

/**
 * Feed the decoder.
 *
 * @param flags  normally KD_FLAG_END_OF_FRAME. It tells the firmware that a
 *               complete frame ends here instead of making it wait for the
 *               next start code, which cuts latency noticeably. Pass 0 when
 *               feeding arbitrary byte stream chunks.
 *
 * KD_ERR_AGAIN means the bitstream queue is full, collect frames first.
 */
int kd_dec_send_stream(kd_dec *dec, const void *data, size_t size, int64_t pts,
		       unsigned flags);

/** Output frame geometry, valid once the resolution is known. Call it after
 *  KD_ERR_SRC_CHANGE. */
int kd_dec_get_frameinfo(kd_dec *dec, kd_dec_frameinfo *info);

/**
 * Attach output buffers, after KD_ERR_SRC_CHANGE. May be called again when
 * the resolution changes once more; the previous group is detached.
 */
int kd_dec_attach_output(kd_dec *dec, kd_bufgroup *grp);

/**
 * Get one decoded frame.
 *
 * KD_ERR_SRC_CHANGE: resolution known or changed, go to kd_dec_get_frameinfo()
 *                    and kd_dec_attach_output(). Never returned when
 *                    cfg.auto_output_buffers is set.
 * KD_ERR_EOS:        stream finished.
 * KD_ERR_AGAIN:      nothing ready yet.
 */
int kd_dec_get_frame(kd_dec *dec, kd_frame *frame, int timeout_ms);

/**
 * Hand a frame back. Required once it has been displayed or copied away, or
 * the decoder runs out of buffers within a few frames.
 */
int kd_dec_release_frame(kd_dec *dec, kd_frame *frame);

/**
 * The underlying fd, for epoll.
 * EPOLLIN -> a frame is ready; EPOLLOUT -> the bitstream queue has room.
 */
int kd_dec_get_fd(kd_dec *dec);

/** Finish up and flush the frames left in the pipeline. Keep calling
 *  kd_dec_get_frame() afterwards until KD_ERR_EOS. */
int kd_dec_stop(kd_dec *dec);

void kd_dec_destroy(kd_dec *dec);

#ifdef __cplusplus
}
#endif

#endif /* VCODEC_H */
