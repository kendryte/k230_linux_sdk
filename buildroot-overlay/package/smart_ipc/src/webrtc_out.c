/*
 * WebRTC output: the sessions, the signaling routes and the sender thread.
 *
 * This is sample_webrtc/main.c (see webrtc/README.md) with the MPP half
 * removed and three things changed, each for a reason that only applies on
 * Linux:
 *
 *  1. Local addresses come from getifaddrs() rather than rt-smart's
 *     netmgmt_utils_*.
 *  2. Parameter sets are recognised by scanning NAL headers rather than by
 *     trusting a K_VENC_HEADER pack type, because kd_enc_read_stream() does
 *     not tell us which packet is which - see find_parameter_sets().
 *  3. webrtc_out_send() queues and returns instead of sending inline, because
 *     demo_ipc calls it from the same loop that drives the camera and the
 *     screen - see the queue comment below.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "webrtc_out.h"

#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <ifaddrs.h>
#include <inttypes.h>
#include <net/if.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#include "peer.h"

#include "webrtc/http_server.h"
#include "webrtc/web_page.h"

#define MAX_WEBRTC_CLIENTS 4
#define NEGOTIATION_TIMEOUT_MS 30000
#define SESSION_ID_HEX_LEN 32
#define PEER_NEGOTIATION_POLL_US 1000
#define PEER_CONNECTED_POLL_US 10000
#define PEER_IDLE_POLL_US 50000

/*
 * Queue depth. Sixteen packets is about half a second at 30 fps, which is far
 * more slack than the sender needs: it only ever waits on g_sessions_mutex,
 * and the longest that is held is one peer_connection_loop() pass. The depth
 * is here to absorb a DTLS handshake on another connection, not to buffer.
 */
#define QUEUE_PACKETS 16

/* ------------------------------------------------------------------ */
/* Sessions                                                            */
/* ------------------------------------------------------------------ */

typedef struct {
	PeerConnection *pc;
	PeerConnectionState state;
	char id[SESSION_ID_HEX_LEN + 1];
	uint64_t last_activity_ms;
	int reserved;
	char client_ip[INET_ADDRSTRLEN];
	char local_ip[INET_ADDRSTRLEN];
} WebRtcSession;

static WebRtcSession g_sessions[MAX_WEBRTC_CLIENTS];
static pthread_mutex_t g_sessions_mutex = PTHREAD_MUTEX_INITIALIZER;
static MediaCodec g_video_codec = CODEC_H264;

/*
 * http_request_handler_t carries no user_data, so the routes reach the
 * sessions through file statics, exactly as the rt-smart sample does. One
 * server per process is therefore the limit, and webrtc_out_open() says so
 * rather than letting a second one quietly share these.
 */
static struct webrtc_out *g_out;

/* ------------------------------------------------------------------ */
/* Packet queue                                                        */
/* ------------------------------------------------------------------ */

struct packet {
	uint8_t *buf;
	size_t cap;	/* allocated */
	size_t size;	/* used */
	uint64_t ts_us;
	int keyframe;
};

struct webrtc_out {
	char url[64];

	pthread_t sender;
	pthread_t poller;
	int sender_started;
	int poller_started;
	int http_started;

	/* Queue, guarded by lock */
	pthread_mutex_t lock;
	pthread_cond_t cond;
	struct packet slot[QUEUE_PACKETS];
	unsigned head, tail, count;
	/* Volatile because the poller thread reads it outside w->lock: it
	 * never touches the queue, so taking that lock just to see a flag
	 * would be the only reason it ever contends with the sender. */
	volatile int stop;
	unsigned long drops;

	/* Cached parameter sets, only the sender thread touches these */
	uint8_t *param_sets;
	size_t param_sets_cap;	/* allocated */
	size_t param_sets_len;	/* used */
	int logged_first_packet;

	/* Set from a libpeer callback, cleared by demo_ipc */
	volatile int keyframe_wanted;
};

/* ------------------------------------------------------------------ */
/* Small helpers, copied from the rt-smart sample                      */
/* ------------------------------------------------------------------ */

static uint64_t get_time_ms(void)
{
	struct timespec ts;

	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (uint64_t)ts.tv_sec * 1000 + (uint64_t)ts.tv_nsec / 1000000;
}

static int session_is_terminal(PeerConnectionState state)
{
	return state == PEER_CONNECTION_CLOSED ||
	       state == PEER_CONNECTION_FAILED ||
	       state == PEER_CONNECTION_DISCONNECTED;
}

/* Read time only while holding g_sessions_mutex, after any activity update. */
static int session_negotiation_expired(const WebRtcSession *session,
				       uint64_t now)
{
	return (session->state == PEER_CONNECTION_NEW ||
		session->state == PEER_CONNECTION_CHECKING ||
		session->state == PEER_CONNECTION_CONNECTED) &&
	       now >= session->last_activity_ms &&
	       now - session->last_activity_ms > NEGOTIATION_TIMEOUT_MS;
}

/** Destroy a session. The caller must hold g_sessions_mutex. */
static void session_destroy_locked(WebRtcSession *session)
{
	if (session->pc) {
		peer_connection_close(session->pc);
		peer_connection_destroy(session->pc);
		session->pc = NULL;
	}
	session->state = PEER_CONNECTION_CLOSED;
	session->id[0] = '\0';
	session->last_activity_ms = 0;
	session->reserved = 0;
	session->client_ip[0] = '\0';
	session->local_ip[0] = '\0';
}

/** Find a session by ID. The caller must hold g_sessions_mutex. */
static WebRtcSession *session_find(const char *id)
{
	for (int i = 0; i < MAX_WEBRTC_CLIENTS; i++) {
		WebRtcSession *session = &g_sessions[i];

		if (session->pc && strcmp(session->id, id) == 0)
			return session;
	}
	return NULL;
}

/** Reserve a free or expired slot. The caller must hold g_sessions_mutex. */
static WebRtcSession *session_available(void)
{
	uint64_t now = get_time_ms();

	for (int i = 0; i < MAX_WEBRTC_CLIENTS; i++) {
		WebRtcSession *session = &g_sessions[i];

		if ((!session->pc && !session->reserved) ||
		    session_is_terminal(session->state) ||
		    session_negotiation_expired(session, now)) {
			if (session->pc)
				session_destroy_locked(session);
			return session;
		}
	}
	return NULL;
}

static int session_matches_client(const WebRtcSession *session,
				  const char *client_ip, const char *local_ip)
{
	if (!client_ip || strcmp(session->client_ip, client_ip) != 0)
		return 0;

	if (local_ip && local_ip[0] != '\0' &&
	    strcmp(local_ip, "0.0.0.0") != 0 &&
	    strcmp(session->local_ip, local_ip) != 0)
		return 0;

	return 1;
}

static int secure_random_hex(char *output, size_t hex_len)
{
	static const char hex[] = "0123456789abcdef";
	uint8_t random[32];
	size_t byte_len = hex_len / 2;

	if (!output || hex_len == 0 || (hex_len & 1) != 0 ||
	    byte_len > sizeof(random))
		return -1;

	if (peer_random_bytes(random, byte_len) != 0)
		return -1;

	for (size_t i = 0; i < byte_len; i++) {
		output[i * 2] = hex[random[i] >> 4];
		output[i * 2 + 1] = hex[random[i] & 0x0f];
	}
	output[hex_len] = '\0';
	return 0;
}

static int path_matches_route(const char *path, const char *route)
{
	size_t route_len = strlen(route);

	return strncmp(path, route, route_len) == 0 &&
	       (path[route_len] == '\0' || path[route_len] == '?');
}

static int query_parameter(const char *path, const char *key, char *output,
			   size_t output_size)
{
	const char *item = strchr(path, '?');
	size_t key_len = strlen(key);

	if (!item || output_size == 0)
		return 0;

	item++;
	while (*item) {
		const char *item_end = strchr(item, '&');
		const char *equals;
		size_t value_len;

		if (!item_end)
			item_end = item + strlen(item);

		equals = memchr(item, '=', (size_t)(item_end - item));
		if (equals && (size_t)(equals - item) == key_len &&
		    strncmp(item, key, key_len) == 0) {
			value_len = (size_t)(item_end - equals - 1);
			if (value_len == 0 || value_len >= output_size)
				return 0;
			memcpy(output, equals + 1, value_len);
			output[value_len] = '\0';
			return 1;
		}
		if (*item_end == '\0')
			break;
		item = item_end + 1;
	}
	return 0;
}

static int parse_session_id(const char *path, char *session_id)
{
	if (!query_parameter(path, "session", session_id,
			     SESSION_ID_HEX_LEN + 1) ||
	    strlen(session_id) != SESSION_ID_HEX_LEN)
		return 0;

	for (size_t i = 0; i < SESSION_ID_HEX_LEN; i++)
		if (!isxdigit((unsigned char)session_id[i]))
			return 0;

	return 1;
}

/** Caller must hold g_sessions_mutex. */
static int session_generate_id(char *session_id)
{
	for (int attempt = 0; attempt < 8; attempt++) {
		if (secure_random_hex(session_id, SESSION_ID_HEX_LEN) != 0)
			return -1;
		if (!session_find(session_id))
			return 0;
	}
	return -1;
}

/** Called while g_sessions_mutex is held. */
static void onconnectionstatechange(PeerConnectionState state, void *data)
{
	WebRtcSession *session = (WebRtcSession *)data;

	printf("webrtc: session %s state: %s\n", session->id,
	       peer_connection_state_to_string(state));
	session->state = state;
}

/*
 * A browser that joins mid-GOP has nothing to decode until the next I-frame,
 * and asks for one over RTCP. The rt-smart sample ignores this because its
 * VENC is not easily nudged; here kd_enc_force_idr() is one call away, so the
 * request is passed out through webrtc_out_take_keyframe_request() and the
 * picture appears at once instead of after up to a whole GOP.
 */
static void on_request_keyframe(void *userdata)
{
	(void)userdata;

	if (g_out)
		g_out->keyframe_wanted = 1;
}

/** Replace mDNS host candidates with the HTTP client's IPv4 address.
 *
 * Browsers may hide their LAN address behind a .local mDNS name. libpeer
 * cannot resolve that name, but the signaling server already knows the
 * browser's address from the accepted TCP connection. Only the candidate
 * address field is changed; ICE credentials and candidate ports are kept.
 */
static int replace_mdns_candidates(char *sdp, size_t capacity,
				   const char *client_ip)
{
	static const char candidate_prefix[] = "a=candidate:";
	const size_t client_ip_len = client_ip ? strlen(client_ip) : 0;
	char *line = sdp;
	int replaced = 0;

	if (!sdp || !client_ip_len)
		return 0;

	while (*line) {
		char *line_end = strstr(line, "\r\n");

		if (!line_end)
			line_end = line + strlen(line);

		if ((size_t)(line_end - line) > sizeof(candidate_prefix) - 1 &&
		    strncmp(line, candidate_prefix,
			    sizeof(candidate_prefix) - 1) == 0) {
			char *address_start = line;

			/* Skip foundation, component, transport, priority. */
			for (int field = 0;
			     field < 4 && address_start < line_end; field++) {
				address_start = memchr(address_start, ' ',
						       line_end - address_start);
				if (!address_start)
					break;
				while (address_start < line_end &&
				       *address_start == ' ')
					address_start++;
			}

			if (address_start && address_start < line_end) {
				char *address_end =
					memchr(address_start, ' ',
					       line_end - address_start);
				size_t address_len;

				if (!address_end)
					address_end = line_end;

				address_len = address_end - address_start;
				if (address_len > 6 &&
				    strncasecmp(address_end - 6, ".local",
						6) == 0) {
					size_t sdp_len = strlen(sdp);
					size_t tail_len =
						sdp_len -
						(address_end - sdp) + 1;
					size_t new_sdp_len = sdp_len -
							     address_len +
							     client_ip_len;

					if (new_sdp_len + 1 > capacity)
						return -1;

					memmove(address_start + client_ip_len,
						address_end, tail_len);
					memcpy(address_start, client_ip,
					       client_ip_len);
					line_end +=
						(ptrdiff_t)client_ip_len -
						(ptrdiff_t)address_len;
					replaced++;
				}
			}
		}

		if (!*line_end)
			break;
		line = line_end + 2;
	}

	return replaced;
}

/* ------------------------------------------------------------------ */
/* Local addresses                                                     */
/* ------------------------------------------------------------------ */

/*
 * The rt-smart sample asks netmgmt for the SoftAP, STA and LAN addresses in
 * that order. getifaddrs() is the Linux equivalent and needs no such list:
 * every up, non-loopback IPv4 interface is offered, in kernel order, which on
 * this board is wlan0 and nothing else.
 */
static int collect_local_ips(char local_ips[][INET_ADDRSTRLEN], int max_ips,
			     void *user_data)
{
	struct ifaddrs *list = NULL, *ifa;
	int count = 0;

	(void)user_data;

	if (getifaddrs(&list) != 0) {
		fprintf(stderr, "webrtc: getifaddrs failed: %s\n",
			strerror(errno));
		return 0;
	}

	for (ifa = list; ifa != NULL && count < max_ips; ifa = ifa->ifa_next) {
		const struct sockaddr_in *sin;
		char ip[INET_ADDRSTRLEN];
		int duplicate = 0;

		if (ifa->ifa_addr == NULL ||
		    ifa->ifa_addr->sa_family != AF_INET)
			continue;
		if (!(ifa->ifa_flags & IFF_UP) || (ifa->ifa_flags & IFF_LOOPBACK))
			continue;

		sin = (const struct sockaddr_in *)(const void *)ifa->ifa_addr;
		if (sin->sin_addr.s_addr == 0)
			continue;
		if (!inet_ntop(AF_INET, &sin->sin_addr, ip, sizeof(ip)))
			continue;

		for (int i = 0; i < count; i++)
			if (strcmp(local_ips[i], ip) == 0)
				duplicate = 1;
		if (duplicate)
			continue;

		snprintf(local_ips[count], INET_ADDRSTRLEN, "%s", ip);
		count++;
	}

	freeifaddrs(list);
	return count;
}

/* ------------------------------------------------------------------ */
/* Parameter sets                                                      */
/* ------------------------------------------------------------------ */

/*
 * WebRTC wants VPS/SPS/PPS in front of every I-frame, because a browser that
 * joins mid-stream never saw the ones at the start.
 *
 * kd_enc_read_stream() hands over whatever the encoder produced without
 * saying what is in it, and the K230 driver is free to emit the parameter
 * sets either as their own packet or glued to the front of each IDR. So
 * rather than guess, every packet is walked for parameter-set NAL units: any
 * that are found are copied out and cached, and a keyframe that does not
 * already carry them gets the cache sent ahead of it. Both layouts work, and
 * a driver that changes its mind halfway also works.
 *
 * The walk is start-code hunting over a packet that was just memcpy'd, so it
 * costs nothing worth measuring.
 */

/* Offset of the start code at or after @from, or @size if there is none.
 * On success *@sc_len is 3 or 4. */
static size_t next_start_code(const uint8_t *d, size_t size, size_t from,
			      size_t *sc_len)
{
	for (size_t i = from; i + 3 <= size; i++) {
		if (d[i] != 0 || d[i + 1] != 0)
			continue;
		if (d[i + 2] == 1) {
			*sc_len = 3;
			return i;
		}
		if (i + 4 <= size && d[i + 2] == 0 && d[i + 3] == 1) {
			*sc_len = 4;
			return i;
		}
	}
	return size;
}

static int nal_is_parameter_set(uint8_t first_byte, int h265)
{
	if (h265) {
		unsigned type = (first_byte >> 1) & 0x3f;

		return type == 32 || type == 33 || type == 34; /* VPS/SPS/PPS */
	}
	return (first_byte & 0x1f) == 7 || (first_byte & 0x1f) == 8; /* SPS/PPS */
}

/*
 * Copy every parameter-set NAL unit of @data, start codes included, into
 * *@out. Returns the number of bytes written, 0 if there were none. *@out is
 * grown as needed and owned by the caller.
 */
static size_t find_parameter_sets(const uint8_t *data, size_t size, int h265,
				  uint8_t **out, size_t *out_cap)
{
	size_t written = 0;
	size_t sc_len = 0;
	size_t pos = next_start_code(data, size, 0, &sc_len);

	while (pos < size) {
		size_t nal_start = pos + sc_len;
		size_t next_sc_len = 0;
		size_t next = next_start_code(data, size, nal_start, &next_sc_len);

		if (nal_start < size &&
		    nal_is_parameter_set(data[nal_start], h265)) {
			size_t run = next - pos;

			if (written + run > *out_cap) {
				size_t want = written + run;
				uint8_t *grown = realloc(*out, want);

				if (!grown)
					return written;
				*out = grown;
				*out_cap = want;
			}
			memcpy(*out + written, data + pos, run);
			written += run;
		}

		pos = next;
		sc_len = next_sc_len;
	}

	return written;
}

/* Does the packet already start with a parameter set? */
static int starts_with_parameter_set(const uint8_t *data, size_t size, int h265)
{
	size_t sc_len = 0;
	size_t pos = next_start_code(data, size, 0, &sc_len);

	return pos < size && pos + sc_len < size &&
	       nal_is_parameter_set(data[pos + sc_len], h265);
}

/* ------------------------------------------------------------------ */
/* Worker threads                                                      */
/* ------------------------------------------------------------------ */

/** Runs peer_connection_loop() frequently during negotiation, then reduces
 *  the polling rate once sessions are established or idle. */
static void *peer_connection_task(void *data)
{
	struct webrtc_out *w = data;

	while (!w->stop) {
		unsigned int poll_delay_us = PEER_IDLE_POLL_US;

		pthread_mutex_lock(&g_sessions_mutex);
		for (int i = 0; i < MAX_WEBRTC_CLIENTS; i++) {
			WebRtcSession *session = &g_sessions[i];
			uint64_t now;

			if (!session->pc)
				continue;

			peer_connection_loop(session->pc);
			now = get_time_ms();
			if (session_is_terminal(session->state) ||
			    session_negotiation_expired(session, now)) {
				if (!session_is_terminal(session->state))
					printf("webrtc: session %s signaling "
					       "timeout after %" PRIu64 " ms\n",
					       session->id,
					       now - session->last_activity_ms);
				session_destroy_locked(session);
				continue;
			}

			if (session->state == PEER_CONNECTION_CHECKING ||
			    session->state == PEER_CONNECTION_CONNECTED) {
				poll_delay_us = PEER_NEGOTIATION_POLL_US;
			} else if (session->state == PEER_CONNECTION_COMPLETED) {
				if (poll_delay_us > PEER_CONNECTED_POLL_US)
					poll_delay_us = PEER_CONNECTED_POLL_US;
			}
		}
		pthread_mutex_unlock(&g_sessions_mutex);

		usleep(poll_delay_us);
	}
	return NULL;
}

static void send_to_sessions(struct webrtc_out *w, const struct packet *p)
{
	int h265 = g_video_codec == CODEC_H265;
	int inline_sets = starts_with_parameter_set(p->buf, p->size, h265);
	size_t found;

	/* Refresh the cache only when this packet actually carried some: a
	 * plain P-frame must not wipe it. */
	found = find_parameter_sets(p->buf, p->size, h265, &w->param_sets,
				    &w->param_sets_cap);
	if (found > 0)
		w->param_sets_len = found;

	if (!w->logged_first_packet) {
		w->logged_first_packet = 1;
		if (found > 0 && inline_sets)
			printf("webrtc: encoder puts %zu bytes of parameter "
			       "sets in front of each keyframe\n", found);
		else if (found > 0)
			printf("webrtc: encoder emits %zu bytes of parameter "
			       "sets in their own packet\n", found);
		else
			printf("webrtc: first packet carries no parameter "
			       "sets, waiting for them\n");
	}

	pthread_mutex_lock(&g_sessions_mutex);
	for (int i = 0; i < MAX_WEBRTC_CLIENTS; i++) {
		WebRtcSession *session = &g_sessions[i];

		if (!session->pc || session->state != PEER_CONNECTION_COMPLETED)
			continue;

		/* Parameter sets and the frame go out separately. Each peer
		 * owns an independent RTP sequence/timestamp state and SRTP
		 * context. */
		if (p->keyframe && w->param_sets_len > 0 && !inline_sets)
			peer_connection_send_video(session->pc, w->param_sets,
						   w->param_sets_len,
						   p->ts_us);

		peer_connection_send_video(session->pc, p->buf, p->size,
					   p->ts_us);
	}
	pthread_mutex_unlock(&g_sessions_mutex);
}

static void *sender_task(void *data)
{
	struct webrtc_out *w = data;
	struct packet scratch = { 0 };

	for (;;) {
		struct packet *slot;
		uint8_t *tmp_buf;
		size_t tmp_cap;

		pthread_mutex_lock(&w->lock);
		while (w->count == 0 && !w->stop)
			pthread_cond_wait(&w->cond, &w->lock);
		if (w->count == 0) {
			pthread_mutex_unlock(&w->lock);
			break;
		}

		/* Swap buffers with the slot rather than copying: the ring
		 * keeps an allocation it can reuse, and the data is ours to
		 * read once the lock is dropped. */
		slot = &w->slot[w->tail];
		tmp_buf = scratch.buf;
		tmp_cap = scratch.cap;
		scratch = *slot;
		slot->buf = tmp_buf;
		slot->cap = tmp_cap;
		slot->size = 0;

		w->tail = (w->tail + 1) % QUEUE_PACKETS;
		w->count--;
		pthread_mutex_unlock(&w->lock);

		send_to_sessions(w, &scratch);
	}

	free(scratch.buf);
	return NULL;
}

/* ------------------------------------------------------------------ */
/* Signaling routes                                                    */
/* ------------------------------------------------------------------ */

static void reply(http_response_t *response, int status, const char *text)
{
	response->status = status;
	response->content_type = "text/plain";
	response->body = text;
	response->body_len = text != NULL ? (int)strlen(text) : 0;
}

static void on_http_request(const char *method, const char *path,
			    const char *body, int body_len,
			    const char *client_ip, const char *local_ip,
			    http_response_t *response)
{
	const char *request_local_ip =
		local_ip && local_ip[0] != '\0' &&
				strcmp(local_ip, "0.0.0.0") != 0
			? local_ip
			: NULL;

	if (strcmp(method, "OPTIONS") == 0) {
		response->status = 204;
		response->content_type = "text/plain";
		response->body = NULL;
		response->body_len = 0;
		return;
	}

	/* -- Serve web page -- */
	if (strcmp(method, "GET") == 0 &&
	    (path_matches_route(path, "/") ||
	     path_matches_route(path, "/index.html"))) {
		response->status = 200;
		response->content_type = "text/html; charset=utf-8";
		response->body = WEB_PAGE_HTML;
		response->body_len = strlen(WEB_PAGE_HTML);

	/* -- Create SDP offer -- */
	} else if (strcmp(method, "GET") == 0 &&
		   path_matches_route(path, "/offer")) {
		WebRtcSession *session;
		PeerConnection *new_pc;
		PeerConfiguration config = { 0 };
		const char *offer;
		/* HTTP requests are serialized; keep the response independent
		 * of peer lifetime once the session lock is released. */
		static char offer_response[8192];

		if (!client_ip || client_ip[0] == '\0' || !request_local_ip) {
			reply(response, 500,
			      "Failed to determine signaling interface");
			return;
		}

		pthread_mutex_lock(&g_sessions_mutex);
		session = session_available();
		if (!session) {
			pthread_mutex_unlock(&g_sessions_mutex);
			reply(response, 503, "Maximum WebRTC clients reached");
			return;
		}

		if (session_generate_id(session->id) != 0) {
			pthread_mutex_unlock(&g_sessions_mutex);
			reply(response, 500, "Failed to create session ID");
			return;
		}
		session->last_activity_ms = get_time_ms();
		session->reserved = 1;
		session->state = PEER_CONNECTION_NEW;
		snprintf(session->client_ip, sizeof(session->client_ip), "%s",
			 client_ip);
		snprintf(session->local_ip, sizeof(session->local_ip), "%s",
			 request_local_ip);
		pthread_mutex_unlock(&g_sessions_mutex);

		config.datachannel = DATA_CHANNEL_NONE;
		config.video_codec = g_video_codec;
		config.audio_codec = CODEC_NONE;
		config.on_request_keyframe = on_request_keyframe;
		config.user_data = session;
		new_pc = peer_connection_create(&config);
		if (!new_pc) {
			pthread_mutex_lock(&g_sessions_mutex);
			session_destroy_locked(session);
			pthread_mutex_unlock(&g_sessions_mutex);
			reply(response, 500, "Failed to create WebRTC peer");
			return;
		}
		if (peer_connection_set_local_ip(new_pc, session->local_ip) != 0) {
			peer_connection_destroy(new_pc);
			pthread_mutex_lock(&g_sessions_mutex);
			session_destroy_locked(session);
			pthread_mutex_unlock(&g_sessions_mutex);
			reply(response, 500, "Failed to bind local ICE address");
			return;
		}

		/* Build the new peer before publishing it to the worker and
		 * sender threads, so existing sessions continue during
		 * certificate generation. */
		offer = peer_connection_create_offer(new_pc);
		if (!offer || strlen(offer) >= sizeof(offer_response)) {
			peer_connection_destroy(new_pc);
			pthread_mutex_lock(&g_sessions_mutex);
			session_destroy_locked(session);
			pthread_mutex_unlock(&g_sessions_mutex);
			reply(response, 500, "Failed to create offer");
			return;
		}

		memcpy(offer_response, offer, strlen(offer) + 1);

		pthread_mutex_lock(&g_sessions_mutex);
		peer_connection_oniceconnectionstatechange(
			new_pc, onconnectionstatechange);
		session->pc = new_pc;
		session->reserved = 0;
		session->last_activity_ms = get_time_ms();

		response->status = 200;
		response->content_type = "application/sdp";
		response->body = offer_response;
		response->body_len = strlen(offer_response);
		snprintf(response->extra_headers,
			 sizeof(response->extra_headers),
			 "X-WebRTC-Session: %s\r\n", session->id);
		printf("webrtc: session %s using local ICE address %s for HTTP "
		       "client %s\n",
		       session->id, session->local_ip, session->client_ip);
		pthread_mutex_unlock(&g_sessions_mutex);

	/* -- Set remote SDP answer -- */
	} else if (strcmp(method, "POST") == 0 &&
		   strncmp(path, "/answer?", strlen("/answer?")) == 0) {
		static const char prefix[] = "a=candidate:";
		char session_id[SESSION_ID_HEX_LEN + 1];
		WebRtcSession *session;
		size_t candidate_count = 0;
		size_t answer_capacity;
		const char *candidate, *body_end;
		char *answer_copy;
		int mdns_replaced;

		if (!parse_session_id(path, session_id)) {
			reply(response, 400, "Missing or invalid session ID");
			return;
		}
		if (!body || body_len <= 0) {
			reply(response, 400, "Missing SDP body");
			return;
		}

		pthread_mutex_lock(&g_sessions_mutex);
		session = session_find(session_id);
		if (!session) {
			pthread_mutex_unlock(&g_sessions_mutex);
			reply(response, 404, "WebRTC session not found");
			return;
		}
		if (!session_matches_client(session, client_ip, local_ip)) {
			pthread_mutex_unlock(&g_sessions_mutex);
			reply(response, 403, "WebRTC session client mismatch");
			return;
		}

		/* Null-terminate the answer SDP for libpeer and leave enough
		 * room to replace short mDNS hostnames with an IPv4 address if
		 * necessary. body is not guaranteed to be null-terminated (it
		 * points into the receive buffer at the start of the body
		 * section), so count the candidates with a scan bounded by
		 * body_len rather than one that stops at a NUL. */
		candidate = body;
		body_end = body + body_len;
		while ((size_t)(body_end - candidate) >= sizeof(prefix) - 1) {
			const char *hit;

			hit = memchr(candidate, prefix[0],
				     (size_t)(body_end - candidate) -
					     (sizeof(prefix) - 2));
			if (hit == NULL)
				break;

			if (memcmp(hit, prefix, sizeof(prefix) - 1) == 0) {
				candidate_count++;
				candidate = hit + sizeof(prefix) - 1;
			} else {
				candidate = hit + 1;
			}
		}

		answer_capacity =
			(size_t)body_len + candidate_count * INET_ADDRSTRLEN + 1;
		answer_copy = calloc(1, answer_capacity);
		if (!answer_copy) {
			pthread_mutex_unlock(&g_sessions_mutex);
			reply(response, 500, "Out of memory");
			return;
		}
		memcpy(answer_copy, body, body_len);
		answer_copy[body_len] = '\0';

		mdns_replaced = replace_mdns_candidates(answer_copy,
							answer_capacity,
							client_ip);
		if (mdns_replaced < 0) {
			free(answer_copy);
			pthread_mutex_unlock(&g_sessions_mutex);
			reply(response, 500, "Failed to process SDP");
			return;
		}
		if (mdns_replaced > 0)
			printf("webrtc: replaced %d mDNS ICE candidate(s) with "
			       "HTTP client IP %s\n",
			       mdns_replaced, client_ip);

		peer_connection_set_remote_description(session->pc, answer_copy,
						       SDP_TYPE_ANSWER);
		if (peer_connection_get_state(session->pc) ==
		    PEER_CONNECTION_FAILED) {
			free(answer_copy);
			session_destroy_locked(session);
			pthread_mutex_unlock(&g_sessions_mutex);
			reply(response, 400, "Invalid SDP answer");
			return;
		}
		session->last_activity_ms = get_time_ms();
		pthread_mutex_unlock(&g_sessions_mutex);

		reply(response, 200, "OK");
		free(answer_copy);

	/* -- Release a WebRTC session -- */
	} else if (strcmp(method, "POST") == 0 &&
		   strncmp(path, "/disconnect?", strlen("/disconnect?")) == 0) {
		char session_id[SESSION_ID_HEX_LEN + 1];
		WebRtcSession *session;

		if (!parse_session_id(path, session_id)) {
			reply(response, 400, "Missing or invalid session ID");
			return;
		}
		pthread_mutex_lock(&g_sessions_mutex);
		session = session_find(session_id);
		if (!session) {
			pthread_mutex_unlock(&g_sessions_mutex);
			reply(response, 200, "OK");
			return;
		}
		if (!session_matches_client(session, client_ip, local_ip)) {
			pthread_mutex_unlock(&g_sessions_mutex);
			reply(response, 403, "WebRTC session client mismatch");
			return;
		}
		printf("webrtc: session %s disconnected by HTTP client %s\n",
		       session->id, session->client_ip);
		session_destroy_locked(session);
		pthread_mutex_unlock(&g_sessions_mutex);
		reply(response, 200, "OK");

	/* -- 404 for everything else -- */
	} else {
		reply(response, 404, "Not Found");
	}
}

/* ------------------------------------------------------------------ */
/* Public interface                                                    */
/* ------------------------------------------------------------------ */

struct webrtc_out *webrtc_out_open(int port, int h265)
{
	struct webrtc_out *w;
	char ips[HTTP_MAX_LISTENERS][INET_ADDRSTRLEN];
	int nips;

	if (g_out != NULL) {
		fprintf(stderr, "webrtc: only one server at a time\n");
		return NULL;
	}

	w = calloc(1, sizeof(*w));
	if (w == NULL) {
		fprintf(stderr, "webrtc: out of memory\n");
		return NULL;
	}
	pthread_mutex_init(&w->lock, NULL);
	pthread_cond_init(&w->cond, NULL);

	g_video_codec = h265 ? CODEC_H265 : CODEC_H264;
	g_out = w;

	if (peer_init() != 0) {
		fprintf(stderr, "webrtc: peer_init failed\n");
		goto fail;
	}

	if (http_server_start(port, on_http_request, collect_local_ips, NULL) != 0) {
		fprintf(stderr, "webrtc: no HTTP listener on port %d\n", port);
		goto fail;
	}
	w->http_started = 1;

	if (pthread_create(&w->poller, NULL, peer_connection_task, w) != 0) {
		fprintf(stderr, "webrtc: cannot start the ICE thread: %s\n",
			strerror(errno));
		goto fail;
	}
	w->poller_started = 1;

