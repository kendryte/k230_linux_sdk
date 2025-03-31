/*
 * Copyright (c) CompanyNameMagicTag 2021-2021. All rights reserved.
 * Description: Function related to wifi aware(NAN) feature.
 * Author: Huanghe
 * Create: 2021-04-16
 */

#ifndef __HMAC_SDP_H__
#define __HMAC_SDP_H__

#include "hmac_vap.h"
#include "frw_hmac.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define NAN_INTERVAL_VALUE      512
#define NAN_IE_ID               0xDD /* NAN同步帧IE id */
#define NAN_PERIOD_SMALL        128 /* 最小值用于建立同步窗口 */

#define SDP_TYPE_PUBLISH        0
#define SDP_TYPE_SUBSCRIBE      1
#define SDP_TYPE_FOLLOWUP       2
#define SDP_TYPE_MASK           0x03
#define SDP_HAS_DATA_MASK       0x10
#define SDP_SRV_ATTR_HDR_LEN    3
#define SDP_SRV_DESC_ATTR_ID    0x03 /* SDP 服务属性ID */
#define SDP_SRV_CTRL_OFFSET     11

#define NAN_DW_ADJ_DLY_CNT      2
#define NAN_DW_ADJ_START        20
#define NAN_DW_ADJ_MAX          60
#define NAN_DW_ADJ_STEP         20

#define SDP_PRI_INFO_TYPE_DW    1
#define SDP_PRI_INFO_TYPE_SLEEP 2
#define SDP_PRI_INFO_TYPE_LEN   4
#define SDP_PRI_INFO_LEN        19

#define SDP_LOW_POWER_RETRIES   200

#define HMAC_SDP_EN_SC 2  /* enable sdp and switch channel test */
#define HMAC_SDP_ENABLE 1  /* enable sdp */

#define SERVICE_DESCRIPTOR_ATTR_ID 0x03
#define COUNTRY_CODE_ATTR_ID 0x0B
#define SERVICE_DESCRIPTOR_EXT_ATTR_ID 0x0E
#define DEVICE_CAP_ATTR_ID 0x0F
#define NAN_AVAILABILITY_ATTR_ID 0x12
#define RANGING_INFO_ATTR_ID 0x1A
#define VENDOR_SPECIFIC_ATTR_ID 0xDD
#define HMAC_MAX_SUBSCRIBE_NUM 1
#define HMAC_MAX_PUBLISH_NUM 0
#define HMAC_MAX_BUFFER_NUM 8
#define HMAC_SDP_TYPE_PUBLISH 0
#define HMAC_SDP_TYPE_SUBSCRIBE 1
#define HMAC_SDP_TYPE_FOLLOWUP 2
#define HMAC_SDP_TYPE_SWITCH_LENGTH 4
#define HMAC_SDP_TYPE_MASK 0x03
#define HMAC_SDP_HAS_DATA_MASK 0x10
#define MAC_ATTR_HDR_LEN 3
#define HMAC_SDP_NO_DATA_LEN 10
#define HMAC_SDP_TX_UESR_ID 0xF
#define HMAC_PUBLISH_TIMEOUT 400 /* publish报文发送间隔时间*ms */

#define SDP_SERVICE_NAME_LEN   6

#define SDP_HASH_SERVICE_NAME_LEN 6
#define WAL_SDP_EN_SC 2  /* enable sdp and switch channel test */
#define WAL_SDP_ENABLE 1  /* enable sdp */
#define WAL_SDP_DISABLE 0 /* disable sdp */

#define WAL_SDP_BEACON_SWITCH_ENABLE 1  /* enable beacon switch */
#define WAL_SDP_BEACON_SWITCH_DISABLE 0 /* disable beacon switch */

enum OAL_SDP_SERVICE_TYPE {
    OAL_SDP_SERVICE_TYPE_DEFAULT = 0,    /* 默认值 */
    OAL_SDP_SERVICE_TYPE_PUBLISH = 1,    /* 对外发布服务 */
    OAL_SDP_SERVICE_TYPE_SUBSCRIBE = 2,  /* 订阅外部服务 */
    OAL_SDP_SERVICE_TYPE_GET_PUBLISH = 4 /* 收到外部发布服务消息 */
};

typedef struct {
    osal_u8 name[SDP_HASH_SERVICE_NAME_LEN];
    osal_u16 rsv;
    osal_u8 *data;
    osal_u8 len;
    osal_u8 local_handle;
    osal_u8 peer_handle;
    osal_u8 rsv_id;
} oal_sdp_info;

typedef struct {
    osal_u8 name[SDP_HASH_SERVICE_NAME_LEN];
    osal_u8 local_handle;
    osal_u8 type;
} oal_sdp_service;

typedef struct {
    osal_u16 dw_time;
    osal_u16 prd_time;
    osal_u16 prd_pre_time;
    osal_u16 rsvp[2]; /* 2 预留 */
} oal_sdp_dw_prd_time_info;

typedef struct {
    osal_u8 attr_len;
    osal_u8 total_type;
    osal_u8 total_len;
    osal_u8 type;
    osal_u32 total_period;
    osal_u8 wake_type;
    osal_u8 wake_len;
    osal_u16 wake_rsv;
    osal_u32 wake_period;
    osal_u8 is_connected;
    osal_u8 ratio;
    osal_u8 resv[2];
} oal_sdp_wakeup_info;

typedef struct {
    osal_u8 attr_len;
    osal_u8 total_type;
    osal_u8 total_len;
    osal_u8 type;
    osal_u8 name[SDP_HASH_SERVICE_NAME_LEN];
    osal_u8 local_handle;
    osal_u8 resv[9];
} oal_sdp_subscribe_info; /* 为了跟 oal_sdp_wakeup_info 结构大小一致 */

enum hmac_sdp_pos {
    HMAC_SDP_POS_PEER_HANDLE = 6,
    HMAC_SDP_POS_LOCAL_HANDLE,
    HMAC_SDP_POS_TYPE,
    HMAC_SDP_POS_DATA_LEN
};

typedef struct {
    osal_u8 category;
    osal_u8 action_code;
    osal_u8 oui[MAC_OUI_LEN];
    osal_u8 type;
} sdp_action_header;

typedef struct {
    osal_u8 attr_id;
    osal_u8 attr_len_low;
    osal_u8 attr_len_high;
    osal_u8 service_name[SDP_HASH_SERVICE_NAME_LEN];
    osal_u8 local_handle;
    osal_u8 peer_handle;
    osal_u8 svc_ctrl;
    osal_u8 info_len;
} hmac_sdp_descriptor_attr;

typedef struct {
    oal_sdp_service info[HMAC_MAX_PUBLISH_NUM + HMAC_MAX_SUBSCRIBE_NUM];
    hmac_vap_stru *mac_vap;
} hmac_sdp_service_info;

typedef struct {
    osal_u8 srv_name[SDP_SERVICE_NAME_LEN];
    osal_u8 handle;
    osal_u8 rsv;
} sdp_srv_info;

typedef struct {
    oal_netbuf_stru *netbuf;
} hmac_sdp_tx_stru;

osal_s32 hmac_sdp_send_data(hmac_vap_stru *hmac_vap, frw_msg *msg);
osal_s32 hmac_sdp_start_subscribe(hmac_vap_stru *hmac_vap, frw_msg *msg);
osal_s32 hmac_sdp_cancel_subscribe(hmac_vap_stru *hmac_vap, frw_msg *msg);
osal_s32 hmac_sdp_init_enable(hmac_vap_stru *hmac_vap, frw_msg *msg);
osal_u32 hmac_sdp_init(hmac_vap_stru *hmac_vap, osal_bool switch_channel, osal_u16 peroid, osal_u8 *input_ratio);
osal_u32 hmac_sdp_deinit(hmac_vap_stru *hmac_vap);
oal_bool_enum hmac_sdp_is_init(osal_void);
osal_u8 hmac_is_sdp_frame(oal_netbuf_stru *netbuf);
osal_u32 hmac_sdp_proc_rx_mgmt(hmac_vap_stru *hmac_vap, oal_netbuf_stru *netbuf, oal_bool_enum_uint8 *reported_host);
const hmac_sdp_service_info *hmac_sdp_get_service_info(osal_void);

osal_u8 *hmac_sdp_parse_rx(oal_netbuf_stru *netbuf, oal_sdp_info *sdp_param);
osal_s32 hmac_sdp_test(hmac_vap_stru *hmac_vap, frw_msg *msg);
osal_s32 hmac_sdp_add_peer_mac(hmac_vap_stru *hmac_vap, frw_msg *msg);
osal_s32 hmac_sdp_dw_prd_time_cfg(hmac_vap_stru *hmac_vap, frw_msg *msg);
osal_u32 hmac_sdp_send_followup_to_host(const hmac_vap_stru *hmac_vap, oal_sdp_info *sdp_info, osal_u8 *data,
    osal_u8 len);

typedef oal_bool_enum (*hmac_sdp_is_init_cb)(osal_void);
typedef osal_u8 (*hmac_is_sdp_frame_cb)(oal_netbuf_stru *netbuf);
typedef osal_u32 (*hmac_sdp_proc_rx_mgmt_cb)(hmac_vap_stru *hmac_vap, oal_netbuf_stru *netbuf,
    oal_bool_enum_uint8 *reported_host);

osal_void hmac_sdp_feature_hook_register(osal_void);
osal_void hmac_sdp_feature_hook_unregister(osal_void);

static osal_u32 hmac_sdp_init_weakref(osal_void) __attribute__ ((weakref("hmac_sdp_msg_init"), used));
static osal_void hmac_sdp_deinit_weakref(osal_void) __attribute__ ((weakref("hmac_sdp_msg_deinit"), used));

#ifdef __cplusplus
#if __cplusplus
    }
#endif
#endif

#endif /* end of hmac_sdp.h */

