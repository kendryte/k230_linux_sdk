/**
 * @file http_server.h
 * @brief Minimal HTTP/1.1 server interface for WebRTC signaling
 *
 * Provides a simple HTTP server that handles one request at a time
 * and dispatches to a user-provided callback. Used for WebRTC
 * SDP offer/answer exchange between the K230 device and a browser.
 */

#ifndef HTTP_SERVER_H_
#define HTTP_SERVER_H_

#include <stdint.h>

/**
 * HTTP response structure.
 * The handler callback fills this in; the server sends it to the client.
 */
typedef struct {
  int status;              /**< HTTP status code (200, 400, 404, 500) */
  const char* content_type; /**< MIME type (e.g. "application/sdp") */
  const char* body;        /**< Response body (must remain valid until send completes) */
  int body_len;            /**< Body length in bytes */
} http_response_t;

/**
 * HTTP request handler callback type.
 *
 * @param method    HTTP method ("GET", "POST", "OPTIONS")
 * @param path      URL path ("/", "/offer", "/answer")
 * @param body      Request body (may be NULL for GET requests)
 * @param body_len  Request body length in bytes
 * @param response  Output: handler fills this with the response
 */
typedef void (*http_request_handler_t)(const char* method, const char* path,
                                       const char* body, int body_len,
                                       http_response_t* response);

/**
 * Start the HTTP server on the given port in a background thread.
 *
 * @param port     TCP port number to listen on
 * @param handler  Callback function for handling HTTP requests
 * @return 0 on success, non-zero on failure (pthread_create error)
 */
int http_server_start(int port, http_request_handler_t handler);

/**
 * Stop the HTTP server and wait for the server thread to finish.
 * Safe to call from the main thread during shutdown.
 */
void http_server_stop();

#endif  // HTTP_SERVER_H_
