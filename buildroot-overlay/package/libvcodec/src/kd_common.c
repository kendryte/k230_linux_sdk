/*
 * libvcodec - shared implementation: logging, device probing, formats
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define _GNU_SOURCE

#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "kd_common.h"

/* Names the VPU driver reports, see querycap in mvx_v4l2_vidioc.c */
#define KD_VPU_DRIVER_NAME "mvx"
#define KD_VPU_CARD_NAME "Linlon Video device"

/* Probing scans /dev/video0 .. /dev/video63 */
#define KD_PROBE_MAX_NODE 64

kd_log_level kd_g_log_level = KD_LOG_ERROR;

/* ========================================================================
 * Logging and error codes
 * ======================================================================== */

void kd_set_log_level(kd_log_level level)
{
	kd_g_log_level = level;
}

void kd_log_impl(kd_log_level level, const char *fmt, ...)
{
	static const char *const tag[] = { "", "E", "W", "I", "D" };
	va_list ap;

	if (level < KD_LOG_ERROR || level > KD_LOG_DEBUG)
		return;

	fprintf(stderr, "[vcodec:%s] ", tag[level]);
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	fputc('\n', stderr);
}

const char *kd_strerror(int err)
{
	switch (err) {
	case KD_OK:
		return "success";
	case KD_ERR_AGAIN:
		return "no data or no free slot right now, retry later";
	case KD_ERR_EOS:
		return "end of stream";
	case KD_ERR_SRC_CHANGE:
		return "resolution known or changed, re-attach output buffers";
	case KD_ERR_INVAL:
		return "invalid argument";
	case KD_ERR_NOMEM:
		return "out of memory";
	case KD_ERR_IO:
		return "driver reported an error";
	case KD_ERR_NODEV:
		return "no VPU device node found";
	case KD_ERR_STATE:
		return "called at the wrong time";
	default:
		return "unknown error";
	}
}

int kd_errno_to_err(int err)
{
	switch (err) {
	case EAGAIN:
		return KD_ERR_AGAIN;
	case EPIPE:
		return KD_ERR_EOS;
	case EINVAL:
		return KD_ERR_INVAL;
	case ENOMEM:
		return KD_ERR_NOMEM;
	case ENODEV:
	case ENOENT:
		return KD_ERR_NODEV;
	default:
		return KD_ERR_IO;
	}
}

int kd_ioctl(int fd, unsigned long req, void *arg)
{
	int ret;

	do {
		ret = ioctl(fd, req, arg);
	} while (ret < 0 && errno == EINTR);

	return ret;
}

int kd_poll(int fd, short events, int timeout_ms, short *revents)
{
	struct pollfd pfd = { .fd = fd, .events = events, .revents = 0 };
	int ret;

	do {
		ret = poll(&pfd, 1, timeout_ms);
	} while (ret < 0 && errno == EINTR);

	if (ret < 0)
		return kd_errno_to_err(errno);

	if (revents != NULL)
		*revents = pfd.revents;

	return ret == 0 ? KD_ERR_AGAIN : KD_OK;
}

/* ========================================================================
 * Device probing
 *
 * Never hard-code the node number. On K230 /dev/video0 is usually a camera
 * (vvcam), and the VPU node is allocated by video_register_device(..., -1),
 * so it moves as soon as the probe order changes.
 * ======================================================================== */

static bool kd_node_is_vpu(int fd)
{
	struct v4l2_capability cap;

	memset(&cap, 0, sizeof(cap));
	if (kd_ioctl(fd, VIDIOC_QUERYCAP, &cap) < 0)
		return false;

	if (strcmp((const char *)cap.driver, KD_VPU_DRIVER_NAME) == 0)
		return true;

	/* Fall back to the card name in case the driver gets renamed */
	return strcmp((const char *)cap.card, KD_VPU_CARD_NAME) == 0;
}

static int kd_probe_fd(char *path_out, size_t len)
{
	int i;

	for (i = 0; i < KD_PROBE_MAX_NODE; i++) {
		char path[32];
		int fd;

		snprintf(path, sizeof(path), "/dev/video%d", i);

		fd = open(path, O_RDWR | O_NONBLOCK | O_CLOEXEC);
		if (fd < 0)
			continue;

		if (kd_node_is_vpu(fd)) {
			KD_LOGI("found VPU device %s", path);
			if (path_out != NULL)
				snprintf(path_out, len, "%s", path);
			return fd;
		}

		close(fd);
	}

	KD_LOGE("scanned /dev/video0..%d without finding driver=\"%s\"; "
		"check that the VPU driver is built in (dmesg | grep -i linlon)",
		KD_PROBE_MAX_NODE - 1, KD_VPU_DRIVER_NAME);

	return KD_ERR_NODEV;
}

