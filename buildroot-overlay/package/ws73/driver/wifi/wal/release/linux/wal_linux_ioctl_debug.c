/*
 * Copyright (c) CompanyNameMagicTag 2016-2023. All rights reserved.
 * Description: linux ioctl配置调试命令
 * Create: 2016年4月8日
 */

#ifdef _PRE_WLAN_CFGID_DEBUG

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "oal_ext_if.h"
#include "oal_kernel_file.h"
#include "oal_cfg80211.h"

#include "oam_ext_if.h"
#include "frw_ext_if.h"
#include "frw_hmac.h"

#include "wlan_spec.h"
#include "wlan_types_common.h"

#include "mac_vap_ext.h"
#include "mac_resource_ext.h"
#include "mac_regdomain.h"
#include "mac_ie.h"

#include "hmac_ext_if.h"
#include "hmac_chan_mgmt.h"
#include "hmac_alg_config.h"

#include "wal_main.h"
#include "wal_ext_if.h"
#include "wal_config.h"
#include "wal_common.h"
#include "wal_ccpriv_common.h"
#include "wal_regdb.h"
#include "wal_linux_util.h"
#include "wal_linux_scan.h"
#include "wal_linux_ioctl.h"
#include "wal_linux_ccpriv.h"
#include "hmac_resource.h"
#include "hmac_csa_ap.h"
#include "hmac_p2p.h"
#include "hmac_11v.h"
#ifdef _PRE_WLAN_DFR_STAT
#include "hmac_dfr.h"
#endif
#ifdef _PRE_WLAN_FEATURE_P2P
#include "wal_linux_cfg80211.h"
#endif

#include "hcc_host_if.h"
#include "plat_cali.h"

#include "frw_common.h"
#ifdef _PRE_WLAN_FEATURE_WOW_OFFLOAD
#include "hmac_arp_offload.h"
#include "hmac_wow.h"
#include "msg_wow_rom.h"
#endif

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
#include "soc_customize_wifi.h"
#endif /* #ifdef _PRE_PLAT_FEATURE_CUSTOMIZE */

#ifdef _PRE_MEM_TRACE
#include "mem_trace.h"
#endif

#include "mac_frame.h"

#include "hmac_m2u.h"
#include "wlan_msg.h"

#ifdef _PRE_WLAN_FEATURE_WUR_TX
#include "hmac_wur_ap.h"
#endif

#include "hmac_sdp.h"
#ifdef _PRE_WLAN_FEATURE_APF
#include "mac_apf.h"
#endif
#ifdef _PRE_WLAN_SMOOTH_PHASE
#include "msg_smooth_phase_rom.h"
#endif
#ifdef _PRE_WLAN_FEATURE_STA_PM
#include "msg_psm_rom.h"
#endif
#include "msg_latency_rom.h"
#include "hmac_slp.h"
#include "hmac_bsrp_nfrp.h"
#include "hmac_11k.h"
#include "oam_struct.h"
#include "hmac_scan.h"
#include "hmac_config.h"
#include "frw_util_notifier.h"
#include "wal_config_sdp.h"
#include "wlan_msg.h"
#include "wal_linux_vap_proc.h"
#include "wal_linux_netdev.h"

#ifdef  _PRE_WLAN_DFT_STAT
#include "hmac_dfx.h"
#endif

#if defined(_PRE_WLAN_FEATURE_SNIFFER)
#include "hmac_sniffer.h"
#endif
#include "hmac_roam_if.h"
#include "hmac_feature_dft.h"

#ifdef _PRE_WLAN_FEATURE_DFR
#include "wal_linux_dfx.h"
#include "wal_linux_netdev.h"
#endif
#include "hmac_dfr.h"
#include "msg_alg_rom.h"
#ifdef _PRE_WLAN_GPIO_SERVICE
#include "gpio_ext.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_WAL_LINUX_IOCTL_DEBUG_C
#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

#define MAX_PRIV_CMD_SIZE   4096
#define APF_HEX_BYTE_SIZE 2
#define APF_HEX_NAME 16

/*****************************************************************************
  1 结构体定义
*****************************************************************************/
/* 算法参数配置结构体 */
typedef struct {
    osal_s8                           *pc_name;        /* 配置命令字符串 */
    mac_dyn_cali_cfg_enum_uint8         dyn_cali_cfg;     /* 配置命令对应的枚举值 */
    osal_u8                           auc_resv[3];    /* 字节对齐 */
} wal_ioctl_dyn_cali_stru;

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
#ifdef _PRE_WLAN_FEATURE_APF
OAL_CONST osal_s8 g_apf_cmd[] = "120c9400d706007c00d288a27c00cd88a47c00c888b87c00c388cd7c00be88e384002d08066a0ea2ad0600\
010800060412147a120182a5021a1c7aa10068a29606ffffffffffff6a26a28e04c0a83207728b84004a08000a17821c1112149c00151fffab0d2a1\
0820e446a3239a202068411035f9e9f72650a1e52f07a5fe01a1e7e00000054ffffffff7e0000004bc0a832ff68a24006ffffffffffff723f723c7c\
000c86dd68a22d06ffffffffffff722c0a147a073a0a267a22ff721f0a367a1b858217886a26a20210ff0200000000000000000000000000017201";
#endif

/* 来自wal_linux_ccpriv.c start */
/*****************************************************************************
  1. 宏定义
*****************************************************************************/
#define DECIMAL_BASE_10 10
#define DECIMAL_BASE_100 100
#define DECIMAL_BASE_1000 1000
#define PHY_SAMPLE_DATA_LENGTH 500  // 数采单次保存数量

/*****************************************************************************
  3 全局变量定义
*****************************************************************************/

#ifdef _PRE_WLAN_FEATURE_TCP_ACK_BUFFER
mac_tcp_ack_buf_cfg_table_stru g_ast_hmac_tcp_ack_buf_cfg_table[] = {
    {"enable",              MAC_TCP_ACK_BUF_ENABLE},
    {"timeout",             MAC_TCP_ACK_BUF_TIMEOUT},
    {"count",               MAC_TCP_ACK_BUF_MAX}
};
#endif

/*****************************************************************************
 功能描述  : 设置管制域国家码
*****************************************************************************/
OAL_STATIC osal_u32  uapi_ccpriv_setcountry(oal_net_device_stru *net_dev, osal_s8 *param)
{
#ifdef _PRE_WLAN_FEATURE_11D
    osal_s32                         ret;
    osal_u32                       arg_ret;
    osal_u32                       off_set;
    osal_s8                         arg[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    osal_s8                        *para;
    ret = wal_get_init_state_vap(net_dev, OAL_PTR_NULL);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_ANY, "{uapi_ccpriv_setcountry::wal_get_init_state_vap err}");
        return (osal_u32)(-ret);
    }
    /* 获取国家码字符串 */
    arg_ret = wal_get_cmd_one_arg(param, arg, OAL_SIZEOF(arg), &off_set);
    if (arg_ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_ccpriv_setcountry::wal_get_cmd_one_arg return err_code [%d]!}",
                         arg_ret);
        return arg_ret;
    }
    para = &arg[0];
    // 查表判断国家码是否合规
    if (wal_regdb_find_db_etc(para) == OSAL_NULL) {
        wifi_printf("wal_ccpriv_setcountry::country is illegal!\r\n");
        return OAL_SUCC;
    }
    hwifi_set_country_code_etc(para, COUNTRY_CODE_LEN);
    // 配置国家码
    hwifi_config_init_ini_country(net_dev); /* 打开wifi的情况下向device抛事件 */
#else
    oam_warning_log0(0, OAM_SF_ANY, "{uapi_ccpriv_setcountry::_PRE_WLAN_FEATURE_11D is not define!}\r\n");
#endif
    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 读取国家码
 输入参数  : net_dev: net device
 输出参数  : pc_extra   : 读取到的国家码
 返 回 值  : 错误码

 修改历史      :
  1.日    期   : 2015年1月16日
    修改内容   : 新生成函数

*****************************************************************************/
OAL_STATIC osal_u32  uapi_ccpriv_getcountry(oal_net_device_stru *net_dev, osal_s8 *param)
{
#ifdef _PRE_WLAN_FEATURE_11D
    osal_s32  ret;
    frw_msg cfg_info;
    mac_cfg_get_country_stru fill_msg;

    memset_s(&cfg_info, sizeof(cfg_info), 0, sizeof(cfg_info));
    cfg_msg_init(OAL_PTR_NULL, 0, (osal_u8 *)&fill_msg, sizeof(fill_msg), &cfg_info);

    ret = send_sync_cfg_to_host(wal_util_get_vap_id(net_dev), WLAN_MSG_W2H_CFG_GET_COUNTRY, &cfg_info);
    if ((ret != OAL_SUCC) || (cfg_info.rsp == OAL_PTR_NULL)) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_ccpriv_getcountry::send_sync_cfg_to_host return err code %d!}", ret);
        return ret;
    }
    wifi_printf("countrycode:%s\r\n", fill_msg.ac_country);
#else
    oam_info_log0(0, OAM_SF_ANY, "{uapi_ccpriv_getcountry::_PRE_WLAN_FEATURE_11D is not define!}");
#endif

    return OAL_SUCC;
}

#if defined(_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC) && defined(_PRE_MULTI_CORE_MODE) && \
    (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
/*****************************************************************************
 功能描述  : 配置aifsn固定值
 输入参数  : 无
 输出参数  : 无

 修改历史      :
  1.日    期   : 2015年11月23日
    修改内容   : 新生成函数

*****************************************************************************/
osal_u32  uapi_ccpriv_aifsn_cfg(oal_net_device_stru *net_dev, osal_s8 *pc_param)
{
    osal_u32                      off_set;
    osal_s8                        ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    mac_edca_cfg_stru               edca_cfg;
    osal_s32                       l_ret;
    osal_u32                      ul_ret;

    memset_s(&edca_cfg, OAL_SIZEOF(edca_cfg), 0, OAL_SIZEOF(edca_cfg));

    /* 获取配置开关 */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &off_set);
    if (OAL_SUCC != ul_ret) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_ccpriv_aifsn_cfg::get wfa switch fail, return err_code[%d]!}", ul_ret);
        return ul_ret;
    }
    pc_param += off_set;
    edca_cfg.switch_code = (oal_bool_enum_uint8)oal_atoi((const osal_s8 *)ac_name);

    /* 获取ac */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &off_set);
    if (OAL_SUCC != ul_ret) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_ccpriv_aifsn_cfg::get wfa ac fail, return err_code[%d]!}", ul_ret);
        return ul_ret;
    }
    pc_param += off_set;
    edca_cfg.ac = (wlan_wme_ac_type_enum_uint8)oal_atoi((const osal_s8 *)ac_name);

    if (OAL_TRUE == edca_cfg.switch_code) {
        /* 获取配置值 */
        ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &off_set);
        if (OAL_SUCC != ul_ret) {
            oam_warning_log1(0, OAM_SF_ANY, "{uapi_ccpriv_aifsn_cfg::get wfa val fail, return err_code[%d]!}", ul_ret);
            return ul_ret;
        }
        pc_param += off_set;
        edca_cfg.val = (osal_u16)oal_atoi((const osal_s8 *)ac_name);
    }
    l_ret = wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(net_dev), WLAN_MSG_W2H_CFG_WFA_CFG_AIFSN,
        (osal_u8 *)&edca_cfg, OAL_SIZEOF(edca_cfg));
    if (OAL_UNLIKELY(OAL_SUCC != l_ret)) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_ccpriv_aifsn_cfg::return err code[%d]!}", ul_ret);
        return (osal_u32)l_ret;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 配置cw固定值
 输入参数  : 无
 输出参数  : 无

 修改历史      :
  1.日    期   : 2015年11月23日
    修改内容   : 新生成函数

*****************************************************************************/
osal_u32  uapi_ccpriv_cw_cfg(oal_net_device_stru *net_dev, osal_s8 *pc_param)
{
    osal_u32                      off_set;
    osal_s8                        ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    mac_edca_cfg_stru               edca_cfg;
    osal_s32                       l_ret;
    osal_u32                      ul_ret;

    memset_s(&edca_cfg, OAL_SIZEOF(edca_cfg), 0, OAL_SIZEOF(edca_cfg));

    /* 获取配置开关 */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &off_set);
    if (OAL_SUCC != ul_ret) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_ccpriv_cw_cfg::get wfa switch fail, return err_code[%d]!}", ul_ret);
        return ul_ret;
    }
    pc_param += off_set;
    edca_cfg.switch_code = (oal_bool_enum_uint8)oal_atoi((const osal_s8 *)ac_name);

    /* 获取ac */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &off_set);
    if (OAL_SUCC != ul_ret) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_ccpriv_cw_cfg::get wfa ac fail, return err_code[%d]!}", ul_ret);
        return ul_ret;
    }
    pc_param += off_set;
    edca_cfg.ac = (wlan_wme_ac_type_enum_uint8)oal_atoi((const osal_s8 *)ac_name);

    if (OAL_TRUE == edca_cfg.switch_code) {
        /* 获取配置值 */
        ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &off_set);
        if (OAL_SUCC != ul_ret) {
            oam_warning_log1(0, OAM_SF_ANY, "{uapi_ccpriv_cw_cfg::get wfa val fail, return err_code[%d]!}", ul_ret);
            return ul_ret;
        }
        pc_param += off_set;
        edca_cfg.val = (osal_u16)osal_strtol(ac_name, OAL_PTR_NULL, 0);
    }

    l_ret = wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(net_dev), WLAN_MSG_W2H_CFG_WFA_CFG_CW,
        (osal_u8 *)(osal_u8 *)&edca_cfg, OAL_SIZEOF(edca_cfg));
    if (OAL_UNLIKELY(OAL_SUCC != l_ret)) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_ccpriv_aifsn_cfg::return err code[%d]!}", ul_ret);
        return (osal_u32)l_ret;
    }

    return OAL_SUCC;
}
#endif

#ifdef _PRE_WLAN_FEATURE_SMPS
/*****************************************************************************
 函 数 名  : uapi_ccpriv_set_smps_mode
 功能描述  : 设置device的SMPS能力
*****************************************************************************/
osal_u32  uapi_ccpriv_set_smps_mode(oal_net_device_stru *net_dev, osal_s8 *pc_param)
{
    osal_s32                   tmp;
    osal_u32                  off_set;
    osal_s8                    name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    osal_s32                   ret;
    osal_u32                  uret;

    /* 此处将解析出"1"、"2"或"3"存入ac_name */
    uret = wal_get_cmd_one_arg(pc_param, name, OAL_SIZEOF(name), &off_set);
    if (uret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_SMPS,
                         "{uapi_ccpriv_set_smps_mode::wal_get_cmd_one_arg return err_code[%d]}", uret);
        return uret;
    }

    /* 针对解析出的不同命令，设置不同SMPS模式 */
    if ((osal_strcmp("1", (const osal_s8 *)name)) == 0) {
        tmp = 1;
    } else if ((osal_strcmp("2", (const osal_s8 *)name)) == 0) {
        tmp = 2;    /* 字符2转为数字2 */
    } else if ((osal_strcmp("3", (const osal_s8 *)name)) == 0) {
        tmp = 3;    /* 字符3转为数字3 */
    } else {
        oam_error_log1(0, OAM_SF_SMPS, "{uapi_ccpriv_set_smps_mode::error cmd[%d],input 1/2/3!}", name);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }
    ret = wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(net_dev), WLAN_MSG_W2H_CFG_SMPS_MODE,
        (osal_u8 *)&tmp, OAL_SIZEOF(tmp));
    if (OAL_UNLIKELY(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_SMPS, "{uapi_ccpriv_set_smps_mode::return err code [%d]!}", ret);
        return (osal_u32)ret;
    }

    return OAL_SUCC;
}
#endif

/* 开启或关闭ampdu发送功能 */
STATIC osal_u32 ccpriv_ampdu_parse_bit_args(osal_s8 *param, mac_cfg_ampdu_tx_on_param_stru *aggr_param)
{
    osal_u32 ret, off_set;
    osal_s8 name[WAL_HIPRIV_CMD_NAME_MAX_LEN];

    /* 第二个参数 */
    ret = wal_get_cmd_one_arg(param, name, OAL_SIZEOF(name), &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_ccpriv_ampdu_tx_on::wal_get_cmd_one_arg err_code [%d]!}", ret);
        return ret;
    }

    aggr_param->snd_type = (osal_u8)oal_atoi((const osal_s8 *)name);
    return OAL_CONTINUE;
}

osal_u32 uapi_ccpriv_ampdu_tx_on(oal_net_device_stru *net_dev, osal_s8 *param)
{
    osal_u32 off_set;
    osal_s8 name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    osal_u32 ret;
    osal_s32 send_ret;
    mac_cfg_ampdu_tx_on_param_stru cfg_aggr_param = {0};

    if (wal_ccpriv_check_cfg_vap(net_dev) == OSAL_TRUE) {
        oam_warning_log0(0, OAM_SF_ANY, "uapi_ccpriv_set_mode vap mode err: cfg");
        return OAL_FAIL;
    }

    ret = wal_get_cmd_one_arg(param, name, OAL_SIZEOF(name), &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_ccpriv_ampdu_tx_on::wal_get_cmd_one_arg err_code [%d]!}", ret);
        return ret;
    }
    param += off_set;

    cfg_aggr_param.aggr_tx_on = (osal_u8)oal_atoi((const osal_s8 *)name);
    cfg_aggr_param.snd_type = 0;

    /* 只有硬件聚合需要配置第二参数 */
    if (cfg_aggr_param.aggr_tx_on & BIT1) {
        ret = ccpriv_ampdu_parse_bit_args(param, &cfg_aggr_param);
        if (ret != OAL_CONTINUE) {
            return ret;
        }
    }

    /* **************************************************************************
                             抛事件到wal层处理
    ************************************************************************** */
    /* 设置配置命令参数 */
    if (cfg_aggr_param.snd_type > 1) {
        cfg_aggr_param.snd_type = 1;
    }

    send_ret = wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(net_dev), WLAN_MSG_W2H_CFG_AMPDU_TX_ON,
        (osal_u8 *)&cfg_aggr_param, OAL_SIZEOF(cfg_aggr_param));
    if (OAL_UNLIKELY(send_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_ccpriv_ampdu_tx_on::return err code [%d]!}", send_ret);
        return (osal_u32)send_ret;
    }

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_SNIFFER
/*****************************************************************************
 功能描述  : 抓包配置命令"sniffer_save_file 1"的处理强制打开抓包模式
*****************************************************************************/
OAL_STATIC osal_void wal_ccpriv_wifi_sniffer_on(const oal_net_device_stru *netdev)
{
    OAL_STATIC osal_u8 proc_create_flag = 0;
    osal_u32 ul_ret;

    if (OAL_UNLIKELY(netdev == OAL_PTR_NULL)) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_ccpriv_wifi_sniffer_on::netdev is null}");
        return;
    }

    if (proc_create_flag != 1) {
        oam_warning_log4(0, OAM_SF_ANY,
            "wal_ccpriv_wifi_sniffer_on:: sniffer_save_file on, [cur,max,num,len]=[%d %d %d %d]",
            g_sniffer_st.cur_file, g_sniffer_st.data_packet_max_len, g_sniffer_st.max_file_num,
            g_sniffer_st.max_file_len);
        ul_ret = wal_wifi_sniffer_proc_file_sec();
        if (ul_ret == OAL_SUCC) {
            proc_create_flag = 1;
        }
    } else {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_ccpriv_wifi_sniffer_on::proc files already created}");
    }

    proc_handle_command_sniffer_enable(1);
}

/*****************************************************************************
 功能描述  : 抓包配置命令"sniffer_save_file 0"的处理.强制关闭混杂模式
*****************************************************************************/
OAL_STATIC osal_void wal_ccpriv_wifi_sniffer_off(oal_net_device_stru *netdev, osal_u8 sniffer_switch)
{
    proc_handle_command_sniffer_enable(sniffer_switch);
    oam_warning_log0(0, OAM_SF_ANY, "wal_ccpriv_wifi_sniffer_off:: sniffer_save_file off");
}

/*****************************************************************************
 功能描述  : 抓包配置命令"sniffer_save_file 1 [FILE_NUM] [FILE_SIZE]"的解析处理
*****************************************************************************/
OAL_STATIC osal_u32  wal_ccpriv_wifi_sniffer_config(const osal_s8 *pc_param)
{
    osal_u32 ret;
    osal_u32 off_set;
    osal_s32  ret_l;
    osal_s32  file_num;
    osal_s32  file_size;
    osal_s32  proc_file_size;
    osal_s8   name[WAL_HIPRIV_CMD_NAME_MAX_LEN];

    ret = wal_get_cmd_one_arg(pc_param, name, OAL_SIZEOF(name), &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_ccpriv_wifi_sniffer_config::get file num fail, return [%d]!}", ret);
        return ret;
    }
    pc_param += off_set;
    file_num = oal_atoi((const osal_s8 *)name);
    if (file_num < PCAP_FILE_NUM_MIN || file_num > PCAP_FILE_NUM_MAX) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_ccpriv_wifi_sniffer_config::invalid file_num[%d]}", file_num);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    ret = wal_get_cmd_one_arg(pc_param, name, sizeof(name), &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_ccpriv_wifi_sniffer_config::get file size fail, return [%d]!}", ret);
        return ret;
    }

    file_size = oal_atoi((const osal_s8 *)name);
    if ((file_size * file_num  < PCAP_FILE_NUM_MIN) || (file_size * file_num > hwifi_get_sniffer_file_len_max())) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_ccpriv_wifi_sniffer_config:invalid file_size[%d]}", file_size * file_num);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    proc_file_size = file_size * 1024 * 1024; /* file_size's unit is Mbit(1024 * 1024) */
    oam_warning_log2(0, OAM_SF_ANY,
        "wal_ccpriv_wifi_sniffer_config:: sniffer_save_file config, file_num = %d, file_len = %d(Mb)",
        file_num, file_size);
    ret_l = proc_sniffer_reset_file_num_len(file_num, proc_file_size);
    if (ret_l) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_ccpriv_wifi_sniffer_config::proc_sniffer_reset_file_num_len, return [%d]!}", ret_l);
        return OAL_ERR_CODE_ALLOC_MEM_FAIL;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 配置抓包参数,开启抓包(sniffer_save_file 1 [FILE_NUM] [FILE_SIZE])
*****************************************************************************/
osal_u32 uapi_ccpriv_wifi_sniffer(oal_net_device_stru *netdev, osal_s8 *pc_param)
{
    osal_u8   sniffer_switch;
    osal_u32  ul_ret;
    osal_u32  off_set;
    osal_u8   ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];

    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &off_set);
    if (ul_ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_ccpriv_wifi_sniffer::get arg return err_code %d!}", ul_ret);
        return ul_ret;
    }
    pc_param += off_set;

    sniffer_switch = (osal_u8)oal_atoi((const osal_s8 *)ac_name);
    if (sniffer_switch == 0 || sniffer_switch == 2) { /* sniffer_save_file 0/2 */
        wal_ccpriv_wifi_sniffer_off(netdev, sniffer_switch);
        oam_warning_log0(0, OAM_SF_ANY, "{uapi_ccpriv_wifi_sniffer::sniffer_save_file off}");
    } else if (sniffer_switch == 1) {    /* sniffer_save_file 1 [FILE_NUM] [FILE_SIZE] */
        ul_ret = wal_ccpriv_wifi_sniffer_config(pc_param);
        if (ul_ret != OAL_SUCC) {
            oam_warning_log0(0, OAM_SF_ANY, "{uapi_ccpriv_wifi_sniffer::config FAIL}");
            return ul_ret;
        }
        wal_ccpriv_wifi_sniffer_on(netdev);
        oam_warning_log0(0, OAM_SF_ANY, "{uapi_ccpriv_wifi_sniffer::sniffer_save_file on}");
    } else {
        oam_error_log0(0, OAM_SF_ANY, "{uapi_ccpriv_wifi_sniffer::invalid cmd}");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    return OAL_SUCC;
}
#endif

