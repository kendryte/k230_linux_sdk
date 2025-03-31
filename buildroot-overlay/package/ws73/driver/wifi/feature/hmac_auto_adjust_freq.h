/*
 * Copyright (c) CompanyNameMagicTag 2020-2020. All rights reserved.
 * Description: dmac auto adjust frequency head
 * Create: 2020-7-5
 */

#ifndef __HMAC_AUTO_ADJUST_FREQ_H__
#define __HMAC_AUTO_ADJUST_FREQ_H__

#include "hmac_device.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/* pps统计结构体 */
typedef struct {
    osal_u32 last_timeout;
    osal_u32 pps_rate;
    osal_u32 hcc_rxtx_total;   /* hcc slave 收发数据帧统计 */

    frw_timeout_stru timer;
} hmac_pps_statistics_stru;

/*****************************************************************************
  2 宏定义
*****************************************************************************/

/*****************************************************************************
  3 枚举定义
*****************************************************************************/
typedef enum {
    WLAN_DEVICE_AUTO_FREQ_OFF = 0,
    WLAN_DEVICE_AUTO_FREQ_ON,
    WLAN_DEVICE_AUTO_FREQ_BUTT
} wlan_auto_freq_switch;

enum {
    FREQ_IDLE = 0,
    FREQ_MIDIUM = 1,
    FREQ_HIGHEST = 2,
    FREQ_BUTT = 3
};
/*****************************************************************************
  4 全局变量声明
*****************************************************************************/

/*****************************************************************************
  7 STRUCT定义
*****************************************************************************/
typedef osal_u8 oal_device_freq_type_enum_uint8;
typedef struct {
    oal_bool_enum_uint8 auto_freq_enable;            /* 使能开关 */
    oal_bool_enum_uint8 pm_enable;                   /* 低功耗状态标志 */
    oal_device_freq_type_enum_uint8 curr_freq_level; /* 当前工作频率级别 */
    oal_device_freq_type_enum_uint8 req_freq_level;  /* 根据netbuf判断需要的频率级别 */
    osal_u32 pps_loop_count;
} hmac_freq_control_stru;

typedef struct {
    osal_u32 speed_level;    /* 吞吐量门限 */
    osal_u32 cpu_freq_level; /* CPU主频下限 */
} device_pps_freq_level_stru;

/*****************************************************************************
  10 函数声明
*****************************************************************************/
typedef osal_void (*hmac_auto_freq_pps_count_cb)(osal_u32 count);
typedef osal_void (*hmac_get_pps_handle_pps_rate_cb)(osal_u32 *pps_rate);
typedef osal_void (*hmac_auto_freq_btcoex_handle_cb)(osal_u8 bt_ldac);

static osal_u32 hmac_auto_freq_init_weakref(osal_void)
    __attribute__ ((weakref("hmac_auto_adjust_freq_init"), used));
static osal_void hmac_auto_freq_deinit_weakref(osal_void)
    __attribute__ ((weakref("hmac_auto_adjust_freq_deinit"), used));

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of hmac_auto_adjust_freq_rom.h */
