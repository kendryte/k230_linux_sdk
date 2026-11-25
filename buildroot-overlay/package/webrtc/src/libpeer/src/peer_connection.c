#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "agent.h"
#include "config.h"
#include "dtls_srtp.h"
#include "peer_connection.h"
#include "ports.h"
#include "rtcp.h"
#include "rtp.h"
#include "sctp.h"
#include "sdp.h"

#define STATE_CHANGED(pc, curr_state)                                 \
  if (pc->oniceconnectionstatechange && pc->state != curr_state) {    \
    pc->oniceconnectionstatechange(curr_state, pc->config.user_data); \
    pc->state = curr_state;                                           \
  }

struct PeerConnection {
  PeerConfiguration config;
  PeerConnectionState state;
  Agent agent;
  DtlsSrtp dtls_srtp;
  Sctp sctp;

  char sdp[CONFIG_SDP_BUFFER_SIZE];

  void (*onicecandidate)(char* sdp, void* user_data);
  void (*oniceconnectionstatechange)(PeerConnectionState state, void* user_data);
  void (*on_connected)(void* userdata);
  void (*on_receiver_packet_loss)(float fraction_loss, uint32_t total_loss, void* user_data);

  uint8_t temp_buf[CONFIG_MTU];
  uint8_t agent_buf[CONFIG_MTU];
  int agent_ret;
  int dtls_recv_available;
  uint8_t* dtls_client_hello;
  uint8_t* dtls_client_hello_map;
  size_t dtls_client_hello_len;
  size_t dtls_client_hello_received;
  uint16_t dtls_client_hello_seq;
  int b_local_description_created;

  RtpEncoder artp_encoder;
  RtpEncoder vrtp_encoder;
  RtpDecoder vrtp_decoder;
  RtpDecoder artp_decoder;

  uint32_t remote_assrc;
  uint32_t remote_vssrc;
};

static void peer_connection_outgoing_rtp_packet(uint8_t* data, size_t size, void* user_data) {
  PeerConnection* pc = (PeerConnection*)user_data;
  dtls_srtp_encrypt_rtp_packet(&pc->dtls_srtp, data, (int*)&size);
  agent_send(&pc->agent, data, size);
}

#define DTLS_RECORD_HEADER_LEN 13
#define DTLS_HANDSHAKE_HEADER_LEN 12
#define DTLS_CONTENT_HANDSHAKE 22
#define DTLS_HANDSHAKE_CLIENT_HELLO 1
#define DTLS_CLIENT_HELLO_MAX 16384

static uint32_t peer_connection_read_u24(const uint8_t* value) {
  return ((uint32_t)value[0] << 16) | ((uint32_t)value[1] << 8) | value[2];
}

static void peer_connection_clear_client_hello(PeerConnection* pc) {
  free(pc->dtls_client_hello);
  free(pc->dtls_client_hello_map);
  pc->dtls_client_hello = NULL;
  pc->dtls_client_hello_map = NULL;
  pc->dtls_client_hello_len = 0;
  pc->dtls_client_hello_received = 0;
  pc->dtls_client_hello_seq = 0;
}

static int peer_connection_reassemble_client_hello(PeerConnection* pc,
                                                    uint8_t* packet,
                                                    size_t packet_len,
                                                    size_t capacity) {
  if (packet_len < DTLS_RECORD_HEADER_LEN + DTLS_HANDSHAKE_HEADER_LEN ||
      packet[0] != DTLS_CONTENT_HANDSHAKE) {
    return (int)packet_len;
  }

  size_t record_len = ((size_t)packet[11] << 8) | packet[12];
  if (record_len + DTLS_RECORD_HEADER_LEN != packet_len ||
      record_len < DTLS_HANDSHAKE_HEADER_LEN) {
    return (int)packet_len;
  }

  uint8_t* handshake = packet + DTLS_RECORD_HEADER_LEN;
  if (handshake[0] != DTLS_HANDSHAKE_CLIENT_HELLO) {
    return (int)packet_len;
  }

  uint32_t total_len = peer_connection_read_u24(handshake + 1);
  uint16_t message_seq = ((uint16_t)handshake[4] << 8) | handshake[5];
  uint32_t fragment_offset = peer_connection_read_u24(handshake + 6);
  uint32_t fragment_len = peer_connection_read_u24(handshake + 9);
  if (fragment_offset == 0 && fragment_len == total_len) {
    peer_connection_clear_client_hello(pc);
    return (int)packet_len;
  }
  if (total_len == 0 || total_len > DTLS_CLIENT_HELLO_MAX ||
      fragment_offset > total_len || fragment_len > total_len - fragment_offset ||
      record_len != DTLS_HANDSHAKE_HEADER_LEN + fragment_len) {
    peer_connection_clear_client_hello(pc);
    return MBEDTLS_ERR_SSL_DECODE_ERROR;
  }

  size_t assembled_len = DTLS_RECORD_HEADER_LEN + DTLS_HANDSHAKE_HEADER_LEN + total_len;
  if (assembled_len > capacity) {
    peer_connection_clear_client_hello(pc);
    return MBEDTLS_ERR_SSL_BUFFER_TOO_SMALL;
  }

  if (pc->dtls_client_hello == NULL ||
      pc->dtls_client_hello_len != assembled_len ||
      pc->dtls_client_hello_seq != message_seq) {
    peer_connection_clear_client_hello(pc);
    pc->dtls_client_hello = malloc(assembled_len);
    pc->dtls_client_hello_map = calloc(total_len, 1);
    if (pc->dtls_client_hello == NULL || pc->dtls_client_hello_map == NULL) {
      peer_connection_clear_client_hello(pc);
      return MBEDTLS_ERR_SSL_ALLOC_FAILED;
    }
    pc->dtls_client_hello_len = assembled_len;
    pc->dtls_client_hello_seq = message_seq;
    memcpy(pc->dtls_client_hello, packet, DTLS_RECORD_HEADER_LEN + DTLS_HANDSHAKE_HEADER_LEN);
    pc->dtls_client_hello[11] = (uint8_t)((DTLS_HANDSHAKE_HEADER_LEN + total_len) >> 8);
    pc->dtls_client_hello[12] = (uint8_t)(DTLS_HANDSHAKE_HEADER_LEN + total_len);
    memset(pc->dtls_client_hello + DTLS_RECORD_HEADER_LEN + 6, 0, 3);
    pc->dtls_client_hello[DTLS_RECORD_HEADER_LEN + 9] = (uint8_t)(total_len >> 16);
    pc->dtls_client_hello[DTLS_RECORD_HEADER_LEN + 10] = (uint8_t)(total_len >> 8);
    pc->dtls_client_hello[DTLS_RECORD_HEADER_LEN + 11] = (uint8_t)total_len;
  }

  uint8_t* destination = pc->dtls_client_hello +
                         DTLS_RECORD_HEADER_LEN + DTLS_HANDSHAKE_HEADER_LEN +
                         fragment_offset;
  const uint8_t* source = handshake + DTLS_HANDSHAKE_HEADER_LEN;
  for (uint32_t i = 0; i < fragment_len; i++) {
    if (pc->dtls_client_hello_map[fragment_offset + i] == 0) {
      pc->dtls_client_hello_map[fragment_offset + i] = 1;
      pc->dtls_client_hello_received++;
    }
    destination[i] = source[i];
  }

  if (pc->dtls_client_hello_received != total_len) {
    return MBEDTLS_ERR_SSL_WANT_READ;
  }

  memcpy(packet, pc->dtls_client_hello, assembled_len);
  peer_connection_clear_client_hello(pc);
  return (int)assembled_len;
}

static int peer_connection_dtls_srtp_recv(void* ctx, unsigned char* buf, size_t len) {
  int ret;
  DtlsSrtp* dtls_srtp = (DtlsSrtp*)ctx;
  PeerConnection* pc = (PeerConnection*)dtls_srtp->user_data;

  /* Consume at most one datagram per tick so a busy or incomplete handshake
   * cannot starve peers. Fragmented ClientHello messages are assembled above
   * because Mbed TLS 3.x rejects them before its general DTLS reassembler. */
  if (!pc->dtls_recv_available) {
    return MBEDTLS_ERR_SSL_WANT_READ;
  }
  pc->dtls_recv_available = 0;

  if (pc->agent_ret > 0) {
    ret = pc->agent_ret;
    pc->agent_ret = 0;
    if ((size_t)ret > len) {
      return MBEDTLS_ERR_SSL_BUFFER_TOO_SMALL;
    }
    memcpy(buf, pc->agent_buf, ret);
  } else {
    ret = agent_recv(&pc->agent, buf, len);
    if (ret == 0) {
      return MBEDTLS_ERR_SSL_WANT_READ;
    }
    if (ret < 0) {
      return ret;
    }
  }

  return peer_connection_reassemble_client_hello(pc, buf, ret, len);
}

static int peer_connection_dtls_srtp_send(void* ctx, const uint8_t* buf, size_t len) {
  DtlsSrtp* dtls_srtp = (DtlsSrtp*)ctx;
  PeerConnection* pc = (PeerConnection*)dtls_srtp->user_data;

  // LOGD("send %.4x %.4x, %ld", *(uint16_t*)buf, *(uint16_t*)(buf + 2), len);
  return agent_send(&pc->agent, buf, len);
}

static void peer_connection_incoming_rtcp(PeerConnection* pc, uint8_t* buf, size_t len) {
  RtcpHeader* rtcp_header;
  size_t pos = 0;

  while (pos < len) {
    rtcp_header = (RtcpHeader*)(buf + pos);

    switch (rtcp_header->type) {
      case RTCP_RR:
        LOGD("RTCP_PR");
        if (rtcp_header->rc > 0) {
// TODO: REMB, GCC ...etc
#if 0
          RtcpRr rtcp_rr = rtcp_parse_rr(buf);
          uint32_t fraction = ntohl(rtcp_rr.report_block[0].flcnpl) >> 24;
          uint32_t total = ntohl(rtcp_rr.report_block[0].flcnpl) & 0x00FFFFFF;
          if(pc->on_receiver_packet_loss && fraction > 0) {

            pc->on_receiver_packet_loss((float)fraction/256.0, total, pc->config.user_data);
          }
#endif
        }
        break;
      case RTCP_PSFB: {
        int fmt = rtcp_header->rc;
        LOGD("RTCP_PSFB %d", fmt);
        // PLI and FIR
        if ((fmt == 1 || fmt == 4) && pc->config.on_request_keyframe) {
          pc->config.on_request_keyframe(pc->config.user_data);
        }
      }
      default:
        break;
    }

    pos += 4 * ntohs(rtcp_header->length) + 4;
  }
}

const char* peer_connection_state_to_string(PeerConnectionState state) {
  switch (state) {
    case PEER_CONNECTION_NEW:
      return "new";
    case PEER_CONNECTION_CHECKING:
      return "checking";
    case PEER_CONNECTION_CONNECTED:
      return "connected";
    case PEER_CONNECTION_COMPLETED:
      return "completed";
    case PEER_CONNECTION_FAILED:
      return "failed";
    case PEER_CONNECTION_CLOSED:
      return "closed";
    case PEER_CONNECTION_DISCONNECTED:
      return "disconnected";
    default:
      return "unknown";
  }
}

PeerConnectionState peer_connection_get_state(PeerConnection* pc) {
  return pc->state;
}

void* peer_connection_get_sctp(PeerConnection* pc) {
  return &pc->sctp;
}

PeerConnection* peer_connection_create(PeerConfiguration* config) {
  PeerConnection* pc = calloc(1, sizeof(PeerConnection));
  if (!pc) {
    return NULL;
  }

  memcpy(&pc->config, config, sizeof(PeerConfiguration));

  if (agent_create(&pc->agent) != 0) {
    free(pc);
    return NULL;
  }
  if (pc->config.local_ip != NULL &&
      agent_set_host_address(&pc->agent, pc->config.local_ip) != 0) {
    LOGW("Ignoring invalid local_ip; falling back to interface detection");
  }

  memset(&pc->sctp, 0, sizeof(pc->sctp));

  if (pc->config.audio_codec) {
    rtp_encoder_init(&pc->artp_encoder, pc->config.audio_codec,
                     peer_connection_outgoing_rtp_packet, (void*)pc);

    rtp_decoder_init(&pc->artp_decoder, pc->config.audio_codec,
                     pc->config.onaudiotrack, pc->config.user_data);
  }

  if (pc->config.video_codec) {
    rtp_encoder_init(&pc->vrtp_encoder, pc->config.video_codec,
                     peer_connection_outgoing_rtp_packet, (void*)pc);

    rtp_decoder_init(&pc->vrtp_decoder, pc->config.video_codec,
                     pc->config.onvideotrack, pc->config.user_data);
  }

  return pc;
}

int peer_connection_set_local_ip(PeerConnection* pc, const char* local_ip) {
  if (pc == NULL) {
    return -1;
  }
  if (pc->state != PEER_CONNECTION_CLOSED && pc->state != PEER_CONNECTION_NEW) {
    LOGW("Cannot change local_ip while peer connection is active");
    return -1;
  }

  return agent_bind_host_address(&pc->agent, local_ip);
}

void peer_connection_destroy(PeerConnection* pc) {
  if (pc) {
    peer_connection_clear_client_hello(pc);
    sctp_destroy_association(&pc->sctp);
    dtls_srtp_deinit(&pc->dtls_srtp);
    agent_destroy(&pc->agent);
    free(pc);
    pc = NULL;
  }
}

void peer_connection_close(PeerConnection* pc) {
  STATE_CHANGED(pc, PEER_CONNECTION_CLOSED);
}

int peer_connection_send_audio(PeerConnection* pc, const uint8_t* buf, size_t len, uint64_t timestamp_us) {
  if (pc->state != PEER_CONNECTION_COMPLETED) {
    // LOGE("dtls_srtp not connected");
    return -1;
  }
  return rtp_encoder_encode(&pc->artp_encoder, buf, len, timestamp_us);
}

