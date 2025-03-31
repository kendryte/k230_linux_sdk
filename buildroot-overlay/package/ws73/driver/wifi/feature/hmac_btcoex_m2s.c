/*
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 * Description: M2S处理
 * Date: 2023-01-31 10:14
 */
#ifndef __HMAC_BTCOEX_M2S_C__
#define __HMAC_BTCOEX_M2S_C__

#include "hmac_mgmt_sta.h"
#include "hmac_feature_interface.h"
#include "hmac_scan.h"
#include "hmac_btcoex.h"
#include "hmac_btcoex_ps.h"
#include "hmac_btcoex_m2s.h"

#ifdef __cplusplus
#if __cplusplus
    extern "C" {
#endif
#endif
#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_HMAC_BTCOEX_M2S_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

#ifdef _PRE_WLAN_FEATURE_M2S
/*****************************************************************************
 函 数 名  : hmac_m2s_btcoex_need_switch_check
 功能描述  : 其他业务结束之后，在回到mimo时，是否需要继续切到siso，保证btcoex业务性能
*****************************************************************************/
oal_bool_enum_uint8 hmac_m2s_btcoex_need_switch_check(const hal_to_dmac_device_stru *hal_device, osal_u8 *m2s_mode_mask)
{
    hal_chip_stru *hal_chip = OSAL_NULL;
    hal_btcoex_btble_status_stru *btble_status = OSAL_NULL;
    hmac_vap_stru *mac_vap_temp = OSAL_NULL;
    osal_u8 vap_index;
    osal_u8 up_vap_num;
    osal_u8 mac_vap_id[WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE] = {0};
    oal_bool_enum_uint8 m2s_wifi_need = OSAL_FALSE; /* btcoex业务满足切换需求 */
    oal_bool_enum_uint8 m2s_bt_need = OSAL_FALSE;   /* wifi业务满足切换需求 */

    hal_chip = hal_get_chip_stru();
    btble_status = hal_btcoex_btble_status();

    /* 1.需要考虑此时是不是只有5g，是的话，不需要切 */
    up_vap_num = hal_device_find_all_up_vap(hal_device, mac_vap_id, WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE);
    for (vap_index = 0; vap_index < up_vap_num; vap_index++) {
        mac_vap_temp = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap_id[vap_index]);
        if (mac_vap_temp == OSAL_NULL) {
            oam_error_log1(0, OAM_SF_COEX,
                "vap_id[%d] hmac_m2s_btcoex_need_switch_check::hmac_vap IS NULL.", mac_vap_id[vap_index]);
            continue;
        }

        /* 存在2G的vap设备，dbac也需要考虑 */
        if (mac_btcoex_check_valid_vap(mac_vap_temp) == OSAL_TRUE) {
            m2s_wifi_need = OSAL_TRUE;
        }
    }

    /* 2.6slot,注意业务不存在时清一下业务标记，很可能是dbdc期间m2s业务置位了，但是中途业务结束，dbdc结束也要同步清bt业务标记
     */
    if (hal_device->device_btcoex_mgr.m2s_6slot == OSAL_TRUE) {
        if (btble_status->bt_status.bt_status.bt_6slot == 2) { /* 6slot为2时，m2s_bt_need才为true */
            *m2s_mode_mask = BT_M2S_6SLOT_MASK;
            m2s_bt_need = OSAL_TRUE;
        }
    }

    /* 3.ldac, 两个标志不会同时存在 */
    if (hal_device->device_btcoex_mgr.m2s_ldac == OSAL_TRUE) {
        if (btble_status->bt_status.bt_status.bt_ldac == 1) {
            *m2s_mode_mask = BT_M2S_LDAC_MASK;
            m2s_bt_need = OSAL_TRUE;
        }
    }

    /* 4.wifi存在2g，并且bt存在切换业务，才允许做切换 */
    if (m2s_wifi_need == OSAL_TRUE && m2s_bt_need == OSAL_TRUE) {
        return OSAL_TRUE;
    }

    return OSAL_FALSE;
}