#ifdef _PRE_WLAN_DFT_STAT
osal_u32 uapi_ccpriv_get_vap_stat(oal_net_device_stru *net_dev, osal_s8 *param)
{
    osal_s8                    args[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    osal_u8                   stat_clr = 0;
    osal_u32                  off_set, ret;
    frw_msg                     cfg_info;
    hmac_vap_query_stats_stru  *fill_msg = OSAL_NULL;

    /* echo "wlan0 get_vap_stat [clr]" > /sys/ccsys/ccpriv" */
    ret = wal_get_cmd_one_arg(param, args, OAL_SIZEOF(args), &off_set);
    if (ret == OAL_SUCC && osal_strcmp("clr", (const osal_s8 *)args) == 0) {
        stat_clr = 1;
    }

    fill_msg = oal_memalloc(sizeof(hmac_vap_query_stats_stru));
    if (fill_msg == OSAL_NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{uapi_ccpriv_get_vap_stat::parse arg failed [%d]!}");
        return OAL_FAIL;
    }
    memset_s(fill_msg, sizeof(hmac_vap_query_stats_stru), 0, sizeof(hmac_vap_query_stats_stru));

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    memset_s(&cfg_info, sizeof(cfg_info), 0, sizeof(cfg_info));
    cfg_msg_init(&stat_clr, sizeof(stat_clr), (osal_u8 *)fill_msg, sizeof(hmac_vap_query_stats_stru), &cfg_info);

    ret = send_sync_cfg_to_host(wal_util_get_vap_id(net_dev), WLAN_MSG_W2H_CFG_GET_VAP_STAT, &cfg_info);
    if ((ret != OAL_SUCC) || (cfg_info.rsp == OAL_PTR_NULL)) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_ccpriv_get_vap_stat::send_sync_cfg_to_host return err code %d!}", ret);
        printk("{uapi_ccpriv_get_vap_stat::send_sync_cfg_to_host return err code %d!}", ret);
        return ret;
    }

    if (stat_clr == 1) {
        printk("dmac vap stats clear success.\r\n");
        oal_free(fill_msg);
        return OAL_SUCC;
    }

    /* 打印返回的统计信息 */
    printk("\r\n");
    printk("====================dmac vap stats========================\r\n");
    printk("signal                          = %d\r\n", fill_msg->signal); // oal_get_real_rssi(fill_msg->signal));
    printk("snr_ant0                        = %d\r\n", fill_msg->snr_ant0);
    printk("snr_ant1                        = %d\r\n", fill_msg->snr_ant1);
    printk("signal_mgmt_data                = %d\r\n", fill_msg->signal_mgmt_data);
    printk("\r\n");

    printk("==================== RX stats DATA========================\r\n");
    printk("rx_d_from_hw_pkts               = %u\r\n", fill_msg->rx_d_from_hw_pkts);
    printk("rx_d_send_hmac_fail             = %u\r\n", fill_msg->rx_d_send_hmac_fail);
    printk("\r\n");

    oal_free(fill_msg);
    return OAL_SUCC;
}

osal_u32 uapi_ccpriv_get_vap_aggre_info(oal_net_device_stru *net_dev, osal_s8 *param)
{
    osal_s32 ret;
    osal_u8 vap_id = wal_util_get_vap_id(net_dev);

    ret = hmac_query_tx_vap_aggre_info(vap_id);
    if (OAL_UNLIKELY(ret != OAL_SUCC)) {
        oam_warning_log2(0, OAM_SF_ANY, "vap_id[%d] {uapi_ccpriv_get_vap_aggre_info::return err code [%d]!}",
                         vap_id, ret);
        return (osal_u32)ret;
    }
    return OAL_SUCC;
}

OAL_STATIC osal_u32 uapi_ccpriv_external_record_enable(oal_net_device_stru *net_dev, osal_s8 *param)
{
    osal_u32 ret;
    osal_u8 enable_switch = 0;
    unref_param(param);

    /* 获取帧方向 */
    ret = wal_cmd_get_u8_with_check_max(&param, 1, &enable_switch);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_CFG, "{uapi_ccpriv_external_record_enable::enable_switch invalid.}");
        return ret;
    }
    if (enable_switch > 1) {
        oam_error_log1(0, OAM_SF_CFG,
            "{uapi_ccpriv_external_record_enable::enable_switch[%u] invalid.}", enable_switch);
        return OAL_FAIL;
    }
    wal_set_external_record_enable(enable_switch);
    return OAL_SUCC;
}
#endif
#ifdef _PRE_WLAN_DFT_STAT
osal_u32 uapi_ccpriv_get_hw_stat(oal_net_device_stru *net_dev, osal_s8 *param)
{
    osal_s32 l_ret;
    osal_u8 vap_id = wal_util_get_vap_id(net_dev);

    l_ret = wal_sync_post2hmac_no_rsp(vap_id, WLAN_MSG_W2H_CFG_GET_HW_STAT, OSAL_NULL, 0);
    if (OAL_UNLIKELY(l_ret != OAL_SUCC)) {
        oam_warning_log2(0, OAM_SF_ANY, "vap_id[%d] {uapi_ccpriv_user_info::return err code [%d]!}",
                         vap_id, l_ret);
        return (osal_u32)l_ret;
    }

    return OAL_SUCC;
}
osal_u32 uapi_ccpriv_hw_stat_enable(oal_net_device_stru *net_dev, osal_s8 *param)
{
    osal_u32 ret;
    osal_s32 ret1;
    osal_u8 enable_switch = 0;
    unref_param(param);

    /* 获取帧方向 */
    ret = wal_cmd_get_u8_with_check_max(&param, 1, &enable_switch);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_CFG, "{wal_ccpriv_user_rate_info_enable::switch_value invalid.}");
        return ret;
    }
    ret1 = wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(net_dev), WLAN_MSG_W2H_CFG_HW_STAT_ENABLE,
        (osal_u8 *)&enable_switch, OAL_SIZEOF(enable_switch));
    if (OAL_UNLIKELY(ret1 != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_ccpriv_user_rate_info_enable::return err code[%d]!}", ret1);
        return (osal_u32)ret1;
    }

    return OAL_SUCC;
}
#endif

#ifdef _PRE_WLAN_DFT_STAT
osal_u32 uapi_ccpriv_get_vap_stat_new(oal_net_device_stru *net_dev, osal_s8 *param)
{
    osal_s32 l_ret;
    dft_vap_statis_command_stru vap_statis_command = {0};
    osal_u8 vap_id = wal_util_get_vap_id(net_dev);

    vap_statis_command.vap_id = vap_id;

    l_ret = hmac_get_vap_stat_new(&vap_statis_command);
    if (OAL_UNLIKELY(l_ret != OAL_SUCC)) {
        oam_warning_log2(0, OAM_SF_ANY, "vap_id[%d] {uapi_ccpriv_get_vap_stat_new::return err code [%d]!}",
                         vap_id, l_ret);
        return (osal_u32)l_ret;
    }
    /* 打印出vap的收发包状态 */
    hmac_vap_stat_tx_display(vap_id);
    hmac_vap_stat_rx_display(vap_id);
    return OAL_SUCC;
}
osal_u32 uapi_ccpriv_vap_stat_enable(oal_net_device_stru *net_dev, osal_s8 *param)
{
    osal_u32 ret;
    dft_vap_statis_command_stru vap_statis_command = {0};
    /* 获取使能参数 */
    ret = wal_cmd_get_u8_with_check_max(&param, 1, &vap_statis_command.enable);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_CFG, "{wal_ccpriv_user_rate_info_enable::switch_value invalid.}");
        return ret;
    }

    vap_statis_command.vap_id = wal_util_get_vap_id(net_dev);
    hmac_config_vap_stat_enable(&vap_statis_command);

    return OAL_SUCC;
}
#endif

/*****************************************************************************
 功能描述  : 统计指定tid的吞吐量
 输入参数  : 无
 输出参数  : 无

 修改历史      :
  1.日    期   : 2014年1月14日
    修改内容   : 新生成函数

*****************************************************************************/
osal_u32  wal_ccpriv_get_mac_addr(osal_s8 *param, osal_u8 auc_mac_addr[], osal_u32 *pul_total_offset)
{
    osal_u32 off_set = 0;
    osal_u32 ret = OAL_SUCC;
    osal_s8 name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};

    /* 获取mac地址 */
    ret = wal_get_cmd_one_arg(param, name, OAL_SIZEOF(name), &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_ccpriv_get_mac_addr::wal_get_cmd_one_arg return err_code [%d]!}",
                         ret);
        return ret;
    }
    oal_strtoaddr((const osal_s8 *)name, auc_mac_addr);

    *pul_total_offset = off_set;

    return OAL_SUCC;
}

oal_module_symbol(wal_ccpriv_get_mac_addr);

/* 来自wal_linux_ccpriv.c end */

/*****************************************************************************
  3 函数实现
*****************************************************************************/
static inline oal_bool_enum_uint8 check_bool(osal_s8 *value)
{
    /* 开关类型的值仅能为字符'0'或'1' */
    if ((osal_strlen((const osal_s8 *)value) != 1) || (value[0] != '0' && value[0] != '1')) {
        return OAL_FALSE;
    }
    return OAL_TRUE;
}

static inline oal_bool_enum_uint8 check_signed(osal_s8 *value)
{
    osal_u32 i = 0;
    /* 符号数字符串允许纯数字字符或以'-'开头的纯数字字符 */
    if ((osal_strlen((const osal_s8 *)value) == 0) ||
        (osal_strlen((const osal_s8 *)value) == 1 && value[0] == '-')) {
        return OAL_FALSE;
    }
    if (value[0] == '-') {
        i++;
    }
    for (; i < osal_strlen((const osal_s8 *)value); i++) {
        if (!oal_isdigit(value[i])) {
            return OAL_FALSE;
        }
    }
    return OAL_TRUE;
}

static inline oal_bool_enum_uint8 check_unsigned(osal_s8 *value)
{
    osal_u32 i;
    /* 无符号数字符串仅允许纯数字字符 */
    if (osal_strlen((const osal_s8 *)value) == 0) {
        return OAL_FALSE;
    }
    for (i = 0; i < osal_strlen((const osal_s8 *)value); i++) {
        if (!oal_isdigit(value[i])) {
            return OAL_FALSE;
        }
    }
    return OAL_TRUE;
}

static inline oal_bool_enum_uint8 check_unsigned_range(osal_s8 *value, osal_u32 min, osal_u32 max)
{
    osal_u32 num;
    if ((osal_strlen((const osal_s8 *)value) == 0) || (check_unsigned(value) != OAL_TRUE)) {
        return OAL_FALSE;
    }
    num = (osal_u32)oal_atoi((const osal_s8 *)value);
    if ((num < min) || (num > max)) {
        return OAL_FALSE;
    }
    return OAL_TRUE;
}

#ifdef _PRE_MEM_TRACE
/*****************************************************************************
 功能描述  : 显示内存跟踪模块的信息
 输入参数  : 无
 输出参数  : 无

 修改历史      :
  1.日    期   : 2017年5月27日
    修改内容   : 新生成函数

*****************************************************************************/
OAL_STATIC osal_u32 wal_ccpriv_mem_trace_info_show(oal_net_device_stru *net_dev, osal_s8 *pc_param)
{
    osal_u32                      ul_ret;
    osal_u32                      off_set;
    osal_s8                        ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    osal_u32                      mode;
    osal_u32                      fileid = ~0;
    osal_u32                      line = 0;

    /* 获取开关 sh ccpriv.sh "Featureid0 mem_trace_show 0 (file_id,line) | 1 | 2" */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &off_set);
    if (ul_ret != OAL_SUCC) {
        return ul_ret;
    }

    mode = (osal_u32)oal_atoi((const osal_s8 *)ac_name);
    if (mode == 0) {
        pc_param += off_set;
        ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &off_set);
        if (ul_ret == OAL_SUCC) {
            fileid = (osal_u32)oal_atoi((const osal_s8 *)ac_name);
            pc_param += off_set;
            ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &off_set);
            if (ul_ret == OAL_SUCC) {
                line = (osal_u32)oal_atoi((const osal_s8 *)ac_name);
            }
        }
    }

    mem_trace_info_show(mode, fileid, line);
    return OAL_SUCC;
}

#endif

STATIC osal_u32 ccpriv_addba_req_parse(osal_s8 *param, mac_cfg_addba_req_param_stru *addba_req)
{
    osal_u32 ret, off_set;
    osal_s8 name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};

    /* 获取mac地址 */
    ret = wal_get_cmd_one_arg(param, name, OAL_SIZEOF(name), &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{ccpriv_addba_req_parse::wal_get_cmd_one_arg err_code [%d]!}", ret);
        return ret;
    }

    oal_strtoaddr((const osal_s8 *)name, addba_req->mac_addr);
    /* 偏移，取下一个参数 */
    param = param + off_set;

    /* 获取tid */
    ret = wal_cmd_get_u8_with_check_max(&param, WLAN_TID_MAX_NUM - 1, &addba_req->tidno);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{ccpriv_addba_req_parse::error!tidno is [%d]!}", addba_req->tidno);
        return ret;
    }

    /* 获取ba_policy */
    ret = wal_get_cmd_one_arg(param, name, OAL_SIZEOF(name), &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{ccpriv_addba_req_parse::wal_get_cmd_one_arg err_code [%d]!}", ret);
        return ret;
    }

    addba_req->ba_policy = (osal_u8)oal_atoi((const osal_s8 *)name);
    if (addba_req->ba_policy != MAC_BA_POLICY_IMMEDIATE) {
        oam_warning_log1(0, OAM_SF_ANY, "{ccpriv_addba_req_parse::error! ba_policy is[%d]!}", addba_req->ba_policy);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    param = param + off_set;

    /* 获取buffsize */
    ret = wal_get_cmd_one_arg(param, name, OAL_SIZEOF(name), &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{ccpriv_addba_req_parse::wal_get_cmd_one_arg err_code [%d]!}", ret);
        return ret;
    }

    addba_req->buff_size = (osal_u16)oal_atoi((const osal_s8 *)name);

    param = param + off_set;

    /* 获取timeout时间 */
    ret = wal_get_cmd_one_arg(param, name, OAL_SIZEOF(name), &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{ccpriv_addba_req_parse::wal_get_cmd_one_arg err_code [%d]!}", ret);
        return ret;
    }

    addba_req->timeout = (osal_u16)oal_atoi((const osal_s8 *)name);
    return OAL_CONTINUE;
}

/* 建立BA会话的调测命令 */
osal_u32 uapi_ccpriv_addba_req(oal_net_device_stru *net_dev, osal_s8 *param)
{
    osal_s32 send_ret;
    mac_cfg_addba_req_param_stru *addba_req = OAL_PTR_NULL;
    osal_u32 ret;
    mac_cfg_addba_req_param_stru cfg_addba_req;

    (osal_void)memset_s(&cfg_addba_req, OAL_SIZEOF(cfg_addba_req), 0, OAL_SIZEOF(cfg_addba_req));
    addba_req = &cfg_addba_req;

    /*
        设置AMPDU关闭的配置命令:
        ccpriv "Featureid0 addba_req xx xx xx xx xx xx(mac地址) tidno ba_policy buffsize timeout"
    */
    ret = ccpriv_addba_req_parse(param, addba_req);
    if (ret != OAL_CONTINUE) {
        return ret;
    }

    /* **************************************************************************
                                抛事件到wal层处理
    ************************************************************************** */
    send_ret = wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(net_dev), WLAN_MSG_W2H_CFG_ADDBA_REQ,
        (osal_u8 *)&cfg_addba_req, OAL_SIZEOF(cfg_addba_req));
    if (OAL_UNLIKELY(send_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_ccpriv_addba_req::return err code[%d]!}", send_ret);
        return (osal_u32)send_ret;
    }

    return OAL_SUCC;
}

/* 删除BA会话的调测命令 */
osal_u32 uapi_ccpriv_delba_req(oal_net_device_stru *net_dev, osal_s8 *param)
{
    osal_u32 off_set, ret;
    osal_s8 name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    osal_s32 send_ret;
    mac_cfg_delba_req_param_stru *delba_req_param = OAL_PTR_NULL;
    mac_cfg_delba_req_param_stru cfg_delba_req_param;

    (osal_void)memset_s(&cfg_delba_req_param, OAL_SIZEOF(cfg_delba_req_param), 0, OAL_SIZEOF(cfg_delba_req_param));
    delba_req_param = &cfg_delba_req_param;

    /*
        设置AMPDU关闭的配置命令:
        ccpriv "Featureid0 delba_req xx xx xx xx xx xx(mac地址) tidno direction reason_code"
    */
    /* 获取mac地址 */
    ret = wal_get_cmd_one_arg(param, name, OAL_SIZEOF(name), &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_ccpriv_delba_req::wal_get_cmd_one_arg err_code [%d]!}", ret);
        return ret;
    }

    oal_strtoaddr((const osal_s8 *)name, delba_req_param->mac_addr);
    /* 偏移，取下一个参数 */
    param = param + off_set;

    /* 获取tid */
    ret = wal_cmd_get_u8_with_check_max(&param, WLAN_TID_MAX_NUM - 1, &delba_req_param->tidno);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_ccpriv_delba_req::tidno[%d] is error!}", delba_req_param->tidno);
        return ret;
    }

    /* 获取direction */
    ret = wal_get_cmd_one_arg(param, name, OAL_SIZEOF(name), &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_ccpriv_delba_req::wal_get_cmd_one_arg err_code [%d]!}", ret);
        return ret;
    }

    delba_req_param->direction = (osal_u8)oal_atoi((const osal_s8 *)name);
    if (delba_req_param->direction >= MAC_BUTT_DELBA) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_ccpriv_delba_req::direction[%d] is error!}",
            delba_req_param->direction);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    /* **************************************************************************
                                抛事件到wal层处理
    ************************************************************************** */
    send_ret = wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(net_dev), WLAN_MSG_W2H_CFG_DELBA_REQ,
        (osal_u8 *)&cfg_delba_req_param, OAL_SIZEOF(cfg_delba_req_param));
    if (OAL_UNLIKELY(send_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_ccpriv_delba_req::return err code[%d]!}", send_ret);
        return (osal_u32)send_ret;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 调整beacon帧的发送策略(0表示关闭双路轮流发送，1表示开启)
 输入参数  : net_dev : net_device
             pc_param    : 参数
 输出参数  : 无
 返 回 值  : 错误码

 修改历史      :
  1.日    期   : 2014年2月13日
    修改内容   : 新生成函数

*****************************************************************************/
osal_u32  uapi_ccpriv_beacon_chain_switch(oal_net_device_stru *net_dev, osal_s8 *pc_param)
{
    osal_s32 tmp;
    osal_u32 off_set;
    osal_s8 ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    osal_s32 msg_ret;
    osal_u32 ret;

    if (OAL_UNLIKELY((net_dev == OAL_PTR_NULL) || (pc_param == OAL_PTR_NULL))) {
        oam_error_log2(0, OAM_SF_ANY,
            "{uapi_ccpriv_beacon_chain_switch::net_dev or pc_param null ptr error %p, %p!}",
            (uintptr_t)net_dev, (uintptr_t)pc_param);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* beacon通道(0/1/2)切换开关的命令: ccpriv "vap0 beacon_chain_switch 0 | 1 | 2"
        此处将解析出"0"或"1"或"2"存入ac_name
    */
    ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
                         "{uapi_ccpriv_beacon_chain_switch::wal_get_cmd_one_arg return err_code %d!}", ret);
        return ret;
    }

    /* 针对解析出的不同命令，配置不同的通道 */
    if ((osal_strcmp("0", (const osal_s8 *)ac_name)) == 0) {
        tmp = 0; /* 0、1、2：不通的通道 */
    } else if ((osal_strcmp("1", (const osal_s8 *)ac_name)) == 0) {
        tmp = 1; /* 0、1、2：不通的通道 */
    } else if ((osal_strcmp("2", (const osal_s8 *)ac_name)) == 0) {
        tmp = 2; /* 0、1、2：不通的通道 */
    } else {
        oam_warning_log1(0, OAM_SF_ANY,
            "{uapi_ccpriv_beacon_chain_switch::the beacon chain switch command is error %d!}",
            oal_atoi((const osal_s8 *)ac_name));
        return OAL_ERR_CODE_INVALID_CONFIG;
    }
    msg_ret = wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(net_dev), WLAN_MSG_W2H_C_CFG_BEACON_CHAIN_SWITCH,
        (osal_u8 *)&tmp, OAL_SIZEOF(tmp));
    if (OAL_UNLIKELY(msg_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_ccpriv_beacon_chain_switch::return err code %d!}", msg_ret);
        return (osal_u32)msg_ret;
    }

    return OAL_SUCC;
}

OAL_STATIC osal_u32 uapi_ccpriv_show_frag_threshold(oal_net_device_stru *net_dev, osal_s8 *pc_param)
{
    osal_s32 ret;
    osal_u32 data = 0;
    ret = wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(net_dev), WLAN_MSG_W2H_CFG_SHOW_FRAG_THRESHOLD_REG,
        (osal_u8 *)&data, OAL_SIZEOF(data));
    if (OAL_UNLIKELY(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_ccpriv_show_frag_threshold::return err code [%d]!}", ret);
        return (osal_u32)ret;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 测试命令，打开/关闭WMM
 输入参数  : 无
 输出参数  : 无

 修改历史      :
  1.日    期   : 2014年1月27日
    修改内容   : 新生成函数

*****************************************************************************/
osal_u32  uapi_ccpriv_wmm_switch(oal_net_device_stru *net_dev, osal_s8 *pc_param)
{
    osal_u32                      ul_ret;
    osal_s32                       l_cfg_rst;
    osal_s8                        ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    osal_u32                      off_set = 0;
    osal_u8                       open_wmm;

    /* 获取设定的值 */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &off_set);
    if (ul_ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
                         "{uapi_ccpriv_wmm_switch::wal_get_cmd_one_arg return err_code [%d]!}", ul_ret);
        return ul_ret;
    }
    open_wmm = (osal_u8)oal_atoi((const osal_s8 *)ac_name);
    pc_param += off_set;

    /***************************************************************************
                              抛事件到wal层处理
    ***************************************************************************/
    l_cfg_rst = wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(net_dev), WLAN_MSG_W2H_CFG_WMM_SWITCH,
        (osal_u8 *)&open_wmm, OAL_SIZEOF(open_wmm));
    if (OAL_UNLIKELY(l_cfg_rst != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_ccpriv_wmm_switch::return err code [%d]!}", l_cfg_rst);
        return (osal_u32)l_cfg_rst;
    }

    return OAL_SUCC;
}

/* 向对端发送数据帧 */
osal_u32 uapi_ccpriv_packet_xmit(oal_net_device_stru *net_dev, osal_s8 *param)
{
    osal_u32 off_set, ret;
    osal_s8 name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    osal_s32 send_ret;
    mac_cfg_mpdu_ampdu_tx_param_stru *aggr_tx_on_param = OAL_PTR_NULL;
    mac_cfg_mpdu_ampdu_tx_param_stru cfg_aggr_tx_on_param;

    (osal_void)memset_s(&cfg_aggr_tx_on_param, OAL_SIZEOF(cfg_aggr_tx_on_param), 0, OAL_SIZEOF(cfg_aggr_tx_on_param));
    aggr_tx_on_param = &cfg_aggr_tx_on_param;

    ret = wal_cmd_get_u8_with_check_max(&param, WLAN_TID_MAX_NUM - 1, &aggr_tx_on_param->tid);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_ccpriv_packet_xmit::wal_cmd_get_u8_with_check_max err_code [%d]!}", ret);
        return ret;
    }

    ret = wal_get_cmd_one_arg(param, name, OAL_SIZEOF(name), &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_ccpriv_packet_xmit::wal_get_cmd_one_arg err_code [%d]!}", ret);
        return ret;
    }
    param = param + off_set;
    aggr_tx_on_param->packet_num = (osal_u8)oal_atoi((const osal_s8 *)name);

    ret = wal_get_cmd_one_arg(param, name, OAL_SIZEOF(name), &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_ccpriv_packet_xmit::wal_get_cmd_one_arg err_code [%d]!}", ret);
        return ret;
    }
    aggr_tx_on_param->packet_len = (osal_u16)oal_atoi((const osal_s8 *)name);
    /* 长度不小于30 */
    if (aggr_tx_on_param->packet_len < 30) {
        return OAL_FAIL;
    }
    param += off_set;

    /* 获取MAC地址字符串 */
    ret = wal_get_cmd_one_arg(param, name, OAL_SIZEOF(name), &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_ccpriv_packet_xmit::get mac err_code [%d]!}", ret);
        return ret;
    }
    /* 地址字符串转地址数组 */
    oal_strtoaddr((const osal_s8 *)name, aggr_tx_on_param->ra_mac);
    param += off_set;

    /* **************************************************************************
                             抛事件到wal层处理
    ************************************************************************** */
    send_ret = wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(net_dev), WLAN_MSG_W2H_CFG_PACKET_XMIT,
        (osal_u8 *)&cfg_aggr_tx_on_param, OAL_SIZEOF(cfg_aggr_tx_on_param));
    if (OAL_UNLIKELY(send_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_ccpriv_packet_xmit::return err code [%d]!}", send_ret);
        return (osal_u32)send_ret;
    }

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_M2S
STATIC osal_u32 ccpriv_m2s_parse_mss(osal_s8 *param, mac_m2s_mgr_stru *m2s_mgr)
{
    osal_u32 ret;
    /* 1.获取第二参数: 期望切换到的状态 */
    ret = wal_cmd_get_u8_with_check_max(&param, 1, &m2s_mgr->pri_data.mss_mode.mss_on);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_M2S, "{ccpriv_m2s_parse_mss::wal_cmd_get_u8_with_check_max err_code [%d]!}", ret);
        return ret;
    }
    return OAL_SUCC;
}

STATIC osal_u32 ccpriv_m2s_parse_hw_test(osal_s8 *param, mac_m2s_mgr_stru *m2s_mgr)
{
    osal_u32 ret;

    /* 1.获取第二参数: 期望切换到的状态 */
    ret = wal_cmd_get_u8_with_check_max(&param, MAC_M2S_COMMAND_STATE_BUTT - 1,
        &m2s_mgr->pri_data.test_mode.m2s_state);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_M2S, "{ccpriv_m2s_parse_hw_test::wal_cmd_get_u8_with_check_max2 err[%d]!}", ret);
        return ret;
    }

    /* 2.获取第三个参数:主路还是辅路 0为主路 1为辅路 暂时不使用 空缺 */
    ret = wal_cmd_get_u8_with_check_max(&param, HAL_DEVICE_ID_BUTT - 1,
        &m2s_mgr->pri_data.test_mode.master_id);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_M2S, "{ccpriv_m2s_parse_hw_test::wal_cmd_get_u8_with_check_max3 err[%d]!}", ret);
        return ret;
    }

    if (m2s_mgr->cfg_m2s_mode == MAC_M2S_MODE_HW_TEST) {
        /* 硬切换测试模式采用默认软切换配置 */
        m2s_mgr->pri_data.test_mode.m2s_type = WLAN_M2S_TYPE_HW;
    } else {
        /* 业务切换，软切换测试模式采用默认软切换配置 */
        m2s_mgr->pri_data.test_mode.m2s_type = WLAN_M2S_TYPE_SW;
    }

    /* 标识业务类型 */
    if (m2s_mgr->cfg_m2s_mode == MAC_M2S_MODE_DELAY_SWITCH) {
        m2s_mgr->pri_data.test_mode.trigger_mode = WLAN_M2S_TRIGGER_MODE_COMMAND;
    } else {
        m2s_mgr->pri_data.test_mode.trigger_mode = WLAN_M2S_TRIGGER_MODE_TEST;
    }
    return OAL_SUCC;
}

STATIC osal_u32 ccpriv_m2s_parse_rssi(osal_s8 *param, mac_m2s_mgr_stru *m2s_mgr)
{
    osal_u32 ret;
    osal_u32 off_set = 0;
    osal_s8 name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};

    /* 1.获取第二个参数 */
    ret = wal_get_cmd_one_arg(param, name, OAL_SIZEOF(name), &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_M2S, "{ccpriv_m2s_parse_rssi::wal_get_cmd_one_arg2 err_code [%d]!}", ret);
        return ret;
    }

    m2s_mgr->pri_data.rssi_mode.opt = (osal_u8)oal_atoi((const osal_s8 *)name);

    /* 偏移，取下一个参数 */
    param = param + off_set;

    ret = wal_get_cmd_one_arg(param, name, OAL_SIZEOF(name), &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_M2S, "{ccpriv_m2s_parse_rssi::wal_get_cmd_one_arg2 err_code [%d]!}", ret);
        return ret;
    }

    m2s_mgr->pri_data.rssi_mode.value = (osal_s8)oal_atoi((const osal_s8 *)name);
    return OAL_SUCC;
}

/*
 * device下所有vap都要切换到siso或者mimo工作方式，涉及到smps和工作模式通知逻辑
 * ccpriv "Featureid0 set_m2s_switch  0/1/2/3(参数查询/配置模式/切换模式/常发常收测试模式)
 * 0/1(软切换/硬切换) 0/1(siso/mimo) 1/2/3(phy chain) 0/1(rf chain) 1/2(tx single chain)"
 */
osal_u32 uapi_ccpriv_set_m2s_switch(oal_net_device_stru *net_dev, osal_s8 *param)
{
    osal_s32 send_ret;
    osal_u32 ret;
    mac_m2s_mgr_stru *m2s_mgr = OAL_PTR_NULL;
    osal_u32 off_set = 0;
    osal_s8 name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    mac_m2s_mgr_stru m2s_mgr_info;
    (osal_void)memset_s(&m2s_mgr_info, OAL_SIZEOF(m2s_mgr_info), 0, OAL_SIZEOF(m2s_mgr_info));
    m2s_mgr = &m2s_mgr_info;

    /* 1.获取第一个参数: mode */
    ret = wal_get_cmd_one_arg(param, name, OAL_SIZEOF(name), &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_M2S, "{uapi_ccpriv_set_m2s_switch::wal_get_cmd_one_arg1 err_code [%d]!}", ret);
        return ret;
    }

    m2s_mgr->cfg_m2s_mode = (mac_m2s_mode_enum_uint8)oal_atoi((const osal_s8 *)name);

    /* 偏移，取下一个参数 */
    param = param + off_set;

    switch (m2s_mgr->cfg_m2s_mode) {
        /* mimo-siso切换参数查询 */
        case MAC_M2S_MODE_QUERY:
            /* 抛事件dmac打印全局管理参数即可 */
            break;

        case MAC_M2S_MODE_MSS:
            ret = ccpriv_m2s_parse_mss(param, m2s_mgr);
            break;

        case MAC_M2S_MODE_DELAY_SWITCH:
        case MAC_M2S_MODE_SW_TEST:
        case MAC_M2S_MODE_HW_TEST:
            ret = ccpriv_m2s_parse_hw_test(param, m2s_mgr);
            break;

        case MAC_M2S_MODE_RSSI:
            ret = ccpriv_m2s_parse_rssi(param, m2s_mgr);
            break;
        default:
            oam_warning_log1(0, OAM_SF_M2S, "{uapi_ccpriv_set_m2s_switch: cfg_m2s_mode[%d] error!}",
                m2s_mgr->cfg_m2s_mode);
            return OAL_FAIL;
    }

    if (ret != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_M2S, "{uapi_ccpriv_set_m2s_switch::parse mode[%d] error[%d]!}",
            m2s_mgr->cfg_m2s_mode, ret);
        return ret;
    }

    send_ret = wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(net_dev), WLAN_MSG_W2H_C_CFG_SET_M2S_SWITCH,
        (osal_u8 *)&m2s_mgr_info, OAL_SIZEOF(m2s_mgr_info));
    if (OAL_UNLIKELY(send_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_M2S, "{wal_ccpriv_set_m2s_stitch::send event return err[%d].}", send_ret);
        return (osal_u32)send_ret;
    }

    return OAL_SUCC;
}
#endif

#ifdef _PRE_WLAN_FEATURE_ISOLATION
/*****************************************************************************
 修改历史      :
   1.日    期   : 2014年7月29日
     修改内容   : 新生成函数

*****************************************************************************/
OAL_STATIC osal_u32  wal_ccpriv_set_isolation_mode(oal_net_device_stru *net_dev, osal_s8 *pc_param)
{
    return wal_post2hmac_uint32_data(net_dev, pc_param, WLAN_MSG_W2H_CFG_ISOLATION_MODE);
}
/*****************************************************************************
 修改历史      :
   1.日    期   : 2014年7月29日
     修改内容   : 新生成函数

*****************************************************************************/
OAL_STATIC osal_u32  wal_ccpriv_set_isolation_type(oal_net_device_stru *net_dev, osal_s8 *pc_param)
{
    osal_s32                       l_ret;
    osal_u16                      len;
    osal_u32                      ul_ret;
    osal_s8                        ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    osal_u32                      off_set = 0;
    osal_u8                       bss;
    osal_u8                       isolation;
    frw_msg msg_info;
    osal_u8 auc_value[WAL_MSG_WRITE_MAX_LEN];
    (osal_void)memset_s(&msg_info, OAL_SIZEOF(msg_info), 0, OAL_SIZEOF(msg_info));
    (osal_void)memset_s(&auc_value, OAL_SIZEOF(auc_value), 0, OAL_SIZEOF(auc_value));

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &off_set);
    if (ul_ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_ccpriv_set_isolation_type:wal_get_cmd_one_arg fail!}");
        return ul_ret;
    }

    bss = (osal_u8)oal_atoi((const osal_s8 *)ac_name);
    pc_param = pc_param + off_set;

    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &off_set);
    if (ul_ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
                         "{wal_ccpriv_set_isolation_type::wal_get_cmd_one_arg return err_code [%d]!}", ul_ret);
        return ul_ret;
    }

    isolation = (osal_u8)oal_atoi((const osal_s8 *)ac_name);

    len = 2; /* 仅下发2字节数据 */
    /***************************************************************************
                             抛事件到wal层处理
    ***************************************************************************/
    auc_value[0] = bss;
    auc_value[1] = isolation;

    l_ret = wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(net_dev), WLAN_MSG_W2H_CFG_ISOLATION_TYPE,
        (osal_u8 *)&auc_value, len);
    if (OAL_UNLIKELY(l_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_CFG, "{wal_ccpriv_set_isolation_type:wal_sync_post2hmac_no_rsp return[%d].}", l_ret);
        return (osal_u32)l_ret;
    }

    return OAL_SUCC;
}
/*****************************************************************************
 修改历史      :
   1.日    期   : 2014年7月29日
     修改内容   : 新生成函数

*****************************************************************************/
OAL_STATIC osal_u32  wal_ccpriv_set_isolation_fwd(oal_net_device_stru *net_dev, osal_s8 *pc_param)
{
    return wal_post2hmac_uint32_data(net_dev, pc_param, WLAN_MSG_W2H_CFG_ISOLATION_FORWARD);
}
/*****************************************************************************
 修改历史      :
   1.日    期   : 2014年7月29日
     修改内容   : 新生成函数

*****************************************************************************/
OAL_STATIC osal_u32  wal_ccpriv_set_isolation_clear(oal_net_device_stru *net_dev, osal_s8 *pc_param)
{
    return wal_post2hmac_uint32_data(net_dev, pc_param, WLAN_MSG_W2H_CFG_ISOLATION_CLEAR);
}
/*****************************************************************************
 修改历史      :
   1.日    期   : 2014年7月29日
     修改内容   : 新生成函数

*****************************************************************************/
OAL_STATIC osal_u32  wal_ccpriv_set_isolation_show(oal_net_device_stru *net_dev, osal_s8 *pc_param)
{
    return wal_post2hmac_uint32_data(net_dev, pc_param, WLAN_MSG_W2H_CFG_ISOLATION_SHOW);
}
#endif  /* _PRE_WLAN_FEATURE_CUSTOM_SECURITY */

/*****************************************************************************
 功能描述  : mpw设置允许睡眠
*****************************************************************************/
osal_u32 uapi_ccpriv_set_sleep(oal_net_device_stru *net_dev, osal_s8 *param)
{
    osal_u32             offset;
    osal_s32             ret;
    osal_s8              cmd[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    osal_u8              allow_sleep;

    if (wal_get_cmd_one_arg(param, cmd, OAL_SIZEOF(cmd), &offset) != EXT_SUCCESS) {
        oam_warning_log0(0, OAM_SF_ANY, "{uapi_ccpriv_set_sleep::get arg one failed!}");
        return EXT_FAILURE;
    }

    allow_sleep = (osal_u8)(oal_atoi((const osal_s8 *)cmd)) & 0x3;

    ret = wal_sync_send2device_no_rsp(wal_util_get_vap_id(net_dev), WLAN_MSG_W2D_C_CFG_SET_SLEEP,
        (osal_u8 *)&allow_sleep, sizeof(osal_u8));
    if (ret != EXT_SUCCESS) {
        oam_warning_log1(0, OAM_SF_PWR, "{uapi_ccpriv_set_sleep::send msg failed[%d]!}", ret);
    }
    return (osal_u32)ret;
}

#ifdef _PRE_WLAN_FEATURE_SINGLE_PROXYSTA
osal_u32 uapi_ccpriv_single_proxysta_debug(oal_net_device_stru *net_dev, osal_s8 *pc_param)
{
    osal_u32                 ul_ret;
    osal_s32                  l_ret;
    osal_u32                 off_set;
    oal_bool_enum_uint8        type;
    osal_s8                   ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];

    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &off_set);
    if (ul_ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ROAM, "uapi_ccpriv_single_proxysta_debug type return err_code [%d]", ul_ret);
        return ul_ret;
    }
    type = (oal_bool_enum_uint8)oal_atoi((const osal_s8 *)ac_name);
    l_ret = wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(net_dev), WLAN_MSG_W2H_CFG_SINGLE_PROXYSTA,
        (osal_u8 *)&type, OAL_SIZEOF(type));
    if (OAL_UNLIKELY(ul_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_ccpriv_single_proxysta_debug::return err code[%d]!}", l_ret);
        return (osal_u32)l_ret;
    }

    return OAL_SUCC;
}
#endif

