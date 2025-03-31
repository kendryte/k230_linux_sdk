/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: wal ccpriv common api.
 * Create: 2022-03-02
 */
#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
#include "wal_ccpriv_common.h"
#include "wal_common.h"
#include "wlan_msg.h"
#include "oam_ext_if.h"
#include "common_dft.h"
#include "msg_alg_rom.h"
#include "hmac_bsrp_nfrp.h"
#ifdef _PRE_WLAN_GPIO_SERVICE
#include "gpio_ext.h"
#endif
#ifdef _PRE_WLAN_FEATURE_WOW_OFFLOAD
#include "hmac_wow.h"
#endif
#ifdef _PRE_WLAN_LATENCY_STAT
#include "hmac_latency_stat.h"
#endif
#if defined(_PRE_WLAN_FEATURE_SNIFFER)
#include "hmac_sniffer.h"
#endif
#include "hmac_m2u.h"
#ifndef _PRE_WLAN_FEATURE_WS73
#include "debug_print.h"
#endif
#ifdef _PRE_WLAN_SMOOTH_PHASE
#include "msg_smooth_phase_rom.h"
#endif
#include "power_ctrl_spec.h"
#include "frw_timer.h"
#include "frw_thread.h"
#include "hmac_thruput_test.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_WAL_CCPRIV_COMMON_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define WAL_CCPRIV_MAC_ADDR_STR_LEN         18                      /* mac地址字符串长度 12位数字+5个分隔符+1个结束符 */
#define CCPRIV_S16_MAX                      0x7FFF
#define CCPRIV_S32_MAX                      0x7FFFFFFF
typedef struct {
    osal_u8 type;
    osal_u16 min;
    osal_u16 max;
} param_check_stru;

typedef struct {
    const osal_char *cmd_name;
    osal_s32 min;
    osal_s32 max;
    osal_u16 msg_id;
} cmd_one_int_args;

#ifdef _PRE_PLT_PM_DEBUG
/* 平台低功耗调试命令 */
typedef enum {
    PM_DEBUG_ID_START = 0,
    PM_DEBUG_CRG_GET_STATE = 0,
    PM_DEBUG_CRG_SWITCH_ENABLE = 1,
    PM_DEBUG_CRG_SWITCH_DISABLE = 2,
    PM_DEBUG_CRG_WIFI_VOTE_60M = 3,
    PM_DEBUG_CRG_WIFI_VOTE_120M = 4,
    PM_DEBUG_CRG_WIFI_VOTE_240M = 5,
    PM_DEBUG_CRG_BT_VOTE_60M = 6,
    PM_DEBUG_CRG_BT_VOTE_120M = 7,
    PM_DEBUG_CRG_BT_VOTE_240M = 8,
    PM_DEBUG_FSM_GET_STATE = 20,
    PM_DEBUG_FSM_SLEEP_SWITCH_ENABLE = 21,
    PM_DEBUG_FSM_SLEEP_SWITCH_DISABLE = 22,
    PM_DEBUG_FSM_LOG_SWITCH_ENABLE = 23,
    PM_DEBUG_FSM_LOG_SWITCH_DISABLE = 24,
    PM_DEBUG_FSM_DEBUG_SWITCH_ENABLE = 25,
    PM_DEBUG_FSM_DEBUG_SWITCH_DISABLE = 26,
    PM_DEBUG_FSM_PF_VOTE_SWITCH_ENABLE = 27,
    PM_DEBUG_FSM_PF_VOTE_SWITCH_DISABLE = 28,
    PM_DEBUG_FSM_WIFI_VOTE_SD = 29,
    PM_DEBUG_FSM_WIFI_VOTE_DS = 30,
    PM_DEBUG_FSM_WIFI_VOTE_LS = 31,
    PM_DEBUG_FSM_WIFI_VOTE_WK = 32,
    PM_DEBUG_FSM_BT_VOTE_SD = 33,
    PM_DEBUG_FSM_BT_VOTE_DS = 34,
    PM_DEBUG_FSM_BT_VOTE_LS = 35,
    PM_DEBUG_FSM_BT_VOTE_WK = 36,
    PM_DEBUG_FSM_PF_VOTE_SD = 37,
    PM_DEBUG_FSM_PF_VOTE_DS = 38,
    PM_DEBUG_FSM_PF_VOTE_LS = 39,
    PM_DEBUG_FSM_PF_VOTE_WK = 40,
    PM_DEBUG_ID_END = 40
} pm_debug_ctrl_id;
#endif

static const cmd_one_int_args g_one_int_args[] = {
    {"enable_vap_aggre_info", CCPRIV_SWITCH_OFF, CCPRIV_SWITCH_ON, WLAN_MSG_W2H_CFG_VAP_AGGRE_ENABLE},
#ifndef _PRE_WLAN_FEATURE_WS73
    {"tx_dscr_debug_on", CCPRIV_SWITCH_OFF, CCPRIV_SWITCH_ON, WLAN_MSG_W2D_C_CFG_SET_TX_DSCR_DEBUG},
    {"tx_pkt_trace", CCPRIV_SWITCH_OFF, CCPRIV_SWITCH_ON, WLAN_MSG_W2H_C_CFG_SET_TX_DATA_TYPE_DEBUG},
    {"stats_dump", CCPRIV_SWITCH_OFF, CCPRIV_SWITCH_ON, WLAN_MSG_W2H_C_CFG_STATS_DUMP},
    {"frw_dump", CCPRIV_SWITCH_OFF, CCPRIV_SWITCH_ON, WLAN_MSG_W2D_C_CFG_FRW_DUMP},
    {"mac_dump", CCPRIV_SWITCH_OFF, CCPRIV_SWITCH_ON, WLAN_MSG_W2H_C_CFG_MAC_DUMP},
    {"phy_dump", CCPRIV_SWITCH_OFF, CCPRIV_SWITCH_ON, WLAN_MSG_W2H_C_CFG_PHY_DUMP},
#endif
#ifdef _PRE_PLT_PM_DEBUG
    {"pm_debug_ctrl", PM_DEBUG_ID_START, PM_DEBUG_ID_END, WLAN_MSG_W2D_C_CFG_PM_DEBUG},
#endif
#ifdef _PRE_PLAT_PM_API
    {"set_pf_pm_enable", CCPRIV_SWITCH_OFF, CCPRIV_SWITCH_ON, WLAN_MSG_W2D_C_CFG_PM_ENABLE},
#endif
#ifdef _PRE_WLAN_FEATURE_CENTRALIZE
    {"tx_sched_timeout", CCPRIV_SWITCH_OFF, CCPRIV_S16_MAX, WLAN_MSG_W2D_C_CFG_SET_TX_SCHED_TIMEOUT},
    {"netbuf_ram_max", CCPRIV_SWITCH_OFF, CCPRIV_S32_MAX, WLAN_MSG_W2D_C_CFG_SET_NETBUF_RAM_MAX},
#endif
    {"frag_threshold", WLAN_FRAG_THRESHOLD_MIN, WLAN_FRAG_THRESHOLD_MAX, WLAN_MSG_W2H_CFG_FRAG_THRESHOLD_REG},
#ifdef _PRE_WLAN_FEATURE_WOW_OFFLOAD
    /* ARP/ND处理下移和广播/组播过滤开关 */
    {"arp_offload_enable", CCPRIV_SWITCH_OFF, CCPRIV_SWITCH_ON, WLAN_MSG_W2H_C_CFG_ENABLE_ARP_OFFLOAD},
    {"arp_offload_free_arp_interval", CCPRIV_SWITCH_OFF, CCPRIV_S16_MAX, WLAN_MSG_W2H_C_CFG_FREE_ARP_INTERVAL},
    {"set_dhcpoffload_enable", CCPRIV_SWITCH_OFF, CCPRIV_SWITCH_ON, WLAN_MSG_W2H_C_CFG_SET_DHCPOFFLOAD_SWITCH},
    {"wow_show_wakeup_reason", CCPRIV_SWITCH_OFF, CCPRIV_SWITCH_ON, WLAN_MSG_W2H_C_CFG_WOW_SHOW_WAKEUP_REASON},
#ifndef _PRE_WLAN_FEATURE_DYNAMIC_OFFLOAD
    /* 配置host睡眠开关: host_sleep [0|1] */
    {"wow_sleep", CCPRIV_SWITCH_OFF, CCPRIV_SWITCH_ON, WLAN_MSG_W2H_CFG_HOST_SLEEP_EN},
#else
    {"wow_sleep", CCPRIV_SWITCH_OFF, CCPRIV_SWITCH_ON, WLAN_MSG_W2H_CFG_WOW_LOAD},
#endif
#endif
#ifdef _PRE_WLAN_FEATURE_APF
    {"force_stop_apf", CCPRIV_SWITCH_OFF, CCPRIV_SWITCH_ON, WLAN_MSG_W2H_CFG_FORCE_STOP_FILTER},
#endif
#ifdef _PRE_WLAN_FEATURE_WS73
#ifdef _PRE_WLAN_FFD
    /* frame_pn_enable和时延统计特性互斥, echo "wlan0 enable_frame_pn 0/1" > /sys/ccsys/ccpriv" */
    {"enable_frame_pn", CCPRIV_SWITCH_OFF, CCPRIV_SWITCH_ON, WLAN_MSG_W2H_CFG_FRAG_PN_ENABLE},
#endif
#ifdef _PRE_WLAN_FEATURE_ALWAYS_TX
    /* 设置常收模式: ccpriv "vap0 al_rx <value: 0/1/2>" */
    {"al_rx", CCPRIV_SWITCH_OFF, HAL_ALWAYS_RX_RESERVED, WLAN_MSG_W2H_C_CFG_SET_ALWAYS_RX},
#endif
#endif
    /* 设置黑白名单类型 */
    {"blacklist_mode", CS_BLACKLIST_MODE_NONE, CS_BLACKLIST_MODE_WHITE, WLAN_MSG_W2H_CFG_BLACKLIST_MODE},
#ifdef _PRE_WLAN_FEATURE_11D
    /* 设置是否根据关联ap更新国家码信息 ccpriv "Featureid0 set_rd_by_ie_switch 0/1" */
    {"set_rd_by_ie_switch", CCPRIV_SWITCH_OFF, CCPRIV_SWITCH_ON, WLAN_MSG_W2H_CFG_SET_RD_IE_SWITCH},
#endif
    /* 设置管制域最大发送功率(可以突破管制域的限制) 范围(1,100]，set_regdomain_pwr_p 20",单位dBm */
    {"set_regdomain_pwr_p", 1, 100, WLAN_MSG_W2H_CFG_REGDOMAIN_PWR}
};

osal_u32 uapi_ccpriv_set_one_arg_s32(oal_net_device_stru *net_dev, osal_s8 *param, const osal_char *cmd)
{
    osal_s32 ret, value;
    osal_u32 id;
    osal_u32 array_size = (osal_u32)uapi_array_size(g_one_int_args);
    const cmd_one_int_args *item = OSAL_NULL;

    for (id = 0; id < array_size; id++) {
        if (osal_strcmp(g_one_int_args[id].cmd_name, cmd) == 0) {
            item = &(g_one_int_args[id]);
            break;
        }
    }
    if (item == OSAL_NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{uapi_ccpriv_set_one_arg_s32::cmd not found.}");
        return OAL_ERR_CODE_CCPRIV_CMD_NOT_FOUND;
    }
    ret = wal_cmd_get_digit_with_range(&param, item->min, item->max, &value);
    if (ret != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_CFG, "{uapi_ccpriv_set_one_arg_s32::msgid[%d] check para error[%d].}",
            item->msg_id, ret);
        return (osal_u32)ret;
    }
    if (item->msg_id < WLAN_MSG_W2D_C_END) {
        ret = wal_sync_send2device_no_rsp(wal_util_get_vap_id(net_dev), item->msg_id, (osal_u8 *)&value,
            OAL_SIZEOF(value));
    } else {
        ret = wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(net_dev), item->msg_id, (osal_u8 *)&value,
            OAL_SIZEOF(value));
    }
    if (ret != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_ANY, "{uapi_ccpriv_set_one_arg_s32::msgid[%d] send msg error[%d]!}", item->msg_id,
            ret);
        return (osal_u32)ret;
    }
    wifi_printf("OK, value[%d]\r\n", value);
    return OAL_SUCC;
}

/* 检查netdev是否为配置vap */
osal_u8 wal_ccpriv_check_cfg_vap(oal_net_device_stru *net_dev)
{
    hmac_vap_stru *hmac_vap = OSAL_NULL;

    if (net_dev == OSAL_NULL) {
        return OSAL_FALSE;
    }
    hmac_vap = net_dev->ml_priv;

    if (hmac_vap == OSAL_NULL) {
        return OSAL_FALSE;
    }
    if (hmac_vap->vap_mode == WLAN_VAP_MODE_CONFIG) {
        return OSAL_TRUE;
    }
    return OSAL_FALSE;
}

static osal_u32 wal_ccpriv_check_mac_addr(osal_s8 *addr, osal_u32 len)
{
    osal_u32 i;
    osal_s8 *addr_tmp = addr;

    if (len != WAL_CCPRIV_MAC_ADDR_STR_LEN) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_ccpriv_check_mac_addr::mac string len [%d] error !}", len);
        return OAL_ERR_CODE_CONFIG_UNSUPPORT;
    }
    for (i = 0; i < WAL_CCPRIV_MAC_ADDR_STR_LEN - 1; i++) {
        /* 校验分隔符 */
        if (i % 3 == 2) { /* MAC地址格式XX:XX:XX:XX:XX:XX, 每隔3位一个分隔符，位数对3求余为2 */
            if ((*addr_tmp == ':') || (*addr_tmp == '-')) { /* 分格符强制要求为“:”或者“-” */
                addr_tmp++;
                continue;
            }
            return OAL_ERR_CODE_CONFIG_UNSUPPORT;
        }
        /* 校验数字 */
        if (oal_strtohex(addr_tmp) > 0xf) {
            return OAL_ERR_CODE_CONFIG_UNSUPPORT;
        }
        addr_tmp++;
    }
    return OAL_SUCC;
}

osal_u32 wal_ccpriv_get_mac_addr_etc(osal_s8 *param, osal_u8 mac_addr[], osal_u32 *total_offset)
{
    osal_u32 off_set = 0;
    osal_u32 ret = OAL_SUCC;
    osal_s8 name[WAL_CCPRIV_CMD_NAME_MAX_LEN] = {0};

    /* 获取mac地址 */
    ret = wal_get_cmd_one_arg(param, name, WAL_CCPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_ccpriv_get_mac_addr_etc::wal_get_cmd_one_arg return err_code [%d]!}",
            ret);
        return ret;
    }
    ret = wal_ccpriv_check_mac_addr(name, off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_ccpriv_get_mac_addr_etc::wal_ccpriv_check_mac_addr return err [%d]!}",
            ret);
        return ret;
    }
    oal_strtoaddr((const osal_char *)name, mac_addr);

    *total_offset = off_set;

    return OAL_SUCC;
}

