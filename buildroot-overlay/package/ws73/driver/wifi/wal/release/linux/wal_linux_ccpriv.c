/*
 * Copyright (c) CompanyNameMagicTag 2021-2021. All rights reserved.
 * Description: wal ccpriv api.
 * Create: 2021-07-19
 */

#include "wal_linux_ccpriv.h"
#include "oal_kernel_file.h"
#include "oam_struct.h"
#include "wal_linux_vap_proc.h"
#include "wal_linux_netdev.h"
#include "plat_pm_wlan.h"
#include "gpio_ext.h"
#ifdef _PRE_WLAN_FEATURE_ALWAYS_TX
#include "hmac_al_tx_rx.h"
#endif
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
#include "soc_customize_wifi.h"
#endif /* #ifdef _PRE_PLAT_FEATURE_CUSTOMIZE */

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_WAL_LINUX_CCPRIV_C
#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

OAL_STATIC osal_u32  uapi_ccpriv_set_freq(oal_net_device_stru *net_dev, osal_s8 *pc_param);
osal_u32 uapi_ccpriv_set_monitor_switch(oal_net_device_stru *netdev, osal_s8 *pc_param);

#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD

#define SET_APP_IE_CMD_PAYLOAD_LEN (MAC_IE_HDR_LEN + MAC_OUI_LEN + MAC_OUITYPE_LEN)
OSAL_STATIC osal_u32 uapi_ccpriv_set_soft_retry_num(oal_net_device_stru *net_dev, osal_s8 *param);

#ifdef _PRE_WLAN_FEATURE_ALWAYS_TX
static osal_u32 g_phyrate_11b_rate_table[] = { 10, 20, 55, 110 };  /* mcs 0~3 单位100k */

static osal_u32 g_phyrate_11g_rate_table[] = { 60, 90, 120, 180, 240, 360, 480, 540 }; /* mcs 0~7 单位100k */

static osal_u32 g_phyrate_11n_rate_table[] = {
    65, 130, 195, 260, 390, 520, 585, 650, 780, 870 /* mcs 0~9 单位100k */
};

static osal_u32 g_phyrate_11ax_rate_table[] = {
    81, 163, 244, 325, 488, 650, 731, 813, 975, 1083, 1219, 1354 /* mcs 0~11 单位100k */
};

static osal_u32 g_duty_ratio = 0;

OAL_STATIC osal_void uapi_ccpriv_rx_info_detail(hmac_vap_stru *hmac_vap, mac_cfg_al_rx_info_stru al_rx_info)
{
    osal_s32 ret;
    frw_msg cfg_info = {0};
    // 更多信息显示
    if (al_rx_info.print_info != 0) {
        cfg_msg_init((osal_u8 *)&al_rx_info, sizeof(al_rx_info), NULL, 0, &cfg_info);
        ret = send_sync_cfg_to_host(hmac_vap->vap_id, WLAN_MSG_W2H_CFG_GET_RX_PPDU_INFO, &cfg_info);
        if (ret != OAL_SUCC) {
            oam_error_log0(hmac_vap->vap_id, OAM_SF_ANY, "uapi_ccpriv_al_rx_info_detail::send_sync_cfg_to_host err!");
        }
    }
}
 /*****************************************************************************
 功能描述  : 打印对应vap的接收帧的信息
 输入参数  : 无
 输出参数  : 无

 修改历史      :
  1.日    期   : 2014年3月8日
    修改内容   : 新生成函数

*****************************************************************************/
OAL_STATIC  osal_u32  uapi_ccpriv_al_rx_info(oal_net_device_stru *net_dev, osal_s8 *pc_param)
{
    osal_s32                   l_ret;
    osal_u32                  ul_ret;
    mac_cfg_al_rx_info_stru    al_rx_info;  /* 临时保存获取的use的信息 */
    hmac_vap_stru                      *hmac_vap = OAL_PTR_NULL;

    hmac_vap = (hmac_vap_stru *)net_dev->ml_priv;
    if (hmac_vap == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{hwifi_config_init_ini_dual_antenna::dev->ml_priv, return null!}");
        return WLAN_VAP_SUPPORT_MAX_NUM_LIMIT;
    }

    /* 打印接收帧的信息:sh ccpriv.sh "vap0 al_rx_info 0/1 1-4" 0/1  0代表不清除，1代表清除 */
    ul_ret = wal_cmd_get_digit_with_check_max(&pc_param, 1, &al_rx_info.data_op);
    if (ul_ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_ccpriv_al_rx_info::the data_op command is error %d!}", ul_ret);
        return ul_ret;
    }

    /* print_info最大为4，超过则不合法 */
    ul_ret = wal_cmd_get_digit_with_check_max(&pc_param, 4, &al_rx_info.print_info);
    if (ul_ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_ccpriv_al_rx_info::the print_info command is error %d!}", ul_ret);
        return ul_ret;
    }
    l_ret = wal_get_rx_pckg(hmac_vap, al_rx_info.data_op, al_rx_info.print_info);
    wifi_printf_always("al_rx_info::rx succ num[mpdu,ampdu]:[%u,%u] fail num:%u rssi:%d\n",
        hmac_vap->atcmdsrv_get_status.rx_pkct_succ_num, hmac_vap->atcmdsrv_get_status.rx_ampdu_succ_num,
        hmac_vap->atcmdsrv_get_status.rx_fail_num, hmac_vap->atcmdsrv_get_status.s_rx_rssi);
    if (l_ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_ccpriv_al_rx_info:: return err code %d!}", l_ret);
        return l_ret;
    }
    uapi_ccpriv_rx_info_detail(hmac_vap, al_rx_info);
    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 常发设置占空比
 输入参数  : 无
 输出参数  : 无

 修改历史      :
  1.日    期   : 2022年11月05日
    修改内容   : 新生成函数

*****************************************************************************/
OAL_STATIC osal_u32 uapi_ccpriv_set_duty_ratio(oal_net_device_stru *net_dev, osal_s8 *param)
{
    osal_u32 off_set;
    osal_u32 ret;
    osal_s8 name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    osal_u32 ratio;
    osal_s32 value = 0;

    /* 获取常发模式占空比参数 */
    ret = wal_get_cmd_one_arg_digit(param, name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set, &value);
    if (ret != EXT_SUCCESS) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_ccpriv_set_duty_ratio::wal_get_cmd_one_arg err_code [%d]!}", ret);
        return ret;
    }
    ratio = (osal_u32)value;
    if (ratio > WAL_HIPRIV_MAX_DUTY_RATIO) {
        ratio = WAL_HIPRIV_MAX_DUTY_RATIO;
    }
    g_duty_ratio = ratio;
    return EXT_SUCCESS;
}

OAL_STATIC osal_u32 uapi_set_al_tx_rate(osal_u32 protocol, osal_u32 mcs)
{
    osal_u32 rate = 0;

    if (protocol == WLAN_11B_PHY_PROTOCOL_MODE) {
        rate = g_phyrate_11b_rate_table[mcs];
    }

    if (protocol == WLAN_LEGACY_OFDM_PHY_PROTOCOL_MODE) {
        rate = g_phyrate_11g_rate_table[mcs];
    }

    if (protocol == WLAN_HT_PHY_PROTOCOL_MODE) {
        rate = g_phyrate_11n_rate_table[mcs];
    }

    if (protocol == WLAN_HE_SU_FORMAT || protocol == WLAN_HE_EXT_SU_FORMAT) {
        rate = g_phyrate_11ax_rate_table[mcs];
    }
    return rate;
}

OAL_STATIC osal_u32 uapi_get_back_off_time(osal_u32 rate, osal_u32 len, osal_u32 ratio)
{
    osal_u32 tx_time; // unit:us
    osal_u32 back_off_time;

    // slot:11b 20us other:9us
    if (rate == 0) {
        return 0;
    }

    tx_time = (len * 8 * 10) / rate; // (len * 8 *10) *100 * 1000 / (rate * 100 * 1000)
    if (ratio == 0) {
        return 0;
    }
    back_off_time = tx_time * 10 / ratio - tx_time; // ratio 1 -> 10%
    back_off_time = (hmac_get_al_tx_protocol() == 0) ? (back_off_time / WAL_HIPRIV_MAX_LONG_SLOT) :
        (back_off_time / WAL_HIPRIV_SHORT_SLOT);
    back_off_time = (back_off_time > WAL_HIPRIV_MAX_BACK_OFF_TIME) ? WAL_HIPRIV_MAX_BACK_OFF_TIME : back_off_time;
    wifi_printf("len = %d, rate = %d, tx_time = %d, back_off_time=%d \r\n", len, rate, tx_time, back_off_time);
    return back_off_time;
}

