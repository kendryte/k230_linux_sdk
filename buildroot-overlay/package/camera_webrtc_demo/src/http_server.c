/**
 * @file http_server.c
 * @brief Minimal HTTP/1.1 server for WebRTC signaling on K230 RT-Smart
 *
 * Design constraints:
 *   - Single-threaded: handles one connection at a time (sufficient for
 *     a single-client WebRTC demo — only one browser connects at a time)
 *   - No dynamic memory allocation for request parsing (stack buffers only)
 *   - select()+timeout based accept loop (RT-Smart does NOT unblock
 *     accept() on socket close, so blocking accept would prevent shutdown)
 *   - CORS enabled for all origins (required for browser signaling)
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
#define PATH_MAX_LEN 256      /**< Max URL path length */

/* ── Module state ────────────────────────────────────────────────── */

static int g_server_fd = -1;                    /**< Listening socket fd */
static int g_running = 0;                       /**< 1 while server is active, 0 to stop */
static http_request_handler_t g_handler = NULL; /**< User-provided request callback */

/* ── HTTP parsing ────────────────────────────────────────────────── */

/**
 * Parse a raw HTTP request buffer into method, path, body, and body_len.
 *
 * Expected format: "METHOD /path HTTP/1.1\r\n...headers...\r\n\r\nbody"
 *
 * @param buf      Raw request data (null-terminated)
 * @param buf_len  Length of data in buf
 * @param method   Output: HTTP method (e.g. "GET", "POST")
 * @param path     Output: URL path (e.g. "/offer", "/answer")
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

  /* Extract path (e.g. "/offer"), stop at query string '?' */
  i = 0;
  while (ptr < end && *ptr != ' ' && *ptr != '?' && i < PATH_MAX_LEN - 1) {
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
 * Formats the status line, Content-Type, Content-Length, and CORS headers,
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
                            : (response->status == 404) ? "Not Found"
                            : (response->status == 500) ? "Internal Server Error"
                                                        : "Unknown";

  /* Format response headers with CORS for browser signaling */
  int header_len = snprintf(send_buf, sizeof(send_buf),
                            "HTTP/1.1 %d %s\r\n"
                            "Content-Type: %s\r\n"
                            "Content-Length: %d\r\n"
                            "Access-Control-Allow-Origin: *\r\n"
                            "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
                            "Access-Control-Allow-Headers: Content-Type\r\n"
                            "\r\n",
                            response->status, status_text,
                            response->content_type ? response->content_type : "text/plain",
                            response->body_len);

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

/**
 * Send a 204 No Content response for CORS preflight (OPTIONS) requests.
 * Browsers send OPTIONS before cross-origin POST /answer.
 */
static void handle_options(int client_fd) {
  const char* resp =
      "HTTP/1.1 204 No Content\r\n"
      "Access-Control-Allow-Origin: *\r\n"
      "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
      "Access-Control-Allow-Headers: Content-Type\r\n"
      "\r\n";
  send(client_fd, resp, strlen(resp), 0);
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
 *    Only one browser client at a time for this WebRTC demo.
 *    This simplifies the signaling state machine (no concurrent offers).
 *
 * 3. SO_RCVTIMEO on client socket:
 *    5-second timeout on recv() to avoid hanging if a client connects
 *    but never sends data.
 */
static void* server_thread(void* data) {
  struct sockaddr_in server_addr;

  /* Create listening socket */
  g_server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (g_server_fd < 0) {
    perror("socket failed");
    return NULL;
  }

  /* Allow address reuse (quick restart without TIME_WAIT blocking) */
  int opt = 1;
  setsockopt(g_server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

  /* Bind to all interfaces on the specified port */
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(*(int*)data);

  if (bind(g_server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
    perror("bind failed");
    close(g_server_fd);
    g_server_fd = -1;
    return NULL;
  }

  if (listen(g_server_fd, 5) < 0) {
    perror("listen failed");
    close(g_server_fd);
    g_server_fd = -1;
    return NULL;
  }

  printf("HTTP server listening on port %d\n", *(int*)data);

  /* ── Accept loop with select() timeout ── */
  while (g_running) {
    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(g_server_fd, &read_fds);
    struct timeval sel_tv = {1, 0};  /* 1 second timeout */
    int sel_ret = select(g_server_fd + 1, &read_fds, NULL, NULL, &sel_tv);

    if (sel_ret <= 0) continue;  /* Timeout or error — check g_running again */

    /* Accept the incoming connection */
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int client_fd = accept(g_server_fd, (struct sockaddr*)&client_addr, &client_len);

    if (client_fd < 0) {
      if (!g_running) break;
      continue;
    }

    /* If server is stopping, reject the connection immediately */
    if (!g_running) {
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
      if (strcmp(method, "OPTIONS") == 0) {
        handle_options(client_fd);
      } else if (g_handler) {
        http_response_t response = {200, "text/plain", "OK", 2};
        g_handler(method, path, body, body_len, &response);
        send_response(client_fd, &response);
      }
    }

    close(client_fd);
  }

  /* Clean up the listening socket */
  if (g_server_fd >= 0) {
    close(g_server_fd);
    g_server_fd = -1;
  }

  return NULL;
}

/* ── Public API ──────────────────────────────────────────────────── */

static pthread_t g_server_thread;

/**
 * Start the HTTP server on the given port.
 *
 * @param port     TCP port number to listen on
 * @param handler  Callback function for handling HTTP requests
 * @return 0 on success (pthread_create return value)
 */
int http_server_start(int port, http_request_handler_t handler) {
  static int s_port;  /* Static so it persists after this function returns */
  s_port = port;
  g_handler = handler;
  g_running = 1;
  return pthread_create(&g_server_thread, NULL, server_thread, &s_port);
}

/**
 * Stop the HTTP server and wait for the server thread to finish.
 *
 * Sets g_running=0 to break the accept loop, then closes the server
 * socket (in case select() is blocked), and joins the thread.
 *
 * ⚠ IMPORTANT: The server socket is closed HERE (not in the signal
 * handler) because close() from a signal handler is NOT async-signal-safe.
 * The select() timeout ensures the loop exits within 1 second even
 * if the close() doesn't wake select() on this platform.
 */
void http_server_stop() {
  g_running = 0;
  if (g_server_fd >= 0) {
    close(g_server_fd);
    g_server_fd = -1;
  }
  pthread_join(g_server_thread, NULL);
}
