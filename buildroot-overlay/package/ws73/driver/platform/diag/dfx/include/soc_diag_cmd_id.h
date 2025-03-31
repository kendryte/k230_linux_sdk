/*
 * Copyright (c) CompanyNameMagicTag 2020-2022. All rights reserved.
 * Description: DIAG - cmd id.
 */

#ifndef __SOC_DIAG_CMD_ID_H__
#define __SOC_DIAG_CMD_ID_H__

#define DIAG_L1_A_ID_MIN 0x0050 /* [0x0050, 0x1000 */
#define DIAG_L1_A_ID_MAX 0x1000
#define DIAG_MAC_A_ID_MIN 0x1000 /* [0x1000, 0x2000 */
#define DIAG_MAC_A_ID_MAX 0x2000

#define DIAG_APP_A_ID_MIN 0x5600 /* [0x5600, 0x5E00 */
#define DIAG_APP_A_ID_MAX 0x5E00

#define DIAG_L1_B_ID_MIN 0x2100 /* [0x2100, 0x2900 */
#define DIAG_L1_B_ID_MAX 0x2900
#define DIAG_MAC_B_ID_MIN 0x2900 /* [0x2900, 0x3000 */
#define DIAG_MAC_B_ID_MAX 0x3000

#define DIAG_APP_B_ID_MIN 0x5E00 /* [0x5E00, 0x6600 */
#define DIAG_APP_B_ID_MAX 0x6600

#define DIAG_CMD_OS_RESET 0x3009
#define DIAG_CMD_VER_QRY_1 0x3013
#define DIAG_CMD_SYS_QRY_1 0x3014
#define DIAG_CMD_TEST_CMD 0x3071
#define DIAG_CMD_TEST_CMD_2 0x3072

#define ID_DIAG_CMD_CYCLE_READ_MEM 0X30FE
#define ID_DIAG_CMD_READ_MEM 0X30FF

#define EXT_DBG_STAT_Q 0x3430          /* EXT_DBG_STAT_Q_S,,EXT_DBG_MEM_MOD_STAT_IND_S */
#define EXT_DBG_DUMP_DIAG_ITEMS 0x3431 /* EXT_DBG_DUMP_DIAG_ITEMS_REQ_S */
#define EXT_DBG_DFX_TASK_TEST 0x3432   /* EXT_DBG_DFX_TASK_TEST_S */

/* *
 * First, initialization and system related DIAG commands [0x5000, 0x5200
 */
#define DIAG_CMD_HOST_CONNECT    0x5000
#define DIAG_CMD_HOST_DISCONNECT 0x5001
#define DIAG_CMD_CONNECT_RANDOM  0x5002
#define DIAG_CMD_CONNECT_M_CHECK 0x5003
#define DIAG_CMD_PWD_CHANGE      0x5004
#define DIAG_CMD_HEART_BEAT      0x5005
#define DIAG_CMD_REMOTE_TEST     0x5805 /* delete node from white list */
#define DIAG_CMD_SIMULATE_AT     0x500D
#define DIAG_CMD_MOCKED_SHELL    0x500E

#define DIAG_CMD_DUMP_SYSERR     0x7003
#define DIAG_CMD_DUMP_SYSERR_MAX 0x7005

#define DIAG_CMD_SIM_INIT_STATUS_IND 0x5050
#define DIAG_CMD_UE_POWERON_IND      0x5051
#define DIAG_CMD_UE_POWEROFF_IND     0x5052
#define DIAG_CMD_TIMER_OUT_IND       0x5053
#define DIAG_CMD_UE_SYS_STATUS_IND   0x5054
#define DIAG_CMD_PORT_TEST_IND       0x5055
#define DIAG_CMD_VCOM_SPY            0x5056
#define DIAG_CMD_LOG_GATTER          0x5060

#define DIAG_CMD_GET_CPUP            0x5070
#define DIAG_CMD_GET_MEM_INFO        0x5071
#define DIAG_CMD_GET_RESOURCE_INFO   0x5072
#define DIAG_CMD_GET_TASK_INFO       0x5073

