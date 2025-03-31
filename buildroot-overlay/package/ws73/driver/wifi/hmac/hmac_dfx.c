/*
 * Copyright (c) CompanyNameMagicTag 2020-2022. All rights reserved.
 * Description: DFX基础维测.
 */

#ifdef _PRE_WLAN_DFT_STAT
/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "hmac_dfx.h"
#include "oal_ext_if.h"
#include "oam_ext_if.h"
#include "wlan_spec.h"
#include "hmac_ext_if.h"
#include "wlan_msg.h"
#include "hmac_scan.h"
#include "hmac_config.h"
#include "oal_netbuf_data.h"
#include "dmac_ext_if_hcm.h"
#include "hmac_feature_interface.h"
#include "mac_ie.h"
#include "hmac_ccpriv.h"
#include "hal_common_ops.h"
#include "fe_rf_customize_power.h"
#include "hmac_hook.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_HMAC_DFX_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
OAL_CONST char *dfx_user_fail_reason_desc[] = {
    "null.",
    "sta conn can't find ap.",
    "sta conn check bss desc err.",
    "sta conn set param err.",
    "sta conn sync cfg err.",
    "sta conn auth fsm err.",
    "sta conn auth rsp err.",
    "sta conn auth timeout.",
    "sta conn asoc fsm err.",
    "sta conn asoc rsp err.",
    "sta conn asoc timeout.",
    "ap auth rsp encap err.",
    "ap auth find user err.",
    "ap auth rsp send err.",
    "ap asoc wait timeout.",
    "ap asoc nego capability err.",
    "ap asoc rsp encap err.",
    "ap asoc rsp send err.",
};

OAL_CONST char *dfx_user_offline_reason_desc[] = {
    "null.",
    "sta rcv deauth or disasoc.",
    "sta other vap clean bss.",
    "ap rcv deatuh.",
    "ap rcv disasoc.",
    "ap blacklist del sta.",
    "ap down del sta.",
    "ap auth rsp del other sta.",
    "ap clean bss.",
    "ap sta noup rcv data.",
    "cfg kick user.",
    "pmf sa query timeout del user.",
    "dmac disasoc notice.",
    "roam conn fail.",
    "roam handshake fail.",
};
#endif

osal_u32 g_hmac_service_control = 0x5; /* 32位全局变量控制32种业务场景, 默认开启低第1位和第3位, 值为5 */
#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
static user_conn_info_head *g_user_conn_info_rd[USER_CONN_INFO_BUTT] = {NULL};
static special_frame_switch_stru g_special_frame_switch = {0};
static mac_cfg_rx_user_rate_stru *g_user_rx_rate_info = OSAL_NULL;
#endif

/*****************************************************************************
  3 函数实现  置换函数名称
*****************************************************************************/
osal_void dfx_set_service_control(osal_u32 value)
{
    g_hmac_service_control = value;
    return;
}

WIFI_HMAC_TCM_TEXT WIFI_TCM_TEXT osal_u32 dfx_get_service_control(osal_void)
{
    return g_hmac_service_control;
}

#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
OAL_STATIC osal_u32 dfx_user_head_exit(user_conn_info_head **conn_head)
{
    struct osal_list_head *node = NULL;
    user_conn_info_head *head = *conn_head;

    if (head != NULL) {
        osal_mutex_lock(&head->lock);
        while (osal_list_empty(&head->user_head) == 0) {
            node = head->user_head.next;
            osal_list_del(node);
            oal_free(node);
        }
        osal_mutex_unlock(&head->lock);
        osal_mutex_destroy(&head->lock);
        oal_free(head);
    }

    *conn_head = NULL;
    return DFX_SUCCESS;
}

OAL_STATIC osal_u32 dfx_user_head_init(user_conn_info_head **conn_head)
{
    user_conn_info_head *head = *conn_head;

    dfx_user_head_exit(conn_head);

    head = oal_memalloc(sizeof(user_conn_info_head));
    if (head == NULL) {
        return DFX_FAIL;
    }
    memset_s(head, sizeof(user_conn_info_head), 0, sizeof(user_conn_info_head));

    osal_mutex_init(&head->lock);
    osal_mutex_lock(&head->lock);
    OSAL_INIT_LIST_HEAD(&head->user_head);
    osal_mutex_unlock(&head->lock);

    *conn_head = head;
    return DFX_SUCCESS;
}

OAL_STATIC osal_u32 dfx_user_item_clear(user_conn_info_head *user_head)
{
    struct osal_list_head *node = NULL;

    if (user_head != NULL) {
        osal_mutex_lock(&user_head->lock);
        while (osal_list_empty(&user_head->user_head) == 0) {
            node = user_head->user_head.next;
            osal_list_del(node);
            oal_free(node);
        }
        user_head->user_num = 0;
        osal_mutex_unlock(&user_head->lock);
    }
    return DFX_SUCCESS;
}

OAL_STATIC osal_u32 dfx_user_rd_init(void)
{
    int i;
    osal_u32 ret;

    for (i = 0; i < USER_CONN_INFO_BUTT; i++) {
        ret = dfx_user_head_init(&g_user_conn_info_rd[i]);
        if (ret != DFX_SUCCESS) {
            wifi_printf("dfx_user_rd_init fail!\r\n");
            return DFX_FAIL;
        }
    }

    wifi_printf("dfx_user_rd_init success!\r\n");
    return DFX_SUCCESS;
}


OAL_STATIC osal_u32 dfx_user_rd_exit(void)
{
    int i;
    osal_u32 ret;

    for (i = 0; i < USER_CONN_INFO_BUTT; i++) {
        ret = dfx_user_head_exit(&g_user_conn_info_rd[i]);
        if (ret != DFX_SUCCESS) {
            wifi_printf("dfx_user_rd_exit fail!\r\n");
            return DFX_FAIL;
        }
    }

    wifi_printf("dfx_user_rd_exit success!\r\n");
    return DFX_SUCCESS;
}

/*****************************************************************************
 功能描述  : 校验规定次数失败记录，与当前时间 时间差均小于规定时间，判定异常
 返回值    : OAL_FALSE 不存在异常
             OAL_TRUE  存在异常
*****************************************************************************/
OAL_STATIC osal_void dfx_check_user_conn_fail_abnormal(user_unconn_reason_rd *user_node)
{
    long curr_time;
    osal_u8 unconn_rd_idx;
    osal_u8 curr_rd_idx;
    static long last_time = 0;
    osal_u8 fail_conn_cnt;

    if (user_node->user_rd_cnt < DFX_CONN_FAIL_CNT) {
        return;
    }
    /* 记录失败信息时,先保存数据,后更新下标,取当前时间时,需要下标前移一位 */
    curr_rd_idx = (user_node->user_rd_ind - 1 + MAX_CONN_INFO_RD_NUM) % MAX_CONN_INFO_RD_NUM;
    curr_time = user_node->unconn_rd[curr_rd_idx].fail_time.tv_sec;

    /* 规定次数内 时间差均小于规定时间 判定异常 */
    for (fail_conn_cnt = 1; fail_conn_cnt < DFX_CONN_FAIL_CNT; fail_conn_cnt++) {
        unconn_rd_idx = curr_rd_idx - fail_conn_cnt;
        if (curr_rd_idx < fail_conn_cnt) {
            unconn_rd_idx += MAX_CONN_INFO_RD_NUM;
        }

        if (curr_time - user_node->unconn_rd[unconn_rd_idx].fail_time.tv_sec > DFX_CONN_FAIL_PERIOD) {
            return;
        }
    }

    /* 为屏蔽恶意攻击，间隔1s内不触发异常 */
    if ((last_time != 0) && (curr_time - last_time < 1)) {
        return;
    }
    last_time = curr_time;
    oam_warning_log4(0, OAM_SF_DFT,
        "dfx_check_user_conn_abnormal::[%d] connection failures within [%d] seconds, cur_time[%d] fail_reason[%d]",
        DFX_CONN_FAIL_CNT, DFX_CONN_FAIL_PERIOD, curr_time, user_node->unconn_rd[curr_rd_idx].fail_reason);
}

/*****************************************************************************
 功能描述  : 校验规定次数下线记录，与当前时间 时间差均小于规定时间，判定异常
 返回值    : OAL_FALSE 不存在异常
             OAL_TRUE  存在异常
*****************************************************************************/
OAL_STATIC osal_void dfx_check_user_offline_abnormal(user_unconn_reason_rd *user_node)
{
    osal_u8 fail_conn_cnt;
    osal_u8 unconn_rd_idx;
    osal_u8 curr_rd_idx;
    long curr_time;
    static long last_time = 0;

    if (user_node->user_rd_cnt < DFX_CONN_FAIL_CNT) {
        return;
    }
    /* 记录失败信息时,先保存数据,后更新下标,取当前时间时,需要下标前移一位 */
    curr_rd_idx = (user_node->user_rd_ind - 1 + MAX_CONN_INFO_RD_NUM) % MAX_CONN_INFO_RD_NUM;
    curr_time = user_node->unconn_rd[curr_rd_idx].fail_time.tv_sec;

    /* 规定次数内 时间差均小于规定时间 判定异常 */
    for (fail_conn_cnt = 1; fail_conn_cnt < DFX_CONN_FAIL_CNT; fail_conn_cnt++) {
        if (curr_rd_idx < fail_conn_cnt) {
            unconn_rd_idx = curr_rd_idx - fail_conn_cnt + MAX_CONN_INFO_RD_NUM;
        } else {
            unconn_rd_idx = curr_rd_idx - fail_conn_cnt;
        }

        if (curr_time - user_node->unconn_rd[unconn_rd_idx].fail_time.tv_sec > DFX_OFFLINE_PERIOD) {
            return;
        }
    }

    /* 为屏蔽恶意攻击，间隔1s内不触发异常 */
    if ((last_time != 0) && (curr_time - last_time < 1)) {
        return;
    }
    last_time = curr_time;
    oam_warning_log4(0, OAM_SF_DFT,
        "dfx_check_user_offline_abnormal::[%d] offline within [%d] seconds, cur_time[%d] fail_reason[%d]",
        DFX_CONN_FAIL_CNT, DFX_OFFLINE_PERIOD, curr_time, user_node->unconn_rd[curr_rd_idx].fail_reason);
}

