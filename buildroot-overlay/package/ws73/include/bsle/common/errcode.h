/**
 * Copyright (c) @CompanyNameMagicTag 2022-2023. All rights reserved. \n
 *
 * Description: Provides error numers \n
 * Author: @CompanyNameTag \n
 * History: \n
 * 2022-06-01， Create file. \n
 */

#ifndef ERRCODE_H
#define ERRCODE_H

#include <stdint.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

/**
 * @defgroup common_errcode Error Code Definition
 * @ingroup  common
 * @{
 */

/**
 * @brief  Definition of error code.
 */
typedef uint32_t errcode_t;

/*
* 1 COMMON ERR (0x8000_0000 - 0x8000_0079)
*/
#define ERRCODE_SUCC                                        0UL
#define ERRCODE_FAIL                                        0xFFFFFFFF
#define ERRCODE_INVALID_PARAM                               0x80000001
#define ERRCODE_NOT_SUPPORT                                 0x80000002

#define ERRCODE_MEMSET                                      0x80000003
#define ERRCODE_MEMCPY                                      0x80000004
#define ERRCODE_MALLOC                                      0x80000005

/*
* 2 soc osa (0x8000_0080 - 0x8000_1000)
*
* HASH Error
*/
#define ERRCODE_HASH_REG_ADDR_INVALID                       0x80000080
#define ERRCODE_HASH_INVALID_PARAMETER                      0x80000081
#define ERRCODE_HASH_UNSUPPORTED                            0x80000082
#define ERRCODE_HASH_FAILED_MEM                             0x80000083
#define ERRCODE_HASH_BUSY                                   0x80000084
#define ERRCODE_HASH_TIMEOUT                                0x80000085

/*
* 3 DRIVER (0x8000_1000 - 0x8000_3000)
*
* Gpio Error.
*/
#define ERRCODE_GPIO_DIR_SET_FAIL                           0x80001000
#define ERRCODE_GPIO_NOT_INIT                               0x80001001
#define ERRCODE_GPIO_RELEASE_FAIL                           0x80001002
#define ERRCODE_GPIO_REG_ADDR_INVALID                       0x80001003
#define ERRCODE_GPIO_INIT_TRX_STATE_FAIL                    0x80001004
#define ERRCODE_GPIO_ADD_QUEUE_FAIL                         0x80001005
#define ERRCODE_GPIO_STATE_MISMATCH                         0x80001006
#define ERRCODE_GPIO_NOT_IMPLEMENT                          0x80001007
#define ERRCODE_GPIO_ALREADY_SET_CALLBACK                   0x80001008

/* CALENDAR Error */
#define ERRCODE_CALENDAR_NOT_INIT                           0x80001020
#define ERRCODE_CALENDAR_INVALID_PARAMETER                  0x80001021
#define ERRCODE_CALENDAR_GET_DATETIME_FAILURE               0x80001022
#define ERRCODE_CALENDAR_GET_TIMESTAMP_FAILURE              0x80001023
#define ERRCODE_CALENDAR_FAILURE                            0x80001024

/* Uart Error. */
#define ERRCODE_UART_NOT_INIT                               0x80001040
#define ERRCODE_UART_CLAIM_FAIL                             0x80001041
#define ERRCODE_UART_RELEASE_FAIL                           0x80001042
#define ERRCODE_UART_REG_ADDR_INVALID                       0x80001043
#define ERRCODE_UART_INIT_TRX_STATE_FAIL                    0x80001044
#define ERRCODE_UART_ADD_QUEUE_FAIL                         0x80001045
#define ERRCODE_UART_STATE_MISMATCH                         0x80001046
#define ERRCODE_UART_NOT_IMPLEMENT                          0x80001047
#define UART_DMA_CFG_PARAM_INVALID                          0x80001048
#define UART_DMA_BUFF_NULL                                  0x80001049
#define UART_DMA_SHAKING_INVALID_OR_UART_FUNCS_NULL         0x8000104A
#define UART_DMA_CONFIGURE_FAIL                             0x8000104B
#define UART_DMA_START_TRANSFER_FAIL                        0x8000104C
#define UART_DMA_TRANSFER_TIMEOUT                           0x8000104D
#define UART_DMA_TRANSFER_ERROR                             0x8000104E

/* PWM Error */
#define ERRCODE_PWM_NOT_INIT                                0x80001080
#define ERRCODE_PWM_NOT_OPEN                                0x80001081
#define ERRCODE_PWM_INVALID_PARAMETER                       0x80001082
#define ERRCODE_PWM_REG_ADDR_INVALID                        0x80001083
#define ERRCODE_PWM_NOT_POWER_ON                            0x80001084

/* DMA Error */
#define ERRCODE_DMA_NOT_INIT                                0x80001100
#define ERRCODE_DMA_REG_ADDR_INVALID                        0x80001101
#define ERRCODE_DMA_INVALID_PARAMETER                       0x80001102
#define ERRCODE_DMA_RET_NO_AVAIL_CH                         0x80001103
#define ERRCODE_DMA_RET_ERROR_CONFIG                        0x80001104
#define ERRCODE_DMA_RET_HANDSHAKING_USING                   0x80001105
#define ERRCODE_DMA_LLI_CFG_ERROR                           0x80001106
#define ERRCODE_DMA_CH_BUSY                                 0x80001107
#define ERRCODE_DMA_RET_ERROR_ADDRESS_ALIGN                 0x80001108
#define ERRCODE_DMA_RET_TOO_MANY_DATA_TO_TRANSFER           0x80001109
#define ERRCODE_DMA_RET_ADDRESS_ACCESS_ERROR                0x80001110

/* ADC Error */
#define ERRCODE_ADC_REG_ADDR_INVALID                        0x80001140
#define ERRCODE_ADC_INVALID_PARAMETER                       0x80001141
#define ERRCODE_ADC_SCAN_NOT_DISABLE                        0x80001142
#define ERRCODE_ADC_INVALID_SAMPLE_VALUE                    0x80001143
#define ERRCODE_ADC_INVALID_CH_TYPE                         0x80001144
#define ERRCODE_ADC_TIMEOUT                                 0x80001145

/* Systick Error. */
#define ERRCODE_SYSTICK_NOT_INIT                            0x80001160
#define ERRCODE_SYSTICK_NOT_CLEARED                         0x80001161
#define ERRCODE_SYSTICK_REG_ADDR_INVALID                    0x80001162
#define ERRCODE_SYSTICK_NOT_IMPLEMENT                       0x80001163
#define ERRCODE_SYSTICK_INIT_STATE_FAIL                     0x80001164

/* TCXO Error */
#define ERRCODE_TCXO_REG_ADDR_INVALID                       0x80001180

/* Pinmux Error */
#define ERRCODE_PIN_INVALID_PARAMETER                       0x80001190
#define ERRCODE_PIN_MODE_NO_FUNC                            0x80001191
#define ERRCODE_PIN_NOT_INIT                                0x80001192

/* Watchdog Error */
#define ERRCODE_WDT_REG_ADDR_INVALID                        0x80001200

/* QDEC Error */
#define ERRCODE_QDEC_INVALID_STATE                          0x80001210
#define ERRCODE_QDEC_INVALID_PARAMETER                      0x80001211
#define ERRCODE_QDEC_DOUBLE_TRANSITION                      0x80001212
#define ERRCODE_QDEC_REG_ADDR_INVALID                       0x80001213

/* KEYSCAN Error */
#define ERRCODE_KEYSCAN_NOT_INIT                            0x80001220
#define ERRCODE_KEYSCAN_NOT_POWER_ON                        0x80001221
#define ERRCODE_KEYSCAN_POWER_ON                            0x80001222
#define ERRCODE_KEYSCAN_NULL                                0x80001223
#define ERRCODE_KEYSCAN_MAP_WRONG_SIZE                      0x80001224
#define ERRCODE_KEYSCAN_SAFE_FUNC_FAIL                      0x80001225
#define ERRCODE_KEYSCAN_NOT_ALLOW_TO_SLEEP                  0x80001226

/* TRNG Error */
#define ERRCODE_TRNG_REG_ADDR_INVALID                       0x80001230
#define ERRCODE_TRNG_INVALID_PARAMETER                      0x80001231
#define ERRCODE_TRNG_TIMEOUT                                0x80001232

/* PKE Error */
#define ERRCODE_PKE_REG_ADDR_INVALID                        0x80001240
#define ERRCODE_PKE_INVALID_PARAMETER                       0x80001241
#define ERRCODE_PKE_UNSUPPORTED                             0x80001242
#define ERRCODE_PKE_INVALID_PADDING                         0x80001243
#define ERRCODE_PKE_ENV_NOT_READY                           0x80001244
#define ERRCODE_PKE_FAILED_MEM                              0x80001245
#define ERRCODE_PKE_TIMEOUT                                 0x80001246

/* KM Error */
#define ERRCODE_KM_REG_ADDR_INVALID                         0x80001260
#define ERRCODE_KM_INVALID_PARAMETER                        0x80001261
#define ERRCODE_KM_UNSUPPORTED                              0x80001262
#define ERRCODE_KM_ENV_NOT_READY                            0x80001263
#define ERRCODE_KM_KEYSLOT_BUSY                             0x80001264
#define ERRCODE_KM_FAILED_MEM                               0x80001265
#define ERRCODE_KM_TIMEOUT                                  0x80001266

/* Cipher Error */
#define ERRCODE_CIPHER_REG_ADDR_INVALID                     0x80001280
#define ERRCODE_CIPHER_INVALID_PARAMETER                    0x80001281
#define ERRCODE_CIPHER_UNSUPPORTED                          0x80001282
#define ERRCODE_CIPHER_FAILED_MEM                           0x80001283
#define ERRCODE_CIPHER_ENV_NOT_READY                        0x80001284
#define ERRCODE_CIPHER_SIZE_NOT_ALIGNED                     0x80001285
#define ERRCODE_CIPHER_BUSY                                 0x80001286
#define ERRCODE_CIPHER_TIMEOUT                              0x80001287

/* I2C Error. */
#define ERRCODE_I2C_NOT_INIT                                0x80001300
#define ERRCODE_I2C_ALREADY_INIT                            0x80001301
#define ERRCODE_I2C_INVALID_PARAMETER                       0x80001302
#define ERRCODE_I2C_NOT_IMPLEMENT                           0x80001303
#define ERRCODE_I2C_EVENT_INIT_FAILURE                      0x80001304
#define ERRCODE_I2C_RATE_INVALID                            0x80001305
#define ERRCODE_I2C_ADDRESS_INVLID                          0x80001306
#define ERRCODE_I2C_PERMISSION_INVALID                      0x80001307
#define ERRCODE_I2C_NO_VALID_DATA                           0x80001308
#define ERRCODE_I2C_SEND_PARAM_INVALID                      0x80001309
#define ERRCODE_I2C_RECEIVE_PARAM_INVALID                   0x80001310
#define ERRCODE_I2C_WAIT_EXCEPTION                          0x80001311
#define ERRCODE_I2C_WAIT_CONTINUE                           0x80001312
#define ERRCODE_I2C_TIMEOUT                                 0x80001313
#define ERRCODE_I2C_ACK_ERR                                 0x80001314
#define ERRCODE_I2C_MUTEX_PROC_FAIL                         0x80001315
#define ERRCODE_I2C_DMA_CONFIG_ERROR                        0x80001316
#define ERRCODE_I2C_DMA_TRANSFER_ERROR                      0x80001317

/* TIMER error */
#define ERRCODE_TIMER_NO_ENOUGH                             0x80001320
#define ERRCODE_TIEMR_NOT_CREATED                           0x80001321
#define ERRCODE_TIMER_INVALID_REG_ADDR                      0x80001322
#define ERRCODE_TIMER_INVALID_STATE                         0x80001323
#define ERRCODE_TIMER_NOT_INIT                              0x80001324

/* SPI/QSPI Error. */
#define ERRCODE_SPI_CONFIG_FAIL                             0x80001330
#define ERRCODE_SPI_REG_ADDR_INVALID                        0x80001331
#define ERRCODE_SPI_MODE_MISMATCH                           0x80001332
#define ERRCODE_SPI_TIMEOUT                                 0x80001333
#define ERRCODE_SPI_RX_FIFO_FULL                            0x80001334
#define ERRCODE_SPI_SLAVE_SELECT_INVALID                    0x80001335
#define ERRCODE_SPI_DMA_CONFIG_ERROR                        0x80001336
#define ERRCODE_SPI_DMA_TRANSFER_ERROR                      0x80001337
#define ERRCODE_SPI_INVALID_BYTES                           0x80001338
#define ERRCODE_SPI_STATE_MISMATCH                          0x8000133A
#define ERRCODE_SPI_ADD_QUEUE_FAIL                          0x8000133B
#define ERRCODE_SPI_DMA_IRQ_MODE_MUTEX                      0x8000133C
#define ERRCODE_SPI_INVALID_TMODE                           0x8000133E

/* SFC error */
#define ERRCODE_SFC_DMA_BUSY                                0x80001339
#define ERRCODE_SFC_INVALID_PARAM                           0x8000133F
#define ERRCODE_SFC_NOT_INIT                                0x80001340
#define ERRCODE_SFC_FLASH_NOT_SUPPORT                       0x80001341
#define ERRCODE_SFC_ADDRESS_OVERSTEP                        0x80001342
#define ERRCODE_SFC_REG_ADDR_INVALID                        0x80001343
#define ERRCODE_SFC_FLASH_TIMEOUT_WAIT_READY                0x80001344
#define ERRCODE_SFC_CMD_ERROR                               0x80001345
#define ERRCODE_SFC_CMD_NOT_SUPPORT                         0x80001346
#define ERRCODE_SFC_ERASE_FORM_ERROR                        0x80001347
#define ERRCODE_SFC_DEFAULT_INIT                            0x80001348
#define ERRCODE_SFC_PORT_INVALID_PARAM                      0x80001349

/* MPU Error. */
#define ERRCODE_MPU_NOT_INIT                                0x80001350
#define ERRCODE_MPU_INVALID_PARAMETER                       0x80001351
#define ERRCODE_MPU_CONFIG_FAILED                           0x80001352
#define ERRCODE_MPU_REG_ADDR_INVALID                        0x80001353
#define ERRCODE_MPU_DREGION_ERROR                           0x80001354
#define ERRCODE_MPU_ADDR_ERROR                              0x80001355

#define ERRCODE_TSENSOR_NOT_INIT                            0x80001360
#define ERRCODE_TSENSOR_INVALID_PARAMETER                   0x80001361
#define ERRCODE_TSENSOR_REG_ADDR_INVALID                    0x80001362
#define ERRCODE_TSENSOR_GET_TEMP_INVALID                    0x80001363
#define ERRCODE_TSENSOR_WORKMODE_ERROR                      0x80001364

/* PSRAM Error. */
#define ERRCODE_PSRAM_RET_UNINIT                            0x80001370
#define ERRCODE_PSRAM_RET_INITED                            0x80001371
#define ERRCODE_PSRAM_RET_TIMEOUT                           0x80001372
#define ERRCODE_PSRAM_RET_WRONG_PARA                        0x80001373
#define ERRCODE_PSRAM_RET_WRONG_DEVICE                      0x80001374
#define ERRCODE_PSRAM_RET_XIP_EN                            0x80001375
#define ERRCODE_PSRAM_RET_XIP_DIS                           0x80001376
#define ERRCODE_PSRAM_RET_SLEEP                             0x80001377
#define ERRCODE_PSRAM_RET_WAKE                              0x80001378
#define ERRCODE_PSRAM_RET_ERROR                             0x80001379

/* EDGE Error */
#define ERRCODE_EDGE_NOT_INITED                             0x80001380
#define ERRCODE_EDGE_INVALID_PARAMETER                      0x80001381
#define ERRCODE_EDGE_REG_ADDR_INVALID                       0x80001382

/* EMBED FLASH ERROR. */
#define ERRCODE_EFLASH_NOT_INIT                             0x80001385
#define ERRCODE_EFLASH_INVALID_PARAMETER                    0x80001386
#define ERRCODE_EFLASH_REG_ADDR_INVALID                     0x80001387

/* EFUSE error */
#define ERRCODE_EFUSE_INVALID_PARAM                         0x80001390
#define ERRCODE_EFUSE_NOT_INIT                              0x80001391
#define ERRCODE_EFUSE_REG_ADDR_INVALID                      0x80001392

/* MEM_MONTINOR Error */
#define ERRCODE_MEM_MONITOR_INVALID_PARAMETER               0x80001400
#define ERRCODE_MEM_MONITOR_REG_ADDR_INVALID                0x80001401
#define ERRCODE_MEM_MONITOR_WORKMODE_ERROR                  0x80001402
#define ERRCODE_MEM_MONITOR_GET_TEMP_INVALID                0x80001403

/* PDM Error. */
#define ERRCODE_PDM_REG_ADDR_INVALID                        0x80001380
/* SIO Error. */
#define ERRCODE_I2S_NOT_INIT                                0x80001410
#define ERRCODE_I2S_NOT_CONFIG                              0x80001411
#define ERRCODE_PCM_NOT_INIT                                0x80001412
#define ERRCODE_PCM_NOT_CONFIG                              0x80001413
#define ERRCODE_SIO_REG_ADDR_INVALID                        0x80001414

/* CAN-FD Error. */
#define ERRCODE_CAN_FD_INITED                               0x80001420
#define ERRCODE_CAN_FD_INVALID_ADDR                         0x80001421
#define ERRCODE_CAN_FD_NOT_INIT                             0x80001422
#define ERRCODE_CAN_FD_SEND_TIMEOUT                         0x80001423
#define ERRCODE_CAN_FD_RX_FIFO_EMPTY                        0x80001424
#define ERRCODE_CAN_ACTIVE_STS                              0x80001425
#define ERRCODE_CAN_PASSIVE_STS                             0x80001426
#define ERRCODE_CAN_BUS_OFF                                 0x80001427

/* RTC error */
#define ERRCODE_RTC_NOT_INITED                              0x80001405
#define ERRCODE_RTC_REG_ADDR_INVALID                        0x80001406
#define ERRCODE_RTC_NO_ENOUGH                               0x80001407
#define ERRCODE_RTC_NOT_CREATED                             0x80001408

/* SECURITY_UNIFIED Error */
#define ERROR_SECURITY_INVALID_PARAM               0x80001500  /* input param's value is not in valid range. */
#define ERROR_SECURITY_PARAM_IS_NULL               0x80001501  /* input param is NULL and required not NULL. */
#define ERROR_SECURITY_NOT_INIT                    0x80001502  /* call other functions before call init function. */
#define ERROR_SECURITY_UNSUPPORT                   0x80001503  /* some configuration is unsupport. */
#define ERROR_SECURITY_UNEXPECTED                  0x80001504  /* reture when unexpected error occurs. */
#define ERROR_SECURITY_CHN_BUSY                    0x80001505  /* try to create channel but all channels are busy. */
#define ERROR_SECURITY_CTX_CLOSED                  0x80001506  /* using one ctx to do something but has been closed. */
#define ERROR_SECURITY_NOT_SET_CONFIG              0x80001507  /* not set_config but need for symc. */
#define ERROR_SECURITY_NOT_ATTACHED                0x80001508  /* not attach but need for symc. */
#define ERROR_SECURITY_NOT_MAC_START               0x80001509  /* not mac_start but need for symc. */
#define ERROR_SECURITY_INVALID_HANDLE              0x80001510  /* pass one invalid handle. */
#define ERROR_SECURITY_GET_PHYS_ADDR               0x80001511  /* transfer from virt_addr to phys_addr failed. */
#define ERROR_SECURITY_SYMC_LEN_NOT_ALIGNED        0x80001512  /* length isn't aligned to 16-Byte except CTR/CCM/GCM. */
/* the phys_addr writing to register is not aligned to 4-Byte. */
#define ERROR_SECURITY_SYMC_ADDR_NOT_ALIGNED       0x80001513
#define ERROR_SECURITY_PKE_RSA_SAME_DATA           0x80001514  /* rsa exp_mod  the input is equal to output. */
#define ERROR_SECURITY_PKE_RSA_CRYPTO_V15_CHECK    0x80001515  /* rsa crypto v15 padding check failed. */
#define ERROR_SECURITY_PKE_RSA_CRYPTO_OAEP_CHECK   0x80001516  /* rsa crypto oaep padding check failed. */
#define ERROR_SECURITY_PKE_RSA_VERIFY_V15_CHECK    0x80001517  /* rsa verify v15 padding check failed. */
#define ERROR_SECURITY_PKE_RSA_VERIFY_PSS_CHECK    0x80001518  /* rsa verify pss padding check failed. */
#define ERROR_SECURITY_PKE_RSA_GEN_KEY             0x80001519  /* rsa generate key failed. */
#define ERROR_SECURITY_PKE_ECDSA_VERIFY_CHECK      0x80001520  /* ecdsa verify check failed. */
/* Outer's Error Code. 0x40 ~ 0x5F. */
#define ERROR_SECURITY_MEMCPY_S                    0x80001540  /* call memcpy_s failed. */
#define ERROR_SECURITY_MALLOC                      0x80001541  /* call xxx_malloc failed. */
#define ERROR_SECURITY_MUTEX_INIT                  0x80001542  /* call xxx_mutex_init failed. */
#define ERROR_SECURITY_MUTEX_LOCK                  0x80001543  /* call xxx_lock failed. */
/* Specific Error Code for UAPI. 0x60 ~ 0x6F. */
#define ERROR_SECURITY_DEV_OPEN_FAILED             0x80001560  /* open dev failed. */
#define ERROR_SECURITY_COUNT_OVERFLOW              0x80001561  /* call init too many times. */
/* Specific Error Code for Dispatch. 0x70 ~ 0x7F. */
#define ERROR_SECURITY_CMD_DISMATCH                0x80001570  /* cmd is dismatched. */
#define ERROR_SECURITY_COPY_FROM_USER              0x80001571  /* call copy_from_user failed. */
#define ERROR_SECURITY_COPY_TO_USER                0x80001572  /* call copy_to_user failed. */
/* parse user's mem handle to kernel's mem handle failed. */
#define ERROR_SECURITY_MEM_HANDLE_GET              0x80001573
#define ERROR_SECURITY_GET_OWNER                   0x80001574  /* call crypto_get_owner failed. */
/* Specific Error Code for KAPI. 0x80 ~ 0x8F. */
#define ERROR_SECURITY_PROCESS_NOT_INIT            0x80001580  /* one process not call kapi_xxx_init first. */
#define ERROR_SECURITY_MAX_PROCESS                 0x80001581  /* process's num is over the limit. */
#define ERROR_SECURITY_MEMORY_ACCESS               0x80001582  /* access the memory that does not belong to itself.  */
#define ERROR_SECURITY_INVALID_PROCESS             0x80001583  /* the process accesses resources of other processes. */
/* Specific Error Code for DRV. 0x90 ~ 0x9F. */

/* Specific Error Code for HAL. 0xA0 ~ 0xAF. */
#define ERROR_SECURITY_HASH_LOGIC                  0x800015A0  /* hash logic's error occurs. */
#define ERROR_SECURITY_PKE_LOGIC                   0x800015A1  /* pke logic's error occurs. */
#define ERROR_SECURITY_INVALID_CPU_TYPE            0x800015A2  /* logic get the invalid cpu type. */
#define ERROR_SECURITY_INVALID_REGISTER_VALUE      0x800015A3  /* value in register is invalid. */
#define ERROR_SECURITY_INVALID_PHYS_ADDR           0x800015A4  /* phys_addr is invalid. */
/* Specific Error Code for Timeout. 0xB0 ~ 0xBF. */
#define ERROR_SECURITY_GET_TRNG_TIMEOUT            0x800015B0  /* logic get rnd timeout. */
#define ERROR_SECURITY_HASH_CLEAR_CHN_TIMEOUT      0x800015B1  /* clear hash channel timeout. */
#define ERROR_SECURITY_HASH_CALC_TIMEOUT           0x800015B2  /* hash calculation timeout. */
#define ERROR_SECURITY_SYMC_CLEAR_CHN_TIMEOUT      0x800015B3  /* clear symc channel timeout. */
#define ERROR_SECURITY_SYMC_CALC_TIMEOUT           0x800015B4  /* symc crypto timeout. */
#define ERROR_SECURITY_SYMC_GET_TAG_TIMEOUT        0x800015B5  /* symc get tag timeout. */
#define ERROR_SECURITY_PKE_LOCK_TIMEOUT            0x800015B6  /* pke lock timeout. */
#define ERROR_SECURITY_PKE_WAIT_DONE_TIMEOUT       0x800015B7  /* pke wait done timeout. */
#define ERROR_SECURITY_PKE_ROBUST_WARNING          0x800015B8  /* pke wait done timeout. */

/*
*
* PARTITION Error
*/
#define ERRCODE_PARTITION_INIT_ERR                          0x80003000
#define ERRCODE_PARTITION_NOT_SUPPORT                       0x80003001
#define ERRCODE_PARTITION_INVALID_PARAMS                    0x80003002
#define ERRCODE_PARTITION_CONFIG_NOT_FOUND                  0x80003003
#define ERRCODE_IMAGE_CONFIG_NOT_FOUND                      0x80003004

/* AT Error */
#define ERRCODE_AT_CHANNEL_BUSY                             0x80003020
#define ERRCODE_AT_CMD_TOO_LONG                             0x80003021
#define ERRCODE_AT_CMD_REPEAT                               0x80003022
#define ERRCODE_AT_CMD_TABLE_PARA_ERROR                     0x80003023
#define ERRCODE_AT_MSG_SEND_ERROR                           0x80003024

/* UPG error */
#define ERRCODE_UPG_NOT_INIT                                0x80003040
#define ERRCODE_UPG_ALREADY_INIT                            0x80003041
#define ERRCODE_UPG_INVALID_PARAMETER                       0x80003042
#define ERRCODE_UPG_WRONG_IMAGE_NUM                         0x80003043
#define ERRCODE_UPG_INVALID_IMAGE_ID                        0x80003044
#define ERRCODE_UPG_NULL_POINTER                            0x80003045
#define ERRCODE_UPG_NOT_SUPPORTED                           0x80003046
#define ERRCODE_UPG_NOT_NEED_TO_UPDATE                      0x80003047
#define ERRCODE_UPG_FLASH_ERASE_ERROR                       0x80003048
#define ERRCODE_UPG_FLASH_WRITE_ERROR                       0x80003049
#define ERRCODE_UPG_FLAG_NOT_INITED                         0x80003050
#define ERRCODE_UPG_NO_ENOUGH_SPACE                         0x80003051
#define ERRCODE_UPG_PARTITION_ALIGN_ERROR                   0x80003052
#define ERRCODE_UPG_INVALID_PREPARE_INFO                    0x80003053
#define ERRCODE_UPG_NOT_PREPARED                            0x80003054
#define ERRCODE_UPG_INVALID_BUFF_LEN                        0x80003055
#define ERRCODE_UPG_INVALID_OFFSET                          0x80003056
#define ERRCODE_UPG_FILE_OPEN_FAIL                          0x80003057
#define ERRCODE_UPG_FILE_WRITE_FAIL                         0x80003058
#define ERRCODE_UPG_FILE_SEEK_FAIL                          0x80003059
#define ERRCODE_UPG_FILE_READ_FAIL                          0x80003060
#define ERRCODE_UPG_EMPTY_FILE                              0x80003061
#define ERRCODE_UPG_DECOMPRESS_FAIL                         0x80003062
#define ERRCODE_UPG_VERIFICATION_KEY_ERROR                  0x80003063
#define ERRCODE_UPG_CHECK_FOTA_ERROR                        0x80003064
#define ERRCODE_UPG_SET_IMG_ERROR                           0x80003065
#define ERRCODE_UPG_BACKUP_UPDATE_ERROR                     0x80003066
#define ERRCODE_UPG_RETRY_ALL_FAIL                          0x80003067
#define ERRCODE_UPG_VERSION_ERROR                           0x80003068

/* NV error */
#define ERRCODE_NV_KEY_NOT_IN_WRITE_QUEUE                   0x80003080
#define ERRCODE_NV_KEY_NOT_FOUND                            0x80003081
#define ERRCODE_NV_GET_BUFFER_TOO_SMALL                     0x80003082
#define ERRCODE_NV_INVALID_PARAMS                           0x80003083
#define ERRCODE_NV_RPC_ERROR                                0x80003084
#define ERRCODE_NV_TRYING_TO_MODIFY_A_PERMANENT_KEY         0x80003085
#define ERRCODE_NV_BUFFER_TOO_SMALL                         0x80003086
#define ERRCODE_NV_NO_ENOUGH_SPACE                          0x80003087
#define ERRCODE_NV_ILLEGAL_OPERATION                        0x80003088
#define ERRCODE_NV_SEM_WAIT_ERR                             0x80003089
#define ERRCODE_NV_NOT_INITIALISED                          0x80003090
#define ERRCODE_NV_NOTIFY_LIST_FULL                         0x80003091
#define ERRCODE_NV_INIT_FAILED                              0x80003092
#define ERRCODE_NV_ZERO_LENGTH_COPY                         0x80003093
#define ERRCODE_NV_NOTIFY_SEGMENT_ERR                       0x80003094
#define ERRCODE_NV_READ_FLASH_ERR                           0x80003095
#define ERRCODE_NV_DATA_MISMATCH                            0x80003096
#define ERRCODE_NV_LENGTH_MISMATCH                          0x80003097
#define ERRCODE_NV_KEY_HASH_BUFFER_NOT_ALLOCATED            0x80003098
#define ERRCODE_NV_KEY_DATA_BUFFER_NOT_ALLOCATED            0x80003099
#define ERRCODE_NV_INVALID_KEY_HEADER                       0x80003100
#define ERRCODE_NV_SEARCH_PATTERN_MISMATCH                  0x80003101
#define ERRCODE_NV_SEARCH_KEY_TYPE_MISMATCH                 0x80003102
#define ERRCODE_NV_SEARCH_KEY_STATE_MISMATCH                0x80003103
#define ERRCODE_NV_HASH_MISMATCH                            0x80003104
#define ERRCODE_NV_PAGE_NOT_FOUND                           0x80003105
#define ERRCODE_NV_INVALID_PAGE                             0x80003106
#define ERRCODE_NV_INVALID_STORE                            0x80003107
#define ERRCODE_NV_DEFRAGMENTATION_NEEDED                   0x80003108
#define ERRCODE_NV_HASH_UNAVAILABLE                         0x80003109
#define ERRCODE_NV_AES_UNAVAILABLE                          0x80003110
#define ERRCODE_NV_BUFFER_PRIMED_PREMATURELY                0x80003111
#define ERRCODE_NV_KEY_HASH_BUFFER_TOO_SMALL                0x80003112
#define ERRCODE_NV_WRITE_BUFFER_NOT_ALLOCATED               0x80003113
#define ERRCODE_NV_WRITE_BUFFER_TOO_SMALL                   0x80003114
#define ERRCODE_NV_WRITE_VETOED                             0x80003115
#define ERRCODE_NV_KEY_HEADER_BUFFER_NOT_ALLOCATED          0x80003116
#define ERRCODE_NV_STATE_INVALID                            0x80003117

/* FLASH error */
#define ERRCODE_FLASH_TASK_COMPLETED                        0x80003130
#define ERRCODE_FLASH_INVALID_PARAM_BEYOND_ADDR             0x80003131
#define ERRCODE_FLASH_TASK_PE_VETO                          0x80003132
#define ERRCODE_FLASH_CONFIG_FAIL                           0x80003133
#define ERRCODE_FLASH_SPI_TRANS_FAIL                        0x80003134
#define ERRCODE_FLASH_TIMEOUT                               0x80003135
#define ERRCODE_FLASH_FALSE_MODE                            0x80003136
#define ERRCODE_FLASH_SPI_CONFIG_FAIL                       0x80003137
#define ERRCODE_FLASH_SPI_INIT_FAIL                         0x80003138
#define ERRCODE_FLASH_INIT_FAIL                             0x80003139

/* IPC Error. */
#define ERRCODE_IPC_NOT_INIT                                0x80003140
#define ERRCODE_IPC_INVALID_PARAMETER                       0x80003141
#define ERRCODE_IPC_REG_ADDR_INVALID                        0x80003142
#define ERRCODE_IPC_NOT_REGISTER_HAL_FUNCS                  0x80003143
#define ERRCODE_IPC_UNDEFINED_ACTION                        0x80003144
#define ERRCODE_IPC_ASYNC_BUF_OVERFLOW                      0x80003145
#define ERRCODE_IPC_MAILBOX_STATUS_BUSY                     0x80003146
#define ERRCODE_IPC_MAILBOX_STATUS_IDLE                     0x80003147

/* Testsuite Error. */
#define ERRCODE_TEST_SUITE_STATUS_RETURN_CONTROL            0x80003160
#define ERRCODE_TEST_SUITE_OK                               0x80003161
#define ERRCODE_TEST_SUITE_UNKNOWN_FUNCTION                 0x80003162
#define ERRCODE_TEST_SUITE_ERROR_TIMED_OUT                  0x80003163
#define ERRCODE_TEST_SUITE_ERROR_BAD_PARAMS                 0x80003164
#define ERRCODE_TEST_SUITE_ERROR_PIN_ALLOCATION             0x80003165
#define ERRCODE_TEST_SUITE_ERROR_PIN_DEALLOCATION           0x80003166
#define ERRCODE_TEST_SUITE_ERROR_UNEXPECTED_INTERRUPT       0x80003167
#define ERRCODE_TEST_SUITE_ALLOC_FAILED                     0x80003168
#define ERRCODE_TEST_SUITE_CONFIG_FAILED                    0x80003169
#define ERRCODE_TEST_SUITE_UNKNOWN_COMMAND                  0x80003170
#define ERRCODE_TEST_SUITE_UNRECOGNIZED_COMMAND             0x80003171
#define ERRCODE_TEST_SUITE_TEST_FAILED                      0x80003172

/* DIAG Error */
#define ERRCODE_DIAG_STAT_INVALID_ID                        0x80003180 /* stat id is invalid */
#define ERRCODE_DIAG_NO_INITILIZATION                       0x80003181 /* diag not initilizate */
#define ERRCODE_DIAG_NOT_FOUND                              0x80003182 /* diag id not found */
#define ERRCODE_DIAG_INVALID_PARAMETER                      0x80003183 /* parameter invalid */
/* If the return value is ERRCODE_DIAG_CONSUMED, it indicates that the user sends a response (local connection)
   to the host through the diag_send_ack_packet_prv API. The DIAG framework does not automatically */
#define ERRCODE_DIAG_CONSUMED                               0x80003184
#define ERRCODE_DIAG_TOO_SMALL_BUFFER                       0x80003185 /* too small buffer */
#define ERRCODE_DIAG_NO_MORE_DATA                           0x80003186 /* uart receive no more data */
#define ERRCODE_DIAG_NOT_ENOUGH_MEMORY                      0x80003187 /* malloc memory fail */
#define ERRCODE_DIAG_INVALID_HEAP_ADDR                      0x80003188 /* addr out of heap range */
#define ERRCODE_DIAG_NOT_CONNECT                            0x80003189 /* diag not connect */
#define ERRCODE_DIAG_BUSY                                   0x8000318A /* channel is busy */
#define ERRCODE_DIAG_TOO_LARGE_FRAME                        0x8000318B /* too large frame */
#define ERRCODE_DIAG_RAM_ALIGN                              0x8000318C /* ram addr not align */
#define ERRCODE_DIAG_NOT_SUPPORT                            0x8000318D /* not support */
#define ERRCODE_DIAG_QUEUE_FULL                             0x8000318E /* queue full */
#define ERRCODE_DIAG_MSG_CFG_NOT_ALLOW                      0x8000318F /* msg config allow send msg to pc */
#define ERRCODE_DIAG_TOO_LARGE_MSG                          0x80003190 /* too large msg */
#define ERRCODE_DIAG_INVALID_CODE_ADDR                      0x80003191 /* invalid code addr */
#define ERRCODE_DIAG_OBJ_NOT_FOUND                          0x80003192 /* object not found */
#define ERRCODE_DIAG_BLOCK_EMPTY                            0x80003193 /* buffer list is empty */
#define ERRCODE_DIAG_NOT_ENOUGH_SHARE_SPACE                 0x80003194 /* share space not enough */
#define ERRCODE_DIAG_CORRUPT_SHARED_MEMORY                  0x80003195 /* share space ctrl is corrupt */
#define ERRCODE_DIAG_NO_NEXT_MESSAGE                        0x80003196 /* no more msg in share mempry */
#define ERRCODE_DIAG_KV_NOT_SUPPORT_ID                      0x80003197 /* not support kv id */
#define ERRCODE_DIAG_BAD_DATA                               0x80003198 /* bad data */
#define ERRCODE_DIAG_CRC_ERROR                              0x80003199 /* crc error */

/* Logfile Error */
#define ERRCODE_DFX_LOGFILE_INTERAL_FAIL                    0x80003210
#define ERRCODE_DFX_LOGFILE_RECORD_INVALID                  0x80003211
#define ERRCODE_DFX_LOGFILE_WRITE_FAIL                      0x80003212
#define ERRCODE_DFX_LOGFILE_OPEN_FAIL                       0x80003213
#define ERRCODE_DFX_LOGFILE_EVENT_FAILURE                   0x80003214
#define ERRCODE_DFX_LOGFILE_MUTEX_FAILURE                   0x80003215
#define ERRCODE_DFX_LOGFILE_TIMER_FAILURE                   0x80003216
#define ERRCODE_DFX_LOGFILE_THREAD_FAILURE                  0x80003217
#define ERRCODE_DFX_LOGFILE_ALREADY_OPEN                    0x80003218
#define ERRCODE_DFX_LOGFILE_MKDIR_FATAL                     0x80003219
#define ERRCODE_DFX_LOGFILE_SUSPENDED                       0x8000321A
#define ERRCODE_DFX_LOGFILE_FLASH_PREPARE_FAIL              0x80003220
#define ERRCODE_DFX_LOGFILE_NOT_ENOUGH_SPACE                0x80003221

/* PM Error */
#define ERRCODE_PM_ERROR                                    0x80003230
#define ERRCODE_PM_INVALID_PARAMETER                        0x80003231
#define ERRCODE_PM_MEM_ALLOC_ERROR                          0x80003232
#define ERRCODE_PM_RET_MEM_FREE_ERROR                       0x80003233
#define ERRCODE_PM_CUSTOM_MODE_EXISTS                       0x80003234
#define ERRCODE_PM_SLEEP_THRESHOLD_TOO_SMALL                0x80003235
#define ERRCODE_PM_SWITCH_FUNC_NULL                         0x80003236
#define ERRCODE_PM_PF_IS_OFF                                0x80003237
#define ERRCODE_PM_SWITCH_FUNC_FAIL                         0x80003238
#define ERRCODE_PM_RECORD_REQ_STS_FAIL                      0x80003239
#define ERRCODE_PM_WFI_PROCESS                              0x80003240
#define ERRCODE_PM_WORK_PROCESS                             0x80003241
#define ERRCODE_PM_DEEPSLEEP_PROCESS                        0x80003242
#define ERRCODE_PM_LIGHTSLEEP_PROCESS                       0x80003243
#define ERRCODE_PM_SERV_ID_NOT_ENABLE                       0x80003244
#define ERRCODE_PM_MODE_DOWNGRADED                          0x80003245
#define ERRCODE_PM_MODE_ONLY_REMOVE_VOTE                    0x80003246
#define ERRCODE_PM_MODE_ONLY_JOIN_VOTE                      0x80003247

/* SDIO Device Error. */
#define ERRCODE_SDIO_NOT_INIT                               0x80003250
#define ERRCODE_SDIO_INVALID_PARAMETER                      0x80003251
#define ERRCODE_SDIO_REG_ADDR_INVALID                       0x80003252
#define ERRCODE_SDIO_ERR_OVERFLOW                           0x80003253
#define ERRCODE_SDIO_ERR_MSG_TO_HOST_NOT_READ               0x80003254
#define ERRCODE_SDIO_ERR_INIT_CARD_RDY_TIMEOUT              0x80003255
#define ERRCODE_SDIO_ERR_INIT_FUN1_RDY_TIMEOUT              0x80003256
#define ERRCODE_SDIO_ERR_STATUS                             0x80003257

/* Security Related Error. */
#define ERRCODE_SEC_KEY_GEN_INVALID_PARAM                   0x80003280
#define ERRCODE_SEC_IAMGE_ID_INVALID                        0x80003281

#define ERRCODE_SEC_IMAGE_MSID_INVALID                      0x80003290
#define ERRCODE_SEC_IMAGE_DIE_ID_INVALID                    0x80003291
#define ERRCODE_SEC_IMAGE_KEY_INVALID_PARAM                 0x80003292
#define ERRCODE_SEC_IMAGE_KEY_ID_INVALID                    0x80003293
#define ERRCODE_SEC_IMAGE_KEY_VERSION_INVALID               0x80003294
#define ERRCODE_SEC_IMAGE_KEY_AREA_VERIFY_FAILED            0x80003295
#define ERRCODE_SEC_IMAGE_KEY_LENGTH_INVALID                0x80003296

#define ERRCODE_SEC_IMAGE_INFO_INVALID_PARAM                0x800032a0
#define ERRCODE_SEC_IMAGE_INFO_ID_INVALID                   0x800032a1
#define ERRCODE_SEC_IMAGE_INFO_VERSION_INVALID              0x800032a2
#define ERRCODE_SEC_IMAGE_INFO_AREA_VERIFY_FAILED           0x800032a3

#define ERRCODE_SEC_IMAGE_CODE_INVALID_PARAM                0x800032b0
#define ERRCODE_SEC_IMAGE_CODE_CHECK_FAILED                 0x800032b1
#define ERRCODE_SEC_IMAGE_CODE_GET_HASH_FAILED              0x800032b2

/* SYS ctrl Related Error. */
#define ERRCODE_SYS_COMMU_INVALID_PARAM                     0x80003301
#define ERRCODE_SYS_COMMU_NO_RESOURCE                       0x80003302
/*
* 5 Wi - Fi (0x8000_4000 - 0x8000_5000)
*/

/*
* 6 AUDIO (0x8000_5000 - 0x8000_5800)
*/

/*
* 7 NFC (0x8000_5800 - 0x8000_6000)
*/

/*
* 8 BT (0x8000_6000 - 0x8000_7000)
*/

/*
* 9 DISPLAY (0x8000_7000 - 0x8000_8000)
*/

/*
* 10 PLC (0x8000_8000 - 0x8000_9000)
*/

/*
* 11 EPMU (0x8000_9000 - 0x8000_9400)
*/
#define ERRCODE_EPMU_CONFIG_ERR                             0x80009000
#define ERRCODE_EPMU_STAT_ERR                               0x80009001
#define ERRCODE_EPMU_NULL_PTR                               0x80009002
#define ERRCODE_EPMU_UNSUPPORT                              0x80009003
#define ERRCODE_EPMU_ADDR_ERR                               0x80009004
#define ERRCODE_EPMU_BUS_ERR                                0x80009005
#define ERRCODE_EPMU_TIMEOUT                                0x80009006
#define ERRCODE_EPMU_NV_ERR                                 0x80009007

/*
* 12 GLP (0x8000_9400 - 0x8000_9800)
*/

/*
* 13 RADAR (0x8000_9800 - 0x8000_9C00)
*/

/*
* 14 TIOT_HOST (0x8000_9C00 - 0x8000_A000)
*/

/*
* 15 GNSS (0x8000_A000 - 0x8000_A400)
*/


/**
 * @}
 */

#ifdef __cplusplus
#if __cplusplus
}

#endif /* __cplusplus */
#endif /* __cplusplus */

#endif
