/*
 * Copyright (c) CompanyNameMagicTag 2021-2021. All rights reserved.
 * Description: wal android cmd proc api.
 * Author: Huanghe
 * Create: 2021-07-20
 */

#include "wal_linux_android_cmd.h"


#include "wal_main.h"
#include "wal_linux_netdev.h"
#include "wal_linux_util.h"
#include "wal_linux_ioctl.h"
#include "wlan_msg.h"
#include "hmac_feature_interface.h"
#include "hmac_11k.h"
#include "hmac_sta_pm.h"
#include "hmac_11v.h"
#include "hmac_11r.h"
#ifdef _PRE_WLAN_FEATURE_DFR
#include "wal_linux_dfx.h"
#endif
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
#include "oal_netbuf.h"
#include "soc_customize_wifi.h"
#endif /* #ifdef _PRE_PLAT_FEATURE_CUSTOMIZE */
#include "plat_pm_wlan.h"
#include "plat_firmware.h"


#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_WAL_LINUX_ANDROID_CMD_C
#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif
/* _PRE_WLAN_ANDROID_CROP 开启时, 裁剪Android命令 */
#ifndef _PRE_WLAN_ANDROID_CROP
#define MAX_PRIV_CMD_SIZE          4096

#define P2P_FRAME_TYPE_OFFSET    2
#define P2P_IE_DATA_INDEX        3

/* Android private command strings */
#define CMD_SET_AP_WPS_P2P_IE   "SET_AP_WPS_P2P_IE"
#define CMD_P2P_SET_NOA         "P2P_SET_NOA"
#define CMD_P2P_SET_PS          "P2P_SET_PS"
#define CMD_SET_POWER_ON        "SET_POWER_ON"
#define CMD_SET_POWER_MGMT_ON   "SET_POWER_MGMT_ON"
#define CMD_COUNTRY             "COUNTRY"
#define CMD_GET_CAPA_DBDC       "GET_CAPAB_RSDB"
#define CMD_CAPA_DBDC_SUPP      "RSDB:1"
#define CMD_CAPA_DBDC_NOT_SUPP  "RSDB:0"

#define CMD_SET_QOS_MAP         "SET_QOS_MAP"
#define CMD_TX_POWER            "TX_POWER"
#define CMD_WPAS_GET_CUST       "WPAS_GET_CUST"
#define CMD_SET_STA_PM_ON       "SET_STA_PM_ON"
/* TAS天线切换命令 */
#define CMD_SET_MEMO_CHANGE     "SET_MEMO_CHANGE"
#ifdef _PRE_WLAN_FEATURE_M2S
#define CMD_SET_M2S_SWITCH      "SET_M2S_SWITCH"
#define CMD_SET_M2S_BLACKLIST   "SET_M2S_BLACKLIST"
#endif

#define CMD_GET_WIFI_PRIV_FEATURE_CAPABILITY   "GET_WIFI_PRIV_FEATURE_CAPABILITY"

#define CMD_SETSUSPENDOPT               "SETSUSPENDOPT"
#define CMD_SETSUSPENDMODE              "SETSUSPENDMODE"

#define CMD_RXFILTER_START  "RXFILTER-START"
#define CMD_RXFILTER_STOP   "RXFILTER-STOP"

typedef osal_s32 (*wal_android_cmd_func)(oal_net_device_stru *net_dev, osal_s8 *cmd, osal_u32 total_len, osal_u8 *data);

typedef struct {
    osal_s8 *cmd_name; /* 命令字符串 */
    wal_android_cmd_func func; /* 命令对应处理函数 */
} wal_android_cmd_entry;

/* 使用wpa_supplicant 下发命令 */
typedef struct wal_android_wifi_priv_cmd {
    osal_u8   *buf;
    osal_s32    l_used_len;
    osal_s32    l_total_len;
} wal_android_wifi_priv_cmd_stru;

