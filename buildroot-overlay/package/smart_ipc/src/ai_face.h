/*
 * Face detection on a camera channel of its own
 *
 * A C door onto the nncase/OpenCV code under ai/, the same way rtsp_out.h is
 * a C door onto librtsp_server. demo_ipc.c stays C.
 *
 * Everything below the door runs in a thread of its own: it opens its own
 * V4L2 node, takes BGR888-planar frames off it, runs them through the KPU and
 * parks the boxes behind a mutex. The main loop never blocks on inference,
 * and the camera buffers the encoder and the screen use are not touched.
 *
 * The caller draws when it wants to, by watching ai_face_generation().
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef AI_FACE_H
#define AI_FACE_H

#include <display.h> /* enum drm_rotation */

#include "osd2d.h" /* struct osd2d_box, struct osd2d_patch */

#ifdef __cplusplus
extern "C" {
#endif

struct ai_face;

struct ai_face_cfg {
	/** V4L2 node to capture from. Must not be one the rest of the demo is
	 *  already using: the ISP gives a sensor several channels, and this
	 *  wants one to itself. */
	const char *dev;

	/** kmodel to load. The 320 and 640 variants of the face detector are
	 *  both understood; which one it is is read out of the model. */
	const char *kmodel;

	/** Capture size. Bigger finds smaller faces and costs more; the
	 *  network itself always sees a letterboxed square. */
	unsigned width, height;

	/** How many capture buffers to ask the driver for. */
	unsigned buffers;

	float obj_thresh; /* keep boxes above this score */
	float nms_thresh; /* overlap at which two boxes are one face */

	/** 0 quiet, 1 per-stage timings, 2 everything. Straight through to
	 *  the vendor code's debug_mode. */
	int debug;
};

/** Fill in the defaults: 1280x720, 3 buffers, 0.6 / 0.2, quiet. */
void ai_face_default_cfg(struct ai_face_cfg *cfg);

/**
 * Open the camera, load the model, start the thread.
 *
 * Returns NULL and explains itself on stderr. @cfg is copied, apart from the
 * strings, which have to outlive the returned handle.
 */
struct ai_face *ai_face_start(const struct ai_face_cfg *cfg);

/** Stop the thread, close the camera, unload the model. NULL is fine. */
void ai_face_stop(struct ai_face *a);

/**
 * Bumped every time a new set of boxes lands, so the caller can skip redrawing
 * an overlay that has not changed. Starts at 0, which no result ever has.
 */
unsigned long ai_face_generation(const struct ai_face *a);

/**
 * Paint the latest boxes into an ARGB8888 buffer, clearing it first.
 *
 * @argb    the top left corner of the part of the overlay the camera picture
 *          sits behind. When the picture is centred on a larger panel, that
 *          is the mapped framebuffer plus the offset it is shown at, not the
 *          start of the buffer.
 * @w, @h   the size of that part, in pixels: the boxes are scaled into it, so
 *          the AI channel and the preview channel can be different sizes.
 * @stride  the buffer's bytes per line
 * @rot     how the picture underneath was turned on its way to the panel; the
 *          coordinates are turned the same way so the boxes land on the faces.
 *          Only the coordinates are turned, never the pixels: a quarter turn
 *          leaves a rectangle axis-aligned, so there is nothing to rotate.
 *
 * Returns how many faces were drawn, or -1 if @a is NULL.
 *
 * Safe to call from the main loop: it takes the lock for as long as it needs
 * to copy the results out, and the drawing happens outside it.
 */
int ai_face_draw(struct ai_face *a, void *argb, unsigned w, unsigned h,
		 unsigned stride, enum drm_rotation rot);

/**
 * The same results, but as shapes for osd2d - for burning into the picture
 * itself rather than painting them on an overlay above it.
 *
 * The two are deliberately separate calls. The overlay is a whole plane that
 * exists anyway, so ai_face_draw() can afford to clear it and draw anywhere
 * on it; the hardware copies every patch into the kernel on every frame, so
 * this one keeps what it hands over down to the size of what is actually
 * being drawn. See osd2d.h.
 *
 * @w, @h   the size of the picture the shapes are for, which is the encoder's
 *          channel and need not match the AI channel - coordinates are scaled.
 *          No rotation: what gets encoded is the camera's own orientation.
 * @max     how many faces to decorate at most. Past that the landmark dots go
 *          first and the boxes last, because a box is the detection itself and
 *          the dots are decoration - and in a crowd every face is small enough
 *          that five dots on it are a smudge anyway. 0 means a sane default.
 *
 * On return @boxes and @patches point at storage owned by @a, valid until the
 * next call. Both counts can be 0, which means there is nothing to draw.
 *
 * Returns the number of faces decorated, or -1 if @a is NULL.
 *
 * Call it from one thread only - the main loop - the way ai_face_draw() is.
 */
int ai_face_shapes(struct ai_face *a, unsigned w, unsigned h, int max,
		   const struct osd2d_box **boxes, int *nbox,
		   const struct osd2d_patch **patches, int *npatch);

/**
 * Push a freshly written overlay out of the CPU's cache, so the display
 * controller reads what was drawn rather than what used to be there.
 *
 * It lives here only because the instruction that does it needs the compiler
 * told which core it is building for, and this file is the one built that
 * way. Nothing about it is specific to faces; call it after writing to any
 * mapped framebuffer by hand.
 */
void ai_face_flush(void *addr, unsigned long size);

/** Frames the KPU has got through, for the closing summary. */
unsigned long ai_face_frames(const struct ai_face *a);

/** Faces in the most recent result. */
unsigned ai_face_count(const struct ai_face *a);

#ifdef __cplusplus
}
#endif

#endif /* AI_FACE_H */
