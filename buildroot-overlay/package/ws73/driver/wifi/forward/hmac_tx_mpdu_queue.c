/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: hmac 发送wifi的队列处理
 * Date: 2022-5-13
 */

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "hmac_tx_mpdu_queue.h"
#include "hmac_feature_dft.h"
#include "wlan_spec.h"
#include "hmac_tid.h"
#include "hmac_tx_mpdu_adapt.h"
#include "wlan_thruput_debug.h"
#include "hmac_alg_notify.h"
#include "hmac_config.h"
#include "hmac_vap.h"
#include "hmac_dfx.h"
#include "alg_traffic_ctl.h"
#include "hmac_slp.h"
#include "hmac_sdp.h"
#include "hmac_feature_interface.h"
#include "hmac_al_tx_rx.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_DEV_DMAC_TX_COMPLETE_ROM_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_DEVICE

#ifdef _PRE_WLAN_FEATURE_ALWAYS_TX
OSAL_STATIC osal_void hmac_11ax_trigger_schedule_notify(const hal_to_dmac_device_stru *hal_device, osal_u8 ac_num)
{
    osal_u8 qos, vap_idx;
    hmac_vap_stru *hmac_vap = OSAL_NULL;
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);

    unref_param(hal_device);
    if (ac_num == HAL_TX_QUEUE_BE) {
        qos = WLAN_WME_AC_BE;
    } else if (ac_num == HAL_TX_QUEUE_BK) {
        qos = WLAN_WME_AC_BK;
    } else if (ac_num == HAL_TX_QUEUE_VI) {
        qos = WLAN_WME_AC_VI;
    } else if (ac_num == HAL_TX_QUEUE_VO) {
        qos = WLAN_WME_AC_VO;
    } else {
        qos = WLAN_WME_AC_BUTT;
    }

    if ((qos > WLAN_WME_AC_VO)) {
        return;
    }

    for (vap_idx = 0; vap_idx < hmac_device->vap_num; vap_idx++) {
        hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(hmac_device->vap_id[vap_idx]);
        if (hmac_vap == OSAL_NULL) {
            oam_error_log0(0, OAM_SF_ANY, "{hmac_11ax_trigger_schedule_notify hmac_vap is null.}");
            continue;
        }

        /* 找到一个vap */
        if (hmac_vap->mib_info != OSAL_NULL) {
            break;
        }
    }

    if (hmac_vap == OSAL_NULL) {
        return;
    }
    /* 100表示长度 */
    hmac_config_send_qos_to_queue(hmac_vap, 100, qos);
}
#endif

/*****************************************************************************
 函 数 名  : hmac_tx_queue_mpdu
 功能描述  : 将mpdu下所有netbuf加入skb链表中的尾部,
                             需要考虑一个mpdu有多个netbuf
*****************************************************************************/
WIFI_TCM_TEXT OSAL_STATIC osal_void hmac_tx_queue_mpdu(oal_netbuf_stru *netbuf, oal_netbuf_head_stru *head)
{
    oal_netbuf_stru *netbuf_tmp;
    oal_netbuf_stru *netbuf_input = netbuf;

    while (netbuf_input != OSAL_NULL) {
        netbuf_tmp = oal_get_netbuf_next(netbuf_input);
        oal_netbuf_add_to_list_tail(netbuf_input, head);
        netbuf_input = netbuf_tmp;
    }
}

