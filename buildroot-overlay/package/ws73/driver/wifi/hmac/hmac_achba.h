/*
 * Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
 * Description: Header file for hmac_achba.c.
 * Create: 2023-04-17
 */

#ifndef HMAC_ACHBA_H
#define HMAC_ACHBA_H

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "dmac_ext_if_hcm.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_ACHBA_H

/*****************************************************************************
  2 宏定义
*****************************************************************************/
#define MAC_ORGANIZATION_IDENTIFIER_LEN 3 /* organization identifiter 长度 */
#define MAC_SUB_TYPE_LEN 1 /* sub_type 长度 */
#define MAC_CHBA_CAP_INFO_LEN 16 /* chba capability info 长度 */
#define MAC_EXT_ACHBA_SUB_TYPE_IE 0x30 /* chba_capablityinfo SUB_TYPE字段 */
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

typedef osal_s32 (*oal_achba_get_chba_cap)(oal_net_device_stru *netdev, osal_u8 *buff, osal_u32 buff_len);
typedef osal_s32 (*oal_achba_wifi_link_add)(oal_net_device_stru *netdev, const osal_u8 *user_mac, osal_u32 mac_len,
    const osal_s8 *achba_cap, osal_u32 cap_len);
typedef osal_s32 (*oal_achba_wifi_link_del)(oal_net_device_stru *netdev, osal_s32 achba_link_id);
typedef osal_s32 (*oal_achba_wifi_link_keepalive)(oal_net_device_stru *netdev, osal_s32 achba_link_id,
    osal_u32 keep_ind);
typedef osal_u8 (*hmac_set_achba_cap_ie_cb)(hmac_vap_stru *hmac_vap, osal_u8 *buffer);
typedef osal_void (*hmac_process_update_achba_cb)(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user, osal_u8 *payload,
    osal_u16 msg_len);
typedef osal_void (*hmac_user_del_achba_cb)(hmac_vap_stru *hmac_vap);

struct oal_achba_callee_intfs {
    oal_achba_get_chba_cap get_chba_cap;
    oal_achba_wifi_link_add wifi_link_add;
    oal_achba_wifi_link_del wifi_link_del;
};
struct oal_achba_caller_intfs {
    oal_achba_wifi_link_keepalive achba_keepalive;
};
typedef struct {
    osal_s32 achba_link_id;
    osal_u32 keep_ind;
} mac_set_achba_link_ind;

/*****************************************************************************
  8 UNION定义
*****************************************************************************/


/*****************************************************************************
  9 OTHERS定义
*****************************************************************************/


/*****************************************************************************
  10 函数声明
*****************************************************************************/
extern osal_s32 hmac_achba_get_achba_link_id(void);
extern osal_u32 hmac_achba_get_keepalive_ind(void);
extern osal_s32 oal_achba_register_wifi_intfs(struct oal_achba_callee_intfs *callee_intfs,
    struct oal_achba_caller_intfs *caller_intfs);
extern osal_s32 hmac_config_ind_achba_keepalive(hmac_vap_stru *mac_vap, frw_msg *msg);
static osal_u32 hmac_achba_init_weakref(osal_void) __attribute__ ((weakref("hmac_achba_init"), used));
static osal_void hmac_achba_deinit_weakref(osal_void) __attribute__ ((weakref("hmac_achba_deinit"), used));

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of hmac_achba */