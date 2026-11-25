#include <stdio.h>
#include <string.h>

#include "address.h"
#include "config.h"
#include "peer_connection.h"
#include "rtp.h"
#include "utils.h"

typedef enum RtpH264Type {

  NALU = 23,
  FU_A = 28,

} RtpH264Type;

typedef struct NaluHeader {
  uint8_t type : 5;
  uint8_t nri : 2;
  uint8_t f : 1;
} NaluHeader;

typedef struct FuHeader {
  uint8_t type : 5;
  uint8_t r : 1;
  uint8_t e : 1;
  uint8_t s : 1;
} FuHeader;

/* H.265/HEVC RTP definitions (RFC 7798) */
#define H265_NAL_FU 49
#define H265_NAL_TYPE(hdr) ((hdr[0] >> 1) & 0x3F)
#define H265_FU_PAYLOAD_SIZE (CONFIG_MTU - sizeof(RtpHeader) - 3) /* PayloadHdr(2) + FuHeader(1) */

typedef struct H265FuHeader {
  uint8_t type : 6; /* FuType: original NAL unit type */
  uint8_t e : 1;    /* End fragment */
  uint8_t s : 1;    /* Start fragment */
} H265FuHeader;

#define RTP_PAYLOAD_SIZE (CONFIG_MTU - sizeof(RtpHeader))
#define FU_PAYLOAD_SIZE (CONFIG_MTU - sizeof(RtpHeader) - sizeof(FuHeader) - sizeof(NaluHeader))

int rtp_packet_validate(uint8_t* packet, size_t size) {
  if (size < 12)
    return 0;

  RtpHeader* rtp_header = (RtpHeader*)packet;
  return ((rtp_header->type < 64) || (rtp_header->type >= 96));
}

uint32_t rtp_get_ssrc(uint8_t* packet) {
  RtpHeader* rtp_header = (RtpHeader*)packet;
  return ntohl(rtp_header->ssrc);
}

static int rtp_encoder_encode_h264_single(RtpEncoder* rtp_encoder, uint8_t* buf, size_t size) {
  RtpPacket* rtp_packet = (RtpPacket*)rtp_encoder->buf;

  rtp_packet->header.version = 2;
  rtp_packet->header.padding = 0;
  rtp_packet->header.extension = 0;
  rtp_packet->header.csrccount = 0;
  rtp_packet->header.markerbit = 0;
  rtp_packet->header.type = rtp_encoder->type;
  rtp_packet->header.seq_number = htons(rtp_encoder->seq_number++);
  rtp_packet->header.timestamp = htonl(rtp_encoder->timestamp);
  rtp_packet->header.ssrc = htonl(rtp_encoder->ssrc);

  if ((*buf & 0x1f) == 0x05 || (*buf & 0x1f) == 0x01) {
    rtp_packet->header.markerbit = 1;
  }

  memcpy(rtp_packet->payload, buf, size);
  rtp_encoder->on_packet(rtp_encoder->buf, size + sizeof(RtpHeader), rtp_encoder->user_data);
  return 0;
}

static int rtp_encoder_encode_h264_fu_a(RtpEncoder* rtp_encoder, uint8_t* buf, size_t size) {
  RtpPacket* rtp_packet = (RtpPacket*)rtp_encoder->buf;

  rtp_packet->header.version = 2;
  rtp_packet->header.padding = 0;
  rtp_packet->header.extension = 0;
  rtp_packet->header.csrccount = 0;
  rtp_packet->header.markerbit = 0;
  rtp_packet->header.type = rtp_encoder->type;
  rtp_packet->header.timestamp = htonl(rtp_encoder->timestamp);
  rtp_packet->header.ssrc = htonl(rtp_encoder->ssrc);
  uint8_t type = buf[0] & 0x1f;
  uint8_t nri = (buf[0] & 0x60) >> 5;
  buf = buf + 1;
  size = size - 1;

  NaluHeader* fu_indicator = (NaluHeader*)rtp_packet->payload;
  FuHeader* fu_header = (FuHeader*)rtp_packet->payload + sizeof(NaluHeader);
  fu_header->s = 1;

  while (size > 0) {
    fu_indicator->type = FU_A;
    fu_indicator->nri = nri;
    fu_indicator->f = 0;
    fu_header->type = type;
    fu_header->r = 0;
    rtp_packet->header.seq_number = htons(rtp_encoder->seq_number++);

    if (size <= FU_PAYLOAD_SIZE) {
      fu_header->e = 1;
      rtp_packet->header.markerbit = 1;
      memcpy(rtp_packet->payload + sizeof(NaluHeader) + sizeof(FuHeader), buf, size);
      rtp_encoder->on_packet(rtp_encoder->buf, size + sizeof(RtpHeader) + sizeof(NaluHeader) + sizeof(FuHeader), rtp_encoder->user_data);
      break;
    }

    fu_header->e = 0;

    memcpy(rtp_packet->payload + sizeof(NaluHeader) + sizeof(FuHeader), buf, FU_PAYLOAD_SIZE);
    rtp_encoder->on_packet(rtp_encoder->buf, CONFIG_MTU, rtp_encoder->user_data);
    size -= FU_PAYLOAD_SIZE;
    buf += FU_PAYLOAD_SIZE;

    fu_header->s = 0;
  }
  return 0;
}

