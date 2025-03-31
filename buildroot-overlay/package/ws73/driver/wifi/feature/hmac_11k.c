/*
 * Copyright (c) CompanyNameMagicTag. 2017-2024. All rights reserved.
 * 文 件 名   : hmac_11k.c
 * 生成日期   : 2017年3月15日
 * 功能描述   : 11K
 */

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "hmac_11k.h"
#include "oal_ext_if.h"
#include "oal_netbuf_ext.h"
#include "oal_netbuf_data.h"
#include "oal_mem_hcm.h"
#include "mac_frame.h"
#include "mac_resource_ext.h"
#include "hmac_resource.h"
#include "mac_ie.h"
#include "mac_vap_ext.h"
#include "mac_user_ext.h"
#include "frw_ext_if.h"
#include "wlan_types_common.h"
#include "hmac_mgmt_bss_comm.h"
#include "hmac_device.h"
#include "mac_device_ext.h"
#include "hmac_scan.h"
#include "hmac_config.h"
#include "hmac_feature_interface.h"
#include "frw_util_notifier.h"
#include "hmac_feature_dft.h"
#include "hmac_mbo.h"
#include "hmac_feature_interface.h"
#include "hmac_hook.h"
#include "hal_common_ops.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_HMAC_11K_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
hmac_11k_vap_info_stru *g_11k_vap_info[WLAN_VAP_MAX_NUM_PER_DEVICE_LIMIT] = {
    OSAL_NULL, OSAL_NULL, OSAL_NULL, OSAL_NULL
};

hmac_11k_user_info_stru *g_11k_user_info[WLAN_USER_MAX_USER_LIMIT];

osal_u8              guc_dialog_token = 0;
osal_u8              guc_meas_token   = 0;

OAL_STATIC osal_u32 hmac_wait_meas_rsp_timeout(osal_void *p_arg);
OAL_STATIC osal_u32 hmac_rrm_bcn_scan_do(hmac_vap_stru *hmac_vap, mac_bcn_req_stru *bcn_req,
    mac_scan_req_stru *scan_req);
OAL_STATIC osal_u32 hmac_rrm_fill_basic_rm_rpt_action(hmac_vap_stru *hmac_vap);
OAL_STATIC osal_u32 hmac_rrm_send_rm_rpt_action(hmac_vap_stru *hmac_vap);
OAL_STATIC osal_void hmac_rrm_parse_beacon_req(hmac_vap_stru *hmac_vap, mac_meas_req_ie_stru  *meas_req_ie);
OAL_STATIC osal_void hmac_rrm_encap_meas_rpt_reject(hmac_vap_stru *hmac_vap, mac_meas_rpt_mode_stru *rptmode);
OAL_STATIC osal_u32 hmac_rrm_bcn_rpt_filter(hmac_vap_stru *hmac_vap, mac_bss_dscr_stru *bss_dscr,
    const mac_scan_req_stru *scan_params);
OAL_STATIC osal_u32 hmac_rrm_encap_meas_rpt_refuse_new_req(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    mac_action_rm_req_stru *rm_req);
OAL_STATIC osal_u32 hmac_rrm_check_user_cap(mac_rrm_enabled_cap_ie_stru *rrm_enabled_cap,
    mac_rrm_req_cfg_stru *req_cfg);
OAL_STATIC osal_void hmac_rrm_beacon_measure_free_all_mem(mac_vap_rrm_info_stru *rrm_info);
OAL_STATIC osal_void hmac_rrm_free_rpt_list(hmac_user_stru *hmac_user, mac_rrm_type_enum reqtype);

/*****************************************************************************
  3 函数实现
*****************************************************************************/
OAL_STATIC hmac_11k_vap_info_stru *hmac_11k_get_vap_info(const hmac_vap_stru *hmac_vap)
{
    if (hmac_vap_id_param_check(hmac_vap->vap_id) != OSAL_TRUE) {
        return OSAL_NULL;
    }

    return g_11k_vap_info[hmac_vap->vap_id];
}

OAL_STATIC mac_vap_rrm_info_stru *hmac_11k_get_vap_rrm_info(const hmac_vap_stru *hmac_vap)
{
    if (hmac_11k_get_vap_info(hmac_vap) == OSAL_NULL) {
        return OSAL_NULL;
    }

    return g_11k_vap_info[hmac_vap->vap_id]->rrm_info;
}

OAL_STATIC hmac_11k_user_info_stru *hmac_11k_get_user_info(osal_u8 user_idx)
{
    if (osal_unlikely(user_idx >= WLAN_USER_MAX_USER_LIMIT)) {
        return OSAL_NULL;
    }

    return g_11k_user_info[user_idx];
}

/*
 * 功能描述  : 删除rrm 扫描结果链表的一个节点项
 */
OAL_STATIC osal_void hmac_rrm_beacon_measure_free_one_rpt_bcn_item(mac_meas_rpt_bcn_stru *meas_rpt_bcn)
{
    if (meas_rpt_bcn == NULL) {
        return;
    }

    /* free mac_meas_rpt_bcn_item_stru内存 */
    if (meas_rpt_bcn->meas_rpt_bcn_item != NULL) {
        oal_mem_free(meas_rpt_bcn->meas_rpt_bcn_item, OSAL_TRUE);
        meas_rpt_bcn->meas_rpt_bcn_item = NULL;
    }

    /* free prt_detail_data */
    if (meas_rpt_bcn->rpt_detail_data != NULL) {
        oal_mem_free(meas_rpt_bcn->rpt_detail_data, OSAL_TRUE);
        meas_rpt_bcn->rpt_detail_data = NULL;
        meas_rpt_bcn->rpt_detail_act_len = 0;
    }

    oal_mem_free(meas_rpt_bcn, OSAL_TRUE);
}

/*
 * 功能描述  : 删除rrm ssdi和reqinfo内存
 */
OAL_STATIC osal_void hmac_rrm_free_ssid_and_reqinfo_ieid_item(mac_vap_rrm_info_stru *rrm_info)
{
    if (rrm_info->bcn_req_info.ssid != NULL) {
        oal_mem_free(rrm_info->bcn_req_info.ssid, OSAL_TRUE);
        rrm_info->bcn_req_info.ssid = NULL;
        rrm_info->bcn_req_info.ssid_len = 0;
    }

    if (rrm_info->bcn_req_info.reqinfo_ieid != NULL) {
        oal_mem_free(rrm_info->bcn_req_info.reqinfo_ieid, OSAL_TRUE);
        rrm_info->bcn_req_info.reqinfo_ieid = NULL;
        rrm_info->bcn_req_info.req_ie_num = 0;
    }
}

/*
 * 功能描述  : 一次Beacon req失败，释放之前申请内存
 */
OAL_STATIC osal_void hmac_rrm_beacon_measure_free_all_mem(mac_vap_rrm_info_stru *rrm_info)
{
    struct osal_list_head *meas_rpt_node = OSAL_NULL;
    mac_meas_rpt_bcn_stru *meas_rpt_bcn = OSAL_NULL;

    /* 查找链表删除各个表项内容 */
    while (osal_list_empty(&(rrm_info->meas_rpt_list)) == 0) {
        meas_rpt_node = rrm_info->meas_rpt_list.next;
        meas_rpt_bcn = osal_list_entry(meas_rpt_node, mac_meas_rpt_bcn_stru, dlist_head);

        /* 删除链表 */
        osal_dlist_delete_entry(meas_rpt_node);
        hmac_rrm_beacon_measure_free_one_rpt_bcn_item(meas_rpt_bcn);
    };
}

/*****************************************************************************
 重置数据结构rrm_info
*****************************************************************************/
OAL_STATIC osal_void reset_rrm_info(mac_vap_rrm_info_stru *rrm_info)
{
    hmac_rrm_beacon_measure_free_all_mem(rrm_info);
    hmac_rrm_free_ssid_and_reqinfo_ieid_item(rrm_info);
    (osal_void)memset_s(rrm_info, sizeof(mac_vap_rrm_info_stru), 0, sizeof(mac_vap_rrm_info_stru));
}

OAL_STATIC osal_bool hmac_11k_vap_info_init(hmac_vap_stru *hmac_vap)
{
    mac_device_voe_custom_stru *mac_voe_custom_param = mac_get_pst_mac_voe_custom_param();
    hmac_11k_vap_info_stru *hmac_11k_vap_info = hmac_11k_get_vap_info(hmac_vap);
    oal_bool_enum_uint8 en_11k = mac_voe_custom_param->en_11k;

    hmac_11k_vap_info->enable_11k = en_11k;
    hmac_11k_vap_info->bcn_table_switch = en_11k;
    mac_mib_set_dot11_radio_measurement_activated(hmac_vap, en_11k);
    mac_mib_set_dot11RMBeaconTableMeasurementActivated(hmac_vap, en_11k);

    if (en_11k == OAL_TRUE) {
        hmac_11k_vap_info->rrm_info = oal_mem_alloc(OAL_MEM_POOL_ID_LOCAL, OAL_SIZEOF(mac_vap_rrm_info_stru), OAL_TRUE);
        if (hmac_11k_vap_info->rrm_info == OAL_PTR_NULL) {
            oam_error_log1(0, OAM_SF_ANY, "vap_id[%d] {hmac_11k_vap_info_init::rrm_info null.}", hmac_vap->vap_id);
            return OAL_FALSE;
        }
        memset_s(hmac_11k_vap_info->rrm_info, OAL_SIZEOF(mac_vap_rrm_info_stru), 0, OAL_SIZEOF(mac_vap_rrm_info_stru));
        OSAL_INIT_LIST_HEAD(&((hmac_11k_vap_info->rrm_info)->meas_rpt_list));
    }

    return OAL_TRUE;
}

/*****************************************************************************
函 数 名  : hmac_11k_init_vap
功能描述  : 创建vap时，初始化11k vap结构体；
输入参数  : hmac_vap
输出参数  : 无
返 回 值  : 无
 *****************************************************************************/
OAL_STATIC osal_bool hmac_11k_add_vap(osal_void *notify_data)
{
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)notify_data;
    osal_void *mem_ptr = OSAL_NULL;
    osal_u8 vap_id;
    oal_bool_enum_uint8 ret;

    if (hmac_vap_mode_param_check(hmac_vap) != OSAL_TRUE) {
        return OSAL_FALSE;
    }

    if (!is_legacy_sta(hmac_vap)) {
        return OSAL_TRUE;
    }

    vap_id = hmac_vap->vap_id;
    if (g_11k_vap_info[vap_id] != OSAL_NULL) {
        oam_warning_log1(0, OAM_SF_ANY, "vap_id[%d] hmac_11k_add_vap mem already malloc!", vap_id);
        return OSAL_TRUE;
    }

    mem_ptr = oal_mem_alloc(OAL_MEM_POOL_ID_LOCAL, sizeof(hmac_11k_vap_info_stru), OAL_TRUE);
    if (mem_ptr == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_ANY, "hmac_11k_add_vap mem alloc fail");
        return OSAL_FALSE;
    }

    (osal_void)memset_s(mem_ptr, sizeof(hmac_11k_vap_info_stru), 0, sizeof(hmac_11k_vap_info_stru));
    /* 注册特性数据结构 */
    g_11k_vap_info[vap_id] = (hmac_11k_vap_info_stru *)mem_ptr;

    ret = hmac_11k_vap_info_init(hmac_vap);
    if (ret != OAL_TRUE) {
        oal_mem_free(mem_ptr, OAL_TRUE);
        g_11k_vap_info[vap_id] = OSAL_NULL;
        return ret;
    }

    return OAL_TRUE;
}

/*****************************************************************************
函 数 名  : hmac_11k_down_vap
功能描述  :down vap时，释放11k vap的rrm info结构体；
输入参数  : notify_data
输出参数  : 无
返 回 值  : 无
 *****************************************************************************/
OAL_STATIC osal_bool hmac_11k_down_vap(osal_void *notify_data)
{
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)notify_data;
    hmac_11k_vap_info_stru *hmac_11k_vap_info;

    if (hmac_vap_mode_param_check(hmac_vap) != OSAL_TRUE) {
        return OSAL_FALSE;
    }

    if (!is_legacy_sta(hmac_vap)) {
        return OSAL_TRUE;
    }

    hmac_11k_vap_info = hmac_11k_get_vap_info(hmac_vap);
    if (hmac_11k_vap_info == OSAL_NULL) {
        return OSAL_FALSE;
    }

    if (hmac_11k_vap_info->rrm_info != OSAL_NULL) {
        frw_destroy_timer_entry(&(hmac_11k_vap_info->rrm_info->meas_status_timer));
        reset_rrm_info(hmac_11k_vap_info->rrm_info);
        oal_mem_free(hmac_11k_vap_info->rrm_info, OAL_TRUE);
        hmac_11k_vap_info->rrm_info = OSAL_NULL;
    }

    return OAL_TRUE;
}

OAL_STATIC osal_bool hmac_11k_del_vap(osal_void *notify_data)
{
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)notify_data;
    osal_u8 vap_id_11k;

    if (hmac_vap_mode_param_check(hmac_vap) != OSAL_TRUE) {
        return OSAL_FALSE;
    }

    if (!is_legacy_sta(hmac_vap)) {
        return OSAL_TRUE;
    }

    vap_id_11k = hmac_vap->vap_id;
    if (g_11k_vap_info[vap_id_11k] != OSAL_NULL) {
        hmac_11k_down_vap(notify_data);
        oal_mem_free(g_11k_vap_info[vap_id_11k], OAL_TRUE);
        g_11k_vap_info[vap_id_11k] = OSAL_NULL;
    }

    return OAL_TRUE;
}

OAL_STATIC osal_bool hmac_11k_add_user(osal_void *notify_data)
{
    hmac_user_stru *hmac_user = (hmac_user_stru *)notify_data;
    osal_u8 *mem_ptr = OSAL_NULL;
    osal_u8 user_idx;

    if (osal_unlikely(hmac_user == OSAL_NULL || hmac_user->assoc_id >= WLAN_USER_MAX_USER_LIMIT)) {
        return OSAL_FALSE;
    }

    user_idx = (osal_u8)hmac_user->assoc_id;
    if (g_11k_user_info[user_idx] != OSAL_NULL) {
        oam_warning_log1(0, OAM_SF_CSA, "user_idx[%d] hmac_11k_add_user mem already malloc!", user_idx);
        return OSAL_TRUE;
    }

    mem_ptr = oal_mem_alloc(OAL_MEM_POOL_ID_LOCAL, sizeof(hmac_11k_user_info_stru), OAL_TRUE);
    if (mem_ptr == OSAL_NULL) {
        oam_error_log1(0, OAM_SF_ANY, "user_idx[%d] hmac_11k_add_user mem alloc fail.", user_idx);
        return OSAL_FALSE;
    }

    (osal_void)memset_s(mem_ptr, sizeof(hmac_11k_user_info_stru), 0, sizeof(hmac_11k_user_info_stru));
    /* 注册特性数据结构 */
    g_11k_user_info[user_idx] = (hmac_11k_user_info_stru *)mem_ptr;

    return OSAL_TRUE;
}

/*****************************************************************************
函 数 名  : hmac_11k_init_user
功能描述  : 创建user时，初始化11k user结构体；
输入参数  : hmac_user
输出参数  : 无
返 回 值  : 无
 *****************************************************************************/
OAL_STATIC osal_void hmac_11k_init_rrm_info(hmac_user_stru *hmac_user)
{
    hmac_11k_user_info_stru *hmac_11k_user_info = hmac_11k_get_user_info((osal_u8)hmac_user->assoc_id);
    mac_user_rrm_info_stru *user_rrm_info = OSAL_NULL;

    if (hmac_11k_user_info == OSAL_NULL) {
        return;
    }

    /* 关联时申请用户结构体空间 */
    if (hmac_11k_user_info->user_rrm_info == OAL_PTR_NULL) {
        user_rrm_info = oal_mem_alloc(OAL_MEM_POOL_ID_LOCAL, OAL_SIZEOF(mac_user_rrm_info_stru), OAL_TRUE);
        if (user_rrm_info == OAL_PTR_NULL) {
            oam_error_log1(0, OAM_SF_ANY, "vap_id[%d] {hmac_11k_init_rrm_info::user_rrm_info null.}",
                hmac_user->vap_id);
            return;
        }

        memset_s(user_rrm_info, OAL_SIZEOF(mac_user_rrm_info_stru), 0, OAL_SIZEOF(mac_user_rrm_info_stru));
        OSAL_INIT_LIST_HEAD(&(user_rrm_info->meas_rpt_list));
        hmac_11k_user_info->user_rrm_info = user_rrm_info;
    }

    return;
}

/*****************************************************************************
函 数 名  : hmac_11k_init_user
功能描述  : 销毁user时，释放11k user结构体；
输入参数  : hmac_user
输出参数  : 无
返 回 值  : 无
 *****************************************************************************/
OAL_STATIC osal_bool hmac_11k_del_user(osal_void *notify_data)
{
    hmac_user_stru *hmac_user = (hmac_user_stru *)notify_data;
    hmac_11k_user_info_stru *hmac_11k_user_info = OSAL_NULL;
    mac_user_rrm_info_stru *user_rrm_info = OSAL_NULL;
    osal_u8 user_idx;

    if (osal_unlikely(hmac_user == OSAL_NULL || hmac_user->assoc_id >= WLAN_USER_MAX_USER_LIMIT)) {
        return OSAL_FALSE;
    }

    user_idx = (osal_u8)hmac_user->assoc_id;
    hmac_11k_user_info = hmac_11k_get_user_info(user_idx);
    if (hmac_11k_user_info == OSAL_NULL) {
        oam_warning_log1(0, OAM_SF_ANY, "user_idx[%d] hmac_11k_del_user mem already free!", user_idx);
        return OSAL_TRUE;
    }

    user_rrm_info = hmac_11k_user_info->user_rrm_info;
    if (user_rrm_info != OAL_PTR_NULL) {
        if (user_rrm_info->timer.is_registerd == OAL_TRUE) {
            frw_destroy_timer_entry(&user_rrm_info->timer);
        }

        /* 释放链表和恢复状态 */
        hmac_rrm_free_rpt_list(hmac_user, user_rrm_info->reqtype);

        oal_mem_free(user_rrm_info, OAL_TRUE);
        hmac_11k_user_info->user_rrm_info = OAL_PTR_NULL;
    }

    oal_mem_free(hmac_11k_user_info, OAL_TRUE);
    g_11k_user_info[user_idx] = OAL_PTR_NULL;

    return OSAL_TRUE;
}

/***************************************************************************
函 数 名  : hmac_rrm_proc_action_ie_head
功能描述  : 处理rrm帧的ie头部信息
输入参数  :  hmac_vap、hmac_user、framebody_len、rm_req
输出参数  :  rrm_info
****************************************************************************/
OAL_STATIC osal_u32 hmac_rrm_proc_action_ie_head(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    osal_u16 framebody_len, mac_action_rm_req_stru *rm_req, mac_vap_rrm_info_stru *rrm_info)
{
    mac_meas_req_ie_stru *meas_req_ie = OSAL_NULL;
    mac_meas_rpt_mode_stru rptmode;

    memset_s(&rptmode, OAL_SIZEOF(mac_meas_rpt_mode_stru), 0, OAL_SIZEOF(mac_meas_rpt_mode_stru));

    /* 判断VAP是否正在进行测量 */
    if (rrm_info->is_measuring == OAL_TRUE) {
        oam_warning_log0(0, OAM_SF_RRM, "{hmac_rrm_proc_rm_request::vap is handling one request now.}");
        hmac_rrm_encap_meas_rpt_refuse_new_req(hmac_vap, hmac_user, rm_req);
        return OAL_FAIL;
    }

    /* 保存req */
    rrm_info->action_code  = rm_req->action_code;
    rrm_info->dialog_token = rm_req->dialog_token;
    rrm_info->req_user_id  = hmac_user->assoc_id;

    /* 是否有Meas Req */
    if (framebody_len <= MAC_RADIO_MEAS_ACTION_REQ_FIX_LEN) {
        /* 如果没有MR IE，也回一个不带Meas Rpt的Radio Meas Rpt */
        /* 申请管理帧内存并填充头部信息 */
        if (hmac_rrm_fill_basic_rm_rpt_action(hmac_vap) != OAL_SUCC) {
            return OAL_FAIL;
        }
        (osal_void)hmac_rrm_send_rm_rpt_action(hmac_vap);
        return OAL_FAIL;
    }

    meas_req_ie = (mac_meas_req_ie_stru *)&(rm_req->req_ies[0]);
    rrm_info->meas_req_ie = meas_req_ie;

    /* 重复测试次数暂不处理，如果对端要求重复测试，则回复incapable bit */
    oam_warning_log4(0, OAM_SF_RRM,
        "{hmac_rrm_proc_rm_request::framebody::Category[%d],Action[%d],Dialog Token[%d],Number of Repetitions[%d].}",
        rm_req->category, rm_req->action_code, rm_req->dialog_token, rm_req->num_rpt);

    if (rm_req->num_rpt != 0) {
        rptmode.incapable = 1;
        hmac_rrm_encap_meas_rpt_reject(hmac_vap, &rptmode);
        oam_warning_log2(0, OAM_SF_RRM,
            "{vap_id[%d] hmac_rrm_proc_rm_request::RepeatedMeasurements not support, num_rpt[%u].}",
            hmac_vap->vap_id, rm_req->num_rpt);
        return OAL_FAIL;
    }

    /**************************************************************************/
    /*                    Measurement Request IE                              */
    /* ---------------------------------------------------------------------- */
    /* |Element ID |Length |Meas Token| Meas Req Mode|Meas Type  | Meas Req | */
    /* ---------------------------------------------------------------------- */
    /* |1          |1      | 1        | 1            |1          |var       | */
    /* ---------------------------------------------------------------------- */
    /*                                                                        */
    /**************************************************************************/
    oam_warning_log4(0, OAM_SF_RRM,
        "hmac_rrm_proc_rm_request::framebody::Element ID[%d],Length[%d],Meas Token[%d],Meas Req Mode[%d].",
        meas_req_ie->eid, meas_req_ie->len, meas_req_ie->token, *((osal_u8*)(&(meas_req_ie->reqmode))));
    oam_warning_log2(0, OAM_SF_RRM, "vap_id[%d] hmac_rrm_proc_rm_request::framebody::Meas Type[%d].",
        hmac_vap->vap_id, meas_req_ie->reqtype);

    return OAL_SUCC;
}

/***************************************************************************
函 数 名  : hmac_rrm_proc_action_ie_body
功能描述  : 处理rrm帧的ie帧体信息
输入参数  :  hmac_vap、hmac_user、meas_req_ie、rm_req
输出参数  :  rrm_info
****************************************************************************/
OAL_STATIC osal_u32 hmac_rrm_proc_action_ie_body(hmac_vap_stru *hmac_vap,
    hmac_user_stru *hmac_user, mac_meas_req_ie_stru *meas_req_ie, mac_vap_rrm_info_stru *rrm_info,
    const mac_action_rm_req_stru *rm_req)
{
    mac_meas_rpt_mode_stru rptmode;
    hmac_11k_user_info_stru *hmac_11k_user_info = hmac_11k_get_user_info((osal_u8)hmac_user->assoc_id);

    if (hmac_11k_user_info == OSAL_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    memset_s(&rptmode, OAL_SIZEOF(mac_meas_rpt_mode_stru), 0, OAL_SIZEOF(mac_meas_rpt_mode_stru));

    if (meas_req_ie->eid == MAC_EID_MEASREQ) {
        if (meas_req_ie->reqmode.enable == 1) {
            /* Req中拒绝某种类型的测量,一旦对端拒绝，在当前关联状态无法恢复测量能力位，需要重新关联 */
            if (meas_req_ie->reqmode.request == 0 && meas_req_ie->reqtype == RM_RADIO_MEAS_BCN) {
                oam_warning_log1(0, OAM_SF_RRM,
                    "{hmac_rrm_proc_action_ie_body::update user rrm capability,reqtype[%d] not support!}",
                    meas_req_ie->reqtype);
                hmac_11k_user_info->rrm_enabled_cap.bcn_active_cap  = 0;
                hmac_11k_user_info->rrm_enabled_cap.bcn_passive_cap = 0;
                hmac_11k_user_info->rrm_enabled_cap.bcn_table_cap   = 0;
                hmac_11k_user_info->rrm_enabled_cap.bcn_meas_rpt_cond_cap = 0;
            }

            /* Req中不允许发送对应的report */
            if (meas_req_ie->reqmode.rpt == 0) {
                oam_warning_log0(0, OAM_SF_RRM, "{hmac_rrm_proc_action_ie_body::user not expect rpt!}");
                return OAL_FAIL;
            }
        }

        /* 并行测试暂不处理，如果对端要求则回复incapable bit */
        if (meas_req_ie->reqmode.parallel != 0) {
            rptmode.incapable = 1;
            hmac_rrm_encap_meas_rpt_reject(hmac_vap, &rptmode);
            oam_warning_log0(0, OAM_SF_RRM, "hmac_rrm_proc_action_ie_body::ParallelMeasurement not support.");
            return OAL_FAIL;
        }

        /* 处理Beacon req */
        if (meas_req_ie->reqtype == RM_RADIO_MEAS_BCN) {
            rrm_info->bcn_req_info.meas_token = meas_req_ie->token;
            rrm_info->bcn_req_info.meas_type  = meas_req_ie->reqtype;
            rrm_info->bcn_req_info.repetition = OAL_NTOH_16(rm_req->num_rpt);
            rrm_info->bcn_req_info.dialog_token = rm_req->dialog_token;
            rrm_info->bcn_req_info.rpt_frag_type = MAC_BCN_RPT_NO_FRAG;
            hmac_rrm_parse_beacon_req(hmac_vap, meas_req_ie);
        } else {
            rptmode.incapable = 1;
            hmac_rrm_encap_meas_rpt_reject(hmac_vap, &rptmode);
            oam_warning_log1(0, OAM_SF_RRM, "{Error Request, Expect Measurement Request, but got reqtype[%d]!}",
                meas_req_ie->reqtype);
            return OAL_FAIL;
        }
    } else { /* MR IE错误，不回，报错 */
        oam_warning_log1(0, OAM_SF_RRM, "{Error, Expect Measurement Request, but got EID[%d]!}", meas_req_ie->eid);
        return OAL_FAIL;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_rrm_proc_rm_request
 功能描述  : 解析Radio Measurement Request，并处理Beacon Request
 输入参数  : hmac_vap       : dmac vap结构体指针
             pst_netbuf         : RM Request的netbuf
 输出参数  : 无
*****************************************************************************/
OAL_STATIC osal_u32 hmac_rrm_proc_rm_request(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    oal_netbuf_stru *netbuf)
{
    mac_action_rm_req_stru          *rm_req              = OSAL_NULL;
    mac_meas_req_ie_stru            *meas_req_ie         = OSAL_NULL;
    dmac_rx_ctl_stru                *rx_ctrl             = OSAL_NULL;
    osal_u16                       framebody_len;
    mac_vap_rrm_info_stru           *rrm_info            = OSAL_NULL;
    osal_u16                       framebody_len_tmp;
    osal_u16                       measurement_ie_len;
    osal_u8                         *data                = OSAL_NULL;
    osal_u32                         ret;

    rrm_info = hmac_11k_get_vap_rrm_info(hmac_vap);
    rx_ctrl = (dmac_rx_ctl_stru *)oal_netbuf_cb(netbuf);
    if (rrm_info == OAL_PTR_NULL || rx_ctrl == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_RRM, "{hmac_rrm_proc_rm_request:: parma is NULL! rrm_info, pst_rx_ctrl}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    framebody_len = mac_get_rx_cb_payload_len(&(rx_ctrl->rx_info));
    data = mac_netbuf_get_payload(netbuf);
    rm_req = (mac_action_rm_req_stru *)data;

    ret = hmac_rrm_proc_action_ie_head(hmac_vap, hmac_user, framebody_len, rm_req, rrm_info);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_RRM, "{hmac_rrm_proc_rm_request::hmac_rrm_pre_proc_request fail! ret[%d]}", ret);
        return OAL_FAIL;
    }
    /**************************************************************************/
    /*                    Radio Measurement Request Frame - Frame Body        */
    /* ---------------------------------------------------------------------- */
    /* |Category |Action |Dialog Token| Number of Repetitions|Meas Req Eles | */
    /* ---------------------------------------------------------------------- */
    /* |1        |1      | 1          | 2                    |var             */
    /* ---------------------------------------------------------------------- */
    /*                                                                        */
    /**************************************************************************/

    /* 可能有多个Measurement Req IEs */
    framebody_len_tmp = framebody_len - MAC_MEASUREMENT_REQUEST_IE_OFFSET;
    while (framebody_len_tmp > MAC_IE_HDR_LEN) {
        meas_req_ie = (mac_meas_req_ie_stru *)&(data[framebody_len - framebody_len_tmp]);
        rrm_info->meas_req_ie = meas_req_ie;
        measurement_ie_len = meas_req_ie->len;

        if ((framebody_len_tmp >= measurement_ie_len + MAC_IE_HDR_LEN) &&
            (measurement_ie_len >= OAL_OFFSET_OF(mac_meas_req_ie_stru, meas_req) - MAC_IE_HDR_LEN)) {
            framebody_len_tmp = framebody_len_tmp - measurement_ie_len - MAC_IE_HDR_LEN;
        } else {
            oam_warning_log2(0, OAM_SF_RRM, "{invalid Measurement_ie_len= [%d] framebody_len_tmp=[%d]!}",
                measurement_ie_len, framebody_len_tmp);
            break;
        }

        ret = hmac_rrm_proc_action_ie_body(hmac_vap, hmac_user, meas_req_ie, rrm_info, rm_req);
        if (ret != OAL_SUCC) {
            oam_warning_log1(0, OAM_SF_RRM, "{hmac_rrm_proc_rm_request::hmac_rrm_handle_meas_req fail! ret[%d]}", ret);
            return OAL_FAIL;
        }
    }
    return OAL_SUCC;
}

OAL_STATIC osal_void hmac_config_encap_mgmt_frame(osal_u8** buffer, oal_netbuf_stru *mgmt_buf,
    hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user)
{
    osal_u8 *tmpbuf = OAL_PTR_NULL;

    tmpbuf = (osal_u8 *)oal_netbuf_header(mgmt_buf);

    /*************************************************************************/
    /*                        Management Frame Format                        */
    /* --------------------------------------------------------------------  */
    /* |Frame Control|Duration|DA|SA|BSSID|Sequence Control|Frame Body|FCS|  */
    /* --------------------------------------------------------------------  */
    /* | 2           |2       |6 |6 |6    |2               |0 - 2312  |4  |  */
    /* --------------------------------------------------------------------  */
    /*                                                                       */
    /*************************************************************************/

    /* All the fields of the Frame Control Field are set to zero. Only the   */
    /* Type/Subtype field is set.                                            */
    mac_hdr_set_frame_control(tmpbuf, WLAN_PROTOCOL_VERSION | WLAN_FC0_TYPE_MGT | WLAN_FC0_SUBTYPE_ACTION);

    /* duration */
    tmpbuf[2] = 0;  /* 2 索引 */
    tmpbuf[3] = 0;  /* 3 索引 */

    /* DA is address of STA requesting association */
    oal_set_mac_addr(tmpbuf + WLAN_HDR_ADDR1_OFFSET, hmac_user->user_mac_addr);

    /* SA */
    oal_set_mac_addr(tmpbuf + WLAN_HDR_ADDR2_OFFSET, mac_mib_get_station_id(hmac_vap));
    oal_set_mac_addr(tmpbuf + WLAN_HDR_ADDR3_OFFSET, hmac_vap->bssid);

    /* seq control */
    tmpbuf[22] = 0;  /* 22 索引 */
    tmpbuf[23] = 0;  /* 23 索引 */

    /**************************************************************************/
    /*                    Radio Measurement Request Frame - Frame Body        */
    /* ---------------------------------------------------------------------- */
    /* |Category |Action |Dialog Token| Number of Repetitions|Meas Req Eles | */
    /* ---------------------------------------------------------------------- */
    /* |1        |1      | 1          | 2                    |var             */
    /* ---------------------------------------------------------------------- */
    /*                                                                        */
    /**************************************************************************/
    tmpbuf += MAC_80211_FRAME_LEN;
    *buffer = tmpbuf;
    return;
}

OAL_STATIC osal_u32 hmac_config_encap_radio_meas_req(mac_rrm_req_cfg_stru *req_cfg, osal_u8 *buffer,
    hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user)
{
    osal_u8 *req_mode = OAL_PTR_NULL;
    mac_meas_req_ie_stru *meas_req_ie = OAL_PTR_NULL;
    mac_action_rm_req_stru *rm_req = OAL_PTR_NULL;
    osal_u8 *reporting_detail = OAL_PTR_NULL;
    mac_bcn_req_stru *bcn_req = OAL_PTR_NULL;

    unref_param(hmac_vap);
    unref_param(hmac_user);

    rm_req                  = (mac_action_rm_req_stru *)buffer;
    rm_req->category     = MAC_ACTION_CATEGORY_RADIO_MEASURMENT;
    rm_req->action_code  = MAC_RM_ACTION_RADIO_MEASUREMENT_REQUEST;
    rm_req->dialog_token = guc_dialog_token;
    rm_req->num_rpt      = req_cfg->rpt_num;
    meas_req_ie             = (mac_meas_req_ie_stru *)rm_req->req_ies;

    /**************************************************************************/
    /*                    Measurement Request IE                              */
    /* ---------------------------------------------------------------------- */
    /* |Element ID |Length |Meas Token| Meas Req Mode|Meas Type  | Meas Req | */
    /* ---------------------------------------------------------------------- */
    /* |1          |1      | 1        | 1            |1          |var       | */
    /* ---------------------------------------------------------------------- */
    /*                                                                        */
    /**************************************************************************/
    meas_req_ie->eid     = MAC_EID_MEASREQ;
    meas_req_ie->token   = guc_meas_token;
    req_mode = (osal_u8 *)(&(meas_req_ie->reqmode));
    *req_mode = req_cfg->req_mode;

    if (req_cfg->reqtype == MAC_RRM_TYPE_BCN) {
        meas_req_ie->len     = 16; /* 16 数据长度 */
        meas_req_ie->reqtype = RM_RADIO_MEAS_BCN;
        if (memcpy_s(meas_req_ie->meas_req, OAL_SIZEOF(mac_bcn_req_stru),
            (mac_bcn_req_stru *)(req_cfg->p_arg), OAL_SIZEOF(mac_bcn_req_stru)) != EOK) {
            oam_warning_log0(0, OAM_SF_CFG, "{hmac_config_encap_radio_meas_req data::memcpy err.}");
        }
        /* optional subelement */
        bcn_req = (mac_bcn_req_stru *)meas_req_ie->meas_req;
        reporting_detail = bcn_req->subelm;
        /* 2,Reporting Detail */
        *reporting_detail       = MAC_RRM_BCN_EID_REPORTING_DATAIL;
        *(reporting_detail + 1) = MAC_RRM_BCN_REPORTING_DETAIL_LEN;
        *(reporting_detail + 2) = MAC_BCN_REPORTING_DETAIL_FIXED_FILELD_AND_ANY_ELEM; /* 2 偏移 */
        meas_req_ie->len += MAC_IE_HDR_LEN + MAC_RRM_BCN_REPORTING_DETAIL_LEN;
    } else if (req_cfg->reqtype == MAC_RRM_TYPE_CHANNEL_LOAD) {
        meas_req_ie->len     = 9;     /* 9 数据长度 */
        meas_req_ie->reqtype = RM_RADIO_MEAS_CHANNEL_LOAD;
        if (memcpy_s(meas_req_ie->meas_req, OAL_SIZEOF(mac_chn_load_req_stru),
            (mac_chn_load_req_stru *)(req_cfg->p_arg), OAL_SIZEOF(mac_chn_load_req_stru)) != EOK) {
            oam_warning_log0(0, OAM_SF_CFG, "{hmac_config_encap_radio_meas_req data:: p2 memcpy err.}");
        }
    }

    return MAC_80211_FRAME_LEN + 5 + MAC_IE_HDR_LEN + meas_req_ie->len;  /* 5 长度 */
}

OAL_STATIC osal_void hmac_config_encap_neighbor_report_req(osal_u16 *frame_len, osal_u8 *buffer)
{
    mac_action_neighbor_req_stru *neighbor_req = OAL_PTR_NULL;
    /* Neighbor Report Request */
    /*************************************************************************/
    /*                    Neighbor Report Request Frame - Frame Body         */
    /* --------------------------------------------------------------------- */
    /* |Category |Action |Dialog Token| Optional subs                      | */
    /* --------------------------------------------------------------------- */
    /* |1        |1      | 1          | var                                  */
    /* --------------------------------------------------------------------- */
    /*                                                                       */
    /*************************************************************************/
    neighbor_req = (mac_action_neighbor_req_stru *)buffer;
    neighbor_req->category     = MAC_ACTION_CATEGORY_RADIO_MEASURMENT;
    neighbor_req->action_code  = MAC_RM_ACTION_NEIGHBOR_REPORT_REQUEST;
    neighbor_req->dialog_token = guc_dialog_token;
    *frame_len = MAC_80211_FRAME_LEN + 3;  /* 3 长度 */
}

OAL_STATIC osal_u32 hmac_config_send_meas_req_send_event(oal_netbuf_stru *mgmt_buf, osal_u16 frame_len,
    hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user)
{
    osal_u32 ul_ret;
    mac_tx_ctl_stru *tx_ctl = OAL_PTR_NULL;

    tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(mgmt_buf);
    mac_get_cb_mpdu_len(tx_ctl)  = frame_len;
    mac_get_cb_tx_user_idx(tx_ctl) = (osal_u8)hmac_user->assoc_id;
    oal_netbuf_put(mgmt_buf, frame_len);
    ul_ret = hmac_tx_mgmt_send_event_etc(hmac_vap, mgmt_buf, frame_len);
    if (ul_ret != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_RRM,
            "vap_id[%d] {hmac_config_send_meas_req::hmac_tx_mgmt_send_event_etc failed[%d].}", hmac_vap->vap_id,
            ul_ret);
    }
    return ul_ret;
}

OAL_STATIC osal_u32 hmac_config_do_send_meas_req(mac_rrm_req_cfg_stru *req_cfg, oal_netbuf_stru *mgmt_buf,
    osal_u8 *buffer, hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user)
{
    osal_u32 ul_ret;
    oal_bool_enum_uint8 timer_en = OAL_TRUE;
    osal_u16 frame_len = 0;
    mac_meas_req_ie_stru *meas_req_ie = OAL_PTR_NULL;
    mac_action_rm_req_stru *rm_req = OAL_PTR_NULL;
    hmac_11k_user_info_stru *hmac_11k_user_info = hmac_11k_get_user_info((osal_u8)hmac_user->assoc_id);

    if (hmac_11k_user_info == OSAL_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* Radio Measurement Request */
    if (req_cfg->reqtype <= MAC_RRM_TYPE_BCN) {
        if (req_cfg->p_arg == OAL_PTR_NULL) {
            oam_warning_log1(0, OAM_SF_RRM, "vap_id[%d] {hmac_config_send_meas_req::radio meas req element null.}",
                             hmac_vap->vap_id);
            hmac_dft_print_drop_frame_info(THIS_FILE_ID, __LINE__, 1, OAL_PTR_NULL);
            return OAL_ERR_CODE_PTR_NULL;
        }

        frame_len = (osal_u16)hmac_config_encap_radio_meas_req(req_cfg, buffer, hmac_vap, hmac_user);
        rm_req = (mac_action_rm_req_stru *)buffer;
        meas_req_ie = (mac_meas_req_ie_stru *)rm_req->req_ies;

        /* Radio Measurement Req中不允许对端发送report，则不起定时器 */
        if ((meas_req_ie->reqmode.enable == 1) && (meas_req_ie->reqmode.rpt == 0)) {
            oam_warning_log1(0, OAM_SF_RRM, "vap_id[%d] {hmac_config_send_meas_req::not expect report.}",
                             hmac_user->vap_id);
            timer_en = OAL_FALSE;
        }
    } else if (req_cfg->reqtype == MAC_RRM_TYPE_NEIGHBOR_RPT) {
        hmac_config_encap_neighbor_report_req(&frame_len, buffer);
    } else {
        hmac_dft_print_drop_frame_info(THIS_FILE_ID, __LINE__, 1, OAL_PTR_NULL);
        return OAL_FAIL;
    }

    ul_ret = hmac_config_send_meas_req_send_event(mgmt_buf, frame_len, hmac_vap, hmac_user);
    if (ul_ret != OAL_SUCC) {
        return ul_ret;
    }

    /* Radio Measurement Req中不允许对端发送report，则不起定时器 */
    if (timer_en != OAL_FALSE) {
        /* 更改状态 */
        hmac_11k_user_info->user_rrm_info->meas_status = MAC_RRM_STATE_WAIT_RSP;
        /* 启动measurement超时定时器, 如果超时没回rsp则启动超时处理 */
        hmac_11k_user_info->user_rrm_info->timer.timeout = 5000; // 5000: 5s

        frw_create_timer_entry(&(hmac_11k_user_info->user_rrm_info->timer), hmac_wait_meas_rsp_timeout,
                               hmac_11k_user_info->user_rrm_info->timer.timeout,
                               (osal_void *)((osal_ulong)(hmac_user->assoc_id)), OAL_FALSE);
    }

    return OAL_SUCC;
}

OAL_STATIC osal_u32 hmac_config_send_meas_req_check(hmac_user_stru *hmac_user, mac_rrm_req_cfg_stru *req_cfg)
{
    osal_u32 ul_ret;
    hmac_11k_user_info_stru *hmac_11k_user_info = OSAL_NULL;

    /* 获取USER结构体 */
    if (hmac_user == OAL_PTR_NULL) {
        oam_error_log1(0, OAM_SF_RRM, "vap_id[%d] {hmac_config_send_meas_req::hmac_user null.}",
                       hmac_user->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 检测用户是否支持11K */
    if (hmac_user->cap_info.enable_11k == OAL_FALSE) {
        oam_warning_log1(0, OAM_SF_RRM, "vap_id[%d] {hmac_config_send_meas_req::user not support 11k.}",
            hmac_user->vap_id);
        return OAL_SUCC;
    }

    hmac_11k_user_info = hmac_11k_get_user_info((osal_u8)hmac_user->assoc_id);
    if (hmac_11k_user_info == OAL_PTR_NULL || hmac_11k_user_info->user_rrm_info == OAL_PTR_NULL) {
        oam_error_log1(0, OAM_SF_RRM, "vap_id[%d] {hmac_config_send_meas_req::11k_user_info/user_rrm_info null.}",
            hmac_user->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 检查用户11k具体能力 */
    ul_ret = hmac_rrm_check_user_cap(&(hmac_11k_user_info->rrm_enabled_cap), req_cfg);
    if (ul_ret != OAL_SUCC) {
        return ul_ret;
    }

    return OAL_CONTINUE;
}

/*****************************************************************************
 发起测量请求
*****************************************************************************/
OAL_STATIC osal_u32 hmac_config_send_meas_req(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    mac_rrm_req_cfg_stru *req_cfg)
{
    osal_u32 ul_ret;
    oal_netbuf_stru *mgmt_buf = OAL_PTR_NULL;
    osal_u8 *buffer = OAL_PTR_NULL;
    hmac_11k_user_info_stru *hmac_11k_user_info = hmac_11k_get_user_info((osal_u8)hmac_user->assoc_id);

    ul_ret = hmac_config_send_meas_req_check(hmac_user, req_cfg);
    if (ul_ret != OAL_CONTINUE) {
        return ul_ret;
    }

    /* 清除rpt mode记录 */
    memset_s(&(hmac_11k_user_info->user_rrm_info->rptmode), OAL_SIZEOF(mac_meas_rpt_mode_stru), 0,
        OAL_SIZEOF(mac_meas_rpt_mode_stru));

    /* 记录测试模式 */
    hmac_11k_user_info->user_rrm_info->reqtype = req_cfg->reqtype;
    hmac_11k_user_info->user_rrm_info->rpt_notify_id = req_cfg->rpt_notify_id;

    /* 检查该用户是否正在进行测量 */
    if (hmac_11k_user_info->user_rrm_info->meas_status != MAC_RRM_STATE_INIT) {
        oam_warning_log2(0, OAM_SF_RRM,
                         "vap_id[%d] {hmac_config_send_meas_req::meas_status[%d] not allow new req.}",
                         hmac_user->vap_id, hmac_11k_user_info->user_rrm_info->meas_status);
        /* 通知模块 */
        return OAL_SUCC;
    }

    /* 申请管理帧BUF */
    mgmt_buf = (oal_netbuf_stru *)OAL_MEM_NETBUF_ALLOC(OAL_NORMAL_NETBUF, WLAN_MEM_NETBUF_SIZE2,
        OAL_NETBUF_PRIORITY_MID);
    if (mgmt_buf == OAL_PTR_NULL) {
        oam_error_log1(0, OAM_SF_RRM, "vap_id[%d] {hmac_config_send_meas_req::mgmt_buf null.}",
            hmac_vap->vap_id);
        return OAL_PTR_NULL;
    }

    memset_s(oal_netbuf_cb(mgmt_buf), OAL_NETBUF_CB_SIZE(), 0, OAL_NETBUF_CB_SIZE());
    // 封装基础帧
    hmac_config_encap_mgmt_frame(&buffer, mgmt_buf, hmac_vap, hmac_user);
    /* dialog_token记录 */
    guc_dialog_token++;
    guc_meas_token++;
    hmac_11k_user_info->user_rrm_info->dialog_token   = guc_dialog_token;
    hmac_11k_user_info->user_rrm_info->meas_token     = guc_meas_token;

    // 封装测量内容并发送报文
    ul_ret = hmac_config_do_send_meas_req(req_cfg, mgmt_buf, buffer, hmac_vap, hmac_user);
    if (ul_ret != OAL_SUCC) {
        oal_netbuf_free(mgmt_buf);
    }
    return ul_ret;
}

/*****************************************************************************
 函 数 名  : hmac_rrm_check_user_cap
 功能描述  : 检查用户是否支持测量方式
 输入参数  : osal_void
 输出参数  : 无
*****************************************************************************/
OAL_STATIC osal_u32 hmac_rrm_check_user_cap(mac_rrm_enabled_cap_ie_stru *rrm_enabled_cap,
    mac_rrm_req_cfg_stru *req_cfg)
{
    osal_u32                  ul_ret  = OAL_SUCC;
    mac_bcn_req_stru            *bcn_req;

    /* Neighbor report Request */
    if (req_cfg->reqtype == MAC_RRM_TYPE_NEIGHBOR_RPT &&
        rrm_enabled_cap->neighbor_rpt_cap == OAL_FALSE) {
        oam_warning_log2(0, OAM_SF_RRM,
            "{hmac_rrm_check_user_cap::reqtype[%d],neighbor_rpt_cap[%d] not support.}",
            req_cfg->reqtype, rrm_enabled_cap->neighbor_rpt_cap);
        return OAL_FAIL;
    }

    /* Channel Load Request */
    if (req_cfg->reqtype == MAC_RRM_TYPE_CHANNEL_LOAD &&
        rrm_enabled_cap->chn_load_cap == OAL_FALSE) {
        oam_warning_log2(0, OAM_SF_RRM, "{hmac_rrm_check_user_cap::reqtype[%d],chn_load_cap[%d] not support.}",
            req_cfg->reqtype, rrm_enabled_cap->chn_load_cap);
        return OAL_FAIL;
    }

    /* Beacon Report */
    if (req_cfg->reqtype == MAC_RRM_TYPE_BCN) {
        if (req_cfg->p_arg == OAL_PTR_NULL) {
            oam_warning_log0(0, OAM_SF_RRM, "{hmac_rrm_check_user_cap::bcn req p_arg null.}");
            return OAL_ERR_CODE_PTR_NULL;
        }
        bcn_req = (mac_bcn_req_stru *)(req_cfg->p_arg);

        if (bcn_req->mode == RM_BCN_REQ_MEAS_MODE_PASSIVE &&
            rrm_enabled_cap->bcn_passive_cap ==  OAL_FALSE) {
            oam_warning_log1(0, OAM_SF_RRM, "{hmac_rrm_check_user_cap::bcn::bcn_passive_cap[%d] not support.}",
                rrm_enabled_cap->bcn_passive_cap);
            return OAL_FAIL;
        }

        if (bcn_req->mode == RM_BCN_REQ_MEAS_MODE_ACTIVE &&
            rrm_enabled_cap->bcn_active_cap == OAL_FALSE) {
            oam_warning_log1(0, OAM_SF_RRM, "{hmac_rrm_check_user_cap::bcn::bcn_active_cap[%d] not support.}",
                rrm_enabled_cap->bcn_active_cap);
            return OAL_FAIL;
        }

        if (bcn_req->mode == RM_BCN_REQ_MEAS_MODE_TABLE &&
            rrm_enabled_cap->bcn_table_cap ==  OAL_FALSE) {
            oam_warning_log1(0, OAM_SF_RRM, "{hmac_rrm_check_user_cap::bcn::bcn_table_cap[%d] not support.}",
                rrm_enabled_cap->bcn_table_cap);
            return OAL_FAIL;
        }
    }

    return ul_ret;
}

OAL_STATIC osal_void hmac_rrm_free_bcn_rpt(hmac_user_stru *hmac_user)
{
    mac_user_rrm_info_stru *rrm_info = OSAL_NULL;
    mac_meas_rpt_bcn_stru *bcn = OAL_PTR_NULL;
    struct osal_list_head *list = OAL_PTR_NULL;
    hmac_11k_user_info_stru *hmac_11k_user_info = hmac_11k_get_user_info((osal_u8)hmac_user->assoc_id);

    if (hmac_11k_user_info == OSAL_NULL) {
        return;
    }

    rrm_info = hmac_11k_user_info->user_rrm_info;
    do {
        /* 获取节点 */
        list = rrm_info->meas_rpt_list.next;
        bcn = osal_list_entry(list, mac_meas_rpt_bcn_stru, dlist_head);
        if (bcn == OAL_PTR_NULL) {
            continue;
        }

        /* 释放节点 */
        osal_list_del(list);
        hmac_rrm_beacon_measure_free_one_rpt_bcn_item(bcn);
        bcn = OAL_PTR_NULL;
    } while (osal_list_empty(&(rrm_info->meas_rpt_list)) == 0);

    return;
}

OAL_STATIC osal_void hmac_rrm_free_chan_load_rpt(hmac_user_stru *hmac_user)
{
    struct osal_list_head *list = OAL_PTR_NULL;
    mac_meas_rpt_chn_load_stru *chn_load = OAL_PTR_NULL;
    hmac_11k_user_info_stru *hmac_11k_user_info = hmac_11k_get_user_info((osal_u8)hmac_user->assoc_id);
    mac_user_rrm_info_stru *rrm_info = OSAL_NULL;

    if (hmac_11k_user_info == OSAL_NULL) {
        return;
    }

    rrm_info = hmac_11k_user_info->user_rrm_info;

    do {
        /* 获取节点 */
        list = rrm_info->meas_rpt_list.next;
        chn_load = osal_list_entry(list, mac_meas_rpt_chn_load_stru, dlist_head_chn_load);
        if (chn_load == OAL_PTR_NULL) {
            continue;
        }

        if (chn_load->meas_rpt_chn_load_item != OAL_PTR_NULL) {
            oal_mem_free(chn_load->meas_rpt_chn_load_item, OAL_TRUE);
            chn_load->meas_rpt_chn_load_item = OAL_PTR_NULL;
        }

        /* 释放节点 */
        osal_list_del(list);
        oal_mem_free(chn_load, OAL_TRUE);
        chn_load = OAL_PTR_NULL;
    } while (osal_list_empty(&(rrm_info->meas_rpt_list)) == 0);
    return;
}

OAL_STATIC osal_void hmac_rrm_free_neighbor_rpt(hmac_user_stru *hmac_user)
{
    mac_meas_rpt_neighbor_stru *neighbor = OAL_PTR_NULL;
    struct osal_list_head *list = OAL_PTR_NULL;
    mac_user_rrm_info_stru *rrm_info = OSAL_NULL;
    hmac_11k_user_info_stru *hmac_11k_user_info = hmac_11k_get_user_info((osal_u8)hmac_user->assoc_id);

    if (hmac_11k_user_info == OSAL_NULL) {
        return;
    }

    rrm_info = hmac_11k_user_info->user_rrm_info;
    do {
        /* 获取节点 */
        list = rrm_info->meas_rpt_list.next;
        neighbor = osal_list_entry(list, mac_meas_rpt_neighbor_stru, dlist_head_neighbor);
        if (neighbor == OAL_PTR_NULL) {
            continue;
        }

        if (neighbor->meas_rpt_neighbor_item != OAL_PTR_NULL) {
            oal_mem_free(neighbor->meas_rpt_neighbor_item, OAL_TRUE);
            neighbor->meas_rpt_neighbor_item = OAL_PTR_NULL;
        }

        /* 释放节点 */
        osal_list_del(list);
        oal_mem_free(neighbor, OAL_TRUE);
        neighbor = OAL_PTR_NULL;
    } while (osal_list_empty(&(rrm_info->meas_rpt_list)) == 0);

    return;
}

/*****************************************************************************
 函 数 名  : hmac_rrm_free_rpt_list
 功能描述  : 释放链表
 输入参数  : osal_void
 输出参数  : 无
*****************************************************************************/
OAL_STATIC osal_void hmac_rrm_free_rpt_list(hmac_user_stru *hmac_user, mac_rrm_type_enum reqtype)
{
    /* 11K状态恢复 */
    hmac_11k_user_info_stru *hmac_11k_user_info = hmac_11k_get_user_info((osal_u8)hmac_user->assoc_id);

    if (hmac_11k_user_info == OSAL_NULL || hmac_11k_user_info->user_rrm_info == OSAL_NULL) {
        return;
    }

    hmac_11k_user_info->user_rrm_info->meas_status = MAC_RRM_STATE_INIT;

    if (osal_list_empty(&(hmac_11k_user_info->user_rrm_info->meas_rpt_list)) != 0) {
        return;
    }

    /* 遍历扫描到的bss信息 */
    /* Beacon Report */
    if (reqtype == MAC_RRM_TYPE_BCN) {
        hmac_rrm_free_bcn_rpt(hmac_user);
    }

    /* CHANNEL LOAD Report */
    if (reqtype == MAC_RRM_TYPE_CHANNEL_LOAD) {
        hmac_rrm_free_chan_load_rpt(hmac_user);
    }

    /* Neighbor Report */
    if (reqtype == MAC_RRM_TYPE_NEIGHBOR_RPT) {
        hmac_rrm_free_neighbor_rpt(hmac_user);
    }

    oam_warning_log0(0, OAM_SF_RRM,
        "{hmac_rrm_free_rpt_list: free meas_rpt_list node and set meas_status as MAC_RRM_STATE_INIT.}");

    return;
}

/*****************************************************************************
 函 数 名  : hmac_wait_meas_rsp_timeout
 功能描述  : 发起测量请求
 输入参数  : hmac_user_stru  *hmac_user;
 输出参数  : 无
*****************************************************************************/
OAL_STATIC osal_u32 hmac_wait_meas_rsp_timeout(osal_void *p_arg)
{
    hmac_user_stru  *hmac_user;
    osal_u16       assoc_id;
    hmac_11k_user_info_stru *hmac_11k_user_info = OSAL_NULL;

    if (p_arg == OSAL_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    assoc_id = (osal_u16)(osal_ulong)(p_arg);

    /* 获取USER结构体 */
    hmac_user = (hmac_user_stru *)mac_res_get_hmac_user_etc(assoc_id);
    if (hmac_user == OAL_PTR_NULL) {
        oam_warning_log0(0, OAM_SF_RRM, "{hmac_wait_meas_rsp_timeout::hmac_user null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_11k_user_info = hmac_11k_get_user_info((osal_u8)assoc_id);
    if (hmac_11k_user_info == OAL_PTR_NULL || hmac_11k_user_info->user_rrm_info == OAL_PTR_NULL) {
        oam_warning_log1(0, OAM_SF_RRM,
            "vap_id[%d] {hmac_wait_meas_rsp_timeout::hmac_11k_user_info/user_rrm_info null.}",
            hmac_user->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    oam_warning_log2(0, OAM_SF_RRM, "vap_id[%d] {hmac_wait_meas_rsp_timeout::reqtype=%d.}",
        hmac_user->vap_id, hmac_11k_user_info->user_rrm_info->reqtype);

    /* 更改状态 */
    hmac_11k_user_info->user_rrm_info->meas_status = MAC_RRM_STATE_RSP_TIMEOUT;

    /* 通知模块 */
    /* 释放链表和恢复状态 */
    hmac_rrm_free_rpt_list(hmac_user, hmac_11k_user_info->user_rrm_info->reqtype);

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_vap_meas_status_timeout
 功能描述  : 发起测量请求
 输入参数  : hmac_vap_stru  *hmac_vap;
 输出参数  : 无
*****************************************************************************/

OAL_STATIC osal_u32 hmac_vap_meas_status_timeout(osal_void *p_arg)
{
    mac_vap_rrm_info_stru *rrm_info;
    rrm_info = (mac_vap_rrm_info_stru *)p_arg;

    if (rrm_info == OAL_PTR_NULL) {
        oam_warning_log0(0, OAM_SF_RRM, "{hmac_vap_meas_status_timeout::rrm_info null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 更改状态 */
    rrm_info->is_measuring = OAL_FALSE;

    reset_rrm_info(rrm_info);
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_rrm_get_meas_start_time
 功能描述  : hmac接收dmac抛回来的查询应答事件
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
OAL_STATIC osal_bool hmac_rrm_get_meas_start_time(osal_void *notify_data)
{
    hmac_device_stru *hmac_device = (hmac_device_stru *)notify_data;
    hmac_vap_stru *hmac_vap = OSAL_NULL;
    osal_u32 meas_start_time[2]; /* 2表示数组大小 */
    hmac_11k_vap_info_stru *hmac_11k_vap_info = OSAL_NULL;

    hmac_vap = mac_res_get_hmac_vap(hmac_device->scan_params.vap_id);
    if (hmac_vap == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_RRM, "{hmac_rrm_get_meas_start_time:: hmac_vap is null.}");
        return OAL_FALSE;
    }

    if (hmac_vap_id_param_check(hmac_vap->vap_id) != OSAL_TRUE) {
        return OSAL_TRUE;
    }

    if (!is_legacy_sta(hmac_vap)) {
        return OSAL_TRUE;
    }

    if (mac_mib_get_dot11_radio_measurement_activated(hmac_vap) != OAL_TRUE) {
        return OSAL_TRUE;
    }

    hmac_11k_vap_info = hmac_11k_get_vap_info(hmac_vap);
    if (hmac_11k_vap_info == OSAL_NULL) {
        return OSAL_FALSE;
    }

    if (hmac_11k_vap_info->enable_11k != 0 && hmac_11k_vap_info->rrm_info != OAL_PTR_NULL) {
        hal_vap_tsf_get_64bit(hmac_vap->hal_vap, (osal_u32 *)&meas_start_time[1], (osal_u32 *)&meas_start_time[0]);
        /* Actual Meas Start Time */
        if (memcpy_s(hmac_11k_vap_info->rrm_info->act_meas_start_time, OAL_SIZEOF(osal_u32) * 2, /* 2倍大小 */
            meas_start_time, OAL_SIZEOF(osal_u32) * 2) != EOK) { /* 2倍大小 */
            oam_warning_log0(0, OAM_SF_CFG, "{hmac_rrm_get_meas_start_time data::memcpy err.}");
        }
    }

    return OAL_TRUE;
}

/*****************************************************************************
 函 数 名  : hmac_rrm_fill_basic_rm_rpt_action
 功能描述  : 申请管理帧内存，填充管理帧和Radio Measurement Report的固定域
 输入参数  : hmac_vap
 输出参数  : 无
*****************************************************************************/
OAL_STATIC osal_u32 hmac_rrm_fill_basic_rm_rpt_action(hmac_vap_stru *hmac_vap)
{
    osal_u8                       *mac_header;
    osal_u8                       *payload;
    hmac_user_stru                *hmac_user = OSAL_NULL;
    mac_vap_rrm_info_stru         *rrm_info = hmac_11k_get_vap_rrm_info(hmac_vap);

    if (rrm_info == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_RRM, "{hmac_rrm_fill_basic_rm_rpt_action::rrm_info NULL!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (rrm_info->rm_rpt_mgmt_buf != OAL_PTR_NULL) {
        oal_netbuf_free(rrm_info->rm_rpt_mgmt_buf);
        rrm_info->rm_rpt_mgmt_buf = OAL_PTR_NULL;
        oam_warning_log1(0, OAM_SF_RRM,
            "vap_id[%d] {hmac_rrm_fill_basic_rm_rpt_action::mgmt_buf not be freed in history.}",
            hmac_vap->vap_id);
    }

    /* 申请管理帧内存 */
    rrm_info->rm_rpt_mgmt_buf = OAL_MEM_NETBUF_ALLOC(OAL_NORMAL_NETBUF, WLAN_LARGE_NETBUF_SIZE,
        OAL_NETBUF_PRIORITY_MID);
    if (rrm_info->rm_rpt_mgmt_buf == OAL_PTR_NULL) {
        oam_error_log1(0, OAM_SF_RRM, "vap_id[%d] {hmac_rrm_fill_basic_rm_rpt_action::mgmt_buf null.}",
            hmac_vap->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }
    oal_set_netbuf_prev(rrm_info->rm_rpt_mgmt_buf, OAL_PTR_NULL);
    oal_set_netbuf_next(rrm_info->rm_rpt_mgmt_buf, OAL_PTR_NULL);

    /*************************************************************************/
    /*                        Management Frame Format                        */
    /* --------------------------------------------------------------------  */
    /* |Frame Control|Duration|DA|SA|BSSID|Sequence Control|Frame Body|FCS|  */
    /* --------------------------------------------------------------------  */
    /* | 2           |2       |6 |6 |6    |2               |0 - 2312  |4  |  */
    /* --------------------------------------------------------------------  */
    /*                                                                       */
    /*************************************************************************/

    /*************************************************************************/
    /*                Set the fields in the frame header                     */
    /*************************************************************************/
    mac_header = oal_netbuf_header(rrm_info->rm_rpt_mgmt_buf);

    /* 帧控制字段全为0，除了type和subtype */
    mac_hdr_set_frame_control(mac_header, WLAN_PROTOCOL_VERSION | WLAN_FC0_TYPE_MGT | WLAN_FC0_SUBTYPE_ACTION);

    /* 设置分片序号为0 */
    mac_hdr_set_fragment_number(mac_header, 0);

    hmac_user = mac_res_get_hmac_user_etc(rrm_info->req_user_id);
    if (hmac_user == OAL_PTR_NULL) {
        /* ERROR to Warning */
        oam_warning_log1(0, OAM_SF_TX, "{hmac_rrm_fill_basic_rm_rpt_action::hmac_user[%d] null.",
            rrm_info->req_user_id);
        oal_netbuf_free(rrm_info->rm_rpt_mgmt_buf);
        rrm_info->rm_rpt_mgmt_buf = OAL_PTR_NULL;
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 设置地址1，用户地址 */
    oal_set_mac_addr(mac_header + WLAN_HDR_ADDR1_OFFSET, hmac_user->user_mac_addr);

    /* 设置地址2为自己的MAC地址 */
    oal_set_mac_addr(mac_header + WLAN_HDR_ADDR2_OFFSET, mac_mib_get_station_id(hmac_vap));

    /* 地址3，为VAP自己的MAC地址 */
    oal_set_mac_addr(mac_header + WLAN_HDR_ADDR3_OFFSET, hmac_vap->bssid);

    /*************************************************************************/
    /*                    Radio Measurement Report Frame - Frame Body        */
    /* --------------------------------------------------------------------- */
    /* |Category |Action |Dialog Token| Measurement Report Elements        | */
    /* --------------------------------------------------------------------- */
    /* |1        |1      | 1          |  var                                 */
    /* --------------------------------------------------------------------- */
    /*                                                                       */
    /*************************************************************************/
    payload = mac_netbuf_get_payload(rrm_info->rm_rpt_mgmt_buf);

    rrm_info->rm_rpt_action                  = (mac_action_rm_rpt_stru *)payload;
    rrm_info->rm_rpt_action->category     = MAC_ACTION_CATEGORY_RADIO_MEASURMENT;
    rrm_info->rm_rpt_action->action_code  = rrm_info->action_code + 1; // radio or neighbor reponse
    rrm_info->rm_rpt_action->dialog_token = rrm_info->dialog_token;

    rrm_info->rm_rpt_action_len = MAC_ACTION_RPT_FIX_LEN + MAC_80211_FRAME_LEN;
    return OAL_SUCC;
}

OAL_STATIC osal_u32 hmac_rrm_send_rm_rpt_action_check(hmac_vap_stru *hmac_vap)
{
    oal_netbuf_stru *mgmt_buf = OAL_PTR_NULL;
    hmac_user_stru *hmac_user = OAL_PTR_NULL;
    mac_vap_rrm_info_stru *rrm_info = OAL_PTR_NULL;

    if (hmac_vap == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_RRM, "{hmac_rrm_send_rm_rpt_action_check::hmac_vap NULL!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    rrm_info = hmac_11k_get_vap_rrm_info(hmac_vap);
    if (rrm_info == OAL_PTR_NULL) {
        oam_error_log1(0, OAM_SF_RRM, "vap_id[%d] {hmac_rrm_send_rm_rpt_action_check::rrm_info NULL!}",
            hmac_vap->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    rrm_info->is_measuring = OAL_FALSE;
    /* 取消定时器 */
    if (rrm_info->meas_status_timer.is_registerd == OAL_TRUE) {
        frw_destroy_timer_entry(&(rrm_info->meas_status_timer));
        oam_warning_log1(0, OAM_SF_RRM, "vap_id[%d] {hmac_rrm_send_rm_rpt_action_check:: cancel meas_status_timer.}",
            hmac_vap->vap_id);
    }

    mgmt_buf = rrm_info->rm_rpt_mgmt_buf;
    if (mgmt_buf == OAL_PTR_NULL) {
        oam_error_log1(0, OAM_SF_RRM, "vap_id[%d] {hmac_rrm_send_rm_rpt_action_check::mgmt_buf NULL!}",
            hmac_vap->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_user = mac_res_get_hmac_user_etc(rrm_info->req_user_id);
    if (hmac_user == OAL_PTR_NULL) {
        oam_error_log2(0, OAM_SF_RRM, "vap_id[%d] {hmac_rrm_send_rm_rpt_action_check::hmac_user[%d] null.",
            hmac_vap->vap_id, rrm_info->req_user_id);
        oal_netbuf_free(rrm_info->rm_rpt_mgmt_buf);
        rrm_info->rm_rpt_mgmt_buf = OAL_PTR_NULL;
        return OAL_ERR_CODE_PTR_NULL;
    }
    return OAL_SUCC;
}

/*****************************************************************************
 发送已经组装好的Radio Measurement Report帧
*****************************************************************************/
OAL_STATIC osal_u32 hmac_rrm_send_rm_rpt_action(hmac_vap_stru *hmac_vap)
{
    mac_tx_ctl_stru *tx_ctl = OAL_PTR_NULL;
    oal_netbuf_stru *mgmt_buf = OAL_PTR_NULL;
    osal_u32 ul_ret;
    hmac_user_stru *hmac_user = OAL_PTR_NULL;
    mac_vap_rrm_info_stru *rrm_info = hmac_11k_get_vap_rrm_info(hmac_vap);

    ul_ret = hmac_rrm_send_rm_rpt_action_check(hmac_vap);
    if (ul_ret != OAL_SUCC) {
        return ul_ret;
    }

    mgmt_buf = rrm_info->rm_rpt_mgmt_buf;
    hmac_user = mac_res_get_hmac_user_etc(rrm_info->req_user_id);

    /* 填写netbuf的cb字段，供发送管理帧和发送完成接口使用 */
    tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(mgmt_buf);
    memset_s(tx_ctl, sizeof(mac_tx_ctl_stru), 0, sizeof(mac_tx_ctl_stru));

    mac_get_cb_tx_user_idx(tx_ctl) = (osal_u8)hmac_user->assoc_id;
    mac_get_cb_wme_ac_type(tx_ctl) = WLAN_WME_AC_MGMT;
    mac_get_cb_mpdu_len(tx_ctl) = rrm_info->rm_rpt_action_len;
    oal_netbuf_put(mgmt_buf, rrm_info->rm_rpt_action_len);

    /* 调用发送管理帧接口 */
    ul_ret = hmac_tx_mgmt_send_event_etc(hmac_vap, mgmt_buf, rrm_info->rm_rpt_action_len);
    if (ul_ret != OAL_SUCC) {
        oal_netbuf_free(mgmt_buf);
        oam_error_log1(0, OAM_SF_RRM, "vap_id[%d] {hmac_rrm_send_rm_rpt_action::tx link meas rpt action failed.}",
            hmac_vap->vap_id);
    }

    rrm_info->rm_rpt_mgmt_buf = OSAL_NULL;
    return ul_ret;
}

/*****************************************************************************
 函 数 名  : hmac_rrm_encap_meas_rpt_reject
 功能描述  : 发送携带拒绝或者不支持信息的Radio Measurement Report
 输入参数  : hmac_vap
             rptmode
 输出参数  : 无
*****************************************************************************/
OAL_STATIC osal_void hmac_rrm_encap_meas_rpt_reject(hmac_vap_stru *hmac_vap, mac_meas_rpt_mode_stru *rptmode)
{
    mac_vap_rrm_info_stru *rrm_info = hmac_11k_get_vap_rrm_info(hmac_vap);
    mac_meas_rpt_ie_stru *meas_rpt_ie;

    if ((hmac_vap == OAL_PTR_NULL) || (rptmode == OAL_PTR_NULL)) {
        oam_error_log0(0, OAM_SF_RRM, "{hmac_rrm_encap_meas_rpt_reject:: null param!}");
        return;
    }

    if (rrm_info == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_RRM, "{hmac_rrm_encap_meas_rpt_reject:: rrm_info null!}");
        return;
    }

    rrm_info->is_measuring = OAL_FALSE;
    /* 取消定时器 */
    if (rrm_info->meas_status_timer.is_registerd == OAL_TRUE) {
        frw_destroy_timer_entry(&(rrm_info->meas_status_timer));
        oam_warning_log1(0, OAM_SF_RRM, "vap_id[%d] {hmac_rrm_encap_meas_rpt_reject:: cancel meas_status_timer.}",
            hmac_vap->vap_id);
    }

    if (hmac_rrm_fill_basic_rm_rpt_action(hmac_vap) != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_RRM, "{hmac_rrm_encap_meas_rpt_reject:: hmac_rrm_fill_basic_rm_rpt_action fail!}");
        return;
    }

    if ((rrm_info->rm_rpt_action == OSAL_NULL) || (rrm_info->meas_req_ie == OSAL_NULL)) {
        oam_warning_log0(0, OAM_SF_RRM, "{hmac_rrm_encap_meas_rpt_reject::rrm_info has NULL PTR!}");
        return;
    }

    meas_rpt_ie = (mac_meas_rpt_ie_stru *)rrm_info->rm_rpt_action->rpt_ies;

    /**************************************************************************/
    /*                   Measurement Report IE - Frame Body                   */
    /* ---------------------------------------------------------------------- */
    /* |Element ID |Length |Meas Token| Meas Rpt Mode | Meas Type | Meas Rpt| */
    /* ---------------------------------------------------------------------- */
    /* |1          |1      | 1        |  1            | 1         | var       */
    /* ---------------------------------------------------------------------- */
    /*                                                                        */
    /**************************************************************************/
    meas_rpt_ie->eid       = MAC_EID_MEASREP;
    meas_rpt_ie->token     = rrm_info->meas_req_ie->token;
    meas_rpt_ie->rpttype   = rrm_info->meas_req_ie->reqtype;
    if (memcpy_s(&(meas_rpt_ie->rptmode), OAL_SIZEOF(mac_meas_rpt_mode_stru),
        rptmode, OAL_SIZEOF(mac_meas_rpt_mode_stru)) != EOK) {
        oam_warning_log0(0, OAM_SF_CFG, "{hmac_rrm_encap_meas_rpt_reject data::memcpy err.}");
    }
    meas_rpt_ie->len       = MAC_MEASUREMENT_RPT_FIX_LEN - MAC_IE_HDR_LEN;

    rrm_info->rm_rpt_action_len          += MAC_MEASUREMENT_RPT_FIX_LEN;

    (osal_void)hmac_rrm_send_rm_rpt_action(hmac_vap);
}

OAL_STATIC osal_void hmac_rrm_fill_meas_frame_body(oal_netbuf_stru *mgmt_buf, hmac_user_stru *hmac_user,
    hmac_vap_stru *hmac_vap, mac_action_rm_req_stru *rm_req, osal_u16 *frame_len)
{
    osal_u8 *mac_hdr = OAL_PTR_NULL;
    osal_u8 *payload = OAL_PTR_NULL;
    mac_meas_rpt_ie_stru *meas_rpt_ie = OAL_PTR_NULL;
    mac_action_rm_rpt_stru *rm_rpt_action = OAL_PTR_NULL;
    mac_tx_ctl_stru *tx_ctl = OAL_PTR_NULL;
    mac_meas_req_ie_stru *meas_req_ie = OAL_PTR_NULL;

    meas_req_ie = (mac_meas_req_ie_stru *)&(rm_req->req_ies[0]);

    /*************************************************************************/
    /*                        Management Frame Format                        */
    /* --------------------------------------------------------------------  */
    /* |Frame Control|Duration|DA|SA|BSSID|Sequence Control|Frame Body|FCS|  */
    /* --------------------------------------------------------------------  */
    /* | 2           |2       |6 |6 |6    |2               |0 - 2312  |4  |  */
    /* --------------------------------------------------------------------  */
    /*                                                                       */
    /*************************************************************************/

    /*************************************************************************/
    /*                Set the fields in the frame header                     */
    /*************************************************************************/
    mac_hdr = oal_netbuf_header(mgmt_buf);

    /* 帧控制字段全为0，除了type和subtype */
    mac_hdr_set_frame_control(mac_hdr, WLAN_PROTOCOL_VERSION | WLAN_FC0_TYPE_MGT | WLAN_FC0_SUBTYPE_ACTION);

    /* 设置分片序号为0 */
    mac_hdr_set_fragment_number(mac_hdr, 0);

    /* 设置地址1，用户地址 */
    oal_set_mac_addr(mac_hdr + WLAN_HDR_ADDR1_OFFSET, hmac_user->user_mac_addr);

    /* 设置地址2为自己的MAC地址 */
    oal_set_mac_addr(mac_hdr + WLAN_HDR_ADDR2_OFFSET, mac_mib_get_station_id(hmac_vap));

    /* 地址3，为VAP自己的MAC地址 */
    oal_set_mac_addr(mac_hdr + WLAN_HDR_ADDR3_OFFSET, hmac_vap->bssid);

    /*************************************************************************/
    /*                    Radio Measurement Report Frame - Frame Body        */
    /* --------------------------------------------------------------------- */
    /* |Category |Action |Dialog Token| Measurement Report Elements        | */
    /* --------------------------------------------------------------------- */
    /* |1        |1      | 1          |  var                                 */
    /* --------------------------------------------------------------------- */
    /*                                                                       */
    /*************************************************************************/
    payload = mac_netbuf_get_payload(mgmt_buf);

    rm_rpt_action                  = (mac_action_rm_rpt_stru *)payload;
    rm_rpt_action->category     = MAC_ACTION_CATEGORY_RADIO_MEASURMENT;
    rm_rpt_action->action_code  = MAC_RM_ACTION_RADIO_MEASUREMENT_REPORT;
    rm_rpt_action->dialog_token = rm_req->dialog_token;

    *frame_len = MAC_ACTION_RPT_FIX_LEN + MAC_MEASUREMENT_RPT_FIX_LEN + MAC_80211_FRAME_LEN;

    meas_rpt_ie   = (mac_meas_rpt_ie_stru *)rm_rpt_action->rpt_ies;

    /**************************************************************************/
    /*                   Measurement Report IE - Frame Body                   */
    /* ---------------------------------------------------------------------- */
    /* |Element ID |Length |Meas Token| Meas Rpt Mode | Meas Type | Meas Rpt| */
    /* ---------------------------------------------------------------------- */
    /* |1          |1      | 1        |  1            | 1         | var       */
    /* ---------------------------------------------------------------------- */
    /*                                                                        */
    /**************************************************************************/
    meas_rpt_ie->eid       = MAC_EID_MEASREP;
    meas_rpt_ie->token     = meas_req_ie->token;
    meas_rpt_ie->rpttype   = meas_req_ie->reqtype;
    meas_rpt_ie->rptmode.refused = 1;
    meas_rpt_ie->len       = MAC_ACTION_RPT_FIX_LEN;

    /* 填写netbuf的cb字段，供发送管理帧和发送完成接口使用 */
    tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(mgmt_buf);
    memset_s(tx_ctl, sizeof(mac_tx_ctl_stru), 0, sizeof(mac_tx_ctl_stru));

    mac_get_cb_tx_user_idx(tx_ctl)   = (osal_u8)hmac_user->assoc_id;
    mac_get_cb_wme_ac_type(tx_ctl)   = WLAN_WME_AC_MGMT;
    return;
}

/*****************************************************************************
 VAP收到多个测量请求，则拒绝新的测量请求
*****************************************************************************/
OAL_STATIC osal_u32 hmac_rrm_encap_meas_rpt_refuse_new_req(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    mac_action_rm_req_stru *rm_req)
{
    oal_netbuf_stru *mgmt_buf = OAL_PTR_NULL;
    osal_u16 action_len = 0;
    osal_u32 ul_ret;
    mac_meas_req_ie_stru *meas_req_ie = OAL_PTR_NULL;

    if (hmac_vap == OAL_PTR_NULL || hmac_user == OAL_PTR_NULL || rm_req == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_RRM, "{hmac_rrm_encap_meas_rpt_refuse_new_req::input is NULL!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    meas_req_ie = (mac_meas_req_ie_stru *)&(rm_req->req_ies[0]);

    /* Req中不允许发送对应的report */
    if ((meas_req_ie->reqmode.enable == 1) && (meas_req_ie->reqmode.rpt == 0)) {
        oam_warning_log0(0, OAM_SF_RRM, "{hmac_rrm_encap_meas_rpt_refuse_new_req::rpt now allowed!}");
        return OAL_SUCC;
    }

    /* 申请管理帧内存 */
    mgmt_buf = OAL_MEM_NETBUF_ALLOC(OAL_NORMAL_NETBUF, WLAN_LARGE_NETBUF_SIZE, OAL_NETBUF_PRIORITY_MID);
    if (mgmt_buf == OAL_PTR_NULL) {
        oam_error_log1(0, OAM_SF_RRM, "vap_id[%d] {hmac_rrm_encap_meas_rpt_refuse_new_req::mgmt_buf null.}",
            hmac_vap->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    memset_s(oal_netbuf_cb(mgmt_buf), OAL_NETBUF_CB_SIZE(), 0, OAL_NETBUF_CB_SIZE());
    hmac_rrm_fill_meas_frame_body(mgmt_buf, hmac_user, hmac_vap, rm_req, &action_len);
    oal_netbuf_put(mgmt_buf, action_len);

    /* 调用发送管理帧接口 */
    ul_ret = hmac_tx_mgmt_send_event_etc(hmac_vap, mgmt_buf, action_len);
    if (ul_ret != OAL_SUCC) {
        oal_netbuf_free(mgmt_buf);
        oam_error_log1(0, OAM_SF_RRM,
            "vap_id[%d] {hmac_rrm_encap_meas_rpt_refuse_new_req::tx link meas rpt action failed.}",
            hmac_vap->vap_id);
        return ul_ret;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_rrm_bcn_rpt_filter
 功能描述  : bcn_rpt_filter
 输入参数  : hmac_vap_stru *hmac_vap,
             mac_bss_dscr_stru *bss_dscr
 输出参数  : 无
*****************************************************************************/
OAL_STATIC osal_u32 hmac_rrm_bcn_rpt_filter(hmac_vap_stru *hmac_vap, mac_bss_dscr_stru *bss_dscr,
    const mac_scan_req_stru *scan_params)
{
    osal_u8                   ssid_len;
    osal_u8                   chan_idx;
    mac_vap_rrm_info_stru      *rrm_info = hmac_11k_get_vap_rrm_info(hmac_vap);

    if (rrm_info == OSAL_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 遍历信道 */
    for (chan_idx = 0; chan_idx < scan_params->channel_nums; chan_idx++) {
        if (bss_dscr->st_channel.chan_number ==
            scan_params->channel_list[chan_idx].chan_number) {
            break;
        }
    }
    if (scan_params->channel_nums == chan_idx) {
        return OAL_FAIL;
    }

    /* BSSID过滤 */
    if (!ETHER_IS_BROADCAST(rrm_info->bcn_req_info.bssid)) {
        /* 请求中的bssid不是wildcard，则需要bssid匹配，不匹配时不上报 */
        if (osal_memcmp(rrm_info->bcn_req_info.bssid, bss_dscr->bssid, WLAN_MAC_ADDR_LEN) != 0) {
            return OAL_FAIL;
        }
    }

    /* SSID过滤，若请求中ssid长度为0，则不过滤 */
    if (rrm_info->bcn_req_info.ssid_len != 0) {
        ssid_len = (osal_u8)osal_strlen(bss_dscr->ac_ssid);
        /* ssid不匹配的不处理 */
        if ((rrm_info->bcn_req_info.ssid_len != ssid_len) ||
            (osal_memcmp(rrm_info->bcn_req_info.ssid, bss_dscr->ac_ssid,
            rrm_info->bcn_req_info.ssid_len) != 0)) {
            return OAL_FAIL;
        }
    }

    return OAL_SUCC;
}

OAL_STATIC hmac_vap_stru *hmac_rrm_bcn_get_vap(hmac_scan_record_stru *scan_record, mac_meas_rpt_mode_stru *rptmode)
{
    /* 获取hmac VAP */
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(scan_record->vap_id);
    if ((hmac_vap == OSAL_NULL) || (hmac_11k_get_vap_rrm_info(hmac_vap) == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_RRM, "{hmac_rrm_bcn_get_vap: hmac_vap or rrm_info is null!}");
        return OSAL_NULL;
    }

    (osal_void)memset_s(rptmode, sizeof(mac_meas_rpt_mode_stru), 0, sizeof(mac_meas_rpt_mode_stru));
    /* 判断扫描状态是否成功 */
    if (scan_record->scan_rsp_status != MAC_SCAN_SUCCESS) {
        rptmode->refused = 1;
        hmac_rrm_encap_meas_rpt_reject(hmac_vap, rptmode);
        oam_warning_log2(0, OAM_SF_RRM, "vap_id[%d] hmac_rrm_bcn_get_vap::scan fail, status[%d].",
            hmac_vap->vap_id, scan_record->scan_rsp_status);
        return OSAL_NULL;
    }
    return hmac_vap;
}

OAL_STATIC osal_void hmac_rrm_fill_meas_sub_elm(hmac_vap_stru *hmac_vap, mac_bcn_rpt_stru *bcn_rpt_item,
    mac_meas_rpt_bcn_stru *meas_rpt_bcn)
{
    /* subElement ID: Reported Frame Body(1) */
    mac_meas_sub_ie_stru *meas_sub_ie = (mac_meas_sub_ie_stru *)bcn_rpt_item->subelm;
    meas_sub_ie->sub_eid = 1;
    meas_sub_ie->len = (osal_u8)meas_rpt_bcn->rpt_detail_act_len;

    unref_param(hmac_vap);
    if (memcpy_s(meas_sub_ie->meas_sub_data, meas_rpt_bcn->rpt_detail_act_len,
        meas_rpt_bcn->rpt_detail_data, meas_rpt_bcn->rpt_detail_act_len) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "hmac_rrm_fill_meas_sub_elm:memcopy fail !");
    }
}

OAL_STATIC osal_void hmac_rrm_fill_frame_body_fragment_id(
    mac_bcn_rpt_stru *bcn_rpt_item, mac_meas_rpt_bcn_stru *meas_rpt_bcn)
{
    /* subElement ID: Reported Frame Body Fragment ID(2) */
    mac_reported_frame_body_fragment_id_stru *rpt_frame_body_frag_id = NULL;
    rpt_frame_body_frag_id = (mac_reported_frame_body_fragment_id_stru *)(bcn_rpt_item->subelm +
        2 + meas_rpt_bcn->rpt_detail_act_len); /* 2: frame_body_frag offset */
    rpt_frame_body_frag_id->sub_eid = 2; /* subele id: 2 */
    rpt_frame_body_frag_id->len = 2; /* Length: 2 */
    rpt_frame_body_frag_id->bit_beacon_report_id = meas_rpt_bcn->bit_beacon_report_id;
    rpt_frame_body_frag_id->bit_fragment_id_number = meas_rpt_bcn->bit_fragment_id_number;
    rpt_frame_body_frag_id->bit_more_frame_body_fragments = meas_rpt_bcn->bit_more_frame_body_fragments;
}

OAL_STATIC mac_last_beacon_report_indication_stru *hmac_rrm_get_last_bcn_rpt_indication(
    mac_bcn_rpt_stru *bcn_rpt_item, mac_meas_rpt_bcn_stru *meas_rpt_bcn)
{
    mac_last_beacon_report_indication_stru *last_bcn_rpt_indication = NULL;
    last_bcn_rpt_indication = (mac_last_beacon_report_indication_stru *)(bcn_rpt_item->subelm +
        6 + meas_rpt_bcn->rpt_detail_act_len); /* 6: last ast_beacon_report_indication offset */
    last_bcn_rpt_indication->sub_eid = 164; /* subele id: 164 */
    last_bcn_rpt_indication->len = 1;
    last_bcn_rpt_indication->last_beacon_report_indication = 0;
    return last_bcn_rpt_indication;
}

/*
 * 函 数 名  : dmac_rrm_extract_bcn_rpt_fix_field
 * 功能描述  : 填充Beacon report固定长度域
 */
OAL_STATIC osal_void hmac_rrm_extract_bcn_rpt_fix_field(hmac_vap_stru *hmac_vap,
    mac_meas_rpt_bcn_item_stru *rpt_item, mac_bss_dscr_stru *dscr)
{
    mac_vap_rrm_info_stru *rrm_info = hmac_11k_get_vap_rrm_info(hmac_vap);

    if (rrm_info == OSAL_NULL) {
        return;
    }
    /**************************************************************************/
    /*                   Beacon Report - Frame Body                           */
    /* ---------------------------------------------------------------------- */
    /* |oper class|chn num|Actual Meas Start Time|Meas Duration|Rpt Frm Info| */
    /* ---------------------------------------------------------------------- */
    /* |1         |1      |8                     |2            | 1          | */
    /* ---------------------------------------------------------------------- */
    /* ---------------------------------------------------------------------- */
    /* | RCPI | RSNI | BSSID | Antenna ID | Parent TSF| Optional Subelements| */
    /* ---------------------------------------------------------------------- */
    /* |1     |1     |6      |1           |4          | Var                 | */
    /* ---------------------------------------------------------------------- */
    /*                                                                        */
    /**************************************************************************/

    /* fixed bcn rpt */
    rpt_item->optclass = rrm_info->bcn_req_info.opt_class;
    rpt_item->channum = dscr->st_channel.chan_number;
    (osal_void)memcpy_s(rpt_item->act_meas_start_time, sizeof(rpt_item->act_meas_start_time),
        rrm_info->act_meas_start_time, sizeof(rrm_info->act_meas_start_time));

    /* Meas Duration,参考商用设备, bcn report与req duration填写一致 */
    rpt_item->duration = rrm_info->bcn_req_info.meas_duration;

    rpt_item->condensed_phy_type    = dscr->phy_type;
    rpt_item->rpt_frm_type          = 0;    /* Beacon/Probe rsp */
    rpt_item->rcpi = (osal_u8)((dscr->c_rssi + 110) * 2); /* 计算+110, *2 */
    rpt_item->rsni = (osal_u8)((dscr->ac_rsni[0] + dscr->ac_rsni[1]) / 2); /* 2 平均 */
    (osal_void)memcpy_s(rpt_item->bssid, sizeof(rpt_item->bssid), dscr->bssid, sizeof(dscr->bssid));

    rpt_item->antenna_id             = 0;    // unknown
    rpt_item->parent_tsf             = dscr->parent_tsf;
    return;
}

/*
 * 函 数 名  : dmac_rrm_alloc_rpt_detail
 * 功能描述  : 申请内存填充Beacon report的frame body
 */
OAL_STATIC osal_u8* hmac_rrm_alloc_rpt_detail(const mac_vap_rrm_info_stru *rrm_info, mac_bss_dscr_stru *dscr,
    osal_u32 payload_len, mac_meas_rpt_bcn_stru *meas_rpt_bcn)
{
    osal_u8 *rpt_detail_data = OSAL_NULL;
    osal_u8 *frame_body = OSAL_NULL;
    osal_u8 fix_len;

    /* 填充Report Frame Body */
    if (rrm_info->bcn_req_info.rpt_detail == 0) {
        oam_warning_log0(0, OAM_SF_RRM, "{hmac_rrm_alloc_rpt_detail::rpt detail is 0}");
        return OSAL_NULL;
    }

    /* 定长field拷贝 */
    fix_len = MAC_TIME_STAMP_LEN + MAC_BEACON_INTERVAL_LEN + MAC_CAP_INFO_LEN;
    if (payload_len <= fix_len) {
        return OSAL_NULL;
    }

    /* report detail内容要放在Meas Rpt中，最长不能超过Meas rpt的长度 */
    rpt_detail_data = oal_mem_alloc(OAL_MEM_POOL_ID_LOCAL, MAC_MAX_RPT_MBO_DETAIL_LEN, OSAL_TRUE);
    if (rpt_detail_data == NULL) {
        return OSAL_NULL;
    }

    meas_rpt_bcn->rpt_detail_data = rpt_detail_data;
    meas_rpt_bcn->rpt_detail_act_len = fix_len;
    frame_body = dscr->mgmt_buff + MAC_80211_FRAME_LEN;
    if (memcpy_s(rpt_detail_data, MAC_MAX_RPT_MBO_DETAIL_LEN, frame_body, fix_len) != EOK) {
        oal_mem_free(rpt_detail_data, OSAL_TRUE);
        oam_error_log0(0, OAM_SF_RRM, "{hmac_rrm_extract_bcn_rpt_detail::memcpy failed!}");
        return OSAL_NULL;
    }
    rpt_detail_data += fix_len;
    return rpt_detail_data;
}

/*
 * 函 数 名  : hmac_rrm_extract_bcn_rpt_detail_remaining
 * 功能描述  : 填充Beacon report的frame body剩余部分，beacon超长了
 */
OAL_STATIC osal_void hmac_rrm_extract_bcn_rpt_detail_remaining(hmac_vap_stru *hmac_vap,
    osal_u8 *frm_body_part2, osal_u8 **frame_body_remaining,
    mac_meas_rpt_bcn_stru *meas_rpt_bcn_remaining, hmac_rrm_rpt_frag_stru* rpt_frag)
{
    osal_u8 *rpt_detail_data = NULL;
    int32_t temp_len;
    mac_vap_rrm_info_stru *rrm_info = hmac_11k_get_vap_rrm_info(hmac_vap);

    if (rrm_info == OSAL_NULL) {
        return;
    }

    /* 填充Report Frame Body */
    if (rrm_info->bcn_req_info.rpt_detail == 0) {
        oam_warning_log0(0, OAM_SF_RRM, "{hmac_rrm_extract_bcn_rpt_detail_remaining::rpt detail is 0}");
        return;
    }

    /* report detail内容要放在Meas Rpt中，Meas rpt的长度最长255 */
    rpt_detail_data = oal_mem_alloc(OAL_MEM_POOL_ID_LOCAL, MAC_MAX_RPT_MBO_DETAIL_LEN, OSAL_TRUE);
    if (rpt_detail_data == NULL) {
        return;
    }

    meas_rpt_bcn_remaining->rpt_detail_data = rpt_detail_data;
    temp_len = rpt_frag->frame_len_remaining;
    while (temp_len > MAC_IE_HDR_LEN) {
        if (meas_rpt_bcn_remaining->rpt_detail_act_len + MAC_IE_HDR_LEN + frm_body_part2[1] >
            MAC_MAX_RPT_MBO_DETAIL_LEN) {
            *frame_body_remaining = frm_body_part2;
            meas_rpt_bcn_remaining->bit_beacon_report_id = rpt_frag->beacon_report_id;
            meas_rpt_bcn_remaining->bit_fragment_id_number = rpt_frag->fragment_id_num;
            meas_rpt_bcn_remaining->bit_more_frame_body_fragments = 1;
            break;
        }

        if (memcpy_s(rpt_detail_data, MAC_IE_HDR_LEN + frm_body_part2[1],
            frm_body_part2, MAC_IE_HDR_LEN + frm_body_part2[1]) != EOK) {
            oam_warning_log0(0, OAM_SF_RRM, "{hmac_rrm_extract_bcn_rpt_detail::memcpy failed!}");
            oal_mem_free(rpt_detail_data, OSAL_TRUE);
            meas_rpt_bcn_remaining->rpt_detail_data = NULL;
            break;
        }

        meas_rpt_bcn_remaining->rpt_detail_act_len += MAC_IE_HDR_LEN + frm_body_part2[1];
        rpt_detail_data  += MAC_IE_HDR_LEN + frm_body_part2[1];
        temp_len         -= MAC_IE_HDR_LEN + frm_body_part2[1];
        frm_body_part2   += MAC_IE_HDR_LEN + frm_body_part2[1];

        if ((temp_len < MAC_IE_HDR_LEN) || (temp_len < (MAC_IE_HDR_LEN + frm_body_part2[1]))) {
            *frame_body_remaining = NULL;
            meas_rpt_bcn_remaining->bit_beacon_report_id = rpt_frag->beacon_report_id;
            meas_rpt_bcn_remaining->bit_fragment_id_number = rpt_frag->fragment_id_num;
            meas_rpt_bcn_remaining->bit_more_frame_body_fragments = 0;
            break;
        }
    }
}

/*
 * 功能描述  : 从接收的Beacon/Probe Rsp中获取Beacon Req所需信息, Active/Passive模式使用
 */
OAL_STATIC osal_void hmac_rrm_get_bcn_frag(hmac_vap_stru *hmac_vap, mac_bss_dscr_stru *dscr,
    osal_u8 *frame_body_remaining, hmac_rrm_rpt_frag_stru* rpt_frag)
{
    osal_u8 *frame_body = dscr->mgmt_buff + MAC_80211_FRAME_LEN;
    osal_u32 payload_len = dscr->mgmt_len - MAC_80211_FRAME_LEN;
    mac_meas_rpt_bcn_stru *meas_rpt_bcn_remaining = NULL;
    mac_meas_rpt_bcn_item_stru *meas_rpt_bcn_item_remaining = NULL;
    mac_vap_rrm_info_stru *rrm_info = hmac_11k_get_vap_rrm_info(hmac_vap);

    if (rrm_info == OSAL_NULL) {
        return;
    }

    while (frame_body_remaining != NULL) {
        rpt_frag->fragment_id_num++;
        meas_rpt_bcn_remaining = (mac_meas_rpt_bcn_stru *)oal_mem_alloc(OAL_MEM_POOL_ID_LOCAL,
            sizeof(mac_meas_rpt_bcn_stru), OSAL_TRUE);
        if (meas_rpt_bcn_remaining == NULL) {
            oam_error_log0(0, OAM_SF_RRM, "{hmac_rrm_get_bcn_frag::meas_rpt_bcn_remaining NULL}");
            return;
        }
        memset_s(meas_rpt_bcn_remaining, sizeof(mac_meas_rpt_bcn_stru), 0, sizeof(mac_meas_rpt_bcn_stru));

        meas_rpt_bcn_item_remaining = (mac_meas_rpt_bcn_item_stru *)oal_mem_alloc(OAL_MEM_POOL_ID_LOCAL,
            sizeof(mac_meas_rpt_bcn_item_stru), OSAL_TRUE);
        if (meas_rpt_bcn_item_remaining == NULL) {
            oal_mem_free(meas_rpt_bcn_remaining, OSAL_TRUE);
            oam_error_log0(0, OAM_SF_RRM, "{hmac_rrm_get_bcn_frag::meas_rpt_bcn_item_remaining NULL}");
            return;
        }

        meas_rpt_bcn_remaining->meas_rpt_bcn_item = meas_rpt_bcn_item_remaining;
        memcpy_s(meas_rpt_bcn_item_remaining, sizeof(mac_meas_rpt_bcn_item_stru),
                 rpt_frag->meas_rpt_bcn->meas_rpt_bcn_item, sizeof(mac_meas_rpt_bcn_item_stru));

        rpt_frag->frame_len_remaining = (osal_u16)(payload_len - (frame_body_remaining - frame_body));

        hmac_rrm_extract_bcn_rpt_detail_remaining(hmac_vap, frame_body_remaining, &frame_body_remaining,
            meas_rpt_bcn_remaining, rpt_frag);

        osal_list_add_tail(&(meas_rpt_bcn_remaining->dlist_head), &(rrm_info->meas_rpt_list));
    }
}

OAL_STATIC osal_u8 hmac_rrm_bcn_rpt_detail_found_ie(mac_vap_rrm_info_stru *rrm_info,
    const osal_u8 *rx_frame)
{
    osal_u8 found_ie = OSAL_FALSE;
    osal_u8 element_idx;
    osal_u8 *req_elements = rrm_info->bcn_req_info.reqinfo_ieid;

    if (rrm_info->bcn_req_info.rpt_detail == 1) {
        for (element_idx = 0; element_idx < rrm_info->bcn_req_info.req_ie_num; element_idx++) {
            if ((req_elements != NULL) && (rx_frame[0] == req_elements[element_idx])) {
                found_ie = OSAL_TRUE;
                break;
            }
        }
    }

    /* MBO认证 rpt detail=2时，beacon帧内容长度超过255 需要分片 */
    if (rrm_info->bcn_req_info.rpt_detail == 2) {
        found_ie = OSAL_TRUE;
    }
    return found_ie;
}

OAL_STATIC osal_void hmac_rrm_extract_bcn_rpt_detail(hmac_vap_stru *hmac_vap,
    mac_meas_rpt_bcn_stru *meas_rpt_bcn, mac_bss_dscr_stru *dscr, osal_u8 **frm_body_part2, osal_u8 beacon_rpt_id)
{
    osal_u32 payload_len = dscr->mgmt_len - MAC_80211_FRAME_LEN;
    osal_u8 *rx_frame = OSAL_NULL;
    osal_s32 tmp_len;
    osal_u8 fix_len;
    osal_u8 *rpt_detail_data = OSAL_NULL;
    mac_vap_rrm_info_stru *rrm_info = hmac_11k_get_vap_rrm_info(hmac_vap);

    if (rrm_info == OSAL_NULL) {
        return;
    }

    rpt_detail_data = hmac_rrm_alloc_rpt_detail(rrm_info, dscr, payload_len, meas_rpt_bcn);
    if (rpt_detail_data == NULL) {
        return;
    }

    fix_len = MAC_TIME_STAMP_LEN + MAC_BEACON_INTERVAL_LEN + MAC_CAP_INFO_LEN;
    rx_frame = dscr->mgmt_buff + MAC_80211_FRAME_LEN + fix_len;
    tmp_len = (osal_s32)(payload_len - fix_len);

    /* 存在有多个相同IE的场景，要找全 */
    while (tmp_len > MAC_IE_HDR_LEN) {
        /* 沿着被搜索对象，依次核对是否有被搜索EID */
        if (hmac_rrm_bcn_rpt_detail_found_ie(rrm_info, rx_frame) == OSAL_TRUE) {
            if (meas_rpt_bcn->rpt_detail_act_len + MAC_IE_HDR_LEN + rx_frame[1] > MAC_MAX_RPT_MBO_DETAIL_LEN) {
                meas_rpt_bcn->bit_beacon_report_id = beacon_rpt_id;
                meas_rpt_bcn->bit_fragment_id_number = 0;
                meas_rpt_bcn->bit_more_frame_body_fragments = 1;
                *frm_body_part2 = rx_frame;
                break;
            }
            if (memcpy_s(rpt_detail_data, MAC_IE_HDR_LEN + rx_frame[1],
                rx_frame, MAC_IE_HDR_LEN + rx_frame[1]) != EOK) {
                oam_warning_log0(0, OAM_SF_RRM, "{hmac_rrm_extract_bcn_rpt_detail::memcpy failed!}");
                return;
            }
            rpt_detail_data += MAC_IE_HDR_LEN + rx_frame[1];
            meas_rpt_bcn->rpt_detail_act_len += MAC_IE_HDR_LEN + rx_frame[1];
        }

        tmp_len -= rx_frame[1] + MAC_IE_HDR_LEN;
        rx_frame += rx_frame[1] + MAC_IE_HDR_LEN;

        if ((tmp_len < MAC_IE_HDR_LEN) || (tmp_len < (MAC_IE_HDR_LEN + rx_frame[1]))) {
            meas_rpt_bcn->bit_beacon_report_id = beacon_rpt_id;
            meas_rpt_bcn->bit_fragment_id_number = 0;
            meas_rpt_bcn->bit_more_frame_body_fragments = 0;
            break;
        }
    }
}

/* 功能描述:为指针*meas_rpt_bcn及(*meas_rpt_bcn)->meas_rpt_bcn_item分配内存空间,并将内存空间清零 */
OAL_STATIC osal_u32 hmac_rrm_alloc_mem_for_meas_rpt_bcn(mac_meas_rpt_bcn_stru **meas_rpt_bcn)
{
    *meas_rpt_bcn = (mac_meas_rpt_bcn_stru *)oal_mem_alloc(OAL_MEM_POOL_ID_LOCAL,
        sizeof(mac_meas_rpt_bcn_stru), OSAL_TRUE);
    if (*meas_rpt_bcn == NULL) {
        return OAL_FAIL;
    }

    memset_s(*meas_rpt_bcn, sizeof(mac_meas_rpt_bcn_stru), 0, sizeof(mac_meas_rpt_bcn_stru));

    (*meas_rpt_bcn)->meas_rpt_bcn_item = (mac_meas_rpt_bcn_item_stru *)oal_mem_alloc(OAL_MEM_POOL_ID_LOCAL,
        sizeof(mac_meas_rpt_bcn_item_stru), OSAL_TRUE);
    if ((*meas_rpt_bcn)->meas_rpt_bcn_item == NULL) {
        oal_mem_free(*meas_rpt_bcn, OSAL_TRUE);
        return OAL_FAIL;
    }
    memset_s((*meas_rpt_bcn)->meas_rpt_bcn_item, sizeof(mac_meas_rpt_bcn_item_stru),
        0, sizeof(mac_meas_rpt_bcn_item_stru));
    return OAL_SUCC;
}

/*
 * 功能描述  : 判断BSSID的信息是否已经保存在漫游链表中
 */
OAL_STATIC osal_u8 hmac_rrm_bssid_saved_in_entry(mac_vap_rrm_info_stru *rrm_info, osal_u8 *bssid, osal_u8 length)
{
    struct osal_list_head *meas_rpt_entry = OSAL_NULL;
    mac_meas_rpt_bcn_stru *meas_rpt_bcn_entry = OSAL_NULL;
    osal_u32 count = 0;

    if (osal_list_empty(&(rrm_info->meas_rpt_list)) == 0) {
        osal_list_for_each(meas_rpt_entry, &(rrm_info->meas_rpt_list)) {
            count++;
            meas_rpt_bcn_entry = osal_list_entry(meas_rpt_entry, mac_meas_rpt_bcn_stru, dlist_head);
            if (meas_rpt_bcn_entry == OSAL_NULL || meas_rpt_bcn_entry->meas_rpt_bcn_item == OSAL_NULL) {
                oam_error_log2(0, OAM_SF_RRM, "{hmac_rrm_bssid_saved_in_entry count[%d] next is null[%d]}",
                    count, ((rrm_info->meas_rpt_list.next == OSAL_NULL) ? 1 : 0));
                break;
            }
            if (memcmp(meas_rpt_bcn_entry->meas_rpt_bcn_item->bssid, bssid, length) == 0) {
                return OSAL_TRUE;
            }
        }
    }
    return OSAL_FALSE;
}

OAL_STATIC osal_void hmac_rrm_get_bcn_info_from_rx(hmac_vap_stru *hmac_vap, struct osal_list_head *bss_list_head)
{
    mac_ieee80211_frame_stru *frame_hdr = OSAL_NULL;
    osal_u8 *frm_body_remaining = OSAL_NULL;
    struct osal_list_head *entry = OSAL_NULL;
    hmac_scanned_bss_info *scanned_bss_info = OSAL_NULL;
    mac_bss_dscr_stru *bss_dscr = OSAL_NULL;
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);
    mac_meas_rpt_bcn_stru *meas_rpt_bcn = OSAL_NULL;
    hmac_rrm_rpt_frag_stru rpt_frag = {0};
    mac_vap_rrm_info_stru *rrm_info = hmac_11k_get_vap_rrm_info(hmac_vap);

    if (rrm_info == OSAL_NULL) {
        return;
    }

    /*************************************************************************/
    /* Beacon Report - Frame Body */
    /* --------------------------------------------------------------------- */
    /* |oper class|chn num|Actual Meas Start Time|Meas Duration|Rpt Frm Info| */
    /* --------------------------------------------------------------------- */
    /* |1         |1      |8                     |2            | 1          | */
    /* --------------------------------------------------------------------- */
    /* --------------------------------------------------------------------- */
    /* | RCPI | RSNI | BSSID | Antenna ID | Parent TSF| Optional Subelements| */
    /* --------------------------------------------------------------------- */
    /* |1     |1     |6      |1           |4          | Var                 | */
    /* --------------------------------------------------------------------- */
    /*************************************************************************/
    /* 申请一个bss 信息 数据信息 */
    /* 遍历扫描到的bss信息 */
    osal_list_for_each(entry, bss_list_head) {
        scanned_bss_info = osal_list_entry(entry, hmac_scanned_bss_info, dlist_head);
        bss_dscr = &(scanned_bss_info->bss_dscr_info);
        frame_hdr = (mac_ieee80211_frame_stru *)bss_dscr->mgmt_buff;

        /* 根据bssid   ssid 过滤 */
        if (hmac_rrm_bcn_rpt_filter(hmac_vap, bss_dscr, &(hmac_device->scan_params)) == OAL_FAIL) {
            continue;
        }

        /* 判断该BSSID的信息是否已经保存过，若已保存过，则不再保存 更新 */
        if (hmac_rrm_bssid_saved_in_entry(rrm_info, frame_hdr->address3, WLAN_MAC_ADDR_LEN) == OSAL_TRUE) {
            continue;
        }

        if (hmac_rrm_alloc_mem_for_meas_rpt_bcn(&meas_rpt_bcn) == OAL_FAIL) {
            continue;
        }

        hmac_rrm_extract_bcn_rpt_fix_field(hmac_vap, meas_rpt_bcn->meas_rpt_bcn_item, bss_dscr);
        hmac_rrm_extract_bcn_rpt_detail(hmac_vap, meas_rpt_bcn, bss_dscr,
            &frm_body_remaining, rpt_frag.beacon_report_id);

        oam_warning_log4(0, OAM_SF_RRM, "{FindBSS %02x:%02x:%02x:%02x:xx:xx}", frame_hdr->address3[MAC_ADDR_0],
            frame_hdr->address3[MAC_ADDR_1], frame_hdr->address3[MAC_ADDR_2], frame_hdr->address3[MAC_ADDR_3]);
        osal_list_add_tail(&(meas_rpt_bcn->dlist_head), &(rrm_info->meas_rpt_list));

        /* 需要分片，申请内存存放超过的部分 */
        rpt_frag.beacon_report_id++;
        rpt_frag.meas_rpt_bcn = meas_rpt_bcn;
        hmac_rrm_get_bcn_frag(hmac_vap, bss_dscr, frm_body_remaining, &rpt_frag);
    }
}

OAL_STATIC osal_void hmac_rrm_get_bcn_info_from_save(hmac_vap_stru *hmac_vap)
{
    mac_ieee80211_frame_stru *frame_hdr = OSAL_NULL;
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);
    osal_u8 *frm_body_remaining = NULL;
    struct osal_list_head *entry = OSAL_NULL;
    hmac_scanned_bss_info *scanned_bss = OSAL_NULL;
    mac_bss_dscr_stru *dscr = OSAL_NULL;
    mac_meas_rpt_bcn_stru *meas_rpt_bcn = OSAL_NULL;
    hmac_bss_mgmt_stru *bss_mgmt = OSAL_NULL;
    hmac_rrm_rpt_frag_stru rpt_frag = {0};
    mac_vap_rrm_info_stru *rrm_info = hmac_11k_get_vap_rrm_info(hmac_vap);

    if (rrm_info == OSAL_NULL) {
        return;
    }

    /* 获取管理扫描的bss结果的结构体 */
    bss_mgmt = &(hmac_device->scan_mgmt.scan_record_mgmt.bss_mgmt);

    /*************************************************************************/
    /* Beacon Report - Frame Body */
    /* --------------------------------------------------------------------- */
    /* |oper class|chn num|Actual Meas Start Time|Meas Duration|Rpt Frm Info| */
    /* --------------------------------------------------------------------- */
    /* |1         |1      |8                     |2            | 1          | */
    /* --------------------------------------------------------------------- */
    /* --------------------------------------------------------------------- */
    /* | RCPI | RSNI | BSSID | Antenna ID | Parent TSF| Optional Subelements| */
    /* --------------------------------------------------------------------- */
    /* |1     |1     |6      |1           |4          | Var                 | */
    /* --------------------------------------------------------------------- */
    /*************************************************************************/
    /* 申请一个bss 信息 数据信息 */
    /* 对链表删操作前加锁 */
    osal_spin_lock(&(bss_mgmt->lock));
    osal_list_for_each(entry, &(bss_mgmt->bss_list_head)) {
        scanned_bss = osal_list_entry(entry, hmac_scanned_bss_info, dlist_head);
        dscr = &(scanned_bss->bss_dscr_info);
        frame_hdr = (mac_ieee80211_frame_stru *)dscr->mgmt_buff;

        /* 根据bssid   ssid 过滤 */
        if (hmac_rrm_bcn_rpt_filter(hmac_vap, dscr, &(hmac_device->scan_params)) == OAL_FAIL) {
            continue;
        }

        /* 判断该BSSID的信息是否已经保存过，若已保存过，则不再保存 更新 */
        if (hmac_rrm_bssid_saved_in_entry(rrm_info, frame_hdr->address3, WLAN_MAC_ADDR_LEN) == OSAL_TRUE) {
            continue;
        }

        if (hmac_rrm_alloc_mem_for_meas_rpt_bcn(&meas_rpt_bcn) == OAL_FAIL) {
            continue;
        }

        hmac_rrm_extract_bcn_rpt_fix_field(hmac_vap, meas_rpt_bcn->meas_rpt_bcn_item, dscr);
        hmac_rrm_extract_bcn_rpt_detail(hmac_vap, meas_rpt_bcn, dscr, &frm_body_remaining, rpt_frag.beacon_report_id);

        oam_warning_log4(0, OAM_SF_RRM, "{FindBSS %02x:%02x:%02x:%02x:xx:xx}", frame_hdr->address3[MAC_ADDR_0],
            frame_hdr->address3[MAC_ADDR_1], frame_hdr->address3[MAC_ADDR_2], frame_hdr->address3[MAC_ADDR_3]);
        osal_list_add_tail(&(meas_rpt_bcn->dlist_head), &(rrm_info->meas_rpt_list));

        /* 需要分片，申请内存存放超过的部分 */
        rpt_frag.beacon_report_id++;
        rpt_frag.meas_rpt_bcn = meas_rpt_bcn;
        hmac_rrm_get_bcn_frag(hmac_vap, dscr, frm_body_remaining, &rpt_frag);
    }
    osal_spin_unlock(&(bss_mgmt->lock));
}


OAL_STATIC osal_void hmac_rrm_fill_bcn_rpt_item(
    hmac_vap_stru *hmac_vap, mac_bcn_rpt_stru *bcn_rpt_item, mac_meas_rpt_bcn_stru *meas_rpt_bcn)
{
    mac_meas_rpt_bcn_item_stru *meas_rpt_bcn_item = meas_rpt_bcn->meas_rpt_bcn_item;

    unref_param(hmac_vap);
    bcn_rpt_item->optclass = meas_rpt_bcn_item->optclass;
    bcn_rpt_item->channum = meas_rpt_bcn_item->channum;
    memcpy_s(bcn_rpt_item->act_meas_start_time, sizeof(bcn_rpt_item->act_meas_start_time),
        meas_rpt_bcn_item->act_meas_start_time, sizeof(meas_rpt_bcn_item->act_meas_start_time));
    bcn_rpt_item->duration = meas_rpt_bcn_item->duration;
    bcn_rpt_item->condensed_phy_type = meas_rpt_bcn_item->condensed_phy_type;
    bcn_rpt_item->rpt_frm_type = meas_rpt_bcn_item->rpt_frm_type;
    bcn_rpt_item->rcpi = meas_rpt_bcn_item->rcpi;
    bcn_rpt_item->rsni = meas_rpt_bcn_item->rsni;
    memcpy_s(bcn_rpt_item->bssid, sizeof(bcn_rpt_item->bssid),
        meas_rpt_bcn_item->bssid, sizeof(meas_rpt_bcn_item->bssid));
    bcn_rpt_item->antenna_id = meas_rpt_bcn_item->antenna_id;
    bcn_rpt_item->parent_tsf = meas_rpt_bcn_item->parent_tsf;
}

/*
 * 功能描述  : 组装Measurement Report IE的固定域
 */
OAL_STATIC osal_u8 hmac_rrm_fill_bcn_rpt_ie(hmac_vap_stru *hmac_vap, mac_meas_rpt_bcn_stru *meas_rpt_bcn,
    mac_meas_rpt_ie_stru *meas_rpt_ie, mac_last_beacon_report_indication_stru **last_bcn_rpt_indication)
{
    mac_bcn_rpt_stru *bcn_rpt_item = (mac_bcn_rpt_stru *)(meas_rpt_ie->meas_rpt);
    osal_u8 curr_beacon_item_len = MAC_BEACON_RPT_FIX_LEN + (osal_u8)meas_rpt_bcn->rpt_detail_act_len;
    mac_vap_rrm_info_stru *rrm_info = hmac_11k_get_vap_rrm_info(hmac_vap);

    if (rrm_info == OSAL_NULL) {
        return curr_beacon_item_len;
    }

    hmac_rrm_fill_bcn_rpt_item(hmac_vap, bcn_rpt_item, meas_rpt_bcn);
    /* Reported Frame Body EID&Len */
    if ((meas_rpt_bcn->rpt_detail_act_len > 0) && (meas_rpt_bcn->rpt_detail_data != NULL)) {
        hmac_rrm_fill_meas_sub_elm(hmac_vap, bcn_rpt_item, meas_rpt_bcn);
        curr_beacon_item_len += 2; // 2: rpt detail data len
        if (rrm_info->bcn_req_info.rpt_frag_type != MAC_BCN_RPT_NO_FRAG) {
            hmac_rrm_fill_frame_body_fragment_id(bcn_rpt_item, meas_rpt_bcn);
            curr_beacon_item_len += 4; // 4: Reported Frame Body ie len + hdr_len
        }
        /* subElement ID: Last Beacon Report Indication(164) */
        if (rrm_info->bcn_req_info.rpt_frag_type == MAC_BCN_RPT_FRAG_WITH_LAST_INDICATION) {
            *last_bcn_rpt_indication = hmac_rrm_get_last_bcn_rpt_indication(bcn_rpt_item, meas_rpt_bcn);
            curr_beacon_item_len += 3; // 3: Last Beacon Report Indication ie len + hdr_len
        }
    }
    return curr_beacon_item_len;
}

OAL_STATIC osal_void hmac_rrm_set_meas_rpt_ie(mac_meas_rpt_ie_stru *meas_rpt_ie, mac_vap_rrm_info_stru *rrm_info)
{
    meas_rpt_ie->eid = MAC_EID_MEASREP;
    meas_rpt_ie->token = rrm_info->bcn_req_info.meas_token;
    meas_rpt_ie->rpttype = rrm_info->bcn_req_info.meas_type;
    memset_s(&(meas_rpt_ie->rptmode), sizeof(mac_meas_rpt_mode_stru), 0, sizeof(mac_meas_rpt_mode_stru));
    meas_rpt_ie->len = MAC_ACTION_RPT_FIX_LEN;
}

OAL_STATIC osal_void hmac_rrm_encap_meas_rpt(hmac_vap_stru *hmac_vap)
{
    osal_u8 find_bss_num = 0;
    osal_u8 curr_beacon_item_len;
    mac_last_beacon_report_indication_stru *last_bcn_rpt_indication = OSAL_NULL;
    struct osal_list_head *meas_rpt_node = OSAL_NULL;
    mac_meas_rpt_bcn_stru *meas_rpt_bcn = OSAL_NULL;
    mac_vap_rrm_info_stru *rrm_info = hmac_11k_get_vap_rrm_info(hmac_vap);
    mac_meas_rpt_ie_stru *meas_rpt_ie = OSAL_NULL;

    if (rrm_info == OSAL_NULL || rrm_info->meas_rpt_ie == OSAL_NULL) {
        return;
    }

    meas_rpt_ie = rrm_info->meas_rpt_ie;
    /*************************************************************************/
    /* Measurement Report IE - Frame Body */
    /* --------------------------------------------------------------------- */
    /* |Element ID |Length |Meas Token| Meas Rpt Mode | Meas Type | Meas Rpt| */
    /* --------------------------------------------------------------------- */
    /* |1          |1      | 1        |  1            | 1         | var */
    /* --------------------------------------------------------------------- */
    /*************************************************************************/
    do {
        hmac_rrm_set_meas_rpt_ie(meas_rpt_ie, rrm_info);
        rrm_info->rm_rpt_action_len += MAC_MEASUREMENT_RPT_FIX_LEN;
        if (osal_list_empty(&(rrm_info->meas_rpt_list)) != 0) {
            break;
        }

        meas_rpt_node = rrm_info->meas_rpt_list.next;
        meas_rpt_bcn = osal_list_entry(meas_rpt_node, mac_meas_rpt_bcn_stru, dlist_head);
        if (meas_rpt_bcn == OSAL_NULL || meas_rpt_bcn->meas_rpt_bcn_item == OSAL_NULL) {
            oam_warning_log0(0, OAM_SF_RRM, "hmac_rrm_encap_meas_rpt::no item or invalid meas_rpt_node");
            break;
        }

        curr_beacon_item_len = (osal_u8)(MAC_BEACON_RPT_FIX_LEN + meas_rpt_bcn->rpt_detail_act_len);
        /* netbuf 长度不足，继续遍历bss链表并删除表项 */
        if (rrm_info->rm_rpt_action_len + curr_beacon_item_len > WLAN_LARGE_NETBUF_SIZE) {
            osal_dlist_delete_entry(meas_rpt_node);
            hmac_rrm_beacon_measure_free_one_rpt_bcn_item(meas_rpt_bcn);
            rrm_info->rm_rpt_action_len -= MAC_MEASUREMENT_RPT_FIX_LEN;
            continue;
        }

        curr_beacon_item_len = hmac_rrm_fill_bcn_rpt_ie(hmac_vap, meas_rpt_bcn, meas_rpt_ie, &last_bcn_rpt_indication);
        meas_rpt_ie->len += curr_beacon_item_len;
        rrm_info->rm_rpt_action_len += curr_beacon_item_len;

        /* 下一个Measurement Report的位置 */
        osal_dlist_delete_entry(meas_rpt_node);
        /* 最后一个片段 last beacon indication置1 */
        if (osal_list_empty(&(rrm_info->meas_rpt_list)) != 0) {
            if (last_bcn_rpt_indication != OSAL_NULL) {
                last_bcn_rpt_indication->last_beacon_report_indication = 1;
            }
        }
        hmac_rrm_beacon_measure_free_one_rpt_bcn_item(meas_rpt_bcn);
        meas_rpt_ie = (mac_meas_rpt_ie_stru *)((osal_u8 *)meas_rpt_ie + meas_rpt_ie->len + MAC_IE_HDR_LEN);
        find_bss_num++;
    } while (osal_list_empty(&(rrm_info->meas_rpt_list)) == 0);

    oam_warning_log2(0, OAM_SF_RRM, "{hmac_rrm_encap_meas_rpt::bcn_req_info.uc_meas=[%d],Encap BSS_Num = [%d].",
        rrm_info->bcn_req_info.meas_token, find_bss_num);
    hmac_rrm_free_ssid_and_reqinfo_ieid_item(rrm_info);
}

/*
 * 功能描述  : 11k Beacon report 组帧回复
 */
OAL_STATIC osal_void hmac_rrm_encap_beacon_rpt(hmac_vap_stru *hmac_vap)
{
    mac_vap_rrm_info_stru *rrm_info = hmac_11k_get_vap_rrm_info(hmac_vap);

    if (rrm_info == OSAL_NULL) {
        return;
    }

    rrm_info->meas_rpt_ie = (mac_meas_rpt_ie_stru *)rrm_info->rm_rpt_action->rpt_ies;

    hmac_rrm_encap_meas_rpt(hmac_vap);

   /* 发帧 */
    hmac_rrm_send_rm_rpt_action(hmac_vap);
}

/*
 * 函 数 名  : hmac_rrm_encap_and_send_bcn_rpt
 * 功能描述  : 组Beacon report rsp Action帧，可能由多个Action帧共同完成信息传递
 */
OAL_STATIC osal_void hmac_rrm_encap_bcn_rpt(hmac_vap_stru *hmac_vap)
{
    mac_vap_rrm_info_stru *rrm_info = hmac_11k_get_vap_rrm_info(hmac_vap);

    if (rrm_info == OSAL_NULL) {
        return;
    }

    do {
        hmac_rrm_encap_beacon_rpt(hmac_vap);
    } while (osal_list_empty(&(rrm_info->meas_rpt_list)) == 0);
}

/*****************************************************************************
 功能描述  : bcn扫描回调函数
*****************************************************************************/
OAL_STATIC osal_void hmac_rrm_bcn_scan_cb(void *scan_record_ptr)
{
    hmac_scan_record_stru *scan_record;
    hmac_bss_mgmt_stru *bss_mgmt;
    osal_u32 ret;
    hmac_vap_stru *hmac_vap = OSAL_NULL;
    mac_meas_rpt_mode_stru rptmode;
    mac_vap_rrm_info_stru *rrm_info = OSAL_NULL;

    /* 判断入参合法性 */
    if (scan_record_ptr == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_RRM, "{hmac_rrm_bcn_scan_cb: input pointer is null!}");
        return;
    }

    scan_record = (hmac_scan_record_stru *)scan_record_ptr;

    /* 获取hmac VAP及判断扫描状态是否成功 */
    hmac_vap = hmac_rrm_bcn_get_vap(scan_record, &rptmode);
    if (hmac_vap == OSAL_NULL) {
        return;
    }

    if (hmac_vap_id_param_check(hmac_vap->vap_id) != OSAL_TRUE) {
        return;
    }

    rrm_info = hmac_11k_get_vap_rrm_info(hmac_vap);
    if (rrm_info == OSAL_NULL) {
        return;
    }

    /* 获取扫描结果的管理结构地址 */
    bss_mgmt = &(scan_record->bss_mgmt);

    /* 获取锁 */
    osal_spin_lock(&(bss_mgmt->lock));

    /* 遍历扫描到的bss信息 */
    hmac_rrm_get_bcn_info_from_rx(hmac_vap, &(bss_mgmt->bss_list_head));

    /* 解除锁 */
    osal_spin_unlock(&(bss_mgmt->lock));

    if (osal_list_empty(&(rrm_info->meas_rpt_list)) != 0) {
        rptmode.refused = 1;
        hmac_rrm_encap_meas_rpt_reject(hmac_vap, &rptmode);
        oam_warning_log2(0, OAM_SF_RRM, "vap_id[%d] hmac_rrm_bcn_scan_cb::scan fail, status[%d].",
            hmac_vap->vap_id, scan_record->scan_rsp_status);
        return;
    }

    /* 申请RPT管理帧内存 */
    ret = hmac_rrm_fill_basic_rm_rpt_action(hmac_vap);
    if (ret != OAL_SUCC) {
        reset_rrm_info(rrm_info);
        rptmode.refused = 1;
        hmac_rrm_encap_meas_rpt_reject(hmac_vap, &rptmode);
        oam_error_log1(0, OAM_SF_RRM, "{hmac_rrm_bcn_scan_cb: hmac_rrm_fill_basic_rm_rpt_action ret[%d].}", ret);
        return;
    }

    hmac_rrm_encap_bcn_rpt(hmac_vap);
    return;
}

/*****************************************************************************
 函 数 名  : hmac_rrm_bcn_means_mode_table
 功能描述  : table mode测量结果上报
*****************************************************************************/
OAL_STATIC osal_void hmac_rrm_bcn_means_mode_table(hmac_vap_stru *hmac_vap)
{
    mac_meas_rpt_mode_stru rptmode;
    osal_u32 ret;
    mac_vap_rrm_info_stru *rrm_info = hmac_11k_get_vap_rrm_info(hmac_vap);

    if (rrm_info == OSAL_NULL) {
        return;
    }

    (osal_void)memset_s(&rptmode, sizeof(mac_meas_rpt_mode_stru), 0, sizeof(mac_meas_rpt_mode_stru));
    hmac_rrm_get_bcn_info_from_save(hmac_vap);

    if (osal_list_empty(&(rrm_info->meas_rpt_list)) != 0) {
        rptmode.refused = 1;
        hmac_rrm_encap_meas_rpt_reject(hmac_vap, &rptmode);
        oam_warning_log1(0, OAM_SF_RRM, "vap_id[%d] hmac_rrm_bcn_means_mode_table::scan fail.", hmac_vap->vap_id);
        return;
    }

    /* 申请RPT管理帧内存 */
    oam_warning_log0(0, OAM_SF_RRM, "vap_id[%d] hmac_rrm_bcn_means_mode_table::hmac_rrm_fill_basic_rm_rpt_action.");
    ret = hmac_rrm_fill_basic_rm_rpt_action(hmac_vap);
    if (ret != OAL_SUCC) {
        reset_rrm_info(rrm_info);
        rptmode.refused = 1;
        hmac_rrm_encap_meas_rpt_reject(hmac_vap, &rptmode);
        oam_error_log1(0, OAM_SF_RRM,
            "{hmac_rrm_bcn_means_mode_table::hmac_rrm_fill_basic_rm_rpt_action ret[%d].}", ret);
        return;
    }

    hmac_rrm_encap_bcn_rpt(hmac_vap);
}

OAL_STATIC osal_void hmac_fill_scan_req_param(hmac_vap_stru *hmac_vap, mac_bcn_req_stru *bcn_req,
    mac_scan_req_stru *scan_req)
{
    mac_vap_rrm_info_stru *rrm_info = hmac_11k_get_vap_rrm_info(hmac_vap);

    if (rrm_info == OSAL_NULL) {
        return;
    }

    scan_req->bss_type = WLAN_MIB_DESIRED_BSSTYPE_INFRA;
    scan_req->scan_mode = WLAN_SCAN_MODE_BACKGROUND_STA;

    /* BSSID*过滤 */
    oal_set_mac_addr(scan_req->bssid[0], bcn_req->bssid);
    scan_req->bssid_num = 1;
    scan_req->vap_id = hmac_vap->vap_id;
    scan_req->scan_func = MAC_SCAN_FUNC_BSS;
    scan_req->max_scan_count_per_channel = 1;
    scan_req->max_send_probe_req_count_per_channel = WLAN_DEFAULT_SEND_PROBE_REQ_COUNT_PER_CHANNEL;
    scan_req->need_switch_back_home_channel = OAL_TRUE;
    scan_req->work_time_on_home_channel   = MAC_WORK_TIME_ON_HOME_CHANNEL_DEFAULT;
    oal_set_mac_addr(scan_req->sour_mac_addr, mac_mib_get_station_id(hmac_vap));
    /* SSID过滤 */
    if (rrm_info->bcn_req_info.ssid != OAL_PTR_NULL) {
        if (memcpy_s(scan_req->mac_ssid_set[0].ssid, rrm_info->bcn_req_info.ssid_len,
            rrm_info->bcn_req_info.ssid, rrm_info->bcn_req_info.ssid_len) != EOK) {
            oam_warning_log0(0, OAM_SF_CFG, "{hmac_fill_scan_req_param data::memcpy err.}");
        }
    }

    /* 回调函数指针 */
    scan_req->fn_cb = hmac_rrm_bcn_scan_cb;
}

/*****************************************************************************
 函 数 名  : hmac_rrm_bcn_scan_do
 功能描述  : bcn扫描请求
 输入参数  : void *p_scan_record
 输出参数  : 无
 返 回 值  : 无
*****************************************************************************/
OAL_STATIC osal_u32 hmac_rrm_bcn_scan_do(hmac_vap_stru *hmac_vap, mac_bcn_req_stru *bcn_req,
    mac_scan_req_stru *scan_req)
{
    osal_u32                        ret;
    errno_t                         ret_memcpy;
    mac_meas_rpt_mode_stru          rptmode;
    hmac_device_stru                *hmac_device = OSAL_NULL;
    mac_vap_rrm_info_stru *rrm_info = hmac_11k_get_vap_rrm_info(hmac_vap);
    osal_void *fhook = OSAL_NULL;

    if (rrm_info == OSAL_NULL) {
        return OAL_FAIL;
    }

    hmac_fill_scan_req_param(hmac_vap, bcn_req, scan_req);
    fhook = hmac_get_feature_fhook(HMAC_FHOOK_MBO_STA_UPDATE_RRM_SCAN_PARAM);
    if (fhook != OSAL_NULL) {
        ((hmac_vap_mbo_update_rrm_scan_params_cb)fhook)(bcn_req, scan_req);
    }

    /* 保存扫描参数，防止扫描参数被释放 */
    hmac_device = hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    if (osal_unlikely(hmac_device == OAL_PTR_NULL)) {
        return OAL_ERR_CODE_PTR_NULL;
    }
    ret_memcpy = memcpy_s((mac_scan_req_stru *)&(hmac_device->scan_params), sizeof(mac_scan_req_stru),
        (mac_scan_req_stru *)scan_req, sizeof(mac_scan_req_stru));
    if (ret_memcpy != EOK) {
        oam_error_log1(0, OAM_SF_RRM, "{hmac_rrm_bcn_scan_do: memcpy error = [%d]}", ret_memcpy);
        return OAL_FAIL;
    }

    /* 直接调用扫描模块扫描请求处理函数 */
    ret = hmac_scan_proc_scan_req_event_etc(hmac_vap, scan_req);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_RRM, "hmac_rrm_bcn_scan_do:hmac_scan_add_req failed, ret=%d", ret);
        /* 扫描失败回复refuse bit */
        memset_s(&rptmode, OAL_SIZEOF(mac_meas_rpt_mode_stru), 0, OAL_SIZEOF(mac_meas_rpt_mode_stru));
        rptmode.refused = 1;
        hmac_rrm_encap_meas_rpt_reject(hmac_vap, &rptmode);
    } else {
        rrm_info->is_measuring = OAL_TRUE;
        rrm_info->meas_status_timer.timeout = MAC_RRM_VAP_MEAS_STAUTS_TIME;
        frw_create_timer_entry(&(rrm_info->meas_status_timer), hmac_vap_meas_status_timeout,
            rrm_info->meas_status_timer.timeout, (osal_void *)rrm_info, OAL_FALSE);
    }

    return ret;
}

OAL_STATIC osal_u32 hmac_rrm_bcn_encap_meas_rpt_reject(hmac_vap_stru *hmac_vap, const mac_bcn_req_stru *bcn_req)
{
    mac_meas_rpt_mode_stru rptmode;

    /* 根据模式进行测量 */
    memset_s(&rptmode, OAL_SIZEOF(mac_meas_rpt_mode_stru), 0, OAL_SIZEOF(mac_meas_rpt_mode_stru));
    switch (bcn_req->mode) {
        /* Passive:触发扫描，不发probe req，测量收到的Beacon和probe rsp */
        case RM_BCN_REQ_MEAS_MODE_PASSIVE:
            if (mac_mib_get_dot11RMBeaconPassiveMeasurementActivated(hmac_vap) == OAL_FALSE) {
                oam_warning_log1(0, OAM_SF_RRM, "vap_id[%d] passive measurement mode not enable",
                    hmac_vap->vap_id);
                rptmode.incapable = 1;
                hmac_rrm_encap_meas_rpt_reject(hmac_vap, &rptmode);
                return OAL_FAIL;
            }
            break;

       /* Active:触发扫描，发probe req，测量收到的Beacon和probe rsp */
        case RM_BCN_REQ_MEAS_MODE_ACTIVE:
            if (mac_mib_get_dot11RMBeaconActiveMeasurementActivated(hmac_vap) == OAL_FALSE) {
                oam_warning_log1(0, OAM_SF_RRM, "vap_id[%d] active measurement mode not enable",
                    hmac_vap->vap_id);
                rptmode.incapable = 1;
                hmac_rrm_encap_meas_rpt_reject(hmac_vap, &rptmode);
                return OAL_FAIL;
            }
            break;

       /* Table:上报保存的扫描结果 */
        case RM_BCN_REQ_MEAS_MODE_TABLE:
            if ((hmac_11k_get_vap_info(hmac_vap)->bcn_table_switch == OAL_FALSE) ||
                (mac_mib_get_dot11RMBeaconTableMeasurementActivated(hmac_vap) == OAL_FALSE)) {
                rptmode.incapable = 1;
                hmac_rrm_encap_meas_rpt_reject(hmac_vap, &rptmode);
                oam_warning_log1(0, OAM_SF_RRM, "vap_id[%d] hmac_rrm_meas_bcn:table mode is shutdown!",
                    hmac_vap->vap_id);
                return OAL_FAIL;
            }
            break;

        default:
            oam_warning_log1(0, OAM_SF_RRM, "vap_id[%d] unkown measurement mode.",
                hmac_vap->vap_id);
            rptmode.refused = 1;
            hmac_rrm_encap_meas_rpt_reject(hmac_vap, &rptmode);
            return OAL_FAIL;
    }
    return OAL_SUCC;
}

/*****************************************************************************
 根据不同测量模式，准备扫描参数并启动扫描
*****************************************************************************/
OAL_STATIC osal_u32 hmac_rrm_meas_bcn(hmac_vap_stru *hmac_vap, mac_bcn_req_stru *bcn_req,
    mac_scan_req_stru *scan_req)
{
    /* 根据模式进行测量 */
    if (hmac_rrm_bcn_encap_meas_rpt_reject(hmac_vap, bcn_req) == OAL_FAIL) {
        return OAL_FAIL;
    }

    switch (bcn_req->mode) {
        /* Passive:触发扫描，不发probe req，测量收到的Beacon和probe rsp */
        case RM_BCN_REQ_MEAS_MODE_PASSIVE:
            scan_req->scan_type = WLAN_SCAN_TYPE_PASSIVE;
            /* 测量时间, 需要足够长的时间以保证可以收到指定beacon */
            scan_req->scan_time = MAC_RRM_BCN_REQ_PASSIVE_SCAN_TIME; // WLAN_DEFAULT_PASSIVE_SCAN_TIME;
            hmac_rrm_bcn_scan_do(hmac_vap, bcn_req, scan_req);
            break;

        /* Active:触发扫描，发probe req，测量收到的Beacon和probe rsp */
        case RM_BCN_REQ_MEAS_MODE_ACTIVE:
            scan_req->scan_type = WLAN_SCAN_TYPE_ACTIVE;
            /* 测量时间 */
            scan_req->scan_time = WLAN_DEFAULT_ACTIVE_SCAN_TIME;

            hmac_rrm_bcn_scan_do(hmac_vap, bcn_req, scan_req);
            break;

        /* Table:上报保存的扫描结果 */
        case RM_BCN_REQ_MEAS_MODE_TABLE:
            hmac_rrm_bcn_means_mode_table(hmac_vap);
            break;

        default:
            break;
    }
    return OAL_SUCC;
}

OAL_STATIC osal_void hmac_rrm_get_bcn_ap_rpt_channels(mac_ap_chn_rpt_stru **ap_chn_rpt, osal_u8 ap_chn_rpt_num,
    mac_scan_req_stru *scan_req, osal_u8 chan_count)
{
    osal_u8 chan_idx;
    osal_u8 chan_num;
    osal_u8 chan_avail_idx = 0;
    osal_u8 ap_chan_rpt_count;
    osal_u8 ap_chan_num;
    osal_u8 ap_chan_idx = 0;
    wlan_channel_band_enum_uint8 chan_band;

    for (ap_chan_rpt_count = 0; ap_chan_rpt_count < ap_chn_rpt_num; ap_chan_rpt_count++) {
        /* 请求的信道个数，根据长度计算 */
        ap_chan_num = ap_chn_rpt[ap_chan_rpt_count]->length - 1;

        for (chan_idx = 0; chan_idx < ap_chan_num; chan_idx++) {
            chan_num = *(&(ap_chn_rpt[ap_chan_rpt_count]->chan[0]) + chan_idx);
            chan_band = mac_get_band_by_channel_num(chan_num);
            if (hmac_get_channel_idx_from_num_etc(chan_band, chan_num, &ap_chan_idx) != OAL_SUCC) {
                continue;
            }

            /* 检查信道号是否有效 */
            if (hmac_is_channel_idx_valid_etc(chan_band, ap_chan_idx) != OAL_SUCC) {
                continue;
            }
            scan_req->channel_list[chan_avail_idx].chan_number = chan_num;
            scan_req->channel_list[chan_avail_idx].band        = chan_band;
            scan_req->channel_list[chan_avail_idx].chan_idx  = ap_chan_idx;
            chan_avail_idx++;

            /* AP chan rpt信道个数不超出管制域扫描信道个数 */
            if (chan_count == chan_avail_idx) {
                break;
            }
        }

        if (chan_count == chan_avail_idx) {
            break;
        }
    }
    scan_req->channel_nums = chan_avail_idx;
}


/*****************************************************************************
 函 数 名  : hmac_rrm_get_bcn_rpt_channels
 功能描述  : 根据channel number及operating class计算所需测量的信道
 输入参数  : hmac_vap       : dmac vap结构体指针
             ppst_ap_chn_rpt    : 指向AP Channel Report的指针数组
             ap_chn_rpt_num  : AP Channel Report个数
             scan_req       : 扫描信息结构体指针
*****************************************************************************/
OAL_STATIC osal_u32 hmac_rrm_get_bcn_rpt_channels(mac_bcn_req_stru *bcn_req, mac_ap_chn_rpt_stru **ppst_ap_chn_rpt,
    osal_u8 ap_chn_rpt_num, mac_scan_req_stru *scan_req, wlan_channel_band_enum_uint8 chan_band)
{
    osal_u8 chan_idx, chan_count;
    osal_u8 chan_num = 0;
    osal_u8 chan_avail_idx = 0;
    osal_u8 ap_chan_idx = 0;
    osal_u32 ul_ret;
    osal_void *fhook = OSAL_NULL;
    wlan_channel_band_enum_uint8 band = chan_band;

    /* 对应指定用例进行打桩 */
    chan_count = (band == WLAN_BAND_2G) ? (osal_u8)MAC_CHANNEL_FREQ_2_BUTT : (osal_u8)MAC_CHANNEL_FREQ_5_BUTT;

    fhook = hmac_get_feature_fhook(HMAC_FHOOK_MBO_STA_MODIFY_CHAN_PARAM);
    if (fhook != OSAL_NULL) {
        ((hmac_mbo_modify_chan_param_cb)fhook)(bcn_req->optclass, &band, &chan_count);
    }

    /* 当前operating class下的所有chan，先不处理regclass，使用2.4G信道总集 */
    if (bcn_req->channum == 0) {
        /* 根据channel_bitmap解析出对应的信道号集合 */
        for (chan_idx = 0; chan_idx < chan_count; chan_idx++) {
            /* 判断信道是不是在管制域内 */
            if (hmac_is_channel_idx_valid_etc(band, chan_idx) == OAL_SUCC) {
                hmac_get_channel_num_from_idx_etc(band, chan_idx, &chan_num);
                scan_req->channel_list[chan_avail_idx].chan_number  = chan_num;
                scan_req->channel_list[chan_avail_idx].band         = band;
                scan_req->channel_list[chan_avail_idx++].chan_idx   = chan_idx;
            }
        }
        scan_req->channel_nums = chan_avail_idx;
    } else if (bcn_req->channum == 255) {  /* 255 */
        /* 当前operating class与AP chan rpt的交集 */
        if (ap_chn_rpt_num == 0) {
            oam_warning_log0(0, OAM_SF_RRM, "hmac_rrm_get_bcn_rpt_channels: channum is 255,but NO ap_chan_rpt ie");
            return OAL_FAIL;
        }
        hmac_rrm_get_bcn_ap_rpt_channels(ppst_ap_chn_rpt, ap_chn_rpt_num, scan_req, chan_count);
    } else {
        /* 当前chan num */
        chan_num = bcn_req->channum;
        scan_req->channel_nums = 1;
        band = mac_get_band_by_channel_num(chan_num);
        ul_ret = hmac_get_channel_idx_from_num_etc(band, chan_num, &ap_chan_idx);
        if (ul_ret != OAL_SUCC) {
            oam_warning_log1(0, OAM_SF_RRM,
                "hmac_rrm_get_bcn_rpt_channels: hmac_get_channel_idx_from_num_etc fail,error_code=%d", ul_ret);
            return ul_ret;
        }
        /* 临时调试，需要更新管制类表格 */
        scan_req->channel_list[0].chan_number = chan_num;
        scan_req->channel_list[0].chan_idx    = ap_chan_idx;
        scan_req->channel_list[0].band        = band;
    }

    return OAL_SUCC;
}

OAL_STATIC osal_u32 hmac_rrm_parse_beacon_ap_chn(hmac_vap_stru *hmac_vap,
    mac_bcn_req_stru *bcn_req, osal_u8 c_bcn_req_sub_len, mac_scan_req_stru *scan_req, mac_meas_rpt_mode_stru rptmode)
{
    osal_u8 *chn_rpt_search_addr = bcn_req->subelm;
    osal_u8  chn_rpt_search_len;
    mac_ap_chn_rpt_stru *apst_ap_chn_rpt[MAC_11K_SUPPORT_AP_CHAN_RPT_NUM];
    osal_u8 ap_chn_rpt_count = 0;
    osal_u32 ret;

    chn_rpt_search_len = c_bcn_req_sub_len;
    do {
        apst_ap_chn_rpt[ap_chn_rpt_count] = (mac_ap_chn_rpt_stru *)mac_find_ie_etc(MAC_EID_AP_CHAN_REPORT,
            chn_rpt_search_addr, chn_rpt_search_len);
        if (apst_ap_chn_rpt[ap_chn_rpt_count] == OAL_PTR_NULL) {
            break;
        }
        chn_rpt_search_addr = (osal_u8 *)apst_ap_chn_rpt[ap_chn_rpt_count] + 2 +     /* 2 偏移 */
            apst_ap_chn_rpt[ap_chn_rpt_count]->length;
        chn_rpt_search_len = (osal_u8)(c_bcn_req_sub_len -
            (osal_u8)((osal_u8 *)apst_ap_chn_rpt[ap_chn_rpt_count] - bcn_req->subelm) -
            (2 + apst_ap_chn_rpt[ap_chn_rpt_count]->length)); /* 2 偏移 */
        ap_chn_rpt_count++;
        if (ap_chn_rpt_count == MAC_11K_SUPPORT_AP_CHAN_RPT_NUM) {
            oam_warning_log1(0, OAM_SF_RRM, "vap_id[%d] ap chan rpt num is larger than 8, truncate the later ones",
                hmac_vap->vap_id);
            break;
        }
    } while (chn_rpt_search_len > 0);

    ret = hmac_rrm_get_bcn_rpt_channels(bcn_req, apst_ap_chn_rpt,  ap_chn_rpt_count, scan_req,
        hmac_vap->channel.band);
    if (ret != OAL_SUCC) {
        rptmode.refused = 1;
        hmac_rrm_encap_meas_rpt_reject(hmac_vap, &rptmode);
        return OAL_FAIL;
    }
    return OAL_SUCC;
}

OAL_STATIC osal_u32 hmac_rrm_parse_beacon_rpt_info(hmac_vap_stru *hmac_vap, osal_u8 *rpt_info_search_addr,
    osal_u8 c_bcn_req_sub_len, mac_meas_rpt_mode_stru rptmode)
{
    mac_vap_rrm_info_stru *rrm_info = hmac_11k_get_vap_rrm_info(hmac_vap);
    osal_u8 *rpt_info_sub_element = OSAL_NULL;

    if (rrm_info == OSAL_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    rpt_info_sub_element = mac_find_ie_etc(1, rpt_info_search_addr, c_bcn_req_sub_len);
    if (rpt_info_sub_element != OAL_PTR_NULL) {
        rrm_info->bcn_req_info.rpt_condition = *(rpt_info_sub_element + 2); /* 2 偏移 */
        rrm_info->bcn_req_info.rpt_ref_val   = *(rpt_info_sub_element + 3); /* 3 偏移 */
        oam_warning_log3(0, OAM_SF_RRM,
            "vap_id[%d] hmac_rrm_parse_beacon_req::frame_body::rpt_condition[%d],rpt_ref_val[%d].",
            hmac_vap->vap_id, rrm_info->bcn_req_info.rpt_condition,
            rrm_info->bcn_req_info.rpt_ref_val);
        /* Repeat模式下使能Report Condition */
        if (rrm_info->bcn_req_info.rpt_condition != 0) {
            oam_warning_log1(0, OAM_SF_RRM, "vap_id[%d] BeaconMeasurementReportingConditions not enable",
                hmac_vap->vap_id);
            rptmode.incapable = 1;
            hmac_rrm_encap_meas_rpt_reject(hmac_vap, &rptmode);
            return OAL_FAIL;
        }
    }
    return OAL_SUCC;
}


OAL_STATIC osal_u32 hmac_rrm_parse_beacon_ssid(hmac_vap_stru *hmac_vap, osal_u8 *ssid_search_addr,
    osal_u8 c_bcn_req_sub_len)
{
    mac_vap_rrm_info_stru *rrm_info = hmac_11k_get_vap_rrm_info(hmac_vap);
    osal_u8 *ssid_sub_element = OSAL_NULL;

    if (rrm_info == OSAL_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    ssid_sub_element = mac_find_ie_etc(MAC_EID_SSID, ssid_search_addr, c_bcn_req_sub_len);
    if (ssid_sub_element != OAL_PTR_NULL) {
        /* check ssid len */
        if (ssid_sub_element[1] >= WLAN_SSID_MAX_LEN || ssid_sub_element[1] == 0) {
            oam_error_log1(0, OAM_SF_RRM, "hmac_rrm_parse_beacon_ssid::ssid_len[%d] invalid", ssid_sub_element[1]);
            return OAL_FAIL;
        }
        rrm_info->bcn_req_info.ssid_len = *(ssid_sub_element + 1);
        rrm_info->bcn_req_info.ssid = oal_mem_alloc(OAL_MEM_POOL_ID_LOCAL, rrm_info->bcn_req_info.ssid_len, OAL_TRUE);
        if (rrm_info->bcn_req_info.ssid == OAL_PTR_NULL) {
            oam_warning_log0(0, OAM_SF_RRM, "{hmac_rrm_parse_beacon_ssid::memalloc ssid fail}");
            return OAL_FAIL;
        }
        if (memcpy_s(rrm_info->bcn_req_info.ssid, rrm_info->bcn_req_info.ssid_len,
                (ssid_sub_element + 2), rrm_info->bcn_req_info.ssid_len) != EOK) { /* 偏移2 */
            oam_warning_log0(0, OAM_SF_CFG, "{hmac_rrm_parse_beacon_ssid ::memcpy err.}");
        }
    }
    return OAL_SUCC;
}

OAL_STATIC osal_void hmac_rrm_parse_beacon_report(hmac_vap_stru *hmac_vap, osal_u8 *detail_search_addr,
    osal_u8 c_bcn_req_sub_len)
{
    mac_vap_rrm_info_stru *rrm_info = hmac_11k_get_vap_rrm_info(hmac_vap);
    osal_u8 *reporting_detail = OSAL_NULL;
    osal_u8 *last_ban_rpt = OSAL_NULL;

    if (rrm_info == OSAL_NULL) {
        return;
    }

    reporting_detail = mac_find_ie_etc(2, detail_search_addr, c_bcn_req_sub_len); /* 2 偏移 */
    if (reporting_detail != OAL_PTR_NULL) {
        rrm_info->bcn_req_info.rpt_detail = *(reporting_detail + 2); /* 2 偏移 */
        oam_warning_log2(0, OAM_SF_RRM, "vap_id[%d] hmac_rrm_parse_beacon_req::frame_body::rpt_detail[%d].",
            hmac_vap->vap_id, rrm_info->bcn_req_info.rpt_detail);
    }

    last_ban_rpt = mac_find_ie_etc(164, detail_search_addr, c_bcn_req_sub_len);  // 164 : LAST_BCN
    if ((last_ban_rpt != NULL) && (*(last_ban_rpt + 1) >= 1)) {
        rrm_info->bcn_req_info.rpt_frag_type = *(last_ban_rpt + 2); /* 2 偏移 */
    }
}

OAL_STATIC osal_u32 hmac_rrm_parse_beacon_reqinfo(hmac_vap_stru *hmac_vap, osal_u8 *reqinfo_search_addr,
    osal_u8 c_bcn_req_sub_len)
{
    mac_vap_rrm_info_stru *rrm_info = hmac_11k_get_vap_rrm_info(hmac_vap);
    osal_u8 *reqinfo = OSAL_NULL;

    if (rrm_info == OSAL_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    reqinfo = mac_find_ie_etc(MAC_EID_REQINFO, reqinfo_search_addr, c_bcn_req_sub_len);
    if (reqinfo != OAL_PTR_NULL) {
        rrm_info->bcn_req_info.req_ie_num = *(reqinfo + 1);
        oam_warning_log2(0, OAM_SF_RRM, "vap_id[%d] hmac_rrm_parse_beacon_req::frame_body::req_ie_num[%d].",
            hmac_vap->vap_id, rrm_info->bcn_req_info.req_ie_num);
        rrm_info->bcn_req_info.reqinfo_ieid = oal_mem_alloc(OAL_MEM_POOL_ID_LOCAL,
            rrm_info->bcn_req_info.req_ie_num, OAL_TRUE);
        if (rrm_info->bcn_req_info.reqinfo_ieid == OAL_PTR_NULL) {
            oam_warning_log0(0, OAM_SF_RRM, "{hmac_rrm_parse_beacon_req::memalloc reqinfo_ieid fail}");
            if (rrm_info->bcn_req_info.ssid != NULL) {
                oal_mem_free(rrm_info->bcn_req_info.ssid, OSAL_TRUE);
                rrm_info->bcn_req_info.ssid = NULL;
                rrm_info->bcn_req_info.ssid_len = 0;
            }
            return OAL_FAIL;
        }
        if (memcpy_s(rrm_info->bcn_req_info.reqinfo_ieid, rrm_info->bcn_req_info.req_ie_num,
            (reqinfo + 2), rrm_info->bcn_req_info.req_ie_num) != EOK) { /* 偏移2 */
            oam_warning_log0(0, OAM_SF_CFG, "{hmac_rrm_parse_beacon_req data::memcpy err.}");
        }
    }
    return OAL_SUCC;
}

OAL_STATIC osal_void hmac_rrm_parse_beacon_request(hmac_vap_stru *hmac_vap, mac_vap_rrm_info_stru *rrm_info,
    mac_bcn_req_stru *bcn_req, mac_scan_req_stru *scan_req)
{
    osal_u32 ret;

    rrm_info->bcn_req_info.opt_class = bcn_req->optclass;
    rrm_info->bcn_req_info.meas_duration = bcn_req->duration;

    ret = hmac_rrm_meas_bcn(hmac_vap, bcn_req, scan_req);
    if (ret != OAL_SUCC) {
        hmac_rrm_free_ssid_and_reqinfo_ieid_item(rrm_info);
        oam_warning_log1(0, OAM_SF_RRM, "{hmac_rrm_parse_beacon_request::hmac_rrm_meas_bcn fail %u}", ret);
    }
}

/*****************************************************************************
 函 数 名  : hmac_rrm_parse_beacon_req
 功能描述  : 解析Beacon Request信息，并准备测量
 输入参数  : hmac_vap       : dmac vap结构体指针
             meas_req_ie    : Measurement Request IE指针
 输出参数  : 无
*****************************************************************************/
OAL_STATIC osal_void hmac_rrm_parse_beacon_req(hmac_vap_stru *hmac_vap, mac_meas_req_ie_stru *meas_req_ie)
{
    mac_bcn_req_stru               *bcn_req = OSAL_NULL;
    mac_ap_chn_rpt_stru            *apst_ap_chn_rpt[MAC_11K_SUPPORT_AP_CHAN_RPT_NUM];
    osal_u8                       ap_chn_rpt_count = 0;
    osal_s8                        c_bcn_req_sub_len;
    mac_scan_req_stru               scan_req;
    mac_vap_rrm_info_stru          *rrm_info = hmac_11k_get_vap_rrm_info(hmac_vap);
    mac_meas_rpt_mode_stru          rptmode;
    osal_u32                      ret;

    if (rrm_info == OSAL_NULL) {
        return;
    }

    memset_s(&scan_req, OAL_SIZEOF(mac_scan_req_stru), 0, OAL_SIZEOF(mac_scan_req_stru));
    bcn_req = (mac_bcn_req_stru *)&(meas_req_ie->meas_req[0]);
    if (memcpy_s(rrm_info->bcn_req_info.bssid, WLAN_MAC_ADDR_LEN, bcn_req->bssid, WLAN_MAC_ADDR_LEN) != EOK) {
        oam_warning_log0(0, OAM_SF_CFG, "{hmac_rrm_parse_beacon_req data::memcpy err.}");
    }
    memset_s(&rptmode, OAL_SIZEOF(mac_meas_rpt_mode_stru), 0, OAL_SIZEOF(mac_meas_rpt_mode_stru));

    /*************************************************************************/
    /*                    Beacon Request                                     */
    /* --------------------------------------------------------------------  */
    /* |Operating Class |Channel Number |Rand Interval| Meas Duration      | */
    /* --------------------------------------------------------------------  */
    /* |1               |1              | 2           | 2                  | */
    /* --------------------------------------------------------------------  */
    /* --------------------------------------------------------------------  */
    /* |Meas Mode       |BSSID          |Optional Subelements              | */
    /* --------------------------------------------------------------------  */
    /* |1               |6              | var                              | */
    /* --------------------------------------------------------------------  */
    /*                                                                       */
    /*************************************************************************/
    oam_warning_log4(0, OAM_SF_RRM, "{Operating Class[%d],Channel Number[%d], Rand Interval[%u],Meas Duration[%u]}",
        bcn_req->optclass, bcn_req->channum, bcn_req->random_ivl, bcn_req->duration);

    oam_warning_log4(0, OAM_SF_RRM, "{BSSID[%x:%x:%x:%x:xx:xx]}", bcn_req->bssid[0],
        bcn_req->bssid[1], bcn_req->bssid[2], bcn_req->bssid[3]); /* 1 2 3 MAC */

    /* 从Subelements中获取被测信道集合,AP Channel Report可能会有多个 */
    c_bcn_req_sub_len = (osal_s8)meas_req_ie->len - 16; /* 16 长度 */
    if (c_bcn_req_sub_len <= 0) {
        ret = hmac_rrm_get_bcn_rpt_channels(bcn_req, apst_ap_chn_rpt,  ap_chn_rpt_count, &scan_req,
            hmac_vap->channel.band);
        if (ret != OAL_SUCC) {
            rptmode.refused = 1;
            hmac_rrm_encap_meas_rpt_reject(hmac_vap, &rptmode);
            return;
        }
    } else {
        if (hmac_rrm_parse_beacon_ap_chn(hmac_vap, bcn_req, (osal_u8)c_bcn_req_sub_len,
            &scan_req, rptmode) != OAL_SUCC) {
            return;
        }

        /* 获取Beacon Reporting Information */
        if (hmac_rrm_parse_beacon_rpt_info(hmac_vap, bcn_req->subelm,
            (osal_u8)c_bcn_req_sub_len, rptmode) != OAL_SUCC) {
            return;
        }

        /* 获取SSID */
        if (hmac_rrm_parse_beacon_ssid(hmac_vap, bcn_req->subelm, (osal_u8)c_bcn_req_sub_len) != OAL_SUCC) {
            return;
        }
        /* 获取Reporting detail */
        hmac_rrm_parse_beacon_report(hmac_vap, bcn_req->subelm, (osal_u8)c_bcn_req_sub_len);
        /* 获取ReqInfo */
        if (hmac_rrm_parse_beacon_reqinfo(hmac_vap, bcn_req->subelm, (osal_u8)c_bcn_req_sub_len) != OAL_SUCC) {
            return;
        }
    }

    hmac_rrm_parse_beacon_request(hmac_vap, rrm_info, bcn_req, &scan_req);
}

OAL_STATIC osal_void hmac_rrm_handle_measure_req_ie(hmac_vap_stru* hmac_vap,
    hmac_user_stru *hmac_user, mac_meas_req_ie_stru *meas_req_ie, mac_action_rm_req_stru *rm_req)
{
    mac_meas_rpt_mode_stru rptmode = {0};
    mac_vap_rrm_info_stru *rrm_info = hmac_11k_get_vap_rrm_info(hmac_vap);
    hmac_11k_user_info_stru *hmac_11k_user_info = hmac_11k_get_user_info((osal_u8)hmac_user->assoc_id);

    if (rrm_info == OSAL_NULL || hmac_11k_user_info == OSAL_NULL) {
        return;
    }

    if (meas_req_ie->reqmode.enable == 1) {
        /* Req中拒绝某种类型的测量,一旦对端拒绝，在当前关联状态无法恢复测量能力位，需要重新关联 */
        if (meas_req_ie->reqmode.request == 0 && meas_req_ie->reqtype == RM_RADIO_MEAS_BCN) {
            oam_warning_log1(0, OAM_SF_RRM,
                "{hmac_rrm_handel_measure_reqs:: reqtype[%d] not support!}", meas_req_ie->reqtype);
            hmac_11k_user_info->rrm_enabled_cap.bcn_active_cap  = 0;
            hmac_11k_user_info->rrm_enabled_cap.bcn_passive_cap = 0;
            hmac_11k_user_info->rrm_enabled_cap.bcn_table_cap   = 0;
            hmac_11k_user_info->rrm_enabled_cap.bcn_meas_rpt_cond_cap = 0;
        }

        /* Req中不允许发送对应的report */
        if (meas_req_ie->reqmode.rpt == 0) {
            oam_warning_log0(0, OAM_SF_RRM, "{hmac_rrm_handel_measure_reqs::user not expect rpt!}");
            return;
        }
    }

    /* 并行测试暂不处理，如果对端要求则回复incapable bit */
    if (meas_req_ie->reqmode.parallel != 0) {
        rptmode.incapable = 1;
        hmac_rrm_encap_meas_rpt_reject(hmac_vap, &rptmode);
        return;
    }

    /* 处理Beacon req */
    if (meas_req_ie->reqtype == RM_RADIO_MEAS_BCN) {
        rrm_info->bcn_req_info.meas_type  = meas_req_ie->reqtype;
        rrm_info->bcn_req_info.meas_token = meas_req_ie->token;
        rrm_info->bcn_req_info.repetition = OAL_NTOH_16(rm_req->num_rpt);
        rrm_info->bcn_req_info.dialog_token = rm_req->dialog_token;
        rrm_info->bcn_req_info.rpt_frag_type = MAC_BCN_RPT_NO_FRAG;
        hmac_rrm_parse_beacon_req(hmac_vap, meas_req_ie);
    } else {
        rptmode.incapable = 1;
        hmac_rrm_encap_meas_rpt_reject(hmac_vap, &rptmode);
        oam_warning_log1(0, OAM_SF_RRM, "{Error Request,reqtype[%d]!}", meas_req_ie->reqtype);
        return;
    }
}

OAL_STATIC osal_u32 hmac_rrm_handel_measure_reqs(hmac_vap_stru* hmac_vap, hmac_user_stru *hmac_user,
    osal_u8 *netbuf, osal_u16 framebody_len)
{
    mac_vap_rrm_info_stru  *rrm_info = OSAL_NULL;
    mac_meas_req_ie_stru   *meas_req_ie = OSAL_NULL;
    mac_action_rm_req_stru *rm_req = OSAL_NULL;
    osal_u16                 framebody_len_tmp;
    osal_u16                 measure_ie_len;

    rrm_info = hmac_11k_get_vap_rrm_info(hmac_vap);
    if (rrm_info == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_RRM, "{hmac_rrm_handel_measure_reqs::rrm_info is NULL}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    rm_req = (mac_action_rm_req_stru *)(netbuf);

    /**************************************************************************/
    /*                    Measurement Request IE                              */
    /* ---------------------------------------------------------------------- */
    /* |Element ID |Length |Meas Token| Meas Req Mode|Meas Type  | Meas Req | */
    /* ---------------------------------------------------------------------- */
    /* |1          |1      | 1        | 1            |1          |var       | */
    /* ---------------------------------------------------------------------- */
    /*                                                                        */
    /**************************************************************************/
    /* 可能有多个Measurement Req IEs */
    framebody_len_tmp = framebody_len - MAC_MEASUREMENT_REQUEST_IE_OFFSET;
    while (framebody_len_tmp > MAC_IE_HDR_LEN) {
        meas_req_ie = (mac_meas_req_ie_stru *)&(netbuf[framebody_len - framebody_len_tmp]);
        rrm_info->meas_req_ie = meas_req_ie;
        measure_ie_len = meas_req_ie->len;

        if ((framebody_len_tmp >= measure_ie_len) && (measure_ie_len > 0)) {
            framebody_len_tmp = framebody_len_tmp - measure_ie_len - MAC_IE_HDR_LEN;
        } else {
            oam_warning_log2(0, OAM_SF_RRM, "{invalid ie_len=[%d] len_tmp=[%d]!}", measure_ie_len, framebody_len_tmp);
            break;
        }

        if (meas_req_ie->eid == MAC_EID_MEASREQ) {
            hmac_rrm_handle_measure_req_ie(hmac_vap, hmac_user, meas_req_ie, rm_req);
        } else {
            oam_warning_log1(0, OAM_SF_RRM, "{Error Request, but got EID[%d]!}", meas_req_ie->eid);
            return OAL_FAIL;
        }
    }
    return OAL_SUCC;
}

OAL_STATIC osal_u32 hmac_config_rrm_proc_rm_request(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    osal_u8 *netbuf, osal_u16 framebody_len)
{
    mac_action_rm_req_stru *rm_req = OSAL_NULL;
    osal_u16                 us_framebody_len;
    mac_vap_rrm_info_stru  *rrm_info = OSAL_NULL;
    mac_meas_rpt_mode_stru rptmode = {0};

    if (hmac_vap == OAL_PTR_NULL || hmac_user == OAL_PTR_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    rrm_info = hmac_11k_get_vap_rrm_info(hmac_vap);
    if (rrm_info == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_RRM, "{hmac_rrm_proc_rm_request::rrm_info is NULL}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    us_framebody_len = framebody_len;
    rm_req = (mac_action_rm_req_stru *)(netbuf);

    /**************************************************************************/
    /*                    Radio Measurement Request Frame - Frame Body        */
    /* ---------------------------------------------------------------------- */
    /* |Category |Action |Dialog Token| Number of Repetitions|Meas Req Eles | */
    /* ---------------------------------------------------------------------- */
    /* |1        |1      | 1          | 2                    |var             */
    /* ---------------------------------------------------------------------- */
    /*                                                                        */
    /**************************************************************************/
    /* 判断VAP是否正在进行测量 */
    if (rrm_info->is_measuring == OAL_TRUE) {
        oam_warning_log0(0, OAM_SF_RRM, "{hmac_rrm_proc_rm_request::vap is handling one request now.}");
        hmac_rrm_encap_meas_rpt_refuse_new_req(hmac_vap, hmac_user, rm_req);
        return OAL_FAIL;
    }

    /* 保存req */
    rrm_info->action_code  = rm_req->action_code;
    rrm_info->dialog_token = rm_req->dialog_token;
    rrm_info->req_user_id  = hmac_user->assoc_id;

    /* 是否有Meas Req */
    if (us_framebody_len <= MAC_RADIO_MEAS_ACTION_REQ_FIX_LEN) {
        /* 如果没有MR IE，也回一个不带Meas Rpt的Radio Meas Rpt */
        /* 申请管理帧内存并填充头部信息 */
        if (hmac_rrm_fill_basic_rm_rpt_action(hmac_vap) != OAL_SUCC) {
            return OAL_FAIL;
        }
        (osal_void)hmac_rrm_send_rm_rpt_action(hmac_vap);
        return OAL_FAIL;
    }

    /* 重复测试次数暂不处理，如果对端要求重复测试，则回复incapable bit */
    oam_warning_log4(0, OAM_SF_RRM,
        "hmac_rrm_proc_rm_request::framebody::Category[%d],Action[%d],Dialog Token[%d],Number of Repetitions[%d].",
        rm_req->category, rm_req->action_code, rm_req->dialog_token, rm_req->num_rpt);

    if (rm_req->num_rpt != 0) {
        rptmode.incapable = 1;
        hmac_rrm_encap_meas_rpt_reject(hmac_vap, &rptmode);
        oam_warning_log2(0, OAM_SF_RRM,
            "vap_id[%d] hmac_rrm_proc_rm_request::RepeatedMeasurements not support, num_rpt[%u].",
            hmac_vap->vap_id, rm_req->num_rpt);
        return OAL_FAIL;
    }
    return hmac_rrm_handel_measure_reqs(hmac_vap, hmac_user, netbuf, framebody_len);
}

/*****************************************************************************
 函 数 名  : hmac_config_send_radio_meas_req
 功能描述  : radio_meas_req
*****************************************************************************/
OAL_STATIC osal_s32 hmac_config_send_radio_meas_req(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    mac_chn_load_req_stru chn_load_req;
    mac_bcn_req_stru bcn_req;
    mac_rrm_req_cfg_stru req_cfg;
    mac_cfg_radio_meas_info_stru *radio_meas_cfg = (mac_cfg_radio_meas_info_stru *)msg->data;
    /* 获取用户 */
    hmac_user_stru *hmac_user = hmac_vap_get_user_by_addr_etc(hmac_vap, radio_meas_cfg->auc_mac_addr,
        WLAN_MAC_ADDR_LEN);
    if (hmac_user == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_CFG, "{hmac_config_send_radio_meas_req::mac_user is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* Neighbor Report Request */
    if (radio_meas_cfg->action_type == MAC_RM_ACTION_NEIGHBOR_REPORT_REQUEST) {
        req_cfg.rpt_notify_id    = HMAC_RRM_RPT_NOTIFY_11V;
        req_cfg.reqtype          = MAC_RRM_TYPE_NEIGHBOR_RPT;
        hmac_config_send_meas_req(hmac_vap, hmac_user, &req_cfg);
        return OAL_SUCC;
    }

    if (radio_meas_cfg->action_type != MAC_RM_ACTION_RADIO_MEASUREMENT_REQUEST) {
        return OAL_FAIL;
    }

    /* Radio Measurement Request */
    req_cfg.rpt_num = radio_meas_cfg->num_rpt;
    req_cfg.req_mode = radio_meas_cfg->req_mode;

    /* param set */
    radio_meas_cfg->random_ivl   = 0;

    /* optclass打桩 */
    radio_meas_cfg->optclass = 0;

    if (radio_meas_cfg->means_type == RM_RADIO_MEAS_CHANNEL_LOAD) {
        /*************************************************************************/
        /*                    Channel Load Request                              */
        /* --------------------------------------------------------------------- */
        /* |Operating Class |Channel Number |Rand Interval| Meas Duration       */
        /* --------------------------------------------------------------------- */
        /* |1               |1              | 2           | 2                   */
        /* --------------------------------------------------------------------- */
        /* --------------------------------------------------------------------- */
        /* |Optional Subelements                                                */
        /* --------------------------------------------------------------------- */
        /* | var                                                                */
        /* --------------------------------------------------------------------- */
        /*                                                                      */
        /*************************************************************************/
        chn_load_req.optclass      = radio_meas_cfg->optclass;
        chn_load_req.channum       = radio_meas_cfg->channum;
        chn_load_req.random_ivl    = radio_meas_cfg->random_ivl;
        chn_load_req.duration      = radio_meas_cfg->duration;

        req_cfg.rpt_notify_id    = HMAC_RRM_RPT_NOTIFY_CHN_LOAD;
        req_cfg.reqtype          = MAC_RRM_TYPE_CHANNEL_LOAD;
        req_cfg.p_arg               = (osal_void *)&chn_load_req;
        hmac_config_send_meas_req(hmac_vap, hmac_user, &req_cfg);
    } else if (radio_meas_cfg->means_type == RM_RADIO_MEAS_BCN) {
        /*************************************************************************/
        /*                    Beacon Request                                     */
        /* --------------------------------------------------------------------- */
        /* |Operating Class |Channel Number |Rand Interval| Meas Duration        */
        /* --------------------------------------------------------------------- */
        /* |1               |1              | 2           | 2                    */
        /* --------------------------------------------------------------------- */
        /* --------------------------------------------------------------------- */
        /* |Meas Mode       |BSSID          |Optional Subelements                */
        /* --------------------------------------------------------------------- */
        /* |1               |6              | var                                */
        /* --------------------------------------------------------------------- */
        /*                                                                       */
        /*************************************************************************/
        bcn_req.optclass      = radio_meas_cfg->optclass;
        bcn_req.channum       = radio_meas_cfg->channum;
        bcn_req.random_ivl    = radio_meas_cfg->random_ivl;
        bcn_req.duration      = radio_meas_cfg->duration;
        bcn_req.mode          = radio_meas_cfg->bcn_mode;
        oal_set_mac_addr(bcn_req.bssid, radio_meas_cfg->bssid);

        req_cfg.rpt_notify_id    = HMAC_RRM_RPT_NOTIFY_HILINK;
        req_cfg.reqtype          = MAC_RRM_TYPE_BCN;
        req_cfg.p_arg            = (osal_void *)&bcn_req;
        hmac_config_send_meas_req(hmac_vap, hmac_user, &req_cfg);
    } else {
        oam_warning_log1(0, OAM_SF_ANY, "{invalid means_type[%d].}", radio_meas_cfg->means_type);
        return OAL_FAIL;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 命令设置radio_measurement_request参数，模拟AP侧发送的帧
*****************************************************************************/
OAL_STATIC osal_u16 hmac_config_encap_meas_req_params(hmac_vap_stru *hmac_vap, osal_u8 *buffer, osal_u8 *param)
{
    osal_u16                       meas_req_frm_len      = 0;
    mac_meas_req_ie_stru         *meas_req_ie          = OSAL_NULL;
    mac_action_rm_req_stru       *rm_req               = OSAL_NULL;
    osal_u8                        *req_mode         = OSAL_NULL;
    osal_u8                        *reporting_detail = OSAL_NULL;
    mac_bcn_req_stru             bcn_req;
    mac_cfg_radio_meas_info_stru *radio_meas_cfg;
    errno_t                       ret;

    unref_param(hmac_vap);
    radio_meas_cfg = (mac_cfg_radio_meas_info_stru *)param;
    /* *********************************************************************** */
    /*                     Radio Measurement Request Frame - Frame Body        */
    /*  ---------------------------------------------------------------------  */
    /*  |Category |Action |Dialog Token| Number of Repetitions|Meas Req Eles | */
    /*  ---------------------------------------------------------------------  */
    /*  |1        |1      | 1          | 2                    |var             */
    /*  ---------------------------------------------------------------------  */
    /*                                                                         */
    /* *********************************************************************** */
    /* Radio Measurement Request */
    rm_req                  = (mac_action_rm_req_stru *)buffer;
    rm_req->category     = MAC_ACTION_CATEGORY_RADIO_MEASURMENT;
    rm_req->action_code  = radio_meas_cfg->action_type;
    rm_req->dialog_token = 10; /* 10表示token值 */
    rm_req->num_rpt      = radio_meas_cfg->num_rpt;
    meas_req_ie             = (mac_meas_req_ie_stru *)rm_req->req_ies;

    /* *********************************************************************** */
    /*                    Measurement Request IE                               */
    /* ---------------------------------------------------------------------   */
    /* |Element ID |Length |Meas Token| Meas Req Mode|Meas Type  | Meas Req |  */
    /* ---------------------------------------------------------------------   */
    /* |1          |1      | 1        | 1            |1          |var       |  */
    /* ---------------------------------------------------------------------   */
    /*                                                                         */
    /* *********************************************************************** */
    /* Beacon Request */
    bcn_req.optclass      = 12; // 12表示信道集
    bcn_req.channum       = radio_meas_cfg->channum;
    bcn_req.random_ivl    = 0;
    bcn_req.duration      = radio_meas_cfg->duration;
    bcn_req.mode          = radio_meas_cfg->bcn_mode;
    oal_set_mac_addr(bcn_req.bssid, radio_meas_cfg->bssid);

    meas_req_ie->eid     = MAC_EID_MEASREQ;
    meas_req_ie->token   = 50;  /* 50代表token值 */
    req_mode = (osal_u8 *)(&(meas_req_ie->reqmode));
    *req_mode = radio_meas_cfg->req_mode;
    meas_req_ie->len     = 16; /* 16表示beacon默认长度 */
    meas_req_ie->reqtype = RM_RADIO_MEAS_BCN;
    ret = memcpy_s((mac_bcn_req_stru *)meas_req_ie->meas_req, sizeof(mac_bcn_req_stru),
                   (mac_bcn_req_stru *)&bcn_req, sizeof(mac_bcn_req_stru));
    if (ret != EOK) {
        oam_error_log1(0, OAM_SF_RRM, "{hmac_config_encap_meas_req_params failed: memcpy error = [%d]}", ret);
        return OAL_FAIL;
    }

    /* optional subelement */
    reporting_detail = ((mac_bcn_req_stru *)meas_req_ie->meas_req)->subelm;
    /* 2,Reporting Detail */
    *(reporting_detail + 0) = 2;
    *(reporting_detail + 1) = 1;
    *(reporting_detail + 2) = 1; /* 2表示字段类型参数 */
    meas_req_ie->len += 3; /* 3表示可选字段长度 */

    meas_req_frm_len = meas_req_frm_len + MAC_IE_HDR_LEN + (osal_u16)(meas_req_ie->len);
    meas_req_ie = (mac_meas_req_ie_stru *)(rm_req->req_ies + meas_req_frm_len);

    meas_req_frm_len += MAC_MEASUREMENT_REQUEST_IE_OFFSET;
    return meas_req_frm_len;
}

/*****************************************************************************
 功能描述  : 命令发送radio_measurement_report帧
*****************************************************************************/
OAL_STATIC osal_s32 hmac_config_send_radio_meas_rpt(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    hmac_user_stru *hmac_user = (hmac_user_stru *)mac_res_get_hmac_user_etc(hmac_vap->assoc_vap_id);
    osal_u8         rpt_param[WLAN_MEM_NETBUF_CNT1] = {0};
    osal_u16        framebody_len;
    framebody_len = hmac_config_encap_meas_req_params(hmac_vap, (osal_u8 *)rpt_param, msg->data);
    return (osal_s32)hmac_config_rrm_proc_rm_request(hmac_vap, hmac_user, (osal_u8 *)rpt_param, framebody_len);
}

OAL_STATIC osal_void hmac_11k_set_tsf(hmac_vap_stru *hmac_vap, mac_bss_dscr_stru *bss_dscr,
    mac_scanned_result_extend_info_stru *scan_result_extend)
{
    if (hmac_vap == OSAL_NULL || bss_dscr == OSAL_NULL || scan_result_extend == OSAL_NULL) {
        return;
    }

    if (hmac_vap_id_param_check(hmac_vap->vap_id) != OSAL_TRUE) {
        return;
    }

    if (!is_legacy_sta(hmac_vap)) {
        return;
    }

    if (g_11k_vap_info[hmac_vap->vap_id] == OSAL_NULL) {
        return;
    }

    if (g_11k_vap_info[hmac_vap->vap_id]->enable_11k == OSAL_TRUE) {
        /* RSNI */
        bss_dscr->ac_rsni[0] = scan_result_extend->snr_ant0;
        bss_dscr->ac_rsni[1] = scan_result_extend->snr_ant1;
        /* phy type */
        bss_dscr->phy_type = (bss_dscr->st_channel.band == WLAN_BAND_2G) ? PHY_TYPE_DSSS : PHY_TYPE_OFDM;
        /* Parent TSF */
        bss_dscr->parent_tsf = scan_result_extend->parent_tsf;
    }

    return;
}

/*****************************************************************************
 函 数 名  : hmac_scan_update_bss_list_rrm
 功能描述  : 判断对方是否携带rrm
*****************************************************************************/
OAL_STATIC osal_void hmac_scan_update_bss_list_rrm(mac_bss_dscr_stru *bss_dscr, osal_u8 *frame_body, osal_u16 frame_len)
{
    osal_u8 *ie = OSAL_NULL;

    if (bss_dscr == OSAL_NULL || frame_body == OSAL_NULL) {
        return;
    }

    ie = mac_find_ie_etc(MAC_EID_RRM, frame_body, frame_len);
    if (ie == OAL_PTR_NULL) {
        bss_dscr->support_rrm = OAL_FALSE;
    } else {
        bss_dscr->support_rrm = OAL_TRUE;
    }

    return;
}

OAL_STATIC osal_void hmac_11k_get_tsf(hmac_vap_stru *hmac_vap,
    mac_scanned_result_extend_info_stru *scan_result_ext_info, dmac_rx_ctl_stru *rx_ctrl)
{
    if (hmac_vap == OSAL_NULL || scan_result_ext_info == OSAL_NULL || rx_ctrl == OSAL_NULL) {
        return;
    }

    if (!is_legacy_sta(hmac_vap)) {
        return;
    }

    hal_vap_tsf_get_32bit(hmac_vap->hal_vap, (osal_u32 *)&(scan_result_ext_info->parent_tsf));
    hal_phy_rx_get_snr_info(hal_chip_get_hal_device(),
        (uint8_t)rx_ctrl->rx_statistic.nss_rate.legacy_rate.protocol_mode,
        (uint16_t)rx_ctrl->rx_statistic.snr_ant0, &scan_result_ext_info->snr_ant0);
    hal_phy_rx_get_snr_info(hal_chip_get_hal_device(),
        (uint8_t)rx_ctrl->rx_statistic.nss_rate.legacy_rate.protocol_mode,
        (uint16_t)rx_ctrl->rx_statistic.snr_ant1, &scan_result_ext_info->snr_ant1);
    return;
}

/*****************************************************************************
 功能描述 : 填充RRM Enabled Cap IE
*****************************************************************************/
OAL_STATIC osal_void hmac_set_rrm_enabled_cap_field_etc(osal_void *vap, osal_u8 *buffer, osal_u8 *ie_len)
{
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)vap;
    oal_rrm_enabled_cap_ie_stru *rrm_enabled_cap_ie;

    if (vap == OSAL_NULL || buffer == OSAL_NULL || ie_len == OSAL_NULL) {
        return;
    }

    if (mac_mib_get_dot11_radio_measurement_activated(hmac_vap) == OSAL_FALSE) {
        *ie_len = 0;
        return;
    }

    buffer[0] = MAC_EID_RRM;
    buffer[1] = MAC_RRM_ENABLE_CAP_IE_LEN;

    rrm_enabled_cap_ie = (oal_rrm_enabled_cap_ie_stru *)(buffer + MAC_IE_HDR_LEN);

    memset_s(rrm_enabled_cap_ie, sizeof(oal_rrm_enabled_cap_ie_stru), 0, sizeof(oal_rrm_enabled_cap_ie_stru));

    /* 只有bit0 4 5 6位置1 */
    rrm_enabled_cap_ie->link_cap = mac_mib_get_dot11RMLinkMeasurementActivated(hmac_vap);
    rrm_enabled_cap_ie->bcn_passive_cap = mac_mib_get_dot11RMBeaconPassiveMeasurementActivated(vap);
    rrm_enabled_cap_ie->bcn_active_cap = mac_mib_get_dot11RMBeaconActiveMeasurementActivated(vap);
    rrm_enabled_cap_ie->bcn_table_cap = mac_mib_get_dot11RMBeaconTableMeasurementActivated(vap);
    rrm_enabled_cap_ie->chn_load_cap = mac_mib_get_dot11RMChannelLoadMeasurementActivated(vap);
    rrm_enabled_cap_ie->neighbor_rpt_cap = mac_mib_get_dot11RMNeighborReportActivated(vap);

#ifdef _PRE_WLAN_FEATURE_FTM
    rrm_enabled_cap_ie->ftm_range_report_cap = mac_mib_get_fine_timing_msmt_range_req_activated(hmac_vap);
#endif

    *ie_len = MAC_IE_HDR_LEN + MAC_RRM_ENABLE_CAP_IE_LEN;
}

/*****************************************************************************
 函 数 名  : hmac_sta_up_update_rrm_capability
 功能描述  : 更新ASOC关联实体中的radio measurement & neighbor信息
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
OAL_STATIC osal_u32 hmac_sta_up_update_rrm_capability(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    osal_u8 *payload, osal_u32 rx_len)
{
    osal_u8                           *ie = OSAL_NULL;
    osal_u8                           len;
    mac_cap_info_stru                   *user_cap_info;
    mac_rrm_enabled_cap_ie_stru         *rrm_enabled_cap_ie;
    hmac_11k_user_info_stru *hmac_11k_user_info = OSAL_NULL;

    if (hmac_vap == OSAL_NULL || hmac_user == OSAL_NULL || payload == OSAL_NULL) {
        return OAL_FAIL;
    }

    hmac_11k_user_info = hmac_11k_get_user_info((osal_u8)hmac_user->assoc_id);
    if (hmac_11k_user_info == OSAL_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 发送assoc req前从AP获取的能力 */
    user_cap_info = (mac_cap_info_stru *)(&(hmac_vap->assoc_user_cap_info));

    if (user_cap_info->radio_measurement == OAL_FALSE) {
        oam_warning_log1(0, OAM_SF_ASSOC,
            "vap_id[%d] {hmac_sta_up_update_rrm_capability::user not support MAC_CAP_RADIO_MEAS", hmac_vap->vap_id);
        return OAL_SUCC;
    }

    ie = mac_find_ie_etc(MAC_EID_RRM, payload, (osal_s32)rx_len);
    if (ie == OAL_PTR_NULL) {
        oam_warning_log1(0, OAM_SF_ASSOC,
            "vap_id[%d] {hmac_sta_up_update_rrm_capability::user support 11k but not fill rrm_enabled_cap_ie.}",
            hmac_vap->vap_id);
        return OAL_FAIL;
    }

    hmac_user->cap_info.enable_11k = OAL_TRUE;

    /* user rrm capability list */
    len = ie[1];
    if (len > sizeof(mac_rrm_enabled_cap_ie_stru)) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_sta_up_update_rrm_capability::ie len invalid}");
        return OAL_FAIL;
    }
    if (memcpy_s(&(hmac_11k_user_info->rrm_enabled_cap), sizeof(mac_rrm_enabled_cap_ie_stru),
        ie + MAC_IE_HDR_LEN, len) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_sta_up_update_rrm_capability::memcpy_s error}");
    }

    rrm_enabled_cap_ie = (mac_rrm_enabled_cap_ie_stru *)(ie + MAC_IE_HDR_LEN);
    oam_warning_log4(0, OAM_SF_ASSOC,
                     "{hmac_sta_up_update_rrm_capability::neighbor[%d], beacon[active:%d][passive:%d][table:%d].}",
                     rrm_enabled_cap_ie->neighbor_rpt_cap,
                     rrm_enabled_cap_ie->bcn_active_cap,
                     rrm_enabled_cap_ie->bcn_passive_cap,
                     rrm_enabled_cap_ie->bcn_table_cap);
    oam_warning_log2(0, OAM_SF_ASSOC,
                     "vap_id[%d] {hmac_sta_up_update_rrm_capability::load[%d].}", hmac_vap->vap_id,
                     rrm_enabled_cap_ie->chn_load_cap);

    hmac_11k_init_rrm_info(hmac_user);

    return OAL_SUCC;
}

OAL_STATIC osal_u32 hmac_sta_up_rx_action_radio_measurment(oal_netbuf_stru **netbuf, hmac_vap_stru *hmac_vap)
{
    osal_u8 *data = OSAL_NULL;
    hmac_user_stru *hmac_user = OSAL_NULL;
    dmac_rx_ctl_stru *rx_ctl = OSAL_NULL;
    mac_ieee80211_frame_stru *frame_hdr = OSAL_NULL;

    if (hmac_vap == OSAL_NULL || netbuf == OSAL_NULL) {
        return OAL_CONTINUE;
    }

    rx_ctl = (dmac_rx_ctl_stru *)oal_netbuf_cb(*netbuf); /* 获取帧头信息 */
    data = oal_netbuf_rx_data(*netbuf);
    frame_hdr = (mac_ieee80211_frame_stru *)mac_get_rx_cb_mac_hdr(&(rx_ctl->rx_info));

    if (!is_legacy_sta(hmac_vap)) {
        return OAL_CONTINUE;
    }

    if (hmac_vap_id_param_check(hmac_vap->vap_id) != OSAL_TRUE) {
        return OAL_CONTINUE;
    }

    if (hmac_11k_get_vap_info(hmac_vap)->enable_11k == OAL_FALSE) {
        return OAL_CONTINUE;
    }

    if (frame_hdr->frame_control.sub_type != WLAN_ACTION || frame_hdr->frame_control.type != WLAN_MANAGEMENT) {
        return OAL_CONTINUE;
    }

    if ((data[MAC_ACTION_OFFSET_ACTION] != MAC_RM_ACTION_RADIO_MEASUREMENT_REQUEST) ||
        (data[MAC_ACTION_OFFSET_CATEGORY] != MAC_ACTION_CATEGORY_RADIO_MEASURMENT)) {
        return OAL_CONTINUE;
    }

    hmac_user = mac_vap_get_hmac_user_by_addr_etc(hmac_vap, frame_hdr->address2);
    if (hmac_user == OAL_PTR_NULL) {
        return OAL_CONTINUE;
    }

    hmac_rrm_proc_rm_request(hmac_vap, hmac_user, *netbuf);

    return OAL_CONTINUE;
}

/*****************************************************************************
 函 数 名  : hmac_ap_up_update_rrm_capability
 功能描述  : 更新ASOC关联实体中的radio measurement & neighbor信息
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
OAL_STATIC osal_u32 hmac_ap_up_update_rrm_capability(hmac_user_stru *hmac_user, osal_u16 us_cap_info,
    osal_u8 *payload, osal_u32 msg_len)
{
    osal_u8                           *ie = OAL_PTR_NULL;
    osal_u8                           len;
    mac_rrm_enabled_cap_ie_stru         *rrm_enabled_cap_ie;
    hmac_11k_user_info_stru *hmac_11k_user_info = OSAL_NULL;

    if (hmac_user == OSAL_NULL || payload == OSAL_NULL) {
        return OAL_FAIL;
    }

    hmac_11k_user_info = hmac_11k_get_user_info((osal_u8)hmac_user->assoc_id);
    if (hmac_11k_user_info == OSAL_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    if ((us_cap_info & MAC_CAP_RADIO_MEAS) != MAC_CAP_RADIO_MEAS) {
        oam_warning_log2(0, OAM_SF_ASSOC,
            "vap_id[%d] {hmac_ap_up_update_rrm_capability::user not support MAC_CAP_RADIO_MEAS[%x].}",
            hmac_user->vap_id, us_cap_info);
        return OAL_SUCC;
    }

    ie = mac_find_ie_etc(MAC_EID_RRM, payload, (osal_s32)msg_len);
    if (ie == OAL_PTR_NULL) {
        oam_warning_log1(0, OAM_SF_ASSOC,
            "vap_id[%d] {hmac_ap_up_update_rrm_capability::user support 11k but not fill rrm_enabled_cap_ie.}",
            hmac_user->vap_id);
        return OAL_FAIL;
    }

    hmac_user->cap_info.enable_11k = OAL_TRUE;

    /* user rrm capability list */
    len = ie[1];
    if (memcpy_s(&(hmac_11k_user_info->rrm_enabled_cap), sizeof(mac_rrm_enabled_cap_ie_stru),
        ie + MAC_IE_HDR_LEN, len) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_ap_up_update_rrm_capability::memcpy_s error}");
    }

    rrm_enabled_cap_ie = (mac_rrm_enabled_cap_ie_stru *)(ie + MAC_IE_HDR_LEN);
    oam_warning_log4(0, OAM_SF_ASSOC,
        "{hmac_ap_up_update_rrm_capability::neighbor[%d], beacon[active:%d][passive:%d][table:%d].}",
        rrm_enabled_cap_ie->neighbor_rpt_cap,
        rrm_enabled_cap_ie->bcn_active_cap,
        rrm_enabled_cap_ie->bcn_passive_cap,
        rrm_enabled_cap_ie->bcn_table_cap);
    oam_warning_log2(0, OAM_SF_ASSOC,
        "vap_id[%d] {hmac_ap_up_update_rrm_capability::load[%d].}", hmac_user->vap_id,
        rrm_enabled_cap_ie->chn_load_cap);

    hmac_11k_init_rrm_info(hmac_user);

    return OAL_SUCC;
}

OAL_STATIC osal_void hmac_get_11k_cap(hmac_vap_stru *hmac_vap, osal_s32 *pl_value)
{
    osal_u8 vap_id;
    osal_u32 val;

    if (hmac_vap == OSAL_NULL || pl_value == OSAL_NULL) {
        return;
    }

    val = (osal_u32)(*pl_value);

    if (!is_legacy_sta(hmac_vap)) {
        return;
    }

    vap_id = hmac_vap->vap_id;
    if (hmac_vap_id_param_check(vap_id) != OSAL_TRUE) {
        return;
    }

    if (g_11k_vap_info[vap_id] != OSAL_NULL && g_11k_vap_info[vap_id]->enable_11k == OAL_TRUE) {
        val |=  BIT(WAL_WIFI_FEATURE_SUPPORT_11K);
        *pl_value = (osal_s32)val;
    }

    return;
}

hmac_netbuf_hook_stru g_11k_netbuf_hook = {
    .hooknum = HMAC_FRAME_MGMT_RX_EVENT_D2H,
    .priority = HMAC_HOOK_PRI_HIGH,
    .hook_func = hmac_sta_up_rx_action_radio_measurment,
};

osal_u32 hmac_11k_init(osal_void)
{
    osal_u32 ret;

    /* 注册监听 */
    frw_util_notifier_register(WLAN_UTIL_NOTIFIER_EVENT_ADD_VAP, hmac_11k_add_vap);
    frw_util_notifier_register(WLAN_UTIL_NOTIFIER_EVENT_DEL_VAP, hmac_11k_del_vap);
    frw_util_notifier_register(WLAN_UTIL_NOTIFIER_EVENT_DOWN_VAP, hmac_11k_down_vap);
    frw_util_notifier_register(WLAN_UTIL_NOTIFIER_EVENT_ADD_USER, hmac_11k_add_user);
    frw_util_notifier_register(WLAN_UTIL_NOTIFIER_EVENT_DEL_USER, hmac_11k_del_user);
    frw_util_notifier_register(WLAN_UTIL_NOTIFIER_EVENT_SCAN_BEGIN, hmac_rrm_get_meas_start_time);
    /* 注册消息 */
    frw_msg_hook_register(WLAN_MSG_W2H_CFG_SEND_RADIO_MEAS_REQ, hmac_config_send_radio_meas_req);
    frw_msg_hook_register(WLAN_MSG_W2H_CFG_SEND_RADIO_MEAS_RPT, hmac_config_send_radio_meas_rpt);
    /* 注册对外接口 */
    hmac_feature_hook_register(HMAC_FHOOK_11K_STA_UPDATE_RRM_CAP, hmac_sta_up_update_rrm_capability);
    hmac_feature_hook_register(HMAC_FHOOK_11K_AP_UPDATE_RRM_CAP, hmac_ap_up_update_rrm_capability);
    hmac_feature_hook_register(HMAC_FHOOK_11K_SET_RRM_CAP, hmac_set_rrm_enabled_cap_field_etc);
    hmac_feature_hook_register(HMAC_FHOOK_11K_GET_TSF, hmac_11k_get_tsf);
    hmac_feature_hook_register(HMAC_FHOOK_11K_UPDATE_BSS_LIST_RRM, hmac_scan_update_bss_list_rrm);
    hmac_feature_hook_register(HMAC_FHOOK_11K_SET_TSF, hmac_11k_set_tsf);
    hmac_feature_hook_register(HMAC_FHOOK_GET_11K_CAP, hmac_get_11k_cap);
    /* 注册转发Hook */
    ret = hmac_register_netbuf_hook(&g_11k_netbuf_hook);
    if (ret != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_RX, "{hmac_11k_init:: MGMT RX IN register_netbuf_hooks error!");
        return ret;
    }

    return OAL_SUCC;
}

osal_void hmac_11k_deinit(osal_void)
{
    /* 去注册监听 */
    frw_util_notifier_unregister(WLAN_UTIL_NOTIFIER_EVENT_ADD_VAP, hmac_11k_add_vap);
    frw_util_notifier_unregister(WLAN_UTIL_NOTIFIER_EVENT_DEL_VAP, hmac_11k_del_vap);
    frw_util_notifier_unregister(WLAN_UTIL_NOTIFIER_EVENT_DOWN_VAP, hmac_11k_down_vap);
    frw_util_notifier_unregister(WLAN_UTIL_NOTIFIER_EVENT_ADD_USER, hmac_11k_add_user);
    frw_util_notifier_unregister(WLAN_UTIL_NOTIFIER_EVENT_DEL_USER, hmac_11k_del_user);
    frw_util_notifier_unregister(WLAN_UTIL_NOTIFIER_EVENT_SCAN_BEGIN, hmac_rrm_get_meas_start_time);
    /* 去注册消息 */
    frw_msg_hook_unregister(WLAN_MSG_W2H_CFG_SEND_RADIO_MEAS_REQ);
    frw_msg_hook_unregister(WLAN_MSG_W2H_CFG_SEND_RADIO_MEAS_RPT);
    /* 去注册对外接口 */
    hmac_feature_hook_unregister(HMAC_FHOOK_11K_STA_UPDATE_RRM_CAP);
    hmac_feature_hook_unregister(HMAC_FHOOK_11K_AP_UPDATE_RRM_CAP);
    hmac_feature_hook_unregister(HMAC_FHOOK_11K_SET_RRM_CAP);
    hmac_feature_hook_unregister(HMAC_FHOOK_11K_GET_TSF);
    hmac_feature_hook_unregister(HMAC_FHOOK_11K_UPDATE_BSS_LIST_RRM);
    hmac_feature_hook_unregister(HMAC_FHOOK_11K_SET_TSF);
    hmac_feature_hook_unregister(HMAC_FHOOK_GET_11K_CAP);
    /* 去注册转发Hook */
    hmac_unregister_netbuf_hook(&g_11k_netbuf_hook);

    return;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
