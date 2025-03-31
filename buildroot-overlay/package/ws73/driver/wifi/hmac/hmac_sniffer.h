/*
 * Copyright (c) CompanyNameMagicTag 2021-2021. All rights reserved.
 * Description: hook function for wifi sniffer.
 */

#ifndef __HMAC_SNIFFER_H
#define __HMAC_SNIFFER_H

#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) \
    && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include "oal_schedule.h"
#include "mac_frame.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#if defined(_PRE_WLAN_FEATURE_SNIFFER)

#define PCAP_FILE_NUM 5
#define PCAP_FILE_NUM_MAX 5
#define PCAP_FILE_NUM_MIN 1
#define PCAP_FILE_LEN (30 * 1024 * 1024)
#define PCAP_FILE_LEN_MIN (1 * 1024 * 1024)
#define PCAP_FILE_LEN_MAX (50 * 1024 * 1024)

#define PCAP_PKT_MIN_LEN 24
#define PCAP_PKT_MAX_LEN 5000

#define PCAP_PKT_HDR_MAGIC 0xa1b2c3d4
#define PCAP_PKT_HDR_VER_MAJ 0x2
#define PCAP_PKT_HDR_VER_MIN 0x4
#define PCAP_PKT_HDR_THISZONE 0x0
#define PCAP_PKT_HDR_SIGFIGS 0x0
#define PCAP_PKT_HDR_SNAPLEN 0xffff
#define PCAP_PKT_HDR_LINKTYPE_802_11 0x69
#define PCAP_PKT_HDR_LINKTYPE_802_11_RADIOTAP 127

#define IEEE80211_MAC_HDR_LEN 26
#define TRIM_DATA_PAYLOAD_LEN 2000
#define IEEE80211_FRAME_TYPE_DATA 0x02
#define IEEE80211_FRAME_TYPE_MGMT 0x00
#define IEEE80211_FRAME_SUBTYPE_QOS 0x08
#define IEEE80211_FRAME_SUBTYPE_DISASSOC 0x0a

#define MAC_ADDR_LEN 6

#define DIRECTION_TX 0
#define DIRECTION_RX 1

#define PCAP_QOS_DATA_PKT_HDR_LEN 26

/* frame control structure */
typedef struct {
    osal_u16  bit_protocol_version    : 2,    /* protocol version */
                bit_type                : 2,    /* frame type */
                bit_sub_type            : 4,    /* subtype */
                bit_to_ds               : 1,    /* to DS */
                bit_from_ds             : 1,    /* from DS */
                bit_more_frag           : 1,    /* more fragment */
                bit_retry               : 1,    /* retransmit frame */
                bit_power_mgmt          : 1,    /* power management */
                bit_more_data           : 1,    /* more data flag */
                bit_protected_frame     : 1,    /* encrypt flag */
                bit_order               : 1;    /* order bit */
} mac_header_control_stru;

/* qos frame 2+2+ 6*3 + 2 +1 +1 = 26 */
typedef struct __attribute__ ((__packed__)) {
    mac_header_control_stru st_frame_control;
    osal_u16              bit_duration_value      : 15, /* duration/id */
                            bit_duration_flag       : 1;
    osal_u8               auc_address1[MAC_ADDR_LEN];
    osal_u8               auc_address2[MAC_ADDR_LEN];
    osal_u8               auc_address3[MAC_ADDR_LEN];
    osal_u16              bit_frag_num            : 4, /* sequence control */
                            bit_seq_num             : 12;
    osal_u8               bit_qc_tid              : 4,
                            bit_qc_eosp             : 1,
                            bit_qc_ack_polocy       : 2,
                            bit_qc_amsdu            : 1;
    union {
        osal_u8           bit_qc_txop_limit;
        osal_u8           bit_qc_ps_buf_state_resv        : 1,
                            bit_qc_ps_buf_state_inducated   : 1,
                            bit_qc_high_priority_buf_ac     : 2,
                            bit_qc_qosap_buf_load           : 4;
    } qos_control;
} mac_ieee80211_qos_stru;

typedef struct {
    osal_s32  sniffer_switch;
    osal_s32  cur_file;
    osal_s32  data_packet_max_len;
    osal_s32  max_file_num;
    osal_s32  max_file_len;
    osal_s32  new_file_num;
    osal_s32  new_file_len;
    osal_s32  file_full[PCAP_FILE_NUM];
    osal_s8  *buffer[PCAP_FILE_NUM];
    osal_s32  curpos[PCAP_FILE_NUM];
    osal_u32 gmttime[PCAP_FILE_NUM];        /* ms, between now and 1970-01-01 00:00:00 */
    osal_u32 microtime[PCAP_FILE_NUM];      /* us, between now and 1970-01-01 00:00:00 */
    osal_u32 packetcount[PCAP_FILE_NUM];
    // osal_u32 promis_count[3][15];           /* 3 帧的类型，15 子帧的类型 */
    osal_semaphore main_sem;
} proc_file_config_struct;

extern proc_file_config_struct g_sniffer_st;

#pragma pack(1)
typedef struct {
    osal_u32 magic;
    osal_u16 version_major;
    osal_u16 version_minor;
    osal_u32 thiszone;
    osal_u32 sigfigs;
    osal_u32 snaplen;
    osal_u32 linktype;
} wal_pcap_filehdr_stru;
#pragma pack()

/* 16 Bytes PCAP frame header's timestamp */
typedef struct {
    osal_u32 gmt_time; /* ms, between now and 1970-01-01 00:00:00 */
    osal_u32 micro_time; /* us, between now and 1970-01-01 00:00:00 */
} wal_pcap_time_stru;

/* PCAP frame header (16 Bytes) */
#pragma pack(1)
typedef struct {
    wal_pcap_time_stru time;    /* PCAP Frame Header's timestamp */
    osal_u32         caplen;  /* length of (Frame Header + Frame Body) */
    osal_u32         len;     /* packet's actual length, len == caplen in common */
} wal_pcap_pkthdr_stru;
#pragma pack()

/* 消息头 */
typedef struct {
    osal_u8 frame_start;
    osal_u8 func_type;
    osal_u8 prime_id;
    osal_u8 reserver[1];
    osal_u16 frame_len;
    osal_u16 sn;
} wal_msg_header_stru;

/* WIFI OTA头部信息 */
typedef struct {
    osal_u32                              tick;            /* 时间戳，精度ms */
    osal_u8                               en_ota_type;        /* OTA类型 */
    osal_u8                               frame_hdr_len;   /* 如果是帧，代表帧头长度；如果是其它类型，此字段没有用 */
    osal_u16                              us_ota_data_len;    /* OTA数据长度，如果是帧，代表帧头和帧体的总长度 */
    osal_u8                               user_macaddr[MAC_ADDR_LEN];
    oam_ota_frame_direction_type_enum_uint8 en_frame_direction; /* 表明帧是属于发送流程还是接收流程，只有ota_type是帧的时候这个字段才有意义 */
    osal_u8                               resv[1];        /* 区分产品 */
} wal_ota_hdr_stru;

typedef struct {
    wal_msg_header_stru oam_header;
    wal_ota_hdr_stru st_ota_hdr;
} wal_ota_stru;


static INLINE__ osal_bool sniffer_enough_to_save(osal_u32 packet_len)
{
    return (g_sniffer_st.curpos[g_sniffer_st.cur_file] + (packet_len)) <= g_sniffer_st.max_file_len;
}

static INLINE__ osal_s32 sniffer_file_pos(osal_s32 i)
{
    return g_sniffer_st.curpos[i];
}

static INLINE__ osal_s32 sniffer_cur_file_pos(osal_void)
{
    return sniffer_file_pos(g_sniffer_st.cur_file);
}

static INLINE__ osal_bool sniffer_is_cur_file_empty(osal_void)
{
    return sniffer_cur_file_pos() == 0;
}

static INLINE__ osal_s8 *sniffer_file_buffer(osal_s32 i)
{
    return g_sniffer_st.buffer[i];
}

static INLINE__ osal_s8 *sniffer_cur_file_buffer(osal_void)
{
    return sniffer_file_buffer(g_sniffer_st.cur_file);
}

osal_s32 proc_handle_command_sniffer_enable(osal_s32 sniffer_mode);
osal_s32 hmac_sniffer_get_switch(void);
void proc_handle_command_save_file(void);
osal_s32 proc_sniffer_reset_file_num_len(osal_s32 new_file_num, osal_s32 new_file_len);
osal_void hmac_sniffer_80211_frame(const osal_u8 *hdr_addr, osal_u8 hdr_len,
    const osal_u8 *frame_addr, osal_u16 frame_len, osal_u32 msg_id);

#ifdef __cplusplus
#if __cplusplus
    }
#endif
#endif

#endif
#endif

#endif
