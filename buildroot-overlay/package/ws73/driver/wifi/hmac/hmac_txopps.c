/*
 * Copyright (c) CompanyNameMagicTag 2020-2020. All rights reserved.
 * Description: dmac txopps rom source
 * Date: 2020-07-14
 */
#ifdef _PRE_WLAN_FEATURE_TXOPPS
#include "hmac_txopps.h"
#include "oal_types.h"
#include "oam_ext_if.h"
#include "hal_ext_if.h"
#include "mac_resource_ext.h"
#include "mac_device_ext.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_DEV_DMAC_TXOPPS_ROM_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_DEVICE

/*****************************************************************************
 函 数 名  : hmac_txopps_set_machw_en_sta
 功能描述  : sta模式，配置mac txop_ps使能寄存器，包括使能位，condition1和co-
             ndition2
*****************************************************************************/
osal_u32 hmac_txopps_set_machw_en_sta(const hmac_vap_stru *hmac_vap,
    const mac_txopps_machw_param_stru *txopps_machw_param)
{
    hal_to_dmac_device_stru *hal_device = OSAL_NULL;

    if (osal_unlikely(hmac_vap == OSAL_NULL || txopps_machw_param == OSAL_NULL)) {
        oam_error_log2(0, OAM_SF_TXOP, "{hmac_txopps_set_machw_en_sta::param is null,vap=[%p],machw_param=[%p]}.",
            (uintptr_t)hmac_vap, (uintptr_t)txopps_machw_param);
        return OAL_ERR_CODE_PTR_NULL;
    }

    hal_device = hmac_vap->hal_device;
    if (hal_device == OSAL_NULL) {
        oam_error_log1(0, OAM_SF_COEX, "vap_id[%d] {hmac_txopps_set_machw_en_sta::hal_device null}", hmac_vap->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    hal_set_txop_ps_enable(hal_device, txopps_machw_param->machw_txopps_en);
    hal_set_txop_ps_condition1(txopps_machw_param->machw_txopps_condition1);
    hal_set_txop_ps_condition2(txopps_machw_param->machw_txopps_condition2);

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_txopps_set_machw
 功能描述  : 根据vap能力，设置mac txop ps使能寄存器
*****************************************************************************/
osal_u32 hmac_txopps_set_machw(hmac_vap_stru *hmac_vap)
{
    mac_txopps_machw_param_stru txopps_machw_param;

    if (!is_legacy_sta(hmac_vap)) {
        return OAL_SUCC;
    }

    if (hmac_vap_get_txopps(hmac_vap) == OSAL_FALSE) {
        txopps_machw_param.machw_txopps_en = 0;
        txopps_machw_param.machw_txopps_condition1 = 0;
        txopps_machw_param.machw_txopps_condition2 = 0;
    } else {
        txopps_machw_param.machw_txopps_en = 1;
        txopps_machw_param.machw_txopps_condition1 = 1;
        txopps_machw_param.machw_txopps_condition2 = 1;
    }

    if (hmac_txopps_set_machw_en_sta(hmac_vap, &txopps_machw_param) != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_TXOP, "{hmac_txopps_set_machw::hmac_txopps_set_machw_en_sta fail!");
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_txopps_set_machw_partialaid_sta
 功能描述  : sta将partial aid写入到mac寄存器
*****************************************************************************/
osal_s32 hmac_txopps_set_machw_partialaid(hmac_vap_stru *hmac_vap, osal_u32 partial_aid)
{
    if (osal_unlikely(hmac_vap == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_TXOP, "{hmac_txopps_set_machw_partialaid::param is null}.");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 只有STA需要设置TXOPPS */
    if (!is_legacy_sta(hmac_vap)) {
        oam_warning_log2(0, OAM_SF_TXOP, "vap_id[%d] {hmac_txopps_set_machw_partialaid_sta::NOT STA[%d].",
            hmac_vap->vap_id, hmac_vap->vap_mode);
        return OAL_SUCC;
    }

    oam_warning_log2(0, OAM_SF_TXOP, "vap_id[%d] {hmac_txopps_set_machw_partialaid_sta::partial aid[%d]}.",
        hmac_vap->vap_id, partial_aid);

    hal_set_txop_ps_partial_aid(hmac_vap->hal_vap, partial_aid);

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述 : 设置vap的txopps
*****************************************************************************/
osal_u8 hmac_vap_get_txopps(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->cap_flag.txop_ps;
}

/*****************************************************************************
 功能描述 : 设置vap的txopps
*****************************************************************************/
void hmac_vap_set_txopps(hmac_vap_stru *hmac_vap, osal_u8 value)
{
    hmac_vap->cap_flag.txop_ps = value;
}

osal_void mac_vap_update_txopps(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user)
{
    /* 如果用户使能txop ps，则vap使能 */
    if (hmac_user->vht_hdl.vht_txop_ps == OSAL_TRUE && mac_mib_get_txopps(hmac_vap) == OSAL_TRUE) {
        hmac_vap_set_txopps(hmac_vap, OSAL_TRUE);
    }

    /* 只要有一个用户支持txop ps就开启vap能力;对于AP只需要在删除user的时候判断一次是否需要设置为false */
}

/*****************************************************************************
 函 数 名  : hmac_sta_set_txopps_partial_aid
 功能描述  : STA收到关联响应帧以后，用AP分配的AID和BSS的BSSID计算自身的part-
             ial aid，用于TXOP PS
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
osal_void hmac_sta_set_txopps_partial_aid(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user)
{
    osal_u16              temp_aid;
    osal_u8               temp_bssid;
    osal_s32 ret;
    osal_u16 partial_aid;
    /* 此处需要注意:按照协议规定(802.11ac-2013.pdf,9.17a)，ap分配给sta的aid，不可以
       使计算出来的partial aid为0，后续如果ap支持的最大关联用户数目超过512，必须要
       对aid做这个合法性检查!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    */

    if (hmac_user->vht_hdl.vht_capable != OAL_TRUE) {
        oam_warning_log0(0, OAM_SF_TXOP, "{hmac_sta_set_txopps_partial_aid:: VHT unsupport}.");
        return;
    }

    /* 计算partial aid */
    if (hmac_vap_get_txopps(hmac_vap) != OAL_TRUE) {
        partial_aid = 0;
    } else {
        temp_aid   = hmac_vap->sta_aid & 0x1FF;
        /* 5:取bssid第5位 4:右移4位 */
        temp_bssid = (hmac_vap->bssid[5] & 0x0F) ^ ((hmac_vap->bssid[5] & 0xF0) >> 4);
        /* 5:计算partial_aid 9：左移9位 */
        partial_aid = (temp_aid + (temp_bssid << 5)) & ((1 << 9) - 1);
    }

    ret = hmac_txopps_set_machw_partialaid(hmac_vap, (osal_u32)partial_aid);
    if (OAL_UNLIKELY(ret != OAL_SUCC)) {
        oam_warning_log2(0, OAM_SF_TXOP,
            "vap_id[%d] {hmac_sta_set_txopps_partial_aid::hmac_txopps_set_machw_partialaid failed[%d].}",
            hmac_vap->vap_id, ret);
    }
}
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif