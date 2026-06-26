#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "ice.h"
#include "mdns.h"
#include "ports.h"
#include "socket.h"
#include "utils.h"

static uint8_t ice_candidate_type_preference(IceCandidateType type) {
  switch (type) {
    case ICE_CANDIDATE_TYPE_HOST:
      return 126;
    case ICE_CANDIDATE_TYPE_SRFLX:
      return 100;
    case ICE_CANDIDATE_TYPE_RELAY:
      return 40;
    default:
      return 0;
  }
}

static uint16_t ice_candidate_local_preference(IceCandidate* candidate) {
  return candidate->addr.port;
}

static void ice_candidate_priority(IceCandidate* candidate) {
  // priority = (2^24)*(type preference) + (2^8)*(local preference) + (256 - component ID)
  candidate->priority = (1 << 24) * ice_candidate_type_preference(candidate->type) + (1 << 8) * ice_candidate_local_preference(candidate) + (256 - candidate->component);
}

void ice_candidate_create(IceCandidate* candidate, int foundation, IceCandidateType type, Address* addr) {
  memcpy(&candidate->addr, addr, sizeof(Address));
  candidate->type = type;

  snprintf(candidate->foundation, sizeof(candidate->foundation), "%d", foundation);
  // 1: RTP, 2: RTCP
  candidate->component = 1;

  ice_candidate_priority(candidate);

  snprintf(candidate->transport, sizeof(candidate->transport), "%s", "UDP");
}

void ice_candidate_to_description(IceCandidate* candidate, char* description, int length) {
  char addr_string[ADDRSTRLEN];
  char typ_raddr[128];

  memset(typ_raddr, 0, sizeof(typ_raddr));
  addr_to_string(&candidate->raddr, addr_string, sizeof(addr_string));

  switch (candidate->type) {
    case ICE_CANDIDATE_TYPE_HOST:
      snprintf(typ_raddr, sizeof(typ_raddr), "host");
      break;
    case ICE_CANDIDATE_TYPE_SRFLX:
      snprintf(typ_raddr, sizeof(typ_raddr), "srflx raddr %s rport %d", addr_string, candidate->raddr.port);
      break;
    case ICE_CANDIDATE_TYPE_RELAY:
      snprintf(typ_raddr, sizeof(typ_raddr), "relay raddr %s rport %d", addr_string, candidate->raddr.port);
      break;
    default:
      break;
  }

  addr_to_string(&candidate->addr, addr_string, sizeof(addr_string));
  snprintf(description, length, "a=candidate:%s %d %s %" PRIu32 " %s %d typ %s\r\n",
           candidate->foundation,
           candidate->component,
           candidate->transport,
           candidate->priority,
           addr_string,
           candidate->addr.port,
           typ_raddr);
}

int ice_candidate_from_description(IceCandidate* candidate, char* description, char* end) {
  char* candidate_start = description;
  uint32_t port;
  char type[16];
  char addrstring[ADDRSTRLEN];

  if (strncmp("a=", candidate_start, strlen("a=")) == 0) {
    candidate_start += strlen("a=");
  }
  candidate_start += strlen("candidate:");

  // a=candidate:448736988 1 udp 2122260223 172.17.0.1 49250 typ host generation 0 network-id 1 network-cost 50
  // a=candidate:udpcandidate 1 udp 120 192.168.1.102 8000 typ host
  if (sscanf(candidate_start, "%s %d %s %" PRIu32 " %s %" PRIu32 " typ %s",
             candidate->foundation,
             &candidate->component,
             candidate->transport,
             &candidate->priority,
             addrstring,
             &port,
             type) != 7) {
    LOGE("Failed to parse ICE candidate description");
    return -1;
  }

  if (strncmp(candidate->transport, "UDP", 3) != 0 && strncmp(candidate->transport, "udp", 3) != 0) {
    LOGE("Only UDP transport is supported");
    return -1;
  }

  if (strncmp(type, "host", 4) == 0) {
    candidate->type = ICE_CANDIDATE_TYPE_HOST;
  } else if (strncmp(type, "srflx", 5) == 0) {
    candidate->type = ICE_CANDIDATE_TYPE_SRFLX;
  } else if (strncmp(type, "relay", 5) == 0) {
    candidate->type = ICE_CANDIDATE_TYPE_RELAY;
  } else {
    LOGE("Unknown candidate type: %s", type);
    return -1;
  }

  addr_set_port(&candidate->addr, port);

  if (strstr(addrstring, "local") != NULL) {
    LOGW("mDNS candidate %s skipped (device cannot resolve .local names)", addrstring);
    LOGW("Disable browser mDNS privacy to expose real IPs:");
    LOGW("  Chrome/Edge: --disable-features=WebRtcHideLocalIpsWithMdns");
    LOGW("  Chrome/Edge: or set *://flags/#enable-webrtc-hide-local-ips-with-mdns to Disabled");
    return -1;
  } else if (addr_from_string(addrstring, &candidate->addr) == 0) {
    return -1;
  }

  if (candidate->type == ICE_CANDIDATE_TYPE_SRFLX || candidate->type == ICE_CANDIDATE_TYPE_RELAY) {
    char* raddr_pos = strstr(candidate_start, "raddr");
    char* rport_pos = strstr(candidate_start, "rport");
    if (raddr_pos && rport_pos && raddr_pos < end && rport_pos < end) {
      char raddr_str[ADDRSTRLEN];
      uint32_t rport_val;
      if (sscanf(raddr_pos + 6, "%s", raddr_str) == 1) {
        addr_from_string(raddr_str, &candidate->raddr);
        addr_set_family(&candidate->raddr, candidate->addr.family);
      }
      if (sscanf(rport_pos + 6, "%" PRIu32, &rport_val) == 1) {
        addr_set_port(&candidate->raddr, (uint16_t)rport_val);
      }
    }
  }

  return 0;
}