/*****************************************************************************
 函 数 名  : hmac_btcoex_s2m_allow_check
 功能描述  : (1)当前处于bt切siso状态
             (2)当前hal device上是否还有2g up vap，没有的话，需要回到mimo
             (3)如果关联失败去关联，需要在去关联接口重新判断当前bt业务状态是否再切siso
*****************************************************************************/
osal_void hmac_btcoex_s2m_allow_check(hal_to_dmac_device_stru *hal_device)
{
    osal_u8 vap_idx;
    osal_u8 up_vap_num;
    oal_bool_enum_uint8 s2m_need = OSAL_TRUE;
    osal_u8 mac_vap_id[WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE] = {0};
    hmac_vap_stru *hmac_vap = OSAL_NULL;

    /* 整个逻辑在dbdc和btcoex切siso on判断之后，此时肯定不是dbdc场景;没有切换到c1 siso，也不存在回mimo */
    /* 1.如果不支持切换直接返回; 注意GET_HAL_DEVICE_BTCOEX_SISO_AP_EXCUTE_ON(hal_device)默认这两个要打开 */
    if (hal_device->device_btcoex_mgr.m2s_6slot == OSAL_FALSE &&
        hal_device->device_btcoex_mgr.m2s_ldac == OSAL_FALSE) {
        return;
    }

    /* 找到所有up的vap设备 */
    up_vap_num = hal_device_find_all_up_vap(hal_device, mac_vap_id, WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE);
    for (vap_idx = 0; vap_idx < up_vap_num; vap_idx++) {
        hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap_id[vap_idx]);
        if (hmac_vap == OSAL_NULL) {
            oam_error_log1(0, OAM_SF_COEX,
                "vap_id[%d] {hmac_btcoex_s2m_allow_check::mac_temp_vap IS NULL.}", mac_vap_id[vap_idx]);
            return;
        }

        /* 存在2G的vap设备，dbac也需要考虑，此时需要保持在c1 siso */
        if (mac_btcoex_check_valid_vap(hmac_vap) == OSAL_TRUE) {
            s2m_need = OSAL_FALSE;
        }
    }

    /* 2.判断是否需要回mimo，需要的话从siso执行 */
    if (s2m_need == OSAL_TRUE) {
        oam_warning_log1(0, OAM_SF_COEX,
            "{hmac_btcoex_s2m_allow_check::up_vap_num[%d]resume to mimo!}", up_vap_num);

        hmac_m2s_handle_event(hal_device, HAL_M2S_EVENT_BT_SISO_TO_MIMO, 0, OSAL_NULL);

        /* 回到mimo的话，需要恢复ps机制 */
        hmac_btcoex_ps_stop_check_and_notify(hal_device);

        /* 此时可能电话 ldac业务已经结束，3s定时器已经创建，需要清定时器 */
        if (hal_device->device_btcoex_mgr.s2m_resume_timer.is_registerd == OSAL_TRUE) {
            frw_destroy_timer_entry(&(hal_device->device_btcoex_mgr.s2m_resume_timer));
        }

        /* 如果是ap siso模式切换，此时需要清标志，重关联时候再切换 */
        hal_device->device_btcoex_mgr.siso_ap_excute_on = OSAL_FALSE;
        frw_destroy_timer_entry(&(hal_device->device_btcoex_mgr.bt_coex_s2m_siso_ap_timer));

        /* 此时可能多个业务同时结束，定时器清除，造成可能某个业务m2s标记一直在，切回mimo，需要清所有业务标记 */
        get_hal_device_btcoex_m2s_mode_bitmap(hal_device) = 0;

        get_hal_device_btcoex_s2m_wait_bitmap(hal_device) = 0;
    }
}