#ifdef _PRE_WLAN_FEATURE_11V
/*****************************************************************************
 功能描述  : 命令配置触发STA发送11V Query帧
 输入参数  : 无
 输出参数  : 无

 修改历史      :
  1.日    期   : 2017年3月30日
    修改内容   : 新生成函数

*****************************************************************************/
OAL_STATIC osal_u32  uapi_ccpriv_11v_tx_query(oal_net_device_stru *net_dev, osal_s8 *pc_param)
{
    osal_s32             l_ret;
    osal_u32            ret;
    osal_u32            off_set;
    osal_s8               name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    hmac_trigger_11v_msg_stru   msg;

    (osal_void)memset_s(&msg, OAL_SIZEOF(hmac_trigger_11v_msg_stru), 0, OAL_SIZEOF(hmac_trigger_11v_msg_stru));
    /* 获取mac地址 */
    ret = wal_get_cmd_one_arg(pc_param, name, OAL_SIZEOF(name), &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_ccpriv_11v_tx_query:: get arg 1 return err_code [%d]!}", ret);
        return ret;
    }
    pc_param += off_set;
    oal_strtoaddr((const osal_s8 *)name, (osal_u8 *)&msg.mac_addr_list_0);

    /* 获取mac地址 */
    ret = wal_get_cmd_one_arg(pc_param, name, OAL_SIZEOF(name), &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_ccpriv_11v_tx_query:: get arg 2 return err_code [%d]!}", ret);
        return ret;
    }
    pc_param += off_set;
    msg.chl_num_list_0 = oal_atoi((const osal_s8 *)name);

    /* 获取mac地址 */
    ret = wal_get_cmd_one_arg(pc_param, name, OAL_SIZEOF(name), &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_ccpriv_11v_tx_query:: get arg 3 return err_code [%d]!}", ret);
        return ret;
    }
    pc_param += off_set;
    oal_strtoaddr((const osal_s8 *)name, (osal_u8 *)&msg.mac_addr_list_1);

    /* 获取mac地址 */
    ret = wal_get_cmd_one_arg(pc_param, name, OAL_SIZEOF(name), &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_ccpriv_11v_tx_query:: get arg 4 return err_code [%d]!}", ret);
        return ret;
    }
    pc_param += off_set;
    msg.chl_num_list_1 = (osal_u8)oal_atoi((const osal_s8 *)name);

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    l_ret = wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(net_dev), WLAN_MSG_W2H_CFG_11V_TX_QUERY,
        (osal_u8 *)&msg, OAL_SIZEOF(msg));
    if (OAL_UNLIKELY(l_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_CFG, "{uapi_ccpriv_11v_tx_query::return err code [%d]!}", l_ret);
        return (osal_u32)l_ret;
    }

    return OAL_SUCC;
}
#endif

#ifdef _PRE_WLAN_FEATURE_PROXY_ARP
/*****************************************************************************
 功能描述  : 开启或关闭proxyarp特性
 输入参数  : 无
 输出参数  : 无

 修改历史      :
  1.日    期   : 2014年8月8日
    修改内容   : 新生成函数

*****************************************************************************/
OAL_STATIC osal_u32  wal_ccpriv_proxyarp_on(oal_net_device_stru *net_dev, osal_s8 *pc_param)
{
    osal_u32                       off_set;
    osal_s8                         ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    osal_u32                       ul_ret;
    osal_s32                        l_ret;
    oal_bool_enum_uint8              proxyarp_on;
    mac_proxyarp_en_stru             proxyarp_param;
    (osal_void)memset_s(&proxyarp_param, OAL_SIZEOF(proxyarp_param), 0, OAL_SIZEOF(proxyarp_param));

    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &off_set);
    if (ul_ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_ccpriv_proxyarp_on::get cmd  err_code [%d]!}\r\n", ul_ret);
        return ul_ret;
    }

    proxyarp_on = (osal_u8)oal_atoi((const osal_s8 *)ac_name);
    /***************************************************************************
                             抛事件到wal层处理
    ***************************************************************************/
    /* 设置配置命令参数 */
    proxyarp_param.proxyarp = proxyarp_on;

    l_ret = wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(net_dev), WLAN_MSG_W2H_CFG_PROXYARP_EN,
        (osal_u8 *)&proxyarp_param, OAL_SIZEOF(proxyarp_param));
    if (OAL_UNLIKELY(l_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_ccpriv_proxyarp_on::return err code [%d]!}\r\n", l_ret);
        return (osal_u32)l_ret;
    }

    return OAL_SUCC;
}
#endif /* #ifdef _PRE_WLAN_FEATURE_PROXY_ARP */

#ifdef _PRE_WLAN_FEATURE_WOW_OFFLOAD
/*****************************************************************************
功能描述: 提供WOW_OFFLOAD模块清空收发包统计的接口配置
创建时间: 2022年9月8日
*****************************************************************************/
osal_u32 uapi_ccpriv_clear_wow_offload_info(oal_net_device_stru *net_dev, osal_s8 *pc_param)
{
    osal_u32 ul_ret;
    osal_s32 l_ret;
    osal_u32 off_set;
    oal_switch_enum_uint8 delete_switch;
    osal_s8 ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];

    ul_ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &off_set);
    if (ul_ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_CFG, "uapi_ccpriv_clear_wow_offload_info: %d", ul_ret);
        return ul_ret;
    }
    delete_switch = (oal_switch_enum_uint8)oal_atoi((const osal_s8 *)ac_name);
    l_ret = wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(net_dev), WLAN_MSG_W2H_C_CFG_CLEAR_WOW_OFFLOAD_INFO,
        (osal_u8 *)&delete_switch, OAL_SIZEOF(delete_switch));
    if (OAL_UNLIKELY(l_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_CFG, "{uapi_ccpriv_clear_wow_offload_info:return err code[%d]!}", l_ret);
    }

    return l_ret;
}
#endif

#ifdef _PRE_WLAN_FEATURE_WOW_OFFLOAD
/*
 * 功能描述: DHCP offload DFX打印
 * 创建时间: 2021年5月7日
 */
osal_u32 uapi_ccpriv_show_dhcpoffload_info(oal_net_device_stru *net_dev, osal_s8 *param)
{
    osal_s32                        ret;
    osal_u32                       off_set;
    osal_s8                         cmd_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    osal_u8                            dhcp_offload_type;

    ret = wal_get_cmd_one_arg(param, cmd_name, OAL_SIZEOF(cmd_name), &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_ccpriv_show_dhcpoffload_info: return err_code [%d]!}", ret);
        return ret;
    }

    dhcp_offload_type = (osal_u8)oal_atoi((const osal_s8 *)cmd_name);
    ret = wal_sync_send2device_no_rsp(wal_util_get_vap_id(net_dev), WLAN_MSG_W2D_C_CFG_SHOW_DHCPOFFLOAD_INFO,
        (osal_u8 *)&dhcp_offload_type, OAL_SIZEOF(dhcp_offload_type));
    if (OAL_UNLIKELY(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_PWR, "{uapi_ccpriv_show_dhcpoffload_info::return err code [%d]!}", ret);
        return (osal_u32)ret;
    }

    return OAL_SUCC;
}
/*
 * 功能描述: DHCP offload 设置ip地址命, set_dhcpoffload_info 0-del/1-add [ipv4_addr]
 * 创建时间: 2021年5月7日
 */
osal_u32 uapi_ccpriv_set_dhcpoffload_info(oal_net_device_stru *net_dev, osal_s8 *param)
{
    osal_u32                       off_set;
    osal_s8                         cmd_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    osal_s32                        ret;
    osal_u32                         ipv4_addr = 0;
    mac_ip_oper_enum_uint8           oper;
    mac_ip_addr_config_stru offload_cfg;
    (osal_void)memset_s(&offload_cfg, OAL_SIZEOF(offload_cfg), 0, OAL_SIZEOF(offload_cfg));

    ret = wal_get_cmd_one_arg(param, cmd_name, OAL_SIZEOF(cmd_name), &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_ccpriv_set_dhcpoffload_info: return err_code [%d]!}", ret);
        return ret;
    }
    oper = (osal_u8)oal_atoi((const osal_s8 *)cmd_name);

    param = param + off_set;
    ret = wal_get_cmd_one_arg(param, cmd_name, OAL_SIZEOF(cmd_name), &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_ccpriv_set_dhcpoffload_info: return err_code [%d]!}", ret);
        return ret;
    }
    ret = wal_ccpriv_parse_ipv4(cmd_name, &ipv4_addr);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_ccpriv_set_dhcpoffload_info: return err_code [%d]!}", ret);
        return ret;
    }
    offload_cfg.type = MAC_CONFIG_IPV4;
    offload_cfg.oper = oper;
    offload_cfg.ip.ipv4 = ipv4_addr;
    ret = wal_sync_send2device_no_rsp(wal_util_get_vap_id(net_dev), WLAN_MSG_W2D_C_CFG_SET_DHCPOFFLOAD_INFO,
        (osal_u8 *)&offload_cfg, OAL_SIZEOF(offload_cfg));
    if (OAL_UNLIKELY(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_PWR, "{uapi_ccpriv_show_dhcpoffload_info::return err code [%d]!}", ret);
        return (osal_u32)ret;
    }

    return OAL_SUCC;
}

#endif

#ifdef _PRE_WLAN_FEATURE_WUR_TX
/*****************************************************************************
 功能描述  : 发送WUR Mode Setup帧的调测命令
*****************************************************************************/
osal_u32 uapi_ccpriv_wur_send_setup(oal_net_device_stru *net_dev, osal_s8 *param)
{
    osal_char name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    osal_u32 off_set = 0;
    osal_s32 ret;
    osal_u32 index;
    osal_u32 value;
    osal_u64 value_set[4]; /* 4:用来暂存从命令中读取的wur参数，共有4个 */
    mac_cfg_wur_param_htd_stru wur_cfg_param;

    (osal_void)memset_s(&wur_cfg_param, OAL_SIZEOF(mac_cfg_wur_param_htd_stru), 0,
        OAL_SIZEOF(mac_cfg_wur_param_htd_stru));
    /* 设置WUR发送WUR MODE SETUP帧的配置命令
     * ccpriv "Featureid0 wur_send_setup xx xx xx xx xx xx(mac地址ap)
     * wur_role action_type wur_duty_cycle_serive_period duty_cycle_period " */
    for (index = 0; index < 5; index++) { /* 本命令总计5个参数 */
        param = param + off_set;

        if (wal_get_cmd_one_arg(param, name, OAL_SIZEOF(name), &off_set) != OAL_SUCC) {
            oam_error_log1(0, OAM_SF_11AX, "{uapi_ccpriv_wur_send_setup::get param err, index=[%d].}", index);
            return OAL_ERR_CODE_INVALID_CONFIG;
        }

        if (index == 0) {
            oal_strtoaddr((const osal_s8 *)name, wur_cfg_param.mac_addr); /* 0: mac addr */
            continue;
        }

        value = (osal_u32)oal_atoi((const osal_s8 *)name);
        value_set[index - 1] = value; /* index >= 1使用数组暂存参数，循环结束后一并读取 */
    }

    wur_cfg_param.wur_param_htd.dialog_token = 1;
    wur_cfg_param.wur_param_htd.wur_role = (osal_u16)value_set[0]; /* value_set[0]含wur的角色, 0 STA 1 AP */
    wur_cfg_param.wur_param_htd.action_type = (osal_u16)value_set[1]; /* value_set[1]含action_type */
    wur_cfg_param.wur_param_htd.cycle_service_period = (osal_u32)value_set[2]; /* value_set[2]含cycle_service_period */
    if (value_set[3] > 0xFFFF) { /* value_set[3] 不能大于0xFFFF */
        oam_error_log0(0, OAM_SF_11AX, "{uapi_ccpriv_wur_send_setup::ERR cycle_period too large!}");
        return OAL_FAIL;
    }
    wur_cfg_param.wur_param_htd.cycle_period = (osal_u16)value_set[3]; /* value_set[3]含cycle_period，取值范围0xFFFF */

    /* 抛事件到wal层处理 */
    ret = wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(net_dev), WLAN_MSG_W2H_CFG_WUR_SEND_SETUP,
        (osal_u8 *)&wur_cfg_param, OAL_SIZEOF(wur_cfg_param));
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_11AX, "{uapi_ccpriv_wur_send_setup::return err code[%d]!}", ret);
    }

    return (osal_u32)ret;
}

/*****************************************************************************
 功能描述  : 发送WUR Mode Teardown帧的调测命令
*****************************************************************************/
osal_u32 uapi_ccpriv_wur_send_teardown(oal_net_device_stru *net_dev, osal_s8 *param)
{
    osal_char name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    osal_u32 off_set = 0;
    osal_s32 ret;
    mac_cfg_wur_mode_teardown_param_stru cfg_wur_teardown_param;

    (osal_void)memset_s(&cfg_wur_teardown_param, OAL_SIZEOF(mac_cfg_wur_mode_teardown_param_stru), 0,
        OAL_SIZEOF(mac_cfg_wur_mode_teardown_param_stru));
    /* 设置WUR接收WUR MODE TEARDOWN帧的配置命令
        ccpriv "Featureid0 wur_send_teardown xx xx xx xx xx xx(mac地址)" */
    /* 本命令总计1个参数 */
    if (wal_get_cmd_one_arg(param, name, OAL_SIZEOF(name), &off_set) != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_11AX, "{uapi_ccpriv_wur_send_teardown::get param err.}");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    oal_strtoaddr((const osal_s8 *)name, cfg_wur_teardown_param.mac_addr);

    /* 抛事件到wal层处理 */
    ret = wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(net_dev), WLAN_MSG_W2H_CFG_WUR_SEND_TEARDOWN,
        (osal_u8 *)&cfg_wur_teardown_param, OAL_SIZEOF(cfg_wur_teardown_param));
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_11AX, "{uapi_ccpriv_wur_send_teardown::return err code[%d]!}", ret);
    }

    return (osal_u32)ret;
}

/*****************************************************************************
 功能描述  : 配置WUR TWBTT发WUR BEACON帧、WUR周期开启以及发WUR唤醒帧类型
*****************************************************************************/
osal_u32 uapi_ccpriv_wur_enable_option(oal_net_device_stru *net_dev, osal_s8 *param)
{
    osal_char name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    osal_u32 off_set = 0;
    osal_u32 ret;
    osal_u32 index;
    osal_u32 value;
    osal_u64 value_set[4]; /* 4:用来暂存从命令中读取的wur参数，共有4个 */
    mac_cfg_wur_enable_option_param_stru *wur_enable_option = OSAL_NULL;
    mac_cfg_wur_enable_option_param_stru wur_enable_option_param;

    (osal_void)memset_s(&wur_enable_option_param, OAL_SIZEOF(wur_enable_option_param),
        0, OAL_SIZEOF(wur_enable_option_param));
    wur_enable_option = &wur_enable_option_param;

    /* 设置发送WUR帧的配置命令
       ccpriv "Featureid0 wur_enable_option wur_enable twbtt_bcn_en wur_period_en wur_frame_type" */
    for (index = 0; index < 4; index++) { /* 本命令总计4个参数 */
        param = param + off_set;

        if (wal_get_cmd_one_arg(param, name, OAL_SIZEOF(name), &off_set) != OAL_SUCC) {
            oam_error_log1(0, OAM_SF_11AX, "{uapi_ccpriv_wur_enable_option::get param err, index=[%d].}", index);
            return OAL_ERR_CODE_INVALID_CONFIG;
        }

        value = (osal_u32)oal_atoi((const osal_s8 *)name);
        value_set[index] = value; /* 使用数组暂存参数，循环结束后一并读取 */
    }

    wur_enable_option->wur_enable = (osal_u8)value_set[0]; /* 0:wur_enable */
    wur_enable_option->twbtt_bcn_en = (osal_u8)value_set[1]; /* 1:twbtt_bcn_en */
    if (wur_enable_option->twbtt_bcn_en > 2) { /* 0:关 1:开低速率 2:开高速率 */
        oam_error_log0(0, OAM_SF_11AX, "{uapi_ccpriv_wur_enable_option::ERR twbtt_bcn_en too large!}");
        return OAL_FAIL;
    }
    wur_enable_option->wur_period_en = (osal_u8)value_set[2]; /* 2:wur_period_en */
    wur_enable_option->wur_frame_type = (osal_u8)value_set[3]; /* 3:wur_frame_type */
    if (wur_enable_option->wur_frame_type > 6) { /* 0:按交互信息配置 <=6 强制6种WUR唤醒帧类型 */
        oam_error_log0(0, OAM_SF_11AX, "{uapi_ccpriv_wur_enable_option::ERR wur_frame_type too large!}");
        return OAL_FAIL;
    }

    /* 抛事件到wal层处理 */
    ret = wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(net_dev), WLAN_MSG_W2H_CFG_WUR_ENABLE_OPTION,
        (osal_u8 *)&wur_enable_option_param, OAL_SIZEOF(wur_enable_option_param));
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_11AX, "{uapi_ccpriv_wur_enable_option::return err code[%d]!}", ret);
    }

    return (osal_u32)ret;
}
#endif

/*****************************************************************************
 功能描述  : 设置ru allocation值，常发需求
*****************************************************************************/
osal_u32 uapi_ccpriv_set_ru_allocation(oal_net_device_stru *net_dev, osal_s8 *param)
{
    osal_u32 ret;
    osal_u8 ru_allocation;

    /* 配置ru allocation的值命令 ccpriv "Featureid0 set_ru_alloc xx" */
    /* 判断取值范围，协议只支持0-68 */
    ret = wal_cmd_get_u8_with_check_max(&param, 68, &ru_allocation);
    if (ret != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_11AX, "{uapi_ccpriv_set_ru_allocation::get param err.}");
        return ret;
    }
    ret = wal_sync_send2device_no_rsp(wal_util_get_vap_id(net_dev), WLAN_MSG_W2D_C_CFG_SET_RU_ALLOCATION,
        (osal_u8 *)&ru_allocation, OAL_SIZEOF(ru_allocation));
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_11AX, "{uapi_ccpriv_set_ru_allocation::return err code[%d]!}", ret);
    }

    return ret;
}

#ifdef _PRE_WLAN_FEATURE_APF
/*****************************************************************************
 功能描述  : 将apf规则字符串转为16进制
*****************************************************************************/
OSAL_STATIC osal_u32 uapi_ccpriv_apf_program_parse_hex(const osal_u8 *input, osal_u32 input_len, osal_u8 **output,
    osal_u32 *output_len)
{
    osal_char *end_ptr = OSAL_NULL;
    osal_char tmp_buf[APF_HEX_BYTE_SIZE + 1]; /* 每两个字符转为一个0xaa十六进制数 */
    osal_u32 index;
    osal_u32 half_len = input_len >> 1;
    *output_len = half_len;

    for (index = 0; index < half_len; index++) {
        tmp_buf[0] = input[index * APF_HEX_BYTE_SIZE];
        tmp_buf[1] = input[index * APF_HEX_BYTE_SIZE + 1];
        tmp_buf[APF_HEX_BYTE_SIZE] = '\0';

        (*output)[index] = osal_strtol(tmp_buf, &end_ptr, APF_HEX_NAME);

        if (end_ptr != tmp_buf + APF_HEX_BYTE_SIZE) {
            oam_error_log0(0, OAM_SF_APF, "uapi_ccpriv_apf_program_parse_hex::parse hex fail.");
            return OAL_FAIL;
        }
    }
    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 处理apf规则字符串
*****************************************************************************/
OSAL_STATIC osal_u32 uapi_ccpriv_apf_param_to_program(const osal_u8 *param, osal_s32 param_len, osal_u8 *program,
    osal_u32 *program_len)
{
    const osal_u8 *copy = param;
    osal_u32 position = 0;
    osal_u8 *temp_program = OSAL_NULL;
    osal_u32 ret;

    /* 去掉字符串开始的空格 */
    while (*copy == ' ') {
        copy++;
        param_len--;
    }

    /* 校验输入参数(apf规则)有效长度 */
    if (param_len >= APF_PROGRAM_MAX_LEN || param_len <= 0) {
        oam_error_log0(0, OAM_SF_APF, "uapi_ccpriv_apf_set_filter_list::program len invalid.");
        return OAL_FAIL;
    }

    temp_program = (osal_s8 *)oal_mem_alloc(OAL_MEM_POOL_ID_LOCAL, APF_PROGRAM_MAX_LEN, OSAL_TRUE);
    if (temp_program == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_APF, "uapi_ccpriv_apf_set_filter_list::alloc program fail.");
        return OAL_FAIL;
    }

    /* 复制规则 */
    while ((*copy != ' ') && (*copy != '\0') && position < APF_PROGRAM_MAX_LEN) {
        temp_program[position] = *copy;
        position++;
        copy++;
    }
    temp_program[position] = '\0';

    /* 转为十六进制串 */
    ret = uapi_ccpriv_apf_program_parse_hex(temp_program, position, &program, program_len);
    oal_mem_free(temp_program, OSAL_TRUE);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_APF, "uapi_ccpriv_apf_param_to_program::parse hex fail[%u].", ret);
        return ret;
    }
    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : ccpriv命令 配置apf过滤规则
*****************************************************************************/
osal_u32 uapi_ccpriv_apf_set_filter_list(oal_net_device_stru *net_dev, osal_s8 *param)
{
    mac_apf_filter_cmd_stru apf_filter_cmd = {0};
    osal_u8 *buff_program = OSAL_NULL;
    osal_u32 program_len = 0;
    osal_s32 ret;

    if (net_dev == OSAL_NULL || param == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_APF, "uapi_ccpriv_apf_set_filter_list::net_dev or param is null.");
        return OAL_FAIL;
    }

    if (hwifi_get_apf_enable() == OSAL_FALSE) {
        oam_warning_log1(0, OAM_SF_APF,
            "{uapi_ccpriv_apf_set_filter_list::apf_enable[%hhu], apf isn't supported.}", hwifi_get_apf_enable());
        return OAL_SUCC;
    }

    /* buff_program在hmac处释放 */
    buff_program = (osal_u8 *)oal_mem_alloc(OAL_MEM_POOL_ID_LOCAL, APF_PROGRAM_MAX_LEN, OSAL_TRUE);
    if (buff_program == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_APF, "uapi_ccpriv_apf_set_filter_list::alloc program fail.");
        return OAL_FAIL;
    }

    /* 获取program及长度 */
    ret = uapi_ccpriv_apf_param_to_program((osal_u8 *)g_apf_cmd, strlen(g_apf_cmd), buff_program, &program_len);
    if (ret != OAL_SUCC) {
        oal_mem_free(buff_program, OAL_TRUE);
        oam_error_log1(0, OAM_SF_APF, "{uapi_ccpriv_apf_set_filter_list::parse arg failed [%d].}", ret);
        return OAL_FAIL;
    }

    apf_filter_cmd.cmd_type = APF_SET_FILTER_CMD;
    apf_filter_cmd.program_len = program_len;
    apf_filter_cmd.program = buff_program;
    /***************************************************************************
        抛事件到hmac层处理
    ***************************************************************************/
    ret = wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(net_dev), WLAN_MSG_W2H_CFG_SET_FILTER_LIST,
        (osal_u8 *)&apf_filter_cmd, OAL_SIZEOF(mac_apf_filter_cmd_stru));
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_ANY, "{uapi_ccpriv_apf_set_filter_list::wal_send_cfg_event_etc ret[%d].}", ret);
    }
    oal_mem_free(buff_program, OAL_TRUE);
    return ret;
}

/*****************************************************************************
 功能描述  : ccpriv命令 显示apf filter到hso
*****************************************************************************/
osal_u32 uapi_ccpriv_apf_get_filter_list(oal_net_device_stru *net_dev, osal_s8 *param)
{
    mac_apf_filter_cmd_stru apf_filter_cmd = {0};
    osal_s32 ret;

    if (net_dev == OSAL_NULL || param == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_APF, "wal_ccpriv_apf_get_filter_list::net_dev or param is null.");
        return OAL_FAIL;
    }

    if (hwifi_get_apf_enable() == OSAL_FALSE) {
        oam_warning_log1(0, OAM_SF_APF,
            "{wal_ccpriv_apf_get_filter_list::apf_enable[%hhu], apf isn't supported.}", hwifi_get_apf_enable());
        return OAL_SUCC;
    }

    apf_filter_cmd.cmd_type = APF_GET_FILTER_CMD;
    /***************************************************************************
        抛事件到hmac层处理
    ***************************************************************************/
    ret = wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(net_dev), WLAN_MSG_W2H_CFG_SET_FILTER_LIST,
        (osal_u8 *)&apf_filter_cmd, OAL_SIZEOF(mac_apf_filter_cmd_stru));
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_APF, "{wal_ccpriv_apf_get_filter_list::wal_send_cfg_event_etc ret[%d].}", ret);
    }
    return ret;
}

/*****************************************************************************
 功能描述  : ccpriv命令 设置暗屏
*****************************************************************************/
osal_u32 uapi_ccpriv_apf_set_suspend_mode(oal_net_device_stru *net_dev, osal_s8 *param)
{
    osal_s8 name[WAL_CCPRIV_CMD_NAME_MAX_LEN] = {0};
    osal_u32 off_set = 0;
    osal_s32 value = 0;
    osal_s32 ret;

    if (net_dev == OSAL_NULL || param == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_APF, "uapi_ccpriv_apf_set_suspend_mode::net_dev or param is null.");
        return OAL_FAIL;
    }

    if (hwifi_get_apf_enable() == OSAL_FALSE) {
        oam_warning_log1(0, OAM_SF_APF,
            "{uapi_ccpriv_apf_set_suspend_mode::apf_enable[%hhu], apf isn't supported.}", hwifi_get_apf_enable());
        return OAL_SUCC;
    }

    ret = wal_get_cmd_one_arg_digit(param, name, WAL_CCPRIV_CMD_NAME_MAX_LEN, &off_set, &value);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_APF, "{uapi_ccpriv_apf_set_suspend_mode::wal_get_cmd_one_arg_digit fail[%d].}", ret);
        return OAL_FAIL;
    }

    if (value >= OSAL_SWITCH_BUTT || value < OSAL_SWITCH_OFF) {
        oam_error_log0(0, OAM_SF_APF, "{uapi_ccpriv_apf_set_suspend_mode::param only support 0 or 1.}");
        return OAL_FAIL;
    }
    /***************************************************************************
        抛事件到hmac层处理
    ***************************************************************************/
    ret = wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(net_dev), WLAN_MSG_W2H_CFG_SET_SUSPEND_MODE,
        (osal_u8 *)&value, OAL_SIZEOF(osal_s32));
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_APF, "{uapi_ccpriv_apf_set_suspend_mode::wal_send_cfg_event_etc ret[%d].}", ret);
    }
    return (osal_u32)ret;
}
#endif

#ifdef _PRE_WLAN_FEATURE_11AX
osal_u32 uapi_ccpriv_set_11ax_softap_para(oal_net_device_stru *net_dev, osal_s8 *param)
{
    osal_u32 off_set;
    osal_u32 ret;
    osal_u16 softap_para;
    osal_s8 name[WAL_HIPRIV_CMD_NAME_MAX_LEN];

    /* 设置11ax softap参数     */
    /* 获取使能参数配置开关 */
    ret = wal_get_cmd_one_arg(param, name, OAL_SIZEOF(name), &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_ccpriv_set_11ax_softap_para::wal_get_cmd_one_arg err_code %d}", ret);
        return ret;
    }
    softap_para = (osal_u8)oal_atoi((const osal_s8 *)name);
    param = param + off_set;

    /* 获取bss color配置参数 */
    ret = wal_get_cmd_one_arg(param, name, OAL_SIZEOF(name), &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_ccpriv_set_11ax_softap_para::wal_get_cmd_one_arg err_code %d}", ret);
        return ret;
    }
    softap_para = softap_para << 8; // 8: softap_param高8位标识是否使能参数，低8位标识bss color
    softap_para |= (osal_u8)oal_atoi((const osal_s8 *)name);
    return wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(net_dev), WLAN_MSG_W2H_CFG_SET_11AX_SOFTAP_PARAM,
        (osal_u8 *)&softap_para, OAL_SIZEOF(softap_para));
}
#endif

#ifdef _PRE_WLAN_FEATURE_WFA_SUPPORT
/*****************************************************************************
 功能描述: sigma跑流添加流数
 参数说明：port: 跑流端口号
*****************************************************************************/
osal_u32 uapi_ccpriv_sigma_add_stream(oal_net_device_stru *net_dev, osal_s8 *param)
{
    osal_u32 off_set;
    osal_s32 ret;
    osal_u16 port;
    osal_s8 cmd[WAL_CCPRIV_CMD_NAME_MAX_LEN];

    ret = wal_get_cmd_one_arg(param, cmd, OAL_SIZEOF(cmd), &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_ANY, "{uapi_ccpriv_sigma_add_stream::get flag failed!}");
        return ret;
    }

    port = (osal_u16)oal_atoi((const osal_s8 *)cmd);
    ret = wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(net_dev), WLAN_MSG_W2H_CFG_SIGMA_TRAFFIC_PORT,
        (osal_u8 *)&port, OAL_SIZEOF(port));
    if (osal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_ccpriv_sigma_add_stream::return err code [%d]!}", ret);
    }
    return (osal_u32)ret;
}

/*****************************************************************************
 功能描述: sigma跑流统计开关,并创建proc文件节点
 参数说明：flag: 1 - 开/ 0 - 关
*****************************************************************************/
osal_u32 uapi_ccpriv_sigma_traffic(oal_net_device_stru *net_dev, osal_s8 *param)
{
    osal_u32 off_set;
    osal_s32 ret;
    osal_u8 flag;
    osal_s8 cmd[WAL_CCPRIV_CMD_NAME_MAX_LEN];

    ret = wal_get_cmd_one_arg(param, cmd, OAL_SIZEOF(cmd), &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_ANY, "{uapi_ccpriv_sigma_traffic::get flag failed!}");
        return ret;
    }

    flag = (osal_u8)oal_atoi((const osal_s8 *)cmd);
    flag &= BIT(0);
    ret = wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(net_dev), WLAN_MSG_W2H_CFG_SIGMA_TRAFFIC_SET,
        (osal_u8 *)&flag, OAL_SIZEOF(flag));
    if (osal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_ccpriv_sigma_traffic::return err code [%d]!}", ret);
        return (osal_u32)ret;
    }
    return OAL_SUCC;
}
#endif  /* end of _PRE_WLAN_FEATURE_WFA_SUPPORT */

OAL_CONST wal_ccpriv_cmd_entry_stru  g_ast_ccpriv_cmd_debug_etc[] = {
    /***********************调试命令***********************/
    /* 打印分片门限 */
    {"show_frag_threshold",     uapi_ccpriv_show_frag_threshold},
#ifdef PRE_WLAN_FEATURE_BLACKLIST
    {"blacklist_add",           uapi_ccpriv_blacklist_add},          /* 1 */
    {"blacklist_del",           uapi_ccpriv_blacklist_del},          /* 2 */
    {"blacklist_show",          uapi_ccpriv_blacklist_show},         /* 4 wal_config_blacklist_show */
#endif

#ifdef _PRE_WLAN_FEATURE_WOW_OFFLOAD
    {"set_dhcpoffload_info",    uapi_ccpriv_set_dhcpoffload_info}, /* 设置dhcp offload ip地址 */
    {"wow_event",              uapi_ccpriv_set_wow},                /* wow事件配置: wow_event_set [value] */
    {"wow_pattern",            uapi_ccpriv_set_wow_pattern},        /* 设置wow模式: wow_pattern [clr|add|del] [index] */
#endif

#ifdef _PRE_WLAN_FEATURE_11V
    /* 11v特性配置:  触发sta发送11v Query帧: ccpriv "vap[x] 11v_tx_query [mac-addr]" */
    {"11v_tx_query",  uapi_ccpriv_11v_tx_query},
#endif

#ifdef _PRE_WLAN_FEATURE_APF
    {"set_apf_list", uapi_ccpriv_apf_set_filter_list},
    {"get_apf_list", uapi_ccpriv_apf_get_filter_list},
    {"suspend_mode", uapi_ccpriv_apf_set_suspend_mode},
#endif
#ifdef _PRE_WLAN_FEATURE_M2U
    {"m2u_snoop_enable",        uapi_ccpriv_m2u_snoop_enable},
    {"m2u_snoop_list",          uapi_ccpriv_m2u_snoop_list},
    {"m2u_snoop_deny_table",    uapi_ccpriv_m2u_snoop_deny_table},
    {"m2u_snoop_send_igmp",     uapi_ccpriv_m2u_snoop_send_igmp},
#endif
    {"get_tx_params", uapi_ccpriv_get_tx_params},
    /************************商用对外发布的私有命令*******************/
    /* 设置国家码命令 ccpriv "Featureid0 setcountry param"param取值为大写的国家码字，例如 CN US */
    {"setcountry",              uapi_ccpriv_setcountry},
    {"getcountry",              uapi_ccpriv_getcountry},               /* 查询国家码命令 ccpriv "Featureid0 getcountry" */
    
#ifdef _PRE_WLAN_FEATURE_SNIFFER
    {"sniffer_save_file",       uapi_ccpriv_wifi_sniffer},   /* sh ccpriv.sh 'wlan0 sniffer_save_file [1|0]' */
#endif

#ifdef _PRE_WLAN_DFT_STAT
    {"get_vap_sniffer_info",        uapi_ccpriv_get_vap_sniffer_result}, /* 查询VAP空口实时状况信息 */
    {"enable_external_record",    uapi_ccpriv_external_record_enable},   /* 使能WiFi对外交互记录日志开关 */
#endif
    {"service_control_set",    uapi_ccpriv_service_control_set},
    {"service_control_get",    uapi_ccpriv_service_control_get},
    {"set_cal_rpwr", uapi_ccpriv_set_rate_power},
    {"cal_tone", uapi_ccpriv_set_cal_tone}
};

osal_u32 wal_ccpriv_get_debug_cmd_size_etc(osal_void)
{
    return OAL_ARRAY_SIZE(g_ast_ccpriv_cmd_debug_etc);
}
EXPORT_SYMBOL(uapi_ccpriv_addba_req);
EXPORT_SYMBOL(uapi_ccpriv_delba_req);
EXPORT_SYMBOL(uapi_ccpriv_beacon_chain_switch);
EXPORT_SYMBOL(uapi_ccpriv_ampdu_tx_on);
EXPORT_SYMBOL(uapi_ccpriv_packet_xmit);
EXPORT_SYMBOL(uapi_ccpriv_wmm_switch);
EXPORT_SYMBOL(uapi_ccpriv_set_sleep);
#ifdef _PRE_WLAN_FEATURE_M2S
EXPORT_SYMBOL(uapi_ccpriv_set_m2s_switch);
#endif
#ifdef _PRE_WLAN_FEATURE_WOW_OFFLOAD
EXPORT_SYMBOL(uapi_ccpriv_show_dhcpoffload_info);
EXPORT_SYMBOL(uapi_ccpriv_clear_wow_offload_info);
#endif
#ifdef _PRE_WLAN_FEATURE_WUR_TX
EXPORT_SYMBOL(uapi_ccpriv_wur_send_setup);
EXPORT_SYMBOL(uapi_ccpriv_wur_send_teardown);
EXPORT_SYMBOL(uapi_ccpriv_wur_enable_option);
#endif
#ifdef _PRE_WLAN_FEATURE_SINGLE_PROXYSTA
EXPORT_SYMBOL(uapi_ccpriv_single_proxysta_debug);
#endif
EXPORT_SYMBOL(uapi_ccpriv_set_ru_allocation);
#ifdef _PRE_WLAN_FEATURE_11AX
EXPORT_SYMBOL(uapi_ccpriv_set_11ax_softap_para);
#endif
#ifdef _PRE_WLAN_FEATURE_WFA_SUPPORT
EXPORT_SYMBOL(uapi_ccpriv_sigma_add_stream);
EXPORT_SYMBOL(uapi_ccpriv_sigma_traffic);
#endif
#ifdef _PRE_WLAN_FEATURE_SMPS
EXPORT_SYMBOL(uapi_ccpriv_set_smps_mode);
#endif
EXPORT_SYMBOL(uapi_ccpriv_cw_cfg);
EXPORT_SYMBOL(uapi_ccpriv_aifsn_cfg);
#ifdef _PRE_WLAN_DFT_STAT
EXPORT_SYMBOL(uapi_ccpriv_get_vap_stat);
EXPORT_SYMBOL(uapi_ccpriv_get_vap_aggre_info);
EXPORT_SYMBOL(uapi_ccpriv_get_hw_stat);
EXPORT_SYMBOL(uapi_ccpriv_hw_stat_enable);
EXPORT_SYMBOL(uapi_ccpriv_get_vap_stat_new);
EXPORT_SYMBOL(uapi_ccpriv_vap_stat_enable);
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
