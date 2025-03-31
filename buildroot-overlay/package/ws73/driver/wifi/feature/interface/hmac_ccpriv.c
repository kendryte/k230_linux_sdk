/*
 * Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
 * Description: hmac侧ccpriv通用接口
 * Create: 2023-02-25
 */

#include "hmac_ccpriv.h"
#include "frw_hmac.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_HMAC_CCPRIV_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

OSAL_STATIC struct osal_list_head g_ccpriv_cmd;

osal_void hmac_ccpriv_register_array(const ccpriv_cmd_func_stru cmd_func[], osal_u32 cmd_fun_len)
{
    osal_u32 i;
 
    for (i = 0; i < cmd_fun_len; i++) {
        hmac_ccpriv_register(cmd_func[i].cmd_name, cmd_func[i].func);
    }
}

/* 注意：cmd_name一定要传入常量字符串，否则注册有问题 */
osal_void hmac_ccpriv_register(const osal_s8 *cmd_name, hmac_ccpriv_cmd_func func)
{
    hmac_ccpriv_cmd_entry *cmd_node = OSAL_NULL;

    if ((cmd_name == OSAL_NULL) || (func == OSAL_NULL)) {
        return;
    }
    cmd_node = (hmac_ccpriv_cmd_entry *)osal_kmalloc(sizeof(hmac_ccpriv_cmd_entry), 0);
    if (cmd_node == OSAL_NULL) {
        return;
    }
    memset_s(cmd_node, sizeof(hmac_ccpriv_cmd_entry), 0, sizeof(hmac_ccpriv_cmd_entry));
    cmd_node->cmd_name = cmd_name;
    cmd_node->func = func;
    osal_list_add_tail(&cmd_node->entry_list, &g_ccpriv_cmd);
    return;
}

OSAL_STATIC hmac_ccpriv_cmd_entry *hmac_ccpriv_get_cmd_entry(const osal_s8 *cmd_name)
{
    struct osal_list_head *entry = OSAL_NULL;
    struct osal_list_head *entry_tmp = OSAL_NULL;
    hmac_ccpriv_cmd_entry *cmd_entry = OSAL_NULL;

    if (cmd_name == OSAL_NULL) {
        return OSAL_NULL;
    }

    osal_list_for_each_safe(entry, entry_tmp, &g_ccpriv_cmd) {
        cmd_entry = osal_list_entry(entry, hmac_ccpriv_cmd_entry, entry_list);
        /* 比较命令名 */
        if (osal_strcmp((const char *)cmd_entry->cmd_name, (const char *)cmd_name) == 0) {
            return cmd_entry;
        }
    }

    return OSAL_NULL;
}

osal_void hmac_ccpriv_unregister(const osal_s8 *cmd_name)
{
    hmac_ccpriv_cmd_entry *cmd_node = OSAL_NULL;

    cmd_node = hmac_ccpriv_get_cmd_entry(cmd_name);
    if (cmd_node == OSAL_NULL) {
        return;
    }
    osal_list_del(&cmd_node->entry_list);
    osal_kfree(cmd_node);
}

OSAL_STATIC osal_void hmac_ccpriv_exit(osal_void)
{
    struct osal_list_head *entry = OSAL_NULL;
    struct osal_list_head *entry_tmp = OSAL_NULL;
    hmac_ccpriv_cmd_entry *cmd_entry = OSAL_NULL;

    osal_list_for_each_safe(entry, entry_tmp, &g_ccpriv_cmd) {
        cmd_entry = osal_list_entry(entry, hmac_ccpriv_cmd_entry, entry_list);
        osal_list_del(&cmd_entry->entry_list);
        osal_kfree(cmd_entry);
    }
    return;
}

OSAL_STATIC osal_s32 hmac_ccpriv_do_cmd(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    hmac_ccpriv_cmd_entry *cmd_node = OSAL_NULL;
    hmac_ccpriv_para *info = (hmac_ccpriv_para *)msg->data;
    const osal_s8 *param = OSAL_NULL;

    if ((hmac_vap == OSAL_NULL) || (info == OSAL_NULL)) {
        return OAL_ERR_CODE_PTR_NULL;
    }
    cmd_node = hmac_ccpriv_get_cmd_entry(info->name);
    if (cmd_node == OSAL_NULL) {
        return OAL_ERR_CODE_CCPRIV_CMD_NOT_FOUND;
    }
    if (cmd_node->func == OSAL_NULL) {
        return OAL_ERR_CODE_INVALID_CONFIG;
    }
    param = info->param;
    return cmd_node->func(hmac_vap, param);
}

/* 获取字符串第一个参数 以空格为参数区分标识 */
osal_s32 hmac_ccpriv_get_one_arg(const osal_s8 **cmd, osal_s8 *arg, osal_u32 arg_len)
{
    const osal_s8 *cmd_copy = OSAL_NULL;
    osal_u32 pos = 0;

    if (OAL_UNLIKELY((cmd == OSAL_NULL) || (*cmd == OSAL_NULL) || (arg == OSAL_NULL))) {
        oam_error_log2(0, OAM_SF_ANY, "{hmac_ccpriv_get_one_arg::cmd/arg null ptr error %p, %p!}",
            (uintptr_t)cmd, (uintptr_t)arg);
        return OAL_ERR_CODE_PTR_NULL;
    }

    cmd_copy = *cmd;

    /* 去掉字符串开始的空格 */
    while (*cmd_copy == ' ') {
        cmd_copy++;
    }

    while ((*cmd_copy != ' ') && (*cmd_copy != '\0')) {
        arg[pos] = *cmd_copy;
        pos++;
        cmd_copy++;

        if (OAL_UNLIKELY(pos >= arg_len)) {
            oam_warning_log2(0, OAM_SF_ANY, "{hmac_ccpriv_get_one_arg::pos[%d] >= arg_len%d}", pos, arg_len);
            return OAL_ERR_CODE_ARRAY_OVERFLOW;
        }
    }

    arg[pos]  = '\0';

    /* 字符串到结尾，返回错误码 */
    if (pos == 0) {
        oam_info_log0(0, OAM_SF_ANY, "{hmac_ccpriv_get_one_arg::return param arg is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    pos = (osal_u32)(cmd_copy - *cmd);
    *cmd = *cmd + pos;

    return OAL_SUCC;
}

osal_s32 hmac_ccpriv_get_one_arg_digit(const osal_s8 **cmd, osal_s32 *data)
{
    osal_s32 ret, i, len;
    osal_s8 arg[CCPRIV_CMD_NAME_MAX_LEN] = {0};

    ret = hmac_ccpriv_get_one_arg(cmd, arg, OAL_SIZEOF(arg));
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_ccpriv_get_one_arg_digit::parse arg failed [%d]!}", ret);
        return ret;
    }
    len = (osal_s32)osal_strlen((const char *)arg);
    for (i = 0; i < len; i++) {
        if ((i == 0) && (arg[i] == '-')) {
            continue;
        }
        if (!oal_isdigit(arg[i])) {
            oam_error_log0(0, OAM_SF_ANY, "{hmac_ccpriv_get_one_arg_digit::cmd is not digit!!}");
            return OAL_ERR_CODE_CONFIG_UNSUPPORT;
        }
    }
    if (((arg[0] == '0') && (len > 1)) || ((arg[0] == '-') && (len > 1) && (arg[1] == '0'))) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_ccpriv_get_one_arg_digit::data is invalid!!}");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }
    /* int32最大 10位，2147483648, 此函数最大解析data值为1999999999 , 11位数字， 2开头的10位数字不解析 */
    if ((len >= 11) || ((arg[0] >= '2') && (len == 10))) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_ccpriv_get_one_arg_digit::data too big!!}");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    *data = oal_atoi((const osal_s8 *)arg);
    return OAL_SUCC;
}

/* [min, max] */
osal_s32 hmac_ccpriv_get_digit_with_range(const osal_s8 **src_head, osal_s32 min, osal_s32 max, osal_s32 *value)
{
    osal_s32 ret, tmp;

    ret = hmac_ccpriv_get_one_arg_digit(src_head, &tmp);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_CFG, "{hmac_ccpriv_get_digit_with_range::get para error.}");
        return ret;
    }
    if ((tmp < min) || (tmp > max)) {
        oam_warning_log3(0, OAM_SF_CFG, "{hmac_ccpriv_get_digit_with_range::invalid, para[%d] min[%d] max[%d].}",
            tmp, min, max);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }
    *value = tmp;
    return OAL_SUCC;
}

osal_s32 hmac_ccpriv_get_s8_with_range(const osal_s8 **src_head, osal_s8 min, osal_s8 max, osal_s8 *value)
{
    osal_s32 ret, tmp;

    ret = hmac_ccpriv_get_digit_with_range(src_head, min, max, &tmp);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_CFG, "{hmac_ccpriv_get_s8_with_range::get digit error[%d].}", ret);
        return ret;
    }
    *value = (osal_s8)tmp;
    return OAL_SUCC;
}

osal_s32 hmac_ccpriv_get_u8_with_check_max(const osal_s8 **src_head, osal_u8 max_value, osal_u8 *value)
{
    return hmac_ccpriv_get_s8_with_range(src_head, 0, (osal_s8)max_value, (osal_s8 *)value);
}

osal_s32 hmac_ccpriv_get_digit_with_check_max(const osal_s8 **src_head, osal_u32 max_value, osal_u32 *value)
{
    return hmac_ccpriv_get_digit_with_range(src_head, 0, (osal_s32)max_value, (osal_s32 *)(uintptr_t)value);
}

OAL_STATIC osal_u32 hmac_ccpriv_check_mac_addr(const osal_char *addr)
{
    osal_u32 i;
    osal_u32 len = (osal_u32)strlen(addr) + 1;
    if (len != CCPRIV_MAC_ADDR_STR_LEN) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_ccpriv_check_mac_addr::mac string len [%d] error !}", len);
        return OAL_ERR_CODE_CONFIG_UNSUPPORT;
    }
    for (i = 0; i < CCPRIV_MAC_ADDR_STR_LEN - 1; i++) {
        /* 校验分隔符 */
        if (i % 3 == 2) { /* MAC地址格式XX:XX:XX:XX:XX:XX, 每隔3位一个分隔符，位数对3求余为2 */
            if ((*addr == ':') || (*addr == '-')) { /* 分格符强制要求为“:”或者“-” */
                addr++;
                continue;
            }
            return OAL_ERR_CODE_CONFIG_UNSUPPORT;
        }
        /* 校验数字 */
        if (oal_strtohex((const osal_s8 *)addr) > 0xf) {
            return OAL_ERR_CODE_CONFIG_UNSUPPORT;
        }
        addr++;
    }
    return OAL_SUCC;
}

osal_u32 hmac_ccpriv_get_mac_addr_etc(const osal_s8 **param, osal_u8 mac_addr[])
{
    osal_u32 ret = OAL_SUCC;
    osal_s8 name[CCPRIV_CMD_NAME_MAX_LEN] = {0};

    /* 获取mac地址 */
    ret = (osal_u32)hmac_ccpriv_get_one_arg(param, name, OAL_SIZEOF(name));
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_ccpriv_get_mac_addr_etc::wal_get_cmd_one_arg return err_code [%d]!}",
            ret);
        return ret;
    }

    ret = hmac_ccpriv_check_mac_addr((const osal_char *)name);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_ccpriv_get_mac_addr_etc::wal_ccpriv_check_mac_addr return err [%d]!}",
            ret);
        return ret;
    }
    oal_strtoaddr((const osal_char *)name, mac_addr);

    return OAL_SUCC;
}


osal_u32 hmac_ccpriv_init(osal_void)
{
    /* 链表初始化 */
    OSAL_INIT_LIST_HEAD(&g_ccpriv_cmd);

    /* 配置接口注册 */
    frw_msg_hook_register(WLAN_MSG_W2H_CFG_CCPRIV_CMD, hmac_ccpriv_do_cmd);

    return OAL_SUCC;
}

osal_void hmac_ccpriv_deinit(osal_void)
{
    /* 配置接口去注册 */
    frw_msg_hook_unregister(WLAN_MSG_W2H_CFG_CCPRIV_CMD);
    hmac_ccpriv_exit();

    return;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
