/*
 * Copyright (c) @CompanyNameMagicTag 2020-2021. All rights reserved.
 * Description: host frw tx and rx adapt interface.
 * Author: Huanghe
 * Create: 2020-01-01
 */

/*****************************************************************************
    头文件包
*****************************************************************************/
#include "frw_hmac_adapt.h"
#include "frw_hmac.h"
#include "frw_hmac_hcc_adapt.h"
#include "frw_ext_if.h"
#include "mac_vap_ext.h"
#include "mac_resource_ext.h"
#include "dmac_ext_if_type.h"
#include "dmac_ext_if_hcm.h"
#include "hmac_dfr.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_DEV_FRW_DMAC_ADAPT_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_DEVICE

#ifdef _PRE_WLAN_FEATURE_CENTRALIZE

OAL_STATIC osal_u32 frw_hmac_netbuf_conver_dmac_netbuf(oal_netbuf_stru *netbuf, oal_dmac_netbuf_stru **dmac_netbuf)
{
    oal_dmac_netbuf_stru *dmacbuf = OSAL_NULL;
    osal_u16 len = (osal_u16)OAL_NETBUF_LEN(netbuf);

    dmacbuf = oal_mem_netbuf_alloc(OAL_NORMAL_NETBUF, len, OAL_NETBUF_PRIORITY_HIGH);
    if (dmacbuf == OSAL_NULL) {
        return OAL_FAIL;
    }

    if (memcpy_s(oal_dmac_netbuf_payload(dmacbuf), (osal_u32)len, oal_netbuf_payload(netbuf), (osal_u32)len) != EOK) {
        oal_mem_netbuf_free(dmacbuf);
        return OAL_FAIL;
    }

    /* 转换成功释放host netbuf */
    oal_netbuf_free(netbuf);
    *dmac_netbuf = dmacbuf;
    return OAL_SUCC;
}
#endif

#ifdef _PRE_LWIP_ZERO_COPY_MEM_ALLOC_PKT_BUF
osal_u32 frw_hmac_netbuf_trans_dmac_netbuf(oal_netbuf_stru *netbuf, oal_dmac_netbuf_stru **dmac_netbuf)
{
    oal_dmac_netbuf_stru *pkt_buf = netbuf->pkt_buf;    /* 获取pkt_buf */

    /* 非pkt ram场景 */
    if (pkt_buf == NULL) {
        return frw_hmac_netbuf_conver_dmac_netbuf(netbuf, dmac_netbuf);
    }

    /*  netbuf->data 代表实际的数据存储位置 应该指向80字节开头的位置,
        oal_dmac_netbuf_payload表示的数据应该存储的位置 算出偏移量 */
    pkt_buf->pkt_buf_offset = 0;
    pkt_buf->pkt_buf_offset = oal_netbuf_payload(netbuf) - oal_dmac_netbuf_payload(pkt_buf);

    if (pkt_buf->pkt_buf_offset < 0) {
        return frw_hmac_netbuf_conver_dmac_netbuf(netbuf, dmac_netbuf);
    }

    *dmac_netbuf = pkt_buf;
    return OAL_SUCC;
}
#endif

WIFI_HMAC_TCM_TEXT osal_u32 frw_hmac_send_data_adapt(oal_netbuf_stru *netbuf, osal_u8 data_type)
{
    unref_param(data_type);
#ifndef _PRE_WLAN_FEATURE_CENTRALIZE
    return frw_tx_wifi_netbuf_by_hcc(netbuf, OAL_NETBUF_LEN(netbuf), data_type);
#else
    osal_u32 ret;
    oal_dmac_netbuf_stru* dmac_netbuf = OSAL_NULL;

    oal_netbuf_push(netbuf, OAL_HCC_HDR_LEN);
#ifdef _PRE_LWIP_ZERO_COPY_MEM_ALLOC_PKT_BUF
    ret = frw_hmac_netbuf_trans_dmac_netbuf(netbuf, &dmac_netbuf);
#else
    ret = frw_hmac_netbuf_conver_dmac_netbuf(netbuf, &dmac_netbuf);
#endif
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_FRW, "frw_hmac_netbuf_conver_dmac_netbuf drop:%u!", ret);
        return ret;
    }

    ret = frw_dmac_rcv_mgnt_data(dmac_netbuf);
    if (ret != OAL_SUCC) {
        /* 不返回异常给调用者，否则netbuf会重复释放，dmac_netbuf在函数内部释放 */
        oam_warning_log1(0, OAM_SF_FRW, "frw_dmac_rcv_mgnt_data drop:%u!", ret);
    }

    return OAL_SUCC;
#endif
}

OAL_STATIC osal_u8 frw_hmac_update_data_type_al_tx(osal_u8 data_type)
{
    osal_u8 send_data_type = data_type;
#ifdef _PRE_WLAN_FEATURE_ALWAYS_TX
    // 常发使用管理帧队列 不考虑流控
    hal_to_dmac_device_stru *device = hal_chip_get_hal_device();
    if (device->al_tx_flag != 0) {
        send_data_type = HCC_MSG_TYPE_MGMT_FRAME;
    }
#endif
    return send_data_type;
}

OAL_STATIC osal_u32 frw_hmac_mac_head_exchange(oal_netbuf_stru *netbuf, mac_tx_ctl_stru *tx_ctl, osal_u16 *data_len)
{
    osal_s32 ret;
    osal_u32 padding_len = 0;
    padding_len = OAL_MAX_MAC_HDR_LEN - tx_ctl->frame_header_length;
    if (mac_pk_mode(netbuf)) {
#ifdef _PRE_WLAN_FEATURE_WS53
        if (padding_len > 0) {
            ret = memmove_s(oal_netbuf_data(netbuf) - tx_ctl->frame_header_length, tx_ctl->frame_header_length,
                oal_netbuf_data(netbuf) - OAL_MAX_MAC_HDR_LEN, tx_ctl->frame_header_length);
            if (ret != EOK) {
                wifi_printf("frw_hmac_send_data::memmove fail[%d].\r\n", ret);
                return OAL_FAIL;
            }
        }
#endif
        *data_len = (osal_u16)(OAL_NETBUF_LEN(netbuf));
        oal_netbuf_push(netbuf, OAL_MAX_MAC_HDR_LEN);
    } else {
        *data_len = (osal_u16)(OAL_NETBUF_LEN(netbuf)) - tx_ctl->frame_header_length;
        oal_netbuf_push(netbuf, padding_len);
#ifndef _PRE_WLAN_FEATURE_WS53
        if (padding_len > 0) {
            ret = memmove_s(oal_netbuf_data(netbuf), tx_ctl->frame_header_length,
                oal_netbuf_data(netbuf) + padding_len, tx_ctl->frame_header_length);
            if (ret != EOK) {
                wifi_printf("frw_hmac_send_data::memmove fail[%d].\r\n", ret);
                return OAL_FAIL;
            }
        }
#endif
    }
    return OAL_SUCC;
}

WIFI_HMAC_TCM_TEXT osal_u32 frw_hmac_send_data(oal_netbuf_stru *netbuf, osal_u8 vap_id, osal_u8 data_type)
{
    osal_s32 ret;
    osal_u16 data_len = 0;
    osal_u8 send_data_type = frw_hmac_update_data_type_al_tx(data_type);
    mac_tx_ctl_stru *tx_ctl;
    if (netbuf == NULL || ((mac_tx_ctl_stru *)oal_netbuf_cb(netbuf))->frame_header_length > OAL_MAX_MAC_HDR_LEN) {
        return OAL_FAIL;
    }
    tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(netbuf);
#ifdef _PRE_WLAN_FEATURE_DFR
    /* 如果是在自愈流程且设置了禁用hcc消息,此处直接返回succ,避免超时后返回err自愈流程无法走完 */
    if (hmac_dfr_get_reset_process_flag()) {
        return OAL_FAIL;
    }
#endif
    if (frw_hmac_mac_head_exchange(netbuf, tx_ctl, &data_len) != OAL_SUCC) {
        return OAL_FAIL;
    }
    /* memcpy cb */
    oal_netbuf_push(netbuf, OAL_MAX_CB_LEN);
    ret = memcpy_s(oal_netbuf_data(netbuf), oal_netbuf_get_len(netbuf), netbuf->cb, OAL_MAX_CB_LEN);
    if (ret != EOK) {
        wifi_printf("frw_hmac_send_data::memcpy_s fail[%d].\r\n", ret);
        return OAL_FAIL;
    }

    /* init frw header */
    oal_netbuf_push(netbuf, frw_get_head_len());
    hmac_frw_hcc_extend_hdr_init(data_len, (struct frw_hcc_extend_hdr *)oal_netbuf_data(netbuf),
        vap_id, 0, send_data_type);

    return frw_hmac_send_data_adapt(netbuf, send_data_type);
}