/*****************************************************************************
 函 数 名  : hmac_btcoex_s2m_resume_callback
 功能描述  : 申请回mimo定时器超时处理函数，判断是否当前业务都结束才申请回mimo，否则继续保持siso不处理
*****************************************************************************/
osal_u32 hmac_btcoex_s2m_resume_callback(osal_void *arg)
{
    hal_to_dmac_device_stru *hal_device = (hal_to_dmac_device_stru *)arg;

    /* 1.统计当前m2s业务状态，各业务下半部根据业务=0/1 完成m2s mode的刷新，此处直接用 */
    get_hal_device_btcoex_m2s_mode_bitmap(hal_device) &= (~(get_hal_device_btcoex_s2m_wait_bitmap(hal_device)));

    /* 临时保持结束，状态清零 */
    get_hal_device_btcoex_s2m_wait_bitmap(hal_device) = 0;

    /* 2.如果bitmap为空，说明不存在业务需要保持在siso，可以申请切换回mimo */
    if (get_hal_device_btcoex_s2m_mode_bitmap(hal_device) == 0) {
        /* 如果是ap模式需要切换 */
        if (hmac_m2s_custom_switch_check(hal_device) == OSAL_TRUE) {
            /* 清btcoex mode业务状态 */
            get_hal_m2s_mode_tpye(hal_device) &= (~WLAN_M2S_TRIGGER_MODE_BTCOEX);

            hmac_m2s_handle_event(hal_device, HAL_M2S_EVENT_CUSTOM_SISO_C1_TO_SISO_C0, 0, OSAL_NULL);
        } else {
            hmac_m2s_handle_event(hal_device, HAL_M2S_EVENT_BT_SISO_TO_MIMO, 0, OSAL_NULL);
        }

        /* 回到mimo/c0 siso的话，需要恢复ps机制 */
        hmac_btcoex_ps_stop_check_and_notify(hal_device);

        oam_warning_log2(0, OAM_SF_COEX,
            "{hmac_btcoex_s2m_resume_callback::succ to mimo, m2s_bitmap[%d] m2s_mode[%d]!}",
            get_hal_device_btcoex_m2s_mode_bitmap(hal_device), get_hal_m2s_mode_tpye(hal_device));

        /* 此时可能多个业务同时结束，定时器清除，造成可能某个业务m2s标记一直在，切回mimo，需要清所有业务标记 */
        get_hal_device_btcoex_m2s_mode_bitmap(hal_device) = 0;
    } else {
        /* 3. 需要sco = 0  a2dp = 0  ldac = 0  都做切回mimo申请，才能接口做得简单 */
        oam_warning_log2(0, OAM_SF_COEX,
            "{hmac_btcoex_s2m_resume_callback::cannot to mimo, need all is ready s2m_bitmap[%d]m2s_bitmap[%d]!}",
            get_hal_device_btcoex_s2m_mode_bitmap(hal_device), get_hal_device_btcoex_m2s_mode_bitmap(hal_device));
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_btcoex_m2s_allow_check
 功能描述  : 当前业务场景，是否执行m2s切换
             (1)方案上考虑2g vap
             (2)sta和ap处于不同的接口时
*****************************************************************************/
osal_void hmac_btcoex_m2s_allow_check(hal_to_dmac_device_stru *hal_device, const hmac_vap_stru *hmac_vap)
{
    hal_chip_stru *hal_chip = hal_get_chip_stru();
    bt_status_stru *bt_status = OSAL_NULL;
    oal_bool_enum_uint8 m2s_need = OSAL_FALSE;
    hal_btcoex_btble_status_stru *status = OSAL_NULL;

    if (hal_device == OSAL_NULL) {
        return;
    }

    /* 2.如果不支持切换直接返回 */
    if (hal_device->device_btcoex_mgr.m2s_6slot == OSAL_FALSE &&
        hal_device->device_btcoex_mgr.m2s_ldac == OSAL_FALSE) {
        return;
    }

    /* 3. sta已经关联上，尽量不要在siso时候关联此ap，可能有风险，ap在黑名单需要保持在miso，不能切换 */
    if (get_hal_device_m2s_del_swi_miso_hold(hal_device) == OSAL_TRUE) {
        oam_warning_log0(0, OAM_SF_COEX,
            "{hmac_btcoex_m2s_allow_check::MISO AP not support change to siso!}");
        return;
    }

    /* 4.已经是c1 siso的话，新入网的设备直接入网就是 */
    if (hal_device->hal_m2s_fsm.oal_fsm.cur_state == HAL_M2S_STATE_SISO &&
        hal_device->cfg_cap_info->phy_chain == WLAN_PHY_CHAIN_ONE) {
        oam_warning_log0(0, OAM_SF_COEX, "{hmac_btcoex_m2s_allow_check::already c1 siso!}");
        return;
    }

    /* 4.1 后续所有切siso业务都要判断，当前spec是不是支持，支持的话可以切换，不支持的话，不能切换;
      主要是2G不支持c1 siso的话 */
    if (hal_get_chip_stru()->rf_custom_mgr.support_rf_chain_2g == WLAN_RF_CHAIN_ZERO) {
        oam_warning_log1(0, OAM_SF_COEX,
            "{hmac_btcoex_m2s_allow_check::spec 2G chain[%d] restrain, not support c1 SISO.}",
            hal_get_chip_stru()->rf_custom_mgr.support_rf_chain_2g);
        return;
    }

    /* 5.当前hal device上另一个vap设备状态无所谓，只要当前入网vap是2G 就考虑切siso */
    /* 当前vap一定是在入网和ap up，find 找不到，此时直接判断入网的vap在2g的话，也需要申请切siso */
    if (hmac_vap->channel.band == WLAN_BAND_2G) {
        m2s_need = OSAL_TRUE;
    }
    status = hal_btcoex_btble_status();
    hal_update_btcoex_btble_status(status);
    bt_status = &(status->bt_status.bt_status);

    /* 5.存在2g vap，开始判断是否存在切siso的蓝牙业务 */
    if (m2s_need == OSAL_TRUE) {
        if (bt_status->bt_6slot == 2 && hal_device->device_btcoex_mgr.m2s_6slot == OSAL_TRUE) { /* 六时隙若为2 */
            hmac_btcoex_m2s_switch_apply(hal_device, BT_M2S_6SLOT_MASK);
        } else if (bt_status->bt_ldac == 1 && hal_device->device_btcoex_mgr.m2s_ldac == OSAL_TRUE) {
            hmac_btcoex_m2s_switch_apply(hal_device, BT_M2S_LDAC_MASK);
        }
    }
    oam_warning_log3(0, OAM_SF_COEX, "{hmac_btcoex_m2s_allow_check::m2s_need[%d]m2s_state[%d]m2s_mode[%d]!}",
        m2s_need, hal_device->hal_m2s_fsm.oal_fsm.cur_state, get_hal_m2s_mode_tpye(hal_device));
    oam_warning_log4(0, OAM_SF_COEX, "{hmac_btcoex_m2s_allow_check::6slot[%d]ldac[%d]vap_band[%d]vap_mode[%d]!}",
        bt_status->bt_6slot, bt_status->bt_ldac, hmac_vap->channel.band, hmac_vap->vap_mode);
}

osal_u32 hmac_btcoex_m2s_switch_other(const hal_to_dmac_device_stru *hal_device,
    hal_m2s_event_tpye_uint16 *m2s_event)
{
    /* 3.有其他业务已经申请切换，不包括自身模块，不支持切换，这里要变厚 */
    if ((get_hal_m2s_mode_tpye(hal_device) & (~WLAN_M2S_TRIGGER_MODE_BTCOEX)) != 0) {
        /* 2.0 如果此时是spec切换到了siso 蓝牙业务期望切换到siso的话，就保持不变；等spec退出时判断要不要切c1
         * siso;暂时不采用共存mode方式 */
        if (hal_m2s_check_spec_on(hal_device) == OSAL_TRUE) {
        } else if (hal_m2s_check_fast_scan_on(hal_device) == OSAL_TRUE) {
            /* 2.1 按照优先级，如果DBDC在运行，在最外层已经判断主路没有2g而返回，dbdc退出时候要单独判断bt业务状态 */
            /* 3.2 虽然scan abort了，但是此处会有并发扫描伴随dbdc，扫描结束不会回mimo，因为dbdc在，并发扫描标记还在 */
            *m2s_event = HAL_M2S_EVENT_BT_SISO_C0_TO_SISO_C1;

            get_hal_m2s_mode_tpye(hal_device) &= (~WLAN_M2S_TRIGGER_MODE_FAST_SCAN);
        } else if (hal_m2s_check_command_on(hal_device) == OSAL_TRUE) {
            /* 3.3 如果只有低优先级的MSS,默认是c0 siso，直接清mss标记，然后切c1 siso即可 */
            /* 如果处于miso探测态，需要停止切换保护，并切c1 siso */
            if (hal_device->hal_m2s_fsm.oal_fsm.cur_state == HAL_M2S_STATE_MISO) {
                *m2s_event = HAL_M2S_EVENT_BT_MISO_TO_SISO_C1;
            } else {
                *m2s_event = HAL_M2S_EVENT_BT_SISO_C0_TO_SISO_C1;
            }

            /* 切换保护功能关闭 */
            get_hal_device_m2s_switch_prot(hal_device) = OSAL_FALSE;

            /* 清业务标志 */
            get_hal_m2s_mode_tpye(hal_device) &= (~WLAN_M2S_TRIGGER_MODE_COMMAND);
        } else if (hal_m2s_check_rssi_on(hal_device) == OSAL_TRUE) {
            /* 3.4 如果只有低优先级的rssi,要判断是c0 miso 还是c1 miso */
            *m2s_event = HAL_M2S_EVENT_BT_MISO_TO_SISO_C1;

            /* 清业务标志 */
            get_hal_m2s_mode_tpye(hal_device) &= (~WLAN_M2S_TRIGGER_MODE_RSSI);
        } else if (hal_m2s_check_custom_on(hal_device) == OSAL_TRUE) {
            /* 3.5 如果custom, 需要切换到c1 siso，而且需要用蓝牙状态，否则后续无法回去 */
            *m2s_event = HAL_M2S_EVENT_BT_SISO_C0_TO_SISO_C1;

            /* 清业务标志，btcoex和custom相互打断，会相互恢复  刚打开热点，蓝牙中断就更新好了 */
            get_hal_m2s_mode_tpye(hal_device) &= (~WLAN_M2S_TRIGGER_MODE_CUSTOM);
        } else {
            oam_error_log1(0, OAM_SF_COEX,
                "{hmac_btcoex_m2s_switch_other::cannot to siso, mode[%d]!}", get_hal_m2s_mode_tpye(hal_device));
            return OAL_FAIL;
        }
    } else {
        /* 4.如果有业务mode bitmap已经是1，说明已经在申请siso过程中,保持siso不处理即可 */
        if (get_hal_device_btcoex_m2s_mode_bitmap(hal_device) != 0) {
            oam_warning_log1(0, OAM_SF_COEX,
                "{hmac_btcoex_m2s_switch_other::keep siso. m2s_bitmap[%d]!}",
                get_hal_device_btcoex_m2s_mode_bitmap(hal_device));
        } else { /*  */
            if (hal_device->hal_m2s_fsm.oal_fsm.cur_state == HAL_M2S_STATE_MIMO) {
                *m2s_event = HAL_M2S_EVENT_BT_MIMO_TO_SISO_C1;
            } else if (hal_device->hal_m2s_fsm.oal_fsm.cur_state == HAL_M2S_STATE_SISO) {
                *m2s_event = HAL_M2S_EVENT_BT_SISO_C0_TO_SISO_C1;
            } else if (hal_device->hal_m2s_fsm.oal_fsm.cur_state == HAL_M2S_STATE_IDLE &&
                hal_device->cfg_cap_info->phy2dscr_chain == WLAN_PHY_CHAIN_DOUBLE &&
                hal_device->cfg_cap_info->phy_chain == WLAN_PHY_CHAIN_DOUBLE) {
                /* 这里希望做到c1 siso入网,默认idle时候要是mimo,不是的话 */
                *m2s_event = HAL_M2S_EVENT_BT_MIMO_TO_SISO_C1;
            } else {
                oam_error_log3(0, OAM_SF_COEX,
                    "{hmac_btcoex_m2s_switch_other::error. m2s_state[%d]phy_chain[%d]phy2dscr_chain[%d]!}",
                    hal_device->hal_m2s_fsm.oal_fsm.cur_state, hal_device->cfg_cap_info->phy_chain,
                    hal_device->cfg_cap_info->phy2dscr_chain);
            }
        }
    }
    return OAL_CONTINUE;
}

/*****************************************************************************
 函 数 名  : hmac_btcoex_m2s_switch_apply
 功能描述  : btcoex下特性业务申请切到siso
             (1)业务进入，申请切siso
             (2)业务退出，申请切mimo，ldac和sco同时在，一个退出，也保持siso
             (3)这里需要对业务做集中管理，比如ldac来的太频繁，要保护到切换完成
             (4)保护标志可以在m2s完成里面清
             (5)要保证中断是1和0  否则可能出现先切mimo的操作，不过应该没关系
*****************************************************************************/
osal_u32 hmac_btcoex_m2s_switch_apply(hal_to_dmac_device_stru *hal_device, osal_u8 m2s_mode_mask)
{
    hal_m2s_event_tpye_uint16 m2s_event = HAL_M2S_EVENT_BUTT;
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);
    osal_u32 ret;

    /* 0.存在2G设备时候才切siso，最外层已经判断，这里肯定是存在,另外防止mss
     * 回miso扫描，bt申请切siso时先abort当次扫描，避免冲突 */
    hmac_scan_abort(hmac_device);

    oam_warning_log2(0, OAM_SF_COEX, "{hmac_btcoex_m2s_switch_apply:: m2s_stru[%d]m2s_state[%d]!}",
        get_hal_m2s_mode_tpye(hal_device), hal_device->hal_m2s_fsm.oal_fsm.cur_state);

    /* 1. ap在黑名单需要保持在miso，不能切换 */
    if (get_hal_device_m2s_del_swi_miso_hold(hal_device) == OSAL_TRUE) {
        oam_warning_log0(0, OAM_SF_COEX,
            "{hmac_btcoex_m2s_switch_apply:: AP not support change to siso!}");
        return OAL_FAIL;
    }

    /* 2. 异频dbac场景，2g期望工作在c1 siso，5g时工作在c0 siso，之间切换，此时要支持切换，2G
       默认支持mimo，此时不处理往下走， 暂时该情况就不支持 */
    if (hmac_m2s_spec_support_siso_switch_check(hal_device, hmac_device, WLAN_M2S_TRIGGER_MODE_BTCOEX) == OSAL_FALSE) {
        oam_warning_log0(0, OAM_SF_COEX,
            "{hmac_btcoex_m2s_switch_apply::2g5g dbac not support change to siso!}");
        return OAL_FAIL;
    }

    ret = hmac_btcoex_m2s_switch_other(hal_device, &m2s_event);
    if (ret != OAL_CONTINUE) {
        return ret;
    }

    if (m2s_event != HAL_M2S_EVENT_BUTT) {
        oam_warning_log2(0, OAM_SF_COEX,
            "{hmac_btcoex_m2s_switch_apply:: excute c1 siso, m2s_event[%d]m2s_state[%d]!}", m2s_event,
            hal_device->hal_m2s_fsm.oal_fsm.cur_state);

        hmac_m2s_handle_event(hal_device, m2s_event, 0, OSAL_NULL);

        hmac_btcoex_ps_stop_check_and_notify(hal_device);
    }

    /* 5.置m2s bitmap标记 */
    get_hal_device_btcoex_m2s_mode_bitmap(hal_device) |= m2s_mode_mask;

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_btcoex_s2m_switch_apply
 功能描述  : btcoex下特性业务申请切回mimo
             (1)业务进入，申请切siso
             (2)业务退出，申请切mimo，ldac和sco同时在，一个退出，也保持siso
             (3)这里需要对业务做集中管理，比如ldac来的太频繁，要保护到切换完成
             (4)保护标志可以在m2s完成里面清
             (5)要保证中断是1和0  否则可能出现先切mimo的操作，不过应该没关系
*****************************************************************************/
osal_u32 hmac_btcoex_s2m_switch_apply(hal_to_dmac_device_stru *hal_device, osal_u8 m2s_mode_mask)
{
    /* 申请回mimo，需要考虑复杂度，一方面要等待3s，另一方面需要判断当前所有a2dp和6slot标记都是0时，才回mimo */
    hal_device_btcoex_mgr_stru *device_btcoex_mgr;

    device_btcoex_mgr = &(hal_device->device_btcoex_mgr);

    /* 0.可能此时已经是mimo状态，比如wifi切换到5g，此时直接返回即可，各业务清自己的s2m业务状态即可 */
    /* 1.要申请回mimo，wait业务标记 bitmap */
    get_hal_device_btcoex_s2m_wait_bitmap(hal_device) |= m2s_mode_mask;

    /* 2.启动定时器 */
    if (device_btcoex_mgr->s2m_resume_timer.is_registerd == OSAL_TRUE) {
        frw_destroy_timer_entry(&(device_btcoex_mgr->s2m_resume_timer));
    }
    frw_create_timer_entry(&(device_btcoex_mgr->s2m_resume_timer), hmac_btcoex_s2m_resume_callback,
        BTCOEX_S2M_RESUME_TIMEOUT, (osal_void *)hal_device, OSAL_FALSE);

    oam_warning_log3(0, OAM_SF_COEX,
        "{hmac_btcoex_s2m_switch_apply::m2s_mode_bitmap[%d]s2m_mode_bitmap[%d]wait_bitmap[%d]!}",
        get_hal_device_btcoex_m2s_mode_bitmap(hal_device), get_hal_device_btcoex_s2m_mode_bitmap(hal_device),
        get_hal_device_btcoex_s2m_wait_bitmap(hal_device));

    return OAL_SUCC;
}
#endif

/*****************************************************************************
 函 数 名  : hmac_btcoex_s2m_siso_ap_resume_callback
 功能描述  : siso ap申请回mimo定时器超时处理函数，只是为了音乐等切歌，或者电话等切换太快，尽量业务保持
*****************************************************************************/
osal_u32 hmac_btcoex_s2m_siso_ap_resume_callback(osal_void *arg)
{
    hal_btcoex_btble_status_stru *btble_status = OSAL_NULL;
    hal_to_dmac_device_stru *hal_device = (hal_to_dmac_device_stru *)arg;

    btble_status = hal_btcoex_btble_status();
    /* 中断上半部会刷新状态值，状态值还是1就不处理，等下一个上半部0再次触发 */
    if (btble_status->bt_status.bt_status.bt_ba == 0) {
#ifdef _PRE_WLAN_FEATURE_M2S
        if (hmac_m2s_custom_switch_check(hal_device) == OSAL_TRUE) {
            /* 清btcoex mode业务状态 */
            get_hal_m2s_mode_tpye(hal_device) &= (~WLAN_M2S_TRIGGER_MODE_BTCOEX);

            hmac_m2s_handle_event(hal_device, HAL_M2S_EVENT_CUSTOM_SISO_C1_TO_SISO_C0, 0, OSAL_NULL);
        } else {
            hmac_m2s_handle_event(hal_device, HAL_M2S_EVENT_BT_SISO_TO_MIMO, 0, OSAL_NULL);
        }
#endif
        /* 回到mimo/c0 siso的话，需要恢复ps机制 */
        hmac_btcoex_ps_stop_check_and_notify(hal_device);

        hal_device->device_btcoex_mgr.siso_ap_excute_on = OSAL_FALSE;

        oam_warning_log2(0, OAM_SF_COEX,
            "{hmac_btcoex_s2m_siso_ap_resume_callback::succ to mimo, m2s_bitmap[%d] m2s_mode[%d]!}",
            get_hal_device_btcoex_m2s_mode_bitmap(hal_device), get_hal_m2s_mode_tpye(hal_device));
    } else {
        oam_warning_log3(0, OAM_SF_COEX,
            "{hmac_btcoex_s2m_siso_ap_resume_callback::a2dp[%d]sco[%d]transfer[%d]delay to mimo wait for next time!}",
            btble_status->bt_status.bt_status.bt_a2dp, btble_status->bt_status.bt_status.bt_sco,
            btble_status->bt_status.bt_status.bt_transfer);
    }

    return OAL_SUCC;
}

OSAL_STATIC osal_void hmac_ap_switch_siso(hal_to_dmac_device_stru *hal_device, hmac_device_stru *hmac_device)
{
    hal_m2s_event_tpye_uint16 m2s_event = HAL_M2S_EVENT_BUTT;

    /* 存在2G设备时候才切siso，最外层已经判断，这里肯定是存在,另外防止mss 回miso扫描，
       bt申请切siso时先abort当次扫描，避免冲突 */
    hmac_scan_abort(hmac_device);

    /* 前面for循环过滤了dbdc 并发扫描 custom(当前是只有ap模式才会)，此时只可能存在mss和rssi */
    if (hal_m2s_check_command_on(hal_device) == OSAL_TRUE) {
        /* 如果处于miso探测态，需要停止切换保护，并切c1 siso */
        if (hal_device->hal_m2s_fsm.oal_fsm.cur_state == HAL_M2S_STATE_MISO) {
            m2s_event = HAL_M2S_EVENT_BT_MISO_TO_SISO_C1;
        } else {
            m2s_event = HAL_M2S_EVENT_BT_SISO_C0_TO_SISO_C1;
        }
#ifdef _PRE_WLAN_FEATURE_M2S
        /* 切换保护功能关闭 */
        get_hal_device_m2s_switch_prot(hal_device) = OSAL_FALSE;
#endif
        /* 清业务标志 */
        get_hal_m2s_mode_tpye(hal_device) &= (~WLAN_M2S_TRIGGER_MODE_COMMAND);
    } else if (hal_m2s_check_rssi_on(hal_device) == OSAL_TRUE) { /* 如果只有低优先级的rssi,要判断是c0 miso 还是c1 miso */
        m2s_event = HAL_M2S_EVENT_BT_MISO_TO_SISO_C1;

        /* 清业务标志 */
        get_hal_m2s_mode_tpye(hal_device) &= (~WLAN_M2S_TRIGGER_MODE_RSSI);
    } else if (hal_m2s_check_btcoex_on(hal_device) == OSAL_TRUE) { /* 如果已经处于bt c1 siso状态的话,直接返回不处理即可 */
        oam_warning_log1(0, OAM_SF_COEX,
            "{hmac_btcoex_assoc_ap_check_process:: already c1 siso mode_stru[%d].}", get_hal_m2s_mode_tpye(hal_device));
    } else {
        m2s_event = HAL_M2S_EVENT_BT_MIMO_TO_SISO_C1;
    }

    if (m2s_event != HAL_M2S_EVENT_BUTT) {
        hal_device->device_btcoex_mgr.siso_ap_excute_on = OSAL_TRUE;
#ifdef _PRE_WLAN_FEATURE_M2S
        hmac_m2s_handle_event(hal_device, m2s_event, 0, OSAL_NULL);
#endif
        hmac_btcoex_ps_stop_check_and_notify(hal_device);

        oam_warning_log2(0, OAM_SF_COEX,
            "{hmac_btcoex_assoc_ap_check_process:: need to switch c1 siso of siso ap event[%d]mode_stru[%d].}",
            m2s_event, get_hal_m2s_mode_tpye(hal_device));
    }
    return;
}

/*****************************************************************************
 函 数 名  : hmac_btcoex_assoc_ap_check_process
 功能描述  : siso ap切换检查
*****************************************************************************/
osal_void hmac_btcoex_assoc_ap_check_process(hal_to_dmac_device_stru *hal_device)
{
    osal_u8               vap_idx, valid_vap_num;
    osal_u8               mac_vap_id[WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE] = {0};
    hmac_vap_stru        *hmac_vap = OSAL_NULL;
    oal_bool_enum_uint8 siso_need = OSAL_FALSE;
    hmac_user_stru      *hmac_user = OSAL_NULL;
    hmac_device_stru     *hmac_device = hmac_res_get_mac_dev_etc(0);

    /* siso ap已经切换到了siso，不处理即可，一定要保证siso ap切换在之前完成 */
    if (hal_device->device_btcoex_mgr.siso_ap_excute_on == OSAL_TRUE) {
        return;
    }

    /* 找到满足要求的vap个数 */
    valid_vap_num = hmac_btcoex_find_all_valid_sta_per_device(hal_device, mac_vap_id,
        WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE);
    /* 1.只考虑单sta模式 */
    if (valid_vap_num != 1) {
        return;
    }

    for (vap_idx = 0; vap_idx < valid_vap_num; vap_idx++) {
        hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap_id[vap_idx]);
        if (hmac_vap == OSAL_NULL) {
            oam_error_log1(0, OAM_SF_COEX, "{hmac_btcoex_assoc_ap_check_process::hmac_vap[%d] IS NULL.}",
                mac_vap_id[vap_idx]);
            return;
        }

        /* 不符合要求 */
        if (mac_btcoex_check_valid_sta(hmac_vap) == OSAL_FALSE) {
            continue;
        }

        /* STA模式获取user */
        hmac_user = (hmac_user_stru *)mac_res_get_hmac_user_etc(hmac_vap->assoc_vap_id);
        if (osal_unlikely(hmac_user == OSAL_NULL)) {
            oam_warning_log1(0, OAM_SF_COEX,
                "vap_id[%d] {hmac_btcoex_assoc_ap_check_process::hmac_user null.}", hmac_vap->vap_id);
            continue;
        }

        /* 关联的是siso ap，需要申请到siso; 可能是dbdc入网，此时仍然不需要处理 */
        if (hmac_user->user_num_spatial_stream == WLAN_SINGLE_NSS) {
            siso_need = OSAL_TRUE;
        }
    }

    /* 该切换方式可以要求苛刻一点;后续多vap之后，能切回来就可以 */
    /* mimo状态下申请切换siso; dbdc会在前面返回; miso等暂时不考虑; 单独优先级，
       无法直接调用hmac_btcoex_m2s_switch_apply接口 */
    if (siso_need == OSAL_TRUE) {
        hmac_ap_switch_siso(hal_device, hmac_device);
    }
    return;
}

#ifdef _PRE_WLAN_FEATURE_M2S
osal_bool hmac_btcoex_m2s_back_to_mimo_check(hal_to_dmac_device_stru *hal_device)
{
    if (hal_m2s_check_btcoex_on(hal_device) == OSAL_TRUE) {
        hmac_btcoex_s2m_allow_check(hal_device);
        return OSAL_TRUE;
    }
    return OSAL_FALSE;
}

osal_bool hmac_btcoex_m2s_back_to_mimo(hal_to_dmac_device_stru *hal_device, osal_u8 *m2s_mode_mask)
{
    if (hmac_m2s_btcoex_need_switch_check(hal_device, m2s_mode_mask) != OSAL_TRUE) {
        /* 1.c0 siso */
        if (hal_device->cfg_cap_info->phy_chain == WLAN_PHY_CHAIN_ZERO) {
            oam_warning_log0(0, OAM_SF_M2S,
                "{hmac_btcoex_m2s_back_to_mimo:: need to switch siso c1 because of btcoex!}");
            /* dbdc和spec都可能在此退出 */
            get_hal_m2s_mode_tpye(hal_device) &= (~trigger_mode);
            hmac_btcoex_m2s_switch_apply(hal_device, m2s_mode_mask);
        } else {
            /* 保持在c1 siso即可,清对应业务标记,增加维测，当前业务管理下出现此种情况属于异常，需要展开定位 */
            oam_error_log1(0, OAM_SF_M2S,
                "{hmac_btcoex_m2s_back_to_mimo:: btcoex apply but is already c1 siso, mode[%d]!}",
                get_hal_m2s_mode_tpye(hal_device));
        }
        return OSAL_TRUE;
    }
    return OSAL_FALSE;
}

osal_bool hmac_btcoex_m2s_update_hal_device(const hal_to_dmac_device_stru *hal_device, osal_u8 *m2s_mode_mask)
{
    if (hmac_m2s_btcoex_need_switch_check(hal_device, &m2s_mode_mask) == OSAL_TRUE) {
        /* btcoex业务在，直接切c1 siso即可 */
        get_hal_m2s_mode_tpye(hal_device) &= (~WLAN_M2S_TRIGGER_MODE_SPEC);
        /* 1.c0 siso */
        if (hal_device->cfg_cap_info->phy_chain == WLAN_PHY_CHAIN_ZERO) {
            oam_warning_log0(0, OAM_SF_M2S,
                "{hmac_m2s_spec_update_hal_device_proc:: need to switch siso c1 because of btcoex!}");

            hmac_btcoex_m2s_switch_apply(hal_device, m2s_mode_mask);
        } else {
            get_hal_m2s_mode_tpye(hal_device) |= WLAN_M2S_TRIGGER_MODE_BTCOEX;

            /* 保持在c1 siso即可,清对应业务标记,增加维测，当前业务管理下出现此种情况属于异常，需要展开定位 */
            oam_error_log1(0, OAM_SF_M2S,
                "{hmac_m2s_spec_update_hal_device_proc:: btcoex apply but is already c1 siso, mode[%d]!}",
                get_hal_m2s_mode_tpye(hal_device));
        }
        return OSAL_TRUE;
    }
    return OSAL_FALSE;
}

osal_void hmac_btcoex_m2s_choose_mimo_siso(hal_to_dmac_device_stru *hal_device)
{
    if (hal_m2s_check_btcoex_on(hal_device) == OSAL_TRUE) {
        hmac_btcoex_s2m_allow_check(hal_device);
    }
    /* sta入网，非dbdc场景下mimo是否需要切到c1 siso */
    hmac_btcoex_m2s_allow_check(hal_device, hmac_vap);
}
#endif

osal_u32 hmac_btcoex_m2s_init(osal_void)
{
#ifdef _PRE_WLAN_FEATURE_M2S
    hmac_feature_hook_register(HMAC_FHOOK_BTCOEX_M2S_M2S_ALLOW_CHECK, hmac_btcoex_m2s_allow_check);
    hmac_feature_hook_register(HMAC_FHOOK_BTCOEX_M2S_BACK_MIMO_CHECK, hmac_btcoex_m2s_back_to_mimo_check);
    hmac_feature_hook_register(HMAC_FHOOK_BTCOEX_M2S_BACK_MIMO, hmac_btcoex_m2s_back_to_mimo);
    hmac_feature_hook_register(HMAC_FHOOK_BTCOEX_M2S_UPDATE_HAL_DEVICE, hmac_btcoex_m2s_update_hal_device);
    hmac_feature_hook_register(HMAC_FHOOK_BTCOEX_M2S_CHOOSE_MIMO_SISO, hmac_btcoex_m2s_choose_mimo_siso);
#endif
    return OAL_SUCC;
}

osal_void hmac_btcoex_m2s_deinit(osal_void)
{
#ifdef _PRE_WLAN_FEATURE_M2S
    hmac_feature_hook_unregister(HMAC_FHOOK_BTCOEX_M2S_M2S_ALLOW_CHECK);
    hmac_feature_hook_unregister(HMAC_FHOOK_BTCOEX_M2S_BACK_MIMO_CHECK);
    hmac_feature_hook_unregister(HMAC_FHOOK_BTCOEX_M2S_BACK_MIMO);
    hmac_feature_hook_unregister(HMAC_FHOOK_BTCOEX_M2S_UPDATE_HAL_DEVICE);
    hmac_feature_hook_unregister(HMAC_FHOOK_BTCOEX_M2S_CHOOSE_MIMO_SISO);
    hmac_feature_hook_unregister(HMAC_FHOOK_BTCOEX_M2S_S2M_ALLOW_CHECK);
#endif
    return;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