	if (pthread_create(&w->sender, NULL, sender_task, w) != 0) {
		fprintf(stderr, "webrtc: cannot start the sender thread: %s\n",
			strerror(errno));
		goto fail;
	}
	w->sender_started = 1;

	/* Whichever address the browser uses is decided per connection by the
	 * http_server; this is only what gets printed, so take the first. */
	nips = http_server_get_local_ips(ips, HTTP_MAX_LISTENERS);
	snprintf(w->url, sizeof(w->url), "http://%s:%d/",
		 nips > 0 ? ips[0] : "<board ip>", port);

	return w;

fail:
	webrtc_out_close(w);
	return NULL;
}

const char *webrtc_out_url(const struct webrtc_out *w)
{
	return w->url;
}

int webrtc_out_send(struct webrtc_out *w, const void *data, size_t size,
		    uint64_t ts_us, int keyframe)
{
	struct packet *slot;
	int dropped = 0;

	if (w == NULL || data == NULL || size == 0)
		return 0;

	pthread_mutex_lock(&w->lock);

	/* Full: drop the oldest rather than the newest, so that what is in
	 * flight stays as close to live as possible. */
	if (w->count == QUEUE_PACKETS) {
		w->tail = (w->tail + 1) % QUEUE_PACKETS;
		w->count--;
		w->drops++;
		dropped = 1;
	}

	slot = &w->slot[w->head];
	if (slot->cap < size) {
		uint8_t *grown = realloc(slot->buf, size);

		if (grown == NULL) {
			pthread_mutex_unlock(&w->lock);
			return -1;
		}
		slot->buf = grown;
		slot->cap = size;
	}
	memcpy(slot->buf, data, size);
	slot->size = size;
	slot->ts_us = ts_us;
	slot->keyframe = keyframe;

	w->head = (w->head + 1) % QUEUE_PACKETS;
	w->count++;

	pthread_mutex_unlock(&w->lock);
	pthread_cond_signal(&w->cond);

	return dropped ? -1 : 0;
}

unsigned long webrtc_out_drops(const struct webrtc_out *w)
{
	return w != NULL ? w->drops : 0;
}

int webrtc_out_take_keyframe_request(struct webrtc_out *w)
{
	int wanted;

	if (w == NULL)
		return 0;

	wanted = w->keyframe_wanted;
	w->keyframe_wanted = 0;
	return wanted;
}

void webrtc_out_close(struct webrtc_out *w)
{
	if (w == NULL)
		return;

	/*
	 * Order matters, and it is the rt-smart sample's: stop taking new
	 * connections first, then the threads that walk the sessions, and only
	 * then free the sessions out from under nobody.
	 */
	pthread_mutex_lock(&w->lock);
	w->stop = 1;
	pthread_mutex_unlock(&w->lock);
	pthread_cond_broadcast(&w->cond);

	if (w->http_started)
		http_server_stop();
	if (w->sender_started)
		pthread_join(w->sender, NULL);
	if (w->poller_started)
		pthread_join(w->poller, NULL);

	pthread_mutex_lock(&g_sessions_mutex);
	for (int i = 0; i < MAX_WEBRTC_CLIENTS; i++)
		session_destroy_locked(&g_sessions[i]);
	pthread_mutex_unlock(&g_sessions_mutex);

	peer_deinit();

	for (int i = 0; i < QUEUE_PACKETS; i++)
		free(w->slot[i].buf);
	free(w->param_sets);

	pthread_cond_destroy(&w->cond);
	pthread_mutex_destroy(&w->lock);

	g_out = NULL;
	free(w);
}
