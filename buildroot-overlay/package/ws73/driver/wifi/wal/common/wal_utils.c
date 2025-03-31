/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: wal util api.
 * Create: 2022-08-22
 */

#include "wal_utils.h"
#include "mac_vap_ext.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_WAL_UTILS_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

WIFI_TCM_TEXT osal_u8 wal_util_get_vap_id(oal_net_device_stru *net_dev)
{
    hmac_vap_stru *hmac_vap;

    hmac_vap = (hmac_vap_stru *)net_dev->ml_priv;
    if (hmac_vap == OAL_PTR_NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_util_get_vap_id::dev->ml_priv, return null!}");
        return WLAN_VAP_SUPPORT_MAX_NUM_LIMIT;
    }

    return hmac_vap->vap_id;
}

/* 获取字符串第一个参数 以空格为参数区分标识 */
osal_u32 wal_get_cmd_one_arg(const osal_s8 *cmd, osal_s8 *arg, osal_u32 arg_len, osal_u32 *cmd_offset)
{
    const osal_s8 *cmd_copy = OSAL_NULL;
    osal_u32 pos = 0;

    if (OAL_UNLIKELY((cmd == OSAL_NULL) || (arg == OSAL_NULL) || (cmd_offset == OSAL_NULL))) {
        oam_error_log3(0, OAM_SF_ANY,
            "{wal_get_cmd_one_arg::cmd/arg/cmd_offset null ptr error %p, %p, %p!}",
            (uintptr_t)cmd, (uintptr_t)arg, (uintptr_t)cmd_offset);
        return OAL_ERR_CODE_PTR_NULL;
    }

    cmd_copy = cmd;

    /* 去掉字符串开始的空格 */
    while (*cmd_copy == ' ') {
        ++cmd_copy;
    }

    while ((*cmd_copy != ' ') && (*cmd_copy != '\0')) {
        arg[pos] = *cmd_copy;
        ++pos;
        ++cmd_copy;

        if (OAL_UNLIKELY(pos >= arg_len)) {
            oam_warning_log1(0, OAM_SF_ANY,
                "{wal_get_cmd_one_arg::pos >= WAL_CCPRIV_CMD_NAME_MAX_LEN, pos %d!}", pos);
            return OAL_ERR_CODE_ARRAY_OVERFLOW;
        }
    }

    arg[pos]  = '\0';

    /* 字符串到结尾，返回错误码 */
    if (pos == 0) {
        oam_info_log0(0, OAM_SF_ANY, "{wal_get_cmd_one_arg::return param arg is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    *cmd_offset = (osal_u32)(cmd_copy - cmd);

    return OAL_SUCC;
}

osal_s32 wal_get_cmd_one_arg_digit(osal_s8 *cmd, osal_s8 *arg, osal_s32 arg_len, osal_u32 *cmd_offset, osal_s32 *data)
{
    osal_s32 ret;
    osal_u32 i;

    ret = (osal_s32)wal_get_cmd_one_arg(cmd, arg, (osal_u32)arg_len, cmd_offset);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_get_cmd_one_arg_digit::parse arg failed [%d]!}", ret);
        return ret;
    }

    for (i = 0; i < osal_strlen((const osal_char *)arg); i++) {
        if ((i == 0) && (arg[i] == '-')) {
            continue;
        }
        if (!oal_isdigit(arg[i])) {
            oam_error_log0(0, OAM_SF_ANY, "{wal_get_cmd_one_arg_digit::cmd is not digit!!}");
            return OAL_ERR_CODE_CONFIG_UNSUPPORT;
        }
    }
    if (((arg[0] == '0') && (osal_strlen((const osal_char *)arg) > 1)) ||
        ((arg[0] == '-') && (osal_strlen((const osal_char *)arg) > 1) && (arg[1] == '0'))) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_get_cmd_one_arg_digit::data is invalid!!}");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }
    /* int32最大 10位，2147483648, 此函数最大解析data值为1999999999 , 11位数字， 2开头的10位数字不解析 */
    if ((osal_strlen((const osal_char *)arg) >= 11) || ((arg[0] >= '2') &&
        (osal_strlen((const osal_char *)arg) == 10))) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_get_cmd_one_arg_digit::data too big!!}");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    *data = oal_atoi(arg);
    return OAL_SUCC;
}

/* [min, max] */
osal_s32 wal_cmd_get_digit_with_range(osal_s8 **src_head, osal_s32 min, osal_s32 max, osal_s32 *value)
{
    osal_s8 *param = *src_head;
    osal_s32 ret;
    osal_u32 off_set;
    osal_s8 str[WAL_CCPRIV_CMD_NAME_MAX_LEN];
    osal_s32 tmp;

    ret = wal_get_cmd_one_arg_digit(param, str, sizeof(str), &off_set, &tmp);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_CFG, "{wal_cmd_get_digit_with_range::get para error.}");
        return ret;
    }
    if ((tmp < min) || (tmp > max)) {
        oam_warning_log3(0, OAM_SF_CFG, "{wal_cmd_get_digit_with_range::invalid, para[%d] min[%d] max[%d].}", tmp, min,
            max);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }
    *value = tmp;
    *src_head = param + off_set;
    return OAL_SUCC;
}

osal_s32 wal_cmd_get_s8_with_range(osal_s8 **src_head, osal_s8 min, osal_s8 max, osal_s8 *value)
{
    osal_s32 ret, tmp;

    ret = wal_cmd_get_digit_with_range(src_head, min, max, &tmp);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_CFG, "{wal_cmd_get_s8_with_range::get digit error[%d].}", ret);
        return ret;
    }
    *value = (osal_s8)tmp;
    return OAL_SUCC;
}

osal_u32 wal_cmd_get_u8_with_check_max(osal_s8 **src_head, osal_u8 max_value, osal_u8 *value)
{
    return (osal_u32)wal_cmd_get_s8_with_range(src_head, 0, (osal_s8)max_value, (osal_s8 *)value);
}

osal_u32 wal_cmd_get_digit_with_check_max(osal_s8 **src_head, osal_u32 max_value, osal_u32 *value)
{
    return (osal_u32)wal_cmd_get_digit_with_range(src_head, 0, (osal_s32)max_value, (osal_s32 *)value);
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