int kd_probe_device(char *path, size_t len)
{
	int fd;

	if (path == NULL || len == 0)
		return KD_ERR_INVAL;

	fd = kd_probe_fd(path, len);
	if (fd < 0)
		return fd;

	close(fd);

	return KD_OK;
}

int kd_open_device(const char *dev)
{
	int fd;

	if (dev == NULL)
		return kd_probe_fd(NULL, 0);

	fd = open(dev, O_RDWR | O_NONBLOCK | O_CLOEXEC);
	if (fd < 0) {
		KD_LOGE("cannot open %s: %s", dev, strerror(errno));
		return kd_errno_to_err(errno);
	}

	if (!kd_node_is_vpu(fd)) {
		KD_LOGE("%s is not the VPU node (driver is not \"%s\"); "
			"pass NULL to let the library probe",
			dev, KD_VPU_DRIVER_NAME);
		close(fd);
		return KD_ERR_NODEV;
	}

	return fd;
}

/* ========================================================================
 * Format conversion
 * ======================================================================== */

uint32_t kd_pixfmt_fourcc(kd_pixfmt f)
{
	switch (f) {
	case KD_PIXFMT_NV12:
		return V4L2_PIX_FMT_NV12;
	case KD_PIXFMT_NV21:
		return V4L2_PIX_FMT_NV21;
	case KD_PIXFMT_P010:
		return V4L2_PIX_FMT_P010;
	case KD_PIXFMT_YUYV:
		return V4L2_PIX_FMT_YUYV;
	default:
		return 0;
	}
}

int kd_fourcc_pixfmt(uint32_t fourcc, kd_pixfmt *out)
{
	kd_pixfmt f;

	switch (fourcc) {
	case V4L2_PIX_FMT_NV12:
		f = KD_PIXFMT_NV12;
		break;
	case V4L2_PIX_FMT_NV21:
		f = KD_PIXFMT_NV21;
		break;
	case V4L2_PIX_FMT_P010:
		f = KD_PIXFMT_P010;
		break;
	case V4L2_PIX_FMT_YUYV:
		f = KD_PIXFMT_YUYV;
		break;
	default:
		return KD_ERR_INVAL;
	}

	if (out != NULL)
		*out = f;

	return KD_OK;
}

uint32_t kd_codec_fourcc(kd_codec c)
{
	switch (c) {
	case KD_CODEC_H264:
		return V4L2_PIX_FMT_H264;
	case KD_CODEC_H265:
		return V4L2_PIX_FMT_HEVC;
	case KD_CODEC_JPEG:
		return V4L2_PIX_FMT_JPEG;
	default:
		return 0;
	}
}

unsigned kd_pixfmt_bpp(kd_pixfmt f)
{
	switch (f) {
	case KD_PIXFMT_NV12:
	case KD_PIXFMT_NV21:
		return 1;
	case KD_PIXFMT_P010:
	case KD_PIXFMT_YUYV:
		return 2;
	default:
		return 1;
	}
}

unsigned kd_default_stride(kd_pixfmt f, unsigned width)
{
	return width * kd_pixfmt_bpp(f);
}

size_t kd_frame_size(kd_pixfmt f, unsigned stride, unsigned height)
{
	switch (f) {
	case KD_PIXFMT_NV12:
	case KD_PIXFMT_NV21:
	case KD_PIXFMT_P010:
		/* 4:2:0, the chroma plane is half the height of luma */
		return (size_t)stride * height * 3 / 2;
	case KD_PIXFMT_YUYV:
		/* packed 4:2:2, chroma is already part of the stride */
		return (size_t)stride * height;
	default:
		return (size_t)stride * height;
	}
}

void kd_pts_to_tv(int64_t pts, struct timeval *tv)
{
	if (pts < 0)
		pts = 0;

	tv->tv_sec = (time_t)(pts / 1000000);
	tv->tv_usec = (suseconds_t)(pts % 1000000);
}

int64_t kd_tv_to_pts(const struct timeval *tv)
{
	return (int64_t)tv->tv_sec * 1000000 + (int64_t)tv->tv_usec;
}

/* ========================================================================
 * STREAMON / STREAMOFF
 * ======================================================================== */

int kd_streamon(int fd, uint32_t type)
{
	uint32_t t = type;

	if (kd_ioctl(fd, VIDIOC_STREAMON, &t) < 0) {
		KD_LOGE("STREAMON(type=%u) failed: %s", type, strerror(errno));
		return kd_errno_to_err(errno);
	}

	return KD_OK;
}

int kd_streamoff(int fd, uint32_t type)
{
	uint32_t t = type;

	if (kd_ioctl(fd, VIDIOC_STREAMOFF, &t) < 0) {
		KD_LOGW("STREAMOFF(type=%u) failed: %s", type, strerror(errno));
		return kd_errno_to_err(errno);
	}

	return KD_OK;
}
