/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: Interface implementation of online tx power calibration.
 * Create: 2022-10-15
 */
/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "fe_extern_if_host.h"
#include "power_ppa_ctrl_spec.h"
#include "cali_online.h"
#ifdef _PRE_WLAN_ONLINE_CALI
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif
#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_CALI_ONLINE

/*****************************************************************************
    全局变量定义
*****************************************************************************/
OSAL_STATIC osal_void online_cali_idle_state_entry(osal_void *ctx);
OSAL_STATIC osal_void online_cali_idle_state_exit(osal_void *ctx);
OSAL_STATIC osal_u32 online_cali_idle_event(osal_void *ctx, osal_u16 event,
    osal_u16 event_data_len, osal_void *event_data);
/* online校准全局状态机函数表 */
oal_fsm_state_info g_alg_online_tx_cali_fsm_info[] = {
    {
        ONLINE_CALI_STATE_INIT,
        "IDLE",
        online_cali_idle_state_entry,
        online_cali_idle_state_exit,
        online_cali_idle_event,
    },
    {
        ONLINE_CALI_STATE_DYN_PWR,
        "PWR",
        online_cali_dyn_pow_state_entry,
        online_cali_dyn_pow_state_exit,
        online_cali_dyn_pow_event,
    },
};

online_cali_stru g_ast_rf_online_cali = {0};

/*****************************************************************************
    函数定义
*****************************************************************************/
online_cali_stru *online_cali_get_alg_rf_online_cali_ctx(osal_void)
{
    return &g_ast_rf_online_cali;
}
/*
 * 功能描述  : online 校准状态切换
 * 1.日    期  : 2020年6月25日
 *   修改内容  : 新生成函数
 */
osal_u32 online_rf_cali_fsm_trans_to_state(online_cali_tx_fsm_info_stru *online_cali_fsm,
    osal_u8 state)
{
    oal_fsm_stru *oal_fsm = &(online_cali_fsm->st_oal_fsm);

    return oal_fsm_trans_to_state(oal_fsm, state);
}

/*
 * 功能描述  : online 校准状态机注册
 * 1.日    期  : 2020年6月25日
 *   修改内容  : 新生成函数
 */
OSAL_STATIC osal_void online_cali_fsm_attach(online_cali_stru *rf_cali)
{
    osal_u32            ret;
    osal_u8             fsm_name[6] = {0}; /* 6:数组大小 */
    oal_fsm_create_stru fsm_create_stru;
    online_cali_tx_fsm_info_stru *online_tx_cali_fsm = &(rf_cali->st_online_tx_cali_fsm);

    if (online_tx_cali_fsm->en_is_fsm_attached == OAL_TRUE) {
        oam_warning_log1(0, OAM_SF_CALIBRATE,
            "{online_cali_fsm_attach::hal device id [%d] fsm have attached.}",
            rf_cali->uc_device_id);
        return;
    }

    fsm_create_stru.oshandle = rf_cali;
    fsm_create_stru.name = fsm_name;
    fsm_create_stru.context = online_tx_cali_fsm;

    memset_s(online_tx_cali_fsm, sizeof(online_cali_tx_fsm_info_stru),
             0, sizeof(online_cali_tx_fsm_info_stru));

    ret = oal_fsm_create(&fsm_create_stru,
                         &(online_tx_cali_fsm->st_oal_fsm),
                         ONLINE_CALI_STATE_INIT,
                         g_alg_online_tx_cali_fsm_info,
                         sizeof(g_alg_online_tx_cali_fsm_info) / sizeof(oal_fsm_state_info));
    if (ret == OAL_SUCC) {
        online_tx_cali_fsm->en_is_fsm_attached = OAL_TRUE;
    }
}

/*
 * 功能描述   : online 校准事件处理入口
 * 1.日    期   : 2019年09月20日
 *   修改内容   : 新生成函数
 */
osal_u32 online_rf_cali_handle_event(online_cali_stru *rf_cali, osal_u16 type,
    osal_u16 event_data_len, osal_u8 *event_data)
{
    osal_u32 ret;
    online_cali_tx_fsm_info_stru *alg_online_cali_fsm;

    alg_online_cali_fsm = &(rf_cali->st_online_tx_cali_fsm);
    if (alg_online_cali_fsm->en_is_fsm_attached == OAL_FALSE) {
        oam_error_log1(0, OAM_SF_CALIBRATE,
            "{online_rf_cali_handle_event::device id = [%d] fsm not attached!}",
            rf_cali->uc_device_id);
        return OAL_FAIL;
    }

    ret = oal_fsm_event_dispatch(&(alg_online_cali_fsm->st_oal_fsm), type, event_data_len, event_data);
    if (ret != OAL_SUCC) {
        oam_error_log3(0, OAM_SF_CALIBRATE,
            "{online_rf_cali_handle_event::state [%d] dispatch event [%d] not succ [%d]!}",
            alg_online_cali_fsm->st_oal_fsm.cur_state, type, ret);
    }
    return ret;
}

/*
 * 函 数 名  : online_cali_init_entry_delay_timeout_handler
 * 功能描述  : 状态机延迟进入init tx notify
 * 1.日    期  : 2020年07月27日
 *   修改内容  : 新生成函数
 */
OSAL_STATIC osal_u32 online_cali_init_entry_delay_timeout_handler(osal_void *prg)
{
    online_cali_stru *rf_cali = online_cali_get_alg_rf_online_cali_ctx();
    unref_param(prg);

    online_cali_trans_to_next_state(rf_cali);
    return OAL_SUCC;
}

/*
 * 功能描述   : online校准进入idle状态时配置
 * 1.日    期   : 2019年09月20日
 *   修改内容   : 新生成函数
 */
OSAL_STATIC osal_void online_cali_idle_state_entry(osal_void *ctx)
{
    hal_device_stru *device = (hal_device_stru *)hal_chip_get_hal_device();
    online_cali_stru *rf_cali = NULL;
    unref_param(ctx);

    rf_cali = online_cali_get_alg_rf_online_cali_ctx();
    /* 初始化时, 快速进入; 由其它状态转移进来时, 延迟进入 */
    if (rf_cali->st_online_tx_cali_fsm.en_is_fsm_attached == 0) {
#ifdef _PRE_WLAN_RF_CALI_DEBUG
        oam_info_log0(0, OAM_SF_CALIBRATE, "init entry fast\n");
#endif
        online_cali_trans_to_next_state(rf_cali);
    } else {
#ifdef _PRE_WLAN_RF_CALI_DEBUG
        oam_info_log0(0, OAM_SF_CALIBRATE, "init entry delay\n");
#endif
        // 状态机切换到下一个状态(功率)的延迟时间
        frw_create_timer_entry(&(rf_cali->dyn_cali_init_entry_delay_timer),
            online_cali_init_entry_delay_timeout_handler, ONLINE_CALI_INIT_ENTRY_DELAY_TIMER_PERIOD,
            device, OAL_FALSE);
    }
    return;
}

/*
 * 功能描述   : online校准退出idle状态时配置
 * 1.日    期   : 2019年09月20日
 *   修改内容   : 新生成函数
 */
OSAL_STATIC osal_void online_cali_idle_state_exit(osal_void *ctx)
{
    unref_param(ctx);
    return;
}

/*
 * 函 数 名   : online_cali_trans_to_next_state
 * 功能描述   : 状态转移到下一状态
 * 1.日    期   : 2020年07月17日
 *   修改内容   : 新生成函数
 */
osal_u32 online_cali_trans_to_next_state(online_cali_stru *rf_cali)
{
    osal_u8 cur_state = rf_cali->st_online_tx_cali_fsm.st_oal_fsm.cur_state;
    osal_u8 next_state = cur_state + 1;
    next_state = (next_state < ONLINE_CALI_STATE_BUTT) ? next_state : ONLINE_CALI_STATE_INIT;
#ifdef _PRE_WLAN_RF_CALI_DEBUG
    oam_info_log2(0, OAM_SF_CALIBRATE, "{next notify::cur state[%d]next state[%d]}\n", cur_state, next_state);
#endif
    if (online_rf_cali_fsm_trans_to_state(&(rf_cali->st_online_tx_cali_fsm), next_state) == OAL_FAIL) {
#ifdef _PRE_WLAN_RF_CALI_DEBUG
        oam_error_log2(0, OAM_SF_CALIBRATE, "{next nofity::cur state[%d] to next[%d] failed}\n",
            rf_cali->st_online_tx_cali_fsm.st_oal_fsm.cur_state, next_state);
#endif
        return OAL_FAIL;
    }

    return OAL_TRUE;
}

/*
 * 功能描述   : online校准idle状态事件处理入口
 * 1.日    期   : 2019年09月20日
 *   修改内容   : 新生成函数
 */
OSAL_STATIC osal_u32 online_cali_idle_event(osal_void *ctx, osal_u16 event,
    osal_u16 event_data_len, osal_void *event_data)
{
    switch (event) {
        case ONLINE_CALI_EVENT_TX_NOTIFY:
            break;
        case ONLINE_CALI_EVENT_TX_COMPLETE:
            break;

        case ONLINE_CALI_EVENT_CHANNEL_CHANGE:
        case ONLINE_CALI_EVENT_BW_CHANGE:
        case ONLINE_CALI_EVENT_EQUIPE_CHANGE:
            break;

        default:
            oam_error_log1(0, OAM_SF_CALIBRATE, "{online_cali_idle_event::event[%d] INVALID!}", event);
            break;
    }
    unref_param(ctx);
    unref_param(event_data_len);
    unref_param(event_data);

    return OAL_SUCC;
}

/*
 * 功能描述  : 设置状态机事件的ID号
 *  1.日    期   : 2020年08月18日
 *    修改内容   : 新生成函数
 */
osal_void online_set_cali_id(online_cali_stru *alg_rf_cali, osal_u8 chip_id, osal_u8 dev_id)
{
    alg_rf_cali->uc_chip_id = chip_id;
    alg_rf_cali->uc_device_id = dev_id;
}

/*
 * 功能描述   : 发送完成过程online校准入口函数
 * 1.日    期   : 2018年7月20日
 *   修改内容   : 新生成函数
 */
osal_s32 online_cali_tx_complete_dyn_process(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    online_cali_para_stru *online_cali = (online_cali_para_stru *)msg->data;
    hal_device_stru *hal_device = (hal_device_stru *)hal_chip_get_hal_device();
    online_cali_stru *rf_cali = online_cali_get_alg_rf_online_cali_ctx();

    if (hal_device == NULL) {
        oam_error_log0(0, OAM_SF_CALIBRATE, "{online_cali_tx_complete_dyn_process: hal_dev is NULL}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 非常发，扫描的时候不处理tx_complete */
    if (online_cali->al_tx_flag == 0) {
        if (hal_device->hal_device_base.hal_scan_params.curr_scan_state != MAC_SCAN_STATE_IDLE) {
            oam_info_log0(0, OAM_SF_CALIBRATE,
                "{online_cali_tx_complete_dyn_process: en_curr_scan_state is running}");
            return OAL_FAIL;
        }
    }

    online_set_cali_id(rf_cali, 0, rf_cali->uc_device_id);
    /* online 校准帧发送完成处理入口 */
    if (online_rf_cali_handle_event(rf_cali, ONLINE_CALI_EVENT_TX_COMPLETE,
        sizeof(online_cali_para_stru), (osal_u8*)online_cali) == OAL_FAIL) {
#ifdef _PRE_WLAN_RF_CALI_DEBUG
        oam_error_log0(0, OAM_SF_CALIBRATE,
            "\t{online_cali_tx_complete_dyn_process::online_rf_cali_handle_event fail.}\n");
#endif
        return OAL_FAIL;
    }
    unref_param(hmac_vap);
    return OAL_SUCC;
}

/*
* 功能描述   : 动态校准定制化参数初始化
* 1.日    期   : 2020年08月01日
*   修改内容   : 新生成函数
*/
OAL_STATIC osal_void online_cali_tx_pow_init_param(hal_to_dmac_device_stru *device)
{
    online_cali_stru *rf_cali = online_cali_get_alg_rf_online_cali_ctx();
    online_cali_dyn_stru *dyn_cali = &rf_cali->dyn_cali_val;
    online_cali_pow_ctrl_stru *txpow_ctrl = online_cali_get_txpow_ctrl(0x0);
    online_cali_tx_fsm_info_stru *online_tx_cali_fsm = &(rf_cali->st_online_tx_cali_fsm);

    if (online_tx_cali_fsm->en_is_fsm_attached == OAL_TRUE) {
        oam_warning_log1(0, OAM_SF_CALIBRATE,
            "{online_cali_tx_pow_init_param::hal device id [%d] fsm have attached.}", rf_cali->uc_device_id);
        return;
    }
    /* 定时器已经初始化过，不能二次初始化 */
    if (dyn_cali->dyn_cali_per_frame_timer.func != NULL) {
        oam_warning_log0(0, OAM_SF_CALIBRATE,
            "{online_cali_tx_pow_init_param::timer func have initialized.}");
        return;
    }

    memset_s(dyn_cali, sizeof(online_cali_dyn_stru), 0, sizeof(online_cali_dyn_stru));
    online_cali_pow_cali_set_dscr_intvl(dyn_cali, WLAN_BAND_2G, online_cali_get_cus_dyn_cali_intvl(WLAN_BAND_2G));

    dyn_cali->dyn_cali_val_min_th = ONLINE_CALI_PWR_MIN_TH;
    dyn_cali->dyn_cali_val_max_th = ONLINE_CALI_PWR_MAX_TH;
    dyn_cali->dyn_cali_adjst = OAL_TRUE;
    dyn_cali->com_upc_switch = OAL_TRUE;
    dyn_cali->en_temp_change_flag = OAL_FALSE;
    dyn_cali->cali_channel_index = -1; /* 初始状态 */

    txpow_ctrl->base_upc_idx = CALI_POW_BASE_2G_UPC_IDX;
    txpow_ctrl->target_gain_num = CALI_POW_PPA_LUT_NUM;
    txpow_ctrl->target_gain_num_online = CALI_POW_PPA_LUT_NUM;
    txpow_ctrl->ppa_lut = cali_get_ppa_lut();
    txpow_ctrl->upc_idx2code_online = cali_get_ppa_lut();
#ifdef _PRE_WLAN_RF_CALI_DEBUG
    oam_info_log0(0, OAM_SF_CALIBRATE, "{online_cali_tx_pow_init_param!}\n");
#endif
}

/*
 * 功能描述   : online 校准参数初始化
 * 1.日    期   : 2019年07月30日
 *   修改内容   : 新生成函数
 */
OAL_STATIC osal_void online_cali_param_init(hal_to_dmac_device_stru *hal_device)
{
    online_cali_tx_pow_init_param(hal_device);
}

/*
 * 功能描述   : 动态校准初始化
 */
osal_void online_cali_init(hal_to_dmac_device_stru *hal_device)
{
    online_cali_stru *rf_cali = NULL;
    rf_cali = online_cali_get_alg_rf_online_cali_ctx();
    online_cali_param_init(hal_device);
    online_cali_fsm_attach(rf_cali);
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif  // _PRE_WLAN_ONLINE_CALI