OAL_STATIC osal_void uapi_set_al_tx_duty_ratio(oal_net_device_stru *net_dev, osal_u8 tx_flag, osal_u32 len)
{
    osal_u32 ret;
    osal_u32 rate;
    osal_u32 back_off_time;
    osal_s8 *slot_time = WAL_HIPRIV_SET_SHORT_SLOT_ORDER;

    /* 关闭的情况下不需要解析后面的参数 */
    if (tx_flag == HAL_ALWAYS_TX_DISABLE) {
        return;
    }

    /* 常发模式下设置back_off_time */
    if (g_duty_ratio != 0) {
        rate = uapi_set_al_tx_rate(hmac_get_al_tx_protocol(), hmac_get_al_tx_mcs());
        back_off_time = uapi_get_back_off_time(rate, len, g_duty_ratio);
        ret = wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(net_dev), WLAN_MSG_W2H_C_CFG_SET_BACKOFF_DELAY,
            (osal_u8 *)&back_off_time, OAL_SIZEOF(osal_u32));
        if (ret != OAL_SUCC) {
            oam_warning_log1(0, OAM_SF_ANY, "{uapi_set_al_tx_duty_ratio::return err code [%d]!}", ret);
            return;
        }
    } else {
        back_off_time = 0;
    }

    slot_time = (hmac_get_al_tx_protocol() == 0) ? WAL_HIPRIV_SET_LONG_SLOT_ORDER : WAL_HIPRIV_SET_SHORT_SLOT_ORDER;

    oam_warning_log3(0, OAM_SF_ANY, "{uapi_set_al_tx_duty_ratio::ratio=[%d] protocol=[%d] mcs=[%d]}",
        g_duty_ratio, hmac_get_al_tx_protocol(), hmac_get_al_tx_mcs());

    ret = wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(net_dev), WLAN_MSG_W2H_C_CFG_REG_WRITE,
        (osal_u8 *)slot_time, osal_strlen((const osal_s8 *)slot_time) + 1);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_set_al_tx_duty_ratio::return err code [%d]!}", ret);
        return;
    }
}

/*****************************************************************************
 功能描述  : 数据常发
 输入参数  : 无
 输出参数  : 无

 修改历史      :
  1.日    期   : 2014年3月12日
    修改内容   : 新生成函数

*****************************************************************************/
STATIC osal_u32  uapi_ccpriv_always_tx(oal_net_device_stru *net_dev, osal_s8 *param)
{
    osal_u32 off_set;
    osal_u32 ret;
    mac_cfg_tx_comp_stru *set_bcast_param;
    osal_s8 name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    osal_u8 tx_flag;
    mac_rf_payload_enum_uint8 payload_flag = RF_PAYLOAD_RAND;
    osal_u32 len = 1000;

    mac_cfg_tx_comp_stru bcast_param;
    (osal_void)memset_s(&bcast_param, OAL_SIZEOF(bcast_param), 0, OAL_SIZEOF(bcast_param));
    set_bcast_param = &bcast_param;

    /* 获取常发模式开关标志 */
    ret = wal_get_cmd_one_arg(param, name, OAL_SIZEOF(name), &off_set);
    if (ret != EXT_SUCCESS) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_ccpriv_always_tx::wal_get_cmd_one_arg err_code [%d]!}", ret);
        return ret;
    }
    tx_flag = (osal_u8)oal_atoi((const osal_s8 *)name);
    /* 暂不支持聚合常发 */
    if (tx_flag >= HAL_ALWAYS_TX_AMPDU_ENABLE) {
        oam_warning_log0(0, OAM_SF_ANY, "{uapi_ccpriv_always_tx::input should be 0 or 1.}");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    /* 关闭的情况下不需要解析后面的参数 */
    if (tx_flag != HAL_ALWAYS_TX_DISABLE) {
        ret = wal_ccpriv_always_tx_get_param(param, off_set, &payload_flag, &len, set_bcast_param);
        if (ret != OAL_CONTINUE) {
            return ret;
        }
    }

    set_bcast_param->payload_flag = payload_flag;
    set_bcast_param->payload_len = len;
    set_bcast_param->param = tx_flag;

    uapi_set_al_tx_duty_ratio(net_dev, tx_flag, len);

    ret = (osal_u32)wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(net_dev), WLAN_MSG_W2H_C_CFG_SET_ALWAYS_TX,
        (osal_u8 *)&bcast_param, OAL_SIZEOF(mac_cfg_tx_comp_stru));
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_ccpriv_always_tx::return err code [%d]!}", ret);
        return ret;
    }

    if ((set_bcast_param->payload_flag == RF_PAYLOAD_DATA) && (set_bcast_param->payload != OSAL_NULL)) {
        oal_mem_free(set_bcast_param->payload, OAL_TRUE);
        set_bcast_param->payload = OSAL_NULL;
    }
    return EXT_SUCCESS;
}
#endif

OAL_STATIC osal_void wal_mem_write(osal_u8 reg32, uintptr_t start_addr, osal_u32 value)
{
    if (reg32) {
        OAL_REG_WRITE32(start_addr, value);
    } else {
        OAL_REG_WRITE16(start_addr, value);
    }
}

