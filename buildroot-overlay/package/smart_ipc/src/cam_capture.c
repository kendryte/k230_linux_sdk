/*
 * Minimal V4L2 camera capture; see cam_capture.h for what it is for.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define _GNU_SOURCE

#include "cam_capture.h"

#include <errno.h>
#include <fcntl.h>
#include <linux/videodev2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

#define CAM_TYPE V4L2_BUF_TYPE_VIDEO_CAPTURE

/*
 * vvcam private controls that pin the sensor's output mode. The definitive
 * copy of these lives in
 * package/vvcam/v4l2-drm/include/vvcam_sensor_target.h; they are repeated
 * here rather than included so that this demo keeps building without the
 * vvcam staging headers.
 */
#define VVCAM_CID_BASE 0x00980900
#define VVCAM_CID_SENSOR_ENABLE (VVCAM_CID_BASE + 2)
#define VVCAM_CID_SENSOR_WIDTH (VVCAM_CID_BASE + 3)
#define VVCAM_CID_SENSOR_HEIGHT (VVCAM_CID_BASE + 4)
#define VVCAM_CID_SENSOR_FPS (VVCAM_CID_BASE + 5)

#define cam_err(cam, fmt, ...) \
	fprintf(stderr, "[cam %s] " fmt "\n", (cam)->dev, ##__VA_ARGS__)

static int cam_ioctl(int fd, unsigned long req, void *arg)
{
	int ret;

	do {
		ret = ioctl(fd, req, arg);
	} while (ret < 0 && errno == EINTR);

	return ret;
}

/*
 * Tell the ISP which mode to drive the sensor in. The kernel only stores the
 * values on the video node; they are read out again when the pipeline is
 * created, and creation is what the first G_FMT triggers. So this has to sit
 * between open() and cam_set_format(), which is the same order v4l2-drm uses
 * (see lib.c, "open video -> S_EXT_CTRLS sensor target -> G_FMT (CREATE)").
 *
 * Failing is not fatal: the ISP then stays on its default mode, which is
 * good enough whenever that mode already covers the requested resolution.
 */
static int cam_set_sensor_mode(struct cam_capture *cam,
			       const struct cam_sensor_mode *mode)
{
	struct v4l2_ext_control ctrl[4];
	struct v4l2_ext_controls ctrls;

	memset(ctrl, 0, sizeof(ctrl));
	ctrl[0].id = VVCAM_CID_SENSOR_ENABLE;
	ctrl[0].value = 1;
	ctrl[1].id = VVCAM_CID_SENSOR_WIDTH;
	ctrl[1].value = (int32_t)mode->width;
	ctrl[2].id = VVCAM_CID_SENSOR_HEIGHT;
	ctrl[2].value = (int32_t)mode->height;
	ctrl[3].id = VVCAM_CID_SENSOR_FPS;
	ctrl[3].value = (int32_t)mode->fps;

	memset(&ctrls, 0, sizeof(ctrls));
	ctrls.ctrl_class = V4L2_CTRL_CLASS_USER;
	ctrls.count = 4;
	ctrls.controls = ctrl;

	if (cam_ioctl(cam->fd, VIDIOC_S_EXT_CTRLS, &ctrls) < 0) {
		cam_err(cam, "sensor mode %ux%u@%u rejected: %s - carrying on "
			     "with the ISP default mode",
			mode->width, mode->height, mode->fps, strerror(errno));
		return -EIO;
	}

	return 0;
}

static int cam_set_format(struct cam_capture *cam, unsigned width,
			  unsigned height, uint32_t pixelformat)
{
	struct v4l2_format f;
	int retry;

	/*
	 * G_FMT first so the driver's current structure is filled in and only
	 * the interesting fields change. G_FMT fails for a moment right after
	 * the ISP comes up, hence the retries.
	 */
	memset(&f, 0, sizeof(f));
	f.type = CAM_TYPE;

	for (retry = 0; retry < 5; retry++) {
		if (retry > 0)
			usleep(100000 * (unsigned)retry);
		if (cam_ioctl(cam->fd, VIDIOC_G_FMT, &f) == 0)
			break;
	}

	if (retry >= 5) {
		cam_err(cam, "G_FMT failed: %s (is the ISP up?)",
			strerror(errno));
		return -EIO;
	}

	f.fmt.pix.pixelformat = pixelformat;
	f.fmt.pix.width = width & ~7U;
	f.fmt.pix.height = height & ~1U;
	f.fmt.pix.field = V4L2_FIELD_NONE;

	if (cam_ioctl(cam->fd, VIDIOC_S_FMT, &f) < 0) {
		cam_err(cam, "S_FMT %ux%u failed: %s", width, height,
			strerror(errno));
		return -EIO;
	}

	/* The driver may adjust the resolution and stride, so what it reads
	 * back is what counts */
	cam->width = f.fmt.pix.width;
	cam->height = f.fmt.pix.height;
	cam->stride = f.fmt.pix.bytesperline;
	cam->pixelformat = f.fmt.pix.pixelformat;
	cam->frame_size = f.fmt.pix.sizeimage;

	if (cam->pixelformat != pixelformat) {
		cam_err(cam, "the driver does not support that format, it "
			     "gave 0x%08x",
			cam->pixelformat);
		return -EINVAL;
	}

	return 0;
}

static int cam_alloc_buffers(struct cam_capture *cam, unsigned count,
			     bool need_mmap)
{
	struct v4l2_requestbuffers req;
	unsigned i;

	memset(&req, 0, sizeof(req));
	req.type = CAM_TYPE;
	req.memory = V4L2_MEMORY_MMAP;
	req.count = count;

	if (cam_ioctl(cam->fd, VIDIOC_REQBUFS, &req) < 0) {
		cam_err(cam, "REQBUFS(%u) failed: %s", count, strerror(errno));
		return -EIO;
	}

	if (req.count == 0) {
		cam_err(cam, "REQBUFS granted no buffers at all");
		return -ENOMEM;
	}

	if (req.count < count)
		cam_err(cam, "only got %u buffers, asked for %u", req.count,
			count);

	cam->bufs = calloc(req.count, sizeof(*cam->bufs));
	if (cam->bufs == NULL)
		return -ENOMEM;

	cam->count = req.count;
	cam->mapped = need_mmap;

	for (i = 0; i < cam->count; i++)
		cam->bufs[i].dmabuf_fd = -1;

	for (i = 0; i < cam->count; i++) {
		struct v4l2_buffer buf;
		struct v4l2_exportbuffer exp;

		memset(&buf, 0, sizeof(buf));
		buf.type = CAM_TYPE;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index = i;

		if (cam_ioctl(cam->fd, VIDIOC_QUERYBUF, &buf) < 0) {
			cam_err(cam, "QUERYBUF(%u) failed: %s", i,
				strerror(errno));
			return -EIO;
		}

		cam->bufs[i].size = buf.length;

		/*
		 * Export the dmabuf. vvcam uses vb2_dma_contig, that is
		 * physically contiguous memory, which is exactly what the
		 * scatter-gather queues of the VPU can import. The other
		 * way round does not work.
		 */
		memset(&exp, 0, sizeof(exp));
		exp.type = CAM_TYPE;
		exp.index = i;
		exp.plane = 0;
		exp.flags = O_CLOEXEC | O_RDWR;

		if (cam_ioctl(cam->fd, VIDIOC_EXPBUF, &exp) < 0) {
			cam_err(cam, "EXPBUF(%u) failed: %s - no dmabuf "
				     "means no zero copy",
				i, strerror(errno));
			return -EIO;
		}

		cam->bufs[i].dmabuf_fd = exp.fd;

		if (!need_mmap)
			continue;

		cam->bufs[i].addr = mmap(NULL, buf.length,
					 PROT_READ | PROT_WRITE, MAP_SHARED,
					 cam->fd, buf.m.offset);
		if (cam->bufs[i].addr == MAP_FAILED) {
			cam->bufs[i].addr = NULL;
			cam_err(cam, "mmap(%u) failed: %s", i, strerror(errno));
			return -ENOMEM;
		}
	}

	return 0;
}

int cam_open(struct cam_capture *cam, const char *dev,
	     const struct cam_sensor_mode *sensor, unsigned width,
	     unsigned height, uint32_t pixelformat, unsigned count,
	     bool need_mmap)
{
	struct v4l2_capability cap;
	int ret;

	if (cam == NULL || dev == NULL || count == 0)
		return -EINVAL;

	memset(cam, 0, sizeof(*cam));
	cam->fd = -1;
	snprintf(cam->dev, sizeof(cam->dev), "%s", dev);

	cam->fd = open(dev, O_RDWR | O_NONBLOCK | O_CLOEXEC);
	if (cam->fd < 0) {
		cam_err(cam, "open failed: %s", strerror(errno));
		return -errno;
	}

	memset(&cap, 0, sizeof(cap));
	if (cam_ioctl(cam->fd, VIDIOC_QUERYCAP, &cap) < 0) {
		cam_err(cam, "QUERYCAP failed: %s", strerror(errno));
		ret = -EIO;
		goto fail;
	}

	if ((cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) == 0 &&
	    (cap.device_caps & V4L2_CAP_VIDEO_CAPTURE) == 0) {
		cam_err(cam, "not a single plane capture device "
			     "(driver=%s card=%s)",
			cap.driver, cap.card);
		ret = -ENODEV;
		goto fail;
	}

	/* Before the first G_FMT, see cam_set_sensor_mode() */
	if (sensor != NULL && sensor->width != 0 && sensor->height != 0 &&
	    sensor->fps != 0)
		(void)cam_set_sensor_mode(cam, sensor);

	ret = cam_set_format(cam, width, height, pixelformat);
	if (ret < 0)
		goto fail;

	ret = cam_alloc_buffers(cam, count, need_mmap);
	if (ret < 0)
		goto fail;

	return 0;

fail:
	cam_close(cam);

	return ret;
}

int cam_start(struct cam_capture *cam)
{
	uint32_t type = CAM_TYPE;
	unsigned i;

	if (cam == NULL || cam->fd < 0)
		return -EINVAL;

	for (i = 0; i < cam->count; i++) {
		int ret = cam_qbuf(cam, i);

		if (ret < 0)
			return ret;
	}

	if (cam_ioctl(cam->fd, VIDIOC_STREAMON, &type) < 0) {
		cam_err(cam, "STREAMON failed: %s", strerror(errno));
		return -EIO;
	}

	cam->streaming = true;

	return 0;
}

int cam_dqbuf(struct cam_capture *cam, unsigned *index, int64_t *pts)
{
	struct v4l2_buffer buf;

	if (cam == NULL || index == NULL)
		return -EINVAL;

	memset(&buf, 0, sizeof(buf));
	buf.type = CAM_TYPE;
	buf.memory = V4L2_MEMORY_MMAP;

	if (cam_ioctl(cam->fd, VIDIOC_DQBUF, &buf) < 0) {
		if (errno == EAGAIN)
			return -EAGAIN;
		cam_err(cam, "DQBUF failed: %s", strerror(errno));
		return -EIO;
	}

	*index = buf.index;

	if (pts != NULL)
		*pts = (int64_t)buf.timestamp.tv_sec * 1000000 +
		       buf.timestamp.tv_usec;

	return 0;
}

int cam_qbuf(struct cam_capture *cam, unsigned index)
{
	struct v4l2_buffer buf;

	if (cam == NULL || index >= cam->count)
		return -EINVAL;

	memset(&buf, 0, sizeof(buf));
	buf.type = CAM_TYPE;
	buf.memory = V4L2_MEMORY_MMAP;
	buf.index = index;

	if (cam_ioctl(cam->fd, VIDIOC_QBUF, &buf) < 0) {
		cam_err(cam, "QBUF(%u) failed: %s", index, strerror(errno));
		return -EIO;
	}

	return 0;
}

void cam_close(struct cam_capture *cam)
{
	unsigned i;

	if (cam == NULL || cam->fd < 0)
		return;

	if (cam->streaming) {
		uint32_t type = CAM_TYPE;

		cam_ioctl(cam->fd, VIDIOC_STREAMOFF, &type);
		cam->streaming = false;
	}

	if (cam->bufs != NULL) {
		for (i = 0; i < cam->count; i++) {
			if (cam->bufs[i].addr != NULL)
				munmap(cam->bufs[i].addr, cam->bufs[i].size);
			if (cam->bufs[i].dmabuf_fd >= 0)
				close(cam->bufs[i].dmabuf_fd);
		}
		free(cam->bufs);
		cam->bufs = NULL;
	}

	cam->count = 0;
	close(cam->fd);
	cam->fd = -1;
}
