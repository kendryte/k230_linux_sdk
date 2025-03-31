/*
 * Copyright (c) CompanyNameMagicTag 2021-2021. All rights reserved.
 * Description: Function related to wifi aware(NAN) feature.
 * Author: Huanghe
 * Create: 2021-04-16
 */

#include "hmac_sdp.h"
#include "hmac_mgmt_bss_comm.h"
#include "wlan_msg.h"
#include "frw_hmac.h"
#include "hmac_feature_dft.h"

#include "dmac_ext_if_hcm.h"
#include "hmac_tx_mgmt.h"
#include "hmac_fcs.h"
#include "mac_data.h"
#include "mac_device_ext.h"
#include "hal_ext_if.h"
#include "oal_netbuf_data.h"
#include "msg_sdp_rom.h"
#include "hmac_feature_interface.h"
#include "hmac_ccpriv.h"
#include "mac_vap_ext.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_HMAC_SDP_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

#define NAN_WAKE_PRE_TIME 8
#define NAN_2G_CHAN 6

OAL_STATIC hmac_sdp_service_info g_service_info;
OAL_STATIC oal_bool_enum g_sdp_init = OAL_FALSE;

OAL_STATIC osal_u8 hmac_sdp_get_service_num_by_type(osal_u8 type)
{
    osal_u32 index;
    osal_u8 num = 0;
    oal_sdp_service *array = g_service_info.info;
    osal_u8 size = (osal_u8)osal_array_size(g_service_info.info);

    for (index = 0; index < size; index++) {
        if ((array[index].type & type) != 0) {
            num++;
        }
    }
    return num;
}

OAL_STATIC osal_u8 hmac_sdp_service_max_size(osal_u8 type)
{
    if ((type & OAL_SDP_SERVICE_TYPE_PUBLISH) != 0) {
        return HMAC_MAX_PUBLISH_NUM;
    } else if ((type & OAL_SDP_SERVICE_TYPE_SUBSCRIBE) != 0) {
        return HMAC_MAX_SUBSCRIBE_NUM;
    } else {
        return 0;
    }
}

OAL_STATIC osal_u32 hmac_sdp_save_service_info(const oal_sdp_service *src_srv)
{
    osal_u8 index;
    oal_sdp_service *array = g_service_info.info;
    osal_u8 array_size = (osal_u8)osal_array_size(g_service_info.info);
    osal_u8 match_index = array_size;
    osal_u8 null_index = array_size;
    osal_u8 max_size;
    osal_u8 current_num;

    /* 目前支持的handle是 1-255 */
    if ((src_srv->local_handle < 1) || (src_srv->local_handle > 255)) {
        oam_warning_log1(0, OAM_SF_SDP,
            "hmac_sdp_save_service_info:can not add new service, src_srv->local_handle[%u]", src_srv->local_handle);
        return OAL_FAIL;
    }
    for (index = 0; index < array_size; index++) {
        if (((array[index].type & src_srv->type) != 0) && (array[index].local_handle == src_srv->local_handle)) {
            match_index = index;
            break;
        }
        if ((array[index].type == OAL_SDP_SERVICE_TYPE_DEFAULT) && (null_index == array_size)) {
            null_index = index;
        }
    }
    /* 未匹配到服务则保存到空的空间 */
    if (match_index == array_size) {
        max_size = hmac_sdp_service_max_size(src_srv->type);
        current_num = hmac_sdp_get_service_num_by_type(src_srv->type);
        /* 当前服务类型已达其最大个数或无空位 */
        if ((current_num >= max_size) || (null_index == array_size)) {
            oam_warning_log2(0, OAM_SF_SDP,
                "hmac_sdp_save_service_info:can not add new service, current[%d], max_size[%d]", current_num, max_size);
            return OAL_FAIL;
        } else {
            match_index = null_index;
        }
    }

    /* 有未使用服务空间则保存 */
    if (memcpy_s(&array[match_index], sizeof(oal_sdp_service), src_srv, sizeof(oal_sdp_service)) == EOK) {
        oam_info_log3(0, OAM_SF_SDP, "hmac_sdp_save_service_info:name[0]=%02x, local_handle=%d, type=%d",
            array[match_index].name[0], array[match_index].local_handle, array[match_index].type);
        return OAL_SUCC;
    } else {
        oam_warning_log1(0, OAM_SF_SDP, "hmac_sdp_save_service_info:memcpy_s failed, name[0]=%02x", src_srv->name[0]);
        return OAL_FAIL;
    }
}

/* 启动订阅操作,用于侦听某一服务信息，入参包含服务名称哈希值和订阅成功之后分配的本端instance id */
osal_s32 hmac_sdp_start_subscribe(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    oal_sdp_service *service_info = OSAL_NULL;

    if ((hmac_vap == OSAL_NULL) || (msg->data == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_CFG, "{hmac_sdp_start_subscribe::param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (hmac_sdp_is_init() != OAL_TRUE) {
        return OAL_FAIL;
    }

    service_info = (oal_sdp_service *)msg->data;
    if (hmac_sdp_save_service_info(service_info) != OAL_SUCC) {
        return OAL_FAIL;
    }

    oam_warning_log1(0, OAM_SF_CFG, "{hmac_sdp_start_subscribe::local_handle %d.}", service_info->local_handle);
    return OAL_SUCC;
}

OAL_STATIC osal_u32 hmac_sdp_clear_service_info(osal_u8 local_handle, osal_u8 type)
{
    osal_u32 index;
    oal_sdp_service *array = g_service_info.info;
    osal_u8 size = (osal_u8)osal_array_size(g_service_info.info);

    for (index = 0; index < size; index++) {
        if (((array[index].type & type) != 0) && (array[index].local_handle == local_handle)) {
            oam_warning_log3(0, OAM_SF_SDP, "hmac_sdp_clear_service_info set local_handle =%d, index %d, size %d\r\n",
                local_handle, index, size);
            (osal_void)memset_s((osal_void *)&array[index], sizeof(array[index]), 0, sizeof(array[index]));
            break;
        }
    }
    if (index >= size) {
        oam_warning_log3(0, OAM_SF_SDP, "hmac_sdp_clear_service_info not find local_handle =%d, index %d, size %d\r\n",
            local_handle, index, size);
        return OAL_FAIL;
    }
    return OAL_SUCC;
}

/* 取消订阅操作，入参包含待取消的服务的本端instance id */
osal_s32 hmac_sdp_cancel_subscribe(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    osal_u8 local_handle = 0;
    if (msg->data == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_CFG, "{hmac_sdp_cancel_subscribe::param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    local_handle = msg->data[0];
    if ((hmac_vap == OSAL_NULL) || (hmac_sdp_is_init() != OAL_TRUE)) {
        return OAL_FAIL;
    }
    oam_warning_log1(0, OAM_SF_SDP, "hmac_sdp_cancel_subscribe local_handle %d", local_handle);
    if (hmac_sdp_clear_service_info(local_handle, OAL_SDP_SERVICE_TYPE_SUBSCRIBE) != OAL_SUCC) {
        return OAL_FAIL;
    }
    return OAL_SUCC;
}

/* 设置dw 和 prd time, prd_pre_time */
osal_s32 hmac_sdp_dw_prd_time_cfg(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    frw_msg msg_info;
    oal_sdp_dw_prd_time_info *time_info = OSAL_NULL;

    /* 为了支持在 sdp初始化之前配置两个全局变量的值, 此处不判断 hmac_sdp_is_init是否为true */
    if ((hmac_vap == OSAL_NULL) || (msg->data == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_CFG, "{hmac_sdp_dw_prd_time_cfg::param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    time_info = (oal_sdp_dw_prd_time_info *)msg->data;
    (osal_void)memset_s(&msg_info, OAL_SIZEOF(msg_info), 0, OAL_SIZEOF(msg_info));
    msg_info.data = (osal_u8 *)time_info;
    msg_info.data_len = (osal_u16)sizeof(oal_sdp_dw_prd_time_info);
    return send_cfg_to_device(hmac_vap->vap_id, WLAN_MSG_H2D_C_CFG_SDP_DW_PRD_TIME_CFG, &msg_info, OSAL_TRUE);
}

/* 发送publish报文的mac给device, 过滤时间同步beacon 或 私有action帧 */
osal_s32 hmac_sdp_add_peer_mac(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    osal_s32 ret;
    oal_sdp_peer_mac_handle *info = OSAL_NULL;
    frw_msg msg_info;

    if ((hmac_vap == OSAL_NULL) || (hmac_sdp_is_init() != OAL_TRUE)) {
        return OAL_FAIL;
    }
    if (msg->data == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_CFG, "{hmac_sdp_add_peer_mac::param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    info = (oal_sdp_peer_mac_handle *)msg->data;
    (osal_void)memset_s(&msg_info, OAL_SIZEOF(msg_info), 0, OAL_SIZEOF(msg_info));
    msg_info.data = (osal_u8 *)info;
    msg_info.data_len = (osal_u16)sizeof(oal_sdp_peer_mac_handle);
    ret = send_cfg_to_device(hmac_vap->vap_id, WLAN_MSG_H2D_C_CFG_SDP_ADD_PEER_MAC, &msg_info, OSAL_TRUE);
    oam_warning_log1(0, OAM_SF_SDP, "hmac_sdp_add_peer_mac ret %d", ret);
    return ret;
}

OAL_STATIC osal_u32 hmac_sdp_init_send_info(hmac_vap_stru *hmac_vap, osal_bool switch_channel,
    osal_u16 peroid, osal_u8 ratio)
{
    mac_sdp_init_info init_info;
    frw_msg msg_info;

    (osal_void)memset_s(&init_info, OAL_SIZEOF(init_info), 0, OAL_SIZEOF(init_info));
    init_info.is_init = (osal_u8)hmac_sdp_is_init();
    init_info.tx_enable = OAL_TRUE;
    init_info.publish_cnt = HMAC_MAX_PUBLISH_NUM;
    init_info.subscribe_cnt = HMAC_MAX_SUBSCRIBE_NUM;
    init_info.timer_enable = switch_channel;
    if (switch_channel) { /* 待拆分初始化命令后 更新此处代码 */
        /* sta已联网状态下, 配置时间窗口接收publish和时间同步报文 */
        init_info.init_period_ms = peroid; /* 默认 200ms */
        init_info.init_dw_ms = (osal_u16)(init_info.init_period_ms * ratio) / 10; /* 10 用于计算百分比 */
        oam_warning_log3(0, OAM_SF_CFG, "{hmac_sdp_init_send_info:peroid[%d] dw_ms[%d] ratio[%d]",
            init_info.init_period_ms, init_info.init_dw_ms, ratio);
    }
    init_info.pre_dw = NAN_WAKE_PRE_TIME;
    init_info.sw_chan.chan_number = NAN_2G_CHAN;
    init_info.sw_chan.band = WLAN_BAND_2G;
    init_info.sw_chan.en_bandwidth = WLAN_BAND_WIDTH_20M;
    (osal_void)hmac_get_channel_idx_from_num_etc(init_info.sw_chan.band, init_info.sw_chan.chan_number,
        &init_info.sw_chan.chan_idx);

    (osal_void)memset_s(&msg_info, OAL_SIZEOF(msg_info), 0, OAL_SIZEOF(msg_info));
    msg_info.data = (osal_u8 *)&init_info;
    msg_info.data_len = (osal_u16)sizeof(init_info);

    return (osal_u32)send_cfg_to_device(hmac_vap->vap_id, WLAN_MSG_H2D_C_CFG_SDP_INIT, &msg_info, OSAL_TRUE);
}

osal_s32 hmac_sdp_init_enable(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    osal_u16 peroid = 524; /* 524 周期 */
    osal_u8 ratio = 5; /* 待适配,默认5,表示50%比例,目前都未用该接口,要么就用sdp test,要么有上层APP场景下用其他接口 */
    osal_u8 value;
    if ((hmac_vap == OSAL_NULL) || (msg->data == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_CFG, "{hmac_sdp_init_enable::param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    value = *(msg->data);

    if (value == HMAC_SDP_ENABLE) {
        return (osal_s32)hmac_sdp_init(hmac_vap, OAL_FALSE, peroid, &ratio);
    } else if (value == HMAC_SDP_EN_SC) {
        return (osal_s32)hmac_sdp_init(hmac_vap, OAL_TRUE, peroid, &ratio);
    } else {
        return (osal_s32)hmac_sdp_deinit(hmac_vap);
    }
}

oal_bool_enum hmac_sdp_is_init(osal_void)
{
    return g_sdp_init;
}

/* SDP功能初始化，初始化时创建定时器并在DMAC设置寄存器允许侦听非本BSS的多播帧 */
osal_u32 hmac_sdp_init(hmac_vap_stru *hmac_vap, osal_bool switch_channel, osal_u16 peroid, osal_u8 *input_ratio)
{
    osal_u32 ret;
    osal_u8 ratio;
    if ((hmac_vap == OSAL_NULL) || (input_ratio == OSAL_NULL)) {
        return OAL_FAIL;
    }

    if (hmac_sdp_is_init() == OAL_TRUE) {
        oam_warning_log0(0, OAM_SF_SDP, "sdp is already init now");
        return OAL_FAIL;
    }
    (osal_void)memset_s((osal_void *)&g_service_info, sizeof(g_service_info), 0, sizeof(g_service_info));
    g_sdp_init = OAL_TRUE;

    ratio = *input_ratio;
    if ((ratio < 2) || (ratio > 8)) { /* 2-8 表示比例20%-80% 预留时间用于信道切换涉及的操作 */
        oam_warning_log1(0, OAM_SF_SDP, "hmac_sdp_init ratio[%d] not valid, use default value 5", ratio);
        wifi_printf("hmac_sdp_init ratio[%d] not valid[2-8], use default value 5\r\n", ratio);
        ratio = 5; /* 5表示缺省50% */
        *input_ratio = ratio;
    }
    ret = hmac_sdp_init_send_info(hmac_vap, switch_channel, peroid, ratio);
    oam_info_log1(0, OAM_SF_SDP, "hmac_sdp_init return[%d]", ret);
    if (ret == OAL_SUCC) {
        /* 协议低功耗会影响收包,   深睡会断电,两者均会影响sdp业务;  不进协议低功耗不会进深睡 */
#ifdef _PRE_WLAN_FEATURE_STA_PM
        ret = hmac_config_set_pm_by_module_etc(hmac_vap, MAC_STA_PM_CTRL_TYPE_SDP, MAC_STA_PM_SWITCH_OFF);
        if (ret != OAL_SUCC) {
            oam_warning_log2(0, OAM_SF_CFG,
                "vap_id[%d] {hmac_sdp_init::hmac_config_set_pm_by_module_etc failed[%d].}", hmac_vap->vap_id, ret);
        }
#endif
    }
    return ret;
}

/* SDP功能去初始化，清除定时器并还原DMAC寄存器配置 */
osal_u32 hmac_sdp_deinit(hmac_vap_stru *hmac_vap)
{
    osal_u32 ret;
    if (hmac_vap == OSAL_NULL) {
        return OAL_FAIL;
    }

    if (hmac_sdp_is_init() == OAL_FALSE) {
        oam_info_log0(0, OAM_SF_SDP, "sdp is not init now");
        return OAL_FAIL;
    }
    (osal_void)memset_s((osal_void *)&g_service_info, sizeof(g_service_info), 0, sizeof(g_service_info));
    g_sdp_init = OAL_FALSE;

    ret = hmac_sdp_init_send_info(hmac_vap, OAL_FALSE, 0, 0);
    oam_info_log1(0, OAM_SF_SDP, "hmac_sdp_deinit return[%d]", ret);

    if (ret == OAL_SUCC) {
        /* 协议低功耗会影响收包,   深睡会断电,两者均会影响sdp业务;  不进协议低功耗不会进深睡 */
#ifdef _PRE_WLAN_FEATURE_STA_PM
        osal_u8 pm_switch = (hmac_vap->sta_pm_handler.last_ps_status != 0) ?
            MAC_STA_PM_SWITCH_ON : MAC_STA_PM_SWITCH_RESET;
        ret = hmac_config_set_pm_by_module_etc(hmac_vap, MAC_STA_PM_CTRL_TYPE_SDP, pm_switch);
        if (ret != OAL_SUCC) {
            oam_warning_log3(0, OAM_SF_CFG, "vap[%d]{hmac_sdp_deinit:hmac_config_set_pm_by_module_etc pm[%d] fail[%d]}",
                hmac_vap->vap_id, pm_switch, ret);
        }
#endif
    }

    return ret;
}

/* 填充SDP followup帧除了action头之后的帧体内容 */
OAL_STATIC osal_void hmac_sdp_fill_followup_content(osal_u8 *data_input, const oal_sdp_info *sdp_info, osal_u16 *len)
{
    /* 未发布服务前提供一个默认值 */
    osal_u8 hash_tmp[SDP_HASH_SERVICE_NAME_LEN] = { 1, 2, 3, 4, 5, 6 };
    /* Service Control配置0x12：follow-up类型报文，且有Service Info */
    osal_u8 svc_ctrl = 0x12;
    osal_u8 index;
    osal_u16 desc_len;
    osal_u8 *hash_name = OSAL_NULL;
    osal_u8 size = (osal_u8)osal_array_size(g_service_info.info);
    osal_u8 type = OAL_SDP_SERVICE_TYPE_PUBLISH | OAL_SDP_SERVICE_TYPE_GET_PUBLISH;
    oal_sdp_service *array = g_service_info.info;
    osal_u8 *data = data_input;
    hmac_sdp_descriptor_attr attr;

    for (index = 0; index < size; index++) {
        if (((array[index].type & type) != 0) && (array[index].local_handle == sdp_info->local_handle)) {
            hash_name = array[index].name;
            break;
        }
    }
    if (hash_name == OSAL_NULL) {
        oam_warning_log1(0, OAM_SF_SDP, "{hmac_sdp_fill_followup_content::no service for handle %d!}",
            sdp_info->local_handle);
        hash_name = hash_tmp;
    }

    desc_len = sdp_info->len + HMAC_SDP_NO_DATA_LEN;
    (osal_void)memset_s((osal_void *)&attr, sizeof(attr), 0, sizeof(attr));
    attr.attr_id = SERVICE_DESCRIPTOR_ATTR_ID;
    attr.attr_len_low = (osal_u8)desc_len;
    attr.attr_len_high = (osal_u8)(desc_len >> 8); /* 长度占两个字节，右移8位写第二个字节 */
    *len = 0;
    if (memcpy_s(attr.service_name, sizeof(attr.service_name), hash_name, SDP_HASH_SERVICE_NAME_LEN) != EOK) {
        oam_error_log0(0, OAM_SF_SDP, "hmac_sdp_fill_followup_content::memcpy_s name failed");
        return;
    }
    attr.local_handle = sdp_info->local_handle;
    attr.peer_handle = sdp_info->peer_handle;
    attr.svc_ctrl = svc_ctrl;
    attr.info_len = sdp_info->len;
    if (memcpy_s(data, sizeof(hmac_sdp_descriptor_attr), &attr, sizeof(hmac_sdp_descriptor_attr)) != EOK) {
        oam_error_log0(0, OAM_SF_SDP, "hmac_sdp_fill_followup_content::memcpy_s attr failed");
        return;
    }
    data += sizeof(hmac_sdp_descriptor_attr);

    if (memcpy_s(data, sdp_info->len, sdp_info->data, sdp_info->len) != EOK) {
        oam_error_log0(0, OAM_SF_SDP, "hmac_sdp_fill_followup_content::memcpy_s data failed");
        return;
    }
    *len = desc_len + MAC_ATTR_HDR_LEN;
    return;
}

/* 封装SDP Action帧，包括Publish和Follow up帧类型，is_publish标示是哪种帧 */
OAL_STATIC osal_u16 hmac_sdp_encap_action(const hmac_vap_stru *hmac_vap, oal_netbuf_stru *mgmt_buf,
    const oal_sdp_info *sdp_info, osal_u8 is_publish)
{
    /* sdp 帧头格式：Category:0x04,action:0x09,OUI:0x50-6F-9A,OUI Type:0x13 */
    osal_u8 action_hdr_fixed[] = { 0x4, 0x9, 0x50, 0x6F, 0x9A, 0x13 };
    osal_u8 mcast_mac[ETH_ALEN] = { 0x51, 0x6F, 0x9A, 0x01, 0x00, 0x00 };
    osal_u8 bssid[ETH_ALEN] = { 0x50, 0x6F, 0x9A, 0x01, 0xFF, 0x00 };
    osal_u8 *data = (osal_u8 *)oal_netbuf_header(mgmt_buf);
    mac_tx_ctl_stru *tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(mgmt_buf);
    const osal_u8 *dst_mac = OSAL_NULL;
    sdp_action_header *action_hdr = OSAL_NULL;
    osal_u8 *payload_addr = OSAL_NULL;
    osal_u16 content_len = 0;

    mac_hdr_set_frame_control(data, WLAN_PROTOCOL_VERSION | WLAN_FC0_TYPE_MGT | WLAN_FC0_SUBTYPE_ACTION);

    /* 设置分片序号为0 */
    mac_hdr_set_fragment_number(data, 0);

    /* 设置地址1，对端地址 */
    dst_mac = (is_publish == OAL_TRUE) ? mcast_mac : sdp_info->name;
    if (memcpy_s(data + WLAN_HDR_ADDR1_OFFSET, WLAN_MAC_ADDR_LEN, dst_mac, WLAN_MAC_ADDR_LEN) != EOK) {
        return 0;
    }

    /* 设置本端的MAC地址 */
    if (memcpy_s(data + WLAN_HDR_ADDR2_OFFSET, WLAN_MAC_ADDR_LEN,
        hmac_vap->mib_info->wlan_mib_sta_config.dot11_station_id, WLAN_MAC_ADDR_LEN) != EOK) {
        return 0;
    }
    /* 地址3, BSSID */
    if (memcpy_s(data + WLAN_HDR_ADDR3_OFFSET, WLAN_MAC_ADDR_LEN, bssid, WLAN_MAC_ADDR_LEN) != EOK) {
        return 0;
    }

    action_hdr = (sdp_action_header *)(data + MAC_80211_FRAME_LEN); /* 取action帧体 */
    if (memcpy_s(action_hdr, sizeof(action_hdr_fixed), action_hdr_fixed, sizeof(action_hdr_fixed)) != EOK) {
        return 0;
    }
    payload_addr = (osal_u8 *)(action_hdr + 1);

    /* 以下为填充SDP除action头之外帧体内容 */
    hmac_sdp_fill_followup_content(payload_addr, sdp_info, &content_len);
    if (content_len == 0) {
        return 0;
    }

    /* 计算帧的总长度,设置相关发送配置 */
    mac_get_cb_frame_header_length(tx_ctl) = MAC_80211_FRAME_LEN;
    mac_get_cb_tx_user_idx(tx_ctl) = HMAC_SDP_TX_UESR_ID; /* 标识发送完成需要获取user结构体 */
    tx_ctl->need_rsp = OAL_FALSE;
    tx_ctl->is_needretry = OAL_FALSE;
    tx_ctl->ismcast = is_publish;
    tx_ctl->tpc_code = hal_get_tpc_code();
    tx_ctl->tpc_code_is_valid = OAL_TRUE;
    mac_get_cb_mpdu_len(tx_ctl) = MAC_80211_FRAME_LEN + (osal_u8)sizeof(sdp_action_header) + content_len;
    return mac_get_cb_mpdu_len(tx_ctl);
}

/* 按照属性ID在报文结构中查找对应属性 */
OAL_STATIC osal_u8 *hmac_sdp_find_attribute(osal_u8 attr_id, osal_u8 *attr_ies_input, osal_u32 len_input)
{
    osal_u32 len = len_input;
    osal_u8 *attr_ies = attr_ies_input;
    if (attr_ies == OSAL_NULL) {
        return OSAL_NULL;
    }
    while (len > MAC_ATTR_HDR_LEN && attr_ies[0] != attr_id) {
        if (len < (osal_u32)((*(osal_u16 *)&attr_ies[1]) + MAC_ATTR_HDR_LEN)) {
            break;
        }
        len -= (*(osal_u16 *)&attr_ies[1]) + MAC_ATTR_HDR_LEN;
        attr_ies += (*(osal_u16 *)&attr_ies[1]) + MAC_ATTR_HDR_LEN;
    }
    if ((len < MAC_ATTR_HDR_LEN) || (len < (osal_u32)(MAC_ATTR_HDR_LEN + (*(osal_u16 *)&attr_ies[1]))) ||
        ((len == MAC_ATTR_HDR_LEN) && (attr_ies[0] != attr_id))) {
        return OSAL_NULL;
    }
    return attr_ies;
}

/* 将底层DMAC接收到的SDP数据信息发送给上层供应用程序解析 */
OAL_STATIC osal_u32 hmac_sdp_send_data_to_host(const hmac_vap_stru *hmac_vap, const oal_sdp_info *sdp_param)
{
    frw_msg msg_info = {0};

    msg_info.data = (osal_void *)sdp_param;
    msg_info.data_len = (osal_u16)OAL_SIZEOF(oal_sdp_info);

    return (osal_u32)frw_asyn_host_post_msg(WLAN_MSG_H2W_SDP_RX_DATA, FRW_POST_PRI_LOW, hmac_vap->vap_id, &msg_info);
}

OAL_STATIC osal_u32 hmac_sdp_get_publish_local_id(const osal_u8 *name, osal_u8 *local_handle)
{
    osal_u32 index;
    oal_sdp_service *array = g_service_info.info;
    osal_u8 size = (osal_u8)osal_array_size(g_service_info.info);

    for (index = 0; index < size; index++) {
        if (((array[index].type & OAL_SDP_SERVICE_TYPE_SUBSCRIBE) != 0) &&
            (osal_memcmp(array[index].name, name, SDP_HASH_SERVICE_NAME_LEN) == 0)) {
            array[index].type |= OAL_SDP_SERVICE_TYPE_GET_PUBLISH;
            *local_handle = array[index].local_handle;
            return OAL_SUCC;
        }
    }

    return OAL_FAIL;
}

/* 将收到的followup报文相关信息通知到上层 */
osal_u32 hmac_sdp_send_followup_to_host(const hmac_vap_stru *hmac_vap, oal_sdp_info *sdp_info, osal_u8 *data,
    osal_u8 len)
{
    osal_u32 ret;

    /* 此处申请内存注意释放，由wal_sdp_proc_rx_data释放 */
    sdp_info->data = (osal_u8 *)oal_mem_alloc(OAL_MEM_POOL_ID_LOCAL, len + 1, OAL_TRUE);
    if (sdp_info->data == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_SDP, "{hmac_sdp_send_followup_to_host:alloc mem failed!}");
        return OAL_FAIL;
    }

    if (memcpy_s(sdp_info->data, len, data, len) != EOK) {
        oam_error_log0(0, OAM_SF_SDP, "hmac_sdp_send_followup_to_host:action_hdr memcpy fail.");
        oal_mem_free(sdp_info->data, OAL_TRUE);
        return OAL_FAIL;
    }

    sdp_info->len = len;
    sdp_info->data[len] = 0;
    /* 打印服务名第0,3位，方便定位问题 */
    oam_info_log2(0, OAM_SF_SDP, "{recv sdp data::peer XX:%d:%d}", sdp_info->name[0], sdp_info->name[3]);

    ret = hmac_sdp_send_data_to_host(hmac_vap, sdp_info);
    if (ret != OAL_SUCC) {
        oal_mem_free(sdp_info->data, OAL_TRUE);
        sdp_info->data = OSAL_NULL;
        oam_warning_log0(0, OAM_SF_SDP, "{hmac_sdp_send_followup_to_host::send data to host failed!}");
        return ret;
    }
    return OAL_SUCC;
}

osal_u8 *hmac_sdp_parse_rx(oal_netbuf_stru *netbuf, oal_sdp_info *sdp_param)
{
    dmac_rx_ctl_stru *rx_ctrl = OSAL_NULL;
    mac_rx_ctl_stru *rx_info = OSAL_NULL;
    mac_ieee80211_frame_stru *frame_hdr = OSAL_NULL;
    sdp_action_header *action_hdr = OSAL_NULL;
    osal_u8 *data = OSAL_NULL;
    osal_u8 *desc_attr = OSAL_NULL;
    osal_u8 sdp_type;

    if ((netbuf == OSAL_NULL) || (sdp_param == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_SDP, "hmac_sdp_parse_rx:parameter NULL.");
        return OSAL_NULL;
    }

    /* 获取帧头和帧体指针 */
    rx_ctrl = (dmac_rx_ctl_stru *)oal_netbuf_cb(netbuf);
    rx_info = (mac_rx_ctl_stru *)(&(rx_ctrl->rx_info));
    frame_hdr = (mac_ieee80211_frame_stru *)(uintptr_t)mac_get_rx_cb_mac_header_addr(rx_info);
    data = (osal_u8 *)frame_hdr + rx_info->mac_header_len;
    action_hdr = (sdp_action_header *)data;
    desc_attr = hmac_sdp_find_attribute(SERVICE_DESCRIPTOR_ATTR_ID, (osal_u8 *)(action_hdr + 1),
        (rx_info->frame_len - rx_info->mac_header_len - sizeof(sdp_action_header)));
    if (desc_attr == OSAL_NULL) {
        oam_warning_log0(0, OAM_SF_SDP, "hmac_sdp_parse_rx:frame received has no service descriptor!");
        return OSAL_NULL;
    }

    desc_attr += MAC_ATTR_HDR_LEN;
    sdp_param->peer_handle = desc_attr[HMAC_SDP_POS_PEER_HANDLE];
    sdp_param->local_handle = desc_attr[HMAC_SDP_POS_LOCAL_HANDLE];
    sdp_type = desc_attr[HMAC_SDP_POS_TYPE] & HMAC_SDP_TYPE_MASK;

    /* 此处name临时用来保存publish报文的源mac地址, 从结构体成员定义来看是保存服务号的 */
    if (memcpy_s(sdp_param->name, sizeof(sdp_param->name), frame_hdr->address2, ETH_ALEN) != EOK) {
        oam_error_log0(0, OAM_SF_SDP, "hmac_sdp_parse_rx:address2 memcpy fail.");
        return OSAL_NULL;
    }
    /* 接收到publish报文，获取local id */
    if (sdp_type == HMAC_SDP_TYPE_PUBLISH) {
        if (hmac_sdp_get_publish_local_id(desc_attr, &sdp_param->local_handle) != OAL_SUCC) {
            oam_warning_log1(0, OAM_SF_SDP, "hmac_sdp_parse_rx:can not get publish id name[0]=%02x!", desc_attr[0]);
            return OSAL_NULL;
        }
        oam_info_log2(0, OAM_SF_SDP, "hmac_sdp_rx_publish peer_handle=%d, local_handle=%d", sdp_param->peer_handle,
            sdp_param->local_handle);
    } else if (sdp_type == HMAC_SDP_TYPE_FOLLOWUP) {
        oam_info_log2(0, OAM_SF_SDP, "hmac_sdp_rx_follow peer_handle=%d, local_handle=%d", sdp_param->peer_handle,
            sdp_param->local_handle);
    }
    return desc_attr;
}

/* 发送SDP数据帧给对端，封装在Follow up报文中 */
osal_s32 hmac_sdp_send_data(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    oal_netbuf_stru *netbuf = OSAL_NULL;
    oal_sdp_info *tmp = (oal_sdp_info *)msg->data;
    osal_u32 ret;
    osal_u16 mgmt_len;

    if ((hmac_vap == OSAL_NULL) || (hmac_sdp_is_init() != OAL_TRUE)) {
        return OAL_FAIL;
    }

    /* 申请管理帧内存 */
    netbuf = OAL_MEM_NETBUF_ALLOC(OAL_MGMT_NETBUF, WLAN_MGMT_NETBUF_SIZE, OAL_NETBUF_PRIORITY_HIGH);
    if (netbuf == OSAL_NULL) {
        oam_warning_log0(0, OAM_SF_SDP, "{hmac_sdp_send_data::alloc netbuf failed.}");
        oal_mem_free(tmp->data, OAL_TRUE);
        return OAL_ERR_CODE_PTR_NULL;
    }

    oal_set_netbuf_prev(netbuf, OSAL_NULL);
    oal_set_netbuf_next(netbuf, OSAL_NULL);
    if (memset_s(oal_netbuf_cb(netbuf), OAL_NETBUF_CB_SIZE(), 0, OAL_NETBUF_CB_SIZE()) != EOK) {
        hmac_dft_print_drop_frame_info(THIS_FILE_ID, __LINE__, 1, OSAL_NULL);
        oal_netbuf_free(netbuf);
        oal_mem_free(tmp->data, OAL_TRUE);
        return OAL_FAIL;
    }

    /* 封装Action帧 */
    mgmt_len = hmac_sdp_encap_action(hmac_vap, netbuf, tmp, OAL_FALSE);
    if (mgmt_len == 0) {
        hmac_dft_print_drop_frame_info(THIS_FILE_ID, __LINE__, 1, OSAL_NULL);
        oal_netbuf_free(netbuf);
        oam_warning_log0(0, OAM_SF_SDP, "{hmac_sdp_send_data::action frame len is 0.}");
        oal_mem_free(tmp->data, OAL_TRUE);
        return OAL_FAIL;
    }

    /* 释放用户侧数据内存 */
    oal_mem_free(tmp->data, OAL_TRUE);
    oal_netbuf_put(netbuf, (osal_u32)mgmt_len);

    /* 调用发送管理帧接口 */
    ret = hmac_tx_mgmt_send_event_etc(hmac_vap, netbuf, mgmt_len);
    if (ret != OAL_SUCC) {
        oal_netbuf_free(netbuf);
        oam_warning_log1(0, OAM_SF_SDP, "{hmac_sdp_send_data::hmac_tx_mgmt_send_event failed[%d].}", ret);
        return (osal_s32)ret;
    }

    return OAL_SUCC;
}

/* 通过传入的Action帧的前六个字节特征，判断是不是SDP类型帧 */
osal_u8 hmac_is_sdp_frame(oal_netbuf_stru *netbuf)
{
    dmac_rx_ctl_stru *rx_ctrl = OSAL_NULL;
    mac_rx_ctl_stru *rx_info = OSAL_NULL;
    mac_ieee80211_frame_stru *frame_hdr = OSAL_NULL;
    osal_u8 *data = OSAL_NULL;
    /* sdp 帧头格式：Category:0x04,action:0x09,OUI:0x50-6F-9A,OUI Type:0x13 */
    osal_u8 auc_value[] = { 0x4, 0x9, 0x50, 0x6F, 0x9A, 0x13 };
    osal_u16 len;

    if ((netbuf == OSAL_NULL) || (hmac_sdp_is_init() != OAL_TRUE)) {
        return OAL_FALSE;
    }

    rx_ctrl = (dmac_rx_ctl_stru *)oal_netbuf_cb(netbuf);
    rx_info = (mac_rx_ctl_stru *)(&(rx_ctrl->rx_info));
    frame_hdr = (mac_ieee80211_frame_stru *)(uintptr_t)mac_get_rx_cb_mac_hdr(rx_info);
    data = oal_netbuf_rx_data(netbuf);
    len = rx_info->frame_len - rx_info->mac_header_len;

    if ((data == OSAL_NULL) || (len < sizeof(auc_value))) {
        return OAL_FALSE;
    }
    if ((frame_hdr->frame_control.type != WLAN_MANAGEMENT) ||
        (frame_hdr->frame_control.sub_type != WLAN_ACTION)) {
        return OAL_FALSE;
    }

    if (osal_memcmp(data, auc_value, sizeof(auc_value)) == 0) {
        return OAL_TRUE;
    }
    return OAL_FALSE;
}

const hmac_sdp_service_info *hmac_sdp_get_service_info(osal_void)
{
    return &g_service_info;
}

#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
OAL_STATIC osal_void hmac_ccpriv_sdp_srt_to_hex(const osal_s8 *param, osal_u8 len, osal_u8 *value)
{
    osal_u8 index;

    for (index = 0; index < len; index++) {
        /* 每2位字符转为一个十六进制，第一位乘以16 */
        value[index / 2] = (osal_u8)(value[index / 2] * 16 * (index % 2) + oal_strtohex(param));
        param++;
    }
}

OAL_STATIC osal_s32 hmac_ccpriv_sdp_init(hmac_vap_stru *hmac_vap, const osal_s8 *param)
{
    osal_u8 tmp;
    osal_s8 name[CCPRIV_CMD_NAME_MAX_LEN] = {0};
    osal_s32 ret;
    frw_msg msg;

    (osal_void)memset_s(&msg, OAL_SIZEOF(msg), 0, OAL_SIZEOF(msg));

    /* sdp初始化命令: ccpriv "wlan0 sdp_enable 0 | 1 ",此处将解析出"0"或"1"存入name */
    ret = hmac_ccpriv_get_one_arg(&param, name, OAL_SIZEOF(name));
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_SDP, "{hmac_ccpriv_sdp_init::hmac_ccpriv_get_one_arg return err_code %d!}", ret);
        return ret;
    }

    /* 针对解析出的不同命令，配置不同的通道 */
    if ((osal_adapt_strncmp("0", (const char *)name, CCPRIV_CMD_NAME_MAX_LEN)) == 0) {
        tmp = WAL_SDP_DISABLE;
    } else if ((osal_adapt_strncmp("1", (const char *)name, CCPRIV_CMD_NAME_MAX_LEN)) == 0) {
        tmp = WAL_SDP_ENABLE;
    } else if ((osal_adapt_strncmp("2", (const char *)name, CCPRIV_CMD_NAME_MAX_LEN)) == 0) {
        tmp = WAL_SDP_EN_SC;
    } else {
        oam_warning_log0(0, OAM_SF_SDP, "{hmac_ccpriv_sdp_init::the sdp init command is error}");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    msg.data = &tmp;
    ret = hmac_sdp_init_enable(hmac_vap, &msg);
    if (OAL_UNLIKELY(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_SDP, "{hmac_ccpriv_sdp_init::return err code %d!}", ret);
        return ret;
    }

    return OAL_SUCC;
}

OAL_STATIC osal_s32 hmac_ccpriv_sdp_start_func(hmac_vap_stru *hmac_vap, const osal_s8 *param, osal_u8 type)
{
    osal_s8 name[WAL_CCPRIV_CMD_NAME_MAX_LEN] = {0};
    osal_s32 ret;
    frw_msg msg;
    oal_sdp_service *service_para = OAL_PTR_NULL;
    oal_sdp_service service_info;
    (osal_void)memset_s(&service_info, OAL_SIZEOF(service_info), 0, OAL_SIZEOF(service_info));
    service_para = &service_info;

    /*
     * sdp发布服务命令:ccpriv "wlan0 sdp_start_publish service_name local_handle"
     * sdp订阅服务命令:ccpriv "wlan0 sdp_start_subscribe service_name local_handle"
     */
    ret = hmac_ccpriv_get_one_arg(&param, name, WAL_CCPRIV_CMD_NAME_MAX_LEN);
    /* 传入的是十六进制字符，长度为name真实长度的2倍 */
    if ((ret != OAL_SUCC) || (osal_strlen((const char *)name) != SDP_HASH_SERVICE_NAME_LEN * 2)) {
        oam_warning_log2(0, OAM_SF_SDP, "{hmac_ccpriv_sdp_start_func::err_code %d!, len(name):%d}", ret,
            osal_strlen((const char *)name));
        return OAL_FAIL;
    }
    hmac_ccpriv_sdp_srt_to_hex((const osal_s8 *)name,
        (osal_u8)osal_strlen((const char *)name), (osal_u8 *)service_para->name);

    ret = hmac_ccpriv_get_one_arg(&param, name, WAL_CCPRIV_CMD_NAME_MAX_LEN);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_SDP, "{hmac_ccpriv_sdp_start_func::hmac_ccpriv_get_one_arg return err_code %d!}",
                         ret);
        return ret;
    }
    service_para->local_handle = (osal_u8)oal_atoi((const osal_s8 *)name);
    if (service_para->local_handle == 0) {
        oam_error_log0(0, OAM_SF_SDP, "{hmac_ccpriv_sdp_start_func:local_handle is 0!}");
        return OAL_FAIL;
    }
    service_para->type = type;
    msg.data = (osal_u8 *)&service_info;

    ret = hmac_sdp_start_subscribe(hmac_vap, &msg);
    if (OAL_UNLIKELY(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_SDP, "{hmac_ccpriv_sdp_start_func::return err code %d!}", ret);
        return ret;
    }

    return OAL_SUCC;
}


OAL_STATIC osal_s32 hmac_ccpriv_sdp_start_subscribe(hmac_vap_stru *hmac_vap, const osal_s8 *param)
{
    return hmac_ccpriv_sdp_start_func(hmac_vap, param, OAL_SDP_SERVICE_TYPE_SUBSCRIBE);
}

OAL_STATIC osal_s32 hmac_ccpriv_sdp_cancle_func(hmac_vap_stru *hmac_vap, const osal_s8 *param)
{
    osal_u8 local_handle;
    osal_s8 name[CCPRIV_CMD_NAME_MAX_LEN] = {0};
    osal_s32 ret;
    frw_msg msg;

    /*
     * sdp取消订阅命令:ccpriv "wlan0 sdp_cancle_subscribe local_handle "
     * sdp取消发布命令:ccpriv "wlan0 sdp_cancle_publish local_handle "
     */
    ret = hmac_ccpriv_get_one_arg(&param, name, OAL_SIZEOF(name));
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_SDP, "{hmac_ccpriv_sdp_cancle_func::hmac_ccpriv_get_one_arg return err_code %d!}",
                         ret);
        return ret;
    }
    local_handle = (osal_u8)oal_atoi((const osal_s8 *)name);
    if (local_handle == 0) {
        oam_error_log0(0, OAM_SF_SDP, "{hmac_ccpriv_sdp_cancle_func:local_handle is 0!}");
        return OAL_FAIL;
    }

    msg.data = (osal_u8 *)&local_handle;
    ret = hmac_sdp_cancel_subscribe(hmac_vap, &msg);
    if (OAL_UNLIKELY(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_SDP, "{hmac_ccpriv_sdp_cancle_func::return err code %d!}", ret);
        return ret;
    }

    return OAL_SUCC;
}

OAL_STATIC osal_s32 hmac_ccpriv_sdp_parse_info(const osal_s8 **src_head, oal_sdp_info *sdp_param)
{
    osal_s8 name[CCPRIV_CMD_NAME_MAX_LEN] = {0};
    osal_s32 ret;
    const osal_s8 *param = *src_head;

    ret = hmac_ccpriv_get_one_arg(&param, name, OAL_SIZEOF(name));
    /* 传入的是十六进制字符，长度为name真实长度的2倍 */
    if ((ret != OAL_SUCC) || (osal_strlen((const char *)name) != SDP_HASH_SERVICE_NAME_LEN * 2)) {
        oam_warning_log2(0, OAM_SF_SDP, "{hmac_ccpriv_sdp_parse_info::err_code %d!, osal_strlen(name):%d}", ret,
                         osal_strlen((const char *)name));
        return OAL_FAIL;
    }
    hmac_ccpriv_sdp_srt_to_hex((const osal_s8 *)name,
        (osal_u8)osal_strlen((const char *)name), (osal_u8 *)sdp_param->name);

    ret = hmac_ccpriv_get_one_arg(&param, name, OAL_SIZEOF(name));
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_SDP, "{hmac_ccpriv_sdp_parse_info::hmac_ccpriv_get_one_arg return err_code %d!}",
                         ret);
        return ret;
    }
    sdp_param->local_handle = (osal_u8)oal_atoi((const osal_s8 *)name);

    ret = hmac_ccpriv_get_one_arg(&param, name, OAL_SIZEOF(name));
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_SDP, "{hmac_ccpriv_sdp_parse_info::hmac_ccpriv_get_one_arg return err_code %d!}",
                         ret);
        return ret;
    }
    sdp_param->peer_handle = (osal_u8)oal_atoi((const osal_s8 *)name);

    ret = hmac_ccpriv_get_one_arg(&param, name, OAL_SIZEOF(name));
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_SDP, "{hmac_ccpriv_sdp_parse_info::hmac_ccpriv_get_one_arg return err_code %d!}",
                         ret);
        return ret;
    }
    sdp_param->len = (osal_u8)oal_atoi((const osal_s8 *)name);

    /* 校验合法性，命令输入的是十六进制字符，data长度不能超过命令最大长度的2分之一 */
    if ((sdp_param->len == 0) || (sdp_param->len >= CCPRIV_CMD_NAME_MAX_LEN / 2) ||
        (sdp_param->local_handle == 0) || (sdp_param->peer_handle == 0)) {
        oam_warning_log3(0, OAM_SF_SDP, "{hmac_ccpriv_sdp_parse_info:: len:[%d],local_handle[%d],peer_handle[%d]!}",
                         sdp_param->len, sdp_param->local_handle, sdp_param->peer_handle);
        return OAL_FAIL;
    }
    *src_head = param;

    return OAL_SUCC;
}

