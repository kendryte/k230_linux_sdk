/*
 * Copyright (c) CompanyNameMagicTag 2020-2022. All rights reserved.
 * Description: 黑白名单功能.
 */

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "hmac_blacklist.h"

#include "hmac_main.h"
#include "oam_struct.h"
#include "mac_vap_ext.h"
#include "hmac_vap.h"
#include "mac_resource_ext.h"
#include "hmac_user.h"
#include "hmac_mgmt_ap.h"
#include "common_dft.h"
#include "frw_util_notifier.h"
#include "hmac_feature_interface.h"
#include "hmac_dfx.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_HMAC_BLACKLIST_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

/*****************************************************************************
  2 STRUCT定义
*****************************************************************************/

/*****************************************************************************
  3 全局变量定义
*****************************************************************************/
mac_blacklist_info_stru *g_blacklist_info = OSAL_NULL;

/*****************************************************************************
  4 函数实现
*****************************************************************************/
OAL_STATIC osal_void hmac_show_blacklist_info_fill(const hmac_vap_stru *hmac_vap,
    mac_blacklist_info_stru *blacklist_info, osal_s8 *pc_print_buff);
OAL_STATIC oal_bool_enum_uint8 hmac_blacklit_filter_check_white(hmac_vap_stru *hmac_vap, osal_u8 *mac_addr,
    const mac_blacklist_info_stru *blacklist_info);
OAL_STATIC osal_u32 hmac_autoblacklist_get_handle(const osal_u8 *mac_addr, osal_u32 second,
    mac_autoblacklist_stru **ppst_autoblacklist, mac_autoblacklist_info_stru *autoblacklist_info);
OAL_STATIC osal_void hmac_blacklist_disconnect_all_etc(hmac_vap_stru *hmac_vap,
    mac_blacklist_info_stru *blacklist_info);
OAL_STATIC oal_bool_enum_uint8 hmac_blacklist_filter_etc(hmac_vap_stru *hmac_vap, osal_u8 *mac_addr);
OAL_STATIC osal_void hmac_blacklist_feature_hook_register(osal_void);
OAL_STATIC osal_void hmac_blacklist_feature_hook_unregister(osal_void);

/*****************************************************************************
 函 数 名  : hmac_blacklist_mac_is_zero
 功能描述  : 判断mac地址是否为0
*****************************************************************************/
OAL_STATIC osal_u32 hmac_blacklist_mac_is_zero(const osal_u8 *mac_addr)
{
    if (mac_addr[0] == 0 &&
        mac_addr[1] == 0 &&
        mac_addr[2] == 0 &&     /* 2:MAC地址索引 */
        mac_addr[3] == 0 &&     /* 3:MAC地址索引 */
        mac_addr[4] == 0 &&     /* 4:MAC地址索引 */
        mac_addr[5] == 0) {     /* 5:MAC地址索引 */
        return 1;
    }

    return 0;
}

/*****************************************************************************
 函 数 名  : hmac_blacklist_mac_is_bcast
 功能描述  : 判断mac地址是否为0
 输出参数  : osal_u32
 返 回 值  : null
*****************************************************************************/
OAL_STATIC osal_u32 hmac_blacklist_mac_is_bcast(const osal_u8 *mac_addr)
{
    if (mac_addr[0] == 0xff &&
        mac_addr[1] == 0xff &&
        mac_addr[2] == 0xff &&     /* 2:MAC地址索引 */
        mac_addr[3] == 0xff &&     /* 3:MAC地址索引 */
        mac_addr[4] == 0xff &&     /* 4:MAC地址索引 */
        mac_addr[5] == 0xff) {     /* 5:MAC地址索引 */
        return 1;
    }

    return 0;
}

/* 黑白名单开启初始化g_blacklist_info */
OAL_STATIC osal_void hmac_blacklist_info_init(hmac_vap_stru *hmac_vap)
{
    osal_u32 len = 0;
    if (g_blacklist_info != OSAL_NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{hmac_blacklist_info_init::g_blacklist_info != OSAL_NULL}\r\n");
        return;
    }

    len = sizeof(mac_blacklist_info_stru) * WLAN_BLACKLIST_MAX;
    g_blacklist_info = (mac_blacklist_info_stru *)osal_kmalloc(len, 0);
    if (g_blacklist_info == OSAL_NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{hmac_blacklist_info_init::g_blacklist_info alloc fail!}\r\n");
        return;
    }
    (osal_void)memset_s(g_blacklist_info, len, 0, len);

    hmac_vap->blacklist_info = g_blacklist_info;
    memset_s(hmac_vap->blacklist_info->black_list, sizeof(mac_blacklist_stru) * WLAN_BLACKLIST_MAX,
        0, sizeof(mac_blacklist_stru) * WLAN_BLACKLIST_MAX);
    memset_s(&(hmac_vap->blacklist_info->autoblacklist_info), sizeof(mac_autoblacklist_info_stru),
        0, sizeof(mac_autoblacklist_info_stru));
    hmac_vap->blacklist_info->mode = 0;
    hmac_vap->blacklist_info->list_num = 0;
}

/* 黑白名单关闭 释放g_blacklist_info内存 */
OAL_STATIC osal_void hmac_blacklist_info_deinit(hmac_vap_stru *hmac_vap)
{
    if (g_blacklist_info == OSAL_NULL) {
        return;
    }

    osal_kfree(g_blacklist_info);
    g_blacklist_info = OSAL_NULL;
    hmac_vap->blacklist_info = OSAL_NULL;
    return;
}

/*****************************************************************************
 函 数 名  : hmac_blacklist_init
 功能描述  : 关闭黑名单功能，并清空所有数据
 输出参数  : osal_void
 返 回 值  : null
*****************************************************************************/
OAL_STATIC osal_void hmac_blacklist_init(hmac_vap_stru *hmac_vap, cs_blacklist_mode_enum_uint8 mode,
    oal_bool_enum_uint8 flush)
{
    mac_blacklist_info_stru     *blacklist_info;

    if (hmac_vap->vap_mode != WLAN_VAP_MODE_BSS_AP) {
        oam_warning_log0(0, OAM_SF_ANY, "{hmac_blacklist_init::not AP mode!}\r\n");
        return;
    }

    if (mode != CS_BLACKLIST_MODE_NONE) {
        /* 对外接口注册 */
        hmac_blacklist_info_init(hmac_vap);
        hmac_blacklist_feature_hook_register();
    } else {
        /* 对外接口去注册 */
        hmac_blacklist_info_deinit(hmac_vap);
        hmac_blacklist_feature_hook_unregister();
    }

    blacklist_info = hmac_vap->blacklist_info;
    if (blacklist_info == OAL_PTR_NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{hmac_blacklist_init::blacklist_info null!}\r\n");
        return;
    }

    /* Max=32 => 新增加mac_vap结构大小= 0x494 = 1172 ; Max=8 => size = 308 */
    if (flush) {
        memset_s(blacklist_info->black_list, sizeof(mac_blacklist_stru) * WLAN_BLACKLIST_MAX, 0,
            sizeof(mac_blacklist_stru) * WLAN_BLACKLIST_MAX);
        memset_s(&(blacklist_info->autoblacklist_info), sizeof(mac_autoblacklist_info_stru), 0,
            sizeof(mac_autoblacklist_info_stru));
        blacklist_info->list_num = 0;
    }
    blacklist_info->mode = mode;
}

OAL_STATIC osal_u32 hmac_blacklist_check_param(const hmac_vap_stru *hmac_vap,
    const osal_u8 *mac_addr, mac_blacklist_info_stru **blacklist_info)
{
    /* 1.1 入参检查 */
    if ((hmac_vap == OAL_PTR_NULL) || (mac_addr == OAL_PTR_NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_autoblacklist_filter_etc::null mac_vap or null mac addr}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    if (hmac_vap->vap_mode != WLAN_VAP_MODE_BSS_AP) {
        oam_warning_log0(0, OAM_SF_ANY, "{hmac_blacklist_check_param::not AP mode.");
        return OAL_SUCC;
    }

    *blacklist_info = hmac_vap->blacklist_info;
    if (*blacklist_info == OAL_PTR_NULL) {
        oam_warning_log1(0, OAM_SF_ANY, "vap_id[%d] {hmac_blacklist_check_param::blacklist_info null!}\r\n",
            hmac_vap->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    return OAL_CONTINUE;
}

/*****************************************************************************
 函 数 名  : hmac_blacklist_is_aged
 功能描述  : 更新表单老化属性
 输出参数  : osal_void
 返 回 值  : OAL_TRUE 已经老化 OAL_FALSE 没有老化
*****************************************************************************/
OAL_STATIC oal_bool_enum_uint8 hmac_blacklist_is_aged(hmac_vap_stru *hmac_vap, mac_blacklist_stru *blacklist)
{
    osal_u32 second;
    mac_blacklist_info_stru      *blacklist_info;
    osal_u8                    *mac_addr;

    /* 1.1 全零地址，属于非法地址 */
    if (hmac_blacklist_mac_is_zero((const osal_u8 *)blacklist->auc_mac_addr) != 0) {
        return OAL_FALSE;
    }

    /* 2.1 老化时间为0表示不需要老化 */
    if (blacklist->aging_time == 0) {
        return OAL_FALSE;
    }

    /* 2.2 没有超过老化时间 */
    second = oal_get_seconds();
    if (second < blacklist->cfg_time + blacklist->aging_time) {
        return OAL_FALSE;
    }

    mac_addr = blacklist->auc_mac_addr;
    oam_info_log4(0, OAM_SF_ANY,
        "{aging time reach delete MAC:=%02X:%02X:%02X:%02X:XX:XX}",
        mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3]);      /* 0,1,2,3:MAC地址索引 */

    /* 3.1 直接从黑名单中删除 */
    blacklist_info = hmac_vap->blacklist_info;
    if (blacklist_info == OAL_PTR_NULL) {
        oam_warning_log1(0, OAM_SF_ANY, "vap_id[%d] {hmac_blacklist_is_aged::blacklist_info null!}\r\n",
            hmac_vap->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }
    if (blacklist_info->mode == CS_BLACKLIST_MODE_BLACK) {
        memset_s(blacklist, sizeof(mac_blacklist_stru), 0, sizeof(mac_blacklist_stru));
        if (blacklist_info->list_num > 0) {
            blacklist_info->list_num--;      /* 2014.7.23 bug fixed */
            oam_info_log1(0, OAM_SF_ANY, "{del from blacklist & num=%d}",
                blacklist_info->list_num);
        }
        return OAL_TRUE;
    }

    /* 4.1 直接从白名单中恢复 */
    if (blacklist_info->mode == CS_BLACKLIST_MODE_WHITE) {
        blacklist->aging_time = 0;
        return OAL_TRUE;
    }

    return OAL_FALSE;
}

/*****************************************************************************
 函 数 名  : hmac_blacklist_get
 功能描述  : 从表单中查找mac地址相同的表单
 输出参数  : osal_void
 返 回 值  : null
*****************************************************************************/
OAL_STATIC osal_u32 hmac_blacklist_get(hmac_vap_stru *hmac_vap, osal_u8 *mac_addr,
    mac_blacklist_stru **ppst_blacklist)
{
    osal_u8                     blacklist_index;
    mac_blacklist_info_stru      *blacklist_info = OAL_PTR_NULL;

    /* 1.1 广播地址，属于非法地址 */
    if (hmac_blacklist_mac_is_bcast((const osal_u8 *)mac_addr) != 0) {
        return OAL_ERR_CODE_SECURITY_MAC_INVALID;
    }

    /* 1.2 全零地址，属于非法地址 */
    if (hmac_blacklist_mac_is_zero((const osal_u8 *)mac_addr) != 0) {
        return OAL_ERR_CODE_SECURITY_MAC_INVALID;
    }

    blacklist_info = hmac_vap->blacklist_info;
    if (blacklist_info == OAL_PTR_NULL) {
        oam_warning_log1(0, OAM_SF_ANY, "vap_id[%d] {hmac_blacklist_get::blacklist_info null!}\r\n",
            hmac_vap->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 2.1 找到表单 */
    for (blacklist_index = 0; blacklist_index < WLAN_BLACKLIST_MAX; blacklist_index++) {
        *ppst_blacklist = &blacklist_info->black_list[blacklist_index];
        if (osal_memcmp((*ppst_blacklist)->auc_mac_addr, mac_addr, OAL_MAC_ADDR_LEN) == 0) {
            break;
        }
        *ppst_blacklist = OAL_PTR_NULL;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_backlist_set_drop
 功能描述  : 减1 drop_counter 值，用以恢复 counter 值 ++
 输出参数  : osal_void
 返 回 值  : null
*****************************************************************************/
OAL_STATIC osal_u32 hmac_backlist_sub_drop(hmac_vap_stru *hmac_vap, osal_u8 *mac_addr)
{
    /* 2014.9.2 Add UT and found no init value set to it ! add initial value null */
    osal_u32  ul_ret;
    mac_blacklist_stru *blacklist = OAL_PTR_NULL;

    ul_ret = hmac_blacklist_get(hmac_vap, mac_addr, &blacklist);

    if (blacklist != OAL_PTR_NULL) {
        if (blacklist->drop_counter > 0) {
            blacklist->drop_counter--;
        }
        ul_ret = OAL_SUCC;
    }
    return ul_ret;
}

/*****************************************************************************
 函 数 名  : hmac_autoblacklist_get
 功能描述  : 从表单中查找mac地址相同的表单
 输出参数  : osal_void
 返 回 值  : null
*****************************************************************************/
OAL_STATIC osal_u32 hmac_autoblacklist_get(hmac_vap_stru *hmac_vap, osal_u8 *mac_addr,
    mac_autoblacklist_stru **ppst_autoblacklist)
{
    osal_u8                         blacklist_index;
    mac_autoblacklist_info_stru      *autoblacklist_info = OAL_PTR_NULL;
    osal_u32 second;
    mac_autoblacklist_stru           *autoblacklist = OAL_PTR_NULL;

    /* 1.1 广播地址，属于非法地址 */
    if (hmac_blacklist_mac_is_bcast((const osal_u8 *)mac_addr) != 0) {
        return OAL_ERR_CODE_SECURITY_MAC_INVALID;
    }

    /* 1.2 全零地址，属于非法地址 */
    if (hmac_blacklist_mac_is_zero((const osal_u8 *)mac_addr) != 0) {
        return OAL_ERR_CODE_SECURITY_MAC_INVALID;
    }

    if (hmac_vap->blacklist_info == OAL_PTR_NULL) {
        oam_error_log1(0, OAM_SF_CFG, "vap_id[%d] {hmac_autoblacklist_get::blacklist_info null.}",
            hmac_vap->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }
    autoblacklist_info = &(hmac_vap->blacklist_info->autoblacklist_info);

    second = oal_get_seconds();
    *ppst_autoblacklist = OAL_PTR_NULL;

    /* 2.1 找到历史表单 */
    for (blacklist_index = 0; blacklist_index < WLAN_BLACKLIST_MAX; blacklist_index++) {
        autoblacklist = &autoblacklist_info->autoblack_list[blacklist_index];
        /* 2.2 无效表单 */
        if (autoblacklist->cfg_time == 0) {
            continue;
        }

        /* 2.2 过期表单直接清0 */
        if (second > autoblacklist->cfg_time + autoblacklist_info->reset_time) {
            if (autoblacklist_info->list_num > 0) {
                autoblacklist_info->list_num--;
            }
            memset_s(autoblacklist, OAL_SIZEOF(mac_autoblacklist_stru), 0, OAL_SIZEOF(mac_autoblacklist_stru));
            continue;
        }

        /* 2.3 有效表单，mac地址比对 */
        if (osal_memcmp(autoblacklist->auc_mac_addr, mac_addr, OAL_MAC_ADDR_LEN) == 0) {
            *ppst_autoblacklist = autoblacklist;
            break;
        }
    }

    if (*ppst_autoblacklist != OAL_PTR_NULL) {
        /* autoblacklist不可能为空，如果为空那说明ppst_autoblacklist也为空，那这个分支不可能进入 */
        oam_info_log4(0, OAM_SF_ANY,
            "vap_id[%d] {Get a history item from auto_blacklist.index=%d.MAC=%02x.%02x.xx.xx.x.x.}",
            hmac_vap->vap_id, blacklist_index, mac_addr[0], mac_addr[1]); /* 0123:MAC地址索引 */
        return OAL_SUCC;
    }

    return hmac_autoblacklist_get_handle(mac_addr, second, ppst_autoblacklist, autoblacklist_info);
}

OAL_STATIC osal_u32 hmac_autoblacklist_get_handle(const osal_u8 *mac_addr, osal_u32 second,
    mac_autoblacklist_stru **ppst_autoblacklist, mac_autoblacklist_info_stru *autoblacklist_info)
{
    osal_u8 blacklist_index;
    osal_u32 cfg_time_old;
    osal_u32 blacklist_index_old;
    mac_autoblacklist_stru *autoblacklist = OAL_PTR_NULL;

    /* 3.1 找到新表单 */
    cfg_time_old = second;
    blacklist_index_old = 0;
    for (blacklist_index = 0; blacklist_index < WLAN_BLACKLIST_MAX; blacklist_index++) {
        autoblacklist = &autoblacklist_info->autoblack_list[blacklist_index];
        /* 2.2 找到空闲表单 */
        if (autoblacklist->cfg_time == 0) {
            autoblacklist_info->list_num++;
            *ppst_autoblacklist = autoblacklist;
            break;
        }

        /* 2.3 记录最老配置的表单 */
        if (autoblacklist->cfg_time < cfg_time_old) {
            cfg_time_old = autoblacklist->cfg_time;
            blacklist_index_old = blacklist_index;
        }
    }

    /* 4.1 如果没有空闲的表单，就使用最老的表单 */
    if (*ppst_autoblacklist == OAL_PTR_NULL) {
        autoblacklist = &autoblacklist_info->autoblack_list[blacklist_index_old];
        *ppst_autoblacklist = autoblacklist;
    }

    /* 5.1 更新表单 */
    oam_info_log4(0, OAM_SF_ANY, "{add to auto_blacklist MAC:=%02X:%02X:%02X:%02X:XX:XX. assoc_count=1.}",
        mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3]);     /* 0,1,2,3:MAC地址索引 */
    if (autoblacklist == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_autoblacklist_get_handle::autoblacklist NULL}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    memcpy_s(autoblacklist->auc_mac_addr, OAL_MAC_ADDR_LEN, mac_addr, OAL_MAC_ADDR_LEN);
    autoblacklist->cfg_time = second;
    autoblacklist->asso_counter = 0; /* bug 1 => 0 fixed */

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_blacklist_update_delete_user
 功能描述  : blacklist更新时 删除用户
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
OAL_STATIC osal_void  hmac_blacklist_update_delete_user(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user)
{
    osal_void *fhook = hmac_get_feature_fhook(HMAC_FHOOK_DFX_USER_CONN_ABNORMAL_RECORD);
    oam_warning_log4(0, OAM_SF_ANY,
        "hmac_blacklist_update_delete_user: send disasoc frame to %2X:%2X:%2X:%2X:XX:XX.",
        hmac_user->user_mac_addr[0], hmac_user->user_mac_addr[1], hmac_user->user_mac_addr[2], /* 1,2:MAC地址索引 */
        hmac_user->user_mac_addr[3]); /* 3:MAC地址索引 */
    oam_warning_log1(0, OAM_SF_ANY, "hmac_blacklist_update_delete_user: blacklist_mode:%d",
        hmac_vap->blacklist_info->mode);
    hmac_mgmt_send_disassoc_frame_etc(hmac_vap, hmac_user->user_mac_addr, MAC_DISAS_LV_SS, OAL_FALSE);

    /* 删除黑名单内用户，需要上报内核 */
    hmac_handle_disconnect_rsp_etc(hmac_vap, hmac_user, MAC_DISAS_LV_SS);

    if (fhook != OSAL_NULL) {
        ((dfx_user_conn_abnormal_record_cb)fhook)(hmac_vap->vap_id, hmac_user->user_mac_addr, AP_BLACKLIST_DEL_STA,
            USER_CONN_OFFLINE);
    }
    hmac_user_del_etc(hmac_vap, hmac_user);

    return;
}

/*****************************************************************************
 函 数 名  : hmac_whitelist_check_user
 功能描述  : 添加白名单删除用户
 输入参数  : 无，遍历所有用户
 输出参数  : osal_void
 返 回 值  : null
*****************************************************************************/
OAL_STATIC osal_u32 hmac_whitelist_check_user(hmac_vap_stru *hmac_vap)
{
    osal_u32                       ret;
    struct osal_list_head             *entry;
    struct osal_list_head             *dlist_tmp;
    hmac_user_stru                  *hmac_user;

    /* 遍历VAP下所有用户，不在白名单里的删除 */
    osal_list_for_each_safe(entry, dlist_tmp, &(hmac_vap->mac_user_list_head)) {
        hmac_user = osal_list_entry(entry, hmac_user_stru, user_dlist);
        if (hmac_user == OAL_PTR_NULL) {
            continue;
        }

        ret = hmac_blacklist_filter_etc(hmac_vap, hmac_user->user_mac_addr);
        if (ret != OAL_TRUE) {
            continue;
        }
        /* 2014.6.30 在如上的hmac_blacklist_filter()中会 drop_counter++ 它不是实际的过滤数，所以--恢复 */
        ret = hmac_backlist_sub_drop(hmac_vap, hmac_user->user_mac_addr);
        if (ret != OAL_SUCC) {
            wifi_printf("vap_id[%d]{hmac_whitelist_check_user::hmac_backlist_sub_drop ret=%u}",
                hmac_vap->vap_id, ret);
        }
        hmac_blacklist_update_delete_user(hmac_vap, hmac_user);
        wifi_printf("hmac_whitelist_check_user:not in whitelist,send DISASSOC,err code[%d]",
            MAC_DISAS_LV_SS);
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_blacklist_vap_check_user_by_macaddr
 功能描述  : 在该vap下检查是否用户存在
             黑名单下，该用户属于assoc用户并且在黑名单中，要删除
             白名单下，该用户属于assoc用户并且不再白名单中，删除
*****************************************************************************/
OAL_STATIC osal_u32 hmac_blacklist_vap_check_user_by_macaddr(hmac_vap_stru *hmac_vap, osal_u8 *mac_addr)
{
    hmac_user_stru               *hmac_user;
    osal_u32                    ret;
    osal_u16                    idx;
    oal_bool_enum_uint8           bool_ret;

    if (hmac_vap->vap_mode != WLAN_VAP_MODE_BSS_AP) {
        wifi_printf("vap_id[%d]{hmac_blacklist_vap_check_user_by_macaddr::not AP mode!}\r\n",
            hmac_vap->vap_id);
        return OAL_SUCC;
    }
    ret = hmac_vap_find_user_by_macaddr_etc(hmac_vap, mac_addr, &idx);
    if (ret != OAL_SUCC) {
        /* 该mac地址在此vap下不存在用户，不需要删除，直接返回成功 */
        return OAL_SUCC;
    }

    hmac_user = (hmac_user_stru *)mac_res_get_hmac_user_etc(idx);
    if (hmac_user == OAL_PTR_NULL) {
        return OAL_FAIL; /* 异常返回 */
    }

    bool_ret = hmac_blacklist_filter_etc(hmac_vap, hmac_user->user_mac_addr);
    if (bool_ret != OAL_TRUE) {
        wifi_printf("vap_id[%d]{hmac_blacklist_vap_check_user_by_macaddr::hmac_blacklist_filter_etc bool_ret=%d}",
            hmac_vap->vap_id, bool_ret);
        return OAL_FAIL;
    }
    /* 2014.6.30 在如上的hmac_blacklist_filter()中会 drop_counter++ 它不是实际的过滤数，所以--恢复 */
    ret = hmac_backlist_sub_drop(hmac_vap, hmac_user->user_mac_addr);
    if (ret != OAL_SUCC) {
        wifi_printf("vap_id[%d]{hmac_blacklist_vap_check_user_by_macaddr::hmac_backlist_sub_drop ret=%u}",
            hmac_vap->vap_id, ret);
    }
    hmac_blacklist_update_delete_user(hmac_vap, hmac_user);
    oam_warning_log1(0, OAM_SF_ANY,
        "hmac_blacklist_vap_check_user_by_macaddr:usr in blacklist,send DISASSOC,err code[%d]", MAC_DISAS_LV_SS);

    return ret;
}

/*****************************************************************************
 函 数 名  : hmac_whitelist_check_all_user
 功能描述  : 更新所有黑白名单下的用户
*****************************************************************************/
OAL_STATIC osal_void hmac_whitelist_check_all_user(hmac_vap_stru *hmac_vap)
{
    osal_u32 ret;

    if (hmac_vap->vap_mode != WLAN_VAP_MODE_BSS_AP) {
        wifi_printf("{hmac_whitelist_check_all_user::not AP mode.");
        return;
    }
    ret = hmac_whitelist_check_user(hmac_vap);
    if (ret != OAL_SUCC) {
        wifi_printf("{hmac_whitelist_check_all_user::hmac_whitelist_check_user ret=%u}", ret);
    }
}

/*****************************************************************************
 函 数 名  : hmac_blacklist_add_etc
 功能描述  : 添加黑/白名单
*****************************************************************************/
OAL_STATIC osal_u32 hmac_blacklist_add_etc(hmac_vap_stru *hmac_vap, osal_u8 *mac_addr, osal_u32 aging_time)
{
    osal_u8                     blacklist_idx;
    mac_blacklist_info_stru      *blacklist_info = OAL_PTR_NULL;
    mac_blacklist_stru           *blacklist = OAL_PTR_NULL;

    osal_u32 ret = hmac_blacklist_check_param(hmac_vap, mac_addr, &blacklist_info);
    if (ret != OAL_CONTINUE) {
        wifi_printf("{hmac_blacklist_add_etc::param error code[%d] \n}", ret);
        return ret;
    }

    /* 3.1 找到表单 */
    for (blacklist_idx = 0; blacklist_idx < WLAN_BLACKLIST_MAX; blacklist_idx++) {
        blacklist = &blacklist_info->black_list[blacklist_idx];
        if (osal_memcmp(blacklist->auc_mac_addr, mac_addr, OAL_MAC_ADDR_LEN) == 0) {
            break;
        }
        blacklist = OAL_PTR_NULL;
    }

    /* 4.1 表单已经存在，只更新老化时间 */
    if (blacklist != OAL_PTR_NULL) {
        wifi_printf("vap_id[%d] {blacklist_add allready exist. update aging = %d}", hmac_vap->vap_id, aging_time);
        blacklist->aging_time = aging_time;
        return OAL_SUCC;
    }

    /* 5.1 找一个地址为空的表单 */
    for (blacklist_idx = 0; blacklist_idx < WLAN_BLACKLIST_MAX; blacklist_idx++) {
        blacklist = &blacklist_info->black_list[blacklist_idx];
        if (hmac_blacklist_mac_is_zero((const osal_u8 *)blacklist->auc_mac_addr) != 0) {
            break;
        }
        blacklist = OAL_PTR_NULL;
    }

    /* 6.1 无可用表单 */
    if (blacklist == OAL_PTR_NULL) {
        wifi_printf("{hmac_blacklist_add_etc:: src mac=%02x.%02x.%02x.%02x.x.x. new count is %d; full return}",
            mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], blacklist_info->list_num); /* 0123 MAC地址 */
        return OAL_ERR_CODE_SECURITY_LIST_FULL;
    }

    /* 7.1 更新表单 */
    if (memcpy_s(blacklist->auc_mac_addr, sizeof(blacklist->auc_mac_addr), mac_addr, OAL_MAC_ADDR_LEN) != EOK) {
        wifi_printf("{hmac_blacklist_add_etc::memcpy_s error}");
    }
    blacklist->cfg_time     = oal_get_seconds();
    blacklist->aging_time   = aging_time;
    blacklist->drop_counter = 0;
    blacklist_info->list_num++;

    wifi_printf("{hmac_blacklist_add_etc:: src mac=%02x.%02x.%02x.%02x.x.x. new count is %d, time=%d}", mac_addr[0],
        mac_addr[1], mac_addr[2], mac_addr[3], blacklist_info->list_num, blacklist->cfg_time); /* 0,1,2,3:索引 */

    /* 黑白名单添加成功后刷新一把用户 */
    if (blacklist_info->mode == CS_BLACKLIST_MODE_BLACK) {
        hmac_blacklist_vap_check_user_by_macaddr(hmac_vap, mac_addr);
    } else if (blacklist_info->mode == CS_BLACKLIST_MODE_WHITE) {
        hmac_whitelist_check_all_user(hmac_vap);
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_blacklist_get_assoc_ap()
 功能描述  : 找到当前mac地址对应关联的ap
 返 回 值  : true/false
*****************************************************************************/
OAL_STATIC oal_bool_enum_uint8 hmac_blacklist_get_assoc_ap(hmac_vap_stru *cur_vap, osal_u8 *mac_addr,
    hmac_vap_stru **assoc_vap)
{
    unref_param(mac_addr);

    *assoc_vap = cur_vap;
    return OAL_TRUE;
}
/*****************************************************************************
 函 数 名  : hmac_blacklist_add_only_etc
 功能描述  : 添加黑/白名单
 输出参数  : osal_void
 返 回 值  : null
*****************************************************************************/
OAL_STATIC osal_u32 hmac_blacklist_add_only_etc(hmac_vap_stru *hmac_vap, osal_u8 *mac_addr, osal_u32 aging_time)
{
    osal_u8                     blacklist_index;
    mac_blacklist_info_stru      *blacklist_info = OAL_PTR_NULL;
    mac_blacklist_stru           *blacklist = OAL_PTR_NULL;
    osal_u32 second;

    osal_u32 ret = hmac_blacklist_check_param(hmac_vap, mac_addr, &blacklist_info);
    if (ret != OAL_CONTINUE) {
        oam_error_log1(0, OAM_SF_ANY, "{hmac_blacklist_add_only_etc::param error code[%d] \n}", ret);
        return ret;
    }

    /* 3.1 找到表单 */
    for (blacklist_index = 0; blacklist_index < WLAN_BLACKLIST_MAX; blacklist_index++) {
        blacklist = &blacklist_info->black_list[blacklist_index];
        if (osal_memcmp(blacklist->auc_mac_addr, mac_addr, OAL_MAC_ADDR_LEN) == 0) {
            break;
        }
        blacklist = OAL_PTR_NULL;
    }

    /* 4.1 表单已经存在，只更新老化时间 */
    if (blacklist != OAL_PTR_NULL) {
        oam_info_log2(0, OAM_SF_ANY, "vap_id[%d] {blacklist_add allready exist. update aging = %d}",
            hmac_vap->vap_id, aging_time);
        blacklist->aging_time = aging_time;
        return OAL_SUCC;
    }

    /* 5.1 找一个地址为空的表单 */
    for (blacklist_index = 0; blacklist_index < WLAN_BLACKLIST_MAX; blacklist_index++) {
        blacklist = &blacklist_info->black_list[blacklist_index];
        if (hmac_blacklist_mac_is_zero((const osal_u8 *)blacklist->auc_mac_addr) != 0) {
            break;
        }
        blacklist = OAL_PTR_NULL;
    }

    /* 6.1 无可用表单 */
    if (blacklist == OAL_PTR_NULL) {
        oam_info_log4(0, OAM_SF_ANY,
            "{hmac_blacklist_add_only_etc:: src mac=%02x.%02x.%02x.x.x.x new count is %d; full return}",
            mac_addr[0], mac_addr[1], mac_addr[2], blacklist_info->list_num); /* 0,1,2:MAC地址索引 */
        return OAL_ERR_CODE_SECURITY_LIST_FULL;
    }

    /* 7.1 更新表单 */
    second = oal_get_seconds();
    if (memcpy_s(blacklist->auc_mac_addr, sizeof(blacklist->auc_mac_addr), mac_addr, OAL_MAC_ADDR_LEN) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_blacklist_add_only_etc::memcpy_s error}");
    }
    blacklist->cfg_time     = second;
    blacklist->aging_time   = aging_time;
    blacklist->drop_counter = 0;
    blacklist_info->list_num++;

    oam_warning_log4(0, OAM_SF_ANY, "{hmac_blacklist_add_only_etc:: src mac=%02x.%02x.x.x.x.x",
        mac_addr[0], mac_addr[1], blacklist_info->list_num, blacklist->cfg_time);    /* 0,1:MAC地址索引 */

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_blacklist_disconnect_etc
 功能描述  : 清除白名单，断连所有的STA
*****************************************************************************/
OAL_STATIC osal_void hmac_blacklist_disconnect_all_etc(hmac_vap_stru *hmac_vap,
    mac_blacklist_info_stru *blacklist_info)
{
    osal_u8 blacklist_index;
    osal_u16 user_idx = 0xffff;
    osal_u32 ret;
    mac_blacklist_stru *blacklist = OAL_PTR_NULL;
    hmac_user_stru *hmac_user = OAL_PTR_NULL;

    if (blacklist_info->mode != CS_BLACKLIST_MODE_WHITE) {
        return;
    }

    /* 白名单列表数量判断 */
    if (blacklist_info->list_num > WLAN_BLACKLIST_MAX) {
        wifi_printf("vap_id[%d] {hmac_blacklist_disconnect_all_etc::list_num exceed 32,list_num=%d.}",
            hmac_vap->vap_id, blacklist_info->list_num);
        return;
    }

    /* 清除白名单列表，断连所有STA */
    for (blacklist_index = 0; blacklist_index < blacklist_info->list_num; blacklist_index++) {
        blacklist = &blacklist_info->black_list[blacklist_index];
        ret = hmac_vap_find_user_by_macaddr_etc(hmac_vap, blacklist->auc_mac_addr, &user_idx);
        if (ret != OAL_SUCC) {
            wifi_printf("vap_id[%d] {hmac_blacklist_disconnect_all_etc::find_user_by_macaddr failed[%d].}",
                hmac_vap->vap_id, ret);
            continue;
        }
        hmac_user = mac_res_get_hmac_user_etc(user_idx);
        if (hmac_user == OAL_PTR_NULL) {
            wifi_printf("vap_id[%d] {hmac_blacklist_disconnect_all_etc::hmac_user null,user_idx=%d.}",
                hmac_vap->vap_id, user_idx);
            continue;
        }
        hmac_blacklist_update_delete_user(hmac_vap, hmac_user);
    }
}

/*****************************************************************************
 函 数 名  : hmac_blacklist_del_etc
 功能描述  : 删除黑/白名单
 输出参数  : osal_void
 返 回 值  : null
*****************************************************************************/
OAL_STATIC osal_u32 hmac_blacklist_del_etc(hmac_vap_stru *hmac_vap, osal_u8 *mac_addr)
{
    osal_u8                     blacklist_index;
    mac_blacklist_stru           *blacklist = OAL_PTR_NULL;
    mac_blacklist_info_stru      *blacklist_info = OAL_PTR_NULL;
    osal_u32                    ul_ret;
    osal_u16                    user_idx   = 0xffff;
    hmac_user_stru               *hmac_user = OAL_PTR_NULL;

    ul_ret = hmac_blacklist_check_param(hmac_vap, mac_addr, &blacklist_info);
    if (ul_ret != OAL_CONTINUE) {
        wifi_printf("{hmac_blacklist_del_etc::param error code[%d] \n}", ul_ret);
        return ul_ret;
    }

    /* 3.1 广播地址，需要删除所有表单 */
    if (hmac_blacklist_mac_is_bcast((const osal_u8 *)mac_addr) != 0) {
        wifi_printf("vap_id[%d] {hmac_blacklist_del_etc::broadcast addr; delete all hmac_blacklist}", hmac_vap->vap_id);

        hmac_blacklist_disconnect_all_etc(hmac_vap, blacklist_info);
        hmac_blacklist_init(hmac_vap, blacklist_info->mode, OAL_TRUE);
        return OAL_SUCC;
    }

    /* 4.1 找到表单 */
    for (blacklist_index = 0; blacklist_index < WLAN_BLACKLIST_MAX; blacklist_index++) {
        blacklist = &blacklist_info->black_list[blacklist_index];
        if (osal_memcmp(blacklist->auc_mac_addr, mac_addr, OAL_MAC_ADDR_LEN) == 0) {
            break;
        }
        blacklist = OAL_PTR_NULL;
    }

    /* 5.1 如果找到表单，删除 */
    if (blacklist != OAL_PTR_NULL) {
        memset_s(blacklist, sizeof(mac_blacklist_stru), 0, sizeof(mac_blacklist_stru));
        blacklist_info->list_num--;
        wifi_printf("{hmac_blacklist_del_etc:: mac=%02x.%02x.%02x.%02x.x.x. new count is %d}", mac_addr[0],
            mac_addr[1], mac_addr[2], mac_addr[3], blacklist_info->list_num); /* 0,1,2,3:MAC地址索引 */

        if (blacklist_info->mode == CS_BLACKLIST_MODE_WHITE) {
            /* 根据mac addr找到sta索引 */
            ul_ret = hmac_vap_find_user_by_macaddr_etc(hmac_vap, mac_addr, &user_idx);
            if (ul_ret != OAL_SUCC) {
                wifi_printf("vap_id[%d] {hmac_blacklist_del_etc::find user failed[%d].}", hmac_vap->vap_id, ul_ret);
                return OAL_PTR_NULL;
            }
            hmac_user = mac_res_get_hmac_user_etc(user_idx);
            if (hmac_user == OAL_PTR_NULL) {
                wifi_printf("vap_id[%d] {hmac_blacklist_del_etc::user null,user_idx=%d.}", hmac_vap->vap_id, user_idx);
                return OAL_ERR_CODE_PTR_NULL;
            }
            hmac_blacklist_update_delete_user(hmac_vap, hmac_user);
            wifi_printf("hmac_blacklist_del_etc:del whitelist,send DISASSOC,err code [%d]", MAC_DISAS_LV_SS);
        }
    } else {
        wifi_printf("{hmac_blacklist_del_etc:: didn't find this mac: %02X:%02X:%02X:%02X:XX:XX}",
            mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3]);     /* 0,1,2,3:MAC地址索引 */
    }
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_blacklist_set_mode_etc
 功能描述  : 黑名单更新
 输出参数  : osal_u32
 返 回 值  : null
*****************************************************************************/
OAL_STATIC osal_u32 hmac_blacklist_set_mode_etc(hmac_vap_stru *hmac_vap, osal_u8 mode)
{
    /* 1.1 入参检查 */
    if (hmac_vap == OAL_PTR_NULL) {
        wifi_printf("{hmac_blacklist_set_mode_etc::null mac_vap}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    if (hmac_vap->vap_mode != WLAN_VAP_MODE_BSS_AP) {
        wifi_printf("{hmac_blacklist_set_mode_etc::not AP mode!}\r\n");
        return OAL_SUCC;
    }

    wifi_printf("vap_id[%d] {hmac_blacklist_set_mode_etc::mode = %d}\r\n",
        hmac_vap->vap_id, mode);
    /* 2.1 根据不同模式增加表单 */
    switch (mode) {
        case CS_BLACKLIST_MODE_NONE:
        case CS_BLACKLIST_MODE_BLACK:
        case CS_BLACKLIST_MODE_WHITE:
            /* ccpriv.sh debug suggest OAL_FALSE: don't flush all bssids in blacklist */
            /* FWK configuration suggest OAL_TRUE:Flush all bssids in blacklist */
            hmac_blacklist_init(hmac_vap, mode, OAL_TRUE); /* OAL_TRUE:Flush blacklist; OAL_FALSE:don't flush */
            break;

        default:
            wifi_printf("vap_id[%d] {unknown mode = %d}", hmac_vap->vap_id, mode);
            return OAL_ERR_CODE_SECURITY_MODE_INVALID;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_blacklist_get_mode
 功能描述  : 获取黑名单模式
 输出参数  : osal_void
 返 回 值  : null
*****************************************************************************/
OAL_STATIC osal_u32 hmac_blacklist_get_mode(hmac_vap_stru *hmac_vap, osal_u8 *mode)
{
    /*  入参检查 */
    if (hmac_vap == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_blacklist_get_mode::null mac_vap}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    if (hmac_vap->vap_mode != WLAN_VAP_MODE_BSS_AP) {
        return OAL_SUCC;
    }

    *((osal_u32 *)mode) =  hmac_vap->blacklist_info->mode;
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_show_autoblacklist_info
 功能描述  : 显示黑名单信息
 输出参数  : osal_void
 返 回 值  : null
*****************************************************************************/
OAL_STATIC osal_void hmac_show_autoblacklist_info(mac_autoblacklist_info_stru *autoblacklist_info)
{
    osal_u8                    blacklist_index;
    osal_u8                   *sa;
    mac_autoblacklist_stru      *autoblacklist;
    osal_s8                    *pc_print_buff;
    osal_u8                    buff_index;

    pc_print_buff = (osal_s8 *)oal_mem_alloc(OAL_MEM_POOL_ID_LOCAL, OAM_REPORT_MAX_STRING_LEN, OAL_TRUE);
    if (pc_print_buff == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_CFG, "{hmac_show_autoblacklist_info::pc_print_buff null.}");
        return;
    }
    memset_s(pc_print_buff, OAM_REPORT_MAX_STRING_LEN, 0, OAM_REPORT_MAX_STRING_LEN);

    /* 2.1 黑名单配置信息 */

    oam_info_log4(0, OAM_SF_ANY,
        "{hmac_show_autoblacklist_info::AUTOBLACKLIST[%d] info: THRESHOLD: %u. RESET_TIME: %u sec. AGING_TIME: %u sec}",
        autoblacklist_info->enabled, autoblacklist_info->ul_threshold,
        autoblacklist_info->reset_time, autoblacklist_info->aging_time);

    buff_index = (osal_u8)snprintf_s((osal_char *)pc_print_buff, OAM_REPORT_MAX_STRING_LEN, OAM_REPORT_MAX_STRING_LEN,
        "\nAUTOBLACKLIST[%u] info:\n"
        "  THRESHOLD: %u\n"
        "  RESET_TIME: %u sec\n"
        "  AGING_TIME: %u sec\n",
        autoblacklist_info->enabled, autoblacklist_info->ul_threshold,
        autoblacklist_info->reset_time, autoblacklist_info->aging_time);

    /* 4.1 打印黑名单表单 */
    for (blacklist_index = 0; blacklist_index < WLAN_BLACKLIST_MAX; blacklist_index++) {
        autoblacklist = &autoblacklist_info->autoblack_list[blacklist_index];
        if (hmac_blacklist_mac_is_zero((const osal_u8 *)autoblacklist->auc_mac_addr) != 0) {
            continue;
        }
        sa = autoblacklist->auc_mac_addr;
        oam_info_log4(0, OAM_SF_ANY,
            "{src mac=%02X:%02X:%02X:%02X:XX:XX}", sa[0], sa[1], sa[2], sa[3]); /* 0,1,2,3:MAC地址索引 */
        oam_info_log2(0, OAM_SF_ANY, "{ ASSO_CNT: %u. cfg_time=%d. }",
            autoblacklist->asso_counter, autoblacklist->cfg_time);

        buff_index += (osal_u8)snprintf_s((osal_char *)pc_print_buff + buff_index,
            (OAM_REPORT_MAX_STRING_LEN - buff_index), (OAM_REPORT_MAX_STRING_LEN - buff_index),
            "\n[%02u] MAC: %02X:%02X:%02X:%02X:XX:XX\n"
            " cfg_time=%u. ASSO_CNT: %u\n",
            blacklist_index,
            autoblacklist->auc_mac_addr[0],
            autoblacklist->auc_mac_addr[1],      /* 1:MAC地址索引 */
            autoblacklist->auc_mac_addr[2],      /* 2:MAC地址索引 */
            autoblacklist->auc_mac_addr[3],      /* 3:MAC地址索引 */
            autoblacklist->cfg_time, autoblacklist->asso_counter);
    }
    dft_report_params_etc((osal_u8 *)pc_print_buff, (osal_u16)osal_strlen((const char *)pc_print_buff),
        SOC_DIAG_MSG_ID_WIFI_FRAME_MAX);
    oal_mem_free(pc_print_buff, OAL_TRUE);
}

/*****************************************************************************
 函 数 名  : hmac_show_blacklist_info_etc
 功能描述  : 显示黑名单信息
 输出参数  : osal_void
 返 回 值  : null
*****************************************************************************/
OAL_STATIC osal_void hmac_show_blacklist_info_etc(hmac_vap_stru *hmac_vap)
{
    mac_blacklist_info_stru      *blacklist_info = OAL_PTR_NULL;
    osal_s8                     *pc_print_buff = OAL_PTR_NULL;
    osal_u32 second;

    /* 1.1 入参检查 */
    if (hmac_vap == OAL_PTR_NULL) {
        wifi_printf("{hmac_show_blacklist_info_etc::null mac_vap}");
        return;
    }
    if (hmac_vap->vap_mode != WLAN_VAP_MODE_BSS_AP) {
        wifi_printf("{hmac_show_blacklist_info_etc::not AP mode!}\r\n");
        return;
    }

    blacklist_info = hmac_vap->blacklist_info;
    if (blacklist_info == OAL_PTR_NULL) {
        wifi_printf("vap_id[%d] {hmac_show_blacklist_info_etc::blacklist_info null.}",
            hmac_vap->vap_id);
        return;
    }

    pc_print_buff = (osal_s8 *)oal_mem_alloc(OAL_MEM_POOL_ID_LOCAL, OAM_REPORT_MAX_STRING_LEN, OAL_TRUE);
    if (pc_print_buff == OAL_PTR_NULL) {
        wifi_printf("vap_id[%d] {hmac_show_blacklist_info_etc::pc_print_buff null.}",
            hmac_vap->vap_id);
        return;
    }
    memset_s(pc_print_buff, OAM_REPORT_MAX_STRING_LEN, 0, OAM_REPORT_MAX_STRING_LEN);

    hmac_show_blacklist_info_fill(hmac_vap, blacklist_info, pc_print_buff);
    oal_mem_free(pc_print_buff, OAL_TRUE);

    /* 4.1 自动黑名单信息 */
    hmac_show_autoblacklist_info(&blacklist_info->autoblacklist_info);

    /* 加入当前时间 */
    second = oal_get_seconds();
    wifi_printf("{ CURR_TIME: %u}", second);
}

OAL_STATIC osal_void hmac_show_blacklist_info_fill(const hmac_vap_stru *hmac_vap,
    mac_blacklist_info_stru *blacklist_info, osal_s8 *pc_print_buff)
{
    osal_u8          *sa = OAL_PTR_NULL;
    osal_u8           buff_index;
    mac_blacklist_stru *blacklist = OAL_PTR_NULL;
    osal_u8           blacklist_index;

    /* 3.1 黑名单模式信息 */
    if (blacklist_info->mode == CS_BLACKLIST_MODE_BLACK) {
        wifi_printf("vap_id[%d] {BLACKLIST[BLACK] num=%d info:}",
            hmac_vap->vap_id, blacklist_info->list_num);
        buff_index = (osal_u8)snprintf_s((osal_char *)pc_print_buff, OAM_REPORT_MAX_STRING_LEN,
            OAM_REPORT_MAX_STRING_LEN, "BLACKLIST[BLACK] num=%u info:\n", blacklist_info->list_num);
    } else if (blacklist_info->mode == CS_BLACKLIST_MODE_WHITE) {
        wifi_printf("vap_id[%d] {BLACKLIST[WHITE] num=%d info:}",
            hmac_vap->vap_id, blacklist_info->list_num);
        buff_index = (osal_u8)snprintf_s((osal_char *)pc_print_buff, OAM_REPORT_MAX_STRING_LEN,
            OAM_REPORT_MAX_STRING_LEN, "BLACKLIST[WHITE] num=%u info:\n", blacklist_info->list_num);
    } else {
        wifi_printf("vap_id[%d] {BLACKLIST not enable! num=%d info:}",
            hmac_vap->vap_id, blacklist_info->list_num);
        buff_index = (osal_u8)snprintf_s((osal_char *)pc_print_buff, OAM_REPORT_MAX_STRING_LEN,
            OAM_REPORT_MAX_STRING_LEN, "BLACKLIST not enable! num=%u info:\n", blacklist_info->list_num);
    }

    /* 5.1 打印黑名单表单 */
    for (blacklist_index = 0; blacklist_index < WLAN_BLACKLIST_MAX; blacklist_index++) {
        blacklist = &blacklist_info->black_list[blacklist_index];
        if (hmac_blacklist_mac_is_zero((const osal_u8 *)blacklist->auc_mac_addr) != 0) {
            continue;
        }
        sa = blacklist->auc_mac_addr;
        wifi_printf("{src mac=%02X:%02X:%02X:%02X:XX:XX}", sa[0], sa[1], sa[2], sa[3]); /* 0,1,2,3:MAC地址索引 */
        wifi_printf("{ CFG_TIME: %u. AGE_TIME: %u. DROP_CNT: %u.}",
            blacklist->cfg_time, blacklist->aging_time, blacklist->drop_counter);

        buff_index += (osal_u8)snprintf_s((osal_char *)pc_print_buff + buff_index,
            (OAM_REPORT_MAX_STRING_LEN - buff_index), (OAM_REPORT_MAX_STRING_LEN - buff_index),
            "\n[%02u] MAC: %02X:%02X:%02X:%02X:XX:XX\n"
            " CFG_TIME: %u\t AGE_TIME: %u\t DROP_CNT: %u\n",
            blacklist_index,
            blacklist->auc_mac_addr[0],
            blacklist->auc_mac_addr[1],     /* 1:MAC地址索引 */
            blacklist->auc_mac_addr[2],     /* 2:MAC地址索引 */
            blacklist->auc_mac_addr[3],     /* 3:MAC地址索引 */
            blacklist->cfg_time, blacklist->aging_time, blacklist->drop_counter);
    }

    dft_report_params_etc((osal_u8 *)pc_print_buff, (osal_u16)osal_strlen((const char *)pc_print_buff),
        SOC_DIAG_MSG_ID_WIFI_FRAME_MAX);
    return;
}

/*****************************************************************************
 函 数 名  : hmac_blacklist_filter_etc
 功能描述  : 黑名单过滤
 输入参数  : mac地址
 输出参数  : osal_void
 返 回 值  : OAL_TRUE 需要被过滤; OAL_FALSE 不需要被过滤
*****************************************************************************/
OAL_STATIC oal_bool_enum_uint8 hmac_blacklist_filter_etc(hmac_vap_stru *hmac_vap, osal_u8 *mac_addr)
{
    mac_blacklist_stru           *blacklist = OAL_PTR_NULL;
    mac_blacklist_info_stru      *blacklist_info = OAL_PTR_NULL;
    osal_u32                    ul_ret;
    oal_bool_enum_uint8           b_ret;

    /* 1.1 入参检查 */
    if ((hmac_vap == OAL_PTR_NULL) || (mac_addr == OAL_PTR_NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_blacklist_filter_etc::null mac_vap or null mac addr}");
        return OAL_FALSE;
    }

    blacklist_info = hmac_vap->blacklist_info;
    /* 1.1 功能没有开启，不需要过滤 */
    if (blacklist_info == OAL_PTR_NULL || blacklist_info->mode == CS_BLACKLIST_MODE_NONE) {
        return OAL_FALSE;
    }

    /* 2.1 黑名单模式下 */
    if (blacklist_info->mode == CS_BLACKLIST_MODE_BLACK) {
        ul_ret = hmac_blacklist_get(hmac_vap, mac_addr, &blacklist);
        /* 表示mac_addr地址为广播地址或者全零地址, 不过滤 */
        if (ul_ret != OAL_SUCC) {
            oam_warning_log4(0, OAM_SF_ANY,
                "vap_id[%d] {hmac_blacklist_filter_etc::there is no MAC=%02x.%02x.%02x.xx.xx.xx in blacklist}",
                hmac_vap->vap_id, mac_addr[0], mac_addr[1], mac_addr[2]); /* 0,1,2,3:MAC地址索引 */
            return OAL_FALSE;
        }
        /* 2.2 如果找不到表单，不需要过滤 */
        if (blacklist == OAL_PTR_NULL) {
            return OAL_FALSE;
        }

        /* 2.3 老化时间到了，不需要过滤 */
        b_ret = hmac_blacklist_is_aged(hmac_vap, blacklist);
        if (b_ret == OAL_TRUE) {
            return OAL_FALSE;
        }
        /* 2.4 其他情况下都需要过滤 */
        blacklist->drop_counter++;
        oam_info_log4(0, OAM_SF_ANY,
            "{hmac_blacklist_filter_etc::blacklist_filter drop_counter = %d MAC:=%02x.%02x.%02x.xx.xx.xx}",
            blacklist->drop_counter, mac_addr[0], mac_addr[1], mac_addr[2]); /* 0,1,2,3:MAC地址索引 */

        return OAL_TRUE;
    }

    return hmac_blacklit_filter_check_white(hmac_vap, mac_addr, blacklist_info);
}

OAL_STATIC oal_bool_enum_uint8 hmac_blacklit_filter_check_white(hmac_vap_stru *hmac_vap, osal_u8 *mac_addr,
    const mac_blacklist_info_stru *blacklist_info)
{
    osal_u32           ul_ret;
    oal_bool_enum_uint8  b_ret;
    mac_blacklist_stru  *blacklist = OAL_PTR_NULL;

    /* 3.1 白名单模式下 */
    if (blacklist_info->mode == CS_BLACKLIST_MODE_WHITE) {
        ul_ret = hmac_blacklist_get(hmac_vap, mac_addr, &blacklist);
        /* 表示mac_addr地址为广播地址或者全零地址, 不过滤 */
        if (ul_ret != OAL_SUCC) {
            return OAL_FALSE;
        }
        /* 3.2 如果找不到表单，需要过滤 */
        if (blacklist == OAL_PTR_NULL) {
            oam_warning_log4(0, OAM_SF_ANY,
                "{hmac_blacklit_filter_check_white::whitelist_filter MAC:=%02X:%02X:%02X:%02X:XX:XX}",
                mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3]); /* 0,1,2,3:MAC地址索引 */
            return OAL_TRUE;
        }

        /* 3.3 老化时间到了，不需要过滤 */
        b_ret = hmac_blacklist_is_aged(hmac_vap, blacklist);
        if (b_ret == OAL_TRUE) {
            oam_info_log4(0, OAM_SF_ANY,
                "{hmac_blacklit_filter_check_white::aging reach. go through=%02X:%02X:%02X:%02X:XX:XX}",
                mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3]); /* 0,1,2,3:MAC地址索引 */
            return OAL_FALSE;
        }

        /* 3.4 老化时间没有到，需要过滤 */
        if (blacklist->aging_time != 0) {
            oam_info_log4(0, OAM_SF_ANY,
                "{hmac_blacklit_filter_check_white::aging not zero; whilte_filter MAC=%02X:%02X:%02X:%02X:XX:XX}",
                mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3]); /* 0,1,2,3:MAC地址索引 */

            return OAL_TRUE;
        }
    }

    /* 不满足黑白名单过滤条件的，均不需要过滤 */
    return OAL_FALSE;
}

/*****************************************************************************
 函 数 名  : hmac_autoblacklist_filter_etc
 功能描述  : 黑名单过滤
 输入参数  : mac地址
 输出参数  : osal_void
 返 回 值  : OAL_TRUE 需要被过滤; OAL_FALSE 不需要被过滤
*****************************************************************************/
OAL_STATIC osal_void hmac_autoblacklist_filter_etc(hmac_vap_stru *hmac_vap, osal_u8 *mac_addr)
{
    mac_autoblacklist_stru           *autoblacklist = OAL_PTR_NULL; /* 添加初始化值，便于做UT打桩后此值是null */
    mac_autoblacklist_info_stru      *autoblacklist_info = OAL_PTR_NULL;
    osal_u32                        ret;
    mac_blacklist_stru               *blacklist = OAL_PTR_NULL;
    mac_blacklist_info_stru          *blacklist_info = OAL_PTR_NULL;

    ret = hmac_blacklist_check_param(hmac_vap, mac_addr, &blacklist_info);
    if (ret != OAL_CONTINUE) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_autoblacklist_filter_etc::param error code \n}");
        return;
    }

    autoblacklist_info = &(blacklist_info->autoblacklist_info);
    /* 1.1 功能没有开启 */
    if (autoblacklist_info->enabled == 0) {
        return;
    }

    /* 1.2 检查是否已经在黑名单中, 若在黑名单中，直接返回 */
    ret = hmac_blacklist_get(hmac_vap, mac_addr, &blacklist);
    if (blacklist != OAL_PTR_NULL) {
        oam_info_log1(0, OAM_SF_ANY, "{hmac_autoblacklist_filter_etc:: hmac_blacklist_get ret %u}", ret);
        return;
    }

    oam_info_log4(0, OAM_SF_ANY,
        "{hmac_autoblacklist_filter_etc:: MAC:=%02X:%02X:%02X:%02X:XX:XX}",
        mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3]); /* 0,1,2,3:MAC地址索引 */

    /* 2.1 找到关联请求统计表单  */
    ret = hmac_autoblacklist_get(hmac_vap, mac_addr, &autoblacklist);
    if (ret != OAL_SUCC) {
        oam_error_log2(0, OAM_SF_ANY, "vap_id[%d] {hmac_autoblacklist_get failed %d}", hmac_vap->vap_id, ret);
        return;
    }
    /* 2.2 如果找不到表单，不处理 */
    if (autoblacklist == OAL_PTR_NULL) {
        oam_error_log1(0, OAM_SF_ANY, "vap_id[%d] {Can't find item to add}", hmac_vap->vap_id);
        return;
    }

    /* 3.1 关联计数器判断 */
    if (++autoblacklist->asso_counter > autoblacklist_info->ul_threshold) {
        oam_info_log3(0, OAM_SF_ANY,
            "vap_id[%d] {hmac_autoblacklist_filter_etc: asso_counter=%d. threshold=%d. add to blacklist}",
            hmac_vap->vap_id, autoblacklist->asso_counter, autoblacklist_info->ul_threshold);
        ret = hmac_blacklist_add_etc(hmac_vap, mac_addr, autoblacklist_info->aging_time);
        if (ret != OAL_SUCC) {
            oam_error_log2(0, OAM_SF_ANY, "vap_id[%d] {hmac_blacklist_add_etc failed %d}", hmac_vap->vap_id,
                ret);
            return;
        }
        /* 3.2 删除统计表单 */
        memset_s(autoblacklist, OAL_SIZEOF(mac_autoblacklist_stru), 0, OAL_SIZEOF(mac_autoblacklist_stru));
        if (autoblacklist_info->list_num > 0) {
            autoblacklist_info->list_num--;
        }
    }

    return;
}

OAL_STATIC osal_u32 hmac_ap_up_rx_mgmt_blacklist_filter(hmac_vap_stru *hmac_vap, osal_u8 *mac_hdr)
{
    osal_u8                  *sa = OAL_PTR_NULL;
    oal_bool_enum_uint8         blacklist_result;
    osal_u8 mgmt_frm_type = mac_get_frame_sub_type(mac_hdr);

    mac_rx_get_sa((mac_ieee80211_frame_stru *)mac_hdr, &sa);

    /* 自动加入黑名单检查 */
    if ((mgmt_frm_type == WLAN_FC0_SUBTYPE_ASSOC_REQ) || (mgmt_frm_type == WLAN_FC0_SUBTYPE_REASSOC_REQ)) {
        hmac_autoblacklist_filter_etc(hmac_vap, sa);
    }

    /* 黑名单过滤检查 */
    blacklist_result = hmac_blacklist_filter_etc(hmac_vap, sa);
    if ((blacklist_result == OAL_TRUE) && (mgmt_frm_type != WLAN_FC0_SUBTYPE_AUTH)) {
        return OAL_SUCC;
    }
    return OAL_FAIL;
}

/*****************************************************************************
 函 数 名  : hmac_config_set_blacklist_mode_etc
 功能描述  : 设置黑名单模式
 输入参数  : event_hdr: 事件头
           len      : 参数长度
           param    : 参数
 输出参数  : 无
*****************************************************************************/
OAL_STATIC osal_s32 hmac_config_set_blacklist_mode_etc(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    osal_u32 ret;
    osal_u32 *cfg_mode;

    if (OAL_UNLIKELY(hmac_vap == OAL_PTR_NULL || msg->data == OAL_PTR_NULL)) {
        wifi_printf("{hmac_config_set_blacklist_mode_etc:: hmac_vap/param is null ptr!}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    cfg_mode = (osal_u32 *)msg->data;
    if (hmac_vap->vap_mode != WLAN_VAP_MODE_BSS_AP) {
        wifi_printf("{hmac_config_set_blacklist_mode_etc::invalid vap mode=%d}", hmac_vap->vap_mode);
        return OAL_SUCC;
    }
    ret = hmac_blacklist_set_mode_etc(hmac_vap, (osal_u8)(*cfg_mode));
    if (ret != OAL_SUCC) {
        wifi_printf("vap_id[%d] {hmac_blacklist_set_mode_etc fail: ret=%d; mode=%d}",
            hmac_vap->vap_id, ret, *cfg_mode);
        return (osal_s32)ret;
    }
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_config_get_blacklist_mode
 功能描述  : 读取黑名单模式
 输入参数  : event_hdr: 事件头
           len      : 参数长度
           param    : 参数
 输出参数  : 无
*****************************************************************************/
OAL_STATIC osal_s32  hmac_config_get_blacklist_mode(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    osal_u32 ret;
    if (OAL_UNLIKELY(hmac_vap == OAL_PTR_NULL || msg->data == OAL_PTR_NULL)) {
        oam_error_log0(0, OAM_SF_CFG, "{hmac_config_get_blacklist_mode:: hmac_vap/param is null ptr!}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    if (hmac_vap->vap_mode != WLAN_VAP_MODE_BSS_AP) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{hmac_config_get_blacklist_mode::invalid vap mode=%d}", hmac_vap->vap_mode);
        return OAL_SUCC;
    }
    msg->rsp_len = OAL_SIZEOF(osal_s32);
    ret = hmac_blacklist_get_mode(hmac_vap, msg->rsp);
    if (ret != OAL_SUCC) {
        oam_error_log3(0, OAM_SF_ANY,
            "vap_id[%d] {hmac_blacklist_get_mode fail: ret=%d; mode=%d}", hmac_vap->vap_id, ret, *(msg->rsp));
        return (osal_s32)ret;
    }
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_config_blacklist_add_etc
 功能描述  : 增加黑名单
 输入参数  : event_hdr: 事件头
           len      : 参数长度
           param    : 参数
 输出参数  : 无
*****************************************************************************/
OAL_STATIC osal_s32  hmac_config_blacklist_add_etc(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    osal_u32 ret;
    mac_blacklist_stru *blklst = OAL_PTR_NULL;
    hmac_vap_stru *assoc_vap = OAL_PTR_NULL;
    oal_bool_enum_uint8 en_assoc_vap;

    if (OAL_UNLIKELY(hmac_vap == OAL_PTR_NULL || msg->data == OAL_PTR_NULL)) {
        wifi_printf("{hmac_config_blacklist_add_etc:: hmac_vap/param is null ptr!}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    blklst = (mac_blacklist_stru *)msg->data;
    if (hmac_vap->vap_mode != WLAN_VAP_MODE_BSS_AP) {
        wifi_printf("{hmac_config_blacklist_add_etc::invalid vap mode=%d!}", hmac_vap->vap_mode);
        return OAL_SUCC;
    }
    en_assoc_vap = hmac_blacklist_get_assoc_ap(hmac_vap, blklst->auc_mac_addr, &assoc_vap);
    if (en_assoc_vap == OAL_TRUE) {
        ret = hmac_blacklist_add_etc(assoc_vap, blklst->auc_mac_addr, blklst->aging_time);
    } else {
        ret = hmac_blacklist_add_etc(hmac_vap, blklst->auc_mac_addr, blklst->aging_time);
    }
    if (ret != OAL_SUCC) {
        wifi_printf("vap_id[%d] {hmac_blacklist_add_etc fail: ret=%d;}\r\n", hmac_vap->vap_id, ret);
        return (osal_s32)ret;
    }
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_config_blacklist_add_only_etc
 功能描述  : 增加黑名单
 输入参数  : hmac_vap  :
           len     : 参数长度
           param   : 参数
 输出参数  : 无
*****************************************************************************/
OAL_STATIC osal_s32 hmac_config_blacklist_add_only_etc(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    osal_u32 ret;
    mac_blacklist_stru *blklst = OAL_PTR_NULL;
    hmac_vap_stru *assoc_vap = OAL_PTR_NULL;
    oal_bool_enum_uint8 en_assoc_vap;

    if (OAL_UNLIKELY(hmac_vap == OAL_PTR_NULL || msg->data == OAL_PTR_NULL)) {
        oam_error_log0(0, OAM_SF_CFG, "{hmac_config_blacklist_add_only_etc:: hmac_vap/param is null ptr!}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    blklst	= (mac_blacklist_stru *)msg->data;
    if (hmac_vap->vap_mode != WLAN_VAP_MODE_BSS_AP) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{hmac_config_blacklist_add_only_etc::invalid vap mode=%d!}", hmac_vap->vap_mode);
        return OAL_SUCC;
    }
    en_assoc_vap = hmac_blacklist_get_assoc_ap(hmac_vap, blklst->auc_mac_addr, &assoc_vap);
    if (en_assoc_vap == OAL_TRUE) {
        ret = hmac_blacklist_add_only_etc(assoc_vap, blklst->auc_mac_addr, 0);
    } else {
        ret = hmac_blacklist_add_only_etc(hmac_vap, blklst->auc_mac_addr, 0);
    }
    if (ret != OAL_SUCC) {
        oam_error_log2(0, OAM_SF_ANY,
            "vap_id[%d] {hmac_blacklist_add_only_etc fail: ret=%d;}\r\n", hmac_vap->vap_id, ret);
        return (osal_s32)ret;
    }
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_config_blacklist_del_etc
 功能描述  : 删除黑名单
 输入参数  : event_hdr: 事件头
           len      : 参数长度
           param    : 参数
 输出参数  : 无
*****************************************************************************/
OAL_STATIC osal_u32  hmac_config_blacklist_del_etc(hmac_vap_stru *hmac_vap, osal_u16 len, osal_u8 *param)
{
    osal_u32 ret;
    hmac_vap_stru *assoc_vap = OAL_PTR_NULL;
    oal_bool_enum_uint8 en_assoc_vap;

    unref_param(len);

    en_assoc_vap = hmac_blacklist_get_assoc_ap(hmac_vap, param, &assoc_vap);
    if (en_assoc_vap == OAL_TRUE) {
        ret = hmac_blacklist_del_etc(assoc_vap, param);
    } else {
        ret = hmac_blacklist_del_etc(hmac_vap, param);
    }
    if (ret != OAL_SUCC) {
        wifi_printf("vap_id[%d] {hmac_config_blacklist_del_etc::blacklist_del fail: ret=%d;}", hmac_vap->vap_id, ret);
        return ret;
    }
    return OAL_SUCC;
}

OAL_STATIC osal_s32 hmac_config_blacklist_del(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    if (OAL_UNLIKELY(hmac_vap == OAL_PTR_NULL || msg->data == OAL_PTR_NULL)) {
        wifi_printf("{hmac_config_blacklist_del:: hmac_vap/param is null ptr!}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    if (hmac_vap->vap_mode != WLAN_VAP_MODE_BSS_AP) {
        wifi_printf("{hmac_config_blacklist_del::invalid vap mode=%d!}", hmac_vap->vap_mode);
        return OAL_SUCC;
    }
    return (osal_s32)hmac_config_blacklist_del_etc(hmac_vap, msg->data_len, msg->data);
}

OAL_STATIC osal_s32 hmac_config_blacklist_clr(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    errno_t err;
    if (OAL_UNLIKELY(hmac_vap == OAL_PTR_NULL || msg->data == OAL_PTR_NULL)) {
        oam_error_log0(0, OAM_SF_CFG, "{hmac_config_blacklist_clr:: hmac_vap/param is null ptr!}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    if (hmac_vap->vap_mode != WLAN_VAP_MODE_BSS_AP) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{hmac_config_blacklist_clr::invalid vap mode=%d!}", hmac_vap->vap_mode);
        return OAL_SUCC;
    }
    err = memcpy_s(msg->data, OAL_MAC_ADDR_LEN, BROADCAST_MACADDR, OAL_MAC_ADDR_LEN);
    if (err != EOK) {
        oam_warning_log0(0, OAM_SF_CFG, "{hmac_config_blacklist_clr::memcpy_s failed}");
        return OAL_FAIL;
    }
    return (osal_s32)hmac_config_blacklist_del_etc(hmac_vap, msg->data_len, msg->data);
}

/*****************************************************************************
 函 数 名  : hmac_config_show_blacklist_etc
 功能描述  : 黑名单信息打印
 输入参数  : event_hdr: 事件头
           len      : 参数长度
           param  : 参数
 输出参数  : 无
*****************************************************************************/
OAL_STATIC osal_s32 hmac_config_show_blacklist_etc(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    unref_param(msg);

    if (hmac_vap->vap_mode != WLAN_VAP_MODE_BSS_AP) {
        wifi_printf("{hmac_config_show_blacklist_etc::invalid vap mode=%d!}", hmac_vap->vap_mode);
        return OAL_SUCC;
    }
    hmac_show_blacklist_info_etc(hmac_vap);
    return OAL_SUCC;
}

/*****************************************************************************
 功能描述 : 释放vap级hmac vap stru中的blacklist指针
*****************************************************************************/
OAL_STATIC osal_bool hmac_blacklist_free_pointer(osal_void *notify_data)
{
    mac_blacklist_info_stru *blacklist_info = OAL_PTR_NULL;

    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)notify_data;
    if (hmac_vap == OAL_PTR_NULL) {
        return OSAL_FALSE;
    }
    blacklist_info = hmac_vap->blacklist_info;
    if (blacklist_info == OAL_PTR_NULL) {
        return OSAL_FALSE;
    }

    if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_AP) {
        memset_s(blacklist_info, OAL_SIZEOF(mac_blacklist_info_stru), 0, OAL_SIZEOF(mac_blacklist_info_stru));
        blacklist_info->blacklist_device_index = 0xFF;
        blacklist_info->blacklist_vap_index = 0xFF;
        hmac_blacklist_info_deinit(hmac_vap);
    }
    return OSAL_TRUE;
}

osal_u32 hmac_blacklist_register_init(osal_void)
{
    /* 消息注册 */
    static const msg_hook_array_stru msg_hook_array[] = {
        {WLAN_MSG_W2H_CFG_ADD_BLACK_LIST, hmac_config_blacklist_add_etc},
        {WLAN_MSG_W2H_CFG_ADD_BLACK_LIST_ONLY, hmac_config_blacklist_add_only_etc},
        {WLAN_MSG_W2H_CFG_BLACKLIST_SHOW, hmac_config_show_blacklist_etc},
        {WLAN_MSG_W2H_CFG_BLACKLIST_MODE, hmac_config_set_blacklist_mode_etc},
        {WLAN_MSG_W2H_CFG_GET_BLACKLIST_MODE, hmac_config_get_blacklist_mode},
        {WLAN_MSG_W2H_CFG_CLR_BLACK_LIST, hmac_config_blacklist_clr},
        {WLAN_MSG_W2H_CFG_DEL_BLACK_LIST, hmac_config_blacklist_del},
    };
    /* 注册监听 */
    frw_util_notifier_register(WLAN_UTIL_NOTIFIER_EVENT_DOWN_VAP, hmac_blacklist_free_pointer);
    frw_msg_hook_register_array(msg_hook_array, osal_array_size(msg_hook_array));
    return OAL_SUCC;
}

osal_void hmac_blacklist_register_deinit(osal_void)
{
    /* 去注册监听 */
    frw_util_notifier_unregister(WLAN_UTIL_NOTIFIER_EVENT_DOWN_VAP, hmac_blacklist_free_pointer);
    /* 消息去注册 */
    frw_msg_hook_unregister(WLAN_MSG_W2H_CFG_ADD_BLACK_LIST);
    frw_msg_hook_unregister(WLAN_MSG_W2H_CFG_ADD_BLACK_LIST_ONLY);
    frw_msg_hook_unregister(WLAN_MSG_W2H_CFG_BLACKLIST_SHOW);
    frw_msg_hook_unregister(WLAN_MSG_W2H_CFG_BLACKLIST_MODE);
    frw_msg_hook_unregister(WLAN_MSG_W2H_CFG_GET_BLACKLIST_MODE);
    frw_msg_hook_unregister(WLAN_MSG_W2H_CFG_CLR_BLACK_LIST);
    frw_msg_hook_unregister(WLAN_MSG_W2H_CFG_DEL_BLACK_LIST);
    return;
}

OAL_STATIC osal_void hmac_blacklist_feature_hook_register(osal_void)
{
    /* 对外接口注册 */
    hmac_feature_hook_register(HMAC_FHOOK_RX_MGMT_BLACKLIST_FILTER, hmac_ap_up_rx_mgmt_blacklist_filter);
    hmac_feature_hook_register(HMAC_FHOOK_BLACKLIST_FILTER_ETC, hmac_blacklist_filter_etc);
    return;
}

OAL_STATIC osal_void hmac_blacklist_feature_hook_unregister(osal_void)
{
    /* 对外接口去注册 */
    hmac_feature_hook_unregister(HMAC_FHOOK_RX_MGMT_BLACKLIST_FILTER);
    hmac_feature_hook_unregister(HMAC_FHOOK_BLACKLIST_FILTER_ETC);
    return;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