OAL_STATIC osal_void dfx_user_conn_rd_set(user_conn_info_rd *user_rd, osal_u16 reason_code)
{
    user_unconn_reason_rd *user_node = (user_unconn_reason_rd*)user_rd->user_rd;

    if (OAL_UNLIKELY(user_node == NULL || user_node->user_rd_ind >= MAX_CONN_INFO_RD_NUM)) {
        return;
    }

    user_node->unconn_rd[user_node->user_rd_ind].fail_reason = reason_code;
    osal_gettimeofday(&user_node->unconn_rd[user_node->user_rd_ind].fail_time);

    user_node->user_rd_ind++;
    if (user_node->user_rd_ind >= MAX_CONN_INFO_RD_NUM) {
        user_node->user_rd_ind = 0;
    }

    if (user_node->user_rd_cnt < MAX_CONN_INFO_RD_NUM) {
        user_node->user_rd_cnt++;
    }
}

OAL_STATIC oal_bool_enum dfx_check_user_is_valid(osal_u8 vap_id, osal_u8 *user_mac)
{
    osal_u32 ret;
    osal_u16 user_idx = 0;
    hmac_vap_stru *hmac_vap = OAL_PTR_NULL;
    hmac_user_stru *hmac_user = OAL_PTR_NULL;

    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(vap_id);
    if (hmac_vap == OAL_PTR_NULL) {
        oam_warning_log1(0, OAM_SF_ANY, "{dfx_check_user_is_valid::vap_id[%d] is invalid!}", vap_id);
        return OAL_FALSE;
    }
    ret = hmac_vap_find_user_by_macaddr_etc(hmac_vap, user_mac, &user_idx);
    if (ret != OAL_SUCC) {
        return OAL_FALSE;
    }
    hmac_user = mac_res_get_hmac_user_etc(user_idx);
    if (hmac_user == OAL_PTR_NULL) {
        return OAL_FALSE;
    }

    return OAL_TRUE;
}

OAL_STATIC user_conn_info_rd *dfx_user_find_and_alloc(user_conn_info_head *user_head,
    osal_u8 vap_id, osal_u8 *user_mac, osal_u16 user_rd_len)
{
    struct osal_list_head *node_tmp = NULL;
    struct osal_list_head *entry = NULL;
    user_conn_info_rd *node_rd = NULL;

    if ((OAL_UNLIKELY(user_mac == NULL || user_head == NULL || user_rd_len > MAX_DFX_USER_RD_LEN) ||
        dfx_check_user_is_valid(vap_id, user_mac) != OAL_TRUE) && (user_head->type != USER_CONN_OFFLINE)) {
        return NULL;
    }
    osal_mutex_lock(&user_head->lock);

    if ((user_head->user_cache != NULL) &&
        (osal_memcmp(((user_conn_info_rd*)user_head->user_cache)->user_mac, user_mac, WLAN_MAC_ADDR_LEN) == 0)) {
        osal_mutex_unlock(&user_head->lock);
        return (user_conn_info_rd *)user_head->user_cache;
    }

    osal_list_for_each_safe(entry, node_tmp, &user_head->user_head) {
        node_rd = osal_list_entry(entry, user_conn_info_rd, user_list);
        if (osal_memcmp(node_rd->user_mac, user_mac, WLAN_MAC_ADDR_LEN) == 0) {
            user_head->user_cache = (osal_void*)node_rd;
            osal_mutex_unlock(&user_head->lock);
            return node_rd;
        }
    }

    if (user_head->user_num >= MAX_DFX_USER_NUM) {
        node_tmp = user_head->user_head.prev;
        osal_list_del(node_tmp);
        node_rd = (user_conn_info_rd *)(uintptr_t)node_tmp;
    } else {
        node_rd = oal_memalloc(sizeof(user_conn_info_rd) + user_rd_len);
        if (node_rd == NULL) {
            osal_mutex_unlock(&user_head->lock);
            return NULL;
        }
        user_head->user_num++;
    }
    memset_s(node_rd, sizeof(user_conn_info_rd) + user_rd_len, 0, sizeof(user_conn_info_rd) + user_rd_len);

    osal_list_add(&node_rd->user_list, &user_head->user_head);
    if (memcpy_s(node_rd->user_mac, WLAN_MAC_ADDR_LEN, user_mac, WLAN_MAC_ADDR_LEN) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{dfx_user_find_and_alloc::memcpy_s error}");
    }
    node_rd->vap_id = vap_id;
    user_head->user_cache = (osal_void*)node_rd;

    osal_mutex_unlock(&user_head->lock);
    return node_rd;
}

OAL_STATIC osal_u32 dfx_user_conn_info_rd_dump(user_conn_info_head *user_head, OAL_CONST char *reason_desc[],
    osal_u16 desc_num)
{
    int i;
    struct osal_list_head   *node_tmp = NULL;
    struct osal_list_head   *entry = NULL;
    user_unconn_reason_rd   *unconn_rd = NULL;
    user_conn_info_rd       *node_rd = NULL;

    if (user_head == NULL) {
        return DFX_FAIL;
    }

    osal_mutex_lock(&user_head->lock);

    osal_list_for_each_safe(entry, node_tmp, &user_head->user_head) {
        node_rd = osal_list_entry(entry, user_conn_info_rd, user_list);
        unconn_rd = (user_unconn_reason_rd *)node_rd->user_rd;
        wifi_printf("user mac: %02x:%02x:%02x:%02x:XX:XX\r\n", node_rd->user_mac[0], node_rd->user_mac[1],
            node_rd->user_mac[2], node_rd->user_mac[3]);

        for (i = 0; i < MAX_CONN_INFO_RD_NUM; i++) {
            if (unconn_rd->unconn_rd[i].fail_reason != 0 && unconn_rd->unconn_rd[i].fail_reason < desc_num) {
                wifi_printf("%u: reason: %s\r\n", i, reason_desc[unconn_rd->unconn_rd[i].fail_reason]);
            }
        }
        wifi_printf("\r\n");
    }

    osal_mutex_unlock(&user_head->lock);
    return DFX_SUCCESS;
}

OAL_STATIC osal_u32 dfx_user_conn_fail_rd_dump(void)
{
    user_conn_info_head     *user_head = g_user_conn_info_rd[USER_CONN_FAIL];
    osal_u16                desc_num;

    if (user_head == NULL) {
        return DFX_FAIL;
    }
    desc_num = (osal_u16)oal_array_size(dfx_user_fail_reason_desc);

    wifi_printf("=========dfx_user_conn_fail_rd_dump=========\r\n");
    dfx_user_conn_info_rd_dump(user_head, dfx_user_fail_reason_desc, desc_num);

    return DFX_SUCCESS;
}

OAL_STATIC osal_u32 dfx_user_conn_offline_rd_dump(void)
{
    user_conn_info_head     *user_head = g_user_conn_info_rd[USER_CONN_OFFLINE];
    osal_u16                desc_num;

    if (user_head == NULL) {
        return DFX_FAIL;
    }
    desc_num = (osal_u16)oal_array_size(dfx_user_offline_reason_desc);
    wifi_printf("=========dfx_user_conn_offline_rd_dump=========\r\n");
    dfx_user_conn_info_rd_dump(user_head, dfx_user_offline_reason_desc, desc_num);

    return DFX_SUCCESS;
}

OAL_STATIC osal_void dfx_user_statistic_rd_dump(osal_u16 type)
{
#ifdef _PRE_WIFI_PRINTK
    int i;
    struct osal_list_head   *node_tmp = NULL;
    struct osal_list_head   *entry = NULL;
    user_statistic_rd       *record = NULL;
    user_conn_info_rd       *node_rd = NULL;
    user_conn_info_head     *user_head = g_user_conn_info_rd[type];
    hmac_vap_stru           *hmac_vap = NULL;
    mac_bss_dscr_stru       *bss_dscr = NULL;
    char *frame_stat[] = {"assoc_req", "assoc_rsp", "reassoc_req", "reassoc_rsp", "resv", "resv", "resv",
        "resv", "resv", "resv", "disassoc", "auth", "deauth", "resv", "resv", "resv", "resv"};

    if (user_head == NULL) {
        return;
    }

    if (type == USER_CONN_STA_STAT) {
        wifi_printf("=========dfx_sta_conn_statistic_rd_dump=========\r\n");
    } else {
        wifi_printf("=========dfx_ap_conn_statistic_rd_dump=========\r\n");
    }

    osal_mutex_lock(&user_head->lock);

    osal_list_for_each_safe(entry, node_tmp, &user_head->user_head) {
        node_rd = osal_list_entry(entry, user_conn_info_rd, user_list);
        record = (user_statistic_rd *)node_rd->user_rd;
        wifi_printf("user mac: %02x:%02x:%02x:%02x:XX:XX\r\n", node_rd->user_mac[0], node_rd->user_mac[1],
            node_rd->user_mac[2], node_rd->user_mac[3]); /* 2 3表示mac地址第2 3位 */

        if (type == USER_CONN_STA_STAT) {
            hmac_vap = (hmac_vap_stru*)mac_res_get_hmac_vap(node_rd->vap_id);
            bss_dscr = (mac_bss_dscr_stru *)hmac_scan_get_scanned_bss_by_bssid(hmac_vap, node_rd->user_mac);
            if (hmac_vap != NULL && bss_dscr != NULL) {
                wifi_printf("ssid: %s \r\n", bss_dscr->ac_ssid);
            }
        }

        for (i = 0; i < WLAN_MGMT_SUBTYPE_BUTT; i++) {
            if (i <= WLAN_REASSOC_RSP || i == WLAN_DISASOC || i == WLAN_AUTH || i == WLAN_DEAUTH) {
                wifi_printf("%s: tx[%u] rx[%u]\r\n", frame_stat[i], record->stat[OAM_OTA_FRAME_DIRECTION_TYPE_TX][i],
                    record->stat[OAM_OTA_FRAME_DIRECTION_TYPE_RX][i]);
            }
        }
        wifi_printf("\r\n");
    }

    osal_mutex_unlock(&user_head->lock);
#endif
}

OAL_STATIC osal_u32 dfx_user_statistic_rd_clear(osal_void)
{
    osal_u32 ret;

    ret = dfx_user_item_clear(g_user_conn_info_rd[USER_CONN_STA_STAT]);
    if (ret != DFX_SUCCESS) {
        wifi_printf("dfx_user_statistic_rd_clear fail!\r\n");
        return DFX_FAIL;
    }

    ret = dfx_user_item_clear(g_user_conn_info_rd[USER_CONN_AP_STAT]);
    if (ret != DFX_SUCCESS) {
        wifi_printf("dfx_user_statistic_rd_clear fail!\r\n");
        return DFX_FAIL;
    }

    wifi_printf("dfx_user_statistic_rd_clear success!\r\n");
    return DFX_SUCCESS;
}