osal_u32 wal_get_user_by_mac(oal_net_device_stru *net_dev, osal_s8 *param, osal_u16 *user_idx)
{
    hmac_vap_stru *hmac_vap = OAL_PTR_NULL;
    osal_s32 l_ret;
    osal_u8 auc_mac_addr[6] = {0};
    osal_u8 char_index;
    osal_s8 *param_tmp = param;

    /* 去除字符串的空格 */
    param_tmp++;

    /* 获取mac地址,16进制转换 */
    for (char_index = 0; char_index < 12; char_index++) { /* mac地址长度共12位字符 */
        if (*param_tmp == ':') {
            param_tmp++;
            if (char_index != 0) {
                char_index--;
            }

            continue;
        }
        auc_mac_addr[char_index / 2] = /* mac地址长度共12位字符，每2位存为1个uint8 */
            (osal_u8)(auc_mac_addr[char_index / 2] * 16 * (char_index % 2) + /* mac地址12位字符，每2位存为1个uint8 ，16进制 */
                        oal_strtohex(param_tmp));
        param_tmp++;
    }

    /* 根据mac地址找用户 */
    hmac_vap = (hmac_vap_stru *)net_dev->ml_priv;

    l_ret = (osal_s32)hmac_vap_find_user_by_macaddr_etc(hmac_vap, (osal_u8 *)auc_mac_addr, user_idx);
    if (l_ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_get_user_by_mac::no such user!}\r\n");
        return OAL_FAIL;
    }
    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 发送随机mac oui到hmac
*****************************************************************************/
osal_u32 uapi_ccpriv_send_random_mac_oui(oal_net_device_stru *net_dev, osal_s8 *param)
{
    osal_u32 ret;
    osal_u32 offset;
    osal_u8 random_mac_oui[WLAN_RANDOM_MAC_OUI_LEN] = {0};
    osal_s8 arg[WAL_CCPRIV_CMD_NAME_MAX_LEN] = {0};
    osal_u8 i;
    osal_s8 *param_tmp = param;

    /* AT+CCPRIV=wlan0,random_mac_oui,0xff,0x22,0x33 命令处理入口会将 逗号 转换为 空格 */
    /* echo "wlan0 random_mac_oui 0xff 0x22 0x33" > /sys/ccsys/ccpriv 要求参数以空格分隔, 因为命令处理入口未做转换 */
    for (i = 0; i < WLAN_RANDOM_MAC_OUI_LEN; i++) {
        /* 获取命令参数 */
        ret = wal_get_cmd_one_arg(param_tmp, arg, WAL_CCPRIV_CMD_NAME_MAX_LEN, &offset);
        if (ret != OAL_SUCC) {
            oam_warning_log2(0, OAM_SF_ANY, "uapi_ccpriv_send_random_mac_oui:wal_get_cmd_one_arg err[%d][%d]", i, ret);
            return ret;
        }
        param_tmp += offset;
        random_mac_oui[i] = (osal_u8)osal_strtol((const osal_char *)arg, NULL, 16); /* 16进制参数解析 */
    }
    ret = (osal_u32)wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(net_dev), WLAN_MSG_W2H_CFG_SET_RANDOM_MAC_OUI,
        random_mac_oui, WLAN_RANDOM_MAC_OUI_LEN);
    if (OAL_UNLIKELY(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_ccpriv_send_random_mac_oui::return err code[%u]!}", ret);
        return ret;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 删除user的配置参数信息
*****************************************************************************/
osal_u32 uapi_ccpriv_kick_user(oal_net_device_stru *netdev, osal_s8 *param)
{
    osal_u32 ret;
    osal_s8 *param_tmp = param;
    osal_u8 mac_addr[6] = {0};    /* 临时保存获取的use的mac地址信息 */
    mac_cfg_kick_user_param_stru kick_user_param = {0};

    if (*param_tmp == ' ') {
        param_tmp++;
    }
    oal_strtoaddr((const osal_char *)param_tmp, (osal_u8 *)mac_addr);

    param_tmp += WLAN_MAC_ADDR_BYTE_LEN;
    /* 判断命令长度是否有问题 */
    if (*param_tmp != '\0') {
        oam_warning_log0(0, OAM_SF_ANY, "{uapi_ccpriv_kick_user::cmd len error!}");
        return OAL_FAIL;
    }

    /* 设置配置命令参数 */
    if (memcpy_s(kick_user_param.mac_addr, WLAN_MAC_ADDR_LEN, (osal_u8*)(mac_addr), WLAN_MAC_ADDR_LEN) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{uapi_ccpriv_kick_user::mem safe function err!}");
        return OAL_FAIL;
    }
    /* 填写去关联reason code */
    kick_user_param.reason_code = MAC_UNSPEC_REASON;

    ret = (osal_u32)wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(netdev), WLAN_MSG_W2H_CFG_KICK_USER,
        (osal_u8 *)&kick_user_param, OAL_SIZEOF(mac_cfg_kick_user_param_stru));
    if (OAL_UNLIKELY(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_ccpriv_kick_user::return err code [%u]!}\r\n", ret);
        return OAL_FAIL;
    }
    wifi_printf("OK\r\n");

    return OAL_SUCC;
}

/*
* 功能描述  : 扫描参数范围校验
*/
osal_u32 wal_ccpriv_check_scan_params(mac_cfg_set_scan_param *scan_param)
{
    osal_u8 i, table_size;
    param_check_stru param_check_table[] = {
        {SCAN_PARAM_COUNT, 1, 10},          /* 设置有效参数 scan_cnt:1~10  , 73不包含0 */
        {SCAN_PARAM_STAY_TIME, 20, 120},    /* 设置有效参数 scan_time:20~120, 73不包含0 */
        {SCAN_PARAM_INTERVAL, 1, 6},        /* 设置有效参数 scan_channel_interval:1~6, 73不包含0 */
        {SCAN_PARAM_ON_HOME_TIME, 30, 120}  /* 设置有效参数 work_time_on_home_channel:30~120, 73不包含0 */
    };

    if (scan_param == OSAL_NULL) {
        return OAL_FAIL;
    }

    table_size = (osal_u8)(OAL_SIZEOF(param_check_table) / OAL_SIZEOF(param_check_table[0]));
    for (i = 0; i < table_size; i++) {
        if (scan_param->type != param_check_table[i].type) {
            continue;
        }

        if ((scan_param->value < param_check_table[i].min) || (scan_param->value > param_check_table[i].max)) {
            oam_warning_log2(0, OAM_SF_ANY, "{wal_ccpriv_check_scan_params:: type[%d] value[%d] failed.}",
                scan_param->type, scan_param->value);
            wifi_printf("{wal_ccpriv_check_scan_params:: type[%d] value[%d] failed.}\r\n",
                scan_param->type, scan_param->value);
            return OAL_FAIL;
        }
        break;
    }

    /* 其他类型参数 不需要在此校验 */
    if (i >= table_size) {
        oam_warning_log2(0, OAM_SF_ANY, "{wal_ccpriv_check_scan_params:type[%d] value[%d] no need check, return succ.}",
            scan_param->type, scan_param->value);
        wifi_printf("{wal_ccpriv_check_scan_params:: type[%d] value[%d] no need check, return succ.}\r\n",
            scan_param->type, scan_param->value);
        return OAL_SUCC;
    }

    /* 扫描总时间不能超过4.5秒, 不在此校验, 由hmac扫描超时定时器处理 */
    return OAL_SUCC;
}

#ifdef _PRE_WLAN_CFGID_DEBUG
/* 业务控制开关, 32bits变量, 支持32种场景 */
osal_u32 uapi_ccpriv_service_control_set(oal_net_device_stru *net_dev, osal_s8 *param)
{
    osal_u32 off_set, tmp;
    osal_u32 ret, loop;
    osal_s8 *param_tmp = param;
    osal_s8 args[WAL_CCPRIV_CMD_NAME_MAX_LEN] = { 0 };
    service_control_msg msg = {0};

    /* echo "wlan0 service_control mask value" > /sys/ccsys/ccpriv"
        mask 取值 0xffffffff 时, value用于配置整个变量的值, 对应到0-31个bit位
        mask其他取值,即 0~31 时, 表示配置单个bit位的值, 此时value值只能是0或1
    */

    /* 命令包含2个参数 */
    for (loop = 0; loop < 2; loop++) {
        ret = wal_get_cmd_one_arg(param_tmp, args, WAL_CCPRIV_CMD_NAME_MAX_LEN, &off_set);
        if (ret != OAL_SUCC) {
            oam_warning_log2(0, OAM_SF_ANY, "uapi_ccpriv_service_control:wal_get_cmd_one_arg err[%d][%d]", loop, ret);
            return ret;
        }
        param_tmp += off_set;
        tmp = (osal_u32)osal_strtoul((const osal_char *)args, NULL, 16); /* 16进制参数解析 */
        if (loop == 0) {
            msg.mask = tmp;
            if ((tmp != 0xffffffff) && (tmp > 31)) { /* mask 取值 0xffffffff 或 0~31 */
                oam_warning_log2(0, OAM_SF_ANY, "uapi_ccpriv_service_control: mask err[%d][%d]", loop, tmp);
                return OAL_FAIL;
            }
        } else {
            msg.value = tmp;
        }
    }

    oam_warning_log2(0, OAM_SF_ANY, "{uapi_ccpriv_service_control::mask[0x%x], value[0x%x]}", msg.mask, msg.value);

    if (wal_set_service_control_etc(&msg) != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_ANY, "{uapi_ccpriv_service_control::set wal_set_service_control_etc fail!}");
        return OAL_FAIL;
    }

    ret = (osal_u32)wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(net_dev), WLAN_MSG_W2H_CFG_SERVICE_CONTROL_PARA,
        (osal_u8 *)&msg, OAL_SIZEOF(msg));
    if (osal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_ccpriv_service_control::event return err code [%d]!}", ret);
        return ret;
    }

    ret = (osal_u32)wal_sync_send2device_no_rsp(wal_util_get_vap_id(net_dev), WLAN_MSG_W2D_C_CFG_SERVICE_CONTROL_PARA,
        (osal_u8 *)&msg, OAL_SIZEOF(msg));
    if (osal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_ccpriv_service_control::event return err code [%d]!}", ret);
        return ret;
    }
    return (osal_u32)ret;
}

/* 业务控制开关, 32bits变量, 支持32种场景 */
osal_u32 uapi_ccpriv_service_control_get(oal_net_device_stru *net_dev, osal_s8 *param)
{
    /* 命令没有参数, wal hmac dmac的值应该是相同的才对 */
    unref_param(net_dev);
    unref_param(param);
    wifi_printf("{uapi_ccpriv_service_control_get: value 0x%x}\r\n", wal_get_service_control());

    return OAL_SUCC;
}
#endif

osal_u32 wal_get_tx_params(hmac_user_stru *hmac_user, osal_u32 *tx_best_rate)
{
    osal_u32 ret;
    frw_msg cfg_info = {0};
    mac_cfg_ar_tx_params_stru ar_tx_params = {0};
    mac_cfg_ar_tx_params_stru rsp_info = {0};

    ar_tx_params.user_id = hmac_user->assoc_id;
    memset_s(&cfg_info, OAL_SIZEOF(cfg_info), 0, OAL_SIZEOF(cfg_info));
    cfg_msg_init((osal_u8 *)&ar_tx_params, OAL_SIZEOF(mac_cfg_ar_tx_params_stru),
        (osal_u8 *)&rsp_info, OAL_SIZEOF(mac_cfg_ar_tx_params_stru), &cfg_info);

    ret = (osal_u32)send_cfg_to_device(0, WLAN_MSG_W2D_C_CFG_GET_TX_PARAMS, &cfg_info, OSAL_TRUE);
    if ((ret != OAL_SUCC) || (cfg_info.rsp == OSAL_NULL)) {
        oam_warning_log2(0, OAM_SF_ANY, "{wal_get_tx_params::user_id = %u, ret:%d}",
            hmac_user->assoc_id, ret);
        return ret;
    }
    *tx_best_rate = rsp_info.tx_best_rate;
    /* 打印tx_rate */
    oam_info_log2(0, OAM_SF_ANY, "{wal_get_tx_params::user_id = %u, tx_rate = %u kbps}",
        rsp_info.user_id, rsp_info.tx_best_rate);
    wifi_printf("wal_get_tx_params::user_id = %u, tx_rate = %u kbps\r\n",
        rsp_info.user_id, rsp_info.tx_best_rate);
    return OAL_SUCC;
}

/* 功能描述  : 查询用户的最佳速率 */
osal_u32 uapi_ccpriv_get_tx_params(oal_net_device_stru *net_dev, osal_s8 *param)
{
    hmac_vap_stru *hmac_vap = OSAL_NULL;
    osal_u32 ret;
    osal_u32 tx_best_rate = 0;
    osal_u16 user_idx = 0;
    hmac_user_stru *hmac_user = OSAL_NULL;

    hmac_vap = (hmac_vap_stru *)net_dev->ml_priv;
    if (osal_unlikely(hmac_vap == OSAL_NULL)) {
        oam_warning_log0(0, OAM_SF_ANY, "{uapi_ccpriv_get_tx_params::hmac_vap null!}");
        return OAL_FAIL;
    }

    if (param == OSAL_NULL || param[0] == 0) {
        oam_warning_log0(0, OAM_SF_ANY, "{uapi_ccpriv_get_tx_params::param error!}");
        return OAL_FAIL;
    } else {
        ret = wal_get_user_by_mac(net_dev, param, &user_idx);
        if (ret != OAL_SUCC) {
            oam_warning_log1(0, OAM_SF_ANY, "{uapi_ccpriv_get_tx_params:: get user failed[%d]!}", ret);
            return OAL_FAIL;
        }
    }
    hmac_user = mac_res_get_hmac_user_etc(user_idx);
    if (osal_unlikely(hmac_user == OSAL_NULL)) {
        oam_warning_log0(0, OAM_SF_ANY, "{uapi_ccpriv_get_user_rate_info::hmac_user null!}");
        return OAL_FAIL;
    }

    ret = wal_get_tx_params(hmac_user, &tx_best_rate);
    if (osal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log0(0, OAM_SF_ANY, "{uapi_ccpriv_get_user_rate_info::no such user!}");
        return OAL_FAIL;
    }

    return OAL_SUCC;
}

static osal_u32 uapi_ccpriv_get_cmd_set_rate_power_param(osal_s8 *param, osal_s32 *protocol,
    osal_s32 *rate, osal_s32 *power_offset)
{
    osal_s8 cmd[WAL_CCPRIV_CMD_NAME_MAX_LEN] = {0};
    osal_u32 ret;
    osal_u32 offset = 0;
    osal_s8 *param_tmp = param;

    ret = wal_get_cmd_one_arg(param_tmp, cmd, OAL_SIZEOF(cmd), &offset);
    if (ret != OAL_SUCC) {
        return ret;
    }
    *protocol = oal_atoi((const osal_s8 *)cmd);

    param_tmp += offset;
    ret = wal_get_cmd_one_arg(param_tmp, cmd, OAL_SIZEOF(cmd), &offset);
    if (ret != OAL_SUCC) {
        return ret;
    }
    *rate = oal_atoi((const osal_s8 *)cmd);

    param_tmp += offset;
    ret = wal_get_cmd_one_arg(param_tmp, cmd, OAL_SIZEOF(cmd), &offset);
    if (ret != OAL_SUCC) {
        return ret;
    }
    *power_offset = oal_atoi((const osal_s8 *)cmd);
    return OAL_SUCC;
}

static osal_u32 uapi_ccpriv_check_cmd_set_rate_power_param(osal_s32 protocol, osal_s32 protocol_num, osal_u8 *rate_max,
    osal_s32 rate, osal_s32 power_offset)
{
    osal_s32 min_power = -100;  // 最小可配置的偏移是-100 -> -10dB
    osal_s32 max_power = 40;    // 最大可配置的偏移是 40 -> 4dB
    osal_s32 rate_num;

    if (protocol >= protocol_num || protocol < 0) {
        return OAL_FAIL;
    }
    rate_num = rate_max[protocol];
    if ((rate > rate_num || rate < 0) || (power_offset < min_power || power_offset > max_power)) {
        return OAL_FAIL;
    }
    return OAL_SUCC;
}
osal_u32 uapi_ccpriv_set_rate_power(oal_net_device_stru *net_dev, osal_s8 *param)
{
    osal_s32 protocol = 0xff;
    osal_s32 rate = 0xff;
    osal_s32 power_offset = 0;
    osal_s32 ret;
    osal_u8 rate_max_num[] = {
        FE_CUSTOMIZE_TARGET_POWER_11B_LEN, FE_CUSTOMIZE_TARGET_POWER_11G_LEN,
        FE_CUSTOMIZE_TARGET_POWER_20M_LEN, FE_CUSTOMIZE_TARGET_POWER_40M_LEN
    };
    osal_u8 ini_rate_num[] = {
        WLAN_POW_11B_RATE_NUM, WLAN_POW_LEGACY_RATE_NUM, WLAN_POW_NONLEGACY_20M_NUM, WLAN_POW_NONLEGACY_40M_NUM
    };
    osal_u8 protocol_num = OAL_SIZEOF(rate_max_num) / OAL_SIZEOF(osal_u8);
    osal_u8 rate_max = 0;
    osal_u8 rate_min = 0;
    osal_u32 config_msg = 0;
    osal_u8 index;
    osal_u8 offset = 0x8;

    if (uapi_ccpriv_get_cmd_set_rate_power_param(param, &protocol, &rate, &power_offset) != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_ANY, "uapi_ccpriv_set_rate_power get param error !");
        return OAL_FAIL;
    }

    if (uapi_ccpriv_check_cmd_set_rate_power_param(protocol, protocol_num,
        (osal_u8 *)rate_max_num, rate, power_offset) != OAL_SUCC) {
        oam_error_log3(0, OAM_SF_ANY, "uapi_ccpriv_set_rate_power proto:%d rate:%d offset:%d.", protocol, rate,
            power_offset);
        return OAL_FAIL;
    }
    index = (osal_u8)protocol;
    while (index-- != 0) {
        rate_min += ini_rate_num[index];
    }
    // 处理一下最大值 修改范围是[rate_min, rate_max] 边界值则修改整个协议内速率的功率; 特定速率则修改当前协议速率的功率
    if (rate == rate_max_num[protocol]) {
        rate_max = rate_min + ini_rate_num[protocol] - 1;
    } else {
        rate_min = (osal_u8)(rate_min + rate);
        rate_max = rate_min;
    }
    // bit 16-23:max rate  bit 8-15:min rate  bit 0-7:power offset
    offset = 0x8;
    config_msg = (((rate_max << offset) + rate_min) << offset) + (osal_u8)power_offset;

    ret = wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(net_dev),
        WLAN_MSG_W2H_CFG_RATE_POWER, (osal_u8 *)&config_msg, OAL_SIZEOF(config_msg));
    if (ret != OAL_SUCC) {
        oam_error_log2(0, OAM_SF_ANY, "uapi_ccpriv_set_rate_power msg:0x%x ret:%u.", config_msg, ret);
        return OAL_FAIL;
    } else {
        wifi_printf("OK\r\n");
        return OAL_SUCC;
    }
}

osal_u32 uapi_ccpriv_set_cal_tone(oal_net_device_stru *net_dev, osal_s8 *param)
{
    osal_u32 config_msg = 0;
    osal_s8 cmd[WAL_CCPRIV_CMD_NAME_MAX_LEN] = {0};
    osal_u32 ret;
    osal_u32 offset = 0;
    osal_s8 *param_tmp = param;
    osal_s32 sw = 0;    // 开或关单音
    osal_s32 tone_freq = 0; // 单音频率 kHz
    const osal_s32 tone_freq_max = 40000;  // 对应100M单音

    ret = wal_get_cmd_one_arg(param_tmp, cmd, OAL_SIZEOF(cmd), &offset);
    if (ret != OAL_SUCC) {
        return ret;
    }
    sw = oal_atoi((const osal_s8 *)cmd);
    if (sw == OSAL_TRUE) {
        param_tmp += offset;
        ret = wal_get_cmd_one_arg(param_tmp, cmd, OAL_SIZEOF(cmd), &offset);
        if (ret != OAL_SUCC) {
            return ret;
        }
        tone_freq = oal_atoi((const osal_s8 *)cmd);
    }
    if ((sw != OSAL_TRUE && sw != OSAL_FALSE) || tone_freq >= tone_freq_max || tone_freq <= (-tone_freq_max)) {
        oam_error_log2(0, OAM_SF_ANY, "uapi_ccpriv_set_cal_tone param err, sw:%s freq:%s.", sw, tone_freq);
        return OAL_FAIL;
    }

    config_msg = ((((osal_u32)sw) << BIT_OFFSET_24) + (((osal_u32)tone_freq) & 0xffffff));
    ret = (osal_s32)wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(net_dev),
        WLAN_MSG_W2H_CFG_CAL_TONE, (osal_u8 *)&config_msg, OAL_SIZEOF(config_msg));
    if (ret != OAL_SUCC) {
        oam_error_log2(0, OAM_SF_ANY, "uapi_ccpriv_set_cal_tone msg:0x%x ret:%u.", config_msg, ret);
        return OAL_FAIL;
    } else {
        wifi_printf("OK\r\n");
        return OAL_SUCC;
    }
}
oal_bool_enum_uint8 check_mac_str(osal_s8 *value)
{
    osal_s8 *curr = value;
    osal_u32 i;

    /* MAC地址字符串固定为17个字符 */
    if (osal_strlen((const osal_char *)value) != 17) {
        return OAL_FALSE;
    }

    /* 对MAC地址分6段分别检查 */
    for (i = 0; i < 6; i++) {
        if (!oal_isxdigit(curr[0]) || !oal_isxdigit(curr[1])) {
            return OAL_FALSE;
        }
        /* 前5段的第2位字符应为':' */
        if (i < 5 && curr[2] != ':') {
            return OAL_FALSE;
        }
        /* 每次加3判断: */
        curr += 3;
    }
    return OAL_TRUE;
}

#ifdef _PRE_WLAN_FEATURE_11D
osal_u32 uapi_ccpriv_get_nvram_power(oal_net_device_stru *net_dev, osal_s8 *param)
{
    unref_param(net_dev);
    unref_param(param);
    wal_print_nvram_list();
    return OAL_SUCC;
}
#endif

#ifdef _PRE_WLAN_CFGID_DEBUG
/*****************************************************************************
 功能描述  : 设置TLV类型命令统一接口
*****************************************************************************/
osal_u32  uapi_ccpriv_set_ampdu_amsdu(oal_net_device_stru *net_dev, osal_s8 *param)
{
    osal_u32 off_set;
    osal_s32 l_ret;
    osal_u32 ret;
    osal_s8 tx, rx;
    osal_s8 *param_tmp = param;
    osal_s8 ac_name[WAL_CCPRIV_CMD_NAME_MAX_LEN];
    mac_cfg_set_ampdu_amsdu_stru  *ampdu_amsdu;
    mac_cfg_set_ampdu_amsdu_stru cfg_ampdu_amsdu;

    (osal_void)memset_s(&cfg_ampdu_amsdu, OAL_SIZEOF(cfg_ampdu_amsdu), 0, OAL_SIZEOF(cfg_ampdu_amsdu));
    ampdu_amsdu = &cfg_ampdu_amsdu;

    if (wal_ccpriv_check_cfg_vap(net_dev) == OSAL_TRUE) {
        oam_warning_log0(0, OAM_SF_ANY, "uapi_ccpriv_set_ampdu_amsdu vap mode err: cfg");
        return OAL_FAIL;
    }
    /* 获取配置参数名称 */
    ret = wal_get_cmd_one_arg(param_tmp, ac_name, OAL_SIZEOF(ac_name), &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
                         "{uapi_ccpriv_set_ampdu_amsdu::wal_get_cmd_one_arg return err_code [%d]!}", ret);
        return ret;
    }
    param_tmp += off_set;
    tx = (osal_s8)oal_atoi((const osal_s8 *)ac_name);
    ret = wal_get_cmd_one_arg(param_tmp, ac_name, OAL_SIZEOF(ac_name), &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
                         "{uapi_ccpriv_set_ampdu_amsdu::wal_get_cmd_one_arg return err_code [%d]!}", ret);
        return ret;
    }
    param_tmp += off_set;
    rx = (osal_s8)oal_atoi((const osal_s8 *)ac_name);

    ampdu_amsdu->tx_switch   = (tx > 0) ? OSAL_TRUE : OSAL_FALSE;
    ampdu_amsdu->rx_switch   = (rx > 0) ? OSAL_TRUE : OSAL_FALSE;

    oam_warning_log2(0, OAM_SF_ANY, "{uapi_ccpriv_set_ampdu_amsdu:: tx_switch[%d] rx_switch[%d]!}",
                     ampdu_amsdu->tx_switch, ampdu_amsdu->rx_switch);
    /***************************************************************************
                             抛事件到wal层处理
    ***************************************************************************/
    l_ret = wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(net_dev), WLAN_MSG_W2H_CFG_AMSDU_AMPDU_SWITCH,
        (osal_u8 *)&cfg_ampdu_amsdu, OAL_SIZEOF(cfg_ampdu_amsdu));
    if (OAL_UNLIKELY(l_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY,
                         "{wal_ccpriv_set_tlv_cmd::wal_sync_post2hmac_no_rsp return err_code [%d]!}", l_ret);
        return (osal_u32)l_ret;
    }

    wifi_printf("OK\r\n");
    return OAL_SUCC;
}
#endif

#ifdef _PRE_WLAN_FEATURE_FTM
static inline oal_bool_enum_uint8 check_bool(osal_s8 *value)
{
    /* 开关类型的值仅能为字符'0'或'1' */
    if ((osal_strlen((const osal_s8 *)value) != 1) || (value[0] != '0' && value[0] != '1')) {
        return OAL_FALSE;
    }
    return OAL_TRUE;
}

static oal_bool_enum_uint8 check_signed(osal_s8 *value)
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

static inline osal_void ftm_debug_cmd_format_info(osal_void)
{
    oam_warning_log0(0, OAM_SF_FTM, "{CMD format::ccpriv.sh wlan0 ftm_debug enable_ftm_initiator[0|1]}");
    oam_warning_log0(0, OAM_SF_FTM, "{enable[0|1] enable_ftm_resp[0|1]}");
    oam_warning_log0(0, OAM_SF_FTM, "{send_ftm bssid[xx:xx:xx:xx:xx:xx]}");
    oam_warning_log0(0, OAM_SF_FTM,
        "{send_iftmr channel[] single_burst[0|1] ftms_per_burst[n] measure_req[0|1] asap[0|1] bssid}");
}

typedef osal_u32 (*ftm_debug_parse_cmd_func)(osal_s8 **src_head, mac_ftm_debug_switch_stru *debug_info);

typedef struct {
    const osal_char *cmd_name; /* 命令字符串 */
    ftm_debug_parse_cmd_func func; /* 命令对应处理函数 */
} ftm_cmd_entry;

STATIC osal_u32 ftm_debug_iftmr_get_channel(osal_s8 **src_head, mac_ftm_debug_switch_stru *debug_info)
{
    osal_s8 *param = *src_head;
    osal_u32 ret, off_set;
    osal_s8 value[WAL_CCPRIV_CMD_NAME_MAX_LEN];
    osal_u8 channel_num;
    osal_u8 channel_idx;

    /* 解析channel */
    ret = wal_get_cmd_one_arg(param, value, OAL_SIZEOF(value), &off_set);
    if ((ret != OAL_SUCC) || (check_unsigned((osal_s8 *)value) != OAL_TRUE)) {
        oam_warning_log0(0, OAM_SF_FTM, "{ftm_debug_iftmr_get_channel::get iftmr mode error,return.}");
        return OAL_FAIL;
    }
    channel_num = (osal_u8)oal_atoi((const osal_s8 *)value);
    /* 0~14的信道为2.4G信道 */
    if ((channel_num > 0) && (channel_num <= 14)) {          /* 2.4G 信道数14 */
        ret = hmac_get_channel_idx_from_num_etc(WLAN_BAND_2G, channel_num, &channel_idx);
        if ((ret != OAL_SUCC) || (hmac_is_channel_idx_valid_etc(WLAN_BAND_2G, channel_idx) != OAL_SUCC)) {
            return OAL_FAIL;
        }
    } else if (channel_num > 14) {                          /* 2.4G 信道数14 */
        ret = hmac_get_channel_idx_from_num_etc(WLAN_BAND_5G, channel_num, &channel_idx);
        if ((ret != OAL_SUCC) || (hmac_is_channel_idx_valid_etc(WLAN_BAND_5G, channel_idx) != OAL_SUCC)) {
            return OAL_FAIL;
        }
    }
    debug_info->send_iftmr.channel_num = channel_num;
    *src_head = param + off_set;
    return OAL_CONTINUE;
}

STATIC osal_u32 ftm_debug_iftmr_parse_cmd(osal_s8 **src_head, mac_ftm_debug_switch_stru *debug_info)
{
    osal_s8 *param = *src_head;
    osal_u32 ret, off_set;
    osal_s8 value[WAL_CCPRIV_CMD_NAME_MAX_LEN];

    /* 解析burst */
    ret = wal_get_cmd_one_arg(param, value, OAL_SIZEOF(value), &off_set);
    /* burst取值范围为0~2 */
    if ((ret != OAL_SUCC) || (check_unsigned_range(value, 0, 2) != OAL_TRUE)) {
        oam_warning_log0(0, OAM_SF_FTM, "{ftm_debug_parse_iftmr_cmd::get burst error,return.}");
        return OAL_FAIL;
    }
    debug_info->send_iftmr.burst_num = (osal_u8)oal_atoi((const osal_s8 *)value);
    param += off_set;

    /* 解析ftms_per_burst */
    ret = wal_get_cmd_one_arg(param, value, OAL_SIZEOF(value), &off_set);
    /* per_burst取值范围为2~4 */
    if ((ret != OAL_SUCC) || (check_unsigned_range(value, 2, 4) != OAL_TRUE)) {
        oam_warning_log0(0, OAM_SF_FTM, "{ftm_debug_parse_iftmr_cmd::get ftms_per_burst error,return.}");
        return OAL_FAIL;
    }
    debug_info->send_iftmr.ftms_per_burst = (osal_u8)oal_atoi((const osal_s8 *)value);
    *src_head = param + off_set;

    return OAL_SUCC;
}

