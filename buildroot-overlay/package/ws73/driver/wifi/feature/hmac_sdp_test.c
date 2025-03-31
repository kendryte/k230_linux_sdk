/*
 * Copyright (c) CompanyNameMagicTag 2021-2021. All rights reserved.
 * Description: Function related to wifi aware(NAN) feature.
 * Author: Huanghe
 * Create: 2021-04-16
 */

#include "hmac_sdp.h"
#include "hmac_mgmt_bss_comm.h"
#include "hmac_feature_dft.h"
#include "msg_sdp_rom.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_SDP_TEST_C /* 后续要改为 SDP_TEST_C */

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

#define SDP_MAX_SEND_FOLLOW_UP_NUM 1 /* 每回发送follow up to apk的次数 */
#define SDP_DATA_SSID_LEN_POS 1
#define SDP_DATA_SSID_POS 2
#define SDP_DATA_KEY_POS 3
#define SDP_SERVER_ID 9          /* 9 sdp的local id */
#define HMAC_FOLLOW_TIMEOUT 4000 /* follow-up报文发送间隔时间*ms */
#define SDP_ADD_PEER_MAC 1
#define SDP_DEL_PEER_MAC 0

typedef enum {
    WAL_SDP_TEST_TYPE_PRIV = 0,                 /* 测试同步命令 */
    WAL_SDP_TEST_TYPE_DISTRIBUTION_ENABLE = 1,  /* 测试无感配网初始化 */
    WAL_SDP_TEST_TYPE_DISTRIBUTION_DISABLE = 2, /* 测试无感配网去初始化 */
    WAL_SDP_TEST_TYPE_SUBSCRIBE_SERVICE = 3,    /* 测试订阅 */
    WAL_SDP_TEST_TYPE_SUBSCRIBE_CANCEL = 4,     /* 测试取消订阅 */
    WAL_SDP_TEST_TYPE_PUBLISH_LIST = 5,         /* 列出发布的服务信息 */
    WAL_SDP_TEST_TYPE_SUBSCRIBE_LIST = 6,       /* 列出订阅的服务信息 */
    WAL_SDP_TEST_TYPE_BUTT
} wal_sdp_test;

typedef enum {
    NAN_DEMO_STATE_INIT = 0X10,
    NAN_DEMO_STATE_PUBLISH_COMPLETE,
    NAN_DEMO_STATE_FOLLOW_UP_COMPLETE,
    NAN_DEMO_STATE_BSS_INFO,
    NAN_DEMO_STATE_CONNECT_SUCCESS,
    NAN_DEMO_STATE_CONNECT_SSID_NO_FIND,
    NAN_DEMO_STATE_CONNECT_KEY_ERROR,
    NAN_DEMO_STATE_BUTT
} nan_demo_state;

typedef struct {
    osal_u8 peer;
    osal_u8 local;
    osal_u8 mac[WLAN_MAC_ADDR_LEN];
    osal_u8 rsv;
} hmac_sdp_handle;

OAL_STATIC hmac_sdp_handle g_sdp_handle = { 0 };
OAL_STATIC hmac_vap_stru *g_sdp_mac_vap = NULL;
OAL_STATIC frw_timeout_stru g_follow_timer = { 0 };
OAL_STATIC osal_u16 g_sdp_init_peroid = 200; /* 默认200ms 支持命令行配置, 后续也可以作为api参数 */

