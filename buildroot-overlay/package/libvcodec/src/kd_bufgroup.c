/*
 * libvcodec - buffer group
 *
 * All four V4L2 queues of the codec (encoder input frames / output
 * bitstream, decoder input bitstream / output frames) share this one buffer
 * container. Zero copy and copy mode differ only in the external / internal
 * flag; the code above this layer is the same either way.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define _GNU_SOURCE

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

#include "kd_common.h"

/* ========================================================================
 * Creation
 * ======================================================================== */

static int kd_bufgroup_alloc(struct kd_bufgroup **out, unsigned count,
			     bool external)
{
	struct kd_bufgroup *grp;
	unsigned i;

	if (out == NULL || count == 0)
		return KD_ERR_INVAL;

	grp = calloc(1, sizeof(*grp));
	if (grp == NULL)
		return KD_ERR_NOMEM;

	grp->slots = calloc(count, sizeof(*grp->slots));
	if (grp->slots == NULL) {
		free(grp);
		return KD_ERR_NOMEM;
	}

	grp->external = external;
	grp->count = count;
	grp->fd = -1;
	grp->src_fd = -1;

	for (i = 0; i < count; i++) {
		unsigned p;

		for (p = 0; p < KD_MAX_PLANES; p++)
			grp->slots[i].plane[p].dmabuf_fd = -1;

		/* internal buffers are allocated by the library, so they
		 * count as committed from the start */
		grp->slots[i].committed = !external;
	}

	grp->ncommitted = external ? 0 : count;

	*out = grp;

	return KD_OK;
}

int kd_bufgroup_create_external(kd_bufgroup **grp, unsigned count)
{
	return kd_bufgroup_alloc(grp, count, true);
}

int kd_bufgroup_create_internal(kd_bufgroup **grp, unsigned count)
{
	return kd_bufgroup_alloc(grp, count, false);
}

/**
 * Frame size of a capture device, which is what VIDIOC_QBUF needs as the
 * length when the codec imports the dmabuf.
 */
static int kd_source_frame_size(int v4l2_fd, size_t *size_out)
{
	struct v4l2_format f;

	memset(&f, 0, sizeof(f));
	f.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	if (kd_ioctl(v4l2_fd, VIDIOC_G_FMT, &f) == 0) {
		*size_out = f.fmt.pix.sizeimage;
		return KD_OK;
	}

	if (errno != EINVAL) {
		KD_LOGE("G_FMT on the source device failed: %s",
			strerror(errno));
		return kd_errno_to_err(errno);
	}

	/* Multi-planar devices would also need planes[] in every QBUF, which
	 * this path does not do. Say so instead of failing later with an
	 * obscure -EINVAL. */
	memset(&f, 0, sizeof(f));
	f.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;

	if (kd_ioctl(v4l2_fd, VIDIOC_G_FMT, &f) == 0) {
		KD_LOGE("the source is a multi-planar device, which this "
			"helper does not handle; export the buffers yourself "
			"and use kd_bufgroup_create_external()");
		return KD_ERR_INVAL;
	}

	KD_LOGE("the source device has no capture format: %s",
		strerror(errno));

	return KD_ERR_INVAL;
}

int kd_bufgroup_create_from_v4l2(kd_bufgroup **grp, int v4l2_fd,
				 unsigned count)
{
	struct kd_bufgroup *g = NULL;
	size_t size = 0;
	unsigned i;
	int ret;

	if (grp == NULL || v4l2_fd < 0 || count == 0)
		return KD_ERR_INVAL;

	ret = kd_source_frame_size(v4l2_fd, &size);
	if (ret != KD_OK)
		return ret;

	if (size == 0) {
		KD_LOGE("the source device reports sizeimage=0; set a format "
			"on it before exporting its buffers");
		return KD_ERR_INVAL;
	}

	ret = kd_bufgroup_alloc(&g, count, true);
	if (ret != KD_OK)
		return ret;

	g->src_fd = v4l2_fd;
	g->src_type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	g->owns_fds = true;

	for (i = 0; i < count; i++) {
		struct v4l2_exportbuffer exp;

		memset(&exp, 0, sizeof(exp));
		exp.type = g->src_type;
		exp.index = i;
		exp.plane = 0;
		exp.flags = O_CLOEXEC | O_RDWR;

		if (kd_ioctl(v4l2_fd, VIDIOC_EXPBUF, &exp) < 0) {
			KD_LOGE("EXPBUF[%u] failed: %s; the device must have "
				"done REQBUFS(MMAP) for at least %u buffers",
				i, strerror(errno), count);
			ret = kd_errno_to_err(errno);
			goto fail;
		}

		g->slots[i].plane[0].dmabuf_fd = exp.fd;
		g->slots[i].plane[0].size = size;
		g->slots[i].nplanes = 1;
		g->slots[i].size = size;
		g->slots[i].committed = true;
		g->ncommitted++;
	}

	KD_LOGI("exported %u buffers of %zu bytes from the source device",
		count, size);

	*grp = g;

	return KD_OK;

fail:
	kd_bufgroup_destroy(g);

	return ret;
}