WIFI_TCM_TEXT OSAL_STATIC osal_void hmac_tid_tx_queue_fetch(hmac_tx_data_stat_stru *tx_data, hmac_tid_stru *tid_queue,
    osal_u8 mpdu_num, const hal_to_dmac_device_stru *hal_device, oal_netbuf_head_stru *buff_head)
{
    oal_netbuf_stru *netbuf = OSAL_NULL;
    mac_tx_ctl_stru *cb = OSAL_NULL;
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);
    osal_u8 mpdu_idx;

    unref_param(hal_device);

    /* 再从TID队列取 */
    for (mpdu_idx = 0; mpdu_idx < mpdu_num; mpdu_idx++) {
        if (oal_netbuf_list_empty(&tid_queue->buff_head) != 0) {
            break;
        }
        netbuf = hmac_tx_dequeue_first_mpdu(&tid_queue->buff_head);
        hmac_tx_tid_buffer_len_decrease(tid_queue, netbuf);
        hmac_tx_queue_mpdu(netbuf, buff_head);
        cb = (mac_tx_ctl_stru *)oal_netbuf_cb(netbuf);
        cb->tx_user_idx = (osal_u8)(tx_data->user->assoc_id & 0x3F); /* 只有6bit保存user_idx */
        tx_data->tx_ctl = cb;
        hmac_tid_tx_dequeue_update(hmac_device, tid_queue, 1);
    }
    oal_set_netbuf_next(netbuf, OSAL_NULL);
}

/*****************************************************************************
 功能描述  : 将一个MPDU从TID缓存队列中删除，并发送
*****************************************************************************/
OSAL_STATIC osal_u32 hmac_tid_tx_queue_remove(const hal_to_dmac_device_stru *hal_device, hmac_vap_stru *hmac_vap,
    hmac_tid_stru *tid_queue, osal_u8 mpdu_num)
{
    oal_netbuf_head_stru buff_head;
    hmac_tx_data_stat_stru tx_data = {0};
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);

    tx_data.user = (hmac_user_stru *)mac_res_get_hmac_user_etc(tid_queue->user_idx);
    tx_data.hmac_vap = hmac_vap;
    if (tx_data.user == OSAL_NULL) {
        oam_warning_log2(0, 0, "vap_id[%d] {user[%d] null.}", hmac_vap->vap_id, tid_queue->user_idx);
        return OAL_ERR_CODE_PTR_NULL;
    }

    oal_netbuf_list_head_init(&buff_head);
    hmac_tx_tid_lock_bh(hmac_device);
    hmac_tid_tx_queue_fetch(&tx_data, tid_queue, mpdu_num, hal_device, &buff_head);
    if (oal_netbuf_list_empty(&buff_head) != 0) {
        oam_warning_log1(0, OAM_SF_TX, "vap_id[%d] {no packets can be transmitted.}", hmac_vap->vap_id);
        hmac_tx_tid_unlock_bh(hmac_device);
        return OAL_FAIL;
    }
    hmac_tx_tid_unlock_bh(hmac_device);

    if (tx_data.tx_ctl == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_TX, "{hmac_tid_tx_queue_remove::cb null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
#ifdef _PRE_WIFI_DEBUG
    pkt_trace_debug((oal_netbuf_stru *)buff_head.next, "HMAC_TID_DEQUE");
#endif
    tx_data.tx_ctl->mpdu_num = mpdu_num;
    hmac_tx_data_send(&tx_data, &buff_head);

    return OAL_SUCC;
}


/*****************************************************************************
 函 数 名  : hmac_tx_schedule_tid
 功能描述  : 根据tid完成发送调度处理
*****************************************************************************/
WIFI_TCM_TEXT OSAL_STATIC osal_u32 hmac_tx_schedule_tid(hmac_user_stru *hmac_user,
    const hal_to_dmac_device_stru *hal_device, mac_tid_schedule_output_stru schedule_ouput, osal_u8 device_mpdu_full)
{
    osal_u8 mpdu_num;
    hmac_vap_stru *hmac_vap = OSAL_NULL;
    hmac_tid_stru *hmac_tid = OSAL_NULL;
    osal_u32 ret = OAL_SUCC;

    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);
    hmac_tid = &(hmac_user->tx_tid_queue[schedule_ouput.tid_num]);
    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(hmac_user->vap_id);
    if (hmac_vap == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_TX, "{hmac_tx_schedule_tid::hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    mpdu_num = schedule_ouput.mpdu_num[0];
    ret = hmac_tid_tx_queue_remove(hal_device, hmac_vap, hmac_tid, mpdu_num);
    hmac_alg_user_info_update_notify(hal_device, hmac_user, schedule_ouput, device_mpdu_full);

    hmac_tx_tid_lock_bh(hmac_device);
    hmac_alg_tid_update_notify(hmac_tid, 0);
    hmac_tx_tid_unlock_bh(hmac_device);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, 0, "vap_id[%d] {hmac_tx_schedule_tid::queue_remove or queue_remove_ampdu failed.}",
            hmac_vap->vap_id);
    }
    return ret;
}

WIFI_TCM_TEXT OSAL_STATIC osal_u32 hmac_tx_schedule_process(hal_to_dmac_device_stru *hal_device, osal_u8 ac_num,
    mac_tid_schedule_output_stru *schedule_ouput)
{
    hmac_user_stru *hmac_user = OSAL_NULL;
    osal_u32 ret;
    hmac_tid_stru *hmac_tid = OSAL_NULL;
    hmac_vap_stru *hmac_vap = OSAL_NULL;
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);
    unref_param(hmac_tid);

#ifdef _PRE_WLAN_FEATURE_ALWAYS_TX
    if ((hal_device->al_rx_flag == EXT_SWITCH_ON) && (hal_device->current_11ax_working == OSAL_TRUE)) {
        /* UL OFDMA qos发送完成后,需要重新将qos帧放入MAC队列等下一次发送 */
        hmac_11ax_trigger_schedule_notify(hal_device, ac_num);
        return OAL_SUCC;
    }
#endif

    /* 上锁之前获取一下vap pause bitmap情况 */
    hmac_vap_refresh_tx_bitmap();
    hmac_tx_tid_lock_bh(hmac_device);
    ret = hmac_alg_tx_schedule_notify(hal_device, ac_num, schedule_ouput);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_TX, "{hmac_tx_schedule_process::hmac_alg_tx_schedule_notify failed[%d].", ret);
        hmac_tx_tid_unlock_bh(hmac_device);
        return ret;
    }
    hmac_tx_tid_unlock_bh(hmac_device);
    if ((schedule_ouput->mpdu_num[0] == 0) && (schedule_ouput->ba_is_jamed == OSAL_FALSE)) {
        /* 若BA窗未卡死，tid队列无数据，return */
            return OAL_SUCC;
    }

    hmac_user = (hmac_user_stru *)mac_res_get_hmac_user_etc(schedule_ouput->user_idx);
    if (osal_unlikely(hmac_user == OSAL_NULL)) {
        oam_warning_log1(0, OAM_SF_TX, "{hmac_tx_schedule_process:hmac_user[%d] null.}", schedule_ouput->user_idx);
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_tid = &(hmac_user->tx_tid_queue[schedule_ouput->tid_num]);
    /* 调度所给的tid可能从属不同的vap因此要在获取一次 */
    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(hmac_user->vap_id);
    if (hmac_vap == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_TX, "{hmac_tx_schedule_process::hmac_vap null.}");
        wifi_printf("\n  DEAD_LOCK  hmac_tx_schedule_process  DEAD_LOCK \n");
        return OAL_ERR_CODE_PTR_NULL;
    }
#ifdef _PRE_WLAN_DFT_STAT
    /* 调度的次数增加 */
    hmac_tid->dispatch_num++;
#endif

    return ret;
}

WIFI_HMAC_TCM_TEXT WIFI_TCM_TEXT OSAL_STATIC osal_u32 hmac_tx_need_schedule(osal_u8 ac_num)
{
    osal_void *fhook = OSAL_NULL;
    osal_void *sdp_fhook = hmac_get_feature_fhook(HMAC_FHOOK_SDP_IS_INIT);
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);

    if (sdp_fhook != OSAL_NULL && (hal_gp_get_sdp_chnl_switch_off() == 1)) {
        oam_info_log1(0, OAM_SF_TX, "{hmac_tx_schedule::sdp chnl switch off, do not tx schedule[%d].}", ac_num);
        return OSAL_FALSE;
    }

    fhook = hmac_get_feature_fhook(HMAC_FHOOK_SLP_TX_DISABLE);
    if (fhook != OSAL_NULL && ((hmac_slp_tx_disable_cb)fhook)() == OSAL_TRUE) {
        oam_warning_log1(0, OAM_SF_TX, "{hmac_tx_schedule::slp tx ctrl, do not tx schedule[%d].}", ac_num);
        return OSAL_FALSE;
    }

    /* 如果tid队列&管理帧fake队列里面没有报文，则无需触发调度 */
    if (hmac_device->total_mpdu_num == 0 && hmac_vap_get_fake_queue_pkt_nums() == 0) {
        return OSAL_FALSE;
    }
    unref_param(ac_num);
    return OSAL_TRUE;
}

/*****************************************************************************
 函 数 名  : hmac_tx_schedule
 功能描述  : 发送完成调度处理
*****************************************************************************/
WIFI_HMAC_TCM_TEXT WIFI_TCM_TEXT osal_u32 hmac_tx_schedule(hal_to_dmac_device_stru *hal_device, osal_u8 ac_num)
{
#if defined(_PRE_WLAN_FEATURE_WS73)
    osal_u32 mpdu_num_left = hcc_get_tx_sched_num();
#elif defined(_PRE_LWIP_ZERO_COPY_MEM_ALLOC_PKT_BUF) || defined(_PRE_WLAN_FEATURE_WS53)
    osal_u32 mpdu_num_left = 100;
#else
    osal_u32 mpdu_num_left = alg_traffic_ctl_get_mpdu_cnt();
#endif
    osal_u8 device_mpdu_full;
    mac_tid_schedule_output_stru schedule_ouput = { 0 };
    hmac_user_stru *hmac_user = OSAL_NULL;
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);

    if (hmac_tx_need_schedule(ac_num) != OSAL_TRUE) {
        return OAL_SUCC;
    }

    if (mpdu_num_left == 0) {
        /* device缓存满时通知算法启动定时器，到期灌包 */
        hmac_alg_tx_schedule_timer_notify(hal_device);
        return OAL_SUCC;
    }

    /* 调度数据帧出队前，先将管理帧fake队列报文出队 */
    hmac_vap_send_all_fake_queue();

    while (mpdu_num_left != 0) {
        if (hmac_device->total_mpdu_num == 0) {
            break;
        }
        hmac_tx_schedule_process(hal_device, ac_num, &schedule_ouput);
        hmac_user = (hmac_user_stru *)mac_res_get_hmac_user_etc(schedule_ouput.user_idx);
        if (osal_unlikely(hmac_user == OSAL_NULL)) {
            oam_info_log1(0, OAM_SF_TX, "{hmac_tx_schedule::hmac_user[%d] null.}",
                schedule_ouput.user_idx);
            return OAL_ERR_CODE_PTR_NULL;
        }

        /* 算法调度不出来任何东西直接退出 */
        if (schedule_ouput.mpdu_num[0] == 0) {
            break;
        }

        if (mpdu_num_left >= schedule_ouput.mpdu_num[0]) {
            device_mpdu_full = OSAL_FALSE;
            hmac_tx_schedule_tid(hmac_user, hal_device, schedule_ouput, device_mpdu_full);
            mpdu_num_left -= schedule_ouput.mpdu_num[0];
        } else if (mpdu_num_left < schedule_ouput.mpdu_num[0]) {
            device_mpdu_full = OSAL_TRUE;
            schedule_ouput.mpdu_num[0] = (osal_u8)mpdu_num_left;
            hmac_tx_schedule_tid(hmac_user, hal_device, schedule_ouput, device_mpdu_full);
            mpdu_num_left -= mpdu_num_left;
        }
    }
    /* 如果device满，有用户还有包，重启定时器 */
    if ((mpdu_num_left == 0) && (hmac_device->total_mpdu_num != 0)) {
        hmac_alg_tx_schedule_timer_notify(hal_device);
    }
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_tx_need_enqueue_tid
 功能描述  : 如果硬件队列满或者对应的TID缓存队列不为空，
                              则将本次将要发送的MPDU入TID缓存队列
*****************************************************************************/
WIFI_TCM_TEXT OAL_STATIC oal_bool_enum_uint8 hmac_tx_need_enqueue_tid(const hmac_vap_stru *hmac_vap,
    hmac_tid_stru *tid_queue, osal_u8 q_num)
{
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);
#ifdef _PRE_WLAN_FEATURE_WS73
    osal_u32 mpdu_num_left = hcc_get_tx_sched_num();
#else
    osal_u32 mpdu_num_left = alg_traffic_ctl_get_mpdu_cnt();
#endif

    unref_param(tid_queue);

    /* 强制发送帧不入队列 */
    if (hmac_tx_is_tx_force(hmac_vap) == OSAL_TRUE) {
        return OSAL_FALSE;
    }

    /* 非WMM AC不入队列 */
    if (q_num > HAL_TX_QUEUE_VO) {
        return OSAL_FALSE;
    }

    if (hmac_is_thruput_enable(THRUPUT_TEST_NO_SCHEDULE) != 0) {
        alg_traffic_ctl_bypass_tid_queue();
        return OSAL_FALSE;
    }

    /* tid队列空/dmac发包未拥塞/管理帧未缓存，则不入队 */
    if (hmac_device->total_mpdu_num == 0 && mpdu_num_left != 0 && hmac_is_thruput_enable(THRUPUT_TEST_SCHEDULE) == 0 &&
        hmac_vap_get_fake_queue_pkt_nums() == 0) {
        alg_traffic_ctl_bypass_tid_queue();
        return OSAL_FALSE;
    }

    alg_traffic_ctl_enqueue_tid_queue();
    return OSAL_TRUE;
}

/*****************************************************************************
 函 数 名  : hmac_tid_tx_queue_enqueue
 功能描述  : 将一个MPDU或多个MPDU加入用户的TID缓存队列
*****************************************************************************/
WIFI_TCM_TEXT OSAL_STATIC osal_u32 hmac_tid_tx_queue_enqueue(hmac_device_stru *hmac_device, hmac_vap_stru *hmac_vap,
    hmac_tid_stru *tid_queue, osal_u8 mpdu_num, oal_netbuf_stru *netbuf)
{
    /* 更新device结构体下的统计信息 */
    if (osal_unlikely((hmac_device->total_mpdu_num + 1) > WLAN_TID_MPDU_NUM_LIMIT)) {
        oam_warning_log2(0, OAM_SF_TX, "vap_id[%d] {hmac_tid_tx_queue_enqueue::total_mpdu_num exceed[%d].}",
            hmac_vap->vap_id, hmac_device->total_mpdu_num);
        return OAL_FAIL;
    }

    hmac_tx_tid_buffer_len_increase(tid_queue, netbuf);
    hmac_tid_tx_enqueue_update(hmac_device, tid_queue, mpdu_num);

    hmac_tx_queue_mpdu(netbuf, &tid_queue->buff_head);

    return OAL_SUCC;
}

WIFI_TCM_TEXT osal_u32 hmac_tx_process_tid_queue(hal_to_dmac_device_stru *hal_device, hmac_vap_stru *hmac_vap,
    oal_netbuf_stru *netbuf, hmac_tid_stru *tid_queue, osal_u8 hal_q)
{
    osal_u32 ret;
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);
    mac_tx_ctl_stru *tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(netbuf);
    wlan_wme_ac_type_enum_uint8 ac = tx_ctl->ac;

    /* 判断数据帧是否需要入发送TID缓存队列 */
    if (hmac_tx_need_enqueue_tid(hmac_vap, tid_queue, hal_q) == OSAL_TRUE && tx_ctl->is_wur == 0) {
        hmac_tx_tid_lock_bh(hmac_device);
        ret = hmac_tid_tx_queue_enqueue(hmac_device, hmac_vap, tid_queue, 1, netbuf);
        if (osal_likely(ret == OAL_SUCC)) {
#ifdef _PRE_WIFI_DEBUG
            pkt_trace_debug(netbuf, "HMAC_TID_ENQUE");
#endif
            /* 入队成功通知算法 */
            hmac_alg_tid_update_notify(tid_queue, 1);
        } else {
#ifdef _PRE_WIFI_DEBUG
            pkt_drop_debug(tx_ctl, "tid_enque_fail");
#endif
            oam_warning_log1(0, OAM_SF_TX,
                "vap_id[%d] {hmac_tx_process_tid_queue::tx_queue_enqueue fail.}", hmac_vap->vap_id);
            /* 入队失败，但是返回SUCC，因此要释放netbuf */
            hmac_tx_excp_free_netbuf(netbuf);
        }
        hmac_tx_tid_unlock_bh(hmac_device);

        if (tid_queue->is_paused == 0) {
            hmac_tx_schedule(hal_device, ac);
        }

        return OAL_SUCC;
    }
    return OAL_CONTINUE;
}

/*****************************************************************************
 函 数 名  : hmac_dequeue_first_mpdu
 功能描述  : 从队列中取出第一个mpdu,将它从队列删除并返回，一个mpdu中可能有多个skb
*****************************************************************************/
WIFI_TCM_TEXT oal_netbuf_stru *hmac_tx_dequeue_first_mpdu(oal_netbuf_head_stru *netbuf_head)
{
    oal_netbuf_stru *first_net_buf = OSAL_NULL;
    mac_tx_ctl_stru *tx_ctrl = OSAL_NULL;

    first_net_buf = netbuf_head->next;
    tx_ctrl = (mac_tx_ctl_stru *)oal_netbuf_cb(first_net_buf);
    /* mpdu不是a-msdu，返回第一个net_buff即可 */
    if (tx_ctrl->is_amsdu == OSAL_FALSE) {
        first_net_buf = oal_netbuf_delist(netbuf_head);
        return first_net_buf;
    }

    /* 出现这种异常只能将所有缓存帧释放掉 */
    if (tx_ctrl->is_first_msdu == OSAL_FALSE) {
        oam_error_log0(0, OAM_SF_TX, "{hmac_tx_dequeue_first_mpdu::not the first msdu.}");
        /* 出现此错误时，需要将此帧出队,否则造成拥塞无法调度 */
        first_net_buf = oal_netbuf_delist(netbuf_head);
        return first_net_buf;
    }

    /* 将第一个mpdu中的所有skb从节能队列中取出，然后组成一个net_buff链 */
    first_net_buf = oal_netbuf_delist(netbuf_head);
    return first_net_buf;
}

osal_void hmac_vap_clear_tx_queue(hmac_vap_stru *hmac_vap)
{
    osal_s32 ret;
    frw_msg msg_info = {0};

    /* 抛事件至Device侧DMAC，释放VAP下的发送队列 */
    ret = frw_send_msg_to_device(hmac_vap->vap_id, WLAN_MSG_H2D_C_CFG_VAP_CLEAR_TX_QUEUE, &msg_info, OSAL_TRUE);
    if (ret != OAL_SUCC) {
        oam_error_log2(0, OAM_SF_ANY, "hmac_vap_clear_tx_queue:fail, vap_id[%d] ret[%d]", hmac_vap->vap_id, ret);
        wifi_printf("hmac_vap_clear_tx_queue:fail, vap_id[%d] ret[%d]", hmac_vap->vap_id, ret);
    }
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