OAL_STATIC osal_void sdp_send_follow_up(osal_u8 *data, osal_u8 len)
{
    oal_sdp_info info;
    osal_u8 count = 0;
    frw_msg msg_info;

    oam_info_log1(0, OAM_SF_SDP, "sdp_send_follow_up: send follow-up state=%02x", *data);

    if (memcpy_s(info.name, sizeof(info.name), g_sdp_handle.mac, sizeof(g_sdp_handle.mac)) != EOK) {
        oam_warning_log0(0, OAM_SF_SDP, "sdp_send_follow_up:memcpy_s g_sdp_handle.mac failed");
        return;
    }
    info.local_handle = g_sdp_handle.local;
    info.peer_handle = g_sdp_handle.peer;
    info.len = len;
    while (count < SDP_MAX_SEND_FOLLOW_UP_NUM) {
        info.data = oal_mem_alloc(OAL_MEM_POOL_ID_LOCAL, info.len, OAL_TRUE);
        if (info.data == OAL_PTR_NULL) {
            oam_warning_log1(0, OAM_SF_SDP, "sdp_send_follow_up:alloc data fail(size:%u)!", info.len);
            return;
        }
        (osal_void)memset_s(info.data, info.len, 0, info.len);
        if (memcpy_s(info.data, info.len, data, len) != EOK) {
            oam_warning_log0(0, OAM_SF_SDP, "sdp_send_follow_up:memcpy_s data failed");
            oal_mem_free(info.data, OAL_TRUE);
            return;
        }

        (osal_void)memset_s(&msg_info, sizeof(msg_info), 0, sizeof(msg_info));
        msg_info.data = (osal_u8 *)&info;
        msg_info.data_len = sizeof(info);
        if (hmac_sdp_send_data(g_sdp_mac_vap, &msg_info) != OAL_SUCC) {
            oam_warning_log0(0, OAM_SF_SDP, "sdp_send_follow_up:send follow-up data failed");
        }

        count++;
    }
}

OAL_STATIC osal_bool hmac_sdp_is_wifi_connected(hmac_vap_stru *hmac_vap)
{
    hmac_device_stru *hmac_device = OSAL_NULL;
    hmac_vap_stru *hmac_vap_tmp = OSAL_NULL;
    osal_u8 index;

    if (hmac_vap == NULL) {
        oam_warning_log0(0, OAM_SF_SDP, "hmac_sdp_is_wifi_connected:hmac_vap null.");
        return OSAL_FALSE;
    }

    hmac_device = hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    if (hmac_device == OSAL_NULL) {
        oam_warning_log0(0, OAM_SF_SDP, "hmac_sdp_is_wifi_connected:mac_device is null");
        return OSAL_FALSE;
    }

    if (hmac_device->vap_num == 0) {
        oam_warning_log1(0, OAM_SF_SDP, "vap_id[%d] {hmac_sdp_is_wifi_connected::none vap.}", hmac_vap->vap_id);
        return OSAL_FALSE;
    }

    for (index = 0; index < hmac_device->vap_num; index++) {
        hmac_vap_tmp = (hmac_vap_stru *)mac_res_get_hmac_vap(hmac_device->vap_id[index]);
        if (hmac_vap_tmp == OSAL_NULL) {
            oam_warning_log2(0, OAM_SF_SDP, "vap_id[%d] {hmac_sdp_is_wifi_connected::ap(%d) null.}", hmac_vap->vap_id,
                hmac_device->vap_id[index]);
            continue;
        }

        if ((hmac_vap_tmp->vap_state == MAC_VAP_STATE_UP) && (hmac_vap_tmp->vap_mode == WLAN_VAP_MODE_BSS_STA)) {
            return OSAL_TRUE;
        }
    }
    return OSAL_FALSE;
}

OAL_STATIC osal_u32 hmac_sdp_follow_timeout_callback(osal_void *arg)
{
    osal_u8 state;

    unref_param(arg);

    if (g_sdp_mac_vap == NULL) {
        oam_warning_log0(0, OAM_SF_SDP, "hmac_sdp_follow_timeout_callback:WiFi App demoAPP test not init.");
        return OAL_FAIL;
    }

    if (hmac_sdp_is_wifi_connected(g_sdp_mac_vap) == OSAL_TRUE) {
        oam_warning_log0(0, OAM_SF_SDP, "hmac_sdp_follow_timeout_callback: wifi is already connected");
        state = NAN_DEMO_STATE_CONNECT_SUCCESS;
        sdp_send_follow_up((osal_u8 *)&state, sizeof(state));
        return OAL_SUCC;
    }
    state = NAN_DEMO_STATE_FOLLOW_UP_COMPLETE;
    sdp_send_follow_up((osal_u8 *)&state, sizeof(state));
    return OAL_SUCC;
}

