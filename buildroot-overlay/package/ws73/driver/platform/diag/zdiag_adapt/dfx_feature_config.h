/*
 * Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
 * Description: dfx feature config
 * This file should be changed only infrequently and with great care.
 */
#ifndef __DFX_FEATURE_CONFIG_H__
#define __DFX_FEATURE_CONFIG_H__
/* host */
#define DFX_YES 1
#define DFX_NO 0

#define CONFIG_DFX_SUPPORT_IDLE_TASK DFX_NO
#define CONFIG_DFX_SUPPORT_TRANSMIT_FILE                DFX_NO
#define CONFIG_DFX_SUPPORT_DIAG                         DFX_YES
#define CONFIG_DFX_SUPOORT_DIAG_CMD_SRC                 DFX_NO
#define CONFIG_DFX_SUPPORT_DIAG_CMD_DST                 DFX_NO
#define CONFIG_DFX_SUPPORT_DIAG_IND_SRC                 DFX_NO
#define CONFIG_DFX_SUPPORT_DIAG_IND_DST                 DFX_NO
#define CONFIG_DFX_SUPPORT_DIAG_RX_PKT                  DFX_NO

#define CONFIG_DFX_SUPPORT_DIAG_CMD_VER                 DFX_NO
#define CONFIG_DFX_SUPPORT_DIAG_CMD_STAT                DFX_NO
#define CONFIG_DFX_SUPPORT_DIAG_CMD_DFX                 DFX_NO
#define CONFIG_DFX_SUPPORT_DIAG_UART_CHANNEL            DFX_NO
#define CONFIG_DFX_SUPPORT_DIAG_LOG_GATHER_DST          DFX_NO
#define CONFIG_DFX_SUPPORT_DIAG_CONNECT_DIRECTLY_ROLE   DFX_YES
#define CONFIG_DFX_SUPPORT_DIAG_UP_MACHINE              DFX_NO
#define CONFIG_DFX_SUPPORT_DIAG_BEAT_HEART              DFX_NO
#define CONFIG_DFX_SUPPORT_DIAG_CONNECT_PASSWORD        DFX_NO
#define CONFIG_DFX_SUPPORT_DIAG_VIRTUAL_AT              DFX_NO
#define CONFIG_DFX_SUPPORT_DIAG_VRTTUAL_SHELL           DFX_NO
#define CONFIG_DFX_SUPPORT_DIAG_ADAPT_SOC_LOG           DFX_NO
#define CONFIG_DFX_SUPPORT_SYS_MONITOR                  DFX_NO
#define CONFIG_DFX_SUPPORT_DELAY_REBOOT                 DFX_NO
#define CONFIG_DFX_SUPPORT_DIAG_CONNECT_USB             DFX_NO
#define CONFIG_DFX_SUPPORT_DIAG_CONNECT_BLE             DFX_NO
#define CONFIG_DFX_SUPPORT_DIAG_LOG_EMMC                DFX_NO

#define CONFIG_DFX_SUPPORT_FAULT_EVENT                  DFX_NO
#define CONFIG_DFX_SUPPORT_KEY_EVENT                    DFX_NO
#define CONFIG_DFX_SUPPORT_STAT_EVENT                   DFX_NO
/*
    特性：获取当前时间接口,单位 秒
    DFX_ADAPT_GET_CUR_SEC_TYPE_TIME(1)：使用time接口获取当前时间
*/
#define DFX_ADAPT_GET_CUR_SEC_TYPE_NOT_SUPPORT 0
#define DFX_ADAPT_GET_CUR_SEC_TYPE_TIME 1
#define CONFIG_DFX_ADAPT_GET_CUR_SEC_TYPE DFX_ADAPT_GET_CUR_SEC_TYPE_NOT_SUPPORT

#endif