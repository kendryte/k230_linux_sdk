/*
 * Hardware OSD over the K230 2D engine; see osd2d.h for what it is for.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define _GNU_SOURCE

#include "osd2d.h"

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <time.h>
#include <unistd.h>

#include <linux/videodev2.h>
#include <nonai2d-v4l2-controls.h>

/*
 * Buffers are queued on the multi-planar type because that is the queue the
 * driver sets up, but the format is set on the plain one: single-planar is
 * how the driver is told that a frame arrives as one buffer with chroma
 * inside it rather than as two separate ones. See osd2d_set_format().
 */
#define OSD2D_TYPE V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE
#define OSD2D_FMT_TYPE V4L2_BUF_TYPE_VIDEO_CAPTURE
#define OSD2D_DRIVER "nonai-2d"
#define OSD2D_MAX_FRAMES 16

/* How much the hardware can hold at once; anything more needs another pass */
#define OSD2D_MAX_PATCH NONAI2D_OSD_REGION_NUM /* 8 */
#define OSD2D_MAX_BOX NONAI2D_BORDER_NUM /* 32 */

/*
 * How the four bytes of a patch pixel are read.
 *
 * The engine takes them as alpha, red, green, blue, in that order from the
 * first byte - the other way round from the DRM ARGB8888 buffer the screen
 * overlay draws into, where the word reads 0xAARRGGBB and alpha is the last
 * byte. osd2d.h says as much, and the caller turns its colours round.
 *
 * The fourcc below neither decides that nor can fix it: the driver maps
 * ARGB32 and ABGR32 onto the same K_2D_OSD_FMT_ARGB8888 (nonai_2d_dev.c:121
 * and :137) and only that reaches the engine. Only the bytes matter.
 */
#define OSD2D_PATCH_FOURCC V4L2_PIX_FMT_ARGB32

