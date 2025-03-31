/*
 * Copyright (c) CompanyNameMagicTag 2020-2023. All rights reserved.
 * Description: hmac_wapi.c的头文件.
 */

#ifndef __HMAC_WAPI_H__
#define __HMAC_WAPI_H__

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "oal_netbuf_ext.h"
#include "mac_resource_ext.h"
#include "hmac_vap.h"
#include "hmac_user.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_WAPI_H
/*****************************************************************************/
/*****************************************************************************
  2 宏定义
*****************************************************************************/
#define WAPI_UCAST_INC                  2       /* 发送或者接收单播帧,pn的步进值 */
#define WAPI_BCAST_INC                  1       /* 发送或者接收组播帧,pn的步进值 */
#define WAPI_WAI_TYPE                  (osal_u16)0x88B4 /* wapi的以太类型 */

#define WAPI_BCAST_KEY_TYPE             1
#define WAPI_UCAST_KEY_TYPE             0

#define SMS4_MIC_LEN                    (osal_u8)16     /* SMS4封包MIC的长度 */

#define SMS4_PN_LEN                     16     /* wapi pn的长度 */
#define SMS4_KEY_IDX                    1      /* WAPI头中 keyidx占1个字节 */
#define SMS4_WAPI_HDR_RESERVE           1      /* WAPI头中保留字段 */
#define HMAC_WAPI_HDR_LEN               (osal_u8)(SMS4_PN_LEN + SMS4_KEY_IDX + SMS4_WAPI_HDR_RESERVE)
#define WAPI_PDU_LEN                    2      /* wapi头中，wapi pdu len字段所占字节数 */
#define SMS4_PADDING_LEN                16     /* mic data按照16字节对齐 */

#define SMS4_MIC_PART1_QOS_LEN          48 /* 按照协议，如果有qos字段，mic第一部分16字节对齐后的长度 */
#define SMS4_MIC_PART1_NO_QOS_LEN       32 /* 按照协议，如果没有qos字段，mic第一部分16字节对齐后的长度 */

#define WAPI_IE_VERSION                     1   /* wapi的version */
#define WAPI_IE_VER_SIZE                    2   /* wapi ver-ie 所占字节数 */
#define WAPI_IE_SUIT_TYPE_SIZE              1   /* suit type size */
#define WAPI_IE_WAPICAP_SIZE                2   /* wapi cap字段所占字节数 */
#define WAPI_IE_BKIDCNT_SIZE                2   /* wapi bkid数字段所占字节数 */
#define WAPI_IE_BKID_SIZE                   16  /* 一个bkid所占字节数 */
#define WAPI_IE_OUI_SIZE                    3   /* wapi oui字节数 */
#define WAPI_IE_SMS4                        1   /* wapi加密类型为sms4 */
#define WAPI_IE_SUITCNT_SIZE                2   /* wapi suit count所占字节数 */
/* wapi key len */
#define WAPI_KEY_LEN                    16
#define WAPI_PN_LEN                     16
#define HMAC_WAPI_MAX_KEYID             2

/*****************************************************************************
  3 枚举定义
*****************************************************************************/


/*****************************************************************************
  4 全局变量声明
*****************************************************************************/


/*****************************************************************************
  5 消息头定义
*****************************************************************************/


/*****************************************************************************
  6 消息定义
*****************************************************************************/


/*****************************************************************************
  7 STRUCT定义
*****************************************************************************/
typedef struct {
    osal_u8   framectl[2];        /* 帧控制 */
    osal_u8   adress1[OAL_MAC_ADDR_LEN];         /* 地址1 */
    osal_u8   adress2[OAL_MAC_ADDR_LEN];         /* 地址2 */
    osal_u8   seqctl[2];          /* 序列控制 */
    osal_u8   adress3[OAL_MAC_ADDR_LEN];         /* 地址3 */
    osal_u8   adress4[OAL_MAC_ADDR_LEN];         /* 地址4 */
} wapi_mic_hdr_stru;

typedef struct {
    osal_u8   wpi_ek[WAPI_KEY_LEN];
    osal_u8   wpi_ck[WAPI_KEY_LEN];
    osal_u8   pn_rx[WAPI_PN_LEN];
    osal_u8   pn_tx[WAPI_PN_LEN];
    osal_u8   key_en;
    osal_u8   rsv[3];
} hmac_wapi_key_stru;