/*****************************************************************************
 函 数 名  : wal_android_parse_wps_p2p_ie
 功能描述  : 提取WPS p2p信息元素
*****************************************************************************/
STATIC osal_u32 wal_android_parse_wps_p2p_ie(oal_app_ie_stru *app_ie, osal_u8 *src, osal_u32 src_len)
{
    osal_u8                      *ie             = OAL_PTR_NULL;
    osal_u32                      ie_len;
    osal_u8                      *buf_remain;
    osal_u32                      len_remain;

    if (OAL_PTR_NULL == app_ie || OAL_PTR_NULL == src) {
        oam_error_log2(0, OAM_SF_CFG, "{wal_android_parse_wps_p2p_ie::param is NULL, app_ie=[%p], src=[%p]!}",
                       (uintptr_t)app_ie, (uintptr_t)src);
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (0 == src_len || WLAN_WPS_IE_MAX_SIZE < src_len) {
        oam_warning_log1(0, OAM_SF_CFG, "{wal_android_parse_wps_p2p_ie::src_len=[%d] is invailid!}", src_len);
        return OAL_FAIL;
    }

    app_ie->ie_len    = 0;
    buf_remain           = src;
    len_remain            = src_len;

    while (len_remain > MAC_IE_HDR_LEN) {
        /* MAC_EID_WPS,MAC_EID_P2P ID均为221 */
        ie = mac_find_ie_etc(MAC_EID_P2P, buf_remain, (osal_s32)len_remain);
        if (OAL_PTR_NULL != ie) {
            ie_len = (osal_u8)ie[1] + MAC_IE_HDR_LEN;
            if ((ie_len > (WLAN_WPS_IE_MAX_SIZE - app_ie->ie_len)) ||
                (src_len < ((osal_u16)(ie - src) + ie_len))) {
                oam_warning_log3(0, OAM_SF_CFG,
                    "{wal_android_parse_wps_p2p_ie::ie_len=[%d],left buffer size=[%d],src_end_len=[%d],param invalid!}",
                    ie_len, WLAN_WPS_IE_MAX_SIZE - app_ie->ie_len, ie - src + ie_len);
                return OAL_FAIL;
            }
            if (memcpy_s(&(app_ie->ie[app_ie->ie_len]), WLAN_WPS_IE_MAX_SIZE - app_ie->ie_len,
                ie, ie_len) != EOK) {
                oam_error_log1(0, OAM_SF_CFG, "{wal_android_parse_wps_p2p_ie::memcpy_s error, len[%d]!}", ie_len);
            }
            app_ie->ie_len  += ie_len;
            buf_remain          = ie + ie_len;
            len_remain           = src_len - (osal_u32)(buf_remain - src);
        } else {
            break;
        }
    }

    if (app_ie->ie_len > 0) {
        return OAL_SUCC;
    }

    return OAL_FAIL;
}

/*****************************************************************************
 功能描述  : 设置WPS p2p信息元素 - 填写并发送消息
 被调函数  : uapi_android_set_wps_p2p_ie(oal_net_device_stru *net_dev,
                osal_u8 *buf, osal_u32 len, app_ie_type_uint8 type)
*****************************************************************************/
OAL_STATIC osal_s32 wal_android_send_wps_p2p_ie_event(oal_net_device_stru *net_dev, oal_app_ie_stru *app_ie)
{
    osal_s32 ret;
    oal_w2h_app_ie_stru w2h_wps_p2p_ie = {0};

    /* 抛事件到wal层处理 */
    w2h_wps_p2p_ie.app_ie_type = app_ie->app_ie_type;
    w2h_wps_p2p_ie.ie_len      = app_ie->ie_len;
    w2h_wps_p2p_ie.data_ie    = app_ie->ie;

    ret = wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(net_dev), WLAN_MSG_W2H_CFG_SET_WPS_P2P_IE,
        (osal_u8 *)&w2h_wps_p2p_ie, OAL_SIZEOF(oal_w2h_app_ie_stru));
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_android_send_wps_p2p_ie_event::send event failed [%d]!}", ret);
        return -OAL_EFAIL;
    }
    return OAL_SUCC;
}

osal_s32 uapi_android_set_wps_p2p_ie_inner(oal_net_device_stru *net_dev, app_ie_type_uint8 type,
    osal_u8 *buf, osal_u32 len)
{
    oal_app_ie_stru app_ie;
    osal_s32 ret;
    hmac_vap_stru *hmac_vap = OSAL_NULL;

    if (len > WLAN_WPS_IE_MAX_SIZE) {
        oam_error_log1(0, OAM_SF_CFG, "{uapi_android_set_wps_p2p_ie:: wrong len: [%u]!}", len);
        return -OAL_EFAIL;
    }

    hmac_vap = (hmac_vap_stru *)net_dev->ml_priv;
    if (hmac_vap == OSAL_NULL) {
        oam_warning_log0(0, OAM_SF_CFG, "{uapi_android_set_wps_p2p_ie::mac_vap is null}");
        return -OAL_EINVAL;
    }

    /* 经排查，wpa原生博通库beacon类型就为1，是需要添加到beacon帧中 */
    type = (type == 1) ? OAL_APP_BEACON_IE : type;

    memset_s(&app_ie, OAL_SIZEOF(oal_app_ie_stru), 0, OAL_SIZEOF(oal_app_ie_stru));
    if ((type == OAL_APP_BEACON_IE) || (type == AP_WPS_P2P_IE_CMD_BEACON) ||
        (type == OAL_APP_PROBE_RSP_IE) || (type == OAL_APP_ASSOC_RSP_IE)) {
        app_ie.app_ie_type = type;
    } else {
        oam_error_log1(0, OAM_SF_CFG, "{uapi_android_set_wps_p2p_ie:: wrong type: [%x]!}", type);
        return -OAL_EFAIL;
    }

    if (is_legacy_vap(hmac_vap) == OAL_FALSE) {
        /* GO满规格时IE长度过大，导致死机。竟分后只提取221的IE */
        if (OAL_SUCC != wal_android_parse_wps_p2p_ie(&app_ie, buf, len)) {
            oam_warning_log1(0, OAM_SF_CFG, "{uapi_android_set_wps_p2p_ie::Type=[%d], parse p2p ie fail,!}", type);
            return -OAL_EFAIL;
        }
    } else {
        if (memcpy_s(app_ie.ie, sizeof(app_ie.ie), buf, len) != EOK) {
            oam_error_log0(0, OAM_SF_ANY, "{uapi_android_set_wps_p2p_ie::memcpy_s error}");
        }
        app_ie.ie_len = len;
    }

    oam_warning_log3(0, OAM_SF_CFG,
                     "{uapi_android_set_wps_p2p_ie::p2p_ie_type=[%d], len=[%d], app_ie.ie_len=[%d]!}",
                     type, len, app_ie.ie_len);

    /***************************************************************************
        抛事件到wal层处理
    ***************************************************************************/
    ret = wal_android_send_wps_p2p_ie_event(net_dev, &app_ie);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_P2P,
                         "{uapi_android_set_wps_p2p_ie:: wal_android_send_wps_p2p_ie_event return err code %d!}", ret);
        return ret;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : uapi_android_set_wps_p2p_ie
 功能描述  : 设置WPS p2p信息元素
