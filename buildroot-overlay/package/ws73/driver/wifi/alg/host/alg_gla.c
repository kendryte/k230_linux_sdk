/*
 * Copyright (c) @CompanyNameMagicTag 2020-2022. All rights reserved.
 * Description: GLA algorithm rom.
 */

#ifdef _PRE_WLAN_FEATURE_GLA
/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "alg_gla.h"
#include "frw_util.h"
#include "hmac_alg_notify.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_ALG_GLA_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
OSAL_STATIC osal_u32 alg_gla_config_param(hmac_vap_stru *hmac_vap, frw_msg *msg);
#endif

static osal_u32 g_alg_gla_switch_enable[ALG_GLA_SWITCH_TYPE_BUTT] = {0};
/*
 * 功能描述   : device算法初始化前，同步GLA参数到device
 */
osal_u32 alg_gla_para_sync_fill(alg_param_sync_stru *data)
{
    memcpy_s(&data->gla_enable_bitmap[0], sizeof(osal_u32) * ALG_GLA_SWITCH_TYPE_BUTT,
             g_alg_gla_switch_enable, sizeof(osal_u32) * ALG_GLA_SWITCH_TYPE_BUTT);

    return OAL_SUCC;
}
/*
 * 功能描述   : 查询算法GLA开关
 */
WIFI_TCM_TEXT oal_bool_enum_uint8 alg_host_get_gla_switch_enable(alg_gla_id_enum_uint32 alg_id,
    alg_gla_switch_type_enum_uint8 gla_switch_type)
{
    if (alg_id >= ALG_GLA_ID_BUTT || gla_switch_type >= ALG_GLA_SWITCH_TYPE_BUTT) {
        oam_error_log2(0, OAM_SF_ANY, "{alg_host_get_gla_switch_enable::ERR!! alg_id[%d] or gla_switch_type[%d]}",
            alg_id, gla_switch_type);
        return OSAL_FALSE;
    }
    if ((g_alg_gla_switch_enable[gla_switch_type] & alg_id) == alg_id) {
        return OSAL_TRUE;
    } else {
        return OSAL_FALSE;
    }
}
/*
 * 功能描述   : 使能算法GLA开关
 */
osal_u32 alg_set_gal_switch_enable(alg_gla_id_enum_uint32 alg_id, alg_gla_switch_type_enum_uint8 gla_switch_type)
{
    if (alg_id >= ALG_GLA_ID_BUTT || gla_switch_type >= ALG_GLA_SWITCH_TYPE_BUTT) {
        oam_error_log2(0, OAM_SF_ANY, "{alg_set_gal_switch_enable::ERR!! alg_id[%d] or gla_switch_type[%d]}",
            alg_id, gla_switch_type);
        return OAL_FAIL;
    }
    g_alg_gla_switch_enable[gla_switch_type] |= alg_id;
    return OAL_SUCC;
}
/*
 * 功能描述   : 去使能算法GLA开关
 */
osal_u32 alg_set_gal_switch_disable(alg_gla_id_enum_uint32 alg_id, alg_gla_switch_type_enum_uint8 gla_switch_type)
{
    if (alg_id >= ALG_GLA_ID_BUTT || gla_switch_type >= ALG_GLA_SWITCH_TYPE_BUTT) {
        oam_error_log2(0, OAM_SF_ANY, "{alg_set_gal_switch_disable::ERR!! alg_id[%d] or gla_switch_type[%d]}",
            alg_id, gla_switch_type);
        return OAL_FAIL;
    }
    g_alg_gla_switch_enable[gla_switch_type] &= (~alg_id);
    return OAL_SUCC;
}
/*
 * 功能描述   : 默认使能各算法GLA常驻开关
 */
osal_void alg_hmac_gla_init_usual_switch(osal_void)
{
    alg_set_gal_switch_enable(ALG_GLA_ID_AUTORATE, ALG_GLA_USUAL_SWITCH);
    alg_set_gal_switch_enable(ALG_GLA_ID_RTS, ALG_GLA_USUAL_SWITCH);
    alg_set_gal_switch_enable(ALG_GLA_ID_AGGR, ALG_GLA_USUAL_SWITCH);
    alg_set_gal_switch_enable(ALG_GLA_ID_DBAC, ALG_GLA_USUAL_SWITCH);
    alg_set_gal_switch_enable(ALG_GLA_ID_TPC, ALG_GLA_USUAL_SWITCH);
    alg_set_gal_switch_enable(ALG_GLA_ID_TRAFFIC_CTL, ALG_GLA_USUAL_SWITCH);
    alg_set_gal_switch_enable(ALG_GLA_ID_SCHEDULE, ALG_GLA_USUAL_SWITCH);
    /* ALG_GLA_ID_INTF_DET和ALG_GLA_ID_WEAK_IMMUNE通过配置开关控制 */
    alg_set_gal_switch_enable(ALG_GLA_ID_CCA, ALG_GLA_USUAL_SWITCH);
    alg_set_gal_switch_enable(ALG_GLA_ID_EDCA, ALG_GLA_USUAL_SWITCH);
    alg_set_gal_switch_enable(ALG_GLA_ID_TEMP_PRTECT, ALG_GLA_USUAL_SWITCH);
    alg_set_gal_switch_enable(ALG_GLA_ID_TXBF, ALG_GLA_USUAL_SWITCH);
}
/*
 * 功能描述   : 默认不使能各算法GLA可选开关
 */
