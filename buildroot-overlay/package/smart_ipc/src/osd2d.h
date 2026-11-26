/*
 * Hardware OSD: boxes and little pictures burnt into a frame
 *
 * The K230 has a 2D engine sitting on a plain V4L2 mem-to-mem node (the
 * kernel module is package/nonai2d, the node calls itself "nonai-2d"). It can
 * do two things this demo wants:
 *
 *   - draw the outline of a rectangle, from nothing but coordinates, a colour
 *     and a line width. 32 of them, and they cost no memory at all;
 *   - blend a small ARGB8888 picture onto the frame. Only 8 of those, and
 *     each one is copied into the kernel every time it is used, so they want
 *     to be no bigger than what is actually being drawn.
 *
 * ffmpeg wraps the same node in its nonai2d_osd filter, which is where this
 * came from, but the node is ordinary V4L2 and this demo already speaks it,
 * so there is no reason to pull libavfilter in for it.
 *
 * The one thing worth knowing before reading the code: in this mode the
 * engine works *in place*. There is no second picture buffer and no output
 * queue - you hand it a frame on the capture queue, and the same frame comes
 * back with the boxes on it. That is what lets more than 8 pictures or 32
 * boxes be drawn: run it again over the result.
 *
 * And the frame it works on is the caller's own, imported as a dmabuf: no
 * copy in, no copy out, the CPU never touches a pixel. This is what the RTOS
 * firmware on the same chip has always done - it hands the 2D engine the very
 * frame the encoder is about to read. It needs a nonai2d module that accepts
 * a single-plane NV12 buffer and works the chroma address out for itself;
 * osd2d_attach() fails with a plain explanation on an older one rather than
 * quietly falling back to copying.
 *
 * Note what that means for the caller: the frame really does come back
 * written on, so anything else reading that same buffer sees the boxes too.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef OSD2D_H
#define OSD2D_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct osd2d;

/** A rectangle outline, drawn by the hardware. Costs no memory. */
struct osd2d_box {
	int x, y, w, h;
	unsigned rgb; /* 0xRRGGBB - turned into what the hardware wants */
	unsigned line; /* line width in pixels */
};

/**
 * A small ARGB8888 picture blended onto the frame. Alpha is per pixel, so
 * everything that should stay untouched is simply left at alpha 0.
 */
struct osd2d_patch {
	int x, y, w, h; /* where it lands on the frame */

	/*
	 * w*h pixels, tightly packed. The engine reads the four bytes of a
	 * pixel as alpha, red, green, blue, in that order - the other way
	 * round from a DRM ARGB8888 buffer, whose word reads 0xAARRGGBB and
	 * keeps alpha in the last byte. Anything left at alpha 0 is where the
	 * frame shows through untouched, so a patch of zeroes draws nothing.
	 */
	const uint32_t *argb;
};

/**
 * Open the 2D node and set it up for NV12 at @width x @height.
 *
 * @dev  NULL to go and find it: every /dev/video* is asked its driver name
 *       and the one answering "nonai-2d" wins.
 *
 * Returns NULL and explains itself on stderr.
 */
struct osd2d *osd2d_open(const char *dev, unsigned width, unsigned height);

/**
 * Name the frames it will be drawing on, once, before the first osd2d_apply().
 *
 * @fds    one dmabuf handle per frame - V4L2's EXPBUF gives these out, and
 *         cam_capture keeps them in cam_buffer.dmabuf_fd
 * @n      how many
 * @size   one frame, Y and UV together. The chroma plane has to start at
 *         width x height into the buffer, which is where a packed NV12 frame
 *         keeps it.
 *
 * The handles are not taken over: they have to outlive the osd2d.
 *
 * Returns 0, or a negative errno after explaining itself on stderr.
 */
int osd2d_attach(struct osd2d *o, const int *fds, unsigned n, size_t size);

/**
 * Draw everything onto frame @index - one of the ones osd2d_attach() was
 * given - where it lies.
 *
 * The hardware's limits - 8 pictures and 32 boxes at a time - are not the
 * caller's problem: anything above that is drawn in further passes over the
 * same frame, which works because each pass adds to what the last one left.
 *
 * Returns 1 when the frame was drawn on, 0 when there was nothing to draw and
 * it was left alone, or a negative errno after saying so on stderr - in which
 * case the frame is untouched and still perfectly good, just undecorated.
 *
 * Boxes and pictures are drawn in the order given, pictures first.
 */
int osd2d_apply(struct osd2d *o, unsigned index, const struct osd2d_box *boxes,
		int nbox, const struct osd2d_patch *patches, int npatch);

/** The node it ended up on, for the startup banner. */
const char *osd2d_dev(const struct osd2d *o);

/** Passes over the frame it has made, for the closing summary. */
unsigned long osd2d_passes(const struct osd2d *o);

/**
 * Where the time went. Every figure is microseconds added up over @frames,
 * so divide to get a per-frame cost; @worst is the slowest single call.
 *
 * Worth having because this sits in the middle of the capture loop: whatever
 * it costs, the panel and the encoder both wait for it. Setting up is one
 * ioctl per shape and grows with how much is being drawn; the engine is the
 * hardware itself and should barely move.
 *
 * copy_in and copy_out are what an earlier version of this spent walking the
 * frame in and out of a buffer of the engine's own - 35 ms a frame at 1080p,
 * which is what made opening the import path worth it. They are kept so
 * that a run can be seen to be reading zero on both: anything else means the
 * frame is being copied somewhere it should not be.
 */
struct osd2d_timing {
	unsigned long frames; /* calls that actually drew something */
	unsigned long copy_in; /* zero, and should stay that way */
	unsigned long ctrls;
	unsigned long engine;
	unsigned long copy_out; /* likewise */
	unsigned long worst; /* all of it, for one frame */
};

void osd2d_timing(const struct osd2d *o, struct osd2d_timing *t);

void osd2d_close(struct osd2d *o);

#ifdef __cplusplus
}
#endif

#endif /* OSD2D_H */
