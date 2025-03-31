/*
 * Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
 * Description: hmac侧ccpriv通用接口
 * Create: 2023-02-25
 */
#ifndef HMAC_CCPRIV_H
#define HMAC_CCPRIV_H

#include "mac_vap_ext.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

typedef osal_s32(*hmac_ccpriv_cmd_func)(hmac_vap_stru *hmac_vap, const osal_s8 *param);

/* 私有命令入口结构定义 */
typedef struct {
    struct osal_list_head entry_list;
    hmac_ccpriv_cmd_func func; /* 命令对应处理函数 */
    const osal_s8 *cmd_name; /* 命令字符串 */
} hmac_ccpriv_cmd_entry;

typedef struct {
    const osal_s8 *cmd_name;
    hmac_ccpriv_cmd_func func;
} ccpriv_cmd_func_stru;

osal_u32 hmac_ccpriv_init(osal_void);
osal_void hmac_ccpriv_deinit(osal_void);
/* 注意：cmd_name一定要传入常量字符串，否则注册有问题 */
osal_void hmac_ccpriv_register(const osal_s8 *cmd_name, hmac_ccpriv_cmd_func func);
osal_void hmac_ccpriv_unregister(const osal_s8 *cmd_name);

osal_s32 hmac_ccpriv_get_one_arg(const osal_s8 **cmd, osal_s8 *arg, osal_u32 arg_len);
osal_s32 hmac_ccpriv_get_one_arg_digit(const osal_s8 **cmd, osal_s32 *data);
osal_s32 hmac_ccpriv_get_digit_with_range(const osal_s8 **src_head, osal_s32 min, osal_s32 max, osal_s32 *value);
osal_s32 hmac_ccpriv_get_s8_with_range(const osal_s8 **src_head, osal_s8 min, osal_s8 max, osal_s8 *value);
osal_s32 hmac_ccpriv_get_u8_with_check_max(const osal_s8 **src_head, osal_u8 max_value, osal_u8 *value);
osal_s32 hmac_ccpriv_get_digit_with_check_max(const osal_s8 **src_head, osal_u32 max_value, osal_u32 *value);
osal_u32 hmac_ccpriv_get_mac_addr_etc(const osal_s8 **param, osal_u8 mac_addr[]);
osal_void hmac_ccpriv_register_array(const ccpriv_cmd_func_stru cmd_func[], osal_u32 cmd_fun_len);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
