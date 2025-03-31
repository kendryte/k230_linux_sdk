/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: wal util api.
 * Create: 2022-08-22
 */

#ifndef __WAL_UTILS_H__
#define __WAL_UTILS_H__

#include "oal_ext_if.h"
#include "wlan_types_common.h"
#include "wlan_spec.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#ifdef _PRE_WLAN_FEATURE_WS92_MERGE
#define WAL_CCPRIV_CMD_NAME_MAX_LEN 40 /* 字符串中每个单词的最大长度 */
#else
#define WAL_CCPRIV_CMD_NAME_MAX_LEN 80 /* 字符串中每个单词的最大长度(原20) */
#endif

#define WAL_CCPRIV_CMD_PKT_NAME_MAX_LEN 1024 /* 报文字符串的最大长度,应大于报文长度的2倍 */
#define WAL_CCPRIV_CMD_PKT_MAX_LEN 490 /* 报文的最大长度 */
#define WAL_CCPRIV_CMD_PKT_MIN_LEN 10 /* 报文的最小长度 */

typedef enum {
    CCPRIV_SWITCH_OFF = 0,
    CCPRIV_SWITCH_ON = 1,

    CCPRIV_SWITCH_BUFF
} ccpriv_switch;

typedef osal_u32 (*wal_ccpriv_debug_cmd_func)(const osal_s8 *cmd_name,
    oal_net_device_stru *net_dev, osal_s8 *param);

typedef osal_u32(*wal_ccpriv_cmd_func)(oal_net_device_stru *net_dev, osal_s8 *pc_param);

/* 私有命令入口结构定义 */
typedef struct {
    char *cmd_name;    /* 命令字符串 */
    wal_ccpriv_cmd_func func;         /* 命令对应处理函数 */
} wal_ccpriv_cmd_entry_stru;

osal_u8 wal_util_get_vap_id(oal_net_device_stru *net_dev);
osal_u32 wal_get_cmd_one_arg(const osal_s8 *cmd, osal_s8 *arg, osal_u32 arg_len, osal_u32 *cmd_offset);
osal_s32 wal_get_cmd_one_arg_digit(osal_s8 *cmd, osal_s8 *arg, osal_s32 arg_len, osal_u32 *cmd_offset, osal_s32 *data);
osal_s32 wal_cmd_get_digit_with_range(osal_s8 **src_head, osal_s32 min, osal_s32 max, osal_s32 *value);
osal_s32 wal_cmd_get_s8_with_range(osal_s8 **src_head, osal_s8 min, osal_s8 max, osal_s8 *value);
osal_u32 wal_cmd_get_u8_with_check_max(osal_s8 **src_head, osal_u8 max_value, osal_u8 *value);
osal_u32 wal_cmd_get_digit_with_check_max(osal_s8 **src_head, osal_u32 max_value, osal_u32 *value);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