int peer_connection_send_video(PeerConnection* pc, const uint8_t* buf, size_t len, uint64_t timestamp_us) {
  if (pc->state != PEER_CONNECTION_COMPLETED) {
    // LOGE("dtls_srtp not connected");
    return -1;
  }
  return rtp_encoder_encode(&pc->vrtp_encoder, buf, len, timestamp_us);
}

int peer_connection_datachannel_send(PeerConnection* pc, char* message, size_t len) {
  return peer_connection_datachannel_send_sid(pc, message, len, 0);
}

int peer_connection_datachannel_send_sid(PeerConnection* pc, char* message, size_t len, uint16_t sid) {
  if (!sctp_is_connected(&pc->sctp)) {
    LOGE("sctp not connected");
    return -1;
  }
  return sctp_outgoing_data(&pc->sctp, message, len, PPID_STRING, sid);
}

int peer_connection_datachannel_send_binary(PeerConnection* pc, const char* data, size_t len) {
  return peer_connection_datachannel_send_binary_sid(pc, data, len, 0);
}

int peer_connection_datachannel_send_binary_sid(PeerConnection* pc, const char* data, size_t len, uint16_t sid) {
  if (!sctp_is_connected(&pc->sctp)) {
    LOGE("sctp not connected");
    return -1;
  }
  return sctp_outgoing_data(&pc->sctp, (char*)data, len, PPID_BINARY, sid);
}

int peer_connection_create_datachannel(PeerConnection* pc, DecpChannelType channel_type, uint16_t priority, uint32_t reliability_parameter, char* label, char* protocol) {
  return peer_connection_create_datachannel_sid(pc, channel_type, priority, reliability_parameter, label, protocol, 0);
}

int peer_connection_create_datachannel_sid(PeerConnection* pc, DecpChannelType channel_type, uint16_t priority, uint32_t reliability_parameter, char* label, char* protocol, uint16_t sid) {
  int rtrn = -1;

  if (!sctp_is_connected(&pc->sctp)) {
    LOGE("sctp not connected");
    return rtrn;
  }

  //  0                   1                   2                   3
  //  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
  // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  // |  Message Type |  Channel Type |            Priority           |
  // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  // |                    Reliability Parameter                      |
  // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  // |         Label Length          |       Protocol Length         |
  // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  // |                                                               |
  // |                             Label                             |
  // |                                                               |
  // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  // |                                                               |
  // |                            Protocol                           |
  // |                                                               |
  // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  int msg_size = 12 + strlen(label) + strlen(protocol);
  uint16_t priority_big_endian = htons(priority);
  uint32_t reliability_big_endian = ntohl(reliability_parameter);
  uint16_t label_length = htons(strlen(label));
  uint16_t protocol_length = htons(strlen(protocol));
  char* msg = calloc(1, msg_size);
  if (!msg) {
    return rtrn;
  }

  msg[0] = DATA_CHANNEL_OPEN;
  memcpy(msg + 2, &priority_big_endian, sizeof(uint16_t));
  memcpy(msg + 4, &reliability_big_endian, sizeof(uint32_t));
  memcpy(msg + 8, &label_length, sizeof(uint16_t));
  memcpy(msg + 10, &protocol_length, sizeof(uint16_t));
  memcpy(msg + 12, label, strlen(label));
  memcpy(msg + 12 + strlen(label), protocol, strlen(protocol));

  rtrn = sctp_outgoing_data(&pc->sctp, msg, msg_size, PPID_CONTROL, sid);
  free(msg);
  return rtrn;
}

static char* peer_connection_dtls_role_setup_value(DtlsSrtpRole d) {
  return d == DTLS_SRTP_ROLE_SERVER ? "a=setup:passive" : "a=setup:active";
}

int peer_connection_loop(PeerConnection* pc) {
  uint32_t ssrc = 0;
  memset(pc->agent_buf, 0, sizeof(pc->agent_buf));
  pc->agent_ret = -1;
  pc->dtls_recv_available = 1;

  switch (pc->state) {
    case PEER_CONNECTION_NEW:
      break;

    case PEER_CONNECTION_CHECKING:
      if (agent_select_candidate_pair(&pc->agent) < 0) {
        STATE_CHANGED(pc, PEER_CONNECTION_FAILED);
      } else if (agent_connectivity_check(&pc->agent) == 0) {
    {
        char local_addr[ADDRSTRLEN] = {0};
        char remote_addr[ADDRSTRLEN] = {0};
        addr_to_string(&pc->agent.selected_pair->local->addr, local_addr, sizeof(local_addr));
        addr_to_string(&pc->agent.selected_pair->remote->addr, remote_addr, sizeof(remote_addr));
        LOGI("[P2P] ========== CONNECTION SUCCESS! ==========");
        LOGI("[P2P] Local:  %s:%d (%s)", local_addr, pc->agent.selected_pair->local->addr.port,
                pc->agent.selected_pair->local->type == ICE_CANDIDATE_TYPE_HOST ? "host" :
              pc->agent.selected_pair->local->type == ICE_CANDIDATE_TYPE_SRFLX ? "srflx" :
                pc->agent.selected_pair->local->type == ICE_CANDIDATE_TYPE_RELAY ? "relay" : "?");
        LOGI("[P2P] Remote: %s:%d (%s)", remote_addr, pc->agent.selected_pair->remote->addr.port,
                pc->agent.selected_pair->remote->type == ICE_CANDIDATE_TYPE_HOST ? "host" :
                pc->agent.selected_pair->remote->type == ICE_CANDIDATE_TYPE_SRFLX ? "srflx" :
                pc->agent.selected_pair->remote->type == ICE_CANDIDATE_TYPE_RELAY ? "relay" : "?");
        printf("===============================================\n");
      }

      STATE_CHANGED(pc, PEER_CONNECTION_CONNECTED);
    }
      break;

    case PEER_CONNECTION_CONNECTED:
    {
      int handshake_ret = dtls_srtp_handshake(&pc->dtls_srtp, NULL);
      if (handshake_ret == 0) {
        LOGD("DTLS-SRTP handshake done");

        if (pc->config.datachannel) {
          LOGI("SCTP create socket");
          sctp_create_association(&pc->sctp, &pc->dtls_srtp);
          pc->sctp.userdata = pc->config.user_data;
        }

        STATE_CHANGED(pc, PEER_CONNECTION_COMPLETED);
      } else if (handshake_ret != MBEDTLS_ERR_SSL_WANT_READ &&
                 handshake_ret != MBEDTLS_ERR_SSL_WANT_WRITE) {
        STATE_CHANGED(pc, PEER_CONNECTION_FAILED);
      }
      break;
    }
    case PEER_CONNECTION_COMPLETED:
      if ((pc->agent_ret = agent_recv(&pc->agent, pc->agent_buf, sizeof(pc->agent_buf))) > 0) {
        LOGD("agent_recv %d", pc->agent_ret);

        if (rtcp_probe(pc->agent_buf, pc->agent_ret)) {
          LOGD("Got RTCP packet");
          dtls_srtp_decrypt_rtcp_packet(&pc->dtls_srtp, pc->agent_buf, &pc->agent_ret);
          peer_connection_incoming_rtcp(pc, pc->agent_buf, pc->agent_ret);

        } else if (dtls_srtp_probe(pc->agent_buf)) {
          int ret = dtls_srtp_read(&pc->dtls_srtp, pc->temp_buf, sizeof(pc->temp_buf));
          LOGD("Got DTLS data %d", ret);

          if (ret > 0) {
            sctp_incoming_data(&pc->sctp, (char*)pc->temp_buf, ret);
          }

        } else if (rtp_packet_validate(pc->agent_buf, pc->agent_ret)) {
          LOGD("Got RTP packet");

          dtls_srtp_decrypt_rtp_packet(&pc->dtls_srtp, pc->agent_buf, &pc->agent_ret);

          ssrc = rtp_get_ssrc(pc->agent_buf);
          if (ssrc == pc->remote_assrc) {
            rtp_decoder_decode(&pc->artp_decoder, pc->agent_buf, pc->agent_ret);
          } else if (ssrc == pc->remote_vssrc) {
            rtp_decoder_decode(&pc->vrtp_decoder, pc->agent_buf, pc->agent_ret);
          }

        } else {
          LOGW("Unknown data");
        }
      }

      if (CONFIG_KEEPALIVE_TIMEOUT > 0 && (ports_get_epoch_time() - pc->agent.binding_request_time) > CONFIG_KEEPALIVE_TIMEOUT) {
        LOGI("binding request timeout");
        STATE_CHANGED(pc, PEER_CONNECTION_CLOSED);
        break;
      }

      if (pc->agent.turn_relay_ready && (ports_get_epoch_time() - pc->agent.turn_allocation_time) > 540000) {
        agent_turn_refresh(&pc->agent);
      }

      break;
    case PEER_CONNECTION_FAILED:
      break;
    case PEER_CONNECTION_DISCONNECTED:
      break;
    case PEER_CONNECTION_CLOSED:
      break;
    default:
      break;
  }

  return 0;
}

