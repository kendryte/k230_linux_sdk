/*
 * Copyright (c) CompanyNameMagicTag 2020-2020. All rights reserved.
 * Description: dmac tid rom
 * Create: 2020-7-8
 */

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "hmac_tid.h"
#include "hmac_main.h"
#include "dmac_ext_if_hcm.h"
#include "hmac_blockack.h"
#include "hal_ext_if.h"
#include "hmac_alg_notify.h"
#include "hmac_tx_mgmt.h"
#include "hmac_mgmt_bss_comm.h"
#include "wlan_types_common.h"
#include "oal_net.h"
#include "mac_data.h"
#include "oal_types.h"

#include "hal_chip.h"

#include "frw_util.h"
#include "hmac_tx_mpdu_adapt.h"
#include "hmac_tx_mpdu_queue.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_DEV_DMAC_TID_ROM_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_DEVICE

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/

/*****************************************************************************
  3 函数声明
*****************************************************************************/

/*****************************************************************************
 函 数 名  : hmac_tid_clear_ba_proc
 功能描述  : tid清除时需要处理的ba流程
*****************************************************************************/
OSAL_STATIC osal_void hmac_tid_clear_ba_proc(hmac_tid_stru *tid_queue, osal_u8 del_user)
{
        /* 重置BA相关的内容 */
        if (tid_queue->ba_rx_hdl.ba_conn_status == HMAC_BA_COMPLETE) {
            tid_queue->ba_rx_hdl.ba_conn_status = HMAC_BA_INIT;
        }
        if (del_user == OSAL_FALSE) {
            return;
        }
        osal_spin_lock_destroy(&(tid_queue->ba_rx_lock));
}

/*****************************************************************************
 函 数 名  : hmac_tid_clear
 功能描述  : 删除tid队列中的所有信息
*****************************************************************************/
osal_u32 hmac_tid_clear(hmac_user_stru *hmac_user, hmac_device_stru *hmac_device, osal_u8 del_user)
{
    osal_u32 tid_idx;
    hmac_tid_stru *tid_queue = OSAL_NULL;
    oal_netbuf_stru *netbuf = OSAL_NULL;

    if (hmac_user == OSAL_NULL || hmac_device == OSAL_NULL) {
        oam_error_log2(0, OAM_SF_ANY, "hmac_tid_clear::hmac_user=%p, mac_device=%p null !",
            (uintptr_t)hmac_user, (uintptr_t)hmac_device);
        return OAL_ERR_CODE_PTR_NULL;
    }

    for (tid_idx = 0; tid_idx < WLAN_TID_MAX_NUM; tid_idx++) {
        osal_u32 irq_status;
        tid_queue = &(hmac_user->tx_tid_queue[tid_idx]);
        irq_status = frw_osal_irq_lock();
        netbuf = tid_queue->buff_head.next;
        if (netbuf == OSAL_NULL) {
            frw_osal_irq_restore(irq_status);
            continue;
        }
        while (netbuf != (oal_netbuf_stru *)&tid_queue->buff_head) {
            /* 再释放netbuf缓存队列 */
            netbuf = hmac_tx_dequeue_first_mpdu(&tid_queue->buff_head);
            /* TID的只有单帧 */
            hmac_tid_tx_dequeue_update(hmac_device, tid_queue, 1);

            hmac_tx_excp_free_netbuf(netbuf);

            netbuf = tid_queue->buff_head.next;
        }
        frw_osal_irq_restore(irq_status);
        tid_queue->mpdu_num = 0;

        tid_queue->tid_buffer_frame_len = 0;
        hmac_tid_clear_ba_proc(tid_queue, del_user);
    }

    if (hmac_device->total_mpdu_num > WLAN_TID_MPDU_NUM_LIMIT) {
        oam_error_log1(0, OAM_SF_TX,
            "{hmac_tid_clear::WLAN_TID_MPDU_NUM_LIMIT exceed, total_mpdu_num = %d.}", hmac_device->total_mpdu_num);
    }

    return OAL_SUCC;
}

static osal_u32 hmac_tid_send_msg(osal_u8 vap_id, osal_u16 msg_id, osal_u8 *data, osal_u32 data_len)
{
    frw_msg msg = {0};

    msg.data = data;
    msg.data_len = (osal_u16)data_len;

    return (osal_u32)frw_host_post_msg(msg_id, 0, vap_id, &msg);
}