OAL_STATIC osal_u32 hmac_sdp_add_peer_mac_pre(osal_u8 type)
{
    frw_msg msg_info;
    oal_sdp_peer_mac_handle peer_info = {0};

    /* 将mac发消息给device,过滤时间同步beacon 或 私有action帧 */
    (osal_void)memset_s(&peer_info, OAL_SIZEOF(peer_info), 0, OAL_SIZEOF(peer_info));
    if (memcpy_s(peer_info.mac, sizeof(peer_info.mac), g_sdp_handle.mac, sizeof(g_sdp_handle.mac)) != EOK) {
        oam_warning_log0(0, OAM_SF_SDP, "hmac_sdp_add_peer_mac_pre:memcpy_s failed.");
        return OAL_FAIL;
    }
    peer_info.peer_handle = g_sdp_handle.peer;
    peer_info.type = type; /* 1-add mac 0-del mac */
    (osal_void)memset_s(&msg_info, OAL_SIZEOF(msg_info), 0, OAL_SIZEOF(msg_info));
    msg_info.data = (osal_u8 *)&peer_info;
    msg_info.data_len = sizeof(peer_info);
    if (hmac_sdp_add_peer_mac(g_sdp_mac_vap, &msg_info) != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_SDP, "hmac_sdp_add_peer_mac_pre: add peer mac fail.");
        return OAL_FAIL;
    }

    return OAL_SUCC;
}

OAL_STATIC osal_void hmac_sdp_recv_publish(const oal_sdp_info *sdp_info)
{
    osal_u8 state = NAN_DEMO_STATE_FOLLOW_UP_COMPLETE;

    if (sdp_info->local_handle != g_sdp_handle.local) {
        oam_warning_log1(0, OAM_SF_SDP, "hmac_sdp_recv_publish:local[%d]recv msg is not distribution network.",
            g_sdp_handle.local);
        return;
    }

    if (g_sdp_handle.peer != 0) {
        oam_warning_log2(0, OAM_SF_SDP, "hmac_sdp_recv_publish:already received publish, peer[%d], recv[%d].",
            g_sdp_handle.peer, sdp_info->peer_handle);
        if (sdp_info->peer_handle == g_sdp_handle.peer) {
            if (hmac_sdp_is_wifi_connected(g_sdp_mac_vap) == OSAL_TRUE) {
                state = NAN_DEMO_STATE_CONNECT_SUCCESS;
            }
            sdp_send_follow_up((osal_u8 *)&state, sizeof(state)); /* 收到publish及时回复一下follow up 不完全靠定时器 */
        }
        return;
    }

    g_sdp_handle.peer = sdp_info->peer_handle;

    /* 此处name临时用来保存publish报文的源mac地址, 从结构体成员定义来看是保存服务号的 */
    if (memcpy_s(g_sdp_handle.mac, sizeof(g_sdp_handle.mac), sdp_info->name, sizeof(sdp_info->name)) != EOK) {
        oam_warning_log0(0, OAM_SF_SDP, "hmac_sdp_recv_publish:memcpy_s failed.");
        return;
    }
    oam_info_log0(0, OAM_SF_SDP, "hmac_sdp_recv_publish:recv data");
    sdp_send_follow_up((osal_u8 *)&state, sizeof(state));

    /* 将mac发消息给device,过滤时间同步beacon 或 私有action帧 */
    if (hmac_sdp_add_peer_mac_pre(SDP_ADD_PEER_MAC) != OAL_SUCC) {
        return;
    }

    if (g_follow_timer.is_enabled != OAL_TRUE) {
        /* 使能定时器，定时器到期则尝试走流程发报文 */
        frw_create_timer_entry(&g_follow_timer, hmac_sdp_follow_timeout_callback, HMAC_FOLLOW_TIMEOUT, OSAL_NULL,
            OAL_TRUE);
    } else {
        oam_info_log0(0, OAM_SF_SDP, "{hmac_sdp_recv_publish::timer already enabled}");
    }
    return;
}

