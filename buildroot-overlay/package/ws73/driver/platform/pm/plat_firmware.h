/*
 * Copyright (c) CompanyNameMagicTag 2021-2023. All rights reserved.
 * Description: host platform firmware header file
 * Author: Huanghe
 * Create: 2021-06-08
 */

#ifndef __PLAT_FIRMWARE_H__
#define __PLAT_FIRMWARE_H__

#include "soc_osal.h"
#if defined(_PRE_MULTI_CORE_MODE) && defined(_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC)
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
#if defined(_PRE_OS_VERSION) && defined(_PRE_OS_VERSION_LINUX) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include "plat_exception_rst.h"
#endif
#endif
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/* memory */
#if defined(_PRE_OS_VERSION) && defined(_PRE_OS_VERSION_LINUX) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
static inline void *os_kzalloc_gfp(size_t size)
{
    return kzalloc(size, (GFP_KERNEL | __GFP_NOWARN));
}
#endif

#define READ_MEG_TIMEOUT            (2000)      /* 200ms */
#define READ_MEG_JUMP_TIMEOUT       (15000)   /* 15s */

#define FILE_CMD_WAIT_TIME_MIN      (5000)     /* 5000us */
#define FILE_CMD_WAIT_TIME_MAX      (5100)     /* 5100us */

#define SEND_BUF_LEN                (520)
#define RECV_BUF_LEN                (512)
#define VERSION_LEN                 (64)

#define READ_CFG_BUF_LEN            (2048)

#define HOST_DEV_TIMEOUT            (3)
#define INT32_STR_LEN               (32)

#define CMD_JUMP_EXEC_RESULT_SUCC   (0)
#define CMD_JUMP_EXEC_RESULT_FAIL   (1)

/* 以下是发往device命令的关键字 */
#define VER_CMD_KEYWORD             "VERSION"
#define JUMP_CMD_KEYWORD            "JUMP"
#define FILES_CMD_KEYWORD           "FILES"
#define SETPM_CMD_KEYWORD           "SETPM"
#define SETBUCK_CMD_KEYWORD         "SETBUCK"
#define SETSYSLDO_CMD_KEYWORD       "SETSYSLDO"
#define SETNFCRETLDO_CMD_KEYWORD    "SETNFCRETLDO"
#define SETPD_CMD_KEYWORD           "SETPD"
#define SETNFCCRG_CMD_KEYWORD       "SETNFCCRG"
#define SETABB_CMD_KEYWORD          "SETABB"
#define SETTCXODIV_CMD_KEYWORD      "SETTCXODIV"
#define RMEM_CMD_KEYWORD            "READM"
#define WMEM_CMD_KEYWORD            "WRITEM"
#define QUIT_CMD_KEYWORD            "QUIT"

/* 以下是device对命令执行成功返回的关键字，host收到一下关键字则命令执行成功 */
#define MSG_FROM_DEV_WRITEM_OK      "WRITEM OK"
#define MSG_FROM_DEV_READM_OK       ""
#define MSG_FROM_DEV_FILES_OK       "FILES OK"
#define MSG_FROM_DEV_READY_OK       "READY"
#define MSG_FROM_DEV_JUMP_OK        "JUMP OK"
#define MSG_FROM_DEV_SET_OK         "SET OK"
#define MSG_FROM_DEV_QUIT_OK        ""


#define COMPART_KEYWORD             ((char)' ')
#define CMD_LINE_SIGN               ((char)';')

#define CFG_INFO_RESERVE_LEN        (8)

#define FILE_COUNT_PER_SEND           (1)
#define MIN_FIRMWARE_FILE_TX_BUF_LEN  (4096)
#define MAX_FIRMWARE_FILE_TX_BUF_LEN  (32 * 1024)
#define MAX_FIRMWARE_FILE_RX_BUF_LEN  (128 * 1024)
#define MAX_PACKAGE_SIZE_INI            8


#define WCPU_ROM_START                     0x00008000
#define WCPU_ROM_END                       0x00090000
#define WCPU_ITCM_START                    0x00090000
#define WCPU_ITCM_END                      0x000CFFFF
#define WCPU_DTCM_START                    0x20000000
#define WCPU_DTCM_END                      0x20047FFF
#define WCPU_PKT_START                     0x60000000
#define WCPU_PKT_END                       0x6007FFFF

#if defined BOARD_ASIC_WIFI || defined BOARD_ASIC
#define BCPU_START                         0x80000000
#define BCPU_END                           0x80087FFF
#else
#define BCPU_START                         0x80010000
#define BCPU_END                           0x800AFFFF
#endif

#define FIRMWARE_FILESIZE_MAX (200 * 1024) // 固件文件大小最大值200K

typedef enum firmware_cfg_cmd_enum {
    ERROR_TYPE_CMD = 0,            /* 错误的命令 */
    FILE_TYPE_CMD,                 /* 下载文件的命令 */
    NUM_TYPE_CMD,                  /* 下载配置参数的命令 */
    QUIT_TYPE_CMD,                 /* 退出命令 */
} firmware_cfg_cmd_enum;

#define FIRMWARE_BOARD_INFO_NAME_LEN       10
#define FIRMWARE_BOARD_INFO_PARA_LEN       50

#define FIRMWARE_DOWNLOAD_BT_ROM_OFFSET    1
#define FIRMWARE_DOWNLOAD_BT_RAM_OFFSET    2
#define FIRMWARE_BOARD_INFO_SET_OFFSET     8
#define FIRMWARE_USER_NUM_SET_OFFSET       9
#define FIRMWARE_H2D_CFG_SET_OFFSET        10

#define FIRMWARE_REG_BOARD_INFO_SET_UU     "2,0x50000204,0x693C"
#define FIRMWARE_REG_BOARD_INFO_SET_UT     "2,0x50000204,0x533C"
#define FIRMWARE_REG_BOARD_INFO_SET_ST     "2,0x50000204,0x5353"
#define FIRMWARE_REG_BOARD_INFO_SET_SS     "2,0x50000204,0x3C53"
#define FIRMWARE_REG_BOARD_INFO_SET_ET     "2,0x50000204,0x5370"
#define FIRMWARE_REG_BOARD_INFO_SET_EU     "2,0x50000204,0x7070"

#define WR_CMU_XO_TRIM_ADDR 0x40019408
#define WR_CMU_XO_TRIM_LEN  4

typedef struct file OS_KERNEL_FILE_STRU;
typedef struct file os_kernel_file_stru;

osal_s32 write_device_reg(osal_u32 address, osal_u32 value);
osal_s32 read_device_reg(osal_u32 address, osal_u32 *value);

extern osal_s32 firmware_download_etc(void);

osal_s32 firmware_sha256_checksum(const osal_char *path);
#if defined(CONFIG_HCC_SUPPORT_UART) && defined(CONFIG_PLAT_UART_IO_CFG)
td_s32 gpio_state_change_uart2cfg(void);
td_s32 gpio_state_change_cfg2uart(void);
#endif

osal_s32 firmware_send_func(osal_u8 *data, osal_s32 data_max_len, osal_s32 len);

osal_s32 recv_device_mem_etc(os_kernel_file_stru *fp, osal_u8 *puc_data_buf, osal_s32 len);

os_kernel_file_stru *open_file_to_readm_etc(osal_u8 *name, osal_s32 flags);

osal_s32 firmware_recv_func(osal_u8 *data, osal_s32 len);

osal_s32 pm_firmware_init(osal_void);
osal_void pm_firmware_deinit(osal_void);

#if defined(CONFIG_SUPPORT_HCC_CONN_CHECK) && (CONFIG_SUPPORT_HCC_CONN_CHECK != 0)
osal_s32 hcc_connect_state_check(osal_void);
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of plat_firmware.h */
