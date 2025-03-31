/*
 * Copyright (c) CompanyNameMagicTag 2014-2023. All rights reserved.
 * Description: oal_cfg80211.c 的头文件
 * Create: 2014年5月9日
 */

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "soc_diag_wdk.h"
#include "oal_net.h"
#include "oal_netbuf_ext.h"
#include "oal_netbuf.h"
#include "oal_cfg80211.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_OAL_CFG80211_C

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/

#if defined(_PRE_PRODUCT_ID_HOST)
#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
static oal_kobj_uevent_env_stru env;
#endif
#endif

/*****************************************************************************
  3 函数实现
*****************************************************************************/
/*****************************************************************************
 函 数 名  : oal_cfg80211_ready_on_channel_etc
 功能描述  : 上报linux 内核已经处于指定信道
*****************************************************************************/
osal_void oal_cfg80211_ready_on_channel_etc(oal_wireless_dev_stru       *wdev, osal_u64             ull_cookie,
    oal_ieee80211_channel_stru *chan, osal_u32                 duration, oal_gfp_enum_uint8     gfp)
{
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 44))
    cfg80211_ready_on_channel(wdev, ull_cookie, chan, duration, gfp);
#endif
}

/*****************************************************************************
 函 数 名  : oal_cfg80211_remain_on_channel_expired_etc
 功能描述  : 监听超时上报
*****************************************************************************/
osal_void oal_cfg80211_remain_on_channel_expired_etc(oal_wireless_dev_stru        *wdev,
    osal_u64                   ull_cookie, oal_ieee80211_channel_stru  *listen_channel,
    oal_gfp_enum_uint8           gfp)
{
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 44))
    cfg80211_remain_on_channel_expired(wdev, ull_cookie, listen_channel, GFP_ATOMIC);
#endif
}

osal_void oal_cfg80211_mgmt_tx_status_etc(struct wireless_dev *wdev, osal_u64 cookie,
    OAL_CONST osal_u8 *buf, size_t len, oal_bool_enum_uint8 ack, oal_gfp_enum_uint8 gfp)
{
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 44))
    cfg80211_mgmt_tx_status(wdev, cookie, buf, len, ack, gfp);
#endif
}

#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
extern struct genl_family nl80211_fam;
extern struct genl_multicast_group nl80211_mlme_mcgrp;
#define NL80211_FAM     (&nl80211_fam)
#define NL80211_GID     (nl80211_mlme_mcgrp.id)

#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,34))
#else
/*****************************************************************************
 函 数 名  : oal_cfg80211_calculate_bitrate
 功能描述  : oal_cfg80211_new_sta上报new sta事件获取比特率值(参考内核实现)
*****************************************************************************/
OAL_STATIC osal_s32 oal_cfg80211_calculate_bitrate(oal_rate_info_stru *rate)
{
    osal_s32  l_modulation;
    osal_s32  l_streams;
    osal_s32  l_bitrate;

    if (0 == (rate->flags & RATE_INFO_FLAGS_MCS)) {
        return rate->legacy;
    }

    /* the formula below does only work for MCS values smaller than 32 */
    if (rate->mcs >= 32) {
        return -OAL_ERR_CODE_CFG80211_MCS_EXCEED;
    }
    l_modulation = rate->mcs & 7; /* 7 */
    l_streams = (rate->mcs >> 3) + 1; /* 3 1 */

    l_bitrate = (rate->flags & RATE_INFO_FLAGS_40_MHZ_WIDTH) ? 13500000 : 6500000; /* 13500000 6500000 */

    if (l_modulation < 4) { /* 4 */
        l_bitrate *= (l_modulation + 1); /* 1 */
    } else if (l_modulation == 4) { /* 4 */
        l_bitrate *= (l_modulation + 2); /* 2 */
    } else {
        l_bitrate *= (l_modulation + 3); /* 3 */
    }
    l_bitrate *= l_streams;

    if (rate->flags & RATE_INFO_FLAGS_SHORT_GI) {
        l_bitrate = (l_bitrate / 9) * 10; /* 9 10 */
    }
    /* do NOT round down here */
    return (l_bitrate + 50000) / 100000; /* 50000 100000 */
}

