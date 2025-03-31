/*
 * Copyright (c) CompanyNameMagicTag 2012-2023. All rights reserved.
 * 文 件 名   : hmac_user.c
 * 生成日期   : 2012年10月19日
 * 功能描述   : Hmac User 主文件
 */

#include "hmac_user.h"
#include "mac_ie.h"
#include "oam_ext_if.h"
#include "oal_ext_if.h"
#include "hal_ext_if.h"
#include "dmac_ext_if_hcm.h"
#include "hmac_main.h"
#include "hmac_tx_amsdu.h"
#include "hmac_protection.h"
#include "hmac_smps.h"
#include "hmac_ext_if.h"
#include "hmac_config.h"
#include "hmac_mgmt_ap.h"
#include "hmac_chan_mgmt.h"
#include "hmac_fsm.h"
#include "hmac_sme_sta.h"
#include "hmac_alg_notify.h"

#include "wlan_msg.h"

#ifdef _PRE_WLAN_FEATURE_PROXY_ARP
#include "hmac_proxy_arp.h"
#endif

#include "hmac_wapi.h"
#include "hmac_feature_interface.h"
#include "hmac_m2u.h"
#include "hmac_roam_if.h"
#include "hmac_blockack.h"
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
#include "soc_customize_wifi.h"
#endif /* #ifdef _PRE_PLAT_FEATURE_CUSTOMIZE */

#ifdef _PRE_WLAN_FEATURE_VIRTUAL_MULTI_STA
#include "hmac_wds.h"
#endif
#include "hmac_scan.h"
#include "hmac_feature_main.h"
#include "hmac_sta_pm.h"
#include "hmac_tid.h"
#include "hmac_psm_ap.h"
#include "hmac_11i.h"
#include "hmac_rx_data_filter.h"
#include "hmac_beacon.h"
#include "hmac_uapsd.h"
#include "hmac_psm_sta.h"
#include "hmac_chan_mgmt.h"
#ifdef _PRE_WLAN_FEATURE_TXOPPS
#include "hmac_txopps.h"
#endif
#include "hmac_feature_interface.h"
#include "hmac_btcoex.h"
#include "hmac_btcoex_notify.h"
#ifdef _PRE_WLAN_FEATURE_PMF
#include "hmac_11w.h"
#endif
#ifdef _PRE_WLAN_FEATURE_M2S
#include "hmac_m2s.h"
#endif
#include "hmac_feature_dft.h"
#include "hmac_tx_mpdu_adapt.h"
#ifdef _PRE_WLAN_FEATURE_WUR_TX
#include "hmac_wur_ap.h"
#endif
#ifdef _PRE_WLAN_FEATURE_ANT_SWITCH
#include "hmac_ant_switch.h"
#endif
#include "frw_util_notifier.h"
#include "msg_psm_rom.h"
#include "hmac_keep_alive.h"
#include "common_log_dbg_rom.h"
#include "hmac_achba.h"
#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST
#include "hmac_obss_sta.h"
#include "hmac_obss_ap.h"
#endif
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_HMAC_USER_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/

/*****************************************************************************
  3 函数实现
*****************************************************************************/
OSAL_STATIC osal_u32 hmac_user_add_prep(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user, osal_s8 rssi);
OSAL_STATIC osal_u32 hmac_user_add_handler(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    mac_ap_type_enum_uint8 ap_type);
OSAL_STATIC osal_void hmac_user_add_post(hmac_vap_stru *hmac_vap);
#if defined _PRE_WLAN_CFGID_DEBUG && defined _PRE_WIFI_PRINTK
OSAL_STATIC osal_s8 *hmac_config_keytype2string_etc(osal_u32 keytype)
{
    osal_char        *pac_keytype2string[] = {"GTK", "PTK", "RX_GTK", "ERR"};
    return hmac_config_index2string_etc(keytype, (osal_s8 **)pac_keytype2string,
                                        OAL_SIZEOF(pac_keytype2string) / OAL_SIZEOF(osal_s8 *));
}

OSAL_STATIC osal_s8 *hmac_config_cipher2string_etc(osal_u32 cipher)
{
    osal_char *pac_cipher2string[] = {"GROUP", "WEP40", "TKIP", "NO_ENCRYP", "CCMP", "WEP104", "BIP", "GROUP_DENYD"};
    return hmac_config_index2string_etc(cipher, (osal_s8 **)pac_cipher2string,
                                        OAL_SIZEOF(pac_cipher2string) / OAL_SIZEOF(osal_s8 *));
}
#endif

/*****************************************************************************
 函 数 名  : mac_res_get_hmac_user_alloc_etc
 功能描述  : 获取对应HMAC USER索引的内存,由HMAC层强转为自己的内存解析
 输入参数  : 对应HMAC USER内存索引
 返 回 值  : 对应内存地址
*****************************************************************************/
static hmac_user_stru* mac_res_get_hmac_user_alloc_etc(osal_u16 idx)
{
    hmac_user_stru *hmac_user;

    hmac_user = (hmac_user_stru *)_mac_res_alloc_hmac_user(idx);
    if (hmac_user == OAL_PTR_NULL) {
        oam_error_log1(0, OAM_SF_UM, "{mac_res_get_hmac_user_init::hmac_user null,user_idx=%d.}", idx);
        return OAL_PTR_NULL;
    }

    return hmac_user;
}

/*****************************************************************************
 函 数 名  : mac_res_get_hmac_user_etc
 功能描述  : 获取对应HMAC USER索引的内存,由HMAC层强转为自己的内存解析
 输入参数  : 对应HMAC USER内存索引
 返 回 值  : 对应内存地址
*****************************************************************************/
WIFI_HMAC_TCM_TEXT WIFI_TCM_TEXT hmac_user_stru* mac_res_get_hmac_user_etc(osal_u16 idx)
{
    hmac_user_stru *hmac_user;

    hmac_user = (hmac_user_stru *)_mac_res_get_hmac_user(idx);
    if (hmac_user == OAL_PTR_NULL) {
        return OAL_PTR_NULL;
    }

    /* 异常: 用户资源已被释放 */
    if (hmac_user->is_user_alloced != OSAL_TRUE) {
        oam_warning_log1(0, OAM_SF_UM, "user_id[%d] {hmac_user is_user_alloced is false!", idx);
        /*
         * host侧获取用户时用户已经释放属于正常，返回空指针，
         * 后续调用者查找用户失败，请打印WARNING并直接释放buf，走其他分支等等
         */
        return OAL_PTR_NULL;
    }

    return hmac_user;
}

/*****************************************************************************
 函 数 名  : hmac_user_del_feature
 功能描述  : user删除时，特性数据结构内存释放
*****************************************************************************/
OAL_STATIC osal_void hmac_user_del_feature(hmac_user_stru *hmac_user)
{
    frw_util_notifier_notify(WLAN_UTIL_NOTIFIER_EVENT_DEL_USER_FEATURE, (osal_void *)hmac_user);

    hmac_user_feature_deinit(hmac_user);
}

/*****************************************************************************
 函 数 名  : hmac_user_add_feature
 功能描述  : user添加时，特性数据结构内存分配和初始化
*****************************************************************************/
OAL_STATIC osal_s32 hmac_user_add_feature(hmac_user_stru *hmac_user)
{
    osal_s32 ret;
    osal_void *fhook = NULL;

    ret = (osal_s32)hmac_user_feature_init(hmac_user);
    if (ret != OAL_SUCC) {
        return ret;
    }
    fhook = hmac_get_feature_fhook(HMAC_FHOOK_BTCOEX_PROC_USER_ADD);
    if (fhook != OSAL_NULL && ((hmac_btcoex_proc_user_add_cb)fhook)(hmac_user) != OAL_SUCC) {
        hmac_user_del_feature(hmac_user);
        return OAL_FAIL;
    }
    return OAL_SUCC;
}


