/*
 * Copyright (c) CompanyNameMagicTag 2019-2020. All rights reserved.
 * Description: Common Error Definition.
 */

#ifndef SOC_ERRNO_H_
#define SOC_ERRNO_H_

typedef enum {
    /*
    * 1 COMMON ERR
    */
    EXT_ERR_SUCCESS                                  = 0,            /*!< succes */
    EXT_ERR_FAILURE                                  = 0xFFFFFFFF,   /*!< failure */

    EXT_SUCCESS                                      = EXT_ERR_SUCCESS,
    EXT_FAILURE                                      = EXT_ERR_FAILURE,

    EXT_SEC_SUCCESS                                  = 0x3CA5965A, /* Legacy name */
    EXT_SEC_FAILURE                                  = 0xC35A69A5, /* Legacy name */

    EXT_ERR_MALLOC_FAILURE                           = 0x80000001,   /*!< malloc fail,only malloc fail from mem pool
                                                                        can use this err code */
    EXT_ERR_TIMEOUT                                  = 0x80000002,   /*!< time out */
    EXT_ERR_MEMCPYS_FAIL                             = 0x80000003,
    EXT_ERR_MEMSETS_FAIL                             = 0x80000004,
    EXT_ERR_INSUFFICIENT_DATA                        = 0x80000007,
    EXT_ERR_INCORRECT_ID                             = 0x80000008,
    EXT_ERR_INCORRECT_VER                            = 0x80000009,
    EXT_ERR_BAD_CHECKSUM                             = 0x8000000A,
    EXT_ERR_BLOCKED                                  = 0x8000000B,
    EXT_ERR_WRONG_CORE                               = 0x8000000C,
    EXT_ERR_NOT_INITIALISED                          = 0x8000000D,
    EXT_ERR_INVALID_STATE                            = 0x8000000E,
    EXT_ERR_NOT_IMPLEMENTED                          = 0x8000000F,
    /*
    * 2 soc osa (0x8000_0080 - 0x8000_1000)
    */
    /* TASK */
    EXT_ERR_TASK_INVALID_PARAM                       = 0x80000081,
    EXT_ERR_TASK_CREATE_FAIL                         = 0x80000082,
    EXT_ERR_TASK_DELETE_FAIL                         = 0x80000083,
    EXT_ERR_TASK_SUPPEND_FAIL                        = 0x80000084,
    EXT_ERR_TASK_RESUME_FAIL                         = 0x80000085,
    EXT_ERR_TASK_GET_PRI_FAIL                        = 0x80000086,
    EXT_ERR_TASK_SET_PRI_FAIL                        = 0x80000087,
    EXT_ERR_TASK_LOCK_FAIL                           = 0x80000088,
    EXT_ERR_TASK_UNLOCK_FAIL                         = 0x80000089,
    EXT_ERR_TASK_DELAY_FAIL                          = 0x8000008A,
    EXT_ERR_TASK_GET_INFO_FAIL                       = 0x8000008B,
    EXT_ERR_TASK_REGISTER_SCHEDULE_FAIL              = 0x8000008C,
    EXT_ERR_TASK_NOT_CREATED                         = 0x8000008D,

    /* ISR */
    EXT_ERR_ISR_INVALID_PARAM                        = 0x800000C1,
    EXT_ERR_ISR_REQ_IRQ_FAIL                         = 0x800000C2,
    EXT_ERR_ISR_ADD_JOB_MALLOC_FAIL                  = 0x800000C3,
    EXT_ERR_ISR_ADD_JOB_SYS_FAIL                     = 0x800000C4,
    EXT_ERR_ISR_FREE_IRQ_FAIL                        = 0x800000C5,
    EXT_ERR_ISR_ALREADY_CREATED                      = 0x800000C6,
    EXT_ERR_ISR_NOT_CREATED                          = 0x800000C7,
    EXT_ERR_ISR_ENABLE_IRQ_FAIL                      = 0x800000C8,
    EXT_ERR_ISR_IRQ_ADDR_NOK                         = 0x800000C9,
    EXT_ERR_ISR_SET_PRI_FAIL                         = 0x800000CA,

    /* MEM */
    EXT_ERR_MEM_INVALID_PARAM                        = 0x80000101, /*!< invalid param return from mem module */
    EXT_ERR_MEM_CREAT_POOL_FAIL                      = 0x80000102,
    EXT_ERR_MEM_CREATE_POOL_NOT_ENOUGH_HANDLE        = 0x80000103,
    EXT_ERR_MEM_FREE_FAIL                            = 0x80000104,
    EXT_ERR_MEM_RE_INIT                              = 0x80000105,
    EXT_ERR_MEM_NOT_INIT                             = 0x80000106,
    EXT_ERR_MEM_CREAT_POOL_MALLOC_FAIL               = 0x80000107,
    EXT_ERR_MEM_GET_INFO_FAIL                        = 0x80000108,
    EXT_ERR_MEM_GET_OS_INFO_NOK                      = 0x80000109,
    EXT_ERR_MEM_PSRAM_INIT_ERROR                     = 0x8000010A,
    EXT_ERR_MEM_PSRAM_RE_INIT                        = 0x8000010B,
    EXT_ERR_MEM_PSRAM_NOT_INIT                       = 0x8000010C,
    EXT_ERR_MEM_PSRAM_FREE_ERROR                     = 0x8000010D,

    /* POOL_MEM */
    EXT_ERR_POOL_MEM_INIT_FAIL                       = 0x80000131,
    EXT_ERR_POOL_MEM_DEINIT_FAIL                     = 0x80000132,

    /* OS Timer */
    EXT_ERR_TIMER_FAILURE                            = 0x80000141,
    EXT_ERR_TIMER_INVALID_PARAM                      = 0x80000142,
    EXT_ERR_TIMER_CREATE_HANDLE_FAIL                 = 0x80000143,
    EXT_ERR_TIMER_START_FAIL                         = 0x80000144,
    EXT_ERR_TIMER_HANDLE_NOT_CREATE                  = 0x80000145,
    EXT_ERR_TIMER_HANDLE_INVALID                     = 0x80000146,
    EXT_ERR_TIMER_STATUS_INVALID                     = 0x80000147,
    EXT_ERR_TIMER_STATUS_START                       = 0x80000148,
    EXT_ERR_TIMER_INVALID_MODE                       = 0x80000149,
    EXT_ERR_TIMER_EXPIRE_INVALID                     = 0x8000014A,
    EXT_ERR_TIMER_FUNCTION_NULL                      = 0x8000014B,
    EXT_ERR_TIMER_HANDLE_MAXSIZE                     = 0x8000014C,
    EXT_ERR_TIMER_MALLOC_FAIL                        = 0x8000014D,
    EXT_ERR_TIMER_NOT_INIT                           = 0x8000014E,

    /* SEM */
    EXT_ERR_SEM_INVALID_PARAM                        = 0x80000181,
    EXT_ERR_SEM_CREATE_FAIL                          = 0x80000182,
    EXT_ERR_SEM_DELETE_FAIL                          = 0x80000183,
    EXT_ERR_SEM_WAIT_FAIL                            = 0x80000184,
    EXT_ERR_SEM_SIG_FAIL                             = 0x80000185,
    EXT_ERR_SEM_WAIT_TIME_OUT                        = 0x80000186,

    /* MUX */
    EXT_ERR_MUX_INVALID_PARAM                        = 0x800001C1,
    EXT_ERR_MUX_CREATE_FAIL                          = 0x800001C2,
    EXT_ERR_MUX_DELETE_FAIL                          = 0x800001C3,
    EXT_ERR_MUX_PEND_FAIL                            = 0x800001C4,
    EXT_ERR_MUX_POST_FAIL                            = 0x800001C5,

    /* MSG */
    EXT_ERR_MSG_INVALID_PARAM                        = 0x80000201,
    EXT_ERR_MSG_CREATE_Q_FAIL                        = 0x80000202,
    EXT_ERR_MSG_DELETE_Q_FAIL                        = 0x80000203,
    EXT_ERR_MSG_WAIT_FAIL                            = 0x80000204,
    EXT_ERR_MSG_SEND_FAIL                            = 0x80000205,
    EXT_ERR_MSG_GET_Q_INFO_FAIL                      = 0x80000206,
    EXT_ERR_MSG_Q_DELETE_FAIL                        = 0x80000207,
    EXT_ERR_MSG_WAIT_TIME_OUT                        = 0x80000208,
    EXT_ERR_MSG_QUEUE_ISFULL                         = 0x80000209,
    EXT_ERR_MSG_QUEUE_NOT_CREATE                     = 0x8000020A,

    /* EVENT */
    EXT_ERR_EVENT_INVALID_PARAM                      = 0x80000241,
    EXT_ERR_EVENT_CREATE_NO_HADNLE                   = 0x80000242,
    EXT_ERR_EVENT_CREATE_SYS_FAIL                    = 0x80000243,
    EXT_ERR_EVENT_SEND_FAIL                          = 0x80000244,
    EXT_ERR_EVENT_WAIT_FAIL                          = 0x80000245,
    EXT_ERR_EVENT_CLEAR_FAIL                         = 0x80000246,
    EXT_ERR_EVENT_RE_INIT                            = 0x80000247,
    EXT_ERR_EVENT_NOT_ENOUGH_MEMORY                  = 0x80000248,
    EXT_ERR_EVENT_NOT_INIT                           = 0x80000249,
    EXT_ERR_EVENT_DELETE_FAIL                        = 0x8000024A,
    EXT_ERR_EVENT_WAIT_TIME_OUT                      = 0x8000024B,
    EXT_ERR_EVENT_CREATE_NO_ENOUGH_HADNLE            = 0x8000024C,

    /* SPINLOCK */
    EXT_ERR_SPINLOCK_INVALID_PARAM                   = 0x80000251,
    EXT_ERR_SPINLOCK_NOT_ENOUGH_MEMORY               = 0x80000252,
    EXT_ERR_SPINLOCK_NOT_INIT                        = 0x80000253,

    /* OS Stat */
    EXT_ERR_OS_STAT_INVALID_PARAM                    = 0x80000281,
    EXT_ERR_OS_STAT_SYSTEM_CALL_ERROR                = 0x80000282,

    /* WAIT */
    EXT_ERR_WAIT_INVALID_PARAM                       = 0x80000291,
    EXT_ERR_WAIT_MALLOC_FAILURE                      = 0x80000292,

    /* km */
    EXT_ERR_KM_INVALID_PARAMETER                     = 0x80000300,
    EXT_ERR_KM_ENV_NOT_READY                         = 0x80000301,
    EXT_ERR_KM_KSLOT_BUSY                            = 0x80000302,
    EXT_ERR_KM_TIMEOUT                               = 0x80000303,
    EXT_ERR_KM_KLAD_BUSY                             = 0x80000304,
    EXT_ERR_KM_RKP_BUSY                              = 0x80000305,

    /* otp */
    EXT_ERR_OTP_INVALID_PARAMETER                    = 0x80000320,
    EXT_ERR_OTP_BUSY                                 = 0x80000321,
    EXT_ERR_OTP_ADDR_NOT_ALIGNED                     = 0x80000322,
    EXT_ERR_OTP_TIMEOUT                              = 0x80000323,
    EXT_ERR_OTP_PERMISSION_FAILURE                   = 0x80000324,

    /* cipher (block 13) */
    EXT_ERR_CIPHER_INVALID_PARAMETER                 = 0x80000340,
    EXT_ERR_CIPHER_UNSUPPORTED                       = 0x80000341,
    EXT_ERR_CIPHER_ENV_NOT_READY                     = 0x80000342,
    EXT_ERR_CIPHER_BUSY                              = 0x80000343,
    EXT_ERR_CIPHER_TIMEOUT                           = 0x80000344,
    EXT_ERR_CIPHER_FAILED_MEM                        = 0x80000345,
    EXT_ERR_CIPHER_ADDR_NOT_ALIGNED                  = 0x80000346,
    EXT_ERR_CIPHER_SIZE_NOT_ALIGNED                  = 0x80000347,
    EXT_ERR_CIPHER_OVERFLOW                          = 0x80000348,

    /* HASH */
    EXT_ERR_HASH_INVALID_PARAMETER                   = 0x80000350,
    EXT_ERR_HASH_UNSUPPORTED                         = 0x80000351,
    EXT_ERR_HASH_ADDR_NOT_ALIGNED                    = 0x80000352,
    EXT_ERR_HASH_SIZE_NOT_ALIGNED                    = 0x80000353,
    EXT_ERR_HASH_ENV_NOT_READY                       = 0x80000354,
    EXT_ERR_HASH_BUSY                                = 0x80000355,
    EXT_ERR_HASH_TIMEOUT                             = 0x80000356,
    EXT_ERR_HASH_FAILED_MEM                          = 0x80000357,
    EXT_ERR_HASH_OVERFLOW                            = 0x80000358,

    /* COMMON */
    EXT_ERR_INVALID_CPU                              = 0x80000360,
    EXT_ERR_FAPC_INVALID_PARAMETER                   = 0x80000361,
    EXT_ERR_RAPC_INVALID_PARAMETER                   = 0x80000362,

    /* PKE (block 14) */
    EXT_ERR_PKE_INVALID_PARAMETER                    = 0x80000380,
    EXT_ERR_PKE_ENV_NOT_READY                        = 0x80000381,
    EXT_ERR_PKE_TIMEOUT                              = 0x80000382,
    EXT_ERR_PKE_BUSY                                 = 0x80000383,
    EXT_ERR_PKE_UNSUPPORTED                          = 0x80000384,
    EXT_ERR_PKE_ROBUST_WARNING                       = 0x80000385,
    EXT_ERR_PKE_INVALID_PADDING                      = 0x80000386,
    EXT_ERR_PKE_BUF_TOO_SMALL                        = 0x80000387,

    /* File System */
    EXT_ERR_FS_INVALID_PARAM                         = 0x80000401,
    EXT_ERR_FS_NO_DEVICE                             = 0x80000402,
    EXT_ERR_FS_NO_SPACE                              = 0x80000403, /* No space left on device */
    EXT_ERR_FS_BAD_DESCRIPTOR                        = 0x80000404,
    EXT_ERR_FS_FILE_EXISTS                           = 0x80000405,
    EXT_ERR_FS_NOT_FOUND                             = 0x80000406,
    EXT_ERR_FS_NAME_TOO_LONG                         = 0x80000407,
    EXT_ERR_FS_READ_ONLY_FS                          = 0x80000408, /* Read-only file system */
    EXT_ERR_FS_IO_ERROR                              = 0x80000409,
    EXT_ERR_FS_NO_MORE_FILES                         = 0x8000040A,

    /*
    * 3 DRIVE (0x8000_1000 - 0x8000_3000)
    *
    * IO moudle
    */
    EXT_ERR_IO_NOT_INIT                              = 0x80001041,
    EXT_ERR_IO_INVALID_PARAMETER                     = 0x80001042,
    EXT_ERR_IO_RELEASE_ERROR                         = 0x80001043,
    EXT_ERR_IO_SET_FUNC_ERROR                        = 0x80001044,
    EXT_ERR_IO_GET_FUNC_ERROR                        = 0x80001045,
    EXT_ERR_IO_GET_PULL_ERROR                        = 0x80001046,
    EXT_ERR_IO_GET_STRENGTH_ERROR                    = 0x80001047,
    EXT_ERR_IO_GET_SLEW_ERROR                        = 0x80001048,
    EXT_ERR_IO_GET_TRIGGER_ERROR                     = 0x80001049,
    EXT_ERR_IO_FAILURE                               = 0x8000104A,

    /* GPIO moudle */
    EXT_ERR_GPIO_NOT_INIT                            = 0x80001081,
    EXT_ERR_GPIO_INVALID_PARAMETER                   = 0x80001082,
    EXT_ERR_GPIO_PIN_ERROR                           = 0x80001083,
    EXT_ERR_GPIO_DIR_ERROR                           = 0x80001084,
    EXT_ERR_GPIO_FAILURE                             = 0x80001085,

    /* RTC moudle */
    EXT_ERR_RTC_NOT_INIT                             = 0x80001091,
    EXT_ERR_RTC_INVALID_PARAMETER                    = 0x80001092,
    EXT_ERR_RTC_NO_VALID                             = 0x80001093,
    EXT_ERR_RTC_FAILURE                              = 0x80001094,

    /* CALENDAR moudle */
    EXT_ERR_CALENDAR_NOT_INIT						= 0x80001095,
    EXT_ERR_CALENDAR_INVALID_PARAMETER				= 0x80001096,
    EXT_ERR_CALENDAR_NO_VALID						= 0x80001097,
    EXT_ERR_CALENDAR_FAILURE 						= 0x80001098,

    /* UART moudle */
    EXT_ERR_UART_INVALID_PARAMETER                   = 0x800010C1,
    EXT_ERR_UART_INVALID_SUSPEND                     = 0x800010C2,
    EXT_ERR_UART_INVALID_PARITY                      = 0x800010C3,
    EXT_ERR_UART_INVALID_DATA_BITS                   = 0x800010C4,
    EXT_ERR_UART_INVALID_STOP_BITS                   = 0x800010C5,
    EXT_ERR_UART_INVALID_BAUD                        = 0x800010C6,
    EXT_ERR_UART_INVALID_COM_PORT                    = 0x800010C7,
    EXT_ERR_UART_SW_BUF_CREATE_FAIL                  = 0x800010C8,
    EXT_ERR_UART_NOT_SUPPORT                         = 0x800010C9,
    EXT_ERR_UART_INVALID_ID                          = 0x800010CA,
    EXT_ERR_UART_CREATE_EVENT_FAIL                   = 0x800010CB,
    EXT_ERR_UART_OPEN_TX_DMA_CHN_FAIL                = 0x800010CC,
    EXT_ERR_UART_CLOSE_TX_DMA_CHN_FAIL               = 0x800010CD,
    EXT_ERR_UART_TX_DMA_NOT_ENABLED                  = 0x800010CE,
    EXT_ERR_UART_CFG_NOT_SUPPORT_NONE_FIFO           = 0x800010CF,
    EXT_ERR_UART_DRIVER_RESOURCE_REINIT              = 0x800010D1,   /*!< uart driver resource have initialized */
    EXT_ERR_UART_DEVICE_RESOURCE_REINIT              = 0x800010D2,   /*!< uart device resource have initialized */
    EXT_ERR_UART_INVALID_FLOW_CTRL                   = 0x800010D3,
    EXT_ERR_UART_BUF_DEINIT_FAIL                     = 0x800010D4,
    EXT_ERR_UART_DEINIT_WRITTING_READING             = 0x800010D5,
    EXT_ERR_UART_SEND_XON_FAIL                       = 0x800010D6,
    EXT_ERR_UART_SEND_XOFF_FAIL                      = 0x800010D7,
    EXT_ERR_UART_MEM_OPT_FAIL                        = 0x800010D8,
    EXT_ERR_UART_RX_BUF_NOT_CREATED                  = 0x800010D9,
    EXT_ERR_UART_ID_INIT_ALREADY                     = 0x800010DA,
    EXT_ERR_UART_ID_NOT_INIT                         = 0x800010DB,

    /* SPI moudle */
    EXT_ERR_SPI_NOT_INIT                             = 0x80001101,
    EXT_ERR_SPI_INVALID_PARAMETER                    = 0x80001102,
    EXT_ERR_SPI_PIN_ERROR                            = 0x80001103,
    EXT_ERR_SPI_BUSY                                 = 0x80001104,
    EXT_ERR_SPI_FAILURE                              = 0x80001105,
    EXT_ERR_SPI_ALREADY_INIT                         = 0x80001106,
    EXT_ERR_SPI_WRITE_TIMEOUT                        = 0x80001107,
    EXT_ERR_SPI_READ_TIMEOUT                         = 0x80001108,
    EXT_ERR_SPI_MODE_ERROR                           = 0x80001109,
    EXT_ERR_SPI_DMA_OPEN_FAILURE                     = 0x8000110A,
    EXT_ERR_SPI_DMA_TRANSFER_FAILURE                 = 0x8000110B,
    EXT_ERR_SPI_CORE_ERROR                           = 0x8000110C,
    EXT_ERR_SPI_NOT_SUPPORT_DMA                      = 0x8000110D,
    EXT_ERR_SPI_NOT_SUPPORT_CRC                      = 0x8000110E,
    /* efuse */
    EXT_ERR_EFUSE_INVALIDATE_ID                      = 0x80001121,
    EXT_ERR_EFUSE_INVALIDATE_PARA                    = 0x80001122,
    EXT_ERR_EFUSE_WRITE_ERR                          = 0x80001123,
    EXT_ERR_EFUSE_INVALIDATE_AUTH                    = 0x80001124,
    EXT_ERR_EFUSE_BUSY                               = 0x80001125,
    EXT_ERR_EFUSE_TIMEOUT                            = 0x80001126,
    EXT_ERR_EFUSE_STATE_ADDR                         = 0x80001127,
    EXT_ERR_EFUSE_WRITE_ALL_ZERO                     = 0x80001128,

    /* I2C moudle */
    EXT_ERR_I2C_NOT_INIT                             = 0x80001141,
    EXT_ERR_I2C_INVALID_PARAMETER                    = 0x80001142,
    EXT_ERR_I2C_PIN_ERROR                            = 0x80001143,
    EXT_ERR_I2C_BUSY_ERROR                           = 0x80001144,
    EXT_ERR_I2C_SEND_DATA_ERROR                      = 0x80001145,
    EXT_ERR_I2C_RECEIVE_DATA_ERROR                   = 0x80001146,
    EXT_ERR_I2C_ACK_ERROR                            = 0x80001147,
    EXT_ERR_I2C_ARB_ERROR                            = 0x80001148,
    EXT_ERR_I2C_TIMEOUT_ERROR                        = 0x80001149,
    EXT_ERR_I2C_FAILURE                              = 0x8000114A,
    EXT_ERR_I2C_TIMEOUT_START                        = 0x8000114B,
    EXT_ERR_I2C_TIMEOUT_WAIT                         = 0x8000114C,
    EXT_ERR_I2C_TIMEOUT_STOP                         = 0x8000114F,
    EXT_ERR_I2C_TIMEOUT_RCV_BYTE                     = 0x80001150,
    EXT_ERR_I2C_TIMEOUT_RCV_BYTE_PROC                = 0x80001151,
    EXT_ERR_I2C_WAIT_SEM_FAIL                        = 0x80001152,
    EXT_ERR_I2C_START_ACK_ERR                        = 0x80001153,
    EXT_ERR_I2C_WAIT_ACK_ERR                         = 0x80001154,
    EXT_ERR_I2C_INITED                               = 0x80001155,
    EXT_ERR_I2C_ARBITRATE_ERR                        = 0x80001156,
    EXT_ERR_I2C_NO_POWER_SUPPLY                      = 0x80001157,

    /* i2s moudle */
    EXT_ERR_I2S_INVALID_PARAMETER                    = 0x80001161,
    EXT_ERR_I2S_WRITE_TIMEOUT                        = 0x80001162,
    EXT_ERR_I2S_MALLOC_FAIL                          = 0x80001163,
    EXT_ERR_I2S_MEMCPY_FAIL                          = 0x80001164,

    /* PWM moudle */
    EXT_ERR_PWM_NOT_INIT                             = 0x80001181,
    EXT_ERR_PWM_INVALID_PARAMETER                    = 0x80001182,
    EXT_ERR_PWM_PIN_ERROR                            = 0x80001183,
    EXT_ERR_PWM_FAILURE                              = 0x80001184,
    EXT_ERR_PWM_UNSUPPORT_DUTY_RATIO                 = 0x80001185,
    EXT_ERR_PWM_INIT_ALREADY                         = 0x80001186,

    /* Flash moudule */
    EXT_ERR_FLASH_NOT_INIT                           = 0x800011C1,
    EXT_ERR_FLASH_INVALID_PARAMETER                  = 0x800011C2,
    EXT_ERR_FLASH_INVALID_PARAM_BEYOND_ADDR          = 0x800011C3,
    EXT_ERR_FLASH_INVALID_PARAM_SIZE_ZERO            = 0x800011C4,
    EXT_ERR_FLASH_INVALID_PARAM_ERASE_NOT_ALIGN      = 0x800011C5,
    EXT_ERR_FLASH_INVALID_PARAM_IOCTRL_DATA_NULL     = 0x800011C6,
    EXT_ERR_FLASH_INVALID_PARAM_DATA_NULL            = 0x800011C7,
    EXT_ERR_FLASH_INVALID_PARAM_PKI_MODIFY           = 0x800011C8,
    EXT_ERR_FLASH_INVALID_PARAM_PAD1                 = 0x800011C9,
    EXT_ERR_FLASH_INVALID_PARAM_PAD2                 = 0x800011CA,
    EXT_ERR_FLASH_INVALID_PARAM_PAD3                 = 0x800011CB,
    EXT_ERR_FLASH_TIME_OUT_WAIT_READY                = 0x800011CC,
    EXT_ERR_FLASH_QUAD_MODE_READ_REG1                = 0x800011CD,
    EXT_ERR_FLASH_QUAD_MODE_READ_REG2                = 0x800011CE,
    EXT_ERR_FLASH_QUAD_MODE_COMPARE_REG              = 0x800011CF,
    EXT_ERR_FLASH_NO_MATCH_FLASH                     = 0x800011D0,
    EXT_ERR_FLASH_WRITE_ENABLE                       = 0x800011D1,
    EXT_ERR_FLASH_NO_MATCH_ERASE_SIZE                = 0x800011D2,
    EXT_ERR_FLASH_MAX_SPI_OP                         = 0x800011D3,
    EXT_ERR_FLASH_NOT_SUPPORT_IOCTRL_ID              = 0x800011D4,
    EXT_ERR_FLASH_INVALID_CHIP_ID                    = 0x800011D5,
    EXT_ERR_FLASH_RE_INIT                            = 0x800011D6,
    EXT_ERR_FLASH_WRITE_NOT_SUPPORT_ERASE            = 0x800011D7,
    EXT_ERR_FLASH_WRITE_COMPARE_WRONG                = 0x800011D8,
    EXT_ERR_FLASH_WAIT_CFG_START_TIME_OUT            = 0x800011D9,
    EXT_ERR_FLASH_PATITION_INIT_FAIL                 = 0x800011DA,
    EXT_ERR_FLASH_INITILIZATION                      = 0x800011DB,
    EXT_ERR_FLASH_ERASE_NOT_4K_ALIGN                 = 0x800011DC,
    EXT_ERR_FLASH_RPC_COMMAND                        = 0x800011DD,
    EXT_ERR_FLASH_ADD_WRITE_TASK                     = 0x800011DE,
    EXT_ERR_FLASH_ADD_ERASE_TASK                     = 0x800011DF,
    EXT_ERR_FLASH_WRITE_TASK                         = 0x800011E0,
    EXT_ERR_FLASH_ERASE_TASK                         = 0x800011E1,
    EXT_ERR_FLASH_RPC_READ                           = 0x800011E2,
    EXT_ERR_FLASH_INVALID_CMD                        = 0x800011E3,
    EXT_ERR_FLASH_INVALID_SR_BIT                     = 0x800011E4,

    /* dma */
    EXT_ERR_DMA_INVALID_PARA                         = 0x80001201,
    EXT_ERR_DMA_NOT_INIT                             = 0x80001202,
    EXT_ERR_DMA_BUSY                                 = 0x80001203,
    EXT_ERR_DMA_TRANSFER_FAIL                        = 0x80001204,
    EXT_ERR_DMA_TRANSFER_TIMEOUT                     = 0x80001205,
    EXT_ERR_DMA_GET_NOTE_FAIL                        = 0x80001206,
    EXT_ERR_DMA_LLI_NOT_CREATE                       = 0x80001207,
    EXT_ERR_DMA_NO_AVAIL_CONTROLLER                  = 0x80001208,
    EXT_ERR_DMA_NO_AVAIL_CH                          = 0x80001209,
    EXT_ERR_DMA_UNSUPPORTED_PERIPHERAL               = 0x8000120A,
    EXT_ERR_DMA_NOT_SUPPORTED_CONFIG                 = 0x8000120B,
    EXT_ERR_DMA_ERROR_CONFIG                         = 0x8000120C,
    EXT_ERR_DMA_INCOMPLETE_CONFIG                    = 0x8000120D,
    EXT_ERR_DMA_ONE_TIME_BATCHED_CONFIGS_EXCEEDED    = 0x8000120E,
    EXT_ERR_DMA_CONFIG_QUEUE_FULL                    = 0x8000120F,
    EXT_ERR_DMA_OUT_OF_MEMORY                        = 0x80001210,
    EXT_ERR_DMA_NO_AVAIL_FREE_CONFIG                 = 0x80001211,
    EXT_ERR_DMA_CHANNEL_NOT_OPEN                     = 0x80001212,
    EXT_ERR_DMA_IRQ_ENABLE_FAIL                      = 0x80001213,
    EXT_ERR_DMA_LLI_NOTE_FULL                        = 0x80001214,
    EXT_ERR_DMA_SRC_PERIPHERAL                       = 0x80001215,
    EXT_ERR_DMA_DST_PERIPHERAL                       = 0x80001216,
    EXT_ERR_DMA_PERIPHERAL                           = 0x80001217,
    EXT_ERR_DMA_CHANNEL                              = 0x80001218,
    EXT_ERR_DMA_DST_WIDTH                            = 0x80001219,
    EXT_ERR_DMA_ADDRESS                              = 0x8000121A,
    EXT_ERR_DMA_PERIPHERAL_CONFIG                    = 0x8000121B,
    EXT_ERR_DMA_LLI_ADDRESS                          = 0x8000121C,
    EXT_ERR_DMA_DEVICE_NOT_INIT                      = 0x8000121D,
    EXT_ERR_DMA_TRANSFER_TYPE                        = 0x8000121E,

    /* WDT */
    EXT_ERR_WDT_PARA_ERROR                           = 0x80001241,
    EXT_ERR_WDT_INTERVAL_INVALID                     = 0x80001242,
    EXT_ERR_WDT_IRQ_ENABLE_FAILED                    = 0x80001243,

    /* HWI */
    EXT_ERR_HWI_INVALID_NUM                          = 0x80001281,

    /* timer */
    EXT_ERR_TIMER_NO_VALID_TIMER                     = 0x800012C1,
    EXT_ERR_TIMER_INVALID_PARAMETER                  = 0x800012C2,

    /* hw_copy */
    EXT_ERR_HW_COPY_INVALID_PARA                     = 0x80001301,
    EXT_ERR_HW_COPY_BUSY                             = 0x80001302,

    /* adc */
    EXT_ERR_ADC_PARAMETER_WRONG                      = 0x80001311,
    EXT_ERR_ADC_INVALID_CHANNEL_ID                   = 0x80001312,
    EXT_ERR_ADC_TIMEOUT                              = 0x80001313,
    EXT_ERR_ADC_NOT_INIT                             = 0x80001314,

    /* swi */
    EXT_ERR_SWI_INVALID_ID                           = 0x80001341,

    /* ANALOG */
    EXT_ERR_ANALOG_MUTEX_ERROR                       = 0x80001381,
    EXT_ERR_ANALOG_RESOURCE_BUSY                     = 0x80001382,
    EXT_ERR_ANALOG_RESOURCE_CONNECT_ERROR            = 0x80001383,
    EXT_ERR_ANALOG_RESOURCE_FREE_ERROR               = 0x80001384,
    EXT_ERR_ANALOG_VOL_TYPE_ERROR                    = 0x80001385,
    EXT_ERR_ANALOG_READ_VOL_ERROR                    = 0x80001386,
    EXT_ERR_ANALOG_AIO_PIN_ERROR                     = 0x80001387,
    EXT_ERR_ANALOG_IDRV_INVALID_RANGE                = 0x80001388,
    EXT_ERR_ANALOG_COMP_NUMBER_ERROR                 = 0x80001389,
    EXT_ERR_ANALOG_COMP_INVALID_RANGE                = 0x8000138A,
    EXT_ERR_ANALOG_VALUE_SET_ERROR                   = 0x8000138B,
    EXT_ERR_ANALOG_CORE_NOT_SUPPORT                  = 0x8000138C,
    EXT_ERR_ANALOG_PARAMETER_ERROR                   = 0x8000138D,
    EXT_ERR_ANALOG_FUNC_UNREADY_ERROR                = 0x8000138E,
    EXT_ERR_ANALOG_ADC_TIME_OUT_ERROR                = 0x8000138F,
    EXT_ERR_ANALOG_ADC_NOT_CALIBRATE_ERROR           = 0x80001391,
    EXT_ERR_ANALOG_RESOURCE_NOT_OWNED_ERROR          = 0x80001392,
    EXT_ERR_ANALOG_COMP_NOT_SET_AIO_PIN              = 0x80001393,
    EXT_ERR_ANALOG_OTP_VALUE_GET_ERROR               = 0x80001394,
    EXT_ERR_ANALOG_AIO_FUNC_ERROR                    = 0x80001395,

    /* YMODEM */
    EXT_ERR_YMODEM_ERR_TIMEOUT                       = 0x80001401,
    EXT_ERR_YMODEM_ERR_CANCEL                        = 0x80001402,
    EXT_ERR_YMODEM_ERR_FRAME                         = 0x80001403,
    EXT_ERR_YMODEM_ERR_CS                            = 0x80001404,
    EXT_ERR_YMODEM_ERR_EOT                           = 0x80001405,
    EXT_ERR_YMODEM_ERR_SEQ                           = 0x80001406,
    EXT_ERR_YMODEM_ERR_LENTH                         = 0x80001407,

    /* upgrade common error */
    EXT_ERR_UPG_NOT_INIT                             = 0x80001501,
    EXT_ERR_UPG_INVALID_PARAMETER                    = 0x80001502,
    EXT_ERR_UPG_WRONG_IMAGE_NUM                      = 0x80001503,
    EXT_ERR_UPG_INVALID_IMAGE_ID                     = 0x80001504,
    EXT_ERR_UPG_NULL_POINTER                         = 0x80001505,
    EXT_ERR_UPG_NOT_SUPPORTED                        = 0x80001506,
    EXT_ERR_UPG_NOT_NEED_TO_UPDATE                   = 0x80001507,
    EXT_ERR_UPG_FLASH_ERASE_ERROR                    = 0x80001508,
    EXT_ERR_UPG_FLASH_WRITE_ERROR                    = 0x80001509,
    EXT_ERR_UPG_FLAG_NOT_INITED                      = 0x8000150A,
    EXT_ERR_UPG_NO_ENOUGH_SPACE                      = 0x8000150B,
    EXT_ERR_UPG_PARTITION_ALIGN_ERROR                = 0x8000150C,
    EXT_ERR_UPG_INVALID_PREPARE_INFO                 = 0x8000150D,
    EXT_ERR_UPG_NOT_PREPARED                         = 0x8000150E,
    EXT_ERR_UPG_INVALID_BUFF_LEN                     = 0x8000150F,
    EXT_ERR_UPG_INVALID_OFFSET                       = 0x80001510,
    EXT_ERR_UPG_FILE_OPEN_FAIL                       = 0x80001511,
    EXT_ERR_UPG_FILE_WRITE_FAIL                      = 0x80001512,
    EXT_ERR_UPG_FILE_READ_FAIL                       = 0x80001513,
    EXT_ERR_UPG_EMPTY_FILE                           = 0x80001514,
    EXT_ERR_UPG_DECOMPRESS_FAIL                      = 0x80001515,
    EXT_ERR_UPG_VERIFICATION_KEY_ERROR               = 0x80001516,
    EXT_ERR_UPG_ALREADY_INIT                         = 0x80001517,

    /* IPC */
    EXT_ERR_IPC_MSG_FULL                             = 0x80001541,
    EXT_ERR_IPC_MSG_ADDR_NULL                        = 0x80001542,
    EXT_ERR_IPC_MSG_ZEROCOPY_WRONG                   = 0x80001543,
    EXT_ERR_IPC_MSG_ALREADY_INIT                     = 0x80001544,
    EXT_ERR_IPC_MSG_TAKE_SEM_FAIL                    = 0x80001545,
    EXT_ERR_IPC_MSG_SEND_BUSY                        = 0x80001546,
    EXT_ERR_IPC_MSG_GIVE_SEM_FAIL                    = 0x80001547,
    EXT_ERR_IPC_MSG_INVALID_PARAM                    = 0x80001548,
    EXT_ERR_IPC_MSG_PRI_QUEUE_FULL                   = 0x80001549,
    EXT_ERR_IPC_MSG_PRI_QUEUE_EMPTY                  = 0x8000154A,
    EXT_ERR_IPC_MSG_RECV_INVALID_ID                  = 0x8000154B,
    EXT_ERR_IPC_MSG_PROCESS_FAILED                   = 0x8000154C,
    EXT_ERR_IPC_MSG_MEM_QUEUE_EMPTY                  = 0x8000154D,
    EXT_ERR_IPC_MSG_INIT_ALREADY                     = 0x8000154E,
    EXT_ERR_IPC_QUEUE_INVALID_ID                     = 0x8000154F,
    EXT_ERR_IPC_QUEUE_ADDR_NULL                      = 0x80001550,
    EXT_ERR_IPC_QUEUE_FULL                           = 0x80001551,
    EXT_ERR_IPC_QUEUE_EMPTY                          = 0x80001552,
    EXT_ERR_IPC_QUEUE_NOT_INIT                       = 0x80001553,
    EXT_ERR_IPC_SHM_REPEATE_RELEASE                  = 0x80001554,
    EXT_ERR_IPC_SHM_INVALID_ID                       = 0x80001555,
    EXT_ERR_IPC_SHM_NULL_POITER                      = 0x80001556,
    EXT_ERR_IPC_SHM_ILLEGAL_POINTER                  = 0x80001557,
    EXT_ERR_IPC_SHM_INVALID_POINTER                  = 0x80001558,
    EXT_ERR_IPC_SHM_NOT_INIT                         = 0x80001559,
    EXT_ERR_IPC_MEMCPY_EXCEPTION                     = 0x8000155A,
    EXT_ERR_IPC_MBX_BUSY                             = 0x8000155B,

    /* MIPI TX1 */
    EXT_ERR_MIPI_NOT_INIT                            = 0x80001581,
    EXT_ERR_MIPI_IRQ_REQUEST_FAILED                  = 0x80001582,
    EXT_ERR_MIPI_IRQ_ENABLE_FAILED                   = 0x80001583,
    EXT_ERR_MIPI_CREATE_MUX_FAIL                     = 0x80001584,
    EXT_ERR_MIPI_PEND_MUX_FAIL                       = 0x80001585,
    EXT_ERR_MIPI_NULL_PARAM                          = 0x80001586,
    EXT_ERR_MIPI_INVALID_CMD_INFO                    = 0x80001587,
    EXT_ERR_MIPI_MALLOC_FAIL                         = 0x80001588,
    EXT_ERR_MIPI_MEMCPY_FAIL                         = 0x80001589,
    EXT_ERR_MIPI_INCALID_IOCTL_CMD                   = 0x8000158A,
    EXT_ERR_MIPI_INVALID_RD_CMD_INFO                 = 0x8000158B,
    EXT_ERR_MIPI_WAIT_RD_FIFO_EMPTY                  = 0x8000158C,
    EXT_ERR_MIPI_GET_DATA_NULL                       = 0x8000158D,
    EXT_ERR_MIPI_SEND_SHORT_PACKET                   = 0x8000158E,
    EXT_ERR_MIPI_READ_FIFO_DATA                      = 0x8000158F,
    EXT_ERR_MIPI_COMBO_DEV_NOT_CONFIG                = 0x80001590,
    EXT_ERR_MIPI_DEV_ALREADY_ENABLE                  = 0x80001591,
    EXT_ERR_MIPI_DEV_NOT_ENABLE                      = 0x80001592,

    /* MIPI TX2 */
    EXT_ERR_MIPI_ALREADY_INIT                        = 0x800015C1,
    EXT_ERR_MIPI_INVALID_COLORBAR_ORIEN              = 0x800015C2,
    EXT_ERR_MIPI_INVALID_COLORBAR_INPUT_MODE         = 0x800015C3,

    /* SDIO */
    EXT_ERR_SDIO_INVALID_PARAMETER                   = 0x80001600,
    EXT_ERR_SDIO_ALREADY_START                       = 0x80001601,
    EXT_ERR_SDIO_PROC_TASK_INIT_FAIL                 = 0x80001602,

    /* SDCC */
    EXT_ERR_SDCC_INVALID_PARAMETER                   = 0x80001610,
    EXT_ERR_SDCC_ALREADY_START                       = 0x80001611,
    EXT_ERR_SDCC_PROC_TASK_INIT_FAIL                 = 0x80001612,

    /* QSPI */
    EXT_ERR_QSPI_INVALID_PARAM                       = 0x80001640,
    EXT_ERR_QSPI_INVALID_PARAM_BEYOND_ADDR           = 0x80001641,
    EXT_ERR_QSPI_INVALID_PARAM_SIZE_ZERO             = 0x80001642,
    EXT_ERR_QSPI_WRITE_COMPARE_WRONG                 = 0x80001643,
    EXT_ERR_QSPI_RE_INIT                             = 0x80001644,
    EXT_ERR_QSPI_INVALID_PARAM_DATA_NULL             = 0x80001645,
    EXT_ERR_QSPI_INVALID_CMD_DUMMY_BYTE              = 0x80001646,
    EXT_ERR_QSPI_INVALID_CMD_SPI_IFTYPE              = 0x80001647,
    EXT_ERR_QSPI_INVALID_CMD_DATA_SIZE               = 0x80001648,
    EXT_ERR_QSPI_INVALID_BUS_RECV_DATA_SIZE          = 0x80001649,
    EXT_ERR_QSPI_INVALID_BUS_RECV_DUMMY_BYTE         = 0x8000164A,
    EXT_ERR_QSPI_INVALID_BUS_RECV_SPI_IFTYPE         = 0x8000164B,
    EXT_ERR_QSPI_INVALID_BUS_TRANS_DATA_SIZE         = 0x8000164C,
    EXT_ERR_QSPI_INVALID_BUS_TRANS_DUMMY_BYTE        = 0x8000164D,
    EXT_ERR_QSPI_INVALID_BUS_TRANS_SPI_IFTYPE        = 0x8000164E,
    EXT_ERR_QSPI_INVALID_BUS_MAP_SIZE                = 0x8000164F,
    EXT_ERR_QSPI_INVALID_GLOBAL_SPI_MODE             = 0x80001650,
    EXT_ERR_QSPI_NOT_INIT                            = 0x80001651,

     /* HW TIMER */
    EXT_ERR_HWTIMER_NO_INIT                          = 0x80001680,
    EXT_ERR_HWTIMER_INVALID_PARAMETER                = 0x80001681,
    EXT_ERR_HWTIMER_INITILIZATION_ALREADY            = 0x80001682,

    /* ETH */
    EXT_ERR_ETH_INVALID_PARAMETER                    = 0x800016C0,

    /* DNTC */
    EXT_ERR_DNTC_INVALID_PARAMETER                   = 0x80001700,
    EXT_ERR_DNTC_INVALID_PARAMETER_PINNUM            = 0x80001701,
    EXT_ERR_DNTC_INVALID_PARAMETER_TEMPCODE          = 0x80001702,
    EXT_ERR_DNTC_INIT_ALREADY                        = 0x80001703,
    EXT_ERR_DNTC_INIT_FAILURE                        = 0x80001704,
    EXT_ERR_DNTC_INIT_NULL                           = 0x80001705,
    EXT_ERR_DNTC_DEINIT_ALREADY                      = 0x80001706,
    EXT_ERR_DNTC_NOT_INIT                            = 0x80001707,
    EXT_ERR_DNTC_PDOWN_FAILURE                       = 0x80001708,
    EXT_ERR_DNTC_REGISTER_ALREADY                    = 0x80001709,
    EXT_ERR_DNTC_READ_FAILURE                        = 0x8000170A,

    /* TSENSOR */
    EXT_ERR_TSENSOR_INVALID_PARAMETER                = 0x80001740,
    EXT_ERR_TSENSOR_NOT_INIT                         = 0x80001741,

    /* CLK */
    EXT_ERR_CLK_INVALID_PARAMETER                    = 0x80001780,
    EXT_ERR_CLK_NOT_REGISTER                         = 0x80001781,

    /* memory */
    EXT_ERR_MEMORY_INVALID_REGION                    = 0x80001A01,
    EXT_ERR_MEMORY_INVALID_PARAMETER                 = 0x80001A02,
    EXT_ERR_MEMORY_NOT_ALIGNED                       = 0x80001A03,

    /* devmng */
    EXT_ERR_DEVMNG_NOT_REGESTER                      = 0x80001C01,
    EXT_ERR_DEVMNG_INVALID_CMD                       = 0x80001C02,

    /**
     * This is a flash task state set by the owning core to indicate that this entry should be discarded by the SCPU.
     * The owning core must not simply delete an entry as the SCPU may have already taken a reference to the memory and
     * be periodically checking that for READY to be set.  Value must remain identical to the flash_task_etypes.h
     * value.
     */
    EXT_ERR_FLASH_TASK_DISCARD                       = 0x80002040,
    /**
     * This is a flash task state set by the SCPU to confirm that the task entry was discarded by the SCPU (either
     * because the owning core specified a DISCARD or because the value was illegal according to the SCPU) to indicate
     * that the flash task was skipped and not executed.  Value must remain identical to the flash_task_etypes.h value.
     */
    EXT_ERR_FLASH_TASK_DISCARDED                     = 0x80002041,
    /**
     * This is a flash task state set by the owning core to inform the SCPU that this flash task can now be fully
     * examined and executed by the SCPU.  Value must remain identical to the flash_task_etypes.h value.
     */
    EXT_ERR_FLASH_TASK_READY                         = 0x80002042,
    /**
     * This is a flash task state set by the SCPU to record that this flash task is being worked on by the SCPU. The
     * owning core will only see this if it wakes up unexpectedly before the anticipated wake-up time set in the sleep
     * co-ordination system.  Value must remain identical to the flash_task_etypes.h value.
     */
    EXT_ERR_FLASH_TASK_BEING_PROCESSED               = 0x80002043,
    /**
     * This is a flash task state set by the SCPU to indicate that the flash task has been completed and the SCPU has
     * dropped all references to the flash task so that the owning core may reclaim the memory.  Value must remain
     * identical to the flash_task_etypes.h value.
     */
    EXT_ERR_FLASH_TASK_COMPLETED                     = 0x80002044,
    /**
     * The flash task struct has an incorrect identifier number used as a sanity check that this memory is what it is
     * expected to be.
     */
    EXT_ERR_FLASH_TASK_WRONG_STRUCT_IDENTIFIER       = 0x80002045,
    /** There are no more flash tasks to process */
    EXT_ERR_FLASH_TASK_NO_MORE_TASKS                 = 0x80002046,
    /** There are only tasks that have not completed */
    EXT_ERR_FLASH_TASK_ONLY_PENDING_TASKS            = 0x80002047,
    /** The task is in progress */
    EXT_ERR_FLASH_TASK_IN_PROGRESS                   = 0x80002048,
    /** The specified task is not found */
    EXT_ERR_FLASH_TASK_NOT_FOUND                     = 0x80002049,
    /** The operation has already been done */
    EXT_ERR_FLASH_TASK_ALREADY_DONE                  = 0x8000204A,
    /** The queue handle was requested */
    EXT_ERR_FLASH_TASK_REQUESTED_QUEUE               = 0x8000204B,
    /** The remote core is not ready for this request */
    EXT_ERR_FLASH_TASK_REMOTE_CORE_NOT_READY         = 0x8000204C,
    /** Invalid protection key */
    EXT_ERR_FLASH_TASK_INVALID_PK                    = 0x8000204D,
    EXT_ERR_FLASH_TASK_INIT_FAILED                   = 0x8000204E,
    EXT_ERR_FLASH_TASK_PE_VETO                       = 0x8000204F,
    EXT_ERR_FLASH_TASK_BUSY                          = 0x80002050,

    /* NEW PWM */
    EXT_ERR_NEW_PWM_INVALID_PARAMETER                = 0x80002080,

    /* CORE MUX */
    EXT_ERR_CORE_MUX_NOT_CREATED                     = 0x800020C0,
    EXT_ERR_CORE_MUX_NOT_ENOUGH                      = 0x800020C1,
    EXT_ERR_CORE_MUX_INVALID_ID                      = 0x800020C2,
    EXT_ERR_CORE_MUX_BUSY                            = 0x800020C3,

    /*
    * 4 Component (0x8000_3000 - 0x8000_4000)
    */
    /* NV */
    EXT_ERR_NV_NOT_INITIALISED                       = 0x80003000,
    EXT_ERR_NV_INVALID_PARAMS                        = 0x80003001,
    EXT_ERR_NV_ERROR_ERASING_PAGE                    = 0x80003002,
    EXT_ERR_NV_NO_ENOUGH_SPACE                       = 0x80003003,
    EXT_ERR_NV_KEY_NOT_FOUND                         = 0x80003004,
    EXT_ERR_NV_TRYING_TO_MODIFY_A_PERMANENT_KEY      = 0x80003005,
    EXT_ERR_NV_WRITE_VETOED                          = 0x80003006,
    EXT_ERR_NV_DEFRAGMENTATION_NEEDED                = 0x80003007,
    EXT_ERR_NV_INVALID_STORE                         = 0x80003008,
    EXT_ERR_NV_WRITE_BUFFER_NOT_ALLOCATED            = 0x80003009,
    EXT_ERR_NV_KEY_HEADER_BUFFER_NOT_ALLOCATED       = 0x8000300A,
    EXT_ERR_NV_KEY_DATA_BUFFER_NOT_ALLOCATED         = 0x8000300B,
    EXT_ERR_NV_KEY_HASH_BUFFER_NOT_ALLOCATED         = 0x8000300C,
    EXT_ERR_NV_WRITE_BUFFER_TOO_SMALL                = 0x8000300D,
    EXT_ERR_NV_KEY_HASH_BUFFER_TOO_SMALL             = 0x8000300E,
    EXT_ERR_NV_GET_BUFFER_TOO_SMALL                  = 0x8000300F,
    EXT_ERR_NV_AES_UNAVAILABLE                       = 0x80003010,
    EXT_ERR_NV_HASH_UNAVAILABLE                      = 0x80003011,
    EXT_ERR_NV_INVALID_PAGE                          = 0x80003012,
    EXT_ERR_NV_ZERO_LENGTH_COPY                      = 0x80003013,
    EXT_ERR_NV_LENGTH_MISMATCH                       = 0x80003014,
    EXT_ERR_NV_HASH_MISMATCH                         = 0x80003015,
    EXT_ERR_NV_DATA_MISMATCH                         = 0x80003016,
    EXT_ERR_NV_SEARCH_PATTERN_MISMATCH               = 0x80003017,
    EXT_ERR_NV_SEARCH_KEY_TYPE_MISMATCH              = 0x80003018,
    EXT_ERR_NV_SEARCH_KEY_STATE_MISMATCH             = 0x80003019,
    EXT_ERR_NV_INVALID_KEY_HEADER                    = 0x8000301A,
    EXT_ERR_NV_PAGE_NOT_FOUND                        = 0x8000301B,
    EXT_ERR_NV_STATE_INVALID                         = 0x8000301C,
    EXT_ERR_NV_KEY_NOT_IN_WRITE_QUEUE                = 0x8000301D,
    EXT_ERR_NV_BUFFER_PRIMED_PREMATURELY             = 0x8000301E,
    EXT_ERR_NV_ILLEGAL_OPERATION                     = 0x8000301F,
    EXT_ERR_NV_RPC_ERROR                             = 0x80003020,
    EXT_ERR_NV_READ_FLASH_ERR                        = 0x80003021,
    EXT_ERR_NV_BUFFER_TOO_SMALL                      = 0x80003022,
    EXT_ERR_NV_FAILURE                               = 0x80003023,
    EXT_ERR_NV_INIT_FAILED                           = 0x80003024,
    EXT_ERR_NV_PAGE_INIT_ERROR                       = 0x80003025,
    EXT_ERR_NV_NO_KEY_DATA                           = 0x80003026,
    EXT_ERR_NV_NOTIFY_LIST_FULL                      = 0x80003027,
    EXT_ERR_NV_NOTIFY_SEGMENT_ERR                    = 0x80003028,

    /* HCC MODULE */
    EXT_ERR_HCC_INIT_ERR                             = 0x80003030,
    EXT_ERR_HCC_PARAM_ERR                            = 0x80003031,
    EXT_ERR_HCC_STATE_OFF                            = 0x80003032,
    EXT_ERR_HCC_STATE_EXCEPTION                      = 0x80003033,
    EXT_ERR_HCC_BUILD_TX_BUF_ERR                     = 0x80003034,
    EXT_ERR_HCC_TX_BUF_ERR                           = 0x80003035,
    EXT_ERR_HCC_BUS_ERR                              = 0x80003036,
    EXT_ERR_HCC_FC_PRE_PROC_ERR                      = 0x80003037,
    EXT_ERR_HCC_HANDLER_ERR                          = 0x80003038,
    EXT_ERR_HCC_FC_PROC_UNBLOCK                      = 0x80003039,
    EXT_ERR_HCC_FC_PROC_BLOCK                        = 0x8000303A,
    EXT_ERR_HCC_HANDLER_REPEAT                       = 0x8000303B,
    EXT_ERR_HCC_SERVICE_REGISTER_REPEAT              = 0x8000303C,
    EXT_ERR_HCC_ROMBLE_HOOK_NULL                     = 0x8000303D,

    /* CYCBUF */
    EXT_ERR_CYCBUF_NOT_ENOUGH_HANDLE                 = 0x80003041,
    EXT_ERR_CYCBUF_INVALID_HANDLE                    = 0x80003042,
    EXT_ERR_CYCBUF_MALLOC_FAIL                       = 0x80003043,
    EXT_ERR_CYCBUF_BUSY                              = 0x80003044,
    EXT_ERR_CYCBUF_NO_DATA                           = 0x80003045,
    EXT_ERR_CYCBUF_MEMOPT_FAIL                       = 0x80003046,
    EXT_ERR_CYCBUF_SUB_INVALID_FREE_LEN              = 0x80003047,
    EXT_ERR_CYCBUF_SUB_INVALID_USED_LEN              = 0x80003048,

    EXT_ERR_IRQ_REQUEST_FAIL                         = 0x80003081,
    EXT_ERR_IRQ_FREE_FAIL                            = 0x80003082,

    EXT_ERR_RAND_NOT_SUPPORT_CRYPT_RAND              = 0x800030c1,

    EXT_ERR_CORE_MUTEX_INVALID_ID                    = 0x80003101,
    EXT_ERR_CORE_MUTEX_GET_FAIL                      = 0x80003102,
    EXT_ERR_CORE_MUTEX_GET_TIME_OUT                  = 0x80003103,

    EXT_ERR_TRACK_INIT_FAIL                          = 0x80003141,
    EXT_ERR_TRACK_START_FAIL                         = 0x80003142,
    EXT_ERR_TRACK_DEINIT_FAIL                        = 0x80003143,
    EXT_ERR_TRACK_GET_INFO_FAIL                      = 0x80003144,
    EXT_ERR_TRACK_RECORD_FAIL                        = 0x80003145,

    /* PARTITION */
    EXT_ERR_PARTITION_INIT_ERR                       = 0x80003160,
    EXT_ERR_PARTITION_NOT_SUPPORT                    = 0x80003161,
    EXT_ERR_PARTITION_INVALID_PARAMS                 = 0x80003162,
    EXT_ERR_PARTITION_CONFIG_NOT_FOUND               = 0x80003163,
    EXT_ERR_IMAGE_CONFIG_NOT_FOUND                   = 0x80003164,

    /* AT */
    EXT_ERR_AT_RECVING                               = 0x80003180,
    EXT_ERR_AT_INVALID_PARAMETER                     = 0x80003181,
    EXT_ERR_AT_NAME_OR_FUNC_REPEAT_REGISTERED        = 0x80003182,
    EXT_ERR_AT_NO_SLEEP                              = 0x80003183,
    EXT_ERR_AT_DEEP_SLEEP                            = 0x80003184,

    /* diag */
    EXT_ERR_DIAG_STAT_INVALID_ID                     = 0x800031A1, /* stat id is invalid */
    EXT_ERR_DIAG_NO_INITILIZATION                    = 0x800031A2, /* diag not initilizate */
    EXT_ERR_DIAG_NOT_FOUND                           = 0x800031A3, /* diag id not found */
    EXT_ERR_DIAG_INVALID_PARAMETER                   = 0x800031A4, /* parameter invalid */
    /* If the return value is EXT_ERR_DIAG_CONSUMED, it indicates that the user sends a response (local connection)
    to the host through the diag_send_ack_packet_prv API. The DIAG framework does not automatically */
    EXT_ERR_DIAG_CONSUMED                            = 0x800031A5,
    EXT_ERR_DIAG_TOO_SMALL_BUFFER                    = 0x800031A6, /* too small buffer */
    EXT_ERR_DIAG_NO_MORE_DATA                        = 0x800031A7, /* uart receive no more data */
    EXT_ERR_DIAG_NOT_ENOUGH_MEMORY                   = 0x800031A8, /* malloc memory fail */
    EXT_ERR_DIAG_INVALID_HEAP_ADDR                   = 0x800031A9, /* addr out of heap range */
    EXT_ERR_DIAG_NOT_CONNECT                         = 0x800031Aa, /* diag not connect */
    EXT_ERR_DIAG_BUSY                                = 0x800031Ab, /* channel is busy */
    EXT_ERR_DIAG_TOO_LARGE_FRAME                     = 0x800031Ac, /* too large frame */
    EXT_ERR_DIAG_RAM_ALIGN                           = 0x800031Ad, /* ram addr not align */
    EXT_ERR_DIAG_NOT_SUPPORT                         = 0x800031Ae, /* not support */
    EXT_ERR_DIAG_QUEUE_FULL                          = 0x800031Af, /* queue full */
    EXT_ERR_DIAG_MSG_CFG_NOT_ALLOW                   = 0x800031B0, /* msg config allow send msg to pc */
    EXT_ERR_DIAG_TOO_LARGE_MSG                       = 0x800031B1, /* too large msg */
    EXT_ERR_DIAG_INVALID_CODE_ADDR                   = 0x800031B2, /* invalid code addr */
    EXT_ERR_DIAG_OBJ_NOT_FOUND                       = 0x800031B3, /* object not found */
    EXT_ERR_DIAG_BLOCK_EMPTY                         = 0x800031B4, /* buffer list is empty */
    EXT_ERR_DIAG_NOT_ENOUGH_SHARE_SPACE              = 0x800031B5, /* share space not enough */
    EXT_ERR_DIAG_CORRUPT_SHARED_MEMORY               = 0x800031B6, /* share space ctrl is corrupt */
    EXT_ERR_DIAG_NO_NEXT_MESSAGE                     = 0x800031B7, /* no more msg in share mempry */
    EXT_ERR_DIAG_KV_NOT_SUPPORT_ID                   = 0x800031B8, /* not support kv id */
    EXT_ERR_DIAG_BAD_DATA                            = 0x800031B9, /* bad data */

    /* SYSERR */
    EXT_ERR_SYSERR_FAILURE                           = 0x800031D0,
    EXT_ERR_SYSERR_INVALID_PARAMETER                 = 0x800031D1,
    EXT_ERR_SYSERR_NO_CRASH_INFO                     = 0x800031D2,
    EXT_ERR_SYSERR_INVALID_OPERATION                 = 0x800031D3,

    /* HCC */
    EXT_ERR_HCC_INVALID_PARAMETER                    = 0x80003281,
    EXT_ERR_HCC_NO_SUCH_DEVICE                       = 0x80003282,
    EXT_ERR_HCC_BUS_HANDLE_INVALID                   = 0x80003283,

    /* system status */
    EXT_ERR_PROTOCOL_FLASH_VETO                      = 0x800032C1,
    EXT_ERR_APPS_FLASH_VETO                          = 0x800032C2,
    EXT_ERR_IOMCU_FLASH_VETO                         = 0x800032C3,
    EXT_ERR_FLASH_VOTE_MUX                           = 0x800032C4,

    /* system cpup_dfx */
    EXT_ERR_CPUP_INVALID_PARAMETER                   = 0x80003300,

    /* GFX */
    EXT_ERR_GFX_INVALID_PARAM                        = 0x80004000,
    EXT_ERR_GFX_NULL_POINTER                         = 0x80004001,
    EXT_ERR_GFX_NO_MEM                               = 0x80004002,
    EXT_ERR_GFX_UNSUPPOTRED_OPERATION                = 0x80004003,
    EXT_ERR_GFX_OPERATION_TIMEOUT                    = 0x80004004,
    EXT_ERR_GFX_OPERATION_INTERRUPT                  = 0x80004005,
    EXT_ERR_GFX_INVALID_RESO                         = 0x80004006,
    EXT_ERR_GFX_INVALID_FMT                          = 0x80004007,
    EXT_ERR_GFX_INVALID_ADDR                         = 0x80004008,
    EXT_ERR_GFX_INVALID_STRIDE                       = 0x80004009,
    EXT_ERR_GFX_INVALID_RECT                         = 0x8000400a,
    EXT_ERR_GFX_INVALID_MODE                         = 0x8000400b,

    /* Cache */
    EXT_ERR_CACHE_INVALID_PARAM                      = 0x80004101,

    /*
    * 5 Protocol stack (0x8000_4000 - 0x8000_5000)
    */
    /* BT 0x8000_4200 - 0x8000_4299 */
    EXT_ERR_BT_DEV_NOT_FOUND                        = 0x80004200,
    EXT_ERR_BT_DEV_NOT_PAIRED                       = 0x80004201,
    EXT_ERR_BT_STREAM_NOT_EXIST                     = 0x80004202,
    /*
    * 6 soc Reserved (0x8000_5000 - 0x8000_7000)
    */

    /*
    * 7 Reserved by customers (0x8000_7000 - 0x8000_8000)
    */
    /* PLC */
    EXT_ERR_PLC_FULL                                 = 0x80008001,
    EXT_ERR_MALLOC_FAILUE                            = 0x80008002,
    EXT_ERR_TF_IDENTIFY_DOING                        = 0x80008003,
    EXT_ERR_NOT_SUPPORT                              = 0x80008004,
    EXT_ERR_BUSY                                     = 0x80008005,
    EXT_ERR_NOT_EXIST                                = 0x80008006,
    EXT_ERR_BAD_DATA                                 = 0x80008007,
    EXT_ERR_FEATURE_NOT_SUPPORT                      = 0x80008008,
    EXT_ERR_NO_INITILIZATION                         = 0x80008009,
    EXT_ERR_NOT_FOUND_NETWORK                        = 0x8000800a,
    EXT_ERR_CONTINUE                                 = 0x8000800b,
    EXT_ERR_MEMCPY_FAIL                              = 0x8000800c,
    EXT_ERR_BAD_FRAME                                = 0x8000800d,
    EXT_ERR_EXIST                                    = 0x8000800e,
    EXT_ERR_SKIP                                     = 0x8000800f,
    EXT_ERR_INVALID_PARAMETER                        = 0x80008010,
    EXT_ERR_NOT_ENOUGH_MEMORY                        = 0x80008011,
    EXT_ERR_TOO_SMALL_BUFFER                         = 0x80008012,

    /* Touch panel (0x8000_8040 - 0x8000_80C0) */
    EXT_ERR_TP_DEV_INFO_NOT_REGISTER                 = 0x80008040,
    EXT_ERR_TP_I2C_DEV_NOT_INIT                      = 0x80008041,
    EXT_ERR_TP_GPIO_DEV_NOT_INIT                     = 0x80008042,
    EXT_ERR_TP_ZTW523_INIT_DATA                      = 0x80008043,
    EXT_ERR_TP_ZTW523_INIT_RESET                     = 0x80008044,
    EXT_ERR_TP_ZTW523_INIT_POWERUP                   = 0x80008045,
    EXT_ERR_TP_ZTW523_INIT_CONFIG                    = 0x80008046,
    EXT_ERR_TP_ZTW523_INIT_GETINFO                   = 0x80008047,
    EXT_ERR_TP_ZTW523_POWERUP_ERROR                  = 0x80008048,
    EXT_ERR_TP_ZTW523_POWERUP_VENDOR_CMD_ENABLE      = 0x80008049,
    EXT_ERR_TP_ZTW523_POWERUP_READ_CHIPID            = 0x8000804A,
    EXT_ERR_TP_ZTW523_POWERUP_INTN_CLEAR             = 0x8000804B,
    EXT_ERR_TP_ZTW523_POWERUP_NVM_INIT               = 0x8000804C,
    EXT_ERR_TP_ZTW523_POWERUP_PROGRAM_START          = 0x8000804D,
    EXT_ERR_TP_ZTW523_POWERUP_REPORT_RATE            = 0x8000804E,
    EXT_ERR_TP_ZTW523_POWERUP_PIXEL_THRESHOLD        = 0x8000804F,
    EXT_ERR_TP_ZTW523_POWERUP_SW_RESET               = 0x80008050,
    EXT_ERR_TP_ZTW523_HWCALIB_TOUCHMODE              = 0x80008051,
    EXT_ERR_TP_ZTW523_HWCALIB_CMD1                   = 0x80008052,
    EXT_ERR_TP_ZTW523_HWCALIB_CLEARINT1              = 0x80008053,
    EXT_ERR_TP_ZTW523_HWCALIB_READE2P                = 0x80008054,
    EXT_ERR_TP_ZTW523_HWCALIB_CMD2                   = 0x80008055,
    EXT_ERR_TP_ZTW523_HWCALIB_CLEARINT2              = 0x80008056,
    EXT_ERR_TP_ZTW523_HWCALIB_OVERTIME               = 0x80008057,
    EXT_ERR_TP_ZTW523_HWCALIB_INITTOUCHMODE          = 0x80008058,
    EXT_ERR_TP_ZTW523_HWCALIB_WRITETOUCHMODE         = 0x80008059,
    EXT_ERR_TP_ZTW523_HWCALIB_WRITEINT               = 0x8000805A,
    EXT_ERR_TP_ZTW523_HWCALIB_WRITECTL1_EN           = 0x8000805B,
    EXT_ERR_TP_ZTW523_HWCALIB_WRITECTL2_EN           = 0x8000805C,
    EXT_ERR_TP_ZTW523_HWCALIB_SAVE                   = 0x8000805D,
    EXT_ERR_TP_ZTW523_HWCALIB_WRITECTL1_DIS          = 0x8000805E,
    EXT_ERR_TP_ZTW523_HWCALIB_WRITECTL2_DIS          = 0x8000805F,
    EXT_ERR_TP_ZTW523_POWERUP_FW_CMD_ENABLE          = 0x80008060,
    EXT_ERR_TP_ZTW523_POWERUP_FW_READ_CHIPCODE       = 0x80008061,
    EXT_ERR_TP_ZTW523_POWERUP_FW_INTN_CLEAR          = 0x80008062,
    EXT_ERR_TP_ZTW523_POWERUP_FW_NVM_INIT            = 0x80008063,
    EXT_ERR_TP_ZTW523_POWERUP_FW_NVM_VPPON           = 0x80008064,
    EXT_ERR_TP_ZTW523_POWERUP_FW_NVM_WPDIS           = 0x80008065,
    EXT_ERR_TP_ZTW523_POWERUP_FW_INIT_FLASH          = 0x80008066,
    EXT_ERR_TP_ZTW523_FW_VERIFY_NVM_VPPON            = 0x80008067,
    EXT_ERR_TP_ZTW523_FW_VERIFY_NVM_WPDIS            = 0x80008068,
    EXT_ERR_TP_ZTW523_FW_VERIFY_INIT_FLASH           = 0x80008069,
    EXT_ERR_TP_ZTW523_FW_VERIFY_OPENFILE             = 0x8000806A,
    EXT_ERR_TP_ZTW523_FW_VERIFY_FSTAT                = 0x8000806B,
    EXT_ERR_TP_ZTW523_FW_VERIFY_PAGESIZE             = 0x8000806C,
    EXT_ERR_TP_ZTW523_FW_VERIFY_LSEEK                = 0x8000806D,
    EXT_ERR_TP_ZTW523_FW_VERIFY_READSIZE             = 0x8000806E,
    EXT_ERR_TP_ZTW523_FW_VERIFY_READ                 = 0x8000806F,
    EXT_ERR_TP_ZTW523_FW_VERIFY_COMPARE              = 0x80008070,
    EXT_ERR_TP_ZTW523_FW_UPGRADE_OPENFILE            = 0x80008071,
    EXT_ERR_TP_ZTW523_FW_UPGRADE_FSTAT               = 0x80008072,
    EXT_ERR_TP_ZTW523_FW_UPGRADE_PAGESIZE            = 0x80008073,
    EXT_ERR_TP_ZTW523_FW_UPGRADE_FWSIZE              = 0x80008074,
    EXT_ERR_TP_ZTW523_FW_UPGRADE_LSEEK               = 0x80008075,
    EXT_ERR_TP_ZTW523_FW_UPGRADE_READSIZE            = 0x80008076,
    EXT_ERR_TP_ZTW523_FW_UPGRADE_WRITE               = 0x80008077,
    EXT_ERR_TP_ZTW523_FW_CHECK_RESET                 = 0x80008078,
    EXT_ERR_TP_ZTW523_FW_CHECK_POWERUP               = 0x80008079,
    EXT_ERR_TP_ZTW523_FW_CHECK_IRQDIS                = 0x8000807A,
    EXT_ERR_TP_ZTW523_FW_CHECK_READFW_VERSION        = 0x8000807B,
    EXT_ERR_TP_ZTW523_FW_CHECK_READMINORFW_VERSION   = 0x8000807C,
    EXT_ERR_TP_ZTW523_FW_CHECK_READ_DATAREG          = 0x8000807D,
    EXT_ERR_TP_ZTW523_FW_CHECK_FAIL                  = 0x8000807E,
    EXT_ERR_TP_ZTW523_FW_CHECK_INIT_TOUCHMODE        = 0x8000807F,
    EXT_ERR_TP_ZTW523_FW_CHECK_WRITE_TOUCHMODE       = 0x80008080,
    EXT_ERR_TP_ZTW523_FW_CHECK_WRITE_FINGERNUM       = 0x80008081,
    EXT_ERR_TP_ZTW523_FW_CHECK_WRITE_XRES            = 0x80008082,
    EXT_ERR_TP_ZTW523_FW_CHECK_WRITE_YRES            = 0x80008083,
    EXT_ERR_TP_ZTW523_FW_CHECK_WRITE_CALIBCMD        = 0x80008084,
    EXT_ERR_TP_ZTW523_FW_CHECK_WRITE_INTMASK         = 0x80008085,
    EXT_ERR_TP_ZTW523_FW_CHECK_OVERTIMES             = 0x80008086,
    EXT_ERR_TP_ZTW523_CHECKID_RESET                  = 0x80008087,
    EXT_ERR_TP_ZTW523_CHECKID_POWERUP                = 0x80008088,
    EXT_ERR_TP_ZTW523_CHECKID_READ_CHIPCODE          = 0x80008089,
    EXT_ERR_TP_ZTW523_CHECKID_READ_VENDORID          = 0x8000808A,
    EXT_ERR_TP_ZTW523_GETFWVER_RESET                 = 0x8000808B,
    EXT_ERR_TP_ZTW523_GETFWVER_POWERUP               = 0x8000808C,
    EXT_ERR_TP_ZTW523_GETFWVER_READFWVER             = 0x8000808D,
    EXT_ERR_TP_ZTW523_GETFWVER_READMINORFWVER        = 0x8000808E,
    EXT_ERR_TP_ZTW523_GETFWVER_READDATAREG           = 0x8000808F,
    EXT_ERR_TP_ZTW523_CONFIG_RESET                   = 0x80008090,
    EXT_ERR_TP_ZTW523_CONFIG_READ_FWVERSION          = 0x80008091,
    EXT_ERR_TP_ZTW523_CONFIG_READ_FWMINORVERSION     = 0x80008092,
    EXT_ERR_TP_ZTW523_CONFIG_READ_DATAREG            = 0x80008093,
    EXT_ERR_TP_ZTW523_CONFIG_INIT_TOUCHMODE          = 0x80008094,
    EXT_ERR_TP_ZTW523_CONFIG_WRITE_TOUCHMODE         = 0x80008095,
    EXT_ERR_TP_ZTW523_CONFIG_WRITE_FINGER_NUM        = 0x80008096,
    EXT_ERR_TP_ZTW523_CONFIG_WRITE_XRES              = 0x80008097,
    EXT_ERR_TP_ZTW523_CONFIG_WRITE_YRES              = 0x80008098,
    EXT_ERR_TP_ZTW523_CONFIG_WRITE_CALIBCMD          = 0x80008099,
    EXT_ERR_TP_ZTW523_CONFIG_WRITE_INTMASK           = 0x8000809A,
    EXT_ERR_TP_ZTW523_CONFIG_WRITE_GESWAKEUP         = 0x8000809B,
    EXT_ERR_TP_ZTW523_CONFIG_CLEAR_INT               = 0x8000809C,
    EXT_ERR_TP_ZTW523_SETMODE_SWRESET                = 0x8000809D,
    EXT_ERR_TP_ZTW523_SETMODE_SETNCOUNT              = 0x8000809E,
    EXT_ERR_TP_ZTW523_SETMODE_SETNAFE                = 0x8000809F,
    EXT_ERR_TP_ZTW523_SETMODE_SETUCOUNT              = 0x800080A0,
    EXT_ERR_TP_ZTW523_SETMODE_SETDELAY               = 0x800080A1,
    EXT_ERR_TP_ZTW523_SETMODE_WRITEMODE              = 0x800080A2,
    EXT_ERR_TP_ZTW523_SETMODE_CLEAR_INT              = 0x800080A3,
    EXT_ERR_TP_ZTW523_GET_RAWDATA_CLEAR_INT1         = 0x800080A4,
    EXT_ERR_TP_ZTW523_GET_RAWDATA_READDATA           = 0x800080A5,
    EXT_ERR_TP_ZTW523_GET_RAWDATA_CLEAR_INT2         = 0x800080A6,
    EXT_ERR_TP_ZTW523_GET_DND_SETMODE                = 0x800080A7,
    EXT_ERR_TP_ZTW523_GET_DND_DATA                   = 0x800080A8,
    EXT_ERR_TP_ZTW523_GET_DND_SET_POINTMODE          = 0x800080A9,
    EXT_ERR_TP_ZTW523_GET_SELFDND_SETMODE            = 0x800080AA,
    EXT_ERR_TP_ZTW523_GET_SELFDND_DATA               = 0x800080AB,
    EXT_ERR_TP_ZTW523_GET_SELFDND_SET_POINTMODE      = 0x800080AC,
    EXT_ERR_TP_ZTW523_GET_HWID_RESET                 = 0x800080AD,
    EXT_ERR_TP_ZTW523_GET_HWID_POWERUP               = 0x800080AE,
    EXT_ERR_TP_ZTW523_GET_HWID_READ_ID               = 0x800080AF,

    EXT_ERR_BOOT_RECEIVER_BUF_ERR                    = 0x80008101,
    EXT_ERR_BOOT_DOWNLOAD_FAIL                       = 0x80008102,
    EXT_ERR_BOOT_BUS_TYPE_ERR                        = 0x80008103,
    EXT_ERR_BOOT_IRQ_REG_FAIL                        = 0x80008104,
    EXT_ERR_BOOT_GET_CMD_FAIL                        = 0x80008105,
    EXT_ERR_BOOT_FIFO_CHECK_FAIL                     = 0x80008106,
    EXT_ERR_BOOT_EXE_CMD_FAIL                        = 0x80008107,
    EXT_ERR_BOOT_PARSE_CMD_FAIL                      = 0x80008108,
    EXT_ERR_BOOT_CPY_VERSION_FAIL                    = 0x80008109,

    /* Panel timing (0x8000_8140 - 0x8000_817F) */
    EXT_ERR_PANEL_TIMING_NOT_INIT                    = 0x80008140,
    EXT_ERR_PANEL_NOT_SUPPORT_SCREEN_TYPE            = 0x80008141,
    EXT_ERR_PANEL_INVALID_NULL_POINTER               = 0x80008142,
    EXT_ERR_PANEL_OVER_REGION                        = 0x80008143,
    EXT_ERR_PANEL_REGION_NOT_MATCH                   = 0x80008144,

    /* LCD driver (0x8000_8180 - 0x8000_81BF) */
    EXT_ERR_LCD_NOT_INIT                             = 0x80008180,
    EXT_ERR_LCD_MUTEX_INIT_FAIL                      = 0x80008181,
    EXT_ERR_LCD_INVALID_NULL_POINTER                 = 0x80008182,
    EXT_ERR_LCD_GET_ATTR_FAIL                        = 0x80008183,
    EXT_ERR_LCD_INVALID_DATA_LENGTH                  = 0x80008184,
    EXT_ERR_LCD_INVALID_DISP_ON_SEQU                 = 0x80008185,
    EXT_ERR_LCD_INVALID_WRITE_CMD                    = 0x80008186,
    EXT_ERR_LCD_INVALID_BUS_OPS                      = 0x80008187,
    EXT_ERR_LCD_INVALID_BUS_WRITE_FUNC               = 0x80008188,
    EXT_ERR_LCD_INVALID_READ_CMD                     = 0x80008189,
    EXT_ERR_LCD_INVALID_DRIVER_OPS                   = 0x8000818A,
    EXT_ERR_LCD_INVALID_SCREEN_OPS                   = 0x8000818B,
    EXT_ERR_LCD_GET_ID_FAIL                          = 0x8000818C,
    EXT_ERR_LCD_INVALID_DISP_ON_CMD                  = 0x8000818D,
    EXT_ERR_LCD_INVALID_DISP_OFF_SEQU                = 0x8000818E,
    EXT_ERR_LCD_INVALID_DISP_OFF_CMD                 = 0x8000818F,
    EXT_ERR_LCD_IN_DISPLAY_OFF_STATE                 = 0x80008190,
    EXT_ERR_LCD_INVALID_BUS_READ_FUNC                = 0x80008191,
    EXT_ERR_LCD_EMPTY_SDLP_FUNC                      = 0x80008192,
    EXT_ERR_LCD_NOT_SUPPORT_OFF_TO_SLEEP             = 0x80008193,
    EXT_ERR_LCD_NOT_IN_SLEEP_MODE                    = 0x80008194,
    EXT_ERR_LCD_NOT_SUPPORT_CUR_TO_IDLE              = 0x80008195,
    EXT_ERR_LCD_NOT_IN_IDLE_MODE                     = 0x80008196,
    EXT_ERR_LCD_INVALID_TE_ON_MODE                   = 0x80008197,
    EXT_ERR_LCD_VIDEO_SCREEN_NOT_SUPPORT_TE          = 0x80008198,
    EXT_ERR_LCD_INVALID_IDLE_ON_CMD                  = 0x80008199,
    EXT_ERR_LCD_INVALID_IDLE_OFF_CMD                 = 0x8000819A,
    EXT_ERR_LCD_INVALID_TE_ON_CMD                    = 0x8000819B,
    EXT_ERR_LCD_INVALID_TE_OFF_CMD                   = 0x8000819C,

    /* !!!!--- Temporary version: errno code ---!!!!! */

    /*****************************************************************************
    * 1 COMMON ERR
    *****************************************************************************/
    EXT_ERR_INVALID_PARAM                            = 0x81000005,
    EXT_ERR_PTR_NULL                                 = 0x81000006,

    /*****************************************************************************
    * 2 soc osa (0x8000_0080 - 0x8000_1000)
    *****************************************************************************/
    EXT_ERR_OS_MUTEX_ACQUIRE                         = 0x81000084,
    EXT_ERR_OS_MUTEX_RELEASE                         = 0x81000085,
    EXT_ERR_OS_GET_TASK_INFO_INVALID_PARAM           = 0x81000086,
    EXT_ERR_OS_GET_TASK_INFO_OS_ERR                  = 0x81000087,
    /* task */
    EXT_ERR_OS_TASK_INVALID_PARAM                    = 0x810000C1,
    EXT_ERR_OS_TASK_CREATE_FAIL                      = 0x810000C2,
    EXT_ERR_OS_TASK_DELETE_FAIL                      = 0x810000C3,
    EXT_ERR_OS_TASK_SUPPEND_FAIL                     = 0x810000C4,
    EXT_ERR_OS_TASK_RESUME_FAIL                      = 0x810000C5,
    EXT_ERR_OS_TASK_GET_PRI_FAIL                     = 0x810000C6,
    EXT_ERR_OS_TASK_SET_PRI_FAIL                     = 0x810000C7,
    EXT_ERR_OS_TASK_LOCK_FAIL                        = 0x810000C8,
    EXT_ERR_OS_TASK_UNLOCK_FAIL                      = 0x810000C9,
    EXT_ERR_OS_TASK_DELAY_FAIL                       = 0x810000CA,
    EXT_ERR_OS_TASK_GET_INFO_FAIL                    = 0x810000CB,
    EXT_ERR_OS_TASK_REGISTER_SCHEDULE_FAIL           = 0x810000CC,
    EXT_ERR_OS_TASK_NOT_CREATED                      = 0x810000CD,

    /* 中断ISR */
    EXT_ERR_OS_ISR_INVALID_PARAM                     = 0x81000101,
    EXT_ERR_OS_ISR_REQ_IRQ_FAIL                      = 0x81000102,
    EXT_ERR_OS_ISR_ADD_JOB_MALLOC_FAIL               = 0x81000103,
    EXT_ERR_OS_ISR_ADD_JOB_SYS_FAIL                  = 0x81000104,
    EXT_ERR_OS_ISR_DEL_IRQ_FAIL                      = 0x81000105,
    EXT_ERR_OS_ISR_ALREADY_CREATED                   = 0x81000106,
    EXT_ERR_OS_ISR_NOT_CREATED                       = 0x81000107,
    EXT_ERR_OS_ISR_ENABLE_IRQ_FAIL                   = 0x81000108,
    EXT_ERR_OS_ISR_IRQ_ADDR_NOK                      = 0x81000109,

    /* 事件 */
    EXT_ERR_OS_EVENT_INVALID_PARAM                   = 0x81000141,
    EXT_ERR_OS_EVENT_CREATE_NO_HADNLE                = 0x81000142,
    EXT_ERR_OS_EVENT_CREATE_SYS_FAIL                 = 0x81000143,
    EXT_ERR_OS_EVENT_SEND_FAIL                       = 0x81000144,
    EXT_ERR_OS_EVENT_WAIT_FAIL                       = 0x81000145,
    EXT_ERR_OS_EVENT_CLEAR_FAIL                      = 0x81000146,
    EXT_ERR_OS_EVENT_RE_INIT                         = 0x81000147,
    EXT_ERR_OS_EVENT_NOT_ENOUGH_MEMORY               = 0x81000148,
    EXT_ERR_OS_EVENT_NOT_INIT                        = 0x81000149,
    EXT_ERR_OS_EVENT_DELETE_FAIL                     = 0x8100014A,
    EXT_ERR_OS_EVENT_WAIT_TIME_OUT                   = 0x8100014B,

    /* Mutex */

    /*****************************************************************************
    * 3 DRIVE (0x8000_1000 - 0x8000_3000)
    *****************************************************************************/
    /* IO moudle */

    /* GPIO moudle */

    /* UART moudle */
    EXT_ERR_UART_NOT_SUPPORT_DMA                     = 0x810010DA,
    EXT_ERR_UART_NOT_BLOCK_MODE                      = 0x810010DB,

    /* SPI moudle */

    /* I2C moudle */

    /* PWM moudle */

    /* Flash moudle */
    EXT_ERR_FLASH_NOT_ENOUGH_SPACE                   = 0x810011C3,
    EXT_ERR_FLASH_ERASE_TIMEOUT                      = 0x810011C4,
    EXT_ERR_FLASH_WRITE_TIMEOUT                      = 0x810011C5,
    EXT_ERR_FLASH_FAILURE                            = 0x810011C6,

    /* dma */
    EXT_ERR_DMA_CH_IRQ_ENABLE_FAIL                   = 0x81001208,

    /* WDT */
    EXT_ERR_WDT_LOAD_VAL_INVALID                     = 0x81001242,
    EXT_ERR_WDT_INT_REGISTER_FAILED                  = 0x81001243,
    EXT_ERR_WDT_B_INT_CREATE_FAILED                  = 0x81001244,
    EXT_ERR_WDT_B_INT_ENABLE_FAILED                  = 0x81001245,

    /* HWI */

    /* timer */
    EXT_ERR_TIMER_INVALID_PRE                        = 0x810012C3,
    EXT_ERR_TIMER_NULL_TIMER_PARA                    = 0x810012C4,
    EXT_ERR_TIMER_INVALID_INDEX                      = 0x810012C5,
    EXT_ERR_TIMER_NULL_CB_FUNC                       = 0x810012C6,
    EXT_ERR_TIMER_RESOLUTION_ZERO                    = 0x810012C7,
    EXT_ERR_TIMER_INVALID_LOAD_TIME                  = 0x810012C8,
    EXT_ERR_TIMER_TIMER_ALREADY_INIT                 = 0x810012C9,
    EXT_ERR_TIMER_INT_REGISTER_FAILED                = 0x810012CA,
    EXT_ERR_TIMER_LOAD_TOO_LARGE                     = 0x810012CC,

    /* hw_copy */

    /* swi */

    /* AIO */
    EXT_ERR_AIO_MUTEX_ERROR                          = 0x81001381,
    EXT_ERR_AIO_RESOURCE_BUSY                        = 0x81001382,
    EXT_ERR_AIO_RESOURCE_CONNECT_ERROR               = 0x81001383,
    EXT_ERR_AIO_RESOURCE_FREE_ERROR                  = 0x81001384,
    EXT_ERR_AIO_VOL_TYPE_ERROR                       = 0x81001385,
    EXT_ERR_AIO_READ_VOL_ERROR                       = 0x81001386,
    EXT_ERR_AIO_AIO_PIN_ERROR                        = 0x81001387,
    EXT_ERR_AIO_IDRV_INVALID_RANGE                   = 0x81001388,
    EXT_ERR_AIO_COMP_NUMBER_ERROR                    = 0x81001389,
    EXT_ERR_AIO_COMP_INVALID_RANGE                   = 0x8100138A,
    EXT_ERR_AIO_VALUE_SET_ERROR                      = 0x8100138B,
    EXT_ERR_AIO_CORE_NOT_SUPPORT                     = 0x8100138C,
    EXT_ERR_AIO_PARAMETER_ERROR                      = 0x8100138D,
    EXT_ERR_AIO_FUNC_UNREADY_ERROR                   = 0x8100138E,
    EXT_ERR_AIO_ADC_TIME_OUT_ERROR                   = 0x8100138F,
    EXT_ERR_AIO_ADC_NOT_CALIBRATE_ERROR              = 0x81001391,
    EXT_ERR_AIO_RESOURCE_NOT_OWNED_ERROR             = 0x81001392,

    /* fpb */
    EXT_ERR_FPB_COMP_REPEAT                          = 0x810013C1,
    EXT_ERR_FPB_NO_COMP                              = 0x810013C1,
    EXT_ERR_FPB_TYPE                                 = 0x810013C2,
    EXT_ERR_FPB_NO_FREE_COMP                         = 0x810013C3,
    EXT_ERR_FPB_ADDR_NOT_ALIGN                       = 0x810013C4,
    EXT_ERR_FPB_TARGET_ADDR                          = 0x810013C5,
    EXT_ERR_FPB_BUSY                                 = 0x810013C6,
    EXT_ERR_FPB_ERROR_INPUT                          = 0x810013C7,

    /* cipher */

    /* ipc */
    EXT_ERR_IPC_INIT_FAIL                            = 0x81001441,
    EXT_ERR_IPC_ALREADY_INIT                         = 0x81001442,
    EXT_ERR_IPC_INVALID_PARAM                        = 0x81001443,
    EXT_ERR_IPC_BUSY                                 = 0x81001444,
    EXT_ERR_IPC_NOT_INIT                             = 0x81001445,

    /* sdio */

    /* i2s */
    EXT_ERR_I2S_TX_BUSY                              = 0x810014C4,
    EXT_ERR_I2S_RX_BUSY                              = 0x810014C5,

    /* tsensor */

    /* trng */
    EXT_ERR_TRNG_GET_TIMEOUT                         = 0x81001600,
    EXT_ERR_TRNG_INVALID_PARAMETER                   = 0x80001601,

    /* usb */
    EXT_ERR_USB_INPUT_INVALID                        = 0x81001700,
    EXT_ERR_USB_BUS_REGISTERED                       = 0x81001701,
    EXT_ERR_USB_BUS_INVALID                          = 0x81001702,
    EXT_ERR_USB_BUS_INPUT_INVALID                    = 0x81001703,
    EXT_ERR_USB_BUS_MATCH_FAIL                       = 0x81001704,
    EXT_ERR_USB_BUS_PROBE_FAIL                       = 0x81001705,
    EXT_ERR_USB_BUS_MUX_FAIL                         = 0x81001706,
    EXT_ERR_USB_DRIVER_MATCH_FAIL                    = 0x81001707,
    EXT_ERR_USB_DRIVER_PROBE_FAIL                    = 0x81001708,
    EXT_ERR_USB_DRIVER_REGISTERED                    = 0x81001709,
    EXT_ERR_USB_DRIVER_NOTFOUND                      = 0x8100170A,
    EXT_ERR_USB_DEVICE_BOUNDED                       = 0x8100170B,
    EXT_ERR_USB_DEVICE_INITIALFAIL                   = 0x8100170C,
    EXT_ERR_USB_DEVICE_REGISTERED                    = 0x8100170D,
    EXT_ERR_USB_DEVICE_BUSY                          = 0x8100170E,
    EXT_ERR_USB_CLEAR_STALL_TIMEOUT_CODE             = 0x8100170F,
    EXT_ERR_USB_SET_STALL_TIMEOUT_CODE               = 0x81001710,
    EXT_ERR_USB_UNKNOWN_SPEED_CODE                   = 0x81001711,
    EXT_ERR_USB_RUN_STOP_DISEABLE_CODE               = 0x81001712,
    EXT_ERR_USB_UNHANDLED_DEV_EVENT_CODE             = 0x81001713,
    EXT_ERR_USB_INVALID_TRB_HANDLE_CODE              = 0x81001714,
    EXT_ERR_USB_CONFIG_INDEX_ERROR_CODE              = 0x81001715,
    EXT_ERR_USB_DES_REQUEST_ERROR_CODE               = 0x81001716,
    EXT_ERR_USB_ENABLE_EP_FAIL_CODE                  = 0x81001717,
    EXT_ERR_USB_INIT_EP_FAIL_CODE                    = 0x81001718,
    EXT_ERR_USB_INVALID_CONFIG_VALUE_CODE            = 0x81001719,
    EXT_ERR_USB_NO_ACTIVE_CONFIG_CODE                = 0x8100171a,
    EXT_ERR_USB_INVALID_DATA_LEN_CODE                = 0x8100171b,
    EXT_ERR_USB_INVALID_ADDRESS_CODE                 = 0x8100171c,
    EXT_ERR_USB_FAIL_ISSUE_COMMAND_CODE              = 0x8100171d,
    EXT_ERR_USB_FAIL_END_PRE_XFER_CODE               = 0x8100171e,
    EXT_ERR_USB_INIT_TRB_FAIL_CODE                   = 0x8100171f,
    EXT_ERR_USB_START_XFER_FAIL_CODE                 = 0x81001720,
    EXT_ERR_USB_POSSIBLE_BUF_OVER_RUN_CODE           = 0x81001721,
    EXT_ERR_USB_STATUS_PKT_STILL_XFER_CODE           = 0x81001722,
    EXT_ERR_USB_SETUP_STAGE_NOT_DATA_OR_STATUS_CODE  = 0x81001723,
    EXT_ERR_USB_RUN_FAIL                             = 0x81001724,
    EXT_ERR_USB_STOP_FAIL                            = 0x81001725,
    EXT_ERR_USB_NOT_STARTED                          = 0x81001726,
    EXT_ERR_USBD_INVALID_PARAMETER                   = 0x81001727,
    EXT_ERR_USBD_RUN_FAIL                            = 0x81001728,
    EXT_ERR_USBD_STOP_FAIL                           = 0x81001729,
    EXT_ERR_USBD_MEM_NOT_ENOUGH                      = 0x8100172a,
    EXT_ERR_USBD_NOT_FOUND_EP                        = 0x8100172b,
    EXT_ERR_USBD_SOFT_RESET_TIMEOUT                  = 0x8100172c,
    EXT_ERR_USBD_NOT_STARTED                         = 0x8100172d,
    EXT_ERR_USBD_OUT_OF_RESOURCE                     = 0x8100172e,
    EXT_ERR_USBD_EP_NOT_READY                        = 0x8100172f,
    EXT_ERR_USBD_BAD_SETUP_ERROR                     = 0x81001730,
    /*****************************************************************************
    * 4 Component (0x8000_3000 - 0x8000_4000)
    *****************************************************************************/

    /* diag */
    /* If the return value is EXT_ERR_DIAG_CONSUMED, it indicates that the user sends a response (local connection)
    to the host through the osal_diag_send_ack_packet API. The DIAG framework does not automatically */

    /* soft timer */
    EXT_ERR_STIMER_NO_TIMER_USED                     = 0x810031C1,
    EXT_ERR_STIMER_TICK_OR_TIMER_LINE_WRONG          = 0x810031C2,
    EXT_ERR_STIMER_GET_CRTTLINE_FAILED               = 0x810031C3,
    EXT_ERR_STIMER_NOT_VALIDATE                      = 0x810031C4,
    EXT_ERR_STIMER_TICK_WRONG                        = 0x810031C5,
    EXT_ERR_STIMER_ALREADY_INIT                      = 0x810031C6,
    EXT_ERR_STIMER_GET_TIMER_LINE_FAILED             = 0x810031C7,
    EXT_ERR_STIMER_FREE_TIMER_LLT_VALID              = 0x810031C8,

    /* pm */
    EXT_ERR_LOWPOWER_INVALID_PARAMETER               = 0x81003201,
    EXT_ERR_LOWPOWER_XTAL_UNSTABLE                   = 0x81003202,

    /* bt transfer control */
    EXT_ERR_BTC_PLATFORM_GET_PAYLOAD_FAIL            = 0x81003241,
    EXT_ERR_BTC_OAM_GET_PAYLOAD_FAIL                 = 0x81003242,
    EXT_ERR_BTC_CHR_GET_PAYLOAD_FAIL                 = 0x81003243,
    EXT_ERR_BTC_QUEUE_FULL                           = 0x81003244,
    EXT_ERR_BTC_START_ERR                            = 0x81003245,
    EXT_ERR_BTC_LEN_ERR                              = 0x81003246,
    EXT_ERR_BTC_TAIL_ERR                             = 0x81003247,
    EXT_ERR_BTC_MSG_FULL                             = 0x81003248,
    EXT_ERR_BTC_TX_TYPE_ERR                          = 0x81003249,
    EXT_ERR_BTC_TX_PARAM_INVALID                     = 0x8100324A,

    /* HCC */
    EXT_ERR_HCC_MSG_COUNT_INVALID                    = 0x81003284,
    EXT_ERR_HCC_RX_HANDLE_HAS_REGISTER               = 0x81003285,
    EXT_ERR_HCC_BUS_INIT_NOT_REGISTER                = 0x81003286,

    /* memory moniter */
    EXT_ERR_MEM_MONITOR_INVALID_MODE                 = 0x81003300,
    EXT_ERR_MEM_MONITOR_INVALID_INDEX                = 0x81003301,

    /* oam module */
    EXT_ERR_OAM_MEM_POOL_NO_SPACE                    = 0x81003340,
    EXT_ERR_OAM_OTA_NOT_INIT                         = 0x81003341,
    EXT_ERR_OAM_INVALID_CONFIG                       = 0x81003342,
    EXT_ERR_OAM_CODE_MSG_LENGTH_ERR                  = 0x81003343,
    EXT_ERR_OAM_CODE_PTR_NULL                        = 0x81003344,
    EXT_ERR_OAM_INVALID_LOG_SWICH_CONFIG             = 0x81003345,
    EXT_ERR_OAM_BT_PCM_NOT_INIT                      = 0x81003346,
    EXT_ERR_OAM_BT_INVALID_PCM_BUF                   = 0x81003347,
    EXT_ERR_OAM_BT_ACK_NOT_INIT                      = 0x81003348,
    EXT_ERR_OAM_BT_INVALID_ACK_BUF_CONFIG            = 0x81003349,
    EXT_ERR_OAM_SEND_FUNC_NOT_REGISTER               = 0x8100334A,
    EXT_ERR_OAM_BTC_QUEUE_FULL                       = 0x8100334B,
    EXT_ERR_OAM_INVALID_LOG_LEN                      = 0x8100334C,
    EXT_ERR_OAM_INVALID_RESOURCE_PARAM               = 0x8100334D,
    EXT_ERR_OAM_INVALID_BLOCK_LEN                    = 0x8100334E,
    EXT_ERR_OAM_INVALID_BLOCK_CNT                    = 0x8100334F,
    EXT_ERR_OAM_INVALID_RES_ADDR                     = 0x81003350,
    EXT_ERR_OAM_INVALID_IDX_ADDR                     = 0x81003351,
    EXT_ERR_OAM_BT_INVALID_ACK_LEN_CONFIG            = 0x81003352,
    EXT_ERR_OAM_BT_INVALID_PCM_LEN                   = 0x81003353,
    EXT_ERR_OAM_INVALID_LOG_LEVEL                    = 0x81003354,
    EXT_ERR_OAM_INVALID_OTA_SWITCH                   = 0x81003355,
    EXT_ERR_OAM_INVALID_PCM_SWITCH                   = 0x81003356,
    EXT_ERR_OAM_CHR_LOG_TOO_LARGE                    = 0x81003357,
    EXT_ERR_OAM_CHR_NETBUF_APPLY_FAIL                = 0x81003358,
    EXT_ERR_OAM_HOOK_OSA_NULL                        = 0x81003359,
    EXT_ERR_OAM_INVALID_RES_IDX                      = 0x8100335A,
    EXT_ERR_OAM_BT_MEM_POOL_NOT_INIT                 = 0x8100335B,
    EXT_ERR_OAM_BT_INVOLID_SEND_PARAM                = 0x8100335C,
    EXT_ERR_OAM_BTC_INVOLID_PRAM                     = 0x8100335D,

    /* share memory */
    EXT_ERR_SHM_RINGBUF_FULL                         = 0x81003380,
    EXT_ERR_SHM_SET_ADDR_FAIL                        = 0x81003381,

    /* pcie */
    EXT_ERR_PCIE_LP_ISR_REG_FAIL                     = 0x81003400,
    EXT_ERR_PCIE_EDMA_ISR_REG_FAIL                   = 0x81003401,
    EXT_ERR_PCIE_MSG_ISR_REG_FAIL                    = 0x81003402,
    EXT_ERR_PCIE_DATA_ISR_REG_FAIL                   = 0x81003403,
    EXT_ERR_PCIE_LINKDOWN_ISR_REG_FAIL               = 0x81003404,
    EXT_ERR_PCIE_RINGBUF_PARA_ERROR                  = 0x81003405,
    EXT_ERR_PCIE_RAM_CFG_NOT_SET                     = 0x81003406,
    EXT_ERR_PCIE_ISR_INIT_FAIL                       = 0x81003407,
    /*****************************************************************************
    * 5 Protocol stack (0x8000_4000 - 0x8000_5000)
    *****************************************************************************/
} ext_errno;

#ifndef EOK
#define EOK EXT_ERR_SUCCESS
#endif
#endif /* SOC_ERRNO_H_ */