static uint8_t* h264_find_nalu(uint8_t* buf_start, uint8_t* buf_end) {
  if (buf_end - buf_start < 3)
    return buf_end;

  uint8_t* p = buf_start + 2;

  while (p < buf_end) {
    if (*(p - 2) == 0x00 && *(p - 1) == 0x00 && *p == 0x01)
      return p + 1;
    p++;
  }

  return buf_end;
}

static int rtp_encoder_encode_h264(RtpEncoder* rtp_encoder, uint8_t* buf, size_t size) {
  uint8_t* buf_end = buf + size;
  uint8_t *pstart, *pend;
  size_t nalu_size;

  for (pstart = h264_find_nalu(buf, buf_end); pstart < buf_end; pstart = pend) {
    pend = h264_find_nalu(pstart, buf_end);
    nalu_size = pend - pstart;

    if (pend != buf_end)
      nalu_size--;

    while (nalu_size > 0 && pstart[nalu_size - 1] == 0x00)
      nalu_size--;

    if (nalu_size == 0)
      continue;

    if (nalu_size <= RTP_PAYLOAD_SIZE) {
      rtp_encoder_encode_h264_single(rtp_encoder, pstart, nalu_size);

    } else {
      rtp_encoder_encode_h264_fu_a(rtp_encoder, pstart, nalu_size);
    }
  }

  return 0;
}

static int rtp_encoder_encode_h265_single(RtpEncoder* rtp_encoder, uint8_t* buf, size_t size) {
  RtpPacket* rtp_packet = (RtpPacket*)rtp_encoder->buf;

  rtp_packet->header.version = 2;
  rtp_packet->header.padding = 0;
  rtp_packet->header.extension = 0;
  rtp_packet->header.csrccount = 0;
  rtp_packet->header.markerbit = 0;
  rtp_packet->header.type = rtp_encoder->type;
  rtp_packet->header.seq_number = htons(rtp_encoder->seq_number++);
  rtp_packet->header.timestamp = htonl(rtp_encoder->timestamp);
  rtp_packet->header.ssrc = htonl(rtp_encoder->ssrc);

  /* Set markerbit for VCL NALs: TRAIL_R(1), IDR_W_RADL(19), IDR_N_LP(20), CRA(21) */
  uint8_t nal_type = H265_NAL_TYPE(buf);
  if (nal_type == 1 || nal_type == 19 || nal_type == 20 || nal_type == 21) {
    rtp_packet->header.markerbit = 1;
  }

  memcpy(rtp_packet->payload, buf, size);
  rtp_encoder->on_packet(rtp_encoder->buf, size + sizeof(RtpHeader), rtp_encoder->user_data);
  return 0;
}

static int rtp_encoder_encode_h265_fu(RtpEncoder* rtp_encoder, uint8_t* buf, size_t size) {
  RtpPacket* rtp_packet = (RtpPacket*)rtp_encoder->buf;

  rtp_packet->header.version = 2;
  rtp_packet->header.padding = 0;
  rtp_packet->header.extension = 0;
  rtp_packet->header.csrccount = 0;
  rtp_packet->header.markerbit = 0;
  rtp_packet->header.type = rtp_encoder->type;
  rtp_packet->header.timestamp = htonl(rtp_encoder->timestamp);
  rtp_packet->header.ssrc = htonl(rtp_encoder->ssrc);

  uint8_t nal_type = H265_NAL_TYPE(buf);
  /* PayloadHdr: keep F bit and LayerId/TID from original, replace type with FU(49) */
  uint8_t payload_hdr_0 = (buf[0] & 0x81) | (H265_NAL_FU << 1);
  uint8_t payload_hdr_1 = buf[1];

  buf = buf + 2; /* skip original 2-byte NAL header */
  size = size - 2;

  H265FuHeader* fu_header = (H265FuHeader*)(rtp_packet->payload + 2);
  fu_header->s = 1;

  while (size > 0) {
    rtp_packet->payload[0] = payload_hdr_0;
    rtp_packet->payload[1] = payload_hdr_1;
    fu_header->type = nal_type;
    rtp_packet->header.seq_number = htons(rtp_encoder->seq_number++);

    if (size <= H265_FU_PAYLOAD_SIZE) {
      fu_header->e = 1;
      rtp_packet->header.markerbit = 1;
      memcpy(rtp_packet->payload + 3, buf, size);
      rtp_encoder->on_packet(rtp_encoder->buf, size + sizeof(RtpHeader) + 3, rtp_encoder->user_data);
      break;
    }

    fu_header->e = 0;

    memcpy(rtp_packet->payload + 3, buf, H265_FU_PAYLOAD_SIZE);
    rtp_encoder->on_packet(rtp_encoder->buf, CONFIG_MTU, rtp_encoder->user_data);
    size -= H265_FU_PAYLOAD_SIZE;
    buf += H265_FU_PAYLOAD_SIZE;

    fu_header->s = 0;
  }

  return 0;
}

static int rtp_encoder_encode_h265(RtpEncoder* rtp_encoder, uint8_t* buf, size_t size) {
  uint8_t* buf_end = buf + size;
  uint8_t *pstart, *pend;
  size_t nalu_size;

  /* H.265 uses the same Annex B start codes as H.264 */
  for (pstart = h264_find_nalu(buf, buf_end); pstart < buf_end; pstart = pend) {
    pend = h264_find_nalu(pstart, buf_end);
    nalu_size = pend - pstart;

    if (pend != buf_end)
      nalu_size--;

    while (nalu_size > 0 && pstart[nalu_size - 1] == 0x00)
      nalu_size--;

    if (nalu_size < 2)
      continue;

    if (nalu_size <= RTP_PAYLOAD_SIZE) {
      rtp_encoder_encode_h265_single(rtp_encoder, pstart, nalu_size);
    } else {
      rtp_encoder_encode_h265_fu(rtp_encoder, pstart, nalu_size);
    }
  }

  return 0;
}

static int rtp_encoder_encode_generic(RtpEncoder* rtp_encoder, uint8_t* buf, size_t size) {
  RtpHeader* rtp_header = (RtpHeader*)rtp_encoder->buf;
  rtp_header->version = 2;
  rtp_header->padding = 0;
  rtp_header->extension = 0;
  rtp_header->csrccount = 0;
  rtp_header->markerbit = 0;
  rtp_header->type = rtp_encoder->type;
  rtp_header->seq_number = htons(rtp_encoder->seq_number++);
  rtp_header->timestamp = htonl(rtp_encoder->timestamp);
  rtp_header->ssrc = htonl(rtp_encoder->ssrc);
  memcpy(rtp_encoder->buf + sizeof(RtpHeader), buf, size);

  rtp_encoder->on_packet(rtp_encoder->buf, size + sizeof(RtpHeader), rtp_encoder->user_data);

  return 0;
}

void rtp_encoder_init(RtpEncoder* rtp_encoder, MediaCodec codec, RtpOnPacket on_packet, void* user_data) {
  rtp_encoder->on_packet = on_packet;
  rtp_encoder->user_data = user_data;
  rtp_encoder->seq_number = 0;
  rtp_encoder->clock_rate = 0;
  rtp_encoder->base_timestamp_us = 0;

  switch (codec) {
    case CODEC_H264:
      rtp_encoder->type = PT_H264;
      rtp_encoder->ssrc = SSRC_H264;
      rtp_encoder->clock_rate = 90000;
      rtp_encoder->encode_func = rtp_encoder_encode_h264;
      break;
    case CODEC_H265:
      rtp_encoder->type = PT_H265;
      rtp_encoder->ssrc = SSRC_H265;
      rtp_encoder->clock_rate = 90000;
      rtp_encoder->encode_func = rtp_encoder_encode_h265;
      break;
    case CODEC_PCMA:
      rtp_encoder->type = PT_PCMA;
      rtp_encoder->ssrc = SSRC_PCMA;
      rtp_encoder->clock_rate = 8000;
      rtp_encoder->encode_func = rtp_encoder_encode_generic;
      break;
    case CODEC_PCMU:
      rtp_encoder->type = PT_PCMU;
      rtp_encoder->ssrc = SSRC_PCMU;
      rtp_encoder->clock_rate = 8000;
      rtp_encoder->encode_func = rtp_encoder_encode_generic;
      break;
    case CODEC_OPUS:
      rtp_encoder->type = PT_OPUS;
      rtp_encoder->ssrc = SSRC_OPUS;
      rtp_encoder->clock_rate = 48000;
      rtp_encoder->encode_func = rtp_encoder_encode_generic;
      break;
    default:
      break;
  }
}

int rtp_encoder_encode(RtpEncoder* rtp_encoder, const uint8_t* buf, size_t size, uint64_t timestamp_us) {
  if (rtp_encoder == NULL || rtp_encoder->encode_func == NULL || buf == NULL || size == 0)
    return -1;

  if (rtp_encoder->base_timestamp_us == 0)
    rtp_encoder->base_timestamp_us = timestamp_us;
  uint64_t delta_us = timestamp_us - rtp_encoder->base_timestamp_us;
  uint32_t rtp_ts = (uint32_t)((delta_us * rtp_encoder->clock_rate) / 1000000);
  rtp_encoder->timestamp = rtp_ts;
  return rtp_encoder->encode_func(rtp_encoder, (uint8_t*)buf, size);
}

static int rtp_decoder_append_nalu(RtpDecoder* rtp_decoder,
                                   const void* data, size_t size) {
  if (size > sizeof(rtp_decoder->nalu_buf) - rtp_decoder->nalu_offset) {
    rtp_decoder->nalu_offset = 0;
    return -1;
  }

  memcpy(rtp_decoder->nalu_buf + rtp_decoder->nalu_offset, data, size);
  rtp_decoder->nalu_offset += size;
  return 0;
}

static int rtp_decode_h264(RtpDecoder* rtp_decoder, uint8_t* buf, size_t size) {
  static const uint32_t nalu_start_4bytecode = 0x01000000;
  if (size <= sizeof(RtpHeader)) {
    return -1;
  }

  RtpPacket* rtp_packet = (RtpPacket*)buf;
  uint8_t nalu_type = *rtp_packet->payload & 0x1f;
  size_t payload_size = size - sizeof(RtpHeader);
  if (nalu_type > 0 && nalu_type < 24) {
    // NALU type 1-23 are single NALUs
    rtp_decoder->nalu_offset = 0;
    if (rtp_decoder_append_nalu(rtp_decoder, &nalu_start_4bytecode,
                                sizeof(nalu_start_4bytecode)) != 0 ||
        rtp_decoder_append_nalu(rtp_decoder, rtp_packet->payload,
                                payload_size) != 0) {
      return -1;
    }
    if (rtp_decoder->on_packet != NULL) {
      rtp_decoder->on_packet(rtp_decoder->nalu_buf, rtp_decoder->nalu_offset,
                             rtp_decoder->user_data);
    }
    rtp_decoder->nalu_offset = 0;
    return (int)size;
  } else if (nalu_type == FU_A && payload_size >= 2) {
    NaluHeader* fu_indicator = (NaluHeader*)rtp_packet->payload;
    FuHeader* fu_header = (FuHeader*)(rtp_packet->payload + sizeof(NaluHeader));
    uint8_t reconstructed_nalu_type = (fu_indicator->f << 7) |
                                      (fu_indicator->nri << 5) |
                                      fu_header->type;
    payload_size -= sizeof(NaluHeader) + sizeof(FuHeader);
    if (fu_header->s) {
      rtp_decoder->nalu_offset = 0;
      if (rtp_decoder_append_nalu(rtp_decoder, &nalu_start_4bytecode,
                                  sizeof(nalu_start_4bytecode)) != 0 ||
          rtp_decoder_append_nalu(rtp_decoder, &reconstructed_nalu_type, 1) != 0 ||
          rtp_decoder_append_nalu(rtp_decoder, rtp_packet->payload + 2,
                                  payload_size) != 0) {
        return -1;
      }
    } else if (rtp_decoder->nalu_offset != 0) {
      if (rtp_decoder_append_nalu(rtp_decoder, rtp_packet->payload + 2,
                                  payload_size) != 0) {
        return -1;
      }
      if (fu_header->e) {
        // end of fragmented NALU
        if (rtp_decoder->on_packet != NULL) {
          rtp_decoder->on_packet(rtp_decoder->nalu_buf,
                                 rtp_decoder->nalu_offset,
                                 rtp_decoder->user_data);
        }
        rtp_decoder->nalu_offset = 0;
      }
    }
  } else {
    rtp_decoder->nalu_offset = 0;
    return -1;
  }
  return 0;
}

static int rtp_decode_h265(RtpDecoder* rtp_decoder, uint8_t* buf, size_t size) {
  static const uint32_t nalu_start_4bytecode = 0x01000000;
  if (size < sizeof(RtpHeader) + 2) {
    return -1;
  }

  RtpPacket* rtp_packet = (RtpPacket*)buf;
  uint8_t nal_type = H265_NAL_TYPE(rtp_packet->payload);
  size_t payload_size = size - sizeof(RtpHeader);

  if (nal_type < 48) {
    /* Single NAL unit packet (types 0-47, excluding AP=48 and FU=49) */
    rtp_decoder->nalu_offset = 0;
    if (rtp_decoder_append_nalu(rtp_decoder, &nalu_start_4bytecode,
                                sizeof(nalu_start_4bytecode)) != 0 ||
        rtp_decoder_append_nalu(rtp_decoder, rtp_packet->payload,
                                payload_size) != 0) {
      return -1;
    }
    if (rtp_decoder->on_packet != NULL) {
      rtp_decoder->on_packet(rtp_decoder->nalu_buf, rtp_decoder->nalu_offset,
                             rtp_decoder->user_data);
    }
    rtp_decoder->nalu_offset = 0;
    return (int)size;
  } else if (nal_type == H265_NAL_FU && payload_size >= 3) {
    /* Fragmentation Unit */
    H265FuHeader* fu_header = (H265FuHeader*)(rtp_packet->payload + 2);
    payload_size -= 3; /* PayloadHdr(2) + FuHeader(1) */

    if (fu_header->s) {
      /* Start of FU: reconstruct original 2-byte NAL header */
      uint8_t nal_hdr[2];
      nal_hdr[0] = (rtp_packet->payload[0] & 0x81) | (fu_header->type << 1);
      nal_hdr[1] = rtp_packet->payload[1];
      rtp_decoder->nalu_offset = 0;
      if (rtp_decoder_append_nalu(rtp_decoder, &nalu_start_4bytecode,
                                  sizeof(nalu_start_4bytecode)) != 0 ||
          rtp_decoder_append_nalu(rtp_decoder, nal_hdr, sizeof(nal_hdr)) != 0 ||
          rtp_decoder_append_nalu(rtp_decoder, rtp_packet->payload + 3,
                                  payload_size) != 0) {
        return -1;
      }
    } else if (rtp_decoder->nalu_offset != 0) {
      if (rtp_decoder_append_nalu(rtp_decoder, rtp_packet->payload + 3,
                                  payload_size) != 0) {
        return -1;
      }
      if (fu_header->e) {
        /* End of FU */
        if (rtp_decoder->on_packet != NULL) {
          rtp_decoder->on_packet(rtp_decoder->nalu_buf,
                                 rtp_decoder->nalu_offset,
                                 rtp_decoder->user_data);
        }
        rtp_decoder->nalu_offset = 0;
      }
    }
  } else {
    rtp_decoder->nalu_offset = 0;
    return -1;
  }

  return 0;
}

static int rtp_decode_generic(RtpDecoder* rtp_decoder, uint8_t* buf, size_t size) {
  RtpPacket* rtp_packet = (RtpPacket*)buf;
  if (rtp_decoder->on_packet != NULL)
    rtp_decoder->on_packet(rtp_packet->payload, size - sizeof(RtpHeader), rtp_decoder->user_data);
  // even if there is no callback set, assume everything is ok for caller and do not return an error
  return (int)size;
}

void rtp_decoder_init(RtpDecoder* rtp_decoder, MediaCodec codec, RtpOnPacket on_packet, void* user_data) {
  rtp_decoder->on_packet = on_packet;
  rtp_decoder->decode_func = NULL;
  rtp_decoder->user_data = user_data;
  rtp_decoder->nalu_offset = 0;

  switch (codec) {
    case CODEC_H264:
      rtp_decoder->decode_func = rtp_decode_h264;
      break;
    case CODEC_H265:
      rtp_decoder->decode_func = rtp_decode_h265;
      break;
    case CODEC_PCMA:
    case CODEC_PCMU:
    case CODEC_OPUS:
      rtp_decoder->decode_func = rtp_decode_generic;
    default:
      break;
  }
}

int rtp_decoder_decode(RtpDecoder* rtp_decoder, const uint8_t* buf, size_t size) {
  if (rtp_decoder->decode_func == NULL)
    return -1;
  return rtp_decoder->decode_func(rtp_decoder, (uint8_t*)buf, size);
}