/*****************************************************************************
 函 数 名  : oal_nl80211_send_station
 功能描述  : netlink上报send new sta事件进行命令符号和属性值填充
*****************************************************************************/
OAL_STATIC osal_u32 oal_nl80211_send_station(oal_netbuf_stru *buf, osal_u32 pid, osal_u32 seq,
    osal_s32 l_flags, oal_net_device_stru *net_dev, const osal_u8 *mac_addr,
    oal_station_info_stru *station_info)
{
    oal_nlattr_stru   *sinfoattr = OAL_PTR_NULL;
    oal_nlattr_stru   *txrate    = OAL_PTR_NULL;
    osal_void          *p_hdr         = OAL_PTR_NULL;
    osal_s32          bitrate;

    /* Add generic netlink header to netlink message, returns pointer to user specific header */
    p_hdr = oal_genlmsg_put(buf, pid, seq, NL80211_FAM, l_flags, PRIV_NL80211_CMD_NEW_STATION);
    if (OAL_PTR_NULL == p_hdr) {
        return OAL_ERR_CODE_CFG80211_SKB_MEM_FAIL;
    }

    oal_nla_put_u32(buf, PRIV_NL80211_ATTR_IFINDEX, net_dev->ifindex);
    OAL_NLA_PUT(buf, PRIV_NL80211_ATTR_MAC, OAL_ETH_ALEN_SIZE, mac_addr);

    /*
        linux-2.6.30内核station_info没有generation成员
        NLA_PUT_U32(buf, PRIV_NL80211_ATTR_GENERATION, station_info->generation);
    */

    sinfoattr = oal_nla_nest_start(buf, PRIV_NL80211_ATTR_STA_INFO);
    if (OAL_PTR_NULL == sinfoattr) {
        oal_genlmsg_cancel(buf, p_hdr);
        return OAL_ERR_CODE_CFG80211_EMSGSIZE;
    }
    if (station_info->filled & STATION_INFO_INACTIVE_TIME) {
        oal_nla_put_u32(buf, PRIV_NL80211_STA_INFO_INACTIVE_TIME, station_info->inactive_time);
    }
    if (station_info->filled & STATION_INFO_RX_BYTES) {
        oal_nla_put_u32(buf, PRIV_NL80211_STA_INFO_RX_BYTES, station_info->rx_bytes);
    }
    if (station_info->filled & STATION_INFO_TX_BYTES) {
        oal_nla_put_u32(buf, PRIV_NL80211_STA_INFO_TX_BYTES, station_info->tx_bytes);
    }
    if (station_info->filled & STATION_INFO_LLID) {
        oal_nla_put_u16(buf, PRIV_NL80211_STA_INFO_LLID, station_info->llid);
    }
    if (station_info->filled & STATION_INFO_PLID) {
        oal_nla_put_u16(buf, PRIV_NL80211_STA_INFO_PLID, station_info->plid);
    }
    if (station_info->filled & STATION_INFO_PLINK_STATE) {
        oal_nla_put_u8(buf, PRIV_NL80211_STA_INFO_PLINK_STATE, station_info->plink_state);
    }
    if (station_info->filled & STATION_INFO_SIGNAL) {
        oal_nla_put_u8(buf, PRIV_NL80211_STA_INFO_SIGNAL, station_info->signal);
    }
    if (station_info->filled & STATION_INFO_TX_BITRATE) {
        txrate = oal_nla_nest_start(buf, PRIV_NL80211_STA_INFO_TX_BITRATE);
        if (OAL_PTR_NULL == txrate) {
            oal_genlmsg_cancel(buf, p_hdr);
            return OAL_ERR_CODE_CFG80211_EMSGSIZE;
        }

        /* cfg80211_calculate_bitrate will return negative for mcs >= 32 */
        bitrate = oal_cfg80211_calculate_bitrate(&station_info->txrate);
        if (bitrate > 0)
            oal_nla_put_u16(buf, PRIV_NL80211_RATE_INFO_BITRATE, bitrate);
        if (station_info->txrate.flags & RATE_INFO_FLAGS_MCS)
            oal_nla_put_u8(buf, PRIV_NL80211_RATE_INFO_MCS, station_info->txrate.mcs);
        if (station_info->txrate.flags & RATE_INFO_FLAGS_40_MHZ_WIDTH)
            oal_nla_put_flag(buf, PRIV_NL80211_RATE_INFO_40_MHZ_WIDTH);
        if (station_info->txrate.flags & RATE_INFO_FLAGS_SHORT_GI)
            oal_nla_put_flag(buf, PRIV_NL80211_RATE_INFO_SHORT_GI);

        oal_nla_nest_end(buf, txrate);
    }
    if (station_info->filled & STATION_INFO_RX_PACKETS) {
        oal_nla_put_u32(buf, PRIV_NL80211_STA_INFO_RX_PACKETS, station_info->rx_packets);
    }
    if (station_info->filled & STATION_INFO_TX_PACKETS) {
        oal_nla_put_u32(buf, PRIV_NL80211_STA_INFO_TX_PACKETS, station_info->tx_packets);
    }

    oal_nla_nest_end(buf, sinfoattr);

    if (oal_genlmsg_end(buf, p_hdr) < 0) {
        return OAL_ERR_CODE_CFG80211_ENOBUFS;
    }

        return OAL_SUCC;

nla_put_failure:
    oal_genlmsg_cancel(buf, p_hdr);
    return OAL_ERR_CODE_CFG80211_EMSGSIZE;
}

/*****************************************************************************
 函 数 名  : oal_nl80211_send_connect_result
 功能描述  : 驱动调用内核netlink接口上报关联结构
*****************************************************************************/
OAL_STATIC osal_u32 oal_nl80211_send_connect_result(oal_netbuf_stru             *buf,
    oal_net_device_stru         *net_device, const osal_u8             *bssid,
    const osal_u8             *req_ie, osal_u32             req_ie_len,
    const osal_u8             *resp_ie, osal_u32             resp_ie_len, osal_u16             status,
    oal_gfp_enum_uint8           gfp)
{
    osal_void         *p_hdr = OAL_PTR_NULL;
    osal_s32         let;

    p_hdr = oal_genlmsg_put(buf, 0, 0, NL80211_FAM, 0, PRIV_NL80211_CMD_CONNECT);
    if (OAL_PTR_NULL == p_hdr) {
        oal_nlmsg_free(buf);
        return OAL_ERR_CODE_CFG80211_SKB_MEM_FAIL;
    }

    /*
        这个属性上层没有解析，可以不上报
        oal_nla_put_u32(buf, PRIV_NL80211_ATTR_WIPHY, net_device->wiphy_idx);
    */
    oal_nla_put_u32(buf, PRIV_NL80211_ATTR_IFINDEX, net_device->ifindex);
    if (OAL_PTR_NULL != bssid) {
        OAL_NLA_PUT(buf, PRIV_NL80211_ATTR_MAC, OAL_ETH_ALEN_SIZE, bssid);
    }
    oal_nla_put_u16(buf, PRIV_NL80211_ATTR_STATUS_CODE, status);
    if (OAL_PTR_NULL != req_ie) {
        OAL_NLA_PUT(buf, PRIV_NL80211_ATTR_REQ_IE, req_ie_len, req_ie);
    }
    if (OAL_PTR_NULL != resp_ie) {
        OAL_NLA_PUT(buf, PRIV_NL80211_ATTR_RESP_IE, resp_ie_len, resp_ie);
    }

    if (oal_genlmsg_end(buf, p_hdr) < 0) {
        oal_nlmsg_free(buf);
        return OAL_ERR_CODE_CFG80211_ENOBUFS;
    }

    let = oal_genlmsg_multicast(NL80211_FAM, buf, 0, NL80211_GID, gfp);
    if (let < 0) {
        /* 如果不加载hostapd和wpa_supplicant的话，这个也会失败，这里报fail，影响使用，去掉报错 */
        return OAL_FAIL;
    }

    return OAL_SUCC;

 nla_put_failure:
    oal_genlmsg_cancel(buf, p_hdr);
    oal_nlmsg_free(buf);
    return OAL_ERR_CODE_CFG80211_EMSGSIZE;
}

/*****************************************************************************
 函 数 名  : oal_nl80211_send_disconnected
*****************************************************************************/
OAL_STATIC osal_u32 oal_nl80211_send_disconnected(oal_net_device_stru *net_device, osal_u16           reason,
    const osal_u8     *ie, osal_u32           ie_len, oal_bool_enum_uint8  from_ap,
    oal_gfp_enum_uint8   gfp)
{
    oal_netbuf_stru   *msg = OAL_PTR_NULL;
    osal_void          *p_hdr  = OAL_PTR_NULL;
    osal_s32          let;

    msg = oal_nlmsg_new(OAL_NLMSG_GOODSIZE, gfp);
    if (OAL_PTR_NULL == msg) {
        return OAL_ERR_CODE_CFG80211_ENOBUFS;
    }

    p_hdr = oal_genlmsg_put(msg, 0, 0, NL80211_FAM, 0, PRIV_NL80211_CMD_DISCONNECT);
    if (OAL_PTR_NULL == p_hdr) {
        oal_nlmsg_free(msg);
        return OAL_ERR_CODE_CFG80211_SKB_MEM_FAIL;
    }

/*
    这个属性上层没有解析，可以不上报
    oal_nla_put_u32(msg, PRIV_NL80211_ATTR_WIPHY, net_device->wiphy_idx);
*/

    oal_nla_put_u32(msg, PRIV_NL80211_ATTR_IFINDEX, net_device->ifindex);
    if (from_ap && reason)
        oal_nla_put_u16(msg, PRIV_NL80211_ATTR_REASON_CODE, reason);
    if (from_ap)
        oal_nla_put_flag(msg, PRIV_NL80211_ATTR_DISCONNECTED_BY_AP);
    if (OAL_PTR_NULL == ie)
        OAL_NLA_PUT(msg, PRIV_NL80211_ATTR_IE, ie_len, ie);

    if (oal_genlmsg_end(msg, p_hdr) < 0) {
        oal_nlmsg_free(msg);
        return OAL_ERR_CODE_CFG80211_ENOBUFS;
    }

    let = oal_genlmsg_multicast(NL80211_FAM, msg, 0, NL80211_GID, gfp);
    if (let < 0) {
        /* oal_genlmsg_multicast接口内部会释放skb，返回失败不需要手动释放 */
        return OAL_FAIL;
    }

    return OAL_SUCC;

 nla_put_failure:
    oal_genlmsg_cancel(msg, p_hdr);
    oal_nlmsg_free(msg);
    return OAL_ERR_CODE_CFG80211_EMSGSIZE;
}
#endif
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 9, 44))
#else
/*****************************************************************************
 函 数 名  : oal_nl80211_send_mgmt
*****************************************************************************/
OAL_STATIC osal_u32 oal_nl80211_send_mgmt(oal_cfg80211_registered_device_stru *rdev,
    oal_net_device_stru *netdev, osal_s32 l_freq, osal_u8 rssi, const osal_u8 *buf, osal_u32 len,
    oal_gfp_enum_uint8 gfp)
{
    oal_netbuf_stru   *msg = OAL_PTR_NULL;
    osal_void          *p_hdr   = OAL_PTR_NULL;
    osal_s32          l_let;

    msg = oal_nlmsg_new(OAL_NLMSG_DEFAULT_SIZE, gfp);
    if (OAL_PTR_NULL == msg) {
        return OAL_ERR_CODE_CFG80211_ENOBUFS;
    }

    p_hdr = oal_genlmsg_put(msg, 0, 0, NL80211_FAM, 0, PRIV_NL80211_CMD_ACTION);
    if (OAL_PTR_NULL == p_hdr) {
        oal_nlmsg_free(msg);
        return OAL_ERR_CODE_CFG80211_SKB_MEM_FAIL;
    }
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE < KERNEL_VERSION(3,10,44))
    l_let |= oal_nla_put_u32(msg, PRIV_NL80211_ATTR_WIPHY, rdev->wiphy_idx);
    l_let |= oal_nla_put_u32(msg, PRIV_NL80211_ATTR_IFINDEX, netdev->ifindex);
    l_let |= oal_nla_put_u32(msg, PRIV_NL80211_ATTR_WIPHY_FREQ, l_freq);
    l_let |= oal_nla_put_u32(msg, PRIV_NL80211_ATTR_RX_SIGNAL_DBM, rssi);  // report rssi to hostapd
    l_let |= oal_nla_put(msg, PRIV_NL80211_ATTR_FRAME, len, buf);
    if (l_let != OAL_SUCC) {
        goto nla_put_failure;
    }
#else
    nla_put(msg, PRIV_NL80211_ATTR_WIPHY, OAL_SIZEOF(rdev->wiphy_idx), &(rdev->wiphy_idx));
    nla_put(msg, PRIV_NL80211_ATTR_IFINDEX, OAL_SIZEOF(netdev->ifindex), &(netdev->ifindex));
    nla_put(msg, PRIV_NL80211_ATTR_WIPHY_FREQ, OAL_SIZEOF(l_freq), &l_freq);
    nla_put(msg, PRIV_NL80211_ATTR_RX_SIGNAL_DBM, OAL_SIZEOF(rssi), &rssi);
    nla_put(msg, PRIV_NL80211_ATTR_FRAME, len, buf);
#endif

    if (oal_genlmsg_end(msg, p_hdr) < 0) {
        oal_nlmsg_free(msg);
        return OAL_ERR_CODE_CFG80211_ENOBUFS;
    }

    l_let = oal_genlmsg_multicast(NL80211_FAM, msg, 0, NL80211_GID, gfp);
    if (l_let < 0) {
        return OAL_FAIL;
    }

    return OAL_SUCC;

#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE < KERNEL_VERSION(3,10,44))
nla_put_failure:
    oal_genlmsg_cancel(msg, p_hdr);
    oal_nlmsg_free(msg);

    return OAL_ERR_CODE_CFG80211_EMSGSIZE;
#endif
}

#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE < KERNEL_VERSION(3,10,44))
/*****************************************************************************
 函 数 名  : oal_nl80211_send_cac_msg
 功能描述  : netlink上报cac事件进行命令符号和属性值填充
*****************************************************************************/
OAL_STATIC osal_s32 oal_nl80211_send_cac_msg(oal_netbuf_stru *buf, oal_net_device_stru *net_dev,
    osal_u32      freq, enum PRIV_NL80211_CHAN_WIDTH  chan_width, enum PRIV_NL80211_RADAR_EVENT event)
{
    osal_void          *p_hdr         = OAL_PTR_NULL;

    /* Add generic netlink header to netlink message, returns pointer to user specific header */
    p_hdr = oal_genlmsg_put(buf, 0, 0, NL80211_FAM, 0,  PRIV_NL80211_CMD_RADAR_DETECT);
    if (OAL_PTR_NULL == p_hdr) {
        return OAL_ERR_CODE_CFG80211_SKB_MEM_FAIL;
    }

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,10,44))
        if (net_dev) {
            if (oal_nla_put_u32(buf, PRIV_NL80211_ATTR_IFINDEX, net_dev->ifindex) != OAL_SUCC) {
                goto nla_put_failure;
            }
        }
        if (oal_nla_put_u32(buf, PRIV_NL80211_ATTR_RADAR_EVENT, event) != OAL_SUCC) {
            goto nla_put_failure;
        }
        if (oal_nla_put_u32(buf, PRIV_NL80211_ATTR_WIPHY_FREQ, freq) != OAL_SUCC) {
            goto nla_put_failure;
        }
        if (oal_nla_put_u32(buf, PRIV_NL80211_ATTR_CHANNEL_WIDTH, chan_width) != OAL_SUCC) {
            goto nla_put_failure;
        }
#else
        if (net_dev) {
            if (nla_put(buf, PRIV_NL80211_ATTR_IFINDEX, OAL_SIZEOF(net_dev->ifindex), &(net_dev->ifindex))) {
                goto nla_put_failure;
            }
        }

        if (nla_put(buf, PRIV_NL80211_ATTR_RADAR_EVENT, OAL_SIZEOF(event), &event)) {
            goto nla_put_failure;
        }

        if (nla_put(buf, PRIV_NL80211_ATTR_WIPHY_FREQ, OAL_SIZEOF(freq), &freq)) {
            goto nla_put_failure;
        }

        if (nla_put(buf, PRIV_NL80211_ATTR_CHANNEL_WIDTH, OAL_SIZEOF(chan_width), &chan_width)) {
            goto nla_put_failure;
        }
#endif
    // 由于我们的驱动是WPA_DRIVER_FLAGS_DFS_OFFLOAD，所以下面的这些属性上报没用
    // 暂时不上报，后面有需求再迭代此函数
    // NL80211_ATTR_WIPHY_CHANNEL_TYPE:
    // NL80211_ATTR_CENTER_FREQ1:
    // NL80211_ATTR_CENTER_FREQ2:
    if (oal_genlmsg_end(buf, p_hdr) < 0) {
        return OAL_ERR_CODE_CFG80211_ENOBUFS;
    }

    return OAL_SUCC;

nla_put_failure:
    oal_genlmsg_cancel(buf, p_hdr);
    return OAL_ERR_CODE_CFG80211_EMSGSIZE;
}
#endif
#endif

/*****************************************************************************
 函 数 名  : oal_cfg80211_notify_cac_event
 功能描述  : 上报CAC事件
*****************************************************************************/
osal_u32 oal_cfg80211_notify_cac_event(oal_net_device_stru     *net_device, osal_u32      freq,
    enum PRIV_NL80211_CHAN_WIDTH  chan_width, enum PRIV_NL80211_RADAR_EVENT event, oal_gfp_enum_uint8 gfp)
{
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 38))
    oal_netbuf_stru   *msg = OAL_PTR_NULL;
    osal_s32          l_let;

    /* 分配一个新的netlink消息 */
    msg = oal_nlmsg_new(OAL_NLMSG_GOODSIZE, gfp);
    if (OAL_PTR_NULL == msg) {
        return OAL_ERR_CODE_CFG80211_ENOBUFS;
    }

    l_let = oal_nl80211_send_cac_msg(msg, net_device, freq, chan_width, event);
    if (OAL_SUCC != l_let) {
        oal_nlmsg_free(msg);
        return OAL_FAIL;
    }

    /* 调用封装的内核netlink广播发送函数，发送成功返回0，失败为负值 */
    l_let = oal_genlmsg_multicast(NL80211_FAM, msg, 0, NL80211_GID, gfp);
    if (l_let < 0) {
        return OAL_FAIL;
    }

    return OAL_SUCC;
#else
    return OAL_FAIL;
#endif
}

#ifdef _PRE_WLAN_FEATURE_PNO_SCAN
/*****************************************************************************
 函 数 名  : oal_cfg80211_sched_scan_result_etc
 功能描述  : 上报调度扫描结果
*****************************************************************************/
osal_void  oal_cfg80211_sched_scan_result_etc(oal_wiphy_stru *wiphy)
{
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4,14,0))
    cfg80211_sched_scan_results(wiphy);
    return;
#else
    cfg80211_sched_scan_results(wiphy, 0);
    /* 51不支持，do nothing */
    return;
#endif
}
#endif

#if defined(_PRE_PRODUCT_ID_HOST)
osal_void oal_kobject_uevent_env_sta_join_etc(oal_net_device_stru *net_device, const osal_u8 *mac_addr)
{
    (osal_void)memset_s(&env, sizeof(env), 0, sizeof(env));
    /* Android上层需要STA_JOIN和mac地址，中间参数无效，但是必须是4个参数 */
    add_uevent_var(&env, "SOFTAP=STA_JOIN wlan0 wlan0 %02x:%02x:%02x:%02x:xx:xx", mac_addr[0], mac_addr[1],
        mac_addr[2], mac_addr[3]); /* mac 2,3,4,5 */
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(4,1,0))
    kobject_uevent_env(&(net_device->dev.kobj), KOBJ_CHANGE, env.envp);