OAL_STATIC osal_void hmac_sdp_recv_follow_up(hmac_vap_stru *hmac_vap, oal_sdp_info *sdp_info,
    osal_u8 *data, osal_u8 len, oal_bool_enum_uint8 *reported_host)
{
    if (g_sdp_handle.peer == 0) {
        oam_warning_log0(0, OAM_SF_SDP, "hmac_sdp_recv_follow_up:not recv publish first.");
        return;
    }

    if (len == 0) {
        oam_warning_log0(0, OAM_SF_SDP, "hmac_sdp_recv_follow_up:data len is 0.");
        return;
    }
    if (g_sdp_handle.peer != sdp_info->peer_handle) {
        oam_warning_log2(0, OAM_SF_SDP, "hmac_sdp_recv_follow_up:record peer[%d] is diff [%d]\n", g_sdp_handle.peer,
            sdp_info->peer_handle);
        return;
    }
    /* 此处name临时用来保存publish报文的源mac地址, 从结构体成员定义来看是保存服务号的 */
    if (osal_memcmp(g_sdp_handle.mac, sdp_info->name, sizeof(g_sdp_handle.mac)) != 0) {
        oam_warning_log0(0, OAM_SF_SDP, "hmac_sdp_recv_follow_up:osal_memcmp mac is fail");
        return;
    }
    oam_info_log0(0, OAM_SF_SDP, "hmac_sdp_recv_follow_up:send follow up to host.");

    *reported_host = OSAL_TRUE;
    hmac_sdp_send_followup_to_host(hmac_vap, sdp_info, data, len);

    return;
}


/* 测试接口，用于构造私有同步帧，后续可删除 */
OAL_STATIC osal_u16 hmac_sdp_sync_private_action(const hmac_vap_stru *hmac_vap, const oal_sdp_wakeup_info *para,
    oal_netbuf_stru *mgmt_buf)
{
    /* sdp 私有帧头格式：Category:127,OUI:0x00-E0-FC,Type:0xFC,SubType:100 */
    osal_u8 action_hdr_fixed[] = { 127, 0x0, 0xE0, 0xFC, 0xFC, 100 };
    osal_u8 mcast_mac[ETH_ALEN] = { 0x51, 0x6F, 0x9A, 0x01, 0x00, 0x00 };
    osal_u8 *data = (osal_u8 *)oal_netbuf_header(mgmt_buf);
    mac_tx_ctl_stru *tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(mgmt_buf);
    sdp_action_header *action_hdr = OSAL_NULL;
    osal_u8 *payload_addr = OSAL_NULL;
    oal_sdp_wakeup_info wake_info;

    if (memcpy_s(&wake_info, sizeof(wake_info), para, sizeof(oal_sdp_wakeup_info)) != EOK) {
        return 0;
    }

    mac_hdr_set_frame_control(data, WLAN_PROTOCOL_VERSION | WLAN_FC0_TYPE_MGT | WLAN_FC0_SUBTYPE_ACTION);

    /* 设置分片序号为0 */
    mac_hdr_set_fragment_number(data, 0);

    /* 设置地址1，对端地址 */
    if (memcpy_s(data + WLAN_HDR_ADDR1_OFFSET, WLAN_MAC_ADDR_LEN, mcast_mac, ETH_ALEN) != EOK) {
        return 0;
    }

    /* 设置本端的MAC地址 */
    if (memcpy_s(data + WLAN_HDR_ADDR2_OFFSET, WLAN_MAC_ADDR_LEN,
        hmac_vap->mib_info->wlan_mib_sta_config.dot11_station_id, WLAN_MAC_ADDR_LEN) != EOK) {
        return 0;
    }
    /* 地址3, BSSID */
    if (memcpy_s(data + WLAN_HDR_ADDR3_OFFSET, WLAN_MAC_ADDR_LEN, hmac_vap->bssid, WLAN_MAC_ADDR_LEN) != EOK) {
        return 0;
    }

    action_hdr = (sdp_action_header *)(data + MAC_80211_FRAME_LEN); /* 取action帧体 */
    if (memcpy_s(action_hdr, sizeof(action_hdr_fixed), action_hdr_fixed, sizeof(action_hdr_fixed)) != EOK) {
        return 0;
    }
    payload_addr = (osal_u8 *)(action_hdr + 1);
    wake_info.total_period = oal_host2net_long(wake_info.total_period);
    wake_info.wake_period = oal_host2net_long(wake_info.wake_period);
    if (memcpy_s(payload_addr, sizeof(oal_sdp_wakeup_info), &wake_info, sizeof(oal_sdp_wakeup_info)) != EOK) {
        return 0;
    }

    /* 计算帧的总长度,设置相关发送配置 */
    mac_get_cb_frame_header_length(tx_ctl) = MAC_80211_FRAME_LEN;
    mac_get_cb_tx_user_idx(tx_ctl) = HMAC_SDP_TX_UESR_ID; /* 发送完成需要获取user结构体 */
    tx_ctl->need_rsp = OAL_FALSE;
    tx_ctl->is_needretry = OAL_FALSE;
    tx_ctl->ismcast = OAL_TRUE;
    mac_get_cb_mpdu_len(tx_ctl) = MAC_80211_FRAME_LEN + sizeof(sdp_action_header) + sizeof(oal_sdp_wakeup_info);

    return mac_get_cb_mpdu_len(tx_ctl);
}

