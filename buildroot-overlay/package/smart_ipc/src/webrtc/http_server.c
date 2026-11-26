/**
 * @file http_server.c
 * @brief Minimal HTTP/1.1 server for WebRTC signaling on K230 RT-Smart
 *
 * Design constraints:
 *   - Single-threaded request processing; WebRTC sessions continue in
 *     independent peer connections after signaling completes
 *   - No dynamic memory allocation for request parsing (stack buffers only)
 *   - select()+timeout based accept loop (RT-Smart does NOT unblock
 *     accept() on socket close, so blocking accept would prevent shutdown)
 *   - Signaling is same-origin; cross-origin browser access is not enabled
 *
 * Limitations:
 *   - Request body limited to RECV_BUF_SIZE (8KB)
 *   - No HTTP keep-alive (connection closed after each response)
 *   - No chunked transfer encoding support
 *   - send() return values not checked (acceptable for LAN demo)
 */

#include <errno.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/time.h>

#include "http_server.h"

/* ── Utility ─────────────────────────────────────────────────────── */

/**
 * Case-insensitive substring search.
 * strcasestr is a GNU extension not available in musl libc on RT-Smart,
 * so we provide a portable implementation using only standard C.
 */
static const char* my_strcasestr(const char* haystack, const char* needle) {
  if (!*needle) return haystack;
  for (; *haystack; haystack++) {
    const char *h = haystack, *n = needle;
    while (*h && *n && tolower((unsigned char)*h) == tolower((unsigned char)*n)) {
      h++;
      n++;
    }
    if (!*n) return haystack;
  }
  return NULL;
}

/* ── Constants ───────────────────────────────────────────────────── */

#define RECV_BUF_SIZE 8192    /**< Max bytes to read from a single request */
#define SEND_BUF_SIZE 16384   /**< Max bytes for response header + chunk */
#define METHOD_MAX_LEN 8      /**< Max HTTP method length (GET/POST/OPTIONS) */
#define PATH_MAX_LEN 256      /**< Max request-target length, including query */

/* ── Module state ────────────────────────────────────────────────── */

typedef struct { int fd; char ip[INET_ADDRSTRLEN]; } HttpListener;
static HttpListener g_listeners[HTTP_MAX_LISTENERS];
static int g_listener_count = 0;
static int g_running = 0;                         /**< 1 while server is active, 0 to stop */
static http_request_handler_t g_handler = NULL;   /**< User-provided request callback */
static http_local_ip_provider_t g_ip_provider = NULL;
static void* g_ip_provider_data = NULL;
static pthread_mutex_t g_start_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t g_start_cond = PTHREAD_COND_INITIALIZER;
static int g_start_complete = 0;
static int g_start_result = -1;

static int server_is_running(void) {
  return __atomic_load_n(&g_running, __ATOMIC_ACQUIRE);
}

static void server_set_running(int running) {
  __atomic_store_n(&g_running, running, __ATOMIC_RELEASE);
}

static void server_report_start(int result) {
  pthread_mutex_lock(&g_start_mutex);
  g_start_result = result;
  g_start_complete = 1;
  pthread_cond_signal(&g_start_cond);
  pthread_mutex_unlock(&g_start_mutex);
}

/* ── HTTP parsing ────────────────────────────────────────────────── */

/**
 * Parse a raw HTTP request buffer into method, path, body, and body_len.
 *
 * Expected format: "METHOD /path HTTP/1.1\r\n...headers...\r\n\r\nbody"
 *
 * @param buf      Raw request data (null-terminated)
 * @param buf_len  Length of data in buf
 * @param method   Output: HTTP method (e.g. "GET", "POST")
 * @param path     Output: request target (e.g. "/answer?session=1234")
 * @param body     Output: pointer into buf at the start of the body
 * @param body_len Output: length of the body in bytes
 * @return 0 on success, -1 on parse error
 */
