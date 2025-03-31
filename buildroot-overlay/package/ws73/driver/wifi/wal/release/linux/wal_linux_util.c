/*
 * Copyright (c) CompanyNameMagicTag 2021-2021. All rights reserved.
 * Description: wal util api.
 * Create: 2021-07-16
 */

#include "wal_linux_util.h"
#include "mac_resource_ext.h"
#include "frw_hmac.h"
#ifdef _PRE_WLAN_FEATURE_11D
#include "wal_linux_11d.h"
#endif
#include "wal_linux_scan.h"
#include "wal_linux_ccpriv.h"
#include "wal_linux_ioctl.h"
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
#include "soc_customize_wifi.h"
#endif
#ifdef _PRE_WLAN_DFT_STAT
#include "plat_firmware.h"
#endif
#include "wlan_msg.h"
#include "diag_log_common.h"
#include "common_dft.h"
#include "mac_vap_ext.h"
#include "oal_kernel_file.h"
#include "wal_linux_netdev.h"
#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_WAL_LINUX_UTIL_C
#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#ifdef _PRE_WLAN_DFT_STAT
#define WAL_FUNC_NAME_MAX_LEN 200
static osal_u8 g_wifi_external_record_enable = 0;
static osal_u32 g_line = 0;

// osal_void wal_set_external_record_enable(osal_u8 enable_value)
// {
//     os_kernel_file_stru *fp = {0};
//     osal_s32 data_len;
//     mm_segment_t fs;

//     memset_s(&fs, sizeof(fs), 0, sizeof(fs));
//     if (enable_value == 1) {
//         fp = filp_open(g_external_record_file_path, O_RDWR | O_CREAT, 0644);  // 0644 表示文件权限
//         if (OAL_IS_ERR_OR_NULL(fp)) {
//             oam_warning_log0(0, OAM_SF_ANY, "{wal_set_external_record_enable::create error!!!}");
//             g_wifi_external_record_enable = 0;
//             return;
//         }
//         fs = oal_get_fs();
//         oal_set_ds();

//         data_len = vfs_llseek(fp, 0, SEEK_END);
//         g_line = data_len / WAL_FUNC_NAME_MAX_LEN;
//         vfs_llseek(fp, 0, SEEK_SET);

//         oal_set_fs(fs);
//         filp_close(fp, NULL);
//     }

//     g_wifi_external_record_enable = enable_value;
//     return;
// }
osal_void wal_set_external_record_enable(osal_u8 enable_value)
{
    struct file *fp = NULL;
    osal_s32 data_len;

    if (enable_value == 1) {
        fp = filp_open(g_external_record_file_path, O_RDWR | O_CREAT, 0644);
        if (IS_ERR_OR_NULL(fp)) {
            oam_warning_log0(0, OAM_SF_ANY, "{wal_set_external_record_enable::create error!!!}");
            g_wifi_external_record_enable = 0;
            return;
        }

        /* 直接使用 vfs_llseek 获取文件大小 */
        data_len = vfs_llseek(fp, 0, SEEK_END);
        g_line = data_len / WAL_FUNC_NAME_MAX_LEN;

        /* 重置文件指针到起始位置 */
        vfs_llseek(fp, 0, SEEK_SET);
        filp_close(fp, NULL);
    }

    g_wifi_external_record_enable = enable_value;
    return;
}

// static osal_void wal_wifi_external_record_to_file(osal_u8 *data, osal_u32 data_len)
// {
//     os_kernel_file_stru *fp = {0};
//     mm_segment_t fs;
//     loff_t pos;
//     static osal_s8 tmp_buf[WAL_FUNC_NAME_MAX_LEN] = {0};
//     static osal_s32 count = 1;
//     osal_u32 cur_time;
//     osal_s8 *print_buff;
//     osal_u32 string_len = 0;

//     memset_s(&fs, sizeof(fs), 0, sizeof(fs));
//     fp = filp_open(g_external_record_file_path, O_RDWR, 0644); // 0644 表示文件权限
//     if (OAL_IS_ERR_OR_NULL(fp)) {
//         oam_warning_log1(0, OAM_SF_ANY, "{wal_wifi_external_record_to_file::create error,fp = %p!}", (uintptr_t)fp);
//         return;
//     }

//     print_buff = (osal_s8 *)oal_mem_alloc(OAL_MEM_POOL_ID_LOCAL, WAL_FUNC_NAME_MAX_LEN, OAL_TRUE);
//     if (print_buff == OAL_PTR_NULL) {
//         oam_warning_log0(0, OAM_SF_ANY, "{wal_wifi_external_record_to_file::print_buff null!}\r\n");
//         return;
//     }
//     memset_s(print_buff, WAL_FUNC_NAME_MAX_LEN, 0, WAL_FUNC_NAME_MAX_LEN);

//     if (osal_adapt_strncmp(tmp_buf, data, data_len) == 0) {
//         count++;
//         g_line--;
//     } else {
//         (osal_void)memcpy_s(tmp_buf, WAL_FUNC_NAME_MAX_LEN, data, data_len);
//         count = 1;
//     }

//     cur_time = (osal_u32)oal_time_get_stamp_ms();
//     string_len = snprintf_s(print_buff, WAL_FUNC_NAME_MAX_LEN - 1, WAL_FUNC_NAME_MAX_LEN - 1,
//         "time[%u] count[%d]:%s", cur_time, count, data);
//     if (string_len < 0) {
//         oam_warning_log1(0, OAM_SF_ANY, "{wal_wifi_external_record_to_file::string_len %d}", string_len);
//     }
//     fs = oal_get_fs();
//     oal_set_ds();

//     g_line %= ((g_external_record_file_size * 1024) / WAL_FUNC_NAME_MAX_LEN); /* 1024: 单位转换为Byte */
//     pos = vfs_llseek(fp, g_line * WAL_FUNC_NAME_MAX_LEN, SEEK_SET);
// #if LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0)
//     vfs_write(fp, print_buff, WAL_FUNC_NAME_MAX_LEN, &pos);
// #else
//     kernel_write(fp, print_buff, WAL_FUNC_NAME_MAX_LEN, &pos);
// #endif
//     oal_set_fs(fs);
//     filp_close(fp, NULL);
//     g_line++;

//     oal_mem_free(print_buff, OAL_TRUE);
//     return;
// }
static osal_void wal_wifi_external_record_to_file(osal_u8 *data, osal_u32 data_len)
{
    struct file *fp = NULL;
    loff_t pos;
    static osal_s8 tmp_buf[WAL_FUNC_NAME_MAX_LEN] = {0};
    static osal_s32 count = 1;
    osal_u32 cur_time;
    osal_s8 *print_buff;
    osal_u32 string_len = 0;

    fp = filp_open(g_external_record_file_path, O_RDWR, 0644);
    if (IS_ERR_OR_NULL(fp)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_wifi_external_record_to_file::create error,fp = %p!}", (uintptr_t)fp);
        return;
    }

    print_buff = (osal_s8 *)oal_mem_alloc(OAL_MEM_POOL_ID_LOCAL, WAL_FUNC_NAME_MAX_LEN, OAL_TRUE);
    if (print_buff == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_wifi_external_record_to_file::print_buff null!}\r\n");
        filp_close(fp, NULL);
        return;
    }
    memset(print_buff, 0, WAL_FUNC_NAME_MAX_LEN);

    if (osal_adapt_strncmp(tmp_buf, data, data_len) == 0) {
        count++;
        g_line--;
    } else {
        memcpy(tmp_buf, data, data_len);
        count = 1;
    }

    cur_time = (osal_u32)oal_time_get_stamp_ms();
    string_len = snprintf(print_buff, WAL_FUNC_NAME_MAX_LEN - 1,
        "time[%u] count[%d]:%s", cur_time, count, data);
    if (string_len < 0) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_wifi_external_record_to_file::string_len %d}", string_len);
    }

    /* 计算文件偏移并写入数据 */
    g_line %= ((g_external_record_file_size * 1024) / WAL_FUNC_NAME_MAX_LEN);
    pos = vfs_llseek(fp, g_line * WAL_FUNC_NAME_MAX_LEN, SEEK_SET);

    /* 统一使用 kernel_write */
    kernel_write(fp, print_buff, WAL_FUNC_NAME_MAX_LEN, &pos);

    filp_close(fp, NULL);
    g_line++;

    oal_mem_free(print_buff, OAL_TRUE);
    return;
}

osal_void wal_record_external_log_by_id(osal_u8 type, osal_u32 id)
{
    osal_s8 *print_buff;
    osal_u32 string_len = 0;
    if (g_wifi_external_record_enable == 0) {
        return;
    }
    print_buff = (osal_s8 *)oal_mem_alloc(OAL_MEM_POOL_ID_LOCAL, WAL_FUNC_NAME_MAX_LEN, OAL_TRUE);
    if (print_buff == OAL_PTR_NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_record_external_log_by_id::print_buff null!}\r\n");
        return;
    }
    memset_s(print_buff, WAL_FUNC_NAME_MAX_LEN, 0, WAL_FUNC_NAME_MAX_LEN);
    switch (type) {
        case WLAN_WIFI_RECORD_ATCMD:
            oam_warning_log1(0, OAM_SF_ANY,
                "{wal_record_external_log_by_id::uapi_atcmdsrv_wifi_priv_cmd_etc case id [%d]!}\r\n", id);
            string_len = snprintf_s(print_buff, WAL_FUNC_NAME_MAX_LEN - 1, WAL_FUNC_NAME_MAX_LEN - 1,
                "wal_record_external_log_by_id::uapi_atcmdsrv_wifi_priv_cmd_etc id [%d]\n", id);
            break;
        case WLAN_WIFI_RECORD_IWINT:
            oam_warning_log1(0, OAM_SF_ANY,
                "{wal_record_external_log_by_id::uapi_iwpriv_get_param case id [%d]!}\r\n", id);
            string_len = snprintf_s(print_buff, WAL_FUNC_NAME_MAX_LEN - 1, WAL_FUNC_NAME_MAX_LEN - 1,
                "wal_record_external_log_by_id::uapi_iwpriv_get_param id [%d]\n", id);
            break;
        case WLAN_WIFI_RECORD_IWCHAR:
            oam_warning_log1(0, OAM_SF_ANY,
                "{wal_record_external_log_by_id::uapi_iwpriv_get_param_char case id [%d]!}\r\n", id);
            string_len = snprintf_s(print_buff, WAL_FUNC_NAME_MAX_LEN - 1, WAL_FUNC_NAME_MAX_LEN - 1,
                "wal_record_external_log_by_id::uapi_iwpriv_get_param_char id [%d]\n", id);
            break;
        case WLAN_WIFI_RECORD_WITPCMD:
            oam_warning_log1(0, OAM_SF_ANY,
                "{wal_record_external_log_by_id::uapi_witp_wifi_priv_cmd case id [%d]!}\r\n", id);
            string_len = snprintf_s(print_buff, WAL_FUNC_NAME_MAX_LEN - 1, WAL_FUNC_NAME_MAX_LEN - 1,
                "wal_record_external_log_by_id::uapi_witp_wifi_priv_cmd id [%d]\n", id);
            break;
        default:
            break;
    }
    wal_wifi_external_record_to_file(print_buff, string_len);
    oal_mem_free(print_buff, OAL_TRUE);
    return;
}

osal_void wal_record_external_log_by_name(osal_u8 type, osal_u8 *name, osal_u32 ret)
{
    osal_s8 *print_buff;
    osal_u32 string_len = 0;
    if (g_wifi_external_record_enable == 0) {
        return;
    }
    print_buff = (osal_s8 *)oal_mem_alloc(OAL_MEM_POOL_ID_LOCAL, WAL_FUNC_NAME_MAX_LEN, OAL_TRUE);
    if (print_buff == OAL_PTR_NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_record_external_log_by_name::print_buff null!}\r\n");
        return;
    }
    memset_s(print_buff, WAL_FUNC_NAME_MAX_LEN, 0, WAL_FUNC_NAME_MAX_LEN);
    switch (type) {
        case WLAN_WIFI_HIPRIV_CMD:
            string_len = snprintf_s(print_buff, WAL_FUNC_NAME_MAX_LEN - 1, WAL_FUNC_NAME_MAX_LEN - 1,
                "wal_record_external_log_by_name::g_ast_ccpriv_cmd name[%s] ret[%u]\n", name, ret);
            break;
        case WLAN_WIFI_HIPRIV_DEBUG_CMD:
            string_len = snprintf_s(print_buff, WAL_FUNC_NAME_MAX_LEN - 1, WAL_FUNC_NAME_MAX_LEN - 1,
                "wal_record_external_log_by_name::g_ast_ccpriv_cmd_debug_etc name[%s] ret[%u]\n", name, ret);
            break;
        case WLAN_WIFI_ANDROID_CMD:
            string_len = snprintf_s(print_buff, WAL_FUNC_NAME_MAX_LEN - 1, WAL_FUNC_NAME_MAX_LEN - 1,
                "wal_record_external_log_by_name::g_android_cmd name[%s] ret[%u]\n", name, ret);
            break;
        case WLAN_WIFI_ONE_INT_ARGS:
            string_len = snprintf_s(print_buff, WAL_FUNC_NAME_MAX_LEN - 1, WAL_FUNC_NAME_MAX_LEN - 1,
                "wal_record_external_log_by_name::g_one_int_args name[%s] ret[%u]\n", name, ret);
            break;
        case WLAN_WIFI_DEBUG_CMD:
            string_len = snprintf_s(print_buff, WAL_FUNC_NAME_MAX_LEN - 1, WAL_FUNC_NAME_MAX_LEN - 1,
                "wal_record_external_log_by_name::g_cmd_debug name[%s] ret[%u]\n", name, ret);
            break;
        default:
            break;
    }

    dft_report_params_etc(name, osal_strlen(name), SOC_DIAG_MSG_ID_WIFI_FRAME_MAX);
    wal_wifi_external_record_to_file(print_buff, string_len);
    oal_mem_free(print_buff, OAL_TRUE);
    return;
}

osal_void wal_record_wifi_external_log(osal_u8 type, const osal_s8 *name)
{
    osal_s8 *print_buff;
    osal_u32 string_len = 0;
    if (g_wifi_external_record_enable == 0) {
        return;
    }
    print_buff = (osal_s8 *)oal_mem_alloc(OAL_MEM_POOL_ID_LOCAL, WAL_FUNC_NAME_MAX_LEN, OAL_TRUE);
    if (print_buff == OAL_PTR_NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_record_wifi_external_log::print_buff null!}\r\n");
        return;
    }
    memset_s(print_buff, WAL_FUNC_NAME_MAX_LEN, 0, WAL_FUNC_NAME_MAX_LEN);
    switch (type) {
        case WLAN_WIFI_CFG80211_OPS:
            oam_warning_log0(0, OAM_SF_ANY, "{wal_record_wifi_external_log::g_wal_cfg80211_ops func!}\r\n");
            string_len = snprintf_s(print_buff, WAL_FUNC_NAME_MAX_LEN - 1, WAL_FUNC_NAME_MAX_LEN - 1,
                "wal_record_wifi_external_log::g_wal_cfg80211_ops func[%s]\n", name);
            break;
        case WLAN_WIFI_VENDER_CMDS:
            oam_warning_log0(0, OAM_SF_ANY, "{wal_record_wifi_external_log::wal_vendor_cmds func!}\r\n");
            string_len = snprintf_s(print_buff, WAL_FUNC_NAME_MAX_LEN - 1, WAL_FUNC_NAME_MAX_LEN - 1,
                "wal_record_wifi_external_log::wal_vendor_cmds func[%s]\n", name);
            break;
        case WLAN_WIFI_NETDEV_OPS:
            oam_warning_log0(0, OAM_SF_ANY, "{wal_record_wifi_external_log::g_st_wal_net_dev_ops_etc func!}\r\n");
            string_len = snprintf_s(print_buff, WAL_FUNC_NAME_MAX_LEN - 1, WAL_FUNC_NAME_MAX_LEN - 1,
                "wal_record_wifi_external_log::g_st_wal_net_dev_ops_etc func[%s]\n", name);
            break;
        case WLAN_WIFI_IW_HANDLERS:
            oam_warning_log0(0, OAM_SF_ANY, "{wal_record_wifi_external_log::g_ast_iw_handlers func!}\r\n");
            string_len = snprintf_s(print_buff, WAL_FUNC_NAME_MAX_LEN - 1, WAL_FUNC_NAME_MAX_LEN - 1,
                "wal_record_wifi_external_log::g_ast_iw_handlers func[%s]\n", name);
            break;
        case WLAN_WIFI_IW_PRIV_HANDLERS:
            oam_warning_log0(0, OAM_SF_ANY, "{wal_record_wifi_external_log::g_ast_iw_priv_handlers func!}\r\n");
            string_len = snprintf_s(print_buff, WAL_FUNC_NAME_MAX_LEN - 1, WAL_FUNC_NAME_MAX_LEN - 1,
                "wal_record_wifi_external_log::g_ast_iw_priv_handlers func[%s]\n", name);
            break;
        default:
            break;
    }

    dft_report_params_etc((osal_u8 *)name, osal_strlen(name), SOC_DIAG_MSG_ID_WIFI_FRAME_MAX);
    wal_wifi_external_record_to_file(print_buff, string_len);
    oal_mem_free(print_buff, OAL_TRUE);
    return;
}
#endif

osal_s32 wal_util_set_country_code(oal_net_device_stru *net_dev, osal_s8 *country_code)
{
#ifndef _PRE_BSLE_GATEWAY
#ifdef _PRE_WLAN_FEATURE_11D
    osal_s32 ret;
    if ((net_dev == OAL_PTR_NULL) || (country_code == OAL_PTR_NULL)) {
        oam_error_log2(0, OAM_SF_ANY, "{wal_util_set_country_code::null ptr.net_dev %p, country %p!}",
                       (uintptr_t)net_dev, (uintptr_t)country_code);
        return -OAL_EFAIL;
    }
#ifdef _PRE_WLAN_FEATURE_DFS
    ret = wal_regdomain_update_for_dfs_etc(net_dev, country_code);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_util_set_country_code::wal_util_set_country_code err [%d]!}", ret);
        return -OAL_EFAIL;
    }
#endif
    ret = wal_regdomain_update_etc(net_dev, country_code);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_util_set_country_code::wal_regdomain_update_etc err [%d]!}", ret);
        return -OAL_EFAIL;
    }
#endif
#endif

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : wal_get_parameter_from_cmd
 功能描述  : 获取字符串第一个参数，以逗号为命令区分标识，并扣除命令前置字符
 输入参数  : pc_cmd         : 传入的字符串
             token      : 命令前置字串
 输出参数  : pc_arg         : 扣掉命令前置字串后的参数
             pul_cmd_offset : 第一个参数的长度
 返 回 值  : 错误码

 修改历史      :
  1.日    期   : 2015年5月22日
    修改内容   : 新生成函数

*****************************************************************************/
osal_u32 wal_get_parameter_from_cmd(osal_s8 *pc_cmd, osal_s8 *pc_arg, OAL_CONST osal_s8 *token,
    osal_u32 *pul_cmd_offset, osal_u32 param_max_len)
{
    osal_s8   *pc_cmd_copy;
    osal_s8    ac_cmd_copy[WAL_IOCTL_PRIV_SUBCMD_MAX_LEN];
    osal_u32  pos = 0;
    osal_u32  arg_len;

    if (OAL_UNLIKELY((OAL_PTR_NULL == pc_cmd) || (OAL_PTR_NULL == pc_arg) || (OAL_PTR_NULL == pul_cmd_offset))) {
        oam_error_log3(0, OAM_SF_ANY,
            "{wal_get_parameter_from_cmd::pc_cmd/pc_arg/pul_cmd_offset null ptr error %p, %p, %p!}",
            (uintptr_t)pc_cmd, (uintptr_t)pc_arg, (uintptr_t)pul_cmd_offset);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pc_cmd_copy = pc_cmd;

    /* 去掉字符串开始的逗号 */
    while (',' == *pc_cmd_copy) {
        ++pc_cmd_copy;
    }
    /* 取得逗号前的字符串 */
    while ((',' != *pc_cmd_copy) && ('\0' != *pc_cmd_copy)) {
        ac_cmd_copy[pos] = *pc_cmd_copy;
        ++pos;
        ++pc_cmd_copy;

        if (OAL_UNLIKELY(pos >= param_max_len)) {
            oam_warning_log1(0, OAM_SF_ANY,
                "{wal_get_parameter_from_cmd::pos >= WAL_HIPRIV_CMD_NAME_MAX_LEN, pos %d!}", pos);
            return OAL_ERR_CODE_ARRAY_OVERFLOW;
        }
    }
    ac_cmd_copy[pos]  = '\0';
    /* 字符串到结尾，返回错误码 */
    if (0 == pos) {
        oam_info_log0(0, OAM_SF_ANY, "{wal_get_parameter_from_cmd::return param pc_arg is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    *pul_cmd_offset = (osal_u32)(pc_cmd_copy - pc_cmd);

    /* 检查字符串是否包含期望的前置命令字符 */
    if (0 != osal_memcmp(ac_cmd_copy, token, osal_strlen(token))) {
        return OAL_FAIL;
    } else {
        /* 扣除前置命令字符，回传参数 */
        arg_len = osal_strlen(ac_cmd_copy) - osal_strlen(token);
        if (memcpy_s(pc_arg, param_max_len, ac_cmd_copy + osal_strlen(token), arg_len) != EOK) {
            oam_error_log0(0, OAM_SF_ANY, "{wal_get_parameter_from_cmd::memcpy_s error}");
        }
        pc_arg[arg_len]  = '\0';
    }
    return OAL_SUCC;
}

/* 发送uint32数据至hmac，不关注response */
osal_u32  wal_post2hmac_uint32_data(oal_net_device_stru *net_dev, osal_s8 *param, osal_u16 msg_id)
{
    osal_s32 send_ret;
    osal_u32 set_value = 0;
    osal_u32 ret, off_set;
    osal_s8 str[WAL_HIPRIV_CMD_NAME_MAX_LEN];

    ret = wal_get_cmd_one_arg_digit(param, str, sizeof(str), &off_set, &set_value);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_CFG, "{wal_post2hmac_uint32_data::get para error.}");
        return ret;
    }

    send_ret = wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(net_dev), msg_id,
        (osal_s8 *)&set_value, OAL_SIZEOF(osal_u32));
    if (send_ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_CFG,
            "{wal_post2hmac_uint32_data:wal_sync_post2hmac_no_rsp return[%d].}\r\n", send_ret);
        return (osal_u32)send_ret;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : mac地址随机化
*****************************************************************************/
static inline osal_void wal_wlan_addr_randomization(osal_u8 *wlan_addr)
{
    wlan_addr[0] |= 0x02; /* 第0位跟0x02做或运算 */
    wlan_addr[4] ^= 0x80; /* 第4位跟0x80做异或运算 */
}

/*****************************************************************************
 功能描述  : 生成mac地址并设置到mib
*****************************************************************************/
STATIC osal_u32 wal_generate_mac_addr(oal_net_device_stru *net_dev, hmac_vap_stru *hmac_vap)
{
    osal_u8 *wlan_addr = mac_mib_get_station_id(hmac_vap);

    return wlan_get_mac(wlan_addr, WLAN_MAC_ADDR_LEN, net_dev);
}

/*****************************************************************************
 功能描述  : 设置随机mac地址到device
*****************************************************************************/
osal_s32 wal_set_random_mac_to_mib_etc(oal_net_device_stru *net_dev)
{
    osal_u8                    *mac_addr;
    osal_u32                    ret;
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)net_dev->ml_priv;
    mac_cfg_staion_id_param_stru staion_id;
    (osal_void)memset_s(&staion_id, OAL_SIZEOF(staion_id), 0, OAL_SIZEOF(staion_id));
    if ((hmac_vap == OAL_PTR_NULL) || (hmac_vap->mib_info == OAL_PTR_NULL)) {
        oam_warning_log1(0, OAM_SF_ANY, "vap_id[%d] {wal_set_random_mac_to_mib_etc::vap or mib is NULL!}",
            (hmac_vap == OAL_PTR_NULL) ? 0xffff : hmac_vap->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }
    if (wal_generate_mac_addr(net_dev, hmac_vap) != OAL_SUCC) {
        return OAL_FAIL;
    }
    mac_addr = mac_mib_get_station_id(hmac_vap);
    oal_set_mac_addr(staion_id.station_id, mac_addr);
    oal_set_mac_addr((osal_u8 *)net_dev->dev_addr, mac_mib_get_station_id(hmac_vap));

    ret = wal_sync_post2hmac_no_rsp(hmac_vap->vap_id, WLAN_MSG_W2H_CFG_STATION_ID,
        (osal_u8 *)&staion_id, OAL_SIZEOF(staion_id));
    if (OAL_UNLIKELY(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_sync_post2hmac_no_rsp fail; return %d!}", ret);
        return -OAL_ENOMEM;
    }

    return OAL_SUCC;
}

osal_s32 wal_get_init_state_vap(oal_net_device_stru *net_dev, hmac_vap_stru **hmac_vap)
{
    hmac_vap_stru *tmp_vap;

    tmp_vap = (hmac_vap_stru *)net_dev->ml_priv;
    if (OAL_UNLIKELY(tmp_vap == OAL_PTR_NULL)) {
        oam_warning_log0(0, OAM_SF_CFG, "{wal_vap_check_init_state::can't get mac vap from netdevice priv data!}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    if (hmac_vap != OAL_PTR_NULL) {
        *hmac_vap = tmp_vap;
    }
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : wal_cfg80211_start_connect_etc
 输入参数  : 无
 输出参数  : 无

 修改历史      :
  1.日    期   : 2013年8月30日
    修改内容   : 新生成函数

*****************************************************************************/
osal_s32  wal_cfg80211_start_connect_etc(oal_net_device_stru *net_dev, mac_conn_param_stru *mac_conn_param)
{
    osal_s32 l_ret;

    if (WAL_MSG_WRITE_MAX_LEN < OAL_SIZEOF(mac_conn_param_stru)) {
        oam_error_log2(0, OAM_SF_SCAN,
                       "{wal_cfg80211_start_connect_etc::len %d > WAL_MSG_WRITE_MAX_LEN %d err!}\r\n",
                       OAL_SIZEOF(mac_conn_param_stru), WAL_MSG_WRITE_MAX_LEN);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    l_ret = wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(net_dev), WLAN_MSG_W2H_CFG_CFG80211_START_CONNECT,
        (osal_u8 *)mac_conn_param, OAL_SIZEOF(mac_conn_param_stru));
    if (OAL_UNLIKELY(OAL_SUCC != l_ret)) {
        oam_warning_log2(0, OAM_SF_ANY, "vap_id[%d] {wal_cfg80211_start_connect_etc::return err code %d!}",
                         wal_util_get_vap_id(net_dev), l_ret);
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : wal_cfg80211_start_disconnect_etc
 输入参数  : 无
 输出参数  : 无

 修改历史      :
  1.日    期   : 2013年8月30日
    修改内容   : 新生成函数

*****************************************************************************/
osal_s32  wal_cfg80211_start_disconnect_etc(oal_net_device_stru *net_dev,
    mac_cfg_kick_user_param_stru *disconnect_param)
{
    hmac_vap_stru    *hmac_vap;
    osal_s32 ret;

    // 终止可能存在的扫描
    if ((hmac_vap = net_dev->ml_priv) && (WLAN_VAP_MODE_BSS_STA == hmac_vap->vap_mode)) {
        wal_force_scan_complete_for_disconnect_scene(net_dev);
    }
    ret = wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(net_dev), WLAN_MSG_W2H_CFG_KICK_USER,
        (osal_u8 *)disconnect_param, OAL_SIZEOF(mac_cfg_kick_user_param_stru));
    if (ret != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_ANY, "vap_id[%d] {wal_cfg80211_start_disconnect_etc::KICK_USER err %d!}",
                         wal_util_get_vap_id(net_dev), ret);
    }
    return ret;
}

osal_void wal_connect_set_auth_type(osal_u8 orgin_type, osal_u8 *new_type)
{
    /* 设置认证类型, 02.11-2016协议与 kernel SAE成员定义值不一致，此处适配 */
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3,8,0))
    if (orgin_type == NL80211_AUTHTYPE_SAE) {
        *new_type = WLAN_WITP_AUTH_SAE;
    } else
#endif
    if (orgin_type == NL80211_AUTHTYPE_NETWORK_EAP) {
        *new_type = WLAN_WITP_AUTH_NETWORK_EAP;
    } else {
        *new_type = orgin_type;
    }
}

osal_u8 bw_enum_to_number(osal_u8 bw)
{
    /* 20/40/80M */
    return bw == 0 ? 20 : bw == 1 ? 40 : 80;
}

osal_u32 tid_stat_to_user(osal_u32 *stat)
{
    /* 0 1 2 3 4 5 6 7 */
    return stat[0] + stat[1] + stat[2] + stat[3] + stat[4] + stat[5] + stat[6] + stat[7];
}
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(5,3,0))
/* 内核升级后 对kernel_read 和 kernel_write 进行了限制 */
MODULE_IMPORT_NS(VFS_internal_I_am_really_a_filesystem_and_am_NOT_a_driver);
#endif
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
