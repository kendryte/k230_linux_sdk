/*
 * Copyright (c) CompanyNameMagicTag 2020-2020. All rights reserved.
 * Description: hmac_slp.h
 * Date: 2020-07-07
 */

#ifndef __HMAC_SLP_H__
#define __HMAC_SLP_H__

#include "mac_vap_ext.h"
#include "mac_frame.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/* 1 其他头文件包含 */
#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_SLP_H
#define MAC_SLP_SESSION_TIME 50 /* 每个SLP会话总时长不超过50ms */
#define MAC_SLP_CTS_RTS_TIME 8  /* SLP抢占空口时间8ms */

typedef enum {
    UNICAST = 0, // 单播，1对1
    BROADCAST,   // 广播，1对多、多对多
} slp_communication_mode;

typedef struct {
    slp_communication_mode communication_mode; // 数据通信方式，单播/广播
    osal_u8 addr[WLAN_MAC_ADDR_LEN];               // 单播时，对端设备的地址; 广播时，广播地址(全f)
} slp_device_para;                            // 测距设备参数

typedef struct {
    osal_u8 *payload;
    osal_u16 payload_len;
} slp_nb_payload_req;

typedef struct {
    osal_s32 cfo;
} slp_nb_cfo_req; /* 从窄带系统处获取的CFO信息 */

// RM->WI-FI
typedef struct {
    slp_nb_payload_req payload_req;
    slp_device_para device_para;
} slp_nb_payload_and_device_req; /* 与窄带系统交互的Payload内容, 包含PCN或PRN信息 */

// WI-FI->RM
typedef struct {
    slp_nb_payload_req payload_req;
    slp_nb_cfo_req cfo_req;
} slp_nb_payload_and_cfo_req; /* 与窄带系统交互的Payload内容, 包含PCN或PRN信息 */

#define WLAN_MAX_CTS_LEN 10  /* RTS帧 mac头长度和总长度一致 */
#define WLAN_MAX_RTS_LEN 16  /* CTS帧 mac头长度和总长度一致 */
#define WLAN_SLP_GTK_IDX_INVAILD 0xFF  /* CTS帧 mac头长度和总长度一致 */
/* Type of Tx Descriptor status */
typedef enum {
    MAC_SLP_REPORT_ACTION_FRAME = 0,
    MAC_SLP_RM_ACTION_FRAME = 1,
    MAC_SLP_CTS_FRAME = 2,
    MAC_SLP_GPIO_FRAME = 3,
    MAC_SLP_FRAME_BUTT
} mac_slp_frame_id_enum;
typedef osal_u8 mac_slp_frame_id_enum_uint8;

typedef enum {
    SLP_RM_REQUEST = 0,
    SLP_RM_RESULT_REPORT = 1,
    SLP_RM_FRAME_SUBTYPE_BUTT
} mac_slp_rm_frame_subtype_enum;
typedef osal_u8 mac_slp_rm_frame_subtype_enum_uint8;

typedef struct {
    osal_u8        category;
    osal_u8        oui[3]; /* 3字节存放OUI信息 */
    osal_u8        oui_type;
    osal_u8        oui_sub_type;
    osal_u16       len;
} mac_slp_action_hdr_stru;

typedef osal_bool (*hmac_slp_tx_disable_cb)(osal_void);

static osal_u32 hmac_slp_init_weakref(osal_void) __attribute__ ((weakref("hmac_slp_init"), used));
static osal_void hmac_slp_deinit_weakref(osal_void) __attribute__ ((weakref("hmac_slp_deinit"), used));
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of hmac_slp.h */