OAL_STATIC osal_u32  uapi_ccpriv_reg_operation(oal_net_device_stru *net_dev, osal_s8 *param,
    osal_u32 type, osal_u32 msg_id)
{
    osal_u32 ret;
    osal_s8 *param_copy = param;
    osal_u32 off_set;
    osal_s8 name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    osal_char *end = OSAL_NULL;
    uintptr_t start_addr;
    osal_u8 reg32 = OSAL_TRUE;

    ret = wal_get_cmd_one_arg(param_copy, name, OAL_SIZEOF(name), &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_ccpriv_reg_operation::get bit width return err_code[%d]!}", ret);
        return ret;
    }
    param_copy = param_copy + off_set;

    reg32 = (osal_strcmp((const osal_s8 *)name, "16") == 0) ? OSAL_FALSE : OSAL_TRUE;
    ret = wal_get_cmd_one_arg(param_copy, name, OAL_SIZEOF(name), &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_ccpriv_reg_operation::get reg type return err_code[%d]!}", ret);
        return ret;
    }
    param_copy = param_copy + off_set;

    /* 读host寄存器(内存) */
    if (osal_strcmp((const osal_s8 *)name, "host") == 0) {
        ret = wal_get_cmd_one_arg(param_copy, name, OAL_SIZEOF(name), &off_set);
        if (ret != OAL_SUCC) {
            oam_warning_log1(0, OAM_SF_ANY, "{uapi_ccpriv_reg_operation:get reg start_addr return err_code[%d]!}", ret);
            return ret;
        }
        param_copy = param_copy + off_set;
        start_addr = (osal_u32)osal_strtol(name, (osal_char **)&end, 16); /* 16表示将字符串转化成16进制数 */

        ret = wal_get_cmd_one_arg(param_copy, name, OAL_SIZEOF(name), &off_set);
        if (ret != OAL_SUCC) {
            oam_warning_log1(0, OAM_SF_ANY, "{uapi_ccpriv_reg_operation::get reg number return err_code[%d]!}", ret);
            return ret;
        }
        /* 10表示将字符串转化成10进制数 */
        if (type == 10) {
            wal_mem_read(reg32, start_addr, (osal_u32)osal_strtol(name, &end, type));
        } else {
            wal_mem_write(reg32, start_addr, (osal_u32)osal_strtol(name, &end, type));
        }

        return OAL_SUCC;
    }
    ret = (osal_u32)wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(net_dev), msg_id,
        (osal_u8 *)param, (osal_u32)(osal_strlen((const osal_s8 *)param) + 1));
    if (OAL_UNLIKELY(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_ccpriv_reg_operation::return err code [%d]!}", ret);
        return ret;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 查询寄存器值(ccpriv "Featureid0 reginfo regtype(soc/mac/phy) startaddr endaddr")
 输入参数  : 无
 输出参数  : 无

 修改历史      :
  1.日    期   : 2013年5月29日
    修改内容   : 新生成函数

*****************************************************************************/
OAL_STATIC osal_u32 uapi_ccpriv_reg_info(oal_net_device_stru *net_dev, osal_s8 *param)
{
    return uapi_ccpriv_reg_operation(net_dev, param, 10, WLAN_MSG_W2H_CFG_REG_INFO); /* 10表示将字符串转化成10进制数 */
}

/*****************************************************************************
 输入参数  : 无
 输出参数  : 无

 修改历史      :
  1.日    期   : 2013年9月6日
    修改内容   : 新生成函数

*****************************************************************************/
OAL_STATIC osal_u32  uapi_ccpriv_reg_write(oal_net_device_stru *net_dev, osal_s8 *param)
{
    return uapi_ccpriv_reg_operation(net_dev, param, 16, WLAN_MSG_W2H_C_CFG_REG_WRITE); /* 16:将字符串转化成16进制数 */
}

/*****************************************************************************
 功能描述  : 设置模式: 包括协议、频段、带宽
 输入参数  : net_dev: net device
             pc_param    : 参数
 输出参数  : 无
 返 回 值  : 错误码

 修改历史      :
  1.日    期   : 2014年12月12日
    修改内容   : 新生成函数

*****************************************************************************/
osal_u32 uapi_ccpriv_set_mode(oal_net_device_stru *net_dev, osal_s8 *param)
{
    osal_s8 mode_str[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};     /* 预留协议模式字符串空间 */
    osal_u32 off_set;
    osal_u32 ret;

    if (OAL_UNLIKELY((net_dev == OAL_PTR_NULL) || (param == OAL_PTR_NULL))) {
        oam_error_log2(0, OAM_SF_ANY, "{uapi_ccpriv_set_mode::net_dev/p_param null [%p] [%p]!}",
            (uintptr_t)net_dev, (uintptr_t)param);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    if (wal_ccpriv_check_cfg_vap(net_dev) == OSAL_TRUE) {
        oam_warning_log0(0, OAM_SF_ANY, "uapi_ccpriv_set_mode vap mode err: cfg");
        return OAL_FAIL;
    }
    /* param指向传入模式参数, 将其取出存放到mode_str中 */
    ret = wal_get_cmd_one_arg(param, mode_str, OAL_SIZEOF(mode_str), &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
                         "{uapi_ccpriv_set_mode::wal_get_cmd_one_arg vap name return err_code %d!}", ret);
        return ret;
    }

    mode_str[OAL_SIZEOF(mode_str) - 1] = '\0';    /* 确保以null结尾 */
    ret = (osal_u32)wal_set_mode(net_dev, g_ast_mode_map_etc, (const osal_s8 *)mode_str);
    if (OAL_UNLIKELY(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_ccpriv_set_mode::wal_set_mode return err code %d!}", ret);
        return ret;
    }

    return OAL_SUCC;
}

OAL_STATIC oal_net_device_stru *uapi_ccpriv_init_net_dev(oal_net_device_stru *cfg_net_dev, osal_s8 *ac_name,
    oal_wireless_dev_stru *wdev)
{
    oal_net_device_stru *net_dev = OAL_PTR_NULL;
    osal_u32 ret;

#if defined(_PRE_WLAN_FEATURE_QUE_CLASSIFY)
    /* 此函数第一个入参代表私有长度，此处不涉及为0 */
    net_dev = oal_net_alloc_netdev_mqs(0, ac_name, oal_ether_setup, WLAN_NET_QUEUE_BUTT, 1);
#else
    net_dev = oal_net_alloc_netdev(0, ac_name, oal_ether_setup); /* 此函数第一个入参代表私有长度，此处不涉及为0 */
#endif
    if (OAL_UNLIKELY(net_dev == OAL_PTR_NULL)) {
        oam_warning_log0(0, OAM_SF_ANY, "{uapi_ccpriv_add_vap::net_dev alloc fail!}");
        return OAL_PTR_NULL;
    }

#ifdef _PRE_WLAN_FEATURE_GSO
    net_dev->features |= NETIF_F_SG;
    net_dev->hw_features |= NETIF_F_SG;
#endif

    /* 对netdevice进行赋值 */
#if defined (_PRE_WLAN_WIRELESS_EXT) && defined(CONFIG_WIRELESS_EXT)
    net_dev->wireless_handlers = &g_st_iw_handler_def_etc;
#endif
    net_dev->netdev_ops = &g_st_wal_net_dev_ops_etc;

    OAL_NETDEVICE_DESTRUCTOR(net_dev) = oal_net_free_netdev;

#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE < KERNEL_VERSION(3,9,0))
    oal_netdevice_master(net_dev) = OAL_PTR_NULL;
#endif

    OAL_NETDEVICE_IFALIAS(net_dev) = OAL_PTR_NULL;
    OAL_NETDEVICE_WATCHDOG_TIMEO(net_dev) = 5; /* watchdog_timeo配置为5 */
    OAL_NETDEVICE_WDEV(net_dev) = wdev;
    oal_netdevice_qdisc(net_dev, OAL_PTR_NULL);
    wdev->netdev = net_dev;

    /* 注册net_device */
    ret = (osal_u32)oal_net_register_netdev(net_dev);
    if (OAL_UNLIKELY(ret != OAL_SUCC)) {
        oal_net_free_netdev(net_dev);
        return OAL_PTR_NULL;
    }
    return net_dev;
}
#ifdef _PRE_WLAN_FEATURE_STA_CHANNEL_SCORING
OAL_STATIC osal_u32 uapi_ccpriv_get_channel_score(oal_net_device_stru *cfg_net_dev, osal_s8 *param)
{
    osal_s32 ret;
    ret = wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(cfg_net_dev),
        WLAN_MSG_W2H_CFG_GET_CHANNEL_SCORING, OSAL_NULL, 0);
    if (OAL_UNLIKELY(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_ccpriv_get_channel_score::return err code %d!}", ret);
        return (osal_u32)ret;
    }
    return OAL_SUCC;
}
#endif
/*****************************************************************************
 功能描述  : 私有命令，创建VAP
 输入参数  : cfg_net_dev: net_device
             pc_param: 参数
 输出参数  : 无
 返 回 值  : 错误码

 修改历史      :
  1.日    期   : 2012年12月10日
    修改内容   : 新生成函数

*****************************************************************************/
OAL_STATIC osal_u32 uapi_ccpriv_add_vap(oal_net_device_stru *cfg_net_dev, osal_s8 *pc_param)
{
    oal_net_device_stru *net_dev;
    osal_u32 ret;
    osal_s32 msg_ret;
    osal_u32 off_set;
    osal_s8 ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    osal_s8 ac_mode[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    wlan_vap_mode_enum_uint8 mode;
    hmac_vap_stru *hmac_vap;
    oal_wireless_dev_stru *wdev;
    hmac_vap_stru *cfg_mac_vap;
    hmac_device_stru *hmac_device;
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    wlan_customize_stru *wlan_customize_etc = (wlan_customize_stru *)hwifi_wlan_customize_etc();
#endif

#ifdef _PRE_WLAN_FEATURE_P2P
    wlan_p2p_mode_enum_uint8    p2p_mode = WLAN_LEGACY_VAP_MODE;
#endif
#if (defined(CONFIG_ATP_FASTIP) && defined(_PRE_WLAN_FASTIP_SUPPORT))
    osal_s32                   fastip_idx;
#endif  /* (defined(CONFIG_ATP_FASTIP) && defined(_PRE_WLAN_FASTIP_SUPPORT)) */
    hmac_cfg_add_vap_param_stru vap_info;
    (osal_void)memset_s(&vap_info, OAL_SIZEOF(vap_info), 0, OAL_SIZEOF(vap_info));

    /* pc_param指向新创建的net_device的name, 将其取出存放到ac_name中 */
    ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_ccpriv_add_vap::wal_get_cmd_one_arg vap name return err_code %d!}",
                         ret);
        return ret;
    }

    /* ac_name length不应超过OAL_IF_NAME_SIZE */
    if (OAL_IF_NAME_SIZE <=  osal_strlen((const osal_s8 *)ac_name)) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_ccpriv_add_vap:: vap name overlength is %d!}", osal_strlen(ac_name));
        /* 输出错误的vap name信息 */
        oal_print_hex_dump((osal_u8 *)ac_name, OAL_IF_NAME_SIZE,
            32, "vap name lengh is overlong:"); /* dump的groupsize为32 */
        return OAL_FAIL;
    }

    pc_param += off_set;

    /* pc_param 指向'ap|sta', 将其取出放到ac_mode中 */
    ret = wal_get_cmd_one_arg(pc_param, ac_mode, OAL_SIZEOF(ac_mode), &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_ccpriv_add_vap::wal_get_cmd_one_arg vap name return err_code %d!}",
                         ret);
        return (osal_u32)ret;
    }

    /* 73规格不支持双sta共存、双ap共存 */
    net_dev = oal_dev_get_by_name(wal_get_wlan_name_config(NETDEV_STA_MAC_INDEX));
    if (net_dev != OAL_PTR_NULL) {
        /* 调用oal_dev_get_by_name后，必须调用oal_dev_put使net_dev的引用计数减一 */
        oal_dev_put(net_dev);
        if ((net_dev->ieee80211_ptr->iftype == NL80211_IFTYPE_STATION &&
            (osal_strcmp("sta", (const osal_s8 *)ac_mode)) == 0) ||
            (net_dev->ieee80211_ptr->iftype == NL80211_IFTYPE_AP &&
            (osal_strcmp("ap", (const osal_s8 *)ac_mode)) == 0)) {
            wifi_printf("uapi_ccpriv_add_vap::the mode param is invalid!\r\n");
            return OAL_FAIL;
        }
    }

    /* 解析ac_mode字符串对应的模式 */
    if ((osal_strcmp("ap", (const osal_s8 *)ac_mode)) == 0) {
        mode = WLAN_VAP_MODE_BSS_AP;
    } else if ((osal_strcmp("sta", (const osal_s8 *)ac_mode)) == 0) {
        mode = WLAN_VAP_MODE_BSS_STA;
#ifdef _PRE_WLAN_FEATURE_P2P
    /* 创建P2P 相关VAP */
    } else if ((osal_strcmp("p2p_device", (const osal_s8 *)ac_mode)) == 0) {
        mode = WLAN_VAP_MODE_BSS_STA;
        p2p_mode = WLAN_P2P_DEV_MODE;
    } else if ((osal_strcmp("p2p_cl", (const osal_s8 *)ac_mode)) == 0) {
        mode = WLAN_VAP_MODE_BSS_STA;
        p2p_mode = WLAN_P2P_CL_MODE;
    } else if ((osal_strcmp("p2p_go", (const osal_s8 *)ac_mode)) == 0) {
        mode = WLAN_VAP_MODE_BSS_AP;
        p2p_mode = WLAN_P2P_GO_MODE;
#endif  /* _PRE_WLAN_FEATURE_P2P */
    } else {
        oam_warning_log0(0, OAM_SF_ANY, "{uapi_ccpriv_add_vap::the mode param is invalid!}");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    /* 如果创建的net device已经存在，直接返回 */
    /* 根据dev_name找到dev */
    net_dev = oal_dev_get_by_name((const osal_s8 *)ac_name);
    if (net_dev != OAL_PTR_NULL) {
        /* 调用oal_dev_get_by_name后，必须调用oal_dev_put使net_dev的引用计数减一 */
        oal_dev_put(net_dev);

        oam_warning_log0(0, OAM_SF_ANY, "{uapi_ccpriv_add_vap::the net_device is already exist!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取mac device */
    cfg_mac_vap = (hmac_vap_stru *)cfg_net_dev->ml_priv;
    hmac_device  = hmac_res_get_mac_dev_etc(cfg_mac_vap->device_id);
    if (OAL_UNLIKELY(hmac_device == OAL_PTR_NULL)) {
        oam_warning_log1(0, OAM_SF_ANY, "vap_id[%d] {uapi_ccpriv_add_vap::hmac_device is null ptr!}",
                         cfg_mac_vap->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    wdev = (oal_wireless_dev_stru *)oal_mem_alloc(OAL_MEM_POOL_ID_LOCAL, OAL_SIZEOF(oal_wireless_dev_stru), OAL_FALSE);
    if (OAL_UNLIKELY(wdev == OAL_PTR_NULL)) {
        oam_error_log1(0, OAM_SF_ANY, "vap_id[%d] {uapi_ccpriv_add_vap::alloc mem, wdev is null ptr!}",
                       cfg_mac_vap->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    (osal_void)memset_s(wdev, sizeof(oal_wireless_dev_stru), 0, sizeof(oal_wireless_dev_stru));

    if (WLAN_VAP_MODE_BSS_AP == mode) {
        wdev->iftype = NL80211_IFTYPE_AP;
    } else if (WLAN_VAP_MODE_BSS_STA == mode) {
        wdev->iftype = NL80211_IFTYPE_STATION;
    }
#ifdef _PRE_WLAN_FEATURE_P2P
    if (WLAN_P2P_DEV_MODE == p2p_mode) {
        wdev->iftype = NL80211_IFTYPE_P2P_DEVICE;
    } else if (WLAN_P2P_CL_MODE == p2p_mode) {
        wdev->iftype = NL80211_IFTYPE_P2P_CLIENT;
    } else if (WLAN_P2P_GO_MODE == p2p_mode) {
        wdev->iftype = NL80211_IFTYPE_P2P_GO;
    }
#endif  /* _PRE_WLAN_FEATURE_P2P */

    wdev->wiphy = hmac_device->wiphy;
    net_dev = uapi_ccpriv_init_net_dev(cfg_net_dev, (osal_s8 *)ac_name, wdev);
    if (net_dev == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{uapi_ccpriv_add_vap::net_dev is null ptr!}");
        oal_mem_free(wdev, OAL_FALSE);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 抛事件之前，尝试开启device，避免一个VAP都没有device下电导致消息不通的情况 */
    wal_wake_lock();
    msg_ret = wlan_open();
    wal_wake_unlock();
    if (msg_ret == OAL_FAIL) {
        oam_error_log0(0, OAM_SF_ANY, "{uapi_ccpriv_add_vap::wlan_open Fail!}\r\n");
        wal_ccpriv_unreg_free_netdev(net_dev, wdev);
        return OAL_FAIL;
    }

    vap_info.net_dev = net_dev;
    vap_info.add_vap.vap_mode = mode;
    vap_info.add_vap.cfg_vap_indx = cfg_mac_vap->vap_id;
#ifdef _PRE_WLAN_FEATURE_P2P
    vap_info.add_vap.p2p_mode = p2p_mode;
#endif
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    vap_info.add_vap.disable_capab_2ght40 = wlan_customize_etc->disable_capab_2ght40;
    wal_add_vap_set_cust(&vap_info);
#endif
    msg_ret = wal_sync_post2hmac_no_rsp(cfg_mac_vap->vap_id, WLAN_MSG_W2H_CFG_ADD_VAP,
        (osal_u8 *)&vap_info, OAL_SIZEOF(vap_info));
    if (OAL_UNLIKELY(msg_ret != OAL_SUCC)) {
        wal_ccpriv_unreg_free_netdev(net_dev, wdev);
        oam_warning_log2(0, OAM_SF_ANY, "vap[%d]{uapi_ccpriv_add_vap::return err %d!}", cfg_mac_vap->vap_id, msg_ret);
        wifi_printf("vap_id[%d] {uapi_ccpriv_add_vap::return err code %d!}\r\n", cfg_mac_vap->vap_id, msg_ret);
        return (osal_u32)msg_ret;
    }
#if defined(_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC) && defined(_PRE_MULTI_CORE_MODE) && \
        (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    if (hmac_device->st_p2p_info.primary_net_device == OAL_PTR_NULL
#ifdef _PRE_WLAN_FEATURE_P2P
    && (p2p_mode == WLAN_LEGACY_VAP_MODE)
#endif
    ) {
        /* 如果创建wlan0， 则保存wlan0 为主net_device,p2p0 和p2p-p2p0 MAC 地址从主netdevice 获取 */
        hmac_device->st_p2p_info.primary_net_device = net_dev;
    }
    if (wal_set_random_mac_to_mib_etc(net_dev) != OAL_SUCC) {
        /* 异常处理，释放内存 */
        wal_ccpriv_unreg_free_netdev(net_dev, wdev);
        return OAL_ERR_CODE_PTR_NULL;
    } /* set random mac to mib */
#endif

    /* 设置netdevice的MAC地址，MAC地址在HMAC层被初始化到MIB中 */
    hmac_vap = (hmac_vap_stru *)net_dev->ml_priv;
#ifdef _PRE_WLAN_FEATURE_P2P
    if (WLAN_P2P_CL_MODE != p2p_mode) {
        hmac_vap->vap_state = MAC_VAP_STATE_INIT;
    }
#else
    hmac_vap->vap_state = MAC_VAP_STATE_INIT;
#endif
    hmac_device->st_p2p_info.second_net_device = net_dev;

#if (defined(CONFIG_ATP_FASTIP) && defined(_PRE_WLAN_FASTIP_SUPPORT))
    fastip_idx = fastip_attach_wifi(net_dev);
    if (-1 != fastip_idx) {
        hmac_vap->fastip_idx = (osal_u32)fastip_idx;
    }
#endif  /* (defined(CONFIG_ATP_FASTIP) && defined(_PRE_WLAN_FASTIP_SUPPORT)) */

    /* 创建VAP对应的proc文件 */
#ifdef _PRE_WLAN_DFT_STAT
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE <= KERNEL_VERSION(2, 6, 34))
    wal_add_vap_proc_file(hmac_vap, (const osal_s8 *)ac_name);
#elif defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 59))
    wal_add_vap_proc_file_sec(hmac_vap, ac_name);
#endif
#endif
    return OAL_SUCC;
}

/* 读管制域最大功率 */
static osal_u32 uapi_ccpriv_get_regdomain_pwr(oal_net_device_stru *net_dev, osal_s8 *param)
{
    osal_s32 ret;
    ret = wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(net_dev), WLAN_MSG_W2H_CFG_GET_REGDOMAIN_PWR, OSAL_NULL, 0);
    if (OAL_UNLIKELY(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_ccpriv_set_freq::return err code %d!}", ret);
        return (osal_u32)ret;
    }
    return OAL_SUCC;
}

OAL_STATIC osal_u32 uapi_ccpriv_adjust_tx_power(oal_net_device_stru *net_dev, osal_s8 *pc_param)
{
    mac_cfg_adjust_tx_power_stru         *adjust_tx_power;
    osal_u32                      off_set;
    osal_s8                       args[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    osal_s32                      ret;

    mac_cfg_adjust_tx_power_stru tx_power;
    (osal_void)memset_s(&tx_power, OAL_SIZEOF(tx_power), 0, OAL_SIZEOF(tx_power));
    adjust_tx_power = &tx_power;

    /* 获取第一个参数 ch */
    ret = wal_get_cmd_one_arg(pc_param, args, OAL_SIZEOF(args), &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_ccpriv_set_extend_ie::parse arg failed [%d]!}", ret);
        return ret;
    }
    pc_param += off_set;
    adjust_tx_power->ch = (osal_u8)oal_atoi((const osal_s8 *)args);

    /* 获取第二个参数 power */
    ret = wal_get_cmd_one_arg(pc_param, args, OAL_SIZEOF(args), &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_ccpriv_set_extend_ie::parse arg failed [%d]!}", ret);
        return ret;
    }
    pc_param = pc_param + off_set;
    adjust_tx_power->power = (osal_s8)oal_atoi((const osal_s8 *)args);

    ret = wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(net_dev), WLAN_MSG_W2H_CFG_ADJUST_TX_POWER,
        (osal_u8 *)&tx_power, OAL_SIZEOF(tx_power));
    if (osal_unlikely(ret != EXT_SUCCESS)) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_ccpriv_adjust_tx_power::return err code [%d]!}", ret);
        return (osal_u32)ret;
    }
    return ret;
}

osal_u32 uapi_ccpriv_tx_power(oal_net_device_stru *net_dev, osal_s8 *param, osal_u32 msg_id)
{
    osal_u32 off_set;
    osal_s8 args[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    osal_u32 ret;
    osal_u8 value;

    /* 获取第一个参数 ch */
    ret = wal_get_cmd_one_arg(param, args, OAL_SIZEOF(args), &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_ccpriv_set_extend_ie::parse arg failed [%d]!}", ret);
        return ret;
    }

    param += off_set;

    value = (osal_u8)oal_atoi((const osal_s8 *)args);
    ret = (osal_u32)wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(net_dev), msg_id, (osal_u8 *)&value,
        OAL_SIZEOF(value));
    if (osal_unlikely(ret != EXT_SUCCESS)) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_ccpriv_tx_power::return err code [%d]!}", ret);
        return ret;
    }
    return ret;
}

