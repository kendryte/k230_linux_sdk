/*
 * Copyright (c) CompanyNameMagicTag 2016-2023. All rights reserved.
 * Description: linux cfgid 配置调试命令
 * Create: 2016年4月8日
 */

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "oam_struct.h"

#include "hmac_config.h"
#include "hmac_ext_if.h"
#include "wal_ext_if.h"
#include "wal_main.h"
#include "wal_config.h"
#include "wal_linux_ioctl.h"
#include "wal_linux_netdev.h"
#include "wal_linux_ccpriv.h"
#include "wlan_msg.h"
#include "diag_log_common.h"
#include "common_dft.h"
#include "hmac_twt.h"
#include "hmac_wmm.h"
#ifdef _PRE_WLAN_FEATURE_OPMODE_NOTIFY
#include "hmac_opmode.h"
#endif
#ifdef _PRE_WLAN_FEATURE_WUR_TX
#include "hmac_wur_ap.h"
#endif
#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST
#include "hmac_obss_sta.h"
#endif
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif
#ifdef _PRE_WLAN_CFGID_DEBUG

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_WAL_CONFIG_DEBUG_C
#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

/*****************************************************************************
  2 结构体定义
*****************************************************************************/
/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
/*****************************************************************************
  3 函数实现
*****************************************************************************/
/*****************************************************************************
 功能描述  : 随便组一个报文
 输入参数  : size表示报文长度， 包含以太网头部， 不包括FCS， 取值范围应该为60~1514
 输出参数  : 无

 修改历史      :
  1.日    期   : 2013年9月22日
    修改内容   : 新生成函数

*****************************************************************************/
oal_netbuf_stru *wal_config_create_packet_ap_etc(osal_u32 size,
    osal_u8 tid, osal_u8 *mac_ra, osal_u8 *mac_ta)
{
    oal_netbuf_stru         *buf;
    mac_ether_header_stru   *ether_header;
    mac_ip_header_stru      *pst_ip;
    osal_u32               loop;
    osal_u32               l_reserve = 256;
    buf = oal_netbuf_alloc(size + l_reserve, (osal_s32)l_reserve, 4); /* 4字节对齐 */
    if (OAL_UNLIKELY(buf == OAL_PTR_NULL)) {
        return OAL_PTR_NULL;
    }

    oal_netbuf_put(buf, size);
    oal_set_mac_addr(&buf->data[0], mac_ra);
    oal_set_mac_addr(&buf->data[6], mac_ta); /* recv addr 偏移6字节为 transmit addr */

    /* 帧体内容 最后6个字节保持为0x00 */
    for (loop = 0; loop < size - 20; loop++) { /* 报文尾部保留20字节 */
        buf->data[14 + loop] = (osal_u8)loop; /* 设置报文第14字节以后的内容 */
    }

    ether_header = (mac_ether_header_stru *)oal_netbuf_data(buf);

    ether_header->ether_type = oal_host2net_short(ETHER_TYPE_IP);
    pst_ip = (mac_ip_header_stru *)(ether_header + 1);      /* 偏移一个以太网头，取ip头 */

    pst_ip->tos = (osal_u8)(tid << WLAN_IP_PRI_SHIFT);

    pst_ip->protocol = MAC_UDP_PROTOCAL;
    buf->next = OAL_PTR_NULL;
    buf->prev = OAL_PTR_NULL;

    memset_s(oal_netbuf_cb(buf), OAL_NETBUF_CB_SIZE(), 0, OAL_NETBUF_CB_SIZE());

    return buf;
}


/*****************************************************************************
 功能描述  : 发包配置命令
 输入参数  : 无
 输出参数  : 无

 修改历史      :
  1.日    期   : 2013年10月8日
    修改内容   : 新生成函数

*****************************************************************************/
OAL_STATIC osal_s32  wal_config_packet_xmit(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    mac_cfg_mpdu_ampdu_tx_param_stru *ampdu_tx_param;
    osal_u8                         skb_num;
    osal_u8                         skb_idx;
    oal_netbuf_stru                  *pst_netbuf;
    osal_u8                         tid;
    osal_u16                        packet_len;
    oal_net_device_stru              *dev;
    oal_netbuf_stru                  *past_netbuf[32] = {OAL_PTR_NULL};

    ampdu_tx_param = (mac_cfg_mpdu_ampdu_tx_param_stru *)msg->data;

    skb_num     = ampdu_tx_param->packet_num;
    tid         = ampdu_tx_param->tid;
    packet_len  = ampdu_tx_param->packet_len;
    dev = hmac_vap_get_net_device_etc(hmac_vap->vap_id);

    /* 为profiling测试修改，只测量xmit时间 */
    if (skb_num <= 32) { /* skb个数不超过32 */
        for (skb_idx = 0; skb_idx < skb_num; skb_idx++) {
            past_netbuf[skb_idx] = wal_config_create_packet_ap_etc(packet_len, tid,
                ampdu_tx_param->ra_mac, mac_mib_get_station_id(hmac_vap));
        }
        for (skb_idx = 0; skb_idx < skb_num; skb_idx++) {
            wal_bridge_vap_xmit_etc(past_netbuf[skb_idx], dev);
        }
    } else {
        for (skb_idx = 0; skb_idx < skb_num; skb_idx++) {
            pst_netbuf = wal_config_create_packet_ap_etc(packet_len, tid, ampdu_tx_param->ra_mac,
                mac_mib_get_station_id(hmac_vap));
            if (pst_netbuf == OAL_PTR_NULL) {
                oam_warning_log2(0, OAM_SF_ANY,
                    "vap_id[%d] {wal_config_create_packet_ap_etc:: alloc netbuf failed, skb_idx: %d!}",
                    hmac_vap->vap_id, skb_idx);
                continue;
            }
            wal_bridge_vap_xmit_etc(pst_netbuf, dev);
        }
    }

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_ISOLATION
/*****************************************************************************
 功能描述  : 隔离信息打印
 输入参数  : 无
 输出参数  : 无

 修改历史      :
  1.日    期   : 2014年7月9日
    修改内容   : 新生成函数

*****************************************************************************/
OAL_STATIC osal_u32  wal_config_isolation_show(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    if (hmac_vap->vap_mode != WLAN_VAP_MODE_BSS_AP) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_config_isolation_show::not ap mode!}");
        return OAL_SUCC;
    }
    return hmac_config_show_isolation(hmac_vap, msg->data_len, msg->data);
}

/*****************************************************************************
 功能描述  : 配置用户隔离模式
 输入参数  : 无
 输出参数  : 无

 修改历史      :
  1.日    期   : 2014年3月27日
    修改内容   : 新生成函数

*****************************************************************************/
OAL_STATIC osal_u32  wal_config_set_isolation_mode(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    if (hmac_vap->vap_mode != WLAN_VAP_MODE_BSS_AP) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_config_set_isolation_mode::not ap mode!}");
        return OAL_SUCC;
    }
    return hmac_config_set_isolation_mode(hmac_vap, msg->data_len, msg->data);
}

/*****************************************************************************
 功能描述  : 配置用户隔离类型
 输入参数  : 无
 输出参数  : 无

 修改历史      :
  1.日    期   : 2014年3月27日
    修改内容   : 新生成函数

*****************************************************************************/
OAL_STATIC osal_u32  wal_config_set_isolation_type(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    if (hmac_vap->vap_mode != WLAN_VAP_MODE_BSS_AP) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_config_set_isolation_type::not ap mode!}");
        return OAL_SUCC;
    }
    return hmac_config_set_isolation_type(hmac_vap, msg->data_len, msg->data);
}

/*****************************************************************************
 功能描述  : 配置用户隔离forward
 输入参数  : 无
 输出参数  : 无

 修改历史      :
  1.日    期   : 2014年3月27日
    修改内容   : 新生成函数

*****************************************************************************/
OAL_STATIC osal_u32 wal_config_set_isolation_forward(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    if (hmac_vap->vap_mode != WLAN_VAP_MODE_BSS_AP) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_config_set_isolation_forward::not ap mode!}");
        return OAL_SUCC;
    }
    return hmac_config_set_isolation_forword(hmac_vap, msg->data_len, msg->data);
}

/*****************************************************************************
 功能描述  : 隔离统计清0
 输入参数  : 无
 输出参数  : 无

 修改历史      :
  1.日    期   : 2014年3月27日
    修改内容   : 新生成函数

*****************************************************************************/
OAL_STATIC osal_u32 wal_config_set_isolation_clear(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    if (hmac_vap->vap_mode != WLAN_VAP_MODE_BSS_AP) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_config_set_isolation_clear::not ap mode!}");
        return OAL_SUCC;
    }
    return hmac_config_set_isolation_clear(hmac_vap, msg->data_len, msg->data);
}
#endif

osal_void wal_cfg_debug_init(osal_void)
{
    frw_msg_hook_register(WLAN_MSG_W2H_CFG_AMPDU_TX_ON, hmac_config_set_ampdu_tx_on_etc);
#ifdef _PRE_WLAN_CFGID_DEBUG
    frw_msg_hook_register(WLAN_MSG_W2H_CFG_AMSDU_AMPDU_SWITCH, hmac_config_amsdu_ampdu_switch_etc);
#endif
    frw_msg_hook_register(WLAN_MSG_W2H_CFG_DELBA_REQ, hmac_config_delba_req_etc);
#ifdef _PRE_WLAN_FEATURE_WUR_TX
    frw_msg_hook_register(WLAN_MSG_W2H_CFG_WUR_SEND_SETUP, hmac_config_wur_send_setup);
    frw_msg_hook_register(WLAN_MSG_W2H_CFG_WUR_SEND_TEARDOWN, hmac_config_wur_send_teardown);
    frw_msg_hook_register(WLAN_MSG_W2H_CFG_WUR_ENABLE_OPTION, hmac_config_wur_enable_option);
#endif
#ifdef _PRE_WLAN_FEATURE_OPMODE_NOTIFY
    frw_msg_hook_register(WLAN_MSG_W2H_CFG_SET_OPMODE_NOTIFY,  hmac_config_set_opmode_notify_etc);
#endif
#ifdef _PRE_WLAN_FEATURE_ISOLATION
    /* user's isolation param config */
    frw_msg_hook_register(WLAN_MSG_W2H_CFG_ISOLATION_CLEAR, wal_config_set_isolation_clear);
    frw_msg_hook_register(WLAN_MSG_W2H_CFG_ISOLATION_FORWARD, wal_config_set_isolation_forward);
    frw_msg_hook_register(WLAN_MSG_W2H_CFG_ISOLATION_MODE, wal_config_set_isolation_mode);
    frw_msg_hook_register(WLAN_MSG_W2H_CFG_ISOLATION_TYPE, wal_config_set_isolation_type);
    frw_msg_hook_register(WLAN_MSG_W2H_CFG_ISOLATION_SHOW, wal_config_isolation_show);
#endif
#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST
    frw_msg_hook_register(WLAN_MSG_W2H_CFG_GET_2040BSS_SW, hmac_config_get_2040bss_sw);
#endif
    frw_msg_hook_register(WLAN_MSG_W2H_CFG_PACKET_XMIT, wal_config_packet_xmit);
}
#endif
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

