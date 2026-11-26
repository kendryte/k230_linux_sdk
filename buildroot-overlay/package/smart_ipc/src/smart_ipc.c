/*
 * A whole IP camera in one process: camera -> screen + stream + AI
 *
 * Capture once and give the same picture to everything that wants it - the
 * HDMI/LCD panel, the encoder behind the stream, and the face detector -
 * without any of the three getting in each other's way.
 *
 * Everything is on when nothing is asked for. Run it bare and the panel
 * lights up, H.264 goes out over RTSP and faces get outlined; an IP camera
 * doing its whole job is the normal case, not a special one. --enable names
 * fewer of the three when fewer are wanted, in any combination, because each
 * of the three is a camera channel plus the one piece of hardware that
 * consumes it and they are independent all the way down. Running one alone
 * and subtracting the idle figures is how its cost gets measured, and on a
 * 128 MB board that number decides what else fits.
 *
 *   main stream (NV12)     sub stream (NV12)        AI stream (BG3P)
 *   /dev/video1            /dev/video2              /dev/video3
 *      |                      |                            |
 *      +--> H.264/H.265       +--> DRM video plane         +--> KPU, face
 *             |                      zero copy, the dmabuf       detection
 *             +--> RTSP              the camera exported         |
 *             +--> WebRTC            goes straight into          +--> DRM
 *                                    drmModeAddFB2()                  overlay
 *                                                                     plane
 *
 * Nothing is ever written to a file. The encoder exists to feed the stream
 * and only that, which is why they are one name in --enable: a 128 MB board
 * has nowhere to put hours of H.264, and a recorder that quietly fills the
 * rootfs is worse than no recorder at all.
 *
 * RTSP and WebRTC are the same picture offered two ways and only one may be
 * on at a time, because each wants its own threads around the same encoder
 * output and there is no reason to pay for both: RTSP for VLC and ffplay,
 * WebRTC for a browser with nothing installed. --protocol picks which.
 *
 * The boxes the AI finds go to two places. One is an overlay plane above the
 * video plane, which the panel composites and nobody else ever sees. The
 * other is the frames themselves, so whoever is watching the stream sees
 * them too. Both follow from --enable: the boxes turn up on whichever of the
 * two outputs is running. The K230's 2D engine does that drawing - see
 * osd2d.h - straight onto the camera's frame where it lies, without copying
 * it anywhere, which is the only way it fits in a frame's time.
 *
 * The screen always gets a channel of its own, even when the encoder happens
 * to want the same size, so the two can change resolution or start and stop
 * independently.
 *
 * The encoder reads the camera's buffer where it lies, with no copy. That
 * took a kernel fix to be possible at all: the VPU describes NV12 as two
 * planes and imports a dmabuf per plane, while the camera exports one fd per
 * frame with the chroma behind the luma. V4L2 has a way to say exactly that -
 * the same fd on both planes, the luma size in plane[1].data_offset - but the
 * driver used to drop the offset on the way to the hardware and the chroma
 * would be read from the address of the luma. See
 * buildroot-overlay/linux/0043-media-canaan-vpu-honour-per-plane-data_offset
 * -on-inp.patch; on a kernel without it the picture comes out green.
 *
 * The cost of not copying is that a frame now stays busy until the VPU is
 * done with it, so the channel feeding the encoder needs one more buffer than
 * the encoder itself holds. How many each channel gets is fixed below, at the
 * smallest counts measured to run clean: a 128 MB board cannot spare a round
 * number here.
 *
 * Why the picture has to be sized for the panel by hand: the K230 video layer
 * only takes a width, a height and a top-left corner. There is no scaler
 * anywhere between the camera buffer and the panel, so a frame bigger than
 * the screen is not shrunk, it is cut off. The ISP is what resizes, which is
 * exactly what the sub stream is for. That is also why the preview size is
 * not an option: there is exactly one right answer, the panel's, and it is
 * read off the connector at startup.
 *
 * None of the plumbing is an option either. The three video nodes are fixed
 * at /dev/video1, 2 and 3 because the ISP lays a sensor's channels out back
 * to back in that order and there is nothing to choose between; the panel's
 * size and orientation come from DRM; the 2D engine is found by asking each
 * node what it is. Going and enumerating /dev/video* to "discover" the
 * camera is worse than useless here - opening and closing an ISP node tears
 * its pipeline down, and the next STREAMON fails.
 *
 * Buffer lifetime is the only part with any concurrency in it. A frame goes
 * back to the camera when every consumer is done with it, counted in
 * frame_pool.refs[]. The encoder holds one from the QBUF until
 * kd_enc_reclaim_frame() hands the slot back; the display holds one from the
 * commit until the page flip that replaces it. The AI needs no entry in it
 * at all: it runs on a thread of its own with a channel and a set of buffers
 * of its own, so the only thing it shares with this loop is the list of
 * boxes it has found.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define _GNU_SOURCE

#include "cam_capture.h"

#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <linux/videodev2.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <drm/drm_fourcc.h>

#include "display.h"
#include "rtsp_out.h"
#include "vcodec.h"

#include "ai_face.h"
#include "sysstat.h"
#include "webrtc_out.h"

#define MAX_BUFFERS 16

/*
 * The plumbing, fixed rather than offered as options.
 *
 * A sensor's channels are laid out back to back by the ISP: the main one
 * first, then its scaled sub channels. On this chip that means video1 for
 * the encoder, video2 for the screen and video3 for the AI, and there is no
 * second arrangement to pick between - so the first is named here once and
 * sub_channel_of() counts along from it.
 *
 * DRM card 0 is the only one on the chip.
 */
#define CAM_MAIN "/dev/video1"
#define DRM_CARD 0

/*
 * How many frames each camera channel is given. On a 128 MB board every one
 * of them is expensive, so they are the smallest counts that were measured
 * to run clean at 30 fps rather than a round number:
 *
 *   screen   the panel keeps two at once, the frame being scanned out and
 *            the one committed but not yet flipped in, so a third has to be
 *            waiting in the camera's queue or the sensor has nowhere to
 *            write. With two the preview sticks on its first frame for good.
 *   encoder  the encoder hands a frame straight back, so two is enough:
 *            one in the encoder, one being filled.
 *   AI       the thread holds exactly one while the KPU runs, and drops
 *            whatever arrives meanwhile by design.
 */
#define SCREEN_BUFFERS 3
#define ENCODER_BUFFERS 2
#define AI_BUFFERS 2

static volatile sig_atomic_t g_stop;

static void on_signal(int sig)
{
	(void)sig;
	g_stop = 1;
}

static double now_sec(void)
{
	struct timespec ts;

	clock_gettime(CLOCK_MONOTONIC, &ts);

	return (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;
}

/* ------------------------------------------------------------------ *
 * Frame ownership
 *
 * One counter per camera buffer. Whoever takes the frame calls get(),
 * whoever is finished calls put(), and the frame goes back to the camera
 * when the count reaches zero. Nothing here is thread safe on purpose:
 * everything runs in the one poll loop below.
 * ------------------------------------------------------------------ */

struct frame_pool {
	struct cam_capture *cam;
	unsigned refs[MAX_BUFFERS];
};

static void frame_get(struct frame_pool *pool, unsigned index)
{
	pool->refs[index]++;
}

static void frame_put(struct frame_pool *pool, unsigned index)
{
	if (pool->refs[index] == 0) {
		fprintf(stderr, "buffer %u released once too often\n", index);
		return;
	}

	if (--pool->refs[index] == 0)
		cam_qbuf(pool->cam, index);
}

/* ------------------------------------------------------------------ *
 * Screen
 * ------------------------------------------------------------------ */

struct screen {
	struct display *disp;
	struct display_plane *plane;
	struct display_buffer *fb[MAX_BUFFERS];
	struct frame_pool *pool; /* camera the fbs above belong to */
	uint32_t x, y; /* top-left corner, so the picture ends up centred */
	uint32_t w, h; /* its size on the panel, i.e. after --rotation */

	/* --ai: the boxes, on a plane above the picture. Updated only when
	 * the AI has something new, and always before the video plane is
	 * committed, so the two land in the same atomic commit. */
	struct display_plane *osd_plane;
	struct display_buffer *osd;

	/* The frame the CRTC is scanning out, and the one committed but not
	 * yet flipped in. -1 means none. */
	int on_screen, flipping;

	unsigned long shown, skipped;
};

static void screen_show(struct screen *s, unsigned index)
{
	/*
	 * Handing the CRTC a second frame before the first one landed would
	 * only be rejected, and at 30 fps into a 60 Hz panel it practically
	 * never happens.
	 */
	if (s->flipping >= 0) {
		s->skipped++;
		return;
	}

	frame_get(s->pool, index);

	if (display_commit_buffer(s->fb[index], s->x, s->y) == 0) {
		s->flipping = (int)index;
		s->shown++;
	} else {
		fprintf(stderr, "committing frame %u to the screen failed\n",
			index);
		frame_put(s->pool, index);
	}
}

/* A page flip finished: the frame that was on screen is no longer being
 * scanned out, so its reference can go. */
static void screen_vsync(struct screen *s)
{
	display_handle_vsync(s->disp);

	if (s->on_screen >= 0)
		frame_put(s->pool, (unsigned)s->on_screen);

	s->on_screen = s->flipping;
	s->flipping = -1;
}

static bool rotation_swaps_axes(enum drm_rotation r)
{
	return r == rotation_90 || r == rotation_270;
}

static const char *connector_kind(uint32_t type)
{
	switch (type) {
	case DRM_MODE_CONNECTOR_DSI:
		return "LCD panel";
	case DRM_MODE_CONNECTOR_HDMIA:
	case DRM_MODE_CONNECTOR_HDMIB:
		return "HDMI";
	default:
		return "display";
	}
}

/*
 * Each sensor owns four video nodes laid out back to back: its main channel,
 * then two scaled sub channels, then the raw one. /dev/video1 is a main
 * channel, /dev/video2 its first sub channel, /dev/video3 its second. That is
 * what lets the screen have a small picture of its own while the encoder
 * keeps the big one, and the AI a third at whatever size it likes.
 *
 * @step is how many channels along to count: 1 for the screen, 2 for the AI.
 */
static int sub_channel_of(const char *dev, unsigned step, char *buf, size_t len)
{
	const char *digits = dev + strlen(dev);
	unsigned long n;

	while (digits > dev && digits[-1] >= '0' && digits[-1] <= '9')
		digits--;

	if (*digits == '\0')
		return -1; /* no trailing number to count on */

	n = strtoul(digits, NULL, 10);

	if (snprintf(buf, len, "%.*s%lu", (int)(digits - dev), dev,
		     n + step) >= (int)len)
		return -1;

	return 0;
}

/*
 * One encoder packet out to whichever of the two servers is running. Exactly
 * one of them always is: with the stream path enabled there is no third
 * option, and without it there is no encoder to produce a packet at all.
 *
 * The RTSP timestamp is taken here rather than carried from the camera:
 * librtsp_server throws it away and stamps the frame when its own thread
 * picks it up, so anything monotonic will do. WebRTC is the other way round
 * and uses the encoder's own @pts, which is why that one is passed in.
 */
static void emit_packet(struct rtsp_out *rtsp, struct webrtc_out *webrtc,
			const unsigned char *data, size_t size, int64_t pts,
			bool keyframe)
{
	if (rtsp != NULL &&
	    rtsp_out_send(rtsp, data, size, (uint64_t)(now_sec() * 1e6)) < 0)
		fprintf(stderr, "handing a packet to the RTSP server failed\n");

	/* A drop here is the queue running ahead of the sender, which is
	 * counted and reported at the end rather than printed per packet:
	 * if it ever happens it happens in bursts. */
	if (webrtc != NULL)
		(void)webrtc_out_send(webrtc, data, size, (uint64_t)pts,
				      keyframe);
}

/* ------------------------------------------------------------------ */

static void usage(const char *prog)
{
	/*
	 * The options and the commands worth copying, and nothing else. What
	 * a help screen gets opened for is "which letter was the bitrate
	 * again" and "how do I run just the encoder"; explanations of how
	 * the thing works belong in README.md, where they do not sit between
	 * the reader and the answer.
	 *
	 * The measuring recipe at the bottom earns its space. It is the
	 * reason --enable exists, it is seven commands nobody remembers the
	 * order of, and the alternative is looking them up somewhere that is
	 * not on the board.
	 *
	 * @prog is not used: the name is written out in the examples so they
	 * stay aligned and can be copied as they are, which is what they are
	 * for. Running it through a path would ruin both.
	 */
	(void)prog;

	printf(
"smart_ipc - camera to panel, to H.264 over the network, and to face detection\n"
"\n"
"OPTIONS\n"
"\n"
"  -E, --enable LIST     paths to start: display, stream, ai, camera\n"
"                        default display,stream,ai\n"
"  -i, --stats-only      start no path, report resource usage only\n"
"\n"
"  -c, --codec NAME      video codec, h264 or h265            default h264\n"
"  -W, --width PIXELS    encoded frame width                  default 1280\n"
"  -H, --height PIXELS   encoded frame height                 default 720\n"
"  -f, --fps RATE        capture frame rate                   default 30\n"
"  -b, --bitrate BPS     encoder bitrate in bits per second\n"
"                        0 derives it from the resolution     default 1500000\n"
"\n"
"  -m, --protocol KIND   delivery protocol, rtsp or webrtc    default rtsp\n"
"  -p, --port N          server listen port                   default 8554\n"
"  -u, --path NAME       stream path, as in rtsp://<board-ip>:8554/live\n"
"                                                             default live\n"
"\n"
"  -x, --ai-width PX     detector input width                 default 1280\n"
"  -y, --ai-height PX    detector input height                default 720\n"
"  -o, --ai-thresh 0..1  detection confidence threshold       default 0.6\n"
"  -n, --ai-nms 0..1     non-maximum suppression threshold    default 0.2\n"
"  -k, --ai-model FILE   detection model\n"
"                        default /usr/share/smart_ipc/face_detection_320.kmodel\n"
"\n"
"  -t, --time SECONDS    run duration, 0 runs until interrupted    default 0\n"
"  -S, --stats SECONDS   resource report interval, 0 disables it   default 5\n"
"  -h, --help            print this help and exit\n"
"\n"
"EXAMPLES\n"
"\n"
"    smart_ipc                                  panel, stream and detection\n"
"    smart_ipc --enable display,stream          no detection\n"
"    smart_ipc --enable stream -W 1920 -H 1080  stream only, at 1080p\n"
"    smart_ipc --protocol webrtc                watch it in a browser\n"
"\n"
"What each path costs: run these and subtract the first one's \"cma used\".\n"
"\n"
"    smart_ipc --stats-only            -t 10    the baseline\n"
"    smart_ipc --enable camera         -t 10    the channel, nothing behind it\n"
"    smart_ipc --enable display        -t 10    the panel on its own\n"
"    smart_ipc --enable stream         -t 10    the encoder on its own\n"
"    smart_ipc --enable ai             -t 10    face detection on its own\n"
"    smart_ipc --enable display,stream -t 10    the pair without detection\n"
"    smart_ipc --enable stream,ai      -t 10    the pair without the panel\n"
"    smart_ipc                         -t 10    all three at once\n"
	);
}

/*
 * strtoul() has two blind spots that matter here. A string it cannot parse
 * at all comes back as 0, which is indistinguishable from someone asking
 * for 0 on purpose - and 0 is a meaningful answer for --bitrate and --time.
 * A number far beyond anything the hardware can do comes back intact and is
 * passed down to the encoder and the ISP, which then fail somewhere much
 * less obvious than the command line.
 *
 * So every numeric option goes through here: the whole argument has to be
 * a number, and the number has to be inside a range the rest of the program
 * can act on. That makes the range part of the error message, which is the
 * one thing the user needs to know, and it means the checks do not have to
 * be repeated after the option loop.
 */
static bool parse_uint(const char *arg, const char *opt, unsigned long lo,
		       unsigned long hi, unsigned *out)
{
	unsigned long v;
	char *end;

	errno = 0;
	v = strtoul(arg, &end, 0);

	if (end == arg || *end != '\0' || errno == ERANGE || v < lo || v > hi) {
		fprintf(stderr, "%s takes %lu to %lu, not \"%s\"\n", opt, lo, hi,
			arg);
		return false;
	}

	*out = (unsigned)v;
	return true;
}

/*
 * --enable, the only switch that decides what runs. Each name is one
 * camera channel plus the one piece of hardware that consumes it, so every
 * subset of them is a configuration the board can really be in - and that
 * is the whole reason this is a list of things to turn on rather than the
 * off switches it replaces. What a path costs is measured by running it
 * alone and subtracting --stats-only, and the off switches could not
 * express most of those runs: --no-display and --no-stream together were
 * refused, which is precisely "detection only".
 *
 * "camera" is the odd one out: it has no consumer at all. It opens the
 * channel, takes the frames and hands them straight back, which splits the
 * cost of "stream" into the two halves anyone actually wants separated -
 * what the sensor and its buffers cost, and what the encoder adds on top.
 *
 * list is written through, which is allowed: it is optarg, a pointer into
 * argv, and nothing reads the argument again afterwards.
 */
static bool parse_enable(char *list, bool *display, bool *encode, bool *ai,
			 bool *camera)
{
	char *save = NULL, *tok;
	bool any = false;

	*display = false;
	*encode = false;
	*ai = false;
	*camera = false;

	for (tok = strtok_r(list, ",", &save); tok != NULL;
	     tok = strtok_r(NULL, ",", &save)) {
		if (strcmp(tok, "display") == 0)
			*display = true;
		else if (strcmp(tok, "stream") == 0)
			*encode = true;
		else if (strcmp(tok, "ai") == 0)
			*ai = true;
		else if (strcmp(tok, "camera") == 0)
			*camera = true;
		else {
			fprintf(stderr,
				"--enable does not know \"%s\". The names are "
				"display, stream, ai and camera, comma "
				"separated, in any combination: "
				"--enable display,ai\n",
				tok);
			return false;
		}

		any = true;
	}

	/*
	 * An empty list is a typo, not a request. The one thing it looks like
	 * it should mean already has a switch, and that switch does more than
	 * start nothing - it keeps printing.
	 */
	if (!any) {
		fprintf(stderr, "--enable needs at least one of display, "
				"stream, ai, camera. To start none of them "
				"and only print the idle figures, use "
				"--stats-only\n");
		return false;
	}

	return true;
}

int main(int argc, char **argv)
{
	/*
	 * Every long option returns its own short letter, so both spellings
	 * land on the same case below. The alternative - giving the long
	 * forms codes of their own - is how they silently stopped working
	 * once: the codes were in this table but never in the switch, so
	 * every long option fell through to usage().
	 */
	static const struct option longopts[] = {
		{ "enable", required_argument, NULL, 'E' },
		{ "codec", required_argument, NULL, 'c' },
		{ "width", required_argument, NULL, 'W' },
		{ "height", required_argument, NULL, 'H' },
		{ "fps", required_argument, NULL, 'f' },
		{ "bitrate", required_argument, NULL, 'b' },
		{ "ai-width", required_argument, NULL, 'x' },
		{ "ai-height", required_argument, NULL, 'y' },
		{ "ai-model", required_argument, NULL, 'k' },
		{ "ai-thresh", required_argument, NULL, 'o' },
		{ "ai-nms", required_argument, NULL, 'n' },
		{ "protocol", required_argument, NULL, 'm' },
		{ "port", required_argument, NULL, 'p' },
		{ "path", required_argument, NULL, 'u' },
		{ "time", required_argument, NULL, 't' },
		{ "stats", required_argument, NULL, 'S' },
		{ "stats-only", no_argument, NULL, 'i' },
		{ "help", no_argument, NULL, 'h' },
		{ NULL, 0, NULL, 0 },
	};
	const char *dev = CAM_MAIN;
	const char *preview_dev = NULL;
	const char *rtsp_path = "live";
	unsigned width = 1280, height = 720, fps = 30;
	unsigned preview_w = 0, preview_h = 0;
	unsigned seconds = 0, bitrate = 1500000;
	/*
	 * How often the CPU and memory lines are printed. On by default: on
	 * a 128 MB board the interesting failures are allocation failures,
	 * and they are invisible unless someone was already watching when
	 * they happened.
	 */
	unsigned stats_interval = 5;
	struct sysstat stats = { 0 };
	double next_stats = 0.0;
	/*
	 * --stats-only: print the numbers and start nothing at all. Still a
	 * switch of its own and not --enable with an empty list, because it
	 * is the baseline every other run is subtracted from, and a baseline
	 * that has to be spelled as "none of the above" is a baseline nobody
	 * measures.
	 */
	bool stats_only = false;
	const unsigned drm_index = DRM_CARD;
	int rtsp_port = 8554;
	/*
	 * All three paths, unless --enable names fewer. Nothing else in the
	 * option loop touches them, so there is no want_x_given to carry:
	 * --enable sets all three at once or none of them is set at all.
	 */
	bool want_display = true, want_encode = true;
	/*
	 * --enable camera: the main channel open with nothing behind it.
	 * Off by default and never implied, because on its own it produces
	 * no picture anywhere - it is a measurement, not a way to run.
	 */
	bool want_camera = false;
	bool want_rtsp = true;
	char sub_dev[64];

	/* --stream webrtc: the same encoder output, offered to a browser */
	bool want_webrtc = false;
	int webrtc_port = 8080;
	bool port_given = false;
	struct webrtc_out *webrtc = NULL;

	/* face detection: a channel, a thread and an overlay plane of its own */
	bool want_ai = true;
	const char *ai_model = NULL;
	unsigned ai_w = 1280, ai_h = 720;
	float ai_thresh = 0.0f, ai_nms = 0.0f;
	/*
	 * The boxes go into the encoded picture too, whenever both ends of
	 * that are running. Not a switch: it follows from --enable, the same
	 * way the panel's boxes do.
	 */
	bool want_burn = true;
	char ai_dev_buf[64];
	const char *ai_dev = NULL;
	unsigned long ai_drawn_gen = 0, ai_osd_updates = 0;
	unsigned long ai_burnt = 0;
	struct ai_face *ai = NULL;
	struct osd2d *osd = NULL;
	bool want_overlay = false;
	/*
	 * The camera's own buffers, handed to the encoder as they are. Held
	 * here only so it can be destroyed after the encoder lets go of it.
	 */
	kd_bufgroup *enc_in = NULL;

	enum drm_rotation rotation = rotation_0;
	kd_codec codec = KD_CODEC_H264;

	/* Main stream: feeds the encoder, and the screen too unless --preview */
	struct cam_capture cam;
	struct frame_pool pool;
	/* Sub stream: only opened for --preview */
	struct cam_capture pcam;
	struct frame_pool ppool;
	/* Whichever of the two the screen reads from */
	struct cam_capture *dcam = NULL;

	struct screen screen;
	struct sigaction sa;
	struct pollfd fds[4];
	nfds_t nfds;
	int cam_slot = -1, pcam_slot = -1, drm_slot = -1;

	kd_enc_cfg cfg;
	kd_enc *enc = NULL;
	unsigned char *packet = NULL;
	size_t packet_cap = 0;

	struct rtsp_out *rtsp = NULL;

	unsigned long frames = 0, pframes = 0, encoded = 0, packets = 0;
	unsigned long skipped_enc = 0;
	unsigned long long bytes = 0;
	double start, now;
	int rc = 1;
	int c, ret;
	unsigned i;

	cam.fd = -1;
	pcam.fd = -1;
	memset(&pool, 0, sizeof(pool));
	memset(&ppool, 0, sizeof(ppool));
	memset(&screen, 0, sizeof(screen));
	screen.on_screen = -1;
	screen.flipping = -1;

	while ((c = getopt_long(argc, argv,
				"W:H:c:f:b:x:y:k:o:n:m:p:u:t:S:E:hi", longopts,
				NULL)) != -1) {
		switch (c) {
		case 'E':
			if (!parse_enable(optarg, &want_display, &want_encode,
					  &want_ai, &want_camera))
				return 1;
			break;
		case 'c':
			if (strcmp(optarg, "h264") == 0) {
				codec = KD_CODEC_H264;
			} else if (strcmp(optarg, "h265") == 0) {
				codec = KD_CODEC_H265;
			} else {
				fprintf(stderr,
					"--codec takes h264 or h265, not %s\n",
					optarg);
				return 1;
			}
			break;
		case 'W':
			if (!parse_uint(optarg, "--width", 16, 8192, &width))
				return 1;
			break;
		case 'H':
			if (!parse_uint(optarg, "--height", 16, 8192, &height))
				return 1;
			break;
		case 'f':
			if (!parse_uint(optarg, "--fps", 1, 240, &fps))
				return 1;
			break;
		case 'b':
			/* 0 is the documented "work it out from the picture" */
			if (!parse_uint(optarg, "--bitrate", 0, 200000000,
					&bitrate))
				return 1;
			break;
		case 'x':
			if (!parse_uint(optarg, "--ai-width", 16, 4096, &ai_w))
				return 1;
			break;
		case 'y':
			if (!parse_uint(optarg, "--ai-height", 16, 4096, &ai_h))
				return 1;
			break;
		case 'k':
			ai_model = optarg;
			break;
		case 'o':
			ai_thresh = strtof(optarg, NULL);
			break;
		case 'n':
			ai_nms = strtof(optarg, NULL);
			break;
		case 'm':
			if (strcmp(optarg, "rtsp") == 0) {
				want_rtsp = true;
				want_webrtc = false;
			} else if (strcmp(optarg, "webrtc") == 0) {
				want_webrtc = true;
				want_rtsp = false;
			} else {
				fprintf(stderr,
					"--protocol takes rtsp or webrtc, not "
					"%s. They are two ways of sending the "
					"same picture: rtsp for VLC or ffplay, "
					"webrtc for a browser.\n",
					optarg);
				return 1;
			}
			break;
		case 'p': {
			unsigned port;

			if (!parse_uint(optarg, "--port", 1, 65535, &port))
				return 1;
			rtsp_port = (int)port;
			webrtc_port = rtsp_port;
			port_given = true;
			break;
		}
		case 'u':
			rtsp_path = optarg;
			break;
		case 't':
			/* 0 is the documented "until Ctrl-C" */
			if (!parse_uint(optarg, "--time", 0, 31536000, &seconds))
				return 1;
			break;
		case 'S':
			/* 0 is the documented "do not print them at all" */
			if (!parse_uint(optarg, "--stats", 0, 3600,
					&stats_interval))
				return 1;
			break;
		case 'i':
			stats_only = true;
			break;
		case 'h':
			usage(argv[0]);
			return 0;
		default:
			usage(argv[0]);
			return 1;
		}
	}

	/*
	 * --enable ai on its own used to be refused: with neither the panel
	 * nor the stream there is nowhere for a picture to go, so it looked
	 * like a mistake. It is not. The detector still prints what it found
	 * and still costs a camera channel, a thread and a model, and running
	 * it alone is the only way to find out how much of each.
	 */

	if (ai_thresh < 0.0f || ai_thresh >= 1.0f) {
		fprintf(stderr, "--ai-thresh is a score between 0 and 1, not "
				"%f\n",
			(double)ai_thresh);
		return 1;
	}

	if (ai_nms < 0.0f || ai_nms >= 1.0f) {
		fprintf(stderr, "--ai-nms is an overlap between 0 and 1, not "
				"%f\n",
			(double)ai_nms);
		return 1;
	}

	/*
	 * Burning the boxes in needs both ends of it: something to draw and
	 * somewhere to draw it. Nothing is said when one is missing, because
	 * nothing was asked for - this follows from --enable rather than from
	 * a switch of its own, and it is visible either way. Boxes appear on
	 * whichever of the two outputs is running, and on both when both are.
	 */
	want_burn = want_ai && want_encode;

	/*
	 * Both name the same channel, and "stream" is the one with something
	 * behind it. Worth saying out loud rather than quietly obeying: a
	 * run meant to measure the bare channel that also started the
	 * encoder would be read as the bare channel's cost.
	 */
	if (want_camera && want_encode) {
		printf("--enable camera is already covered by stream, which "
		       "opens the same channel\n");
		want_camera = false;
	}

	/* The two have different defaults, and -p set both */
	if (want_webrtc && !port_given)
		webrtc_port = 8080;

	if (!want_encode) {
		want_rtsp = false;
		want_webrtc = false;
	}

	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = on_signal;
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);

	/*
	 * The signaling server writes HTTP responses with send(..., 0) from
	 * its own thread. A browser that refreshes or closes the tab mid
	 * response leaves it writing to a closed socket, and the default
	 * disposition of SIGPIPE would take the whole camera down with it -
	 * display, encoder and detection included. A failed send() returning
	 * EPIPE is all we want here.
	 *
	 * This has to be process wide rather than MSG_NOSIGNAL at the call
	 * sites: http_server.c is a verbatim copy of the rt-smart original
	 * and is meant to stay that way. In RTSP mode live555 happens to do
	 * the same thing for us from ignoreSigPipeOnSocket(), but that is a
	 * side effect of a library we may not even start.
	 */
	signal(SIGPIPE, SIG_IGN);

	/*
	 * Which paths this run is, on one line and before anything is opened.
	 * Memory figures mean nothing without it: these lines get compared
	 * across runs, and "which one was this" is not something anyone
	 * reconstructs from a scrollback an hour later.
	 */
	if (!stats_only)
		printf("enabled:%s%s%s%s\n", want_display ? " display" : "",
		       want_encode ? " stream" : "", want_ai ? " ai" : "",
		       want_camera ? " camera" : "");

	/*
	 * --stats-only stops here, before anything is opened: no sensor, no
	 * encoder, no screen, no detection. So there is nothing to poll and
	 * nothing to tear down, and the numbers are the idle baseline a
	 * running camera can be compared against - in particular how much of
	 * the CMA area the hardware is holding when no hardware is running.
	 *
	 * Reported on the same grid and in the same format as a real run, so
	 * the two can be put side by side.
	 */
	if (stats_only) {
		double base, at, next;

		if (stats_interval == 0) {
			fprintf(stderr, "--stats-only together with --stats 0 "
					"would print nothing at all\n");
			return 1;
		}

		printf("stats only: nothing is started, these are the idle "
		       "figures\n");

		base = now_sec();
		sysstat_init(&stats);
		next = base + (double)stats_interval;

		while (!g_stop) {
			at = now_sec();

			if (at >= next) {
				sysstat_report(&stats, at - base);

				do {
					next += (double)stats_interval;
				} while (next <= at);
			}

			if (seconds > 0 && at - base >= (double)seconds)
				break;

			/* same 100 ms as the real loop's poll, so Ctrl-C
			 * is answered just as quickly */
			usleep(100000);
		}

		return 0;
	}

	/* ---- screen first: its size is what the preview stream is sized to ---- */
	if (want_display) {
		screen.disp = display_init(drm_index);
		if (screen.disp == NULL) {
			fprintf(stderr, "opening DRM card %u failed, is a "
					"panel connected?\n",
				drm_index);
			goto cleanup;
		}

		/*
		 * A panel wired portrait but looked at landscape - the
		 * 01studio LCD is 480x800 - needs the picture turned a
		 * quarter before it goes out. HDMI is landscape already.
		 */
		if (screen.disp->height > screen.disp->width)
			rotation = rotation_90;

		/*
		 * Has to be set before display_get_plane(): for a turned
		 * picture only a plane that carries a "rotation" property
		 * will do, and that is the condition the search uses.
		 */
		screen.disp->drm_rotation = rotation;

		screen.plane = display_get_plane(screen.disp, DRM_FORMAT_NV12);
		if (screen.plane == NULL) {
			fprintf(stderr, "no free NV12 plane on card %u\n",
				drm_index);
			goto cleanup;
		}
		screen.plane->drm_rotation = rotation;

		printf("display: %s %ux%u, plane %u, rotation %u\n",
		       connector_kind(screen.disp->conn->connector_type),
		       screen.disp->width, screen.disp->height,
		       screen.plane->plane_id,
		       rotation == rotation_90	? 90 :
		       rotation == rotation_180 ? 180 :
		       rotation == rotation_270 ? 270 :
						  0);
	}

	/* ---- cameras ---- */
	if (want_encode || want_camera) {
		/* Mapped only for the encoder, which reads the frames with
		 * the CPU. With nothing behind the channel there is nobody to
		 * map them for, and leaving them unmapped keeps the figures
		 * about the camera rather than about our address space.
		 * NULL for the sensor mode: the ISP keeps whatever it was set
		 * up with, which is the sensor's native mode */
		ret = cam_open(&cam, dev, NULL, width, height,
			       V4L2_PIX_FMT_NV12, ENCODER_BUFFERS,
			       want_encode);
		if (ret < 0)
			goto cleanup;

		pool.cam = &cam;

		printf("%s: NV12 %ux%u stride=%u, %u buffers%s\n", dev,
		       cam.width, cam.height, cam.stride, cam.count,
		       want_camera ? " (nothing reads them)" : "");

		/*
		 * The encoder assumes the chroma plane starts right after the
		 * luma one. Say so when the driver reports a frame size that
		 * does not fit, instead of showing a green picture.
		 */
		if (want_encode &&
		    cam.frame_size != (size_t)cam.stride * cam.height * 3 / 2)
			fprintf(stderr,
				"warning: the camera reports %zu bytes per frame, not "
				"the %zu a packed NV12 frame takes. The chroma plane "
				"is probably not at offset stride x height, so the "
				"colours may come out wrong.\n",
				cam.frame_size,
				(size_t)cam.stride * cam.height * 3 / 2);
	}

	if (want_display) {
		/* How much picture the panel can take, counted the way the
		 * camera produces it: with a quarter turn the two axes swap */
		unsigned fit_w = rotation_swaps_axes(rotation) ?
					 screen.disp->height :
					 screen.disp->width;
		unsigned fit_h = rotation_swaps_axes(rotation) ?
					 screen.disp->width :
					 screen.disp->height;
		unsigned out_w, out_h;

		/*
		 * The screen is fed a picture that is already the panel's
		 * size, because nothing downstream can resize it. This is why
		 * the preview size is not an option: there is exactly one
		 * right answer and the panel just told us what it is.
		 */
		preview_w = fit_w;
		preview_h = fit_h;

		/*
		 * The screen always gets its own channel off the sensor, even
		 * when the encoder happens to want the same size. The two
		 * sides are independent that way: either can change
		 * resolution, start or stop without disturbing the other.
		 */
		if (sub_channel_of(dev, 1, sub_dev, sizeof(sub_dev)) < 0) {
			fprintf(stderr,
				"the screen needs the channel next to %s and "
				"its node name cannot be worked out from "
				"that\n",
				dev);
			goto cleanup;
		}
		preview_dev = sub_dev;

		if (!want_encode)
			printf("display takes %s at %ux%u\n", preview_dev,
			       preview_w, preview_h);
		else
			printf("display takes its own channel: %s at %ux%u "
			       "(%s stays at %ux%u for the encoder)\n",
			       preview_dev, preview_w, preview_h, dev,
			       cam.width, cam.height);

		/* Not mapped: the picture goes to the panel by dmabuf, the
		 * CPU never looks at it */
		ret = cam_open(&pcam, preview_dev, NULL, preview_w, preview_h,
			       V4L2_PIX_FMT_NV12, SCREEN_BUFFERS, false);
		if (ret < 0)
			goto cleanup;

		ppool.cam = &pcam;
		dcam = &pcam;
		screen.pool = &ppool;

		printf("%s: NV12 %ux%u stride=%u, %u buffers (preview)\n",
		       preview_dev, pcam.width, pcam.height, pcam.stride,
		       pcam.count);

		out_w = rotation_swaps_axes(rotation) ? dcam->height :
							dcam->width;
		out_h = rotation_swaps_axes(rotation) ? dcam->width :
							dcam->height;

		/*
		 * The preview channel was asked for exactly fit_w x fit_h, so
		 * this only happens when the ISP rounded the request up to
		 * something it preferred.
		 */
		if (out_w > screen.disp->width || out_h > screen.disp->height) {
			fprintf(stderr,
				"asked %s for %ux%u, the panel's own size, and "
				"got %ux%u back, which does not fit the %ux%u "
				"panel%s. Nothing between the camera and the "
				"screen can shrink a picture, so it would just "
				"be cut off.\n",
				preview_dev, fit_w, fit_h, dcam->width,
				dcam->height, screen.disp->width,
				screen.disp->height,
				rotation_swaps_axes(rotation) ?
					" (after the turn)" :
					"");
			goto cleanup;
		}

		/*
		 * A quarter turn is done by the GDMA into a scratch buffer the
		 * driver owns, and it wants the framebuffer described at the
		 * size the turn produces, with the camera's own layout inside.
		 * That only lines up when the camera adds no padding.
		 */
		if (rotation_swaps_axes(rotation) && dcam->stride != dcam->width) {
			fprintf(stderr,
				"the camera pads its lines to %u bytes for a %u "
				"pixel picture, which the quarter turn this "
				"portrait panel needs cannot describe.\n",
				dcam->stride, dcam->width);
			goto cleanup;
		}

		/* Centre it, so a picture smaller than the panel is not stuck
		 * in the corner */
		screen.x = (screen.disp->width - out_w) / 2;
		screen.y = (screen.disp->height - out_h) / 2;
		screen.w = out_w;
		screen.h = out_h;

		/*
		 * Import once, at startup. A DRM framebuffer is just a
		 * description of memory, so the same camera buffer keeps the
		 * same fb id for the whole run and showing a frame costs one
		 * atomic commit, no allocation and no copy.
		 */
		for (i = 0; i < dcam->count; i++) {
			screen.fb[i] = display_import_buffer(
				screen.plane, dcam->bufs[i].dmabuf_fd, out_w,
				out_h,
				rotation_swaps_axes(rotation) ? out_w :
								dcam->stride);
			if (screen.fb[i] == NULL) {
				fprintf(stderr,
					"importing camera buffer %u into DRM "
					"failed\n",
					i);
				goto cleanup;
			}
		}

		printf("preview: %ux%u at (%u,%u) on the panel%s\n", out_w,
		       out_h, screen.x, screen.y,
		       (out_w == screen.disp->width &&
			out_h == screen.disp->height) ?
			       ", full screen" :
			       "");

		/*
		 * The boxes get a plane of their own, above the video one, so
		 * the panel can show them without anything being drawn into
		 * the picture that goes out over the network. Allocated at
		 * the panel's full size and left unrotated: only rectangles
		 * and dots are drawn, and a quarter turn leaves a rectangle
		 * axis-aligned, so ai_face_draw() turns the handful of
		 * coordinates instead of a megabyte of pixels.
		 *
		 * The burnt-in copy is a separate path and the two never
		 * collide: the screen watches its own channel, which the 2D
		 * engine never touches.
		 */
		want_overlay = want_ai;

		if (want_overlay) {
			screen.osd_plane = display_get_plane(
				screen.disp, DRM_FORMAT_ARGB8888);
			if (screen.osd_plane == NULL)
				printf("note: no free ARGB8888 plane on card "
				       "%u, so the faces will not be outlined "
				       "on the panel\n",
				       drm_index);
		}

		if (screen.osd_plane != NULL) {
			screen.osd = display_allocate_buffer(
				screen.osd_plane, screen.disp->width,
				screen.disp->height);
			if (screen.osd == NULL) {
				fprintf(stderr, "allocating the overlay "
						"buffer failed\n");
				goto cleanup;
			}

			/* Start clear, and commit once to get the plane
			 * switched on before anything is drawn on it */
			memset(screen.osd->map, 0, screen.osd->size);
			ai_face_flush(screen.osd->map, screen.osd->size);
			display_commit_buffer(screen.osd, 0, 0);
		}
	}

	/* ---- encoder ---- */
	if (want_encode) {
		kd_enc_cfg_default(&cfg);
		cfg.codec = codec;
		cfg.pixfmt = KD_PIXFMT_NV12;
		cfg.width = cam.width;
		cfg.height = cam.height;
		cfg.stride = cam.stride;
		cfg.framerate = fps;
		cfg.bitrate = bitrate;

		ret = kd_enc_create(&enc, &cfg);
		if (ret != KD_OK) {
			fprintf(stderr, "creating the encoder failed: %s\n",
				kd_strerror(ret));
			goto cleanup;
		}

		/*
		 * Feed the encoder the camera's own buffers. One dmabuf per
		 * frame, carrying both planes; the library splits it along
		 * the plane sizes the driver reports and the kernel patch
		 * named at the top of this file is what makes the split
		 * reach the hardware.
		 *
		 * Not kd_bufgroup_create_from_v4l2(): that one hands a
		 * finished frame straight back to the camera, and here the
		 * screen may still be scanning it out. The reference count
		 * below decides when it goes back.
		 */
		ret = kd_bufgroup_create_external(&enc_in, cam.count);
		if (ret != KD_OK) {
			fprintf(stderr, "creating the encoder's input group "
					"failed: %s\n",
				kd_strerror(ret));
			goto cleanup;
		}

		for (i = 0; i < cam.count; i++) {
			ret = kd_bufgroup_commit(enc_in, i,
						 cam.bufs[i].dmabuf_fd,
						 cam.frame_size);
			if (ret != KD_OK) {
				fprintf(stderr,
					"handing camera buffer %u to the "
					"encoder failed: %s\n",
					i, kd_strerror(ret));
				goto cleanup;
			}
		}

		ret = kd_enc_attach_input(enc, enc_in);
		if (ret != KD_OK) {
			fprintf(stderr, "attaching the camera buffers to the "
					"encoder failed: %s\n",
				kd_strerror(ret));
			goto cleanup;
		}

		ret = kd_enc_start(enc);
		if (ret != KD_OK) {
			fprintf(stderr, "starting the encoder failed: %s\n",
				kd_strerror(ret));
			goto cleanup;
		}

		packet_cap = cam.frame_size;
		packet = malloc(packet_cap);
		if (packet == NULL)
			goto cleanup;

		if (want_rtsp) {
			rtsp = rtsp_out_open(rtsp_port, rtsp_path,
					     codec == KD_CODEC_H265);
			if (rtsp == NULL)
				goto cleanup;
		}

		if (want_webrtc) {
			/* Not fatal: plenty of machines do play H.265 in the
			 * browser, and the ones that do not fail in a way that
			 * is hard to recognise, so say it up front. */
			if (codec == KD_CODEC_H265)
				printf("note: Firefox cannot decode H.265 over "
				       "WebRTC at all, and Chrome and Safari "
				       "only manage it on hardware that can. "
				       "Use -c h264 if the page stays black.\n");

			webrtc = webrtc_out_open(webrtc_port,
						 codec == KD_CODEC_H265);
			if (webrtc == NULL)
				goto cleanup;
		}

		if (bitrate != 0)
			printf("encoding %s %ux%u @%u, %u bps -> %s\n",
			       codec == KD_CODEC_H265 ? "H.265" : "H.264",
			       cam.width, cam.height, fps, bitrate,
			       want_webrtc ? "WebRTC" : "RTSP");
		else
			printf("encoding %s %ux%u @%u, %u bps (worked out from "
			       "the size) -> %s\n",
			       codec == KD_CODEC_H265 ? "H.265" : "H.264",
			       cam.width, cam.height, fps,
			       cam.width * cam.height * fps / 15,
			       want_webrtc ? "WebRTC" : "RTSP");

		if (rtsp != NULL)
			printf("streaming at %s\n", rtsp_out_url(rtsp));
		if (webrtc != NULL)
			printf("open %s in a browser on the same network\n",
			       webrtc_out_url(webrtc));
	}

	/*
	 * Last, so the sensor mode is already pinned by whoever opened the
	 * first channel: this one only asks the ISP for a scaled copy.
	 */
	if (want_ai) {
		struct ai_face_cfg aicfg;

		ai_face_default_cfg(&aicfg);

		if (sub_channel_of(dev, 2, ai_dev_buf,
				   sizeof(ai_dev_buf)) < 0) {
			fprintf(stderr,
				"the detector needs the second channel along "
				"from %s and its node name cannot be worked "
				"out from that\n",
				dev);
			goto cleanup;
		}
		ai_dev = ai_dev_buf;

		aicfg.dev = ai_dev;
		aicfg.buffers = AI_BUFFERS;
		if (ai_model != NULL)
			aicfg.kmodel = ai_model;
		aicfg.width = ai_w;
		aicfg.height = ai_h;
		if (ai_thresh > 0.0f)
			aicfg.obj_thresh = ai_thresh;
		if (ai_nms > 0.0f)
			aicfg.nms_thresh = ai_nms;

		printf("AI takes its own channel: %s at %ux%u BG3P, %s\n",
		       aicfg.dev, aicfg.width, aicfg.height, aicfg.kmodel);

		/*
		 * A missing kmodel or a busy channel is not a reason to take
		 * the camera down with it: detection is on because it is the
		 * default, not because this run was about it. The error from
		 * ai_face_start() has already been printed.
		 */
		ai = ai_face_start(&aicfg);
		if (ai == NULL) {
			printf("carrying on without face detection\n");
			want_ai = false;
			want_burn = false;
		} else if (screen.osd == NULL && !want_burn) {
			printf("no screen to outline faces on, the count is "
			       "printed at the end instead\n");
		}
	}

	if (want_burn) {
		/*
		 * The engine is given coordinates and works out where each
		 * line of the picture starts from the width alone, so a
		 * camera that pads its lines would come out sheared. Same
		 * again: say what is happening and leave the boxes on the
		 * panel rather than stop.
		 */
		if (cam.stride != cam.width) {
			printf("note: %s pads its lines to %u bytes for a %u "
			       "pixel picture and the 2D engine cannot draw on "
			       "that, so the boxes stay out of the stream. A "
			       "--width the driver takes unpadded gets them "
			       "back.\n",
			       dev, cam.stride, cam.width);
			want_burn = false;
		}
	}

	if (want_burn) {
		osd = osd2d_open(NULL, cam.width, cam.height);
		if (osd == NULL) {
			printf("carrying on with the boxes on the panel "
			       "only\n");
			want_burn = false;
		}
	}

	if (want_burn) {
		/*
		 * Hand the engine the camera's own frames, by the handles the
		 * capture already exported. Nothing is allocated for it and
		 * nothing is copied into it: it draws on the frame the encoder
		 * is about to read, which is how the RTOS firmware on this
		 * chip has always done it.
		 */
		{
			int fds2d[MAX_BUFFERS];
			unsigned i;

			for (i = 0; i < cam.count; i++)
				fds2d[i] = cam.bufs[i].dmabuf_fd;

			if (osd2d_attach(osd, fds2d, cam.count,
					 cam.frame_size) < 0)
				goto cleanup;
		}

		printf("burning the boxes into the encoder's frames with %s\n",
		       osd2d_dev(osd));
	}

	if (cam.fd >= 0 && cam_start(&cam) < 0)
		goto cleanup;

	if (pcam.fd >= 0 && cam_start(&pcam) < 0)
		goto cleanup;

	memset(fds, 0, sizeof(fds));
	nfds = 0;

	if (cam.fd >= 0) {
		cam_slot = (int)nfds;
		fds[nfds].fd = cam.fd;
		fds[nfds].events = POLLIN;
		nfds++;
	}

	if (pcam.fd >= 0) {
		pcam_slot = (int)nfds;
		fds[nfds].fd = pcam.fd;
		fds[nfds].events = POLLIN;
		nfds++;
	}

	/* Only here to wake the loop as soon as a packet is ready; the drain
	 * below runs unconditionally, so revents is never looked at */
	if (enc != NULL) {
		fds[nfds].fd = kd_enc_get_fd(enc);
		fds[nfds].events = POLLIN;
		nfds++;
	}

	if (screen.disp != NULL) {
		drm_slot = (int)nfds;
		fds[nfds].fd = screen.disp->fd;
		fds[nfds].events = POLLIN;
		nfds++;
	}

	start = now_sec();

	/*
	 * Primed here rather than at the top of main() so that the first
	 * CPU percentage covers the running camera and not the sensor and
	 * encoder setup, which is a burst of work nobody is asking about.
	 */
	if (stats_interval > 0) {
		sysstat_init(&stats);
		next_stats = start + (double)stats_interval;
	}

	while (!g_stop) {
		if (poll(fds, nfds, 100) < 0) {
			if (errno == EINTR)
				continue;
			break;
		}

		if (drm_slot >= 0 && (fds[drm_slot].revents & POLLIN) != 0)
			screen_vsync(&screen);

		/*
		 * Before the frames are drained, because that is where
		 * screen_show() commits: display_update_buffer() only adds
		 * the overlay to the pending atomic request, and the commit
		 * the video plane does flushes both at once. Doing it the
		 * other way round would leave the boxes a frame behind.
		 *
		 * Only when the AI has something new to say - which is once
		 * per inference, not once per camera frame, so most frames
		 * leave the overlay exactly as it is and cost nothing.
		 *
		 * Note that an inference finding nothing still counts as
		 * something new, so a camera looking at an empty room clears
		 * the plane again every time round. Measured at 15 fps on
		 * the 480x800 panel that costs no frames, so it is left
		 * alone.
		 */
		if (ai != NULL && screen.osd != NULL) {
			unsigned long gen = ai_face_generation(ai);

			if (gen != ai_drawn_gen) {
				unsigned char *at = (unsigned char *)
							    screen.osd->map +
						    (size_t)screen.y *
							    screen.osd->stride +
						    (size_t)screen.x * 4;

				ai_face_draw(ai, at, screen.w, screen.h,
					     screen.osd->stride, rotation);
				ai_face_flush(screen.osd->map,
					      screen.osd->size);
				display_update_buffer(screen.osd, 0, 0);

				ai_drawn_gen = gen;
				ai_osd_updates++;
			}
		}

		while (cam_slot >= 0 &&
		       (fds[cam_slot].revents & POLLIN) != 0) {
			unsigned index;
			int64_t pts;

			if (cam_dqbuf(&cam, &index, &pts) < 0)
				break; /* -EAGAIN, the camera is drained */

			frames++;

			/* Ours until every consumer below has let go */
			frame_get(&pool, index);

			/*
			 * The 2D engine draws straight onto the frame that
			 * just came off the camera - no copy, and nothing to
			 * hand back, because the picture the encoder is about
			 * to read is the one that was drawn on.
			 *
			 * Not gated on the generation counter the overlay
			 * uses: that one skips redrawing a plane that is
			 * still on screen, but every frame here is a new one
			 * and needs drawing on from scratch.
			 */
			if (osd != NULL) {
				const struct osd2d_box *boxes;
				const struct osd2d_patch *patches;
				int nbox, npatch;

				/* 0 = however many the shape builder is
				 * willing to produce, which is 32 */
				ai_face_shapes(ai, cam.width, cam.height, 0,
					       &boxes, &nbox, &patches,
					       &npatch);

				if (osd2d_apply(osd, index, boxes, nbox,
						patches, npatch) > 0)
					ai_burnt++;
			}

			/*
			 * Encode first. The encoder reads this very buffer, so
			 * it keeps a reference of its own until the hardware
			 * is done with it - see the reclaim loop below.
			 */
			if (enc != NULL) {
				frame_get(&pool, index);

				ret = kd_enc_send_frame(enc, index, pts);

				if (ret == KD_OK) {
					encoded++;
				} else {
					skipped_enc++;
					frame_put(&pool, index);
				}
			}

			frame_put(&pool, index);
		}

		while (pcam_slot >= 0 &&
		       (fds[pcam_slot].revents & POLLIN) != 0) {
			unsigned index;
			int64_t pts;

			if (cam_dqbuf(&pcam, &index, &pts) < 0)
				break;

			pframes++;

			frame_get(&ppool, index);
			screen_show(&screen, index);
			frame_put(&ppool, index);
		}

		if (enc != NULL) {
			/*
			 * A browser that joined partway through a GOP has
			 * nothing to decode and asks for a keyframe over
			 * RTCP. Answering costs one larger frame and saves
			 * the viewer a wait of up to a whole GOP staring at
			 * a black page.
			 */
			if (webrtc != NULL &&
			    webrtc_out_take_keyframe_request(webrtc))
				kd_enc_force_idr(enc);
			for (;;) {
				int64_t pts = 0;
				bool keyframe = false;

				ret = kd_enc_read_stream(enc, packet,
							 packet_cap, &pts,
							 &keyframe, 0);
				if (ret <= 0)
					break;

				emit_packet(rtsp, webrtc, packet, (size_t)ret,
					    pts, keyframe);

				packets++;
				bytes += (unsigned long long)ret;
			}

			/*
			 * Frames the VPU has finished reading. Until one shows
			 * up here the camera buffer is still being read by the
			 * hardware, so this is what releases the reference
			 * taken when the frame was sent.
			 */
			for (;;) {
				unsigned done;

				if (kd_enc_reclaim_frame(enc, &done) != KD_OK)
					break;

				frame_put(&pool, done);
			}
		}

		now = now_sec();

		/*
		 * Catch up by stepping the deadline rather than setting it
		 * from now: if a report is late - the poll above blocks for
		 * up to 100 ms, and the AI can hold the loop longer - the
		 * next one is still on the original grid, and the intervals
		 * the percentages are computed over stay even.
		 */
		if (stats_interval > 0 && now >= next_stats) {
			sysstat_report(&stats, now - start);

			do {
				next_stats += (double)stats_interval;
			} while (next_stats <= now);
		}

		if (seconds > 0 && now - start >= (double)seconds)
			break;
	}

	if (enc != NULL) {
		/* Drain: flush whatever is still inside the encoder */
		kd_enc_stop(enc);

		for (;;) {
			int64_t pts = 0;
			bool keyframe = false;

			ret = kd_enc_read_stream(enc, packet, packet_cap, &pts,
						 &keyframe, 200);
			if (ret <= 0)
				break;

			emit_packet(rtsp, webrtc, packet, (size_t)ret, pts,
				    keyframe);

			packets++;
			bytes += (unsigned long long)ret;
		}
	}

	{
		double elapsed = now_sec() - start;

		if (cam.fd >= 0)
			printf("captured %lu frames in %.1f s (%.1f fps)\n",
			       frames, elapsed,
			       elapsed > 0 ? (double)frames / elapsed : 0.0);

		if (pcam.fd >= 0)
			printf("preview captured %lu frames (%.1f fps)\n",
			       pframes,
			       elapsed > 0 ? (double)pframes / elapsed : 0.0);

		if (screen.plane != NULL)
			printf("displayed %lu, %lu skipped waiting for a "
			       "flip\n",
			       screen.shown, screen.skipped);

		if (ai != NULL) {
			unsigned long kpu = ai_face_frames(ai);

			/*
			 * The KPU rate is its own thing: it reads from its
			 * own channel and only has to keep up with itself, so
			 * it being below the camera's is normal and means
			 * nothing was held up.
			 */
			printf("AI ran %lu frames (%.1f fps), %u faces at the "
			       "end, overlay redrawn %lu times\n",
			       kpu,
			       elapsed > 0 ? (double)kpu / elapsed : 0.0,
			       ai_face_count(ai), ai_osd_updates);
		}

		if (osd != NULL) {
			struct osd2d_timing t;
			double n;

			printf("boxes burnt into %lu frames, %lu passes "
			       "through the 2D engine\n",
			       ai_burnt, osd2d_passes(osd));

			/*
			 * All of this sits between the camera and the encoder,
			 * so it is time the whole loop spends waiting - which
			 * is why it is worth knowing which part of it is big.
			 */
			osd2d_timing(osd, &t);
			n = t.frames > 0 ? (double)t.frames : 1.0;

			printf("  per frame: %.1f us in, %.1f us setting up, "
			       "%.1f us in the engine, %.1f us out "
			       "(%.1f us total, worst %lu us)\n",
			       t.copy_in / n, t.ctrls / n, t.engine / n,
			       t.copy_out / n,
			       (t.copy_in + t.ctrls + t.engine + t.copy_out) / n,
			       t.worst);
		}

		if (enc != NULL) {
			const char *dest = "nowhere";

			if (rtsp != NULL)
				dest = rtsp_out_url(rtsp);
			if (webrtc != NULL)
				dest = webrtc_out_url(webrtc);

			printf("encoded %lu frames, %lu refused, %lu packets "
			       "/ %llu bytes (%.2f Mbps) -> %s\n",
			       encoded, skipped_enc, packets, bytes,
			       elapsed > 0 ?
				       (double)bytes * 8.0 / elapsed / 1e6 :
				       0.0,
			       dest);
		}

		if (webrtc != NULL) {
			unsigned long dropped = webrtc_out_drops(webrtc);

			/*
			 * Anything other than zero is worth saying out loud:
			 * a dropped packet is a hole in the stream the
			 * browser cannot fill in, and if the hole was a
			 * keyframe the picture stays broken until the next
			 * one.
			 */
			if (dropped > 0)
				printf("WebRTC dropped %lu packets - the "
				       "sender could not keep up\n", dropped);
		}

		/*
		 * One last sample, so the figures printed next to the frame
		 * counts are the ones that belong with them. The cma
		 * failure count in particular is cumulative, so this line
		 * is the whole run's answer to "did anything fail to get
		 * memory", whether or not anyone watched it go past.
		 */
		if (stats_interval > 0)
			sysstat_report(&stats, elapsed);
	}

	rc = 0;

cleanup:
	/*
	 * Let the last flip land before anything is torn down, so the frame
	 * the CRTC is reading is not pulled out from under it.
	 */
	if (screen.disp != NULL && screen.flipping >= 0)
		display_wait_vsync(screen.disp);

	/* Stops the thread and closes its camera, so nothing is still
	 * drawing when the overlay buffer below goes away */
	ai_face_stop(ai);

	/* After it, because the shapes it hands out are its own memory */
	osd2d_close(osd);

	if (rtsp != NULL)
		rtsp_out_close(rtsp);

	/* Before the encoder goes away: the sender thread is still holding
	 * packets that came out of it */
	if (webrtc != NULL)
		webrtc_out_close(webrtc);

	free(packet);

	if (enc != NULL)
		kd_enc_destroy(enc);

	/* After the encoder, which is still pointing at it, and before the
	 * camera, which owns the dmabufs it names */
	if (enc_in != NULL)
		kd_bufgroup_destroy(enc_in);

	/* The fbs only describe the camera buffers, so drop them before the
	 * camera frees the memory they point at */
	for (i = 0; i < MAX_BUFFERS; i++)
		if (screen.fb[i] != NULL)
			display_free_buffer(screen.fb[i]);

	if (screen.osd != NULL)
		display_free_buffer(screen.osd);

	if (screen.osd_plane != NULL)
		display_free_plane(screen.osd_plane);

	if (screen.plane != NULL)
		display_free_plane(screen.plane);

	if (screen.disp != NULL)
		display_exit(screen.disp);

	cam_close(&pcam);
	cam_close(&cam);

	return rc;
}