OAL_STATIC osal_u32 uapi_ccpriv_restore_tx_power(oal_net_device_stru *net_dev, osal_s8 *param)
{
    return uapi_ccpriv_tx_power(net_dev, param, WLAN_MSG_W2H_CFG_RESTORE_TX_POWER);
}

static osal_void wal_ccpriv_str_to_hex(osal_s8 *param, osal_s16 len, osal_u8 *value)
{
    osal_s16 index;
    if (param == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_ANY, "wal_ccpriv_str_to_hex input param is NULL.");
        return;
    }
    for (index = 0; index < len; index++) {
        /* 每2位字符转为一个十六进制，第一位乘以16 */
        value[index / 2] = (osal_u8)(value[index / 2] * 16 * (index % 2) + oal_strtohex(param));
        param++;
    }
}

static osal_u32 wal_send_custom_pkt(oal_net_device_stru *net_dev, osal_u8 *data, osal_u32 len)
{
    osal_u32 ret;

    if (data == NULL) {
        oam_error_log0(0, 0, "wal_send_custom_pkt data NULL.");
        return OAL_FAIL;
    }

    if ((len < WAL_CCPRIV_CMD_PKT_MIN_LEN) || (len > WAL_CCPRIV_CMD_PKT_MAX_LEN)) {
        oam_error_log1(0, OAM_SF_ANY, "wal_send_custom_pkt invalid len[%d].", len);
        return OAL_FAIL;
    }

    /* 打印构造的数据包信息 */
    oam_warning_log4(0, OAM_SF_ANY, "{wal_send_custom_pkt:: len[%d]::%02x xxxxxxx %02x %02x}", len, data[0], data[1],
        data[len - 1]); /* 打印数据包后1 2个字节的信息 */
    wifi_printf("{wal_send_custom_pkt:: len[%d]::%02x xxxxxxx %02x %02x} \r\n", len, data[0], data[1],
        data[len - 1]); /* 打印数据包后1 2个字节的信息 */

    ret = wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(net_dev), WLAN_MSG_W2H_C_CFG_SEND_CUSTOM_PKT, data, len);
    if (osal_unlikely(ret != OAL_SUCC)) {
        oam_error_log1(0, OAM_SF_ANY, "wal_send_custom_pkt failed ret[%d].", ret);
    }
    return ret;
}