OAL_STATIC osal_s32 hmac_ccpriv_sdp_send_data(hmac_vap_stru *hmac_vap, const osal_s8 *param)
{
    osal_s8 name[CCPRIV_CMD_NAME_MAX_LEN] = {0};
    osal_s32 ret;
    oal_sdp_info *sdp_param = OAL_PTR_NULL;
    oal_sdp_info sdp_info;
    frw_msg msg;
    (osal_void)memset_s(&sdp_info, OAL_SIZEOF(sdp_info), 0, OAL_SIZEOF(sdp_info));
    sdp_param = &sdp_info;

    /* sdp发送数据命令: ccpriv "wlan0 sdp_send_data peer_mac local_handle peer_handle len data" */
    ret = hmac_ccpriv_sdp_parse_info(&param, sdp_param);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_SDP, "{hmac_ccpriv_sdp_send_data::hmac_ccpriv_sdp_parse_info return err_code %d!}",
                         ret);
        return ret;
    }
    ret = hmac_ccpriv_get_one_arg(&param, name, OAL_SIZEOF(name));
    /* 命令输入的是十六进制字符，长度为data真实长度的2倍 */
    if ((ret != OAL_SUCC) || (osal_strlen((const char *)name) != sdp_param->len * 2)) {
        oam_warning_log2(0, OAM_SF_SDP, "{hmac_ccpriv_sdp_send_data::err_code %d!, osal_strlen(name):%d}", ret,
                         osal_strlen((const char *)name));
        return OAL_FAIL;
    }
    /*
     * 申请内存存放用户信息，将内存指针作为事件payload抛下去
     * 此处申请的内存在事件处理函数释放(hmac_sdp_send_data)
     */
    sdp_param->data = oal_mem_alloc(OAL_MEM_POOL_ID_LOCAL, sdp_param->len, OAL_TRUE);
    if (sdp_param->data == OAL_PTR_NULL) {
        oam_error_log1(0, OAM_SF_SDP, "{hmac_ccpriv_sdp_send_data::alloc data fail(size:%u), return null ptr!}",
                       sdp_param->len);
        return OAL_ERR_CODE_ALLOC_MEM_FAIL;
    }
    (osal_void)memset_s(sdp_param->data, sdp_param->len, 0, sdp_param->len);
    hmac_ccpriv_sdp_srt_to_hex((const osal_s8 *)name, (osal_u8)osal_strlen((const char *)name), sdp_param->data);
    msg.data = (osal_u8 *)&sdp_info;

    ret = hmac_sdp_send_data(hmac_vap, &msg);
    if (OAL_UNLIKELY(ret != OAL_SUCC)) {
        oal_mem_free(sdp_param->data, OAL_TRUE);
        oam_warning_log1(0, OAM_SF_SDP, "{hmac_ccpriv_sdp_send_data::return err code %d!}", ret);
        return ret;
    }

    return OAL_SUCC;
}

OAL_STATIC osal_s32 hmac_ccpriv_sdp_dw_prd_time_cfg(hmac_vap_stru *hmac_vap, const osal_s8 *param)
{
    /* 待实现 */
    unref_param(hmac_vap);
    unref_param(param);
    return OAL_SUCC;
}
#endif

osal_void hmac_sdp_feature_hook_register(osal_void)
{
    /* 注册对外接口 */
    hmac_feature_hook_register(HMAC_FHOOK_SDP_IS_INIT, hmac_sdp_is_init);
    hmac_feature_hook_register(HMAC_FHOOK_IS_SDP_FRAME, hmac_is_sdp_frame);
    hmac_feature_hook_register(HMAC_FHOOK_SDP_PROC_RX_MGMT, hmac_sdp_proc_rx_mgmt);
    return;
}

osal_void hmac_sdp_feature_hook_unregister(osal_void)
{
    /* 去注册对外接口 */
    hmac_feature_hook_unregister(HMAC_FHOOK_SDP_IS_INIT);
    hmac_feature_hook_unregister(HMAC_FHOOK_IS_SDP_FRAME);
    hmac_feature_hook_unregister(HMAC_FHOOK_SDP_PROC_RX_MGMT);
    return;
}

osal_u32 hmac_sdp_msg_init(osal_void)
{
    static const msg_hook_array_stru msg_hook_array[] = {
        {WLAN_MSG_W2H_CFG_SDP_START_SUBSCRIBE, hmac_sdp_start_subscribe},
        {WLAN_MSG_W2H_CFG_SDP_CANCEL_SUBSCRIBE, hmac_sdp_cancel_subscribe},
        {WLAN_MSG_W2H_CFG_SDP_DW_PRD_TIME_CFG, hmac_sdp_dw_prd_time_cfg},
        {WLAN_MSG_W2H_CFG_SDP_INIT, hmac_sdp_init_enable},
        {WLAN_MSG_W2H_CFG_SDP_SEND_DATA, hmac_sdp_send_data},
        {WLAN_MSG_W2H_CFG_SDP_TEST_SYNC_PRIVATE, hmac_sdp_test},
    };
#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
    static const ccpriv_cmd_func_stru cmd_func[] = {
        {(const osal_s8 *)"sdp_enable", hmac_ccpriv_sdp_init},
        {(const osal_s8 *)"sdp_start_subscribe", hmac_ccpriv_sdp_start_subscribe},
        {(const osal_s8 *)"sdp_cancle_subscribe", hmac_ccpriv_sdp_cancle_func},
        {(const osal_s8 *)"sdp_send_data", hmac_ccpriv_sdp_send_data},
        {(const osal_s8 *)"sdp_dw_prd_time_cfg", hmac_ccpriv_sdp_dw_prd_time_cfg},
    };

    /* ccpriv命令注册 */
    hmac_ccpriv_register_array(cmd_func, osal_array_size(cmd_func));
#endif
    /* 消息注册 */
    frw_msg_hook_register_array(msg_hook_array, osal_array_size(msg_hook_array));
    return OAL_SUCC;
}

osal_void hmac_sdp_msg_deinit(osal_void)
{
#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
    /* ccpriv命令去注册 */
    hmac_ccpriv_unregister((const osal_s8 *)"sdp_enable");
    hmac_ccpriv_unregister((const osal_s8 *)"sdp_start_subscribe");
    hmac_ccpriv_unregister((const osal_s8 *)"sdp_cancle_subscribe");
    hmac_ccpriv_unregister((const osal_s8 *)"sdp_send_data");
    hmac_ccpriv_unregister((const osal_s8 *)"sdp_dw_prd_time_cfg");
#endif

    /* 消息去注册 */
    frw_msg_hook_unregister(WLAN_MSG_W2H_CFG_SDP_START_SUBSCRIBE);
    frw_msg_hook_unregister(WLAN_MSG_W2H_CFG_SDP_CANCEL_SUBSCRIBE);
    frw_msg_hook_unregister(WLAN_MSG_W2H_CFG_SDP_DW_PRD_TIME_CFG);
    frw_msg_hook_unregister(WLAN_MSG_W2H_CFG_SDP_INIT);
    frw_msg_hook_unregister(WLAN_MSG_W2H_CFG_SDP_SEND_DATA);
    frw_msg_hook_unregister(WLAN_MSG_W2H_CFG_SDP_TEST_SYNC_PRIVATE);
    return;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

