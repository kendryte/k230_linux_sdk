/*
 * Copyright (c) CompanyNameMagicTag 2016-2023. All rights reserved.
 * Description: wal linux vendor api.
 * Create: 2016-07-19
 */

#include "wal_linux_cfgvendor.h"
#include "oal_netbuf.h"
#include "oal_cfg80211.h"
#include "oal_ext_if.h"
#include "frw_ext_if.h"
#include "wal_linux_util.h"
#include "wal_linux_ioctl.h"
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
#include "soc_customize_wifi.h"
#endif /* #ifdef _PRE_PLAT_FEATURE_CUSTOMIZE */
#include "wlan_msg.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_WAL_LINUX_CFGVENDOR_C
#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

#define OUI_GOOGLE  0x001A11
#define OUI_SOC     0x001018

#if defined (_PRE_PRODUCT_ID_HOST) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3,14,0))
wal_cfgvendor_radio_stat_stru g_st_wifi_radio_stat_etc;

/*****************************************************************************
 函 数 名  : wal_cfgvendor_copy_channel_list
 功能描述  : 将对应band 的信道列表拷贝到缓冲区
*****************************************************************************/
OAL_STATIC osal_u32 wal_cfgvendor_copy_channel_list(mac_vendor_cmd_channel_list_stru *pst_channel_list,
    osal_u32   band,
    osal_s32   *pl_channel_list,
    osal_u32 *pul_num_channels)
{
    osal_u8  *chanel_list;
    osal_u32  loop;
    osal_u32  channel_num = 0;

    *pul_num_channels = 0;

    if ((pst_channel_list->channel_num_5g == 0) && (pst_channel_list->channel_num_2g == 0)) {
        oam_warning_log1(0, OAM_SF_ANY,
                         "{wal_cfgvendor_copy_channel_list::get 5G or 2G channel list fail,band = %d", band);
        return OAL_FAIL;
    }

    /* 获取2G,信道列表 */
    if ((band & WIFI_BAND_24_GHZ) && (pst_channel_list->channel_num_2g != 0)) {
        chanel_list = pst_channel_list->channel_list_2g;
        for (loop = 0; loop < pst_channel_list->channel_num_2g; loop++) {
            pl_channel_list[channel_num++] =
                oal_ieee80211_channel_to_frequency(chanel_list[loop], OAL_IEEE80211_BAND_2GHZ);
        }
    }

    /* 获取5G Band信道列表，包括非DFS Channels和DFS Channels */
    if ((band & (WIFI_BAND_5_GHZ | WIFI_BAND_5_GHZ_DFS_ONLY)) && (pst_channel_list->channel_num_5g != 0)) {
        oal_bool_enum_uint8 dfs;
        chanel_list = pst_channel_list->channel_list_5g;

        for (loop = 0; loop < pst_channel_list->channel_num_5g; loop++) {
            dfs = mac_is_dfs_channel(MAC_RC_START_FREQ_5, chanel_list[loop]);
            if (((band & WIFI_BAND_5_GHZ_DFS_ONLY) && (dfs == OAL_TRUE))
                || ((band & WIFI_BAND_5_GHZ) && (dfs == OAL_FALSE))) {
                pl_channel_list[channel_num++] =
                    oal_ieee80211_channel_to_frequency(chanel_list[loop], OAL_IEEE80211_BAND_5GHZ);
            }
        }
    }

    if (channel_num == 0) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_cfgvendor_copy_channel_list::get 5G or 2G channel list fail,channel_num = %d", channel_num);
        return OAL_FAIL;
    }

    *pul_num_channels = channel_num;

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : wal_cfgvendor_get_current_channel_list
 功能描述  : 根据band ，获取当前信道列表
*****************************************************************************/
OAL_STATIC osal_u32 wal_cfgvendor_get_current_channel_list(oal_net_device_stru *netdev,
    osal_u32 band,
    osal_s32 *channel_list,
    osal_u32 *num_channels)
{
    osal_s32  ret;
    frw_msg    cfg_info;
    mac_vendor_cmd_channel_list_stru fill_msg;
    mac_vendor_cmd_channel_list_stru *rsp_info;

    if (netdev == OAL_PTR_NULL || channel_list == OAL_PTR_NULL || num_channels == OAL_PTR_NULL) {
        oam_error_log3(0, OAM_SF_ANY,
            "{wal_cfgvendor_get_current_channel_list::channel_list or num_channel is \
            NULL!netdev %p, channel_list %p, num_channels %p.}", (uintptr_t)netdev, (uintptr_t)channel_list,
            (uintptr_t)num_channels);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 上层在任何时候都可能下发此命令，需要先判断当前netdev的状态并及时返回 */
    if (OAL_UNLIKELY(netdev->ml_priv == OAL_PTR_NULL)) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_cfgvendor_get_current_channel_list::NET_DEV_PRIV is NULL.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    memset_s(&cfg_info, sizeof(cfg_info), 0, sizeof(cfg_info));
    cfg_info.rsp_buf_len = (osal_u16)sizeof(fill_msg);
    cfg_info.rsp = (osal_u8 *)&fill_msg;

    ret = send_sync_cfg_to_host(wal_util_get_vap_id(netdev), WLAN_MSG_W2H_CFG_VENDOR_CMD_GET_CHANNEL_LIST, &cfg_info);
    if ((ret != OAL_SUCC) || (cfg_info.rsp == OAL_PTR_NULL)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_cfgvendor_get_current_channel_list:: return err code %d!}", ret);
        return ret;
    }

    rsp_info = (mac_vendor_cmd_channel_list_stru *)(cfg_info.rsp);

    if (wal_cfgvendor_copy_channel_list(rsp_info, band, channel_list, num_channels) != OAL_SUCC) {
        return OAL_FAIL;
    }

    return OAL_SUCC;
}