int kd_bufgroup_commit(kd_bufgroup *grp, unsigned index, int dmabuf_fd,
		       size_t size)
{
	return kd_bufgroup_commit_planes(grp, index, &dmabuf_fd, &size, 1);
}

int kd_bufgroup_commit_planes(kd_bufgroup *grp, unsigned index,
			      const int *dmabuf_fds, const size_t *sizes,
			      unsigned nplanes)
{
	size_t total = 0;
	unsigned p;

	if (grp == NULL || index >= grp->count)
		return KD_ERR_INVAL;

	if (dmabuf_fds == NULL || sizes == NULL || nplanes == 0 ||
	    nplanes > KD_MAX_PLANES) {
		KD_LOGE("commit[%u]: %u planes is not a usable count", index,
			nplanes);
		return KD_ERR_INVAL;
	}

	if (!grp->external) {
		KD_LOGE("an internal group does not need commit");
		return KD_ERR_STATE;
	}

	if (grp->owns_fds) {
		KD_LOGE("this group was built by kd_bufgroup_create_from_v4l2 "
			"and is already filled in");
		return KD_ERR_STATE;
	}

	if (grp->attached) {
		KD_LOGE("the group is attached to a codec and can no longer "
			"be changed");
		return KD_ERR_STATE;
	}

	for (p = 0; p < nplanes; p++) {
		if (dmabuf_fds[p] < 0 || sizes[p] == 0) {
			/* length is mandatory when V4L2 imports a dmabuf, and
			 * a zero makes QBUF fail with -EINVAL. That is the
			 * single most common zero copy mistake, so stop it
			 * here. */
			KD_LOGE("commit[%u] plane %u: fd=%d size=%zu is not "
				"usable, size must be the real byte count",
				index, p, dmabuf_fds[p], sizes[p]);
			return KD_ERR_INVAL;
		}
		total += sizes[p];
	}

	if (!grp->slots[index].committed)
		grp->ncommitted++;

	for (p = 0; p < nplanes; p++) {
		grp->slots[index].plane[p].dmabuf_fd = dmabuf_fds[p];
		grp->slots[index].plane[p].size = sizes[p];
		/* An allocation per plane, so each one starts at the top of
		 * its own. Only attaching ever sets this to anything else. */
		grp->slots[index].plane[p].offset = 0;
	}

	grp->slots[index].nplanes = nplanes;
	grp->slots[index].size = total;
	grp->slots[index].committed = true;

	return KD_OK;
}

void *kd_bufgroup_map(kd_bufgroup *grp, unsigned index, size_t *size)
{
	return kd_bufgroup_plane(grp, index, 0, size);
}

void *kd_bufgroup_plane(const struct kd_bufgroup *grp, unsigned index,
			unsigned p, size_t *size)
{
	if (grp == NULL || index >= grp->count || p >= KD_MAX_PLANES)
		return NULL;

	if (size != NULL)
		*size = grp->slots[index].plane[p].size;

	return grp->slots[index].plane[p].addr;
}

unsigned kd_bufgroup_count(const kd_bufgroup *grp)
{
	return grp != NULL ? grp->count : 0;
}

void kd_bufgroup_destroy(kd_bufgroup *grp)
{
	unsigned i;

	if (grp == NULL)
		return;

	if (grp->attached)
		KD_LOGW("the group is still attached to a codec; destroy the "
			"codec first, then the group");

	kd_bufgroup_detach(grp);

	if (grp->owns_fds) {
		for (i = 0; i < grp->count; i++) {
			unsigned p;

			for (p = 0; p < KD_MAX_PLANES; p++) {
				if (grp->slots[i].plane[p].dmabuf_fd >= 0)
					close(grp->slots[i].plane[p].dmabuf_fd);
			}
		}
	}

	free(grp->slots);
	free(grp);
}

