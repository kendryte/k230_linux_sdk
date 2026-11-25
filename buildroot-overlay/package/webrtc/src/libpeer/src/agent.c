#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "agent.h"
#include "base64.h"
#include "ice.h"
#include "ports.h"
#include "socket.h"
#include "stun.h"
#include "utils.h"

#define AGENT_CONNCHECK_MAX 100
#define AGENT_CONNCHECK_INTERVAL_MS 50
#define AGENT_STUN_RECV_MAXTIMES 1000

static int agent_is_private_ipv4(Address* addr) {
  uint32_t ip;
  if (addr->family != AF_INET) return 0;
  ip = ntohl(addr->sin.sin_addr.s_addr);
  if ((ip >> 24) == 10) return 1;                              /* 10.0.0.0/8 */
  if ((ip >> 20) == ((172 << 4) | 1)) return 1;                /* 172.16.0.0/12 */
  if ((ip >> 16) == ((192 << 8) | 168)) return 1;              /* 192.168.0.0/16 */
  return 0;
}

static int agent_candidate_pair_is_impossible(Agent* agent, IceCandidate* local, IceCandidate* remote) {
  int local_private, remote_private;

  if (local->addr.family != AF_INET || remote->addr.family != AF_INET) return 0;

  local_private = agent_is_private_ipv4(&local->addr);
  remote_private = agent_is_private_ipv4(&remote->addr);

  /* Rule 1: Remote is a private host, but local is NOT a private host.
   * Public internet / TURN server / srflx cannot reach private addresses.
   * However, if both are private hosts, DO NOT filter based on subnet —
   * devices may be behind the same NAT with inter-VLAN routing.
   * Let ICE connectivity checks determine actual reachability. */
  if (remote->type == ICE_CANDIDATE_TYPE_HOST && remote_private) {
    if (!(local->type == ICE_CANDIDATE_TYPE_HOST && local_private)) {
      return 1;
    }
  }

  /* Rule 2: Both srflx with same public IP - needs hairpin NAT, almost never works */
  if (local->type == ICE_CANDIDATE_TYPE_SRFLX && remote->type == ICE_CANDIDATE_TYPE_SRFLX &&
      local->addr.sin.sin_addr.s_addr == remote->addr.sin.sin_addr.s_addr) {
    return 1;
  }

  /* Rule 3 removed: Private host → srflx is NOT hairpin, it is normal outbound
   * NAT traversal. The local host sends from its private IP to the remote's
   * public srflx address, NAT translates the source — this always works.
   * Previously this was incorrectly filtered when both sides shared the
   * same public IP, blocking the optimal host→srflx path. */

  return 0;
}

void agent_clear_candidates(Agent* agent) {
  agent->local_candidates_count = 0;
  agent->remote_candidates_count = 0;
  agent->candidate_pairs_num = 0;
  agent->active_pairs_count = 0;
}

int agent_create(Agent* agent) {
  int ret;
  agent->udp_sockets[0].fd = -1;
  agent->udp_sockets[1].fd = -1;
  agent->b_host_addr = 0;
  agent->b_bound_host_addr = 0;
  memset(&agent->host_addr, 0, sizeof(agent->host_addr));
  memset(&agent->bound_host_addr, 0, sizeof(agent->bound_host_addr));

  if ((ret = udp_socket_open(&agent->udp_sockets[0], AF_INET, 0)) < 0) {
    LOGE("Failed to create UDP socket.");
    return ret;
  }
  LOGI("create IPv4 UDP socket: %d", agent->udp_sockets[0].fd);

#if CONFIG_IPV6
  if ((ret = udp_socket_open(&agent->udp_sockets[1], AF_INET6, 0)) < 0) {
    LOGE("Failed to create IPv6 UDP socket.");
    udp_socket_close(&agent->udp_sockets[0]);
    return ret;
  }
  LOGI("create IPv6 UDP socket: %d", agent->udp_sockets[1].fd);
#endif

  agent_clear_candidates(agent);
  agent->active_pairs_count = 0;
  memset(agent->remote_ufrag, 0, sizeof(agent->remote_ufrag));
  memset(agent->remote_upwd, 0, sizeof(agent->remote_upwd));
  memset(&agent->turn_server_addr, 0, sizeof(agent->turn_server_addr));
  agent->turn_relay_ready = 0;
  agent->turn_channel = 0;
  agent->turn_channel_bound = 0;
  agent->turn_allocation_time = 0;
  memset(agent->turn_username, 0, sizeof(agent->turn_username));
  memset(agent->turn_credential, 0, sizeof(agent->turn_credential));
  memset(agent->turn_nonce, 0, sizeof(agent->turn_nonce));
  memset(agent->turn_realm, 0, sizeof(agent->turn_realm));
  return 0;
}

void agent_destroy(Agent* agent) {
  if (agent->udp_sockets[0].fd >= 0) {
    udp_socket_close(&agent->udp_sockets[0]);
  }

#if CONFIG_IPV6
  if (agent->udp_sockets[1].fd >= 0) {
    udp_socket_close(&agent->udp_sockets[1]);
  }
#endif
}

static int agent_socket_recv(Agent* agent, Address* addr, uint8_t* buf, int len) {
  int ret;
  int i;
  int addr_type[] = { AF_INET,
#if CONFIG_IPV6
                      AF_INET6,
#endif
  };

  for (i = 0; i < sizeof(addr_type) / sizeof(addr_type[0]); i++) {
    if (agent->udp_sockets[i].fd < 0) {
      continue;
    }
    ret = udp_socket_recvfrom_nonblocking(&agent->udp_sockets[i], addr, buf, len);
    if (ret != 0) {
      return ret;
    }
  }

  return 0;
}

static int agent_socket_recv_attempts(Agent* agent, Address* addr, uint8_t* buf, int len, int maxtimes) {
  int ret = -1;
  int i = 0;
  for (i = 0; i < maxtimes; i++) {
    if ((ret = agent_socket_recv(agent, addr, buf, len)) != 0) {
      break;
    }
    ports_sleep_ms(1);
  }
  return ret;
}

static int agent_socket_send(Agent* agent, Address* addr, const uint8_t* buf, int len) {
  switch (addr->family) {
    case AF_INET6:
      return udp_socket_sendto(&agent->udp_sockets[1], addr, buf, len);
    case AF_INET:
    default:
      return udp_socket_sendto(&agent->udp_sockets[0], addr, buf, len);
  }
  return -1;
}

static int agent_create_host_addr(Agent* agent) {
  int i, j;
  const char* iface_prefx[] = {CONFIG_IFACE_PREFIX};
  IceCandidate* ice_candidate;
  int addr_type[] = { AF_INET,
#if CONFIG_IPV6
                      AF_INET6,
#endif
  };

  if (agent->b_host_addr) {
    for (i = 0; i < sizeof(addr_type) / sizeof(addr_type[0]); i++) {
      if (agent->host_addr.family != addr_type[i]) {
        continue;
      }

      Address candidate_addr = agent->host_addr;
      addr_set_port(&candidate_addr, agent->udp_sockets[i].bind_addr.port);
      if (agent->local_candidates_count >= AGENT_MAX_CANDIDATES) {
        LOGE("Too many local ICE candidates");
        return -1;
      }
      ice_candidate = agent->local_candidates + agent->local_candidates_count;
      ice_candidate_create(ice_candidate, agent->local_candidates_count,
                           ICE_CANDIDATE_TYPE_HOST, &candidate_addr);
      agent->local_candidates_count++;
    }
    return 0;
  }

  for (i = 0; i < sizeof(addr_type) / sizeof(addr_type[0]); i++) {
    for (j = 0; j < sizeof(iface_prefx) / sizeof(iface_prefx[0]); j++) {
      if (agent->local_candidates_count >= AGENT_MAX_CANDIDATES) {
        LOGE("Too many local ICE candidates");
        return -1;
      }
      ice_candidate = agent->local_candidates + agent->local_candidates_count;
      // only copy port and family to addr of ice candidate
      ice_candidate_create(ice_candidate, agent->local_candidates_count, ICE_CANDIDATE_TYPE_HOST,
                           &agent->udp_sockets[i].bind_addr);
      // if resolve host addr, add to local candidate
      if (ports_get_host_addr(&ice_candidate->addr, iface_prefx[j])) {
        agent->local_candidates_count++;
      }
    }
  }

  return 0;
}

static int agent_parse_host_address(const char* address, Address* host_addr) {
  memset(host_addr, 0, sizeof(*host_addr));
  if (!addr_from_string(address, host_addr)) {
    LOGE("Invalid local host address: %s", address);
    return -1;
  }
  if (host_addr->family != AF_INET) {
    LOGE("Only an IPv4 local host address is supported: %s", address);
    return -1;
  }

  return 0;
}

int agent_set_host_address(Agent* agent, const char* address) {
  Address host_addr;

  if (address == NULL || address[0] == '\0') {
    agent->b_host_addr = 0;
    memset(&agent->host_addr, 0, sizeof(agent->host_addr));
    return 0;
  }

  if (agent_parse_host_address(address, &host_addr) != 0) {
    return -1;
  }

  agent->host_addr = host_addr;
  agent->b_host_addr = 1;
  return 0;
}

int agent_bind_host_address(Agent* agent, const char* address) {
  Address host_addr;

  if (address == NULL || address[0] == '\0') {
    if (agent->b_bound_host_addr &&
        udp_socket_rebind(&agent->udp_sockets[0], NULL) != 0) {
      LOGE("Failed to restore UDP socket binding to all interfaces");
      return -1;
    }
    agent->b_bound_host_addr = 0;
    memset(&agent->bound_host_addr, 0, sizeof(agent->bound_host_addr));
    return agent_set_host_address(agent, NULL);
  }

  if (agent_parse_host_address(address, &host_addr) != 0) {
    return -1;
  }

  if (agent->b_bound_host_addr &&
      agent->bound_host_addr.sin.sin_addr.s_addr == host_addr.sin.sin_addr.s_addr) {
    agent->host_addr = host_addr;
    agent->b_host_addr = 1;
    return 0;
  }

  if (udp_socket_rebind(&agent->udp_sockets[0], &host_addr) != 0) {
    LOGE("Failed to bind ICE UDP socket to local address: %s", address);
    return -1;
  }

  agent->host_addr = host_addr;
  agent->b_host_addr = 1;
  agent->bound_host_addr = host_addr;
  agent->b_bound_host_addr = 1;
  return 0;
}

static int agent_create_stun_addr(Agent* agent, Address* serv_addr) {
  int ret = -1;
  Address bind_addr;
  StunMessage send_msg;
  StunMessage recv_msg;
  uint32_t transaction_id[3];
  memset(&send_msg, 0, sizeof(send_msg));
  memset(&recv_msg, 0, sizeof(recv_msg));

  if (stun_msg_create(&send_msg, STUN_CLASS_REQUEST | STUN_METHOD_BINDING) != 0) {
    LOGE("Failed to generate STUN transaction ID.");
    return -1;
  }
  memcpy(transaction_id, ((StunHeader*)send_msg.buf)->transaction_id,
         sizeof(transaction_id));

  ret = agent_socket_send(agent, serv_addr, send_msg.buf, send_msg.size);

  if (ret == -1) {
    LOGE("Failed to send STUN Binding Request.");
    return ret;
  }

  ret = agent_socket_recv_attempts(agent, NULL, recv_msg.buf, sizeof(recv_msg.buf), AGENT_STUN_RECV_MAXTIMES);
  if (ret <= 0) {
    LOGD("Failed to receive STUN Binding Response.");
    return ret;
  }

  if (stun_parse_msg_buf(&recv_msg, (size_t)ret) != 0) {
    LOGE("Invalid STUN Binding response");
    return -1;
  }
  if (recv_msg.stunclass != STUN_CLASS_RESPONSE ||
      recv_msg.stunmethod != STUN_METHOD_BINDING ||
      memcmp(((StunHeader*)recv_msg.buf)->transaction_id, transaction_id,
             sizeof(transaction_id)) != 0) {
    LOGE("Invalid STUN Binding response");
    return -1;
  }
  if (agent->local_candidates_count >= AGENT_MAX_CANDIDATES) {
    LOGE("Too many local ICE candidates");
    return -1;
  }
  memcpy(&bind_addr, &recv_msg.mapped_addr, sizeof(Address));
  IceCandidate* ice_candidate = agent->local_candidates + agent->local_candidates_count++;
  ice_candidate_create(ice_candidate, agent->local_candidates_count, ICE_CANDIDATE_TYPE_SRFLX, &bind_addr);
  if (agent->local_candidates_count > 0) {
    memcpy(&ice_candidate->raddr, &agent->local_candidates[0].addr, sizeof(Address));
  }
  return ret;
}

#define TURN_ALLOCATE_MAX_RETRIES 2

static int agent_create_turn_addr(Agent* agent, Address* serv_addr, const char* username, const char* credential) {
  int ret = -1;
  uint32_t attr = ntohl(0x11000000);
  Address turn_addr;
  StunMessage send_msg;
  StunMessage recv_msg;
  int attempt;

  for (attempt = 0; attempt < TURN_ALLOCATE_MAX_RETRIES; attempt++) {
    memset(&recv_msg, 0, sizeof(recv_msg));
    memset(&send_msg, 0, sizeof(send_msg));
    if (stun_msg_create(&send_msg, STUN_METHOD_ALLOCATE) != 0) {
      LOGE("Failed to generate TURN transaction ID.");
      return -1;
    }
    stun_msg_write_attr(&send_msg, STUN_ATTR_TYPE_REQUESTED_TRANSPORT, sizeof(attr), (char*)&attr);

    StunHeader* first_header = (StunHeader*)send_msg.buf;
    uint32_t first_txn_id[3];
    memcpy(first_txn_id, first_header->transaction_id, sizeof(first_txn_id));

    ret = agent_socket_send(agent, serv_addr, send_msg.buf, send_msg.size);
    if (ret == -1) {
      LOGE("Failed to send TURN Allocate Request.");
      return -1;
    }

    ret = agent_socket_recv_attempts(agent, NULL, recv_msg.buf, sizeof(recv_msg.buf), AGENT_STUN_RECV_MAXTIMES);
    if (ret <= 0) {
      LOGD("Failed to receive TURN Allocate Response.");
      return ret;
    }

    if (stun_parse_msg_buf(&recv_msg, (size_t)ret) != 0) {
      LOGE("Invalid TURN Allocate response");
      return -1;
    }

    if (recv_msg.stunclass == STUN_CLASS_ERROR && recv_msg.stunmethod == STUN_METHOD_ALLOCATE) {
      if (recv_msg.error_code == 437) {
        /* Allocation Mismatch: the TURN server still considers the 5-tuple
         * as having an active allocation.  Close and reopen the UDP socket
         * to obtain a new source port (new 5-tuple), then retry. */
        LOGE("TURN Allocate 437 (Allocation Mismatch), reopening socket (attempt %d/%d)",
             attempt + 1, TURN_ALLOCATE_MAX_RETRIES);
        if (agent_reopen_udp_socket(agent) != 0) {
          return -1;
        }
        continue;   /* retry from the top with new 5-tuple */
      }
      if (recv_msg.error_code != 401 && recv_msg.error_code != 438) {
        LOGE("TURN Allocate failed (error code: %d), not retrying.", recv_msg.error_code);
        return -1;
      }
      if (strlen(recv_msg.nonce) == 0 || strlen(recv_msg.realm) == 0) {
        LOGE("TURN Allocate 401 missing NONCE/REALM.");
        return -1;
      }
      strncpy(agent->turn_nonce, recv_msg.nonce, sizeof(agent->turn_nonce) - 1);
      strncpy(agent->turn_realm, recv_msg.realm, sizeof(agent->turn_realm) - 1);
      memset(&send_msg, 0, sizeof(send_msg));
      if (stun_msg_create(&send_msg, STUN_CLASS_REQUEST | STUN_METHOD_ALLOCATE) != 0) {
        LOGE("Failed to generate TURN transaction ID.");
        return -1;
      }
      stun_msg_write_attr(&send_msg, STUN_ATTR_TYPE_REQUESTED_TRANSPORT, sizeof(attr), (char*)&attr);
      stun_msg_write_attr(&send_msg, STUN_ATTR_TYPE_USERNAME, strlen(username), (char*)username);
      stun_msg_write_attr(&send_msg, STUN_ATTR_TYPE_NONCE, strlen(recv_msg.nonce), recv_msg.nonce);
      stun_msg_write_attr(&send_msg, STUN_ATTR_TYPE_REALM, strlen(recv_msg.realm), recv_msg.realm);
      stun_msg_finish(&send_msg, STUN_CREDENTIAL_LONG_TERM, credential, strlen(credential));
    } else if (recv_msg.stunclass == STUN_CLASS_RESPONSE && recv_msg.stunmethod == STUN_METHOD_ALLOCATE) {
      StunHeader* resp_header = (StunHeader*)recv_msg.buf;
      if (memcmp(resp_header->transaction_id, first_txn_id, sizeof(first_txn_id)) != 0) {
        LOGE("TURN Allocate response transaction ID mismatch.");
        return -1;
      }
      if (agent->local_candidates_count >= AGENT_MAX_CANDIDATES) {
        LOGE("Too many local ICE candidates");
        return -1;
      }
      memcpy(&turn_addr, &recv_msg.relayed_addr, sizeof(Address));
      IceCandidate* ice_candidate = agent->local_candidates + agent->local_candidates_count++;
      ice_candidate_create(ice_candidate, agent->local_candidates_count, ICE_CANDIDATE_TYPE_RELAY, &turn_addr);
      memcpy(&ice_candidate->raddr, &recv_msg.mapped_addr, sizeof(Address));
      memcpy(&agent->turn_server_addr, serv_addr, sizeof(Address));
      agent->turn_allocation_time = ports_get_epoch_time();
      strncpy(agent->turn_username, username, sizeof(agent->turn_username) - 1);
      strncpy(agent->turn_credential, credential, sizeof(agent->turn_credential) - 1);
      LOGI("TURN allocation succeeded (no auth needed)");
      return ret;
    } else {
      LOGE("Invalid TURN Allocate Response.");
      return -1;
    }

    /* --- Authenticated retry (after 401/438) --- */
    uint32_t retry_txn_id[3];
    {
      StunHeader* retry_header = (StunHeader*)send_msg.buf;
      memcpy(retry_txn_id, retry_header->transaction_id, sizeof(retry_txn_id));
    }

    ret = agent_socket_send(agent, serv_addr, send_msg.buf, send_msg.size);
    if (ret < 0) {
      LOGE("Failed to send TURN Allocate Request (with auth).");
      return -1;
    }

    memset(&recv_msg, 0, sizeof(recv_msg));
    ret = agent_socket_recv_attempts(agent, NULL, recv_msg.buf, sizeof(recv_msg.buf), AGENT_STUN_RECV_MAXTIMES);
    if (ret <= 0) {
      LOGD("Failed to receive TURN Allocate Response (with auth).");
      return ret;
    }

    if (stun_parse_msg_buf(&recv_msg, (size_t)ret) != 0) {
      LOGE("Invalid TURN Allocate response");
      return -1;
    }

    if (recv_msg.stunclass == STUN_CLASS_ERROR && recv_msg.stunmethod == STUN_METHOD_ALLOCATE) {
      if (recv_msg.error_code == 437) {
        LOGW("TURN Allocate 437 after auth, reopening socket (attempt %d/%d)",
             attempt + 1, TURN_ALLOCATE_MAX_RETRIES);
        if (agent_reopen_udp_socket(agent) != 0) {
          return -1;
        }
        continue;   /* retry from the top with new 5-tuple */
      }
      LOGE("TURN Allocate failed (auth error, code=%d).", recv_msg.error_code);
      return -1;
    }

    if (recv_msg.stunclass == STUN_CLASS_ERROR) {
      LOGE("TURN Allocate failed (auth error).");
      return -1;
    }

    StunHeader* retry_resp_header = (StunHeader*)recv_msg.buf;
    if (memcmp(retry_resp_header->transaction_id, retry_txn_id, sizeof(retry_txn_id)) != 0) {
      LOGE("TURN Allocate retry response transaction ID mismatch.");
      return -1;
    }

    if (agent->local_candidates_count >= AGENT_MAX_CANDIDATES) {
      LOGE("Too many local ICE candidates");
      return -1;
    }
    memcpy(&turn_addr, &recv_msg.relayed_addr, sizeof(Address));
    IceCandidate* ice_candidate = agent->local_candidates + agent->local_candidates_count++;
    ice_candidate_create(ice_candidate, agent->local_candidates_count, ICE_CANDIDATE_TYPE_RELAY, &turn_addr);
    memcpy(&ice_candidate->raddr, &recv_msg.mapped_addr, sizeof(Address));
    memcpy(&agent->turn_server_addr, serv_addr, sizeof(Address));
    agent->turn_allocation_time = ports_get_epoch_time();
    strncpy(agent->turn_username, username, sizeof(agent->turn_username) - 1);
    strncpy(agent->turn_credential, credential, sizeof(agent->turn_credential) - 1);
    LOGI("TURN allocation succeeded (with auth)");
    return ret;
  } /* end retry loop */

  LOGE("TURN Allocate failed after %d attempts (437 Allocation Mismatch).", TURN_ALLOCATE_MAX_RETRIES);
  return -1;
}

static int agent_is_relay_active(Agent* agent) {
  return agent->turn_relay_ready &&
         agent->nominated_pair &&
         agent->nominated_pair->local->type == ICE_CANDIDATE_TYPE_RELAY;
}

static int agent_turn_send_stun_with_auth(Agent* agent, StunMessage* send_msg, Address* peer_addr, const char* username, const char* credential, const uint32_t* refresh_lifetime) {
  StunMessage recv_msg;
  int ret;
  uint16_t original_method;
  int recv_attempts;

  StunHeader* orig_header = (StunHeader*)send_msg->buf;
  uint32_t orig_txn_id[3];
  memcpy(orig_txn_id, orig_header->transaction_id, sizeof(orig_txn_id));
  original_method = (ntohs(orig_header->type) & 0x3E00) >> 2 |
                    (ntohs(orig_header->type) & 0x00E0) >> 1 |
                    (ntohs(orig_header->type) & 0x000F);

  ret = agent_socket_send(agent, &agent->turn_server_addr, send_msg->buf, send_msg->size);
  if (ret < 0) {
    LOGE("Failed to send TURN request to server.");
    return -1;
  }

  for (recv_attempts = 0; recv_attempts < 10; recv_attempts++) {
    memset(&recv_msg, 0, sizeof(recv_msg));
    ret = agent_socket_recv_attempts(agent, NULL, recv_msg.buf, sizeof(recv_msg.buf), AGENT_STUN_RECV_MAXTIMES);
    if (ret <= 0) {
      LOGD("Failed to receive TURN response.");
      return -1;
    }

    if (stun_parse_msg_buf(&recv_msg, (size_t)ret) != 0) {
      LOGE("Invalid TURN response");
      return -1;
    }

    if (recv_msg.stunclass == STUN_CLASS_ERROR && recv_msg.stunmethod == original_method) {
      StunHeader* resp_header = (StunHeader*)recv_msg.buf;
      if (memcmp(resp_header->transaction_id, orig_txn_id, sizeof(orig_txn_id)) != 0) {
        LOGD("TURN response txn ID mismatch, retrying recv (%d/10)", recv_attempts + 1);
        continue;
      }
      if (recv_msg.error_code != 401 && recv_msg.error_code != 438) {
        LOGE("TURN request failed (error code: %d).", recv_msg.error_code);
        return -1;
      }
      if (strlen(recv_msg.nonce) == 0 || strlen(recv_msg.realm) == 0) {
        LOGE("TURN 401/438 missing NONCE/REALM, cannot retry.");
        return -1;
      }
      strncpy(agent->turn_nonce, recv_msg.nonce, sizeof(agent->turn_nonce) - 1);
      strncpy(agent->turn_realm, recv_msg.realm, sizeof(agent->turn_realm) - 1);
      StunMessage retry_msg;
      memset(&retry_msg, 0, sizeof(retry_msg));
      if (stun_msg_create(&retry_msg, STUN_CLASS_REQUEST | original_method) != 0) {
        LOGE("Failed to generate TURN transaction ID.");
        return -1;
      }

      StunHeader* retry_header = (StunHeader*)retry_msg.buf;
      uint32_t retry_txn_id[3];
      memcpy(retry_txn_id, retry_header->transaction_id, sizeof(retry_txn_id));

      stun_msg_write_attr(&retry_msg, STUN_ATTR_TYPE_USERNAME, strlen(username), (char*)username);
      stun_msg_write_attr(&retry_msg, STUN_ATTR_TYPE_NONCE, strlen(recv_msg.nonce), recv_msg.nonce);
      stun_msg_write_attr(&retry_msg, STUN_ATTR_TYPE_REALM, strlen(recv_msg.realm), recv_msg.realm);

      if (original_method == STUN_METHOD_CREATE_PERMISSION) {
        char peer_addr_attr[32];
        memset(peer_addr_attr, 0, sizeof(peer_addr_attr));
        uint8_t mask[16];
        *((uint32_t*)mask) = htonl(MAGIC_COOKIE);
        memcpy(mask + 4, retry_header->transaction_id, sizeof(retry_header->transaction_id));
        int addr_len = stun_set_mapped_address(peer_addr_attr, mask, peer_addr);
        stun_msg_write_attr(&retry_msg, STUN_ATTR_TYPE_XOR_PEER_ADDRESS, addr_len, peer_addr_attr);
      } else if (original_method == STUN_METHOD_CHANNEL_BIND) {
        char peer_addr_attr[32];
        memset(peer_addr_attr, 0, sizeof(peer_addr_attr));
        uint8_t mask[16];
        *((uint32_t*)mask) = htonl(MAGIC_COOKIE);
        memcpy(mask + 4, retry_header->transaction_id, sizeof(retry_header->transaction_id));
        int addr_len = stun_set_mapped_address(peer_addr_attr, mask, peer_addr);
        stun_msg_write_attr(&retry_msg, STUN_ATTR_TYPE_XOR_PEER_ADDRESS, addr_len, peer_addr_attr);
        uint8_t channel_attr[4];
        uint16_t channel_net = htons(agent->turn_channel);
        memset(channel_attr, 0, sizeof(channel_attr));
        memcpy(channel_attr, &channel_net, sizeof(channel_net));
        stun_msg_write_attr(&retry_msg, STUN_ATTR_TYPE_CHANNEL_NUMBER, sizeof(channel_attr), (char*)channel_attr);
      } else if (original_method == STUN_METHOD_REFRESH) {
        if (refresh_lifetime) {
          uint32_t lifetime_attr = *refresh_lifetime;
          stun_msg_write_attr(&retry_msg, STUN_ATTR_TYPE_LIFETIME, sizeof(lifetime_attr), (char*)&lifetime_attr);
        }
      }

      stun_msg_finish(&retry_msg, STUN_CREDENTIAL_LONG_TERM, credential, strlen(credential));

      ret = agent_socket_send(agent, &agent->turn_server_addr, retry_msg.buf, retry_msg.size);
      if (ret < 0) {
        LOGE("Failed to send TURN request (with auth).");
        return -1;
      }

      for (int retry_recv = 0; retry_recv < 10; retry_recv++) {
        memset(&recv_msg, 0, sizeof(recv_msg));
        ret = agent_socket_recv_attempts(agent, NULL, recv_msg.buf, sizeof(recv_msg.buf), AGENT_STUN_RECV_MAXTIMES);
        if (ret <= 0) {
          LOGD("Failed to receive TURN response (with auth).");
          return -1;
        }

        if (stun_parse_msg_buf(&recv_msg, (size_t)ret) != 0) {
          LOGE("Invalid TURN retry response");
          return -1;
        }

        StunHeader* retry_resp_header = (StunHeader*)recv_msg.buf;
        if (memcmp(retry_resp_header->transaction_id, retry_txn_id, sizeof(retry_txn_id)) != 0) {
          LOGD("TURN retry response txn ID mismatch, retrying recv (%d/10)", retry_recv + 1);
          continue;
        }
        break;
      }

      StunHeader* retry_resp_header = (StunHeader*)recv_msg.buf;
      if (memcmp(retry_resp_header->transaction_id, retry_txn_id, sizeof(retry_txn_id)) != 0) {
        LOGE("TURN retry response txn ID mismatch after max retries.");
        return -1;
      }
    } else {
      StunHeader* resp_header = (StunHeader*)recv_msg.buf;
      if (memcmp(resp_header->transaction_id, orig_txn_id, sizeof(orig_txn_id)) != 0) {
        LOGD("TURN response txn ID mismatch, retrying recv (%d/10)", recv_attempts + 1);
        continue;
      }
    }

    if (recv_msg.stunclass == STUN_CLASS_ERROR) {
      LOGE("TURN request failed (error code: %d).", recv_msg.error_code);
      return -1;
    }

    return 0;
  }

  LOGE("TURN: failed to get matching response after max retries.");
  return -1;
}

static int agent_turn_create_permission(Agent* agent, Address* peer_addr, const char* username, const char* credential) {
  StunMessage send_msg;
  char peer_addr_attr[32];
  memset(peer_addr_attr, 0, sizeof(peer_addr_attr));
  uint8_t mask[16];
  StunHeader* header;
  int addr_len;

  memset(&send_msg, 0, sizeof(send_msg));
  if (stun_msg_create(&send_msg, STUN_CLASS_REQUEST | STUN_METHOD_CREATE_PERMISSION) != 0) {
    LOGE("Failed to generate TURN transaction ID.");
    return -1;
  }

  header = (StunHeader*)send_msg.buf;
  *((uint32_t*)mask) = htonl(MAGIC_COOKIE);
  memcpy(mask + 4, header->transaction_id, sizeof(header->transaction_id));
  addr_len = stun_set_mapped_address(peer_addr_attr, mask, peer_addr);
  stun_msg_write_attr(&send_msg, STUN_ATTR_TYPE_XOR_PEER_ADDRESS, addr_len, peer_addr_attr);

  if (strlen(agent->turn_nonce) > 0 && strlen(agent->turn_realm) > 0) {
    stun_msg_write_attr(&send_msg, STUN_ATTR_TYPE_USERNAME, strlen(username), (char*)username);
    stun_msg_write_attr(&send_msg, STUN_ATTR_TYPE_NONCE, strlen(agent->turn_nonce), agent->turn_nonce);
    stun_msg_write_attr(&send_msg, STUN_ATTR_TYPE_REALM, strlen(agent->turn_realm), agent->turn_realm);
    stun_msg_finish(&send_msg, STUN_CREDENTIAL_LONG_TERM, credential, strlen(credential));
  } else {
    stun_msg_add_fingerprint(&send_msg);
  }

  LOGI("Sending TURN CreatePermission for peer");
  return agent_turn_send_stun_with_auth(agent, &send_msg, peer_addr, username, credential, NULL);
}

static int agent_turn_channel_bind(Agent* agent, Address* peer_addr, uint16_t channel_number, const char* username, const char* credential) {
  StunMessage send_msg;
  char peer_addr_attr[32];
  memset(peer_addr_attr, 0, sizeof(peer_addr_attr));
  uint8_t mask[16];
  StunHeader* header;
  int addr_len;
  uint8_t channel_attr[4];
  uint16_t channel_net = htons(channel_number);
  memset(channel_attr, 0, sizeof(channel_attr));
  memcpy(channel_attr, &channel_net, sizeof(channel_net));

  memset(&send_msg, 0, sizeof(send_msg));
  if (stun_msg_create(&send_msg, STUN_CLASS_REQUEST | STUN_METHOD_CHANNEL_BIND) != 0) {
    LOGE("Failed to generate TURN transaction ID.");
    return -1;
  }

  header = (StunHeader*)send_msg.buf;
  *((uint32_t*)mask) = htonl(MAGIC_COOKIE);
  memcpy(mask + 4, header->transaction_id, sizeof(header->transaction_id));
  addr_len = stun_set_mapped_address(peer_addr_attr, mask, peer_addr);
  stun_msg_write_attr(&send_msg, STUN_ATTR_TYPE_XOR_PEER_ADDRESS, addr_len, peer_addr_attr);
  stun_msg_write_attr(&send_msg, STUN_ATTR_TYPE_CHANNEL_NUMBER, sizeof(channel_attr), (char*)channel_attr);

  if (strlen(agent->turn_nonce) > 0 && strlen(agent->turn_realm) > 0) {
    stun_msg_write_attr(&send_msg, STUN_ATTR_TYPE_USERNAME, strlen(username), (char*)username);
    stun_msg_write_attr(&send_msg, STUN_ATTR_TYPE_NONCE, strlen(agent->turn_nonce), agent->turn_nonce);
    stun_msg_write_attr(&send_msg, STUN_ATTR_TYPE_REALM, strlen(agent->turn_realm), agent->turn_realm);
    stun_msg_finish(&send_msg, STUN_CREDENTIAL_LONG_TERM, credential, strlen(credential));
  } else {
    stun_msg_add_fingerprint(&send_msg);
  }

  LOGI("Sending TURN ChannelBind for channel 0x%04x", channel_number);
  return agent_turn_send_stun_with_auth(agent, &send_msg, peer_addr, username, credential, NULL);
}

int agent_turn_setup_relay(Agent* agent) {
  int i;
  IceCandidate* best_remote = NULL;

  if (agent->turn_relay_ready) {
    return 0;
  }

  if (!agent->nominated_pair || agent->nominated_pair->local->type != ICE_CANDIDATE_TYPE_RELAY) {
    return 0;
  }

  if (agent->turn_server_addr.family == 0) {
    LOGE("TURN server address not set.");
    return -1;
  }

  for (i = 0; i < agent->remote_candidates_count; i++) {
    Address* peer_addr = &agent->remote_candidates[i].addr;
    char addr_str[ADDRSTRLEN];
    addr_to_string(peer_addr, addr_str, sizeof(addr_str));
    if (peer_addr->family != agent->turn_server_addr.family) {
      LOGI("TURN skip CreatePermission for %s (address family mismatch)", addr_str);
      continue;
    }
    LOGI("TURN CreatePermission for remote %s:%d", addr_str, peer_addr->port);
    if (agent_turn_create_permission(agent, peer_addr, agent->turn_username, agent->turn_credential) < 0) {
      LOGW("TURN CreatePermission failed for %s, continuing", addr_str);
    }
  }

  for (i = 0; i < agent->remote_candidates_count; i++) {
    IceCandidate* c = &agent->remote_candidates[i];
    if (c->addr.family != agent->turn_server_addr.family) {
      continue;
    }
    if (c->type == ICE_CANDIDATE_TYPE_RELAY) {
      best_remote = c;
      break;
    }
    if (c->type == ICE_CANDIDATE_TYPE_SRFLX && (!best_remote || best_remote->type == ICE_CANDIDATE_TYPE_HOST)) {
      best_remote = c;
    } else if (!best_remote) {
      best_remote = c;
    }
  }
  if (!best_remote && agent->remote_candidates_count > 0) {
    best_remote = &agent->remote_candidates[0];
  }
  if (!best_remote) {
    LOGE("No remote candidates for TURN relay.");
    return -1;
  }

  memcpy(&agent->turn_peer_addr, &best_remote->addr, sizeof(Address));

  uint16_t channel_random;
  if (utils_random_bytes((uint8_t*)&channel_random, sizeof(channel_random)) != 0) {
    LOGE("Failed to generate TURN channel number.");
    return -1;
  }
  agent->turn_channel = 0x4000 + (channel_random & 0x3FFF);
  if (agent_turn_channel_bind(agent, &agent->turn_peer_addr, agent->turn_channel, agent->turn_username, agent->turn_credential) < 0) {
    LOGE("TURN ChannelBind failed.");
    return -1;
  }

  agent->turn_channel_bound = 1;
  agent->turn_relay_ready = 1;

  {
    char addr_str[ADDRSTRLEN];
    addr_to_string(&agent->turn_peer_addr, addr_str, sizeof(addr_str));
    LOGI("TURN relay setup complete, channel=0x%04x, peer=%s:%d", agent->turn_channel, addr_str, agent->turn_peer_addr.port);
  }
  return 0;
}

static int agent_turn_relay_send(Agent* agent, const uint8_t* buf, int len, Address* peer_addr) {
  int use_channel = agent->turn_channel_bound && addr_equal(peer_addr, &agent->turn_peer_addr);
  if (use_channel) {
    uint8_t channel_buf[CONFIG_MTU + 4];
    int padded_len;
    if (len + 4 > sizeof(channel_buf)) {
      LOGE("Data too large for ChannelData.");
      return -1;
    }
    uint16_t channel_net = htons(agent->turn_channel);
    uint16_t length_net = htons(len);
    memcpy(channel_buf, &channel_net, 2);
    memcpy(channel_buf + 2, &length_net, 2);
    memcpy(channel_buf + 4, buf, len);
    padded_len = 4 + ((len + 3) & ~3);
    LOGD("TURN send ChannelData, ch=0x%04x, len=%d", agent->turn_channel, len);
    return agent_socket_send(agent, &agent->turn_server_addr, channel_buf, padded_len);
  } else {
    StunMessage send_msg;
    char peer_addr_attr[32];
    uint8_t mask[16];
    StunHeader* header;
    int addr_len;

    memset(&send_msg, 0, sizeof(send_msg));
    if (stun_msg_create(&send_msg, STUN_CLASS_INDICATION | STUN_METHOD_SEND) != 0) {
      LOGE("Failed to generate TURN transaction ID.");
      return -1;
    }

    header = (StunHeader*)send_msg.buf;
    *((uint32_t*)mask) = htonl(MAGIC_COOKIE);
    memcpy(mask + 4, header->transaction_id, sizeof(header->transaction_id));
    addr_len = stun_set_mapped_address(peer_addr_attr, mask, peer_addr);
    stun_msg_write_attr(&send_msg, STUN_ATTR_TYPE_XOR_PEER_ADDRESS, addr_len, peer_addr_attr);
    stun_msg_write_attr(&send_msg, STUN_ATTR_TYPE_DATA, len, (char*)buf);
    stun_msg_add_fingerprint(&send_msg);

    return agent_socket_send(agent, &agent->turn_server_addr, send_msg.buf, send_msg.size);
  }
}

static int agent_turn_unwrap(Agent* agent, uint8_t* buf, int len, Address* src_addr, Address* peer_addr) {
  if (len >= 4 && (buf[0] & 0xC0) == 0x40) {
    uint16_t data_len = (buf[2] << 8) | buf[3];
    if (data_len > (len - 4)) {
      LOGE("ChannelData length mismatch.");
      return -1;
    }
    LOGD("TURN recv ChannelData, len=%d", data_len);
    memmove(buf, buf + 4, data_len);
    if (peer_addr) {
      memcpy(peer_addr, &agent->turn_peer_addr, sizeof(Address));
    }
    return data_len;
  }

  if (len >= sizeof(StunHeader) && stun_probe(buf, len) == 0) {
    StunMessage msg;
    if (len > sizeof(msg.buf)) {
      return -2;
    }
    memcpy(msg.buf, buf, len);
    msg.size = len;
    if (stun_parse_msg_buf(&msg, (size_t)len) != 0) {
      return -2;
    }

    if (msg.stunclass == STUN_CLASS_INDICATION && msg.stunmethod == STUN_METHOD_DATA) {
      LOGD("TURN recv Data indication, len=%d", msg.data_len);
      if (peer_addr) {
        memcpy(peer_addr, &msg.peer_addr, sizeof(Address));
      }
      if (msg.data_len > 0 && msg.data_offset + msg.data_len <= len) {
        memmove(buf, buf + msg.data_offset, msg.data_len);
        return msg.data_len;
      }
    }

    int from_turn_server = 0;
    if (src_addr && agent->turn_server_addr.family != 0) {
      from_turn_server = addr_equal(src_addr, &agent->turn_server_addr);
    }
    if (from_turn_server) {
      LOGD("TURN recv non-data STUN msg from server (class=0x%04x method=0x%04x), skipping",
           msg.stunclass, msg.stunmethod);
      return 0;
    }
  }

  return -2;
}

int agent_turn_refresh(Agent* agent) {
  StunMessage send_msg;

  if (agent->turn_server_addr.family == 0) {
    return -1;
  }

  memset(&send_msg, 0, sizeof(send_msg));
  if (stun_msg_create(&send_msg, STUN_CLASS_REQUEST | STUN_METHOD_REFRESH) != 0) {
    LOGE("Failed to generate TURN transaction ID.");
    return -1;
  }

  if (agent_turn_send_stun_with_auth(agent, &send_msg, NULL, agent->turn_username, agent->turn_credential, NULL) == 0) {
    agent->turn_allocation_time = ports_get_epoch_time();
    LOGI("TURN Refresh succeeded.");
    return 0;
  }

  LOGE("TURN Refresh failed.");
  return -1;
}

int agent_reopen_udp_socket(Agent* agent) {
  const Address* local_addr =
      agent->b_bound_host_addr ? &agent->bound_host_addr : NULL;

  if (udp_socket_rebind(&agent->udp_sockets[0], local_addr) < 0) {
    LOGE("Failed to reopen UDP socket after TURN deallocate.");
    return -1;
  }

  LOGI("Reopened UDP socket (fd=%d, port=%d) — new 5-tuple avoids TURN 437",
       agent->udp_sockets[0].fd, agent->udp_sockets[0].bind_addr.port);
  return 0;
}

int agent_turn_deallocate(Agent* agent) {
  StunMessage send_msg;
  uint32_t lifetime_zero = 0;
  int ret = 0;

  if (!agent->turn_relay_ready && agent->turn_server_addr.family == 0) {
    /* Nothing was ever allocated on this agent, nothing to release. */
    return 0;
  }

  if (agent->turn_server_addr.family == 0) {
    goto reset_state;
  }

  memset(&send_msg, 0, sizeof(send_msg));
  if (stun_msg_create(&send_msg, STUN_CLASS_REQUEST | STUN_METHOD_REFRESH) != 0) {
    LOGE("Failed to generate TURN transaction ID.");
    ret = -1;
    goto reset_state;
  }

  ret = agent_turn_send_stun_with_auth(agent, &send_msg, NULL, agent->turn_username, agent->turn_credential, &lifetime_zero);

  if (ret == 0) {
    LOGI("TURN Deallocate (Refresh LIFETIME=0) succeeded.");
  } else {
    LOGW("TURN Deallocate failed, clearing local state anyway.");
  }

reset_state:
  /* Whether or not the server acked the deallocate, the previous allocation
   * (and its 5-tuple state) must no longer be treated as usable locally so
   * the next offer starts a fresh Allocate. If the server did receive the
   * Refresh, the allocation is gone immediately; if it didn't, it will
   * simply expire after its LIFETIME and stop matching our new Allocate's
   * transaction id (which only fails with 437 while it is still alive AND
   * we reuse the same 5-tuple without deallocating first). */
  agent->turn_relay_ready = 0;
  agent->turn_channel_bound = 0;
  agent->turn_channel = 0;
  agent->turn_allocation_time = 0;
  memset(&agent->turn_server_addr, 0, sizeof(agent->turn_server_addr));
  memset(&agent->turn_peer_addr, 0, sizeof(agent->turn_peer_addr));
  memset(agent->turn_nonce, 0, sizeof(agent->turn_nonce));
  memset(agent->turn_realm, 0, sizeof(agent->turn_realm));
  return ret;
}

void agent_gather_candidate(Agent* agent, const char* urls, const char* username, const char* credential) {
  char* pos;
  int port;
  char hostname[64];
  char addr_string[ADDRSTRLEN];
  int i;
  int addr_type[1] = {AF_INET};  // ipv6 no need stun
  Address resolved_addr;
  memset(hostname, 0, sizeof(hostname));

  if (urls == NULL) {
    agent_create_host_addr(agent);
    return;
  }

  if ((pos = strstr(urls + 5, ":")) == NULL) {
    LOGE("Invalid URL");
    return;
  }

  port = atoi(pos + 1);
  if (port <= 0) {
    LOGE("Cannot parse port");
    return;
  }

  int hostname_len = pos - (urls + 5);
  if (hostname_len <= 0 || hostname_len >= (int)sizeof(hostname)) {
    LOGE("Invalid hostname length");
    return;
  }
  memcpy(hostname, urls + 5, hostname_len);
  hostname[hostname_len] = '\0';
  LOGI("agent_gather_candidate: urls='%s', hostname='%s', hostname_len=%d", urls, hostname, hostname_len);

  for (i = 0; i < sizeof(addr_type) / sizeof(addr_type[0]); i++) {
    if (ports_resolve_addr(hostname, &resolved_addr) == 0) {
      addr_set_port(&resolved_addr, port);
      addr_to_string(&resolved_addr, addr_string, sizeof(addr_string));
      LOGI("Resolved stun/turn server %s:%d", addr_string, port);

      if (strncmp(urls, "stun:", 5) == 0) {
        LOGD("Create stun addr");
        agent_create_stun_addr(agent, &resolved_addr);
      } else if (strncmp(urls, "turn:", 5) == 0) {
        LOGD("Create turn addr");
        agent_create_turn_addr(agent, &resolved_addr, username, credential);
      }
    }
  }
}

int agent_create_ice_credential(Agent* agent) {
  memset(agent->local_ufrag, 0, sizeof(agent->local_ufrag));
  memset(agent->local_upwd, 0, sizeof(agent->local_upwd));

  if (utils_random_string(agent->local_ufrag, 4) != 0 ||
      utils_random_string(agent->local_upwd, 24) != 0) {
    memset(agent->local_ufrag, 0, sizeof(agent->local_ufrag));
    memset(agent->local_upwd, 0, sizeof(agent->local_upwd));
    LOGE("Failed to generate ICE credentials.");
    return -1;
  }
  return 0;
}

void agent_get_local_description(Agent* agent, char* description, int length) {
  for (int i = 0; i < agent->local_candidates_count; i++) {
    ice_candidate_to_description(&agent->local_candidates[i], description + strlen(description), length - strlen(description));
  }

  // remove last \n
  description[strlen(description)] = '\0';
  LOGD("local description:\n%s", description);
}

int agent_send(Agent* agent, const uint8_t* buf, int len) {
  if (agent_is_relay_active(agent)) {
    return agent_turn_relay_send(agent, buf, len, &agent->nominated_pair->remote->addr);
  }
  return agent_socket_send(agent, &agent->nominated_pair->remote->addr, buf, len);
}

static int agent_create_binding_response(Agent* agent, StunMessage* msg, Address* addr) {
  int size = 0;
  char username[584];
  char mapped_address[32];
  uint8_t mask[16];
  StunHeader* header;
  if (stun_msg_create(msg, STUN_CLASS_RESPONSE | STUN_METHOD_BINDING) != 0) {
    LOGE("Failed to initialize STUN Binding response.");
    return -1;
  }
  header = (StunHeader*)msg->buf;
  memcpy(header->transaction_id, agent->transaction_id, sizeof(header->transaction_id));
  snprintf(username, sizeof(username), "%s:%s", agent->local_ufrag, agent->remote_ufrag);
  *((uint32_t*)mask) = htonl(MAGIC_COOKIE);
  memcpy(mask + 4, agent->transaction_id, sizeof(agent->transaction_id));
  size = stun_set_mapped_address(mapped_address, mask, addr);
  stun_msg_write_attr(msg, STUN_ATTR_TYPE_XOR_MAPPED_ADDRESS, size, mapped_address);
  stun_msg_write_attr(msg, STUN_ATTR_TYPE_USERNAME, strlen(username), username);
  stun_msg_finish(msg, STUN_CREDENTIAL_SHORT_TERM, agent->local_upwd, strlen(agent->local_upwd));
  return 0;
}

static int agent_create_binding_request(Agent* agent, StunMessage* msg) {
  uint64_t tie_breaker = 0;
  uint32_t priority_net;
  if (stun_msg_create(msg, STUN_CLASS_REQUEST | STUN_METHOD_BINDING) != 0) {
    LOGE("Failed to generate STUN transaction ID.");
    return -1;
  }
  char username[584];
  memset(username, 0, sizeof(username));
  snprintf(username, sizeof(username), "%s:%s", agent->remote_ufrag, agent->local_ufrag);
  stun_msg_write_attr(msg, STUN_ATTR_TYPE_USERNAME, strlen(username), username);
  priority_net = htonl(agent->nominated_pair->local->priority);
  stun_msg_write_attr(msg, STUN_ATTR_TYPE_PRIORITY, 4, (char*)&priority_net);
  if (agent->mode == AGENT_MODE_CONTROLLING) {
    stun_msg_write_attr(msg, STUN_ATTR_TYPE_USE_CANDIDATE, 0, NULL);
    stun_msg_write_attr(msg, STUN_ATTR_TYPE_ICE_CONTROLLING, 8, (char*)&tie_breaker);
  } else {
    stun_msg_write_attr(msg, STUN_ATTR_TYPE_ICE_CONTROLLED, 8, (char*)&tie_breaker);
  }
  stun_msg_finish(msg, STUN_CREDENTIAL_SHORT_TERM, agent->remote_upwd, strlen(agent->remote_upwd));
  LOGD("Binding request: username=%s, priority=%u, size=%zu",
       username, agent->nominated_pair->local->priority, msg->size);
  return 0;
}

void agent_process_stun_request(Agent* agent, StunMessage* stun_msg, Address* addr) {
  StunMessage msg;
  StunHeader* header;
  switch (stun_msg->stunmethod) {
    case STUN_METHOD_BINDING:
      if (stun_msg_is_valid(stun_msg->buf, stun_msg->size, agent->local_upwd, STUN_CREDENTIAL_SHORT_TERM) == 0) {
        header = (StunHeader*)stun_msg->buf;
        memcpy(agent->transaction_id, header->transaction_id, sizeof(header->transaction_id));
        if (agent_create_binding_response(agent, &msg, addr) != 0) {
          break;
        }
        if (agent_is_relay_active(agent)) {
          agent_turn_relay_send(agent, msg.buf, msg.size, addr);
        } else {
          agent_socket_send(agent, addr, msg.buf, msg.size);
        }
        agent->binding_request_time = ports_get_epoch_time();
      }
      break;
    default:
      break;
  }
}

void agent_process_stun_response(Agent* agent, StunMessage* stun_msg, Address* src_addr) {
  int i;
  switch (stun_msg->stunmethod) {
    case STUN_METHOD_BINDING:
      if (stun_msg_is_valid(stun_msg->buf, stun_msg->size, agent->remote_upwd, STUN_CREDENTIAL_SHORT_TERM) == 0) {
        LOGI("STUN Binding Response validated, pair succeeded");
        for (i = 0; i < agent->active_pairs_count; i++) {
          if (agent->active_pairs[i]->state == ICE_CANDIDATE_STATE_INPROGRESS &&
              addr_equal(&agent->active_pairs[i]->remote->addr, src_addr)) {
            agent->active_pairs[i]->state = ICE_CANDIDATE_STATE_SUCCEEDED;
            /* Only update nominated_pair if no pair has been nominated yet.
             * A late STUN response (e.g. from a relay path arriving after
             * host→host has already been selected) must not override the
             * existing nominated_pair, otherwise DTLS data gets routed to
             * the wrong path and the handshake fails. */
            if (!agent->nominated_pair ||
                agent->nominated_pair->state != ICE_CANDIDATE_STATE_SUCCEEDED) {
              agent->nominated_pair = agent->active_pairs[i];
              agent->binding_request_time = ports_get_epoch_time();
            } else if (agent->nominated_pair != agent->active_pairs[i]) {
              char src_addr_str[ADDRSTRLEN] = {0};
              addr_to_string(src_addr, src_addr_str, sizeof(src_addr_str));
              LOGI("Nominated pair already established, keeping current (late response from %s:%d ignored for nomination)",
                   src_addr_str, src_addr->port);
            } else {
              agent->binding_request_time = ports_get_epoch_time();
            }
            break;
          }
        }
        if (i == agent->active_pairs_count && agent->nominated_pair) {
          if (addr_equal(&agent->nominated_pair->remote->addr, src_addr)) {
            /* An authenticated response from the selected path proves that
             * the connection is alive even before the peer's next consent
             * request arrives. */
            agent->binding_request_time = ports_get_epoch_time();
            LOGD("Ignoring duplicate STUN response for nominated pair");
          } else {
            char src_addr_str[ADDRSTRLEN] = {0};
            addr_to_string(src_addr, src_addr_str, sizeof(src_addr_str));
            LOGW("Ignoring STUN response from unmatched address %s:%d",
                 src_addr_str, src_addr->port);
          }
        }
      } else {
        LOGI("STUN Binding Response validation failed");
      }
      break;
    default:
      break;
  }
}

int agent_recv(Agent* agent, uint8_t* buf, int len) {
  int ret = -1;
  StunMessage stun_msg;
  Address addr;
  Address peer_addr;
  Address* effective_addr;

  if ((ret = agent_socket_recv(agent, &addr, buf, len)) <= 0) {
    return ret;
  }

  /* Always try to unwrap TURN data if TURN allocation exists.
   * Previously this only ran when agent_is_relay_active(), which meant
   * ChannelData/Data Indication from the TURN server was silently dropped
   * when the nominated pair was not a relay type. This caused peer's
   * Binding Requests/Responses arriving via TURN relay to be lost. */
  if (agent->turn_relay_ready && ret >= 4) {
    int unwrap_ret = agent_turn_unwrap(agent, buf, ret, &addr, &peer_addr);
    if (unwrap_ret > 0) {
      ret = unwrap_ret;
      {
        char addr_str[ADDRSTRLEN];
        addr_to_string(&peer_addr, addr_str, sizeof(addr_str));
        LOGI("TURN relay recv %d bytes from %s:%d", ret, addr_str, peer_addr.port);
      }
      effective_addr = &peer_addr;
    } else if (unwrap_ret == 0) {
      return 0;
    } else if (unwrap_ret == -2) {
      effective_addr = &addr;
    } else {
      return unwrap_ret;
    }
  } else {
    effective_addr = &addr;
  }

  if (stun_probe(buf, ret) == 0) {
    if (ret > sizeof(stun_msg.buf)) {
      return ret;
    }
    memcpy(stun_msg.buf, buf, ret);
    stun_msg.size = ret;
    if (stun_parse_msg_buf(&stun_msg, (size_t)ret) != 0) {
      LOGW("Dropping malformed STUN message");
      return 0;
    }
    switch (stun_msg.stunclass) {
      case STUN_CLASS_REQUEST:
        agent_process_stun_request(agent, &stun_msg, effective_addr);
        break;
      case STUN_CLASS_RESPONSE:
        LOGI("recv STUN Binding Response");
        agent_process_stun_response(agent, &stun_msg, effective_addr);
        break;
      case STUN_CLASS_ERROR:
        LOGI("recv STUN Error Response (code=%d)", stun_msg.error_code);
        break;
      default:
        break;
    }
    ret = 0;
  } else {
    LOGD("recv non-STUN data %d bytes", ret);
  }
  return ret;
}

int agent_set_remote_description(Agent* agent, char* description) {
  /*
  a=ice-ufrag:Iexb
  a=ice-pwd:IexbSoY7JulyMbjKwISsG9
  a=candidate:1 1 UDP 1 36.231.28.50 38143 typ srflx
  */
  int i;
  char remote_ufrag[ICE_UFRAG_LENGTH + 1] = {0};
  char remote_upwd[ICE_UPWD_LENGTH + 1] = {0};
  IceCandidate remote_candidates[AGENT_MAX_CANDIDATES];
  int remote_candidates_count = 0;
  size_t description_len;

  if (agent == NULL || description == NULL) {
    return -1;
  }
  description_len = strnlen(description, AGENT_MAX_DESCRIPTION + 1);
  if (description_len > AGENT_MAX_DESCRIPTION) {
    LOGE("Remote description exceeds maximum length");
    return -1;
  }
  memset(remote_candidates, 0, sizeof(remote_candidates));

  LOGD("Set remote description:\n%s", description);

  char* line_start = description;
  char* line_end = NULL;
  char* description_end = description + description_len;

  while (line_start < description_end) {
    line_end = strstr(line_start, "\r\n");
    if (line_end == NULL) {
      line_end = description_end;
    }
    if (strncmp(line_start, "a=ice-ufrag:", strlen("a=ice-ufrag:")) == 0) {
      ptrdiff_t ufrag_len = line_end - line_start - (ptrdiff_t)strlen("a=ice-ufrag:");
      if (ufrag_len <= 0 || ufrag_len > ICE_UFRAG_LENGTH) {
        LOGE("Invalid remote ICE username fragment length");
        return -1;
      }
      memcpy(remote_ufrag, line_start + strlen("a=ice-ufrag:"), (size_t)ufrag_len);
      remote_ufrag[ufrag_len] = '\0';

    } else if (strncmp(line_start, "a=ice-pwd:", strlen("a=ice-pwd:")) == 0) {
      ptrdiff_t upwd_len = line_end - line_start - (ptrdiff_t)strlen("a=ice-pwd:");
      if (upwd_len <= 0 || upwd_len > ICE_UPWD_LENGTH) {
        LOGE("Invalid remote ICE password length");
        return -1;
      }
      memcpy(remote_upwd, line_start + strlen("a=ice-pwd:"), (size_t)upwd_len);
      remote_upwd[upwd_len] = '\0';

    } else if (strncmp(line_start, "a=candidate:", strlen("a=candidate:")) == 0) {
      IceCandidate candidate;
      if (ice_candidate_from_description(&candidate, line_start, line_end) == 0) {
        for (i = 0; i < remote_candidates_count; i++) {
          if (ice_candidate_equal(&remote_candidates[i], &candidate)) {
            break;
          }
        }
        if (i == remote_candidates_count) {
          if (remote_candidates_count >= AGENT_MAX_CANDIDATES) {
            LOGE("Remote description has too many ICE candidates");
            return -1;
          }
          remote_candidates[remote_candidates_count++] = candidate;
        }
      }
    }

    if (line_end == description_end) {
      break;
    }
    line_start = line_end + 2;
  }

  if (remote_ufrag[0] == '\0' || remote_upwd[0] == '\0') {
    LOGE("Remote description is missing ICE credentials");
    return -1;
  }

  memcpy(agent->remote_ufrag, remote_ufrag, sizeof(remote_ufrag));
  memcpy(agent->remote_upwd, remote_upwd, sizeof(remote_upwd));
  memcpy(agent->remote_candidates, remote_candidates, sizeof(remote_candidates));
  agent->remote_candidates_count = remote_candidates_count;

  LOGD("remote ufrag: %s", agent->remote_ufrag);
  LOGD("remote upwd: %s", agent->remote_upwd);
  return 0;
}

void agent_update_candidate_pairs(Agent* agent) {
  int i, j;
  agent->candidate_pairs_num = 0;
  agent->active_pairs_count = 0;
  for (i = 0; i < agent->local_candidates_count; i++) {
    for (j = 0; j < agent->remote_candidates_count; j++) {
      if (agent->local_candidates[i].addr.family != agent->remote_candidates[j].addr.family)
        continue;
      if (agent_candidate_pair_is_impossible(agent, &agent->local_candidates[i], &agent->remote_candidates[j])) {
        LOGI("Skipping impossible pair: local type=%d remote type=%d",
             agent->local_candidates[i].type, agent->remote_candidates[j].type);
        continue;
      }
      if (agent->candidate_pairs_num >= AGENT_MAX_CANDIDATE_PAIRS) {
        LOGE("Too many ICE candidate pairs");
        return;
      }
      agent->candidate_pairs[agent->candidate_pairs_num].local = &agent->local_candidates[i];
      agent->candidate_pairs[agent->candidate_pairs_num].remote = &agent->remote_candidates[j];
      agent->candidate_pairs[agent->candidate_pairs_num].priority = agent->local_candidates[i].priority + agent->remote_candidates[j].priority;
      agent->candidate_pairs[agent->candidate_pairs_num].state = ICE_CANDIDATE_STATE_FROZEN;
      agent->candidate_pairs[agent->candidate_pairs_num].conncheck = 0;
      agent->candidate_pairs[agent->candidate_pairs_num].conncheck_timestamp = 0;
      agent->candidate_pairs_num++;
    }
  }

  for (i = 0; i < agent->candidate_pairs_num - 1; i++) {
    for (j = i + 1; j < agent->candidate_pairs_num; j++) {
      if (agent->candidate_pairs[i].priority < agent->candidate_pairs[j].priority) {
        IceCandidatePair tmp = agent->candidate_pairs[i];
        agent->candidate_pairs[i] = agent->candidate_pairs[j];
        agent->candidate_pairs[j] = tmp;
      }
    }
  }

  LOGI("candidate pairs num: %d (filtered from %d possible)",
       agent->candidate_pairs_num,
       agent->local_candidates_count * agent->remote_candidates_count);
}

int agent_connectivity_check(Agent* agent) {
  uint8_t buf[1400];
  StunMessage msg;
  uint32_t now = ports_get_epoch_time();
  int i;
  int any_active = 0;

  for (i = 0; i < agent->active_pairs_count; i++) {
    IceCandidatePair* pair = agent->active_pairs[i];
    if (pair->state != ICE_CANDIDATE_STATE_INPROGRESS) continue;

    any_active = 1;

    if (pair->local->type == ICE_CANDIDATE_TYPE_RELAY && !agent->turn_relay_ready) {
      if (agent_turn_setup_relay(agent) < 0) {
        LOGE("TURN relay setup failed.");
        pair->state = ICE_CANDIDATE_STATE_FAILED;
        continue;
      }
    }

    if (pair->conncheck >= AGENT_CONNCHECK_MAX &&
        (uint32_t)(now - pair->conncheck_timestamp) >= AGENT_CONNCHECK_INTERVAL_MS) {
      pair->state = ICE_CANDIDATE_STATE_FAILED;
      continue;
    }

    if (pair->conncheck == 0 ||
        (uint32_t)(now - pair->conncheck_timestamp) >= AGENT_CONNCHECK_INTERVAL_MS) {
      agent->nominated_pair = pair;
      pair->conncheck++;
      pair->conncheck_timestamp = now;
      memset(&msg, 0, sizeof(msg));

      if (pair->local->type == ICE_CANDIDATE_TYPE_RELAY && !agent->turn_relay_ready) {
        LOGD("relay pair not ready, skipping binding request");
      } else if (agent_is_relay_active(agent)) {
        if (agent_create_binding_request(agent, &msg) == 0) {
          agent_turn_relay_send(agent, msg.buf, msg.size, &pair->remote->addr);
        }
      } else {
        if (agent_create_binding_request(agent, &msg) == 0) {
          agent_socket_send(agent, &pair->remote->addr, msg.buf, msg.size);
        }
      }
    }
  }

  if (!any_active) return -1;

  agent_recv(agent, buf, sizeof(buf));

  for (i = 0; i < agent->active_pairs_count; i++) {
    if (agent->active_pairs[i]->state == ICE_CANDIDATE_STATE_SUCCEEDED) {
      agent->selected_pair = agent->active_pairs[i];
      agent->nominated_pair = agent->active_pairs[i];
      return 0;
    }
  }

  return -1;
}

int agent_select_candidate_pair(Agent* agent) {
  int i, j;

  for (i = 0; i < agent->active_pairs_count; i++) {
    if (agent->active_pairs[i]->state == ICE_CANDIDATE_STATE_SUCCEEDED) {
      agent->selected_pair = agent->active_pairs[i];
      return 0;
    }
  }

  for (i = agent->active_pairs_count - 1; i >= 0; i--) {
    if (agent->active_pairs[i]->state == ICE_CANDIDATE_STATE_FAILED) {
      for (j = i; j < agent->active_pairs_count - 1; j++) {
        agent->active_pairs[j] = agent->active_pairs[j + 1];
      }
      agent->active_pairs_count--;
    }
  }

  for (i = agent->active_pairs_count - 1; i >= 0; i--) {
    if (agent->active_pairs[i]->state == ICE_CANDIDATE_STATE_FAILED) {
      for (j = i; j < agent->active_pairs_count - 1; j++) {
        agent->active_pairs[j] = agent->active_pairs[j + 1];
      }
      agent->active_pairs_count--;
    }
  }

  for (i = 0; i < agent->candidate_pairs_num && agent->active_pairs_count < AGENT_MAX_ACTIVE_PAIRS; i++) {
    if (agent->candidate_pairs[i].state == ICE_CANDIDATE_STATE_FROZEN) {
      agent->candidate_pairs[i].conncheck = 0;
      agent->candidate_pairs[i].conncheck_timestamp = 0;
      agent->candidate_pairs[i].state = ICE_CANDIDATE_STATE_INPROGRESS;
      agent->active_pairs[agent->active_pairs_count++] = &agent->candidate_pairs[i];
    }
  }

  if (agent->active_pairs_count > 0) {
    return 0;
  }

  return -1;
}