static int parse_http_request(const char* buf, int buf_len, char* method, char* path,
                              char** body, int* body_len) {
  const char *ptr = buf, *end = buf + buf_len;

  if (ptr >= end) return -1;

  /* Extract method (e.g. "GET") */
  int i = 0;
  while (ptr < end && *ptr != ' ' && i < METHOD_MAX_LEN - 1) {
    method[i++] = *ptr++;
  }
  method[i] = '\0';
  if (ptr >= end || *ptr != ' ') return -1;
  ptr++;

  /* Extract the complete request target, including any query string. */
  i = 0;
  while (ptr < end && *ptr != ' ' && i < PATH_MAX_LEN - 1) {
    path[i++] = *ptr++;
  }
  path[i] = '\0';

  /* Find end of headers to locate body */
  const char* header_end = strstr(buf, "\r\n\r\n");
  if (!header_end) return -1;

  int header_len = (header_end - buf) + 4;
  *body = (char*)(buf + header_len);
  *body_len = buf_len - header_len;
  if (*body_len < 0) *body_len = 0;

  return 0;
}

/* ── Response sending ────────────────────────────────────────────── */

/**
 * Send an HTTP response to the client.
 *
 * Formats the status line, Content-Type, Content-Length, and security headers,
 * then sends the header followed by the body in SEND_BUF_SIZE chunks.
 *
 * ⚠ NOTE: send() return values are not checked. For a LAN demo this is
 * acceptable — if the connection breaks, the next recv() will fail and
 * we'll close the socket. For production code, partial sends should be
 * retried.
 */
static void send_response(int client_fd, http_response_t* response) {
  char send_buf[SEND_BUF_SIZE];

  /* Map status code to reason phrase */
  const char* status_text = (response->status == 200)   ? "OK"
                            : (response->status == 204) ? "No Content"
                            : (response->status == 400) ? "Bad Request"
                            : (response->status == 403) ? "Forbidden"
                            : (response->status == 404) ? "Not Found"
                            : (response->status == 503) ? "Service Unavailable"
                            : (response->status == 500) ? "Internal Server Error"
                                                        : "Unknown";

  /* The embedded page uses same-origin signaling, so CORS is intentionally
   * omitted. This prevents unrelated web sites from driving the camera API. */
  int header_len = snprintf(send_buf, sizeof(send_buf),
                            "HTTP/1.1 %d %s\r\n"
                            "Content-Type: %s\r\n"
                            "Content-Length: %d\r\n"
                            "Cache-Control: no-store\r\n"
                            "X-Content-Type-Options: nosniff\r\n"
                            "Referrer-Policy: no-referrer\r\n"
                            "Content-Security-Policy: default-src 'self'; "
                            "script-src 'self' 'unsafe-inline'; "
                            "style-src 'self' 'unsafe-inline'; "
                            "media-src 'self' blob:; connect-src 'self'; "
                            "frame-ancestors 'none'\r\n"
                            "Connection: close\r\n"
                            "%s"
                            "\r\n",
                            response->status, status_text,
                            response->content_type ? response->content_type : "text/plain",
                            response->body_len, response->extra_headers);

  send(client_fd, send_buf, header_len, 0);

  /* Send body in chunks (for large SDP offers that exceed SEND_BUF_SIZE) */
  if (response->body && response->body_len > 0) {
    int offset = 0;
    while (offset < response->body_len) {
      int chunk = response->body_len - offset;
      if (chunk > SEND_BUF_SIZE) chunk = SEND_BUF_SIZE;
      send(client_fd, response->body + offset, chunk, 0);
      offset += chunk;
    }
  }
}

/* ── Server thread ───────────────────────────────────────────────── */

/**
 * Main server loop: accept connections, parse HTTP, dispatch to handler.
 *
 * Key design decisions for RT-Smart compatibility:
 *
 * 1. select() + 1s timeout instead of blocking accept():
 *    RT-Smart does NOT unblock accept() when the socket is closed
 *    (unlike Linux where close() from another thread wakes accept).
 *    Using select() with timeout lets us check g_running every second.
 *
 * 2. Single-threaded request handling:
 *    Signaling requests are serialized while established WebRTC sessions
 *    continue concurrently in the sample's peer worker.
 *
 * 3. SO_RCVTIMEO on client socket:
 *    5-second timeout on recv() to avoid hanging if a client connects
 *    but never sends data.
 */
