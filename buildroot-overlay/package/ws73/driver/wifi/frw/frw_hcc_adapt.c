/*
 * Copyright (c) @CompanyNameMagicTag 2021-2021. All rights reserved.
 * Description: hcc service adapt layer.
 * Author: Huanghe
 * Create: 2021-09-01
 */
#include "frw_hmac_hcc_adapt.h"
#include "wlan_thruput_debug.h"
#include "mac_vap_ext.h"
#include "hmac_vap.h"
#include "frw_hmac.h"
#ifndef _PRE_WLAN_FEATURE_CENTRALIZE
#include "hcc_channel.h"
#endif
#if defined(_PRE_LWIP_ZERO_COPY_MEM_ALLOC_PKT_BUF)
#include "wal_net.h"
#endif
#ifdef _PRE_WLAN_FEATURE_POWERSAVE
#include "hmac_sta_pm.h"
#include "hmac_feature_interface.h"
#endif
#include "hmac_thruput_test.h"
#include "soc_customize_wifi.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_FRW_HCC_ADAPT_C

#define HCC_DMA_DATA_ALIGN 32

static inline osal_u32 oal_round_up(osal_u16 _old_len, osal_u16 _align)
{
    return ((((_old_len) + ((_align) - 1)) / (_align)) * (_align));
}

osal_void *frw_alloc_hcc_cfg_data(osal_u32 size)
{
    osal_u32 size_align = oal_round_up((osal_u16)size, HCC_DMA_DATA_ALIGN);

#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    if (in_interrupt() != 0 || irqs_disabled() != 0 || in_atomic() != 0) {
        return kzalloc(size_align, GFP_ATOMIC);
    }
    return kzalloc(size_align, GFP_KERNEL);
#endif

#if defined(_PRE_OS_VERSION_LITEOS) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
    return osal_kmalloc(size_align, 0);
#endif
}

osal_void frw_free_hcc_cfg_data(osal_u8 *data)
{
#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    kfree(data);
#endif

#if defined(_PRE_OS_VERSION_LITEOS) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
    osal_kfree(data);
#endif
}

#ifndef _PRE_WLAN_FEATURE_CENTRALIZE
OAL_STATIC hcc_adapt_ops g_wifi_service = {0};
OAL_STATIC hcc_adapt_ops g_customize_service = {0};

WIFI_HMAC_TCM_TEXT OAL_STATIC osal_u32 frw_alloc_hcc_netbuf(hcc_queue_type queue_id, osal_u32 len, osal_u8 **buf,
    osal_u8 **user_param)
{
    osal_u32 ret;
    oal_netbuf_stru *netbuf = OSAL_NULL;

    if (buf == OSAL_NULL || user_param == OSAL_NULL) {
        return OAL_FAIL;
    }

    if (queue_id == DATA_LO_QUEUE) {
        ret = frw_alloc_pbuf(len, &netbuf);
    } else {
        ret = frw_alloc_netbuf(len, &netbuf);
    }
    if (ret != OAL_SUCC) {
        *buf = OSAL_NULL;
        *user_param = OSAL_NULL;
        return OAL_FAIL;
    }

    *buf = oal_netbuf_payload(netbuf);
    *user_param = (osal_u8 *)netbuf;
    return OAL_SUCC;
}

WIFI_HMAC_TCM_TEXT OAL_STATIC osal_void frw_free_hcc_netbuf(hcc_queue_type queue_id, osal_u8 *buf, osal_u8 *user_param)
{
    if (user_param == OSAL_NULL) {
        return;
    }

    oal_netbuf_free((oal_netbuf_stru *)user_param);
}

OAL_STATIC osal_u32 frw_alloc_hcc_buf(hcc_queue_type queue_id, osal_u32 len, osal_u8 **buf, osal_u8 **user_param)
{
    if (buf == OSAL_NULL || user_param == OSAL_NULL) {
        return OAL_FAIL;
    }

    *buf = frw_alloc_hcc_cfg_data(len);
    *user_param = OSAL_NULL;
     if (*buf == OSAL_NULL) {
        return OAL_FAIL;
    }
    return OAL_SUCC;
}

OAL_STATIC osal_void frw_free_hcc_buf(hcc_queue_type queue_id, osal_u8 *buf, osal_u8 *user_param)
{
    if (buf == OSAL_NULL) {
        return;
    }
    frw_free_hcc_cfg_data(buf);
}

WIFI_HMAC_TCM_TEXT OAL_STATIC osal_u32 frw_netbuf_len_align(oal_netbuf_stru *netbuf, osal_u32 align_len)
{
    osal_s32 ret;
    osal_u32 len_algin, tail_room_len;
    osal_u32 len = OAL_NETBUF_LEN(netbuf);
    if ((len & (align_len - 1)) == 0) {
        return OAL_SUCC;
    }
    /* align the netbuf */
    len_algin = oal_round_up(len, align_len);
    tail_room_len = len_algin - len;
    if (tail_room_len > oal_netbuf_tailroom(netbuf)) {
        /* tailroom not enough */
        ret = oal_netbuf_expand_head(netbuf, 0, (osal_s32)tail_room_len, OSAL_GFP_KERNEL);
        if (ret != OAL_SUCC) {
            oal_netbuf_free(netbuf);
            return OAL_FAIL;
        }
    }
    oal_netbuf_put(netbuf, tail_room_len);

    return OAL_SUCC;
}

osal_u32 frw_start_hcc_service(hcc_service_type service_type, hcc_adapt_priv_rx_process rx_process)
{
    hcc_adapt_priv_free hcc_free;
    hcc_adapt_priv_alloc hcc_alloc;
    hcc_adapt_ops *adapt = OSAL_NULL;
    osal_u32 ret;

    switch (service_type) {
        case HCC_ACTION_TYPE_WIFI:
            hcc_alloc = frw_alloc_hcc_netbuf;
            hcc_free = frw_free_hcc_netbuf;
            adapt = &g_wifi_service;
            break;
        case HCC_ACTION_TYPE_CUSTOMIZE:
            hcc_alloc = frw_alloc_hcc_buf;
            hcc_free = frw_free_hcc_buf;
            adapt = &g_customize_service;
            break;
        default:
            break;
    }

    if (adapt != OSAL_NULL) {
        adapt->alloc = hcc_alloc;
        adapt->free = hcc_free;
        adapt->rx_proc = rx_process;
#ifdef _PRE_WLAN_FEATURE_OFFLOAD_FLOWCTL
        adapt->start_subq = hmac_vap_net_start_subqueue_etc;
        adapt->stop_subq = hmac_vap_net_stop_subqueue_etc;
#endif
        ret = (osal_u32)hcc_service_init(HCC_CHANNEL_AP, service_type, adapt);
        wifi_printf("frw_start_hcc_service type:%d, ret=%d\r\n", service_type, ret);
        return ret;
    } else {
        return OAL_FAIL;
    }
}

osal_u8 frw_get_devmsg_inqueue_state(osal_void)
{
    return hcc_get_state(HCC_CHANNEL_AP);
}

osal_u32 frw_stop_hcc_service(hcc_service_type service_type)
{
    osal_u32 ret;
    ret = hcc_service_deinit(HCC_CHANNEL_AP, service_type);
    wifi_printf("frw_stop_hcc_service type:%d, ret=%d\r\n", service_type, ret);
    return ret;
}

/* 根据数据类型配置相应的队列及流控方式 */
WIFI_HMAC_TCM_TEXT OAL_STATIC osal_void hmac_config_hcc_flowctrl_type(osal_u8 data_type, osal_u16 *fc_flag,
    osal_u8 *queue_id)
{
    switch (data_type) {
        case HCC_MSG_TYPE_CFG:           /* 配置消息 */
        case HCC_MSG_TYPE_RSP:           /* 消息响应 */
            *fc_flag = HCC_FC_NONE;      /* 无流控 */
            *queue_id = CTRL_QUEUE;
            break;
        case HCC_MSG_TYPE_MGMT_FRAME:    /* 管理帧 */
            *fc_flag = HCC_FC_WAIT;      /* 阻塞等待 */
            *queue_id = DATA_HI_QUEUE;
            break;
        case HCC_MSG_TYPE_DATA_FRAME:    /* 数据帧 */
            *fc_flag = HCC_FC_WAIT;      /* 阻塞等待 */
            *queue_id = DATA_LO_QUEUE;
            break;
        default:
            *fc_flag = HCC_FC_NONE;
            *queue_id = DATA_LO_QUEUE;
    }
}