/*****************************************************************************
 函 数 名  : hmac_user_alloc_etc
 功能描述  : 创建HMAC用户实体
 输入参数  : oal_mem_stru **ppst_mem_stru
 输出参数  : oal_mem_stru **ppst_mem_stru
 返 回 值  : 成功或失败原因
*****************************************************************************/
OSAL_STATIC osal_u32  hmac_user_alloc_etc(osal_u16 *pus_user_idx)
{
    hmac_user_stru *hmac_user;
    osal_u32      rslt;
    osal_u16      user_idx_temp = 0;

    if (osal_unlikely(pus_user_idx == OAL_PTR_NULL)) {
        oam_error_log0(0, OAM_SF_UM, "{hmac_user_alloc_etc::pus_user_idx null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 申请hmac user内存 */
    rslt = mac_res_alloc_hmac_user(&user_idx_temp);
    if (rslt != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_UM, "{hmac_user_alloc_etc::mac_res_alloc_hmac_user failed[%d].}", rslt);
        return rslt;
    }

    hmac_user = mac_res_get_hmac_user_alloc_etc(user_idx_temp);
    if (hmac_user == OAL_PTR_NULL) {
        hmac_res_free_mac_user_etc(user_idx_temp);
        _mac_res_free_hmac_user(user_idx_temp);
        oam_error_log1(0, OAM_SF_UM, "{hmac_user_alloc_etc::hmac_user null,user_idx=%d.}", user_idx_temp);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 初始清0 */
    memset_s(hmac_user, OAL_SIZEOF(hmac_user_stru), 0, OAL_SIZEOF(hmac_user_stru));
    hmac_user->is_user_alloced = OSAL_TRUE;

    /* hmac_user特性指针数组内存申请 */
    rslt = (osal_u32)hmac_user_add_feature(hmac_user);
    if (rslt != OAL_SUCC) {
        hmac_res_free_mac_user_etc(user_idx_temp);
        _mac_res_free_hmac_user(user_idx_temp);
        oam_warning_log1(0, OAM_SF_UM, "{hmac_user_alloc_etc::hmac_user_feature_arr_alloc failed[%d].}", rslt);
        return rslt;
    }

    *pus_user_idx = user_idx_temp;

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_user_free_etc
 功能描述  : 释放对应HMAC USER的内存
 输入参数  : 对应HMAC USER内存索引
 返 回 值  : OAL_SUCC/OAL_FAIL
*****************************************************************************/
OSAL_STATIC osal_u32 hmac_user_free_etc(hmac_user_stru *hmac_user)
{
    osal_u32      ul_ret;

    if (hmac_user == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_UM, "{hmac_user_free_etc::hmac_user null}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* hmac_user特性指针数组内存释放 */
    hmac_user_del_feature(hmac_user);

    ul_ret = hmac_res_free_mac_user_etc(hmac_user->assoc_id);
    _mac_res_free_hmac_user(hmac_user->assoc_id);

    return ul_ret;
}

/*****************************************************************************
 函 数 名  : hmac_user_free_asoc_req_ie
*****************************************************************************/
osal_u32 hmac_user_free_asoc_req_ie(hmac_user_stru    *hmac_user)
{
    if (hmac_user == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_UM, "{hmac_user_free_asoc_req_ie::hmac_user null}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    if (hmac_user->assoc_req_ie_buff != OAL_PTR_NULL) {
        oal_mem_free(hmac_user->assoc_req_ie_buff, OAL_TRUE);
        hmac_user->assoc_req_ie_buff = OAL_PTR_NULL;
        hmac_user->assoc_req_ie_len   = 0;
    } else {
        hmac_user->assoc_req_ie_len   = 0;
    }
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_user_set_asoc_req_ie
*****************************************************************************/
osal_u32 hmac_user_set_asoc_req_ie(hmac_user_stru *hmac_user, osal_u8 *payload, osal_u32 len,
    osal_u8 reass_flag)
{
    osal_u32 payload_len = len;

    /* 重关联比关联请求帧头多了AP的MAC地址  */
    payload_len -= ((reass_flag == OAL_TRUE) ? WLAN_MAC_ADDR_LEN : 0);

    hmac_user->assoc_req_ie_buff = oal_mem_alloc(OAL_MEM_POOL_ID_LOCAL, (osal_u16)payload_len, OAL_TRUE);
    if (hmac_user->assoc_req_ie_buff == OAL_PTR_NULL) {
        oam_error_log2(0, OAM_SF_ASSOC,
            "vap_id[%d] {hmac_user_set_asoc_req_ie :: Alloc %u bytes failed for assoc_req_ie_buff failed .}",
            hmac_user->vap_id,
            (osal_u16)payload_len);
        hmac_user->assoc_req_ie_len = 0;
        return OAL_FAIL;
    }
    (osal_void)memcpy_s(hmac_user->assoc_req_ie_buff, payload_len,
        payload + ((reass_flag == OAL_TRUE) ? WLAN_MAC_ADDR_LEN : 0), payload_len);
    hmac_user->assoc_req_ie_len = payload_len;

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_user_init_etc
 功能描述  : 初始化HMAC 用户
 输入参数  : hmac_user_stru *hmac_user
 返 回 值  : 成功或失败原因
*****************************************************************************/
static osal_u32  hmac_user_init_etc(hmac_user_stru *hmac_user)
{
    osal_u8        tid_loop;
    hmac_ba_tx_stru *tx_ba;

    /* 初始化tid信息 */
    for (tid_loop = 0; tid_loop < WLAN_TID_MAX_NUM; tid_loop++) {
        hmac_user->tx_tid_queue[tid_loop].tid      = (osal_u8)tid_loop;

        /* 初始化ba rx操作句柄 */
        hmac_user->tx_tid_queue[tid_loop].ba_rx_info = OAL_PTR_NULL;
        osal_spin_lock_init(&(hmac_user->tx_tid_queue[tid_loop].ba_tx_info.ba_status_lock));
        hmac_user->tx_tid_queue[tid_loop].ba_tx_info.ba_status     = HMAC_BA_INIT;
        hmac_user->tx_tid_queue[tid_loop].ba_tx_info.addba_attemps = 0;
        hmac_user->tx_tid_queue[tid_loop].ba_tx_info.dialog_token  = 0;
        hmac_user->tx_tid_queue[tid_loop].ba_tx_info.ba_policy     = 0;
        hmac_user->tx_tid_queue[tid_loop].ba_handle_tx_enable         = OAL_TRUE;
        hmac_user->tx_tid_queue[tid_loop].ba_handle_rx_enable         = OAL_TRUE;

        hmac_user->ba_flag[tid_loop] = 0;

        /* addba req超时处理函数入参填写 */
        tx_ba = &hmac_user->tx_tid_queue[tid_loop].ba_tx_info;
        tx_ba->alarm_data.ba = (osal_void *)tx_ba;
        tx_ba->alarm_data.tid = tid_loop;
        tx_ba->alarm_data.mac_user_idx = hmac_user->assoc_id;
        tx_ba->alarm_data.vap_id = hmac_user->vap_id;

        /* 初始化用户关联请求帧参数 */
        hmac_user->assoc_req_ie_buff = OAL_PTR_NULL;
        hmac_user->assoc_req_ie_len   = 0;
    }
    /* owe info init */
    hmac_user->hmac_cap_info.owe = OAL_FALSE;
#ifdef _PRE_WLAN_FEATURE_WPA3
    hmac_user->owe_ie = OSAL_NULL;
    hmac_user->owe_ie_len = 0;
    hmac_user->auth_alg = WLAN_WITP_AUTH_OPEN_SYSTEM;
#endif
#ifdef _PRE_WLAN_FEATURE_PMF
    hmac_user->sa_query_info.sa_query_count      = 0;
    hmac_user->sa_query_info.sa_query_start_time = 0;
#endif
    hmac_user->rx_pkt_drop = 0;

    hmac_user->first_add_time = (osal_u32)oal_time_get_stamp_ms();

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_11AX
static osal_u32 hmac_user_set_he_cap(hmac_user_stru *hmac_user,
    mac_he_hdl_stru *he_hdl,
    wlan_nss_enum_uint8 *users_spatial_strm)
{
    osal_u32 ul_ret = OAL_SUCC;
    mac_frame_he_mcs_nss_bit_map_stru *mcs_below_80mhz = &he_hdl->he_cap_ie.he_mcs_nss.rx_he_mcs_below_80mhz;

    if (mcs_below_80mhz->max_he_mcs_for_4ss != 3) { /* 3:参数判断 */
        *users_spatial_strm = WLAN_FOUR_NSS;
    } else if (mcs_below_80mhz->max_he_mcs_for_3ss != 3) { /* 3:参数判断 */
        *users_spatial_strm = WLAN_TRIPLE_NSS;
    } else if (mcs_below_80mhz->max_he_mcs_for_2ss != 3) { /* 3:参数判断 */
        *users_spatial_strm = WLAN_DOUBLE_NSS;
    } else if (mcs_below_80mhz->max_he_mcs_for_1ss != 3) { /* 3:参数判断 */
        *users_spatial_strm = WLAN_SINGLE_NSS;
    } else {
        oam_warning_log1(0, OAM_SF_ANY, "vap_id[%d] {hmac_user_set_avail_num_space_stream_etc::invalid he nss.}",
                         hmac_user->vap_id);
        ul_ret =  OAL_FAIL;
    }

    return ul_ret;
}
#endif

static osal_u32 hmac_user_set_vht_cap(hmac_user_stru *hmac_user,
    const mac_vht_hdl_stru *pst_mac_vht_hdl, wlan_nss_enum_uint8 *users_spatial_strm)
{
    osal_u32 ul_ret = OAL_SUCC;

    if (pst_mac_vht_hdl->rx_max_mcs_map.max_mcs_4ss != 3) { /* 3:参数判断 */
        *users_spatial_strm = WLAN_FOUR_NSS;
    } else if (pst_mac_vht_hdl->rx_max_mcs_map.max_mcs_3ss != 3) { /* 3:参数判断 */
        *users_spatial_strm = WLAN_TRIPLE_NSS;
    } else if (pst_mac_vht_hdl->rx_max_mcs_map.max_mcs_2ss != 3) { /* 3:参数判断 */
        *users_spatial_strm = WLAN_DOUBLE_NSS;
    } else if (pst_mac_vht_hdl->rx_max_mcs_map.max_mcs_1ss != 3) { /* 3:参数判断 */
        *users_spatial_strm = WLAN_SINGLE_NSS;
    } else {
        oam_warning_log1(0, OAM_SF_ANY, "vap_id[%d] {hmac_user_set_avail_num_space_stream_etc::invalid vht nss.}",
                         hmac_user->vap_id);

        ul_ret =  OAL_FAIL;
    }
    return ul_ret;
}

static osal_u32 hmac_user_set_ht_cap(hmac_user_stru *hmac_user,
    const mac_user_ht_hdl_stru *mac_ht_hdl, wlan_nss_enum_uint8 *users_spatial_strm)
{
    osal_u32 ul_ret = OAL_SUCC;

    if (mac_ht_hdl->rx_mcs_bitmask[3] > 0) { /* 3:索引值 */
        *users_spatial_strm = WLAN_FOUR_NSS;
    } else if (mac_ht_hdl->rx_mcs_bitmask[2] > 0) { /* 2:索引值 */
        *users_spatial_strm = WLAN_TRIPLE_NSS;
    } else if (mac_ht_hdl->rx_mcs_bitmask[1] > 0) { /* 1:索引值 */
        *users_spatial_strm = WLAN_DOUBLE_NSS;
    } else if (mac_ht_hdl->rx_mcs_bitmask[0] > 0) { /* 0:索引值 */
        *users_spatial_strm = WLAN_SINGLE_NSS;
    } else {
        oam_warning_log1(0, OAM_SF_ANY, "vap_id[%d] {hmac_user_set_avail_num_space_stream_etc::invalid ht nss.}",
                         hmac_user->vap_id);
        ul_ret =  OAL_FAIL;
    }
    return ul_ret;
}

/*****************************************************************************
 函 数 名  : hmac_user_set_avail_num_space_stream_etc
 功能描述  : 获取用户和VAP公共可用的空间流数
*****************************************************************************/
osal_u32  hmac_user_set_avail_num_space_stream_etc(hmac_user_stru *hmac_user, wlan_nss_enum_uint8 vap_nss)
{
    mac_user_ht_hdl_stru         *mac_ht_hdl;
    mac_vht_hdl_stru             *pst_mac_vht_hdl;
#ifdef _PRE_WLAN_FEATURE_11AX
    mac_he_hdl_stru              *pst_mac_he_hdl;
#endif
    wlan_nss_enum_uint8           users_spatial_strm = WLAN_SINGLE_NSS;
    osal_u32                    ul_ret = OAL_SUCC;

    /* AP(STA)为legacy设备，只支持1根天线，不需要再判断天线个数 */

    /* 获取HT和VHT结构体指针 */
    mac_ht_hdl  = &(hmac_user->ht_hdl);
    pst_mac_vht_hdl = &(hmac_user->vht_hdl);
#ifdef _PRE_WLAN_FEATURE_11AX
    pst_mac_he_hdl = &(hmac_user->he_hdl);
    if (pst_mac_he_hdl->he_capable == OAL_TRUE) {
        ul_ret = hmac_user_set_he_cap(hmac_user, pst_mac_he_hdl, &users_spatial_strm);
    } else if (pst_mac_vht_hdl->vht_capable == OAL_TRUE)
#else
    if (pst_mac_vht_hdl->vht_capable == OAL_TRUE)
#endif
    {
        ul_ret = hmac_user_set_vht_cap(hmac_user, pst_mac_vht_hdl, &users_spatial_strm);
    } else if (mac_ht_hdl->ht_capable == OAL_TRUE) {
        ul_ret = hmac_user_set_ht_cap(hmac_user, mac_ht_hdl, &users_spatial_strm);
    } else {
        users_spatial_strm = WLAN_SINGLE_NSS;
    }

    /* 赋值给用户结构体变量 */
    hmac_user_set_num_spatial_stream_etc(hmac_user, users_spatial_strm);
    hmac_user_set_avail_num_spatial_stream_etc(hmac_user, OAL_MIN(hmac_user->user_num_spatial_stream,
        vap_nss));
    return ul_ret;
}

#ifdef _PRE_WLAN_FEATURE_PMF
/*****************************************************************************
 函 数 名  : hmac_stop_sa_query_timer
 功能描述  : 删除sa query timer
*****************************************************************************/
OAL_STATIC osal_void hmac_stop_sa_query_timer(hmac_user_stru *hmac_user)
{
    frw_timeout_stru    *timer;

    timer = &(hmac_user->sa_query_info.sa_query_interval_timer);
    if (timer->is_registerd != OAL_FALSE) {
        frw_destroy_timer_entry(timer);
    }

    /* 删除timers的入参存储空间 */
    if (timer->timeout_arg != OAL_PTR_NULL) {
        oal_mem_free((osal_void *)timer->timeout_arg, OAL_TRUE);
        timer->timeout_arg = OAL_PTR_NULL;
    }
}
#endif

/*****************************************************************************
 函 数 名  : hmac_compability_ap_tpye_identify_etc
 功能描述  : 兼容性问题AP识别
*****************************************************************************/
static mac_ap_type_enum_uint8 hmac_compability_ap_tpye_identify_etc(hmac_vap_stru *hmac_vap, osal_u8 *mac_addr)
{
#if defined(_PRE_WLAN_MP13_DDC_BUGFIX)
    mac_bss_dscr_stru              *bss_dscr = OAL_PTR_NULL;
    wlan_nss_enum_uint8             support_max_nss = WLAN_SINGLE_NSS;
#endif
    osal_void *fhook = NULL;

    if (mac_is_golden_ap(mac_addr)) {
        return MAC_AP_TYPE_GOLDENAP;
    }

#ifdef _PRE_WLAN_MP13_DDC_BUGFIX
    /* DDC白名单: AP OUI + chip OUI + 三流 + 2G */
    if (mac_is_belkin_ap(mac_addr) || mac_is_trendnet_ap(mac_addr) || mac_is_netgear_wndr_ap(mac_addr)) {
        bss_dscr = (mac_bss_dscr_stru *)hmac_scan_get_scanned_bss_by_bssid(hmac_vap, mac_addr);
        if (bss_dscr != OAL_PTR_NULL) {
#ifdef _PRE_WLAN_FEATURE_M2S
            support_max_nss = bss_dscr->support_max_nss;
#endif
            if ((bss_dscr->ddc_whitelist_chip_oui == OAL_TRUE) &&
                ((mac_is_netgear_wndr_ap(mac_addr) && (support_max_nss == WLAN_DOUBLE_NSS)) ||
                ((mac_is_belkin_ap(mac_addr) || mac_is_trendnet_ap(mac_addr)) &&
                (support_max_nss == WLAN_TRIPLE_NSS) && (hmac_vap->channel.band == WLAN_BAND_2G)))) {
                oam_warning_log1(0, OAM_SF_ANY,
                    "vap_id[%d] hmac_compability_ap_tpye_identify_etc is in ddc whitelist!",
                    hmac_vap->vap_id);
                return MAC_AP_TYPE_DDC_WHITELIST;
            }
        }
    }
#endif

    fhook = hmac_get_feature_fhook(HMAC_FHOOK_BTCOEX_CHECK_AP_TYPE_BLACKLIST);
    if (fhook != OSAL_NULL &&
        ((hmac_btcoex_check_ap_type_blacklist_cb)fhook)(hmac_vap, bss_dscr, mac_addr, &support_max_nss) != OSAL_TRUE) {
            return MAC_AP_TYPE_BTCOEX_PS_BLACKLIST;
    }

    return MAC_AP_TYPE_NORMAL;
}

static osal_void hmac_user_del_amsdu_etc(hmac_user_stru *user)
{
    osal_u32 amsdu_idx;
    hmac_amsdu_stru *hmac_amsdu = OSAL_NULL;

    for (amsdu_idx = 0; amsdu_idx < sizeof(user->hmac_amsdu) / sizeof(user->hmac_amsdu[0]); amsdu_idx++) {
        hmac_amsdu = &(user->hmac_amsdu[amsdu_idx]);
        osal_spin_lock_destroy(&hmac_amsdu->amsdu_lock);
    }
    oam_warning_log1(0, OAM_SF_UM, "{hmac_user_del_amsdu_etc::destory [%u] lock}", amsdu_idx);
}

/*****************************************************************************
 函 数 名  : hmac_del_user
 功能描述  : 删除user
*****************************************************************************/
osal_u32  hmac_user_del_etc(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user)
{
    osal_u32                        ret;
    mac_ap_type_enum_uint8          ap_type = MAC_AP_TYPE_BUTT;
    hmac_ctx_del_user_stru payload = {0};
    frw_msg msg_info = {0};
    osal_void                       *fhook = OAL_PTR_NULL;

    if (osal_unlikely((hmac_vap == OAL_PTR_NULL) || (hmac_user == OAL_PTR_NULL))) {
        oam_error_log0(0, OAM_SF_UM, "{hmac_user_del_etc::param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (hmac_user->is_user_alloced == OSAL_FALSE) {
        oam_error_log1(0, OAM_SF_UM, "{hmac_user_del_etc::user has del %d.}", hmac_user->assoc_id);
        return OAL_FAIL;
    }

    payload.user_idx = hmac_user->assoc_id;
    payload.ap_type  = ap_type; /* device不需要该数据 */
    payload.user_type = MAC_USER_TYPE_DEFAULT;
#if defined(_PRE_OS_VERSION_WIN32) && defined(_PRE_OS_VERSION) \
    &&(_PRE_OS_VERSION_WIN32 != _PRE_OS_VERSION)
    /* 用户 mac地址和idx 需至少一份有效，供dmac侧查找待删除的用户 */
    (osal_void)memcpy_s(payload.user_mac_addr, WLAN_MAC_ADDR_LEN, hmac_user->user_mac_addr, WLAN_MAC_ADDR_LEN);
#endif
    cfg_msg_init((osal_u8 *)&payload, OAL_SIZEOF(payload), OAL_PTR_NULL, 0, &msg_info);
    /* 抛事件至Device侧DMAC，同步删除user */
    ret = (osal_u32)frw_send_msg_to_device(hmac_vap->vap_id, WLAN_MSG_H2D_C_CFG_DEVICE_DEL_USER, &msg_info, OSAL_TRUE);
    if (ret != OAL_SUCC) {
        oam_error_log2(0, OAM_SF_ANY, "hmac_user_del:usr[%d]frw_send_msg_to_device fail[%d]", hmac_user->assoc_id, ret);
        wifi_printf("{hmac_user_del::user[%d] frw_send_msg_to_device failed[%d].}\r\n", hmac_user->assoc_id, ret);
        return ret;
    }
    mac_vap_intrrupt_disable();
    hmac_user->is_user_alloced = OSAL_FALSE;
    osal_adapt_atomic_dec(&(hmac_user->use_cnt));
    mac_vap_intrrupt_enable();
    while (osal_adapt_atomic_read(&(hmac_user->use_cnt)) != 0) {
        osal_msleep(1); /* 休眠1ms让出CPU */
    }

    frw_util_notifier_notify(WLAN_UTIL_NOTIFIER_EVENT_DEL_USER, hmac_user);

    oam_warning_log2(0, OAM_SF_UM, "{hmac_user_del_etc:del user[%d],is multi user[%d]}",
        hmac_user->assoc_id, hmac_user->is_multi_user);
    oam_warning_log4(0, OAM_SF_UM, "{user mac:%02X:%02X:%02X:%02X:XX:XX}",
        /* 0 1 2 3:MAC索引值 */
        hmac_user->user_mac_addr[0], hmac_user->user_mac_addr[1], /* 打印第 0, 1 位mac */
        hmac_user->user_mac_addr[2], hmac_user->user_mac_addr[3]); /* 打印第 2, 3 位mac */
    fhook = hmac_get_feature_fhook(HMAC_FHOOK_BTCOEX_CLEAR_ARP_TIMER);
    if (fhook != OSAL_NULL) {
        ((hmac_btcoex_clear_arp_timer_cb)fhook)(hmac_user);
    }

    /* 删除user时候，需要更新保护机制 */
    ret = hmac_protection_del_user_etc(hmac_vap, hmac_user);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_UM, "{hmac_user_del_etc::hmac_protection_del_user_etc[%d]}", ret);
    }

    /* 删除hmac user 的关联请求帧空间 */
    hmac_user_free_asoc_req_ie(hmac_user);

#ifdef _PRE_WLAN_FEATURE_PMF
    hmac_stop_sa_query_timer(hmac_user);
#endif

    /* 用户去关联时清空snoop链表中的该成员 */
    fhook = hmac_get_feature_fhook(HMAC_FHOOK_M2U_CLEANUP_SNOOPWDS_NODE);
    if (hmac_vap->m2u != OAL_PTR_NULL) {
        if (fhook != OSAL_NULL) {
            ((hmac_m2u_cleanup_snoopwds_node_cb)fhook)(hmac_user);
        }
    }

    if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_STA) {
        /* host删用户时mac vap下的关联id清零 */
#ifdef _PRE_WLAN_FEATURE_STA_PM
        hmac_vap_set_aid_etc(hmac_vap, 0);
#endif

        ap_type = hmac_compability_ap_tpye_identify_etc(hmac_vap, hmac_user->user_mac_addr);
    }

    /* 刪除SR的颜色冲突定时器 */
    if (hmac_vap->collision_handler.collision_timer.is_registerd == OSAL_TRUE) {
        frw_destroy_timer_entry(&(hmac_vap->collision_handler.collision_timer));
    }

    /* 删除接入CHBA的WIFI链路信息 */
    fhook = hmac_get_feature_fhook(HMAC_FHOOK_ACHBA_DEL_LINK_ID);
    if (fhook != OSAL_NULL) {
        ((hmac_user_del_achba_cb)fhook)(hmac_vap);
    }
    /* 原dmac删除逻辑 */
    hmac_user_del(hmac_vap, hmac_user, ap_type);
#ifdef _PRE_WLAN_FEATURE_SMPS
    hmac_user_set_sm_power_save(hmac_user, 0);
#endif

    hmac_tid_clear_etc(hmac_vap, hmac_user, OSAL_TRUE);
    hmac_user_del_amsdu_etc(hmac_user);

    if (hmac_user->mgmt_timer.is_registerd == OAL_TRUE) {
        frw_destroy_timer_entry(&hmac_user->mgmt_timer);
    }

    /* 从vap中删除用户 */
    hmac_vap_del_user_etc(hmac_vap, hmac_user);

#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST_AP
    hmac_chan_update_40m_intol_user_etc(hmac_vap);
#endif

    if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_STA) {
        hmac_fsm_change_state_etc(hmac_vap, MAC_VAP_STATE_STA_FAKE_UP);
    }

    /* 释放用户内存 */
    ret = hmac_user_free_etc(hmac_user);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_UM, "{hmac_user_del_etc::hmac_res_free_mac_user_etc fail[%d].}", ret);
    }
    return OAL_SUCC;
}

OAL_STATIC osal_u32 hmac_user_add_check(hmac_vap_stru *hmac_vap, osal_u8 *mac_addr, const osal_u16 *pus_user_index)
{
    osal_u32 ul_ret;
    osal_u16 usr_idx = 0;
    osal_void *fhook = hmac_get_feature_fhook(HMAC_FHOOK_WAPI_USER_ADD_CHECK);

    if (osal_unlikely((hmac_vap == OAL_PTR_NULL) || (mac_addr == OAL_PTR_NULL) ||
        (pus_user_index == OAL_PTR_NULL))) {
        oam_error_log0(0, OAM_SF_UM, "{hmac_user_add_check::param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (fhook != OSAL_NULL) {
        ul_ret = ((hmac_wapi_user_add_check_cb)fhook)(hmac_vap->device_id);
        if (ul_ret != OAL_CONTINUE) {
            return ul_ret;
        }
    }

    /* vap现有下挂设备数量不能达到vap接入数量限制 */
    if (hmac_vap->user_nums >= mac_mib_get_MaxAssocUserNums(hmac_vap)) {
        oam_warning_log3(0, OAM_SF_UM,
            "vap_id[%d] {hmac_user_add_check::invalid vap_user_nums[%d], user_nums_max[%d].}",
            hmac_vap->vap_id, hmac_vap->user_nums, mac_mib_get_MaxAssocUserNums(hmac_vap));
        return OAL_ERR_CODE_CONFIG_EXCEED_SPEC;
    }
    /* 整个芯片现有下挂设备数量不能达到芯片接入数量限制 */
    if (hmac_vap->hal_device->assoc_user_nums >= mac_chip_get_max_asoc_user(hmac_vap->chip_id)) {
        oam_warning_log3(0, OAM_SF_UM,
            "vap_id[%d] {hmac_user_add_check::invalid chip_user_nums[%d], user_nums_max[%d].}",
            hmac_vap->vap_id, hmac_vap->hal_device->assoc_user_nums, mac_chip_get_max_asoc_user(hmac_vap->chip_id));
        return OAL_ERR_CODE_CONFIG_EXCEED_SPEC;
    }
    /* 如果此用户已经创建，则返回失败 */
    ul_ret = hmac_vap_find_user_by_macaddr_etc(hmac_vap, mac_addr, &usr_idx);
    if (ul_ret == OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_UM,
            "vap_id[%d] {hmac_user_add_check::hmac_vap_find_user_by_macaddr_etc success[%d].}",
            hmac_vap->vap_id, ul_ret);
        return OAL_FAIL;
    }
    return OAL_SUCC;
}

OAL_STATIC osal_u32 hmac_user_add_prepare(hmac_vap_stru *hmac_vap, osal_u8 *mac_addr, mac_ap_type_enum_uint8 *ap_type)
{
    if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_STA) {
        /* P2P作为CL时可以添加2个用户，一个用于发送管理帧，一个用户发送数据帧 */
#ifdef _PRE_WLAN_FEATURE_P2P
        if (is_p2p_cl(hmac_vap)) {
            if (hmac_vap->user_nums >= 2) { /* 2个用户 */
                oam_warning_log1(0, OAM_SF_UM, "vap_id[%d] {hmac_user_add_etc::a STA can only associated with 2 ap.}",
                                 hmac_vap->vap_id);
                return OAL_FAIL;
            }
        } else {
#endif
            if (hmac_vap->user_nums >= 1) {
                oam_warning_log1(0, OAM_SF_ANY,
                    "vap_id[%d] {hmac_user_add_etc::a STA can only associated with one ap.}", hmac_vap->vap_id);
                return OAL_FAIL;
            }
            *ap_type = hmac_compability_ap_tpye_identify_etc(hmac_vap, mac_addr);
#ifdef _PRE_WLAN_FEATURE_P2P
        }
#endif
    }

    return OAL_SUCC;
}

OAL_STATIC osal_u32 hmac_add_user_event_proc(hmac_vap_stru *hmac_vap, osal_u8 *mac_addr, hmac_user_stru *hmac_user,
    mac_ap_type_enum_uint8 ap_type)
{
    mac_bss_dscr_stru *bss_dscr = OAL_PTR_NULL;

    osal_u32 ret;
    hmac_ctx_add_user_stru payload;
    frw_msg msg_info;

    (osal_void)memset_s(&msg_info, OAL_SIZEOF(msg_info), 0, OAL_SIZEOF(msg_info));
    (osal_void)memset_s(&payload, OAL_SIZEOF(payload), 0, OAL_SIZEOF(payload));

    payload.user_idx = hmac_user->assoc_id;
    payload.ap_type  = ap_type;
    payload.user_type = MAC_USER_TYPE_DEFAULT;
    oal_set_mac_addr(payload.user_mac_addr, mac_addr);

    /* 获取扫描的bss信息 */
    bss_dscr = (mac_bss_dscr_stru *)hmac_scan_get_scanned_bss_by_bssid(hmac_vap, mac_addr);
    if (bss_dscr != OAL_PTR_NULL) {
        payload.rssi = bss_dscr->c_rssi;
    } else {
        payload.rssi = oal_get_real_rssi((osal_s16)OAL_RSSI_INIT_MARKER);
    }

    cfg_msg_init((osal_u8 *)&payload, OAL_SIZEOF(payload), OAL_PTR_NULL, 0, &msg_info);
    /* 先抛事件至Device侧DMAC，同步添加USER，若device侧添加失败，则不添加ko侧user */
    ret = (osal_u32)frw_send_msg_to_device(hmac_vap->vap_id, WLAN_MSG_H2D_C_CFG_DEVICE_ADD_USER, &msg_info, OSAL_TRUE);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_CFG, "{hmac_user_add::frw_send_msg_to_device failed[%d].}", ret);
        return ret;
    }

    /* 设置速率 */
    if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_STA) {
        hmac_vap_init_tx_frame_params(hmac_vap, OSAL_TRUE);
    }

    ret = hmac_user_add_prep(hmac_vap, hmac_user, payload.rssi);
    if (ret != OAL_CONTINUE) {
        return ret;
    }

    ret = hmac_user_add_handler(hmac_vap, hmac_user, ap_type);
    if (ret != OAL_CONTINUE) {
        return ret;
    }

    hmac_user_add_post(hmac_vap);

    /* user初始化信息 则同步至device */
    hmac_user_sync(hmac_user);

    return OAL_SUCC;
}