/* ========================================================================
 * Binding to a V4L2 queue
 * ======================================================================== */

/**
 * How many planes the driver wants on this queue, and how big each one is.
 * The buffer type alone does not say: a multi-planar queue carrying NV12 is
 * two planes, the same queue carrying YUYV is one. Ask with G_FMT so the
 * answer always matches the format that was just set.
 *
 * The sizes matter as much as the count. They are what a single committed
 * allocation is cut up along when it has to stand in for several planes -
 * see the splitting in kd_bufgroup_attach().
 */
static int kd_bufgroup_probe_planes(struct kd_bufgroup *grp, int fd,
				    uint32_t type)
{
	struct v4l2_format f;
	unsigned p;

	grp->mplane = V4L2_TYPE_IS_MULTIPLANAR(type);

	memset(grp->plane_size, 0, sizeof(grp->plane_size));

	memset(&f, 0, sizeof(f));
	f.type = type;

	if (!grp->mplane) {
		/* A single-planar queue is one plane whatever the format, so
		 * the size is a nicety here and not worth failing over. */
		grp->nplanes = 1;

		if (kd_ioctl(fd, VIDIOC_G_FMT, &f) == 0)
			grp->plane_size[0] = f.fmt.pix.sizeimage;

		return KD_OK;
	}

	if (kd_ioctl(fd, VIDIOC_G_FMT, &f) < 0) {
		KD_LOGE("G_FMT(type=%u) failed while counting planes: %s", type,
			strerror(errno));
		return kd_errno_to_err(errno);
	}

	grp->nplanes = f.fmt.pix_mp.num_planes;

	if (grp->nplanes == 0 || grp->nplanes > KD_MAX_PLANES) {
		KD_LOGE("the driver reports %u planes, this library handles up "
			"to %d",
			grp->nplanes, KD_MAX_PLANES);
		return KD_ERR_INVAL;
	}

	for (p = 0; p < grp->nplanes; p++)
		grp->plane_size[p] = f.fmt.pix_mp.plane_fmt[p].sizeimage;

	return KD_OK;
}

/**
 * One allocation standing in for several planes.
 *
 * A camera hands out one dmabuf per frame with the chroma right behind the
 * luma, while the codec describes that same NV12 frame as two planes and
 * imports a dmabuf per plane. The two meet through data_offset: name the
 * same fd on every plane and say how far in each one starts. Cut the
 * committed allocation along the sizes the driver just reported.
 *
 * Needs a kernel that carries data_offset through to the hardware - see
 * buildroot-overlay/linux/0043-media-canaan-vpu-honour-per-plane-data_offset
 * -on-inp.patch. Without it the chroma is read from the address of the luma
 * and the picture comes out green.
 */
static int kd_bufgroup_split_slot(struct kd_bufgroup *grp, unsigned index)
{
	struct kd_bufslot *slot = &grp->slots[index];
	size_t need = 0;
	size_t off = 0;
	unsigned p;

	for (p = 0; p < grp->nplanes; p++)
		need += grp->plane_size[p];

	if (slot->size < need) {
		KD_LOGE("buffer[%u] is one dmabuf of %zu bytes, too small to "
			"hold the %u planes of %zu bytes the driver asks for",
			index, slot->size, grp->nplanes, need);
		return KD_ERR_INVAL;
	}

	for (p = 0; p < grp->nplanes; p++) {
		slot->plane[p].dmabuf_fd = slot->plane[0].dmabuf_fd;
		slot->plane[p].size = grp->plane_size[p];
		slot->plane[p].offset = off;
		off += grp->plane_size[p];
	}

	slot->nplanes = grp->nplanes;
	slot->split = true;

	return KD_OK;
}

static void kd_bufgroup_unsplit_slot(struct kd_bufgroup *grp, unsigned index)
{
	struct kd_bufslot *slot = &grp->slots[index];
	unsigned p;

	if (!slot->split)
		return;

	/* The aliases must go before anything closes the fds, or the one
	 * dmabuf behind them would be closed once per plane. */
	for (p = 1; p < KD_MAX_PLANES; p++) {
		slot->plane[p].dmabuf_fd = -1;
		slot->plane[p].size = 0;
		slot->plane[p].offset = 0;
	}

	slot->plane[0].size = slot->size;
	slot->plane[0].offset = 0;
	slot->nplanes = 1;
	slot->split = false;
}

