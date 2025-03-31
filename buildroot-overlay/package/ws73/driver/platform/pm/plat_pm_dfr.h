/*
 * Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
 * Description: host platform pm dfr header file
 * Create: 2023-02
 */

#ifndef __PLAT_PM_DFR_H__
#define __PLAT_PM_DFR_H__

#include "soc_osal.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#if defined(CONFIG_PLAT_SUPPORT_DFR)

#define PLAT_DFR_CMD_WAIT_MS 100
#define PLAT_DEVICE_RESET_REG 0x4001F118

#define DFR_BUS_STATE_CLOSE 0
#define DFR_BUS_STATE_OPEN 1
#define DFR_BUS_STATE_CLOSE_REG 2

// DFR STATUS
#define DFR_ATOMIC_STATUS_IDLE          0
#define DFR_ATOMIC_STATUS_DFR_PREPARE   1
#define DFR_ATOMIC_STATUS_DFR_PROCESS   2
#define DFR_ATOMIC_STATUS_STR_SUSPEND   3
#define DFR_ATOMIC_STATUS_STR_RESUME    4

typedef void (*recovery_complete)(void);

osal_s32 plat_exception_init(osal_void);
osal_void plat_exception_exit(osal_void);
td_u8 plat_is_device_in_recovery(td_void);
td_void plat_update_device_recovery_flag(td_u8 flag);
osal_void wlan_set_dfr_recovery_flag(osal_u8 dfr_flag);
osal_u32 plat_exception_reset_process(osal_bool should_dump_trace);
osal_u32 plat_exception_reset_work(osal_u8 *data);
extern osal_s32 plat_wifi_exception_rst_register_etc(osal_void *data);

osal_void plat_dfr_lock(osal_void);
osal_void plat_dfr_unlock(osal_void);
osal_void plat_dfr_enable_set(osal_bool enable);
osal_s32 plat_dfr_status_get(osal_void);
osal_void plat_dfr_status_set(osal_s32 dfr_status);
osal_bool plat_dfr_wait_flag_get(osal_void);
osal_void plat_dfr_wait_flag_set(osal_bool flag);
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* __PLAT_PM_DFR_H__ */

