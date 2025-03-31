/*
 * Copyright (c) CompanyNameMagicTag. 2022-2022. All rights reserved.
 * Description: pm crg rom header.
 * Create: 2022-10-19
 */

#ifndef __PM_INTERFACE_H__
#define __PM_INTERFACE_H__

#include "td_type.h"

typedef enum _pm_result {
    PM_SUCC = 0,
    PM_FAIL = 0xFFFFFFFF
} pm_result;

// 低功耗业务类型枚举
enum pm_fsm_service_id_enum {
    PM_FSM_SVC_BEGIN = 0,

    PM_FSM_SVC_WLAN = PM_FSM_SVC_BEGIN,
    PM_FSM_SVC_BT,
    PM_FSM_SVC_PLT,

    PM_FSM_SVC_END,
    PM_FSM_SVC_NUM = PM_FSM_SVC_END
};

// 低功耗状态机状态/业务投票状态枚举
enum pm_fsm_status_enum {
    PM_FSM_STS_SD = 0, // shutdown
    PM_FSM_STS_DS, // deepsleep
    PM_FSM_STS_LS, // lightsleep
    PM_FSM_STS_WK, // work

    PM_FSM_STS_NUM
};

// 休眠相关外设或模块类型枚举
enum pm_ctrl_module_id_enum {
    PM_CTRL_MODULE_ID_PM = 0, // PM模块
    PM_CTRL_MODULE_ID_SDIO, // SDIO模块
    PM_CTRL_MODULE_ID_USB, // USB模块
    PM_CTRL_MODULE_ID_HCC, // HCC模块
    PM_CTRL_MODULE_ID_FRW, // FRW模块
    PM_CTRL_MODULE_ID_DIAG, // DIAG模块

    PM_CTRL_MODULE_ID_NUM
};

typedef td_u32 (*PM_FSM_FUNC_PTR)(td_void);

// fsm接口
td_u32 pm_fsm_svc_vote_rom(td_u8 sid, td_u8 state);
td_u32 pm_fsm_callback_register_rom(PM_FSM_FUNC_PTR callback, td_u8 src_status, td_u8 dst_status);
td_u32 pm_fsm_callback_unregister_rom(td_u8 src_status, td_u8 dst_status);

// pm接口
td_void pm_ctrl_init_rom(td_void);
td_void pm_ctrl_set_host_sleep(td_void);
td_void pm_ctrl_set_host_awake(td_void);
td_bool pm_ctrl_is_host_sleep_rom(td_void);
td_bool pm_ctrl_check_host_sleep_state_rom(td_u8 idx);
td_u32 pm_ctrl_awake_host_rom(td_u8 idx);
td_u32 pm_init_rom(td_void);
td_void pm_ctrl_awake_host(td_u8 svc_id, td_u8 is_reboot);

#endif /* __PM_INTERFACE_ROM_H__ */