int kd_bufgroup_attach(struct kd_bufgroup *grp, int fd, uint32_t type,
		       size_t min_size)
{
	struct v4l2_requestbuffers req;
	unsigned i;
	int ret;

	if (grp == NULL)
		return KD_ERR_INVAL;

	if (grp->attached)
		return KD_ERR_STATE;

	if (grp->external && grp->ncommitted != grp->count) {
		KD_LOGE("only %u of %u slots of the external group are "
			"committed",
			grp->ncommitted, grp->count);
		return KD_ERR_STATE;
	}

	ret = kd_bufgroup_probe_planes(grp, fd, type);
	if (ret != KD_OK)
		return ret;

	/*
	 * V4L2 imports one dmabuf per plane, and a format the driver reports
	 * as several - NV12 and NV21 are two planes, I420 three - therefore
	 * wants several fds. An application that has one allocation per plane
	 * commits them with kd_bufgroup_commit_planes() and there is nothing
	 * to do here.
	 *
	 * A camera does not work that way: a frame is one allocation with the
	 * chroma behind the luma, and one fd is all it can export. Cut that
	 * one fd into the planes the driver asked for, each at its own
	 * offset, rather than refusing the only shape the capture side can
	 * produce.
	 */
	if (grp->external) {
		for (i = 0; i < grp->count; i++) {
			if (grp->slots[i].nplanes == grp->nplanes)
				continue;

			if (grp->slots[i].nplanes != 1) {
				KD_LOGE("buffer[%u] was committed as %u planes "
					"but the driver reports this format as "
					"%u; commit one per plane, or one "
					"allocation holding them all",
					i, grp->slots[i].nplanes,
					grp->nplanes);
				return KD_ERR_INVAL;
			}

			ret = kd_bufgroup_split_slot(grp, i);
			if (ret != KD_OK)
				goto unsplit;
		}
	}

	grp->memory = grp->external ? V4L2_MEMORY_DMABUF : V4L2_MEMORY_MMAP;

	memset(&req, 0, sizeof(req));
	req.count = grp->count;
	req.type = type;
	req.memory = grp->memory;

	if (kd_ioctl(fd, VIDIOC_REQBUFS, &req) < 0) {
		KD_LOGE("REQBUFS(count=%u type=%u memory=%s) failed: %s",
			grp->count, type,
			grp->external ? "DMABUF" : "MMAP", strerror(errno));
		ret = kd_errno_to_err(errno);
		goto unsplit;
	}

	if (req.count < grp->count) {
		KD_LOGE("asked for %u buffers, the driver granted %u",
			grp->count, req.count);
		ret = KD_ERR_NOMEM;
		goto unsplit;
	}

	grp->fd = fd;
	grp->type = type;
	grp->attached = true;

	if (grp->external) {
		/* The memory already belongs to the application, so all
		 * there is to do is check the sizes */
		for (i = 0; i < grp->count; i++) {
			if (grp->slots[i].size >= min_size)
				continue;

			KD_LOGE("buffer[%u] is %zu bytes, the codec needs %zu",
				i, grp->slots[i].size, min_size);
			ret = KD_ERR_INVAL;
			goto fail;
		}

		return KD_OK;
	}

	/* internal: QUERYBUF + mmap, one mapping per plane */
	for (i = 0; i < grp->count; i++) {
		struct v4l2_buffer buf;
		struct v4l2_plane planes[KD_MAX_PLANES];
		size_t total = 0;
		unsigned p;

		memset(&buf, 0, sizeof(buf));
		memset(planes, 0, sizeof(planes));
		buf.type = type;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index = i;

		if (grp->mplane) {
			buf.m.planes = planes;
			buf.length = grp->nplanes;
		}

		if (kd_ioctl(fd, VIDIOC_QUERYBUF, &buf) < 0) {
			KD_LOGE("QUERYBUF[%u] failed: %s", i, strerror(errno));
			ret = kd_errno_to_err(errno);
			goto fail;
		}

		for (p = 0; p < grp->nplanes; p++) {
			size_t len;
			off_t off;
			void *addr;

			if (grp->mplane) {
				len = planes[p].length;
				off = (off_t)planes[p].m.mem_offset;
			} else {
				len = buf.length;
				off = (off_t)buf.m.offset;
			}

			addr = mmap(NULL, len, PROT_READ | PROT_WRITE,
				    MAP_SHARED, fd, off);
			if (addr == MAP_FAILED) {
				KD_LOGE("mmap of buffer[%u] plane %u (%zu "
					"bytes) failed: %s",
					i, p, len, strerror(errno));
				ret = kd_errno_to_err(errno);
				goto fail;
			}

			grp->slots[i].plane[p].addr = addr;
			grp->slots[i].plane[p].size = len;
			total += len;
		}

		grp->slots[i].size = total;

		if (total < min_size)
			KD_LOGW("buffer[%u] is %zu bytes over %u plane(s), less "
				"than the %zu expected",
				i, total, grp->nplanes, min_size);
	}

	return KD_OK;

fail:
	kd_bufgroup_detach(grp);

	return ret;

unsplit:
	for (i = 0; i < grp->count; i++)
		kd_bufgroup_unsplit_slot(grp, i);

	return ret;
}

void kd_bufgroup_detach(struct kd_bufgroup *grp)
{
	struct v4l2_requestbuffers req;
	unsigned i;

	if (grp == NULL)
		return;

	/* Before anything else, and whether or not the group ever got as far
	 * as being attached: while a slot is split, several of its planes
	 * name the one dmabuf, and destroy() would close it once per plane. */
	for (i = 0; i < grp->count; i++)
		kd_bufgroup_unsplit_slot(grp, i);

	if (!grp->attached)
		return;

	for (i = 0; i < grp->count; i++) {
		unsigned p;

		for (p = 0; p < KD_MAX_PLANES; p++) {
			if (grp->slots[i].plane[p].addr == NULL)
				continue;

			munmap(grp->slots[i].plane[p].addr,
			       grp->slots[i].plane[p].size);
			grp->slots[i].plane[p].addr = NULL;
			grp->slots[i].plane[p].size = 0;
		}

		grp->slots[i].queued = false;
	}

	memset(&req, 0, sizeof(req));
	req.count = 0;
	req.type = grp->type;
	req.memory = grp->memory;
	kd_ioctl(grp->fd, VIDIOC_REQBUFS, &req);

	grp->attached = false;
	grp->fd = -1;
}

/* ========================================================================
 * Queue operations
 * ======================================================================== */

int kd_bufgroup_qbuf(struct kd_bufgroup *grp, unsigned index, size_t bytesused,
		     int64_t pts, uint32_t v4l2_flags)
{
	struct v4l2_buffer buf;
	struct v4l2_plane planes[KD_MAX_PLANES];

	if (grp == NULL || !grp->attached || index >= grp->count)
		return KD_ERR_INVAL;

	if (grp->slots[index].queued) {
		KD_LOGE("buffer[%u] is already queued", index);
		return KD_ERR_STATE;
	}

	memset(&buf, 0, sizeof(buf));
	memset(planes, 0, sizeof(planes));
	buf.type = grp->type;
	buf.memory = grp->memory;
	buf.index = index;
	buf.flags = v4l2_flags;
	kd_pts_to_tv(pts, &buf.timestamp);

	if (grp->mplane) {
		const struct kd_bufslot *slot = &grp->slots[index];
		size_t left = bytesused;
		unsigned p;

		buf.m.planes = planes;
		buf.length = grp->nplanes;

		for (p = 0; p < grp->nplanes; p++) {
			size_t off = slot->plane[p].offset;
			size_t len = slot->plane[p].size;
			size_t share;

			if (grp->external)
				planes[p].m.fd = slot->plane[p].dmabuf_fd;

			/*
			 * bytesused is the whole frame, but the driver checks
			 * it per plane (__verify_length() rejects a plane that
			 * claims more bytes than it holds). So hand each plane
			 * its own share instead of putting the total on the
			 * first one, which is what NV12 used to trip over.
			 */
			share = left < len ? left : len;

			/*
			 * A split slot has every plane naming the one dmabuf,
			 * so length has to span the whole allocation - V4L2
			 * reads it as the size of the memory being imported,
			 * not as the size of this plane - and data_offset says
			 * where the plane begins. bytesused counts from the
			 * start of the buffer and therefore includes the
			 * offset, which is also what keeps it above
			 * data_offset: videobuf2 rejects a plane whose data
			 * starts at or past the end of its own payload.
			 */
			planes[p].length = (uint32_t)(off + len);
			planes[p].data_offset = (uint32_t)off;
			planes[p].bytesused = (uint32_t)(off + share);

			left -= share;
		}
	} else {
		buf.bytesused = (uint32_t)bytesused;

		if (grp->external) {
			buf.m.fd = grp->slots[index].plane[0].dmabuf_fd;
			/* length is mandatory when importing a dmabuf */
			buf.length = (uint32_t)grp->slots[index].plane[0].size;
		}
	}

	if (kd_ioctl(grp->fd, VIDIOC_QBUF, &buf) < 0) {
		int err = errno;

		if (err != EAGAIN) {
			KD_LOGE("QBUF[%u] (type=%u bytesused=%zu) failed: %s",
				index, grp->type, bytesused, strerror(err));

			/* The driver checks the byte count plane by plane, so
			 * the totals above are not enough to see what it did
			 * not like. Print the split it was given. */
			if (grp->mplane) {
				unsigned p;

				for (p = 0; p < grp->nplanes; p++)
					KD_LOGE("  plane %u: bytesused=%u of "
						"length=%u",
						p, planes[p].bytesused,
						planes[p].length);
			}
		}

		return kd_errno_to_err(err);
	}

	grp->slots[index].queued = true;

	return KD_OK;
}