*****************************************************************************/
STATIC osal_s32 uapi_android_set_wps_p2p_ie(oal_net_device_stru *net_dev, osal_s8 *cmd,
    osal_u32 total_len, osal_u8 *data)
{
    osal_s32 ret;
    osal_u32 skip = osal_strlen(CMD_SET_AP_WPS_P2P_IE) + P2P_IE_DATA_INDEX;
    osal_u8 *buf = cmd + skip;
    osal_u32 len = total_len - skip;
    app_ie_type_uint8  type = *(cmd + skip - P2P_FRAME_TYPE_OFFSET) - '0';
#ifdef _PRE_WLAN_CHBA_SUPPLICANT_KERNEL
    osal_u32 cmd_set_ap_wps_p2p_ie_len = strlen(CMD_SET_AP_WPS_P2P_IE);
    osal_u32 ul_skip = cmd_set_ap_wps_p2p_ie_len + 1;
    oal_app_ie_stru *pst_wps_p2p_ie = NULL;
    osal_s32 l_ret;

    /* 外部输入参数判断，外部输入数据长度必须要满足oal_app_ie_stru结构体头部大小 */
    l_ret = wal_ioctl_judge_input_param_length(total_len, cmd_set_ap_wps_p2p_ie_len,
                                               (sizeof(oal_app_ie_stru) -
                                                (sizeof(osal_u8) * WLAN_WPS_IE_MAX_SIZE)));
    if (l_ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_android_set_wps_p2p_ie:length is too short!at least need[%d]",
                         (ul_skip + sizeof(oal_app_ie_stru) - (sizeof(osal_u8) * WLAN_WPS_IE_MAX_SIZE)));
        return -OAL_EFAIL;
    }

    pst_wps_p2p_ie = (oal_app_ie_stru *)(cmd + ul_skip);

    if ((ul_skip + pst_wps_p2p_ie->ie_len + offsetof(oal_app_ie_stru, ie)) > total_len) {
        oam_error_log1(0, OAM_SF_ANY, "{uapi_android_set_wps_p2p_ie::SET_AP_WPS_P2P_IE param len is \
            too short. need %d.}\r\n", (ul_skip + pst_wps_p2p_ie->ie_len));
        return -OAL_EFAIL;
    }
#endif

    ret = uapi_android_set_wps_p2p_ie_inner(net_dev, type, buf, len);
    return (ret > 0) ? -OAL_EFAIL : ret;
}

#ifdef _PRE_WLAN_FEATURE_P2P
/*****************************************************************************
 函 数 名  : uapi_android_set_p2p_noa
 功能描述  : 设置p2p noa节能参数
****************************************************************************/
OAL_STATIC osal_s32 uapi_android_set_p2p_noa(oal_net_device_stru *net_dev, osal_s8 *cmd,
    osal_u32 total_len, osal_u8 *data)
{
    osal_s32                       l_ret;
    osal_u32 skip = osal_strlen(CMD_P2P_SET_NOA) + 1;
    mac_cfg_p2p_noa_param_stru  p2p_noa_param;

    if ((skip + OAL_SIZEOF(p2p_noa_param)) > total_len) {
        oam_error_log1(0, OAM_SF_ANY,
                       "{uapi_android_set_p2p_noa::CMD_P2P_SET_NOA param len is too short. need %d.}",
                       skip + OAL_SIZEOF(p2p_noa_param));
        return -OAL_EFAIL;
    }
    if (memcpy_s(&p2p_noa_param, sizeof(p2p_noa_param), cmd + skip,
        OAL_SIZEOF(mac_cfg_p2p_noa_param_stru)) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{uapi_android_set_p2p_noa::memcpy_s error}");
    }

    l_ret = wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(net_dev), WLAN_MSG_W2H_CFG_SET_P2P_PS_NOA,
        (osal_u8 *)&p2p_noa_param, OAL_SIZEOF(mac_cfg_p2p_noa_param_stru));
    if (l_ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_HS20, "{uapi_android_set_qos_map:: wal_sync_post2hmac_no_rsp err %d!}", l_ret);
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : uapi_android_set_p2p_ops
 功能描述  : 设置p2p ops节能参数
****************************************************************************/
OAL_STATIC osal_s32 uapi_android_set_p2p_ops(oal_net_device_stru *net_dev, osal_s8 *cmd,
    osal_u32 total_len, osal_u8 *data)
{
    osal_s32                       l_ret;
    osal_u32 skip = osal_strlen(CMD_P2P_SET_PS) + 1;
    mac_cfg_p2p_ops_param_stru  p2p_ops_param;

    if ((skip + OAL_SIZEOF(p2p_ops_param)) > total_len) {
        oam_error_log1(0, OAM_SF_ANY, "{uapi_android_set_p2p_ops::CMD_P2P_SET_PS param len is too short.need %d.}",
                       skip + OAL_SIZEOF(p2p_ops_param));
        return -OAL_EFAIL;
    }
    if (memcpy_s(&p2p_ops_param, sizeof(p2p_ops_param), cmd + skip,
        OAL_SIZEOF(mac_cfg_p2p_ops_param_stru)) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{uapi_android_set_p2p_ops::memcpy_s error}");
    }

    l_ret = wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(net_dev), WLAN_MSG_W2H_CFG_SET_P2P_PS_OPS,
        (osal_u8 *)&p2p_ops_param, OAL_SIZEOF(mac_cfg_p2p_ops_param_stru));
    if (l_ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_HS20, "{uapi_android_set_qos_map:: wal_sync_post2hmac_no_rsp err %d!}", l_ret);
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}
#endif

#ifdef _PRE_WLAN_FEATURE_HS20
/*****************************************************************************
 函 数 名  : wal_ioctl_set_qos_map
 功能描述  : 设置QoSMap信息元素
*****************************************************************************/
OAL_STATIC osal_s32 uapi_android_set_qos_map(oal_net_device_stru *net_dev, osal_s8 *cmd,
    osal_u32 total_len, osal_u8 *data)
{
    osal_s32 ret;
    osal_u32 skip = osal_strlen(CMD_SET_QOS_MAP) + 1;
    hmac_cfg_qos_map_param_stru qos_map_param;
    if ((skip + OAL_SIZEOF(qos_map_param)) > total_len) {
        oam_error_log1(0, OAM_SF_ANY, "{uapi_android_set_qos_map::CMD_SET_QOS_MAP param len is too short.need %d.}",
                       skip + OAL_SIZEOF(qos_map_param));
        return -OAL_EFAIL;
    }
    if (memcpy_s(&qos_map_param, sizeof(qos_map_param), cmd + skip,
        OAL_SIZEOF(hmac_cfg_qos_map_param_stru)) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{uapi_android_set_qos_map::memcpy_s error}");
    }

    ret = wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(net_dev), WLAN_MSG_W2H_CFG_SET_QOS_MAP,
        (osal_u8 *)&qos_map_param, OAL_SIZEOF(hmac_cfg_qos_map_param_stru));
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_HS20, "{uapi_android_set_qos_map:: wal_sync_post2hmac_no_rsp err %d!}", ret);
        return ret;
    }
    return OAL_SUCC;
}
#endif // _PRE_WLAN_FEATURE_HS20

OAL_STATIC osal_s32 uapi_android_set_power_on(oal_net_device_stru *net_dev, osal_s8 *cmd,
    osal_u32 total_len, osal_u8 *data)
{
    osal_s32 ret;
    osal_s32 power_flag = -1;
    wal_ap_config_stru *ap_config_info = wal_get_ap_config_info();
    /* 格式:SET_POWER_ON 1 or SET_POWER_ON 0 */
    if (osal_strlen(cmd) < (osal_strlen((osal_s8 *)CMD_SET_POWER_ON) + 2)) { /* 与命令本体长度+2比较，保证有输入 */
        oam_error_log1(0, OAM_SF_ANY,
            "{uapi_android_priv_cmd_etc::CMD_SET_POWER_ON cmd len must larger than 17. Now the cmd len:%d.}",
            osal_strlen(cmd));
        return -OAL_EFAIL;
    }

    power_flag = oal_atoi(cmd + osal_strlen(CMD_SET_POWER_ON) + 1);
    oam_warning_log1(0, OAM_SF_ANY, "{uapi_android_priv_cmd_etc::CMD_SET_POWER_ON command,power flag:%d}", power_flag);
    /* ap上下电，配置VAP */
    if (power_flag == 0) { /* 下电 */
        /* 下电host device_stru去初始化 */
        wal_host_dev_exit(net_dev);

        wal_wake_lock();
        wlan_close();
        wal_wake_unlock();

        ap_config_info->l_ap_power_flag = OAL_FALSE;
    } else if (power_flag == 1) { // 上电
        ap_config_info->l_ap_power_flag = OAL_TRUE;

        wal_wake_lock();
        ret = wlan_open();
        wal_wake_unlock();
        if (ret == OAL_FAIL) {
            oam_error_log0(0, OAM_SF_ANY, "{uapi_android_priv_cmd_etc::wlan_pm_open_etc Fail!}");
            return -OAL_EFAIL;
        } else if (ret != OAL_ERR_CODE_ALREADY_OPEN) {
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
            /* 重新上电时置为FALSE */
            hwifi_config_init_force_etc();
#endif
            // 重新上电场景，下发配置VAP
            ret = wal_cfg_vap_h2d_event_etc(net_dev);
            if (ret != OAL_SUCC) {
                return -OAL_EFAIL;
            }

            /* 上电host device_stru初始化 */
            ret = wal_host_dev_init_etc(net_dev);
            if (ret != OAL_SUCC) {
                oam_warning_log1(0, OAM_SF_ANY, "wal_host_dev_init_etc FAIL %d", ret);
                return -OAL_EFAIL;
            }
        }
    } else {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_android_priv_cmd_etc::pupower_flag:%d error.}", power_flag);
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}

OAL_STATIC osal_s32 uapi_android_set_country(oal_net_device_stru *net_dev, osal_s8 *cmd,
    osal_u32 total_len, osal_u8 *data)
{
#ifdef _PRE_WLAN_FEATURE_11D
    osal_s8 *country_code;
    osal_s8        auc_country_code[3] = {0};
    osal_s32       l_ret;

    /* 格式:COUNTRY CN */
    if (osal_strlen(cmd) < (osal_strlen((osal_s8 *)CMD_COUNTRY) + 3)) { /* 与命令本体长度+3比较，保证有输入 */
        oam_warning_log0(0, OAM_SF_ANY, "{uapi_android_priv_cmd_etc::command len error.}");

        return -OAL_EFAIL;
    }

    country_code = cmd + osal_strlen((osal_s8 *)CMD_COUNTRY) + 1;
    if (memcpy_s(auc_country_code, sizeof(auc_country_code), country_code, 2) != EOK) { /* 国家码固定2个字符 */
        oam_error_log0(0, OAM_SF_ANY, "{uapi_android_set_country::memcpy_s error}");
    }

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    if (OAL_TRUE == g_st_cust_country_code_ignore_flag.country_code_ingore_ini_flag) {
        oam_warning_log3(0, OAM_SF_ANY, "{uapi_android_priv_cmd_etc::wlan_pm_set_country[%c %c] is ignore, ini[%d]",
            auc_country_code[0], auc_country_code[1],
            g_st_cust_country_code_ignore_flag.country_code_ingore_ini_flag);

        return OAL_SUCC;
    }
#endif

    l_ret = wal_util_set_country_code(net_dev, (osal_s8 *)auc_country_code);
    if (OAL_UNLIKELY(OAL_SUCC != l_ret)) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_android_priv_cmd_etc::return err code [%d]!}", l_ret);

        return -OAL_EFAIL;
    }
#else
    oam_warning_log0(0, OAM_SF_ANY, "{uapi_android_priv_cmd_etc::_PRE_WLAN_FEATURE_11D is not define!}");
#endif
    return OAL_SUCC;
}
/*****************************************************************************
 函 数 名  : uapi_android_reduce_sar
 功能描述  : 设置RTS 门限值
*****************************************************************************/
OAL_STATIC osal_s32 uapi_android_reduce_sar(oal_net_device_stru *net_dev, osal_s8 *cmd,
    osal_u32 total_len, osal_u8 *data)
{
    osal_u8                   lvl_idx = 0;
    osal_s32                   l_ret;
    osal_u32  skip = osal_strlen((osal_s8 *)CMD_TX_POWER) + 1;
    osal_u16  tx_power = (osal_u16)oal_atoi(cmd + skip);

    oam_warning_log1(0, OAM_SF_TPC, "uapi_android_reduce_sar::supplicant set tx_power[%d] for reduce SAR purpose.",
                     tx_power);
    if ((tx_power >= 1001) && (tx_power <= 1003)) { /* 取值范围在1001到1003 */
        lvl_idx = tx_power & 0x3;
    }

    /* vap未创建时，不处理supplicant命令 */
    if (OAL_PTR_NULL == net_dev->ml_priv) {
        oam_warning_log0(0, OAM_SF_CFG, "uapi_android_reduce_sar::vap not created yet, ignore the cmd!");
        return -OAL_EINVAL;
    }

    l_ret = wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(net_dev), WLAN_MSG_W2H_CFG_REDUCE_SAR,
        (osal_u8 *)&lvl_idx, OAL_SIZEOF(lvl_idx));
    if (OAL_UNLIKELY(OAL_SUCC != l_ret)) {
        oam_error_log1(0, OAM_SF_ANY, "uapi_android_reduce_sar::wal_sync_send2device_no_rsp error[%d]!", l_ret);
    }
    /* 驱动打印错误码，返回成功，防止supplicant 累计4次 ioctl失败导致wifi异常重启 */
    return OAL_SUCC;
}

OAL_STATIC osal_s32 uapi_android_set_memo_change(oal_net_device_stru *net_dev, osal_s8 *cmd,
    osal_u32 total_len, osal_u8 *data)
{
    osal_u8 memo_change_tag = 0;
    /* 0:默认态 1:tas态 */
    memo_change_tag = (osal_u8)oal_atoi(cmd + osal_strlen((osal_s8 *)CMD_SET_MEMO_CHANGE) + 1);
    oam_warning_log1(0, OAM_SF_M2S, "{uapi_android_set_memo_change::CMD_SET_MEMO_CHANGE %d.}", memo_change_tag);
    /* board中实现为空不进行操作, 已删除 board_wifi_tas_set 调用 */
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : uapi_android_get_wifi_priv_feature_cap
 功能描述  : 获取wifi 特性能力位
*****************************************************************************/
OAL_STATIC osal_s32 uapi_android_get_wifi_priv_feature_cap(oal_net_device_stru *net_dev, osal_s8 *cmd,
    osal_u32 total_len, osal_u8 *data)
{
    hmac_vap_stru                   *hmac_vap;
    osal_s32 *pl_value = OAL_PTR_NULL;
    osal_u8 *fhook = hmac_get_feature_fhook(HMAC_FHOOK_GET_11K_CAP);

    /* vap未创建时，不处理supplicant命令 */
    if (OAL_PTR_NULL == net_dev->ml_priv) {
        oam_warning_log0(0, OAM_SF_CFG,
                         "uapi_android_get_wifi_priv_feature_cap::vap not created yet, ignore the cmd!");
        return -OAL_EINVAL;
    }
    /* 将buf清零 */
    (osal_void)memset_s(cmd, total_len + 1, 0, total_len + 1);
    cmd[total_len] = '\0';
    pl_value  = (osal_u32 *)cmd;

    *pl_value    = 0;

    /* 获取mac_vap */
    hmac_vap = (hmac_vap_stru *)net_dev->ml_priv;
    if (hmac_vap != OAL_PTR_NULL) {
        if (fhook != OSAL_NULL) {
            ((hmac_get_11k_cap_cb)fhook)(hmac_vap, pl_value);
        }

        fhook = hmac_get_feature_fhook(HMAC_FHOOK_GET_11V_CAP);
        if (fhook != OSAL_NULL) {
            ((hmac_get_11v_cap_cb)fhook)(hmac_vap, pl_value);
        }

        fhook = hmac_get_feature_fhook(HMAC_FHOOK_11R_GET_11R_CAP);
        if (fhook != OSAL_NULL) {
            ((hmac_get_11r_cap_cb)fhook)(hmac_vap, pl_value);
        }
    } else {
        oam_warning_log0(0, OAM_SF_CFG, "{uapi_android_get_wifi_priv_feature_cap::netdevice->hmac_vap is null.}");
    }

    oam_warning_log1(0, OAM_SF_ANY,
                     "uapi_android_priv_cmd_etc::CMD_GET_WIFI_PRVI_FEATURE_CAPABILITY = [%x]!", *pl_value);
    /* 偏移8位赋值 */
    if (osal_copy_to_user(data + 8, cmd, total_len)) {
        oam_error_log0(0, OAM_SF_ANY,
            "uapi_android_priv_cmd_etc:CMD_GET_WIFI_PRVI_FEATURE_CAPABILITY Failed to copy ioctl_data to user !");
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_M2S
/*****************************************************************************
 函 数 名  : wal_ioctl_set_m2s_switch_blacklist
 功能描述  : 黑名单下发到device侧进行保存
*****************************************************************************/
OAL_STATIC osal_s32  uapi_android_set_m2s_blacklist(oal_net_device_stru *net_dev, osal_s8 *cmd,
    osal_u32 total_len, osal_u8 *data)
{
    osal_s32                       l_ret;
    mac_m2s_ie_stru                *m2s_ie;
    osal_u32 skip = osal_strlen(CMD_SET_M2S_BLACKLIST) + 1;
    osal_u8 *buf = (cmd + skip + 1);
    osal_u8 m2s_blacklist_cnt = *(cmd + skip);

    mac_m2s_ie_stru m2s_ie_info;
    (osal_void)memset_s(&m2s_ie_info, OAL_SIZEOF(m2s_ie_info), 0, OAL_SIZEOF(m2s_ie_info));
    m2s_ie = &m2s_ie_info;
    oam_warning_log1(0, OAM_SF_M2S, "{uapi_android_set_m2s_blacklist::CMD_SET_M2S_BLACKLIST[%d].}", *(cmd + skip));

    if (m2s_blacklist_cnt > WLAN_M2S_BLACKLIST_MAX_NUM) {
        oam_warning_log1(0, OAM_SF_M2S,
                         "{uapi_android_set_m2s_blacklist::blacklist[%d] is beyond scope.}", m2s_blacklist_cnt);
        return OAL_FAIL;
    }

    /***************************************************************************
          抛事件到wal层处理
      ***************************************************************************/

    m2s_ie->blacklist_cnt = m2s_blacklist_cnt;

    if (memcpy_s(m2s_ie->m2s_blacklist, sizeof(m2s_ie->m2s_blacklist), buf,
        m2s_blacklist_cnt * OAL_SIZEOF(wlan_m2s_mgr_vap_stru)) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{uapi_android_set_m2s_blacklist::memcpy_s error}");
    }
    l_ret = wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(net_dev), WLAN_MSG_W2H_C_CFG_SET_M2S_BLACKLIST,
        (osal_u8 *)&m2s_ie_info, OAL_SIZEOF(mac_m2s_ie_stru));
    if (OAL_UNLIKELY(OAL_SUCC != l_ret)) {
        oam_warning_log1(0, OAM_SF_M2S,
                         "{uapi_android_set_m2s_blacklist::wal_sync_send2device_no_rsp err code = [%d].}", l_ret);
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : wal_ioctl_set_m2s_switch_blacklist
 功能描述  : 黑名单下发到device侧进行保存
*****************************************************************************/
osal_s32  uapi_android_set_m2s_mss(oal_net_device_stru *net_dev, osal_s8 *cmd,
    osal_u32 total_len, osal_u8 *data)
{
    osal_s32 l_ret;
    osal_u8 m2s_mode;

    if (osal_strlen(cmd) < (osal_strlen((osal_s8 *)CMD_SET_M2S_SWITCH + 2))) { /* 加2保证有有效输入 */
        oam_warning_log0(0, OAM_SF_M2S, "{uapi_android_set_m2s_mss::CMD_SET_M2S_SWITCH command len error.}");
        return -OAL_EFAIL;
    }

    m2s_mode = (osal_u8)oal_atoi(cmd + osal_strlen((osal_s8 *)CMD_SET_M2S_SWITCH) + 1);
    oam_warning_log1(0, OAM_SF_M2S, "{uapi_android_set_m2s_mss::CMD_SET_M2S_SWITCH %d.}", m2s_mode);

    if (m2s_mode > MAC_M2S_COMMAND_MODE_GET_STATE) {
        oam_warning_log1(0, OAM_SF_M2S, "{uapi_android_set_m2s_mss::m2s_mode[%d] is beyond scope.}", m2s_mode);
        return OAL_FAIL;
    }
    l_ret = wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(net_dev), WLAN_MSG_W2H_C_CFG_SET_M2S_MSS,
        (osal_u8 *)&m2s_mode, OAL_SIZEOF(osal_s32));
    if (OAL_UNLIKELY(OAL_SUCC != l_ret)) {
        oam_warning_log1(0, OAM_SF_M2S,
                         "{uapi_android_set_m2s_mss::wal_sync_send2device_no_rsp return err code = [%d].}", l_ret);
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}

#endif

#ifndef CONFIG_HAS_EARLYSUSPEND
OAL_STATIC osal_s32  uapi_android_set_suspend_opt(oal_net_device_stru *net_dev, osal_s8 *cmd,
    osal_u32 total_len, osal_u8 *data)
{
    /* 目前还没有发现上层下发该命令,加维测出现后再分析 */
    oam_error_log1(0, 0, "uapi_android_set_suspend_opt:CMD_SETSUSPENDOPT param%d",
                   *(cmd + osal_strlen(CMD_SETSUSPENDOPT) + 1) - '0');
    return OAL_SUCC;
}

OAL_STATIC osal_s32  uapi_android_set_suspend_mode(oal_net_device_stru *net_dev, osal_s8 *cmd,
    osal_u32 total_len, osal_u8 *data)
{
    osal_s32                       l_ret;
    osal_u8 suspend = *(cmd + osal_strlen(CMD_SETSUSPENDMODE) + 1) - '0';

    if (OAL_UNLIKELY((OAL_PTR_NULL == net_dev))) {
        oam_error_log0(0, OAM_SF_ANY, "{uapi_android_set_suspend_mode::net_dev null ptr error!}");
        return -OAL_EFAUL;
    }

    /***************************************************************************
        抛事件到wal层处理
    ***************************************************************************/

    l_ret = wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(net_dev), WLAN_MSG_W2H_CFG_SET_SUSPEND_MODE,
        (osal_u8 *)&suspend, OAL_SIZEOF(suspend));
    return l_ret;
}
#endif

OAL_STATIC osal_s32  uapi_android_set_sta_pm_on(oal_net_device_stru *net_dev, osal_s8 *cmd,
    osal_u32 total_len, osal_u8 *data)
{
    osal_s32                       l_ret;
    mac_pm_switch_enum_uint8 enable;

    if (osal_strlen(cmd) < (osal_strlen((osal_s8 *)CMD_SET_STA_PM_ON + 2))) { /* 加2保证有有效输入 */
        oam_warning_log0(0, OAM_SF_ANY, "{uapi_android_set_sta_pm_on::CMD_SET_STA_PM_ON command len error.}");
        return -OAL_EFAIL;
    }

    enable = *(cmd + osal_strlen((osal_s8 *)CMD_SET_STA_PM_ON) + 1);
    l_ret = hmac_sta_pm_on((hmac_vap_stru *)net_dev->ml_priv, enable, MAC_STA_PM_CTRL_TYPE_HOST);
    if (OAL_UNLIKELY(OAL_SUCC != l_ret)) {
        oam_warning_log1(0, OAM_SF_ANY,
                         "{uapi_android_set_sta_pm_on::wal_set_sta_pm_on return err code [%d]!}", l_ret);
        return -OAL_EFAIL;
    }
    return OAL_SUCC;
}

OAL_STATIC osal_s32  uapi_android_get_capa_dbdc(oal_net_device_stru *net_dev, osal_s8 *cmd,
    osal_u32 total_len, osal_u8 *data)
{
    osal_s32 l_ret;
    osal_s32      cmd_len = osal_strlen(CMD_CAPA_DBDC_SUPP);
    osal_s32      ret_len = 0;

    if (total_len < osal_strlen(CMD_CAPA_DBDC_SUPP)) {
        oam_error_log1(0, OAM_SF_ANY, "{uapi_android_get_capa_dbdc::CMD_GET_CAPA_DBDC length is to short. need %d}",
                       osal_strlen(CMD_CAPA_DBDC_SUPP));
        return -OAL_EFAIL;
    }

    /* 将buf清零 */
    ret_len = OAL_MAX(total_len, cmd_len);
    (osal_void)memset_s(cmd, (osal_u32)(ret_len + 1), 0, (osal_u32)(ret_len + 1));
    cmd[ret_len] = '\0';

    /* support DBDC */
    if (memcpy_s(cmd, total_len, CMD_CAPA_DBDC_SUPP, osal_strlen(CMD_CAPA_DBDC_SUPP)) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{uapi_android_get_capa_dbdc::memcpy_s error}");
    }

    l_ret = osal_copy_to_user(data + 8, cmd, ret_len); /* 偏移8位赋值 */
    if (OAL_UNLIKELY(OAL_SUCC != l_ret)) {
        oam_error_log0(0, OAM_SF_ANY,
                       "uapi_android_get_capa_dbdc:CMD_GET_CAPA_DBDC Failed to copy ioctl_data to user !");
        return -OAL_EFAIL;
    }
    oam_warning_log2(0, OAM_SF_ANY, "{uapi_android_get_capa_dbdc::CMD_GET_CAPA_DBDC reply len=%d, l_ret=%d}",
                     osal_strlen(cmd), l_ret);
    return OAL_SUCC;
}

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
OAL_STATIC osal_s32  uapi_android_get_cust(oal_net_device_stru *net_dev, osal_s8 *cmd,
    osal_u32 total_len, osal_u8 *data)
{
    wlan_customize_stru *wlan_customize_etc = (wlan_customize_stru *)hwifi_wlan_customize_etc();
    /* 将buf清零 */
    (osal_void)memset_s(cmd, total_len + 1, 0, total_len + 1);
    cmd[total_len] = '\0';
    /* 赋值ht40禁止位 */
    *cmd = wlan_customize_etc->disable_capab_2ght40;
    /* 偏移8位赋值 */
    if (osal_copy_to_user(data + 8, cmd, total_len)) {
        oam_error_log0(0, OAM_SF_ANY, "uapi_android_priv_cmd_etc: Failed to copy ioctl_data to user !");
        /* 返回错误，通知supplicant拷贝失败，supplicant侧做参数保护处理 */
        return -OAL_EFAIL;
    }
    return OAL_SUCC;
}
#endif

/* Android private command */
OAL_STATIC OAL_CONST wal_android_cmd_entry g_android_cmd[] = {
    {CMD_SET_AP_WPS_P2P_IE, uapi_android_set_wps_p2p_ie},
#ifdef _PRE_WLAN_FEATURE_P2P
    {CMD_P2P_SET_NOA, uapi_android_set_p2p_noa},
    {CMD_P2P_SET_PS, uapi_android_set_p2p_ops},
#endif
#ifdef _PRE_WLAN_FEATURE_HS20
    {CMD_SET_QOS_MAP, uapi_android_set_qos_map},
#endif
    {CMD_SET_POWER_ON, uapi_android_set_power_on},
    {CMD_COUNTRY, uapi_android_set_country},
    {CMD_GET_CAPA_DBDC, uapi_android_get_capa_dbdc},
    {CMD_TX_POWER, uapi_android_reduce_sar},
    {CMD_SET_MEMO_CHANGE, uapi_android_set_memo_change},
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    {CMD_WPAS_GET_CUST, uapi_android_get_cust},
#endif
    {CMD_GET_WIFI_PRIV_FEATURE_CAPABILITY, uapi_android_get_wifi_priv_feature_cap},
#ifndef CONFIG_HAS_EARLYSUSPEND
    {CMD_SETSUSPENDOPT, uapi_android_set_suspend_opt},
    {CMD_SETSUSPENDMODE, uapi_android_set_suspend_mode},
#endif

    {CMD_SET_STA_PM_ON, uapi_android_set_sta_pm_on},
#ifdef _PRE_WLAN_FEATURE_M2S
    {CMD_SET_M2S_SWITCH, uapi_android_set_m2s_mss},
    {CMD_SET_M2S_BLACKLIST, uapi_android_set_m2s_blacklist},
#endif
};
/*****************************************************************************
 函 数 名  : uapi_android_priv_cmd_etc
 功能描述  : 私有接口函数
*****************************************************************************/
osal_s32 uapi_android_priv_cmd_etc(oal_net_device_stru *net_dev, oal_ifreq_stru *ifr, osal_s32 ul_cmd)
{
    wal_android_wifi_priv_cmd_stru  priv_cmd;
    osal_s8 *cmd = OAL_PTR_NULL;
    osal_s32 l_ret = 0;
    osal_u32 cmd_idx;

    if (!capable(CAP_NET_ADMIN)) {
        return -EPERM;
    }

    if (OAL_PTR_NULL == ifr->ifr_data) {
        return -OAL_EINVAL;
    }
#ifdef _PRE_WLAN_FEATURE_DFR
    if (plat_is_device_in_recovery() == OSAL_TRUE) {
        return OAL_SUCC;
    }
#endif
    (osal_void)memset_s((char *)&priv_cmd, sizeof(wal_android_wifi_priv_cmd_stru), 0,
        sizeof(wal_android_wifi_priv_cmd_stru));
    if (osal_copy_from_user((osal_u8 *)&priv_cmd, ifr->ifr_data, sizeof(wal_android_wifi_priv_cmd_stru))) {
        return -OAL_EINVAL;
    }

    if (priv_cmd.l_total_len > MAX_PRIV_CMD_SIZE || priv_cmd.l_total_len < 0) {
        oam_error_log1(0, OAM_SF_ANY, "{uapi_android_priv_cmd_etc::too long cmd.len:%d.}", priv_cmd.l_total_len);
        return -OAL_EINVAL;
    }

    /* 申请内存保存wpa_supplicant 下发的命令和数据 */
    cmd = oal_memalloc((osal_u32)(priv_cmd.l_total_len + 1)); /* total len 为priv cmd 后面buffer 长度 */
    if (OAL_PTR_NULL == cmd) {
        oam_error_log0(0, OAM_SF_ANY, "{uapi_android_priv_cmd_etc::mem alloc failed.}");
        return -OAL_ENOMEM;
    }

    /* 拷贝wpa_supplicant 命令到内核态中 */
    (osal_void)memset_s(cmd, (osal_u32)(priv_cmd.l_total_len + 1), 0, (osal_u32)(priv_cmd.l_total_len + 1));
    l_ret = (osal_s32)osal_copy_from_user(cmd, (void *)priv_cmd.buf, (osal_u32)(priv_cmd.l_total_len));
    if (l_ret != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_ANY, "{uapi_android_priv_cmd_etc::osal_copy_from_user: -OAL_EFAIL }");
        oal_free(cmd);
        return -OAL_EFAIL;
    }
    cmd[priv_cmd.l_total_len] = '\0';
    oam_warning_log2(0, OAM_SF_ANY, "{total_len:%d, used_len:%d.}", priv_cmd.l_total_len, priv_cmd.l_used_len);

    /* 驱动对于不支持的命令，返回成功，否则上层wpa_supplicant认为ioctl失败，导致异常重启wifi */
    for (cmd_idx = 0; cmd_idx < OAL_ARRAY_SIZE(g_android_cmd); cmd_idx++) {
        if (osal_strncasecmp_tmp_new(cmd, g_android_cmd[cmd_idx].cmd_name,
            osal_strlen(g_android_cmd[cmd_idx].cmd_name)) == 0) {
            l_ret = g_android_cmd[cmd_idx].func(net_dev, cmd, (osal_u32)priv_cmd.l_total_len, ifr->ifr_data);
            wal_record_external_log_by_name(WLAN_WIFI_ANDROID_CMD, cmd, l_ret);
            break;
        }
    }
    oal_free(cmd);
    return l_ret;
}
#endif // end of _PRE_WLAN_ANDROID_CROP

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
