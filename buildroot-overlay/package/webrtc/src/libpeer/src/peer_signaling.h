#ifndef PEER_SIGNALING_H_
#define PEER_SIGNALING_H_

#include <cJSON.h>
#include "peer_connection.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef DISABLE_PEER_SIGNALING

/*
 * Custom RPC method handler for application-specific methods
 * not recognized by the built-in signaling layer (offer/answer/state/close).
 *
 * @param method   The RPC method name (e.g. "list_recordings")
 * @param params   The "params" field as a cJSON object (may be NULL)
 * @param id       The JSON-RPC request id
 * @param result   Output: set to a cJSON result object (caller takes ownership)
 * @param error    Output: set to a cJSON error object (caller takes ownership)
 * @param userdata User-provided context pointer
 *
 * @return 0 if the method was handled (*result or *error must be set),
 *         -1 if not handled (signaling layer will return METHOD_NOT_FOUND)
 *
 * Thread safety: called from the signaling thread (peer_signaling_loop).
 * Implementations must be thread-safe with respect to application state.
 */
typedef int (*peer_signaling_custom_rpc_cb)(const char* method, cJSON* params, int id,
                                            cJSON** result, cJSON** error, void* userdata);

int peer_signaling_connect(const char* url, const char* token, PeerConnection* pc);

void peer_signaling_disconnect();

int peer_signaling_loop();

/*
 * Register a callback for handling custom RPC methods.
 * Must be called after peer_signaling_connect().
 * Only one handler can be registered at a time; subsequent calls replace the previous.
 * Pass NULL to unregister.
 */
void peer_signaling_set_custom_rpc_handler(peer_signaling_custom_rpc_cb cb, void* userdata);

/*
 * Publish a message to an arbitrary MQTT topic via the signaling layer's
 * existing MQTT connection. Reuses the connection established by
 * peer_signaling_connect(), avoiding the need for a separate MQTT client.
 *
 * Only works in MQTT mode (proto == 0). Returns -1 for HTTP mode or if
 * the MQTT connection is not yet established.
 *
 * @param topic    MQTT topic to publish to (e.g. "/devices/heartbeat")
 * @param message  Message payload (JSON string)
 * @return 0 on success, -1 on failure
 */
int peer_signaling_publish(const char* topic, const char* message);

#endif  // DISABLE_PEER_SIGNALING

#ifdef __cplusplus
}
#endif

#endif  // PEER_SIGNALING_H_
