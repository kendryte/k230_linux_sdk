/*
 * Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
 * Description: hmac侧sta uapsd功能所在文件
 * Create: 2023-3-3
 */

#include "hmac_uapsd_sta.h"
#include "hmac_feature_interface.h"
#include "hmac_ccpriv.h"
#include "frw_util_notifier.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_HMAC_UAPSD_STA_C
 
#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

mac_cfg_uapsd_sta_stru *g_sta_uapsd_info[WLAN_VAP_MAX_NUM_PER_DEVICE_LIMIT] = {
    OSAL_NULL, OSAL_NULL, OSAL_NULL, OSAL_NULL
};

OSAL_STATIC mac_cfg_uapsd_sta_stru *hmac_sta_get_uapsd_info(osal_u8 vap_id)
{
    if (hmac_vap_id_param_check(vap_id) != OSAL_TRUE) {
        return OSAL_NULL;
    }

    return g_sta_uapsd_info[vap_id];
}

/*****************************************************************************
 功能描述 : 获取uapsd的配置参数max sp len
*****************************************************************************/
OSAL_STATIC osal_u8 mac_get_uapsd_config_max_sp_len_etc(osal_u8 vap_id)
{
    if (hmac_sta_get_uapsd_info(vap_id) != OSAL_NULL) {
        return g_sta_uapsd_info[vap_id]->max_sp_len;
    }

    return 0;
}

/*****************************************************************************
 功能描述 : 设置qos info字段
*****************************************************************************/
OSAL_STATIC osal_u8 mac_get_uapsd_config_ac_etc(osal_u8 vap_id, osal_u8 ac)
{
    if (hmac_sta_get_uapsd_info(vap_id) != OSAL_NULL && ac < WLAN_WME_AC_BUTT) {
        return g_sta_uapsd_info[vap_id]->trigger_enabled[ac];
    }

    return 0;
}

/*****************************************************************************
 功能描述 : 设置sta qos info字段
*****************************************************************************/
OSAL_STATIC osal_void hmac_uapsd_sta_set_qos_info(hmac_vap_stru *hmac_vap, osal_u8 *buffer)
{
    osal_u8 qos_info = 0;
    osal_u8 max_sp_bits;
    osal_u8 max_sp_length;
    osal_u8 vap_id;

    if (hmac_vap == OSAL_NULL || buffer == OSAL_NULL) {
        return;
    }

    /* QoS Information field                                          */
    /* -------------------------------------------------------------- */
    /* | B0    | B1    | B2    | B3    | B4      | B5:B6 | B7       | */
    /* -------------------------------------------------------------- */
    /* | AC_VO | AC_VI | AC_BK | AC_BE |         | Max SP|          | */
    /* | U-APSD| U-APSD| U-APSD| U-APSD| Reserved| Length| Reserved | */
    /* | Flag  | Flag  | Flag  | Flag  |         |       |          | */
    /* -------------------------------------------------------------- */

    /* Set the UAPSD configuration information in the QoS info field if the  */
    /* BSS type is Infrastructure and the AP supports UAPSD.                 */
    vap_id = hmac_vap->vap_id;

    if (hmac_vap->uapsd_cap == OSAL_TRUE) {
        max_sp_length = mac_get_uapsd_config_max_sp_len_etc(vap_id);
        qos_info |= (mac_get_uapsd_config_ac_etc(vap_id, WLAN_WME_AC_VO) << 0);
        qos_info |= (mac_get_uapsd_config_ac_etc(vap_id, WLAN_WME_AC_VI) << 1);
        qos_info |= (mac_get_uapsd_config_ac_etc(vap_id, WLAN_WME_AC_BK) << 2);    /* 左移2 BIT */
        qos_info |= (mac_get_uapsd_config_ac_etc(vap_id, WLAN_WME_AC_BE) << 3);    /* 左移3 BIT */
        if (max_sp_length <= 6) { /* 最大sp长度不超过6 */
            max_sp_bits = max_sp_length >> 1;
            qos_info |= ((max_sp_bits & 0x03) << 5); /* 左移5 bit */
        }
    }

    buffer[0] = qos_info;
}

#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
OSAL_STATIC osal_s32 hmac_config_uapsd_status_sync(osal_u8 vap_id)
{
    frw_msg msg_info = {0};
    mac_cfg_uapsd_sta_stru *uapsd_info;
    mac_cfg_uapsd_sta_stru st_sta_uapsd_cfg = {0};
    osal_s32 ret;

    uapsd_info = hmac_sta_get_uapsd_info(vap_id);
    if (uapsd_info == OSAL_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 同步dmac_vap */
    (osal_void)memcpy_s(&st_sta_uapsd_cfg, sizeof(st_sta_uapsd_cfg),
        uapsd_info, sizeof(mac_cfg_uapsd_sta_stru));

    /* 抛事件至Device侧DMAC，同步dmac_user_stru */
    frw_msg_init((osal_u8 *)&st_sta_uapsd_cfg, sizeof(st_sta_uapsd_cfg), OSAL_NULL, 0, &msg_info);
    ret = frw_send_msg_to_device(vap_id, WLAN_MSG_H2D_C_CFG_DEVICE_SET_UAPSD_PARA, &msg_info, OSAL_TRUE);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_config_uapsd_status_sync::frw_send_msg_to_device failed[%d]!}", ret);
        return ret;
    }
    return OAL_SUCC;
}

/*****************************************************************************
 功能描述 : 设置vap的uapsd参数
*****************************************************************************/
OSAL_STATIC osal_void hmac_vap_set_uapsd_para_etc(osal_u8 vap_id, mac_cfg_uapsd_sta_stru *uapsd_info)
{
    mac_cfg_uapsd_sta_stru *sta_uapsd_info = hmac_sta_get_uapsd_info(vap_id);
    osal_u8 ac;

    if (sta_uapsd_info == OSAL_NULL) {
        return;
    }

    sta_uapsd_info->max_sp_len = uapsd_info->max_sp_len;
    sta_uapsd_info->uapsd_switch = uapsd_info->uapsd_switch;
    for (ac = 0; ac < WLAN_WME_AC_BUTT; ac++) {
        sta_uapsd_info->delivery_enabled[ac] = uapsd_info->delivery_enabled[ac];
        sta_uapsd_info->trigger_enabled[ac] = uapsd_info->trigger_enabled[ac];
    }
}

/*****************************************************************************
 函 数 名  : hmac_config_set_uapsd_para_etc
 功能描述  : sta uspad 配置命令
 输入参数  : osal_void
 输出参数  : 无
*****************************************************************************/
OSAL_STATIC osal_s32  hmac_config_set_uapsd_para_etc(hmac_vap_stru *hmac_vap, mac_cfg_uapsd_sta_stru *uapsd_param)
{
    osal_s32 ret = OAL_SUCC;

    if (osal_unlikely(hmac_vap == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_PWR, "{hmac_config_set_uapsd_para_etc:: hmac_vap is null ptr}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (hmac_vap_id_param_check(hmac_vap->vap_id) != OSAL_TRUE) {
        return OAL_ERR_CODE_ARRAY_OVERFLOW;
    }

    if (!is_legacy_sta(hmac_vap)) {
        oam_warning_log2(0, OAM_SF_PWR, "{hmac_config_set_uapsd_para_etc:: vap mode is wrong :: \
            vap mode[%d] p2p mode[%d]!}", hmac_vap->vap_mode, hmac_vap->p2p_mode);
        return OAL_FAIL;
    }

    /* max_sp_len最大为6 */
    if (osal_unlikely((uapsd_param->max_sp_len > 6) && (uapsd_param->uapsd_switch == OSAL_TRUE))) {
        oam_error_log2(0, OAM_SF_PWR, "vap_id[%d] {hmac_config_set_uapsd_para_etc::max_sp_len[%d] > 6!}",
                       hmac_vap->vap_id, uapsd_param->max_sp_len);
        return OAL_FAIL;
    }

    if ((hmac_vap->vap_state == MAC_VAP_STATE_UP) || (hmac_vap->vap_state == MAC_VAP_STATE_PAUSE)
#ifdef _PRE_WLAN_FEATURE_P2P
        || (hmac_vap->vap_state == MAC_VAP_STATE_LISTEN  && hmac_vap->user_nums > 0)
#endif
#ifdef _PRE_WLAN_FEATURE_ROAM
        || (hmac_vap->vap_state == MAC_VAP_STATE_ROAMING)
#endif
    ) {
        oam_warning_log1(0, OAM_SF_PWR, "{hmac_config_set_uapsd_para_etc:: vap up do not change the uapsd para:: \
            vap state[%d]!}", hmac_vap->vap_state);
        return OAL_ERR_CODE_CONFIG_BASE;
    }

    hmac_vap_set_uapsd_para_etc(hmac_vap->vap_id, uapsd_param);

    ret = hmac_config_uapsd_status_sync(hmac_vap->vap_id);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_PWR, "{hmac_config_set_uapsd_para_etc:: set uapsd para fail ret[%d]!}", ret);
        return ret;
    }

    oam_warning_log2(0, OAM_SF_PWR, "{hmac_config_set_uapsd_para_etc:: uapsd_switch[%d] max_sp_len[%d]!}",
        uapsd_param->uapsd_switch, uapsd_param->max_sp_len);

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : uapi_ccpriv_set_uapsd_para
 功能描述  : 私有命令，UAPSD参数配置
 输入参数  : cfg_net_dev: net_device
             pc_param: 参数
 输出参数  : 无
 返 回 值  : 错误码

 修改历史      :
  1.日    期   : 2014年12月22日
    修改内容   : 新生成函数

*****************************************************************************/
OAL_STATIC osal_s32 hmac_ccpriv_set_uapsd_para(hmac_vap_stru *hmac_vap, const osal_s8 *pc_param)
{
    osal_s32 l_ret;
    osal_s32 value;
    mac_cfg_uapsd_sta_stru uapsd_param = {0};
    osal_u8 ac;

    l_ret = hmac_ccpriv_get_one_arg_digit(&pc_param, &value);
    if (l_ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
                         "{hmac_ccpriv_set_uapsd_para::get uapsd_switch return err_code [%d]!}", l_ret);
        return l_ret;
    }
    uapsd_param.uapsd_switch = (osal_u8)value;

    l_ret = hmac_ccpriv_get_one_arg_digit(&pc_param, &value);
    if (l_ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
                         "{hmac_ccpriv_set_uapsd_para::get uapsd_max_length return err_code [%d]!}", l_ret);
        return l_ret;
    }
    uapsd_param.max_sp_len = (osal_u8)value;

    for (ac = 0; ac < WLAN_WME_AC_BUTT; ac++) {
        l_ret = hmac_ccpriv_get_one_arg_digit(&pc_param, &value);
        if (l_ret != OAL_SUCC) {
            oam_warning_log1(0, OAM_SF_ANY,
                             "{hmac_ccpriv_set_uapsd_para::get ac return err_code [%d]!}", l_ret);
            return l_ret;
        }
        /* delivery enable 参数的设置 */
        uapsd_param.delivery_enabled[ac] = (osal_u8)value;
        /* trigger_enabled 参数的设置 */
        uapsd_param.trigger_enabled[ac] = (osal_u8)value;
    }

    return hmac_config_set_uapsd_para_etc(hmac_vap, &uapsd_param);
}
#endif

OSAL_STATIC osal_bool hmac_uapsd_sta_vap_add(osal_void *notify_data)
{
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)notify_data;
    osal_void *mem_ptr = OSAL_NULL;

    if (hmac_vap == OSAL_NULL) {
        return OSAL_FALSE;
    }

    if (hmac_vap_id_param_check(hmac_vap->vap_id) != OSAL_TRUE) {
        return OSAL_TRUE;
    }

    if (!is_legacy_sta(hmac_vap)) {
        return OSAL_TRUE;
    }

    if (g_sta_uapsd_info[hmac_vap->vap_id] != OSAL_NULL) {
        oam_warning_log1(0, OAM_SF_PWR, "vap_id[%d] hmac_uapsd_sta_vap_add mem already malloc!", hmac_vap->vap_id);
        return OSAL_TRUE;
    }

    mem_ptr = oal_mem_alloc(OAL_MEM_POOL_ID_LOCAL, sizeof(mac_cfg_uapsd_sta_stru), OAL_TRUE);
    if (mem_ptr == OSAL_NULL) {
        oam_error_log1(0, OAM_SF_PWR, "vap_id[%d] hmac_uapsd_sta_vap_add malloc null!", hmac_vap->vap_id);
        return OSAL_FALSE;
    }

    (osal_void)memset_s(mem_ptr, sizeof(mac_cfg_uapsd_sta_stru), 0, sizeof(mac_cfg_uapsd_sta_stru));
    g_sta_uapsd_info[hmac_vap->vap_id] = (mac_cfg_uapsd_sta_stru *)mem_ptr;

    return OSAL_TRUE;
}

/*****************************************************************************
 函 数 名  : hmac_roam_exit_etc
 功能描述  : roam模块控制信息卸载
 输入参数  : hmac_vap 需要卸载roam模块的vap
 返 回 值  : OAL_SUCC 或 失败错误码
*****************************************************************************/
OSAL_STATIC osal_bool hmac_uapsd_sta_vap_del(osal_void *notify_data)
{
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)notify_data;

    if (hmac_vap == OSAL_NULL) {
        return OSAL_FALSE;
    }

    if (hmac_vap_id_param_check(hmac_vap->vap_id) != OSAL_TRUE) {
        return OSAL_TRUE;
    }

    if (g_sta_uapsd_info[hmac_vap->vap_id] == OAL_PTR_NULL) {
        oam_warning_log1(0, OAM_SF_PWR, "vap_id[%d] {hmac_uapsd_sta_vap_del::sta_uapsd_info is NULL.}",
                         hmac_vap->vap_id);
        return OSAL_TRUE;
    }

    oal_mem_free(g_sta_uapsd_info[hmac_vap->vap_id], OAL_TRUE);
    g_sta_uapsd_info[hmac_vap->vap_id] = OAL_PTR_NULL;

    return OSAL_TRUE;
}

osal_u32 hmac_uapsd_sta_init(osal_void)
{
#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
    /* ccpriv命令注册 */
    hmac_ccpriv_register((const osal_s8 *)"set_uapsd_para", hmac_ccpriv_set_uapsd_para);
#endif

    frw_util_notifier_register(WLAN_UTIL_NOTIFIER_EVENT_ADD_VAP, hmac_uapsd_sta_vap_add);
    frw_util_notifier_register(WLAN_UTIL_NOTIFIER_EVENT_DEL_VAP, hmac_uapsd_sta_vap_del);

    hmac_feature_hook_register(HMAC_FHOOK_STA_UAPSD_SET_QOS_INFO, hmac_uapsd_sta_set_qos_info);
    hmac_feature_hook_register(HMAC_FHOOK_STA_UAPSD_GET_UAPSD_INFO, hmac_sta_get_uapsd_info);

    return OAL_SUCC;
}

osal_void hmac_uapsd_sta_deinit(osal_void)
{
#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
    /* ccpriv命令注册 */
    hmac_ccpriv_unregister((const osal_s8 *)"set_uapsd_para");
#endif

    frw_util_notifier_unregister(WLAN_UTIL_NOTIFIER_EVENT_ADD_VAP, hmac_uapsd_sta_vap_add);
    frw_util_notifier_unregister(WLAN_UTIL_NOTIFIER_EVENT_DEL_VAP, hmac_uapsd_sta_vap_del);

    hmac_feature_hook_unregister(HMAC_FHOOK_STA_UAPSD_SET_QOS_INFO);
    hmac_feature_hook_unregister(HMAC_FHOOK_STA_UAPSD_GET_UAPSD_INFO);
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif