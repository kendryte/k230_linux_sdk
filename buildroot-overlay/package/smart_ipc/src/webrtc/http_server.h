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

#include <arpa/inet.h>
#include <stdint.h>

#define HTTP_MAX_LISTENERS 4

/**
 * HTTP response structure.
 * The handler callback fills this in; the server sends it to the client.
 */
typedef struct {
  int status;              /**< HTTP status code */
  const char* content_type; /**< MIME type (e.g. "application/sdp") */
  const char* body;        /**< Response body (must remain valid until send completes) */
  int body_len;            /**< Body length in bytes */
  char extra_headers[256]; /**< Optional complete HTTP header lines */
} http_response_t;

/**
 * HTTP request handler callback type.
 *
 * @param method    HTTP method ("GET", "POST", "OPTIONS")
 * @param path      Request target, including any query string
 * @param body      Request body (may be NULL for GET requests)
 * @param body_len  Request body length in bytes
 * @param client_ip IPv4 address of the connected HTTP client
 * @param local_ip  Local IPv4 address that accepted the HTTP connection
 * @param response  Output: handler fills this with the response
 */
typedef void (*http_request_handler_t)(const char* method, const char* path,
                                       const char* body, int body_len,
                                       const char* client_ip,
                                       const char* local_ip,
                                       http_response_t* response);

typedef int (*http_local_ip_provider_t)(char local_ips[][INET_ADDRSTRLEN],
                                        int max_ips, void* user_data);

/**
 * Start the HTTP server on the given port in a background thread.
 *
 * @param port     TCP port number to listen on
 * @param handler  Callback function for handling HTTP requests
 * @return 0 after at least one listener is active, non-zero on failure
 */
int http_server_start(int port, http_request_handler_t handler,
                      http_local_ip_provider_t provider, void* user_data);

/** Copy the IPv4 addresses used by active HTTP listeners. */
int http_server_get_local_ips(char local_ips[][INET_ADDRSTRLEN], int max_ips);

/**
 * Stop the HTTP server and wait for the server thread to finish.
 * Safe to call from the main thread during shutdown.
 */
void http_server_stop();

#endif  // HTTP_SERVER_H_