OAL_STATIC osal_void dfx_user_conn_abnormal_record(osal_u8 vap_id, osal_u8 *mac, osal_u16 reason, osal_u16 type)
{
    user_conn_info_head *user_head = g_user_conn_info_rd[type];
    user_conn_info_rd *user_rd = NULL;

    if (user_head == NULL || mac == NULL) {
        return;
    }

    user_head->type = (osal_u32)type;
    user_rd = dfx_user_find_and_alloc(user_head, vap_id, mac, sizeof(user_unconn_reason_rd));
    if (user_rd == NULL) {
        return;
    }

    dfx_user_conn_rd_set(user_rd, reason);
    if (type == USER_CONN_FAIL) {
        dfx_check_user_conn_fail_abnormal((user_unconn_reason_rd*)user_rd->user_rd);
    } else if (type == USER_CONN_OFFLINE) {
        dfx_check_user_offline_abnormal((user_unconn_reason_rd*)user_rd->user_rd);
    }
}

OAL_STATIC osal_void dfx_user_conn_statistic_record(osal_u8 vap_id, osal_u8 *mac, osal_u8 sub_type,
    osal_u16 mode, osal_u8 direction)
{
    osal_u16 type = (mode == WLAN_VAP_MODE_BSS_STA) ? USER_CONN_STA_STAT : USER_CONN_AP_STAT;
    user_conn_info_head *user_head = g_user_conn_info_rd[type];
    user_conn_info_rd *user_rd = NULL;
    user_statistic_rd *rd = NULL;

    if (user_head == NULL || mac == NULL || sub_type >= WLAN_MGMT_SUBTYPE_BUTT ||
        direction >= OAM_OTA_FRAME_DIRECTION_TYPE_BUTT) {
        return;
    }

    user_head->type = (osal_u32)type;
    user_rd = dfx_user_find_and_alloc(user_head, vap_id, mac, sizeof(user_statistic_rd));
    if (user_rd == NULL) {
        return;
    }

    rd = (user_statistic_rd*)user_rd->user_rd;
    rd->stat[direction][sub_type]++;
}

/*****************************************************************************
 功能描述 : 解析ie中的chan信息，先在HT operation IE中找chan信息，
           如找不到，再在DSSS Param set ie中寻找，帧上报不做合法性校验
*****************************************************************************/
OAL_STATIC osal_u8 hmac_dfx_get_chan_num(osal_u8 *frame_body, osal_u16 payload_len, osal_u16 offset)
{
    osal_u8 *ie_start_addr;

    /* 在DSSS Param set ie中解析chan num */
    ie_start_addr = mac_find_ie_etc(MAC_EID_DSPARMS, frame_body + offset, payload_len - offset);
    if ((ie_start_addr != OSAL_NULL) && (ie_start_addr[1] == MAC_DSPARMS_LEN)) {
        return ie_start_addr[MAC_IE_HDR_LEN];
    }

    /* 在HT operation ie中解析 chan num */
    ie_start_addr = mac_find_ie_etc(MAC_EID_HT_OPERATION, frame_body + offset, payload_len - offset);
    if ((ie_start_addr != OSAL_NULL) && (ie_start_addr[1] >= 1)) {
        return ie_start_addr[MAC_IE_HDR_LEN];
    }

    return 0;
}

OAL_STATIC osal_void hmac_get_adjust_hso_param(oal_netbuf_stru *netbuf, osal_u8 direction,
    adjust_hso_param_stru *adjust_hso_param)
{
    osal_u8 sub_type;
    osal_u8 *mac_hdr = OSAL_NULL;
    mac_tx_ctl_stru *tx_ctl = OSAL_NULL;
    mac_rx_ctl_stru *rx_ctl = OSAL_NULL;
    hmac_vap_stru *hmac_vap = OSAL_NULL;
    osal_u8 frame_channel;
    osal_u16 offset = MAC_TIME_STAMP_LEN + MAC_BEACON_INTERVAL_LEN + MAC_CAP_INFO_LEN;

    if (netbuf == OSAL_NULL || adjust_hso_param == OSAL_NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "hmac_get_adjust_hso_param::netbuf or adjust_hso_param is NULL.");
        return;
    }

    if (direction >= OAM_OTA_FRAME_DIRECTION_TYPE_BUTT) {
        oam_warning_log1(0, OAM_SF_ANY, "hmac_get_adjust_hso_param::direction[%d] error.", direction);
        return;
    }

    if (direction == OAM_OTA_FRAME_DIRECTION_TYPE_TX) {
        tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(netbuf);
        mac_hdr = (osal_u8*)mac_get_frame_header_addr(tx_ctl);
        adjust_hso_param->rssi = 0;
        hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(tx_ctl->tx_vap_index);
        if (hmac_vap == OSAL_NULL) {
            oam_warning_log0(0, OAM_SF_DFT, "{hmac_get_adjust_hso_param:hmac_vap is NULL.}");
            return;
        }
        /* 屏蔽P2P扫描时，切换监听信道修改hmac_vap中channel的值 */
        if (hmac_vap->p2p_mode != WLAN_P2P_DEV_MODE) {
            adjust_hso_param->channel = hmac_vap->channel.chan_number;
        }
    } else {
        rx_ctl = (mac_rx_ctl_stru *)oal_netbuf_cb(netbuf);
        mac_hdr = (osal_u8 *)mac_get_rx_cb_mac_hdr(rx_ctl);
        frame_channel = hmac_dfx_get_chan_num((osal_u8*)oal_netbuf_rx_data(netbuf),
            rx_ctl->frame_len - MAC_80211_FRAME_LEN, offset);
        adjust_hso_param->rssi = ((dmac_rx_ctl_stru *)(uintptr_t)rx_ctl)->rx_statistic.rssi_dbm;
        adjust_hso_param->channel = frame_channel;
    }

    sub_type = mac_get_frame_type_and_subtype(mac_hdr);
    if (sub_type == (WLAN_FC0_SUBTYPE_PROBE_RSP | WLAN_FC0_TYPE_MGT) ||
        sub_type == (WLAN_FC0_SUBTYPE_BEACON | WLAN_FC0_TYPE_MGT)) {
        adjust_hso_param->is_probe_rsp_or_beacon = OSAL_TRUE;
    }

    /* 校验Beacon帧，防止上报接口的msg_id同时存在beacon帧和其他帧的情况 */
    if (sub_type == (WLAN_FC0_SUBTYPE_BEACON | WLAN_FC0_TYPE_MGT)) {
        adjust_hso_param->msg_id = (direction == OAM_OTA_FRAME_DIRECTION_TYPE_TX) ?
            SOC_DIAG_MSG_ID_WIFI_TX_BEACON : SOC_DIAG_MSG_ID_WIFI_RX_BEACON;
    }
}

/* 获取beacon帧上报开关 */
OAL_STATIC osal_u8 dft_get_beacon_frame_switch(osal_void)
{
    return g_special_frame_switch.beacon_switch;
}

/* 上报beacon帧 */
OAL_STATIC osal_void dft_beacon_frame_report(oal_netbuf_stru *netbuf, osal_u8 direction)
{
    osal_u8 beacon_frame_switch;
    osal_u8 *bcn_payload = OSAL_NULL;
    osal_u32 beacon_len;
    mac_rx_ctl_stru *rx_ctl = OSAL_NULL;
    adjust_hso_param_stru adjust_hso_param = {0};
    osal_u8 frame_channel;
    osal_u16 offset = MAC_TIME_STAMP_LEN + MAC_BEACON_INTERVAL_LEN + MAC_CAP_INFO_LEN;

    beacon_frame_switch = dft_get_beacon_frame_switch();
    if (beacon_frame_switch != DFT_SWITCH_ON) {
        return;
    }
    hmac_get_adjust_hso_param(netbuf, direction, &adjust_hso_param);
    if (direction == OAM_OTA_FRAME_DIRECTION_TYPE_TX) {
        bcn_payload = oal_netbuf_header(netbuf);
        beacon_len = oal_netbuf_get_len(netbuf);
        frame_channel = hmac_ie_get_chan_num_etc(bcn_payload + MAC_80211_FRAME_LEN,
            (osal_u16)(beacon_len - MAC_80211_FRAME_LEN), offset, 0);
        adjust_hso_param.channel = frame_channel;
        dft_report_80211_frame(bcn_payload, MAC_80211_FRAME_LEN, bcn_payload + MAC_80211_FRAME_LEN,
            (osal_u16)beacon_len, &adjust_hso_param);
    } else {
        rx_ctl = (mac_rx_ctl_stru *)oal_netbuf_cb(netbuf);
        dft_report_80211_frame((osal_u8 *)mac_get_rx_cb_mac_hdr(rx_ctl), rx_ctl->mac_header_len,
            oal_netbuf_rx_data_const(netbuf), rx_ctl->frame_len, &adjust_hso_param);
    }
}

OAL_STATIC osal_u8 dft_is_beacon_frame(oal_netbuf_stru *netbuf, osal_u8 direction)
{
    osal_u8 sub_type;
    osal_u8 *mac_hdr = OSAL_NULL;
    mac_tx_ctl_stru *tx_ctl = OSAL_NULL;
    mac_rx_ctl_stru *rx_ctl = OSAL_NULL;

    if (direction == OAM_OTA_FRAME_DIRECTION_TYPE_TX) {
        tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(netbuf);
        mac_hdr = (osal_u8*)mac_get_frame_header_addr(tx_ctl);
    } else {
        rx_ctl = (mac_rx_ctl_stru *)oal_netbuf_cb(netbuf);
        mac_hdr = (osal_u8 *)mac_get_rx_cb_mac_hdr(rx_ctl);
    }

    sub_type = mac_get_frame_type_and_subtype(mac_hdr);
    if (sub_type == (WLAN_FC0_SUBTYPE_BEACON | WLAN_FC0_TYPE_MGT)) {
        return OSAL_TRUE;
    }

    return OSAL_FALSE;
}

/* 获取vip帧的上报开关 */
OAL_STATIC osal_u8 dft_get_vip_frame_switch(osal_void)
{
    return g_special_frame_switch.vip_frame_switch;
}

OAL_STATIC osal_u8 dft_is_vip_frame(oal_netbuf_stru *netbuf, osal_u8 direction)
{
    osal_u8 frame_type;
    osal_u8 frame_subtype;
    osal_u8 data_type;
    osal_u8 *frame_hdr = OSAL_NULL;

    frame_hdr = oal_netbuf_header(netbuf);
    frame_type = mac_frame_get_type_value(frame_hdr);
    frame_subtype = mac_frame_get_subtype_value(frame_hdr);
    if (frame_type != WLAN_DATA_BASICTYPE || frame_subtype == WLAN_NULL_FRAME) {
        return OSAL_FALSE;
    }

    if (direction == OAM_OTA_FRAME_DIRECTION_TYPE_TX) {
        data_type = hmac_get_tx_data_type_etc(netbuf);
    } else {
        data_type = hmac_get_rx_data_type_etc(netbuf);
    }

    return (data_type <= MAC_DATA_ARP_REQ) ? OSAL_TRUE : OSAL_FALSE;
}

/* host侧VIP帧上报总接口 */
OAL_STATIC osal_void dft_vip_frame_report(oal_netbuf_stru *netbuf, osal_u8 direction)
{
    osal_u8 vip_frame_switch;
    mac_tx_ctl_stru *tx_ctl = OSAL_NULL;
    mac_rx_ctl_stru *rx_ctl = OSAL_NULL;
    adjust_hso_param_stru adjust_hso_param = {0};

    vip_frame_switch = dft_get_vip_frame_switch();
    if (vip_frame_switch != DFT_SWITCH_ON) {
        return;
    }
    if (direction == OAM_OTA_FRAME_DIRECTION_TYPE_TX) {
        tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(netbuf);
        /* 上报TX方向的VIP帧 */
        adjust_hso_param.msg_id = SOC_DIAG_MSG_ID_WIFI_TX_80211_FRAME;
        dft_report_80211_frame((osal_u8*)mac_get_frame_header_addr(tx_ctl), tx_ctl->frame_header_length,
            (osal_u8*)oal_netbuf_tx_data(netbuf), tx_ctl->mpdu_payload_len + tx_ctl->frame_header_length,
            &adjust_hso_param);
    } else {
        rx_ctl = (mac_rx_ctl_stru *)oal_netbuf_cb(netbuf);
        /* 上报RX方向的VIP帧 */
        adjust_hso_param.msg_id = SOC_DIAG_MSG_ID_WIFI_RX_80211_FRAME;
        dft_report_80211_frame((osal_u8 *)mac_get_rx_cb_mac_hdr(rx_ctl), rx_ctl->mac_header_len,
            oal_netbuf_rx_data_const(netbuf), rx_ctl->frame_len, &adjust_hso_param);
    }
}

/* host侧80211帧上报总接口 */
OAL_STATIC osal_void dfx_80211_frame_report(oal_netbuf_stru *netbuf, osal_u8 direction)
{
    dft_80211_frame_ctx frame_ctx = {0};
    mac_tx_ctl_stru *tx_ctl = OSAL_NULL;
    mac_rx_ctl_stru *rx_ctl = OSAL_NULL;
    adjust_hso_param_stru adjust_hso_param = {0};

    if (direction == OAM_OTA_FRAME_DIRECTION_TYPE_TX) {
        tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(netbuf);
        dft_get_frame_report_switch((osal_u8*)mac_get_frame_header_addr(tx_ctl), direction, &frame_ctx);
        /* 上报TX方向的帧 */
        if (frame_ctx.frame_content_switch == DFT_SWITCH_ON) {
            adjust_hso_param.msg_id = SOC_DIAG_MSG_ID_WIFI_TX_80211_FRAME;
            /* 适配HSO解析rssi和channel */
            hmac_get_adjust_hso_param(netbuf, direction, &adjust_hso_param);
            dft_report_80211_frame((osal_u8*)mac_get_frame_header_addr(tx_ctl), tx_ctl->frame_header_length,
                (osal_u8*)oal_netbuf_tx_data(netbuf), tx_ctl->mpdu_payload_len + tx_ctl->frame_header_length,
                &adjust_hso_param);
        }
    } else {
        rx_ctl = (mac_rx_ctl_stru *)oal_netbuf_cb(netbuf);
        dft_get_frame_report_switch((osal_u8 *)mac_get_rx_cb_mac_hdr(rx_ctl), direction, &frame_ctx);
        /* 上报RX方向的帧 */
        if (frame_ctx.frame_content_switch == DFT_SWITCH_ON) {
            adjust_hso_param.msg_id = SOC_DIAG_MSG_ID_WIFI_RX_80211_FRAME;
            /* 适配HSO解析rssi和channel */
            hmac_get_adjust_hso_param(netbuf, direction, &adjust_hso_param);
            dft_report_80211_frame((osal_u8 *)mac_get_rx_cb_mac_hdr(rx_ctl), rx_ctl->mac_header_len,
                oal_netbuf_rx_data_const(netbuf), rx_ctl->frame_len, &adjust_hso_param);
        }
    }
}

/* host侧80211帧上报总接口 */
OAL_STATIC osal_void dfx_frame_report_hso_etc(oal_netbuf_stru *netbuf, osal_u8 direction)
{
    dft_80211_frame_ctx frame_ctx = {0};
    mac_tx_ctl_stru *tx_ctl = OSAL_NULL;
    mac_rx_ctl_stru *rx_ctl = OSAL_NULL;

    if (dft_is_beacon_frame(netbuf, direction) != 0) {
        dft_beacon_frame_report(netbuf, direction);
    } else if (dft_is_vip_frame(netbuf, direction) != 0) {
        dft_vip_frame_report(netbuf, direction);
    } else {
        dfx_80211_frame_report(netbuf, direction);
    }

    if (direction == OAM_OTA_FRAME_DIRECTION_TYPE_TX) {
        tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(netbuf);
        dft_get_frame_report_switch((osal_u8*)mac_get_frame_header_addr(tx_ctl), direction, &frame_ctx);
    } else {
        rx_ctl = (mac_rx_ctl_stru *)oal_netbuf_cb(netbuf);
        dft_get_frame_report_switch((osal_u8 *)mac_get_rx_cb_mac_hdr(rx_ctl), direction, &frame_ctx);
    }

    if (frame_ctx.frame_cb_switch == DFT_SWITCH_ON) {
        dft_report_netbuf_cb_etc((osal_u8 *)oal_netbuf_cb(netbuf), OAL_MAX_CB_LEN,
            (direction == OAM_OTA_FRAME_DIRECTION_TYPE_TX) ? SOC_DIAG_MSG_ID_WIFI_TX_CB : SOC_DIAG_MSG_ID_WIFI_RX_CB);
    }
}

WIFI_HMAC_TCM_TEXT WIFI_TCM_TEXT OAL_STATIC osal_void dfx_frame_report_hso(oal_netbuf_stru *netbuf, osal_u8 direction)
{
    if ((netbuf == OSAL_NULL) || (direction >= OAM_OTA_FRAME_DIRECTION_TYPE_BUTT) ||
        (dft_get_global_frame_switch() != DFT_SWITCH_ON)) {
        return;
    }

    dfx_frame_report_hso_etc(netbuf, direction);
}

OAL_STATIC osal_void dfx_user_rate_rx_info_record_etc(osal_u8 id, dmac_rx_ctl_stru *cb_ctrl)
{
    if (cb_ctrl->rx_status.he_flag != 0) {
        g_user_rx_rate_info[id].protocol_type = WLAN_HE_SU_FORMAT;
        g_user_rx_rate_info[id].nss_mode = cb_ctrl->rx_statistic.nss_rate.he_nss_mcs.nss_mode;
    } else {
        g_user_rx_rate_info[id].protocol_type = cb_ctrl->rx_statistic.nss_rate.legacy_rate.protocol_mode;
    }
    g_user_rx_rate_info[id].rssi_dbm = cb_ctrl->rx_statistic.rssi_dbm;
    if (g_user_rx_rate_info[id].protocol_type == WLAN_HT_PHY_PROTOCOL_MODE) {
        g_user_rx_rate_info[id].nss_rate = cb_ctrl->rx_statistic.nss_rate.ht_rate.ht_mcs;
    } else {
        g_user_rx_rate_info[id].nss_rate = cb_ctrl->rx_statistic.nss_rate.legacy_rate.legacy_rate;
    }
    g_user_rx_rate_info[id].freq_bandwidth_mode = cb_ctrl->rx_status.freq_bandwidth_mode;
    g_user_rx_rate_info[id].gi_type = cb_ctrl->rx_status.gi;
    g_user_rx_rate_info[id].preabmle = cb_ctrl->rx_status.preabmle;
    g_user_rx_rate_info[id].snr_ant0 = cb_ctrl->rx_statistic.snr_ant0;
}

WIFI_HMAC_TCM_TEXT WIFI_TCM_TEXT OAL_STATIC osal_void dfx_user_rate_rx_info_record(osal_u8 id,
    dmac_rx_ctl_stru *cb_ctrl)
{
    hmac_user_stru *hmac_user = OSAL_NULL;

    if (g_user_rx_rate_info == OSAL_NULL) {
        return;
    }

    if (id > WLAN_USER_MAX_USER_LIMIT) {
        return;
    }

    hmac_user = (hmac_user_stru *)mac_res_get_hmac_user_etc(id);
    if (hmac_user == OSAL_NULL) {
        return;
    }

    dfx_user_rate_rx_info_record_etc(id, cb_ctrl);
}

#ifdef _PRE_WIFI_PRINTK
OAL_STATIC osal_void dfx_print_user_info(const hmac_vap_stru *hmac_vap, const hmac_user_stru *hmac_user,
    const mac_cfg_ar_tx_params_stru *rsp_info, const mac_cfg_rx_user_rate_stru *rx_user_rate_fill,
    const mac_cfg_tx_user_rate_stru *tx_user_rate_fill)
{
    char *protocol_string[] = {"11b", "11g/a", "11n", "11ac", "11ax", "11ax", "11ax", "11ax", "error"};
    char *negotiate_string[] = {"11a", "11b", "11g", "11bg", "11g", "11n", "11ac", "11n",
        "11ac", "11ag", "11ax", "error"};
    char *preamble_string[] = {"short", "long", "error"};
    char *gi_string[] = {"long", "short", "mid", "error"};
    char *freq_bw_string[] = {"20M", "20M", "20M", "40M", "40M(DUP)", "error"};
    char *bandwidth_string[] = {"20M", "40M", "40M", "80M", "80M", "80M", "80M", "5M", "10M", "40M", "80M", "error"};

    // 打印出信道、带宽、协商速率
    wifi_printf("[negotiation info]: channel:%d, bandwidth:%s, protocol:%s, best_rate:%dkbps\r\n",
        hmac_vap->channel.chan_number, bandwidth_string[hmac_vap->channel.en_bandwidth],
        negotiate_string[hmac_user->avail_protocol_mode], rsp_info->tx_best_rate);

    // 打印出tx描述符和rx描述符中的相关信息
    if (g_user_rx_rate_info != OSAL_NULL) {
        wifi_printf("[rx info]: protocol:%s, rssi:%ddbm, mcs:%d, freq_bw:%s, gi:%s, preamble:%s, snr:%ddB\r\n",
            protocol_string[rx_user_rate_fill->protocol_type], rx_user_rate_fill->rssi_dbm,
            rx_user_rate_fill->nss_rate, freq_bw_string[rx_user_rate_fill->freq_bandwidth_mode],
            gi_string[rx_user_rate_fill->gi_type], preamble_string[rx_user_rate_fill->preabmle],
            rx_user_rate_fill->snr_ant0);
        wifi_printf("[tx info]: protocol:%s, rssi:%ddbm, mcs:%d, freq_bw:%s, gi:%s, preamble:%s, ",
            protocol_string[tx_user_rate_fill->protocol_type], tx_user_rate_fill->rssi_dbm,
            tx_user_rate_fill->nss_rate, freq_bw_string[tx_user_rate_fill->freq_bw],
            gi_string[tx_user_rate_fill->gi_type], preamble_string[tx_user_rate_fill->preamble_mode]);
        wifi_printf("max_retransmit_cnt:%d, transmit_power:%d(0.1dbm)", tx_user_rate_fill->data_cnt,
            /* tx_user_rate_fill->transmit_power 每加1表示降低0.5dB, 乘以5表示减低了多少dB */
            fe_custom_get_chip_max_power(WLAN_BAND_2G) - tx_user_rate_fill->transmit_power * 5);
        wifi_printf("\r\n");
    }
    wifi_printf("\r\n");
}
#endif

OAL_STATIC osal_u32 dfx_show_user_info(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user)
{
    osal_s32 ret;
    frw_msg cfg_info;
    frw_msg tx_cfg_info;
    mac_cfg_ar_tx_params_stru ar_tx_params;
    mac_cfg_ar_tx_params_stru rsp_info;
    mac_cfg_tx_user_rate_stru tx_user_rate_fill;
    mac_cfg_rx_user_rate_stru rx_user_rate_fill;
    osal_u16 user_id = hmac_user->assoc_id;

    (osal_void)memset_s(&cfg_info, sizeof(frw_msg), 0, sizeof(frw_msg));
    (osal_void)memset_s(&tx_cfg_info, sizeof(frw_msg), 0, sizeof(frw_msg));
    (osal_void)memset_s(&ar_tx_params, sizeof(mac_cfg_ar_tx_params_stru), 0, sizeof(mac_cfg_ar_tx_params_stru));
    (osal_void)memset_s(&rsp_info, sizeof(mac_cfg_ar_tx_params_stru), 0, sizeof(mac_cfg_ar_tx_params_stru));
    (osal_void)memset_s(&tx_user_rate_fill, sizeof(mac_cfg_tx_user_rate_stru), 0, sizeof(mac_cfg_tx_user_rate_stru));
    (osal_void)memset_s(&rx_user_rate_fill, sizeof(mac_cfg_rx_user_rate_stru), 0, sizeof(mac_cfg_rx_user_rate_stru));

    ar_tx_params.user_id = user_id;
    cfg_msg_init((osal_u8 *)&ar_tx_params, OAL_SIZEOF(mac_cfg_ar_tx_params_stru),
        (osal_u8 *)&rsp_info, OAL_SIZEOF(mac_cfg_ar_tx_params_stru), &cfg_info);
    ret = send_cfg_to_device(hmac_vap->vap_id, WLAN_MSG_H2D_C_CFG_GET_TX_PARAMS, &cfg_info, OSAL_TRUE);
    if ((ret != OAL_SUCC) || (cfg_info.rsp == OSAL_NULL)) {
        oam_warning_log2(0, OAM_SF_ANY, "{dfx_show_user_info::user_id = %u, ret:%d}", user_id, ret);
        return (osal_u32)ret;
    }

    cfg_msg_init((osal_u8 *)&user_id, OAL_SIZEOF(osal_u16),
        (osal_u8 *)&tx_user_rate_fill, OAL_SIZEOF(mac_cfg_tx_user_rate_stru), &tx_cfg_info);
    ret = send_cfg_to_device(hmac_vap->vap_id, WLAN_MSG_H2D_C_CFG_GET_TX_USER_RATE_INFO, &tx_cfg_info, OSAL_TRUE);
    if (OAL_UNLIKELY(ret != OAL_SUCC)) {
        oam_warning_log2(0, OAM_SF_ANY, "vap_id[%d] {dfx_show_user_info::return err code [%d]!}",
                         hmac_vap->vap_id, ret);
        return (osal_u32)ret;
    }

    if (g_user_rx_rate_info != OSAL_NULL) {
        (osal_void)memcpy_s(&rx_user_rate_fill, sizeof(mac_cfg_rx_user_rate_stru), &g_user_rx_rate_info[user_id],
            sizeof(mac_cfg_rx_user_rate_stru));

        /* SNR单位需要做转换，从报文描述符中取出的SNR单位不是dBm */
        hal_phy_rx_get_snr_info(hal_chip_get_hal_device(), (uint8_t)rx_user_rate_fill.protocol_type,
            (osal_u16)g_user_rx_rate_info[user_id].snr_ant0, &rx_user_rate_fill.snr_ant0);
    }

#ifdef _PRE_WIFI_PRINTK
    dfx_print_user_info(hmac_vap, hmac_user, &rsp_info, &rx_user_rate_fill, &tx_user_rate_fill);
#endif
    return OAL_SUCC;
}

OAL_STATIC osal_void dfx_show_all_user_info(hmac_vap_stru *hmac_vap)
{
    struct osal_list_head *entry = OAL_PTR_NULL;
    struct osal_list_head *dlist_tmp = OAL_PTR_NULL;
    hmac_user_stru *hmac_user = OAL_PTR_NULL;
    osal_u32 ret;

    osal_list_for_each_safe(entry, dlist_tmp, &(hmac_vap->mac_user_list_head)) {
        hmac_user = osal_list_entry(entry, hmac_user_stru, user_dlist);
        if (hmac_user == OAL_PTR_NULL) {
            continue;
        }

        wifi_printf("[mac addr]:%02X:%02X:%02X:%02X:XX:XX, assoc_id:%d\r\n",
            hmac_user->user_mac_addr[0], hmac_user->user_mac_addr[1], // 0, 1 is mac_addr
            hmac_user->user_mac_addr[2], hmac_user->user_mac_addr[3], // 2, 3 is mac_addr
            hmac_user->assoc_id);
        ret = dfx_show_user_info(hmac_vap, hmac_user);
        if (ret != OAL_SUCC) {
            oam_warning_log0(0, OAM_SF_ANY, "{dfx_show_all_user_info::no such user!}\r\n");
            return;
        }
    }
}

#if defined(_PRE_OS_VERSION_LITEOS) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
OAL_STATIC osal_s32 dfx_get_rx_user_rate_info(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    osal_u8 user_id;
    mac_cfg_rx_user_rate_stru rate_info;
    osal_u8 rate_info_switch = EXT_SWITCH_ON;
    frw_msg rate_msg;
    osal_s32 ret;

    unref_param(hmac_vap);
    if (msg == OAL_PTR_NULL || msg->data == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_CFG, "{dfx_get_rx_user_rate_info:: msg is null ptr!}");
        return OAL_FAIL;
    }

    (osal_void)memset_s(&rate_info, sizeof(mac_cfg_rx_user_rate_stru), 0, sizeof(mac_cfg_rx_user_rate_stru));
    /* 打开速率信息上报使能开关才会有数值 */
    if (g_user_rx_rate_info == OSAL_NULL) {
        g_user_rx_rate_info = (mac_cfg_rx_user_rate_stru *)oal_mem_alloc(OAL_MEM_POOL_ID_LOCAL,
            WLAN_USER_MAX_USER_LIMIT * sizeof(mac_cfg_rx_user_rate_stru), OAL_TRUE);
        (osal_void)memset_s((osal_u8 *)g_user_rx_rate_info,
            WLAN_USER_MAX_USER_LIMIT * sizeof(mac_cfg_rx_user_rate_stru),
            0, WLAN_USER_MAX_USER_LIMIT * sizeof(mac_cfg_rx_user_rate_stru));
        frw_msg_init((osal_u8 *)&rate_info_switch, sizeof(rate_info_switch), OSAL_NULL, 0, &rate_msg);
        ret = frw_send_msg_to_device(0, WLAN_MSG_H2D_C_CFG_USER_RATE_ENABLE, &rate_msg, OSAL_TRUE);
        if (ret != OAL_SUCC) {
            oal_mem_free(g_user_rx_rate_info, OAL_TRUE);
            g_user_rx_rate_info = OSAL_NULL;
            oam_warning_log1(0, OAM_SF_CFG, "{dfx_get_rx_user_rate_info::send rate_msg fail[%d].}", ret);
        }
    }
    user_id = *(msg->data);
    if (memcpy_s(msg->rsp, sizeof(mac_cfg_rx_user_rate_stru), (osal_u8 *)&g_user_rx_rate_info[user_id],
        sizeof(mac_cfg_rx_user_rate_stru)) != EOK) {
        oam_error_log0(0, OAM_SF_DFR, "{dfx_get_rx_user_rate_info::memcpy_s failed.}");
        return OAL_FAIL;
    }

    msg->rsp_len = (osal_u16)sizeof(rate_info);
    return OAL_SUCC;
}

OAL_STATIC osal_s32 dfx_show_sta_info(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    unref_param(msg);
    dfx_show_all_user_info(hmac_vap);
    return OAL_SUCC;
}
#endif

OAL_STATIC osal_s32 hmac_ccpriv_show_device_meminfo(hmac_vap_stru *hmac_vap, const osal_s8 *param)
{
    osal_s32 ret;
    osal_s8 cmd[CCPRIV_CMD_NAME_MAX_LEN] = {0};
    osal_u32 dfx_switch = 0;
    frw_msg msg_info;

    ret = hmac_ccpriv_get_one_arg(&param, cmd, OAL_SIZEOF(cmd));
    if (ret != OAL_SUCC) {
        /* 仅打印一次内存信息 */
        frw_msg_init((osal_u8 *)OSAL_NULL, 0, OSAL_NULL, 0, &msg_info);
        ret = frw_send_msg_to_device(hmac_vap->vap_id, WLAN_MSG_H2D_C_CFG_DEVICE_MEM_INFO, &msg_info, OSAL_TRUE);
        if (ret != OAL_SUCC) {
            oam_warning_log1(0, OAM_SF_CFG, "hmac_ccpriv_show_device_meminfo:send msg fail[%d]", ret);
        }
        return ret;
    }
    if (osal_strcmp((const osal_char *)cmd, "dfx_switch") != 0) {
        oam_warning_log0(0, OAM_SF_ANY, "{hmac_ccpriv_show_device_meminfo::not support cmd!}");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }
    ret = hmac_ccpriv_get_digit_with_check_max(&param, EXT_SWITCH_ON, &dfx_switch);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_ccpriv_show_device_meminfo::get dfx_switch err[%d]!}", ret);
        return ret;
    }

    frw_msg_init((osal_u8 *)&dfx_switch, sizeof(osal_u32), OSAL_NULL, 0, &msg_info);
    ret = frw_send_msg_to_device(hmac_vap->vap_id, WLAN_MSG_H2D_C_CFG_DEVICE_MEM_INFO, &msg_info, OSAL_TRUE);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_CFG, "hmac_ccpriv_show_device_meminfo:send switch fail[%d]", ret);
    }
    return ret;
}

OAL_STATIC osal_s32 hmac_ccpriv_get_user_conn_record(hmac_vap_stru *hmac_vap, const osal_s8 *param)
{
    osal_s32 ret;
    osal_s8 cmd[CCPRIV_CMD_NAME_MAX_LEN] = {0};
    osal_u8 param_index;
    osal_u8 param_max_num;
    const char *dfx_user_conn_info_param_name[] = {"enable", "disable", "fail", "offline", "stat", "stat_clr"};

    unref_param(hmac_vap);
    ret = hmac_ccpriv_get_one_arg(&param, cmd, OAL_SIZEOF(cmd));
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_ccpriv_get_user_conn_record::parse arg failed [%d]!}", ret);
        return ret;
    }

    param_max_num = OAL_ARRAY_SIZE(dfx_user_conn_info_param_name);

    for (param_index = 0; param_index < param_max_num; param_index++) {
        if (osal_strcmp(dfx_user_conn_info_param_name[param_index], (const osal_char *)cmd) == 0) {
            break;
        }
    }
    if (param_index == param_max_num) {
        oam_warning_log0(0, OAM_SF_ANY, "{hmac_ccpriv_get_user_conn_record::no such param for get_user_conn_record!}");
        return OAL_FAIL;
    }

    switch (param_index) {
        case DFX_USER_CONN_INFO_RECORD_CMD_ENABLE:
            dfx_user_rd_init();
            break;
        case DFX_USER_CONN_INFO_RECORD_CMD_DISABLE:
            dfx_user_rd_exit();
            break;
        case DFX_USER_CONN_INFO_RECORD_CMD_FAIL:
            dfx_user_conn_fail_rd_dump();
            break;
        case DFX_USER_CONN_INFO_RECORD_CMD_OFFLINE:
            dfx_user_conn_offline_rd_dump();
            break;
        case DFX_USER_CONN_INFO_RECORD_CMD_STATISTIC:
            dfx_user_statistic_rd_dump(USER_CONN_STA_STAT);
            dfx_user_statistic_rd_dump(USER_CONN_AP_STAT);
            break;
        case DFX_USER_CONN_INFO_RECORD_CMD_STATCLR:
            dfx_user_statistic_rd_clear();
            break;
        default:
            oam_warning_log1(0, OAM_SF_ANY, "{invalid user_conn_info_cmd[%d].}", param_index);
    }
    return OAL_SUCC;
}

OAL_STATIC osal_s32 hmac_ccpriv_set_global_frame_switch(hmac_vap_stru *hmac_vap, const osal_s8 *param)
{
    osal_s32 ret;
    osal_s32 msg_ret;
    osal_u8 global_frame_switch;
    frw_msg msg;

    ret = hmac_ccpriv_get_u8_with_check_max(&param, EXT_SWITCH_ON, &global_frame_switch);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_ccpriv_set_global_frame_switch::parse value err[%d]!}", ret);
        return ret;
    }

    ret = (osal_s32)dft_set_global_frame_switch(global_frame_switch);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_CFG, "{hmac_ccpriv_set_global_frame_switch::set global frame switch failed!}\r\n");
        return ret;
    }

    frw_msg_init((osal_u8 *)&global_frame_switch, sizeof(osal_u8), OSAL_NULL, 0, &msg);
    msg_ret = frw_send_msg_to_device(hmac_vap->vap_id, WLAN_MSG_H2D_C_CFG_DFT_GLOBAL_FRAME_SWITCH, &msg, OSAL_TRUE);
    if (msg_ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_CFG,
            "{hmac_ccpriv_set_global_frame_switch: frw_send_msg_to_device failed ret:%d!}", msg_ret);
    }

    wifi_printf("global_frame_switch[%u]\r\n", global_frame_switch);
    return OAL_SUCC;
}

OAL_STATIC osal_s32 hmac_ccpriv_set_beacon_frame_switch(hmac_vap_stru *hmac_vap, const osal_s8 *param)
{
    osal_s32 ret;
    osal_u8 beacon_switch;

    ret = hmac_ccpriv_get_u8_with_check_max(&param, EXT_SWITCH_ON, &beacon_switch);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_ccpriv_set_beacon_switch::set beacon_switch err[%u]!}", ret);
        return ret;
    }

    g_special_frame_switch.beacon_switch = beacon_switch;
    oam_warning_log1(0, OAM_SF_DFT, "{hmac_ccpriv_set_beacon_switch [%d].}", g_special_frame_switch.beacon_switch);
    wifi_printf("beacon_frame_switch[%d]\r\n", beacon_switch);

    if (beacon_switch == DFT_SWITCH_OFF || dft_get_global_frame_switch() == DFT_SWITCH_ON) {
        return OAL_SUCC;
    }
    /* beacon开关启用，自动开启帧上报总开关 */
    ret = hmac_ccpriv_set_global_frame_switch(hmac_vap, (const osal_s8 *)"1");
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_ccpriv_set_beacon_switch::set global_frame_switch err[%d]!}", ret);
        return ret;
    }

    return OAL_SUCC;
}

OAL_STATIC osal_s32 hmac_ccpriv_set_vip_frame_switch(hmac_vap_stru *hmac_vap, const osal_s8 *param)
{
    osal_s32 ret;
    osal_u8 vip_frame_switch;

    ret = hmac_ccpriv_get_u8_with_check_max(&param, EXT_SWITCH_ON, &vip_frame_switch);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_ccpriv_set_vip_frame_switch::set vip_frame_switch err[%u]!}", ret);
        return ret;
    }

    g_special_frame_switch.vip_frame_switch = vip_frame_switch;
    oam_warning_log1(0, OAM_SF_DFT, "{hmac_ccpriv_set_vip_frame_switch [%d]}", g_special_frame_switch.vip_frame_switch);
    wifi_printf("vip_frame_switch[%d]\r\n", vip_frame_switch);

    if (vip_frame_switch == DFT_SWITCH_OFF || dft_get_global_frame_switch() == DFT_SWITCH_ON) {
        return OAL_SUCC;
    }
    /* vip开关启用，自动开启帧上报总开关 */
    ret = hmac_ccpriv_set_global_frame_switch(hmac_vap, (const osal_s8 *)"1");
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_ccpriv_set_vip_frame_switch::set global_frame_switch err[%d]!}", ret);
        return ret;
    }

    return OAL_SUCC;
}

OAL_STATIC osal_s32 hmac_ccpriv_set_80211_frame_switch(hmac_vap_stru *hmac_vap, const osal_s8 *param)
{
    osal_s32 ret;
    osal_u32 msg_ret;
    mac_cfg_80211_frame_switch_stru frame_switch_stru;
    dft_80211_frame_ctx frame_ctx;
    frw_msg msg;

    /* sh ccpriv.sh "vap0 80211_frame_switch 0|1(帧类型：单播|组播) 0|1(帧方向tx|rx) 0|1(帧类型:管理帧|数据帧)
                                             0|1(帧内容开关) 0|1(CB开关) 0|1(描述符开关)"
    */
    memset_s(&frame_switch_stru, OAL_SIZEOF(mac_cfg_80211_frame_switch_stru),
        0, OAL_SIZEOF(mac_cfg_80211_frame_switch_stru));
    if ((hmac_ccpriv_get_u8_with_check_max(&param, EXT_SWITCH_ON, &frame_switch_stru.frame_mode) != OAL_SUCC) ||
        (hmac_ccpriv_get_u8_with_check_max(&param, EXT_SWITCH_ON, &frame_switch_stru.frame_direction) != OAL_SUCC) ||
        (hmac_ccpriv_get_u8_with_check_max(&param, EXT_SWITCH_ON, &frame_switch_stru.frame_type) != OAL_SUCC) ||
        (hmac_ccpriv_get_u8_with_check_max(&param, EXT_SWITCH_ON, &frame_switch_stru.frame_switch) != OAL_SUCC) ||
        (hmac_ccpriv_get_u8_with_check_max(&param, EXT_SWITCH_ON, &frame_switch_stru.cb_switch) != OAL_SUCC) ||
        (hmac_ccpriv_get_u8_with_check_max(&param, EXT_SWITCH_ON, &frame_switch_stru.dscr_switch) != OAL_SUCC)) {
        oam_warning_log0(0, OAM_SF_ANY, "{hmac_ccpriv_set_80211_frame_switch parse param fail!}");
        return OAL_FAIL;
    }

    /* 操作所有用户的帧上报开关 */
    memset_s(&frame_ctx, sizeof(dft_80211_frame_ctx), 0, sizeof(dft_80211_frame_ctx));
    frame_ctx.frame_content_switch = frame_switch_stru.frame_switch;
    frame_ctx.frame_cb_switch = frame_switch_stru.cb_switch;
    frame_ctx.frame_dscr_switch = frame_switch_stru.dscr_switch;

    msg_ret = dft_report_80211_frame_set_switch(frame_switch_stru.frame_mode, frame_switch_stru.frame_direction,
        frame_switch_stru.frame_type, &frame_ctx);
    wifi_printf("set frame switch::frame_mode[%d], frame_direction[%d], frame_type[%d]\r\n",
        frame_switch_stru.frame_mode, frame_switch_stru.frame_direction, frame_switch_stru.frame_type);
    wifi_printf("set frame switch::frame_switch[%d], cb_switch[%d], dscr_switch[%d]\r\n",
        frame_ctx.frame_content_switch, frame_ctx.frame_cb_switch, frame_ctx.frame_dscr_switch);
    if (msg_ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_CFG, "{hmac_ccpriv_set_80211_frame_switch::set switch failed[%d]!.}", msg_ret);
        return (osal_s32)msg_ret;
    }
    /* 同步device侧开关 */
    frw_msg_init((osal_u8 *)&frame_switch_stru, sizeof(frame_switch_stru), OSAL_NULL, 0, &msg);
    ret = frw_send_msg_to_device(hmac_vap->vap_id, WLAN_MSG_H2D_C_CFG_DFT_80211_FRAME_SWITCH, &msg, OSAL_TRUE);
    if (ret != OAL_SUCC) {
        oam_warning_log4(0, OAM_SF_CFG,
            "{hmac_ccpriv_set_80211_frame_switch::failed[%d]!frame_switch[%d],cb_switch[%d],dscr_switch[%d].}",
            ret, frame_switch_stru.frame_switch, frame_switch_stru.cb_switch, frame_switch_stru.dscr_switch);
        return ret;
    }
    if ((frame_ctx.frame_content_switch == DFT_SWITCH_OFF && frame_ctx.frame_cb_switch == DFT_SWITCH_OFF &&
        frame_ctx.frame_dscr_switch == DFT_SWITCH_OFF) || dft_get_global_frame_switch() == DFT_SWITCH_ON) {
        return OAL_SUCC;
    }
    /* 任意开关启用，自动开启帧上报总开关 */
    ret = hmac_ccpriv_set_global_frame_switch(hmac_vap, (const osal_s8 *)"1");
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_ccpriv_set_vip_frame_switch::set global_frame_switch err[%d]!}", ret);
        return ret;
    }

    return OAL_SUCC;
}

OAL_STATIC osal_s32 hmac_ccpriv_get_scan_result(hmac_vap_stru *hmac_vap, const osal_s8 *param)
{
    hmac_device_stru        *hmac_device;
    hmac_bss_mgmt_stru      *bss_mgmt;
    hmac_scanned_bss_info   *scanned_bss;
    mac_bss_dscr_stru       *bss_dscr;
    struct osal_list_head     *entry;

    unref_param(param);
    hmac_device = hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    if (hmac_device == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_ROAM, "{hmac_ccpriv_get_scan_result::device is null}");
        return OAL_ERR_CODE_ROAM_INVALID_VAP;
    }

    bss_mgmt = &(hmac_device->scan_mgmt.scan_record_mgmt.bss_mgmt);

    osal_spin_lock(&(bss_mgmt->lock));

    wifi_printf("==================SCAN RESULT==================\n");
    osal_list_for_each(entry, &(bss_mgmt->bss_list_head)) {
        scanned_bss = osal_list_entry(entry, hmac_scanned_bss_info, dlist_head);
        bss_dscr = &(scanned_bss->bss_dscr_info);

        if (hmac_device->band_cap == WLAN_BAND_CAP_2G &&
            bss_dscr->st_channel.chan_number > 14) { /* 2g频段 超出14信道不打印 */
            continue;
        } else if (hmac_device->band_cap == WLAN_BAND_CAP_5G &&
            bss_dscr->st_channel.chan_number < 36) { /* 5g频段 低于36信道不打印 */
            continue;
        }

        wifi_printf("[SSID]:%s\r\n", bss_dscr->ac_ssid);
        wifi_printf("[MAC ADDR]:%02X:%02X:%02X:%02X:XX:XX\r\n",
            bss_dscr->auc_mac_addr[0], bss_dscr->auc_mac_addr[1], /* MAC地址的第0字节.第1字节 */
            bss_dscr->auc_mac_addr[2], bss_dscr->auc_mac_addr[3]); /* MAC地址的第2字节.第3字节 */
        wifi_printf("[rssi]:%d\r\n", bss_dscr->c_rssi);
        wifi_printf("[channel]:%d\r\n", bss_dscr->st_channel.chan_number);
    }

    osal_spin_unlock(&(bss_mgmt->lock));

    return OAL_SUCC;
}

OAL_STATIC osal_s32 hmac_ccpriv_enable_user_rate_info(hmac_vap_stru *hmac_vap, const osal_s8 *param)
{
    osal_s32 ret;
    osal_u8 record_switch;
    frw_msg msg;

    unref_param(hmac_vap);
    if (hmac_ccpriv_get_u8_with_check_max(&param, EXT_SWITCH_ON, &record_switch) != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_CFG, "{hmac_ccpriv_enable_user_rate_info::param error.}");
        return OAL_FAIL;
    }

    // 关闭开关并且之前打开过，则释放内存
    if (record_switch == 0) {
        if (g_user_rx_rate_info == OSAL_NULL) {
            oam_warning_log0(0, OAM_SF_CFG, "{hmac_ccpriv_enable_user_rate_info::before not malloc memory.}");
        } else {
            oam_warning_log0(0, OAM_SF_CFG, "{hmac_ccpriv_enable_user_rate_info::free the memory.}");
            oal_mem_free(g_user_rx_rate_info, OAL_TRUE);
            g_user_rx_rate_info = OSAL_NULL;
        }
    } else {
        if (g_user_rx_rate_info != OSAL_NULL) {
            (osal_void)memset_s((osal_u8 *)g_user_rx_rate_info,
                WLAN_USER_MAX_USER_LIMIT * sizeof(mac_cfg_rx_user_rate_stru),
                0, WLAN_USER_MAX_USER_LIMIT * sizeof(mac_cfg_rx_user_rate_stru));
            oam_warning_log0(0, OAM_SF_CFG, "{hmac_ccpriv_enable_user_rate_info::before malloc the memory.}");
        } else {
            g_user_rx_rate_info = (mac_cfg_rx_user_rate_stru *)oal_mem_alloc(OAL_MEM_POOL_ID_LOCAL,
                WLAN_USER_MAX_USER_LIMIT * sizeof(mac_cfg_rx_user_rate_stru), OAL_TRUE);
            (osal_void)memset_s((osal_u8 *)g_user_rx_rate_info,
                WLAN_USER_MAX_USER_LIMIT * sizeof(mac_cfg_rx_user_rate_stru),
                0, WLAN_USER_MAX_USER_LIMIT * sizeof(mac_cfg_rx_user_rate_stru));
            oam_warning_log0(0, OAM_SF_CFG, "{hmac_ccpriv_enable_user_rate_info::malloc the memory.}");
        }
    }

    frw_msg_init((osal_u8 *)&record_switch, sizeof(record_switch), OSAL_NULL, 0, &msg);
    ret = frw_send_msg_to_device(0, WLAN_MSG_H2D_C_CFG_USER_RATE_ENABLE, &msg, OSAL_TRUE);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_CFG, "{hmac_ccpriv_enable_user_rate_info::send msg fail[%d].}", ret);
    }
    return ret;
}

OAL_STATIC osal_s32 hmac_ccpriv_get_connection_info(hmac_vap_stru *hmac_vap, const osal_s8 *param)
{
    osal_u32 ret;
    osal_u16 user_idx = 0;
    osal_u8 mac_addr[WLAN_MAC_ADDR_LEN];
    osal_s8 cmd[CCPRIV_CMD_NAME_MAX_LEN] = {0};
    hmac_user_stru *hmac_user = OAL_PTR_NULL;

    ret = (osal_u32)hmac_ccpriv_get_one_arg(&param, cmd, OAL_SIZEOF(cmd));
    if (ret != OAL_SUCC) {
        /* 打印所有用户信息 */
        dfx_show_all_user_info(hmac_vap);
        return OAL_SUCC;
    }

    memset_s(mac_addr, WLAN_MAC_ADDR_LEN, 0, WLAN_MAC_ADDR_LEN);
    oal_strtoaddr((const osal_char *)cmd, (osal_u8 *)mac_addr);
    ret = hmac_vap_find_user_by_macaddr_etc(hmac_vap, (osal_u8 *)mac_addr, &user_idx);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_ANY, "{hmac_ccpriv_get_connection_info::no such user!}");
        return OAL_FAIL;
    }
    hmac_user = mac_res_get_hmac_user_etc(user_idx);
    if (hmac_user == OAL_PTR_NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{hmac_ccpriv_get_connection_info::hmac_user null!}");
        return OAL_FAIL;
    }

    ret = dfx_show_user_info(hmac_vap, hmac_user);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_ANY, "{hmac_ccpriv_get_connection_info::no such user!}");
        return OAL_FAIL;
    }
    return OAL_SUCC;
}

OAL_STATIC osal_s32 hmac_ccpriv_get_tid_info(hmac_vap_stru *hmac_vap, const osal_s8 *param)
{
    hmac_tid_stru *tid_queue = OAL_PTR_NULL;
    hmac_user_stru *hmac_user = OAL_PTR_NULL;
    struct osal_list_head *entry = OAL_PTR_NULL;
    struct osal_list_head *dlist_tmp = OAL_PTR_NULL;
    osal_u32 tid;

    unref_param(param);
    /* 判断当前的vap_id下是否有user */
    if (hmac_vap->user_nums == 0) {
        oam_warning_log0(0, OAM_SF_ANY, "{hmac_ccpriv_get_tid_info::no any user!}");
        return OAL_SUCC;
    }

    osal_list_for_each_safe(entry, dlist_tmp, &(hmac_vap->mac_user_list_head)) {
        hmac_user = osal_list_entry(entry, hmac_user_stru, user_dlist);
        if (hmac_user == OAL_PTR_NULL) {
            continue;
        }

        oam_warning_log4(0, OAM_SF_DFT, "[MAC ADDR]:%02X:%02X:%02X:%02X:XX:XX, tid_info",
            hmac_user->user_mac_addr[0], hmac_user->user_mac_addr[1],  // 0, 1 is mac_addr
            hmac_user->user_mac_addr[2], hmac_user->user_mac_addr[3]); // 2, 3 is mac_addr
        for (tid = 0; tid < WLAN_TID_MAX_NUM; tid++) {
            tid_queue = &(hmac_user->tx_tid_queue[tid]);
            oam_warning_log3(0, OAM_SF_DFT, "tid_info tid[%u]::is_paused[%u] mpdu_num[%u].",
                tid, tid_queue->is_paused, tid_queue->mpdu_num);
            oam_warning_log4(0, OAM_SF_DFT, "tid_info tid[%u]::enqueue_num[%u] dequeue_num[%u] dispatch_num[%u].",
                tid, tid_queue->in_num, tid_queue->out_num, tid_queue->dispatch_num);
            tid_queue->in_num = 0;
            tid_queue->out_num = 0;
            tid_queue->dispatch_num = 0;
        }
    }
    return OAL_SUCC;
}

OAL_STATIC osal_s32 hmac_ccpriv_set_dscr_switch(hmac_vap_stru *hmac_vap, const osal_s8 *param)
{
    osal_s32 ret;
    osal_u8 dscr_switch;
    frw_msg msg;

    ret = hmac_ccpriv_get_u8_with_check_max(&param, EXT_SWITCH_ON, &dscr_switch);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_ccpriv_set_dscr_switch::set dscr_switch err[%u]!}", ret);
        return ret;
    }

    frw_msg_init((osal_u8 *)&dscr_switch, sizeof(osal_u8), OSAL_NULL, 0, &msg);
    ret = frw_send_msg_to_device(hmac_vap->vap_id, WLAN_MSG_H2D_C_DEVICE_PERFORMANCE_DSCR_REPORT, &msg, OSAL_TRUE);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_CFG,
            "{hmac_ccpriv_set_dscr_switch: frw_send_msg_to_device failed ret:%d!}", ret);
        return ret;
    }

    wifi_printf("dscr_switch[%u]\r\n", dscr_switch);
    if (dscr_switch == DFT_SWITCH_OFF || dft_get_global_frame_switch() == DFT_SWITCH_ON) {
        return OAL_SUCC;
    }
    /* 描述符开关启用，自动开启帧上报总开关 */
    ret = hmac_ccpriv_set_global_frame_switch(hmac_vap, (const osal_s8 *)"1");
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_ccpriv_set_dscr_switch::set global_frame_switch err[%d]!}", ret);
        return ret;
    }

    return OAL_SUCC;
}

OAL_STATIC osal_u32 dfx_frame_report_hso_rx(oal_netbuf_stru **netbuf, hmac_vap_stru *hmac_vap)
{
    osal_u8 sub_type;
    unref_param(hmac_vap);

    if (netbuf == OSAL_NULL) {
        return OAL_CONTINUE;
    }

    sub_type = mac_get_frame_type_and_subtype((osal_u8 *)oal_netbuf_header(*netbuf));
    if (sub_type == (WLAN_FC0_SUBTYPE_NODATA | WLAN_FC0_TYPE_DATA)) {
        return OAL_CONTINUE;
    }

    dfx_frame_report_hso(*netbuf, OAM_OTA_FRAME_DIRECTION_TYPE_RX);
    return OAL_CONTINUE;
}

/* host侧扫描时帧上报接口 */
OAL_STATIC osal_void dfx_scan_frame_report_hso(mac_bss_dscr_stru *bss_dscr)
{
    mac_ieee80211_frame_stru *frame_hdr = OSAL_NULL;
    osal_u8 sub_type;
    adjust_hso_param_stru adjust_hso_param = {0};
    osal_u16 offset = MAC_TIME_STAMP_LEN + MAC_BEACON_INTERVAL_LEN + MAC_CAP_INFO_LEN;

    if (g_special_frame_switch.scan_frame_switch == DFT_SWITCH_OFF) {
        return;
    }

    /* 上报beacon帧或者probe rsp帧 */
    frame_hdr  = (mac_ieee80211_frame_stru *)bss_dscr->mgmt_buff;
    adjust_hso_param.msg_id = SOC_DIAG_MSG_ID_WIFI_RX_BEACON;
    sub_type = mac_get_frame_type_and_subtype((osal_u8 *)frame_hdr);
    if (sub_type == (WLAN_FC0_SUBTYPE_PROBE_RSP | WLAN_FC0_TYPE_MGT)) {
        adjust_hso_param.msg_id = SOC_DIAG_MSG_ID_WIFI_RX_80211_FRAME;
    }
    adjust_hso_param.is_probe_rsp_or_beacon = OSAL_TRUE;
    adjust_hso_param.rssi = (osal_s8)bss_dscr->c_rssi;
    adjust_hso_param.channel = hmac_dfx_get_chan_num(bss_dscr->mgmt_buff + MAC_80211_FRAME_LEN,
        (osal_u16)(bss_dscr->mgmt_len - MAC_80211_FRAME_LEN), offset);
    dft_report_80211_frame((osal_u8 *)frame_hdr, MAC_80211_FRAME_LEN,
        bss_dscr->mgmt_buff + MAC_80211_FRAME_LEN, (osal_u16) bss_dscr->mgmt_len, &adjust_hso_param);
}

OAL_STATIC osal_s32 hmac_ccpriv_set_scan_frame_switch(hmac_vap_stru *hmac_vap, const osal_s8 *param)
{
    osal_s32 ret;
    osal_u8 scan_frame_switch;

    ret = hmac_ccpriv_get_u8_with_check_max(&param, EXT_SWITCH_ON, &scan_frame_switch);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_ccpriv_set_scan_frame_switch::set scan_frame_switch err[%u]!}", ret);
        return ret;
    }

    g_special_frame_switch.scan_frame_switch = scan_frame_switch;
    oam_warning_log1(0, OAM_SF_DFT, "{hmac_ccpriv_set_scan_frame_switch [%d]}",
        g_special_frame_switch.scan_frame_switch);
    wifi_printf("scan_frame_switch[%d]\r\n", scan_frame_switch);

    if (scan_frame_switch == DFT_SWITCH_OFF || dft_get_global_frame_switch() == DFT_SWITCH_ON) {
        return OAL_SUCC;
    }
    /* scan帧上报开关启用，自动开启帧上报总开关 */
    ret = hmac_ccpriv_set_global_frame_switch(hmac_vap, (const osal_s8 *)"1");
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_ccpriv_set_scan_frame_switch::set global_frame_switch err[%d]!}", ret);
        return ret;
    }

    return OAL_SUCC;
}

hmac_netbuf_hook_stru g_frame_report_rx_hook = {
    .hooknum = HMAC_FRAME_DATA_RX_EVENT_D2H,
    .priority = HMAC_HOOK_PRI_LOW,
    .hook_func = dfx_frame_report_hso_rx,
};

osal_u32 hmac_dfx_init(osal_void)
{
    static const ccpriv_cmd_func_stru cmd_func[] = {
        {(const osal_s8 *)"memoryinfo", hmac_ccpriv_show_device_meminfo},
        {(const osal_s8 *)"get_user_conn_record", hmac_ccpriv_get_user_conn_record},
        {(const osal_s8 *)"global_frame_switch", hmac_ccpriv_set_global_frame_switch},
        {(const osal_s8 *)"beacon_frame_switch", hmac_ccpriv_set_beacon_frame_switch},
        {(const osal_s8 *)"vip_frame_switch", hmac_ccpriv_set_vip_frame_switch},
        {(const osal_s8 *)"80211_frame_switch", hmac_ccpriv_set_80211_frame_switch},
        {(const osal_s8 *)"scan_frame_switch", hmac_ccpriv_set_scan_frame_switch},
        {(const osal_s8 *)"get_scan_result", hmac_ccpriv_get_scan_result},
        {(const osal_s8 *)"enable_user_rate_info", hmac_ccpriv_enable_user_rate_info},
        {(const osal_s8 *)"get_connection_info", hmac_ccpriv_get_connection_info},
        {(const osal_s8 *)"get_tid_info", hmac_ccpriv_get_tid_info},
        {(const osal_s8 *)"dscr_switch", hmac_ccpriv_set_dscr_switch},
    };
 
    /* ccpriv命令注册 */
    hmac_ccpriv_register_array(cmd_func, osal_array_size(cmd_func));
    /* 对外接口注册 */
    hmac_feature_hook_register(HMAC_FHOOK_DFX_USER_CONN_ABNORMAL_RECORD, dfx_user_conn_abnormal_record);
    hmac_feature_hook_register(HMAC_FHOOK_DFX_USER_CONN_STATISTIC_RECORD, dfx_user_conn_statistic_record);
    hmac_feature_hook_register(HMAC_FHOOK_DFX_FRAME_REPORT_HSO, dfx_frame_report_hso);
    hmac_feature_hook_register(HMAC_FHOOK_DFX_SCAN_FRAME_REPORT_HSO, dfx_scan_frame_report_hso);
    (osal_void)hmac_register_netbuf_hook(&g_frame_report_rx_hook);
    hmac_feature_hook_register(HMAC_FHOOK_DFX_USER_RATE_RX_INFO_RECORD, dfx_user_rate_rx_info_record);
    /* DFX:用户连接统计，默认使能 */
    dfx_user_rd_init();
    /* 消息注册 */
#if defined(_PRE_OS_VERSION_LITEOS) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
    frw_msg_hook_register(WLAN_MSG_W2H_CFG_GTE_RX_USER_RATE_INFO, dfx_get_rx_user_rate_info);
    frw_msg_hook_register(WLAN_MSG_W2H_CFG_SHOW_STA_INFO, dfx_show_sta_info);
#endif
    return OAL_SUCC;
}

osal_void hmac_dfx_deinit(osal_void)
{
    /* 对外接口注销 */
    hmac_feature_hook_unregister(HMAC_FHOOK_DFX_USER_CONN_ABNORMAL_RECORD);
    hmac_feature_hook_unregister(HMAC_FHOOK_DFX_USER_CONN_STATISTIC_RECORD);
    hmac_feature_hook_unregister(HMAC_FHOOK_DFX_FRAME_REPORT_HSO);
    (osal_void)hmac_unregister_netbuf_hook(&g_frame_report_rx_hook);
    hmac_feature_hook_unregister(HMAC_FHOOK_DFX_USER_RATE_RX_INFO_RECORD);
    /* DFX:用户连接统计，去使能 */
    dfx_user_rd_exit();

    /* ccpriv命令去注册 */
    hmac_ccpriv_unregister((const osal_s8 *)"memoryinfo");
    hmac_ccpriv_unregister((const osal_s8 *)"get_user_conn_record");
    hmac_ccpriv_unregister((const osal_s8 *)"global_frame_switch");
    hmac_ccpriv_unregister((const osal_s8 *)"beacon_frame_switch");
    hmac_ccpriv_unregister((const osal_s8 *)"vip_frame_switch");
    hmac_ccpriv_unregister((const osal_s8 *)"80211_frame_switch");
    hmac_ccpriv_unregister((const osal_s8 *)"get_scan_result");
    hmac_ccpriv_unregister((const osal_s8 *)"enable_user_rate_info");
    hmac_ccpriv_unregister((const osal_s8 *)"get_connection_info");
    hmac_ccpriv_unregister((const osal_s8 *)"get_tid_info");
    hmac_ccpriv_unregister((const osal_s8 *)"dscr_switch");
    return;
}
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif /* #ifdef _PRE_WLAN_DFT_STAT */