void peer_connection_set_remote_description(PeerConnection* pc, const char* sdp, SdpType type) {
  static const char fingerprint_prefix[] = "a=fingerprint:sha-256 ";
  static const char ice_ufrag_prefix[] = "a=ice-ufrag:";
  const char* start;
  const char* line;
  const char* sdp_end;
  size_t sdp_len;
  uint32_t* ssrc = NULL;
  DtlsSrtpRole role = DTLS_SRTP_ROLE_SERVER;
  int is_update = 0;
  Agent* agent;

  if (pc == NULL || sdp == NULL) {
    return;
  }
  agent = &pc->agent;
  sdp_len = strnlen(sdp, AGENT_MAX_DESCRIPTION + 1);
  if (sdp_len > AGENT_MAX_DESCRIPTION) {
    LOGE("Remote SDP exceeds maximum length.");
    STATE_CHANGED(pc, PEER_CONNECTION_FAILED);
    return;
  }
  start = sdp;
  sdp_end = sdp + sdp_len;

  while (start < sdp_end) {
    size_t line_len;
    line = strstr(start, "\r\n");
    if (line == NULL) {
      line = sdp_end;
    }
    line_len = (size_t)(line - start);

    if (line_len == strlen("a=setup:passive") &&
        memcmp(start, "a=setup:passive", line_len) == 0) {
      role = DTLS_SRTP_ROLE_CLIENT;
    }

    if (line_len >= sizeof(fingerprint_prefix) - 1 &&
        memcmp(start, fingerprint_prefix, sizeof(fingerprint_prefix) - 1) == 0) {
      size_t fingerprint_len = line_len - (sizeof(fingerprint_prefix) - 1);
      if (fingerprint_len == 0 ||
          fingerprint_len >= sizeof(pc->dtls_srtp.remote_fingerprint)) {
        LOGE("Invalid remote DTLS fingerprint length.");
        STATE_CHANGED(pc, PEER_CONNECTION_FAILED);
        return;
      }
      memcpy(pc->dtls_srtp.remote_fingerprint,
             start + sizeof(fingerprint_prefix) - 1, fingerprint_len);
      pc->dtls_srtp.remote_fingerprint[fingerprint_len] = '\0';
      LOGD("remote fingerprint: %s", pc->dtls_srtp.remote_fingerprint);
    }

    if (line_len >= sizeof(ice_ufrag_prefix) - 1 &&
        memcmp(start, ice_ufrag_prefix, sizeof(ice_ufrag_prefix) - 1) == 0 &&
        agent->remote_ufrag[0] != '\0') {
      size_t remote_ufrag_len = strlen(agent->remote_ufrag);
      size_t offered_ufrag_len = line_len - (sizeof(ice_ufrag_prefix) - 1);
      if (offered_ufrag_len == remote_ufrag_len &&
          memcmp(start + sizeof(ice_ufrag_prefix) - 1,
                 agent->remote_ufrag, remote_ufrag_len) == 0) {
        is_update = 1;
      }
    }

    if (line_len >= strlen("m=video") &&
        memcmp(start, "m=video", strlen("m=video")) == 0) {
      ssrc = &pc->remote_vssrc;
      if (type == SDP_TYPE_ANSWER) {
        const char* port_str = start + strlen("m=video") + 1;
        if (port_str < line && *port_str == '0' &&
            (port_str + 1 == line || port_str[1] < '0' || port_str[1] > '9')) {
          LOGE("Remote answer rejected video m-line (port=0): browser has no "
               "compatible video codec, e.g. no H.265/HEVC decode support. "
               "Try restarting with -t h264.");
        }
      }
    } else if (line_len >= strlen("m=audio") &&
               memcmp(start, "m=audio", strlen("m=audio")) == 0) {
      ssrc = &pc->remote_assrc;
      if (type == SDP_TYPE_ANSWER) {
        const char* port_str = start + strlen("m=audio") + 1;
        if (port_str < line && *port_str == '0' &&
            (port_str + 1 == line || port_str[1] < '0' || port_str[1] > '9')) {
          LOGE("Remote answer rejected audio m-line (port=0): browser has no "
               "compatible audio codec.");
        }
      }
    }

    if (ssrc && line_len > strlen("a=ssrc:") &&
        memcmp(start, "a=ssrc:", strlen("a=ssrc:")) == 0) {
      const char* value = start + strlen("a=ssrc:");
      uint64_t parsed = 0;
      int has_digit = 0;
      while (value < line && *value >= '0' && *value <= '9') {
        has_digit = 1;
        parsed = parsed * 10 + (uint64_t)(*value - '0');
        if (parsed > UINT32_MAX) {
          has_digit = 0;
          break;
        }
        value++;
      }
      if (has_digit) {
        *ssrc = (uint32_t)parsed;
        LOGD("SSRC: %" PRIu32, *ssrc);
      }
    }

    if (line == sdp_end) {
      break;
    }
    start = line + 2;
  }

  if (is_update) {
    return;
  }

  if (agent_set_remote_description(&pc->agent, (char*)sdp) != 0) {
    LOGE("Invalid remote ICE description.");
    STATE_CHANGED(pc, PEER_CONNECTION_FAILED);
    return;
  }
  if (type == SDP_TYPE_ANSWER) {
    agent_update_candidate_pairs(&pc->agent);
    STATE_CHANGED(pc, PEER_CONNECTION_CHECKING);
  }
}

static const char* peer_connection_create_sdp(PeerConnection* pc, SdpType sdp_type) {
  char* description = (char*)pc->temp_buf;

  memset(pc->temp_buf, 0, sizeof(pc->temp_buf));
  DtlsSrtpRole role = DTLS_SRTP_ROLE_SERVER;

  sctp_reset(&pc->sctp);

  switch (sdp_type) {
    case SDP_TYPE_OFFER:
      role = DTLS_SRTP_ROLE_SERVER;
      if (pc->agent.turn_relay_ready || pc->agent.turn_server_addr.family != 0) {
        LOGI("New offer: releasing previous TURN allocation before re-gathering candidates");
        agent_turn_deallocate(&pc->agent);
        /* Close and reopen the UDP socket to get a new source port.
         * This guarantees a different 5-tuple (src_ip:NEW_port → turn_ip:turn_port),
         * so the new Allocate request cannot conflict with the just-released
         * allocation on the TURN server — eliminating the 437 Mismatch error
         * that occurs when the server hasn't fully processed the deallocation. */
        if (agent_reopen_udp_socket(&pc->agent) != 0) {
          STATE_CHANGED(pc, PEER_CONNECTION_FAILED);
          return NULL;
        }
      }
      agent_clear_candidates(&pc->agent);
      pc->agent.mode = AGENT_MODE_CONTROLLING;
      break;
    case SDP_TYPE_ANSWER:
      role = DTLS_SRTP_ROLE_CLIENT;
      pc->agent.mode = AGENT_MODE_CONTROLLED;
      break;
    default:
      break;
  }

  peer_connection_clear_client_hello(pc);
  dtls_srtp_reset_session(&pc->dtls_srtp);
  if (dtls_srtp_init(&pc->dtls_srtp, role, pc) != 0) {
    STATE_CHANGED(pc, PEER_CONNECTION_FAILED);
    return NULL;
  }
  pc->dtls_srtp.udp_recv = peer_connection_dtls_srtp_recv;
  pc->dtls_srtp.udp_send = peer_connection_dtls_srtp_send;

  memset(pc->sdp, 0, sizeof(pc->sdp));
  // TODO: check if we have video or audio codecs
  sdp_create(pc->sdp,
             pc->config.video_codec != CODEC_NONE,
             pc->config.audio_codec != CODEC_NONE,
             pc->config.datachannel);

  if (agent_create_ice_credential(&pc->agent) != 0) {
    STATE_CHANGED(pc, PEER_CONNECTION_FAILED);
    return NULL;
  }
  sdp_append(pc->sdp, "a=ice-ufrag:%s", pc->agent.local_ufrag);
  sdp_append(pc->sdp, "a=ice-pwd:%s", pc->agent.local_upwd);
  sdp_append(pc->sdp, "a=fingerprint:sha-256 %s", pc->dtls_srtp.local_fingerprint);
  sdp_append(pc->sdp, peer_connection_dtls_role_setup_value(role));

  pc->b_local_description_created = 1;

  agent_gather_candidate(&pc->agent, NULL, NULL, NULL);  // host address
  for (int i = 0; i < sizeof(pc->config.ice_servers) / sizeof(pc->config.ice_servers[0]); ++i) {
    if (pc->config.ice_servers[i].urls) {
      LOGI("ice server: %s", pc->config.ice_servers[i].urls);
      agent_gather_candidate(&pc->agent, pc->config.ice_servers[i].urls, pc->config.ice_servers[i].username, pc->config.ice_servers[i].credential);
    }
  }

  // Candidates must be in the first m= section for BUNDLE;
  // Chrome only extracts candidates from the BUNDLE-base section.
  // Build: m=video → candidates → m=audio → m=datachannel
  if (pc->config.video_codec == CODEC_H264) {
    sdp_append_h264(pc->sdp);
  } else if (pc->config.video_codec == CODEC_H265) {
    sdp_append_h265(pc->sdp);
  }

  agent_get_local_description(&pc->agent, description, sizeof(pc->temp_buf));
  sdp_append(pc->sdp, description);

  switch (pc->config.audio_codec) {
    case CODEC_PCMA:
      sdp_append_pcma(pc->sdp);
      break;
    case CODEC_PCMU:
      sdp_append_pcmu(pc->sdp);
      break;
    case CODEC_OPUS:
      sdp_append_opus(pc->sdp, pc->config.audio_sample_rate);
      break;
    default:
      break;
  }

  if (pc->config.datachannel) {
    sdp_append_datachannel(pc->sdp);
  }

  if (pc->onicecandidate) {
    pc->onicecandidate(pc->sdp, pc->config.user_data);
  }

  return pc->sdp;
}

const char* peer_connection_create_offer(PeerConnection* pc) {
  return peer_connection_create_sdp(pc, SDP_TYPE_OFFER);
}

const char* peer_connection_create_answer(PeerConnection* pc) {
  const char* sdp = peer_connection_create_sdp(pc, SDP_TYPE_ANSWER);
  if (sdp == NULL) {
    return NULL;
  }
  agent_update_candidate_pairs(&pc->agent);
  STATE_CHANGED(pc, PEER_CONNECTION_CHECKING);
  return sdp;
}

int peer_connection_send_rtcp_pil(PeerConnection* pc, uint32_t ssrc) {
  int ret = -1;
  uint8_t plibuf[128];
  rtcp_get_pli(plibuf, 12, ssrc);

  // TODO: encrypt rtcp packet
  // guint size = 12;
  // dtls_transport_encrypt_rctp_packet(pc->dtls_transport, plibuf, &size);
  // ret = nice_agent_send(pc->nice_agent, pc->stream_id, pc->component_id, size, (gchar*)plibuf);

  return ret;
}

// callbacks
void peer_connection_on_connected(PeerConnection* pc, void (*on_connected)(void* userdata)) {
  pc->on_connected = on_connected;
}

void peer_connection_on_receiver_packet_loss(PeerConnection* pc,
                                             void (*on_receiver_packet_loss)(float fraction_loss, uint32_t total_loss, void* userdata)) {
  pc->on_receiver_packet_loss = on_receiver_packet_loss;
}

void peer_connection_onicecandidate(PeerConnection* pc, void (*onicecandidate)(char* sdp, void* userdata)) {
  pc->onicecandidate = onicecandidate;
}

void peer_connection_oniceconnectionstatechange(PeerConnection* pc,
                                                void (*oniceconnectionstatechange)(PeerConnectionState state, void* userdata)) {
  pc->oniceconnectionstatechange = oniceconnectionstatechange;
}

void peer_connection_ondatachannel(PeerConnection* pc,
                                   void (*onmessage)(char* msg, size_t len, void* userdata, uint16_t sid),
                                   void (*onopen)(void* userdata),
                                   void (*onclose)(void* userdata)) {
  if (pc) {
    sctp_onopen(&pc->sctp, onopen);
    sctp_onclose(&pc->sctp, onclose);
    sctp_onmessage(&pc->sctp, onmessage);
  }
}

int peer_connection_lookup_sid(PeerConnection* pc, const char* label, uint16_t* sid) {
  for (int i = 0; i < pc->sctp.stream_count; i++) {
    if (strncmp(pc->sctp.stream_table[i].label, label, sizeof(pc->sctp.stream_table[i].label)) == 0) {
      *sid = pc->sctp.stream_table[i].sid;
      return 0;
    }
  }
  return -1;  // Not found
}

char* peer_connection_lookup_sid_label(PeerConnection* pc, uint16_t sid) {
  for (int i = 0; i < pc->sctp.stream_count; i++) {
    if (pc->sctp.stream_table[i].sid == sid) {
      return pc->sctp.stream_table[i].label;
    }
  }
  return NULL;  // Not found
}

int peer_connection_add_ice_candidate(PeerConnection* pc, char* candidate) {
  Agent* agent;
  IceCandidate parsed_candidate;
  int i;

  if (pc == NULL || candidate == NULL) {
    return -1;
  }
  agent = &pc->agent;
  if (ice_candidate_from_description(&parsed_candidate, candidate,
                                     candidate + strlen(candidate)) != 0) {
    return -1;
  }
  for (i = 0; i < agent->remote_candidates_count; i++) {
    if (ice_candidate_equal(&agent->remote_candidates[i], &parsed_candidate)) {
      return 0;
    }
  }
  if (agent->remote_candidates_count >= AGENT_MAX_CANDIDATES) {
    LOGE("Too many remote ICE candidates");
    return -1;
  }
  agent->remote_candidates[agent->remote_candidates_count] = parsed_candidate;
  LOGD("Add candidate: %s", candidate);
  agent->remote_candidates_count++;
  return 0;
}