#define DIAG_CMD_MEM_MEM32          0x5074
#define DIAG_CMD_MEM_MEM16          0x5075
#define DIAG_CMD_MEM_MEM8           0x5076
#define DIAG_CMD_MEM_W1             0x5077
#define DIAG_CMD_MEM_W2             0x5078
#define DIAG_CMD_MEM_W4             0x5079

/* *
 * Second, equipment operation instructions [0x5200, 0x5300
 */
#define DIAG_CMD_POWER_OFF 0x5200 /* Shutdown */
#define DIAG_CMD_UE_RST 0x5201    /* Restart the device */
#define DIAG_CMD_POWER_INFO_IND 0x5202
#define DIAG_CMD_SCPU_START 0x5203 /* Turn on the slave CPU including DSP */

/* UE read and write commands, such as memory, registers, Flash and other peripherals. */
#define DIAG_CMD_MEM_RD 0x5203
#define DIAG_CMD_MEM_WR 0x5204
#define DIAG_CMD_REG_RD 0x5205
#define DIAG_CMD_REG_WR 0x5206
#define DIAG_CMD_GPIO_WR 0x5208
#define DIAG_CMD_GPIO_RD 0x5207
#define DIAG_CMD_MEM_DUMP 0x5209 /* Data DUMP */
#define DIAG_CMD_M 0x5220 /* M */
#define DIAG_CMD_D 0x5221 /* D */
#define DIAG_CMD_MEM_DUMP_IND 0x520A

/* *
 * DIAG_CMD_SW_VER_QRY: Software version information observable DIAG command
 * DIAG_CMD_HW_VER_QRY: Hardware version information observable DIAG command
 * DIAG_CMD_IMEI_QRY: Query IMEI number
 * DIAG_CMD_HW_INFO_QRY: device name in product version information
 * DIAG_CMD_SOC_INFO_QRY: Query the version of the chip
 */
#define DIAG_CMD_SW_VER_QRY 0x520B
#define DIAG_CMD_HW_VER_QRY 0x520C
#define DIAG_CMD_IMEI_QRY 0x520D
#define DIAG_CMD_HW_INFO_QRY 0x520E
#define DIAG_CMD_SOC_INFO_QRY 0x520F
/* DIAG_CMD_BUF_CFG : LOG emergency and normal buffer settings, the purpose is to store buffered data in MUX */
#define DIAG_CMD_BUF_CFG 0x5210
#define DIAG_CMD_GET_UNLOCK_CODE 0x5212
#define DIAG_CMD_GETBUF_CFG 0x5211 /* DIAG_CMD_GETBUF_CFG : Get emergency and normal buffer settings */


/* *
 * Third, the log command [0x5300, 0x5400
 */
/* Message configuration and reporting commands */
#define DIAG_CMD_LOG_AIR_IND 0x5304
#define DIAG_CMD_MSG_CFG_SET_SYS 0x5310
#define DIAG_CMD_MSG_CFG_SET_DEV 0x5311
#define DIAG_CMD_MSG_CFG_SET_USR 0x5312
#define DIAG_CMD_MSG_CFG_RST 0x5313
#define DIAG_CMD_MSG_RPT_SYS 0x5314
#define DIAG_CMD_MSG_RPT_LAYER 0x5315
#define DIAG_CMD_MSG_RPT_USR 0x5316
#define DIAG_CMD_MSG_RPT_AIR 0x5304
#define DIAG_CMD_MSG_CFG_SET_AIR 0x5512
#define DIAG_CMD_MSG_CFG_SET_LEVEL 0x5517

#define DIAG_CMD_VER_QRY 0x5318 /* ver command deleted */

#define DIAG_CMD_SYS_QRY 0x5319    /* sdm_sq */
#define DIAG_CMD_SYSE_QRY_R 0x5323 /* sdm_sq */
#define DIAG_CMD_SYS_END 0x5325
#define DIAG_CMD_CHL_SET 0x5333
#define DIAG_CMD_CHL_SET_END 0x5338

/* *
 * Fourth, the measurable command [0x5400, 0x5500
 */
#define DIAG_CMD_GET_CPU_INFO 0x5400

#define DIAG_CMD_HSO_AT 0x5451
#define DIAG_CMD_HSO_AT_SWT 0x5450
#define DIAG_CMD_MSP_SDM_QURY 0x5452
#define DIAG_CMD_MSP_SDM_QURY_IND 0x5453
#define DIAG_CMD_GTR_SET 0x5454

/* *
 * Five, NV read and write commands [0x5500, 0x5600
 */
#define DIAG_CMD_NV_RD 0x5500
#define DIAG_CMD_NV_WR 0x5501
#define DIAG_CMD_NV_RD_IND 0x5502
#define DIAG_CMD_NV_QRY 0x5503
#define DIAG_CMD_NV_EXPORT 0x5505
#define DIAG_CMD_NV_IMPORT 0x5504
#define DIAG_CMD_NV_EXPORT_IND 0x5506
#define DIAG_CMD_NV_DEL 0x5507
#define DIAG_CMD_NV_FILE_LOAD 0x5508
#define DIAG_CMD_NV_FILE_UPDATE 0x5509
#define DIAG_CMD_NV_FILE_EXPORT 0x550A
#define DIAG_CMD_NV_BACK 0x550C
#define DIAG_CMD_NV_UPDATE 0x550B
#define DIAG_CMD_NV_REFRESH 0x550D
#define DIAG_CMD_NV_CHANGESYSMODE 0x550E
#define DIAG_CMD_CHANGE_UART_CONFIG 0x550F

#define ID_DIAG_CMD_REMOTE 0x7000
#define ID_DIAG_CMD_REMOTE_USR 0x7001
#define ID_DIAG_CMD_REMOTE_END 0x7002
#define ID_DIAG_CMD_DO_FRAME_ERR 0x7007 /* DIAG error report */
#define DIAG_CMD_SIMULATE_AT_IND 0xA011

#define DIAG_CMD_TRANSMIT_ID_ST     0x7190
#define DIAG_CMD_TRANSMIT_GET_DATA  0x7190
#define DIAG_CMD_TRANSMIT_LS        0x7191
#define DIAG_CMD_TRANSMIT_DUMP_FILE 0x7192

#define DIAG_CMD_ID_TRANSMIT_REQUEST 0x7193
#define DIAG_CMD_ID_TRANSMIT_REPLY   0x7194
#define DIAG_CMD_ID_TRANSMIT_START   0x7195
#define DIAG_CMD_ID_STATE_NOTIFY     0x7196

#define DIAG_CMD_TRANSMIT_ID_END    0x71B0

#define DIAG_CMD_UPDATE_PREPARE     0x71B1
#define DIAG_CMD_UPDATE_REQUEST     0x71B2

#define DIAG_CMD_ID_DIAG_DFX_REQ    0x71C0
#define DIAG_CMD_ID_DIAG_DFX_STAT   0x71C1
#define DIAG_CMD_ID_DIAG_DFX_END    0x71CF
/* PSD */
#define DIAG_CMD_ID_PSD_ENABLE 0x71D0
#define DIAG_CMD_ID_PSD_REPORT 0x71D1
/* sample data */
#define DIAG_CMD_ID_BSLE_SAMPLE   0x71D4
#define DIAG_CMD_ID_WLAN_SAMPLE   0x71D5
#define DIAG_CMD_ID_SOC_SAMPLE    0x71D6
#define DIAG_CMD_ID_WLAN_PHY_SAMPLE    0x71D7
#define DIAG_CMD_ID_SAMPLE_DATA   0x7194
#define DIAG_CMD_ID_SAMPLE_FINISH 0x7196   /* 数采数据上报结束ID */

/* SHELL */
#define DIAG_CMD_ID_SHELL_CMD 0xA012

#endif /* __SOC_DIAG_CMD_ID_H__ */