static osal_u32 uapi_ccpriv_send_pkt(oal_net_device_stru *net_dev, osal_s8 *param)
{
    osal_u32 ret;
    osal_u32 off_set = 0;
    osal_s8 *pkt_name;
    osal_u8 *pkt_data = OSAL_NULL;
    osal_u32 pkt_len;
    osal_s8 *temp = (osal_s8 *)param;

    if (net_dev == OSAL_NULL || param == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_ANY, "uapi_ccpriv_send_pkt input net_dev or param is NULL.");
        return OAL_FAIL;
    }
    /* 数组申请内存 */
    pkt_name = (osal_s8 *)oal_mem_alloc(OAL_MGMT_NETBUF, WAL_CCPRIV_CMD_PKT_NAME_MAX_LEN, OAL_NETBUF_PRIORITY_MID);
    if (pkt_name == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{uapi_ccpriv_send_pkt::pkt_name malloc fail!}");
        return OAL_FAIL;
    }
    (osal_void)memset_s(pkt_name, WAL_CCPRIV_CMD_PKT_NAME_MAX_LEN, 0, WAL_CCPRIV_CMD_PKT_NAME_MAX_LEN);
    /* 获取第一个参数 data	*/
    ret = wal_get_cmd_one_arg(temp, pkt_name, WAL_CCPRIV_CMD_PKT_NAME_MAX_LEN, &off_set);
    /* 传入的是十六进制字符，长度为name真实长度的2倍 */
    if ((ret != OAL_SUCC)) {
        oam_warning_log2(0, OAM_SF_ANY, "{wal_get_cmd_one_arg_etc_pkt::err %d!, strlen(%d).}", ret, strlen(pkt_name));
        oal_mem_free(pkt_name, OAL_TRUE);
        return OAL_FAIL;
    }
    /* 获取报文长度等于字符串长度除以2 */
    pkt_len = (osal_u32)strlen(pkt_name) / 2;
    /* 判断最大长度时,避免输入长度为奇数除2后将最后一字节截掉的情况,直接用输入的原始字符串长度比较 */
    if ((pkt_len < WAL_CCPRIV_CMD_PKT_MIN_LEN) || (strlen(pkt_name) > WAL_CCPRIV_CMD_PKT_MAX_LEN * 2)) {
        oam_error_log1(0, OAM_SF_ANY, "uapi_ccpriv_send_pkt invalid input len[%d].", strlen(pkt_name));
        oal_mem_free(pkt_name, OAL_TRUE);
        return OAL_FAIL;
    }
    /*
     * 申请内存存放用户信息，将内存指针作为事件payload抛下去
     * 此处申请的内存在事件处理函数释放(hmac_config_send_custom_pkt)
     */
    pkt_data = oal_mem_alloc(OAL_MGMT_NETBUF, pkt_len, OAL_NETBUF_PRIORITY_MID);
    if (pkt_data == OSAL_NULL) {
        oam_error_log1(0, OAM_SF_ANY, "{uapi_ccpriv_send_pkt::data fail(size:%u), return null!}", pkt_len);
        oal_mem_free(pkt_name, OAL_TRUE);
        return OAL_FAIL;
    }
    (osal_void)memset_s(pkt_data, pkt_len, 0, pkt_len);
    wal_ccpriv_str_to_hex(pkt_name, osal_strlen(pkt_name), pkt_data);
    /* 打印构造的数据包信息 */
    oam_warning_log4(0, OAM_SF_ANY, "{uapi_ccpriv_send_pkt::create pkt:len[%d]::%02x xxxxxxx %02x %02x}", pkt_len,
        pkt_data[0], pkt_data[pkt_len - 2], pkt_data[pkt_len - 1]); /* 打印数据包后1 2个字节的信息 */
    /* 释放数组内存 */
    oal_mem_free(pkt_name, OAL_TRUE);
    /* 抛事件到wal层处理 */
    ret = wal_send_custom_pkt(net_dev, pkt_data, pkt_len);
    if (osal_unlikely(ret != OAL_SUCC)) {
        oam_error_log1(0, OAM_SF_ANY, "uapi_ccpriv_send_pkt failed ret[%d].", ret);
    }
    /* 释放用户侧数据内存 */
    oal_mem_free(pkt_data, OAL_TRUE);
    return ret;
}

#ifdef CONFIG_MFG_GPIO_CTRL
#ifdef _PRE_WLAN_GPIO_SERVICE
/*****************************************************************************
 功能描述  : 设置 GPIO 的 val
*****************************************************************************/
static osal_u32 uapi_ccpriv_set_gpio_val(oal_net_device_stru *net_dev, osal_s8 *param)
{
    osal_u32    off_set;
    osal_s8     cmd[WAL_CCPRIV_CMD_NAME_MAX_LEN];
    osal_u32    ret;
    gpio_param_stru     cfg = {0};

    ret = wal_get_cmd_one_arg(param, cmd, OAL_SIZEOF(cmd), &off_set);
    cfg.id = (osal_u8)oal_atoi((const osal_s8 *)cmd);
    param += off_set;

    ret |= wal_get_cmd_one_arg(param, cmd, OAL_SIZEOF(cmd), &off_set);
    cfg.val = (osal_u8)oal_atoi((const osal_s8 *)cmd);
    if (osal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_ccpriv_set_gpio_val::return err_code [%d]!}", ret);
        return ret;
    }
    wifi_printf("gpio: %d, val: %d.\r\n", cfg.id, cfg.val);

    ret = wal_async_send2device_no_rsp(wal_util_get_vap_id(net_dev), WLAN_MSG_W2D_C_CFG_SET_GPIO_VAL,
        (td_u8 *)&cfg, sizeof(cfg));
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_PWR, "{uapi_ccpriv_set_gpio_val::send msg failed[%d]!}", ret);
        return OAL_FAIL;
    }

    return OAL_SUCC;
}
#endif
#endif

OSAL_STATIC osal_u32 uapi_ccpriv_set_soft_retry_num(oal_net_device_stru *net_dev, osal_s8 *param)
{
    osal_u32 offset;
    osal_s32 ret;
    osal_s8  cmd[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    osal_s32 value = 0;
    mac_cfg_soft_retry_times_stru retry = {0};

    ret = wal_get_cmd_one_arg_digit(param, cmd, WAL_HIPRIV_CMD_NAME_MAX_LEN, &offset, &value);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_ANY, "{uapi_ccpriv_set_soft_retry_num::return err_code [%d]!}", ret);
        return (osal_u32)ret;
    }
    retry.data_retry_times = (osal_u8)value;

    param = param + offset;
    ret = wal_get_cmd_one_arg_digit(param, cmd, WAL_HIPRIV_CMD_NAME_MAX_LEN, &offset, &value);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_ANY, "{uapi_ccpriv_set_soft_retry_num::return err_code [%d]!}", ret);
        return (osal_u32)ret;
    }

    retry.mgmt_retry_times = (osal_u8)value;

    ret = wal_sync_send2device_no_rsp(wal_util_get_vap_id(net_dev), WLAN_MSG_W2D_C_CFG_SET_SOFT_RETRY_TIMES,
        (osal_u8 *)&retry, sizeof(retry));
    if (ret != EXT_SUCCESS) {
        oam_warning_log1(0, OAM_SF_PWR, "{uapi_ccpriv_set_soft_retry_num::send msg failed[%d]!}", ret);
    }
    return (osal_u32)ret;
}

OAL_STATIC OAL_CONST wal_ccpriv_cmd_entry_stru g_ast_ccpriv_cmd[] = {
    {"create", uapi_ccpriv_add_vap}, /* 创建vap私有命令为: ccpriv "Featureid0 create vap0 ap|sta" */
    {"destroy", uapi_ccpriv_del_vap_etc}, /* 删除vap私有命令为: ccpriv "vap0 destroy" */

    {"get_regdomain_pwr", uapi_ccpriv_get_regdomain_pwr},

    {"adjust_tx_power", uapi_ccpriv_adjust_tx_power},
    {"restore_tx_power", uapi_ccpriv_restore_tx_power},
    {"send_custom_pkt", uapi_ccpriv_send_pkt}, /* 发送任意报文命令为:"wlan0 send_custom_pkt data" */
    {"set_soft_retry_num", uapi_ccpriv_set_soft_retry_num}, /* sh ccpriv.sh 'wlan0 set_soft_retry_num 3 5' */
#ifdef _PRE_WLAN_FEATURE_ALWAYS_TX
    /* 设置常发模式:ccpriv "vap0 al_tx <value: 0/1/2>  <len>" 由于mac限制
    11a,b,g下只支持4095以下数据发送,可以使用set_mcast_data对速率进行设置 */
    {"al_tx",                   uapi_ccpriv_always_tx},
    {"set_al_tx_ratio",         uapi_ccpriv_set_duty_ratio}, /* 常发设置占空比 */
    /* 打印接收帧的信息:ccpriv "vap0 al_rx_info 0/1 1/2/3/4" 0/1  0代表不清除，1代表清除 */
    {"al_rx_info",             uapi_ccpriv_al_rx_info},
#endif
    {"freq",                    uapi_ccpriv_set_freq},                    /* 设置AP 信道 */
    {"mode",                    uapi_ccpriv_set_mode},                    /* 设置AP 协议模式 */
    /* 打印寄存器信息: ccpriv "Featureid0 reginfo 16|32(51没有16位寄存器读取功能)regtype(soc/mac/phy) startaddr endaddr" */
    {"reginfo",                 uapi_ccpriv_reg_info},
    /* 打印寄存器信息: ccpriv "Featureid0 regwrite 32/16(51没有16位写寄存器功能)
       regtype(soc/mac/phy) addr val" addr val必须都是16进制0x开头 */
    {"regwrite",                uapi_ccpriv_reg_write},
#ifdef CONFIG_MFG_GPIO_CTRL
#ifdef _PRE_WLAN_GPIO_SERVICE
    {"set_gpio_val",            uapi_ccpriv_set_gpio_val},
#endif
#endif
    {"set_monitor",             uapi_ccpriv_set_monitor_switch}, /* 开关monitor模式:  set_monitor [0|1|2|3|4] */
#ifdef _PRE_WLAN_FEATURE_STA_CHANNEL_SCORING
    {"channel_scoring", uapi_ccpriv_get_channel_score},
#endif
};

#else // _PRE_WLAN_SUPPORT_CCPRIV_CMD
OAL_STATIC OAL_CONST wal_ccpriv_cmd_entry_stru g_ast_ccpriv_cmd[] = {
    {"freq", uapi_ccpriv_set_freq}, /* 设置信道 */
    {"set_monitor", uapi_ccpriv_set_monitor_switch}, /* 开关monitor模式 */
};
#endif // _PRE_WLAN_SUPPORT_CCPRIV_CMD

/*****************************************************************************
 功能描述  : 设置频点/信道
 输入参数  : 无
 输出参数  : 无

 修改历史      :
  1.日    期   : 2014年12月12日
    修改内容   : 新生成函数

*****************************************************************************/
OAL_STATIC osal_u32  uapi_ccpriv_set_freq(oal_net_device_stru *net_dev, osal_s8 *pc_param)
{
    osal_s32                   l_freq;
    osal_u32                  off_set;
    osal_s8                    ac_freq[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    osal_u32                  ul_ret;
    osal_s32                   l_ret;

    /* pc_param指向新创建的net_device的name, 将其取出存放到ac_name中 */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_freq, OAL_SIZEOF(ac_freq), &off_set);
    if (OAL_SUCC != ul_ret) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_ccpriv_set_freq::wal_get_cmd_one_arg vap name return err_code %d!}",
                         ul_ret);
        return ul_ret;
    }

    l_freq = oal_atoi((const osal_s8 *)ac_freq);
    oam_info_log1(0, OAM_SF_ANY, "{uapi_ccpriv_set_freq::l_freq = %d!}", l_freq);

    l_ret = wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(net_dev), WLAN_MSG_W2H_CFG_CURRENT_CHANEL,
        (osal_u8 *)&l_freq, OAL_SIZEOF(l_freq));
    if (OAL_UNLIKELY(OAL_SUCC != l_ret)) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_ccpriv_set_freq::return err code %d!}", l_ret);
        return (osal_u32)l_ret;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 混杂模式命令参数解析
 修改历史      :
  1.日    期   : 2021年4月21日
    修改内容   : 新生成函数
*****************************************************************************/
osal_u32 wal_ccpriv_set_monitor_cmd_proc(osal_s8 *param, osal_u8 *cmd)
{
    osal_u8 cur_para;
    osal_s32 i;
    osal_u32 ret;
    osal_u32 off_set = 0;
    osal_s8 arg_list[4][WAL_HIPRIV_CMD_NAME_MAX_LEN]; /* 4:用于存储四种帧类型的开关参数 */

    for (i = 0; i < 4; ++i) { /* 第1/2/3/4个参数分别是:mdata,udata,mmgmt,umgmt */
        param += off_set;
        ret = wal_get_cmd_one_arg(param, &(arg_list[i][0]), OAL_SIZEOF(arg_list[i]), &off_set);
        if (ret != OAL_SUCC) {
            oam_warning_log2(0, OAM_SF_ANY, "{wal_ccpriv_set_monitor_cmd_proc::parse cmd err=%d, i=%d}", ret, i);
            return OAL_CONTINUE;
        }
        cur_para = (osal_u8)oal_atoi((const osal_s8 *)arg_list[i]);
        if ((cur_para == 0) || (cur_para == 1)) {
            *cmd |= (cur_para << i);
        } else {
            oam_error_log2(0, OAM_SF_ANY, "{wal_ccpriv_set_monitor_cmd_proc::invalid cur_para=%d, i=%d}", cur_para, i);
            return OAL_ERR_CODE_INVALID_CONFIG;
        }
    }

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 开启或关闭monitor模式

 修改历史      :
  1.日    期   : 2016年3月14日
    修改内容   : 新生成函数
*****************************************************************************/
osal_u32 uapi_ccpriv_set_monitor_switch(oal_net_device_stru *netdev, osal_s8 *pc_param)
{
    osal_u32           off_set;
    osal_char          ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    osal_u8            msg_parma[2] = {0}; /* 2：表示msg参数有两个 */
    osal_u8            cmd_para = 0;
    osal_u8            cmd_para_switch = 0;
    osal_u32           ret;

    ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &off_set);
    if (ret != EXT_SUCCESS) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_ccpriv_set_monitor_switch::wal_get_cmd_one_arg fail [%d]}", ret);
        return ret;
    }

    cmd_para_switch = (osal_u8)oal_atoi((const osal_s8 *)ac_name);
    if (cmd_para_switch == 0) {
        /* 关闭混杂模式 */
        cmd_para = 0;
    } else if (cmd_para_switch == 1) {
        /* 开启混杂模式 */
        ret = wal_ccpriv_set_monitor_cmd_proc(pc_param + off_set, &cmd_para);
        if (ret != OAL_SUCC && ret != OAL_CONTINUE) {
            oam_warning_log1(0, OAM_SF_ANY, "{uapi_ccpriv_set_monitor_switch::cmd proc err [%d]}", ret);
            return ret;
        }
    } else if (cmd_para_switch != 2) { /* 2: 打印混杂统计的帧数据 */
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_ccpriv_set_monitor_switch::invalid switch para[%d]}", cmd_para_switch);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    oam_warning_log2(0, OAM_SF_ANY, "{uapi_ccpriv_set_monitor_switch::switch=[%d], cmd_para=[%d]}",
                     cmd_para_switch, cmd_para);

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    msg_parma[0] = cmd_para;
    msg_parma[1] = cmd_para_switch;
    ret = wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(netdev), WLAN_MSG_W2H_CFG_MONITOR_EN,
        (osal_u8 *)&msg_parma, sizeof(msg_parma));
    if (OAL_UNLIKELY(ret != EXT_SUCCESS)) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_ccpriv_set_monitor_switch::return err code [%u]!}", ret);
        return ret;
    }

    return EXT_SUCCESS;
}

/*****************************************************************************
 功能描述  : 查找私有命令表
*****************************************************************************/
OAL_STATIC osal_u32 wal_ccpriv_find_cmd(const osal_s8 *cmd_name, wal_ccpriv_cmd_entry_stru **cmd_id)
{
    osal_u32                cmd_idx;
    int                       l_ret;

    if (OAL_UNLIKELY((cmd_name == OAL_PTR_NULL) || (cmd_id == OAL_PTR_NULL))) {
        oam_error_log2(0, OAM_SF_ANY, "{wal_ccpriv_find_cmd::pc_cmd_name/cmd_id null [%p] [%p]!}",
            (uintptr_t)cmd_name, (uintptr_t)cmd_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    *cmd_id = NULL;
    for (cmd_idx = 0; cmd_idx < OAL_ARRAY_SIZE(g_ast_ccpriv_cmd); cmd_idx++) {
        l_ret = osal_strcmp((const osal_s8 *)g_ast_ccpriv_cmd[cmd_idx].cmd_name, cmd_name);
        if (l_ret == 0) {
            *cmd_id = (wal_ccpriv_cmd_entry_stru *)&g_ast_ccpriv_cmd[cmd_idx];
            oam_warning_log1(0, OAM_SF_ANY, "{wal_ccpriv_find_cmd::wal_ccpriv_find_cmd cmd_idx = %d.}", cmd_idx);
            return OAL_SUCC;
        }
    }

#ifdef _PRE_WLAN_CFGID_DEBUG
    for (cmd_idx = 0; cmd_idx < wal_ccpriv_get_debug_cmd_size_etc(); cmd_idx++) {
        l_ret = osal_strcmp((const osal_s8 *)g_ast_ccpriv_cmd_debug_etc[cmd_idx].cmd_name, cmd_name);
        if (l_ret == 0) {
            *cmd_id = (wal_ccpriv_cmd_entry_stru *)&g_ast_ccpriv_cmd_debug_etc[cmd_idx];
            oam_warning_log1(0, OAM_SF_ANY, "{wal_ccpriv_find_cmd::wal_ccpriv_find_cmd cmd_idx = %d.}", cmd_idx);
            return OAL_SUCC;
        }
    }
#endif

    OAM_IO_PRINTK("cmd name[%s] is not exist. \r\n", cmd_name);
    return OAL_FAIL;
}

/*****************************************************************************
 功能描述  : 获取命令对应的net_dev
 输入参数  : pc_cmd: 命令字符串
 输出参数  : ppst_net_dev: 得到net_device
            pul_off_set: 取net_device偏移的字节
 返 回 值  : 错误码

 修改历史      :
  1.日    期   : 2012年12月18日
    修改内容   : 新生成函数

*****************************************************************************/
OAL_STATIC osal_u32 wal_ccpriv_get_cmd_net_dev(osal_s8 *pc_cmd, oal_net_device_stru **ppst_net_dev,
    osal_u32 *pul_off_set)
{
    oal_net_device_stru  *net_dev;
    osal_s8              ac_dev_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    osal_u32            ul_ret;

    if (OAL_UNLIKELY((OAL_PTR_NULL == pc_cmd) || (OAL_PTR_NULL == ppst_net_dev) || (OAL_PTR_NULL == pul_off_set))) {
        oam_error_log3(0, OAM_SF_ANY, "{wal_ccpriv_get_cmd_net_dev:: null [%p] [%p] [%p]!}",
            (uintptr_t)pc_cmd, (uintptr_t)ppst_net_dev, (uintptr_t)pul_off_set);
        return OAL_ERR_CODE_PTR_NULL;
    }

    ul_ret = wal_get_cmd_one_arg(pc_cmd, ac_dev_name, OAL_SIZEOF(ac_dev_name), pul_off_set);
    if (OAL_SUCC != ul_ret) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_ccpriv_get_cmd_net_dev::wal_get_cmd_one_arg return err_code [%d]!}",
                         ul_ret);
        return ul_ret;
    }

    /* 根据dev_name找到dev */
    net_dev = oal_dev_get_by_name((const osal_s8 *)ac_dev_name);
    if (OAL_PTR_NULL == net_dev) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_ccpriv_get_cmd_net_dev::oal_dev_get_by_name return null ptr!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 调用oal_dev_get_by_name后，必须调用oal_dev_put使net_dev的引用计数减一 */
    oal_dev_put(net_dev);

    *ppst_net_dev = net_dev;

    return OAL_SUCC;
}

static wal_ccpriv_debug_cmd_func g_debug_cmd_func = OSAL_NULL;

osal_void wal_register_debug_cmd(wal_ccpriv_debug_cmd_func cb)
{
    g_debug_cmd_func = cb;
}

osal_void wal_unregister_debug_cmd(osal_void)
{
    g_debug_cmd_func = OSAL_NULL;
}

EXPORT_SYMBOL(wal_register_debug_cmd);
EXPORT_SYMBOL(wal_unregister_debug_cmd);

/*****************************************************************************
 功能描述  : 解析私有配置命令
 输入参数  : pc_cmd: 命令
 输出参数  : 无
 返 回 值  : 错误码

 修改历史      :
  1.日    期   : 2012年12月11日
    修改内容   : 新生成函数

*****************************************************************************/
OAL_STATIC osal_u32 wal_ccpriv_parse_cmd_etc(osal_s8 *pc_cmd)
{
    oal_net_device_stru        *net_dev;
    wal_ccpriv_cmd_entry_stru  *ccpriv_cmd_entry = NULL;
    osal_u32                  off_set = 0;
    osal_u32                  ul_ret;
    hmac_ccpriv_para info = {0};

    /***************************************************************************
        cmd格式约束
        网络设备名 命令      参数   Featureid0 create vap0
        1~15Byte   1~15Byte
    **************************** ***********************************************/
    ul_ret = wal_ccpriv_get_cmd_net_dev(pc_cmd, &net_dev, &off_set);
    if (OAL_SUCC != ul_ret) {
        oam_warning_log1(0, OAM_SF_ANY,
                         "{wal_ccpriv_parse_cmd_etc::wal_ccpriv_get_cmd_net_dev return error code [%d]!}", ul_ret);
        return ul_ret;
    }

    pc_cmd += off_set;
    ul_ret = wal_get_cmd_one_arg(pc_cmd, info.name, OAL_SIZEOF(info.name), &off_set);
    if (OAL_SUCC != ul_ret) {
        oam_warning_log1(0, OAM_SF_ANY,
                         "{wal_ccpriv_parse_cmd_etc::get cmd name return err_code [%d]!}", ul_ret);
        return ul_ret;
    }
    pc_cmd += off_set;
    /* 根据命令名找到命令枚举 */
    ul_ret = wal_ccpriv_find_cmd((const osal_s8 *)info.name, &ccpriv_cmd_entry);
    if (ul_ret == OAL_SUCC) {
        /* 调用命令对应的函数 */
        ul_ret = ccpriv_cmd_entry->func(net_dev, pc_cmd);
        wal_record_external_log_by_name(WLAN_WIFI_HIPRIV_CMD, (osal_u8 *)info.name, ul_ret);
        return ul_ret;
    }

    if (g_debug_cmd_func != NULL) {
        ul_ret = g_debug_cmd_func((const osal_s8 *)info.name, net_dev, pc_cmd);
        if (ul_ret != OAL_ERR_CODE_CCPRIV_CMD_NOT_FOUND) {
            wal_record_external_log_by_name(WLAN_WIFI_DEBUG_CMD, (osal_u8 *)info.name, ul_ret);
            return ul_ret;
        }
    }
#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
    /* 执行单个int类型入参命令通用处理函数 */
    ul_ret = uapi_ccpriv_set_one_arg_s32(net_dev, pc_cmd, (const osal_char *)info.name);
    if (ul_ret == OAL_ERR_CODE_CCPRIV_CMD_NOT_FOUND) {
        info.param = pc_cmd;
        ul_ret = (osal_u32)wal_sync_send2device_no_rsp(wal_util_get_vap_id(net_dev), WLAN_MSG_W2H_CFG_CCPRIV_CMD,
            (osal_u8 *)&info, OAL_SIZEOF(info));
    }
    if (ul_ret != OAL_ERR_CODE_CCPRIV_CMD_NOT_FOUND) {
        wal_record_external_log_by_name(WLAN_WIFI_ONE_INT_ARGS, (osal_u8 *)info.name, ul_ret);
    }
#endif
    return ul_ret;
}

#ifdef _PRE_CONFIG_CONN_EXT_SYSFS_SUPPORT
/*****************************************************************************
 功能描述  : sys write函数
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 处理字节的长度

 修改历史      :
  1.日    期   : 2014年10月17日
    修改内容   : 新生成函数

*****************************************************************************/
#ifdef CONTROLLER_CUSTOMIZATION
ssize_t uapi_ccpriv_sys_write(struct kobject *kobj, struct kobj_attribute *attr, const char *pc_buffer, size_t count)
#else
oal_ssize_t uapi_ccpriv_sys_write(oal_device_stru *dev, oal_device_attribute_stru *attr,
    const char *pc_buffer, oal_size_t count)
#endif
{
    osal_s8  *pc_cmd;
    osal_u32 ul_ret;
    osal_u32 len = (osal_u32)count;

    if (len > WAL_HIPRIV_CMD_MAX_LEN) {
        oam_error_log1(0, OAM_SF_ANY, "{uapi_ccpriv_sys_write::len>WAL_CCPRIV_CMD_MAX_LEN, len [%d]!}", len);
        return -OAL_EINVAL;
    }

    pc_cmd = oal_mem_alloc(OAL_MEM_POOL_ID_LOCAL, WAL_HIPRIV_CMD_MAX_LEN, OAL_TRUE);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pc_cmd)) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_ccpriv_proc_write::alloc mem return null ptr!}");
        return -OAL_ENOMEM;
    }

    memset_s(pc_cmd, WAL_HIPRIV_CMD_MAX_LEN, 0, WAL_HIPRIV_CMD_MAX_LEN);
    if (memcpy_s(pc_cmd, WAL_HIPRIV_CMD_MAX_LEN, pc_buffer, len) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{uapi_ccpriv_sys_write::memcpy_s error}");
    }

    pc_cmd[len - 1] = '\0';

    ul_ret = wal_ccpriv_parse_cmd_etc(pc_cmd);
    if (OAL_SUCC == ul_ret) {
        wifi_printf_always("OK.\n");
    } else {
        wifi_printf_always("INPUT_ERROR or CMD_NOT_FOUND.\n");
        oam_warning_log1(0, OAM_SF_ANY, "{wal_ccpriv_proc_write::parse cmd return err code[%d]!}", ul_ret);
    }

    oal_mem_free(pc_cmd, OAL_TRUE);

    return (osal_s32)len;
}

/*****************************************************************************
 功能描述  : sys read函数 空函数；防止编译警告
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 处理字节的长度

 修改历史      :
  1.日    期   : 2014年10月17日
    修改内容   : 新生成函数

*****************************************************************************/
#define SYS_READ_MAX_STRING_LEN (4096-40)   /* 当前命令字符长度20字节内，预留40保证不会超出 */
#ifdef CONTROLLER_CUSTOMIZATION
ssize_t uapi_ccpriv_sys_read(struct kobject *kobj, struct kobj_attribute *attr, char *pc_buffer)
#else
oal_ssize_t uapi_ccpriv_sys_read(oal_device_stru *dev, oal_device_attribute_stru *attr, char *pc_buffer)
#endif
{
    osal_u32              cmd_idx;
    osal_u32              buff_index = 0;

    for (cmd_idx = 0; cmd_idx < OAL_ARRAY_SIZE(g_ast_ccpriv_cmd); cmd_idx++) {
        buff_index += snprintf_s(pc_buffer + buff_index, (SYS_READ_MAX_STRING_LEN - buff_index),
            (SYS_READ_MAX_STRING_LEN - buff_index), "\t%s\n", g_ast_ccpriv_cmd[cmd_idx].cmd_name);
        if (buff_index > SYS_READ_MAX_STRING_LEN) {
            buff_index += snprintf_s(pc_buffer + buff_index, (SYS_READ_MAX_STRING_LEN - buff_index),
                (SYS_READ_MAX_STRING_LEN - buff_index), "\tmore...\n");
            break;
        }
    }
#ifdef _PRE_WLAN_CFGID_DEBUG
    for (cmd_idx = 0; cmd_idx < wal_ccpriv_get_debug_cmd_size_etc(); cmd_idx++) {
        buff_index += snprintf_s(pc_buffer + buff_index, (SYS_READ_MAX_STRING_LEN - buff_index),
            (SYS_READ_MAX_STRING_LEN - buff_index), "\t%s\n", g_ast_ccpriv_cmd_debug_etc[cmd_idx].cmd_name);
        if (buff_index > SYS_READ_MAX_STRING_LEN) {
            buff_index += snprintf_s(pc_buffer + buff_index, (SYS_READ_MAX_STRING_LEN - buff_index),
                (SYS_READ_MAX_STRING_LEN - buff_index), "\tmore...\n");
            break;
        }
    }
#endif

    return buff_index;
}
#endif


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