#define osd2d_err(o, fmt, ...) \
	fprintf(stderr, "[osd2d %s] " fmt "\n", (o)->dev, ##__VA_ARGS__)

struct osd2d {
	int fd;
	char dev[64];

	unsigned width, height;
	size_t frame_size; /* the whole NV12 frame, Y and UV together */

	bool streaming;
	unsigned long passes;

	struct osd2d_timing timing;

	/*
	 * Border slots the hardware has been told about at some point. It
	 * never forgets them (see osd2d_set_boxes), so this is how far the
	 * overwriting has to reach.
	 */
	int box_slots;

	/*
	 * The caller's frames, by dmabuf handle. Buffer index i always gets
	 * fds[i], which matters: vb2 keeps the import it made last time and
	 * only redoes the work when the handle under an index changes.
	 */
	int fds[OSD2D_MAX_FRAMES];
	unsigned nframes;
};

static unsigned long osd2d_now(void)
{
	struct timespec ts;

	clock_gettime(CLOCK_MONOTONIC, &ts);

	return (unsigned long)ts.tv_sec * 1000000ul +
	       (unsigned long)ts.tv_nsec / 1000ul;
}

static int osd2d_ioctl(int fd, unsigned long req, void *arg)
{
	int ret;

	do {
		ret = ioctl(fd, req, arg);
	} while (ret < 0 && errno == EINTR);

	return ret;
}

/*
 * Which driver is behind a video node, read rather than asked.
 *
 * /sys/class/video4linux/videoN/device/driver is a symlink to whichever
 * driver claimed the node, and its basename is the same string that
 * VIDIOC_QUERYCAP reports in cap.driver.
 *
 * Opening each node and asking it would be the obvious way, and is what this
 * used to do. It is not safe here. vvcam_video_release() in the ISP driver
 * calls schedule_work(&destroy_work) on *every* close, without checking
 * whether other handles on that node are still open, and that work tears the
 * channel's pipeline down in the ISP daemon. So a scan that opens and closes
 * every /dev/video* in turn silently destroys the pipeline of a channel this
 * same process is in the middle of streaming - the hardware keeps running, so
 * the current run looks fine, but the daemon is left with half-torn-down
 * state and refuses STREAMON on the next one. That is a kernel bug, but it
 * costs nothing to stay out of its way: look at the nodes, do not touch them.
 */
static int osd2d_driver_of(const char *node, char *out, size_t len)
{
	char link[128];
	char target[256];
	const char *base;
	ssize_t n;

	if (snprintf(link, sizeof(link),
		     "/sys/class/video4linux/%s/device/driver",
		     node) >= (int)sizeof(link))
		return -ENAMETOOLONG;

	n = readlink(link, target, sizeof(target) - 1);
	if (n < 0)
		return -errno;
	target[n] = '\0';

	base = strrchr(target, '/');
	base = base != NULL ? base + 1 : target;

	if (snprintf(out, len, "%s", base) >= (int)len)
		return -ENAMETOOLONG;

	return 0;
}

/*
 * The 2D engine is just another /dev/video*, and which number it lands on
 * depends on probe order, so ask rather than guess.
 */
static int osd2d_find(char *out, size_t len)
{
	DIR *d = opendir("/sys/class/video4linux");
	struct dirent *e;
	int found = -ENODEV;

	if (d == NULL)
		return -errno;

	while ((e = readdir(d)) != NULL) {
		char driver[64];

		if (strncmp(e->d_name, "video", 5) != 0)
			continue;

		if (osd2d_driver_of(e->d_name, driver, sizeof(driver)) < 0)
			continue;

		if (strcmp(driver, OSD2D_DRIVER) != 0)
			continue;

		/* A name too long to hold is not a node this is looking for */
		if (snprintf(out, len, "/dev/%s", e->d_name) >= (int)len)
			continue;

		found = 0;
		break;
	}

	closedir(d);

	return found;
}

static int osd2d_set_mode(struct osd2d *o)
{
	struct v4l2_control c;

	memset(&c, 0, sizeof(c));
	c.id = V4L2_CID_USER_NONAI2D_MODE;
	c.value = NONAI2D_MODE_OSD_BORDER;

	if (osd2d_ioctl(o->fd, VIDIOC_S_CTRL, &c) < 0) {
		osd2d_err(o, "selecting OSD+border mode failed: %s",
			  strerror(errno));
		return -EIO;
	}

	return 0;
}

/*
 * Ask for NV12 on the *single-planar* capture type.
 *
 * That choice is the whole trick. Set multi-planar and the driver takes NV12
 * to mean two separate buffers, which is a shape nothing else on this chip
 * produces: a camera frame is one block with chroma sitting at width x height
 * into it, behind a single dmabuf handle, and one handle cannot describe two
 * buffers. Single-planar says "one buffer, planes inside", and the driver
 * then works the chroma address out the way the RTOS firmware always has.
 */
static int osd2d_set_format(struct osd2d *o)
{
	struct v4l2_format f;
	struct v4l2_pix_format *p = &f.fmt.pix;
	size_t want = (size_t)o->width * o->height * 3 / 2;

	memset(&f, 0, sizeof(f));
	f.type = OSD2D_FMT_TYPE;
	p->pixelformat = V4L2_PIX_FMT_NV12;
	p->width = o->width;
	p->height = o->height;
	p->field = V4L2_FIELD_NONE;
	p->bytesperline = o->width;
	p->sizeimage = (uint32_t)want;

	if (osd2d_ioctl(o->fd, VIDIOC_S_FMT, &f) < 0) {
		osd2d_err(o, "S_FMT NV12 %ux%u failed: %s", o->width,
			  o->height, strerror(errno));
		return -EIO;
	}

	/*
	 * A module that still splits NV12 in two sizes the frame at the luma
	 * plane alone. Importing a camera frame into that would draw the boxes
	 * on the picture and leave the colours behind, so stop here and say
	 * which half of the pair is out of date.
	 */
	if (p->sizeimage < want) {
		osd2d_err(o,
			  "the driver sizes a %ux%u NV12 frame at %u bytes, "
			  "not the %zu a whole one takes. This needs the "
			  "nonai2d module that handles single-plane NV12; the "
			  "one loaded is older than this program.",
			  o->width, o->height, p->sizeimage, want);
		return -EINVAL;
	}

	/*
	 * The engine works out where each line starts from the width alone
	 * (calc_video_line_stride() in the kernel module); it never looks at
	 * bytesperline. A camera that pads its lines would come out sheared,
	 * and it is far easier to understand that here than on the screen.
	 */
	if (p->bytesperline != o->width) {
		osd2d_err(o, "this device only handles unpadded lines, and "
			     "it reports %u bytes for a %u pixel line",
			  p->bytesperline, o->width);
		return -EINVAL;
	}

	o->frame_size = p->sizeimage;

	return 0;
}

struct osd2d *osd2d_open(const char *dev, unsigned width, unsigned height)
{
	struct osd2d *o;

	if (width == 0 || height == 0 || (width & 1) != 0 || (height & 1) != 0)
		return NULL;

	o = calloc(1, sizeof(*o));
	if (o == NULL)
		return NULL;

	o->fd = -1;
	o->width = width;
	o->height = height;

	if (dev != NULL && *dev != '\0') {
		snprintf(o->dev, sizeof(o->dev), "%s", dev);
	} else if (osd2d_find(o->dev, sizeof(o->dev)) < 0) {
		fprintf(stderr,
			"[osd2d] no 2D engine found. Looked through "
			"/sys/class/video4linux/*/device/driver for one "
			"called \"%s\"; is the nonai2d module loaded? If it "
			"is there but under another name, name the node with "
			"--osd-dev.\n",
			OSD2D_DRIVER);
		free(o);
		return NULL;
	}

	o->fd = open(o->dev, O_RDWR | O_NONBLOCK | O_CLOEXEC);
	if (o->fd < 0) {
		fprintf(stderr, "[osd2d %s] open failed: %s\n", o->dev,
			strerror(errno));
		free(o);
		return NULL;
	}

	/* Mode before format: the mode decides which queues the driver will
	 * even look at */
	if (osd2d_set_mode(o) < 0 || osd2d_set_format(o) < 0)
		goto fail;

	return o;

fail:
	osd2d_close(o);

	return NULL;
}

/*
 * Take the caller's frames as they are, rather than keeping a frame here and
 * copying theirs in and out of it.
 *
 * The queue is asked for DMABUF memory, which means every buffer slot is empty
 * until a QBUF names a handle for it - the engine never owns a frame of its
 * own, it is pointed at one of the caller's for the length of a pass and then
 * let go of. This is what the RTOS firmware on the same chip does, and it is
 * the reason there is no memcpy anywhere below.
 *
 * The handles are only recorded here. vb2 does not look at them until the
 * first QBUF, so a wrong one shows up there and not in this function.
 */
int osd2d_attach(struct osd2d *o, const int *fds, unsigned n, size_t size)
{
	struct v4l2_requestbuffers req;
	unsigned i;

	if (o == NULL || fds == NULL || n == 0)
		return -EINVAL;

	if (n > OSD2D_MAX_FRAMES) {
		osd2d_err(o, "%u frames is more than the %d this can hold", n,
			  OSD2D_MAX_FRAMES);
		return -EINVAL;
	}

	/*
	 * Their frame has to be a whole NV12 one laid out the way the driver
	 * was told, because that layout is all it has to go on: chroma is read
	 * from width x height into the buffer, sight unseen.
	 */
	if (size < o->frame_size) {
		osd2d_err(o, "a %ux%u frame is %zu bytes, these are %zu",
			  o->width, o->height, o->frame_size, size);
		return -EINVAL;
	}

	memset(&req, 0, sizeof(req));
	req.type = OSD2D_TYPE;
	req.memory = V4L2_MEMORY_DMABUF;
	req.count = n;

	if (osd2d_ioctl(o->fd, VIDIOC_REQBUFS, &req) < 0) {
		osd2d_err(o, "REQBUFS for %u imported frame(s) failed: %s", n,
			  strerror(errno));
		return -EIO;
	}

	if (req.count < n) {
		osd2d_err(o, "asked for %u imported frame(s), got %u", n,
			  req.count);
		return -ENOMEM;
	}

	for (i = 0; i < n; i++) {
		if (fds[i] < 0) {
			osd2d_err(o, "frame %u has no dmabuf handle; the "
				     "camera has to be opened with export on",
				  i);
			return -EINVAL;
		}

		o->fds[i] = fds[i];
	}

	o->nframes = n;

	return 0;
}

/*
 * Load up the 8 picture slots.
 *
 * Every slot is written every time, including the ones being turned off: the
 * kernel decides how many slots to program from the highest one that is on,
 * and it reaches down from there through all the lower ones whether they are
 * on or not. Keeping them filled from 0 upwards is what makes that harmless.
 *
 * The pixels themselves are copied into the kernel here, once per slot per
 * call, which is the whole reason these are kept down to the size of what is
 * being drawn rather than being one picture the size of the screen.
 */
static int osd2d_set_patches(struct osd2d *o,
			     const struct osd2d_patch *patches, int n)
{
	nonai2d_osd_attr attr[NONAI2D_OSD_REGION_NUM];
	struct v4l2_ext_control ext;
	struct v4l2_ext_controls exts;
	int i;

	memset(attr, 0, sizeof(attr));

	for (i = 0; i < NONAI2D_OSD_REGION_NUM; i++) {
		attr[i].index = (uint8_t)i;

		if (i >= n || patches[i].argb == NULL ||
		    patches[i].w <= 0 || patches[i].h <= 0) {
			attr[i].valid = 0;
			continue;
		}

		attr[i].valid = 1;
		attr[i].startx = (uint16_t)patches[i].x;
		attr[i].starty = (uint16_t)patches[i].y;
		attr[i].width = (uint16_t)patches[i].w;
		attr[i].height = (uint16_t)patches[i].h;
		attr[i].data = (uint8_t *)(uintptr_t)patches[i].argb;
		attr[i].pixfmt = OSD2D_PATCH_FOURCC;

		/*
		 * Bottom to top: the picture, then the overlay. There is a
		 * third layer available underneath for a flat background
		 * colour, which is why bg_alpha and bg_color exist; leaving
		 * it out of the order keeps them out of it too.
		 *
		 * Both alphas are wide open because the transparency that
		 * matters is the one in each pixel: everything the drawing
		 * did not touch is left at alpha 0 and disappears.
		 */
		attr[i].add_order = NONAI2D_ADD_ORDER_VIDEO_OSD;
		attr[i].video_alpha = 255;
		attr[i].osd_alpha = 255;
	}

	memset(&ext, 0, sizeof(ext));
	ext.id = V4L2_CID_USER_NONAI2D_OSD_ATTR;
	ext.ptr = attr;
	ext.size = sizeof(attr);

	memset(&exts, 0, sizeof(exts));
	exts.which = V4L2_CTRL_WHICH_CUR_VAL;
	exts.count = 1;
	exts.controls = &ext;

	if (osd2d_ioctl(o->fd, VIDIOC_S_EXT_CTRLS, &exts) < 0) {
		osd2d_err(o, "loading %d overlay picture(s) failed: %s", n,
			  strerror(errno));
		return -EIO;
	}

	return 0;
}

/*
 * A colour the border hardware understands.
 *
 * The vendor's header calls the field (R << 16) | (G << 8) | B and the driver
 * passes it to the register untouched, but the register is not RGB: the
 * border unit writes its pixels straight into the output frame without going
 * near the colour space converter (TRM 7.4.3: "does not need to load data
 * through the bus, and directly writes the pixel value of the border"), and
 * the output frame is NV12. So the three bytes are Y, U and V.
 *
 * Which was easy to miss and plain to see: a box asked to be white came out
 * bright pink, white read as YUV being (255, 121, 255) in RGB. The ffmpeg
 * filter over the same hardware gives it away too - its default background
 * colour is 0xC88080, and 0x80 twice over is neutral chroma, not a colour
 * anybody picks in RGB.
 *
 * And it is packed the other way round from the field it is named after:
 * Cr, Cb, Y from the top byte down, so Y is where the header says B. The
 * RTOS side of this chip pinned that down on hardware with two single
 * colour tests - 0x55FF4C came out solid blue and 0x4C55FF solid green,
 * which no other permutation of the three bytes fits - and draws its red
 * box with 0xFF554C, BT.601 red (Y 76, Cb 85, Cr 255). Same register, same
 * answer here.
 *
 * BT.601 full range, which is what the encoder is fed elsewhere in this demo.
 */
static uint32_t osd2d_colour(unsigned rgb)
{
	int r = (int)((rgb >> 16) & 0xff);
	int g = (int)((rgb >> 8) & 0xff);
	int b = (int)(rgb & 0xff);
	int y = (299 * r + 587 * g + 114 * b) / 1000;
	int cb = (-169 * r - 331 * g + 500 * b) / 1000 + 128;
	int cr = (500 * r - 419 * g - 81 * b) / 1000 + 128;

	y = y < 0 ? 0 : (y > 255 ? 255 : y);
	cb = cb < 0 ? 0 : (cb > 255 ? 255 : cb);
	cr = cr < 0 ? 0 : (cr > 255 ? 255 : cr);

	return ((uint32_t)cr << 16) | ((uint32_t)cb << 8) | (uint32_t)y;
}

/*
 * Snap one edge pair onto the chroma grid, inward.
 *
 * The frame is NV12: one pair of chroma bytes covers a 2x2 block of pixels.
 * An edge landing on an odd row or column writes into a block it only half
 * covers and its two chroma bytes come back swapped, so that edge is drawn in
 * a different colour from the rest - a box in two colours at once rather than
 * a box in the wrong colour. Even coordinates keep every edge inside its own
 * block.
 *
 * Inward on both sides, because the hardware works the far edge out as
 * start+len and runs on into the next scanline if that overruns: an edge the
 * caller had flush with the picture must not be rounded past it.
 */
static void osd2d_snap(int start, int len, int limit, uint16_t *out_start,
		       uint16_t *out_len)
{
	int a = (start + 1) & ~1;
	int b = (start + len) & ~1;

	if (limit < 2)
		limit = 2;
	if (a > limit - 2)
		a = limit - 2;
	if (a < 0)
		a = 0;
	if (b < a + 2)
		b = a + 2;

	*out_start = (uint16_t)a;
	*out_len = (uint16_t)(b - a);
}

/*
 * Load up the border slots, one ioctl each.
 *
 * Switching a border off is not something the hardware offers - once a slot
 * has been used it is drawn on every frame from then on. So the slots left
 * over from a frame with more faces in it are not cleared, they are filled
 * with a copy of a box that is being drawn anyway. Drawing the same
 * rectangle twice lands on the same pixels, so it cannot be seen.
 *
 * When a pass has no boxes at all there is nothing to copy, and the slots are
 * given a zero line width instead - a line no pixels wide being invisible is
 * the one assumption here that the hardware has not been seen to confirm.
 * That path is only reached by a caller that asks for patches and no boxes;
 * with neither, osd2d_apply() never runs the engine and the stale slots are
 * left unread rather than cleared.
 */
static int osd2d_set_boxes(struct osd2d *o, const struct osd2d_box *boxes,
			   int n)
{
	int slots = n > o->box_slots ? n : o->box_slots;
	int i;

	if (slots > OSD2D_MAX_BOX)
		slots = OSD2D_MAX_BOX;

	for (i = 0; i < slots; i++) {
		const struct osd2d_box *b = (i < n) ? &boxes[i] :
						      (n > 0 ? &boxes[0] : NULL);
		nonai2d_border_attr attr;
		struct v4l2_ext_control ext;
		struct v4l2_ext_controls exts;

		memset(&attr, 0, sizeof(attr));
		attr.index = (uint8_t)i;

		if (b != NULL) {
			osd2d_snap(b->x, b->w, (int)o->width, &attr.startx,
				   &attr.width);
			osd2d_snap(b->y, b->h, (int)o->height, &attr.starty,
				   &attr.height);
			attr.line_width = (uint16_t)b->line;
			attr.color = osd2d_colour(b->rgb);
		}

		memset(&ext, 0, sizeof(ext));
		ext.id = V4L2_CID_USER_NONAI2D_BORDER_ATTR;
		ext.ptr = &attr;
		ext.size = sizeof(attr);

		memset(&exts, 0, sizeof(exts));
		exts.which = V4L2_CTRL_WHICH_CUR_VAL;
		exts.count = 1;
		exts.controls = &ext;

		if (osd2d_ioctl(o->fd, VIDIOC_S_EXT_CTRLS, &exts) < 0) {
			osd2d_err(o, "setting box %d failed: %s", i,
				  strerror(errno));
			return -EIO;
		}
	}

	if (slots > o->box_slots)
		o->box_slots = slots;

	return 0;
}

/*
 * One trip through the engine over frame @index, where it lies.
 *
 * Only one plane is handed over even though the frame holds two: that is the
 * point of the single-planar format set in osd2d_set_format(). One buffer, one
 * handle, chroma inside it - and the driver works out where from the width and
 * height it was given.
 */
static int osd2d_run(struct osd2d *o, unsigned index)
{
	struct v4l2_buffer buf;
	struct v4l2_plane planes[1];
	struct pollfd pfd;
	uint32_t type = OSD2D_TYPE;

	memset(&buf, 0, sizeof(buf));
	memset(planes, 0, sizeof(planes));
	buf.type = OSD2D_TYPE;
	buf.memory = V4L2_MEMORY_DMABUF;
	buf.index = index;
	buf.length = 1;
	buf.m.planes = planes;

	planes[0].m.fd = o->fds[index];
	planes[0].bytesused = o->frame_size;
	planes[0].length = o->frame_size;
	planes[0].data_offset = 0;

	if (osd2d_ioctl(o->fd, VIDIOC_QBUF, &buf) < 0) {
		osd2d_err(o, "QBUF of frame %u failed: %s", index,
			  strerror(errno));
		return -EIO;
	}

	if (!o->streaming) {
		if (osd2d_ioctl(o->fd, VIDIOC_STREAMON, &type) < 0) {
			osd2d_err(o, "STREAMON failed: %s", strerror(errno));
			return -EIO;
		}
		o->streaming = true;
	}

	memset(&pfd, 0, sizeof(pfd));
	pfd.fd = o->fd;
	pfd.events = POLLIN;

	if (poll(&pfd, 1, 1000) <= 0) {
		osd2d_err(o, "the engine did not finish within a second");
		return -ETIMEDOUT;
	}

	memset(&buf, 0, sizeof(buf));
	memset(planes, 0, sizeof(planes));
	buf.type = OSD2D_TYPE;
	buf.memory = V4L2_MEMORY_DMABUF;
	buf.length = 1;
	buf.m.planes = planes;

	if (osd2d_ioctl(o->fd, VIDIOC_DQBUF, &buf) < 0) {
		osd2d_err(o, "DQBUF failed: %s", strerror(errno));
		return -EIO;
	}

	o->passes++;

	return 0;
}

int osd2d_apply(struct osd2d *o, unsigned index, const struct osd2d_box *boxes,
		int nbox, const struct osd2d_patch *patches, int npatch)
{
	int pass, passes;
	int ret;
	unsigned long t0, t1, spent;

	if (o == NULL)
		return -EINVAL;

	if (index >= o->nframes) {
		osd2d_err(o, "frame %u was never attached (%u were)", index,
			  o->nframes);
		return -EINVAL;
	}

	if (nbox < 0)
		nbox = 0;
	if (npatch < 0)
		npatch = 0;

	/* Nothing to draw: the frame is left exactly as it came in, and the
	 * engine is not run over an unchanged picture */
	if (nbox == 0 && npatch == 0)
		return 0;

	/*
	 * Everything that did not fit goes round again. This works because
	 * each pass draws onto whatever the last one left behind - the engine
	 * reads and writes the same frame - so the passes add up instead of
	 * replacing one another.
	 */
	passes = 1;
	while (passes * OSD2D_MAX_BOX < nbox ||
	       passes * OSD2D_MAX_PATCH < npatch)
		passes++;

	/* t0 is the start of the whole call, t1 the stage being timed */
	t0 = osd2d_now();

	for (pass = 0; pass < passes; pass++) {
		int bfirst = pass * OSD2D_MAX_BOX;
		int pfirst = pass * OSD2D_MAX_PATCH;
		int bn = nbox - bfirst;
		int pn = npatch - pfirst;

		if (bn < 0)
			bn = 0;
		if (bn > OSD2D_MAX_BOX)
			bn = OSD2D_MAX_BOX;
		if (pn < 0)
			pn = 0;
		if (pn > OSD2D_MAX_PATCH)
			pn = OSD2D_MAX_PATCH;

		t1 = osd2d_now();

		ret = osd2d_set_patches(o, pn > 0 ? patches + pfirst : NULL,
					pn);
		if (ret < 0)
			return ret;

		/* Even with no boxes of its own: slots used on an earlier
		 * frame are still live and have to be written over */
		ret = osd2d_set_boxes(o, bn > 0 ? boxes + bfirst : NULL, bn);
		if (ret < 0)
			return ret;

		o->timing.ctrls += osd2d_now() - t1;

		t1 = osd2d_now();

		ret = osd2d_run(o, index);
		if (ret < 0)
			return ret;

		o->timing.engine += osd2d_now() - t1;
	}

	spent = osd2d_now() - t0;
	if (spent > o->timing.worst)
		o->timing.worst = spent;

	o->timing.frames++;

	return 1;
}

void osd2d_timing(const struct osd2d *o, struct osd2d_timing *t)
{
	if (t == NULL)
		return;

	if (o == NULL)
		memset(t, 0, sizeof(*t));
	else
		*t = o->timing;
}

const char *osd2d_dev(const struct osd2d *o)
{
	return o != NULL ? o->dev : "";
}

unsigned long osd2d_passes(const struct osd2d *o)
{
	return o != NULL ? o->passes : 0;
}

void osd2d_close(struct osd2d *o)
{
	if (o == NULL)
		return;

	if (o->fd >= 0) {
		if (o->streaming) {
			uint32_t type = OSD2D_TYPE;

			osd2d_ioctl(o->fd, VIDIOC_STREAMOFF, &type);
			o->streaming = false;
		}

		/* Nothing was allocated here and nothing was mapped: the
		 * frames belong to the caller, and closing the node is what
		 * lets go of the imports made of them */
		close(o->fd);
	}

	free(o);
}
