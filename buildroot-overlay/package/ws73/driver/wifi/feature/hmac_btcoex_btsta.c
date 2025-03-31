/*
 * Copyright: Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
 * Description: bt业务场景变化处理
 * Date: 2023-01-16 15:01
 */

#ifndef __HMAC_BTCOEX_BTSTA_C__
#define __HMAC_BTCOEX_BTSTA_C__

#include "frw_ext_if.h"
#include "oam_ext_if.h"
#include "hmac_btcoex.h"
#include "hmac_btcoex_ba.h"
#include "hmac_btcoex_m2s.h"
#include "hmac_btcoex_ps.h"
#include "hmac_alg_notify.h"
#include "hmac_feature_interface.h"
#include "hmac_btcoex_btsta.h"

#ifdef __cplusplus
#if __cplusplus
    extern "C" {
#endif
#endif
#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_HMAC_BTCOEX_BTSTA_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

#ifdef _PRE_WLAN_FEATURE_M2S
/*****************************************************************************
 函 数 名  : hmac_btcoex_double_chain_timeout_callback()
 功能描述  : 电话双链路下业务结束延迟处理定时器超时处理函数
*****************************************************************************/
OSAL_STATIC osal_u32 hmac_btcoex_double_chain_timeout_callback(osal_void *arg)
{
    hal_btcoex_btble_status_stru *btcoex_btble_status = OSAL_NULL;
    hal_to_dmac_chip_stru *hal_chip = OSAL_NULL;
    hmac_vap_stru *hmac_vap = OSAL_NULL;
    hmac_user_stru *hmac_user = OSAL_NULL;
    hmac_user_btcoex_delba_stru *btcoex_delba = OSAL_NULL;

    if (arg == OSAL_NULL) {
        oam_warning_log0(0, 0, "{hmac_btcoex_double_chain_timeout_callback:: arg null}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_vap = (hmac_vap_stru *)arg;
    hal_chip = hmac_vap->hal_chip;
    if (hal_chip == OSAL_NULL) {
        oam_warning_log1(0, OAM_SF_COEX,
            "vap_id[%d] {hmac_btcoex_double_chain_timeout_callback:: hal_chip null}", hmac_vap->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_user = (hmac_user_stru *)mac_res_get_hmac_user_etc(hmac_vap->assoc_vap_id);
    if (osal_unlikely(hmac_user == OSAL_NULL)) {
        oam_warning_log2(0, 0, "vap_id[%d] {hmac_btcoex_double_chain_timeout_callback::hmac_user[%d] null.}",
            hmac_vap->vap_id, hmac_vap->assoc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    btcoex_btble_status = hal_btcoex_btble_status();
    btcoex_delba = &(hmac_btcoex_get_user_info(hmac_user)->hmac_user_btcoex_delba);

    /* 更新标记刷新 */
    hmac_btcoex_get_vap_info(hmac_vap)->bitmap_update_flag = OSAL_FALSE;

    /* 3s之后蓝牙电话结束 */
    if (btcoex_btble_status->bt_status.bt_status.bt_6slot == OSAL_FALSE) {
        /* 双链路电话结束,恢复聚合64 */
        btcoex_delba->ba_size_real_index = BTCOEX_RX_WINDOW_SIZE_INDEX_3;
        btcoex_delba->ba_size_expect_index = BTCOEX_RX_WINDOW_SIZE_INDEX_3;
        hmac_btcoex_update_ba_size(hmac_vap, btcoex_delba, btcoex_btble_status);
        hmac_btcoex_delba_trigger(hmac_vap, OSAL_TRUE, btcoex_delba);
    } else {
        oam_warning_log3(0, OAM_SF_COEX,
            "vap_id[%d] {hmac_btcoex_sco_rx_rate_statistics_flag_callback:: 6s change[%d] to [%d].}", hmac_vap->vap_id,
            hmac_vap_get_vap_btcoex_mgr_6slot(hmac_vap), btcoex_btble_status->bt_status.bt_status.bt_6slot);

        /* 要判断是否出现了双链接切换，从6slot切换到12slot的话，不删聚合，12slot切到6slot需要删聚合到1 */
        if (hmac_vap_get_vap_btcoex_mgr_6slot(hmac_vap) != btcoex_btble_status->bt_status.bt_status.bt_6slot) {
            /* 1.如果出现12slot切换到6slot，需要继续删聚合到1(这里记录第一次的电话状态) */
            if ((hmac_vap_get_vap_btcoex_mgr_6slot(hmac_vap) == 1) &&
                (hmac_btcoex_get_vap_info(hmac_vap)->delba_on_6slot == OSAL_FALSE)) {
                /* 如果没有切换过6slot，才删聚合到1，已经聚合到了1，就一直保持1，直到删64恢复 */
                    btcoex_delba->ba_size = 1;
                    hmac_btcoex_delba_trigger(hmac_vap, OSAL_TRUE, btcoex_delba);
            }
        }

        /* 完成之后，需要做一次赋值更新 */
        hmac_btcoex_get_vap_info(hmac_vap)->bitmap_6slot = btcoex_btble_status->bt_status.bt_status.bt_6slot;
    }
    return OAL_SUCC;
}

OSAL_STATIC osal_u32 hmac_btcoex_sco_status_delba_prepare(hal_to_dmac_device_stru *hal_device,
    const bt_status_stru *bt_status)
{
    /* c1 siso不需要执行 */
    if (hal_device->hal_m2s_fsm.oal_fsm.cur_state == HAL_M2S_STATE_SISO &&
        hal_device->cfg_cap_info->phy_chain == WLAN_PHY_CHAIN_ONE) {
        return OAL_SUCC;
    }

    /* ac86u上行电话位置敏感概率掉坑，需要调整聚合时间 */
    if (bt_status->bt_6slot == 1) {
        /* mimo和siso场景有区分 */
        if (hal_device->hal_m2s_fsm.oal_fsm.cur_state == HAL_M2S_STATE_MIMO ||
            hal_device->hal_m2s_fsm.oal_fsm.cur_state == HAL_M2S_STATE_SIMO) {
            hmac_alg_cfg_btcoex_state_notify(hal_device, HMAC_ALG_BT_AGGR_TIME_4MS);
        } else {
            hmac_alg_cfg_btcoex_state_notify(hal_device, HMAC_ALG_BT_AGGR_TIME_2MS);
        }
    } else if (bt_status->bt_6slot == 2) { /* 六时隙时是否为2 */
        /* 6slot场景下，mimo和siso都需要此档位 */
        hmac_alg_cfg_btcoex_state_notify(hal_device, HMAC_ALG_BT_AGGR_TIME_1MS);
    } else {
        hmac_alg_cfg_btcoex_state_notify(hal_device, HMAC_ALG_BT_AGGR_TIME_OFF);
    }

    return OAL_CONTINUE;
}
#endif

/*****************************************************************************
 函 数 名  : hmac_btcoex_ldac_status_ps_process
 功能描述  : ldac场景处理方式(1)m2s切换 (2)ps+关闭动态调频
*****************************************************************************/
OSAL_STATIC osal_void hmac_btcoex_ldac_status_ps_process(hal_to_dmac_device_stru *hal_device,
    const bt_status_stru *bt_status)
{
    osal_void *fhook = hmac_get_feature_fhook(HMAC_FHOOK_AUTO_FREQ_BTCOEX);

    /* c1 siso不需要执行 */
    if ((hal_device->hal_m2s_fsm.oal_fsm.cur_state == HAL_M2S_STATE_SISO &&
        hal_device->cfg_cap_info->phy_chain == WLAN_PHY_CHAIN_ONE) ||
        (bt_status == OSAL_NULL)) {
        return;
    }

    if (fhook != OSAL_NULL) {
        ((hmac_auto_freq_btcoex_handle_cb)fhook)((osal_u8)bt_status->bt_ldac);
    }

    hmac_btcoex_restart_ps_timer(hal_device);
    // ldac状态变化后，更新ps param
    hmac_btcoex_set_dev_ps_frame(hal_device);

    return;
}

/*****************************************************************************
 函 数 名  : hmac_btcoex_update_coex_pri
 功能描述  : 根据ldac更新onepkt优先级
*****************************************************************************/
OSAL_STATIC osal_void hmac_btcoex_update_coex_pri(hal_to_dmac_device_stru *hal_device)
{
    hal_btcoex_btble_status_stru *btble_status = hal_btcoex_btble_status();
    /* ldac业务期间，禁止onepkt优先级配置 */
    if (btble_status->bt_status.bt_status.bt_ldac == OSAL_FALSE) {
        hal_device->btcoex_sw_preempt.coex_pri_forbit = OSAL_FALSE;
    } else {
        if ((btble_status->bt_status.bt_status.bt_a2dp != 0) ||
            (btble_status->bt_status.bt_status.bt_transfer != 0)) {
            hal_device->btcoex_sw_preempt.coex_pri_forbit = OSAL_TRUE;
        }
    }
}

/*****************************************************************************
 函 数 名  : hmac_btcoex_wifi_and_bt_insulate_check
 功能描述  : 查询bt和wifi是否可以隔离开，并判断是否需要刷新聚合大小，对应btcoex下业务不需要考虑处理
             (1)dbdc (2)主路dbac  (3)主路单vap  (4)主路多vap同信道  无单独辅路工作
             (5)当前c0 c1 siso/mimo都需要刷新发送聚合大小，因为放置在这里统一保证有效执行
             (6)业务assoc也要处理
*****************************************************************************/
OSAL_STATIC oal_bool_enum_uint8 hmac_btcoex_wifi_and_bt_insulate_check(hal_to_dmac_device_stru *hal_device,
    const bt_status_stru *bt_status, oal_bool_enum_uint8 aggr_num_check)
{
    hmac_vap_stru *hmac_vap = OSAL_NULL;
    osal_u8 vap_index;
    osal_u8 up_vap_num;
    osal_u8 mac_vap_id[WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE] = {0};
    oal_bool_enum_uint8 insulate = OSAL_TRUE;

    /* 2.主路device上存在一个5g，就等同于多vap同信道或者单vap工作在5g  不需要执行 */
    /* 找到所有up的vap设备 */
    up_vap_num = hal_device_find_all_up_vap(hal_device, mac_vap_id, WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE);
    for (vap_index = 0; vap_index < up_vap_num; vap_index++) {
        hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap_id[vap_index]);
        if (hmac_vap == OSAL_NULL) {
            oam_error_log1(0, OAM_SF_COEX,
                "vap_id[%d] hmac_btcoex_wifi_and_bt_insulate_check::hmac_vap IS NULL.", mac_vap_id[vap_index]);
            continue;
        }

        /* 存在2G的设备，单2G  2G dbac或者同信道多vap 2G */
        if (mac_btcoex_check_valid_vap(hmac_vap) == OSAL_TRUE) {
            insulate = OSAL_FALSE;
        }
    }

    if (aggr_num_check == OSAL_TRUE) {
        /* 1.存在2g vap，dbac也算，2.主路只有单5g下，但是dbdc的话也需要处理(辅路默认不单独工作) */
        if (insulate == OSAL_FALSE) {
            /* 1.当前6slot/ldac下即使c1
              siso下也不是空分，降低聚合时间保证上行性能稳定; 2.需要提前配置，dbdc/c1
              siso也需要处理，因此放置在这里; 3.纯5g场景不需要处理，反而影响性能
              4. 中断标志过来，本身和6slot是互斥的，不冲突 */
            if (bt_status->bt_ldac == 1 || bt_status->bt_6slot == 2) { /* WiFi蓝牙共存需要设ldac为1,6slot为2 */
                /* ldac场景下，mimo和siso都需要此档位 */
                hmac_alg_cfg_btcoex_state_notify(hal_device, HMAC_ALG_BT_AGGR_TIME_1MS);
            } else {
                hmac_alg_cfg_btcoex_state_notify(hal_device, HMAC_ALG_BT_AGGR_TIME_OFF);
            }
        }
    }

    return insulate;
}

#ifdef _PRE_WLAN_FEATURE_M2S
/*****************************************************************************
 函 数 名  : hmac_btcoex_sco_status_delba_process
 功能描述  : BT电话场景处理方式(1)m2s切换 (2)删聚合
*****************************************************************************/
OSAL_STATIC osal_u32 hmac_btcoex_sco_status_delba_process(hal_to_dmac_device_stru *hal_device,
    const hal_btcoex_btble_status_stru *btble_status)
{
    const bt_status_stru *bt_status = &(btble_status->bt_status.bt_status);
    hmac_user_btcoex_delba_stru *btcoex_delba = OSAL_NULL;
    hmac_user_stru *hmac_user = OSAL_NULL;
    oal_bool_enum_uint8 need_delba = OSAL_TRUE;
    osal_u8 vap_idx;
    osal_u8 valid_vap_num;
    osal_u8                           mac_vap_id[WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE] = {0};
    hmac_vap_stru                       *hmac_vap[WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE] = {OSAL_NULL};

    if (hmac_btcoex_sco_status_delba_prepare(hal_device, bt_status) != OAL_CONTINUE) {
        return OAL_SUCC;
    }

    /* 找到满足要求的vap个数 */
    valid_vap_num = hmac_btcoex_find_all_valid_sta_per_device(hal_device, mac_vap_id,
        WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE);

    /* valid的vap设备都做对应处理， 02只处理legacy sta   03要处理gc sta */
    for (vap_idx = 0; vap_idx < valid_vap_num; vap_idx++) {
        hmac_vap[vap_idx] = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap_id[vap_idx]);
        if (hmac_vap[vap_idx] == OSAL_NULL) {
            oam_error_log1(0, OAM_SF_COEX, "hmac_btcoex_sco_status_handler::hmac_vap[%d] NULL !", mac_vap_id[vap_idx]);
            return OAL_ERR_CODE_PTR_NULL;
        }

        hmac_user = (hmac_user_stru *)mac_res_get_hmac_user_etc((hmac_vap[vap_idx])->assoc_vap_id);
        if (osal_unlikely(hmac_user == OSAL_NULL)) {
            return OAL_ERR_CODE_PTR_NULL;
        }

        btcoex_delba = &(hmac_btcoex_get_user_info(hmac_user)->hmac_user_btcoex_delba);

        /* 正常bt打开和关闭场景下，对聚合进行处理,中断上半部能保证不会连续来6slot标志相同的下半部，
           双链接来了，这里也需要切换聚合个数，下面聚合档位一致，能保证不再删聚合
           但是12slot切到6slot，需要继续删聚合才能保证基本吞吐
           电话来时候，就不做速率统计了，立即删聚合，定时器单独用于sco结束做3s的状态保持，不立即恢复聚合，用于双链接 */
        if (bt_status->bt_6slot != 0) {
            /* 电话模式需要固定到BTCOEX_RX_WINDOW_SIZE_INDEX_1聚合方式，和当前聚合个数一致不需要删除BA，否则删除BA */
            /* Tplink5630不识别聚合2个，还是高聚合发送造成电话下容易掉底断流 */
            btcoex_delba->ba_size_expect_index = BTCOEX_RX_WINDOW_SIZE_INDEX_1;

            if (btcoex_delba->ba_size_expect_index == btcoex_delba->ba_size_real_index) {
                need_delba = OSAL_FALSE;
            }

            /* 6slot=0开启统计时，不能更新bitmap */
            if (hmac_btcoex_get_vap_info(hmac_vap[vap_idx])->bitmap_update_flag == OSAL_FALSE) {
                /* 支持双链接，每次电话业务触发时记录前一次，也即是当前6slot电话场景，便于后续出现切换时做删聚合操作 */
                hmac_btcoex_get_vap_info(hmac_vap[vap_idx])->bitmap_6slot = bt_status->bt_6slot;
            }

            hmac_btcoex_update_ba_size(hmac_vap[vap_idx], btcoex_delba, btble_status);

            hmac_btcoex_delba_trigger(hmac_vap[vap_idx], need_delba, btcoex_delba);
        } else {
            hmac_btcoex_get_vap_info(hmac_vap[vap_idx])->bitmap_update_flag = OSAL_TRUE;

            /* 电话业务结束，需要使能双链路统计定制器 */
            if (hmac_btcoex_get_vap_info(hmac_vap[vap_idx])->bt_coex_double_chain_timer.is_registerd == OSAL_TRUE) {
                frw_destroy_timer_entry(&(hmac_btcoex_get_vap_info(hmac_vap[vap_idx])->bt_coex_double_chain_timer));
            }
            frw_create_timer_entry(&(hmac_btcoex_get_vap_info(hmac_vap[vap_idx])->bt_coex_double_chain_timer),
                hmac_btcoex_double_chain_timeout_callback, BTCOEX_RX_STATISTICS_TIME,
                (osal_void *)(hmac_vap[vap_idx]), OSAL_FALSE);
        }
    }

    return OAL_SUCC;
}
#endif

/*****************************************************************************
 函 数 名  : hmac_btcoex_sco_status_handler
 功能描述  : BT电话场景处理入口: 处理聚合个数调整
*****************************************************************************/
OSAL_STATIC osal_s32 hmac_btcoex_sco_status_handler(hmac_vap_stru *hmac_vap)
{
    hal_chip_stru *hal_chip = OSAL_NULL;
    hal_to_dmac_device_stru *hal_device = OSAL_NULL;
    hal_btcoex_btble_status_stru *btble_status = OSAL_NULL;

    unref_param(hmac_vap);

    /* 获取chip指针 */
    hal_chip = hal_get_chip_stru();

    /* 暂时只是处理主路的STA */
    hal_device = &hal_chip->device.hal_device_base;

    btble_status = hal_btcoex_btble_status();

    /* 刷新ps stop标记 */
    hmac_btcoex_ps_stop_check_and_notify(hal_device);

    /* 1.如果是不需要执行，只有5g，或者无设备关联状态，dbdc包含在此范畴，直接返回 */
    /* 2.已经是c1 siso的话，这个需要在删聚合，以及m2s接口两套处理接口里面单独判断，不然m2s会被误返回 */
    /* siso ap已经切换到了siso，不处理即可，一定要保证siso ap切换在之前完成 */
    if ((hmac_btcoex_wifi_and_bt_insulate_check(hal_device, &(btble_status->bt_status.bt_status), OSAL_TRUE) ==
        OSAL_TRUE) || (hal_device->device_btcoex_mgr.siso_ap_excute_on == OSAL_TRUE)) {
        return OAL_SUCC;
    }
#ifdef _PRE_WLAN_FEATURE_M2S
    if (hal_device->device_btcoex_mgr.m2s_6slot == OSAL_TRUE) {
        /* 如果是0，并且已经触发了切siso, 不能用m2s_mode做判断，没有正式切mimo，但是标记已经清掉，此时需要申请回mimo */
        if (btble_status->bt_status.bt_status.bt_6slot == 0) {
            /* 电话业务结束 */
            get_hal_device_btcoex_s2m_mode_bitmap(hal_device) &= (~BT_M2S_6SLOT_MASK);

            if (hal_m2s_check_btcoex_on(hal_device) == OSAL_TRUE) {
                /* 申请切换回mimo */
                hmac_btcoex_s2m_switch_apply(hal_device, BT_M2S_6SLOT_MASK);
            } else {
                hmac_btcoex_sco_status_delba_process(hal_device, btble_status);
            }
        } else if (btble_status->bt_status.bt_status.bt_6slot == 2) { /* 判断六时隙时是否为2 */
            get_hal_device_btcoex_s2m_mode_bitmap(hal_device) |= BT_M2S_6SLOT_MASK;

            /* 双链接下，从2 0 1 0 2时，因为不会有a2dp标记存在,此时6slot 1又没有了，m2s_mode是等于0 就触发siso下切siso了
             */
            if (hal_m2s_check_btcoex_on(hal_device) == OSAL_TRUE) {
                /* 置m2s bitmap标记 */
                get_hal_device_btcoex_m2s_mode_bitmap(hal_device) |= BT_M2S_6SLOT_MASK;

                oam_warning_log1(0, OAM_SF_COEX,
                    "{hmac_btcoex_sco_status_handler::already siso m2s_mode[%d]!}",
                    get_hal_device_btcoex_m2s_mode_bitmap(hal_device));
            } else if (hmac_btcoex_m2s_switch_apply(hal_device, BT_M2S_6SLOT_MASK) != OAL_SUCC) {
                /* 申请切换siso */
                hmac_btcoex_sco_status_delba_process(hal_device, btble_status);
            }
        } else {
            get_hal_device_btcoex_s2m_mode_bitmap(hal_device) |= BT_M2S_6SLOT_MASK;

            /* 双链接时，从2 0 1，或者ldac切换到电话，此时判断已经处于siso切换状态下，就保持不处理即可 */
            if (hal_m2s_check_btcoex_on(hal_device) == OSAL_TRUE) {
                oam_warning_log1(0, OAM_SF_COEX,
                    "{hmac_btcoex_sco_status_handler::already siso m2s_mode[%d]!}",
                    get_hal_device_btcoex_m2s_mode_bitmap(hal_device));
            } else {
                /* 需要先判断是不是已经是SISO 那就也不需要删聚合了 */
                hmac_btcoex_sco_status_delba_process(hal_device, btble_status);
            }
        }
    } else {
        hmac_btcoex_sco_status_delba_process(hal_device, btble_status);
    }
#endif
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_btcoex_ldac_status_handler
 功能描述  : BT音乐场景针对ldac，识别处理
             音乐先来，bt侧做速率识别之后，再产生ldac中断，因此ldac使能时需要判断音乐状态
             音乐结束之后，会立即来ldac中断， 因此ldac禁能时不需要判断音乐状态
*****************************************************************************/
OSAL_STATIC osal_s32 hmac_btcoex_ldac_status_handler(hmac_vap_stru *hmac_vap)
{
    hal_chip_stru *hal_chip = OSAL_NULL;
    hal_to_dmac_device_stru *hal_device = OSAL_NULL;
    bt_status_stru *bt_status = OSAL_NULL;

    unref_param(hmac_vap);

    /* 获取chip指针 */
    hal_chip = hal_get_chip_stru();

    /* 暂时只是处理主路的STA */
    hal_device = &hal_chip->device.hal_device_base;
    hmac_btcoex_update_coex_pri(hal_device);

    bt_status = &(hal_btcoex_btble_status()->bt_status.bt_status);
    /* 1.如果是不需要执行，只有5g，或者无设备关联状态，dbdc包含在此范畴，直接返回 */
    if (hmac_btcoex_wifi_and_bt_insulate_check(hal_device, bt_status, OSAL_TRUE) == OSAL_TRUE) {
        return OAL_SUCC;
    }

    /* 2.已经是c1 siso的话，这个需要在删聚合，以及m2s接口两套处理接口里面单独判断，不然m2s会被误返回 */
    /* siso ap已经切换到了siso，不处理即可，一定要保证siso ap切换在之前完成 */
    if (hal_device->device_btcoex_mgr.siso_ap_excute_on == OSAL_TRUE) {
        return OAL_SUCC;
    }

    /* LDAC先执行方案1，并申请方案2，方案2切换siso成功的话，再关闭方案1 */
    hmac_btcoex_ldac_status_ps_process(hal_device, bt_status);
#ifdef _PRE_WLAN_FEATURE_M2S
    /* LDAC打开m2s方案, 如果切换失败，需要保持在miso时，等中断0来切回mimo，m2s本身逻辑不打断 */
    if (hal_device->device_btcoex_mgr.m2s_ldac == OSAL_TRUE) {
        /* ldac使能时 1.触发切换m2s 2.切换成功，暂停ps,业务置位 3.m2s切换失败，不处理即可 */
        if (bt_status->bt_ldac == OSAL_TRUE) {
            /* 双链接下，从ldac 1 0 到a2dp 1 保持siso时，此时来了a2dp = 0 ldac = 1 会出现siso下切siso */
            if (hal_m2s_check_btcoex_on(hal_device) == OSAL_TRUE) {
                /* 置m2s bitmap标记 */
                get_hal_device_btcoex_m2s_mode_bitmap(hal_device) |= BT_M2S_LDAC_MASK;

                oam_warning_log1(0, OAM_SF_COEX,
                    "{hmac_btcoex_ldac_status_handler::already siso m2s_mode[%d]!}",
                    get_hal_device_btcoex_m2s_mode_bitmap(hal_device));
            } else {
                hmac_btcoex_m2s_switch_apply(hal_device, BT_M2S_LDAC_MASK);
            }

            get_hal_device_btcoex_s2m_mode_bitmap(hal_device) |= BT_M2S_LDAC_MASK;
        } else {
            get_hal_device_btcoex_s2m_mode_bitmap(hal_device) &= (~BT_M2S_LDAC_MASK);

            if (hal_m2s_check_btcoex_on(hal_device) == OSAL_TRUE) {
                /* ldac禁用时  1.如果初始切换到siso，触发切换m2s 回mimo模块综合之后，看是否回mimo 2.恢复ps */
                hmac_btcoex_s2m_switch_apply(hal_device, BT_M2S_LDAC_MASK);
            }
        }
    }
#endif
    oam_warning_log3(0, OAM_SF_COEX,
        "{hmac_btcoex_ldac_status_handler::ldac state[%d],m2s_bitmap[%d],s2m_bitmap[%d].}", bt_status->bt_ldac,
        get_hal_device_btcoex_m2s_mode_bitmap(hal_device), get_hal_device_btcoex_s2m_mode_bitmap(hal_device));

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_btcoex_status_event_handler
 功能描述  : BT音乐场景处理入口: 处理聚合个数调整, 配置聚合个数，在接收速率逻辑进行删除BA操作
*****************************************************************************/
OSAL_STATIC osal_s32 hmac_btcoex_a2dp_status_handler(hmac_vap_stru *hmac_vap)
{
    osal_u8 vap_idx, valid_vap_num;
    osal_u8 mac_vap_id[WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE] = {0};
    hmac_vap_stru *hmac_valid_vap = OSAL_NULL;
    osal_u32 ret;
    hal_to_dmac_device_stru *hal_device = &hal_get_chip_stru()->device.hal_device_base;
    hal_btcoex_btble_status_stru *btble_status = hal_btcoex_btble_status();

    unref_param(hmac_vap);
    /* 1.如果是不需要执行，只有5g，或者无设备关联状态，dbdc包含在此范畴，直接返回 */
    if (hmac_btcoex_wifi_and_bt_insulate_check(hal_device, &(btble_status->bt_status.bt_status), OSAL_FALSE) ==
        OSAL_TRUE) {
        return OAL_SUCC;
    }
    /* 2.已经是c1 siso的话，这个需要在删聚合，以及m2s接口两套处理接口里面单独判断，不然m2s会被误返回 */
    /* 找到满足要求的vap个数 */
    valid_vap_num = hmac_btcoex_find_all_valid_sta_per_device(hal_device, mac_vap_id,
        WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE);
#ifdef _PRE_WLAN_FEATURE_M2S
    /* 如果是支持m2s切换，在音乐结束,并且bt业务已经在siso状态时做申请切回mimo */
    if (hal_device->device_btcoex_mgr.m2s_ldac == OSAL_TRUE) {
        if (btble_status->bt_status.bt_status.bt_a2dp == 0) {
            get_hal_device_btcoex_s2m_mode_bitmap(hal_device) &= (~BT_M2S_A2DP_MASK);

            if (hal_m2s_check_btcoex_on(hal_device) == OSAL_TRUE &&
                hal_device->device_btcoex_mgr.siso_ap_excute_on == OSAL_FALSE) {
                hmac_btcoex_s2m_switch_apply(hal_device, BT_M2S_A2DP_MASK);
            }
        } else {
            get_hal_device_btcoex_s2m_mode_bitmap(hal_device) |= BT_M2S_A2DP_MASK;
        }
    }
#endif
    /* 如果定时器创建了，需要按照新的时间来刷新 */
    hmac_btcoex_restart_ps_timer(hal_device);

    /* valid的vap设备都做对应处理 */
    for (vap_idx = 0; vap_idx < valid_vap_num; vap_idx++) {
        hmac_valid_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap_id[vap_idx]);
        if (hmac_valid_vap == OSAL_NULL) {
            oam_error_log1(0, OAM_SF_COEX, "{hmac_btcoex_a2dp_status_handler::vap[%d] IS NULL.}", mac_vap_id[vap_idx]);
            return OAL_ERR_CODE_PTR_NULL;
        }

        /* 找到valid sta，进行删减BA逻辑处理 */
        ret = hmac_btcoex_delba_event_process(btble_status, hmac_valid_vap);
        if (ret != OAL_SUCC) {
            oam_warning_log1(0, OAM_SF_COEX,
                "{hmac_btcoex_a2dp_status_handler::hmac_btcoex_delba_event_process() fail!, ret=[%d]}", ret);
        }
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_btcoex_assoc_ap_check_status_handler
 功能描述  : BT ba业务状态下场景处理入口: 如果存在siso legacy ap，就切siso，bt最高优先级切siso业务
             主要是解决行车记录仪  运动dv场景共存下wifi性能差
*****************************************************************************/
OSAL_STATIC osal_s32 hmac_btcoex_assoc_ap_check_status_handler(hmac_vap_stru *hmac_vap)
{
    hal_chip_stru *hal_chip = OSAL_NULL;
    hal_to_dmac_device_stru *hal_device = OSAL_NULL;
    hal_btcoex_btble_status_stru *btble_status = OSAL_NULL;

    unref_param(hmac_vap);
    /* 获取chip指针 */
    hal_chip = hal_get_chip_stru();
    /* 暂时只是处理主路的STA */
    hal_device = &hal_chip->device.hal_device_base;
    btble_status = hal_btcoex_btble_status();
    /* 1.如果是不需要执行，只有5g，或者无设备关联状态，dbdc包含在此范畴，直接返回 */
    if (hmac_btcoex_wifi_and_bt_insulate_check(hal_device, &(btble_status->bt_status.bt_status), OSAL_FALSE) ==
        OSAL_TRUE) {
        return OAL_SUCC;
    }

    /* 2.已经是c1 siso的话，这个需要在删聚合，以及m2s接口两套处理接口里面单独判断，不然m2s会被误返回 */
    /* 清除定时器，连续来了0 1 时也需要清定时器 */
    if (hal_device->device_btcoex_mgr.bt_coex_s2m_siso_ap_timer.is_registerd == OSAL_TRUE) {
        frw_destroy_timer_entry(&(hal_device->device_btcoex_mgr.bt_coex_s2m_siso_ap_timer));
    }

    /* 查找当前device是否存在legacy ap是siso的,看是否需要申请切到c1 siso */
    if (btble_status->bt_status.bt_status.bt_ba != 0) {
        hmac_btcoex_assoc_ap_check_process(hal_device);
    } else { /* 对应业务结束，查看是否已经置位c1 siso，启动定时器回mimo，防止音乐和电话切换太频繁 */
        /* 在该模式执行siso阶段，此时要保证能力和m2s状态一致 */
        if (hal_device->device_btcoex_mgr.siso_ap_excute_on == OSAL_TRUE) {
            frw_create_timer_entry(&(hal_device->device_btcoex_mgr.bt_coex_s2m_siso_ap_timer),
                hmac_btcoex_s2m_siso_ap_resume_callback, BTCOEX_S2M_RESUME_TIMEOUT,
                (osal_void *)hal_device, OSAL_FALSE);
        }
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_btcoex_page_scan_handler
 功能描述  : BT PAGE场景处理入口: beacon count刷新
*****************************************************************************/
OSAL_STATIC osal_s32 hmac_btcoex_page_scan_handler(hmac_vap_stru *hmac_vap)
{
    hmac_vap_btcoex_occupied_stru *btcoex_occupied = OSAL_NULL;
    hal_chip_stru *hal_chip = OSAL_NULL;
    hal_to_dmac_device_stru *hal_device = OSAL_NULL;
    hmac_vap_stru *hmac_valid_vap = OSAL_NULL;
    osal_u8 vap_idx;
    osal_u8 valid_vap_num;
    osal_u8                      mac_vap_id[WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE] = {0};

    unref_param(hmac_vap);

    /* 获取chip指针 */
    hal_chip = hal_get_chip_stru();

    /* 暂时只是处理主路的STA */
    hal_device = &hal_chip->device.hal_device_base;
    /* 1.如果是不需要执行，只有5g，或者无设备关联状态，dbdc包含在此范畴，直接返回 */
    if (hmac_btcoex_wifi_and_bt_insulate_check(hal_device,
        &(hal_btcoex_btble_status()->bt_status.bt_status), OSAL_FALSE) == OSAL_TRUE) {
        return OAL_SUCC;
    }

    /* 2.已经是c1 siso的话，这个需要在删聚合，以及m2s接口两套处理接口里面单独判断，不然m2s会被误返回 */
    /* 找到满足要求的vap个数 */
    valid_vap_num = hmac_btcoex_find_all_valid_ap_per_device(hal_device, mac_vap_id,
        WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE);

    /* valid的vap设备都做对应处理， 02只处理legacy sta   03要处理gc sta */
    for (vap_idx = 0; vap_idx < valid_vap_num; vap_idx++) {
        hmac_valid_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap_id[vap_idx]);
        if (hmac_valid_vap == OSAL_NULL) {
            oam_error_log1(0, OAM_SF_COEX, "{hmac_btcoex_page_scan_handler::hmac_vap[%d] IS NULL.}",
                mac_vap_id[vap_idx]);
            return OAL_ERR_CODE_PTR_NULL;
        }

        /* 找到valid sta，进行beacon miss逻辑处理 */
        btcoex_occupied = &(hmac_btcoex_get_vap_info(hmac_vap)->hmac_vap_btcoex_occupied);

        btcoex_occupied->ap_beacon_count = hal_btcoex_update_ap_beacon_count();
        btcoex_occupied->beacon_miss_cnt = 0;
    }

    /* 如果定时器创建了，需要按照新的时间来刷新 */
    hmac_btcoex_restart_ps_timer(hal_device);
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_btcoex_inquiry_status_handler
 功能描述  : BT inquiry场景处理入口: 如果ps定时器在启动，需要刷新寄存器时间
*****************************************************************************/
OSAL_STATIC osal_s32 hmac_btcoex_inquiry_status_handler(hmac_vap_stru *hmac_vap)
{
    hal_chip_stru *hal_chip = OSAL_NULL;
    hal_to_dmac_device_stru *hal_device = OSAL_NULL;

    unref_param(hmac_vap);

    /* 获取chip指针 */
    hal_chip = hal_get_chip_stru();

    /* 暂时只是处理主路的STA */
    hal_device = &hal_chip->device.hal_device_base;
    /* 1.如果是不需要执行，只有5g，或者无设备关联状态，dbdc包含在此范畴，直接返回 */
    if (hmac_btcoex_wifi_and_bt_insulate_check(hal_device,
        &(hal_btcoex_btble_status()->bt_status.bt_status), OSAL_FALSE) == OSAL_TRUE) {
        return OAL_SUCC;
    }

    /* 2.已经是c1 siso的话，这个需要在删聚合，以及m2s接口两套处理接口里面单独判断，不然m2s会被误返回 */
    /* 如果定时器创建了，需要按照新的时间来刷新 */
    hmac_btcoex_restart_ps_timer(hal_device);
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_btcoex_transfer_status_handler
 功能描述  : BT文件传输场景处理入口: 处理聚合个数调整
*****************************************************************************/
OSAL_STATIC osal_s32 hmac_btcoex_transfer_status_handler(hmac_vap_stru *hmac_vap)
{
    hal_btcoex_btble_status_stru *btble_status = hal_btcoex_btble_status();
    hal_to_dmac_device_stru *hal_device = &hal_get_chip_stru()->device.hal_device_base;
    osal_u8 vap_idx;
    osal_u8 valid_vap_num;
    osal_u8 mac_vap_id[WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE] = {0};
    hmac_vap_stru *hmac_valid_vap = OSAL_NULL;
    osal_u32 ret;

    unref_param(hmac_vap);

    /* 1.如果是不需要执行，只有5g，或者无设备关联状态，dbdc包含在此范畴，直接返回 */
    if (hmac_btcoex_wifi_and_bt_insulate_check(hal_device, &(btble_status->bt_status.bt_status), OSAL_FALSE) ==
        OSAL_TRUE) {
        return OAL_SUCC;
    }

    /* 2.已经是c1 siso的话，这个需要在删聚合，以及m2s接口两套处理接口里面单独判断，不然m2s会被误返回 */
    /* 找到满足要求的vap个数 */
    valid_vap_num = hmac_btcoex_find_all_valid_sta_per_device(hal_device, mac_vap_id,
        WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE);

    /* 如果定时器创建了，需要按照新的时间来刷新 */
    hmac_btcoex_restart_ps_timer(hal_device);

    /* valid的vap设备都做对应处理 */
    for (vap_idx = 0; vap_idx < valid_vap_num; vap_idx++) {
        hmac_valid_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap_id[vap_idx]);
        if (hmac_valid_vap == OSAL_NULL) {
            oam_error_log1(0, OAM_SF_COEX, "{hmac_btcoex_transfer_status_handler::hmac_vap[%d] IS NULL.}",
                mac_vap_id[vap_idx]);
            return OAL_ERR_CODE_PTR_NULL;
        }

        /* 找到valid sta，进行删减BA逻辑处理 */
        ret = hmac_btcoex_delba_event_process(btble_status, hmac_valid_vap);
        if (ret != OAL_SUCC) {
            oam_warning_log1(0, OAM_SF_COEX,
                "{hmac_btcoex_transfer_status_handler::hmac_btcoex_delba_event_process() fail!, ret=[%d]}", ret);
        }
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_btcoex_status_dispatch
 功能描述  : 状态寄存器处理分发
*****************************************************************************/
osal_void hmac_btcoex_status_dispatch(hmac_vap_stru *hmac_vap,
    const hal_btcoex_btble_status_stru *status_old, hal_btcoex_btble_status_stru *status_new)
{
    /* 数据传输 */
    if (status_old->bt_status.bt_status.bt_ba != status_new->bt_status.bt_status.bt_ba) {
        oam_warning_log1(0, OAM_SF_COEX, "{hmac_btcoex_status_dispatch::bt_ba change to %u}",
            status_new->bt_status.bt_status.bt_ba);
        hmac_btcoex_assoc_ap_check_status_handler(hmac_vap);
    }

    /* LDAC音乐 */
    if (status_old->bt_status.bt_status.bt_ldac != status_new->bt_status.bt_status.bt_ldac) {
        oam_warning_log1(0, OAM_SF_COEX, "{hmac_btcoex_status_dispatch::bt_ldac change to %u}",
            status_new->bt_status.bt_status.bt_ldac);
        hmac_btcoex_ldac_status_handler(hmac_vap);
    }

    /* SCO */
    if (status_old->bt_status.bt_status.bt_6slot != status_new->bt_status.bt_status.bt_6slot) {
        oam_warning_log1(0, OAM_SF_COEX, "{hmac_btcoex_status_dispatch::bt_6slot change to %u}",
            status_new->bt_status.bt_status.bt_6slot);
        hmac_btcoex_sco_status_handler(hmac_vap);
    }

    /* A2DP音乐 */
    if (status_old->bt_status.bt_status.bt_a2dp != status_new->bt_status.bt_status.bt_a2dp) {
        oam_warning_log1(0, OAM_SF_COEX, "{hmac_btcoex_status_dispatch::bt_a2dp change to %u}",
            status_new->bt_status.bt_status.bt_a2dp);
        hmac_btcoex_a2dp_status_handler(hmac_vap);
    }

    /* Page scan */
    if (status_old->bt_status.bt_status.bt_page != status_new->bt_status.bt_status.bt_page) {
        oam_warning_log1(0, OAM_SF_COEX, "{hmac_btcoex_status_dispatch::bt_page change to %u}",
            status_new->bt_status.bt_status.bt_page);
        hmac_btcoex_page_scan_handler(hmac_vap);
    }

    /* Inquiry */
    if (status_old->bt_status.bt_status.bt_inquiry != status_new->bt_status.bt_status.bt_inquiry) {
        oam_warning_log1(0, OAM_SF_COEX, "{hmac_btcoex_status_dispatch::bt_inquiry change to %u}",
            status_new->bt_status.bt_status.bt_inquiry);
        hmac_btcoex_inquiry_status_handler(hmac_vap);
    }

    /* Transfer */
    if (status_old->bt_status.bt_status.bt_transfer != status_new->bt_status.bt_status.bt_transfer) {
        oam_warning_log1(0, OAM_SF_COEX, "{hmac_btcoex_status_dispatch::bt_transfer change to %u}",
            status_new->bt_status.bt_status.bt_transfer);
        hmac_btcoex_transfer_status_handler(hmac_vap);
    }
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