OAL_STATIC osal_void hcc_service_init_hdr(hcc_transfer_param *param, osal_u32 main_type, osal_u32 sub_type,
    osal_u8 queue_id, osal_u16 fc_flag)
{
    param->service_type = main_type;
    param->sub_type = sub_type;
    param->queue_id = queue_id;
    param->fc_flag = fc_flag;
}

osal_u32 frw_tx_wifi_cfg_by_hcc(osal_u8 *buf, osal_u16 len, osal_u8 data_type)
{
    hcc_transfer_param hcc_tx_param;
    osal_u16 fc_flag;
    osal_u8 queue_id;
    osal_u32 len_align = oal_round_up(len, HCC_DMA_DATA_ALIGN);

#ifdef _PRE_WLAN_FEATURE_POWERSAVE
    osal_void *fhook_wkup_dev = hmac_get_feature_fhook(HMAC_FHOOK_PM_STA_WAKEUP_DEV);
    if ((fhook_wkup_dev != OSAL_NULL) &&
        (((hmac_sta_pm_wakeup_dev_cb)fhook_wkup_dev)((uintptr_t *)buf, len, data_type) != OAL_SUCC)) {
        oam_warning_log0(0, OAM_SF_FRW, "frw_tx_wifi_cfg_by_hcc: sta_pm");
        return OAL_FAIL;
    }
#endif

    hmac_config_hcc_flowctrl_type(data_type, &fc_flag, &queue_id);
    hcc_service_init_hdr(&hcc_tx_param, HCC_ACTION_TYPE_CUSTOMIZE, CUSTOM_CFGID_MSG, queue_id, fc_flag);
    hcc_tx_param.user_param = OSAL_NULL;
    return (osal_u32)hcc_tx_data(HCC_CHANNEL_AP, buf, len_align, &hcc_tx_param);
}

OAL_STATIC osal_void frw_msg_adapt_init(frw_msg_adapt_comm *msg_adapt, osal_u16 msg_id, osal_u8 vap_id,
    osal_u32 msg_seq, frw_msg *cfg)
{
    msg_adapt->msg_id = msg_id;
    msg_adapt->vap_id = vap_id;
    msg_adapt->msg_seq = msg_seq;
    msg_adapt->cfg.data = 0;
    msg_adapt->cfg.rsp = 0;
    msg_adapt->cfg.data_len = cfg->data_len;
    msg_adapt->cfg.rsp_buf_len = cfg->rsp_buf_len;
    msg_adapt->cfg.rsp_len = cfg->rsp_len;
    msg_adapt->cfg.sync = cfg->sync;
    msg_adapt->cfg.type = cfg->type;
    msg_adapt->cfg.rsv = cfg->rsv;
    return;
}

