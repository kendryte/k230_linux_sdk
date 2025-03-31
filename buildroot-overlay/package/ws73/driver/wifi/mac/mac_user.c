/*
 * Copyright (c) CompanyNameMagicTag 2020-2023. All rights reserved.
 * Description: Source file of mac user.
 * Create: 2020-01-01
 */

/*****************************************************************************
    头文件包含
*****************************************************************************/
#include "hmac_user.h"
#include "mac_resource_ext.h"
#include "mac_vap_ext.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_MAC_USER_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

/*****************************************************************************
    函数实现
*****************************************************************************/
#ifdef _PRE_WLAN_FEATURE_11AX
/*****************************************************************************
 功能描述 : 获取用户的he信息
*****************************************************************************/
void mac_user_get_he_hdl(hmac_user_stru *hmac_user, mac_he_hdl_stru *he_hdl)
{
    (osal_void)memcpy_s((osal_u8 *)he_hdl, OAL_SIZEOF(mac_he_hdl_stru),
        (osal_u8 *)(&hmac_user->he_hdl), OAL_SIZEOF(mac_he_hdl_stru));
}

/*****************************************************************************
 功能描述 : 对用户的vht信息进行设置
*****************************************************************************/
void mac_user_set_he_hdl(hmac_user_stru *hmac_user, mac_he_hdl_stru *he_hdl)
{
    (osal_void)memcpy_s((osal_u8 *)(&hmac_user->he_hdl), OAL_SIZEOF(mac_he_hdl_stru),
        (osal_u8 *)he_hdl, OAL_SIZEOF(mac_he_hdl_stru));
}
#endif