STATIC osal_u32 ftm_debug_parse_iftmr_cmd(osal_s8 **src_head, mac_ftm_debug_switch_stru *debug_info)
{
    osal_s8 *param = *src_head;
    osal_u32 ret, off_set;
    osal_s8 value[WAL_CCPRIV_CMD_NAME_MAX_LEN] = {0};

    ret = ftm_debug_iftmr_get_channel(&param, debug_info);
    if (ret != OAL_CONTINUE) {
        oam_warning_log0(0, OAM_SF_FTM, "{ftm_debug_parse_iftmr_cmd::get channel check error,return.}");
        return OAL_FAIL;
    }

    ret = ftm_debug_iftmr_parse_cmd(&param, debug_info);
    if (ret != OAL_CONTINUE) {
        oam_warning_log0(0, OAM_SF_FTM, "{ftm_debug_parse_iftmr_cmd::cmd check error,return.}");
        return OAL_FAIL;
    }

    /* 解析measure_req */
    ret = wal_get_cmd_one_arg(param, value, OAL_SIZEOF(value), &off_set);
    if ((ret != OAL_SUCC) || (check_bool((osal_s8 *)value) != OAL_TRUE)) {
        oam_warning_log0(0, OAM_SF_FTM, "{ftm_debug_parse_iftmr_cmd::get measure_req error,return.}");
        return OAL_FAIL;
    }
    debug_info->send_iftmr.measure_req = (osal_u8)oal_atoi((const osal_s8 *)value);
    param += off_set;

    /* 解析asap */
    ret = wal_get_cmd_one_arg(param, value, OAL_SIZEOF(value), &off_set);
    if ((ret != OAL_SUCC) || (check_bool((osal_s8 *)value) != OAL_TRUE)) {
        oam_warning_log0(0, OAM_SF_FTM, "{ftm_debug_parse_iftmr_cmd::get measure_req error,return.}");
        return OAL_FAIL;
    }
    debug_info->send_iftmr.asap = (osal_u8)oal_atoi((const osal_s8 *)value);
    param += off_set;

    /* 解析bssid */
    ret = wal_get_cmd_one_arg(param, value, OAL_SIZEOF(value), &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_FTM, "{ftm_debug_parse_iftmr_cmd::No bssid,set the associated bssid.}");
        (osal_void)memset_s(debug_info->send_iftmr.bssid, WLAN_MAC_ADDR_LEN, 0, WLAN_MAC_ADDR_LEN);
    } else {
        if (check_mac_str((osal_s8 *)value) != OAL_TRUE) {
            oam_warning_log0(0, OAM_SF_FTM, "{ftm_debug_parase_iftmr_cmd::ger bssid error.}");
            return OAL_FAIL;
        }
        oal_strtoaddr((const osal_char *)value, debug_info->send_iftmr.bssid);
    }
    *src_head = param + off_set;
    debug_info->cmd_bit_map |= FTM_SEND_IFTMR;

    return OAL_SUCC;
}

/* 解析ftm命令参数 */
STATIC osal_u32 ftm_debug_parse_ftm_cmd(osal_s8 **src_head, mac_ftm_debug_switch_stru *debug_info)
{
    osal_s8 *param = *src_head;
    osal_u32 ret;
    osal_s8 value[WAL_CCPRIV_CMD_NAME_MAX_LEN] = {0};
    osal_u32 off_set = 0;

    /* 解析mac */
    ret = wal_get_cmd_one_arg(param, value, OAL_SIZEOF(value), &off_set);
    if ((ret != OAL_SUCC) || (check_mac_str((osal_s8 *)value) != OAL_TRUE)) {
        oam_warning_log0(0, OAM_SF_FTM, "{ftm_debug_parse_ftm_cmd::ger mac error.}");
        return OAL_FAIL;
    }
    oal_strtoaddr((const osal_char *)value, debug_info->send_ftm.mac);
    *src_head = param + off_set;
    debug_info->cmd_bit_map |= FTM_SEND_FTM;

    return OAL_SUCC;
}

STATIC osal_u32 ftm_debug_parse_m2s_cmd(osal_s8 **src_head, mac_ftm_debug_switch_stru *debug_info)
{
    osal_s8 *param = *src_head;
    osal_u32 ret;
    osal_s8 value[WAL_CCPRIV_CMD_NAME_MAX_LEN];
    osal_u32 off_set = 0;
    ftm_m2s_stru *m2s = &(debug_info->m2s);

    /* 解析m2s */
    ret = wal_get_cmd_one_arg(param, value, OAL_SIZEOF(value), &off_set);
    if ((ret != OAL_SUCC) || (check_bool((osal_s8 *)value) != OAL_TRUE)) {
        oam_warning_log0(0, OAM_SF_FTM, "{ftm_debug_parse_m2s_cmd::get m2s error,return.}");
        return OAL_FAIL;
    }
    m2s->is_mimo = (osal_u8)oal_atoi((const osal_s8 *)value);
    param += off_set;

    /* 解析tx_chain */
    ret = wal_get_cmd_one_arg(param, value, OAL_SIZEOF(value), &off_set);
    if ((ret != OAL_SUCC) || (check_unsigned_range(value, 1, 3) != OAL_TRUE)) {      /* tx_chain的取值为1~3 */
        oam_warning_log0(0, OAM_SF_FTM, "{ftm_debug_parse_m2s_cmd::get tx_chain error,return.}");
        return OAL_FAIL;
    }
    m2s->tx_chain_selection = (osal_u8)oal_atoi((const osal_s8 *)value);
    *src_head = param + off_set;
    debug_info->cmd_bit_map |= FTM_SET_M2S;

    return OAL_SUCC;
}

/* 解析ftm命令参数 */
STATIC osal_u32 ftm_debug_parse_ftm_time_cmd(osal_s8 **src_head, mac_ftm_debug_switch_stru *debug_info)
{
    osal_s8 *param = *src_head;
    osal_u32 ret;
    osal_u8 value[WAL_CCPRIV_CMD_NAME_MAX_LEN];
    osal_u32 off_set = 0;

    /* 解析t1 */
    ret = wal_get_cmd_one_arg(param, value, OAL_SIZEOF(value), &off_set);
    if ((ret != OAL_SUCC) || (check_unsigned((osal_s8 *)value) != OAL_TRUE)) {
        oam_warning_log0(0, OAM_SF_FTM, "{ftm_debug_parse_ftm_time_cmd::get correct time1 error,return.}");
        return OAL_FAIL;
    }

    debug_info->ftm_time.ftm_correct_time1 = (osal_u32)oal_atoi((const osal_s8 *)value);
    param += off_set;

    /* 解析t2 */
    ret = wal_get_cmd_one_arg(param, value, OAL_SIZEOF(value), &off_set);
    if ((ret != OAL_SUCC) || (check_unsigned((osal_s8 *)value) != OAL_TRUE)) {
        oam_warning_log0(0, OAM_SF_FTM, "{ftm_debug_parse_ftm_time_cmd::get correct time2 error,return.}");
        return OAL_FAIL;
    }
    debug_info->ftm_time.ftm_correct_time2 = (osal_u32)oal_atoi((const osal_s8 *)value);
    param += off_set;

    /* 解析t3 */
    ret = wal_get_cmd_one_arg(param, value, OAL_SIZEOF(value), &off_set);
    if ((ret != OAL_SUCC) || (check_unsigned((osal_s8 *)value) != OAL_TRUE)) {
        oam_warning_log0(0, OAM_SF_FTM, "{ftm_debug_parse_ftm_time_cmd::get correct time3 error,return.}");
        return OAL_FAIL;
    }

    debug_info->ftm_time.ftm_correct_time3 = (osal_u32)oal_atoi((const osal_s8 *)value);
    param += off_set;

    /* 解析t4 */
    ret = wal_get_cmd_one_arg(param, value, OAL_SIZEOF(value), &off_set);
    if ((ret != OAL_SUCC) || (check_unsigned((osal_s8 *)value) != OAL_TRUE)) {
        oam_warning_log0(0, OAM_SF_FTM, "{ftm_debug_parse_ftm_time_cmd::get correct time4 error,return.}");
        return OAL_FAIL;
    }
    debug_info->ftm_time.ftm_correct_time4 = (osal_u32)oal_atoi((const osal_s8 *)value);
    *src_head = param + off_set;
    debug_info->cmd_bit_map |= FTM_SET_CORRECT_TIME;
    return OAL_SUCC;
}

/* 解析ftm 使能命令参数 */
STATIC osal_u32 ftm_debug_parse_ftm_enable_cmd(osal_s8 **src_head, mac_ftm_debug_switch_stru *debug_info)
{
    osal_s8 *param = *src_head;
    osal_u32 ret;
    osal_u8 value[WAL_CCPRIV_CMD_NAME_MAX_LEN];
    osal_u32 off_set = 0;

    /* 解析enable 状态 */
    ret = wal_get_cmd_one_arg(param, value, OAL_SIZEOF(value), &off_set);
    if ((ret != OAL_SUCC) || (check_bool((osal_s8 *)value) != OAL_TRUE)) {
        oam_warning_log0(0, OAM_SF_FTM, "{ftm_debug_parse_ftm_enable_cmd::get ftm enable error,return.}");
        return OAL_FAIL;
    }

    debug_info->ftm_enable.enable = (osal_u8)oal_atoi((const osal_s8 *)value);
    param += off_set;

    /* 解析session num */
    ret = wal_get_cmd_one_arg(param, value, OAL_SIZEOF(value), &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_FTM, "{ftm_debug_parse_ftm_enable_cmd::set default session num[4]}");
        /* 默认FTM会话数为4 */
        debug_info->ftm_enable.session_num = 4;
    } else {
        /* session num的取值范围为1~8 */
        if (check_unsigned_range(value, 1, 8) != OAL_TRUE) {
            oam_warning_log0(0, OAM_SF_FTM, "{ftm_debug_parse_ftm_enable_cmd::get ftm session num error,return.}");
            return OAL_FAIL;
        }

        debug_info->ftm_enable.session_num = (osal_u8)oal_atoi((const osal_s8 *)value);
    }
    *src_head = param + off_set;
    debug_info->cmd_bit_map |= FTM_ENABLE;
    return OAL_SUCC;
}

STATIC osal_u32 ftm_debug_parse_enable_initiator_cmd(osal_s8 **src_head, mac_ftm_debug_switch_stru *debug_info)
{
    osal_s8 *param = *src_head;
    osal_s8 value[WAL_CCPRIV_CMD_NAME_MAX_LEN];
    osal_u32 off_set = 0;
    osal_u32 ret;

    /* 取命令配置值 */
    ret = wal_get_cmd_one_arg(param, value, OAL_SIZEOF(value), &off_set);
    if ((ret != OAL_SUCC) || (check_bool((osal_s8 *)value) != OAL_TRUE)) {
        return OAL_FAIL;
    }
    *src_head = param + off_set;
    /* 填写结构体 */
    debug_info->ftm_initiator = ((osal_u8)oal_atoi((const osal_s8 *)value));
    debug_info->cmd_bit_map |= FTM_INITIATOR;
    return OAL_SUCC;
}

STATIC osal_u32 ftm_debug_parse_cali_cmd(osal_s8 **src_head, mac_ftm_debug_switch_stru *debug_info)
{
    osal_u32 ret;
    osal_u32 off_set = 0;
    osal_s8 *param = *src_head;
    osal_s8 value[WAL_CCPRIV_CMD_NAME_MAX_LEN];

    ret = wal_get_cmd_one_arg(param, value, OAL_SIZEOF(value), &off_set);
    if ((ret != OAL_SUCC) || (check_bool((osal_s8 *)value) != OAL_TRUE)) {
        return OAL_FAIL;
    }
    *src_head = param + off_set;
    /* 填写结构体 */
    debug_info->cali = ((osal_u8)oal_atoi((const osal_s8 *)value));
    debug_info->cmd_bit_map |= FTM_CALI;

    return OAL_SUCC;
}

STATIC osal_u32 ftm_debug_parse_enable_resp_cmd(osal_s8 **src_head, mac_ftm_debug_switch_stru *debug_info)
{
    osal_s8 value[WAL_CCPRIV_CMD_NAME_MAX_LEN];
    osal_s8 *param = *src_head;
    osal_u32 ret;
    osal_u32 off_set = 0;

    /* 取命令配置值 */
    ret = wal_get_cmd_one_arg(param, value, OAL_SIZEOF(value), &off_set);
    if ((ret != OAL_SUCC) || (check_bool((osal_s8 *)value) != OAL_TRUE)) {
        return OAL_FAIL;
    }
    *src_head = param + off_set;
    /* 填写结构体 */
    debug_info->ftm_resp = ((osal_u8)oal_atoi((const osal_s8 *)value));
    debug_info->cmd_bit_map |= FTM_RESPONDER;

    return OAL_SUCC;
}

STATIC osal_u32 ftm_debug_parse_auto_bandwidth_cmd(osal_s8 **src_head, mac_ftm_debug_switch_stru *debug_info)
{
    osal_s8 *param = *src_head;
    osal_s8 value[WAL_CCPRIV_CMD_NAME_MAX_LEN];
    osal_u32 ret;
    osal_u32 off_set = 0;

    /* 取命令配置值 */
    ret = wal_get_cmd_one_arg(param, value, OAL_SIZEOF(value), &off_set);
    /* auto_bandwidth的取值范围为0~1与8~16 */
    if ((ret != OAL_SUCC) ||
        (check_unsigned_range(value, 0, 1) != OAL_TRUE && check_unsigned_range(value, 8, 16) != OAL_TRUE)) { /* 8~16 */
        return OAL_FAIL;
    }
    *src_head = param + off_set;
    /* 填写结构体 */
    debug_info->auto_bandwidth = ((osal_u8)oal_atoi((const osal_s8 *)value));
    debug_info->cmd_bit_map |= FTM_DBG_AUTO_BANDWIDTH;

    return OAL_SUCC;
}

STATIC osal_u32 ftm_debug_parse_range_cmd(osal_s8 **src_head, mac_ftm_debug_switch_stru *debug_info)
{
    osal_u32 ret;
    osal_s8 *param = *src_head;
    osal_s8 value[WAL_CCPRIV_CMD_NAME_MAX_LEN];
    osal_u32 off_set = 0;

    /* 取命令配置值 */
    ret = wal_get_cmd_one_arg(param, value, OAL_SIZEOF(value), &off_set);
    if ((ret != OAL_SUCC) || (check_bool((osal_s8 *)value) != OAL_TRUE)) {
        return OAL_FAIL;
    }
    *src_head = param + off_set;
    /* 填写结构体 */
    debug_info->ftm_range = ((osal_u8)oal_atoi((const osal_s8 *)value));
    debug_info->cmd_bit_map |= FTM_RANGE;

    return OAL_SUCC;
}

STATIC osal_u32 ftm_debug_parse_tsf_cmd(osal_s8 **src_head, mac_ftm_debug_switch_stru *debug_info)
{
    osal_s8 *param = *src_head;
    osal_s8 value[WAL_CCPRIV_CMD_NAME_MAX_LEN];
    osal_u32 ret;
    osal_u32 off_set = 0;

    /* 取命令配置值 */
    ret = wal_get_cmd_one_arg(param, value, OAL_SIZEOF(value), &off_set);
    if ((ret != OAL_SUCC) || (check_signed((osal_s8 *)value) != OAL_TRUE)) {
        return OAL_FAIL;
    }
    *src_head = param + off_set;
    /* 填写结构体 */
    debug_info->add_tsf = ((osal_s16)oal_atoi((const osal_s8 *)value));
    debug_info->cmd_bit_map |= FTM_ADD_TSF;

    return OAL_SUCC;
}

STATIC osal_u32 ftm_debug_do_cmd(const osal_char *name, osal_s8 **src_head, mac_ftm_debug_switch_stru *debug_info)
{
    OAL_CONST ftm_cmd_entry ftm_cmd[] = {
        {"enable_ftm_initiator", ftm_debug_parse_enable_initiator_cmd},
        {"enable", ftm_debug_parse_ftm_enable_cmd},
        {"cali", ftm_debug_parse_cali_cmd},
        {"enable_ftm_resp", ftm_debug_parse_enable_resp_cmd},
        {"send_ftm", ftm_debug_parse_ftm_cmd},
        {"send_iftmr", ftm_debug_parse_iftmr_cmd},
        {"set_ftm_time", ftm_debug_parse_ftm_time_cmd},
        {"set_ftm_m2s", ftm_debug_parse_m2s_cmd},
        {"set_ftm_auto_bandwidth", ftm_debug_parse_auto_bandwidth_cmd},
        {"set_ftm_range", ftm_debug_parse_range_cmd},
        {"ftm_add_tsf", ftm_debug_parse_tsf_cmd}
    };
    osal_u32 cmd_idx;
    osal_u32 ret = OAL_FAIL;

    for (cmd_idx = 0; cmd_idx < OAL_ARRAY_SIZE(ftm_cmd); cmd_idx++) {
        if (osal_strcmp(ftm_cmd[cmd_idx].cmd_name, name) == 0) {
            ret = ftm_cmd[cmd_idx].func(src_head, debug_info);
            break;
        }
    }
    return ret;
}

osal_u32 uapi_ccpriv_ftm(oal_net_device_stru *net_dev, osal_s8 *param)
{
    osal_u32 off_set = 0;
    osal_s8 name[WAL_CCPRIV_CMD_NAME_MAX_LEN];
    mac_ftm_debug_switch_stru ftm_debug;
    osal_u32 ret;
    osal_s32 send_ret;

    memset_s(&ftm_debug, sizeof(ftm_debug), 0, sizeof(ftm_debug));

    do {
        /* 获取命令关键字 */
        ret = wal_get_cmd_one_arg(param, name, OAL_SIZEOF(name), &off_set);
        if ((ret != OAL_SUCC) && (off_set != 0)) {
            ftm_debug_cmd_format_info();
            return ret;
        }
        param += off_set;

        if (off_set == 0) {
            break;
        }

        ret = ftm_debug_do_cmd((const osal_char *)name, &param, &ftm_debug);
        if (ret != OAL_SUCC) {
            ftm_debug_cmd_format_info();
            return ret;
        }
    } while (*param != '\0');

    oam_warning_log1(0, OAM_SF_ANY, "{uapi_ccpriv_ftm::cmd_bit_map: 0x%08x.}", ftm_debug.cmd_bit_map);

    /* **************************************************************************
                                抛事件到wal层处理
    ************************************************************************** */
    send_ret = wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(net_dev), WLAN_MSG_W2H_CFG_FTM_DBG,
        (osal_u8 *)&ftm_debug, OAL_SIZEOF(ftm_debug));
    if (osal_unlikely(send_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_FTM, "{uapi_ccpriv_ftm::return err code[%d]!}", send_ret);
        return (osal_u32)send_ret;
    }
    return OAL_SUCC;
}
#endif

/*****************************************************************************
 功能描述  : STA的multi bssid功能使能的调测命令
*****************************************************************************/
osal_u32 uapi_ccpriv_multi_bssid_enable(oal_net_device_stru *net_dev, osal_s8 *param)
{
    osal_u32  off_set, ret;
    osal_s8 name[WAL_CCPRIV_CMD_NAME_MAX_LEN];
    osal_u8 cfg_multi_bssid_switch;

    if (wal_ccpriv_check_cfg_vap(net_dev) == OSAL_TRUE) {
        oam_warning_log0(0, OAM_SF_ANY, "uapi_ccpriv_multi_bssid_enable vap mode err: cfg");
        return OAL_FAIL;
    }
    /*
     * 设置AMPDU关闭的配置命令:
     * ccpriv "Featureid0 multi_bssid_enable 0/1"
     */
    oam_warning_log0(0, OAM_SF_11AX, "{uapi_ccpriv_multi_bssid_enable!}");

    /* 获取txop_duration_threshold */
    ret = wal_get_cmd_one_arg(param, name, OAL_SIZEOF(name), &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_11AX,
                         "{uapi_ccpriv_multi_bssid_enable::wal_get_cmd_one_arg return err=%d!}", ret);
        return ret;
    }

    /* 抛事件到wal层处理 */
    cfg_multi_bssid_switch = (osal_u8)oal_atoi((const osal_s8 *)name);
    ret = (osal_u32)wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(net_dev), WLAN_MSG_W2H_CFG_MULTI_BSSID_ENABLE,
        (osal_u8 *)&cfg_multi_bssid_switch, OAL_SIZEOF(osal_u8));
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_11AX, "{uapi_ccpriv_multi_bssid_enable::return err code[%d]!}", ret);
    }

    return ret;
}

#ifdef _PRE_WLAN_CFGID_DEBUG
osal_u32  uapi_ccpriv_set_ap_user_aging_time(oal_net_device_stru *net_dev, osal_s8 *param)
{
    osal_u32 ret;
    osal_u32 aging_time;

    /*
     * 设置AP keepalive老化时间命令:
     * echo "wlan0 set_aging_time 10 [10, 600]" > /sys/ccsys/ccpriv
     */
    /* 老化时间设置范围[10, 600]s */
    ret = wal_cmd_get_digit_with_check_max(&param, 600, &aging_time);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_ccpriv_set_ap_user_aging_time::set aging_time err[%d]!}", ret);
        return ret;
    }
    oam_warning_log1(0, OAM_SF_ANY, "{call uapi_ccpriv_set_ap_user_aging_time param[%d]!}", aging_time);
    ret = (osal_u32)wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(net_dev), WLAN_MSG_W2H_C_CFG_SET_AP_USER_AGING_TIME,
        (osal_u8 *)&aging_time, OAL_SIZEOF(aging_time));
    if (OAL_UNLIKELY(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_ccpriv_set_ap_user_aging_time::return err code [%d]!}", ret);
    }
    return ret;
}
#endif

#ifdef _PRE_WLAN_FEATURE_OPMODE_NOTIFY
/*****************************************************************************
 函 数 名  : uapi_ccpriv_set_opmode_notify
 功能描述  : 设置工作模式通知能力
*****************************************************************************/
osal_u32 uapi_ccpriv_set_opmode_notify(oal_net_device_stru *net_dev, osal_s8 *param)
{
    osal_s32                       l_ret;
    osal_u32                      ul_ret;
    osal_s8                        ac_name[WAL_CCPRIV_CMD_NAME_MAX_LEN] = {0};
    osal_u32                      off_set = 0;
    osal_u8                       value = 0;

    if (OAL_UNLIKELY((net_dev == OAL_PTR_NULL) || (param == OAL_PTR_NULL))) {
        oam_error_log2(0, OAM_SF_OPMODE,
            "{uapi_ccpriv_set_opmode_notify::net_dev or pc_param null ptr error %d, %d!}",  net_dev, param);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 设置工作模式通知能力: 0关闭 | 1开始  此处将解析出"1"或"0"存入ac_name */
    ul_ret = wal_get_cmd_one_arg(param, ac_name, OAL_SIZEOF(ac_name), &off_set);
    if (ul_ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_OPMODE,
                         "{uapi_ccpriv_set_opmode_notify::wal_get_cmd_one_arg return err_code %d!}", ul_ret);
        return ul_ret;
    }

    /* 针对解析出的不同命令，配置不同的通道 */
    if ((osal_strcmp((const osal_char *)ac_name, "0")) == 0) {
        value = 0;
    } else if ((osal_strcmp((const osal_char *)ac_name, "1")) == 0) {
        value = 1;
    } else {
        oam_warning_log1(0, OAM_SF_OPMODE,
                         "{uapi_ccpriv_set_opmode_notify::opmode notify command is error [%d]!}", ac_name);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }
    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    l_ret = wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(net_dev), WLAN_MSG_W2H_CFG_SET_OPMODE_NOTIFY,
        (osal_u8 *)&value, OAL_SIZEOF(osal_u8));
    if (OAL_UNLIKELY(l_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_OPMODE,
                         "{uapi_ccpriv_set_opmode_notify::wal_ccpriv_reset_device return err code = [%d].}", l_ret);
        return (osal_u32)l_ret;
    }

    return OAL_SUCC;
}
#endif

#ifdef _PRE_WLAN_FEATURE_WOW_OFFLOAD
/*****************************************************************************
 功能描述  : ccpriv set wow function
*****************************************************************************/
osal_u32 uapi_ccpriv_set_wow(oal_net_device_stru *netdev, osal_s8 *pc_param)
{
    td_s32 l_tmp;
    td_u32 off_set;
    td_char ac_name[WAL_CCPRIV_CMD_NAME_MAX_LEN] = {0};
    td_u32 ret;
    td_char *pc_end = OSAL_NULL;

    /* Enable/disable WOW events: ccpriv "wlan0 wow <value>" 0:clear all events, Bit 0:Magic Packet Bit 1:NetPattern
       Bit 2:loss-of-link Bit 3:retrograde tsf Bit 4:loss of beacon Bit 17:TCP keep alive timeout */
    ret = wal_get_cmd_one_arg(pc_param, ac_name, OAL_SIZEOF(ac_name), &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_ccpriv_set_wow::wal_get_cmd_one_arg return err_code [%u]}", ret);
        return ret;
    }

    if ((*ac_name != '0') || ((*(ac_name + 1) != 'x') && (*(ac_name + 1) != 'X'))) {
        oam_warning_log0(0, OAM_SF_ANY, "{uapi_ccpriv_set_wow::invalid wow value}");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    l_tmp = osal_strtol(ac_name, &pc_end, 16); /* 16: 转换为16进制 */

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    ret = wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(netdev), WLAN_MSG_W2H_CFG_SET_WOW,
        (osal_u8 *)&l_tmp, OAL_SIZEOF(osal_s32));
    if (osal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_ccpriv_set_wow::return err code[%u]}", ret);
        return ret;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 获取add wow pattern的参数
*****************************************************************************/
OAL_STATIC osal_u32 wal_ccpriv_get_add_wow_pattern_param(osal_char *pc_param,
    hmac_cfg_wow_pattern_param_stru *wow_param)
{
    td_char ac_value[WAL_CCPRIV_CMD_NAME_MAX_LEN] = {0};
    td_s8 *pc_end = OSAL_NULL;
    td_u32 offset, off_set, pattern_len;
    td_u32 idx;
    td_char high, low;

    /* pc_param 指向'index', 将其取出放到 ac_index 中 */
    if (wal_get_cmd_one_arg(pc_param, ac_value, OAL_SIZEOF(ac_value), &off_set) != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_ANY, "{uapi_ccpriv_set_wow_pattern::get index failed!}");
        return OAL_FAIL;
    }
    wow_param->pattern_index = (osal_u16)osal_strtol(ac_value, (osal_char **)&pc_end, 10); /* 10: 转换为10进制 */
    pc_param += off_set;

    /* pc_param 指向'value', 将其取出放到ac_value中 */
    if (wal_get_cmd_one_arg(pc_param, ac_value, OAL_SIZEOF(ac_value), &off_set) != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_ANY, "{uapi_ccpriv_set_wow_pattern::get value failed!}");
        return OAL_FAIL;
    }
    if ((*ac_value != '0') || ((*(ac_value + 1) != 'x') && (*(ac_value + 1) != 'X'))) {
        oam_warning_log0(0, OAM_SF_ANY, "{uapi_ccpriv_set_wow_pattern::invalid wow value}");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    wow_param->pattern_option = MAC_WOW_PATTERN_PARAM_OPTION_ADD;
    /* 长度减去 [ 空格 + 0 + X ] */
    pattern_len = (off_set > 3) ? ((off_set - 3) >> 1) : 0;  /* 3: 数据偏移量 */
    pattern_len = (pattern_len > WAL_CCPRIV_CMD_NAME_MAX_LEN) ? (WAL_CCPRIV_CMD_NAME_MAX_LEN) : pattern_len;
    wow_param->pattern_len = pattern_len;

    /* 转化 netpattern 格式, 命令方式最多支持 40字节 */
    for (idx = 0; idx < pattern_len; idx++) {
        /* 内容避开 [ 0 + X ]  */
        offset = 2; /* 2: 数据偏移 */
        if (oal_isxdigit(ac_value[(idx << 1) + offset]) && oal_isxdigit(ac_value[(idx << 1) + offset + 1])) {
            high = oal_isdigit(ac_value[(idx << 1) + offset]) ? (ac_value[(idx << 1) + offset] - '0') :
                   (oal_tolower((osal_u8)ac_value[(idx << 1) + offset]) - 'a' + 10); /* 10: 偏移量 */
            low  = oal_isdigit(ac_value[(idx << 1) + offset + 1]) ? (ac_value[(idx << 1) + offset + 1] - '0') :
                   (oal_tolower((osal_u8)ac_value[(idx << 1) + offset + 1]) - 'a' + 10); /* 10: 偏移量 */
            if ((osal_u8)high > 0xF || (osal_u8)low > 0xF) {
                oam_warning_log0(0, OAM_SF_ANY, "{uapi_ccpriv_set_wow_pattern::invalid wow value}");
                break;
            }
            wow_param->pattern_value[idx] = (((osal_u8)high & 0xF) << 4) | ((osal_u8)low & 0xF); /* 4: 左移4位 */
        } else {
            oam_warning_log0(0, OAM_SF_ANY, "{uapi_ccpriv_set_wow_pattern::invalid wow value}");
            break;
        }
    }
    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : ccpriv set wow pattern function
           ccpriv.sh 'wlan0 wow_pattern add index 0x983B16F8F39C'
           ccpriv.sh 'wlan0 wow_pattern del index'
           ccpriv.sh 'wlan0 wow_pattern clr'
*****************************************************************************/
osal_u32 uapi_ccpriv_set_wow_pattern(oal_net_device_stru *netdev, osal_s8 *pc_param)
{
    hmac_cfg_wow_pattern_param_stru cfg_wow_param = {0};
    td_char         *pc_end = OSAL_NULL;
    td_char          ac_option[WAL_CCPRIV_CMD_NAME_MAX_LEN] = {0};
    td_char          ac_index[WAL_CCPRIV_CMD_NAME_MAX_LEN] = {0};
    td_u32           retval, off_set;
    hmac_cfg_wow_pattern_param_stru cfg_pattern_wow_param;

    (osal_void)memset_s(&cfg_pattern_wow_param, OAL_SIZEOF(cfg_pattern_wow_param),
        0, OAL_SIZEOF(cfg_pattern_wow_param));

    /* No options ? lists existing pattern list */
    if (pc_param == OSAL_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* pc_param 指向'clr|add|del', 将其取出放到ac_option中 */
    if (wal_get_cmd_one_arg(pc_param, (osal_s8 *)ac_option, OAL_SIZEOF(ac_option), &off_set) != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_ANY, "{uapi_ccpriv_set_wow_pattern::get option failed");
        return OAL_FAIL;
    }
    pc_param += off_set;
    /* add wow事件 */
    if (osal_strcmp(ac_option, "add") == 0) {
        if (wal_ccpriv_get_add_wow_pattern_param(pc_param, &cfg_wow_param) != OAL_SUCC) {
            oam_error_log0(0, OAM_SF_ANY, "{uapi_ccpriv_set_wow_pattern::get add param failed.}");
            return OAL_FAIL;
        }
        if (memcpy_s(&(cfg_pattern_wow_param.pattern_value[0]),
                     WAL_CCPRIV_CMD_NAME_MAX_LEN, cfg_wow_param.pattern_value, cfg_wow_param.pattern_len) != EOK) {
            oam_error_log0(0, OAM_SF_ANY, "{uapi_ccpriv_set_wow_pattern::copy pattern value failed.}");
            return OAL_FAIL;
        }
    } else if (osal_strcmp(ac_option, "del") == 0) {
        /* pc_param 指向'index', 将其取出放到 ac_index 中 */
        if (wal_get_cmd_one_arg(pc_param, ac_index, OAL_SIZEOF(ac_index), &off_set) != OAL_SUCC) {
            oam_warning_log0(0, OAM_SF_ANY, "{uapi_ccpriv_set_wow_pattern::get del index failed!}");
            return OAL_FAIL;
        }
        cfg_wow_param.pattern_option = MAC_WOW_PATTERN_PARAM_OPTION_DEL;
        cfg_wow_param.pattern_index = (osal_u16)osal_strtol(ac_index, &pc_end, 10); /* 10: 转换为10进制 */
    } else if (osal_strcmp(ac_option, "clr") == 0) {
        cfg_wow_param.pattern_option = MAC_WOW_PATTERN_PARAM_OPTION_CLR;
    } else {
        oam_warning_log0(0, OAM_SF_ANY, "{uapi_ccpriv_set_wow_pattern::invalid pattern OPTION}");
        return OAL_FAIL;
    }

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    /* 设置pattern option参数 */
    cfg_pattern_wow_param.pattern_option = cfg_wow_param.pattern_option;
    /* 设置pattern index参数 */
    cfg_pattern_wow_param.pattern_index = cfg_wow_param.pattern_index;
    /* 设置pattern pattern len参数 */
    cfg_pattern_wow_param.pattern_len = cfg_wow_param.pattern_len;
    retval = wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(netdev), WLAN_MSG_W2H_CFG_SET_WOW_PATTERN,
        (osal_u8 *)&cfg_pattern_wow_param, OAL_SIZEOF(cfg_pattern_wow_param));
    if (osal_unlikely(retval != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_ccpriv_set_wow_pattern::return err code[%u]!}", retval);
    }

    return retval;
}
#endif

#if defined (_PRE_WLAN_FEATURE_M2U) && defined (_PRE_WLAN_CFGID_DEBUG)
/*****************************************************************************
 函 数 名  : uapi_ccpriv_m2u_snoop_enable
 功能描述  : 开启关闭组播转单播功能，配置模式。
*****************************************************************************/
osal_u32 uapi_ccpriv_m2u_snoop_enable(oal_net_device_stru *net_dev, osal_s8 *param)
{
    osal_s32 ret;
    osal_u32 off_set;
    osal_s8 *param_tmp = param;
    osal_s8 cmd_name[WAL_CCPRIV_CMD_NAME_MAX_LEN];
    mac_cfg_m2u_snoop_on_param_stru snoop_on_param;

    ret = (osal_s32)wal_get_cmd_one_arg(param_tmp, cmd_name, OAL_SIZEOF(cmd_name), &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_ccpriv_m2u_snoop_enable: return err_code [%d]!}", ret);
        return (osal_u32)ret;
    }
    snoop_on_param.m2u_snoop_on = (osal_u8)oal_atoi((const osal_s8 *)cmd_name);

    param_tmp += off_set;
    ret = (osal_s32)wal_get_cmd_one_arg(param_tmp, cmd_name, OAL_SIZEOF(cmd_name), &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_ccpriv_m2u_snoop_enable: return err_code [%d]!}", ret);
        return (osal_u32)ret;
    }
    snoop_on_param.m2u_mcast_mode = (osal_u8)oal_atoi((const osal_s8 *)cmd_name);

    /***************************************************************************
                             抛事件到wal层处理
    ***************************************************************************/
    /* 抛事件到wal层处理 */
    ret = wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(net_dev), WLAN_MSG_W2H_CFG_M2U_SNOOP_ON,
        (osal_u8 *)&snoop_on_param, OAL_SIZEOF(mac_cfg_m2u_snoop_on_param_stru));
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_11AX, "{uapi_ccpriv_m2u_snoop_enable::return err code[%d]!}", ret);
    }

    return (osal_u32)ret;
}

/*****************************************************************************
 函 数 名  : uapi_ccpriv_m2u_snoop_list
 功能描述  : 打印组播转单播表
*****************************************************************************/
osal_u32 uapi_ccpriv_m2u_snoop_list(oal_net_device_stru *net_dev, osal_s8 *param)
{
    osal_s32 ret;
    osal_u8 value = 0;
    unref_param(param);
    /***************************************************************************
                             抛事件到wal层处理
    ***************************************************************************/
    ret = wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(net_dev), WLAN_MSG_W2H_CFG_SHOW_M2U_SNOOP_TABLE,
        (osal_u8 *)&value, OAL_SIZEOF(osal_u8));
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_11AX, "{uapi_ccpriv_m2u_snoop_list::return err code[%d]!}", ret);
    }

    return (osal_u32)ret;
}

/*****************************************************************************
 函 数 名  : wal_ccpriv_m2u_snoop_deny_table_get_param
 功能描述  : 配置面的参数解析
*****************************************************************************/
OAL_STATIC osal_u32 wal_ccpriv_m2u_snoop_deny_table_get_param(osal_s8 *param,
    mac_cfg_m2u_deny_table_stru *cfg_deny_table)
{
    osal_u32 off_set;
    osal_s32 ret;
    osal_s8 *param_tmp = param;
    osal_s8 cmd_name[WAL_CCPRIV_CMD_NAME_MAX_LEN] = {0};
    osal_u8 i;
    struct {
        const char *key_string;
        osal_u8 cfg_type;
        osal_void(*oal_strtoip)(const osal_s8 *ip_str, osal_u8 *ip_addr);
        osal_u8 *ip_addr;
    } cfg_m2u_deny_table [] = {
        {"list",     HMAC_H2U_DENY_TABLE_LIST, OSAL_NULL, OSAL_NULL},
        {"clear",    HMAC_H2U_DENY_TABLE_CLEAR, OSAL_NULL, OSAL_NULL},
        {"add_ipv4", HMAC_H2U_DENY_TABLE_ADD_IPV4, oal_strtoipv4, (osal_u8 *)(&cfg_deny_table->deny_group_ipv4_addr)},
        {"add_ipv6", HMAC_H2U_DENY_TABLE_ADD_IPV6, oal_strtoipv6, cfg_deny_table->deny_group_ipv6_addr},
        {"del_ipv4", HMAC_H2U_DENY_TABLE_DEL_IPV4, oal_strtoipv4, (osal_u8 *)(&cfg_deny_table->deny_group_ipv4_addr)},
        {"del_ipv6", HMAC_H2U_DENY_TABLE_DEL_IPV6, oal_strtoipv6, cfg_deny_table->deny_group_ipv6_addr}
    };

    ret = (osal_s32)wal_get_cmd_one_arg(param_tmp, cmd_name, OAL_SIZEOF(cmd_name), &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{m2u_snoop_deny_table_get_param:err_code [%d]!}", ret);
        return (osal_u32)ret;
    }

    for (i = 0; i < sizeof(cfg_m2u_deny_table) / sizeof(cfg_m2u_deny_table[0]); i++) {
        if (osal_strcmp(cfg_m2u_deny_table[i].key_string, (const osal_char *)cmd_name) == 0) { // 匹配命令字符串
            cfg_deny_table->cfg_type = cfg_m2u_deny_table[i].cfg_type;
            if (cfg_m2u_deny_table[i].oal_strtoip == OSAL_NULL) {
                break;
            }
            param_tmp += off_set;
            ret = (osal_s32)wal_get_cmd_one_arg(param_tmp, cmd_name, OAL_SIZEOF(cmd_name), &off_set);
            if (ret != OAL_SUCC) {
                oam_warning_log1(0, OAM_SF_ANY, "{m2u_snoop_deny_table_get_param:err_code [%d]!}", ret);
                return (osal_u32)ret;
            }
            cfg_m2u_deny_table[i].oal_strtoip(cmd_name, cfg_m2u_deny_table[i].ip_addr); // 字符串转化为ip存放
            break;
        } else {
            if (i == (sizeof(cfg_m2u_deny_table) / sizeof(cfg_m2u_deny_table[0])) - 1) { // 最后一个字符串不匹配
                return OAL_ERR_CODE_INVALID_CONFIG;
            }
        }
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : uapi_ccpriv_m2u_snoop_list
 功能描述  : 组播转单播黑名单操作
*****************************************************************************/
osal_u32 uapi_ccpriv_m2u_snoop_deny_table(oal_net_device_stru *net_dev, osal_s8 *param)
{
    osal_s32 ret;
    mac_cfg_m2u_deny_table_stru cfg_deny_table = {0};

    ret = (osal_s32)wal_ccpriv_m2u_snoop_deny_table_get_param(param, &cfg_deny_table); /* 从param中解析参数填充到事件中 */
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_PWR, "{uapi_ccpriv_m2u_snoop_deny_table::return err code [%d]!}", ret);
        return OAL_FAIL;
    }

    ret = wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(net_dev), WLAN_MSG_W2H_CFG_M2U_DENY_TABLE,
        (osal_u8 *)&cfg_deny_table, OAL_SIZEOF(mac_cfg_m2u_deny_table_stru));
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_11AX, "{uapi_ccpriv_m2u_snoop_deny_table::return err code[%d]!}", ret);
    }

    return (osal_u32)ret;
}

/*****************************************************************************
 函 数 名  : wal_ccpriv_m2u_snoop_send_igmp_get_param
 功能描述  : 配置面的参数解析
*****************************************************************************/
OAL_STATIC osal_u32 wal_ccpriv_m2u_snoop_send_igmp_get_param(osal_s8 *param, mac_cfg_mpdu_ampdu_tx_param_stru *tx_param)
{
    osal_u32 off_set;
    osal_u32 ret;
    osal_s8 *param_tmp = param;
    osal_s8 cmd_name[WAL_CCPRIV_CMD_NAME_MAX_LEN] = {0};

    ret = wal_cmd_get_u8_with_check_max(&param_tmp, WLAN_TID_MAX_NUM - 1, &tx_param->tid);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_ccpriv_m2u_snoop_send_igmp_get_param:err_code [%d]!}", ret);
        return ret;
    }

    ret = wal_get_cmd_one_arg(param_tmp, cmd_name, OAL_SIZEOF(cmd_name), &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_ccpriv_m2u_snoop_send_igmp_get_param:err_code [%d]!}", ret);
        return ret;
    }
    tx_param->packet_num = (osal_u8)oal_atoi((const osal_s8 *)cmd_name);
    param_tmp += off_set;

    ret = wal_get_cmd_one_arg(param_tmp, cmd_name, OAL_SIZEOF(cmd_name), &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_ccpriv_m2u_snoop_send_igmp_get_param:err_code [%d]!}", ret);
        return ret;
    }
    tx_param->packet_len = (osal_u8)oal_atoi((const osal_s8 *)cmd_name);
    if (tx_param->packet_len < 50) {  /* 报文长度不允许小于50 */
        return OAL_FAIL;
    }
    param_tmp += off_set;

    ret = wal_get_cmd_one_arg(param_tmp, cmd_name, OAL_SIZEOF(cmd_name), &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_ccpriv_m2u_snoop_send_igmp_get_param:err_code [%d]!}", ret);
        return ret;
    }
    oal_strtoaddr((const osal_char *)cmd_name, tx_param->ra_mac);
    param_tmp += off_set;
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : uapi_ccpriv_m2u_snoop_list
 功能描述  : 构造并发送igmp报文
*****************************************************************************/
osal_u32 uapi_ccpriv_m2u_snoop_send_igmp(oal_net_device_stru *net_dev, osal_s8 *param)
{
    osal_s32 ret;
    mac_cfg_mpdu_ampdu_tx_param_stru ampdu_tx_param = {0};

    ret = (osal_s32)wal_ccpriv_m2u_snoop_send_igmp_get_param(param, &ampdu_tx_param); /* 从param中解析参数填充到事件中 */
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_PWR, "{uapi_ccpriv_m2u_snoop_deny_table::return err code [%d]!}", ret);
        return OAL_FAIL;
    }

    ret = wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(net_dev), WLAN_MSG_W2H_CFG_IGMP_PACKET_XMIT,
        (osal_u8 *)&ampdu_tx_param, OAL_SIZEOF(mac_cfg_mpdu_ampdu_tx_param_stru));
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_11AX, "{uapi_ccpriv_m2u_snoop_enable::return err code[%d]!}", ret);
    }

    return (osal_u32)ret;
}
#endif

#ifdef _PRE_WLAN_CFGID_DEBUG
/*****************************************************************************
 功能描述  : Keepalive配置模式命令
*****************************************************************************/
osal_u32 uapi_ccpriv_set_keepalive_mode(oal_net_device_stru *net_dev, osal_s8 *param)
{
    osal_u32 ret;
    osal_u32 keepalive_mode;
    osal_s32 ret_val;

    if (wal_ccpriv_check_cfg_vap(net_dev) == OSAL_TRUE) {
        oam_warning_log0(0, OAM_SF_ANY, "uapi_ccpriv_set_keepalive_mode vap mode err: cfg");
        return OAL_FAIL;
    }
    /*
     * 设置keepalive mode命令:
     * liteos: AT+CCPRIV=wlan0,set_keepalive_mode,0/1
     * linux: echo "wlan0 set_keepalive_mode 0/1" > /sys/ccsys/ccpriv
     */
    oam_warning_log0(0, OAM_SF_ANY, "{call uapi_ccpriv_set_keepalive_mode!}");
    /* 最大值为1, 0/1表示关闭 打开 keepalive功能 */
    ret = wal_cmd_get_digit_with_check_max(&param, 1, &keepalive_mode);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_ccpriv_set_keepalive_mode::get keepalive_mode err[%d]!}", ret);
        return ret;
    }
    ret_val = wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(net_dev), WLAN_MSG_W2H_C_CFG_SET_KEEPALIVE_MODE,
        (osal_u8 *)&keepalive_mode, OAL_SIZEOF(keepalive_mode));
    if (OAL_UNLIKELY(ret_val != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_ccpriv_set_keepalive_mode::return err code [%d]!}", ret);
    }
    return (osal_u32)ret_val;
}

/*****************************************************************************
 功能描述  : 通用debug指令
*****************************************************************************/
osal_u32  uapi_ccpriv_common_debug(oal_net_device_stru *net_dev, osal_s8 *param)
{
    osal_u32 ret;
    osal_u32 common_debug_index;
    osal_s32 ret_val;

    /*
     * 设置keepalive mode命令:
     * liteos: AT+CCPRIV=wlan0,common_debug,X
     * linux: echo "wlan0 common_debug 0" > /sys/ccsys/ccpriv
     * 0 ---- keepalive
     */
    /* 最大值为1, 0/1表示关闭 打开 keepalive功能 */
    ret = wal_cmd_get_digit_with_check_max(&param, COMMON_DEBUG_BUTT, &common_debug_index);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_ccpriv_common_debug::get keepalive_mode err[%d]!}", ret);
        return ret;
    }
    oam_warning_log1(0, OAM_SF_ANY, "{call uapi_ccpriv_common_debug [%d]!}", common_debug_index);
    ret_val = wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(net_dev), WLAN_MSG_W2H_C_CFG_COMMON_DEBUG,
        (osal_u8 *)&common_debug_index, OAL_SIZEOF(common_debug_index));
    if (OAL_UNLIKELY(ret_val != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_ccpriv_common_debug::return err code [%d]!}", ret);
    }
    return (osal_u32)ret_val;
}
#endif

#ifdef _PRE_WLAN_FEATURE_11K
osal_u32 wal_handel_radio_meas_rpt_cmd(mac_cfg_radio_meas_info_stru *radio_meas_cfg, osal_s8 *param)
{
    osal_u32 ret;
    osal_u32 off_set = 0;
    osal_u8 mac_addr[WLAN_MAC_ADDR_LEN] = {0};    /* MAC地址 */
    osal_s8 name[WAL_CCPRIV_CMD_NAME_MAX_LEN] = {0};

    if (radio_meas_cfg->action_type == MAC_RM_ACTION_RADIO_MEASUREMENT_REQUEST) {
        /* meas type */
        ret = wal_get_cmd_one_arg(param, name, OAL_SIZEOF(name), &off_set);
        if (ret != OAL_SUCC) {
            oam_warning_log1(0, OAM_SF_ANY, "{wal_handel_radio_meas_rpt_cmd::type err_code [%d]!}", ret);
            return ret;
        }
        radio_meas_cfg->means_type = (osal_u8)oal_atoi((const osal_s8 *)name);

        /* meas duration */
        param += off_set;
        ret = wal_get_cmd_one_arg(param, name, OAL_SIZEOF(name), &off_set);
        if (ret != OAL_SUCC) {
            oam_warning_log1(0, OAM_SF_ANY, "{wal_handel_radio_meas_rpt_cmd:: duration err_code [%d]!}", ret);
            return ret;
        }
        radio_meas_cfg->duration = (osal_u16)oal_atoi((const osal_s8 *)name);

        /* channum */
        param += off_set;
        ret = wal_get_cmd_one_arg(param, name, OAL_SIZEOF(name), &off_set);
        if (ret != OAL_SUCC) {
            oam_warning_log1(0, OAM_SF_ANY, "{wal_handel_radio_meas_rpt_cmd::channum err_code [%d]!}", ret);
            return ret;
        }
        radio_meas_cfg->channum = (osal_u8)oal_atoi((const osal_s8 *)name);

        oam_warning_log3(0, OAM_SF_ANY, "{wal_handel_radio_meas_rpt_cmd::duration=%d, means_type=%d, channum=%d}",
                         radio_meas_cfg->duration, radio_meas_cfg->means_type, radio_meas_cfg->channum);

        if (radio_meas_cfg->means_type == RM_RADIO_MEAS_BCN) {
            /* means_mode for bcn */
            param += off_set;
            ret = wal_get_cmd_one_arg(param, name, OAL_SIZEOF(name), &off_set);
            if (ret != OAL_SUCC) {
                oam_warning_log1(0, OAM_SF_ANY, "{wal_handel_radio_meas_rpt_cmd::bcn_mode err_code [%d]!}", ret);
                return ret;
            }
            radio_meas_cfg->bcn_mode = (osal_u8)oal_atoi((const osal_s8 *)name);

            /* 获取BSSID */
            param += off_set;
            ret = wal_ccpriv_get_mac_addr_etc(param, (osal_u8 *)mac_addr, &off_set);
            if (ret != OAL_SUCC) {
                oam_warning_log0(0, OAM_SF_ANY, "{wal_handel_radio_meas_rpt_cmd::bssid failed!}");
                return ret;
            }
            oal_set_mac_addr(radio_meas_cfg->bssid, (osal_u8 *)mac_addr);
            oam_warning_log4(0, OAM_SF_ANY, "{wal_handel_radio_meas_rpt_cmd: bssid=%x:%x:%x:%x:xx:xx}",
                radio_meas_cfg->bssid[0], radio_meas_cfg->bssid[1],
                radio_meas_cfg->bssid[2], radio_meas_cfg->bssid[3]); /* index: 2, 3 */
        }
    }
    return OAL_SUCC;
}
#endif

#ifdef _PRE_WLAN_CFGID_DEBUG
osal_u32 uapi_ccpriv_dhcp_debug(oal_net_device_stru *net_dev, osal_s8 *param)
{
    osal_u32 off_set;
    osal_s32 value, ret;
    osal_u8 args[WAL_CCPRIV_CMD_NAME_MAX_LEN] = { 0 };

    /* 获取第一个参数 type */
    ret = wal_get_cmd_one_arg_digit(param, (osal_s8 *)args, WAL_CCPRIV_CMD_NAME_MAX_LEN, &off_set, &value);
    if ((ret != OAL_SUCC) || (value < 0) || (value > 1)) {
        oam_error_log2(0, OAM_SF_ANY, "{uapi_ccpriv_dhcp_debug::ret:%d value:%d !}", ret, value);
        return OAL_ERR_CODE_CONFIG_UNSUPPORT;
    }

    oam_warning_log1(0, OAM_SF_ANY, "{uapi_ccpriv_dhcp_debug::value [%d]}", value);

    ret = wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(net_dev), WLAN_MSG_W2H_CFG_GET_DHCP_DEBUG,
        (osal_u8 *)&value, OAL_SIZEOF(osal_u8));
    if (osal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_ccpriv_dhcp_debug::event return err code [%d]!}", ret);
        return (osal_u32)ret;
    }
    return (osal_u32)ret;
}
#endif

osal_u32 uapi_ccpriv_set_2040_coext_support(oal_net_device_stru *net_dev, osal_s8 *param)
{
    osal_u32 off_set;
    osal_u32 ret;
    osal_s32 ret1;
    osal_u8 csp;
    osal_s8 ac_name[WAL_CCPRIV_CMD_NAME_MAX_LEN];

    ret = wal_get_cmd_one_arg(param, ac_name, OAL_SIZEOF(ac_name), &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{uapi_ccpriv_set_2040_coext_support::wal_get_cmd_one_arg return err_code %d!}", ret);
        return ret;
    }

    if ((osal_strcmp("0", (const osal_char *)ac_name)) == 0) {
        csp = 0;
    } else if ((osal_strcmp("1", (const osal_char *)ac_name)) == 0) {
        csp = 1;
    } else {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_ccpriv_set_2040_coext_support::the command is err %p!}",
                         (uintptr_t)ac_name);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    /***************************************************************************
                             抛事件到wal层处理
    ***************************************************************************/
    ret1 = wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(net_dev), WLAN_MSG_W2H_CFG_2040_COEXISTENCE,
        (osal_u8 *)&csp, OAL_SIZEOF(osal_s32));
    if (OAL_UNLIKELY(ret1 != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_ccpriv_set_2040_coext_support::return err code %d!}", ret1);
        return (osal_u32)ret1;
    }
    return OAL_SUCC;
}

#ifdef _PRE_WLAN_CFGID_DEBUG
/*****************************************************************************
 功能描述  : 解析命令第二个参数
 被调函数  : uapi_ccpriv_set_scan_param(oal_net_device_stru *net_dev, osal_s8 *param);
*****************************************************************************/
OAL_STATIC osal_u32 wal_ccpriv_parse_second_arg(oal_net_device_stru *net_dev, osal_s8 **param,
    mac_cfg_set_scan_param *scan_param)
{
    osal_u32 off_set, i;
    osal_s32 value, ret;
    osal_s8 args[WAL_CCPRIV_CMD_NAME_MAX_LEN] = { 0 };
    wlan_channel_band_enum_uint8 channel_band;
    unref_param(net_dev);
    if (scan_param->type == SCAN_PARAM_CHANNEL_LIST) {
        /* 获取第二个参数 信道个数 0~43 */
        ret = wal_get_cmd_one_arg_digit(*param, args, OAL_SIZEOF(args), &off_set, &value);
        if (ret != OAL_SUCC) {
            oam_warning_log1(0, OAM_SF_ANY, "{wal_ccpriv_parse_second_arg::parse channel Enable failed [%d]!}", ret);
            return (osal_u32)ret;
        }
        *param += off_set;
        scan_param->value = (osal_u16)value;
        if (scan_param->value > OAL_ARRAY_SIZE(scan_param->chan_list)) {
            return  OAL_ERR_CODE_CONFIG_UNSUPPORT;
        }
        for (i = 0; i < scan_param->value; i++) {
            /* 获取信道列表，中间以空格分隔 */
            ret = (osal_s32)wal_get_cmd_one_arg_digit(*param, args, OAL_SIZEOF(args), &off_set, &value);
            if (ret != OAL_SUCC) {
                oam_warning_log1(0, OAM_SF_ANY, "{wal_ccpriv_parse_second_arg::parse arg failed [%d]!}", ret);
                return (osal_u32)ret;
            }

            channel_band = mac_get_band_by_channel_num((osal_u8)value);
            ret = (osal_s32)hmac_is_channel_num_valid_etc(channel_band, (osal_u8)value);
            if (ret != OAL_SUCC) {
                oam_warning_log1(0, OAM_SF_ANY, "{wal_ccpriv_parse_second_arg::channel invalid [%u]!}", value);
                return OAL_ERR_CODE_CONFIG_UNSUPPORT;
            }

            *param += off_set;
            scan_param->chan_list[i] = (osal_u8)value;
            if (**param == '\0') {
                break;
            }
        }
    } else {
        /* 获取第二个参数 包括除信道列表之外的参数类型 */
        ret = wal_get_cmd_one_arg_digit((osal_s8 *)(*param), args, WAL_CCPRIV_CMD_NAME_MAX_LEN, &off_set, &value);
        if (ret != OAL_SUCC) {
            oam_warning_log1(0, OAM_SF_ANY, "{wal_ccpriv_parse_second_arg::parse second arg failed [%d]!}", ret);
            return (osal_u32)ret;
        }
        scan_param->value = (osal_u16)value;
    }
    return OAL_SUCC;
}

osal_u32 uapi_ccpriv_set_scan_param(oal_net_device_stru *net_dev, osal_s8 *param)
{
    osal_u32 off_set, ret_code;
    osal_s32 value, ret;
    osal_s8 *param_tmp = param;
    osal_s8 args[WAL_CCPRIV_CMD_NAME_MAX_LEN] = { 0 };

    /* 解析并设置配置命令参数 */
    /* echo "wlan0 set_scan_param 5 6 1 6 9 36 40 165" > /sys/ccsys/ccpriv
       参数5表示类型为信道列表，参数6为信道个数，1 6 9 36 40 165表示信道列表 */
    /* echo "wlan0 set_scan_param 6 2" > /sys/ccsys/ccpriv
       参数6表示类型为扫描间隔，参数为2表示间隔信道个数 */
    mac_cfg_set_scan_param scan_param = {0};

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    /* 获取第一个参数 type */
    ret = wal_get_cmd_one_arg_digit(param_tmp, args, OAL_SIZEOF(args), &off_set, &value);
    if ((ret != OAL_SUCC) || (value <= SCAN_PARAM_START) || (value >= SCAN_PARAM_MAX)) {
        oam_error_log2(0, OAM_SF_ANY, "{uapi_ccpriv_set_scan_param::ret:%d value:%d !}", ret, value);
        return OAL_ERR_CODE_CONFIG_UNSUPPORT;
    }

    param_tmp += off_set;
    scan_param.type = (osal_u8)value;

    ret_code = wal_ccpriv_parse_second_arg(net_dev, &param_tmp, &scan_param);
    if (ret_code != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_ccpriv_set_scan_param::parse_second_arg err [%d]!}", ret_code);
        return ret_code;
    }

    oam_warning_log3(0, OAM_SF_ANY, "{uapi_ccpriv_set_scan_param::type [%d] value[%d] stru size[%d]!}",
                     scan_param.type, scan_param.value, sizeof(mac_cfg_set_scan_param));

    ret_code = wal_ccpriv_check_scan_params(&scan_param);
    if (ret_code != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_ccpriv_set_scan_param:wal_ccpriv_check_scan_params err[%d]}", ret_code);
        return ret_code;
    }

    ret = wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(net_dev), WLAN_MSG_W2H_CFG_SET_SCAN_PARAM,
        (osal_u8 *)&scan_param, OAL_SIZEOF(mac_cfg_set_scan_param));
    if (osal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_ccpriv_set_scan_param::event return err code [%d]!}", ret);
        return (osal_u32)ret;
    }
    return (osal_u32)ret;
}
#endif

#ifdef _PRE_WLAN_DFT_STAT
osal_u32 uapi_ccpriv_get_vap_sniffer_result(oal_net_device_stru *net_dev, osal_s8 *param)
{
    osal_s32 ret;
    frw_msg cfg_info = {0};
    wlan_scan_chan_stats_stru sniffer_info = {0};
    unref_param(param);

    ret = wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(net_dev), WLAN_MSG_W2H_CFG_START_SNIFFER_INFO,
        OAL_PTR_NULL, 0);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "uapi_ccpriv_get_vap_sniffer_result:w2h event return err [%d]", ret);
        return (osal_u32)ret;
    }

    cfg_msg_init(OAL_PTR_NULL, 0, (osal_u8 *)&sniffer_info, OAL_SIZEOF(sniffer_info), &cfg_info);
    ret = send_sync_cfg_to_host(wal_util_get_vap_id(net_dev), WLAN_MSG_W2H_CFG_GET_SNIFFER_INFO, &cfg_info);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_ANY,
                       "{uapi_ccpriv_get_vap_sniffer_result::send_sync_cfg_to_host return err code [%d]!}", ret);
        return (osal_u32)ret;
    }

    wifi_printf("====================vap sniffer info========================\r\n");
    wifi_printf("Total_time                      = %d\r\n", sniffer_info.total_stats_time_us);
    wifi_printf("Free_time_20M                   = %d\r\n", sniffer_info.total_free_time_20m_us);
    wifi_printf("Free_time_40M                   = %d\r\n", sniffer_info.total_free_time_40m_us);
    wifi_printf("Free_time_80M                   = %d\r\n", sniffer_info.total_free_time_80m_us);
    wifi_printf("Rx_time                         = %d\r\n", sniffer_info.total_recv_time_us);
    wifi_printf("Tx_time                         = %d\r\n", sniffer_info.total_send_time_us);
    wifi_printf("\r\n");

    return OAL_SUCC;
}
#endif

osal_u32 wal_ccpriv_parse_ipv4(osal_char *ip_str, osal_u32 *result)
{
    const osal_char *delims = ".";
    osal_char *byte = OSAL_NULL;
    osal_char *pcmdstr = OSAL_NULL;
    osal_u8 count = 0;
    osal_u32 byte_value;

    if (osal_strlen(ip_str) > 15) { /* IPV4 字符串长度最大 15 字节 */
        oam_error_log0(0, OAM_SF_ANY, "{wal_ccpriv_parse_ipv4::ip len wrong!}");
        return OAL_FAIL;
    }

    byte = strtok_s(ip_str, delims, &pcmdstr);
    while (byte != OSAL_NULL) {
        byte_value = (osal_u32)oal_atoi((const osal_s8 *)byte);
        byte_value <<= count * 8; /* 移位3次，每次8比特。 */
        count++;
        *result += byte_value;
        byte = strtok_s(OSAL_NULL, delims, &pcmdstr);
    }

    if (count != 4) { /* IPV4的长度为4个字节 */
        oam_error_log0(0, OAM_SF_ANY, "{wal_ccpriv_parse_ipv4::ip format wrong!}");
        return OAL_FAIL;
    }

    return OAL_SUCC;
}

osal_void wal_mem_read(osal_u8 reg32, uintptr_t start_addr, osal_u32 read_number)
{
    osal_u32 i;
    osal_u32 format = 0;

    if (reg32 != 0) {
        osal_u32 val;
        if (start_addr % 4 != 0) { /* 32 bit读，地址需要4字节对齐 */
            return;
        }
        for (i = 0; i < read_number * 4; i = i + 4) { /* 4:乘4 加4 */
            val = OAL_REG_READ32(start_addr + i);
            wifi_printf("0x%X", val);
            wifi_printf(" ");

            format = format + 1;
            if (format % 4 == 0) { /* 4:除4取余数 */
                wifi_printf("\r\n");
            }
        }
    } else { /* 16bit读需要 2字节对齐 */
        osal_u16 val;
        if (start_addr % 2 != 0) { /* 16 bit读，地址需要2 byte对齐 */
            return;
        }
        for (i = 0; i < read_number * 2; i = i + 2) { /* 2:乘2 加2 */
            val = OAL_REG_READ16(start_addr + i);
            wifi_printf("0x%X", val);
            wifi_printf(" ");

            format = format + 1;
            if (format % 4 == 0) { /* 4:除4取余数 */
                wifi_printf("\r\n");
            }
        }
    }
    wifi_printf("\r\n");
}

#ifdef _PRE_WLAN_FEATURE_11AX
/*****************************************************************************
 功能描述  : 设置描述符参数配置命令
*****************************************************************************/
osal_u32  uapi_ccpriv_set_sts_sung_mung(oal_net_device_stru *net_dev, osal_s8 *param)
{
    osal_u32 off_set;
    osal_u32 ret;
    osal_s8 *parm_tmp = param;
    osal_s8 arg[WAL_CCPRIV_CMD_NAME_MAX_LEN];
    hmac_vap_stru *hmac_vap = OAL_PTR_NULL;

    hmac_vap = (hmac_vap_stru *)net_dev->ml_priv;
    if (hmac_vap == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{uapi_ccpriv_set_sts_sung_mung::net_dev->ml_priv is null ptr.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (wal_ccpriv_check_cfg_vap(net_dev) == OSAL_TRUE) {
        oam_warning_log0(0, OAM_SF_ANY, "uapi_ccpriv_set_sts_sung_mung err cfg mode: vap");
        return OAL_ERR_CODE_PTR_NULL;
    }
    // 获取第一个参数beamformee_sts_below_80mhz
    ret = wal_get_cmd_one_arg(parm_tmp, arg, OAL_SIZEOF(arg), &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_ccpriv_set_sts_sung_mung::wal_get_cmd_one_arg err [%d]}", ret);
        return ret;
    }

    hmac_vap->beamformee_sts_below_80mhz = (osal_u8)oal_atoi((const osal_s8 *)arg);

    // 获取第二个参数ng16_su_feedback
    parm_tmp += off_set;
    ret = wal_get_cmd_one_arg(parm_tmp, arg, OAL_SIZEOF(arg), &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_ccpriv_set_sts_sung_mung::wal_get_cmd_one_arg err=[%d]}", ret);
        return ret;
    }

    hmac_vap->ng16_su_feedback = (osal_u8)oal_atoi((const osal_s8 *)arg);

    // 获取第三个参数ng16_mu_feedback
    parm_tmp += off_set;
    ret = wal_get_cmd_one_arg(parm_tmp, arg, OAL_SIZEOF(arg), &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_ccpriv_set_sts_sung_mung::wal_get_cmd_one_arg err is %d}", ret);
        return ret;
    }

    hmac_vap->ng16_mu_feedback = (osal_u8)oal_atoi((const osal_s8 *)arg);

    if (hmac_vap->ng16_mu_feedback > 1) {
        hmac_vap->ng16_mu_feedback = 1; /* 该mib变量是bool类型, 只支持0 和 1 */
    }
    mac_mib_set_he_ng16_mufeedback_support(hmac_vap, hmac_vap->ng16_mu_feedback);

    return OAL_SUCC;
}
#endif

#ifdef _PRE_WLAN_CFGID_DEBUG
#ifdef _PRE_WLAN_SMOOTH_PHASE
osal_u32 uapi_ccpriv_set_smooth_phase_en(oal_net_device_stru *net_dev, osal_s8 *param)
{
    osal_u32 ret;
    osal_u32 off_set;
    osal_s8 *param_tmp = param;
    osal_s8 cmd_name[WAL_CCPRIV_CMD_NAME_MAX_LEN];
    smooth_phase_stru smooth_phase_info;

    ret = wal_get_cmd_one_arg(param_tmp, cmd_name, OAL_SIZEOF(cmd_name), &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_ccpriv_set_smooth_phase_en: return err_code [%d]!}", ret);
        return ret;
    }
    smooth_phase_info.enable = (osal_u8)oal_atoi((const osal_s8 *)cmd_name);

    param_tmp += off_set;
    ret = wal_get_cmd_one_arg(param_tmp, cmd_name, OAL_SIZEOF(cmd_name), &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_ccpriv_set_smooth_phase_en: return err_code [%d]!}", ret);
        return ret;
    }
    smooth_phase_info.extra_value = oal_atoi((const osal_s8 *)cmd_name);

    ret = (osal_u32)wal_sync_send2device_no_rsp(wal_util_get_vap_id(net_dev), WLAN_MSG_W2D_C_CFG_SMOOTH_PHASE,
        (osal_u8 *)&smooth_phase_info, OAL_SIZEOF(smooth_phase_info));
    if (OAL_UNLIKELY(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_PWR, "{uapi_ccpriv_set_smooth_phase_en::return err code [%d]!}", ret);
        return ret;
    }

    return OAL_SUCC;
}
#endif
#endif
/*****************************************************************************
 功能描述  : rssi门限开关和阈值 配置 - 获取配置值
 被调函数  : uapi_ccpriv_rssi_limit(oal_net_device_stru *net_dev, osal_s8 *param);
*****************************************************************************/
OAL_STATIC osal_u32 wal_ccpriv_get_rssi_limit_cfg(osal_s8 **param, mac_cfg_rssi_limit_stru *rssi_limit)
{
    osal_u32 ret_code;
    osal_u32 off_set;
    osal_s8 ac_name[WAL_CCPRIV_CMD_NAME_MAX_LEN];

    /* 获取参数的具体配置值 */
    if (rssi_limit->rssi_limit_type != MAC_RSSI_LIMIT_SHOW_INFO) {
        ret_code = wal_get_cmd_one_arg(*param, ac_name, OAL_SIZEOF(ac_name), &off_set);
        *param = *param + off_set;
        if (ret_code != OAL_SUCC) {
            oam_warning_log1(0, OAM_SF_ANY,
                "{wal_ccpriv_get_rssi_limit_cfg::wal_get_cmd_one_arg return err_code[%d]!}", ret_code);
            return ret_code;
        }
    }

    if (rssi_limit->rssi_limit_type == MAC_RSSI_LIMIT_DELTA) {
        rssi_limit->rssi_delta = (osal_s8)oal_atoi((const osal_s8 *)ac_name);
    } else if (rssi_limit->rssi_limit_type == MAC_RSSI_LIMIT_ENABLE) {
        rssi_limit->rssi_limit_enable_flag = (oal_bool_enum_uint8)oal_atoi((const osal_s8 *)ac_name);
    } else if (rssi_limit->rssi_limit_type == MAC_RSSI_LIMIT_THRESHOLD) {
        rssi_limit->rssi = (osal_s8)oal_atoi((const osal_s8 *)ac_name);
        /* 若配置的rssi值超过了合法范围，函数直接返回 */
        if ((rssi_limit->rssi < OAL_RSSI_SIGNAL_MIN) || (rssi_limit->rssi > OAL_RSSI_SIGNAL_MAX)) {
            oam_warning_log1(0, OAM_SF_ANY,
                "{wal_ccpriv_get_rssi_limit_cfg:: rssi limit err [%d]}", (osal_s32)rssi_limit->rssi);
            return OAL_FAIL;
        }

        if (rssi_limit->rssi > WLAN_FAR_DISTANCE_RSSI) {
            oam_warning_log2(0, OAM_SF_ANY, "{wal_ccpriv_get_rssi_limit_cfg::rssi_limit exceed WLAN_FAR_DISTANCE_RSSI! \
                s_rssi[%d], WLAN_FAR_DISTANCE_RSSI[%d]}", (osal_s32)rssi_limit->rssi, WLAN_FAR_DISTANCE_RSSI);
        }
    }
    return OAL_SUCC;
}
/*****************************************************************************
 功能描述  : rssi门限开关和阈值 配置
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
osal_u32 uapi_ccpriv_rssi_limit(oal_net_device_stru *net_dev, osal_s8 *param)
{
    osal_s8 ac_name[WAL_CCPRIV_CMD_NAME_MAX_LEN];
    osal_u8 idx;
    osal_s32 ret;
    osal_s8 *param_tmp = param;
    osal_u32 ret_code;
    osal_u32 off_set;
    mac_cfg_rssi_limit_stru *rssi_limit;
    mac_cfg_rssi_limit_stru rssi_limit_info;
    OAL_CONST mac_rssi_cfg_table_stru rssi_config_table[] = {
    {"show_info",         MAC_RSSI_LIMIT_SHOW_INFO, {0, 0, 0}},
    {"enable",            MAC_RSSI_LIMIT_ENABLE, {0, 0, 0}},
    {"delta",             MAC_RSSI_LIMIT_DELTA, {0, 0, 0}},
    {"threshold",         MAC_RSSI_LIMIT_THRESHOLD, {0, 0, 0}}
    };
    (osal_void)memset_s(&rssi_limit_info, OAL_SIZEOF(rssi_limit_info), 0, OAL_SIZEOF(rssi_limit_info));
    rssi_limit = &rssi_limit_info;

    /* 获取rssi_limit 参数 */
    ret_code = wal_get_cmd_one_arg(param_tmp, ac_name, OAL_SIZEOF(ac_name), &off_set);
    param_tmp += off_set;
    if (ret_code != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_ccpriv_rssi_limit::wal_get_cmd_one_arg err_code [%d]!}", ret_code);
        return ret_code;
    }

    /* 根据输入的参数来获取对应的参数配置类型 */
    for (idx = 0; idx < MAC_RSSI_LIMIT_TYPE_BUTT; idx++) {
        if (osal_strcmp((const osal_char *)ac_name, rssi_config_table[idx].car_name) == 0) {
            break;
        }
    }
    if (idx == MAC_RSSI_LIMIT_TYPE_BUTT) {
        oam_warning_log0(0, OAM_SF_CAR, "{uapi_ccpriv_rssi_limit:: parameter error !");
        return idx;
    }
    rssi_limit->rssi_limit_type = rssi_config_table[idx].rssi_cfg_id;

    /* 获取参数的具体配置值 */
    ret_code = wal_ccpriv_get_rssi_limit_cfg(&param_tmp, rssi_limit);
    if (ret_code != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_ccpriv_rssi_limit:wal_ccpriv_get_rssi_limit_cfg err [%d]!}", ret_code);
        return ret_code;
    }
    ret = wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(net_dev), WLAN_MSG_W2H_CFG_RSSI_LIMIT_CFG,
        (osal_u8 *)&rssi_limit_info, OAL_SIZEOF(rssi_limit_info));
    if (OAL_UNLIKELY(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_CAR, "{uapi_ccpriv_rssi_limit::wal_sync_post2hmac_no_rsp err [%d]!}", ret);
        return (osal_u32)ret;
    }

    return OAL_SUCC;
}
#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#ifdef _PRE_WLAN_CFGID_DEBUG
EXPORT_SYMBOL(uapi_ccpriv_send_random_mac_oui);
EXPORT_SYMBOL(uapi_ccpriv_kick_user);
EXPORT_SYMBOL(uapi_ccpriv_get_nvram_power);
EXPORT_SYMBOL(uapi_ccpriv_set_scan_param);
EXPORT_SYMBOL(uapi_ccpriv_set_ampdu_amsdu);
EXPORT_SYMBOL(uapi_ccpriv_set_keepalive_mode);
EXPORT_SYMBOL(uapi_ccpriv_common_debug);
EXPORT_SYMBOL(uapi_ccpriv_set_ap_user_aging_time);
EXPORT_SYMBOL(uapi_ccpriv_dhcp_debug);
#ifdef _PRE_WLAN_SMOOTH_PHASE
EXPORT_SYMBOL(uapi_ccpriv_set_smooth_phase_en);
#endif
#ifdef _PRE_WLAN_FEATURE_OPMODE_NOTIFY
EXPORT_SYMBOL(uapi_ccpriv_set_opmode_notify);
#endif
#ifdef _PRE_WLAN_FEATURE_FTM
EXPORT_SYMBOL(uapi_ccpriv_ftm);
#endif
EXPORT_SYMBOL(uapi_ccpriv_multi_bssid_enable);
EXPORT_SYMBOL(uapi_ccpriv_set_2040_coext_support);
#ifdef _PRE_WLAN_FEATURE_11AX
EXPORT_SYMBOL(uapi_ccpriv_set_sts_sung_mung);
#endif
EXPORT_SYMBOL(uapi_ccpriv_rssi_limit);
EXPORT_SYMBOL(wal_get_user_by_mac);
#ifdef _PRE_WLAN_FEATURE_11K
EXPORT_SYMBOL(wal_handel_radio_meas_rpt_cmd);
#endif
#endif /* end of _PRE_WLAN_CFGID_DEBUG */
#endif /* end of _PRE_OS_VERSION_LINUX */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif
