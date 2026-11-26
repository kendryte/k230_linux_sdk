/*
 * RTSP output for the demos: a C door onto librtsp_server
 *
 * See rtsp_out.h. There is no thread here: KdRtspServer::Start() runs the
 * live555 event loop on one of its own, and SendVideoData() only copies the
 * packet onto a queue under a mutex, so the demo's poll loop can call it
 * directly.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "rtsp_out.h"

#include <cstdio>
#include <cstdlib>
#include <new>
#include <string>

#include <rtsp_server.h>

struct rtsp_out {
	KdRtspServer server;
	std::string path;
	std::string url;
	bool running;
};

struct rtsp_out *rtsp_out_open(int port, const char *path, int h265)
{
	struct rtsp_out *r = new (std::nothrow) struct rtsp_out;

	if (r == NULL)
		return NULL;

	r->path = (path != NULL && *path != '\0') ? path : "live";
	r->running = false;

	if (r->server.Init(port) < 0) {
		fprintf(stderr,
			"starting the RTSP server on port %d failed. Is "
			"something already listening there?\n",
			port);
		delete r;
		return NULL;
	}

	SessionAttr attr;

	attr.with_video = true;
	attr.with_audio = false;
	attr.video_type = h265 ? VideoType::kVideoTypeH265 :
				 VideoType::kVideoTypeH264;

	if (r->server.CreateSession(r->path, attr) < 0) {
		fprintf(stderr, "creating the RTSP stream \"%s\" failed\n",
			r->path.c_str());
		r->server.DeInit();
		delete r;
		return NULL;
	}

	/* The URL is worked out while the session is being created */
	char *url = r->server.GetRtspUrl(r->path);

	if (url != NULL) {
		r->url = url;
		free(url);
	} else {
		r->url = "rtsp://<board ip>:" + std::to_string(port) + "/" +
			 r->path;
	}

	r->server.Start();
	r->running = true;

	return r;
}

const char *rtsp_out_url(const struct rtsp_out *r)
{
	return r != NULL ? r->url.c_str() : "";
}

int rtsp_out_send(struct rtsp_out *r, const void *data, size_t size,
		  uint64_t ts_us)
{
	if (r == NULL || !r->running)
		return -1;

	return r->server.SendVideoData(r->path,
				       static_cast<const uint8_t *>(data), size,
				       ts_us) < 0 ?
		       -1 :
		       0;
}

void rtsp_out_close(struct rtsp_out *r)
{
	if (r == NULL)
		return;

	/*
	 * Stop() stops the live555 event loop and then drops every session,
	 * in that order: live555 is single threaded, so nothing it owns can
	 * be freed while the loop is still running. DestroySession() is not
	 * called here because Stop() has already done it for every session.
	 */
	if (r->running)
		r->server.Stop();

	r->server.DeInit();

	delete r;
}
