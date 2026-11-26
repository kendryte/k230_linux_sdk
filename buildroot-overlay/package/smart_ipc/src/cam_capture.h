/*
 * Minimal V4L2 camera capture
 *
 * Only the steps an encoder needs: open -> S_FMT -> REQBUFS(MMAP) -> EXPBUF
 * -> QBUF -> STREAMON. The dmabuf fds EXPBUF hands out go straight into the
 * codec library, so the YUV data never passes through the CPU.
 *
 * libv4l2-drm is deliberately not used: its v4l2_drm_run() brings its own
 * event loop, which fights the epoll loop this demo is built around. This
 * file doubles as a worked example of attaching any V4L2 source to the
 * encoder.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CAM_CAPTURE_H
#define CAM_CAPTURE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

struct cam_buffer {
	int dmabuf_fd; /* fd handed out by EXPBUF */
	void *addr; /* set when need_mmap was asked for, else NULL */
	size_t size;
};

/**
 * The mode the sensor itself is to run in, which is a different thing from
 * the format of the video node: the node only scales and crops whatever the
 * sensor produces. Leaving this to chance is the usual reason a capture that
 * works under v4l2-drm fails here, so pin it.
 *
 * All three fields have to be filled in, the ISP resolves them together.
 */
struct cam_sensor_mode {
	unsigned width;
	unsigned height;
	unsigned fps;
};

struct cam_capture {
	int fd;
	char dev[64];

	unsigned width;
	unsigned height;
	unsigned stride; /* bytesperline reported by the driver */
	uint32_t pixelformat; /* V4L2 fourcc */
	size_t frame_size; /* sizeimage reported by the driver */

	unsigned count;
	struct cam_buffer *bufs;

	bool mapped;
	bool streaming;
};

/**
 * Open the camera, pin the sensor mode, set the format and allocate buffers.
 *
 * @param sensor     sensor mode to request, NULL to leave the ISP on whatever
 *                   it defaults to
 * @param need_mmap  also map the buffers into the process, which --copy mode
 *                   needs
 *
 * The driver adjusts what it is given, so read the resolution, stride and
 * frame size back out of @cam once this returns.
 */
int cam_open(struct cam_capture *cam, const char *dev,
	     const struct cam_sensor_mode *sensor, unsigned width,
	     unsigned height, uint32_t pixelformat, unsigned count,
	     bool need_mmap);

/** Queue every buffer and STREAMON. */
int cam_start(struct cam_capture *cam);

/**
 * Take one frame.
 * Returns 0 on success, -EAGAIN when nothing is ready, another negative
 * value on error.
 * @param pts  frame timestamp in microseconds
 */
int cam_dqbuf(struct cam_capture *cam, unsigned *index, int64_t *pts);

/** Give one frame back to the camera. */
int cam_qbuf(struct cam_capture *cam, unsigned index);

void cam_close(struct cam_capture *cam);

#endif /* CAM_CAPTURE_H */