OAL_STATIC osal_u32 hmac_user_do_add(hmac_vap_stru *hmac_vap, osal_u16 *user_idx, osal_u8 *mac_addr,
    mac_ap_type_enum_uint8 ap_type)
{
    osal_u32 ul_ret;
    hmac_user_stru *hmac_user = OAL_PTR_NULL;
    osal_void *fhook = hmac_get_feature_fhook(HMAC_FHOOK_AMSDU_TX_INIT_USER_ETC);

    /* 申请hmac用户内存，并初始清0 */
    ul_ret = hmac_user_alloc_etc(user_idx);
    if (ul_ret != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_UM, "vap_id[%d] {hmac_user_do_add::alloc failed[%d].}", hmac_vap->vap_id, ul_ret);
        return ul_ret;
    }

    /* 单播用户不能使用userid为0，需重新申请一个。将userid作为aid分配给对端，处理psm时会出错 */
    if (*user_idx == 0) {
        ul_ret = hmac_user_alloc_etc(user_idx);
        /* 重新申请user_idx成功后，再释放user_idx为0的user，否则将再次申请到0 */
        hmac_user = (hmac_user_stru *)mac_res_get_hmac_user_etc(0);
        hmac_user_free_etc(hmac_user);
        if (ul_ret != OAL_SUCC) {
            return ul_ret;
        }

        /* 依然申请到user_idx 0, 释放user_idx资源并异常返回 */
        if (*user_idx == 0) {
            oam_warning_log1(0, OAM_SF_UM, "vap_id[%d] {hmac_user_do_add:alloc user_idx[0] twice}", hmac_vap->vap_id);
            goto hmac_user_free;
        }
    }

    hmac_user = (hmac_user_stru *)mac_res_get_hmac_user_etc(*user_idx);
    if (hmac_user == OAL_PTR_NULL) {
        hmac_user_free_etc(hmac_user);
        return OAL_ERR_CODE_PTR_NULL;
    }

    mac_user_init_etc(hmac_user, *user_idx, mac_addr, hmac_vap);
    /* 设置amsdu域 */
    if (fhook != OSAL_NULL) {
        ((hmac_amsdu_init_user_etc_cb)fhook)(hmac_user);
    }

    /***************************************************************************
        抛事件到DMAC层, 创建dmac用户
    ***************************************************************************/
    ul_ret = hmac_add_user_event_proc(hmac_vap, mac_addr, hmac_user, ap_type);
    if (ul_ret != OAL_SUCC) {
        /* 异常处理，释放内存，device下关联用户数还没有++，这里不需要判断返回值做--操作 */
        goto hmac_user_free;
    }

    /* 添加用户到MAC VAP */
    ul_ret = hmac_vap_add_assoc_user_etc(hmac_vap, hmac_user);
    if (ul_ret != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_UM, "vap_id[%d] {hmac_user_do_add::add user failed[%d].}", hmac_vap->vap_id, ul_ret);
        /* 异常处理，释放内存，device下关联用户数还没有++，这里不需要判断返回值做--操作 */
        goto hmac_user_free;
    }

    return OAL_SUCC;

hmac_user_free:
    ul_ret = OAL_FAIL;
    hmac_user = (hmac_user_stru *)mac_res_get_hmac_user_etc(*user_idx);
    hmac_user_free_etc(hmac_user);
    return ul_ret;
}


/*****************************************************************************
 函 数 名  : hmac_user_add_etc
 功能描述  : 添加用户配置命令
*****************************************************************************/
osal_u32 hmac_user_add_etc(hmac_vap_stru *hmac_vap, osal_u8 *mac_addr, osal_u16 *pus_user_index)
{
    osal_u32 ul_ret;
    osal_u16 user_idx;
    mac_ap_type_enum_uint8 ap_type = MAC_AP_TYPE_BUTT;
    hmac_user_stru *hmac_user = OAL_PTR_NULL;

    ul_ret = hmac_user_add_check(hmac_vap, mac_addr, pus_user_index);
    if (ul_ret != OAL_SUCC) {
        return ul_ret;
    }

    ul_ret = hmac_user_add_prepare(hmac_vap, mac_addr, &ap_type);
    if (ul_ret != OAL_SUCC) {
        return ul_ret;
    }

    ul_ret = hmac_user_do_add(hmac_vap, &user_idx, mac_addr, ap_type);
    if (ul_ret != OAL_SUCC) {
        return ul_ret;
    }

    *pus_user_index = user_idx;  /* 出参赋值 */

    hmac_user = (hmac_user_stru *)mac_res_get_hmac_user_etc(user_idx);
    /* 初始话hmac user部分信息 */
    hmac_user_init_etc(hmac_user);

    hmac_user->user_type = MAC_USER_TYPE_DEFAULT;

    osal_adapt_atomic_set(&(hmac_user->use_cnt), 1);

    frw_util_notifier_notify(WLAN_UTIL_NOTIFIER_EVENT_ADD_USER, hmac_user);

    oam_warning_log4(0, OAM_SF_UM, "{mac:%02X:%02X:%02X:%02X:XX:XX}",
        mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3]); /* 0 1 2 3:MAC地址 */

    oam_warning_log3(0, OAM_SF_CFG,
        "{hmac_user_add_handler::add user[%d] SUCC! vap user nums[%d],hal device user nums[%d].}",
        user_idx, hmac_vap->user_nums, hmac_vap->hal_device->assoc_user_nums);
    oam_warning_log3(0, OAM_SF_CFG,
        "{hmac_user_add_etc:bw[%d]ap_type[%d]lut idx[%d]}", hmac_user->bandwidth_cap, ap_type, hmac_user->lut_index);

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_add_multi_user
 功能描述  : hmac层创建组播用户
*****************************************************************************/
osal_u32  hmac_user_add_multi_user_etc(hmac_vap_stru *hmac_vap, osal_u16 *pus_user_index)
{
    osal_u32      ul_ret;
    osal_u16      user_index;
    hmac_user_stru  *hmac_user;
    osal_void *fhook = hmac_get_feature_fhook(HMAC_FHOOK_WAPI_INIT);

    ul_ret = hmac_user_alloc_etc(&user_index);
    if (ul_ret != OAL_SUCC) {
        oam_error_log2(0, OAM_SF_UM, "vap_id[%d] {hmac_user_add_multi_user_etc::hmac_user_alloc_etc failed[%d].}",
                       hmac_vap->vap_id, ul_ret);
        return ul_ret;
    }

    /* 初始化组播用户基本信息 */
    hmac_user = (hmac_user_stru *)mac_res_get_hmac_user_etc(user_index);
    if (hmac_user == OAL_PTR_NULL) {
        oam_warning_log2(0, OAM_SF_UM, "vap_id[%d] {hmac_user_add_multi_user_etc::hmac_user[%d] null.}",
                         hmac_vap->vap_id, user_index);
        return OAL_ERR_CODE_PTR_NULL;
    }

    mac_user_init_etc(hmac_user, user_index, OAL_PTR_NULL, hmac_vap);
    *pus_user_index = user_index;
    osal_adapt_atomic_set(&(hmac_user->use_cnt), 1);

    if (fhook != OSAL_NULL) {
        /* 初始化wapi对象 */
        ((hmac_wapi_init_etc_cb)fhook)(user_index, OAL_FALSE);
    }

    oam_warning_log2(0, OAM_SF_ANY,
        "vap_id[%d] {hmac_user_add_multi_user_etc, user index[%d].}", hmac_vap->vap_id, user_index);

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_user_del_multi_user_etc
 功能描述  : hmac层删除multiuser
*****************************************************************************/
osal_u32  hmac_user_del_multi_user_etc(hmac_vap_stru *hmac_vap)
{
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);
    hmac_user_stru *hmac_user = OSAL_NULL;
    osal_void *fhook = hmac_get_feature_fhook(HMAC_FHOOK_WAPI_DEINIT);

    hmac_user = (hmac_user_stru *)mac_res_get_hmac_user_etc(hmac_vap->multi_user_idx);
    if (hmac_user == OSAL_NULL) {
        oam_error_log2(0, OAM_SF_ANY, "vap_id[%d] {hmac_user_del_multi_user_etc::get hmac_user[%d] null.}",
                       hmac_vap->vap_id, hmac_vap->multi_user_idx);
        return OAL_ERR_CODE_PTR_NULL;
    }

    mac_vap_intrrupt_disable();
    hmac_user->is_user_alloced = OSAL_FALSE;
    osal_adapt_atomic_dec(&(hmac_user->use_cnt));
    mac_vap_intrrupt_enable();
    while (osal_adapt_atomic_read(&(hmac_user->use_cnt)) != 0) {
        osal_msleep(1); /* 休眠1ms让出CPU */
    }

    if (fhook != OSAL_NULL) {
        ((hmac_wapi_deinit_etc_cb)fhook)(hmac_vap->multi_user_idx);
    }

    /* dmac user相关操作去注册 */
    hmac_alg_del_assoc_user_notify(hmac_vap, hmac_user);

    /* 删除tid队列中的所有信息 */
    hmac_tid_clear(hmac_user, hmac_device, OSAL_TRUE);

    /* 删除用户节能结构 */
    hmac_psm_user_ps_structure_destroy(hmac_user);

    hmac_user_free_etc(hmac_user);

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_user_add_notify_alg_etc
 功能描述  : 抛事件给dmac，让其在dmac挂算法钩子
*****************************************************************************/
osal_u32  hmac_user_add_notify_alg_etc(hmac_vap_stru *hmac_vap, osal_u16 user_idx)
{
    hmac_user_stru                 *hmac_user;

    osal_s32 ret;
    hmac_ctx_add_user_stru payload;
    frw_msg msg_info;
    (osal_void)memset_s(&msg_info, OAL_SIZEOF(msg_info), 0, OAL_SIZEOF(msg_info));
    (osal_void)memset_s(&payload, OAL_SIZEOF(payload), 0, OAL_SIZEOF(payload));

    payload.user_idx = user_idx;
    payload.sta_aid = hmac_vap->sta_aid;
    hmac_user = (hmac_user_stru *)mac_res_get_hmac_user_etc(user_idx);
    if (osal_unlikely(hmac_user == OAL_PTR_NULL)) {
        oam_error_log1(0, OAM_SF_CFG, "{hmac_user_add_notify_alg_etc::null param,hmac_user[%d].}", user_idx);
        return OAL_ERR_CODE_PTR_NULL;
    }

#ifdef _PRE_WLAN_FEATURE_11AX
    (osal_void)memcpy_s(&payload.he_hdl, sizeof(mac_he_hdl_stru),
                        &(hmac_user->he_hdl), sizeof(mac_he_hdl_stru));
#endif
    hmac_user_get_vht_hdl_etc(hmac_user, &payload.vht_hdl);
    hmac_user_get_ht_hdl_etc(hmac_user, &payload.ht_hdl);

    hmac_config_mvap_set_sta_pm_on(hmac_vap, OSAL_TRUE);

    cfg_msg_init((osal_u8 *)&payload, OAL_SIZEOF(payload), OAL_PTR_NULL, 0, &msg_info);
    ret = hmac_user_add_notify_alg(hmac_vap, hmac_user, &msg_info);
    if (ret != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_ANY,
            "vap_id[%d] {hmac_user_add_notify_alg_etc::send_cfg_to_device failed[%d].}",
            hmac_vap->vap_id, ret);
        return (osal_u32)ret;
    }
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : mac_vap_get_hmac_user_by_addr_etc
 功能描述  : 根据mac地址获取mac_user指针
*****************************************************************************/
hmac_user_stru  *mac_vap_get_hmac_user_by_addr_etc(hmac_vap_stru *hmac_vap, const osal_u8  *mac_addr)
{
    osal_u32              ul_ret;
    osal_u16              user_idx   = 0xffff;
    hmac_user_stru         *hmac_user = OAL_PTR_NULL;

    /* 根据mac addr找sta索引 */
    ul_ret = hmac_vap_find_user_by_macaddr_etc(hmac_vap, mac_addr, &user_idx);
    if (ul_ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{mac_vap_get_hmac_user_by_addr_etc::find_user_by_macaddr failed[%d]}", ul_ret);
        if (mac_addr != OAL_PTR_NULL) {
            oam_warning_log4(0, OAM_SF_ANY, "{mac_vap_get_hmac_user_by_addr_etc:: mac_addr[%02x %02x %02x %02x XX XX}",
                mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3]); /* 0 1 2 3:索引值 */
        }
        return OAL_PTR_NULL;
    }

    /* 根据sta索引找到user内存区域 */
    hmac_user = mac_res_get_hmac_user_etc(user_idx);
    if (hmac_user == OAL_PTR_NULL) {
        oam_error_log1(0, OAM_SF_ANY, "{mac_vap_get_hmac_user_by_addr_etc::user[%d] ptr null.}", user_idx);
    }
    return hmac_user;
}