static void* server_thread(void* data) {
  int port = *(int*)data;
  char ips[HTTP_MAX_LISTENERS][INET_ADDRSTRLEN];
  g_listener_count = 0;
  memset(ips, 0, sizeof(ips));
  int count = g_ip_provider ? g_ip_provider(ips, HTTP_MAX_LISTENERS, g_ip_provider_data) : 0;
  if (g_ip_provider && count <= 0) {
    fprintf(stderr, "HTTP server has no usable local IPv4 address\n");
    server_set_running(0);
    server_report_start(-1);
    return NULL;
  }
  if (count <= 0) count = 1;
  for (int i = 0; i < count && i < HTTP_MAX_LISTENERS; i++) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) continue;
    int opt = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = (count == 1 && ips[0][0] == '\0') ? htonl(INADDR_ANY) : inet_addr(ips[i]);
    if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
      fprintf(stderr, "HTTP bind failed for %s:%d: %s\n",
              ips[i][0] ? ips[i] : "0.0.0.0", port, strerror(errno));
      close(fd);
      continue;
    }
    if (listen(fd, 5) < 0) {
      fprintf(stderr, "HTTP listen failed for %s:%d: %s\n",
              ips[i][0] ? ips[i] : "0.0.0.0", port, strerror(errno));
      close(fd);
      continue;
    }
    g_listeners[g_listener_count].fd = fd;
    const char* bind_ip = count == 1 && ips[0][0] == '\0' ? "0.0.0.0" : ips[i];
    strncpy(g_listeners[g_listener_count].ip, bind_ip, INET_ADDRSTRLEN - 1);
    g_listeners[g_listener_count].ip[INET_ADDRSTRLEN - 1] = '\0';
    g_listener_count++;
  }
  if (g_listener_count == 0) {
    server_set_running(0);
    server_report_start(-1);
    return NULL;
  }

  printf("HTTP server listening on port %d\n", port);
  server_report_start(0);

  /* ── Accept loop with select() timeout ── */
  while (server_is_running()) {
    fd_set read_fds;
    FD_ZERO(&read_fds);
    int max_fd = -1;
    for (int i = 0; i < g_listener_count; i++) {
      FD_SET(g_listeners[i].fd, &read_fds);
      if (g_listeners[i].fd > max_fd) max_fd = g_listeners[i].fd;
    }
    struct timeval sel_tv = {1, 0};  /* 1 second timeout */
    int sel_ret = select(max_fd + 1, &read_fds, NULL, NULL, &sel_tv);

    if (sel_ret <= 0) continue;  /* Timeout or error; check running again. */

    /* Accept the incoming connection */
    int listener = -1;
    for (int i = 0; i < g_listener_count; i++) if (FD_ISSET(g_listeners[i].fd, &read_fds)) { listener = i; break; }
    if (listener < 0) continue;
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int client_fd = accept(g_listeners[listener].fd, (struct sockaddr*)&client_addr, &client_len);

    if (client_fd < 0) {
      if (!server_is_running()) break;
      continue;
    }

    /* If server is stopping, reject the connection immediately */
    if (!server_is_running()) {
      close(client_fd);
      break;
    }

    /* Set 5s recv timeout to avoid hanging on dead connections */
    struct timeval tv = {5, 0};
    setsockopt(client_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    /* ── Read the full HTTP request ──
     * We read incrementally until we have:
     *   - The complete header section (ends with \r\n\r\n)
     *   - The complete body (Content-Length bytes after headers)
     * Or until the buffer is full. */
    char recv_buf[RECV_BUF_SIZE];
    int total = 0;
    int n;

    while (total < RECV_BUF_SIZE - 1) {
      n = recv(client_fd, recv_buf + total, RECV_BUF_SIZE - 1 - total, 0);
      if (n <= 0) break;
      total += n;
      recv_buf[total] = '\0';

      /* Check if we have the complete headers */
      if (strstr(recv_buf, "\r\n\r\n")) {
        const char* header_end = strstr(recv_buf, "\r\n\r\n");
        int header_len = (header_end - recv_buf) + 4;

        /* If Content-Length is present, wait until we have the full body.
         * ⚠ NOTE: cl_str + 15 skips "Content-Length:" — this assumes
         * exactly one space after the colon. Variants like
         * "Content-Length:  123" (double space) would parse incorrectly.
         * Acceptable for this controlled signaling use case. */
        const char* cl_str = my_strcasestr(recv_buf, "Content-Length:");
        if (cl_str) {
          int content_length = atoi(cl_str + 15);
          if (total < header_len + content_length) continue;
        }
        break;
      }
    }

    if (total <= 0) {
      close(client_fd);
      continue;
    }

    recv_buf[total] = '\0';

    /* ── Parse and dispatch ── */
    char method[METHOD_MAX_LEN] = {0};
    char path[PATH_MAX_LEN] = {0};
    char* body = NULL;
    int body_len = 0;

    if (parse_http_request(recv_buf, total, method, path, &body, &body_len) == 0) {
      if (g_handler) {
        char client_ip[INET_ADDRSTRLEN] = {0};
        char local_ip[INET_ADDRSTRLEN] = {0};
        struct sockaddr_in local_addr;
        socklen_t local_len = sizeof(local_addr);
        http_response_t response = {
          .status = 200,
          .content_type = "text/plain",
          .body = "OK",
          .body_len = 2,
        };
        if (!inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip))) {
          client_ip[0] = '\0';
        }
        memset(&local_addr, 0, sizeof(local_addr));
        if (getsockname(client_fd, (struct sockaddr*)&local_addr, &local_len) != 0 ||
            local_addr.sin_family != AF_INET ||
            !inet_ntop(AF_INET, &local_addr.sin_addr, local_ip, sizeof(local_ip))) {
          snprintf(local_ip, sizeof(local_ip), "%s", g_listeners[listener].ip);
        }
        g_handler(method, path, body, body_len, client_ip, local_ip, &response);
        send_response(client_fd, &response);
      }
    }

    close(client_fd);
  }

  /* Clean up the listening socket */
  for (int i = 0; i < g_listener_count; i++) close(g_listeners[i].fd);
  g_listener_count = 0;

  return NULL;
}

/* ── Public API ──────────────────────────────────────────────────── */

static pthread_t g_server_thread;

/**
 * Start the HTTP server on the given port.
 *
 * @param port     TCP port number to listen on
 * @param handler  Callback function for handling HTTP requests
 * @return 0 after at least one listener is active, non-zero on failure
 */
int http_server_start(int port, http_request_handler_t handler,
                      http_local_ip_provider_t provider, void* user_data) {
  static int s_port;
  int result;

  s_port = port;
  g_handler = handler;
  g_ip_provider = provider;
  g_ip_provider_data = user_data;
  pthread_mutex_lock(&g_start_mutex);
  g_start_complete = 0;
  g_start_result = -1;
  pthread_mutex_unlock(&g_start_mutex);
  server_set_running(1);

  result = pthread_create(&g_server_thread, NULL, server_thread, &s_port);
  if (result != 0) {
    server_set_running(0);
    return result;
  }

  pthread_mutex_lock(&g_start_mutex);
  while (!g_start_complete) {
    pthread_cond_wait(&g_start_cond, &g_start_mutex);
  }
  result = g_start_result;
  pthread_mutex_unlock(&g_start_mutex);
  if (result != 0) {
    pthread_join(g_server_thread, NULL);
  }
  return result;
}

int http_server_get_local_ips(char local_ips[][INET_ADDRSTRLEN], int max_ips) {
  if (!local_ips || max_ips <= 0) {
    return 0;
  }

  int count = g_listener_count < max_ips ? g_listener_count : max_ips;
  for (int i = 0; i < count; i++) {
    snprintf(local_ips[i], INET_ADDRSTRLEN, "%s", g_listeners[i].ip);
  }
  return count;
}

/**
 * Stop the HTTP server and wait for the server thread to finish.
 *
 * Sets g_running=0 to break the accept loop and joins the thread. The server
 * thread owns and closes all listening descriptors.
 *
 * The select() timeout ensures the loop exits within one second without
 * closing descriptors concurrently from another thread.
 */
void http_server_stop() {
  server_set_running(0);
  pthread_join(g_server_thread, NULL);
}