/* 测试接口，用于发送私有同步帧，后续可删除 */
OAL_STATIC osal_u32 hmac_sdp_test_sync_private(hmac_vap_stru *hmac_vap, const osal_u8 *param)
{
    oal_netbuf_stru *netbuf = OSAL_NULL;
    osal_u32 ret;
    osal_u16 mgmt_len;

    if ((hmac_vap == OSAL_NULL) || (param == OSAL_NULL)) {
        return OAL_FAIL;
    }

    /* 申请管理帧内存 */
    netbuf = OAL_MEM_NETBUF_ALLOC(OAL_MGMT_NETBUF, WLAN_MGMT_NETBUF_SIZE, OAL_NETBUF_PRIORITY_HIGH);
    if (netbuf == OSAL_NULL) {
        oam_warning_log0(0, OAM_SF_SDP, "{hmac_sdp_test_sync_private::alloc netbuf failed.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    oal_set_netbuf_prev(netbuf, OSAL_NULL);
    oal_set_netbuf_next(netbuf, OSAL_NULL);

    if (memset_s(oal_netbuf_cb(netbuf), OAL_NETBUF_CB_SIZE(), 0, OAL_NETBUF_CB_SIZE()) != EOK) {
        hmac_dft_print_drop_frame_info(THIS_FILE_ID, __LINE__, 1, OSAL_NULL);
        oal_netbuf_free(netbuf);
        return OAL_FAIL;
    }

    /* 封装Action帧 */
    mgmt_len = hmac_sdp_sync_private_action(hmac_vap, (const oal_sdp_wakeup_info *)param, netbuf);
    if (mgmt_len == 0) {
#ifdef _PRE_WLAN_DFT_STAT
        hmac_dft_print_drop_frame_info(THIS_FILE_ID, __LINE__, 1, OSAL_NULL);
#endif
        oal_netbuf_free(netbuf);
        oam_warning_log0(0, OAM_SF_SDP, "{hmac_sdp_test_sync_private::action frame len is 0.}");
        return OAL_FAIL;
    }

    oal_netbuf_put(netbuf, (osal_u32)mgmt_len);

    /* 调用发送管理帧接口 */
    ret = hmac_tx_mgmt_send_event_etc(hmac_vap, netbuf, mgmt_len);
    if (ret != OAL_SUCC) {
        oal_netbuf_free(netbuf);
        oam_warning_log1(0, OAM_SF_SDP, "{hmac_sdp_test_sync_private::hmac_tx_mgmt_send_event failed[%d].}", ret);
        return ret;
    }

    oam_info_log1(0, OAM_SF_SDP, "hmac_sdp_test_sync_private send sync success mgmt_len=%d", mgmt_len);
    return OAL_SUCC;
}

/* 测试无感配网接口: 订阅服务 */
/* 启动订阅操作,用于侦听某一服务信息，入参包含服务名称哈希值和订阅成功之后分配的本端instance id */
OAL_STATIC osal_u32 hmac_sdp_test_distribution_subscribe(hmac_vap_stru *hmac_vap, osal_u8 *param)
{
    oal_sdp_service service_info;
    frw_msg msg_info;
    oal_sdp_subscribe_info *subscribe_info = (oal_sdp_subscribe_info *)param;

    if (g_sdp_mac_vap == NULL) {
        oam_warning_log0(0, OAM_SF_SDP, "hmac_sdp_test_distribution_subscribe: not start, can not subscribe.");
        return OAL_FAIL;
    }
    (osal_void)memset_s(&service_info, sizeof(service_info), 0, sizeof(service_info));
    service_info.local_handle = subscribe_info->local_handle;
    service_info.type = OAL_SDP_SERVICE_TYPE_SUBSCRIBE;
    if (memcpy_s(service_info.name, sizeof(service_info.name), subscribe_info->name,
            sizeof(subscribe_info->name)) != EOK) {
        oam_warning_log0(0, OAM_SF_SDP, "hmac_sdp_test_distribution_subscribe:memcpy_s hash_server_name failed.");
        return OAL_FAIL;
    }

    (osal_void)memset_s(&msg_info, sizeof(msg_info), 0, sizeof(msg_info));
    msg_info.data = (osal_u8 *)&service_info;
    msg_info.data_len = sizeof(service_info);
    if (hmac_sdp_start_subscribe(hmac_vap, &msg_info) != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_SDP, "hmac_sdp_test_distribution_subscribe: failed.");
        return OAL_FAIL;
    }
    oam_info_log1(0, OAM_SF_SDP, "hmac_sdp_test_distribution_subscribe: suc, local_handle[%d].",
        service_info.local_handle);
    (osal_void)memset_s(&g_sdp_handle, sizeof(g_sdp_handle), 0, sizeof(g_sdp_handle));
    g_sdp_handle.local = service_info.local_handle; /* 不再固定 SDP_SERVER_ID, 收publish时会获取该值,替换 */
    oam_info_log1(0, OAM_SF_SDP, "{hmac_sdp_test_distribution_subscribe:local[%d].}", g_sdp_handle.local);
    return OAL_SUCC;
}

OAL_STATIC osal_u32 hmac_sdp_test_distribution_subscribe_cancel(hmac_vap_stru *hmac_vap, osal_u8 *param)
{
    frw_msg msg_info;
    oal_sdp_subscribe_info *subscribe_info = (oal_sdp_subscribe_info *)param;
    osal_u8 local_handle;

    if ((g_sdp_mac_vap == NULL) || (g_sdp_handle.local == 0)) {
        oam_warning_log0(0, OAM_SF_SDP, "hmac_sdp_test_distribution_subscribe not start/subscribe,cannot cancel");
        return OAL_FAIL;
    }
    local_handle = subscribe_info->local_handle;

    (osal_void)memset_s(&msg_info, sizeof(msg_info), 0, sizeof(msg_info));
    msg_info.data = &local_handle;
    msg_info.data_len = sizeof(local_handle);
    if (hmac_sdp_cancel_subscribe(hmac_vap, &msg_info) != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_SDP, "hmac_sdp_test_distribution_subscribe_cancel: failed.");
        return OAL_FAIL;
    }
    oam_info_log1(0, OAM_SF_SDP, "hmac_sdp_test_distribution_subscribe_cancel:suc local_handle[%d]", local_handle);
    (osal_void)memset_s(&g_sdp_handle, sizeof(g_sdp_handle), 0, sizeof(g_sdp_handle));
    frw_destroy_timer_entry(&g_follow_timer);

    /* 将mac发消息给device,过滤时间同步beacon 或 私有action帧 */
    if (hmac_sdp_add_peer_mac_pre(SDP_DEL_PEER_MAC) != OAL_SUCC) {
        return OAL_FAIL;
    }
    oam_info_log1(0, OAM_SF_SDP, "{hmac_sdp_test_distribution_subscribe_cancel:local[%d].}", g_sdp_handle.local);
    return OAL_SUCC;
}

