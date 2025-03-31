/*
 * Copyright (c) CompanyNameMagicTag 2014-2023. All rights reserved.
 * 文 件 名   : hmac_isolation.c
 * 生成日期   : 2014年3月26日
 * 功能描述   : 用户隔离
 */

#ifdef _PRE_WLAN_FEATURE_ISOLATION


/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "hmac_main.h"
#include "hmac_isolation.h"
#include "mac_vap_ext.h"
#include "hmac_vap.h"
#include "mac_resource_ext.h"
#include "hmac_user.h"
#include "hmac_mgmt_ap.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_HMAC_ISOLATION_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

/*****************************************************************************
  2 STRUCT定义
*****************************************************************************/


/*****************************************************************************
  3 全局变量定义
*****************************************************************************/

/*****************************************************************************
  4 函数实现
*****************************************************************************/

/*****************************************************************************
 函 数 名  : hmac_isolation_set_mode
 功能描述  : 设置隔离模式
 输入参数  : mode bit0 = 1 广播 bit1 = 1 组播 bit2 = 1 单播
 输出参数  : osal_u32
 返 回 值  : null
 修改历史      :
  1.日    期   : 2014年3月26日
    修改内容   : 新生成函数
*****************************************************************************/
osal_u32 hmac_isolation_set_mode(hmac_vap_stru *hmac_vap, osal_u8 mode)
{
    mac_isolation_info_stru       *isolation_info = OAL_PTR_NULL;

    /* 1.1 入参检查 */
    if (hmac_vap == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_isolation_set_mode::null mac_vap}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 2.1 获取参数 */
    mode = mode & 0x7;
    if (mode == 0) {
        oam_warning_log1(0, OAM_SF_ANY, "vap_id[%d] {hmac_isolation_set_mode invalid, The valid Para is:(1~7)}",
            hmac_vap->vap_id);
        /* add mode check chenchongbao 2014.7.7 */
        return OAL_ERR_CODE_SECURITY_MODE_INVALID;
    }

    isolation_info = &hmac_vap->isolation_info;

    /* 3.1 重新初始化 */
    isolation_info->mode = mode;

    oam_info_log2(0, OAM_SF_ANY, "vap_id[%d] {hmac_isolation_set_mode::isolation mode is set to %d}",
        hmac_vap->vap_id, mode);
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_isolation_set_type
 功能描述  : 设置隔离类型
 输出参数  : CS_ISOLATION_TYPE_MULTI_BSS CS_ISOLATION_TYPE_SINGLE_BSS
 返 回 值  : null
 修改历史      :
  1.日    期   : 2014年3月26日
    修改内容   : 新生成函数
*****************************************************************************/
osal_u32 hmac_isolation_set_type(hmac_vap_stru *hmac_vap, osal_u8 bss_type, osal_u8 isolation_type)
{
    mac_isolation_info_stru       *isolation_info = OAL_PTR_NULL;

    /* 1.1 入参检查 */
    if (hmac_vap == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_isolation_set_type::null mac_vap}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 2.1 获取参数 */
    if ((bss_type != CS_ISOLATION_TYPE_MULTI_BSS) && (bss_type != CS_ISOLATION_TYPE_SINGLE_BSS)) {
        oam_error_log1(0, OAM_SF_ANY, "{hmac_isolation_set_type::bss_type=%d is wrong.}", bss_type);
        return OAL_FAIL;
    }
    if ((isolation_type != OAL_TRUE) && (isolation_type != OAL_FALSE)) {
        oam_error_log1(0, OAM_SF_ANY, "{hmac_isolation_set_type::isolation_type=%d is wrong.}", isolation_type);
        return OAL_FAIL;
    }

    isolation_info = &hmac_vap->isolation_info;

    /* 3.1 设置模式 */
    if (bss_type == CS_ISOLATION_TYPE_MULTI_BSS) {
        isolation_info->multi_type = isolation_type;
    } else {
        isolation_info->single_type = isolation_type;
    }

    oam_info_log3(0, OAM_SF_ANY, "vap_id[%d] {hmac_isolation_set_type::bss_type=%d, isolation type=%d}",
        hmac_vap->vap_id, bss_type, isolation_type);
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_isolation_set_forword
 功能描述  : 设置隔离forword方式
 输出参数  : osal_u32
 返 回 值  : null
 修改历史      :
  1.日    期   : 2014年3月26日
    修改内容   : 新生成函数
*****************************************************************************/
osal_u32 hmac_isolation_set_forward(hmac_vap_stru *hmac_vap, osal_u8 forward)
{
    mac_isolation_info_stru       *isolation_info = OAL_PTR_NULL;

    /* 1.1 入参检查 */
    if (hmac_vap == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_isolation_set_forward::null mac_vap}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 2.1 获取参数 */
    if (forward >= CS_ISOLATION_FORWORD_BUTT) {
        forward = CS_ISOLATION_FORWORD_TOLAN;
    }

    isolation_info = &hmac_vap->isolation_info;
    isolation_info->forward = forward;
    oam_info_log2(0, OAM_SF_ANY, "vap_id[%d] {hmac_isolation_set_forward::isolation forward is set to %d}",
        hmac_vap->vap_id, forward);

    return OAL_SUCC;
}


/*****************************************************************************
 函 数 名  : hmac_isolation_clear_counter
 功能描述  : 去/使能自动黑名单
  输出参数  : osal_u32
 返 回 值  : null
 修改历史      :
  1.日    期   : 2014年3月26日
    修改内容   : 新生成函数
*****************************************************************************/
osal_u32 hmac_isolation_clear_counter(hmac_vap_stru *hmac_vap)
{
    mac_isolation_info_stru       *isolation_info = OAL_PTR_NULL;

    /* 1.1 入参检查 */
    if (hmac_vap == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_isolation_clear_counter::null mac_vap}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    isolation_info = &hmac_vap->isolation_info;

    /* 2.1 刷新计数器 */
    isolation_info->counter_bcast = 0;
    isolation_info->counter_mcast = 0;
    isolation_info->counter_ucast = 0;

    oam_info_log1(0, OAM_SF_ANY, "vap_id[%d] {isolation counters is cleared }", hmac_vap->vap_id);
    return OAL_SUCC;
}
/*****************************************************************************
 函 数 名  : hmac_show_isolation_info
 功能描述  : 显示隔离信息
 输出参数  : osal_void
 返 回 值  : null
 修改历史      :
  1.日    期   : 2014年7月9日
    修改内容   : 新生成函数
*****************************************************************************/
osal_void hmac_show_isolation_info(hmac_vap_stru *hmac_vap)
{
    mac_isolation_info_stru         *isolation_info = OAL_PTR_NULL;
    osal_s8                        *pc_print_buff = OAL_PTR_NULL;

    /* 1.1 入参检查 */
    if (hmac_vap == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_show_isolation_info::null mac_vap}");
        return;
    }

    pc_print_buff = (osal_s8 *)oal_mem_alloc(OAL_MEM_POOL_ID_LOCAL, OAM_REPORT_MAX_STRING_LEN, OAL_TRUE);
    if (pc_print_buff == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_CFG, "{hmac_show_autoblacklist_info::pc_print_buff null.}");
        return;
    }
    memset_s(pc_print_buff, OAM_REPORT_MAX_STRING_LEN, 0, OAM_REPORT_MAX_STRING_LEN);

    isolation_info = &hmac_vap->isolation_info;

    /* 1.2 打印隔离信息 */
    oam_info_log1(0, OAM_SF_ANY, "vap_id[%d] {hmac_show_isolation_info::isolation info is :}", hmac_vap->vap_id);
    oam_info_log4(0, OAM_SF_ANY, "{ mode:%d. single_bss_type:%d. multi_bss_type=%d.forward:%d.}",
        isolation_info->mode, isolation_info->single_type, isolation_info->multi_type,
        isolation_info->forward);
    oam_info_log4(0, OAM_SF_ANY, "vap_id[%d] { bcast_cnt: %u.mcast_cnt: %u.ucast_cnt: %u.}", hmac_vap->vap_id,
        isolation_info->counter_bcast, isolation_info->counter_mcast,
        isolation_info->counter_ucast);

    (osal_void)snprintf_s(pc_print_buff, OAM_REPORT_MAX_STRING_LEN, OAM_REPORT_MAX_STRING_LEN,
        "vap%d isolation info is :\n"
        "\tmode:%d. single_type:%d. multi_tyep:%d. forward:%d.\n"
        "\tbcast_cnt: %u\n"
        "\tmcast_cnt: %u\n"
        "\tucast_cnt: %u\n",
        hmac_vap->vap_id,
        isolation_info->mode, isolation_info->single_type, isolation_info->multi_type,
        isolation_info->forward,
        isolation_info->counter_bcast,
        isolation_info->counter_mcast,
        isolation_info->counter_ucast);

    dft_report_params_etc(pc_print_buff, osal_strlen(pc_print_buff), SOC_DIAG_MSG_ID_WIFI_FRAME_MAX);
    oal_mem_free(pc_print_buff, OAL_TRUE);

    return;
}

OAL_STATIC osal_u32 hmac_isolation_multi_filter(hmac_vap_stru *hmac_vap,
    osal_u8 *mac_addr, mac_isolation_info_stru *isolation_info, cs_isolation_forward_enum *action)
{
    osal_u16 user_idx;
    hmac_user_stru *hmac_user = OSAL_NULL;

    /* 1.2 广播隔离 */
    if (ETHER_IS_BROADCAST(mac_addr)) {
        if (isolation_info->mode & CS_ISOLATION_MODE_BROADCAST) {
            isolation_info->counter_bcast++;
            oam_info_log2(0, OAM_SF_ANY, "vap_id[%d] {hmac_isolation_filter::isolation MultiBSS Bcast=%d}",
                hmac_vap->vap_id, isolation_info->counter_bcast);
            *action = (cs_isolation_forward_enum)isolation_info->forward;
            return OAL_SUCC;
        } else {
            goto fwd_none;
        }
    }
    /* 1.3 组播隔离 */
    if (ETHER_IS_MULTICAST(mac_addr)) {
        if (isolation_info->mode & CS_ISOLATION_MODE_MULTICAST) {
            isolation_info->counter_mcast++;
            oam_info_log2(0, OAM_SF_ANY, "vap_id[%d] {hmac_isolation_filter::isolation MultiBSS Mcast=%d}",
                hmac_vap->vap_id, isolation_info->counter_mcast);
            *action = (cs_isolation_forward_enum)isolation_info->forward;
            return OAL_SUCC;
        } else {
            goto fwd_none;
        }
    }

    /*
        * 2.4 单播隔离,如果在本bss中找到用户，不隔离处理，
        * 否则需要在其他bss中找，找到就隔离
        */
    if ((isolation_info->mode & CS_ISOLATION_MODE_UNICAST) &&
        (hmac_vap_find_user_by_macaddr_etc(hmac_vap, mac_addr, &user_idx) != OAL_SUCC)) {
        if (hmac_device_find_user_by_macaddr_etc(mac_addr, &user_idx) == OAL_SUCC) {
            hmac_user = mac_res_get_hmac_user_etc(user_idx);
            if (hmac_user == OSAL_NULL) {
                goto fwd_none;
            }
            isolation_info->counter_ucast++;
            oam_info_log4(0, OAM_SF_ANY,
                "{hmac_isolation_filter::isolation MultiBSS Ucast=%d. to %02x.x.x.x.%02x.%02x}",
                /* 0表示MAC地址第0位，4表示MAC地址第4位，5表示MAC地址第5位 */
                isolation_info->counter_ucast, mac_addr[0], mac_addr[4], mac_addr[5]);
            *action = (cs_isolation_forward_enum)isolation_info->forward;
            return OAL_CONTINUE;
        }
    }

fwd_none:
    *action = CS_ISOLATION_FORWORD_NONE;
    return OAL_SUCC;
}

OAL_STATIC cs_isolation_forward_enum hmac_isolation_single_filter(hmac_vap_stru *hmac_vap,
    osal_u8 *mac_addr, mac_isolation_info_stru *isolation_info, cs_isolation_forward_enum *action)
{
    osal_u16 user_idx;
    hmac_user_stru *hmac_user = OSAL_NULL;

    /* 2.2 广播隔离 */
    if (ETHER_IS_BROADCAST(mac_addr)) {
        if (isolation_info->mode & CS_ISOLATION_MODE_BROADCAST) {
            isolation_info->counter_bcast++;
            oam_info_log2(0, OAM_SF_ANY,
                "vap_id[%d] {hmac_isolation_filter::isolation SingleBSS Bcast=%d}", hmac_vap->vap_id,
                isolation_info->counter_bcast);
            *action = (cs_isolation_forward_enum)isolation_info->forward;
            return OAL_SUCC;
        } else {
            goto fwd_none;
        }
    }
    /* 2.3 组播隔离 */
    if (ETHER_IS_MULTICAST(mac_addr)) {
        if (isolation_info->mode & CS_ISOLATION_MODE_MULTICAST) {
            isolation_info->counter_mcast++;
            oam_info_log2(0, OAM_SF_ANY,
                "vap_id[%d] {hmac_isolation_filter::isolation SingleBSS Mcast=%d}",
                hmac_vap->vap_id, isolation_info->counter_mcast);
            *action = (cs_isolation_forward_enum)isolation_info->forward;
            return OAL_SUCC;
        } else {
            goto fwd_none;
        }
    }
    /* 2.4 单播隔离，如果在本bss中找到用户就隔离，否则不处理 */
    if (isolation_info->mode & CS_ISOLATION_MODE_UNICAST) {
        if (hmac_vap_find_user_by_macaddr_etc(hmac_vap, mac_addr, &user_idx) == OAL_SUCC) {
            hmac_user = mac_res_get_hmac_user_etc(user_idx);
            if (hmac_user == OSAL_NULL) {
                goto fwd_none;
            }
            isolation_info->counter_ucast++;
            oam_info_log4(0, OAM_SF_ANY,
                "{hmac_isolation_filter::isolation SingleBSS Ucast=%d. to %02x.x.x.x.%02x.%02x}",
                /* 0表示MAC地址第0位，4表示MAC地址第4位，5表示MAC地址第5位 */
                isolation_info->counter_ucast, mac_addr[0], mac_addr[4], mac_addr[5]);
            *action = (cs_isolation_forward_enum)isolation_info->forward;
            return OAL_CONTINUE;
        }
    }

fwd_none:
    *action = CS_ISOLATION_FORWORD_NONE;
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_isolation_filter
 功能描述  : 用户隔离过滤
 输入参数  : mac地址
 输出参数  : osal_void
 返 回 值  : OAL_TRUE 需要被过滤; OAL_FALSE 不需要被过滤
 修改历史      :
  1.日    期   : 2014年3月29日
    修改内容   : 新生成函数
*****************************************************************************/
cs_isolation_forward_enum hmac_isolation_filter(hmac_vap_stru *hmac_vap, osal_u8 *mac_addr)
{
    mac_isolation_info_stru *isolation_info = OSAL_NULL;
    osal_u32 ret = OAL_CONTINUE;
    cs_isolation_forward_enum fwd = CS_ISOLATION_FORWORD_NONE;

    /* 1.1 入参检查 */
    if ((hmac_vap == OSAL_NULL) || (mac_addr == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_isolation_filter::null mac_vap or null mac addr}");
        return CS_ISOLATION_FORWORD_NONE;
    }

    isolation_info = &hmac_vap->isolation_info;
    /* 1.1 多BSS隔离 */
    if (isolation_info->multi_type == OAL_TRUE) {
        ret = hmac_isolation_multi_filter(hmac_vap, mac_addr, isolation_info, &fwd);
        if (ret != OAL_CONTINUE) {
            return fwd;
        }
    }
    /* 2.1 单BSS隔离 */
    if (isolation_info->single_type == OAL_TRUE) {
        ret = hmac_isolation_single_filter(hmac_vap, mac_addr, isolation_info, &fwd);
        if (ret != OAL_CONTINUE) {
            return fwd;
        }
    }
    return fwd;
}

osal_bool hmac_isolation_filter_drop(hmac_vap_stru *hmac_vap, oal_netbuf_stru *netbuf)
{
    osal_u8 *da = OSAL_NULL;
    mac_rx_ctl_stru *rx_ctrl = (mac_rx_ctl_stru *)oal_netbuf_cb(netbuf);
    mac_ieee80211_frame_stru *frame_hdr = (mac_ieee80211_frame_stru *)(uintptr_t)mac_get_rx_cb_mac_header_addr(rx_ctrl);
    mac_rx_get_da(frame_hdr, &da);

    if (hmac_isolation_filter(hmac_vap, da) == CS_ISOLATION_FORWORD_DROP) {
        return OAL_TRUE;
    }
    return OAL_FALSE;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* #ifdef _PRE_WLAN_FEATURE_ISOLATION */
