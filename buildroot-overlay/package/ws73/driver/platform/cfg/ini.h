/*
 * Copyright (c) CompanyNameMagicTag. 2023-2023. All rights reserved.
 * Description: ini header file.
 * Author: CompanyName
 * Create: 2023-01-01
 */

#ifndef __INI_H__
#define __INI_H__

#include "soc_osal.h"

#define INI_KERNEL_READ_LEN             (512)

#define EXT_CUST_NVRAM_READ     (1)
#define EXT_CUST_NVRAM_WRITE    (0)
#define EXT_CUST_NVRAM_NUM      (340)
#define EXT_CUST_NVRAM_LEN      (104)

#define INI_MODU_WIFI           (0x101)
#define INI_MODU_GNSS           (0x102)
#define INI_MODU_BT             (0x103)
#define INI_MODU_FM             (0x104)
#define INI_MODU_PLAT           (0x105)
#define INI_MODU_HOST_VERSION   (0x106)
#define INI_MODU_WIFI_MAC       (0x107)
#define INI_MODU_COEXIST        (0x108)
#define INI_MODU_DEV_WIFI       (0x109)
#define INI_MODU_DEV_GNSS       (0x10A)
#define INI_MODU_DEV_BT         (0x10B)
#define INI_MODU_DEV_FM         (0x10C)
#define INI_MODU_DEV_BFG_PLAT   (0x10D)
#define INI_MODU_FCC_NVRAM (0x10E)
#define INI_MODU_JP_NVRAM (0x10F)
#define INI_MODU_CE_NVRAM (0x110)


#define CUST_MODU_NVRAM         (0x200)

#define INI_MODU_POWER_FCC      (0xe1)
#define INI_MODU_POWER_ETSI     (0xe2)
#define INI_MODU_POWER_JP       (0xe3)
#define INI_MODU_INVALID        (0xff)

#define INI_STR_WIFI_NORMAL     ("[HOST_WIFI_NORMAL]")
#define INI_STR_BT_NORMAL       ("[HOST_BT_NORMAL]")
#define INI_STR_PLAT            ("[HOST_PLAT]")
#define INI_STR_WIFI_MAC        ("[HOST_WIFI_MAC]")
#define INT_STR_HOST_VERSION    ("[HOST_VERSION]")
#define INI_STR_COEXIST         ("[HOST_COEXIST]")
#define INI_STR_DEVICE_WIFI     ("[DEVICE_WIFI]")
#define INI_STR_DEVICE_BT       ("[DEVICE_BT]")
#define INI_STR_DEVICE_BFG_PLAT ("[DEVICE_BFG_PLAT]")
#define INI_STR_POWER_COMMON    ("[HOST_WIFI_POWER_COMMON]")
#define INI_STR_WIFI_FCC_NVRAM ("[HOST_WIFI_FCC_NVRAM]")
#define INI_STR_WIFI_JP_NVRAM ("[HOST_WIFI_JP_NVRAM]")
#define INI_STR_WIFI_CE_NVRAM ("[HOST_WIFI_CE_NVRAM]")

#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#define ini_init_mutex(mutex)       mutex_init(mutex)
#define ini_destroy_mutex(mutex)    mutex_destroy(mutex)
#define ini_mutex_lock(mutex)       mutex_lock(mutex)
#define ini_mutex_unlock(mutex)     mutex_unlock(mutex)
#elif defined(_PRE_OS_VERSION_LITEOS) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
#define ini_init_mutex(mutex)       osal_mutex_init(mutex)
#define ini_destroy_mutex(mutex)    osal_mutex_destroy(mutex)
#define ini_mutex_lock(mutex)       osal_mutex_lock(mutex)
#define ini_mutex_unlock(mutex)     osal_mutex_unlock(mutex)
#endif

#define INI_STR_MODU_LEN        (40)
#define MAX_READ_LINE_NUM       (192)
#define INI_FILE_PATH_LEN       (128)
#define INI_READ_VALUE_LEN      (128)
#define INI_VERSION_STR_LEN     (32)
#define INI_READ_VALUE_COUNT    (128)   /* 定制项最大可以解析的参数字节长度 */
/* 每个定制化参数的字节长度 */
#define INI_PARAM_BYTE_ONE   (1)
#define INI_PARAM_BYTE_TWO   (2)
#define INI_PARAM_BYTE_FOUR   (4)
#define INI_SUCC                (0)
#define INI_FAILED              (-1)

#define INI_FILE_TIMESPEC_UNRECONFIG (0)
#define INI_FILE_TIMESPEC_RECONFIG   (BIT0)
#define INI_NVRAM_RECONFIG           (BIT1)
#define inf_file_get_ctime(file_dentry) ((file_dentry)->d_inode->__i_ctime.tv_sec)

#define CONFIG_INI_PLAT_KERNEL_LOG_LEVEL 7
#define CONFIG_INI_PLAT_REBOOT_TYPE 0
 
#define CONFIG_INI_POWER_GPIO (-1)
#define CONFIG_INI_POWER_ON_GPIO_LEVEL 0
#define CONFIG_INI_WKUP_GPIO (-1)
#define CONFIG_INI_WKUP_GPIO_LEVEL 1

/* 总线聚合度 */
#define CONFIG_INI_BUS_D2H_SCHED_COUNT 8
#define CONFIG_INI_BUS_H2D_SCHED_COUNT 12
/* device平台低功耗使能 */
#define CONFIG_INI_DEVICE_PLT_PM_ENABLE 1
/* device唤醒host的GPIO配置 */
#define CONFIG_INI_DEVICE_AWAKE_HOST_GPIO_IDX 10
#define CONFIG_INI_DEVICE_AWAKE_HOST_GPIO_LEVEL 1
 
#define CONFIG_INI_CMU_XO_TRIM 0x83c
 
#define CONFIG_INI_XO_TRIM_TEMP_COMP_SW 0   // 频偏温补开关

#define CONFIG_INI_ZDIAG_FILE_DIR "/tmp/diag_log"
#define CONFIG_INI_ZDIAG_FILE_MAX_CNT 5
#define CONFIG_INI_ZDIAG_FILE_MAX_SIZE 131072
/*
 * 4 Message Header Definition
 */


/*
 * 5 Message Definition
 */


/*
 * 6 STRUCT Type Definition
 */
typedef struct ini_board_vervion {
    unsigned char board_version[INI_VERSION_STR_LEN];
} ini_board_version_stru;

typedef struct ini_param_vervion {
    unsigned char param_version[INI_VERSION_STR_LEN];
} ini_param_version_stru;

typedef struct file ini_file;

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
extern osal_s32 get_cust_conf_int32_etc(osal_s32 tag_index, osal_char *puc_var, osal_s32 *pul_value);
extern osal_s32 get_cust_conf_string_etc(osal_s32 tag_index, osal_char *puc_var, osal_char *puc_value, osal_u32 size);

extern osal_s32 ini_cfg_init_etc(void);
extern void  ini_cfg_exit_etc(void);
extern osal_s32 ini_file_check_conf_update(void);

osal_u32 ini_get_cust_item_list(const char *cust_param, osal_u8 *param_list, osal_u8 param_len,
    osal_u8 *list_len, osal_u8 max_len);
#endif