/* 测试无感配网接口: 初始化, 20220919更新的, init 和 subscribe分开, 同时 73中is_connected 不会从命令行传入了 */
OAL_STATIC osal_u32 hmac_sdp_test_distribution_enable(hmac_vap_stru *hmac_vap, osal_u8 is_connected, osal_u8 ratio)
{
     /* 如果STA已连上，初始化启定时器，否则不启定时器 */
    if (hmac_sdp_init(hmac_vap, is_connected, g_sdp_init_peroid, &ratio) != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_SDP, "hmac_sdp_test_sync_distribution:init failed.");
        return OAL_FAIL;
    }

    g_sdp_mac_vap = hmac_vap;
    oam_warning_log3(0, OAM_SF_SDP, "hmac_sdp_test_distribution_enable:is_connected[%d], peroid[%d] ratio[%d]",
        is_connected, g_sdp_init_peroid, ratio);
    /* sdp初始化时注册 */
    hmac_sdp_feature_hook_register();
    return OAL_SUCC;
}

/* 测试无感配网接口: 去初始化 */
OAL_STATIC osal_u32 hmac_sdp_test_distribution_disable(hmac_vap_stru *hmac_vap, osal_u8 is_connected)
{
    unref_param(is_connected);
    /* sdp去初始化时注册 */
    hmac_sdp_feature_hook_unregister();

    if (g_sdp_mac_vap == NULL) {
        oam_warning_log0(0, OAM_SF_SDP, "hmac_sdp_test_distribution_disable: not start, do not need disable.");
        return OAL_FAIL;
    }

    (osal_void)memset_s(&g_sdp_handle, sizeof(g_sdp_handle), 0, sizeof(g_sdp_handle));
    g_sdp_mac_vap = OSAL_NULL;
    frw_destroy_timer_entry(&g_follow_timer);

    if (hmac_sdp_deinit(hmac_vap) != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_SDP, "hmac_sdp_test_sync_distribution:init failed.");
        return OAL_FAIL;
    }
    return OAL_SUCC;
}