osal_void alg_hmac_gla_init_optional_switch(osal_void)
{
    alg_set_gal_switch_disable(ALG_GLA_ID_AUTORATE, ALG_GLA_OPTIONAL_SWITCH);
    alg_set_gal_switch_disable(ALG_GLA_ID_RTS, ALG_GLA_OPTIONAL_SWITCH);
    alg_set_gal_switch_disable(ALG_GLA_ID_AGGR, ALG_GLA_OPTIONAL_SWITCH);
    alg_set_gal_switch_disable(ALG_GLA_ID_DBAC, ALG_GLA_OPTIONAL_SWITCH);
    alg_set_gal_switch_disable(ALG_GLA_ID_TPC, ALG_GLA_OPTIONAL_SWITCH);
    alg_set_gal_switch_disable(ALG_GLA_ID_TRAFFIC_CTL, ALG_GLA_OPTIONAL_SWITCH);
    alg_set_gal_switch_disable(ALG_GLA_ID_SCHEDULE, ALG_GLA_OPTIONAL_SWITCH);
    alg_set_gal_switch_disable(ALG_GLA_ID_INTF_DET, ALG_GLA_OPTIONAL_SWITCH);
    alg_set_gal_switch_disable(ALG_GLA_ID_WEAK_IMMUNE, ALG_GLA_OPTIONAL_SWITCH);
    alg_set_gal_switch_disable(ALG_GLA_ID_CCA, ALG_GLA_OPTIONAL_SWITCH);
    alg_set_gal_switch_disable(ALG_GLA_ID_EDCA, ALG_GLA_OPTIONAL_SWITCH);
    alg_set_gal_switch_disable(ALG_GLA_ID_TEMP_PRTECT, ALG_GLA_OPTIONAL_SWITCH);
    alg_set_gal_switch_disable(ALG_GLA_ID_TXBF, ALG_GLA_OPTIONAL_SWITCH);
}
/*
 * 功能描述   : HOST GLA模块初始化
 */
osal_void alg_hmac_gla_init(osal_void)
{
    alg_hmac_gla_init_usual_switch();
    alg_hmac_gla_init_optional_switch();
    hmac_alg_register_para_sync_notify_func(ALG_PARAM_SYNC_GLA, alg_gla_para_sync_fill);
#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
    hmac_alg_register_para_cfg_notify_func(ALG_PARAM_CFG_GLA, alg_gla_config_param);
#endif
}
/*
 * 功能描述   : HOST GLA模块退出
 */
osal_void alg_hmac_gla_exit(osal_void)
{
    memset_s(g_alg_gla_switch_enable, sizeof(osal_u32) * ALG_GLA_SWITCH_TYPE_BUTT,
             0, sizeof(osal_u32) * ALG_GLA_SWITCH_TYPE_BUTT);
    hmac_alg_unregister_para_sync_notify_func(ALG_PARAM_SYNC_GLA);
#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
    hmac_alg_unregister_para_cfg_notify_func(ALG_PARAM_CFG_GLA);
#endif
}

#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
/*
 * 功能描述   : GLA配置命令入口
 */
osal_u32 alg_gla_config_param(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    mac_ioctl_alg_param_stru *alg_param = (mac_ioctl_alg_param_stru *)msg->data;

    unref_param(hmac_vap);
    if (osal_unlikely(alg_param == NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "{alg_gla_config_param:: alg_param == NULL!}");
        return OAL_FAIL;
    }
    if ((alg_param->alg_cfg <= MAC_ALG_CFG_GLA_START) || (alg_param->alg_cfg >= MAC_ALG_CFG_GLA_END)) {
        return OAL_SUCC;
    }
    switch (alg_param->alg_cfg) {
        case MAC_ALG_CFG_GLA_USUAL_SWITCH :
            oam_warning_log2(0, OAM_SF_ANY, "{alg_gla_config_param::SET GLA_USUAL_SWITCH FROM [0x%x] TO [0x%x]!}",
                g_alg_gla_switch_enable[ALG_GLA_USUAL_SWITCH], alg_param->value);
            g_alg_gla_switch_enable[ALG_GLA_USUAL_SWITCH] = alg_param->value;
            break;
        case MAC_ALG_CFG_GLA_OPTIONAL_SWITCH :
            oam_warning_log2(0, OAM_SF_ANY, "{alg_gla_config_param::SET GLA_OPTIONAL_SWITCH FROM [0x%x] TO [0x%x]!}",
                g_alg_gla_switch_enable[ALG_GLA_OPTIONAL_SWITCH], alg_param->value);
            g_alg_gla_switch_enable[ALG_GLA_OPTIONAL_SWITCH] = alg_param->value;
            break;
        case MAC_ALG_CFG_GET_GLA_USUAL_SWITCH :
            oam_warning_log1(0, OAM_SF_ANY, "{alg_gla_config_param::GET GLA_USUAL_SWITCH[0x%x]!}",
                g_alg_gla_switch_enable[ALG_GLA_USUAL_SWITCH]);
            break;
        case MAC_ALG_CFG_GET_GLA_OPTIONAL_SWITCH :
            oam_warning_log1(0, OAM_SF_ANY, "{alg_gla_config_param::GET GLA_OPTIONAL_SWITCH[0x%x]!}",
                g_alg_gla_switch_enable[ALG_GLA_OPTIONAL_SWITCH]);
            break;
        default :
            oam_error_log1(0, OAM_SF_ANY, "{alg_gla_config_param::unkown cfg type[%d]!}", alg_param->alg_cfg);
    }

    if (frw_send_msg_to_device(hmac_vap->vap_id, WLAN_MSG_H2D_C_CFG_ALG_PARAM, msg, OSAL_TRUE) != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_ANY, "{alg_gla_config_param::frw_send_msg_to_device failed!}");
        return OAL_FAIL;
    }
    return OAL_SUCC;
}
#endif
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif
