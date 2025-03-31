/*
 * Copyright (c) CompanyNameMagicTag 2014-2023. All rights reserved.
 * 文 件 名   : hmac_protection.c
 * 生成日期   : 2014年1月18日
 * 功能描述   : 放置与保护相关的函数
 */


/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "hmac_protection.h"
#include "hmac_user.h"
#include "hmac_main.h"
#include "hmac_vap.h"
#include "mac_vap_ext.h"
#include "mac_ie.h"
#include "hmac_config.h"
#include "hmac_beacon.h"
#include "msg_protect_mode_rom.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_HMAC_PROTECTION_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

/*****************************************************************************
 函 数 名  : hmac_user_protection_sync_data
 功能描述  : 同步保护相关的参数到Dmac
 输入参数  : hmac_vap : hmac vap结构体指针
 输出参数  : 无
 返 回 值  : osal_u32
***************************************************************************/
osal_u32 hmac_user_protection_sync_data(hmac_vap_stru *hmac_vap)
{
    oal_bool_enum_uint8 full_protec_act;
    oal_bool_enum_uint8 non_gf_ent_prest;
    oal_bool_enum_uint8 rifs_mode;
    oal_bool_enum_uint8 ht_protection;
    osal_u32 ret = OAL_SUCC;

    /* 更新vap的en_dot11NonGFEntitiesPresent字段 */
    non_gf_ent_prest = (hmac_vap->protection.sta_non_gf_num != 0) ? OAL_TRUE : OAL_FALSE;
    mac_mib_set_non_gf_entities_present(hmac_vap, non_gf_ent_prest);

    /* 更新vap的en_dot11LSIGTXOPFullProtectionActivated字段 */
    full_protec_act = (hmac_vap->protection.sta_no_lsig_txop_num == 0) ? OAL_TRUE : OAL_FALSE;
    mac_mib_set_lsig_txop_full_protection_activated(hmac_vap, full_protec_act);

    /* 更新vap的en_dot11HTProtection和en_dot11RIFSMode字段 */
    if (hmac_vap->protection.sta_non_ht_num != 0) {
        ht_protection = WLAN_MIB_HT_NON_HT_MIXED;
        rifs_mode     = OAL_FALSE;
    } else if (hmac_vap->protection.obss_non_ht_present == OAL_TRUE) {
        ht_protection = WLAN_MIB_HT_NONMEMBER_PROTECTION;
        rifs_mode     = OAL_FALSE;
    } else if ((hmac_vap->channel.en_bandwidth != WLAN_BAND_WIDTH_20M) &&
        (hmac_vap->protection.sta_20m_only_num != 0)) {
        ht_protection = WLAN_MIB_HT_20MHZ_PROTECTION;
        rifs_mode     = OAL_TRUE;
    } else {
        ht_protection = WLAN_MIB_HT_NO_PROTECTION;
        rifs_mode     = OAL_TRUE;
    }

    mac_mib_set_ht_protection(hmac_vap, ht_protection);
    mac_mib_set_rifs_mode(hmac_vap, rifs_mode);

    if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_AP) {
        ret = hmac_protection_update_mib_ap(hmac_vap);
    }

#ifdef _PRE_WIFI_DEBUG
    mac_dump_protection_etc(hmac_vap);
#endif

    return ret;
}

/*****************************************************************************
 函 数 名  : hmac_protection_del_user_stat_legacy_ap
 功能描述  : 删除保护模式相关user统计(legacy)
 输入参数  : hmac_vap  : mac vap结构体指针
             hmac_user : mac user结构体指针
 输出参数  : 无
 返 回 值  : osal_u32
*****************************************************************************/
OAL_STATIC osal_u32  hmac_protection_del_user_stat_legacy_ap(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user)
{
    mac_protection_stru    *protection = &(hmac_vap->protection);

    if (hmac_user == OAL_PTR_NULL) {
        oam_error_log2(0, OAM_SF_ANY,
            "vap_id[%d] hmac_protection_del_user_stat_legacy_ap::Get Hmac_user(idx=%d) NULL POINT!",
            hmac_vap->vap_id, hmac_user->assoc_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 如果去关联的站点不支持ERP */
    if ((hmac_user->hmac_cap_info.erp == OAL_FALSE) &&
        (hmac_user->user_stats_flag.no_erp_stats_flag == OAL_TRUE) &&
        (protection->sta_non_erp_num != 0)) {
        protection->sta_non_erp_num--;
    }

    /* 如果去关联的站点不支持short preamble */
    if ((hmac_user->hmac_cap_info.short_preamble == OAL_FALSE) &&
        (hmac_user->user_stats_flag.no_short_preamble_stats_flag == OAL_TRUE) &&
        (protection->sta_no_short_preamble_num != 0)) {
        protection->sta_no_short_preamble_num--;
    }

    /* 如果去关联的站点不支持short slot */
    if ((hmac_user->hmac_cap_info.short_slot_time == OAL_FALSE) &&
        (hmac_user->user_stats_flag.no_short_slot_stats_flag == OAL_TRUE) &&
        (protection->sta_no_short_slot_num != 0)) {
        protection->sta_no_short_slot_num--;
    }

    hmac_user->user_stats_flag.no_short_slot_stats_flag     = OAL_FALSE;
    hmac_user->user_stats_flag.no_short_preamble_stats_flag = OAL_FALSE;
    hmac_user->user_stats_flag.no_erp_stats_flag            = OAL_FALSE;

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_protection_del_user_stat_ht_ap
 功能描述  : 删除保护模式相关user统计(ht)
 输入参数  : hmac_vap  : mac vap结构体指针
             hmac_user : mac user结构体指针
 输出参数  : 无
 返 回 值  : osal_u32
*****************************************************************************/
OAL_STATIC osal_u32  hmac_protection_del_user_stat_ht_ap(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user)
{
    mac_user_ht_hdl_stru   *pst_ht_hdl     = &(hmac_user->ht_hdl);
    mac_protection_stru    *protection = &(hmac_vap->protection);

    /* 如果去关联的站点不支持HT */
    if ((pst_ht_hdl->ht_capable == OAL_FALSE) &&
        (hmac_user->user_stats_flag.no_ht_stats_flag == OAL_TRUE) &&
        (protection->sta_non_ht_num != 0)) {
        protection->sta_non_ht_num--;
    } else { /* 支持HT */
        /* 如果去关联的站点不支持20/40Mhz频宽 */
        if ((pst_ht_hdl->supported_channel_width == OAL_FALSE) &&
            (hmac_user->user_stats_flag.only_stats_20m_flag == OAL_TRUE) &&
            (protection->sta_20m_only_num != 0)) {
            protection->sta_20m_only_num--;
        }

        /* 如果去关联的站点不支持GF */
        if ((pst_ht_hdl->ht_green_field == OAL_FALSE) &&
            (hmac_user->user_stats_flag.no_gf_stats_flag == OAL_TRUE) &&
            (protection->sta_non_gf_num != 0)) {
            protection->sta_non_gf_num--;
        }

        /* 如果去关联的站点不支持L-SIG TXOP Protection */
        if ((pst_ht_hdl->lsig_txop_protection == OAL_FALSE) &&
            (hmac_user->user_stats_flag.no_lsig_txop_stats_flag == OAL_TRUE) &&
            (protection->sta_no_lsig_txop_num != 0)) {
            protection->sta_no_lsig_txop_num--;
        }

        /* 如果去关联的站点不支持40Mhz cck */
        if ((pst_ht_hdl->dsss_cck_mode_40mhz == OAL_FALSE) &&
            (pst_ht_hdl->supported_channel_width == OAL_TRUE) &&
            (hmac_user->user_stats_flag.no_40dsss_stats_flag == OAL_TRUE) &&
            (protection->sta_no_40dsss_cck_num != 0)) {
            protection->sta_no_40dsss_cck_num--;
        }
    }

    hmac_user->user_stats_flag.no_ht_stats_flag             = OAL_FALSE;
    hmac_user->user_stats_flag.no_gf_stats_flag             = OAL_FALSE;
    hmac_user->user_stats_flag.only_stats_20m_flag          = OAL_FALSE;
    hmac_user->user_stats_flag.no_40dsss_stats_flag         = OAL_FALSE;
    hmac_user->user_stats_flag.no_lsig_txop_stats_flag      = OAL_FALSE;

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_protection_del_user_stat_ap
 功能描述  : 删除保护模式相关user统计
 输入参数  : hmac_vap  : mac vap结构体指针
             hmac_user : mac user结构体指针
 输出参数  : 无
 返 回 值  : osal_u32
*****************************************************************************/
OAL_STATIC osal_void  hmac_protection_del_user_stat_ap(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user)
{
    osal_u32 ret;
    ret = hmac_protection_del_user_stat_legacy_ap(hmac_vap, hmac_user);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{hmac_protection_del_user_stat_ap::hmac_protection_del_user_stat_legacy_ap failed[%d].}", ret);
    }
    ret = hmac_protection_del_user_stat_ht_ap(hmac_vap, hmac_user);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{hmac_protection_del_user_stat_ap::hmac_protection_del_user_stat_ht_ap failed[%d].}", ret);
    }
}

/*****************************************************************************
 函 数 名  : hmac_protection_del_user_etc
 功能描述  : AP:删除user统计， 并更新保护模式
             STA: 更新为无保护模式
 输入参数  : hmac_vap  : mac vap结构体指针
             hmac_user : mac user结构体指针
 输出参数  : 无
 返 回 值  : osal_u32
*****************************************************************************/
osal_u32 hmac_protection_del_user_etc(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user)
{
    osal_u32 ul_ret = OAL_SUCC;

    if ((hmac_vap == OAL_PTR_NULL) || (hmac_user == OAL_PTR_NULL)) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* AP 更新VAP结构体统计量，更新保护机制 */
    if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_AP) {
        /* 删除保护模式相关user统计 */
        hmac_protection_del_user_stat_ap(hmac_vap, hmac_user);
        /* 删除用户统计保护相关的信息 */
        hmac_user_protection_sync_data(hmac_vap);
    }

    return ul_ret;
}

osal_s32 hmac_protection_sync(hmac_vap_stru *hmac_vap, wlan_prot_mode_enum_uint8 mode)
{
    mac_protection_sync_stru protection = {0};
    frw_msg msg_info = {0};
    osal_s32 ret = OAL_SUCC;

    /* AP模式下关闭ERP功能，防止出现兼容性问题 */
    if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_AP) {
        return OAL_SUCC;
    }

    protection.erp_mode = (mode == WLAN_PROT_ERP) ? OSAL_TRUE : OSAL_FALSE;

    frw_msg_init((osal_u8 *)&protection, sizeof(mac_protection_sync_stru), OSAL_NULL, 0, &msg_info);
    ret = frw_send_msg_to_device(hmac_vap->vap_id, WLAN_MSG_H2D_C_CFG_PROTECTION_SYNC, &msg_info, OSAL_FALSE);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_CFG, "hmac_protection_sync::sync msg ret [%d]", ret);
    }

    return ret;
}

/*****************************************************************************
 函 数 名  : hmac_config_set_prot_mode_etc
 功能描述  : 设置保护模式
 输入参数  : event_hdr: 事件头
             len       : 参数长度
             param    : 参数
 输出参数  : 无
*****************************************************************************/
osal_s32  hmac_config_set_prot_mode_etc(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    osal_s32 l_value;

    l_value = *((osal_s32 *)msg->data);

    if (osal_unlikely(l_value >= WLAN_PROT_BUTT)) {
        oam_warning_log2(0, OAM_SF_CFG, "vap_id[%d] {hmac_config_set_prot_mode_etc::invalid l_value[%d].}",
                         hmac_vap->vap_id, l_value);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    hmac_vap->protection.protection_mode = (osal_u8)l_value;

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_config_get_prot_mode_etc
 功能描述  : 读取保护模式
 输入参数  : event_hdr: 事件头
 输出参数  : pus_len      : 参数长度
             param    : 参数
*****************************************************************************/
osal_s32  hmac_config_get_prot_mode_etc(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    *((osal_s32 *)msg->rsp) = hmac_vap->protection.protection_mode;
    msg->rsp_len = OAL_SIZEOF(osal_s32);

    return OAL_SUCC;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

