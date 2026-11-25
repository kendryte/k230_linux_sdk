#include <srtp2/srtp.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "peer.h"
#include "sctp.h"
#include "utils.h"

int peer_init() {
  srtp_err_status_t status = srtp_init();
  if (status != srtp_err_status_ok) {
    LOGE("libsrtp init failed: %d", status);
    return -1;
  }
  sctp_usrsctp_init();
  return 0;
}

void peer_deinit() {
  srtp_shutdown();
  sctp_usrsctp_deinit();
}

int peer_random_bytes(uint8_t* output, size_t len) {
  return utils_random_bytes(output, len);
}