OAL_STATIC osal_void hmac_sdp_test_print_hex(const osal_u8 *name, osal_u8 len)
{
    osal_u8 index;

    if (len == HMAC_SDP_TYPE_SWITCH_LENGTH) {
        /* 打印开关灯报文 0:第一位 1:第二位 2:第三位 3: 第四位 */
        oam_warning_log4(0, OAM_SF_SDP, "%02x%02x%02x%02x", name[0], name[1], name[2], name[3]);
    }

    for (index = 0; index < len; index++) {
        oam_warning_log1(0, OAM_SF_SDP, "%02x", name[index]);
    }
}

/* 列出type类型服务的信息 */
OAL_STATIC osal_u32 hmac_sdp_test_print_info(osal_u8 type)
{
    const hmac_sdp_service_info *service = OSAL_NULL;
    const oal_sdp_service *array = OSAL_NULL;
    osal_u8 array_size;
    osal_u8 index;

    if (hmac_sdp_is_init() != OAL_TRUE) {
        oam_warning_log0(0, OAM_SF_SDP, "hmac_sdp_test_print_info:sdp not init.");
        return OAL_FAIL;
    }

    service = hmac_sdp_get_service_info();
    array = service->info;
    array_size = sizeof(service->info) / sizeof(service->info[0]);
    for (index = 0; index < array_size; index++) {
        if ((array[index].type & type) != 0) {
            oam_warning_log2(0, OAM_SF_SDP, "service local handle:%d.service type:%d.service name:",
                array[index].local_handle, array[index].type);
            hmac_sdp_test_print_hex(array[index].name, sizeof(array[index].name));
        }
    }

    return OAL_SUCC;
}

osal_s32 hmac_sdp_test(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    osal_u8 *param = msg->data;
    osal_u8 type = ((const oal_sdp_wakeup_info *)param)->type;
    osal_u8 ratio = ((const oal_sdp_wakeup_info *)param)->ratio;
    osal_u8 is_connected = hmac_sdp_is_wifi_connected(hmac_vap);

    switch (type) {
        case WAL_SDP_TEST_TYPE_PRIV:
            return (osal_s32)hmac_sdp_test_sync_private(hmac_vap, param);
        case WAL_SDP_TEST_TYPE_DISTRIBUTION_ENABLE:
            return (osal_s32)hmac_sdp_test_distribution_enable(hmac_vap, is_connected, ratio);
        case WAL_SDP_TEST_TYPE_DISTRIBUTION_DISABLE:
            return (osal_s32)hmac_sdp_test_distribution_disable(hmac_vap, is_connected);
        case WAL_SDP_TEST_TYPE_PUBLISH_LIST:
            return (osal_s32)hmac_sdp_test_print_info(OAL_SDP_SERVICE_TYPE_PUBLISH);
        case WAL_SDP_TEST_TYPE_SUBSCRIBE_LIST:
            return (osal_s32)hmac_sdp_test_print_info(OAL_SDP_SERVICE_TYPE_SUBSCRIBE);
        case WAL_SDP_TEST_TYPE_SUBSCRIBE_SERVICE:
            return (osal_s32)hmac_sdp_test_distribution_subscribe(hmac_vap, param);
        case WAL_SDP_TEST_TYPE_SUBSCRIBE_CANCEL:
            return (osal_s32)hmac_sdp_test_distribution_subscribe_cancel(hmac_vap, param);
        default:
            return OAL_FAIL;
    }
}

osal_u32 hmac_sdp_proc_rx_mgmt(hmac_vap_stru *hmac_vap, oal_netbuf_stru *netbuf,
    oal_bool_enum_uint8 *reported_host)
{
    osal_u8 *desc_attr = OSAL_NULL;
    oal_sdp_info sdp_param;
    osal_u8 sdp_type;
    osal_u8 msg_len;

    if ((hmac_vap == OSAL_NULL) || (netbuf == OSAL_NULL)) {
        return OAL_FAIL;
    }

    (osal_void)memset_s(&sdp_param, sizeof(sdp_param), 0, sizeof(sdp_param));
    desc_attr = hmac_sdp_parse_rx(netbuf, &sdp_param);
    if (desc_attr == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_SDP, "hmac_sdp_proc_rx_mgmt: hmac_sdp_parse_rx failed.");
        return OAL_FAIL;
    }

    if (g_sdp_mac_vap == NULL) {
        oam_warning_log0(0, OAM_SF_SDP, "hmac_sdp_proc_rx_mgmt:WiFi App demoAPP test not init.");
        return OAL_FAIL;
    }

    sdp_type = desc_attr[HMAC_SDP_POS_TYPE] & HMAC_SDP_TYPE_MASK;
    /* 接收到publish报文，发送follow到对端 */
    if (sdp_type == HMAC_SDP_TYPE_PUBLISH) {
        hmac_sdp_recv_publish(&sdp_param);
    } else if (sdp_type == HMAC_SDP_TYPE_FOLLOWUP) { /* 接收到follow-up帧发送到host侧 */
        msg_len = desc_attr[HMAC_SDP_POS_DATA_LEN];
        hmac_sdp_test_print_hex(desc_attr + HMAC_SDP_NO_DATA_LEN, msg_len);
        hmac_sdp_recv_follow_up(hmac_vap, &sdp_param, desc_attr + HMAC_SDP_NO_DATA_LEN, msg_len, reported_host);
    } else {
        oam_warning_log0(0, OAM_SF_SDP, "hmac_sdp_proc_rx_mgmt:frame received is invalid!");
        return OAL_FAIL;
    }

    return OAL_SUCC;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

