 /*
 * Copyright (c) @CompanyNameMagicTag 2020-2022. All rights reserved.
 * Description: schedule algorithm
 */

#ifdef _PRE_WLAN_FEATURE_SCHEDULE

/******************************************************************************
  1 头文件包含
 ******************************************************************************/
#include "alg_schedule.h"
#include "alg_schedule_if.h"
#include "hmac_alg_notify.h"
#include "hmac_device.h"
#include "alg_main.h"
#include "hmac_user.h"
#include "hal_ext_if.h"

#include "hal_device.h"

#include "hmac_tx_mpdu_queue.h"
#include "dmac_ext_if_type.h"

#include "hmac_tx_mgmt.h"
#include "alg_gla.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_ALG_SCHEDULE_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST
/******************************************************************************
  2 内部静态函数声明
 ******************************************************************************/
OSAL_STATIC osal_u32 alg_schedule_init_txtid_info(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    alg_schedule_stru *schedule);
OSAL_STATIC osal_u32 alg_schedule_init_user_info(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user);
OSAL_STATIC osal_u32 alg_schedule_free_user_info(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user);
osal_u32 alg_schedule_update_txtid_dlist(hmac_tid_stru *txtid, osal_u8 in_mpdu_num);
osal_u32 alg_schedule_update_target_user(const hal_to_dmac_device_stru *hal_device, hmac_user_stru *hmac_user,
    mac_tid_schedule_output_stru schedule_ouput, osal_u8 device_mpdu_full);
osal_u32 alg_schedule_tx_tid_sch(const hal_to_dmac_device_stru *hal_device, osal_u8 ac_num,
    mac_tid_schedule_output_stru *sch_output);
osal_u32 alg_schedule_sch_timer_handle(osal_void *void_code);
OSAL_STATIC osal_u32 alg_schedule_stat_timer_log(osal_void *void_code);
OSAL_STATIC osal_void alg_schedule_recycle_user_mem(hmac_user_stru *hmac_user);
osal_u32 alg_schedule_sch_timer_restart_handle(const hal_to_dmac_device_stru *hal_device);
osal_u32 alg_schedule_update_sch_result(hmac_device_stru *hmac_device, alg_schedule_stru *schedule,
    const alg_sch_output_stru *sch_result, mac_tid_schedule_output_stru *sch_output);
osal_u32 alg_schedule_rr_sch(alg_schedule_stru *schedule, osal_u8 vap_id, osal_s8 ac_num,
    alg_sch_output_stru *sch_result);


#ifdef _PRE_WLAN_FEATURE_SCH_STRATEGY_WMM_ENSURE
osal_u32 alg_schedule_sch_bcast_txtid(alg_schedule_stru *schedule, osal_u8 vap_id,
    wlan_tidno_enum_uint8 txtid_index, alg_sch_output_stru *sch_output);

osal_u32 alg_schedule_wmm_sch(alg_schedule_stru *schedule, osal_u8 vap_id, osal_u8 ac_num,
    alg_sch_output_stru *sch_result);
#endif

osal_u32 alg_schedule_round_robin_sch(alg_schedule_stru *schedule, osal_u8 vap_id, osal_u8 ac_num,
    alg_sch_output_stru *sch_result);

/******************************************************************************
  3 全局变量定义
 ******************************************************************************/
/******************************
    device级 schedule结构体
******************************/
alg_schedule_stru g_schedule;
struct osal_list_head g_user_dlist_head; /* 用户链表头 */
struct osal_list_head *g_target_user_head = &g_user_dlist_head;

#ifdef _PRE_WLAN_FEATURE_SCH_STRATEGY_WMM_ENSURE
/* 定义调度函数指针 */
typedef osal_u32 (*p_sch_txtid_func)(alg_schedule_stru *schedule, osal_u8 vap_id,
    wlan_tidno_enum_uint8 txtid_index, alg_sch_output_stru *sch_output);

/* 定义各个tid对应的优先级 */
WIFI_TCM_RODATA const osal_u8 g_tid_prio[WLAN_TID_MAX_NUM] = {
    WLAN_TIDNO_BCAST,
    WLAN_TIDNO_VOICE,
    WLAN_TIDNO_VIDEO,
    WLAN_TIDNO_ANT_TRAINING_HIGH_PRIO,
    WLAN_TIDNO_ANT_TRAINING_LOW_PRIO,
    WLAN_TIDNO_UAPSD,
    WLAN_TIDNO_BEST_EFFORT,
    WLAN_TIDNO_BACKGROUND
};
#endif

/* 调度算法选择定制化选项 */
#ifdef _PRE_WLAN_FEATURE_SCH_STRATEGY_WMM_ENSURE
osal_u8 g_sch_switch = ALG_SCH_STRATEGY_WMM_ENSURE;
#else
osal_u8 g_sch_switch = ALG_SCH_STRATEGY_ROUND_ROBIN;
#endif

osal_u8 g_stat_log = 0;
/******************************************************************************
  3 函数实现
*****************************************************************************/
/*****************************************************************************
 功能描述  : 获取调度模块信息
*****************************************************************************/
WIFI_TCM_TEXT alg_schedule_stru *alg_schedule_get_dev_info(osal_void)
{
    return &g_schedule;
}

/******************************************************************************
 函 数 名  : alg_schedule_config_param
 功能描述  : 配置算法参数,只能传递一个参数
******************************************************************************/
WIFI_TCM_TEXT oal_bool_enum_uint8 alg_schedule_is_vap_paused(hmac_vap_stru *hmac_vap)
{
    if (hmac_vap->vap_state != MAC_VAP_STATE_UP) {
        return OAL_TRUE;
    }

    if (hmac_vap_is_pause_tx_bitmap(hmac_vap)) {
        return OAL_TRUE;
    }
    return OAL_FALSE;
}

#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
/******************************************************************************
 函 数 名  : alg_schedule_config_param
 功能描述  : 配置算法参数,只能传递一个参数
******************************************************************************/
osal_u32 alg_schedule_config_param(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    alg_schedule_stru *schedule = alg_schedule_get_dev_info();
    mac_ioctl_alg_param_stru *alg_param = (mac_ioctl_alg_param_stru *)msg->data;

    if ((alg_param->alg_cfg <= MAC_ALG_CFG_SCHEDULE_START) || (alg_param->alg_cfg >= MAC_ALG_CFG_SCHEDULE_END)) {
        return OAL_SUCC;
    }

    if (hmac_vap->hal_device == OSAL_NULL) {
        oam_warning_log1(0, OAM_SF_SCHEDULE, "{alg_schedule_config_param::vap[%d], hal_device null}", hmac_vap->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    oam_warning_log2(0, OAM_SF_ANY, "{alg_schedule_config_param: alg_sch_config ID[%d] value[%d]!}",
        alg_param->alg_cfg, alg_param->value);

    switch (alg_param->alg_cfg) {
        case MAC_ALG_CFG_SCHEDULE_STAT_LOG:
            g_stat_log = (osal_u8)alg_param->value;
            break;
#ifdef _PRE_WLAN_FEATURE_SCH_STRATEGY_WMM_ENSURE
        case MAC_ALG_CFG_SCHEDULE_SCH_CYCLE_MS:
            schedule->sch_timer.timeout = (osal_u16)alg_param->value;
            frw_create_timer_entry(&(schedule->sch_timer), alg_schedule_sch_timer_handle,
                schedule->sch_timer.timeout, (osal_void *)schedule, OSAL_FALSE);
            break;
#endif
        case MAC_ALG_CFG_SCHEDULE_SCH_METHOD:
            g_sch_switch = (osal_u8)alg_param->value;
            break;
        case MAC_ALG_CFG_SCHEDULE_FIX_SCH_MODE:
            schedule->fix_sch_mode = ((osal_u8)alg_param->value > 1) ? 0 : ((osal_u8)alg_param->value);
            break;
        case MAC_ALG_CFG_SCHEDULE_TX_TIME_METHOD:
        case MAC_ALG_CFG_SCHEDULE_TX_TIME_DEBUG:
        case MAC_ALG_CFG_SCHEDULE_DMAC_STAT_LOG:
            if (frw_send_msg_to_device(hmac_vap->vap_id, WLAN_MSG_H2D_C_CFG_ALG_PARAM, msg, OSAL_TRUE) != OAL_SUCC) {
            oam_warning_log0(0, OAM_SF_ANY, "{alg_schedule_config_param::frw_send_msg_to_device failed!}");
            return OAL_FAIL;
            }
            break;
        default:
            oam_warning_log1(0, OAM_SF_SCHEDULE, "{alg_schedule_config_param fail! param_type=%d", alg_param->alg_cfg);
            return OAL_FAIL;
    }

    return OAL_SUCC;
}
#endif
OSAL_STATIC osal_u32 alg_schedule_change_target_time(const alg_schedule_stru *schedule)
{
    hmac_user_stru *hmac_user = NULL;
    alg_schedule_user_info_stru *user_info = NULL;
    hmac_vap_stru *hmac_vap;
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);
    osal_u8 vap_index;
    osal_u32 ret;

    struct osal_list_head *list_pos = OSAL_NULL;
    alg_device_foreach_vap(hmac_vap, hmac_device, vap_index) {
        alg_vap_foreach_user(hmac_user, hmac_vap, list_pos) {
            ret = hmac_alg_get_user_priv_stru(hmac_user, HMAC_ALG_USER_STRU_ID_SCHEDULE, (osal_void **)&user_info);
            if (osal_unlikely(ret != OAL_SUCC)) {
                oam_warning_log1(0, OAM_SF_SCHEDULE,
                    "{alg_schedule_change_target_time::get_user_priv_stru user[%d] failed}", hmac_user->assoc_id);
                continue;
            }
            if (schedule->user_num != 0) {
                user_info->target_time = ALG_SCH_WMM_AIR_USER_PERIOD / schedule->user_num; /* 单位10us */
            } else {
                user_info->target_time = ALG_SCH_WMM_AIR_USER_PERIOD; /* 单位10us */
            }
        }
    }
    return OAL_SUCC;
}
#ifdef _PRE_WLAN_FEATURE_SCH_STRATEGY_WMM_ENSURE
OSAL_STATIC osal_u32 alg_schedule_init_user_info_val(const hmac_vap_stru *hmac_vap, const hmac_user_stru *hmac_user,
    alg_schedule_stru *schedule)
{
    alg_schedule_user_info_stru *sch_user_info = OSAL_NULL;
    osal_u32 ret;
    unref_param(hmac_vap);

    /* user私有信息结构体并初始化 */
    sch_user_info = (alg_schedule_user_info_stru *)alg_mem_alloc(ALG_MEM_SCHEDULE,
        sizeof(alg_schedule_user_info_stru));
    if (sch_user_info == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_SCHEDULE, "{alg_schedule_init_user_info_val: mem alloc fail!}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    memset_s(sch_user_info, sizeof(alg_schedule_user_info_stru), 0, sizeof(alg_schedule_user_info_stru));

    /* 挂载用户级别的调度私有信息结构体 */
    ret = hmac_alg_register_user_priv_stru(hmac_user, HMAC_ALG_USER_STRU_ID_SCHEDULE, (osal_void *)sch_user_info);
    if (osal_unlikely(ret != OAL_SUCC)) {
        osal_kfree(sch_user_info);
        return ret;
    }

    if (hmac_user->is_multi_user != OSAL_TRUE) {
        schedule->user_num += 1;
        OSAL_INIT_LIST_HEAD(&(sch_user_info->user_dlist_entry));
        osal_list_add_tail(&(sch_user_info->user_dlist_entry), &g_user_dlist_head);
    }

    /* 初始化WMM空口公平性参数 */
    sch_user_info->target_quota = ALG_SCH_WMM_AIR_USER_TARGET_TUOTA_MAX;
    sch_user_info->remain_quota = sch_user_info->target_quota;
    sch_user_info->user_id = (osal_u8)hmac_user->assoc_id;

    alg_schedule_change_target_time(schedule);

    return ret;
}
#endif

/******************************************************************************
函 数 名  : alg_schedule_init_user_info
功能描述  : 1.初始化STA时，初始化STA 调度信息结构体；
            2.初始化STA所属的所有txtid的调度信息结构体；
 ******************************************************************************/
OSAL_STATIC osal_u32 alg_schedule_init_user_info(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user)
{
    hal_to_dmac_device_stru *hal_device = OSAL_NULL;
    alg_schedule_stru *schedule = alg_schedule_get_dev_info();
    osal_u32 ret;

    /* 获取device结构体 */
    hal_device = hmac_user_get_hal_device(hmac_user);
    if (hal_device == OSAL_NULL) {
        oam_error_log1(0, OAM_SF_SCHEDULE,
            "{alg_schedule_init_user_info::vap id[%d]get hal_device is NULL!}", hmac_user->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 申请user私有信息结构体，初始化并挂载 */
    if (hmac_user->assoc_id >= hmac_board_get_max_user()) {
        oam_error_log2(0, 0, "vap_id[%d] {alg_schedule_init_user_info::valid assoc id[%d]}", hmac_vap->vap_id,
            hmac_user->assoc_id);
        return OAL_ERR_CODE_ARRAY_OVERFLOW;
    }

#ifdef _PRE_WLAN_FEATURE_SCH_STRATEGY_WMM_ENSURE
    ret = alg_schedule_init_user_info_val(hmac_vap, hmac_user, schedule);
    if (osal_unlikely(ret != OAL_SUCC)) {
        return ret;
    }
#endif

    /* 申请txtid私有信息结构体，初始化并挂载 */
    ret = alg_schedule_init_txtid_info(hmac_vap, hmac_user, schedule);
    if (osal_unlikely(ret != OAL_SUCC)) {
        /* 回收user, txtid私有信息结构体内存 */
        alg_schedule_recycle_user_mem(hmac_user);
        return ret;
    }

    return OAL_SUCC;
}

/******************************************************************************
功能描述  :  释放user所属的所有txtid的调度信息结构体；
 ******************************************************************************/
OSAL_STATIC osal_void alg_schedule_free_txtid_info(hmac_user_stru *hmac_user)
{
    alg_schedule_txtid_info_stru *sch_txtid_info = OSAL_NULL;
    osal_u8 txtid_index;
    osal_u32 ret;

    /* 对该用户的每个txtid */
    for (txtid_index = WLAN_TIDNO_BEST_EFFORT; txtid_index < WLAN_TID_MAX_NUM; txtid_index++) {
        /* 获取tid级别的调度私有信息结构体 */
        ret = hmac_alg_get_tid_priv_stru(hmac_user, txtid_index, HMAC_ALG_TID_STRU_ID_SCHEDULE,
            (osal_void **)&sch_txtid_info);
        if ((ret != OAL_SUCC) || (sch_txtid_info == OSAL_NULL)) {
            oam_error_log0(0, 0, "{alg_schedule_free_txtid_info::hmac_alg_get_tid_priv_stru fail!}");
            continue;
        }

        /* 从链表中删除该txtid元素 */
        if (sch_txtid_info->is_scheduling == OSAL_TRUE) {
            osal_dlist_delete_entry(&(sch_txtid_info->txtid_dlist_entry));
        }

        /* 注销结构体 */
        ret = hmac_alg_unregister_tid_priv_stru(hmac_user, txtid_index, HMAC_ALG_TID_STRU_ID_SCHEDULE);
        if (osal_unlikely(ret != OAL_SUCC)) {
            oam_warning_log0(0, 0, "{alg_schedule_free_txtid_info::hmac_alg_unregister_tid_priv_stru fail!}");
            osal_kfree(sch_txtid_info);
            continue;
        }
        osal_kfree(sch_txtid_info);
    }
}

/******************************************************************************
功能描述  :  1.用户去关联时，释放user调度信息结构体；
             2.释放user所属的所有txtid的调度信息结构体；
 ******************************************************************************/
osal_u32 alg_schedule_free_user_info(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user)
{
    osal_u32 ret;
#ifdef _PRE_WLAN_FEATURE_SCH_STRATEGY_WMM_ENSURE
    alg_schedule_user_info_stru *sch_user_info = OSAL_NULL;
    hal_to_dmac_device_stru *hal_dev;
    alg_schedule_stru *schedule = alg_schedule_get_dev_info();
#endif
    unref_param(hmac_vap);

#ifdef _PRE_WLAN_FEATURE_SCH_STRATEGY_WMM_ENSURE
    hal_dev = hmac_vap->hal_device;
    if (hal_dev == OSAL_NULL) {
        oam_error_log1(0, OAM_SF_SCHEDULE,
            "{alg_schedule_free_user_info::vap id[%d]HMAC_VAP_GET_HAL_DEVICE is NULL!}", hmac_vap->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }
#endif
    /* 卸载并释放txtid私有信息结构体 */
    alg_schedule_free_txtid_info(hmac_user);
#ifdef _PRE_WLAN_FEATURE_SCH_STRATEGY_WMM_ENSURE
    /* 获取并释放用户级别的调度私有信息结构体 */
    ret = hmac_alg_get_user_priv_stru(hmac_user, HMAC_ALG_USER_STRU_ID_SCHEDULE, (osal_void **)&sch_user_info);
    if (osal_unlikely(ret != OAL_SUCC)) {
        return ret;
    }
    if (hmac_user->is_multi_user != OSAL_TRUE) {
        schedule->user_num -= 1;
        if (&(sch_user_info->user_dlist_entry) == g_target_user_head) {
            g_target_user_head = sch_user_info->user_dlist_entry.next;
        }
        osal_dlist_delete_entry(&(sch_user_info->user_dlist_entry));
    }
    alg_schedule_change_target_time(schedule);
    /* 注销结构体 */
    ret = hmac_alg_unregister_user_priv_stru(hmac_user, HMAC_ALG_USER_STRU_ID_SCHEDULE);
    if (osal_unlikely(ret != OAL_SUCC)) {
        osal_kfree(sch_user_info);
        return ret;
    }
    osal_kfree(sch_user_info);
#endif

    return OAL_SUCC;
}

OSAL_STATIC osal_void alg_schedule_init_basic_param(alg_schedule_stru *schedule, hal_to_dmac_device_stru *hal_device)
{
    osal_u8 index;
    memset_s(schedule, sizeof(alg_schedule_stru), 0, sizeof(alg_schedule_stru));

    /* mac_device指针赋值 */
    schedule->hal_device = hal_device;

    /* 初始化txtid调度相关参数 */
    schedule->sch_timer.timeout = ALG_SCHEDULE_SCH_CYCLE_MS;
    /* 初始化所有txtid链表头结节点 */
    for (index = 0; index < HAL_TX_QUEUE_NUM; index++) {
        OSAL_INIT_LIST_HEAD(&(schedule->ac_dlist_head[index]));
    }

    for (index = 0; index < WLAN_TID_MAX_NUM; index++) {
        OSAL_INIT_LIST_HEAD(&(schedule->ac_txtid_dlist[index].txtid_dlist_head));
        osal_list_add_tail(&(schedule->ac_txtid_dlist[index].ac_dlist_entry),
            &(schedule->ac_dlist_head[wlan_wme_tid_to_ac(index)]));
    }

#ifdef _PRE_WLAN_FEATURE_SCH_STRATEGY_WMM_ENSURE
    /* 初始化WMM空口公平性参数 */
    schedule->quota_step = ALG_SCH_WMM_AIR_MOVE_STEP_MPDU; /* 每次移动MPDU数 */
    schedule->tolerance = ALG_SCH_WMM_AIR_USER_PERIOD_TORLERANCE; /* 单位10us */
    schedule->target_mpdu_total = ALG_SCH_WMM_AIR_USER_TARGET_TUOTA_MAX;
    schedule->target_user_id = 0;
    schedule->temp_cnt = 0;
    schedule->user_num = 0;
    OSAL_INIT_LIST_HEAD(&g_user_dlist_head);
#endif
}

OSAL_STATIC osal_u32 alg_sch_para_sync_fill(alg_param_sync_stru *data)
{
    alg_param_sync_sch_stru *sch_sync_data = &data->sch_para;
    sch_sync_data->report_period = ALG_SCH_WMM_DMAC_TIMEOUT_MS;
    sch_sync_data->tx_time_method = 0; /* 默认使用芯片时间 */

    return OAL_SUCC;
}

OSAL_STATIC osal_void alg_schedule_function_init(osal_void)
{
    /* 注册钩子函数 */
    hmac_alg_register_add_user_notify_func(HMAC_ALG_ADD_USER_NOTIFY_SCHEDULE, alg_schedule_init_user_info);
    hmac_alg_register_del_user_notify_func(HMAC_ALG_DEL_USER_NOTIFY_SCHEDULE, alg_schedule_free_user_info);
    hmac_alg_register_tx_schedule_func(alg_schedule_tx_tid_sch);
    hmac_alg_register_tx_schedule_timer_func(alg_schedule_sch_timer_restart_handle);
    hmac_alg_register_para_sync_notify_func(ALG_PARAM_SYNC_SCH, alg_sch_para_sync_fill);
#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
    hmac_alg_register_para_cfg_notify_func(ALG_PARAM_CFG_SCHEDULE, alg_schedule_config_param);
#endif

    hmac_alg_register_tid_update_notify_func(alg_schedule_update_txtid_dlist);
    hmac_alg_register_user_info_update_notify_func(alg_schedule_update_target_user);
}

/******************************************************************************
函 数 名  : alg_schedule_init
功能描述  : 1.创建alg_schedule结构体，并初始化所有变量；
            2.将该结构体与相应的device关联；
            3.注册并start相关定时器；包括WME调整定时器，拥塞控制定时器和令牌注入定时器；
******************************************************************************/
osal_s32 alg_schedule_init(osal_void)
{
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);
    hal_to_dmac_device_stru *hal_device = hal_chip_get_hal_device();
    alg_schedule_stru *schedule = OSAL_NULL;
    osal_u32 ret;

    ret = hmac_alg_register(HMAC_ALG_ID_SCHEDULE);
    if (osal_unlikely(ret != OAL_SUCC)) {
        return (osal_s32)ret;
    }

    /* 设备未初始化 */
    if (hmac_device->device_state != OSAL_TRUE) {
        oam_error_log1(0, OAM_SF_SCHEDULE,
            "{alg_schedule_init::device is not init.device_state = %d}", hmac_device->device_state);
    } else {
        /* 为alg_schedule_stru 开辟空间，并初始化 */
        schedule = &g_schedule;

        alg_schedule_init_basic_param(schedule, hal_device);

        /* 初始化所调度相关定时器，并start */
        frw_create_timer_entry(&(schedule->sch_stat_timer), alg_schedule_stat_timer_log, ALG_SCHEUDLE_STAT_TIMER_MS,
            (osal_void *)(schedule), OSAL_TRUE);

        /* 挂接调度算法结构体 */
        hmac_alg_register_device_priv_stru(hal_device, HAL_ALG_DEVICE_STRU_ID_SCHEDULE, (osal_void *)schedule);
    }

    alg_schedule_function_init();

    return OAL_SUCC;
}

OSAL_STATIC osal_void alg_schedule_function_exit(osal_void)
{
    /* 注销钩子函数 */
    hmac_alg_unregister_add_user_notify_func(HMAC_ALG_ADD_USER_NOTIFY_SCHEDULE);
    hmac_alg_unregister_del_user_notify_func(HMAC_ALG_DEL_USER_NOTIFY_SCHEDULE);
    hmac_alg_unregister_tx_schedule_func();
    hmac_alg_unregister_tx_schedule_timer_func();
    hmac_alg_unregister_tid_update_notify_func();
    hmac_alg_unregister_user_info_update_notify_func();
    hmac_alg_unregister_para_sync_notify_func(ALG_PARAM_SYNC_SCH);
#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
    hmac_alg_unregister_para_cfg_notify_func(ALG_PARAM_CFG_SCHEDULE);
#endif
}

/******************************************************************************
函 数 名  : alg_schedule_exit
功能描述  : 1.销毁alg_schedule结构体；
            2.调用alg_schedule_free_user_info销毁相关结构体；
            3.停止定时器并销毁;
******************************************************************************/
osal_void alg_schedule_exit(osal_void)
{
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);
    hal_to_dmac_device_stru *hal_device = OSAL_NULL;
    hmac_user_stru *hmac_user = OSAL_NULL;
    hmac_vap_stru *hmac_vap = OSAL_NULL;
    alg_schedule_stru *schedule = alg_schedule_get_dev_info();
    osal_u8 vap_index;
    osal_u32 ret;

    struct osal_list_head *list_pos = OSAL_NULL;

    /* 注销算法 */
    ret = hmac_alg_unregister(HMAC_ALG_ID_SCHEDULE);
    if (osal_unlikely(ret != OAL_SUCC)) {
        return;
    }
    alg_schedule_function_exit();

    /* 释放所有device下挂接的调度算法私有结构体，包括所有user, txtid的调度算法私有结构体 */
    hal_device = hal_chip_get_hal_device();

    /* 停止所调度相关定时器，并注销 */
    frw_destroy_timer_entry(&(schedule->sch_timer));
    frw_destroy_timer_entry(&(schedule->sch_stat_timer));

    alg_device_foreach_vap(hmac_vap, hmac_device, vap_index) {
        /* 非本次hal device不处理 */
        if (hal_device != hmac_vap->hal_device) {
            continue;
        }

        /* 遍列所有vap，释放所有vap和user私有结构体相关信息 */
        alg_vap_foreach_user(hmac_user, hmac_vap, list_pos) {
            /* 释放user信息结构体 */
            ret = alg_schedule_free_user_info(hmac_vap, hmac_user);
            if (osal_unlikely(ret != OAL_SUCC)) {
                oam_warning_log0(0, OAM_SF_SCHEDULE,
                    "{alg_schedule_exit::alg_schedule_free_user_info failed}");
                continue;
            }
        }
    }

    /* 注销device级别信息结构体 */
    hmac_alg_unregister_device_priv_stru(hal_device, HAL_ALG_DEVICE_STRU_ID_SCHEDULE);
}

/******************************************************************************
函 数 名  :  alg_schedule_certify_sch
功能描述  :  从高优先级开始调度
 ******************************************************************************/
WIFI_TCM_TEXT osal_u32 alg_schedule_certify_sch(alg_schedule_stru *schedule, osal_u8 vap_id,
    alg_sch_output_stru *sch_result)
{
    osal_u32 ret;
    osal_u8 index;
    osal_u8 ac_num[] = {WLAN_WME_AC_VO, WLAN_WME_AC_VI, WLAN_WME_AC_BE, WLAN_WME_AC_BK};

    for (index = 0; index < oal_array_size(ac_num); index++) {
        ret = alg_schedule_rr_sch(schedule, vap_id, ac_num[index], sch_result);
        if (osal_unlikely(ret != OAL_SUCC)) {
            oam_warning_log0(0, OAM_SF_SCHEDULE, "{alg_schedule_certify_sch::alg_schedule_rr_sch failed}");
            return ret;
        }
        if (sch_result->sch_flag == OSAL_TRUE) {
            break;
        }
    }
    return OAL_SUCC;
}
/******************************************************************************
函 数 名  : alg_schedule_tx_tid_sch
功能描述  : 对于该device上所有vap，所有STA, 所有非空txtid队列进行调度:
            注意:(1)广播帧对应专门的广播队列;(2) 管理帧不在本调度范围内；
 ******************************************************************************/
WIFI_TCM_TEXT osal_u32 alg_schedule_tx_tid_sch(const hal_to_dmac_device_stru *hal_device, osal_u8 ac_num,
    mac_tid_schedule_output_stru *sch_output)
{
    alg_schedule_stru *schedule = alg_schedule_get_dev_info();
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);
    osal_u8 vap_id = 0;
    osal_u32 ret;
    alg_sch_output_stru output = { OSAL_NULL, OSAL_NULL, OSAL_FALSE, { 0 } };
    unref_param(hal_device);

    /* 出参初始化 */
    sch_output->tid_num = WLAN_TID_MAX_NUM;
    sch_output->tx_mode = DMAC_TX_MODE_BUTT;
    sch_output->mpdu_num[0] = 0;
    sch_output->mpdu_num[1] = 0;
    sch_output->user_idx = 0xFFFF;
    sch_output->ba_is_jamed = OSAL_FALSE;

    if (g_sch_switch == ALG_SCH_STRATEGY_ROUND_ROBIN) {
        ret = alg_schedule_round_robin_sch(schedule, vap_id, ac_num, &output);
#ifdef _PRE_WLAN_FEATURE_SCH_STRATEGY_WMM_ENSURE
    } else if (g_sch_switch == ALG_SCH_STRATEGY_WMM_ENSURE) {
        ret = alg_schedule_wmm_sch(schedule, vap_id, ac_num, &output);
#endif
    } else if (g_sch_switch == ALG_SCH_STRATEGY_CERTIFY) {
        ret = alg_schedule_certify_sch(schedule, vap_id, &output);
    } else {
        oam_error_log1(0, OAM_SF_SCHEDULE, "{alg_schedule_update_sch::Schedule para error[%d]!}", g_sch_switch);
        g_sch_switch = ALG_SCH_STRATEGY_ROUND_ROBIN;
        ret = alg_schedule_round_robin_sch(schedule, vap_id, ac_num, &output);
    }

    if (osal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log0(0, OAM_SF_SCHEDULE, "{alg_schedule_update_sch::alg_schedule_qos_wmm_rr_sch failed!}");
        return ret;
    }

    if ((output.user == OSAL_NULL) || (output.txtid == OSAL_NULL)) {
        oam_info_log0(0, OAM_SF_SCHEDULE, "{alg_schedule_update_sch::Nothing to be scheduled}");
        return OAL_SUCC;
    }

    ret = alg_schedule_update_sch_result(hmac_device, schedule, &output, sch_output);
    if (osal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log0(0, OAM_SF_SCHEDULE, "{alg_schedule_update_sch::alg_schedule_update_sch_result failed!}");
        return ret;
    }
    return OAL_SUCC;
}

/******************************************************************************
函 数 名  : alg_schedule_update_txtid_dlist
功能描述  : tid有出/入队，或者被pause或者解除pause事件发生时，更新该txtid对应的dlist链表
 ******************************************************************************/
WIFI_TCM_TEXT osal_u32 alg_schedule_update_txtid_dlist(hmac_tid_stru *txtid, osal_u8 in_mpdu_num)
{
    alg_schedule_txtid_info_stru *sch_txtid_info = OSAL_NULL;
    alg_schedule_stru *schedule = OSAL_NULL;
    hmac_user_stru *hmac_user = OSAL_NULL;
    alg_schedule_user_info_stru *sch_user_info = OSAL_NULL;
    osal_u32 ret;

    if (osal_unlikely(txtid->tid >= WLAN_TID_MAX_NUM)) {
        oam_warning_log0(0, OAM_SF_SCHEDULE, "{alg_schedule_update_txtid_dlist:: tidno invalid !}");
        return OAL_SUCC;
    }

    /* 查找该用户所属的user */
    hmac_user = (hmac_user_stru *)mac_res_get_hmac_user_etc(txtid->user_idx);
    if (osal_unlikely(hmac_user == OSAL_NULL)) {
        oam_warning_log1(0, OAM_SF_SCHEDULE, "{alg_schedule_update_txtid_dlist::mac_user[%d] is NULL!}",
            txtid->user_idx);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取txtid的私有信息结构体 */
    ret = hmac_alg_get_tid_priv_stru(hmac_user, txtid->tid, HMAC_ALG_TID_STRU_ID_SCHEDULE,
        (osal_void **)&sch_txtid_info);
    if (osal_unlikely(ret != OAL_SUCC)) {
        return ret;
    }
    ret = hmac_alg_get_user_priv_stru(hmac_user, HMAC_ALG_USER_STRU_ID_SCHEDULE, (osal_void **)&sch_user_info);
    if (ret != OAL_SUCC) {
        return OAL_SUCC;
    }
    if (in_mpdu_num != 0) {
        sch_user_info->stat_in_mpdu_num[txtid->tid] += in_mpdu_num;
    }
    /* 如果txtid没有处于调度当中，但是txtid队列当前不为空且tid没有被puase掉，则需要添加至dlist当中 */
    if ((sch_txtid_info->is_scheduling == OSAL_FALSE) && ((txtid->mpdu_num != 0) && (txtid->is_paused == 0))) {
        schedule = sch_txtid_info->schedule;
        osal_list_add_tail(&(sch_txtid_info->txtid_dlist_entry),
            &(schedule->ac_txtid_dlist[txtid->tid].txtid_dlist_head));
        sch_txtid_info->is_scheduling = OSAL_TRUE;
    } else if ((sch_txtid_info->is_scheduling == OSAL_TRUE) && ((txtid->mpdu_num == 0) || (txtid->is_paused > 0))) {
        /* 如果txtid已处于调度当中，但是txtid队列已为空或者pst_txtid已被pause掉 ，则从dlist当中删除 */
        osal_dlist_delete_entry(&(sch_txtid_info->txtid_dlist_entry));
        sch_txtid_info->is_scheduling = OSAL_FALSE;
    }
    return OAL_SUCC;
}
/****************************************************************************
功能描述  : alg_schedule_sch_timer_restart_handle调度主流程开启定时器
******************************************************************************/
WIFI_TCM_TEXT osal_u32 alg_schedule_sch_timer_restart_handle(const hal_to_dmac_device_stru *hal_device)
{
    alg_schedule_stru *schedule = alg_schedule_get_dev_info();
    unref_param(hal_device);
    if (schedule->sch_timer.is_registerd != OAL_TRUE) {
        frw_create_timer_entry(&(schedule->sch_timer), alg_schedule_sch_timer_handle, schedule->sch_timer.timeout,
            (osal_void *)schedule, OSAL_FALSE);
    }
    return OAL_SUCC;
}
/****************************************************************************
功能描述  : alg_schedule_sch_timer_handle周期灌包定时器
******************************************************************************/
WIFI_TCM_TEXT osal_u32 alg_schedule_sch_timer_handle(osal_void *void_code)
{
    alg_schedule_stru *schedule = OSAL_NULL;

    /* 对入参进行合法性判断 */
    if (void_code == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_SCHEDULE, "{alg_schedule_sch_timer_handle::input param invalid !}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    schedule = (alg_schedule_stru *)void_code;
    oam_info_log1(0, OAM_SF_SCHEDULE, "alg_schedule_sch_timer_handle::timeout[%u]", schedule->sch_timer.timeout);
    /* 周期性下包AC0 */
    hmac_tx_schedule(hal_chip_get_hal_device(), WLAN_WME_AC_BE);
    return OAL_SUCC;
}

/******************************************************************************
函 数 名  : alg_schedule_get_tid_by_info_stru
功能描述  : 根据txtid私有信息结构体，找到对应的txtid
******************************************************************************/
WIFI_TCM_TEXT osal_u32 alg_schedule_get_user_tid_by_info_stru(const alg_schedule_txtid_info_stru *sch_txtid_info,
    hmac_user_stru **hmac_user, hmac_tid_stru **txtid)
{
    hmac_user_stru *temp_user = OSAL_NULL;

    temp_user = (hmac_user_stru *)mac_res_get_hmac_user_etc(sch_txtid_info->assoc_id);
    if (osal_unlikely((temp_user == OSAL_NULL) || (sch_txtid_info->tidno > WLAN_TID_MAX_NUM))) {
        oam_warning_log1(0, OAM_SF_SCHEDULE, "{alg_schedule_get_user_tid_by_info_stru::user[%d] is NULL!}",
            sch_txtid_info->assoc_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 根据tid号获取tid队列指针 */
    hmac_user_get_tid_by_num(temp_user, sch_txtid_info->tidno, txtid);

    *hmac_user = temp_user;

    return OAL_SUCC;
}


/******************************************************************************
函 数 名  : alg_schedule_init_txtid_info
功能描述  : 初始化STA所属的所有txtid的调度信息结构体；
******************************************************************************/
osal_u32 alg_schedule_init_txtid_info(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user, alg_schedule_stru *schedule)
{
#ifdef _PRE_WLAN_FEATURE_SCH_STRATEGY_WMM_ENSURE
    alg_schedule_user_info_stru *sch_user_info = OSAL_NULL;
#endif
    hmac_tid_stru *txtid = OSAL_NULL;
    osal_u8 txtid_index;
    osal_u32 ret;
    unref_param(hmac_vap);
    /* 对该用户的每个txtid，申请私有信息结构体，初始化并挂载 */
    for (txtid_index = WLAN_TIDNO_BEST_EFFORT; txtid_index < WLAN_TID_MAX_NUM; txtid_index++) {
        alg_schedule_txtid_info_stru *sch_info = OSAL_NULL;
        /* 根据tid号获取tid队列指针 */
        hmac_user_get_tid_by_num(hmac_user, txtid_index, &txtid);

#ifdef _PRE_WLAN_FEATURE_SCH_STRATEGY_WMM_ENSURE
        /* 获取USER私有结构体指针 */
        ret = hmac_alg_get_user_priv_stru(hmac_user, HMAC_ALG_USER_STRU_ID_SCHEDULE, (osal_void **)&sch_user_info);
        if (osal_unlikely(ret != OAL_SUCC)) {
            oam_warning_log0(0, OAM_SF_SCHEDULE, "{alg_schedule_init_txtid_info:get_user_priv_stru fail}");
            return ret;
        }
#endif

        /* 为该用户的每个txtid申请私有信息结构体 */
        if (hmac_user->assoc_id >= WLAN_USER_MAX_USER_LIMIT) {
            oam_warning_log1(0, OAM_SF_SCHEDULE, "user assoc_id=%d, error}", hmac_user->assoc_id);
            return OAL_FAIL;
        }
        sch_info = (alg_schedule_txtid_info_stru *)alg_mem_alloc(ALG_MEM_SCHEDULE,
            sizeof(alg_schedule_txtid_info_stru));
        if (sch_info == OSAL_NULL) {
            oam_error_log0(0, OAM_SF_SCHEDULE, "{alg_schedule_init_txtid_info: mem alloc fail!}");
            return OAL_ERR_CODE_PTR_NULL;
        }
        memset_s(sch_info, sizeof(alg_schedule_txtid_info_stru), 0, sizeof(alg_schedule_txtid_info_stru));

        ret = hmac_alg_register_tid_priv_stru(hmac_user, txtid_index, HMAC_ALG_TID_STRU_ID_SCHEDULE,
            (osal_void *)sch_info);
        if (osal_unlikely(ret != OAL_SUCC)) {
            osal_kfree(sch_info);
            oam_warning_log0(0, OAM_SF_SCHEDULE, "{alg_schedule_init_txtid_info:get tid priv fail");
            return ret;
        }

        /* 初始化txtid级别的调度私有信息结构体 */
        sch_info->is_scheduling = OSAL_FALSE;
        sch_info->assoc_id = hmac_user->assoc_id;
        sch_info->tidno = txtid->tid;
        sch_info->vap_id = hmac_user->vap_id;
        /* 赋值TID对应的私有user及vap及device指针 */
        sch_info->schedule = schedule;

#ifdef _PRE_WLAN_FEATURE_SCH_STRATEGY_WMM_ENSURE
        sch_info->is_stopped = OSAL_FALSE;
        sch_info->bad_link_flag = OSAL_FALSE;
        sch_info->sch_user_info = sch_user_info;
#endif
        OSAL_INIT_LIST_HEAD(&(sch_info->txtid_dlist_entry));
        /* 挂载用户级别的调度私有信息结构体 */
    }
    return OAL_SUCC;
}

/******************************************************************************
函 数 名  : alg_schedule_recycle_mem
功能描述  : 回收内存
******************************************************************************/
osal_void alg_schedule_recycle_user_mem(hmac_user_stru *hmac_user)
{
#ifdef _PRE_WLAN_FEATURE_SCH_STRATEGY_WMM_ENSURE
    alg_schedule_user_info_stru *sch_user_info = OSAL_NULL;
    alg_schedule_stru *schedule = alg_schedule_get_dev_info();
#endif
    alg_schedule_txtid_info_stru *sch_txtid_info = OSAL_NULL;
    osal_u8 txtid_index;
    osal_u32 ret;

    if (hmac_user == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_SCHEDULE, "{alg_schedule_recycle_user_mem: input pointer is null!}");
        return;
    }

    /* 对该用户的每个txtid，释放其已占有的内存 */
    for (txtid_index = WLAN_TIDNO_BEST_EFFORT; txtid_index < WLAN_TID_MAX_NUM; txtid_index++) {
        /* 获取tid级别的调度私有信息结构体 */
        ret = hmac_alg_get_tid_priv_stru(hmac_user, txtid_index, HMAC_ALG_TID_STRU_ID_SCHEDULE,
            (osal_void **)&sch_txtid_info);
        if (osal_unlikely(ret != OAL_SUCC)) {
            continue;
        }

        if (sch_txtid_info != OSAL_NULL) {
            /* 释放内存前，注销txtid私有信息结构体，避免产生野指针 */
            hmac_alg_unregister_tid_priv_stru(hmac_user, txtid_index, HMAC_ALG_TID_STRU_ID_SCHEDULE);
            osal_kfree(sch_txtid_info);
        }
    }

#ifdef _PRE_WLAN_FEATURE_SCH_STRATEGY_WMM_ENSURE
    ret = hmac_alg_get_user_priv_stru(hmac_user, HMAC_ALG_USER_STRU_ID_SCHEDULE, (osal_void **)&sch_user_info);
    if (osal_unlikely(ret != OAL_SUCC)) {
        return;
    }
    if (hmac_user->is_multi_user != OSAL_TRUE) {
        schedule->user_num -= 1;
        if (&(sch_user_info->user_dlist_entry) == g_target_user_head) {
            g_target_user_head = sch_user_info->user_dlist_entry.next;
        }
        osal_dlist_delete_entry(&(sch_user_info->user_dlist_entry));
    }
    alg_schedule_change_target_time(schedule);
    if (sch_user_info != OSAL_NULL) {
        /* 释放内存前，注销user私有信息结构体，避免产生野指针 */
        hmac_alg_unregister_user_priv_stru(hmac_user, HMAC_ALG_USER_STRU_ID_SCHEDULE);
        osal_kfree(sch_user_info);
    }
#endif
    return;
}

WIFI_TCM_TEXT osal_u32 alg_schedule_update_sch_result_in_certain_circumstances(alg_schedule_stru *schedule,
    mac_tid_schedule_output_stru *sch_output, const alg_sch_output_stru *sch_result)
{
    hmac_user_stru *hmac_user = sch_result->user;
    hmac_tid_stru *txtid = sch_result->txtid;
    alg_schedule_user_info_stru *user_info = NULL;
    osal_u32 ret;
    unref_param(schedule);

    /* 如果为节能用户，则每次只调度一个mpdu */
    if (hmac_user_get_ps_mode(hmac_user) == OSAL_TRUE && (txtid->tid != WLAN_TIDNO_UAPSD)) {
        sch_output->user_idx = hmac_user->assoc_id;
        sch_output->tid_num = txtid->tid;
        sch_output->tx_mode = txtid->tx_mode;
        sch_output->mpdu_num[0] = (osal_u8)(osal_min(1, txtid->mpdu_num));
        sch_output->mpdu_num[1] = 0;
        hmac_tid_pause(txtid, DMAC_TID_PAUSE_RESUME_TYPE_PS);
        return OAL_SUCC;
    }
    ret = hmac_alg_get_user_priv_stru(hmac_user, HMAC_ALG_USER_STRU_ID_SCHEDULE, (osal_void **)&user_info);
    if (ret != OAL_SUCC) {
        return OAL_SUCC;
    }
    if (g_sch_switch == ALG_SCH_STRATEGY_WMM_ENSURE) {
        /* 广播报文不考虑阈值 */
        if (txtid->tid == WLAN_TIDNO_BCAST) {
            sch_output->mpdu_num[0] = (osal_u8)txtid->mpdu_num;
        } else {
            sch_output->mpdu_num[0] = (osal_u8)osal_min(user_info->remain_quota, txtid->mpdu_num);
        }
    } else {
        sch_output->mpdu_num[0] = (osal_u8)osal_min(1, txtid->mpdu_num);
    }
    /* 更新输出结果 */
    sch_output->tid_num = txtid->tid;
    sch_output->tx_mode = txtid->tx_mode;
    sch_output->user_idx = hmac_user->assoc_id;
    sch_output->mpdu_num[1] = 0;
    user_info->stat_out_mpdu_num[txtid->tid] += sch_output->mpdu_num[0];
    user_info->stat_sch_num[txtid->tid] += 1;
    return OAL_SUCC;
}

/******************************************************************************
函 数 名  : alg_schedule_update_sch_result
功能描述  : 根据选择的user和tid,更新调度结果;
******************************************************************************/
WIFI_TCM_TEXT osal_u32 alg_schedule_update_sch_result(hmac_device_stru *hmac_device, alg_schedule_stru *schedule,
    const alg_sch_output_stru *sch_result, mac_tid_schedule_output_stru *sch_output)
{
    hmac_tid_stru *txtid = sch_result->txtid;
    oal_netbuf_stru *netbuf = OSAL_NULL;
    osal_u32 ret;
    unref_param(hmac_device);

    /* 对于UAPSD tid队列进行特殊处理 */
    if (txtid->tid == WLAN_TIDNO_UAPSD) {
        ret = hmac_tid_get_mpdu_by_index(txtid, (osal_u16)0, &netbuf);
        if (osal_unlikely(ret != OAL_SUCC)) {
            oam_warning_log0(0, OAM_SF_SCHEDULE, "{hmac_tid_get_mpdu_by_index::hmacAlgGetUserPrivStru failed}");
            return ret;
        }
    }

    alg_schedule_update_sch_result_in_certain_circumstances(schedule, sch_output, sch_result);
    return OAL_SUCC;
}

/****************************************************************************
函 数 名  : alg_schedule_rr_sch
功能描述  : 简单的轮询调度算法
 ******************************************************************************/
WIFI_TCM_TEXT osal_u32 alg_schedule_rr_sch(alg_schedule_stru *schedule, osal_u8 vap_id, osal_s8 ac_num,
    alg_sch_output_stru *sch_result)
{
    struct osal_list_head *ac_dlist_pos = OSAL_NULL;
    struct osal_list_head *temp1 = OSAL_NULL;
    struct osal_list_head *temp2 = OSAL_NULL;
    alg_sch_ac_txtid_dlist_stru *ac_txtid_entry = OSAL_NULL;
    struct osal_list_head *tid_dlist_pos = OSAL_NULL;
    alg_schedule_txtid_info_stru *sch_txtid_info = OSAL_NULL;
    hmac_user_stru *hmac_user = OSAL_NULL;
    hmac_tid_stru *txtid = OSAL_NULL;
    osal_u32 ret;
    /* 找到对应的AC链表 */
    struct osal_list_head *ac_dlist_head = &(schedule->ac_dlist_head[ac_num]);

    unref_param(vap_id);

    /* 遍列AC链表下的txtid链表头结点 */
    osal_list_for_each_safe(ac_dlist_pos, temp1, ac_dlist_head) {
        struct osal_list_head *txtid_dlist_head = OSAL_NULL;
        ac_txtid_entry = osal_list_entry(ac_dlist_pos, alg_sch_ac_txtid_dlist_stru, ac_dlist_entry);
        txtid_dlist_head = &(ac_txtid_entry->txtid_dlist_head);
        if (osal_list_empty(txtid_dlist_head) == OSAL_TRUE) {
            continue;
        }

        /* 遍列txtid链表下的tid */
        osal_list_for_each_safe(tid_dlist_pos, temp2, txtid_dlist_head) {
            hmac_vap_stru *hmac_vap = OSAL_NULL;
            sch_txtid_info = osal_list_entry(tid_dlist_pos, alg_schedule_txtid_info_stru, txtid_dlist_entry);
            ret = alg_schedule_get_user_tid_by_info_stru(sch_txtid_info, &hmac_user, &txtid);
            if (osal_unlikely(ret != OAL_SUCC)) {
                return ret;
            }

            /* 判断vap是否处于暂停状态 */
            hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(hmac_user->vap_id);
            if (osal_unlikely(hmac_vap == OSAL_NULL)) {
                oam_error_log0(0, OAM_SF_SCHEDULE, "{alg_schedule_rr_sch::mac_res_get_hmac_vap fail!}");
                return OAL_ERR_CODE_PTR_NULL;
            }

            if (alg_schedule_is_vap_paused(hmac_vap)) {
                continue;
            }

            if (txtid->mpdu_num == 0) {
                continue;
            }
            /* 出参赋值 */
            sch_result->user = hmac_user;
            sch_result->txtid = txtid;
            sch_result->sch_flag = OSAL_TRUE;
            /* 将 tid 插入队尾 */
            osal_dlist_delete_entry(&(sch_txtid_info->txtid_dlist_entry));
            osal_list_add_tail(&(sch_txtid_info->txtid_dlist_entry), txtid_dlist_head);

            /* 将 ac 插入队尾 */
            osal_dlist_delete_entry(&(ac_txtid_entry->ac_dlist_entry));
            osal_list_add_tail(&(ac_txtid_entry->ac_dlist_entry), ac_dlist_head);

            return OAL_SUCC;
        }
    }
    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_SCH_STRATEGY_WMM_ENSURE
/******************************************************************************
函 数 名  :  alg_schedule_get_user_tid_info
功能描述  :  统计打印处理周期性函数
 ******************************************************************************/
OSAL_STATIC void alg_schedule_get_user_tid_info(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    alg_schedule_user_info_stru *user_info)
{
    osal_u8 index;
    hmac_tid_stru *tid_queue = OSAL_NULL;
    for (index = 0; index < WLAN_TIDNO_BUTT; index++) {
        tid_queue = &(hmac_user->tx_tid_queue[index]);
        if ((g_stat_log != 0) && (user_info->stat_in_mpdu_num[index] != 0 || tid_queue->mpdu_num != 0)) {
#ifdef _PRE_WLAN_ALG_UART_PRINT
            wifi_printf("alg_schedule_stat_timer_log::tid [%d], out MPDU num[%d], in MPDU num[%d], \
                sch num[%d], user id[%d]\n", index, user_info->stat_out_mpdu_num[index],
                user_info->stat_in_mpdu_num[index], user_info->stat_sch_num[index], user_info->user_id);
#endif
            oam_warning_log4(0, OAM_SF_SCHEDULE, "[SCH_STAT]alg_schedule_stat_timer_log::out MPDU num[%d], \
                in MPDU num[%d], sch num[%d], user id[%d]\n", user_info->stat_out_mpdu_num[index],
                user_info->stat_in_mpdu_num[index], user_info->stat_sch_num[index], user_info->user_id);
            oam_warning_log4(0, OAM_SF_SCHEDULE, "[SCH_STAT]alg_schedule_stat_timer_log::tid [%d], \
                queue_num[%d], pause[%d], state[%u]\n", index, tid_queue->mpdu_num, tid_queue->is_paused,
                hmac_vap->vap_state);
        }
        user_info->stat_sch_num[index] = 0;
        user_info->stat_out_mpdu_num[index] = 0;
        user_info->stat_in_mpdu_num[index] = 0;
    }
    return;
}

/******************************************************************************
函 数 名  :  alg_schedule_stat_log
功能描述  :  统计打印处理周期性函数
 ******************************************************************************/
OSAL_STATIC osal_u32 alg_schedule_stat_timer_log(osal_void *void_code)
{
    hmac_user_stru *hmac_user = OSAL_NULL;
    alg_schedule_user_info_stru *user_info = OSAL_NULL;
    hmac_vap_stru *hmac_vap;
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);
    osal_u8 vap_index, ac_index;
    struct osal_list_head *list_pos = OSAL_NULL;
    alg_schedule_stru *schedule = alg_schedule_get_dev_info();
    osal_u32 ret;

    if (void_code == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_SCHEDULE, "{alg_schedule_stat_timer_log::input param invalid !}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    /* 对AC保护调度重置 */
    for (ac_index = 0; ac_index < WLAN_WME_AC_BUTT; ac_index++) {
        schedule->sch_nums_per_ac[ac_index] = 0;
    }
    alg_device_foreach_vap(hmac_vap, hmac_device, vap_index) {
        alg_vap_foreach_user(hmac_user, hmac_vap, list_pos) {
            ret = hmac_alg_get_user_priv_stru(hmac_user, HMAC_ALG_USER_STRU_ID_SCHEDULE, (osal_void **)&user_info);
            if (ret != OAL_SUCC) {
                return ret;
            }
            alg_schedule_get_user_tid_info(hmac_vap, hmac_user, user_info);
        }
    }
    return OAL_SUCC;
}

/******************************************************************************
函 数 名  :  alg_schedule_check_total_tx_time
功能描述  :  判断是否需要更新
 ******************************************************************************/
OSAL_STATIC osal_u32 alg_schedule_check_total_tx_time(dmac_to_hmac_sch_user_info_stru *sync_info)
{
    osal_u8 i;
    osal_u32 all_user_tx_time = 0;
    osal_u32 all_user_tx_bytes = 0;
    for (i = 0; i < sync_info->header.user_num; i++) {
        all_user_tx_time += sync_info->user_info[i].total_tx_time;
        all_user_tx_bytes += sync_info->user_info[i].tx_mpdu_bytes;
    }
    if ((all_user_tx_time < ALG_SCH_WMM_AIR_TOTAL_VALID_TXTIME_MIN) ||
            (all_user_tx_bytes) < ALG_SCH_WMM_AIR_TOTAL_VALID_TXBYTES_MIN) {
        return OSAL_FALSE;
    }
    return OSAL_TRUE;
}

/******************************************************************************
函 数 名  :alg_schedule_print_gla_info
功能描述  : 调度算法图形化维测打印
 ******************************************************************************/
OSAL_STATIC osal_void alg_schedule_print_gla_info(osal_u32 real_time_consume,
    alg_schedule_user_info_stru *sch_pri_user_info, osal_u8 user_id)
{
    oam_warning_log4(0, OAM_SF_SCHEDULE, "[GLA][ALG][SCH]:user_id=%d,target_time=%d,target_mpdu=%d,real_time=%d",
        user_id, sch_pri_user_info->target_time, sch_pri_user_info->target_quota, real_time_consume);
}

/******************************************************************************
函 数 名  :  alg_schedule_user_info_update
功能描述  :  device 到 host侧用户统计同步更新
 ******************************************************************************/
OSAL_STATIC osal_void alg_schedule_user_info_update(dmac_to_hmac_sch_user_info_stru *sync_info)
{
    osal_u8 i;
    osal_u32 ret, real_time_consume;
    dmac_to_hmac_user_rate_stru *user_info = OSAL_NULL;
    alg_schedule_user_info_stru *sch_pri_user_info = OSAL_NULL;
    hmac_user_stru *hmac_user = OSAL_NULL;
    alg_schedule_stru *schedule = alg_schedule_get_dev_info();
    oal_bool_enum_uint8 switch_enable = alg_host_get_gla_switch_enable(ALG_GLA_ID_SCHEDULE, ALG_GLA_OPTIONAL_SWITCH);

    for (i = 0; i < sync_info->header.user_num; i++) {
        user_info = &(sync_info->user_info[i]);
        hmac_user = (hmac_user_stru *)mac_res_get_hmac_user_etc(user_info->user_id);

        ret = hmac_alg_get_user_priv_stru(hmac_user, HMAC_ALG_USER_STRU_ID_SCHEDULE, (osal_void **)&sch_pri_user_info);
        if (ret != OAL_SUCC) {
            return;
        }

        /* 计算实际空口消耗时间 */
        if (user_info->tx_mpdu_bytes > ALG_SCH_WMM_AIR_USER_VALID_TXBYTES_MIN) {
            real_time_consume = sch_pri_user_info->target_quota * (ALG_SCH_WMM_AIR_MAX_BUF_BYTES *
                user_info->total_tx_time / user_info->tx_mpdu_bytes);
        } else {
            real_time_consume = sch_pri_user_info->target_time; /* 小流量的用户不调整 */
        }

        /* 判断是否更新目标报文个数 */
        if (real_time_consume + schedule->tolerance < sch_pri_user_info->target_time) {
            sch_pri_user_info->target_quota += schedule->quota_step;
        } else if (real_time_consume - schedule->tolerance > sch_pri_user_info->target_time) {
            sch_pri_user_info->target_quota -= schedule->quota_step;
        }

        /* 目标报文个数不能小于阈值 */
        if (sch_pri_user_info->target_quota < ALG_SCH_WMM_AIR_USER_TARGET_TUOTA_MIN) {
            sch_pri_user_info->target_quota = ALG_SCH_WMM_AIR_USER_TARGET_TUOTA_MIN;
        } else if (sch_pri_user_info->target_quota > ALG_SCH_WMM_AIR_USER_TARGET_TUOTA_MAX) {
            sch_pri_user_info->target_quota = ALG_SCH_WMM_AIR_USER_TARGET_TUOTA_MAX;
        }
        if (osal_unlikely(g_stat_log != 0)) {
#ifdef _PRE_WLAN_ALG_UART_PRINT
            wifi_printf("{alg_schedule_user_info_sync:target_mpdu[%d], target_time [%d], user id[%d]\n",
                sch_pri_user_info->target_quota, sch_pri_user_info->target_time, user_info->user_id);
#endif
            oam_warning_log3(0, OAM_SF_SCHEDULE, "{[SCH_STAT]alg_schedule_user_info_sync:target_mpdu[%d],\
                target_time [%d],user id[%d]\n", sch_pri_user_info->target_quota,
                sch_pri_user_info->target_time, user_info->user_id);
        }

        if (switch_enable == OSAL_TRUE) {
            alg_schedule_print_gla_info(real_time_consume, sch_pri_user_info, user_info->user_id);
        }
    }
}

/******************************************************************************
函 数 名  :  alg_schedule_user_info_sync
功能描述  :  device 到 host侧信息同步函数
 ******************************************************************************/
osal_s32 alg_schedule_user_info_sync(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    dmac_to_hmac_sch_user_info_stru *sync_info = OSAL_NULL;

    if (osal_unlikely((msg == OAL_PTR_NULL) || (hmac_vap == OAL_PTR_NULL))) {
        oam_error_log0(0, OAM_SF_ROAM, "{del_ba_event::msg or hmac_vap is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    sync_info = (dmac_to_hmac_sch_user_info_stru *)msg->data;

    /* 空口实际发送时间及报文字节过少不调整 */
    if (alg_schedule_check_total_tx_time(sync_info) == OSAL_FALSE) {
        return OAL_SUCC;
    }

    alg_schedule_user_info_update(sync_info);
    return OAL_SUCC;
}

/******************************************************************************
函 数 名  :  alg_schedule_target_user
功能描述  :  wmm调度指定用户
 ******************************************************************************/
WIFI_TCM_TEXT osal_u32 alg_schedule_target_user(alg_schedule_stru *schedule, osal_u8 vap_id, osal_u8 ac_num,
    alg_sch_output_stru *sch_output)
{
    osal_u8 tid_no = 0, index;
    osal_u32 ret;
    osal_u8 user_id = schedule->target_user_id;
    hmac_user_stru *hmac_user = NULL;
    hmac_tid_stru *txtid = OSAL_NULL;
    hmac_vap_stru *hmac_vap = OSAL_NULL;
    alg_schedule_user_info_stru *user_info = NULL;
    unref_param(vap_id);
    unref_param(ac_num);

    hmac_user = (hmac_user_stru *)mac_res_get_hmac_user_etc(user_id);
    if (hmac_user == NULL) {
        oam_warning_log1(0, OAM_SF_SCHEDULE, "{alg_schedule_target_user::alg_wmm_get_user_info "
            "get user info fail, user_id[%d]]}", user_id);
        return OAL_ERR_CODE_PTR_NULL;
    }
    /* 组播用户不调度 */
    if (osal_unlikely(hmac_user->is_multi_user == OSAL_TRUE)) {
        return OAL_SUCC;
    }
    /* 判断vap是否处于暂停状态 */
    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(hmac_user->vap_id);
    if (osal_unlikely(hmac_vap == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_SCHEDULE, "{alg_schedule_rr_sch::mac_res_get_hmac_vap fail!}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    /* VAP在非UP状态，不进行调度 */
    if (alg_schedule_is_vap_paused(hmac_vap)) {
        return OAL_SUCC;
    }

    ret = hmac_alg_get_user_priv_stru(hmac_user, HMAC_ALG_USER_STRU_ID_SCHEDULE, (osal_void **)&user_info);
    if (ret != OAL_SUCC) {
        return ret;
    }
    /* 用户无剩余阈值不调度 */
    if (user_info->remain_quota < 1) {
        return OAL_SUCC;
    }
    /* 广播已优先调度，不再调度广播 */
    for (index = 0; index < WLAN_TIDNO_BUTT; index++) {
        tid_no = g_tid_prio[index];
        hmac_user_get_tid_by_num(hmac_user, tid_no, &txtid);
        if ((txtid->mpdu_num) != 0 && (tid_no != WLAN_TIDNO_BCAST) && (txtid->is_paused == 0)) {
            sch_output->user = hmac_user;
            sch_output->txtid = txtid;
            sch_output->sch_flag = OSAL_TRUE;
            break;
        }
    }
    return OAL_SUCC;
}

/******************************************************************************
函 数 名  :  alg_schedule_update_target_user
功能描述  :  wmm调度入口函数
 ******************************************************************************/
WIFI_TCM_TEXT osal_u32 alg_schedule_update_target_user(const hal_to_dmac_device_stru *hal_device,
    hmac_user_stru *hmac_user, mac_tid_schedule_output_stru schedule_ouput, osal_u8 device_mpdu_full)
{
    alg_schedule_user_info_stru *user_info = NULL;
    osal_u32 ret;
    osal_u8 mpdu_num;
    unref_param(hal_device);

    mpdu_num = schedule_ouput.mpdu_num[0];
    if (g_sch_switch != ALG_SCH_STRATEGY_WMM_ENSURE) {
        return OAL_SUCC;
    }

    ret = hmac_alg_get_user_priv_stru(hmac_user, HMAC_ALG_USER_STRU_ID_SCHEDULE, (osal_void **)&user_info);
    if (ret != OAL_SUCC) {
        return ret;
    }
    /* 广播报文不考虑阈值 */
    if (schedule_ouput.tid_num == WLAN_TIDNO_BCAST) {
        return OAL_SUCC;
    }
    if (user_info->remain_quota > mpdu_num) {
        user_info->remain_quota -= mpdu_num;
        /* 若是因device MPDU满导致，不切换目标用户 */
        if (device_mpdu_full == OSAL_TRUE) {
            return OAL_SUCC;
        }
    } else {
        user_info->remain_quota = 0;
    }
    /* 若device MPDU没有满，或用户已无剩余阈值，则更新到下一个用户 */
    if (user_info->user_dlist_entry.next != &g_user_dlist_head) {
        g_target_user_head = user_info->user_dlist_entry.next;
    } else {
        g_target_user_head = g_user_dlist_head.next;
    }
    /* 重置配额 */
    user_info->remain_quota = user_info->target_quota;
    return OAL_SUCC;
}
/******************************************************************************
函 数 名  :  alg_schedule_wmm_sch
功能描述  :  wmm调度入口函数
 ******************************************************************************/
WIFI_TCM_TEXT osal_u32 alg_schedule_wmm_sch(alg_schedule_stru *schedule, osal_u8 vap_id, osal_u8 ac_num,
    alg_sch_output_stru *sch_result)
{
    osal_u32 ret;
    struct osal_list_head *user_dlist_pos = OSAL_NULL;
    osal_u8 user_num, temp;
    alg_schedule_user_info_stru *user_info = NULL;

    user_num = schedule->user_num;
    /* step 0.0 : 判断bcast是否有调度需求　 */
    ret = alg_schedule_sch_bcast_txtid(schedule, vap_id, WLAN_TIDNO_BCAST, sch_result);
    if (osal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_SCHEDULE, "{alg_schedule_wmm_sch::result of alg_schedule_sch_bcast_txtid is [%d]}",
            ret);
        return ret;
    }
    if (sch_result->sch_flag == OSAL_TRUE) {
        return OAL_SUCC;
    }
    /* g_target_user_head指向目标用户的对应结构体 */
    oal_dlist_search_for_each_sch(user_dlist_pos, temp, user_num, g_target_user_head) {
        /* 环形双向链表，不遍历链表头 */
        if (user_dlist_pos == &g_user_dlist_head) {
            continue;
        }
        user_info = osal_list_entry(user_dlist_pos, alg_schedule_user_info_stru, user_dlist_entry);
        schedule->target_user_id = user_info->user_id;
        ret = alg_schedule_target_user(schedule, vap_id, ac_num, sch_result);
        if (osal_unlikely(ret != OAL_SUCC)) {
            return ret;
        }

        if (sch_result->sch_flag == OSAL_TRUE) {
            return OAL_SUCC;
        }
    }
    return OAL_SUCC;
}

/****************************************************************************
函 数 名  : alg_schedule_sch_bcast_txtid
功能描述  : 调度广播帧队列，采用严格优先级；
******************************************************************************/
WIFI_TCM_TEXT osal_u32 alg_schedule_sch_bcast_txtid(alg_schedule_stru *schedule, osal_u8 vap_id,
    wlan_tidno_enum_uint8 txtid_index, alg_sch_output_stru *sch_output)
{
    alg_schedule_txtid_info_stru *sch_txtid_info = OSAL_NULL;
    struct osal_list_head *dlist_pos = OSAL_NULL;
    struct osal_list_head *temp = OSAL_NULL;
    hmac_user_stru *hmac_user = OSAL_NULL;
    hmac_vap_stru *hmac_vap = OSAL_NULL;
    hmac_tid_stru *txtid = OSAL_NULL;
    struct osal_list_head *txtid_dlist_head = OSAL_NULL;
    osal_u32 ret;
    unref_param(vap_id);
    unref_param(txtid_index);

    txtid_dlist_head =  &(schedule->ac_txtid_dlist[WLAN_TIDNO_BCAST].txtid_dlist_head);
    osal_list_for_each_safe(dlist_pos, temp, txtid_dlist_head) {
        sch_txtid_info = osal_list_entry(dlist_pos, alg_schedule_txtid_info_stru, txtid_dlist_entry);

        /* 判断vap是否处于暂停状态 */
        hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(sch_txtid_info->vap_id);
        if (osal_unlikely(hmac_vap == OSAL_NULL)) {
            oam_error_log0(0, OAM_SF_SCHEDULE, "{alg_schedule_sch_bcast_txtid::mac_res_get_hmac_vap fail!}");
            return OAL_ERR_CODE_PTR_NULL;
        }

        /* VAP在非UP状态，不进行调度 */
        if (alg_schedule_is_vap_paused(hmac_vap)) {
            continue;
        }

        /* 获取user, tid结构体指针 */
        ret = alg_schedule_get_user_tid_by_info_stru(sch_txtid_info, &hmac_user, &txtid);
        if (osal_unlikely(ret != OAL_SUCC)) {
            return ret;
        }

        /* 如果广播用户队列非空，则调度之  */
        if (txtid->mpdu_num != 0) {
            sch_output->user = hmac_user;
            sch_output->txtid = txtid;
            sch_output->sch_flag = OSAL_TRUE;
            /* 将 tid 插入队尾 */
            osal_dlist_delete_entry(&(sch_txtid_info->txtid_dlist_entry));
            osal_list_add_tail(&(sch_txtid_info->txtid_dlist_entry), txtid_dlist_head);
            return OAL_SUCC;
        }
    }
    return OAL_SUCC;
}
#endif
/******************************************************************************
函 数 名  :  alg_schedule_round_robin_sch
功能描述  :  轮询调度算法入口
 ******************************************************************************/
WIFI_TCM_TEXT osal_u32 alg_schedule_round_robin_sch(alg_schedule_stru *schedule, osal_u8 vap_id, osal_u8 ac_num,
    alg_sch_output_stru *sch_result)
{
    osal_u32 ret;
    osal_s8 index;
    /* step 1.0: 调度该硬件队列对应的txtid */
    ret = alg_schedule_rr_sch(schedule, vap_id, (osal_s8)ac_num, sch_result);
    if (osal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log0(0, OAM_SF_SCHEDULE, "{alg_schedule_wmm_sch::alg_schedule_rr_sch failed}");
        return ret;
    }

    if (sch_result->sch_flag == OSAL_TRUE) {
        return OAL_SUCC;
    }

    /* step2.0: check所有AC是否有报文需要发送 */
    for (index = 0; index < WLAN_WME_AC_BUTT; index++) {
        ret = alg_schedule_rr_sch(schedule, vap_id, index, sch_result);
        if (osal_unlikely(ret != OAL_SUCC)) {
            oam_warning_log0(0, OAM_SF_SCHEDULE, "{alg_schedule_wmm_sch::alg_schedule_rr_sch failed}");
            return ret;
        }

        if (sch_result->sch_flag == OSAL_TRUE) {
            return OAL_SUCC;
        }
    }

    return OAL_SUCC;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif /* #ifdef _PRE_WLAN_FEATURE_SCHEDULE */