typedef struct tag_hmac_wapi_stru {
    osal_u8              port_valid;                       /* wapi控制端口 */
    osal_u8              keyidx;
    osal_u8              keyupdate_flg;                    /* key更新标志 */
    osal_u8              pn_inc;                           /* pn步进值 */

    hmac_wapi_key_stru     wapi_key[HMAC_WAPI_MAX_KEYID];   /* keyed: 0~1 */

    osal_u8(*wapi_filter_frame)(struct tag_hmac_wapi_stru *wapi, oal_netbuf_stru *netbuff);
    oal_bool_enum_uint8(*wapi_is_pn_odd)(const osal_u8 *pn);    /* 判断pn是否为奇数 */
    osal_u32(*wapi_decrypt)(struct tag_hmac_wapi_stru *wapi, oal_netbuf_stru *netbuff);
    osal_u32(*wapi_encrypt)(struct tag_hmac_wapi_stru *wapi, oal_netbuf_stru *pst_netbuf);
    oal_netbuf_stru *(*wapi_netbuff_txhandle)(struct tag_hmac_wapi_stru *wapi, oal_netbuf_stru *pst_netbuf);
    oal_netbuf_stru *(*wapi_netbuff_rxhandle)(struct tag_hmac_wapi_stru *wapi, oal_netbuf_stru *pst_netbuf);
} hmac_wapi_stru;

/*****************************************************************************
  8 UNION定义
*****************************************************************************/

/*****************************************************************************
  9 OTHERS定义
*****************************************************************************/

/*****************************************************************************
  10 函数声明
*****************************************************************************/
typedef osal_void (*hmac_wapi_deinit_etc_cb)(osal_u16 user_idx);
typedef osal_u32 (*hmac_wapi_init_etc_cb)(osal_u16 user_idx, osal_u8 pairwise);
typedef oal_netbuf_stru* (*hmac_rx_frame_wapi_cb)(hmac_vap_stru *hmac_vap, oal_netbuf_stru *netbuf,
    mac_ieee80211_frame_stru *frame_hdr, hmac_user_stru *hmac_user, mac_rx_ctl_stru **rx_ctrl);
typedef osal_u32 (*hmac_wapi_tx_classify_special_cb)(hmac_vap_stru *hmac_vap, osal_u16 type,
    mac_tx_ctl_stru *tx_ctl, osal_u8 *tid);
typedef osal_u32 (*hmac_tx_lan_to_wlan_wapi_cb)(hmac_vap_stru *hmac_vap, oal_netbuf_stru **netbuf);
typedef osal_u32 (*hmac_11i_reset_wapi_cb)(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user);
typedef osal_u32 (*hmac_wapi_add_key_and_sync_etc_cb)(hmac_vap_stru *hmac_vap, osal_u32 type,
    mac_addkey_param_stru *payload_addkey_param);
typedef osal_void (*hmac_wapi_update_legcy_only_cb)(mac_bss_dscr_stru *bss_dscr, oal_bool_enum_uint8 *legcy_only);
typedef osal_bool (*hmac_wapi_ether_type_check_cb)(osal_u16 ether_type);
typedef osal_u32 (*hmac_wapi_connect_check_bss_dscr_cb)(mac_bss_dscr_stru *bss_dscr,
    hmac_vap_stru *hmac_vap, mac_conn_param_stru *connect_param);
typedef osal_bool (*hmac_wapi_disable_cb)(osal_u8 wapi);
typedef osal_s32 (*hmac_wapi_update_roam_cb)(hmac_vap_stru *hmac_vap, osal_u8 *roming_now);
typedef osal_u32 (*hmac_wapi_user_add_check_cb)(osal_u8 device_id);
typedef osal_bool (*hmac_wapi_get_ether_type_cb)(osal_u16 ether_type);
typedef osal_u8 (*hmac_wapi_connected_etc_cb)(osal_u8 device_id);
typedef osal_u8 (*hmac_wapi_update_connect_param_cb)(osal_u32 wpa_versions);

static osal_u32 hmac_wapi_init_weakref(osal_void) __attribute__ ((weakref("hmac_wapi_init"), used));
static osal_void hmac_wapi_deinit_weakref(osal_void) __attribute__ ((weakref("hmac_wapi_deinit"), used));

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of hmac_wapi.h */