int kd_bufgroup_dqbuf(struct kd_bufgroup *grp, struct kd_dqinfo *info)
{
	struct v4l2_buffer buf;
	struct v4l2_plane planes[KD_MAX_PLANES];

	if (grp == NULL || !grp->attached || info == NULL)
		return KD_ERR_INVAL;

	memset(&buf, 0, sizeof(buf));
	memset(planes, 0, sizeof(planes));
	buf.type = grp->type;
	buf.memory = grp->memory;

	if (grp->mplane) {
		buf.m.planes = planes;
		buf.length = grp->nplanes;
	}

	if (kd_ioctl(grp->fd, VIDIOC_DQBUF, &buf) < 0) {
		int err = errno;

		if (err != EAGAIN && err != EPIPE)
			KD_LOGE("DQBUF(type=%u) failed: %s", grp->type,
				strerror(err));

		return kd_errno_to_err(err);
	}

	if (buf.index >= grp->count) {
		KD_LOGE("DQBUF returned index=%u, out of range for %u buffers",
			buf.index, grp->count);
		return KD_ERR_IO;
	}

	grp->slots[buf.index].queued = false;

	info->index = buf.index;

	if (grp->mplane) {
		/* On a multi-planar queue the byte count lives in the planes,
		 * one per plane, so the frame is their sum. A decoded NV12
		 * frame fills two of them; taking only the first would report
		 * the luma size and lose the chroma. An empty buffer still
		 * sums to 0, which is what signals the end of the stream. */
		unsigned p;

		info->bytesused = 0;
		for (p = 0; p < grp->nplanes; p++) {
			uint32_t used = planes[p].bytesused;

			/* V4L2 counts data_offset inside bytesused, so a
			 * plane that is an offset into a shared allocation
			 * would otherwise have the plane in front of it
			 * counted again as its own payload. */
			if (used > planes[p].data_offset)
				used -= planes[p].data_offset;

			info->bytesused += used;
		}
	} else {
		info->bytesused = buf.bytesused;
	}
	info->pts = kd_tv_to_pts(&buf.timestamp);
	info->flags = buf.flags;
	info->reserved2 = buf.reserved2;

	return KD_OK;
}

int kd_bufgroup_queue_all(struct kd_bufgroup *grp)
{
	unsigned i;

	if (grp == NULL || !grp->attached)
		return KD_ERR_INVAL;

	for (i = 0; i < grp->count; i++) {
		int ret;

		if (grp->slots[i].queued)
			continue;

		ret = kd_bufgroup_qbuf(grp, i, 0, 0, 0);
		if (ret != KD_OK)
			return ret;
	}

	return KD_OK;
}

void kd_bufgroup_reset_queued(struct kd_bufgroup *grp)
{
	unsigned i;

	if (grp == NULL)
		return;

	for (i = 0; i < grp->count; i++)
		grp->slots[i].queued = false;
}

int kd_bufgroup_requeue_source(struct kd_bufgroup *grp, unsigned index)
{
	struct v4l2_buffer buf;

	if (grp == NULL || grp->src_fd < 0 || index >= grp->count)
		return KD_ERR_INVAL;

	memset(&buf, 0, sizeof(buf));
	buf.type = grp->src_type;
	buf.memory = V4L2_MEMORY_MMAP;
	buf.index = index;

	if (kd_ioctl(grp->src_fd, VIDIOC_QBUF, &buf) < 0) {
		KD_LOGE("handing buffer[%u] back to the capture device "
			"failed: %s",
			index, strerror(errno));
		return kd_errno_to_err(errno);
	}

	return KD_OK;
}