#else
    kobject_uevent_env(&(net_device->dev.kobj), KOBJ_CHANGE, (char**)&env);
#endif
}

osal_void oal_kobject_uevent_env_sta_leave_etc(oal_net_device_stru *net_device, const osal_u8 *mac_addr)
{
    (osal_void)memset_s(&env, sizeof(env), 0, sizeof(env));
    /* Android上层需要STA_LEAVE和mac地址，中间参数无效，但是必须是4个参数 */
    add_uevent_var(&env, "SOFTAP=STA_LEAVE wlan0 wlan0 %02x:%02x:%02x:%02x:xx:xx", mac_addr[0], mac_addr[1],
        mac_addr[2], mac_addr[3]); /* mac 2,3,4,5 */
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(4,1,0))
    kobject_uevent_env(&(net_device->dev.kobj), KOBJ_CHANGE, env.envp);
#else
    kobject_uevent_env(&(net_device->dev.kobj), KOBJ_CHANGE, (char**)&env);
#endif
}
#endif

/*****************************************************************************
 函 数 名  : oal_cfg80211_put_bss_etc
*****************************************************************************/
osal_void  oal_cfg80211_put_bss_etc(oal_wiphy_stru *wiphy, oal_cfg80211_bss_stru *cfg80211_bss)
{
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3,9,0))
    cfg80211_put_bss(wiphy, cfg80211_bss);
#else
    cfg80211_put_bss(cfg80211_bss);
#endif
}

/*****************************************************************************
 函 数 名  : oal_cfg80211_get_bss_etc
 功能描述  : 根据bssid 和ssid 查找内核保存的bss 信息
*****************************************************************************/
oal_cfg80211_bss_stru *oal_cfg80211_get_bss_etc(oal_wiphy_stru *wiphy, oal_ieee80211_channel_stru *channel,
    osal_u8 *bssid, osal_u8 *ssid, osal_u32 ssid_len)
{
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(4,1,0))
    return cfg80211_get_bss(wiphy, channel, bssid, ssid, ssid_len,
                            IEEE80211_BSS_TYPE_ANY, IEEE80211_PRIVACY_ANY);
#else
    return cfg80211_get_bss(wiphy, channel, bssid, ssid, ssid_len,
                            WLAN_CAPABILITY_ESS, WLAN_CAPABILITY_ESS);
#endif
}

/*****************************************************************************
 函 数 名  : oal_cfg80211_inform_bss_frame_etc
*****************************************************************************/
oal_cfg80211_bss_stru *oal_cfg80211_inform_bss_frame_etc(oal_wiphy_stru              *wiphy,
    oal_ieee80211_channel_stru  *ieee80211_channel, oal_ieee80211_mgmt_stru     *mgmt,
    osal_u32                   len, osal_s32                    l_signal, oal_gfp_enum_uint8           ftp)
{
    return cfg80211_inform_bss_frame(wiphy, ieee80211_channel, mgmt, len, l_signal, ftp);
}

/*****************************************************************************
 函 数 名  : oal_cfg80211_scan_done_etc
 功能描述  : 上报扫描完成结果
*****************************************************************************/
osal_void  oal_cfg80211_scan_done_etc(oal_cfg80211_scan_request_stru *cfg80211_scan_request, osal_s8 c_aborted)
{
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(4,8,0))
    struct cfg80211_scan_info info = {0};
    info.aborted = c_aborted;
    cfg80211_scan_done(cfg80211_scan_request, &info);
#else
    cfg80211_scan_done(cfg80211_scan_request, c_aborted);
#endif
}


/*****************************************************************************
 函 数 名  : oal_cfg80211_connect_result_etc
 功能描述  : STA上报给关联结果结构体
*****************************************************************************/
osal_u32 oal_cfg80211_connect_result_etc(oal_net_device_stru         *net_device,
    const osal_u8             *bssid, const osal_u8             *req_ie,
    osal_u32             req_ie_len, const osal_u8             *resp_ie,
    osal_u32             resp_ie_len, osal_u16             status, oal_gfp_enum_uint8           gfp)
{
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,34))
    cfg80211_connect_result(net_device, bssid, req_ie, req_ie_len,
                            resp_ie, resp_ie_len, status, gfp);

    return OAL_SUCC;
#else
    oal_netbuf_stru         *msg  = OAL_PTR_NULL;
    oal_wireless_dev_stru   *wdev = OAL_PTR_NULL;

    /* 分配一个新的netlink消息 */
    msg = oal_nlmsg_new(OAL_NLMSG_GOODSIZE, gfp);
    if (OAL_PTR_NULL == msg) {
        return OAL_ERR_CODE_CFG80211_ENOBUFS;
    }

    wdev = net_device->ieee80211_ptr;
    if (OAL_PTR_NULL == wdev) {
        oal_nlmsg_free(msg);
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (wdev->iftype != NL80211_IFTYPE_STATION) {
        oal_nlmsg_free(msg);
        return OAL_ERR_CODE_CONFIG_UNSUPPORT;
    }

    /* 如果不加载hostapd和wpa_supplicant的话，这个也会失败，这里报fail，影响使用，去掉报错
        成功的话，打印SUCC, 不成功的话，不打印 */
    return oal_nl80211_send_connect_result(msg, net_device, bssid, req_ie, req_ie_len, resp_ie,
        resp_ie_len, status, gfp);
#endif
}

/*****************************************************************************
 函 数 名  : oal_cfg80211_disconnected_etc
 功能描述  : STA上报给内核去关联结果
*****************************************************************************/
osal_u32 oal_cfg80211_disconnected_etc(oal_net_device_stru        *net_device, osal_u16 reason, osal_u8 *ie,
    osal_u32 ie_len, osal_bool locally_generated, oal_gfp_enum_uint8 gfp)
{
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(4,4,0))
    cfg80211_disconnected(net_device, reason, ie, ie_len, locally_generated, gfp);

    return OAL_SUCC;
#elif defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,34))
    cfg80211_disconnected(net_device, reason, ie, ie_len, gfp);

    return OAL_SUCC;
#else
    oal_wireless_dev_stru   *wdev = OAL_PTR_NULL;

    wdev = net_device->ieee80211_ptr;
    if (OAL_PTR_NULL == wdev) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (wdev->iftype != NL80211_IFTYPE_STATION) {
        return OAL_ERR_CODE_CONFIG_UNSUPPORT;
    }

    /*
     * If this disconnect was due to a disassoc, we
     * we might still have an auth BSS around. For
     * the userspace SME that's currently expected,
     * but for the kernel SME (nl80211 CONNECT or
     * wireless extensions) we want to clear up all
     * state.
     */
    return oal_nl80211_send_disconnected(net_device, reason, ie,
                                         ie_len, OAL_TRUE, gfp);
#endif
}

/*****************************************************************************
 函 数 名  : oal_cfg80211_roamed_etc
 功能描述  : STA上报给内核去关联结果
*****************************************************************************/
osal_u32 oal_cfg80211_roamed_etc(oal_net_device_stru *net_device, struct ieee80211_channel *channel,
    const osal_u8             *bssid, const osal_u8             *req_ie,
    osal_u32             req_ie_len, const osal_u8             *resp_ie,
    osal_u32             resp_ie_len, oal_gfp_enum_uint8           gfp)
{
#ifdef CONTROLLER_CUSTOMIZATION
    struct cfg80211_roam_info info = {
        .req_ie = req_ie,
        .req_ie_len = req_ie_len,
        .resp_ie = resp_ie,
        .resp_ie_len = resp_ie_len,
        };
    cfg80211_roamed(net_device, &info, gfp);
    return OAL_ERR_CODE_CONFIG_UNSUPPORT;
#else
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE < KERNEL_VERSION(4,14,0))
    cfg80211_roamed(net_device, channel, bssid,
                    req_ie, req_ie_len,
                    resp_ie, resp_ie_len, gfp);

    return OAL_SUCC;

#else
    struct cfg80211_roam_info info = {
        .links[0].bss = NULL,      // 使用 links[0].bss 替代 bss
        // .bssid = bssid,            // 如果 bssid 字段仍存在
        .req_ie = req_ie,
        .req_ie_len = req_ie_len,
        .resp_ie = resp_ie,
        .resp_ie_len = resp_ie_len,
    };
    cfg80211_roamed(net_device, &info, gfp);
    return OAL_ERR_CODE_CONFIG_UNSUPPORT;
#endif
#endif
}

/*****************************************************************************
 函 数 名  : oal_cfg80211_ft_event_etc
 功能描述  : STA上报给内核ft事件
*****************************************************************************/
osal_u32  oal_cfg80211_ft_event_etc(oal_net_device_stru *net_device, oal_cfg80211_ft_event_stru  *ft_event)
{
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0))
    cfg80211_ft_event(net_device, ft_event);
    return OAL_SUCC;

#else
    return OAL_ERR_CODE_CONFIG_UNSUPPORT;
#endif
}

/*****************************************************************************
 函 数 名  : cfg80211_new_sta
 功能描述  : AP上报新关联某个STA情况
*****************************************************************************/
osal_u32 oal_cfg80211_new_sta_etc(oal_net_device_stru     *net_device, const osal_u8         *mac_addr,
    oal_station_info_stru   *station_info, oal_gfp_enum_uint8      gfp)
{
#if defined(_PRE_PRODUCT_ID_HOST) && (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,34))
    oal_kobject_uevent_env_sta_join_etc(net_device, mac_addr);
#endif

#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,34))
    cfg80211_new_sta(net_device, mac_addr, station_info, gfp);

    return OAL_SUCC;
#else
    oal_netbuf_stru   *msg = OAL_PTR_NULL;
    osal_u32         ul_ret;
    osal_s32          l_let;

    /* 分配一个新的netlink消息 */
    msg = oal_nlmsg_new(OAL_NLMSG_GOODSIZE, gfp);
    if (OAL_PTR_NULL == msg) {
        return OAL_ERR_CODE_CFG80211_ENOBUFS;
    }

    ul_ret = oal_nl80211_send_station(msg, 0, 0, 0, net_device, mac_addr, station_info);
    if (OAL_SUCC != ul_ret) {
        oal_nlmsg_free(msg);
        return ul_ret;
    }

    /* 调用封装的内核netlink广播发送函数，发送成功返回0，失败为负值 */
    l_let = oal_genlmsg_multicast(NL80211_FAM, msg, 0, NL80211_GID, gfp);
    if (l_let < 0) {
        return OAL_FAIL;
    }

    return OAL_SUCC;
#endif
}

/*****************************************************************************
 函 数 名  : oal_cfg80211_mic_failure_etc
 功能描述  : 上报mic攻击
*****************************************************************************/
osal_void oal_cfg80211_mic_failure_etc(oal_net_device_stru     *net_device, const osal_u8         *mac_addr,
    enum nl80211_key_type    key_type, osal_s32                key_id, const osal_u8         *tsc,
    oal_gfp_enum_uint8       gfp)
{
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,34))
    cfg80211_michael_mic_failure(net_device, mac_addr, key_type, key_id, tsc, gfp);
#else
    oal_wireless_dev_stru               *wdev  = OAL_PTR_NULL;
    oal_cfg80211_registered_device_stru *rdev  = OAL_PTR_NULL;
    oal_netbuf_stru                     *msg   = OAL_PTR_NULL;
    osal_void                            *p_hdr     = OAL_PTR_NULL;
    osal_s32                            l_let;

    wdev = net_device->ieee80211_ptr;
    if (OAL_PTR_NULL == wdev) {
        return;
    }

    if (OAL_PTR_NULL == wdev->wiphy) {
        return;
    }
    rdev = oal_wiphy_to_dev(wdev->wiphy);

    msg = oal_nlmsg_new(OAL_NLMSG_DEFAULT_SIZE, gfp);
    if (OAL_PTR_NULL == msg) {
        return;
    }

    p_hdr = oal_genlmsg_put(msg, 0, 0, NL80211_FAM, 0, PRIV_NL80211_CMD_MICHAEL_MIC_FAILURE);
    if (OAL_PTR_NULL == p_hdr) {
        oal_nlmsg_free(msg);
        return;
    }

    /* rdev对应内核core.h中的cfg80211_registered_device结构体，这个属性在上层没有处理 */
    oal_nla_put_u32(msg, PRIV_NL80211_ATTR_WIPHY, rdev->wiphy_idx);
    oal_nla_put_u32(msg, PRIV_NL80211_ATTR_IFINDEX, net_device->ifindex);

    if (OAL_PTR_NULL != mac_addr) {
        OAL_NLA_PUT(msg, PRIV_NL80211_ATTR_MAC, OAL_ETH_ALEN_SIZE, mac_addr);
    }
    oal_nla_put_u32(msg, PRIV_NL80211_ATTR_KEY_TYPE, key_type);
    oal_nla_put_u8(msg, PRIV_NL80211_ATTR_KEY_IDX, key_id);
    if (OAL_PTR_NULL != tsc) {
        OAL_NLA_PUT(msg, PRIV_NL80211_ATTR_KEY_SEQ, 6, tsc); /* 6 */
    }

    if (oal_genlmsg_end(msg, p_hdr) < 0) {
        oal_nlmsg_free(msg);
        return;
    }

    l_let = oal_genlmsg_multicast(NL80211_FAM, msg, 0, NL80211_GID, gfp);
    if (l_let < 0) {
        return;
    }
    return;
#endif
}

/*****************************************************************************
 函 数 名  : oal_cfg80211_del_sta_etc
 功能描述  : AP上报去关联某个STA情况
*****************************************************************************/
osal_s32 oal_cfg80211_del_sta_etc(oal_net_device_stru *net_device, const osal_u8      *mac_addr,
    oal_gfp_enum_uint8    gfp)
{
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE < KERNEL_VERSION(3, 0, 0))
    oal_netbuf_stru  *msg = OAL_PTR_NULL;
    osal_void         *p_hdr   = OAL_PTR_NULL;
    osal_s32         l_let;
#endif
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,34)) && defined(_PRE_PRODUCT_ID_HOST)
    oal_kobject_uevent_env_sta_leave_etc(net_device, mac_addr);
#endif

#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 0, 0))
    cfg80211_del_sta(net_device, mac_addr, gfp);

    return OAL_SUCC;
#else

    /*
        内核2.6.34中没有内核3.x版本中 cfg80211_del_sta这个函数
        linux-2.6.30和linux-2.6.34实现方式可以统一，目前34内核仍打桩，不然需要修改5115内核代码
    */

    msg = oal_nlmsg_new(OAL_NLMSG_DEFAULT_SIZE, gfp);
    if (OAL_PTR_NULL == msg) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    p_hdr = oal_genlmsg_put(msg, 0, 0, NL80211_FAM, 0, PRIV_NL80211_CMD_DEL_STATION);
    if (OAL_PTR_NULL == p_hdr) {
        oal_nlmsg_free(msg);
        return OAL_ERR_CODE_CFG80211_SKB_MEM_FAIL;
    }

    if (oal_nla_put_u32(msg, PRIV_NL80211_ATTR_IFINDEX, net_device->ifindex) ||
       oal_nla_put(msg, PRIV_NL80211_ATTR_MAC, OAL_ETH_ALEN_SIZE, mac_addr)) {
        oal_genlmsg_cancel(msg, p_hdr);
        oal_nlmsg_free(msg);
        return OAL_ERR_CODE_CFG80211_EMSGSIZE;
    }

    if (oal_genlmsg_end(msg, p_hdr) < 0) {
        oal_nlmsg_free(msg);
        return OAL_ERR_CODE_CFG80211_ENOBUFS;
    }

/*
    liuux-2.6.30和liuux-2.6.34内核都是从这个函数上,都能达到要求
    linux-2.6.34内核接着调用genlmsg_multicast_netns(&init_net......)
    linux-2.6.30内核接着调用nlmsg_multicast(genl_sock......)
*/
    l_let = oal_genlmsg_multicast(NL80211_FAM, msg, 0, NL80211_GID, gfp);

    return l_let;
#endif
}

/*****************************************************************************
 函 数 名  : oal_cfg80211_rx_mgmt_etc
 功能描述  : 上报接收到的管理帧
*****************************************************************************/
osal_u32 oal_cfg80211_rx_mgmt_etc(oal_net_device_stru *dev, osal_s32               l_freq,
    osal_u8               rssi, const osal_u8        *buf, osal_u32              len,
    oal_gfp_enum_uint8      gfp)
{
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 38))
    oal_wireless_dev_stru   *wdev = OAL_PTR_NULL;
    osal_u32               ul_ret;
    oal_bool_enum_uint8      ret;
    wdev = dev->ieee80211_ptr;
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3,18,0))
    ret   = cfg80211_rx_mgmt(wdev, l_freq, 0, buf, len, gfp);
#elif defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE < KERNEL_VERSION(3,12,0))
    ret   = cfg80211_rx_mgmt(wdev, l_freq, 0, buf, len, gfp);
#else
    ret   = cfg80211_rx_mgmt(wdev, l_freq, 0, buf, len, 1, gfp);
#endif
    /* 返回值适配 */
    (OAL_TRUE == ret) ? (ul_ret = OAL_SUCC) : (ul_ret = OAL_FAIL);
    return ul_ret;
#else
    oal_wireless_dev_stru               *wdev = OAL_PTR_NULL;
    oal_cfg80211_registered_device_stru *rdev = OAL_PTR_NULL;
    const osal_u8                     *action_data;

    wdev = dev->ieee80211_ptr;
    if (OAL_PTR_NULL == wdev) {
        return OAL_ERR_CODE_PTR_NULL;
    }
    rdev = oal_wiphy_to_dev(wdev->wiphy);

    /* action data starts with category */
    action_data = buf + OAL_IEEE80211_MIN_ACTION_SIZE - 1;

    return oal_nl80211_send_mgmt(rdev, dev, l_freq, rssi, buf, len, gfp);
#endif
}

/*****************************************************************************
 函 数 名  : oal_cfg80211_vendor_cmd_alloc_reply_skb_etc
 功能描述  : 申请厂家自定义返回数据
*****************************************************************************/
oal_netbuf_stru *oal_cfg80211_vendor_cmd_alloc_reply_skb_etc(oal_wiphy_stru *wiphy, osal_u32 len)
{
#if defined(LINUX_VERSION_CODE) && defined(_PRE_PRODUCT_ID_HOST) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3,14,0))
    return cfg80211_vendor_cmd_alloc_reply_skb(wiphy, len);
#else
    return OAL_PTR_NULL;
#endif
}

/*****************************************************************************
 函 数 名  : oal_cfg80211_vendor_cmd_reply_etc
 功能描述  : 厂家自定义数据上报
*****************************************************************************/
osal_s32 oal_cfg80211_vendor_cmd_reply_etc(oal_netbuf_stru *skb)
{
#if defined(LINUX_VERSION_CODE) && defined(_PRE_PRODUCT_ID_HOST) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3,14,0))
    return cfg80211_vendor_cmd_reply(skb);
#else
    return OAL_SUCC;
#endif
}

/*****************************************************************************
 功能描述  : 申请厂家自定义返回事件空间
 输入参数  : oal_wiphy_stru *wiphy: wiphy 结构
            oal_wireless_dev_stru *wdev 结构
            osal_u32 len 数据长度
            osal_u32 event_idx 事件在vendor event中的索引
*****************************************************************************/
oal_netbuf_stru *oal_cfg80211_vendor_event_alloc(oal_wiphy_stru *wiphy,
    oal_wireless_dev_stru *wdev, osal_u32 len, osal_u32 event_idx)
{
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 2, 0))
    return cfg80211_vendor_event_alloc(wiphy, wdev, len, event_idx, GFP_KERNEL);
#elif defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 14, 0))
    return cfg80211_vendor_event_alloc(wiphy, len, event_idx, GFP_KERNEL);
#else
    return NULL;
#endif
}

/*****************************************************************************
 功能描述  : 厂家自定义事件上报
*****************************************************************************/
osal_void oal_cfg80211_vendor_event(oal_netbuf_stru *skb)
{
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
    cfg80211_vendor_event(skb, GFP_KERNEL);
#elif defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 14, 0))
    __cfg80211_send_event_skb(skb, GFP_KERNEL);
#endif
}

/*****************************************************************************
 函 数 名  : oal_cfg80211_m2s_status_report
 功能描述  : 上报linux 内核m2s切换结果
*****************************************************************************/
osal_void oal_cfg80211_m2s_status_report(oal_net_device_stru       *netdev,
    oal_gfp_enum_uint8 gfp, osal_u8 *buf, osal_u32 len)
{
#ifdef  CONFIG_HW_WIFI_MSS
    /* 此接口为终端实现的内核接口，定义处用内核宏CONFIG_HW_WIFI_MSS包裹 */
    cfg80211_drv_mss_result(netdev, gfp, buf, len);
#endif
}
#endif

/*
 * 函 数 名  : oal_cfg80211_ch_switch_notify
 * 功能描述  : 上报内核ch_switch 事件
 */
void oal_cfg80211_ch_switch_notify(oal_net_device_stru *pst_netdev,
                                   oal_cfg80211_chan_def *pst_chandef)
{
#ifdef CONTROLLER_CUSTOMIZATION
    cfg80211_ch_switch_notify(pst_netdev, pst_chandef, 0, 0);
#else
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 5, 0))
    cfg80211_ch_switch_notify(pst_netdev, pst_chandef, 0, 0);
#endif
#endif
}

/*
 * 函 数 名  : oal_cfg80211_unlink_bss
 * 功能描述  : 从kernel中删除old BSS entry
 */
void oal_cfg80211_unlink_bss(oal_wiphy_stru *pst_wiphy, oal_cfg80211_bss_stru *pst_cfg80211_bss)
{
    cfg80211_unlink_bss(pst_wiphy, pst_cfg80211_bss);
}

/*
 * 函 数 名  : oal_cfg80211_chba_acs_test
 * 功能描述  : 上报内核acs参数
 */
void oal_cfg80211_link_meas_res_report(oal_net_device_stru *netdev, gfp_t gfp,
    osal_u8 *buf, osal_u32 len)
{
#ifdef _PRE_WLAN_CHBA_SUPPLICANT_KERNEL
    cfg80211_drv_chba_acs_report(netdev, gfp, buf, len);
#else
    return;
#endif
}

oal_module_symbol(oal_cfg80211_put_bss_etc);
oal_module_symbol(oal_cfg80211_get_bss_etc);
oal_module_symbol(oal_cfg80211_inform_bss_frame_etc);
oal_module_symbol(oal_cfg80211_scan_done_etc);
#ifdef _PRE_WLAN_FEATURE_PNO_SCAN
oal_module_symbol(oal_cfg80211_sched_scan_result_etc);
#endif
oal_module_symbol(oal_cfg80211_connect_result_etc);
oal_module_symbol(oal_cfg80211_roamed_etc);
oal_module_symbol(oal_cfg80211_ft_event_etc);
oal_module_symbol(oal_cfg80211_disconnected_etc);
oal_module_symbol(oal_cfg80211_new_sta_etc);
oal_module_symbol(oal_cfg80211_mic_failure_etc);
oal_module_symbol(oal_cfg80211_del_sta_etc);
oal_module_symbol(oal_cfg80211_rx_mgmt_etc);
oal_module_symbol(oal_cfg80211_mgmt_tx_status_etc);
oal_module_symbol(oal_cfg80211_ready_on_channel_etc);
oal_module_symbol(oal_cfg80211_remain_on_channel_expired_etc);
#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#if defined(_PRE_PRODUCT_ID_HOST)
oal_module_symbol(oal_kobject_uevent_env_sta_join_etc);
oal_module_symbol(oal_kobject_uevent_env_sta_leave_etc);
#endif
#endif
oal_module_symbol(oal_cfg80211_vendor_cmd_alloc_reply_skb_etc);
oal_module_symbol(oal_cfg80211_vendor_cmd_reply_etc);
oal_module_symbol(oal_cfg80211_m2s_status_report);
#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif
