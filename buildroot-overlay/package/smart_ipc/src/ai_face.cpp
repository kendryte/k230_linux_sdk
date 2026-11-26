/*
 * Face detection on a camera channel of its own - the implementation
 *
 * See ai_face.h for what this is for. The shape of the file:
 *
 *   ai_face_start()  opens the V4L2 node and starts the thread
 *   ai_face_thread() loads the model, then loops: dqbuf -> KPU -> results
 *   ai_face_draw()   called from the main loop, paints the last results
 *
 * The only thing crossing between the two threads is a vector of boxes behind
 * a mutex, plus a counter the caller watches to know when it changed. The
 * camera buffers the encoder and the screen use are never touched: this opens
 * a node of its own, and the ISP gives a sensor several.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "ai_face.h"

#include <atomic>
#include <algorithm>
#include <climits>
#include <cstdio>
#include <cstring>
#include <exception>
#include <future>
#include <memory>
#include <mutex>
#include <new>
#include <thread>
#include <vector>

#include <poll.h>
#include <linux/videodev2.h>

/* The headers under ai/ are a verbatim copy of the face_detect package's, so
 * they can still be diffed against it when it moves upstream. Tidying their
 * warnings would break that, so they are silenced on the way in instead. */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreorder"
#include "ai/face_detection.h"
#pragma GCC diagnostic pop

extern "C" {
#include "cam_capture.h"
#include "thead.h"
}

/*
 * One colour per landmark, not per face: the same five colours mark the same
 * five features on every face, so the eyes are always blue and magenta and
 * the nose always cyan however many people are in shot. Defined in
 * ai/face_detection.cc, and the order there is B, G, R, A - which is the byte
 * order of an ARGB8888 framebuffer on a little endian machine, so they go
 * straight in.
 *
 * The box itself is white, the way the face_detect demo draws it.
 */
extern cv::Scalar color_list_for_osd_det[];
#define OSD_LANDMARKS 5
#define OSD_BOX_COLOUR cv::Scalar(255, 255, 255, 255)
#define OSD_BOX_RGB 0xffffffu /* the same white, as osd2d wants it */

/* How many faces ai_face_shapes() decorates when the caller has no opinion */
#define OSD_SHAPE_FACES 32

/*
 * Patch sizes are rounded up to this.
 *
 * The 2D engine keeps a kernel buffer per patch and throws it away and takes
 * a new one whenever the size changes. A face is never still, so its patch
 * would otherwise be a slightly different size every frame and the kernel
 * would spend its time reallocating contiguous memory. Rounded up, the size
 * only changes when the face crosses a step.
 */
#define OSD_SHAPE_STEP 32

/* Below this a face's five dots are one smudge; the box alone says more */
#define OSD_SHAPE_MIN 12

/* BGR888 planar: three full-size planes, B then G then R. What the ISP's SP2
 * channel can hand out and what the network wants to see. */
#define AI_PIXELFORMAT v4l2_fourcc('B', 'G', '3', 'P')
#define AI_CHANNELS 3

struct ai_face {
	struct cam_capture cam;
	struct ai_face_cfg cfg;

	std::thread thread;
	std::atomic<bool> stop;

	std::atomic<unsigned long> frames; /* what the KPU got through */
	std::atomic<unsigned long> gen; /* bumped on every new result */
	std::atomic<unsigned> count; /* faces in the latest result */

	std::mutex lock;
	std::vector<FaceDetectionInfo> results; /* guarded by lock */

	/* Scratch for ai_face_shapes(). Touched by the calling thread only,
	 * and kept between calls so a steady scene stops allocating. */
	std::vector<FaceDetectionInfo> shown;
	std::vector<struct osd2d_box> boxes;
	std::vector<struct osd2d_patch> patches;
	std::vector<std::vector<uint32_t> > pixels;

	ai_face() : stop(false), frames(0), gen(0), count(0)
	{
		memset(&cam, 0, sizeof(cam));
		cam.fd = -1;
		memset(&cfg, 0, sizeof(cfg));
	}
};

void ai_face_default_cfg(struct ai_face_cfg *cfg)
{
	if (cfg == NULL)
		return;

	memset(cfg, 0, sizeof(*cfg));
	cfg->dev = "/dev/video3";
	/* This package's own copy, installed next to the binary by its
	 * CMakeLists. Not the face_detect demo's: that one can be turned off
	 * in the config or move its files, and smart_ipc would stop finding
	 * a model through no fault of its own.
	 */
	cfg->kmodel = "/usr/share/smart_ipc/face_detection_320.kmodel";
	cfg->width = 1280;
	cfg->height = 720;
	cfg->buffers = 3;
	cfg->obj_thresh = 0.6f;
	cfg->nms_thresh = 0.2f;
	cfg->debug = 0;
}

/*
 * The thread. Everything nncase owns is created and destroyed here, on the one
 * thread that uses it, which is also what the vendor demo does.
 *
 * @ready carries the outcome of loading the model back to ai_face_start(), so
 * a missing or broken kmodel is an error the caller can report rather than a
 * thread that quietly does nothing.
 */
static void ai_face_thread(struct ai_face *a, std::promise<bool> ready)
{
	const size_t isp_size =
		(size_t)a->cam.width * a->cam.height * AI_CHANNELS;
	std::unique_ptr<FaceDetection> fd;
	runtime_tensor in_tensor;

	try {
		fd.reset(new FaceDetection(
			a->cfg.kmodel, a->cfg.obj_thresh, a->cfg.nms_thresh,
			{ AI_CHANNELS, a->cam.height, a->cam.width },
			a->cfg.debug));

		/* One input tensor, reused for every frame: the frame is
		 * copied into it and ai2d scales it down from there. */
		dims_t in_shape{ 1, AI_CHANNELS, a->cam.height, a->cam.width };
		in_tensor =
			hrt::create(typecode_t::dt_uint8, in_shape,
				    hrt::pool_shared)
				.expect("creating the AI input tensor failed");
	} catch (const std::exception &e) {
		fprintf(stderr, "loading %s failed: %s\n", a->cfg.kmodel,
			e.what());
		ready.set_value(false);
		return;
	} catch (...) {
		fprintf(stderr, "loading %s failed\n", a->cfg.kmodel);
		ready.set_value(false);
		return;
	}

	ready.set_value(true);

	while (!a->stop.load(std::memory_order_relaxed)) {
		struct pollfd pfd;
		unsigned index;
		int64_t pts;

		pfd.fd = a->cam.fd;
		pfd.events = POLLIN;
		pfd.revents = 0;

		/*
		 * A timeout rather than a wait, so stopping does not have to
		 * sit through one more frame. 200ms is well over a frame
		 * interval, so this costs nothing while frames are flowing.
		 */
		if (poll(&pfd, 1, 200) <= 0)
			continue;

		if (cam_dqbuf(&a->cam, &index, &pts) < 0)
			continue;

		if (a->cam.bufs[index].addr != NULL) {
			std::vector<FaceDetectionInfo> out;

			/*
			 * One copy per frame. The KPU could be pointed
			 * straight at the camera buffer - it is contiguous and
			 * its physical address is known - but that means
			 * importing the dmabuf into ai2d, which is a bigger
			 * change than this is worth until the copy actually
			 * shows up in a profile.
			 */
			auto buf = in_tensor.impl()
					   ->to_host()
					   .unwrap()
					   ->buffer()
					   .as_host()
					   .unwrap()
					   .map(map_access_::map_write)
					   .unwrap()
					   .buffer();
			memcpy(reinterpret_cast<char *>(buf.data()),
			       a->cam.bufs[index].addr,
			       std::min(isp_size, a->cam.bufs[index].size));
			hrt::sync(in_tensor, sync_op_t::sync_write_back, true)
				.expect("syncing the AI input tensor failed");

			fd->pre_process(in_tensor);
			fd->inference();
			fd->post_process({ a->cam.width, a->cam.height }, out);

			a->count.store((unsigned)out.size(),
				       std::memory_order_relaxed);
			{
				std::lock_guard<std::mutex> guard(a->lock);
				a->results.swap(out);
			}
			a->frames.fetch_add(1, std::memory_order_relaxed);
			a->gen.fetch_add(1, std::memory_order_release);
		}

		cam_qbuf(&a->cam, index);
	}

	/* The model and the tensor go first, then the pool they came out of */
	fd.reset();
	in_tensor = runtime_tensor();
	shrink_memory_pool();
}

struct ai_face *ai_face_start(const struct ai_face_cfg *cfg)
{
	struct ai_face_cfg use;
	struct ai_face *a;
	std::promise<bool> ready;
	std::future<bool> ok;
	int ret;

	ai_face_default_cfg(&use);
	if (cfg != NULL) {
		if (cfg->dev != NULL)
			use.dev = cfg->dev;
		if (cfg->kmodel != NULL)
			use.kmodel = cfg->kmodel;
		if (cfg->width != 0)
			use.width = cfg->width;
		if (cfg->height != 0)
			use.height = cfg->height;
		if (cfg->buffers != 0)
			use.buffers = cfg->buffers;
		if (cfg->obj_thresh > 0.0f)
			use.obj_thresh = cfg->obj_thresh;
		if (cfg->nms_thresh > 0.0f)
			use.nms_thresh = cfg->nms_thresh;
		use.debug = cfg->debug;
	}

	a = new (std::nothrow) ai_face();
	if (a == NULL) {
		fprintf(stderr, "out of memory starting face detection\n");
		return NULL;
	}
	a->cfg = use;

	/*
	 * Mapped, because the frame is copied into the KPU's tensor. The
	 * sensor mode is deliberately left alone: whoever opened the first
	 * channel already pinned it, and asking again from here would only
	 * fight them over it.
	 */
	ret = cam_open(&a->cam, use.dev, NULL, use.width, use.height,
		       AI_PIXELFORMAT, use.buffers, true);
	if (ret < 0) {
		fprintf(stderr,
			"opening %s for face detection failed. The AI wants a "
			"channel of its own, so this has to be a video node "
			"nothing else in the demo is using.\n",
			use.dev);
		delete a;
		return NULL;
	}

	/*
	 * The network is fed whole planes, so padding between lines would
	 * shear the picture. Say so rather than detect faces in a mess.
	 */
	if (a->cam.stride != a->cam.width)
		fprintf(stderr,
			"warning: %s pads its lines to %u bytes for a %u pixel "
			"picture. The AI reads the planes unpadded, so the "
			"detections will be off. Pick an --ai-size the driver "
			"takes unpadded.\n",
			use.dev, a->cam.stride, a->cam.width);

	ret = cam_start(&a->cam);
	if (ret < 0) {
		fprintf(stderr, "starting %s failed\n", use.dev);
		cam_close(&a->cam);
		delete a;
		return NULL;
	}

	ok = ready.get_future();
	try {
		a->thread = std::thread(ai_face_thread, a, std::move(ready));
	} catch (const std::exception &e) {
		fprintf(stderr,
			"starting the face detection thread failed: %s\n",
			e.what());
		cam_close(&a->cam);
		delete a;
		return NULL;
	}

	/* Loading a kmodel takes a moment; wait for it, so a bad path is an
	 * error at startup rather than a silent lack of boxes */
	if (!ok.get()) {
		a->stop.store(true);
		a->thread.join();
		cam_close(&a->cam);
		delete a;
		return NULL;
	}

	return a;
}

void ai_face_stop(struct ai_face *a)
{
	if (a == NULL)
		return;

	a->stop.store(true);
	if (a->thread.joinable())
		a->thread.join();

	cam_close(&a->cam);
	delete a;
}

unsigned long ai_face_generation(const struct ai_face *a)
{
	if (a == NULL)
		return 0;

	return a->gen.load(std::memory_order_acquire);
}

unsigned long ai_face_frames(const struct ai_face *a)
{
	if (a == NULL)
		return 0;

	return a->frames.load(std::memory_order_relaxed);
}

void ai_face_flush(void *addr, unsigned long size)
{
	if (addr == NULL)
		return;

	thead_csi_dcache_clean_invalid_range(addr, size);
}

unsigned ai_face_count(const struct ai_face *a)
{
	if (a == NULL)
		return 0;

	return a->count.load(std::memory_order_relaxed);
}

int ai_face_draw(struct ai_face *a, void *argb, unsigned w, unsigned h,
		 unsigned stride, enum drm_rotation rot)
{
	std::vector<FaceDetectionInfo> shown;
	float src_w, src_h;
	int drawn = 0;

	if (a == NULL || argb == NULL || w == 0 || h == 0)
		return -1;

	/*
	 * @w and @h describe the part of the overlay the camera picture sits
	 * behind, which is not always the whole plane: when the picture is
	 * centred on a larger panel, the caller points @argb at its top left
	 * corner and passes the picture's size. Nothing is ever drawn outside
	 * that rectangle, so nothing outside it needs clearing either.
	 */
	cv::Mat canvas(h, w, CV_8UC4, argb, stride);

	{
		std::lock_guard<std::mutex> guard(a->lock);
		shown = a->results;
	}

	canvas.setTo(cv::Scalar(0, 0, 0, 0));

	src_w = (float)a->cam.width;
	src_h = (float)a->cam.height;

	/*
	 * Turn one point from the AI channel's frame into the panel's own
	 * coordinates. Everything goes through fractions of the picture, so
	 * the two channels being different sizes costs nothing extra.
	 *
	 * rotation_90 is, on screen, a quarter turn clockwise - that is what
	 * the video plane does to the camera picture, so the boxes have to
	 * make the same turn to stay on the faces.
	 */
	auto to_panel = [&](float x, float y, cv::Point2f &out) {
		float u = x / src_w;
		float v = y / src_h;
		float pu, pv;

		switch (rot) {
		case rotation_90:
			pu = 1.0f - v;
			pv = u;
			break;
		case rotation_180:
			pu = 1.0f - u;
			pv = 1.0f - v;
			break;
		case rotation_270:
			pu = v;
			pv = 1.0f - u;
			break;
		default:
			pu = u;
			pv = v;
			break;
		}

		out.x = pu * (float)w;
		out.y = pv * (float)h;
	};

	/*
	 * Thick enough to see from across the room, thin enough not to cover
	 * the face. The vendor hard coded 6 pixels for a 1080p screen; this
	 * is the same number worked out from the size actually in use, so a
	 * 480x800 panel gets a proportionally thinner line instead of a slab.
	 */
	int line = std::max(2, (int)h / 180);
	int dot = std::max(2, (int)h / 135);

	for (size_t i = 0; i < shown.size(); i++) {
		const FaceDetectionInfo &f = shown[i];
		cv::Point2f c1, c2;
		cv::Rect box;

		/*
		 * A quarter turn leaves a rectangle axis-aligned, so turning
		 * two opposite corners and sorting them is enough - no need to
		 * carry all four around.
		 */
		to_panel(f.bbox.x, f.bbox.y, c1);
		to_panel(f.bbox.x + f.bbox.w, f.bbox.y + f.bbox.h, c2);

		box.x = (int)std::min(c1.x, c2.x);
		box.y = (int)std::min(c1.y, c2.y);
		box.width = (int)std::max(c1.x, c2.x) - box.x;
		box.height = (int)std::max(c1.y, c2.y) - box.y;

		/* OpenCV clips for us, but a box clipped away to nothing is
		 * not worth the call */
		box &= cv::Rect(0, 0, (int)w, (int)h);
		if (box.width <= 0 || box.height <= 0)
			continue;

		cv::rectangle(canvas, box, OSD_BOX_COLOUR, line);

		/* The five landmarks: both eyes, the nose, both mouth
		 * corners, each in its own colour */
		for (int k = 0; k < OSD_LANDMARKS; k++) {
			cv::Point2f p;

			to_panel(f.sparse_kps.points[2 * k],
				 f.sparse_kps.points[2 * k + 1], p);
			if (p.x < 0 || p.y < 0 || p.x >= (float)w ||
			    p.y >= (float)h)
				continue;

			cv::circle(canvas, cv::Point((int)p.x, (int)p.y), dot,
				   color_list_for_osd_det[k], cv::FILLED);
		}

		drawn++;
	}

	return drawn;
}

/* Round up, but never past the edge of the picture */
static int ai_face_fit(int v, int start, int limit)
{
	int up = (v + OSD_SHAPE_STEP - 1) / OSD_SHAPE_STEP * OSD_SHAPE_STEP;

	return (start + up > limit) ? limit - start : up;
}

int ai_face_shapes(struct ai_face *a, unsigned w, unsigned h, int max,
		   const struct osd2d_box **boxes, int *nbox,
		   const struct osd2d_patch **patches, int *npatch)
{
	float sx, sy;
	int line, dot;

	if (boxes != NULL)
		*boxes = NULL;
	if (patches != NULL)
		*patches = NULL;
	if (nbox != NULL)
		*nbox = 0;
	if (npatch != NULL)
		*npatch = 0;

	if (a == NULL || w == 0 || h == 0)
		return -1;

	if (max <= 0)
		max = OSD_SHAPE_FACES;

	{
		std::lock_guard<std::mutex> guard(a->lock);
		a->shown = a->results;
	}

	/*
	 * Over budget: keep the biggest faces. They are the ones near the
	 * camera, which is what anyone looking at the recording afterwards
	 * cares about, and the small ones at the back are where the dots
	 * stop being legible anyway.
	 */
	if ((int)a->shown.size() > max) {
		std::partial_sort(
			a->shown.begin(), a->shown.begin() + max,
			a->shown.end(),
			[](const FaceDetectionInfo &p,
			   const FaceDetectionInfo &q) {
				return p.bbox.w * p.bbox.h > q.bbox.w * q.bbox.h;
			});
		a->shown.resize(max);
	}

	a->boxes.clear();
	a->patches.clear();
	if (a->pixels.size() < a->shown.size())
		a->pixels.resize(a->shown.size());

	/* Both channels show the same scene, so this is just a scale - and
	 * unlike the screen overlay there is no rotation to undo: what goes
	 * to the encoder is the camera's own orientation. */
	sx = (float)w / (float)a->cam.width;
	sy = (float)h / (float)a->cam.height;

	/* The same thicknesses the overlay uses, so the two copies of a box
	 * look alike when both are on */
	line = std::max(2, (int)h / 180);
	dot = std::max(2, (int)h / 135);

	for (size_t i = 0; i < a->shown.size(); i++) {
		const FaceDetectionInfo &f = a->shown[i];
		struct osd2d_box box;
		struct osd2d_patch patch;
		cv::Point pts[OSD_LANDMARKS];
		int which[OSD_LANDMARKS]; /* which feature each point is */
		int x0, y0, x1, y1;
		int k, n;

		box.x = (int)(f.bbox.x * sx);
		box.y = (int)(f.bbox.y * sy);
		box.w = (int)(f.bbox.w * sx);
		box.h = (int)(f.bbox.h * sy);

		/* A face half out of shot gives a box half out of shot, and
		 * the hardware is given coordinates, not a canvas to clip
		 * against - so clip here */
		if (box.x < 0) {
			box.w += box.x;
			box.x = 0;
		}
		if (box.y < 0) {
			box.h += box.y;
			box.y = 0;
		}
		if (box.x + box.w > (int)w)
			box.w = (int)w - box.x;
		if (box.y + box.h > (int)h)
			box.h = (int)h - box.y;
		if (box.w <= 0 || box.h <= 0)
			continue;

		box.rgb = OSD_BOX_RGB;
		box.line = (unsigned)line;
		a->boxes.push_back(box);

		/*
		 * The five dots, in a patch of their own. One patch per face,
		 * not one per landmark: there are only eight patches to go
		 * round and the five features sit in a small part of the face
		 * anyway, so they share.
		 */
		x0 = y0 = INT_MAX;
		x1 = y1 = INT_MIN;
		n = 0;

		for (k = 0; k < OSD_LANDMARKS; k++) {
			int px = (int)(f.sparse_kps.points[2 * k] * sx);
			int py = (int)(f.sparse_kps.points[2 * k + 1] * sy);

			if (px < 0 || py < 0 || px >= (int)w || py >= (int)h)
				continue;

			pts[n].x = px;
			pts[n].y = py;
			which[n] = k; /* the colour follows the feature, not
					 the order they survived clipping in */
			n++;

			x0 = std::min(x0, px);
			y0 = std::min(y0, py);
			x1 = std::max(x1, px);
			y1 = std::max(y1, py);
		}

		if (n == 0)
			continue;

		/* Room for the dots themselves, and an even corner so the
		 * patch lands on a whole chroma sample */
		x0 = std::max(0, (x0 - dot - 1) & ~1);
		y0 = std::max(0, (y0 - dot - 1) & ~1);
		x1 = std::min((int)w, x1 + dot + 2);
		y1 = std::min((int)h, y1 + dot + 2);

		if (x1 - x0 < OSD_SHAPE_MIN || y1 - y0 < OSD_SHAPE_MIN)
			continue;

		patch.x = x0;
		patch.y = y0;
		patch.w = ai_face_fit(x1 - x0, x0, (int)w);
		patch.h = ai_face_fit(y1 - y0, y0, (int)h);

		if (patch.w <= 0 || patch.h <= 0)
			continue;

		{
			std::vector<uint32_t> &buf = a->pixels[a->patches.size()];
			size_t want = (size_t)patch.w * patch.h;

			if (buf.size() < want)
				buf.resize(want);

			/* Everything not drawn on stays fully transparent,
			 * which is how only the dots end up on the picture */
			memset(buf.data(), 0, want * sizeof(uint32_t));

			cv::Mat canvas(patch.h, patch.w, CV_8UC4, buf.data(),
				       (size_t)patch.w * 4);

			/*
			 * The same colours as the screen overlay, with their
			 * four bytes turned round: the 2D engine reads a pixel
			 * as alpha, red, green, blue, where the DRM plane the
			 * overlay draws into keeps alpha last. Written the
			 * overlay's way round, blue is taken for alpha - the
			 * two mouth corners have no blue in them and vanish
			 * altogether, and the rest come out the wrong colour.
			 */
			for (k = 0; k < n; k++) {
				const cv::Scalar &c =
					color_list_for_osd_det[which[k]];

				cv::circle(canvas,
					   cv::Point(pts[k].x - x0,
						     pts[k].y - y0),
					   dot, cv::Scalar(c[3], c[2], c[1],
							   c[0]),
					   cv::FILLED);
			}

			patch.argb = buf.data();
		}

		a->patches.push_back(patch);
	}

	if (boxes != NULL)
		*boxes = a->boxes.data();
	if (nbox != NULL)
		*nbox = (int)a->boxes.size();
	if (patches != NULL)
		*patches = a->patches.data();
	if (npatch != NULL)
		*npatch = (int)a->patches.size();

	return (int)a->boxes.size();
}
