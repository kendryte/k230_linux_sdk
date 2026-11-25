#ifndef PEER_H_
#define PEER_H_

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "peer_connection.h"
#include "peer_signaling.h"

int peer_init();

void peer_deinit();

/** Fill output using the platform's cryptographic RNG; no peer_init required. */
int peer_random_bytes(uint8_t* output, size_t len);

#ifdef __cplusplus
}
#endif

#endif  // PEER_H_