/*****************************************************************************
 功能描述 : 添加wep密钥到指定的密钥槽
*****************************************************************************/
osal_u32 hmac_user_add_wep_key_etc(hmac_user_stru *hmac_user, osal_u8 key_index, mac_key_params_stru *key)
{
    if (key_index >= WLAN_NUM_TK) {
        return OAL_ERR_CODE_SECURITY_KEY_ID;
    }

    if ((osal_u32)key->key_len > WLAN_WEP104_KEY_LEN) {
        return OAL_ERR_CODE_SECURITY_KEY_LEN;
    }

    if ((osal_u32)key->seq_len > WLAN_WPA_SEQ_LEN) {
        return OAL_ERR_CODE_SECURITY_KEY_LEN;
    }

    hmac_user->key_info.key[key_index].key_len = (osal_u32)key->key_len;
    hmac_user->key_info.key[key_index].seq_len = (osal_u32)key->seq_len;
    hmac_user->key_info.key[key_index].cipher = key->cipher;

    (osal_void)memcpy_s(&hmac_user->key_info.key[key_index].key, WLAN_WPA_KEY_LEN, key->key, (osal_u32)key->key_len);
    (osal_void)memcpy_s(&hmac_user->key_info.key[key_index].seq, WLAN_WPA_SEQ_LEN, key->seq, (osal_u32)key->seq_len);

    hmac_user->user_tx_info.security.cipher_key_type = WLAN_KEY_TYPE_TX_GTK;
    /* set default key接口未适配，此处写死使用index 0 */
    if (key_index == 0) {
        hmac_user->key_info.cipher_type = (osal_u8)key->cipher;
        hmac_user->key_info.default_index = key_index;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述 : 更新rsn单播信息
*****************************************************************************/
osal_u32 hmac_user_add_rsn_key_etc(hmac_user_stru *hmac_user, osal_u8 key_index, mac_key_params_stru *key)
{
    if (key_index >= WLAN_NUM_TK) {
        return OAL_ERR_CODE_SECURITY_KEY_ID;
    }
    if ((osal_u32)key->key_len > WLAN_WPA_KEY_LEN) {
        return OAL_ERR_CODE_SECURITY_KEY_LEN;
    }

    if ((osal_u32)key->seq_len > WLAN_WPA_SEQ_LEN) {
        return OAL_ERR_CODE_SECURITY_KEY_LEN;
    }

    hmac_user->key_info.key[key_index].cipher = key->cipher;
    hmac_user->key_info.key[key_index].key_len = (osal_u32)key->key_len;
    hmac_user->key_info.key[key_index].seq_len = (osal_u32)key->seq_len;

    (osal_void)memcpy_s(&hmac_user->key_info.key[key_index].key, WLAN_WPA_KEY_LEN, key->key, (osal_u32)key->key_len);
    (osal_void)memcpy_s(&hmac_user->key_info.key[key_index].seq, WLAN_WPA_SEQ_LEN, key->seq, (osal_u32)key->seq_len);

    hmac_user->key_info.cipher_type = (osal_u8)key->cipher;
    hmac_user->key_info.default_index = key_index;

    hmac_user_sync(hmac_user);
    return OAL_SUCC;
}

/*****************************************************************************
 功能描述 : 更新bip信息
*****************************************************************************/
osal_u32 hmac_user_add_bip_key_etc(hmac_user_stru *hmac_user, osal_u8 key_index, mac_key_params_stru *key)
{
    /* keyid校验 */
    if (key_index < WLAN_NUM_TK || key_index > WLAN_MAX_IGTK_KEY_INDEX) {
        return OAL_ERR_CODE_SECURITY_KEY_ID;
    }

    if ((osal_u32)key->key_len > WLAN_WPA_KEY_LEN) {
        return OAL_ERR_CODE_SECR_KEY_LEN;
    }
    if ((osal_u32)key->seq_len > WLAN_WPA_SEQ_LEN) {
        return OAL_ERR_CODE_SECR_KEY_LEN;
    }

    hmac_user->key_info.key[key_index].cipher = key->cipher;
    hmac_user->key_info.key[key_index].key_len = (osal_u32)key->key_len;
    hmac_user->key_info.key[key_index].seq_len = (osal_u32)key->seq_len;

    (osal_void)memcpy_s(&hmac_user->key_info.key[key_index].key, WLAN_WPA_KEY_LEN, key->key, (osal_u32)key->key_len);
    (osal_void)memcpy_s(&hmac_user->key_info.key[key_index].seq, WLAN_WPA_SEQ_LEN, key->seq, (osal_u32)key->seq_len);

    hmac_user->key_info.igtk_key_index = key_index;

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述 : 根据en_key_type，调用相应的函数，更新vap信息
*****************************************************************************/
wlan_priv_key_param_stru *hmac_user_get_key_etc(hmac_user_stru *hmac_user, osal_u8 key_id)
{
    if (key_id >= WLAN_NUM_TK + WLAN_NUM_IGTK) {
        return OAL_PTR_NULL;
    }
    return &hmac_user->key_info.key[key_id];
}

/*****************************************************************************
 功能描述 : 复位11X端口
*****************************************************************************/
osal_void hmac_user_set_port_etc(hmac_user_stru *hmac_user, oal_bool_enum_uint8 port_valid)
{
    hmac_user->port_valid = port_valid;
}

/*****************************************************************************
 功能描述 : 初始化用户的密钥信息
*****************************************************************************/
osal_void hmac_user_init_key_etc(hmac_user_stru *hmac_user)
{
    memset_s(&hmac_user->key_info, sizeof(mac_key_mgmt_stru), 0, sizeof(mac_key_mgmt_stru));
    hmac_user->key_info.cipher_type = WLAN_80211_CIPHER_SUITE_NO_ENCRYP;
    hmac_user->key_info.last_gtk_key_idx = 0x7F;
    hmac_user->cap_info.pmf_active = OAL_FALSE;
}

/*****************************************************************************
 功能描述 : 设置用户的安全信息
*****************************************************************************/
osal_void hmac_user_set_key_etc(hmac_user_stru *multiuser, wlan_cipher_key_type_enum_uint8 keytype,
    wlan_ciper_protocol_type_enum_uint8 ciphertype, osal_u8 keyid)
{
    multiuser->user_tx_info.security.cipher_key_type = keytype;
    multiuser->user_tx_info.security.cipher_protocol_type = ciphertype;
    multiuser->user_tx_info.security.cipher_key_id = keyid;
    oam_warning_log4(0, OAM_SF_WPA, "{hmac_user_set_key_etc::keytpe==%u, ciphertype==%u, keyid==%u, usridx==%u}",
        keytype, ciphertype, keyid, multiuser->assoc_id);
}

/*****************************************************************************
 功能描述 : 初始化mac user公共部分
*****************************************************************************/
osal_u32 mac_user_init_etc(hmac_user_stru *hmac_user, osal_u16 user_idx, osal_u8 *mac_addr,
    hmac_vap_stru *hmac_vap)
{
#ifdef _PRE_WLAN_FEATURE_WMMAC
    osal_u8 ac_loop;
#endif

    if (OAL_UNLIKELY(hmac_user == OAL_PTR_NULL || hmac_vap == OAL_PTR_NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "{mac_user_init_etc::param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 初始化chip id, device ip, vap id */
    hmac_user->chip_id = hmac_vap->chip_id;
    hmac_user->device_id = hmac_vap->device_id;
    hmac_user->vap_id = hmac_vap->vap_id;
    hmac_user->assoc_id = user_idx;

    /* 初始化密钥 */
    hmac_user->user_tx_info.security.cipher_key_type = WLAN_KEY_TYPE_PTK;
    hmac_user->user_tx_info.security.cipher_protocol_type = WLAN_80211_CIPHER_SUITE_NO_ENCRYP;

    /* 初始化安全加密信息 */
    hmac_user_init_key_etc(hmac_user);
    hmac_user_set_key_etc(hmac_user, WLAN_KEY_TYPE_PTK, WLAN_80211_CIPHER_SUITE_NO_ENCRYP, 0);
    hmac_user_set_port_etc(hmac_user, OAL_FALSE);
    hmac_user->user_asoc_state = MAC_USER_STATE_BUTT;
    hmac_user->ch_text = OSAL_NULL;
    if (mac_addr == OAL_PTR_NULL) {
        hmac_user->is_multi_user = OAL_TRUE;
        hmac_user->user_asoc_state = MAC_USER_STATE_ASSOC;
    } else {
        /* 初始化一个用户是否是组播用户的标志，组播用户初始化时不会调用本函数 */
        hmac_user->is_multi_user = OAL_FALSE;

        /* 设置mac地址 */
        oal_set_mac_addr(hmac_user->user_mac_addr, mac_addr);
    }

    /* 初始化能力 */
    hmac_user_set_pmf_active_etc(hmac_user, OAL_FALSE);
    hmac_user->cap_info.proxy_arp = OAL_FALSE;

    hmac_user_set_avail_num_spatial_stream_etc(hmac_user, WLAN_SINGLE_NSS);

#ifdef _PRE_WLAN_FEATURE_WMMAC
    /* TS信息初始化 */
    for (ac_loop = 0; ac_loop < WLAN_WME_AC_BUTT; ac_loop++) {
        hmac_user->ts_info[ac_loop].up = wlan_wme_ac_to_tid(ac_loop);
        hmac_user->ts_info[ac_loop].ts_status = MAC_TS_NONE;
        hmac_user->ts_info[ac_loop].vap_id = hmac_user->vap_id;
        hmac_user->ts_info[ac_loop].mac_user_idx = hmac_user->assoc_id;
        hmac_user->ts_info[ac_loop].tsid = 0xFF;
    }
#endif

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述 : 设置user公共区域en_avail_num_spatial_stream Tx和Rx支持Nss的交集,供算法调用的值
*****************************************************************************/
void hmac_user_avail_bf_num_spatial_stream_etc(hmac_user_stru *hmac_user, osal_u8 value)
{
    hmac_user->avail_bf_num_spatial_stream = value;
}

/*****************************************************************************
 功能描述 : 设置user公共区域en_avail_num_spatial_stream Tx和Rx支持Nss的交集,供算法调用的值
*****************************************************************************/
void hmac_user_set_avail_num_spatial_stream_etc(hmac_user_stru *hmac_user, osal_u8 value)
{
    hmac_user->avail_num_spatial_stream = value;
}

/*****************************************************************************
 功能描述 : 设置user公共区域en_user_num_spatial_stream用户支持空间流个数的值
*****************************************************************************/
void hmac_user_set_num_spatial_stream_etc(hmac_user_stru *hmac_user, osal_u8 value)
{
    hmac_user->user_num_spatial_stream = value;
}

/*****************************************************************************
 功能描述 : 设置用户的bandwidth能力
*****************************************************************************/
void hmac_user_set_bandwidth_cap_etc(hmac_user_stru *hmac_user, wlan_bw_cap_enum_uint8 bandwidth_value)
{
    hmac_user->bandwidth_cap = bandwidth_value;
}

/*****************************************************************************
 功能描述 : 设置可用带宽的信息
*****************************************************************************/
void hmac_user_set_bandwidth_info_etc(hmac_user_stru *hmac_user, wlan_bw_cap_enum_uint8 avail_bandwidth,
    wlan_bw_cap_enum_uint8 cur_bandwidth)
{
    hmac_user->avail_bandwidth = avail_bandwidth;
    hmac_user->cur_bandwidth = cur_bandwidth;

    /* Autorate将协议从11n切换成11b后, cur_bandwidth会变为20M
       此时如果软件将带宽改为40M, cur_bandwidth仍需要保持20M */
    if ((hmac_user->cur_protocol_mode == WLAN_LEGACY_11B_MODE) && (hmac_user->cur_bandwidth != WLAN_BW_CAP_20M)) {
        hmac_user->cur_bandwidth = WLAN_BW_CAP_20M;
    }
}

/*****************************************************************************
 功能描述 : 获取用户的带宽
*****************************************************************************/
void hmac_user_get_sta_cap_bandwidth_etc(hmac_user_stru *hmac_user, wlan_bw_cap_enum_uint8 *bandwidth_cap)
{
    mac_user_ht_hdl_stru *mac_ht_hdl = OSAL_NULL;
    mac_vht_hdl_stru *mac_vht_hdl = OSAL_NULL;
#ifdef _PRE_WLAN_FEATURE_11AX
    mac_he_hdl_stru *mac_he_hdl = OSAL_NULL;
#endif
    hmac_vap_stru *hmac_vap = OSAL_NULL;
    wlan_bw_cap_enum_uint8 bandwidth_cap_val;

    hmac_vap = mac_res_get_hmac_vap(hmac_user->vap_id);
    if (hmac_vap == OAL_PTR_NULL) {
        return;
    }
    /* 获取HT和VHT结构体指针 */
    mac_ht_hdl = &(hmac_user->ht_hdl);
    mac_vht_hdl = &(hmac_user->vht_hdl);
#ifdef _PRE_WLAN_FEATURE_11AX
    mac_he_hdl = &(hmac_user->he_hdl);
    if (mac_he_hdl->he_capable == OAL_TRUE) {
        if ((mac_he_hdl->he_cap_ie.he_phy_cap.channel_width_set & 0x2) != 0) {
            /* Bit2 指示5G 80MHz */
            bandwidth_cap_val = WLAN_BW_CAP_80M;
        } else if ((mac_he_hdl->he_cap_ie.he_phy_cap.channel_width_set & 0x1) != 0) {
            bandwidth_cap_val = WLAN_BW_CAP_40M;
        } else {
            bandwidth_cap_val = WLAN_BW_CAP_20M;
        }
        hmac_user_set_bandwidth_cap_etc(hmac_user, bandwidth_cap_val);
        *bandwidth_cap = bandwidth_cap_val;
        return; /* 11ax处理完成后返回 */
    }
#endif
    if (mac_vht_hdl->vht_capable == OAL_TRUE) {
        /* 2.4g band不应根据vht cap获取带宽信息 */
        if ((hmac_vap->channel.band == WLAN_BAND_2G) && (mac_ht_hdl->ht_capable == OAL_TRUE)) {
            bandwidth_cap_val = (mac_ht_hdl->supported_channel_width == 1) ? WLAN_BW_CAP_40M : WLAN_BW_CAP_20M;
        } else {
            bandwidth_cap_val = (mac_vht_hdl->supported_channel_width == 0) ? WLAN_BW_CAP_80M : WLAN_BW_CAP_160M;
        }
    } else if (mac_ht_hdl->ht_capable == OAL_TRUE) {
        bandwidth_cap_val = (mac_ht_hdl->supported_channel_width == 1) ? WLAN_BW_CAP_40M : WLAN_BW_CAP_20M;
    } else {
        bandwidth_cap_val = WLAN_BW_CAP_20M;
    }

    hmac_user_set_bandwidth_cap_etc(hmac_user, bandwidth_cap_val);

    /* 将带宽值由出参带出 */
    *bandwidth_cap = bandwidth_cap_val;
}

/*****************************************************************************
 功能描述 : 根据AP的operation ie获取ap的工作带宽cap
*****************************************************************************/
osal_u32 hmac_user_update_ap_bandwidth_cap(hmac_user_stru *hmac_user)
{
    mac_user_ht_hdl_stru *mac_ht_hdl;
    mac_vht_hdl_stru *mac_vht_hdl;

    if (hmac_user == OAL_PTR_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取HT和VHT结构体指针 */
    mac_ht_hdl = &(hmac_user->ht_hdl);
    mac_vht_hdl = &(hmac_user->vht_hdl);

    hmac_user->bandwidth_cap = WLAN_BW_CAP_20M;

    if (mac_ht_hdl->ht_capable == OAL_TRUE) {
        if (mac_ht_hdl->secondary_chan_offset != MAC_SCN) {
            hmac_user->bandwidth_cap = WLAN_BW_CAP_40M;
        }
    }

    if (mac_vht_hdl->vht_capable == OAL_TRUE) {
        /* channel_width的取值，0 -- 20/40M, 1 -- 80M, 2 -- 160M */

        if (mac_vht_hdl->channel_width == WLAN_MIB_VHT_OP_WIDTH_80) {
            if (mac_vht_hdl->channel_center_freq_seg1 != 0) {
                hmac_user->bandwidth_cap = WLAN_BW_CAP_160M;
            } else {
                hmac_user->bandwidth_cap = WLAN_BW_CAP_80M;
            }
        } else if (mac_vht_hdl->channel_width == WLAN_MIB_VHT_OP_WIDTH_160) {
            hmac_user->bandwidth_cap = WLAN_BW_CAP_160M;
        } else if (mac_vht_hdl->channel_width == WLAN_MIB_VHT_OP_WIDTH_80PLUS80) {
            hmac_user->bandwidth_cap = WLAN_BW_CAP_80M;
        }
    }

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述 : 这只mac user中的user与vap协议模式的交集模式
*****************************************************************************/
void hmac_user_set_avail_protocol_mode_etc(hmac_user_stru *hmac_user, wlan_protocol_enum_uint8 avail_protocol_mode)
{
    hmac_user->avail_protocol_mode = avail_protocol_mode;
}

/*****************************************************************************
 功能描述 : 设置mac user中的cur_protocol_mode
*****************************************************************************/
void hmac_user_set_cur_protocol_mode_etc(hmac_user_stru *hmac_user, wlan_protocol_enum_uint8 cur_protocol_mode)
{
    hmac_user->cur_protocol_mode = cur_protocol_mode;
}

/*****************************************************************************
 功能描述 : 设置mac user中的带宽信息
*****************************************************************************/
void hmac_user_set_cur_bandwidth_etc(hmac_user_stru *hmac_user, wlan_bw_cap_enum_uint8 cur_bandwidth)
{
    hmac_user->cur_bandwidth = cur_bandwidth;
}

/*****************************************************************************
 功能描述 : 这只mac user中的user与vap协议模式的交集模式
*****************************************************************************/
void hmac_user_set_protocol_mode_etc(hmac_user_stru *hmac_user, wlan_protocol_enum_uint8 protocol_mode)
{
    hmac_user->protocol_mode = protocol_mode;
}

/*****************************************************************************
 功能描述 : 设置en_user_asoc_state 的统一接口
*****************************************************************************/
void hmac_user_set_asoc_state(hmac_user_stru *hmac_user, mac_user_asoc_state_enum_uint8 value)
{
    mac_user_asoc_state_enum_uint8 old_user_asoc_state = hmac_user->user_asoc_state;
    hmac_user->user_asoc_state = value;

    if (hmac_user->user_asoc_state != old_user_asoc_state) {
        hmac_user_sync(hmac_user);
    }
}

/*****************************************************************************
 功能描述 : 设置user下可用速率集
*****************************************************************************/
void hmac_user_set_avail_op_rates_etc(hmac_user_stru *hmac_user, osal_u8 rs_nrates, osal_u8 *rs_rates)
{
    hmac_user->avail_op_rates.rs_nrates = rs_nrates;
    (osal_void)memcpy_s(hmac_user->avail_op_rates.rs_rates, WLAN_MAX_SUPP_RATES, rs_rates, WLAN_MAX_SUPP_RATES);
}

/*****************************************************************************
 功能描述 : 对用户的vht信息进行设置
*****************************************************************************/
void hmac_user_set_vht_hdl_etc(hmac_user_stru *hmac_user, mac_vht_hdl_stru *vht_hdl)
{
    (osal_void)memcpy_s((osal_u8 *)(&hmac_user->vht_hdl), OAL_SIZEOF(mac_vht_hdl_stru),
        (osal_u8 *)vht_hdl, OAL_SIZEOF(mac_vht_hdl_stru));
}

/*****************************************************************************
 功能描述 : 获取用户的vht信息
*****************************************************************************/
void hmac_user_get_vht_hdl_etc(hmac_user_stru *hmac_user, mac_vht_hdl_stru *vht_hdl)
{
    (osal_void)memcpy_s((osal_u8 *)vht_hdl, OAL_SIZEOF(mac_vht_hdl_stru),
        (osal_u8 *)(&hmac_user->vht_hdl), OAL_SIZEOF(mac_vht_hdl_stru));
}

/*****************************************************************************
 功能描述 : 对用户的vht信息进行设置
*****************************************************************************/
void hmac_user_set_ht_hdl_etc(hmac_user_stru *hmac_user, mac_user_ht_hdl_stru *ht_hdl)
{
    (osal_void)memcpy_s((osal_u8 *)(&hmac_user->ht_hdl), OAL_SIZEOF(mac_user_ht_hdl_stru),
        (osal_u8 *)ht_hdl, OAL_SIZEOF(mac_user_ht_hdl_stru));
}

/*****************************************************************************
 功能描述 : 获取用户的ht信息
*****************************************************************************/
void hmac_user_get_ht_hdl_etc(hmac_user_stru *hmac_user, mac_user_ht_hdl_stru *ht_hdl)
{
    (osal_void)memcpy_s((osal_u8 *)ht_hdl, OAL_SIZEOF(mac_user_ht_hdl_stru),
        (osal_u8 *)(&hmac_user->ht_hdl), OAL_SIZEOF(mac_user_ht_hdl_stru));
}

/*****************************************************************************
 功能描述 : 设置ht cap信息
*****************************************************************************/
void hmac_user_set_ht_capable_etc(hmac_user_stru *hmac_user, oal_bool_enum_uint8 ht_capable)
{
    hmac_user->ht_hdl.ht_capable = ht_capable;
}

#ifdef _PRE_WLAN_FEATURE_SMPS
/*****************************************************************************
 功能描述 : 设置用户的ht能力中sm_power_save的信息
*****************************************************************************/
void hmac_user_set_sm_power_save(hmac_user_stru *hmac_user, osal_u8 sm_power_save)
{
    hmac_user->ht_hdl.sm_power_save = sm_power_save;
}
#endif

/*****************************************************************************
 功能描述 : 设置与用户pmf能力协商的结果
*****************************************************************************/
void hmac_user_set_pmf_active_etc(hmac_user_stru *hmac_user, oal_bool_enum_uint8 pmf_active)
{
    hmac_user->cap_info.pmf_active = pmf_active;
}

/*****************************************************************************
 功能描述 : 设置与用户的qos使能结果
*****************************************************************************/
void hmac_user_set_qos_etc(hmac_user_stru *hmac_user, oal_bool_enum_uint8 qos_mode)
{
    if (qos_mode == hmac_user->cap_info.qos) {
        return;
    }
    hmac_user->cap_info.qos = qos_mode;
}

/*****************************************************************************
 功能描述 : 设置更新用户spectrum_mgmt能力的信息
*****************************************************************************/
void hmac_user_set_spectrum_mgmt_etc(hmac_user_stru *hmac_user, oal_bool_enum_uint8 spectrum_mgmt)
{
    hmac_user->cap_info.spectrum_mgmt = spectrum_mgmt;
}

/*****************************************************************************
 功能描述 : 设置用户公共区域的apsd能力
*****************************************************************************/
void hmac_user_set_apsd_etc(hmac_user_stru *hmac_user, oal_bool_enum_uint8 apsd)
{
    hmac_user->cap_info.apsd = apsd;
}

/*****************************************************************************
 功能描述 : 设置用户wep加密密钥信息
*****************************************************************************/
osal_u32 hmac_user_update_wep_key_etc(hmac_user_stru *hmac_user, osal_u16 multi_user_idx)
{
    hmac_user_stru *multi_user = OAL_PTR_NULL;

    if (hmac_user == OAL_PTR_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    multi_user = (hmac_user_stru *)mac_res_get_hmac_user_etc(multi_user_idx);
    if (multi_user == OAL_PTR_NULL) {
        return OAL_ERR_CODE_SECURITY_USER_INVAILD;
    }

    if (multi_user->key_info.cipher_type != WLAN_80211_CIPHER_SUITE_WEP_104 &&
        multi_user->key_info.cipher_type != WLAN_80211_CIPHER_SUITE_WEP_40) {
        oam_error_log1(0, OAM_SF_WPA, "{mac_wep_add_usr_key::cipher_type==%d}",
            multi_user->key_info.cipher_type);
        return OAL_ERR_CODE_SECURITY_CHIPER_TYPE;
    }

    if (multi_user->key_info.default_index >= WLAN_MAX_WEP_KEY_COUNT) {
        return OAL_ERR_CODE_SECURITY_KEY_ID;
    }

    /* wep加密下，拷贝组播用户的密钥信息到单播用户 */
    (osal_void)memcpy_s(&hmac_user->key_info, OAL_SIZEOF(mac_key_mgmt_stru),
        &multi_user->key_info, OAL_SIZEOF(mac_key_mgmt_stru));

    /* 发送信息要挪出去 */
    hmac_user->user_tx_info.security.cipher_key_type =
        hmac_user->key_info.default_index + HAL_KEY_TYPE_PTK; // 获取WEP default key id

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述 : 判断mac地址是否全0
*****************************************************************************/
oal_bool_enum_uint8 hmac_addr_is_zero_etc(osal_u8 *mac)
{
    osal_u8    mac_zero[OAL_MAC_ADDR_LEN] = {0};

    if (mac == OAL_PTR_NULL) {
        return OAL_TRUE;
    }

    return (osal_memcmp(mac_zero, mac, OAL_MAC_ADDR_LEN) == 0);
}

oal_bool_enum_uint8 hmac_ip_is_zero_etc(osal_u8 *ip_addr)
{
    osal_u8    ip_zero[OAL_MAC_ADDR_LEN] = {0};

    if (ip_addr == OAL_PTR_NULL) {
        return OAL_TRUE;
    }

    return (osal_memcmp(ip_addr, ip_zero, OAL_IPV4_ADDR_SIZE) == 0);
}

// from mac_user_rom.c
/*****************************************************************************
 函 数 名  : hmac_user_set_assoc_id_etc
 功能描述  : 设置user的assoc id
*****************************************************************************/
osal_void hmac_user_set_assoc_id_etc(hmac_user_stru *hmac_user, osal_u16 assoc_id)
{
    hmac_user->assoc_id = assoc_id;
    hmac_user_sync(hmac_user);
}

/*****************************************************************************
 函 数 名  : hmac_user_set_barker_preamble_mode_etc
 功能描述  : 设置用户barker_preamble的模式
*****************************************************************************/
osal_void hmac_user_set_barker_preamble_mode_etc(hmac_user_stru *hmac_user, oal_bool_enum_uint8 barker_preamble_mode)
{
    hmac_user->cap_info.barker_preamble_mode = barker_preamble_mode;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
