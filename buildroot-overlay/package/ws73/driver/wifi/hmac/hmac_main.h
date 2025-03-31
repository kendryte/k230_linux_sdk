/*
 * Copyright (c) CompanyNameMagicTag 2021-2021. All rights reserved.
 * Description: Header file for hmac_main.c.
 * Create: 2021-12-15
 */

#ifndef __HMAC_MAIN_H__
#define __HMAC_MAIN_H__


/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "dmac_ext_if_hcm.h"
#include "frw_hmac.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_MAIN_H

/*****************************************************************************
  2 宏定义
*****************************************************************************/

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
    osal_u32  time_stamp;
} hmac_timeout_event_stru;

typedef struct {
    osal_u32  cfg_id;
    osal_u32  ac;
    osal_u32  value;
} hmac_config_wmm_para_stru;

#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) \
    && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
typedef struct {
    struct semaphore            rxdata_sema;
    osal_task                   *rxdata_thread;
    osal_spinlock               lock;
    osal_wait rxdata_wq;
    oal_netbuf_head_stru        rxdata_netbuf_head;
    osal_u32                    pkt_loss_cnt;
    oal_bool_enum_uint8         rxthread_enable;
} hmac_rxdata_thread_stru;
#endif

typedef osal_void (*hmac_set_dscr_func)(osal_s32, osal_u8, hmac_vap_stru*);

/*****************************************************************************
  8 UNION定义
*****************************************************************************/


/*****************************************************************************
  9 OTHERS定义
*****************************************************************************/


/*****************************************************************************
  10 函数声明
*****************************************************************************/
extern osal_u32  hmac_event_fsm_register_etc(osal_void);
extern osal_void  hmac_main_exit_etc(osal_void);
extern osal_s32  hmac_main_init_etc(osal_void);
#ifdef _PRE_WLAN_ONLINE_DPD
osal_s32 hmac_sdt_up_dpd_data(oal_netbuf_stru *netbuf);
#endif

extern osal_void  hmac_rxdata_netbuf_enqueue_etc(oal_netbuf_stru  *pst_netbuf);
extern osal_void  hmac_rxdata_sched_etc(osal_void);
extern oal_bool_enum_uint8 hmac_get_rxthread_enable_etc(osal_void);

osal_s32 hmac_syn_info_event(hmac_vap_stru *hmac_vap, frw_msg *msg);
osal_u32  hmac_proc_query_response_event(hmac_vap_stru *hmac_vap, osal_u8 len, osal_u8 *param);
osal_s32 hmac_del_ba_event(hmac_vap_stru *vap, frw_msg *msg);

osal_void hmac_event_fsm_register(osal_void);
osal_void hmac_event_fsm_unregister(osal_void);

osal_void hmac_wake_lock(osal_void);
osal_void hmac_wake_unlock(osal_void);
osal_s32 hmac_main_init_later(osal_void);
osal_void alg_hmac_sync_param(osal_void);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of hmac_main */