/*****************************************************************************
 函 数 名  : hmac_tid_resume
 功能描述  : 恢复TID队列
*****************************************************************************/
osal_u32 hmac_tid_resume(hmac_tid_stru *tid, osal_u8 type)
{
    hmac_tid_resume_stru hmac_resume_tid = {0};

    if (tid == OSAL_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (type == DMAC_TID_PAUSE_RESUME_TYPE_BA) {
        tid->is_paused &= ~BIT0;
    } else if (type == DMAC_TID_PAUSE_RESUME_TYPE_PS) {
        tid->is_paused &= ~BIT1;
    }

    if (tid->is_paused == 0) {
        /* 通知算法 - 通过消息将该操作切至tx_data线程 */
        hmac_resume_tid.tid = tid->tid;
        hmac_resume_tid.user_idx = tid->user_idx;
        hmac_resume_tid.from_resume = OSAL_TRUE;
        /* 抛事件将tid操作切至发包线程 */
        hmac_tid_send_msg(tid->vap_id, WLAN_MSG_H2H_C_CFG_TID_RESUME, (osal_u8 *)&hmac_resume_tid,
            sizeof(hmac_tid_resume_stru));
    }

    return OAL_SUCC;
}

static osal_void hmac_tid_delete_head(hmac_tid_stru *tid_queue, osal_u16 mpdu_num, hmac_device_stru *hmac_device)
{
    osal_u16 mpdu_idx;
    osal_u16 num_tmp;
    oal_netbuf_stru *netbuf = OSAL_NULL;

    num_tmp = mpdu_num;
    for (mpdu_idx = 0; mpdu_idx < num_tmp; mpdu_idx++) {
        osal_u32 irq_status;
        irq_status = frw_osal_irq_lock();
        if (oal_netbuf_list_empty(&tid_queue->buff_head) == OSAL_TRUE) {
            frw_osal_irq_restore(irq_status);
            break;
        }
        netbuf = hmac_tx_dequeue_first_mpdu(&tid_queue->buff_head);
        frw_osal_irq_restore(irq_status);
        hmac_tx_excp_free_netbuf(netbuf);

        hmac_tid_tx_dequeue_update(hmac_device, tid_queue, 1);
    }
    return;
}

/*****************************************************************************
 函 数 名  : hmac_tid_delete_mpdu_head
 功能描述  : 将一个MPDU从TID缓存队列中删除并释放，算法只有突发情况才会调用,性能不敏感
*****************************************************************************/
osal_u32 hmac_tid_delete_mpdu_head(hmac_tid_stru *tid_queue, osal_u16 mpdu_num)
{
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);
    hmac_user_stru *hmac_user = OSAL_NULL;

    if (osal_unlikely(tid_queue == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_tid_delete_mpdu_head::tid_queue null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_user = (hmac_user_stru *)mac_res_get_hmac_user_etc(tid_queue->user_idx);
    if (hmac_user == OSAL_NULL) {
        oam_warning_log2(0, OAM_SF_ANY, "vap_id[%d] {hmac_tid_delete_mpdu_head::user[%d] null.}", tid_queue->vap_id,
            tid_queue->user_idx);

        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_tid_delete_head(tid_queue, mpdu_num, hmac_device);

    /* 更新tid_dlist链表 */
    hmac_alg_tid_update_notify(tid_queue, 0);

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_tid_tx_enqueue_update
 功能描述  : 每次操作TID enqueue skb后,都需要增加TID及Device的帧数统计
*****************************************************************************/
WIFI_TCM_TEXT osal_void hmac_tid_tx_enqueue_update(hmac_device_stru *hmac_device, hmac_tid_stru *tid_queue,
    osal_u8 mpdu_num)
{
    if (osal_unlikely(hmac_device == OSAL_NULL || tid_queue == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_tid_tx_enqueue_update::param null.}");
        return;
    }

    /* 更新TID包计数 */
    tid_queue->mpdu_num += mpdu_num;
#ifdef _PRE_WLAN_DFT_STAT
    /* 更新入队列的统计值 */
    tid_queue->in_num += mpdu_num;
#endif
    /* 更新DEV包计数 */
    hmac_device->total_mpdu_num += mpdu_num;
    hmac_device->aus_ac_mpdu_num[wlan_wme_tid_to_ac(tid_queue->tid)] += (osal_u16)mpdu_num;
}

/*****************************************************************************
 函 数 名  : hmac_tid_tx_dequeue_update
 功能描述  : 每次操作TID dequeue skb后,都需要减少TID及Device的帧数统计
*****************************************************************************/
WIFI_TCM_TEXT osal_void hmac_tid_tx_dequeue_update(hmac_device_stru *hmac_device, hmac_tid_stru *tid_queue,
    osal_u8 mpdu_num)
{
    if (osal_unlikely(hmac_device == OSAL_NULL || tid_queue == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_tid_tx_dequeue_update::param null.}");
        return;
    }

    /* 更新TID包计数 */
    tid_queue->mpdu_num -= mpdu_num;
#ifdef _PRE_WLAN_DFT_STAT
    /* 更新出队列的统计值 */
    tid_queue->out_num += mpdu_num;
#endif
    /* 更新DEV包计数 */
    hmac_device->total_mpdu_num -= mpdu_num;
    hmac_device->aus_ac_mpdu_num[wlan_wme_tid_to_ac(tid_queue->tid)] -= (osal_u16)mpdu_num;

    if ((hmac_device->total_mpdu_num > WLAN_TID_MPDU_NUM_LIMIT) ||
        (hmac_device->aus_ac_mpdu_num[wlan_wme_tid_to_ac(tid_queue->tid)] > WLAN_TID_MPDU_NUM_LIMIT)) {
        oam_error_log2(0, OAM_SF_TX,
            "vap_id[%d] {hmac_tid_tx_dequeue_update::NUM exceed, total_mpdu_num = %d.}",
            tid_queue->vap_id, hmac_device->total_mpdu_num);
    }
}

/*****************************************************************************
 功能描述  : 初始化一个用户的TID缓存队列
           hmac_user_init已将整个结构体清0 此处不再显示赋0
*****************************************************************************/
osal_void hmac_tid_tx_queue_init(hmac_tid_stru *tx_tid_queue, hmac_user_stru *hmac_user)
{
    hmac_tid_stru  *tid_queue   = OSAL_NULL;
    osal_u8 tid;

    if (osal_unlikely(tx_tid_queue == OSAL_NULL || hmac_user == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_tid_tx_queue_init::param null.}");
        return;
    }

    for (tid = 0; tid < WLAN_TID_MAX_NUM; tid++) {
        osal_u32 irq_status;
        tid_queue = &tx_tid_queue[tid];
        tid_queue->tid           = tid;
        tid_queue->user_idx      = hmac_user->assoc_id;
        tid_queue->vap_id        = hmac_user->vap_id;
#ifdef _PRE_WLAN_DFT_STAT
        tid_queue->in_num        = 0;
        tid_queue->out_num       = 0;
        tid_queue->dispatch_num  = 0;
#endif
        irq_status = frw_osal_irq_lock();
        oal_netbuf_list_head_init(&tid_queue->buff_head);
        frw_osal_irq_restore(irq_status);
        /* 初始化HT相关的内容 */
        tid_queue->tx_mode = DMAC_TX_MODE_NORMAL;
        /* 初始化RX BA相关的内容 */
        tid_queue->ba_rx_hdl.ba_conn_status = HMAC_BA_INIT;
        osal_spin_lock_init(&tid_queue->ba_rx_lock);
    }
}

static oal_netbuf_stru *hmac_tid_get_netbuf_by_index(const hmac_tid_stru *tid_queue, osal_u16 mpdu_index)
{
    osal_u32 irq_status;
    osal_u8 mpdu_idx;
    oal_netbuf_stru *netbuf_tmp = OSAL_NULL;
    irq_status = frw_osal_irq_lock();
    netbuf_tmp = tid_queue->buff_head.next;

    for (mpdu_idx = 0; mpdu_idx < mpdu_index; mpdu_idx++) {
        netbuf_tmp = netbuf_tmp->next;
        if (netbuf_tmp == (oal_netbuf_stru *)&tid_queue->buff_head) {
            netbuf_tmp = OSAL_NULL;
            break;
        }
    }
    frw_osal_irq_restore(irq_status);
    return netbuf_tmp;
}

/*****************************************************************************
 函 数 名  : hmac_tid_get_mpdu_by_index
 功能描述  : 获取TID的某个MPDU,
*****************************************************************************/
osal_u32 hmac_tid_get_mpdu_by_index(hmac_tid_stru *tid_queue, osal_u16 mpdu_num, oal_netbuf_stru **netbuf_stru)
{
    osal_u32 irq_status;
    oal_netbuf_stru *netbuf_tmp = OSAL_NULL;
    osal_u16 mpdu_index = mpdu_num;

    if (osal_unlikely(tid_queue == OSAL_NULL || netbuf_stru == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_tid_get_mpdu_by_index::param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 判断us_mpdu_index是否超出当前的mpdu数量 */
    if (osal_unlikely(mpdu_index > tid_queue->mpdu_num)) {
        oam_warning_log2(0, OAM_SF_ANY, "{hmac_tid_get_mpdu_by_index::mpdu_index[%d] > mpdu_num[%d].}",
                         mpdu_index, tid_queue->mpdu_num);
        *netbuf_stru = OSAL_NULL;
        return OAL_ERR_CODE_PTR_NULL;
    }

    irq_status = frw_osal_irq_lock();
    /* 再遍历netbuf队列 */
    if (oal_netbuf_list_empty(&tid_queue->buff_head) == OSAL_TRUE) {
        *netbuf_stru = OSAL_NULL;
        frw_osal_irq_restore(irq_status);
        oam_warning_log0(0, OAM_SF_ANY, "{hmac_tid_get_mpdu_by_index::queue empty.}");
        return OAL_FAIL;
    }
    frw_osal_irq_restore(irq_status);
    netbuf_tmp = hmac_tid_get_netbuf_by_index(tid_queue, mpdu_index);
    /* 在netbuf队列找到 */
    if (netbuf_tmp != OSAL_NULL) {
        *netbuf_stru = netbuf_tmp;
        return OAL_SUCC;
    }

    /* 没有找到 */
    *netbuf_stru = OSAL_NULL;
    return OAL_FAIL;
}

/*****************************************************************************
 功能描述  : 暂停TID队列
*****************************************************************************/
osal_void hmac_tid_pause(hmac_tid_stru *tid, osal_u8 type)
{
    hmac_tid_resume_stru hmac_resume_tid = {0};
    osal_u8 is_paused;

    if (osal_unlikely(tid == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_tid_pause::param null.}");
        return;
    }

    is_paused = tid->is_paused;

    if (type == DMAC_TID_PAUSE_RESUME_TYPE_BA) {
        tid->is_paused |= BIT0;
    } else if (type == DMAC_TID_PAUSE_RESUME_TYPE_PS) {
        tid->is_paused |= BIT1;
    }
    if (is_paused == 0) {
        hmac_resume_tid.tid = tid->tid;
        hmac_resume_tid.user_idx = tid->user_idx;
        hmac_resume_tid.from_resume = OSAL_FALSE;
        /* 抛事件将tid操作切至发包线程 */
        hmac_tid_send_msg(tid->vap_id, WLAN_MSG_H2H_C_CFG_TID_RESUME, (osal_u8 *)&hmac_resume_tid,
            sizeof(hmac_tid_resume_stru));
    }
}

/*****************************************************************************
 函 数 名  : hmac_config_tid_resume_process
 功能描述  : 恢复TID队列处理，通知算法
*****************************************************************************/
osal_s32 hmac_config_tid_resume_process(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    hmac_tid_resume_stru *hmac_resume_tid = (hmac_tid_resume_stru *)msg->data;
    hal_to_dmac_device_stru *hal_device = hal_chip_get_hal_device();
    hmac_user_stru *hmac_user = OSAL_NULL;
    hmac_tid_stru *tid = OSAL_NULL;

    if (hmac_vap == OSAL_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }
    hmac_user = (hmac_user_stru *)mac_res_get_hmac_user_etc(hmac_resume_tid->user_idx);
    if (hmac_user == OSAL_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }
    tid = &(hmac_user->tx_tid_queue[hmac_resume_tid->tid]);
    /* 通知算法 */
    hmac_alg_tid_update_notify(tid, 0);
    if (hmac_resume_tid->from_resume == OSAL_TRUE) {
        hmac_tx_schedule(hal_device, wlan_wme_tid_to_ac(tid->tid));
    }
    return OAL_SUCC;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