// from hmac_user_rom.h
/*****************************************************************************
  2 函数声明
*****************************************************************************/

/*****************************************************************************
  3 函数实现
*****************************************************************************/
/*****************************************************************************
 函 数 名  : hmac_user_resume
 功能描述  : 恢复用户所有TID
*****************************************************************************/
osal_u32 hmac_user_resume(hmac_user_stru *hmac_user)
{
    osal_u8 tid_idx;
    hmac_tid_stru *tid = OSAL_NULL;
    osal_u32 ret;
    hal_to_dmac_device_stru *hal_device = OSAL_NULL;

    if (hmac_user == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_user_resume::hmac_user null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    hal_device = hmac_user_get_hal_device(hmac_user);
    if (hal_device == OSAL_NULL) {
        oam_warning_log0(0, OAM_SF_COEX, "{hmac_user_resume::hal_device null}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 防止用户已经删除后，P2P NOA中断事件才得到处理，导致device获取失败 */
    if (hmac_user->user_asoc_state == MAC_USER_STATE_BUTT) {
        oam_warning_log0(0, OAM_SF_COEX, "{hmac_user_resume::user in BUTT_mode, cannot resume tid!}");
        return OAL_SUCC;
    }

    for (tid_idx = 0; tid_idx < WLAN_TID_MAX_NUM; tid_idx++) {
        tid = &(hmac_user->tx_tid_queue[tid_idx]);

        ret = hmac_tid_resume(tid, DMAC_TID_PAUSE_RESUME_TYPE_PS);
        if (ret != OAL_SUCC) {
            return ret;
        }
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_user_init
 功能描述  : 初始化DMAC 用户
*****************************************************************************/
static osal_u32 hmac_user_init(hmac_user_stru *hmac_user)
{
    hmac_vap_stru *hmac_vap = OSAL_NULL;

    /* RSSI统计量初始化 */
    hmac_user->rx_rssi = OAL_RSSI_INIT_MARKER;
    /* 初始化时间戳 */
    hmac_user->last_active_timestamp = osal_get_time_stamp_ms();
    /* DMAC USER TID 初始化 */
    hmac_tid_tx_queue_init(hmac_user->tx_tid_queue, hmac_user);
    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(hmac_user->vap_id);
    if (hmac_vap == OSAL_NULL) {
        oam_error_log1(0, OAM_SF_TX, "vap_id[%d] {hmac_user_init::hmac_vap null.}", hmac_user->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 设置dmac_user_stru中的gruopid和partial aid两个成员变量,beaforming和txop ps会用到 */
    if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_STA) {
        /* 把bssid中48个bit的高9bit对应的10进制值作为paid,STA时给txbf ndp使用 */
        osal_u16 temp_partial_aid = hmac_vap->bssid[0];
        temp_partial_aid = (temp_partial_aid << 8) | hmac_vap->bssid[1]; /* 左移8位组成u16 */
        hmac_user->groupid = 0;
        hmac_user->partial_aid = temp_partial_aid >> 7;                 /* 右移7位取高9位 */
    } else if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_AP) {
        /* AP保存对端user的属性，发送txop使用 */
        osal_u16 temp_aid   = hmac_user->assoc_id & 0x1FF;
        /* 5表示bssid的第5位，4表示右移4位，以得到bssid[5]的高4位,这里用高4位与低4位异或 */
        osal_u8 temp_bssid = (hmac_vap->bssid[5] & 0x0F) ^ ((hmac_vap->bssid[5] & 0xF0) >> 4);
        hmac_user->groupid = 63; /* 63表示设置groupid为63 */
        /* 5表示左移5位，以和temp_aid一致，9表示1左移9位，(1 << 9) - 1=0xff */
        hmac_user->partial_aid = (osal_u16)(temp_aid + (temp_bssid << 5)) & (osal_u16)((1 << 9) - 1);
    } else {
        hmac_user->groupid     = 63; /* 63表示groupid设置为63 */
        hmac_user->partial_aid = 0;
    }

    /* 初始化用户状态为新用户 */
    hmac_user->new_add_user = OSAL_TRUE;
    hmac_user->is_rx_eapol_key_open = OSAL_TRUE;
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_user_add_multi_user
 功能描述  : dmac创建组播用户
             (因为组播用户是跟随vap的创建而创建，因此不能在add时候从hmac抛事件来创建，删除时候可以hmac抛事件来删除)
*****************************************************************************/
osal_u32 hmac_user_add_multi_user(hmac_vap_stru *hmac_vap, osal_u16 multi_user_idx)
{
    hmac_user_stru *hmac_multi_user = OSAL_NULL;
    osal_u16 user_idx;
    osal_u32 ret;

    user_idx = multi_user_idx;
    hmac_multi_user = mac_res_get_hmac_user_etc(user_idx);
    if (hmac_multi_user == OSAL_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (osal_unlikely(hmac_vap == OSAL_NULL)) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    ret = hmac_user_init(hmac_multi_user);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_CFG, "{hmac_user_add_multi_user::hmac_user_init() fail, ret = [%d]}", ret);
    }

    /* 组播用户都是活跃的 */
    hmac_multi_user->active_user = OSAL_TRUE;

    hmac_alg_add_assoc_user_notify(hmac_vap, hmac_multi_user);

    /* 初始化节能队列 */
    hmac_psm_user_ps_structure_init(hmac_multi_user);

    oam_warning_log2(0, 0, "vap_id[%d] {hmac_user_add_multi_user, user index[%d].}", hmac_vap->vap_id, user_idx);
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_user_inactive
 功能描述  : 设置用户为非活跃, 归还lut资源
*****************************************************************************/
static osal_u32 hmac_user_inactive(hmac_user_stru *hmac_user)
{
    hmac_vap_stru *hmac_vap = OSAL_NULL;
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);

    /* 已经是非活跃用户，直接返回 */
    if (hmac_user->active_user == OSAL_FALSE) {
        return OAL_SUCC;
    }

    hmac_vap = mac_res_get_hmac_vap(hmac_user->vap_id);
    if (hmac_vap == OSAL_NULL) {
        oam_error_log4(0, OAM_SF_ANY,
            "hmac_user_inactive::hmac_vap ptr null, uid=%d, user_mac_addr[%02x-xx-xx-xx-%02x-%02x].",
            hmac_user->assoc_id, hmac_user->user_mac_addr[0],
            /* 用户mac地址的第4、第5元素 */
            hmac_user->user_mac_addr[4], hmac_user->user_mac_addr[5]);
        return OAL_ERR_CODE_PTR_NULL;
    }

    oam_info_log3(0, OAM_SF_ANY, "vap_id[%d] hmac_user_inactive user_idx=%d, return lut index=%d.", hmac_vap->vap_id,
                  hmac_user->assoc_id, hmac_user->lut_index);

    /* 删除MAC LUT表 */
    hmac_user_del_lut_info(hmac_user->lut_index);

    /* 将gtk的乒乓指示位清0 注意:此位ap无作用，sta使用 */
    hmac_reset_gtk_token(hmac_vap);

    /* 归还lut index */
    mac_user_del_ra_lut_index(hmac_device->ra_lut_index_table, hmac_user->lut_index);

    /* set inactive user lut index invalid */
    hmac_user->lut_index = HMAC_INVALID_USER_LUT_INDEX;

    hmac_user->active_user = OSAL_FALSE;

    /* 同步active user */
    hmac_device_sync(hmac_device);
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_user_active
 功能描述  : 设置用户为活跃，申请lut资源
*****************************************************************************/
static osal_u32 hmac_user_active(hmac_user_stru *hmac_user)
{
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);
    hmac_vap_stru *hmac_vap = OSAL_NULL;
    osal_u8 lut_idx;

    /* 已经是活跃用户，直接返回 */
    if (hmac_user->active_user == OSAL_TRUE) {
        return OAL_SUCC;
    }

    hmac_vap = mac_res_get_hmac_vap(hmac_user->vap_id);
    if (hmac_vap == OSAL_NULL) {
        oam_error_log1(0, OAM_SF_ANY, "vap_id[%d] {hmac_user_active::null mac vap }", hmac_user->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 申请lut index */
    lut_idx = mac_user_get_ra_lut_index(hmac_device->ra_lut_index_table, 0, WLAN_ACTIVE_USER_MAX_NUM);

    hmac_user->lut_index = lut_idx; /* 置为合法lut idx，表示用户是active有效用户 */
    hmac_user->active_user = OSAL_TRUE;

    oam_info_log2(0, 0, "hmac_user_active user_idx=%d, apply lut index=%d.", hmac_user->assoc_id,
        hmac_user->lut_index);

    /* 同步active user */
    hmac_device_sync(hmac_device);
    /* 设置peer地址 */
    if (hmac_vap->hal_device != OSAL_NULL) {
        hmac_user_set_lut_info(hmac_vap->hal_device, hmac_user->lut_index, hmac_user->user_mac_addr);
    }
    /* 设置密钥 */
    return hmac_11i_add_key_from_user(hmac_vap, hmac_user);
}
/*****************************************************************************
 函 数 名  : hmac_user_key_search_fail_handler
 功能描述  : 接收帧key search fail处理
*****************************************************************************/
osal_void hmac_user_key_search_fail_handler(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    mac_ieee80211_frame_stru *frame_hdr)
{
    osal_u32 ret;

    /* 如果用户不存在，什么都不做 */
    if (hmac_user == OSAL_NULL) {
        oam_warning_log0(0, OAM_SF_CFG, "{hmac_user_key_search_fail_handler::hmac_user null,Deauth it...}");
        hmac_rx_data_user_is_null(hmac_vap, frame_hdr);
        return;
    }

    /* active if not up to maximum */
    ret = hmac_user_active(hmac_user);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_CFG,
            "{hmac_user_key_search_fail_handler::hmac_user_active() fail! ret = [%d]}", ret);
    }
}

/*****************************************************************************
 函 数 名  : hmac_qosnull_cb_init
 功能描述  : 填写qosnull帧cb字段
*****************************************************************************/
static osal_void hmac_qosnull_cb_init(const hmac_vap_stru *hmac_vap, const hmac_user_stru *hmac_user, osal_u8 ac,
    oal_netbuf_stru *net_buf)
{
    mac_tx_ctl_stru *tx_ctrl = (mac_tx_ctl_stru *)oal_netbuf_cb(net_buf);
    /* 填写tx部分 */
    tx_ctrl->ack_policy = WLAN_TX_NORMAL_ACK;
    tx_ctrl->ac = ac;
    tx_ctrl->retried_num = 0;
    tx_ctrl->tid = wlan_wme_ac_to_tid(ac);
    tx_ctrl->tx_vap_index = hmac_vap->vap_id;
    tx_ctrl->tx_user_idx = (osal_u8)hmac_user->assoc_id;
    /* 填写tx rx公共部分 */
    tx_ctrl->frame_header_length = sizeof(mac_ieee80211_qos_frame_stru);
    tx_ctrl->mpdu_num = 1;
    tx_ctrl->msdu_num = 1;
    tx_ctrl->netbuf_num = 1;
    tx_ctrl->mpdu_payload_len = 0;
    tx_ctrl->tx_user_idx = (osal_u8)hmac_user->assoc_id;

    oal_netbuf_put(net_buf, tx_ctrl->frame_header_length);
    return;
}

/*****************************************************************************
 函 数 名  : hmac_send_qosnull
 功能描述  : 组帧并发送Qos null帧
*****************************************************************************/
static osal_u32 hmac_send_qosnull(hmac_vap_stru *hmac_vap, const hmac_user_stru *hmac_user, osal_u8 ac,
    oal_bool_enum_uint8 ps)
{
    oal_netbuf_stru *net_buf;
    osal_u32 ret;
    mac_ieee80211_qos_frame_stru *mac_header;
    osal_u16 tx_direction;
    /* 入参检查 */
    if ((hmac_vap == OSAL_NULL) || (hmac_vap->hal_device == OSAL_NULL) || (hmac_user == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_KEEPALIVE, "{hmac_uapsd_send_qosnull:: func input  is null.}");
        return OAL_ERR_CODE_KEEPALIVE_PTR_NULL;
    }
    if (ac >= MAC_AC_PARAM_LEN) {
        oam_error_log1(0, OAM_SF_KEEPALIVE, "{hmac_uapsd_send_qosnull:: ac %d is too large.}", ac);
        return OAL_FAIL;
    }
    /* 申请net_buff */
    net_buf = oal_netbuf_alloc_ext(OAL_NORMAL_NETBUF, WLAN_SHORT_NETBUF_SIZE, OAL_NETBUF_PRIORITY_HIGH);
    if (net_buf == OSAL_NULL) {
        oam_error_log1(0, OAM_SF_KEEPALIVE,
            "vap_id[%d] {hmac_uapsd_send_qosnull::net_buf failed.}", hmac_vap->vap_id);
        return OAL_ERR_CODE_ALLOC_MEM_FAIL;
    }

    oal_set_netbuf_prev(net_buf, OSAL_NULL);
    oal_set_netbuf_next(net_buf, OSAL_NULL);
    /* null帧发送方向From AP || To AP */
    tx_direction = (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_AP) ? WLAN_FRAME_FROM_AP : WLAN_FRAME_TO_AP;
    /* 填写帧头 */
    (osal_void)memset_s(oal_netbuf_header(net_buf), sizeof(mac_ieee80211_qos_frame_stru),
        0, sizeof(mac_ieee80211_qos_frame_stru));
    mac_null_data_encap(oal_netbuf_header(net_buf),
        ((osal_u16)(WLAN_PROTOCOL_VERSION | WLAN_FC0_TYPE_DATA | WLAN_FC0_SUBTYPE_QOS_NULL) | tx_direction),
        hmac_user->user_mac_addr, mac_mib_get_station_id(hmac_vap));
    mac_header = (mac_ieee80211_qos_frame_stru *)oal_netbuf_header(net_buf);
    mac_header->qc_tid = wlan_wme_ac_to_tid(ac);
    mac_header->qc_eosp = 0;
    mac_header->frame_control.power_mgmt = ps;

    /* 协议规定单播的QOS NULL DATA只允许normal ack */
    mac_header->qc_ack_polocy = WLAN_TX_NORMAL_ACK;

    hmac_qosnull_cb_init(hmac_vap, hmac_user, ac, net_buf);
    ret = hmac_tx_process_data(hmac_vap->hal_device, hmac_vap, net_buf);
    if (ret != OAL_SUCC) {
        oam_error_log2(0, OAM_SF_KEEPALIVE,
            "vap_id[%d] {hmac_uapsd_send_qosnull::tx_process_data failed[%d].}", hmac_vap->vap_id, ret);
        hmac_tx_excp_free_netbuf(net_buf);
        return ret;
    }
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_send_null_frame_to_sta
 功能描述  : keepalive超时发送null帧函数
*****************************************************************************/
OSAL_STATIC osal_u32 hmac_send_null_frame_to_sta(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user)
{
    osal_u8 ac = WLAN_WME_AC_VO;
    osal_char i;
    osal_void *fhook = OSAL_NULL;
    hmac_user_uapsd_info_stru *user_uapsd_info = OSAL_NULL;

    if (hmac_vap->vap_mode != WLAN_VAP_MODE_BSS_AP) {
        oam_error_log2(0, OAM_SF_KEEPALIVE, "vap_id[%d] {hmac_send_null_frame_to_sta:: vap mode %d is wrong.}",
            hmac_vap->vap_id, hmac_vap->vap_mode);
        return OAL_FAIL;
    }

    /* AP侧根据user节能状态下选择发null帧还是Qos null帧 */
    fhook = hmac_get_feature_fhook(HMAC_FHOOK_AP_UAPSD_GET_USER_INFO);
    if (fhook != OSAL_NULL) {
        user_uapsd_info = ((hmac_ap_get_user_uapsd_info_cb)fhook)(hmac_user->assoc_id);
    }

    if (user_uapsd_info != OSAL_NULL && (user_uapsd_info->uapsd_flag & HMAC_USR_UAPSD_EN) != OSAL_FALSE) {
        for (i = MAC_AC_PARAM_LEN - 1; i >= 0; i--) {
            if (user_uapsd_info->uapsd_status.ac_trigger_ena[(osal_u8)i] == OSAL_TRUE) {
                ac = (osal_u8)i;
                break;
            }
        }
        return hmac_send_qosnull(hmac_vap, hmac_user, ac, OSAL_FALSE);
    }
    /* 用户处于非节能状态 */
    return hmac_psm_send_null_data(hmac_vap, hmac_user, OSAL_FALSE, OSAL_FALSE);
}

/*****************************************************************************
 功能描述  : STA根据AP带宽设置SEC20M_OFFSET
*****************************************************************************/
OSAL_STATIC osal_void hmac_sta_set_sec_bandwidth_offset(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user)
{
    mac_user_ht_hdl_stru *mac_ht_hdl;
    mac_vht_hdl_stru *mac_vht_hdl;
    wlan_channel_bandwidth_enum_uint8 bandwidth_ap = WLAN_BAND_WIDTH_20M;
#ifdef _PRE_WLAN_FEATURE_11AX
    mac_he_hdl_stru *mac_he_hdl;
#endif

    /* 获取HT和VHT结构体指针 */
    mac_ht_hdl = &(hmac_user->ht_hdl);
    mac_vht_hdl = &(hmac_user->vht_hdl);
#ifdef _PRE_WLAN_FEATURE_11AX
    mac_he_hdl = &(hmac_user->he_hdl);
#endif

    /******************* VHT BSS operating channel width ****************
     -----------------------------------------------------------------
     |HT Oper Chl Width |VHT Oper Chl Width |BSS Oper Chl Width|
     -----------------------------------------------------------------
     |       0          |        0          |    20MHZ         |
     -----------------------------------------------------------------
     |       1          |        0          |    40MHZ         |
     -----------------------------------------------------------------
     |       1          |        1          |    80MHZ         |
     -----------------------------------------------------------------
     |       1          |        2          |    160MHZ        |
     -----------------------------------------------------------------
     |       1          |        3          |    80+80MHZ      |
     -----------------------------------------------------------------
    **********************************************************************/
    if (mac_vht_hdl->vht_capable == OAL_TRUE) {
        bandwidth_ap = mac_get_bandwith_from_center_freq_seg0_seg1(mac_vht_hdl->channel_width,
            hmac_vap->channel.chan_number, mac_vht_hdl->channel_center_freq_seg0,
            mac_vht_hdl->channel_center_freq_seg1);
    }

#ifdef _PRE_WLAN_FEATURE_11AX
    if ((mac_he_hdl->he_capable == OAL_TRUE) &&
        (mac_he_hdl->he_oper_ie.he_oper_param.vht_operation_info_present == OSAL_TRUE)) {
        bandwidth_ap = mac_get_bandwith_from_center_freq_seg0_seg1(
            mac_he_hdl->he_oper_ie.vht_operation_info.channel_width, hmac_vap->channel.chan_number,
            mac_he_hdl->he_oper_ie.vht_operation_info.center_freq_seg0,
            mac_he_hdl->he_oper_ie.vht_operation_info.center_freq_seg1);
    }
#endif

    /* ht 20/40M带宽的处理 */
    if ((mac_ht_hdl->ht_capable == OAL_TRUE) && (bandwidth_ap <= WLAN_BAND_WIDTH_40MINUS) &&
        (mac_mib_get_forty_mhz_operation_implemented(hmac_vap) == OAL_TRUE)) {
        /* 更新带宽模式 */
        bandwidth_ap = mac_get_bandwidth_from_sco(mac_ht_hdl->secondary_chan_offset);
    }

    // 次信道配置
    hal_set_sec_bandwidth_offset(bandwidth_ap);
}

/****************************************************************************
 函 数 名  : hmac_user_set_bandwith_hander
 功能描述  : user设置带宽
*****************************************************************************/
osal_u32 hmac_user_set_bandwith_handler(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    wlan_bw_cap_enum_uint8 bw_cap)
{
    osal_u8 old_avail_bandwidth = hmac_user->avail_bandwidth;
    osal_u8 old_cur_bandwidth = hmac_user->cur_bandwidth;

    hmac_user_set_bandwidth_info_etc(hmac_user, bw_cap, bw_cap);

    if (old_avail_bandwidth != hmac_user->avail_bandwidth || old_cur_bandwidth != hmac_user->cur_bandwidth) {
        /* 带宽更新 则同步至device */
        hmac_user_sync(hmac_user);
        if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_STA) {
            hmac_sta_set_sec_bandwidth_offset(hmac_vap, hmac_user);
        }
    }

    oam_warning_log4(0, OAM_SF_ASSOC,
                     "{hmac_user_set_bandwith_handler:usr id[%d] bw cap[%d]avil[%d]cur[%d].\r\n}", hmac_user->assoc_id,
                     hmac_user->bandwidth_cap, hmac_user->avail_bandwidth, hmac_user->cur_bandwidth);

    return OAL_SUCC;
}

OSAL_STATIC osal_u32 hmac_user_keepalive_proc(hmac_vap_stru *hmac_vap, osal_u32 aging_time,
    osal_u32 send_null_frame_time)
{
    osal_u64 present_time;
    struct osal_list_head *entry = OSAL_NULL;
    struct osal_list_head *dlist_tmp = OSAL_NULL;
    hmac_user_stru *hmac_user = OSAL_NULL;
    osal_void *fhook = OSAL_NULL;
    osal_u8 is_uapsd_queue_empty = OSAL_TRUE;

    osal_list_for_each_safe(entry, dlist_tmp, &(hmac_vap->mac_user_list_head)) {
        osal_u64 runtime;
        hmac_user = osal_list_entry(entry, hmac_user_stru, user_dlist);
        if (hmac_user == OSAL_NULL) {
            oam_error_log1(0, OAM_SF_KEEPALIVE, "vap_id[%d] user_tmp is null.", hmac_vap->vap_id);
            return OAL_ERR_CODE_PTR_NULL;
        }
        present_time = osal_get_time_stamp_ms();
        runtime = osal_get_runtime_u64(hmac_user->last_active_timestamp, present_time);
        if (runtime > aging_time) { // 5mins
            oam_warning_log1(0, OAM_SF_DFT, "hmac_user_keepalive_proc:keepalive timeout,send DISASSOC,err code[%d]",
                DMAC_DISASOC_MISC_KEEPALIVE);
            wifi_printf("[keepalive] aging sta [%02X:%02X:%02X:%02X:XX:XX]\r\n", hmac_user->user_mac_addr[0],
                /* 打印用户mac地址的1、2、3位 */
                hmac_user->user_mac_addr[1], hmac_user->user_mac_addr[2], hmac_user->user_mac_addr[3]);
            hmac_send_disassoc_misc_event(hmac_vap, hmac_user->assoc_id, DMAC_DISASOC_MISC_KEEPALIVE);
        } else if (runtime > send_null_frame_time) { // 15s
            /* 发送队列与节能队列无数据缓冲，发送null 帧触发keepalive ; 否则不发送null 帧 */
            fhook = hmac_get_feature_fhook(HMAC_FHOOK_AP_UAPSD_IS_QUEUE_EMPTY);
            if (fhook != OSAL_NULL) {
                is_uapsd_queue_empty = ((hmac_psm_is_uapsd_empty_cb)fhook)(hmac_user);
            }
            if ((hmac_psm_is_psm_empty(hmac_user) == OSAL_TRUE) &&
                (hmac_psm_is_tid_empty(hmac_user) == OSAL_TRUE) && (is_uapsd_queue_empty == OSAL_TRUE)) {
                hmac_send_null_frame_to_sta(hmac_vap, hmac_user);
                common_log_dbg4(0, OAM_SF_KEEPALIVE,
                    "{hmac_user_keepalive_proc::send null frame to %02X:%02X:%02X:%02X:XX:XX.}",
                    hmac_user->user_mac_addr[0],
                    /* 打印用户mac地址的1、2、3字节 */
                    hmac_user->user_mac_addr[1], hmac_user->user_mac_addr[2], hmac_user->user_mac_addr[3]);
            } else {
                common_log_dbg4(0, OAM_SF_KEEPALIVE,
                    "{hmac_user_keepalive_proc::do not send null frame to %02X:%02X:%02X:%02X:XX:XX.}",
                    hmac_user->user_mac_addr[0],
                    /* 打印用户mac地址的1、2、3字节 */
                    hmac_user->user_mac_addr[1], hmac_user->user_mac_addr[2], hmac_user->user_mac_addr[3]);
                common_log_dbg3(0, OAM_SF_KEEPALIVE, "{keepalive: psm_empty[%u] tid_empty[%u] uapsd_empty[%u].}",
                    hmac_psm_is_psm_empty(hmac_user), hmac_psm_is_tid_empty(hmac_user), is_uapsd_queue_empty);
            }
        }
    }
    return OAL_CONTINUE;
}

/*****************************************************************************
 函 数 名  : hmac_user_keepalive_timer
 功能描述  : 用户keepalive定时器到期处理函数
*****************************************************************************/
osal_u32 hmac_user_keepalive_timer(osal_void *arg)
{
    hmac_device_stru *hmac_device = (hmac_device_stru *)arg;
    osal_u8 vap_idx = 0;
    osal_u32 keepalive_flag = OSAL_FALSE;
#ifdef _PRE_WLAN_FEATURE_P2P
    osal_void *fhook = OSAL_NULL;
#endif
    /* 遍历device下的所有用户，将到期老化的删除掉 */
    /* 业务vap从1开始 */
    for (vap_idx = 0; vap_idx < hmac_device->vap_num; vap_idx++) {
        osal_u32 aging_time, send_null_frame_time, ret;
        hmac_vap_stru *hmac_vap = mac_res_get_hmac_vap(hmac_device->vap_id[vap_idx]);
        if (osal_unlikely(hmac_vap == OSAL_NULL)) {
            oam_warning_log1(0, OAM_SF_KEEPALIVE, "vap_id[%d] {hmac_vap null.}", hmac_device->vap_id[vap_idx]);
            return OAL_ERR_CODE_PTR_NULL;
        }
        /* 如果keepalive 开关已关, 此时定时器还是开启的话就关闭定时器 */
        if ((hmac_vap->cap_flag.keepalive == OSAL_FALSE) && (hmac_device->keepalive_timer.is_registerd == OSAL_TRUE)) {
            frw_destroy_timer_entry(&(hmac_device->keepalive_timer));
        }

        /* 用户老化只针对AP模式，非AP模式则跳出 或没有keepalive能力则跳出 */
        if ((hmac_vap->vap_mode != WLAN_VAP_MODE_BSS_AP) || (hmac_vap->cap_flag.keepalive == OSAL_FALSE)) {
            continue;
        }
        /* 置位keepalive flag 表示此时存在AP类vap打开了keepalive能力 */
        keepalive_flag = OSAL_TRUE;

        aging_time = hmac_config_get_ap_user_aging_time();
        send_null_frame_time = WLAN_AP_KEEPALIVE_INTERVAL;

#ifdef _PRE_WLAN_FEATURE_P2P
        if (is_p2p_go(hmac_vap) == OSAL_TRUE) {
            aging_time = WLAN_P2PGO_USER_AGING_TIME;
            fhook = hmac_get_feature_fhook(HMAC_FHOOK_BTCOEX_KEEPALIVE_TIMER_ECT);
            if (fhook != OSAL_NULL) {
                ((hmac_btcoex_keepalive_timer_ect_cb)fhook)(&aging_time);
            }
            /* dbac时，aging_time为2倍WLAN_P2PGO_USER_AGING_TIME */
            if (mac_is_dbac_running(hmac_device) == OSAL_TRUE) {
                aging_time = 2 * WLAN_P2PGO_USER_AGING_TIME; /* 2: 翻倍 */
            }
            send_null_frame_time = WLAN_GO_KEEPALIVE_INTERVAL;
        }
#endif
        common_log_dbg3(0, OAM_SF_KEEPALIVE, "{hmac_user_keepalive_timer::mode[%d] aging_time[%d] send_time[%d].}",
            hmac_vap->vap_mode, aging_time, send_null_frame_time);
        ret = hmac_user_keepalive_proc(hmac_vap, aging_time, send_null_frame_time);
        if (ret != OAL_CONTINUE) {
            return ret;
        }
    }
    /* 如果遍历完所有VAP仍然没有 AP类VAP打开了keepalive能力，关闭定时器 */
    if ((keepalive_flag == OSAL_FALSE) && (hmac_device->keepalive_timer.is_registerd == OSAL_TRUE)) {
        frw_destroy_timer_entry(&(hmac_device->keepalive_timer));
    }
    return OAL_SUCC;
}

WIFI_TCM_TEXT osal_u32 hmac_alg_distance_notify_hook(hal_to_dmac_device_stru *hal_device, osal_u8 distance)
{
    hal_alg_stat_info_stru *hal_alg_stat = OSAL_NULL;

    if (hal_device == OSAL_NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{hmac_alg_distance_notify_hook::hal_device null}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    hal_alg_stat = &hal_device->hal_alg_stat;
    if (hal_alg_stat->alg_distance_stat == distance) {
        return OAL_SUCC;
    }

    if (distance > HAL_ALG_USER_DISTANCE_BUTT) {
        oam_error_log2(0, OAM_SF_ANY,
            "{hmac_alg_distance_notify_hook::alg distance status is out of the way2.old:%d, new:%d}",
            hal_alg_stat->alg_distance_stat, distance);
        return OAL_FAIL;
    }
    oam_warning_log1(0, OAM_SF_ANY, "{hmac_alg_distance_notify distance[%u]}", distance);
    hal_alg_stat->alg_distance_stat = distance;
    return OAL_SUCC;
}

/*****************************************************************************hmac_user_add_multi_user
 函 数 名  : hmac_compatibility_handler
 功能描述  : 兼容性问题特殊配置处理函数
*****************************************************************************/
static osal_void hmac_compatibility_handler(hmac_vap_stru *hmac_vap, mac_ap_type_enum_uint8 ap_type,
    oal_bool_enum_uint8 is_add_user)
{
    osal_void *fhook = OAL_PTR_NULL;
    if (osal_unlikely(hmac_vap == OSAL_NULL)) {
        return;
    }

    if (is_add_user == OAL_FALSE) {
        if (is_legacy_sta(hmac_vap)) {
            fhook = hmac_get_feature_fhook(HMAC_FHOOK_BTCOEX_COMPATIBILITY_SET_PS_TYPE);
            if (fhook != OSAL_NULL) {
                ((hmac_btcoex_compatibility_set_ps_type_cb)fhook)(hmac_vap);
            }
        }
        return;
    }

    fhook = hmac_get_feature_fhook(HMAC_FHOOK_BTCOEX_COMPATIBILITY_SET_AP_TYPE);
    if (fhook != OSAL_NULL) {
        ((hmac_btcoex_compatibility_set_ap_type_cb)fhook)(hmac_vap, ap_type);
    }

    return;
}

/*****************************************************************************
 函 数 名  : hmac_user_set_lut_info
 功能描述  : DMAC用户添加MAC地址时,更新MAC LUT相关操作
*****************************************************************************/
osal_void hmac_user_set_lut_info(hal_to_dmac_device_stru *hal_device, osal_u8 lut_index, osal_u8 *mac_addr)
{
    /* 控制帧功率由MAC全局寄存器和MAC LUT表两种方式控制。
     * MAC处理逻辑:
     * 1. 发帧时通过MAC地址查找Peer Addr LUT表,
     * 2. MAC在Peer Addr LUT表中找到匹配的MAC Addr,获取对应LUT index
     * 继续查找RSP POWER LUT表,找到其功率值作为反馈控制帧功率,
     * 3. MAC在RSP POWER LUT表中发现对应LUT index的功率值为0则使用全局的Normal档位反馈功率
     * 4. MAC在Peer Addr LUT表未能找到对应的MAC Addr则根据全局的三个档位功率值反馈控制帧功率,
     * 主要用于关联前的帧发送 */
    unref_param(hal_device);
    hal_ce_add_peer_macaddr(lut_index, mac_addr);

    /* 根据以上逻辑,Peer addr lut的操作与rsp power lut表的操作必须同时进行 */
#ifdef _PRE_WLAN_FEATURE_USER_RESP_POWER
    hal_pow_set_user_resp_frame_tx_power(hal_device, lut_index, WLAN_DISTANCE_NORMAL);
#endif
}

/*****************************************************************************
 功能描述  : DMAC用户删除MAC地址时,更新MAC LUT相关操作
*****************************************************************************/
osal_void hmac_user_del_lut_info(osal_u8 lut_index)
{
    /* 控制帧功率由MAC全局寄存器和MAC LUT表两种方式控制。
       MAC处理逻辑:
       1. 发帧时通过MAC地址查找Peer Addr LUT表,
       2. MAC在Peer Addr LUT表中找到匹配的MAC Addr,获取对应LUT index
          继续查找RSP POWER LUT表,找到其功率值作为反馈控制帧功率,
       3. MAC在RSP POWER LUT表中发现对应LUT index的功率值为0则使用全局的Normal档位反馈功率
       4. MAC在Peer Addr LUT表未能找到对应的MAC Addr则根据全局的三个档位功率值反馈控制帧功率,
          主要用于关联前的帧发送 */
    hal_ce_del_peer_macaddr(lut_index);

    /* 根据以上逻辑,Peer addr lut的操作与rsp power lut表的操作必须同时进行 */
#ifdef _PRE_WLAN_FEATURE_USER_RESP_POWER
    hal_pow_del_machw_resp_power_lut_entry(lut_index);
#endif
}

OSAL_STATIC osal_u32 hmac_user_add_prep(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user, osal_s8 rssi)
{
    osal_u32 ret;
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);

    if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_STA) {
        /* 关联时初始化信道切换结构体 */
        (osal_void)memset_s(&(hmac_vap->ch_switch_info), sizeof(mac_ch_switch_info_stru),
            0, sizeof(mac_ch_switch_info_stru));
    }

    /* 重新关联用户的时候，重置乒乓位 */
    hmac_reset_gtk_token(hmac_vap);

    /* dmac user初始化 */
    ret = hmac_user_init(hmac_user);
    if (ret != OAL_SUCC) {
        oam_warning_log2(0, 0, "vap_id[%d] {hmac_user_add_prep::user_init() fail! ret = [%d].}", hmac_vap->vap_id, ret);
    }

    /* add user时，初始化user保存的rssi值 */
    oal_rssi_smooth(&(hmac_user->rx_rssi), rssi);

    /* 初始化节能队列 */
    hmac_psm_user_ps_structure_init(hmac_user);

    /* 初始化linkloss的状态 */
    hmac_vap_linkloss_clean(hmac_vap);

    /* AP类vap启用keepalive定时器, 若定时器已开启, 则不用再开启 */
    if ((hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_AP) && (hmac_device->keepalive_timer.is_registerd == OSAL_FALSE) &&
        (hmac_vap->cap_flag.keepalive == OSAL_TRUE)) {
        frw_create_timer_entry(&(hmac_device->keepalive_timer), hmac_user_keepalive_timer,
            WLAN_AP_KEEPALIVE_TRIGGER_TIME, hmac_device, OSAL_TRUE);
    }

    return OAL_CONTINUE;
}
OSAL_STATIC osal_u32 hmac_user_add_handler(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    mac_ap_type_enum_uint8 ap_type)
{
    osal_u32 ret;
    osal_void *fhook = OAL_PTR_NULL;

    /* active用户，申请lut资源 */
    ret = hmac_user_active(hmac_user);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_CFG,
            "vap_id[%d] {hmac_user_add_handler::user_active failed, free user.}", hmac_user->vap_id);
        return ret;
    }

    /* 本hal device上用户数+1 */
    hal_device_inc_assoc_user_nums(hmac_vap->hal_device);
    /* 清零芯片维护的序列号 */
    hmac_clear_tx_qos_seq_num(hmac_user);

    if (is_legacy_sta(hmac_vap)) {
        /* 带宽切换状态机 */
        hmac_sta_bw_switch_fsm_post_event(hmac_vap, HMAC_STA_BW_SWITCH_EVENT_CHAN_SYNC, 0, OSAL_NULL);
    }

    hmac_compatibility_handler(hmac_vap, ap_type, OSAL_TRUE);

    fhook = hmac_get_feature_fhook(HMAC_FHOOK_BTCOEX_HAL_USER_ADD_HANDLE);
    if (fhook != OSAL_NULL) {
        ((hmac_btcoex_hal_user_add_handle_cb)fhook)(hmac_vap, MAC_USER_TYPE_DEFAULT);
    }

    return OAL_CONTINUE;
}

OSAL_STATIC osal_void hmac_user_add_post(hmac_vap_stru *hmac_vap)
{
#ifdef _PRE_WLAN_FEATURE_MAC_PARSE_TIM
        hmac_vap->sta_pm_handler.max_skip_bcn_cnt = (is_legacy_sta(hmac_vap) == 0) ?
            0 : HMAC_PSM_MAX_SKIP_BCN_CNT;
#endif
}

osal_void hmac_ap_resume_all_user(hmac_vap_stru *hmac_vap)
{
    struct osal_list_head *entry = OSAL_NULL;
    hmac_user_stru *hmac_user = OSAL_NULL;
    osal_u32 ret;

    /* 遍历vap下所有用户,pause tid 队列 */
    osal_list_for_each(entry, &(hmac_vap->mac_user_list_head)) {
        hmac_user = osal_list_entry(entry, hmac_user_stru, user_dlist);
        if (hmac_user == OSAL_NULL) {
            oam_warning_log0(0, OAM_SF_ANY, "{hmac_ap_resume_all_user::user_tmp null.");
            continue;
        }

        /* 恢复tid，并将节能队列的包发出去。 */
        ret = hmac_user_resume(hmac_user);
        if (ret != OAL_SUCC) {
            oam_warning_log2(0, OAM_SF_PWR,
                "vap_id[%d] hmac_ap_resume_all_user:: hmac_user_resume() fail, ret = [%d]", hmac_vap->vap_id, ret);
        }

        /* 如果用户此时不在doze状态，才发包 */
        if (hmac_user->ps_mode != OSAL_TRUE) {
            /* 恢复该vap下的所有用户的硬件队列的发送 */
            hal_tx_disable_peer_sta_ps_ctrl(hmac_user->lut_index);
            /* 将所有的缓存帧发送出去 */
            hmac_psm_queue_flush(hmac_vap, hmac_user);
        }
    }
}
/*****************************************************************************
 函 数 名  : hmac_user_del_offload
 功能描述  : 在DMAC OFFLOAD模式下dmac侧删除user的添加操作
*****************************************************************************/
static osal_u32 hmac_user_del_offload(hmac_vap_stru *hmac_vap)
{
    if (hmac_vap == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_user_del_offload::hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* STA模式下将关联的VAP的id置为非法值 */
    if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_STA) {
        hmac_vap_set_assoc_id_etc(hmac_vap, MAC_INVALID_USER_ID);

#ifdef _PRE_WLAN_FEATURE_TXOPPS
        hmac_txopps_set_machw(hmac_vap);
        /* 去关联时MAC partial aid配置为0 */
        hmac_txopps_set_machw_partialaid(hmac_vap, 0);
#endif
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_psm_overrun_throw_half
 功能描述  : 如果用户连续5个dtim周期没有取包，并且系统内存不足，则将属于该用户的包丢掉一半。
             先tid队列的，再丢节能队列的。
*****************************************************************************/
static osal_u32 hmac_psm_overrun_throw_half(hmac_user_stru *hmac_user, osal_u32 mpdu_delete_num)
{
    osal_u32 tid_mpdu_num;   /* tid队列中包的数目，包括重传队列的 */
    osal_u32 psm_mpdu_num;   /* 节能队列中包的数目 */
    osal_u32 psm_delete_num; /* 节能队列需要删除的mpdu数目 */
    osal_u8 tid_idx;
    osal_u32 ret;

    hmac_tid_stru *tid_queue = OSAL_NULL;

    /* 得到用户当前tid队列中的包 */
    tid_mpdu_num = hmac_psm_tid_mpdu_num(hmac_user);
    psm_mpdu_num = (osal_u32)osal_adapt_atomic_read(&hmac_user->ps_structure.mpdu_num);

    /* 先删tid队列的 */
    for (tid_idx = 0; tid_idx < WLAN_TID_MAX_NUM; tid_idx++) {
        tid_queue = &(hmac_user->tx_tid_queue[tid_idx]);
        ret = hmac_tid_delete_mpdu_head(tid_queue, hmac_user->tx_tid_queue[tid_idx].mpdu_num);
        if (osal_unlikely(ret != OAL_SUCC)) {
            oam_warning_log1(0, OAM_SF_ANY, "{hmac_psm_overrun_throw_half::tid[%d] throw mpdu failed !}", tid_idx);
            return ret;
        }
    }
    /* 如果tid队列中的mpdu数目小于需要删除的mpdu数目，再删节能队列的 */
    if (tid_mpdu_num < mpdu_delete_num) {
        psm_delete_num = mpdu_delete_num - tid_mpdu_num;
        if (psm_delete_num <= psm_mpdu_num) {
            hmac_psm_delete_ps_queue_head(hmac_user, psm_delete_num);
        }
    }
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_multi_user_ps_queue_overrun_notify
 功能描述  : 遍历用户，如果连续5个dtim周期用户节能队列中都有包，且此时包的数目大于128，
             返回TRUE，否则，返回FALSE。
*****************************************************************************/
osal_void hmac_user_ps_queue_overrun_notify(const hmac_vap_stru *hmac_vap)
{
    hmac_user_stru *hmac_user = OSAL_NULL;
    struct osal_list_head *entry = OSAL_NULL;
    osal_u32 ps_mpdu_num;
    osal_u32 mpdu_num_sum;

    /* 遍历VAP下所有USER */
    osal_list_for_each(entry, &hmac_vap->mac_user_list_head) {
        hmac_user = osal_list_entry(entry, hmac_user_stru, user_dlist);
        if (osal_unlikely(hmac_user == OSAL_NULL)) {
            oam_error_log0(0, OAM_SF_CFG, "{hmac_multi_user_ps_queue_overrun_notify::null pointer.}");
            continue;
        }

        /* 用户处于节能状态，且节能队列中有包时，节能计数器加1；如果节能队列为空，则计数器清零。 */
        if (hmac_user->ps_mode != OSAL_TRUE) {
            continue;
        }
        if (hmac_psm_is_psm_empty(hmac_user) == OSAL_FALSE) {
            hmac_user->ps_structure.ps_time_count++;
        } else {
            hmac_user->ps_structure.ps_time_count = 0;
            continue;
        }

        /* 如果连续5次节能队列有包，且此节能队列中包的数目大于门限，          避免内存耗尽，丢包                            */
        ps_mpdu_num = (osal_u32)osal_adapt_atomic_read(&hmac_user->ps_structure.mpdu_num);
        mpdu_num_sum = ps_mpdu_num + hmac_psm_tid_mpdu_num(hmac_user);
        if (hmac_user->ps_structure.ps_time_count >= 5 && /* 5 : 连续5次有包 */
            ps_mpdu_num > (MAX_MPDU_NUM_IN_PS_QUEUE / hmac_vap->user_nums)) {
            /* 丢包时先丢tid队列的，再丢节能队列的 */
            oam_warning_log2(0, OAM_SF_ANY,
                "{hmac_user_ps_queue_overrun_notify::PS mpdu num[%d], TID mpdu num[%d]!}", ps_mpdu_num,
                hmac_psm_tid_mpdu_num(hmac_user));
            hmac_psm_overrun_throw_half(hmac_user, mpdu_num_sum / 2); /* 2 : 将属于该用户的包丢掉一半 */
            hmac_user->ps_structure.ps_time_count = 0;
        }
    }
}

static osal_void hmac_user_add_notify_alg_proc(hmac_vap_stru *hmac_vap, const frw_msg *msg,
    hmac_user_stru *hmac_user)
{
#ifdef _PRE_WLAN_FEATURE_11AX
    hmac_ctx_add_user_stru *add_user_payload = (hmac_ctx_add_user_stru *)msg->data;
#endif
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);
    hal_to_dmac_device_stru *hal_device = hmac_vap->hal_device;
    mac_channel_stru channel;
    osal_void *fhook = NULL;

#ifdef _PRE_WLAN_FEATURE_11AX
    (osal_void)memcpy_s(&hmac_user->he_hdl, sizeof(mac_he_hdl_stru), &add_user_payload->he_hdl,
        sizeof(mac_he_hdl_stru));
#else
    unref_param(msg);
#endif

    /* 变量初始化为当前vap信息 */
    channel = hmac_vap->channel;

    /* 选择需要设置的信道信息 */
    hmac_chan_select_real_channel(hmac_device, &channel, channel.chan_number);

    oam_warning_log4(0, OAM_SF_UM,
        "{hmac_user_add_notify_alg_proc:vap bw[%d], new set bandwith[%d], hal_device bandwidth=[%d], dbac_running=[%d]",
        hmac_vap->channel.en_bandwidth, channel.en_bandwidth, hal_device->wifi_channel_status.en_bandwidth,
        hmac_device->dbac_running);

    /* 带宽不一致时才需要刷新硬件带宽 */
    if ((mac_is_dbac_running(hmac_device) == OSAL_FALSE) &&
        (channel.en_bandwidth != hal_device->wifi_channel_status.en_bandwidth)) {
        /* 切换信道不需要清fifo，传入FALSE */
        hmac_mgmt_switch_channel(hal_device, &channel, OSAL_FALSE);
    }

    /* 需要在此处通知，才能是已经完成了up dbdc和dbac过程running判断 */
    fhook = hmac_get_feature_fhook(HMAC_FHOOK_BTCOEX_ASSOC_STATE_SYN);
    if (fhook != OSAL_NULL) {
        ((hmac_config_btcoex_assoc_state_syn_cb)fhook)(hmac_vap, hmac_user);
    }
}

/*****************************************************************************
 函 数 名  : hmac_user_add_notify_alg
 功能描述  : 通知算法增加user
*****************************************************************************/
osal_s32 hmac_user_add_notify_alg(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user, frw_msg *msg)
{
    hal_to_dmac_device_stru *hal_device = OSAL_NULL;
    hmac_ctx_add_user_stru *add_user_payload = OSAL_NULL;
    osal_s32 ret;

    if ((hmac_vap == OSAL_NULL) || (msg == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_user_add_notify_alg::null param.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    hal_device = hmac_vap->hal_device;
    if (hal_device == OSAL_NULL) {
        oam_warning_log1(0, OAM_SF_UM, "vap_id[%d] {hmac_user_add_notify_alg::HAL_DEVICE null}", hmac_vap->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    add_user_payload = (hmac_ctx_add_user_stru *)msg->data;
    hmac_user_set_assoc_id_etc(hmac_user, add_user_payload->user_idx);

#ifdef _PRE_WLAN_FEATURE_TXOPPS
    mac_vap_update_txopps(hmac_vap, hmac_user);
#endif

    /* 先通知dbac判断是否需要迁移,后续各个算法再注册 */
    if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_STA) {
        hmac_alg_vap_up_notify(hmac_vap);
    }

    ret = (osal_s32)hmac_alg_add_assoc_user_notify(hmac_vap, hmac_user);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, 0, "{hmac_user_add_notify_alg::user_notify fail[%d].", ret);
    }

    hmac_user_add_notify_alg_proc(hmac_vap, msg, hmac_user);
    /* 抛事件至Device侧DMAC，同步添加user下挂算法结构体 */
    ret = frw_send_msg_to_device(hmac_vap->vap_id, WLAN_MSG_H2D_C_CFG_DEVICE_NOTIFY_ALG_ADD_USER, msg, OSAL_TRUE);
    if (ret != OAL_SUCC) {
        oam_error_log2(0, OAM_SF_CFG, "vap_id[%d] {user_add_notify_alg::fail [%d].}", hmac_vap->vap_id, ret);
        return ret;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_alg_stat_info_reset
 功能描述  : 当DMAC删除一个关联用户时，删除用户相关统计信息。
*****************************************************************************/
static osal_void hmac_alg_stat_info_reset(hmac_vap_stru *hmac_vap)
{
    hal_to_dmac_device_stru *hal_device = OSAL_NULL;

    if (osal_unlikely(hmac_vap == OSAL_NULL)) {
        return;
    }

    hal_device = hmac_vap->hal_device;
    if (osal_unlikely(hal_device == OSAL_NULL)) {
        return;
    }

    hal_device->hal_alg_stat.adj_intf_state = HAL_ALG_INTF_DET_ADJINTF_NO;
    hal_device->hal_alg_stat.co_intf_state = OSAL_FALSE;
    hal_device->hal_alg_stat.alg_distance_stat = HAL_ALG_USER_DISTANCE_NORMAL;
}

static osal_void hmac_user_del_ext_sta(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user)
{
#ifdef _PRE_WLAN_FEATURE_P2P
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);
#endif
    mac_cfg_ps_open_stru ps_cfg = {MAC_STA_PM_CTRL_TYPE_HOST, MAC_STA_PM_SWITCH_RESET};

#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST
    hmac_scan_destroy_obss_timer(hmac_vap);
#endif

    hmac_vap_set_aid_etc(hmac_vap, 0);
    /* 去关联后需要关闭协议低功耗否则会在下次重新关联时还会获取dhcp前进入低功耗模式--发睡眠null帧 */
    hmac_set_sta_pm_on_cb(hmac_vap, &ps_cfg);
#ifdef _PRE_WLAN_FEATURE_MAC_PARSE_TIM
    /* STA去关联时清空aid寄存器 */
    hal_set_mac_aid(hmac_vap->hal_vap, 0);
#endif

    /* 删除STA用户时清除bss color设置 */
    if (is_legacy_sta(hmac_vap)) {
        hal_set_bss_color(hmac_vap->hal_vap, 0);
    }

    /* 恢复STA为无保护状态 */
    hmac_set_protection_mode(hmac_vap, hmac_user, WLAN_PROT_NO);

#ifdef _PRE_WLAN_FEATURE_P2P
    /* Begin:P2P CLIENT 删除用户，表示此STA去关联，
        停止p2p client 时，清空P2P 节能参数，停止P2P 节能寄存器 */
    if (is_p2p_cl(hmac_vap)) {
        hal_vap_set_noa(hmac_vap->hal_vap, 0, 0, 0, 0);
        hal_vap_set_ops(hmac_vap->hal_vap, 0, 0);
        (osal_void)memset_s(&(hmac_vap->p2p_ops_param), sizeof(mac_cfg_p2p_ops_param_stru),
            0, sizeof(mac_cfg_p2p_ops_param_stru));
        (osal_void)memset_s(&(hmac_vap->p2p_noa_param), sizeof(mac_cfg_p2p_noa_param_stru),
            0, sizeof(mac_cfg_p2p_noa_param_stru));
        /* 删除vap时清空P2P节能状态 */
        hmac_device->st_p2p_info.p2p_ps_pause = OSAL_FALSE;
        hmac_device_sync(hmac_device);
    }
    /* End:P2P CLIENT 删除用户，表示此STA去关联，
        停止p2p client 时，清空P2P 节能参数，停止P2P 节能寄存器 */
#endif
}

static osal_void hmac_user_del_ext(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user)
{
    /* 如果是STA删除用户，表示此STA去关联了，调用vap down通知链 */
    if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_STA) {
        hmac_user_del_ext_sta(hmac_vap, hmac_user);
    }

    /* 删除用户节能结构,清除vap保存的该用户的tim_bitmap信息 */
    if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_AP) {
        hmac_psm_set_local_bitmap(hmac_vap, hmac_user, 0);

        if (hmac_user->ps_mode == OSAL_TRUE) {
            hmac_vap->ps_user_num--;
        }

#ifdef _PRE_WLAN_FEATURE_WUR_TX
        hmac_wur_user_del_handle(hmac_vap, hmac_user);
#endif
    }

    hmac_psm_user_ps_structure_destroy(hmac_user);
}

static osal_void hmac_user_del_reset_settings(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    mac_ap_type_enum_uint8 ap_type)
{
#ifdef _PRE_WLAN_FEATURE_M2S
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);
#endif

    /* hmac_user_inactive会置user的lut index为无效值，对lut的操作需要在该函数之前执行 */
    hal_tx_disable_peer_sta_ps_ctrl(hmac_user->lut_index);
    /* 用户去关联，需要清ps配置，做保护  */
    hal_tx_disable_resp_ps_bit_ctrl(hmac_user->lut_index);

    /* dmac user相关操作去注册 */
    hmac_alg_del_assoc_user_notify(hmac_vap, hmac_user);

    /* inactive，lut资源置为非法 */
    if (hmac_user_inactive(hmac_user) != OAL_SUCC) {
        oam_warning_log1(0, 0, "vap_id[%d] {hmac_user_del_reset_settings::user_inactive fail!", hmac_vap->vap_id);
    }

    /* 如果VAP模式是STA，则需要恢复STA寄存器到初始状态 */
    if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_STA) {
        hmac_vap_sta_reset(hmac_vap);
#ifdef _PRE_WLAN_FEATURE_M2S
        /* END:STA 去关联时，恢复硬件发送，
           避免由于MAC PA 寄存器发送挂起导致不能继续关联 */
        /* sta去关联时，要vap能力回到mimo默认最大能力 */
        if (mac_vap_spec_is_sw_need_m2s_switch(hmac_vap)) {
            hmac_m2s_spec_update_vap_sw_capbility(hmac_device, hmac_vap, WLAN_DOUBLE_NSS);
        }
#endif
    }

    if (hmac_user_del_offload(hmac_vap) != OAL_SUCC) {
        oam_warning_log1(0, 0, "vap_id[%d] {hmac_user_del_reset_settings::del_offload fail!}", hmac_vap->vap_id);
    }

    /* 删除用户，更新hal device下用户数统计，做-- */
    hal_device_dec_assoc_user_nums(hmac_vap->hal_device);

    frw_util_notifier_notify(WLAN_UTIL_NOTIFIER_EVENT_DEL_USER_RESET, hmac_vap);

#ifdef _PRE_WLAN_FEATURE_M2S
    hmac_config_m2s_disassoc_state_syn(hmac_vap);
#endif

    hmac_compatibility_handler(hmac_vap, ap_type, OSAL_FALSE);
    oam_warning_log1(0, OAM_SF_UM, "vap_id[%d] {hmac_user_del_reset_settings::Del user SUCC!", hmac_vap->vap_id);

    /* 将dmac_vap 中的速率信息设为初始值 */
    if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_STA) {
        hmac_vap_init_rate_info(&hmac_vap->vap_curr_rate);
        hmac_vap_init_tx_frame_params(hmac_vap, OSAL_TRUE);
    }
}

/*****************************************************************************
 函 数 名  : hmac_user_del
 功能描述  : 删除用户
*****************************************************************************/
osal_void hmac_user_del(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user, mac_ap_type_enum_uint8 ap_type)
{
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);

#ifdef _PRE_WLAN_FEATURE_PMF
    /* AP 侧硬件PMF控制开关填写 */
    hmac_11w_update_users_status(hmac_vap, hmac_user, OSAL_FALSE);
#endif
    if (hmac_user->ch_text != OSAL_NULL) {
        oal_mem_free(hmac_user->ch_text, OAL_TRUE);
        hmac_user->ch_text = OSAL_NULL;
    }

    /* 删除tid队列中的所有信息 */
    hmac_tid_clear(hmac_user, hmac_device, OSAL_TRUE);

    /* 删除用户时，初始化distance信息和cca门限 */
    hmac_alg_stat_info_reset(hmac_vap);
    if (is_legacy_sta(hmac_vap)) {
        /* 带宽切换状态机 */
        hmac_sta_bw_switch_fsm_post_event(hmac_vap, HMAC_STA_BW_SWITCH_EVENT_USER_DEL, 0, OSAL_NULL);
#ifdef _PRE_WLAN_FEATURE_11AX
        /* 标志vap工作在11ax */
        if (mac_mib_get_he_option_implemented(hmac_vap) == OSAL_TRUE && (hmac_user->he_hdl.he_capable == OSAL_TRUE)) {
            hal_set_dev_support_11ax(hmac_vap->hal_device, OSAL_FALSE);
            hal_set_phy_aid(0);
        }
#endif
    }
    hmac_user_del_ext(hmac_vap, hmac_user);
    hmac_user_del_reset_settings(hmac_vap, hmac_user, ap_type);
}

/*****************************************************************************
 函 数 名  : hmac_user_get_tid_by_num
 功能描述  : 根据tid num获取dmac user下的tid队列指针
*****************************************************************************/
WIFI_TCM_TEXT osal_u32 hmac_user_get_tid_by_num(hmac_user_stru *hmac_user, osal_u8 tid_num, hmac_tid_stru **tid_queue)
{
    *tid_queue = &(hmac_user->tx_tid_queue[tid_num]);

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_user_get_ps_mode
 功能描述  : 读取用户的ps_mode

*****************************************************************************/
WIFI_TCM_TEXT oal_bool_enum_uint8 hmac_user_get_ps_mode(const hmac_user_stru  *hmac_user)
{
    if (osal_unlikely(hmac_user == OSAL_NULL)) {
        return OSAL_FALSE;
    }

    return (oal_bool_enum_uint8)(hmac_user->ps_mode);
}

/*****************************************************************************
 功能描述  : 根据用户状态设置slottime类型
*****************************************************************************/
osal_void hmac_user_init_slottime(const hmac_user_stru *hmac_user)
{
    if (hmac_user->cur_protocol_mode == WLAN_LEGACY_11B_MODE) {
        hal_cfg_slottime_type(1);
    } else {
        hal_cfg_slottime_type(0);
    }
}

/*****************************************************************************
 函 数 名  : hmac_psm_tid_mpdu_num
 功能描述  : 判断某一个用户的所有tid是否都为空，只要有一个tid不为空就返回false
*****************************************************************************/
osal_u32 hmac_psm_tid_mpdu_num(const hmac_user_stru  *hmac_user)
{
    osal_u8                     tid_idx;
    osal_u32                    tid_mpdu_num    = 0;

    if (osal_unlikely(hmac_user == OSAL_NULL)) {
        return 0;
    }

    for (tid_idx = 0; tid_idx < WLAN_TID_MAX_NUM; tid_idx++) {
        tid_mpdu_num += hmac_user->tx_tid_queue[tid_idx].mpdu_num;
    }

    return tid_mpdu_num;
}
/*****************************************************************************
 函 数 名  : hmac_psm_is_psm_empty
 功能描述  : 判断某一个用户的psm是否都为空
*****************************************************************************/
oal_bool_enum_uint8 hmac_psm_is_psm_empty(hmac_user_stru *hmac_user)
{
    return (osal_adapt_atomic_read(&hmac_user->ps_structure.mpdu_num) == 0);
}

/*****************************************************************************
 函 数 名  : hmac_psm_is_tid_empty
 功能描述  : 判断某一个用户的所有tid是否都为空，只要有一个tid不为空就返回false
*****************************************************************************/
oal_bool_enum_uint8 hmac_psm_is_tid_empty(const hmac_user_stru *hmac_user)
{
    return (hmac_psm_tid_mpdu_num(hmac_user) == 0);
}

/*****************************************************************************
 功能描述  : 暂停用户所有TID
*****************************************************************************/
osal_void hmac_user_pause(hmac_user_stru *hmac_user)
{
    osal_u8       tid_idx;
    hmac_tid_stru  *tid = OSAL_NULL;

    if (hmac_user == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_user_pause::hmac_user null.}");
        return;
    }
    if (hmac_user->user_asoc_state != MAC_USER_STATE_ASSOC) {
        return;
    }

    for (tid_idx = 0; tid_idx < WLAN_TID_MAX_NUM; tid_idx++) {
        tid = &(hmac_user->tx_tid_queue[tid_idx]);
        hmac_tid_pause(tid, DMAC_TID_PAUSE_RESUME_TYPE_PS);
    }
}

/*****************************************************************************
 函 数 名  : hmac_ap_pause_all_user
 功能描述  : 暂停该vap下的所有用户的硬件队列的发送
*****************************************************************************/
osal_void hmac_ap_pause_all_user(const hmac_vap_stru *hmac_vap)
{
    struct osal_list_head                 *entry = OSAL_NULL;
    hmac_user_stru                       *hmac_user = OSAL_NULL;

    /* 遍历vap下所有用户,pause tid 队列 */
    osal_list_for_each(entry, &(hmac_vap->mac_user_list_head)) {
        hmac_user = osal_list_entry(entry, hmac_user_stru, user_dlist);
        if (hmac_user == OSAL_NULL) {
            oam_warning_log0(0, OAM_SF_ANY, "{hmac_ap_pause_all_user::user_tmp null.");
            continue;
        }

        /* pause tid */
        hmac_user_pause(hmac_user);
        /* 暂停该vap下的所有用户的硬件队列的发送, 硬件上报psm_back,软件再回收 */
        hal_tx_enable_peer_sta_ps_ctrl(hmac_user->lut_index);
    }
}

/*****************************************************************************
 函 数 名  : hmac_user_sync
 功能描述  : 同步用户信息至 device侧
*****************************************************************************/
osal_s32 hmac_user_sync(hmac_user_stru *hmac_user)
{
    frw_msg msg_info = {0};
    mac_user_info_h2d_sync_stru user_info_sync = {0};
    hmac_vap_stru *hmac_vap = OSAL_NULL;
    osal_s32 ret;

    /* 同步dmac_user_stru */
    user_info_sync.assoc_id = hmac_user->assoc_id;
    user_info_sync.lut_index = hmac_user->lut_index;
    user_info_sync.is_multi_user = hmac_user->is_multi_user;
    user_info_sync.cur_protocol_mode = hmac_user->cur_protocol_mode;
    user_info_sync.cipher_type = hmac_user->key_info.cipher_type;

    user_info_sync.avail_protocol_mode  = hmac_user->avail_protocol_mode;
    user_info_sync.avail_bandwidth  = hmac_user->avail_bandwidth;
    user_info_sync.cur_bandwidth = hmac_user->cur_bandwidth;
    user_info_sync.avail_num_spatial_stream = hmac_user->avail_num_spatial_stream;
    user_info_sync.user_num_spatial_stream = hmac_user->user_num_spatial_stream;
    user_info_sync.avail_bf_num_spatial_stream = hmac_user->avail_bf_num_spatial_stream;
    user_info_sync.vip_flag = hmac_user->vip_flag;
    user_info_sync.user_asoc_state = hmac_user->user_asoc_state;
    user_info_sync.qos = hmac_user->cap_info.qos;

    /* 同步dmac_user MAC地址 */
    (osal_void)memcpy_s((osal_void *)(user_info_sync.user_mac_addr), WLAN_MAC_ADDR_LEN,
        (osal_void *)(hmac_user->user_mac_addr), WLAN_MAC_ADDR_LEN);

    /* 同步发送相关参数 */
    (osal_void)memcpy_s(&(user_info_sync.user_tx_info), sizeof(mac_user_tx_param_stru),
        &(hmac_user->user_tx_info), sizeof(mac_user_tx_param_stru));

    /* 同步密钥信息 */
    (osal_void)memcpy_s(&(user_info_sync.key_info), sizeof(mac_key_mgmt_stru), &(hmac_user->key_info),
        sizeof(mac_key_mgmt_stru));

    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(hmac_user->vap_id);
    if (hmac_vap == OSAL_NULL) {
        oam_error_log1(0, OAM_SF_TX, "vap_id[%d] {hmac_user_sync::hmac_vap null.}", hmac_user->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 抛事件至Device侧DMAC，同步dmac_user_stru */
    frw_msg_init((osal_u8 *)&user_info_sync, sizeof(user_info_sync), OSAL_NULL, 0, &msg_info);
    ret = frw_send_msg_to_device(hmac_vap->vap_id, WLAN_MSG_H2D_C_CFG_DEVICE_SYNC_USER, &msg_info, OSAL_TRUE);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_CFG, "{hmac_user_sync::sync msg fail[%d].}", ret);
    }
    return ret;
}

WIFI_HMAC_TCM_TEXT WIFI_TCM_TEXT osal_void hmac_user_use_cnt_dec(hmac_user_stru *hmac_user)
{
    if (hmac_user == OSAL_NULL) {
        return;
    }
    mac_vap_intrrupt_disable();

    /* 引用计数为0，用户正在删除，没有引用自加，不做自减操作 */
    if (osal_adapt_atomic_read(&(hmac_user->use_cnt)) == 0) {
        mac_vap_intrrupt_enable();
        return;
    }

    osal_adapt_atomic_dec(&(hmac_user->use_cnt));
    mac_vap_intrrupt_enable();
}

WIFI_HMAC_TCM_TEXT WIFI_TCM_TEXT osal_void hmac_user_use_cnt_inc(osal_u16 user_idx, hmac_user_stru **user)
{
    hmac_user_stru *hmac_user = OSAL_NULL;
    mac_vap_intrrupt_disable();
    /* 引用计数自加通过外部接口获取user，避免用户已经删除后获取到user */
    hmac_user = (hmac_user_stru *)mac_res_get_hmac_user_etc(user_idx);
    *user = hmac_user;
    if (hmac_user == OSAL_NULL) {
        mac_vap_intrrupt_enable();
        return;
    }
    osal_adapt_atomic_inc(&(hmac_user->use_cnt));
    mac_vap_intrrupt_enable();
}

/*****************************************************************************
 函 数 名  : hmac_config_set_max_user_etc
 功能描述  : 设置最大用户数
 输入参数  : event_hdr: 事件头
             len       : 参数长度
             param    : 参数
 输出参数  : 无
*****************************************************************************/
osal_u32  hmac_config_set_max_user_etc(hmac_vap_stru *hmac_vap, osal_u16 len, osal_u32 max_user)
{
    unref_param(len);
    /* P2P GO最大用户数不能超过p2p限制，普通模式不能超过芯片最大用户数约束 */
    if (((is_p2p_go(hmac_vap) == OSAL_TRUE) && (max_user > WLAN_P2P_GO_ASSOC_USER_MAX_NUM_SPEC)) ||
        (max_user > mac_chip_get_max_asoc_user(hmac_vap->chip_id))) {
        oam_error_log3(0, OAM_SF_CFG,
            "vap_id[%d] {hmac_config_set_max_user_etc:: AP/GO want to set value[%d] more than the threshold[%d]!}",
            hmac_vap->vap_id, max_user, (is_p2p_go(hmac_vap) ? WLAN_P2P_GO_ASSOC_USER_MAX_NUM_SPEC :
            mac_chip_get_max_asoc_user(hmac_vap->chip_id)));
        return OAL_FAIL;
    }

    mac_mib_set_MaxAssocUserNums(hmac_vap, (osal_u16)max_user);

    oam_warning_log3(0, OAM_SF_CFG,
        "vap_id[%d] {hmac_config_set_max_user_etc::chip user_num_max[%d], user_nums_max[%d].}",
        hmac_vap->vap_id,
        mac_chip_get_max_asoc_user(hmac_vap->chip_id), mac_mib_get_MaxAssocUserNums(hmac_vap));

    return OAL_SUCC;
}

osal_s32 hmac_config_set_max_user_cb(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    osal_u32 max_user;
    max_user = *((osal_u32 *)msg->data);
    return (osal_s32)hmac_config_set_max_user_etc(hmac_vap, msg->data_len, max_user);
}

#ifdef _PRE_WLAN_CFGID_DEBUG
OAL_STATIC osal_void hmac_config_user_info_printf(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user)
{
    osal_u8 tid_index;
    wifi_printf("user_asoc_state :  %d \n", hmac_user->user_asoc_state);
    wifi_printf("is_wds :           %d \n", hmac_user->is_wds);
    wifi_printf("amsdu_maxsize :    %d \n", hmac_user->amsdu_maxsize);
    wifi_printf("11ac2g :           %d \n", hmac_user->hmac_cap_info.support_11ac2g);
    wifi_printf("\n");

    for (tid_index = 0; tid_index < WLAN_TIDNO_BUTT; tid_index++) {
        wifi_printf("tid            %d \n", tid_index);
        wifi_printf("amsdu_maxnum : %d \n", hmac_user->hmac_amsdu[tid_index].amsdu_maxnum);
        wifi_printf("amsdu_size :   %d \n", hmac_user->hmac_amsdu[tid_index].amsdu_size);
        wifi_printf("msdu_num :     %d \n", hmac_user->hmac_amsdu[tid_index].msdu_num);
        wifi_printf("\n");
    }

    wifi_printf("us_assoc_id :      %d \n", hmac_user->assoc_id);
    wifi_printf("vap_id :           %d \n", hmac_user->vap_id);
    wifi_printf("device_id :        %d \n", hmac_user->device_id);
    wifi_printf("chip_id :          %d \n", hmac_user->chip_id);
    wifi_printf("amsdu_supported :  %d \n", hmac_user->amsdu_supported);
    wifi_printf("htc_support :      %d \n", hmac_user->ht_hdl.htc_support);
    wifi_printf("en_ht_support :    %d \n", hmac_user->ht_hdl.ht_capable);
    wifi_printf("short gi 20 40 80: %d %d %d \n", hmac_user->ht_hdl.short_gi_20mhz,
        hmac_user->ht_hdl.short_gi_40mhz,
        hmac_user->vht_hdl.short_gi_80mhz);
    wifi_printf("\n");

    wifi_printf("Privacy info : \r\n");
    wifi_printf("    port_valid   :                     %d \r\n", hmac_user->port_valid);
    wifi_printf("    user_tx_info.security.cipher_key_type:      %s \r\n"
        "    user_tx_info.security.cipher_protocol_type: %s \r\n",
        hmac_config_keytype2string_etc(hmac_user->user_tx_info.security.cipher_key_type),
        hmac_config_cipher2string_etc(hmac_user->key_info.cipher_type));

    if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_AP) {
        wifi_printf("    STA:cipher_type :                           %s \r\n",
            hmac_config_cipher2string_etc(hmac_user->key_info.cipher_type));
    }
    wifi_printf("\n");

    wifi_printf("Rssi info: \n");
    wifi_printf("rssi :              %d \n", oal_get_real_rssi(hmac_user->rx_rssi));
    wifi_printf("free_power  :       %d \n", hmac_config_get_free_power(hmac_vap));
    wifi_printf("\n");
    return;
}

/*****************************************************************************
 函 数 名  : hmac_config_user_info_etc
 功能描述  : 打印user信息
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
osal_s32  hmac_config_user_info_etc(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    hmac_user_stru *hmac_user = OAL_PTR_NULL;
    mac_cfg_user_info_param_stru *hmac_event = OAL_PTR_NULL;

    hmac_event = (mac_cfg_user_info_param_stru *)msg->data;
    hmac_user = (hmac_user_stru *)mac_res_get_hmac_user_etc(hmac_event->user_idx);
    if (hmac_user == OAL_PTR_NULL) {
        oam_warning_log2(0, OAM_SF_CFG, "vap_id[%d] {hmac_config_user_info_etc::hmac_user[%d] null.}",
                         hmac_vap->vap_id, hmac_event->user_idx);
        return OAL_FAIL;
    }

    hmac_config_user_info_printf(hmac_vap, hmac_user);

    oam_warning_log3(0, OAM_SF_KEEPALIVE,
        "vap_id[%d] {hmac_config_user_info::assoc_id is %d, last_active_timestamp[%llu]}", hmac_vap->vap_id,
        hmac_user->assoc_id, hmac_user->last_active_timestamp);

    return OAL_SUCC;
}
#endif

osal_s32 hmac_config_set_user_app_ie_cb(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    oal_app_ie_stru *user_app_ie = OSAL_NULL;

    if ((hmac_vap == OSAL_NULL) || (msg->data == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_CFG, "{hmac_config_set_user_app_ie_cb::null param.}");
        return OAL_FAIL;
    }

    user_app_ie = (oal_app_ie_stru *)msg->data;

    return (osal_s32)hmac_config_set_app_ie_to_vap_etc(hmac_vap, user_app_ie, user_app_ie->app_ie_type);
}

/*****************************************************************************
 函 数 名  : hmac_config_user_rate_info_syn_etc
 功能描述  : hmac_offload架构下同步user速率信息到device侧
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
osal_u32  hmac_config_user_rate_info_syn_etc(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user)
{
    osal_u32                  ret;
    mac_h2d_usr_rate_info_stru  rate_info;
    frw_msg msg_info;
    (osal_void)memset_s(&msg_info, OAL_SIZEOF(msg_info), 0, OAL_SIZEOF(msg_info));

    rate_info.user_idx   = hmac_user->assoc_id;

    /* legacy速率集信息，同步到dmac */
    rate_info.avail_rs_nrates = hmac_user->avail_op_rates.rs_nrates;
    if (memcpy_s(rate_info.avail_rs_rates, sizeof(rate_info.avail_rs_rates), hmac_user->avail_op_rates.rs_rates,
        WLAN_MAX_SUPP_RATES) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_config_user_rate_info_syn_etc::memcpy_s error}");
    }

    /* ht速率集信息，同步到dmac */
    hmac_user_get_ht_hdl_etc(hmac_user, &rate_info.ht_hdl);

    /* vht速率集信息，同步到dmac */
    hmac_user_get_vht_hdl_etc(hmac_user, &rate_info.vht_hdl);

#ifdef _PRE_WLAN_FEATURE_11AX
    /* he速率集信息，同步到dmac */
    mac_user_get_he_hdl(hmac_user, &(rate_info.he_hdl));
    /* 标志vap工作在11ax */
    if (is_legacy_sta(hmac_vap)) {
        if ((mac_mib_get_he_option_implemented(hmac_vap) == OSAL_TRUE) && (rate_info.he_hdl.he_capable == OSAL_TRUE)) {
            hal_set_dev_support_11ax(hmac_vap->hal_device, OSAL_TRUE);
        }
        if (rate_info.he_hdl.bss_color_exist == 1) {
            hal_set_bss_color(hmac_vap->hal_vap, rate_info.he_hdl.bss_color);
        }
    }
#endif
    rate_info.amsdu_maxsize = hmac_user->amsdu_maxsize;

#ifdef _PRE_WLAN_FEATURE_TXOPPS
    /* 信息同步后续处理 */
    mac_vap_update_txopps(hmac_vap, hmac_user);
#endif

    msg_info.data = (osal_u8 *)(&rate_info);
    msg_info.data_len = sizeof(mac_h2d_usr_rate_info_stru);
    /* 先抛事件至Device侧DMAC，同步dmac_user速率信息，若Device侧同步失败，则不执行ko侧同步 */
    ret = (osal_u32)frw_send_msg_to_device(hmac_vap->vap_id, WLAN_MSG_H2D_C_CFG_DEVICE_USER_RATE_SYN, &msg_info,
        OSAL_TRUE);
    if (ret != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_TX, "vap_id[%d] {hmac_config_user_rate_info_syn::frw_send_msg_to_device failed[%d]}",
            hmac_vap->vap_id, ret);
    }

    return ret;
}

/*****************************************************************************
 函 数 名  : hmac_config_usr_info_syn
 功能描述  : hmac_offload架构下同步sta usr的状态到dmac
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
osal_u32  hmac_config_user_info_syn_etc(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user)
{
    /* 更新mac地址，漫游时mac会更新 */
    if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_STA) {
        (osal_void)memcpy_s((osal_void *)(hmac_user->user_mac_addr), WLAN_MAC_ADDR_LEN,
            (osal_void *)(hmac_vap->bssid), WLAN_MAC_ADDR_LEN);
    }

#ifdef _PRE_WLAN_FEATURE_PMF
    /* 同步user pmf的能力 */
    hmac_11w_update_users_status(hmac_vap, hmac_user, hmac_user->cap_info.pmf_active);
#endif

#ifdef _PRE_WLAN_FEATURE_ANT_SWITCH
    /* 目前更改协议模式只有这个入口，若之后添加其他入口，需要再更新门限值 */
    if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_STA) {
        if (hmac_vap->hal_device == OSAL_NULL) {
            oam_error_log1(0, 0, "vap_id[%d] {hmac_config_sta_usr_info_syn::hal_dev is NULL.}", hmac_vap->vap_id);
        } else {
            hmac_calc_rssi_th(hmac_user, &(hmac_vap->hal_device->rssi));
        }
    }
#endif

    if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_STA) {
        /* 初始化slottime */
        hmac_user_init_slottime(hmac_user);
        hmac_sta_set_sec_bandwidth_offset(hmac_vap, hmac_user);
    }

    hmac_user_sync(hmac_user);

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_init_user_security_port_etc
 功能描述  : 初始化用户的加密端口标志
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
osal_u32 hmac_init_user_security_port_etc(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user)
{
    hmac_vap_init_user_security_port_etc(hmac_vap, hmac_user);

    return OAL_SUCC;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