osal_s32 frw_send_msg_adapt_by_hcc(osal_u16 msg_id, frw_msg *cfg, osal_u8 vap_id, osal_u32 msg_seq, osal_bool sync)
{
    osal_s32 ret;
    osal_u8 *cfg_buf = OSAL_NULL;
    frw_msg_adapt_comm msg_adapt = {0};
    osal_u32 payload_len = (osal_u32)(sizeof(frw_msg_adapt_comm) + cfg->data_len);
    osal_u32 extend_len = frw_get_head_len() + hcc_get_head_len();

    frw_msg_adapt_init(&msg_adapt, msg_id, vap_id, msg_seq, cfg);
    msg_adapt.sync = sync;
    msg_adapt.type = HCC_MSG_TYPE_CFG;

    cfg_buf = frw_alloc_hcc_cfg_data(payload_len + extend_len);
    if (cfg_buf == OSAL_NULL) {
        oam_warning_log2(vap_id, OAM_SF_FRW, "frw_send_msg_adapt: alloc failed. msg_id %d sync %d", msg_id, sync);
        return OAL_ERR_CODE_ALLOC_MEM_FAIL;
    }

    cfg_buf += extend_len;
    if (memcpy_s(cfg_buf, sizeof(frw_msg_adapt_comm), &msg_adapt, sizeof(frw_msg_adapt_comm)) != EOK) {
        cfg_buf -= extend_len;
        frw_free_hcc_cfg_data(cfg_buf);
        oam_error_log1(vap_id, OAM_SF_FRW, "frw_send_msg_adapt: memcpy_s failed. msg_id %d", msg_id);
        return OAL_FAIL;
    }

    if (cfg->data_len != 0 && cfg->data != OSAL_NULL) {
        if (memcpy_s(cfg_buf + sizeof(frw_msg_adapt_comm), cfg->data_len, cfg->data, cfg->data_len) != EOK) {
            cfg_buf -= extend_len;
            frw_free_hcc_cfg_data(cfg_buf);
            oam_error_log1(vap_id, OAM_SF_FRW, "frw_send_msg_adapt: memcpy_s failed. msg_id %d", msg_id);
            return OAL_FAIL;
        }
    }
    cfg_buf -= frw_get_head_len();
    hmac_frw_hcc_extend_hdr_init(payload_len, (struct frw_hcc_extend_hdr *)cfg_buf, vap_id, msg_id, msg_adapt.type);

    cfg_buf -= hcc_get_head_len();
    ret = (osal_s32)frw_tx_wifi_cfg_by_hcc(cfg_buf, payload_len + extend_len, msg_adapt.type);
    if (ret != OAL_SUCC) {
        oam_warning_log2(vap_id, OAM_SF_FRW, "frw_send_msg_adapt:: msg_id %d drop! ret 0x%x", msg_id, ret);
        wifi_printf("[E]frw_send_msg_adapt[%u:%u] drop! ret = 0x%x\n", msg_adapt.type, msg_id, ret);
        frw_free_hcc_cfg_data(cfg_buf);
    }

    return ret;
}

#if defined(_PRE_WLAN_FEATURE_QUE_CLASSIFY)
WIFI_HMAC_TCM_TEXT osal_void hmac_config_netbuf_queue_id(oal_netbuf_stru *netbuf, osal_u16 *fc_flag, osal_u8 *queue_id)
{
    osal_u16 queue_mapping = oal_skb_get_queue_mapping(netbuf);

    switch (queue_mapping) {
        case WLAN_TCP_ACK_QUEUE:           /* TCP ACK */
            *fc_flag = HCC_FC_WAIT;        /* 阻塞等待 */
            *queue_id = wlan_data_queue_to_hcc(queue_mapping); /* used for tcp ack */
            break;
        case WLAN_TCP_DATA_QUEUE:          /* TCP DATA */
            *fc_flag = HCC_FC_WAIT;        /* 阻塞等待 */
            *queue_id = wlan_data_queue_to_hcc(queue_mapping); /* used for tcp data */
            break;
        case WLAN_UDP_DATA_QUEUE:
            *fc_flag = HCC_FC_NET;       /* 网络层流控 */
#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
            if (hwifi_get_udp_hcc_fc_drop() == OSAL_TRUE) {
                *fc_flag = HCC_FC_DROP;       /* UDP丢包流控 */
            }
#endif
#endif
            *queue_id = wlan_data_queue_to_hcc(queue_mapping);
            break;
        default:
            break;
    }
}
#endif

WIFI_HMAC_TCM_TEXT osal_u32 frw_tx_wifi_netbuf_by_hcc(oal_netbuf_stru *netbuf, osal_u16 payload_len, osal_u8 data_type)
{
    hcc_transfer_param hcc_tx_param;
    osal_u16 fc_flag;
    osal_u8 queue_id;
    osal_u32 len_algin = HCC_DMA_DATA_ALIGN;

    hcc_handler *hcc = hcc_get_handler(HCC_CHANNEL_AP);
    if (hcc != OSAL_NULL) {
        len_algin = hcc->bus->len_align;
    }

    /* config hcc header */
    hmac_config_hcc_flowctrl_type(data_type, &fc_flag, &queue_id);
#if defined(_PRE_WLAN_FEATURE_QUE_CLASSIFY)
    if (data_type == HCC_MSG_TYPE_DATA_FRAME) {
        hmac_config_netbuf_queue_id(netbuf, &fc_flag, &queue_id);
    }
#endif
    hcc_service_init_hdr(&hcc_tx_param, HCC_ACTION_TYPE_WIFI, 0, queue_id, fc_flag);

    hcc_tx_param.user_param = (osal_u8 *)netbuf;
    oal_netbuf_push(netbuf, hcc_get_head_len());

    if (frw_netbuf_len_align(netbuf, len_algin) != OAL_SUCC) {
        wifi_printf("hcc_wifi_tx_data aling fail\r\n");
        return OAL_FAIL;
    }

    host_print_tx_timestamp(netbuf);

#ifdef _PRE_WLAN_FEATURE_POWERSAVE
    osal_void *fhook_wkup_dev = hmac_get_feature_fhook(HMAC_FHOOK_PM_STA_WAKEUP_DEV);
    if ((fhook_wkup_dev != OSAL_NULL) &&
        (((hmac_sta_pm_wakeup_dev_cb)fhook_wkup_dev)((uintptr_t *)netbuf, payload_len, data_type) != OAL_SUCC)) {
        oam_warning_log0(0, OAM_SF_FRW, "frw_tx_wifi_netbuf_by_hcc: sta_pm");
        return OAL_FAIL;
    }
#endif

    if (hmac_is_thruput_enable(THRUPUT_TX_BEFORE_HCC)) {
        hmac_update_thruput(OAL_NETBUF_LEN(netbuf));
        oal_netbuf_free(netbuf);
        return OAL_SUCC;
    }

    hmac_delay(6); /* 6：发送打点时延 */
    return (osal_u32)hcc_tx_data(HCC_CHANNEL_AP, oal_netbuf_payload(netbuf), OAL_NETBUF_LEN(netbuf), &hcc_tx_param);
}

osal_void hcc_hmac_config_bus_ini(osal_u8 *param)
{
    bus_cus_config_stru *bus_config = (bus_cus_config_stru *)param;
    hcc_set_tx_sched_count(HCC_CHANNEL_AP, bus_config->bus_h2d_sched_count);
}

osal_void hcc_hmac_wifi_msg_register(osal_u8 msg_id, osal_void *callback)
{
    hcc_message_register(HCC_CHANNEL_AP, HCC_ACTION_TYPE_TEST, (hcc_rx_msg_type)msg_id, (hcc_msg_rx)callback, NULL);
}

#else

osal_u32 frw_start_hcc_service(osal_u8 service_type, osal_void *rx_process)
{
    unref_param(service_type);
    unref_param(rx_process);
    return OAL_SUCC;
}

osal_u8 frw_get_devmsg_inqueue_state(osal_void)
{
    return OSAL_TRUE;
}

osal_u32 frw_stop_hcc_service(osal_u8 service_type)
{
    unref_param(service_type);
    return OAL_SUCC;
}

osal_void hcc_hmac_config_bus_ini(osal_u8 *param)
{
    unref_param(param);
    return;
}

osal_void hcc_hmac_wifi_msg_register(osal_u8 msg_id, osal_void *callback)
{
    unref_param(msg_id);
    unref_param(callback);
    return;
}

OAL_STATIC osal_u32 frw_dmac_netbuf_conver_hmac_netbuf(oal_dmac_netbuf_stru *dmac_netbuf, osal_u32 payload_len,
    oal_netbuf_stru **netbuf)
{
    oal_netbuf_stru *hmacbuf = OSAL_NULL;

    if (frw_alloc_pbuf(payload_len, &hmacbuf) != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_FRW, "***frw_alloc_netbuf fail\r\n");
        return OAL_FAIL;
    }

    if (hmacbuf == OSAL_NULL) {
        return OAL_FAIL;
    }

    if (memcpy_s(oal_netbuf_payload(hmacbuf), payload_len, oal_dmac_netbuf_payload(dmac_netbuf),
                 payload_len) != EOK) {
        oal_netbuf_free(hmacbuf);
        return OAL_FAIL;
    }
    oal_netbuf_set_len(hmacbuf, payload_len);
    /* 转换成功释放dmac netbuf */
    oal_mem_netbuf_free((oal_dmac_netbuf_stru *)dmac_netbuf);
    *netbuf = hmacbuf;
    return OAL_SUCC;
}

#ifdef _PRE_LWIP_ZERO_COPY_MEM_ALLOC_PKT_BUF
osal_u32 frw_dmac_netbuf_trans_hmac_netbuf(oal_dmac_netbuf_stru *dmac_netbuf, osal_u16 payload_len,
    oal_netbuf_stru **netbuf)
{
    oal_netbuf_stru *netbuf_tmp = (oal_netbuf_stru *)oal_netbuf_skb(dmac_netbuf);

    memset_s(netbuf_tmp, sizeof(oal_netbuf_stru), 0, sizeof(oal_netbuf_stru));

    netbuf_tmp->mem_head   = OSAL_NULL;
    netbuf_tmp->pkt_buf    = dmac_netbuf;
    dmac_netbuf->pkt_buf_offset = 0;
    netbuf_tmp->head       = (td_u8 *)oal_dmac_netbuf_payload(dmac_netbuf);
    netbuf_tmp->data       = (td_u8 *)netbuf_tmp->head;
    netbuf_tmp->end        = oal_netbuf_dscr(dmac_netbuf) - netbuf_tmp->head;
    oal_netbuf_set_len(netbuf_tmp, payload_len);

    *netbuf = netbuf_tmp;
    return OAL_SUCC;
}
#endif

osal_u32 frw_rx_netbuf(oal_dmac_netbuf_stru *netbuf, osal_u32 payload_len)
{
    osal_u32 ret;
    oal_netbuf_stru *hmac_netbuf = OSAL_NULL;

#ifdef _PRE_LWIP_ZERO_COPY_MEM_ALLOC_PKT_BUF
    ret = frw_dmac_netbuf_trans_hmac_netbuf(netbuf, payload_len, &hmac_netbuf);
#else
    ret = frw_dmac_netbuf_conver_hmac_netbuf(netbuf, payload_len, &hmac_netbuf);
#endif
    if (ret != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_FRW, "[E]frw_dmac_netbuf_conver_hmac_netbuf drop!\n");
        return ret;
    }

    ret = frw_hmac_rcv_netbuf(hmac_netbuf);
    if (ret != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_FRW, "[E]frw_hmac_rcv_netbuf drop!\n");
    }
    return OAL_SUCC;
}
#endif

WIFI_HMAC_TCM_TEXT osal_void hmac_frw_hcc_extend_hdr_init(osal_u16 data_len,
    struct frw_hcc_extend_hdr *hdr, osal_u8 vap_id, osal_u16 msg_id, osal_u8 msg_type)
{
    hdr->msg.msg_id = msg_id;
    hdr->vap_id = vap_id;
    hdr->msg_type = msg_type;
    hdr->data_len = data_len;
    hdr->rsv = 0;
    hdr->pay_type = 0;
}

osal_s32 frw_send_msg_adapt(osal_u16 msg_id, frw_msg *cfg, osal_u8 vap_id, osal_u32 msg_seq, osal_bool sync)
{
#ifdef _PRE_WLAN_FEATURE_CENTRALIZE
    osal_s32 ret;
    // host和device部署在一个CPU上，不需要hcc
    cfg->sync = sync;
    ret = is_msgid_in_device_range(msg_id);
    if (ret != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_FRW, "frw_send_msg_adapt msgid_not_in_device_range");
        return ret;
    }
    return dmac_frw_post_async_msg(msg_id, FRW_POST_PRI_HIGH, vap_id, cfg, msg_seq);
#else
    cfg->sync = sync;
    return frw_send_msg_adapt_by_hcc(msg_id, cfg, vap_id, msg_seq, sync);
#endif
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
