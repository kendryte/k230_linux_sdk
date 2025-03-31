/*
 * Copyright (c) CompanyNameMagicTag 2021-2021. All rights reserved.
 * Description: Header file for soc_ini.c.
 * Create: 2021-12-15
 */


#ifndef __SOC_INI_H__
#define __SOC_INI_H__

/*
 * 1 Other Header File Including
 */
#include "oal_plat_type.h"

/*
 * 2 Macro Definition
 */
#define EXT_CUST_NVRAM_LEN      (104)

#define INI_MODU_WIFI           (0x101)
#define INI_MODU_FCC_NVRAM      (0x10E)
#define INI_MODU_JP_NVRAM       (0x10F)
#define INI_MODU_CE_NVRAM       (0x110)
#define INI_READ_VALUE_LEN      (128)
#define INI_READ_VALUE_COUNT    (128)   /* 定制项最大可以解析的参数字节长度 */
/* 每个定制化参数的字节长度 */
#define INI_PARAM_BYTE_ONE   (1)
#define INI_PARAM_BYTE_TWO   (2)
#define INI_PARAM_BYTE_FOUR   (4)
#define INI_SUCC                (0)
#define INI_FAILED              (-1)

#define INI_FILE_TIMESPEC_UNRECONFIG (0)
/*
 * 4 Message Header Definition
 */


/*
 * 5 Message Definition
 */


/*
 * 6 STRUCT Type Definition
 */


/*
 * 7 Global Variable Declaring
 */


/*
 * 8 UNION Type Definition
 */


/*
 * 9 OTHERS Definition
 */


/*
 * 10 Function Declare
 */
extern osal_s32 get_cust_conf_int32_etc(osal_s32 tag_index, osal_s8 *var, osal_s32 *pul_value);
extern osal_s32 get_cust_conf_string_etc(osal_s32 tag_index, osal_s8 *var, osal_s8 *value, osal_u32 size);
extern osal_s32 find_download_channel_etc(osal_u8 *buff, osal_s8 *var);
osal_s32 find_bt_bus(osal_u8 *buff, osal_s8 *var);

extern osal_s32 ini_cfg_init_etc(void);
extern void ini_cfg_exit_etc(void);
extern osal_s32 ini_file_check_conf_update(void);
extern osal_u32 ini_get_cust_item_list(const char *cust_param, osal_u8 *param_list, osal_u8 param_len,
    osal_u8 *list_len, osal_u8 max_len);
#endif