OAL_STATIC osal_s32 uapi_cfgvendor_get_channel_list(oal_wiphy_stru *wiphy,
    oal_wireless_dev_stru *wdev, OAL_CONST osal_void *data, osal_s32 len)
{
    osal_s32 err = 0;
    osal_s32 type;
    osal_s32 al_channel_list[VENDOR_CHANNEL_LIST_ALL + 1] = {0};
    osal_u32 band = 0; /* 上层下发的band值 */
    osal_u32 mem_needed, ret;
    osal_u32 num_channels = 0;
    osal_u32 reply_len = 0;
    oal_netbuf_stru *skb = OSAL_NULL;

    wal_record_wifi_external_log(WLAN_WIFI_VENDER_CMDS, __func__);
    if (wdev == OAL_PTR_NULL || wdev->netdev == OAL_PTR_NULL) {
        oam_error_log1(0, OAM_SF_ANY, "uapi_cfgvendor_get_channel_list::wdev or netdev NULL!wdev %p", (uintptr_t)wdev);
        return -OAL_EFAIL;
    }

    if (data == OSAL_NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "uapi_cfgvendor_get_channel_list data is null");
        return -OAL_EFAIL;
    }

    type = oal_nla_type(data);
    if (type == GSCAN_ATTRIBUTE_BAND) {
        /* 获取band值 bit0:2G信道列表 bit1:5G信道列表 bit2:DFS channel */
        band = oal_nla_get_u32(data);
    } else {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_cfgvendor_get_channel_list::invalid nla type! type %d.}", type);
        return -OAL_EFAIL;
    }

    ret = wal_cfgvendor_get_current_channel_list(wdev->netdev, band, al_channel_list, &num_channels);
    if (ret != OAL_SUCC) {
        oam_warning_log3(0, OAM_SF_ANY,
            "{uapi_cfgvendor_get_channel_list::fail. ret %d, band %d, num_channels %d.}", ret, band, num_channels);
        return -OAL_EFAIL;
    }

    reply_len = (osal_u32)OAL_SIZEOF(al_channel_list[0]) * (num_channels);
    mem_needed = reply_len + VENDOR_REPLY_OVERHEAD + (ATTRIBUTE_U32_LEN * 2);

    /* 申请SKB 上报信道列表 */
    skb = oal_cfg80211_vendor_cmd_alloc_reply_skb_etc(wiphy, mem_needed);
    if (OAL_UNLIKELY(!skb)) {
        oam_warning_log1(0, OAM_SF_ANY, "uapi_cfgvendor_get_channel_list::skb alloc failed.len %d\r\n",  mem_needed);
        return -OAL_ENOMEM;
    }

    oal_nla_put_u32(skb, GSCAN_ATTRIBUTE_NUM_CHANNELS, num_channels);
    oal_nla_put(skb, GSCAN_ATTRIBUTE_CHANNEL_LIST, reply_len, al_channel_list);

    err = oal_cfg80211_vendor_cmd_reply_etc(skb);
    if (OAL_UNLIKELY(err)) {
        oam_warning_log1(0, OAM_SF_ANY, "uapi_cfgvendor_get_channel_list::vendor_cmd reply failed ret:%d", err);
        return err;
    }

    oam_warning_log3(0, OAM_SF_ANY, "{uapi_cfgvendor_get_channel_list::ret_value %d, band %d, num_channels %d.}",
                     err, band, num_channels);

    return err;
}


OAL_STATIC osal_s32 uapi_cfgvendor_set_country(oal_wiphy_stru *wiphy,
    oal_wireless_dev_stru *wdev, OAL_CONST osal_void *data, osal_s32 len)
{
    osal_s32 rem;
    osal_s32 type;
    osal_s32 l_ret = OAL_SUCC;
    osal_s8  auc_country_code[WLAN_COUNTRY_STR_LEN] = {0};
    OAL_CONST oal_nlattr_stru *iter;

    wal_record_wifi_external_log(WLAN_WIFI_VENDER_CMDS, __func__);
    if (data == OSAL_NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{uapi_cfgvendor_set_country::data null");
        return -OAL_EFAUL;
    }
    /* 测试阶段可采用定制化99配置 */
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    if (OAL_TRUE == g_st_cust_country_code_ignore_flag.country_code_ingore_ini_flag) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_cfgvendor_set_country::set_country ignored, flag ini[%d]",
                         g_st_cust_country_code_ignore_flag.country_code_ingore_ini_flag);
        return l_ret;
    }
#endif

    OAL_NLA_FOR_EACH_ATTR(iter, data, len, rem) {
        memset_s(auc_country_code, WLAN_COUNTRY_STR_LEN, 0, WLAN_COUNTRY_STR_LEN);
        type = oal_nla_type(iter);
        switch (type) {
            case ANDR_WIFI_ATTRIBUTE_COUNTRY:
#ifdef _PRE_WLAN_FEATURE_11D
                if (memcpy_s(auc_country_code, sizeof(auc_country_code), oal_nla_data(iter),
                    OAL_MIN(oal_nla_len(iter), OAL_SIZEOF(auc_country_code))) != EOK) {
                    oam_error_log0(0, OAM_SF_ANY, "{uapi_cfgvendor_set_country::memcpy_s error}");
                }
                oam_warning_log4(0, OAM_SF_ANY, "{uapi_cfgvendor_set_country::country code:%c %c %c, len = %d!}",
                                 auc_country_code[0], auc_country_code[1], auc_country_code[2], oal_nla_len(iter));
                /* 设置国家码到wifi 驱动 */
                l_ret = wal_util_set_country_code(wdev->netdev, auc_country_code);
#else
                oam_warning_log0(0, OAM_SF_ANY,
                                 "{uapi_cfgvendor_set_country::_PRE_WLAN_FEATURE_11D is not define!}\r\n");
#endif
                break;
            default:
                oam_warning_log1(0, OAM_SF_ANY, "{uapi_cfgvendor_set_country::unknow type:%d!}\r\n", type);
                break;
        }
    }

    return l_ret;
}

OAL_STATIC osal_u32 wal_cfgvendor_do_get_feature_set(mac_wiphy_priv_stru  *wiphy_priv)
{
    osal_u32 feature_set = 0;
    hmac_device_stru  *hmac_device;

    if (wiphy_priv == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_cfgvendor_do_get_feature_set::hmac_device is null!}");
        return feature_set;
    }
    hmac_device = wiphy_priv->hmac_device;
    /* Begin: if chip support 5G, set WIFI_FEATURE_INFRA_5G */
    if (hmac_device != NULL) {
        feature_set |= ((OAL_TRUE == hmac_device_check_5g_enable(hmac_device->device_id)) ?
                          WIFI_FEATURE_INFRA_5G : 0);
    }
    /* End: if chip support 5G, set WIFI_FEATURE_INFRA_5G */

#ifdef _PRE_WLAN_FEATURE_HS20
    feature_set |= WIFI_FEATURE_HOTSPOT;
#endif
    feature_set |= WIFI_FEATURE_LINK_LAYER_STATS; /** 0x10000 Link layer stats collection */
    /* According to Google VTS test, The following bit control Google roaming,
     * which is prior to GSCAN_SUBCMD_GET_CAPABILITIES--wal_cfgvendor_get_gscan_capabilities.
     * So we need to enable them at the same time, or disable them at the same time. */
    return feature_set;
}

OAL_STATIC osal_s32 uapi_cfgvendor_get_feature_set(oal_wiphy_stru *wiphy,
    oal_wireless_dev_stru *wdev, OAL_CONST osal_void  *data, osal_s32 len)
{
    osal_s32 err = 0;
    osal_s32 reply;
    osal_u32 reply_len = (osal_u32)OAL_SIZEOF(osal_s32);
    oal_netbuf_stru       *skb;
    mac_wiphy_priv_stru   *wiphy_priv;

    wal_record_wifi_external_log(WLAN_WIFI_VENDER_CMDS, __func__);
    if (NULL == wdev) {
        oam_warning_log0(0, OAM_SF_ANY, "{uapi_cfgvendor_get_feature_set::wdev is null!!}");
        return -OAL_EFAUL;
    }

    wiphy_priv = (mac_wiphy_priv_stru *)oal_wiphy_priv(OAL_WIRELESS_DEV_WIPHY(wdev));

    reply = wal_cfgvendor_do_get_feature_set(wiphy_priv);

    skb = oal_cfg80211_vendor_cmd_alloc_reply_skb_etc(wiphy, reply_len);
    if (OAL_UNLIKELY(!skb)) {
        oam_warning_log1(0, OAM_SF_ANY, "uapi_cfgvendor_get_feature_set::skb alloc failed.len %d", reply_len);
        return -OAL_ENOMEM;
    }

    oal_nla_put_nohdr(skb, reply_len, &reply);

    err = oal_cfg80211_vendor_cmd_reply_etc(skb);
    if (OAL_UNLIKELY(err)) {
        oam_error_log1(0, OAM_SF_ANY, "uapi_cfgvendor_get_feature_set::Vendor Command reply failed ret:%d.", err);
    }

    oam_warning_log1(0, OAM_SF_ANY, "{uapi_cfgvendor_get_feature_set::set flag:0x%x.}", reply);

    return err;
}


/*****************************************************************************
 函 数 名  : wal_send_rand_mac_oui
 功能描述  : 发送随机mac oui到hmac
*****************************************************************************/
OAL_STATIC osal_s32 wal_send_random_mac_oui(oal_net_device_stru *net_dev, osal_u8 *random_mac_oui)
{
    osal_s32 ret;

    if (OAL_PTR_NULL == net_dev || OAL_PTR_NULL == random_mac_oui) {
        oam_error_log2(0, OAM_SF_ANY,
                       "{wal_send_random_mac_oui:: null point argument,net_dev:%p, random_mac_oui:%p.}",
                       (uintptr_t)net_dev, (uintptr_t)random_mac_oui);
        return -OAL_EFAIL;
    }

    ret = wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(net_dev), WLAN_MSG_W2H_CFG_SET_RANDOM_MAC_OUI,
        random_mac_oui, WLAN_RANDOM_MAC_OUI_LEN);
    if (OAL_UNLIKELY(OAL_SUCC != ret)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_send_random_mac_oui::send rand_mac_oui failed, err code %d!}", ret);
        return ret;
    }

    return ret;
}

/*****************************************************************************
 函 数 名  : wl_cfgvendor_set_rand_mac_oui
 功能描述  : 设置随机mac oui
*****************************************************************************/
OAL_STATIC osal_s32 uapi_cfgvendor_set_random_mac_oui(oal_wiphy_stru *wiphy,
    oal_wireless_dev_stru *wdev,
    OAL_CONST osal_void *data,
    osal_s32 l_len)
{
    osal_s32 l_type;
    osal_s32 l_ret;
    osal_u8 random_mac_oui[WLAN_RANDOM_MAC_OUI_LEN] = {0};

    wal_record_wifi_external_log(WLAN_WIFI_VENDER_CMDS, __func__);
    if (data == OSAL_NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "uapi_cfgvendor_set_random_mac_oui:: data is null");
        return -OAL_EFAUL;
    }
    l_type = oal_nla_type(data);

    if (ANDR_WIFI_ATTRIBUTE_RANDOM_MAC_OUI == l_type) {
        /* 随机mac地址前3字节(mac oui)由Android下发,wps pbc场景和hilink关联场景会将此3字节清0 */
        if (memcpy_s(random_mac_oui, sizeof(random_mac_oui), oal_nla_data(data), WLAN_RANDOM_MAC_OUI_LEN) != EOK) {
            oam_error_log0(0, OAM_SF_ANY, "{uapi_cfgvendor_set_random_mac_oui::memcpy_s error}");
        }
        oam_warning_log3(0, OAM_SF_ANY, "{uapi_cfgvendor_set_random_mac_oui::mac_ou:0x%.2x:%.2x:%.2x}\r\n",
                         random_mac_oui[0], random_mac_oui[1], random_mac_oui[2]);
    } else {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_cfgvendor_set_random_mac_oui:: unknow type:%x}\r\n", l_type);
        return -OAL_EFAIL;
    }

    l_ret = wal_send_random_mac_oui(wdev->netdev, random_mac_oui);
    if (OAL_UNLIKELY(OAL_SUCC != l_ret)) {
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}

OAL_STATIC osal_u32  wal_cfgvendor_blacklist_mode(oal_net_device_stru *net_dev, osal_u8 mode)
{
    osal_s32                       ret;
    ret = wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(net_dev), WLAN_MSG_W2H_CFG_BLACKLIST_MODE,
        (osal_u8 *)&mode, OAL_SIZEOF(mode));
    if (OAL_UNLIKELY(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_CFG, "{wal_cfgvendor_blacklist_mode:wal_sync_post2hmac_no_rsp return[%d].}", ret);
        return (osal_u32)ret;
    }

    return OAL_SUCC;
}

OAL_STATIC osal_u32  wal_cfgvendor_blacklist_add(oal_net_device_stru *net_dev, osal_u8 *mac_addr)
{
    osal_s32                       ret;

    mac_blacklist_stru blacklist;
    (osal_void)memset_s(&blacklist, OAL_SIZEOF(blacklist), 0, OAL_SIZEOF(blacklist));

    if (memcpy_s(blacklist.auc_mac_addr, sizeof(blacklist.auc_mac_addr), mac_addr, OAL_MAC_ADDR_LEN) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_cfgvendor_blacklist_add::memcpy_s error}");
    }
    ret = wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(net_dev), WLAN_MSG_W2H_CFG_ADD_BLACK_LIST,
        (osal_u8 *)&blacklist, OAL_SIZEOF(blacklist));
    if (OAL_UNLIKELY(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_CFG, "{wal_cfgvendor_blacklist_add:wal_sync_post2hmac_no_rsp return[%d].}", ret);
        return (osal_u32)ret;
    }

    return OAL_SUCC;
}

OAL_STATIC osal_u32  wal_cfgvendor_blacklist_del(oal_net_device_stru *net_dev, osal_u8 *mac_addr)
{
    osal_s32                       ret;
    ret = wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(net_dev), WLAN_MSG_W2H_CFG_DEL_BLACK_LIST,
        (osal_u8 *)mac_addr, OAL_MAC_ADDR_LEN);
    if (OAL_UNLIKELY(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_CFG, "{wal_cfgvendor_blacklist_del:wal_sync_post2hmac_no_rsp return[%d].}", ret);
        return (osal_u32)ret;
    }

    return OAL_SUCC;
}

OAL_STATIC osal_void wal_cfgvender_flush_bssid_blacklist(oal_nlattr_stru *nla, oal_wireless_dev_stru *wdev)
{
    oal_bool_enum_uint8 flush = OAL_FALSE;
    osal_s32 type;

    type = oal_nla_type(nla);
    if (type == GSCAN_ATTRIBUTE_BSSID_BLACKLIST_FLUSH) {
        flush = oal_nla_get_u32(nla);
    }
    oam_warning_log2(0, OAM_SF_ANY, "{uapi_cfgvendor_set_bssid_blacklist::l_type=%d flush=%d}", type, flush);

    if (flush == OAL_TRUE) {
        osal_u8 mac_addr[OAL_MAC_ADDR_LEN] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
        wal_cfgvendor_blacklist_del(wdev->netdev, mac_addr); /* broadcast address as delete all bssids */
    }
}

OAL_STATIC osal_void wal_cfgvender_set_blacklist(oal_wireless_dev_stru *wdev, wal_wifi_bssid_params *bssid_info,
    oal_nlattr_stru **nla)
{
    osal_u8 i = 0;
    osal_s32 type;

    for (i = 0; i < bssid_info->num_bssid; i++) {
        type = oal_nla_type(*nla);
        if (type == GSCAN_ATTRIBUTE_BLACKLIST_BSSID) {
            if (memcpy_s(bssid_info->bssids[i], OAL_MAC_ADDR_LEN, oal_nla_data(*nla), OAL_MAC_ADDR_LEN) != EOK) {
                oam_error_log0(0, OAM_SF_ANY, "{wal_cfgvender_set_blacklist::memcpy_s error}");
            }
        }
        *nla = (oal_nlattr_stru *)((osal_u8 *)(*nla) + oal_nla_total_size(*nla));
    }

    wal_cfgvendor_blacklist_mode(wdev->netdev, CS_BLACKLIST_MODE_BLACK);

    for (i = 0; i < bssid_info->num_bssid; i++) {
        wal_cfgvendor_blacklist_add(wdev->netdev, bssid_info->bssids[i]);
    }

    oam_warning_log4(0, OAM_SF_ANY, "{uapi_cfgvendor_set_bssid_blacklist::bssid[0]=%02x:%02x:%02x:%02x:xx:xx}\r\n",
                     bssid_info->bssids[0][0], bssid_info->bssids[0][1],
                     bssid_info->bssids[0][2], bssid_info->bssids[0][3]); /* 0：下标，2、3：mac地址 */
    oam_warning_log4(0, OAM_SF_ANY, "{uapi_cfgvendor_set_bssid_blacklist::bssid[1]=%02x:%02x:%02x:%02x:xx:xx}\r\n",
                     bssid_info->bssids[1][0], bssid_info->bssids[1][1],
                     bssid_info->bssids[1][2], bssid_info->bssids[1][3]); /* 1：下标，2、3：mac地址 */
    oam_warning_log4(0, OAM_SF_ANY, "{uapi_cfgvendor_set_bssid_blacklist::bssid[2]=%02x:%02x:%02x:%02x:xx:xx}\r\n",
                     bssid_info->bssids[2][0], bssid_info->bssids[2][1],  /* 2：下标，0、1：mac地址 */
                     bssid_info->bssids[2][2], bssid_info->bssids[1][3]); /* 2：下标，2、3：mac地址 */
}

OAL_STATIC osal_s32 uapi_cfgvendor_set_bssid_blacklist(oal_wiphy_stru *wiphy,
    oal_wireless_dev_stru *wdev,
    OAL_CONST osal_void  *data,
    osal_s32 len)
{
    osal_s32 type = 0;
    oal_nlattr_stru *nla;
    wal_wifi_bssid_params bssid_info;

    wal_record_wifi_external_log(WLAN_WIFI_VENDER_CMDS, __func__);
    if (OAL_PTR_NULL == wdev || OAL_PTR_NULL == wdev->netdev) {
        oam_error_log1(0, OAM_SF_ANY,
                       "{uapi_cfgvendor_set_bssid_blacklist::wdev or netdev is NULL! wdev=%p.}", (uintptr_t)wdev);
        return -OAL_EFAIL;
    }

    if ((OAL_PTR_NULL == data) || (len <= NLA_HDRLEN)) {
        oam_error_log2(0, OAM_SF_ANY,
                       "{uapi_cfgvendor_set_bssid_blacklist::invalid para data %p, len=%d.}", (uintptr_t)data, len);
        return -OAL_EINVAL;
    }

    memset_s(&bssid_info, sizeof(bssid_info), 0, sizeof(bssid_info));
    nla = (oal_nlattr_stru *)data;
    type = oal_nla_type(nla);
    if (GSCAN_ATTRIBUTE_NUM_BSSID == type) {
        bssid_info.num_bssid = oal_nla_get_u32(nla);
        oam_warning_log1(0, OAM_SF_ANY,
                         "{uapi_cfgvendor_set_bssid_blacklist::num_bssid=%d}\r\n", bssid_info.num_bssid);
    } else {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_cfgvendor_set_bssid_blacklist:: unknown bssid type:%d}\r\n", type);
        return -OAL_EFAIL;
    }

    nla = (oal_nlattr_stru *)((osal_u8 *)nla + oal_nla_total_size(nla));
    if (!bssid_info.num_bssid) { /* Flush BSSID Blacklist */
        wal_cfgvender_flush_bssid_blacklist(nla, wdev);
    } else { /* set BSSID Blacklist */
        wal_cfgvender_set_blacklist(wdev, &bssid_info, &nla);
    }

    return OAL_SUCC;
}

#ifdef _PRE_WIFI_DEBUG
/*****************************************************************************
 函 数 名  : uapi_cfgvendor_dbg_get_feature
 功能描述  : 获取特性值
*****************************************************************************/
OAL_STATIC osal_s32 uapi_cfgvendor_dbg_get_feature(oal_wiphy_stru *wiphy,
    oal_wireless_dev_stru *wdev, OAL_CONST osal_void  *data, osal_s32 len)
{
    osal_s32 l_ret;
    osal_u32 supported_features = VENDOR_DBG_MEMORY_DUMP_SUPPORTED;
    oal_netbuf_stru *skb;

    wal_record_wifi_external_log(WLAN_WIFI_VENDER_CMDS, __func__);
    skb = oal_cfg80211_vendor_cmd_alloc_reply_skb_etc(wiphy, OAL_SIZEOF(osal_u32));
    if (OAL_UNLIKELY(!skb)) {
        oam_warning_log1(0, OAM_SF_ANY,
                         "uapi_cfgvendor_dbg_get_feature::skb alloc failed. len %d\r\n", OAL_SIZEOF(osal_u32));
        return -OAL_ENOMEM;
    }

    oal_nla_put_nohdr(skb, OAL_SIZEOF(osal_u32), &supported_features);

    l_ret = oal_cfg80211_vendor_cmd_reply_etc(skb);
    if (OAL_UNLIKELY(l_ret)) {
        oam_error_log1(0, OAM_SF_ANY, "uapi_cfgvendor_dbg_get_feature::Vendor Command reply failed:%d.\r\n", l_ret);
    }
    oam_warning_log1(0, OAM_SF_ANY,
                     "uapi_cfgvendor_dbg_get_feature::SUCCESS. supported feature:0x%X.\r\n", supported_features);

    return l_ret;
}

/*****************************************************************************
 函 数 名  : uapi_cfgvendor_dbg_get_version
 功能描述  : 获取driver版本号
*****************************************************************************/
OAL_STATIC osal_s32 uapi_cfgvendor_dbg_get_version(oal_wiphy_stru *wiphy,
    oal_wireless_dev_stru *wdev, OAL_CONST osal_void  *data, osal_s32 len)
{
    osal_s8 driver_version[] = "Soc Host Driver, version V100";
    osal_s32 ret;
    osal_u32 buf_len = (osal_u32)OAL_SIZEOF(driver_version);
    oal_netbuf_stru *skb;

    wal_record_wifi_external_log(WLAN_WIFI_VENDER_CMDS, __func__);
    skb = oal_cfg80211_vendor_cmd_alloc_reply_skb_etc(wiphy, buf_len);
    if (OAL_UNLIKELY(!skb)) {
        oam_warning_log1(0, OAM_SF_ANY, "uapi_cfgvendor_dbg_get_version::skb alloc failed.len %d", buf_len);
        return -OAL_ENOMEM;
    }

    oal_nla_put_nohdr(skb, buf_len, driver_version);

    ret = oal_cfg80211_vendor_cmd_reply_etc(skb);
    if (OAL_UNLIKELY(ret)) {
        oam_error_log1(0, OAM_SF_ANY, "uapi_cfgvendor_dbg_get_version::Vendor Command reply failed:%d.", ret);
    }

    oam_warning_log1(0, OAM_SF_ANY, "uapi_cfgvendor_dbg_get_version::SUCCESS. driver version len %d", buf_len);
    return ret;
}

osal_s32 uapi_cfgvendor_dbg_get_ring_status_etc(oal_wiphy_stru *wiphy,
    oal_wireless_dev_stru *wdev, OAL_CONST osal_void  *data, osal_s32 len)
{
    osal_s32 ret;
    osal_u32 buf_len;
    debug_ring_status_st debug_ring_status = {{VENDOR_FW_EVENT_RING_NAME}, 0, FW_EVENT_RING_ID,
        VENDOR_FW_EVENT_RING_SIZE, 0, 0, 0, 0
    };
    oal_netbuf_stru *skb;

    wal_record_wifi_external_log(WLAN_WIFI_VENDER_CMDS, __func__);
    buf_len = (osal_u32)OAL_SIZEOF(debug_ring_status_st);

    skb = oal_cfg80211_vendor_cmd_alloc_reply_skb_etc(wiphy, buf_len + 100);
    if (OAL_UNLIKELY(!skb)) {
        oam_warning_log1(0, OAM_SF_ANY, "uapi_cfgvendor_dbg_get_ring_status_etc::skb alloc failed.len %d\r\n",
                         buf_len + 100);
        return -OAL_ENOMEM;
    }

    oal_nla_put_u32(skb, DEBUG_ATTRIBUTE_RING_NUM, 1);
    oal_nla_put(skb, DEBUG_ATTRIBUTE_RING_STATUS, buf_len, &debug_ring_status);

    ret = oal_cfg80211_vendor_cmd_reply_etc(skb);
    if (OAL_UNLIKELY(ret)) {
        oam_error_log1(0, OAM_SF_ANY, "uapi_cfgvendor_dbg_get_ring_status_etc::Vendor Command reply failed:%d.", ret);
    }

    oam_warning_log1(0, OAM_SF_ANY, "uapi_cfgvendor_dbg_get_ring_status_etc::SUCCESS. driver version len %d", buf_len);
    return ret;
}

osal_s32 uapi_cfgvendor_dbg_trigger_mem_dump_etc(oal_wiphy_stru *wiphy,
    oal_wireless_dev_stru *wdev, OAL_CONST osal_void  *data, osal_s32 len)
{
    oam_warning_log0(0, OAM_SF_ANY, "uapi_cfgvendor_dbg_trigger_mem_dump_etc::ENTER");
    return 0;
}

OAL_STATIC osal_s32 uapi_cfgvendor_dbg_start_logging(oal_wiphy_stru *wiphy,
    oal_wireless_dev_stru *wdev, OAL_CONST osal_void  *data, osal_s32 len)
{
    oam_warning_log0(0, OAM_SF_ANY, "uapi_cfgvendor_dbg_start_logging::ENTER");
    return 0;
}

OAL_STATIC osal_s32 uapi_cfgvendor_dbg_get_ring_data(oal_wiphy_stru *wiphy,
    oal_wireless_dev_stru *wdev, OAL_CONST osal_void  *data, osal_s32 len)
{
    oam_warning_log0(0, OAM_SF_ANY, "uapi_cfgvendor_dbg_get_ring_data::ENTER");
    return 0;
}
#endif

/*****************************************************************************
 函 数 名  : uapi_cfgvendor_lstats_get_info
 功能描述  : 获取连接状态
*****************************************************************************/
OAL_STATIC osal_s32 uapi_cfgvendor_lstats_get_info(oal_wiphy_stru *wiphy,
    oal_wireless_dev_stru *wdev, OAL_CONST osal_void  *data, osal_s32 len)
{
    osal_s32            err = 0;
    osal_u32           reply_len;
    osal_void            *out_data;
    oal_netbuf_stru     *skb;
    wal_wifi_radio_stat_stru *radio_stat;
    wal_wifi_iface_stat_stru *iface_stat;
    wal_cfgvendor_radio_stat_stru *cfgvendor_radio_stat = wal_cfg80211_get_radio_stat();

    wal_record_wifi_external_log(WLAN_WIFI_VENDER_CMDS, __func__);
    if (wiphy == OAL_PTR_NULL || wdev == OAL_PTR_NULL) {
        oam_error_log2(0, OAM_SF_ANY,
                       "{uapi_cfgvendor_lstats_get_info:wiphy or wdev or data is null. %p, %p}",
                       (uintptr_t)wiphy, (uintptr_t)wdev);
        return -OAL_EFAUL;
    }

    reply_len = (osal_u32)(OAL_SIZEOF(*radio_stat) + OAL_SIZEOF(*iface_stat));
    out_data = (osal_void *)oal_memalloc(reply_len);
    if (out_data == OAL_PTR_NULL) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_cfgvendor_lstats_get_info:alloc memory fail.[%d]}", reply_len);
        return -OAL_ENOMEM;
    }
    memset_s(out_data, reply_len, 0, reply_len);

    /* ??radio ?? */
    radio_stat = (wal_wifi_radio_stat_stru *)out_data;
    radio_stat->num_channels = VENDOR_NUM_CHAN;
    radio_stat->on_time      =
        oal_jiffies_to_msecs(OAL_TIME_JIFFY - cfgvendor_radio_stat->ull_wifi_on_time_stamp);
    radio_stat->tx_time      = 0;
    radio_stat->rx_time      = 0;

    /* ??interfac ?? */
    iface_stat = (wal_wifi_iface_stat_stru *)(out_data + OAL_SIZEOF(*radio_stat));
    iface_stat->num_peers           = VENDOR_NUM_PEER;
    iface_stat->peer_info->num_rate = VENDOR_NUM_RATE;

    /* ??link ?? */
    skb = oal_cfg80211_vendor_cmd_alloc_reply_skb_etc(wiphy, reply_len);
    if (OAL_UNLIKELY(!skb)) {
        oal_free(out_data);
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_cfgvendor_lstats_get_info::skb alloc failed.len %d}", reply_len);
        return -OAL_ENOMEM;
    }

    oal_nla_put_nohdr(skb, reply_len, out_data);

    err =  oal_cfg80211_vendor_cmd_reply_etc(skb);
    oam_warning_log4(0, OAM_SF_ANY, "{uapi_cfgvendor_lstats_get_info::on_time %d, tx_time %d, rx_time %d, err %d",
                     radio_stat->on_time,
                     radio_stat->tx_time,
                     radio_stat->rx_time,
                     err);
    oal_free(out_data);
    return err;
}

OAL_STATIC OAL_CONST oal_wiphy_vendor_command_stru wal_vendor_cmds[] = {
    {
        {
            .vendor_id = OUI_GOOGLE,
            .subcmd = GSCAN_SUBCMD_GET_CHANNEL_LIST
        },
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 3, 0))
        .policy = VENDOR_CMD_RAW_DATA,
#endif
        .flags = WIPHY_VENDOR_CMD_NEED_WDEV | WIPHY_VENDOR_CMD_NEED_NETDEV,
        .doit = uapi_cfgvendor_get_channel_list
    },
    {
        {
            .vendor_id = OUI_GOOGLE,
            .subcmd = ANDR_WIFI_SET_COUNTRY
        },
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 3, 0))
        .policy = VENDOR_CMD_RAW_DATA,
#endif
        .flags = WIPHY_VENDOR_CMD_NEED_WDEV | WIPHY_VENDOR_CMD_NEED_NETDEV,
        .doit = uapi_cfgvendor_set_country
    },
    {
        {
            .vendor_id = OUI_GOOGLE,
            .subcmd = ANDR_WIFI_SUBCMD_GET_FEATURE_SET
        },
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 3, 0))
        .policy = VENDOR_CMD_RAW_DATA,
#endif
        .flags = WIPHY_VENDOR_CMD_NEED_WDEV | WIPHY_VENDOR_CMD_NEED_NETDEV,
        .doit = uapi_cfgvendor_get_feature_set
    },
    {
        {
            .vendor_id = OUI_GOOGLE,
            .subcmd = ANDR_WIFI_RANDOM_MAC_OUI
        },
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 3, 0))
        .policy = VENDOR_CMD_RAW_DATA,
#endif
        .flags = WIPHY_VENDOR_CMD_NEED_WDEV | WIPHY_VENDOR_CMD_NEED_NETDEV,
        .doit = uapi_cfgvendor_set_random_mac_oui
    },
    {
        {
            .vendor_id = OUI_GOOGLE,
            .subcmd = WIFI_SUBCMD_SET_BSSID_BLACKLIST
        },
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 3, 0))
        .policy = VENDOR_CMD_RAW_DATA,
#endif
        .flags = WIPHY_VENDOR_CMD_NEED_WDEV | WIPHY_VENDOR_CMD_NEED_NETDEV,
        .doit = uapi_cfgvendor_set_bssid_blacklist
    },
#ifdef _PRE_WIFI_DEBUG
    {
        {
            .vendor_id = OUI_GOOGLE,
            .subcmd = DEBUG_GET_FEATURE
        },
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 3, 0))
        .policy = VENDOR_CMD_RAW_DATA,
#endif
        .flags = WIPHY_VENDOR_CMD_NEED_WDEV | WIPHY_VENDOR_CMD_NEED_NETDEV,
        .doit = uapi_cfgvendor_dbg_get_feature
    },
    {
        {
            .vendor_id = OUI_GOOGLE,
            .subcmd = DEBUG_GET_VER
        },
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 3, 0))
        .policy = VENDOR_CMD_RAW_DATA,
#endif
        .flags = WIPHY_VENDOR_CMD_NEED_WDEV | WIPHY_VENDOR_CMD_NEED_NETDEV,
        .doit = uapi_cfgvendor_dbg_get_version
    },
    {
        {
            .vendor_id = OUI_GOOGLE,
            .subcmd = DEBUG_GET_RING_STATUS
        },
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 3, 0))
        .policy = VENDOR_CMD_RAW_DATA,
#endif
        .flags = WIPHY_VENDOR_CMD_NEED_WDEV | WIPHY_VENDOR_CMD_NEED_NETDEV,
        .doit = uapi_cfgvendor_dbg_get_ring_status_etc
    },
    {
        {
            .vendor_id = OUI_GOOGLE,
            .subcmd = DEBUG_TRIGGER_MEM_DUMP
        },
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 3, 0))
        .policy = VENDOR_CMD_RAW_DATA,
#endif
        .flags = WIPHY_VENDOR_CMD_NEED_WDEV | WIPHY_VENDOR_CMD_NEED_NETDEV,
        .doit = uapi_cfgvendor_dbg_trigger_mem_dump_etc
    },
    {
        {
            .vendor_id = OUI_GOOGLE,
            .subcmd = DEBUG_START_LOGGING
        },
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 3, 0))
        .policy = VENDOR_CMD_RAW_DATA,
#endif
        .flags = WIPHY_VENDOR_CMD_NEED_WDEV | WIPHY_VENDOR_CMD_NEED_NETDEV,
        .doit = uapi_cfgvendor_dbg_start_logging
    },
    {
        {
            .vendor_id = OUI_GOOGLE,
            .subcmd = DEBUG_GET_RING_DATA
        },
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 3, 0))
        .policy = VENDOR_CMD_RAW_DATA,
#endif
        .flags = WIPHY_VENDOR_CMD_NEED_WDEV | WIPHY_VENDOR_CMD_NEED_NETDEV,
        .doit = uapi_cfgvendor_dbg_get_ring_data
    },
#endif
    {
        {
            .vendor_id = OUI_GOOGLE,
            .subcmd = LSTATS_SUBCMD_GET_INFO
        },
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 3, 0))
        .policy = VENDOR_CMD_RAW_DATA,
#endif
        .flags = WIPHY_VENDOR_CMD_NEED_WDEV | WIPHY_VENDOR_CMD_NEED_NETDEV,
        .doit = uapi_cfgvendor_lstats_get_info
    },
};

OAL_STATIC OAL_CONST oal_nl80211_vendor_cmd_info_stru wal_vendor_events[] = {
    { OUI_SOC, EXT_VENDOR_EVENT_UNSPEC },
    { OUI_SOC, EXT_VENDOR_EVENT_PRIV_STR },
    { OUI_QCA, QCA_VENDOR_SUBCMD_NAN },
    { OUI_QCA, QCA_VENDOR_SUBCMD_NDP }
};

osal_void wal_cfgvendor_init_etc(oal_wiphy_stru *wiphy)
{
    wiphy->vendor_commands  = wal_vendor_cmds;
    wiphy->n_vendor_commands = (osal_u8)OAL_ARRAY_SIZE(wal_vendor_cmds);
    wiphy->vendor_events    = wal_vendor_events;
    wiphy->n_vendor_events  = (osal_u8)OAL_ARRAY_SIZE(wal_vendor_events);
}

osal_void wal_cfgvendor_deinit_etc(oal_wiphy_stru *wiphy)
{
    wiphy->vendor_commands  = NULL;
    wiphy->vendor_events    = NULL;
    wiphy->n_vendor_commands = 0;
    wiphy->n_vendor_events  = 0;
}

/* 获取 wal_vendor_events 结构体数组中 指定id和subcmd的索引 */
osal_u32 wal_cfgvendor_get_event_index(osal_u32 vendor_id, osal_u32 subcmd, osal_u32 *index)
{
    osal_u32 i;
    osal_u32 array_size = (osal_u32)osal_array_size(wal_vendor_events);

    if (index == OSAL_NULL) {
        return OAL_FAIL;
    }

    for (i = 0; i < array_size; i++) {
        if ((wal_vendor_events[i].vendor_id == vendor_id) &&
            (wal_vendor_events[i].subcmd == subcmd)) {
            *index = i;
            return OAL_SUCC;
        }
    }

    oam_error_log2(0, OAM_SF_ANY, "{wal_cfgvendor_get_event_index::not find id[%d] subcmd[%d].}", vendor_id, subcmd);
    return OAL_FAIL;
}

/*
* 函数功能: event机制具体实现
*/
osal_s32 wal_cfgvendor_nan_event_report(oal_wiphy_stru *wiphy, oal_wireless_dev_stru *wdev,
    osal_u32 rsp_len, const void *data)
{
    osal_u32 ret;
    osal_u32 index = 0;
    osal_u32 mem_needed;
    oal_netbuf_stru *skb = OSAL_NULL;
    oal_nlattr_stru *vendor_attr = OSAL_NULL;

    /* 内核会对skb进行对齐操作, 要预留空间, 2表示两倍 */
    mem_needed = rsp_len + VENDOR_REPLY_OVERHEAD + (ATTRIBUTE_U32_LEN * 2);

    /* 获取{ OUI_QCA, QCA_VENDOR_SUBCMD_NAN } 在wal_vendor_events 数组中的索引 */
    ret = wal_cfgvendor_get_event_index(OUI_QCA, QCA_VENDOR_SUBCMD_NAN, &index);
    if (ret != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_SDP, "call wal_cfgvendor_get_event_index fail.}");
        return -OAL_EFAIL;
    }

    skb = oal_cfg80211_vendor_event_alloc(wiphy, wdev, mem_needed, index);
    if (skb == OSAL_NULL) {
        oam_error_log2(0, OAM_SF_SDP, "wal_cfgvendor_nan_event_report::skb alloc failed, rsp_len %u mem_needed %u.\r\n",
            rsp_len, mem_needed);
        return -OAL_ENOMEM;
    }

    vendor_attr = oal_nla_nest_start(skb, QCA_WLAN_VENDOR_ATTR_NAN);
    if (osal_unlikely(!vendor_attr)) {
        oam_error_log2(0, OAM_SF_SDP, "nan_event_report::vendor_attr is null, len %u mem %u.\r\n", rsp_len, mem_needed);

        /* 释放上面申请的skb */
        kfree_skb(skb);
        return -OAL_ENOMEM;
    }

    /* int nla_put_nohdr(struct sk_buff *skb, int attrlen, const void *data)
       第三个参数的原型就是 void类型指针 */
    oal_nla_put_nohdr(skb, rsp_len, data);
    oal_nla_nest_end(skb, vendor_attr);

    oal_cfg80211_vendor_event(skb);

    return OAL_SUCC;
}

wal_cfgvendor_radio_stat_stru *wal_cfg80211_get_radio_stat(void)
{
    return &g_st_wifi_radio_stat_etc;
}

#endif  /* defined (_PRE_PRODUCT_ID_HOST) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3,14,0))) */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

