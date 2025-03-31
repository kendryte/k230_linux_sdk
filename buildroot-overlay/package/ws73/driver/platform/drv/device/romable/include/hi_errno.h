/*
 * Copyright (c) CompanyNameMagicTag 2019-2020. All rights reserved.
 * Description: Common Error Definition.
 */

#ifndef HI_ERRNO_H_
#define HI_ERRNO_H_

typedef enum {
    /*
    * 1 COMMON ERR
    */
    HI_ERR_SUCCESS                                  = 0,            /*!< succes */
    HI_ERR_FAILURE                                  = 0xFFFFFFFF,   /*!< failure */

    HI_SUCCESS                                      = HI_ERR_SUCCESS,
    HI_FAILURE                                      = HI_ERR_FAILURE,

    HI_SEC_SUCCESS                                  = 0x3CA5965A, /* Legacy name */
    HI_SEC_FAILURE                                  = 0xC35A69A5, /* Legacy name */

    HI_ERR_MALLOC_FAILURE                           = 0x80000001,   /*!< malloc fail,only malloc fail from mem pool
                                                                        can use this err code */
    HI_ERR_TIMEOUT                                  = 0x80000002,   /*!< time out */
    HI_ERR_MEMCPYS_FAIL                             = 0x80000003,
    HI_ERR_MEMSETS_FAIL                             = 0x80000004,
    HI_ERR_INSUFFICIENT_DATA                        = 0x80000007,
    HI_ERR_INCORRECT_ID                             = 0x80000008,
    HI_ERR_INCORRECT_VER                            = 0x80000009,
    HI_ERR_BAD_CHECKSUM                             = 0x8000000A,
    HI_ERR_BLOCKED                                  = 0x8000000B,
    HI_ERR_WRONG_CORE                               = 0x8000000C,
    HI_ERR_NOT_INITIALISED                          = 0x8000000D,
    HI_ERR_INVALID_STATE                            = 0x8000000E,
    HI_ERR_NOT_IMPLEMENTED                          = 0x8000000F,
    /*
    * 2 osa (0x8000_0080 - 0x8000_1000)
    */
    /* TASK */
    HI_ERR_TASK_INVALID_PARAM                       = 0x80000081,
    HI_ERR_TASK_CREATE_FAIL                         = 0x80000082,
    HI_ERR_TASK_DELETE_FAIL                         = 0x80000083,
    HI_ERR_TASK_SUPPEND_FAIL                        = 0x80000084,
    HI_ERR_TASK_RESUME_FAIL                         = 0x80000085,
    HI_ERR_TASK_GET_PRI_FAIL                        = 0x80000086,
    HI_ERR_TASK_SET_PRI_FAIL                        = 0x80000087,
    HI_ERR_TASK_LOCK_FAIL                           = 0x80000088,
    HI_ERR_TASK_UNLOCK_FAIL                         = 0x80000089,
    HI_ERR_TASK_DELAY_FAIL                          = 0x8000008A,
    HI_ERR_TASK_GET_INFO_FAIL                       = 0x8000008B,
    HI_ERR_TASK_REGISTER_SCHEDULE_FAIL              = 0x8000008C,
    HI_ERR_TASK_NOT_CREATED                         = 0x8000008D,

    /* ISR */
    HI_ERR_ISR_INVALID_PARAM                        = 0x800000C1,
    HI_ERR_ISR_REQ_IRQ_FAIL                         = 0x800000C2,
    HI_ERR_ISR_ADD_JOB_MALLOC_FAIL                  = 0x800000C3,
    HI_ERR_ISR_ADD_JOB_SYS_FAIL                     = 0x800000C4,
    HI_ERR_ISR_FREE_IRQ_FAIL                        = 0x800000C5,
    HI_ERR_ISR_ALREADY_CREATED                      = 0x800000C6,
    HI_ERR_ISR_NOT_CREATED                          = 0x800000C7,
    HI_ERR_ISR_ENABLE_IRQ_FAIL                      = 0x800000C8,
    HI_ERR_ISR_IRQ_ADDR_NOK                         = 0x800000C9,
    HI_ERR_ISR_SET_PRI_FAIL                         = 0x800000CA,

    /* MEM */
    HI_ERR_MEM_INVALID_PARAM                        = 0x80000101, /*!< invalid param return from mem module */
    HI_ERR_MEM_CREAT_POOL_FAIL                      = 0x80000102,
    HI_ERR_MEM_CREATE_POOL_NOT_ENOUGH_HANDLE        = 0x80000103,
    HI_ERR_MEM_FREE_FAIL                            = 0x80000104,
    HI_ERR_MEM_RE_INIT                              = 0x80000105,
    HI_ERR_MEM_NOT_INIT                             = 0x80000106,
    HI_ERR_MEM_CREAT_POOL_MALLOC_FAIL               = 0x80000107,
    HI_ERR_MEM_GET_INFO_FAIL                        = 0x80000108,
    HI_ERR_MEM_GET_OS_INFO_NOK                      = 0x80000109,

    /* OS Timer */
    HI_ERR_TIMER_FAILURE                            = 0x80000141,
    HI_ERR_TIMER_INVALID_PARAM                      = 0x80000142,
    HI_ERR_TIMER_CREATE_HANDLE_FAIL                 = 0x80000143,
    HI_ERR_TIMER_START_FAIL                         = 0x80000144,
    HI_ERR_TIMER_HANDLE_NOT_CREATE                  = 0x80000145,
    HI_ERR_TIMER_HANDLE_INVALID                     = 0x80000146,
    HI_ERR_TIMER_STATUS_INVALID                     = 0x80000147,
    HI_ERR_TIMER_STATUS_START                       = 0x80000148,
    HI_ERR_TIMER_INVALID_MODE                       = 0x80000149,
    HI_ERR_TIMER_EXPIRE_INVALID                     = 0x8000014A,
    HI_ERR_TIMER_FUNCTION_NULL                      = 0x8000014B,
    HI_ERR_TIMER_HANDLE_MAXSIZE                     = 0x8000014C,
    HI_ERR_TIMER_MALLOC_FAIL                        = 0x8000014D,
    HI_ERR_TIMER_NOT_INIT                           = 0x8000014E,

    /* SEM */
    HI_ERR_SEM_INVALID_PARAM                        = 0x80000181,
    HI_ERR_SEM_CREATE_FAIL                          = 0x80000182,
    HI_ERR_SEM_DELETE_FAIL                          = 0x80000183,
    HI_ERR_SEM_WAIT_FAIL                            = 0x80000184,
    HI_ERR_SEM_SIG_FAIL                             = 0x80000185,
    HI_ERR_SEM_WAIT_TIME_OUT                        = 0x80000186,

    /* MUX */
    HI_ERR_MUX_INVALID_PARAM                        = 0x800001C1,
    HI_ERR_MUX_CREATE_FAIL                          = 0x800001C2,
    HI_ERR_MUX_DELETE_FAIL                          = 0x800001C3,
    HI_ERR_MUX_PEND_FAIL                            = 0x800001C4,
    HI_ERR_MUX_POST_FAIL                            = 0x800001C5,

    /* MSG */
    HI_ERR_MSG_INVALID_PARAM                        = 0x80000201,
    HI_ERR_MSG_CREATE_Q_FAIL                        = 0x80000202,
    HI_ERR_MSG_DELETE_Q_FAIL                        = 0x80000203,
    HI_ERR_MSG_WAIT_FAIL                            = 0x80000204,
    HI_ERR_MSG_SEND_FAIL                            = 0x80000205,
    HI_ERR_MSG_GET_Q_INFO_FAIL                      = 0x80000206,
    HI_ERR_MSG_Q_DELETE_FAIL                        = 0x80000207,
    HI_ERR_MSG_WAIT_TIME_OUT                        = 0x80000208,
    HI_ERR_MSG_QUEUE_ISFULL                         = 0x80000209,
    HI_ERR_MSG_QUEUE_NOT_CREATE                     = 0x8000020A,

    /* EVENT */
    HI_ERR_EVENT_INVALID_PARAM                      = 0x80000241,
    HI_ERR_EVENT_CREATE_NO_HADNLE                   = 0x80000242,
    HI_ERR_EVENT_CREATE_SYS_FAIL                    = 0x80000243,
    HI_ERR_EVENT_SEND_FAIL                          = 0x80000244,
    HI_ERR_EVENT_WAIT_FAIL                          = 0x80000245,
    HI_ERR_EVENT_CLEAR_FAIL                         = 0x80000246,
    HI_ERR_EVENT_RE_INIT                            = 0x80000247,
    HI_ERR_EVENT_NOT_ENOUGH_MEMORY                  = 0x80000248,
    HI_ERR_EVENT_NOT_INIT                           = 0x80000249,
    HI_ERR_EVENT_DELETE_FAIL                        = 0x8000024A,
    HI_ERR_EVENT_WAIT_TIME_OUT                      = 0x8000024B,
    HI_ERR_EVENT_CREATE_NO_ENOUGH_HADNLE            = 0x8000024C,

    /* SPINLOCK */
    HI_ERR_SPINLOCK_INVALID_PARAM                   = 0x80000251,
    HI_ERR_SPINLOCK_NOT_ENOUGH_MEMORY               = 0x80000252,
    HI_ERR_SPINLOCK_NOT_INIT                        = 0x80000253,

    /* OS Stat */
    HI_ERR_OS_STAT_INVALID_PARAM                    = 0x80000281,
    HI_ERR_OS_STAT_SYSTEM_CALL_ERROR                = 0x80000282,

    /* km */
    HI_ERR_KM_INVALID_PARAMETER                     = 0x80000300,
    HI_ERR_KM_ENV_NOT_READY                         = 0x80000301,
    HI_ERR_KM_KSLOT_BUSY                            = 0x80000302,
    HI_ERR_KM_TIMEOUT                               = 0x80000303,
    HI_ERR_KM_KLAD_BUSY                             = 0x80000304,
    HI_ERR_KM_RKP_BUSY                              = 0x80000305,

    /* otp */
    HI_ERR_OTP_INVALID_PARAMETER                    = 0x80000320,
    HI_ERR_OTP_BUSY                                 = 0x80000321,
    HI_ERR_OTP_ADDR_NOT_ALIGNED                     = 0x80000322,
    HI_ERR_OTP_TIMEOUT                              = 0x80000323,
    HI_ERR_OTP_PERMISSION_FAILURE                   = 0x80000324,

    /* cipher (block 13) */
    HI_ERR_CIPHER_INVALID_PARAMETER                 = 0x80000340,
    HI_ERR_CIPHER_UNSUPPORTED                       = 0x80000341,
    HI_ERR_CIPHER_ENV_NOT_READY                     = 0x80000342,
    HI_ERR_CIPHER_BUSY                              = 0x80000343,
    HI_ERR_CIPHER_TIMEOUT                           = 0x80000344,
    HI_ERR_CIPHER_FAILED_MEM                        = 0x80000345,
    HI_ERR_CIPHER_ADDR_NOT_ALIGNED                  = 0x80000346,
    HI_ERR_CIPHER_SIZE_NOT_ALIGNED                  = 0x80000347,
    HI_ERR_CIPHER_OVERFLOW                          = 0x80000348,

    /* HASH */
    HI_ERR_HASH_INVALID_PARAMETER                   = 0x80000350,
    HI_ERR_HASH_UNSUPPORTED                         = 0x80000351,
    HI_ERR_HASH_ADDR_NOT_ALIGNED                    = 0x80000352,
    HI_ERR_HASH_SIZE_NOT_ALIGNED                    = 0x80000353,
    HI_ERR_HASH_ENV_NOT_READY                       = 0x80000354,
    HI_ERR_HASH_BUSY                                = 0x80000355,
    HI_ERR_HASH_TIMEOUT                             = 0x80000356,
    HI_ERR_HASH_FAILED_MEM                          = 0x80000357,
    HI_ERR_HASH_OVERFLOW                            = 0x80000358,

    /* COMMON */
    HI_ERR_INVALID_CPU                              = 0x80000360,
    HI_ERR_FAPC_INVALID_PARAMETER                   = 0x80000361,
    HI_ERR_RAPC_INVALID_PARAMETER                   = 0x80000362,

    /* PKE (block 14) */
    HI_ERR_PKE_INVALID_PARAMETER                    = 0x80000380,
    HI_ERR_PKE_ENV_NOT_READY                        = 0x80000381,
    HI_ERR_PKE_TIMEOUT                              = 0x80000382,
    HI_ERR_PKE_BUSY                                 = 0x80000383,
    HI_ERR_PKE_UNSUPPORTED                          = 0x80000384,
    HI_ERR_PKE_ROBUST_WARNING                       = 0x80000385,
    HI_ERR_PKE_INVALID_PADDING                      = 0x80000386,
    HI_ERR_PKE_BUF_TOO_SMALL                        = 0x80000387,

    /* File System */
    HI_ERR_FS_INVALID_PARAM                         = 0x80000401,
    HI_ERR_FS_NO_DEVICE                             = 0x80000402,
    HI_ERR_FS_NO_SPACE                              = 0x80000403, /* No space left on device */
    HI_ERR_FS_BAD_DESCRIPTOR                        = 0x80000404,
    HI_ERR_FS_FILE_EXISTS                           = 0x80000405,
    HI_ERR_FS_NOT_FOUND                             = 0x80000406,
    HI_ERR_FS_NAME_TOO_LONG                         = 0x80000407,
    HI_ERR_FS_READ_ONLY_FS                          = 0x80000408, /* Read-only file system */
    HI_ERR_FS_IO_ERROR                              = 0x80000409,
    HI_ERR_FS_NO_MORE_FILES                         = 0x8000040A,

    /*
    * 3 DRIVE (0x8000_1000 - 0x8000_3000)
    *
    * IO moudle
    */
    HI_ERR_IO_NOT_INIT                              = 0x80001041,
    HI_ERR_IO_INVALID_PARAMETER                     = 0x80001042,
    HI_ERR_IO_RELEASE_ERROR                         = 0x80001043,
    HI_ERR_IO_SET_FUNC_ERROR                        = 0x80001044,
    HI_ERR_IO_GET_FUNC_ERROR                        = 0x80001045,
    HI_ERR_IO_GET_PULL_ERROR                        = 0x80001046,
    HI_ERR_IO_GET_STRENGTH_ERROR                    = 0x80001047,
    HI_ERR_IO_GET_SLEW_ERROR                        = 0x80001048,
    HI_ERR_IO_GET_TRIGGER_ERROR                     = 0x80001049,
    HI_ERR_IO_FAILURE                               = 0x8000104A,

    /* GPIO moudle */
    HI_ERR_GPIO_NOT_INIT                            = 0x80001081,
    HI_ERR_GPIO_INVALID_PARAMETER                   = 0x80001082,
    HI_ERR_GPIO_PIN_ERROR                           = 0x80001083,
    HI_ERR_GPIO_DIR_ERROR                           = 0x80001084,
    HI_ERR_GPIO_FAILURE                             = 0x80001085,

    /* RTC moudle */
    HI_ERR_RTC_NOT_INIT                             = 0x80001091,
    HI_ERR_RTC_INVALID_PARAMETER                    = 0x80001092,
    HI_ERR_RTC_NO_VALID                             = 0x80001093,
    HI_ERR_RTC_FAILURE                              = 0x80001094,

	/* CALENDAR moudle */
    HI_ERR_CALENDAR_NOT_INIT						= 0x80001095,
    HI_ERR_CALENDAR_INVALID_PARAMETER				= 0x80001096,
    HI_ERR_CALENDAR_NO_VALID						= 0x80001097,
    HI_ERR_CALENDAR_FAILURE 						= 0x80001098,

    /* UART moudle */
    HI_ERR_UART_INVALID_PARAMETER                   = 0x800010C1,
    HI_ERR_UART_INVALID_SUSPEND                     = 0x800010C2,
    HI_ERR_UART_INVALID_PARITY                      = 0x800010C3,
    HI_ERR_UART_INVALID_DATA_BITS                   = 0x800010C4,
    HI_ERR_UART_INVALID_STOP_BITS                   = 0x800010C5,
    HI_ERR_UART_INVALID_BAUD                        = 0x800010C6,
    HI_ERR_UART_INVALID_COM_PORT                    = 0x800010C7,
    HI_ERR_UART_SW_BUF_CREATE_FAIL                  = 0x800010C8,
    HI_ERR_UART_NOT_SUPPORT                         = 0x800010C9,
    HI_ERR_UART_INVALID_ID                          = 0x800010CA,
    HI_ERR_UART_CREATE_EVENT_FAIL                   = 0x800010CB,
    HI_ERR_UART_OPEN_TX_DMA_CHN_FAIL                = 0x800010CC,
    HI_ERR_UART_CLOSE_TX_DMA_CHN_FAIL               = 0x800010CD,
    HI_ERR_UART_TX_DMA_NOT_ENABLED                  = 0x800010CE,
    HI_ERR_UART_CFG_NOT_SUPPORT_NONE_FIFO           = 0x800010CF,
    HI_ERR_UART_DRIVER_RESOURCE_REINIT              = 0x800010D1,   /*!< uart driver resource have initialized */
    HI_ERR_UART_DEVICE_RESOURCE_REINIT              = 0x800010D2,   /*!< uart device resource have initialized */
    HI_ERR_UART_INVALID_FLOW_CTRL                   = 0x800010D3,
    HI_ERR_UART_BUF_DEINIT_FAIL                     = 0x800010D4,
    HI_ERR_UART_DEINIT_WRITTING_READING             = 0x800010D5,
    HI_ERR_UART_SEND_XON_FAIL                       = 0x800010D6,
    HI_ERR_UART_SEND_XOFF_FAIL                      = 0x800010D7,
    HI_ERR_UART_MEM_OPT_FAIL                        = 0x800010D8,
    HI_ERR_UART_RX_BUF_NOT_CREATED                  = 0x800010D9,

    /* SPI moudle */
    HI_ERR_SPI_NOT_INIT                             = 0x80001101,
    HI_ERR_SPI_INVALID_PARAMETER                    = 0x80001102,
    HI_ERR_SPI_PIN_ERROR                            = 0x80001103,
    HI_ERR_SPI_BUSY                                 = 0x80001104,
    HI_ERR_SPI_FAILURE                              = 0x80001105,
    HI_ERR_SPI_ALREADY_INIT                         = 0x80001106,
    HI_ERR_SPI_WRITE_TIMEOUT                        = 0x80001107,
    HI_ERR_SPI_READ_TIMEOUT                         = 0x80001108,
    HI_ERR_SPI_MODE_ERROR                           = 0x80001109,
    HI_ERR_SPI_DMA_OPEN_FAILURE                     = 0x8000110A,
    HI_ERR_SPI_DMA_TRANSFER_FAILURE                 = 0x8000110B,
    HI_ERR_SPI_CORE_ERROR                           = 0x8000110C,

    /* efuse */
    HI_ERR_EFUSE_INVALIDATE_ID                      = 0x80001121,
    HI_ERR_EFUSE_INVALIDATE_PARA                    = 0x80001122,
    HI_ERR_EFUSE_WRITE_ERR                          = 0x80001123,
    HI_ERR_EFUSE_INVALIDATE_AUTH                    = 0x80001124,
    HI_ERR_EFUSE_BUSY                               = 0x80001125,
    HI_ERR_EFUSE_TIMEOUT                            = 0x80001126,
    HI_ERR_EFUSE_STATE_ADDR                         = 0x80001127,
    HI_ERR_EFUSE_WRITE_ALL_ZERO                     = 0x80001128,

    /* I2C moudle */
    HI_ERR_I2C_NOT_INIT                             = 0x80001141,
    HI_ERR_I2C_INVALID_PARAMETER                    = 0x80001142,
    HI_ERR_I2C_PIN_ERROR                            = 0x80001143,
    HI_ERR_I2C_BUSY_ERROR                           = 0x80001144,
    HI_ERR_I2C_SEND_DATA_ERROR                      = 0x80001145,
    HI_ERR_I2C_RECEIVE_DATA_ERROR                   = 0x80001146,
    HI_ERR_I2C_ACK_ERROR                            = 0x80001147,
    HI_ERR_I2C_ARB_ERROR                            = 0x80001148,
    HI_ERR_I2C_TIMEOUT_ERROR                        = 0x80001149,
    HI_ERR_I2C_FAILURE                              = 0x8000114A,
    HI_ERR_I2C_TIMEOUT_START                        = 0x8000114B,
    HI_ERR_I2C_TIMEOUT_WAIT                         = 0x8000114C,
    HI_ERR_I2C_TIMEOUT_STOP                         = 0x8000114F,
    HI_ERR_I2C_TIMEOUT_RCV_BYTE                     = 0x80001150,
    HI_ERR_I2C_TIMEOUT_RCV_BYTE_PROC                = 0x80001151,
    HI_ERR_I2C_WAIT_SEM_FAIL                        = 0x80001152,
    HI_ERR_I2C_START_ACK_ERR                        = 0x80001153,
    HI_ERR_I2C_WAIT_ACK_ERR                         = 0x80001154,
    HI_ERR_I2C_INITED                               = 0x80001155,
    HI_ERR_I2C_ARBITRATE_ERR                        = 0x80001156,

    /* i2s moudle */
    HI_ERR_I2S_INVALID_PARAMETER                    = 0x80001161,
    HI_ERR_I2S_WRITE_TIMEOUT                        = 0x80001162,
    HI_ERR_I2S_MALLOC_FAIL                          = 0x80001163,
    HI_ERR_I2S_MEMCPY_FAIL                          = 0x80001164,

    /* PWM moudle */
    HI_ERR_PWM_NOT_INIT                             = 0x80001181,
    HI_ERR_PWM_INVALID_PARAMETER                    = 0x80001182,
    HI_ERR_PWM_PIN_ERROR                            = 0x80001183,
    HI_ERR_PWM_FAILURE                              = 0x80001184,
    HI_ERR_PWM_UNSUPPORT_DUTY_RATIO                 = 0x80001185,
    HI_ERR_PWM_INIT_ALREADY                         = 0x80001186,

    /* Flash moudule */
    HI_ERR_FLASH_NOT_INIT                           = 0x800011C1,
    HI_ERR_FLASH_INVALID_PARAMETER                  = 0x800011C2,
    HI_ERR_FLASH_INVALID_PARAM_BEYOND_ADDR          = 0x800011C3,
    HI_ERR_FLASH_INVALID_PARAM_SIZE_ZERO            = 0x800011C4,
    HI_ERR_FLASH_INVALID_PARAM_ERASE_NOT_ALIGN      = 0x800011C5,
    HI_ERR_FLASH_INVALID_PARAM_IOCTRL_DATA_NULL     = 0x800011C6,
    HI_ERR_FLASH_INVALID_PARAM_DATA_NULL            = 0x800011C7,
    HI_ERR_FLASH_INVALID_PARAM_PKI_MODIFY           = 0x800011C8,
    HI_ERR_FLASH_INVALID_PARAM_PAD1                 = 0x800011C9,
    HI_ERR_FLASH_INVALID_PARAM_PAD2                 = 0x800011CA,
    HI_ERR_FLASH_INVALID_PARAM_PAD3                 = 0x800011CB,
    HI_ERR_FLASH_TIME_OUT_WAIT_READY                = 0x800011CC,
    HI_ERR_FLASH_QUAD_MODE_READ_REG1                = 0x800011CD,
    HI_ERR_FLASH_QUAD_MODE_READ_REG2                = 0x800011CE,
    HI_ERR_FLASH_QUAD_MODE_COMPARE_REG              = 0x800011CF,
    HI_ERR_FLASH_NO_MATCH_FLASH                     = 0x800011D0,
    HI_ERR_FLASH_WRITE_ENABLE                       = 0x800011D1,
    HI_ERR_FLASH_NO_MATCH_ERASE_SIZE                = 0x800011D2,
    HI_ERR_FLASH_MAX_SPI_OP                         = 0x800011D3,
    HI_ERR_FLASH_NOT_SUPPORT_IOCTRL_ID              = 0x800011D4,
    HI_ERR_FLASH_INVALID_CHIP_ID                    = 0x800011D5,
    HI_ERR_FLASH_RE_INIT                            = 0x800011D6,
    HI_ERR_FLASH_WRITE_NOT_SUPPORT_ERASE            = 0x800011D7,
    HI_ERR_FLASH_WRITE_COMPARE_WRONG                = 0x800011D8,
    HI_ERR_FLASH_WAIT_CFG_START_TIME_OUT            = 0x800011D9,
    HI_ERR_FLASH_PATITION_INIT_FAIL                 = 0x800011DA,
    HI_ERR_FLASH_INITILIZATION                      = 0x800011DB,
    HI_ERR_FLASH_ERASE_NOT_4K_ALIGN                 = 0x800011DC,
    HI_ERR_FLASH_RPC_COMMAND                        = 0x800011DD,
    HI_ERR_FLASH_ADD_WRITE_TASK                     = 0x800011DE,
    HI_ERR_FLASH_ADD_ERASE_TASK                     = 0x800011DF,
    HI_ERR_FLASH_WRITE_TASK                         = 0x800011E0,
    HI_ERR_FLASH_ERASE_TASK                         = 0x800011E1,
    HI_ERR_FLASH_RPC_READ                           = 0x800011E2,

    /* dma */
    HI_ERR_DMA_INVALID_PARA                         = 0x80001201,
    HI_ERR_DMA_NOT_INIT                             = 0x80001202,
    HI_ERR_DMA_BUSY                                 = 0x80001203,
    HI_ERR_DMA_TRANSFER_FAIL                        = 0x80001204,
    HI_ERR_DMA_TRANSFER_TIMEOUT                     = 0x80001205,
    HI_ERR_DMA_GET_NOTE_FAIL                        = 0x80001206,
    HI_ERR_DMA_LLI_NOT_CREATE                       = 0x80001207,
    HI_ERR_DMA_NO_AVAIL_CONTROLLER                  = 0x80001208,
    HI_ERR_DMA_NO_AVAIL_CH                          = 0x80001209,
    HI_ERR_DMA_UNSUPPORTED_PERIPHERAL               = 0x8000120A,
    HI_ERR_DMA_NOT_SUPPORTED_CONFIG                 = 0x8000120B,
    HI_ERR_DMA_ERROR_CONFIG                         = 0x8000120C,
    HI_ERR_DMA_INCOMPLETE_CONFIG                    = 0x8000120D,
    HI_ERR_DMA_ONE_TIME_BATCHED_CONFIGS_EXCEEDED    = 0x8000120E,
    HI_ERR_DMA_CONFIG_QUEUE_FULL                    = 0x8000120F,
    HI_ERR_DMA_OUT_OF_MEMORY                        = 0x80001210,
    HI_ERR_DMA_NO_AVAIL_FREE_CONFIG                 = 0x80001211,
    HI_ERR_DMA_CHANNEL_NOT_OPEN                     = 0x80001212,
    HI_ERR_DMA_IRQ_ENABLE_FAIL                      = 0x80001213,
    HI_ERR_DMA_LLI_NOTE_FULL                        = 0x80001214,
    HI_ERR_DMA_SRC_PERIPHERAL                       = 0x80001215,
    HI_ERR_DMA_DST_PERIPHERAL                       = 0x80001216,
    HI_ERR_DMA_PERIPHERAL                           = 0x80001217,
    HI_ERR_DMA_CHANNEL                              = 0x80001218,
    HI_ERR_DMA_DST_WIDTH                            = 0x80001219,
    HI_ERR_DMA_ADDRESS                              = 0x8000121A,
    HI_ERR_DMA_PERIPHERAL_CONFIG                    = 0x8000121B,
    HI_ERR_DMA_LLI_ADDRESS                          = 0x8000121C,
    HI_ERR_DMA_DEVICE_NOT_INIT                      = 0x8000121D,
    HI_ERR_DMA_TRANSFER_TYPE                        = 0x8000121E,

    /* WDT */
    HI_ERR_WDT_PARA_ERROR                           = 0x80001241,
    HI_ERR_WDT_INTERVAL_INVALID                     = 0x80001242,
    HI_ERR_WDT_IRQ_ENABLE_FAILED                    = 0x80001243,

    /* HWI */
    HI_ERR_HWI_INVALID_NUM                          = 0x80001281,

    /* timer */
    HI_ERR_TIMER_NO_VALID_TIMER                     = 0x800012C1,
    HI_ERR_TIMER_INVALID_PARAMETER                  = 0x800012C2,

    /* hw_copy */
    HI_ERR_HW_COPY_INVALID_PARA                     = 0x80001301,
    HI_ERR_HW_COPY_BUSY                             = 0x80001302,

    /* adc */
    HI_ERR_ADC_PARAMETER_WRONG                      = 0x80001311,
    HI_ERR_ADC_INVALID_CHANNEL_ID                   = 0x80001312,
    HI_ERR_ADC_TIMEOUT                              = 0x80001313,
    HI_ERR_ADC_NOT_INIT                             = 0x80001314,

    /* swi */
    HI_ERR_SWI_INVALID_ID                           = 0x80001341,

    /* ANALOG */
    HI_ERR_ANALOG_MUTEX_ERROR                       = 0x80001381,
    HI_ERR_ANALOG_RESOURCE_BUSY                     = 0x80001382,
    HI_ERR_ANALOG_RESOURCE_CONNECT_ERROR            = 0x80001383,
    HI_ERR_ANALOG_RESOURCE_FREE_ERROR               = 0x80001384,
    HI_ERR_ANALOG_VOL_TYPE_ERROR                    = 0x80001385,
    HI_ERR_ANALOG_READ_VOL_ERROR                    = 0x80001386,
    HI_ERR_ANALOG_AIO_PIN_ERROR                     = 0x80001387,
    HI_ERR_ANALOG_IDRV_INVALID_RANGE                = 0x80001388,
    HI_ERR_ANALOG_COMP_NUMBER_ERROR                 = 0x80001389,
    HI_ERR_ANALOG_COMP_INVALID_RANGE                = 0x8000138A,
    HI_ERR_ANALOG_VALUE_SET_ERROR                   = 0x8000138B,
    HI_ERR_ANALOG_CORE_NOT_SUPPORT                  = 0x8000138C,
    HI_ERR_ANALOG_PARAMETER_ERROR                   = 0x8000138D,
    HI_ERR_ANALOG_FUNC_UNREADY_ERROR                = 0x8000138E,
    HI_ERR_ANALOG_ADC_TIME_OUT_ERROR                = 0x8000138F,
    HI_ERR_ANALOG_ADC_NOT_CALIBRATE_ERROR           = 0x80001391,
    HI_ERR_ANALOG_RESOURCE_NOT_OWNED_ERROR          = 0x80001392,
    HI_ERR_ANALOG_COMP_NOT_SET_AIO_PIN              = 0x80001393,
    HI_ERR_ANALOG_OTP_VALUE_GET_ERROR               = 0x80001394,
    HI_ERR_ANALOG_AIO_FUNC_ERROR                    = 0x80001395,

    /* YMODEM */
    HI_ERR_YMODEM_ERR_TIMEOUT                       = 0x80001401,
    HI_ERR_YMODEM_ERR_CANCEL                        = 0x80001402,
    HI_ERR_YMODEM_ERR_FRAME                         = 0x80001403,
    HI_ERR_YMODEM_ERR_CS                            = 0x80001404,
    HI_ERR_YMODEM_ERR_EOT                           = 0x80001405,
    HI_ERR_YMODEM_ERR_SEQ                           = 0x80001406,
    HI_ERR_YMODEM_ERR_LENTH                         = 0x80001407,

    /* upgrade common error */
    HI_ERR_UPG_COMMON                               = 0x80001501,
    HI_ERR_UPG_LOW_KERNEL_VER                       = 0x80001502,
    HI_ERR_UPG_FULL_KERNEL_VER                      = 0x80001503,
    HI_ERR_UPG_LOW_BOOT_VER                         = 0x80001504,
    HI_ERR_UPG_FULL_BOOT_VER                        = 0x80001505,
    HI_ERR_UPG_FIRST_PACKET_OFFSET                  = 0x80001506,
    HI_ERR_UPG_UPDATE_VER_TIMEOUT                   = 0x80001507,
    HI_ERR_UPG_UPDATE_VER_FAIL                      = 0x80001508,
    HI_ERR_UPG_UPDATE_VER_INVALID_PARAM             = 0x80001509,
    HI_ERR_UPG_KERNEL_VER_OVER                      = 0x8000150A,
    HI_ERR_UPG_BOOT_VER_OVER                        = 0x8000150B,

    /* IPC */
    HI_ERR_IPC_MSG_FULL                             = 0x80001541,
    HI_ERR_IPC_MSG_ADDR_NULL                        = 0x80001542,
    HI_ERR_IPC_MSG_ZEROCOPY_WRONG                   = 0x80001543,
    HI_ERR_IPC_MSG_ALREADY_INIT                     = 0x80001544,
    HI_ERR_IPC_MSG_TAKE_SEM_FAIL                    = 0x80001545,
    HI_ERR_IPC_MSG_SEND_BUSY                        = 0x80001546,
    HI_ERR_IPC_MSG_GIVE_SEM_FAIL                    = 0x80001547,
    HI_ERR_IPC_MSG_INVALID_PARAM                    = 0x80001548,
    HI_ERR_IPC_MSG_PRI_QUEUE_FULL                   = 0x80001549,
    HI_ERR_IPC_MSG_PRI_QUEUE_EMPTY                  = 0x8000154A,
    HI_ERR_IPC_MSG_RECV_INVALID_ID                  = 0x8000154B,
    HI_ERR_IPC_MSG_PROCESS_FAILED                   = 0x8000154C,
    HI_ERR_IPC_MSG_MEM_QUEUE_EMPTY                  = 0x8000154D,
    HI_ERR_IPC_QUEUE_INVALID_ID                     = 0x8000154E,
    HI_ERR_IPC_QUEUE_ADDR_NULL                      = 0x8000154F,
    HI_ERR_IPC_QUEUE_FULL                           = 0x80001550,
    HI_ERR_IPC_QUEUE_EMPTY                          = 0x80001551,
    HI_ERR_IPC_QUEUE_NOT_INIT                       = 0x80001552,
    HI_ERR_IPC_SHM_REPEATE_RELEASE                  = 0x80001553,
    HI_ERR_IPC_SHM_INVALID_ID                       = 0x80001554,
    HI_ERR_IPC_SHM_NULL_POITER                      = 0x80001555,
    HI_ERR_IPC_SHM_ILLEGAL_POINTER                  = 0x80001556,
    HI_ERR_IPC_SHM_INVALID_POINTER                  = 0x80001557,
    HI_ERR_IPC_SHM_NOT_INIT                         = 0x80001558,
    HI_ERR_IPC_MEMCPY_EXCEPTION                     = 0x80001559,
    HI_ERR_IPC_MBX_BUSY                             = 0x8000155A,

    /* MIPI TX1 */
    HI_ERR_MIPI_NOT_INIT                            = 0x80001581,
    HI_ERR_MIPI_IRQ_REQUEST_FAILED                  = 0x80001582,
    HI_ERR_MIPI_IRQ_ENABLE_FAILED                   = 0x80001583,
    HI_ERR_MIPI_CREATE_MUX_FAIL                     = 0x80001584,
    HI_ERR_MIPI_PEND_MUX_FAIL                       = 0x80001585,
    HI_ERR_MIPI_NULL_PARAM                          = 0x80001586,
    HI_ERR_MIPI_INVALID_CMD_INFO                    = 0x80001587,
    HI_ERR_MIPI_MALLOC_FAIL                         = 0x80001588,
    HI_ERR_MIPI_MEMCPY_FAIL                         = 0x80001589,
    HI_ERR_MIPI_INCALID_IOCTL_CMD                   = 0x8000158A,
    HI_ERR_MIPI_INVALID_RD_CMD_INFO                 = 0x8000158B,
    HI_ERR_MIPI_WAIT_RD_FIFO_EMPTY                  = 0x8000158C,
    HI_ERR_MIPI_GET_DATA_NULL                       = 0x8000158D,
    HI_ERR_MIPI_SEND_SHORT_PACKET                   = 0x8000158E,
    HI_ERR_MIPI_READ_FIFO_DATA                      = 0x8000158F,
    HI_ERR_MIPI_COMBO_DEV_NOT_CONFIG                = 0x80001590,
    HI_ERR_MIPI_DEV_ALREADY_ENABLE                  = 0x80001591,
    HI_ERR_MIPI_DEV_NOT_ENABLE                      = 0x80001592,

    /* MIPI TX2 */
    HI_ERR_MIPI_ALREADY_INIT                        = 0x800015C1,
    HI_ERR_MIPI_INVALID_COLORBAR_ORIEN              = 0x800015C2,
    HI_ERR_MIPI_INVALID_COLORBAR_INPUT_MODE         = 0x800015C3,

    /* SDIO */
    HI_ERR_SDIO_INVALID_PARAMETER                   = 0x80001600,
    HI_ERR_SDIO_ALREADY_START                       = 0x80001601,
    HI_ERR_SDIO_PROC_TASK_INIT_FAIL                 = 0x80001602,

    /* SDCC */
    HI_ERR_SDCC_INVALID_PARAMETER                   = 0x80001610,
    HI_ERR_SDCC_ALREADY_START                       = 0x80001611,
    HI_ERR_SDCC_PROC_TASK_INIT_FAIL                 = 0x80001612,

    /* QSPI */
    HI_ERR_QSPI_INVALID_PARAM                       = 0x80001640,
    HI_ERR_QSPI_INVALID_PARAM_BEYOND_ADDR           = 0x80001641,
    HI_ERR_QSPI_INVALID_PARAM_SIZE_ZERO             = 0x80001642,
    HI_ERR_QSPI_WRITE_COMPARE_WRONG                 = 0x80001643,
    HI_ERR_QSPI_RE_INIT                             = 0x80001644,
    HI_ERR_QSPI_INVALID_PARAM_DATA_NULL             = 0x80001645,
    HI_ERR_QSPI_INVALID_CMD_DUMMY_BYTE              = 0x80001646,
    HI_ERR_QSPI_INVALID_CMD_SPI_IFTYPE              = 0x80001647,
    HI_ERR_QSPI_INVALID_CMD_DATA_SIZE               = 0x80001648,
    HI_ERR_QSPI_INVALID_BUS_RECV_DATA_SIZE          = 0x80001649,
    HI_ERR_QSPI_INVALID_BUS_RECV_DUMMY_BYTE         = 0x8000164A,
    HI_ERR_QSPI_INVALID_BUS_RECV_SPI_IFTYPE         = 0x8000164B,
    HI_ERR_QSPI_INVALID_BUS_TRANS_DATA_SIZE         = 0x8000164C,
    HI_ERR_QSPI_INVALID_BUS_TRANS_DUMMY_BYTE        = 0x8000164D,
    HI_ERR_QSPI_INVALID_BUS_TRANS_SPI_IFTYPE        = 0x8000164E,
    HI_ERR_QSPI_INVALID_BUS_MAP_SIZE                = 0x8000164F,
    HI_ERR_QSPI_INVALID_GLOBAL_SPI_MODE             = 0x80001650,
    HI_ERR_QSPI_NOT_INIT                            = 0x80001651,

     /* HW TIMER */
    HI_ERR_HWTIMER_NO_INIT                          = 0x80001680,
    HI_ERR_HWTIMER_INVALID_PARAMETER                = 0x80001681,
    HI_ERR_HWTIMER_INITILIZATION_ALREADY            = 0x80001682,

    /* ETH */
    HI_ERR_ETH_INVALID_PARAMETER                    = 0x800016C0,

    /* DNTC */
    HI_ERR_DNTC_INVALID_PARAMETER                   = 0x80001700,
    HI_ERR_DNTC_INVALID_PARAMETER_PINNUM            = 0x80001701,
    HI_ERR_DNTC_INVALID_PARAMETER_TEMPCODE          = 0x80001702,
    HI_ERR_DNTC_INIT_ALREADY                        = 0x80001703,
    HI_ERR_DNTC_INIT_FAILURE                        = 0x80001704,
    HI_ERR_DNTC_INIT_NULL                           = 0x80001705,
    HI_ERR_DNTC_DEINIT_ALREADY                      = 0x80001706,
    HI_ERR_DNTC_NOT_INIT                            = 0x80001707,
    HI_ERR_DNTC_PDOWN_FAILURE                       = 0x80001708,
    HI_ERR_DNTC_REGISTER_ALREADY                    = 0x80001709,
    HI_ERR_DNTC_READ_FAILURE                        = 0x8000170A,

    /* TSENSOR */
    HI_ERR_TSENSOR_INVALID_PARAMETER                = 0x80001740,
    HI_ERR_TSENSOR_NOT_INIT                         = 0x80001741,

    /* CLK */
    HI_ERR_CLK_INVALID_PARAMETER                    = 0x80001780,
    HI_ERR_CLK_NOT_REGISTER                         = 0x80001781,

    /* memory */
    HI_ERR_MEMORY_INVALID_REGION                    = 0x80001A01,
    HI_ERR_MEMORY_INVALID_PARAMETER                 = 0x80001A02,
    HI_ERR_MEMORY_NOT_ALIGNED                       = 0x80001A03,

    /**
     * This is a flash task state set by the owning core to indicate that this entry should be discarded by the SCPU.
     * The owning core must not simply delete an entry as the SCPU may have already taken a reference to the memory and
     * be periodically checking that for READY to be set.  Value must remain identical to the flash_task_etypes.h
     * value.
     */
    HI_ERR_FLASH_TASK_DISCARD                       = 0x80002040,
    /**
     * This is a flash task state set by the SCPU to confirm that the task entry was discarded by the SCPU (either
     * because the owning core specified a DISCARD or because the value was illegal according to the SCPU) to indicate
     * that the flash task was skipped and not executed.  Value must remain identical to the flash_task_etypes.h value.
     */
    HI_ERR_FLASH_TASK_DISCARDED                     = 0x80002041,
    /**
     * This is a flash task state set by the owning core to inform the SCPU that this flash task can now be fully
     * examined and executed by the SCPU.  Value must remain identical to the flash_task_etypes.h value.
     */
    HI_ERR_FLASH_TASK_READY                         = 0x80002042,
    /**
     * This is a flash task state set by the SCPU to record that this flash task is being worked on by the SCPU. The
     * owning core will only see this if it wakes up unexpectedly before the anticipated wake-up time set in the sleep
     * co-ordination system.  Value must remain identical to the flash_task_etypes.h value.
     */
    HI_ERR_FLASH_TASK_BEING_PROCESSED               = 0x80002043,
    /**
     * This is a flash task state set by the SCPU to indicate that the flash task has been completed and the SCPU has
     * dropped all references to the flash task so that the owning core may reclaim the memory.  Value must remain
     * identical to the flash_task_etypes.h value.
     */
    HI_ERR_FLASH_TASK_COMPLETED                     = 0x80002044,
    /**
     * The flash task struct has an incorrect identifier number used as a sanity check that this memory is what it is
     * expected to be.
     */
    HI_ERR_FLASH_TASK_WRONG_STRUCT_IDENTIFIER       = 0x80002045,
    /** There are no more flash tasks to process */
    HI_ERR_FLASH_TASK_NO_MORE_TASKS                 = 0x80002046,
    /** There are only tasks that have not completed */
    HI_ERR_FLASH_TASK_ONLY_PENDING_TASKS            = 0x80002047,
    /** The task is in progress */
    HI_ERR_FLASH_TASK_IN_PROGRESS                   = 0x80002048,
    /** The specified task is not found */
    HI_ERR_FLASH_TASK_NOT_FOUND                     = 0x80002049,
    /** The operation has already been done */
    HI_ERR_FLASH_TASK_ALREADY_DONE                  = 0x8000204A,
    /** The queue handle was requested */
    HI_ERR_FLASH_TASK_REQUESTED_QUEUE               = 0x8000204B,
    /** The remote core is not ready for this request */
    HI_ERR_FLASH_TASK_REMOTE_CORE_NOT_READY         = 0x8000204C,
    /** Invalid protection key */
    HI_ERR_FLASH_TASK_INVALID_PK                    = 0x8000204D,
    HI_ERR_FLASH_TASK_INIT_FAILED                   = 0x8000204E,
    HI_ERR_FLASH_TASK_PE_VETO                       = 0x8000204F,
    HI_ERR_FLASH_TASK_BUSY                          = 0x80002050,

    /* NEW PWM */
    HI_ERR_NEW_PWM_INVALID_PARAMETER                = 0x80002080,

    /* CORE MUX */
    HI_ERR_CORE_MUX_NOT_CREATED                     = 0x800020C0,
    HI_ERR_CORE_MUX_NOT_ENOUGH                      = 0x800020C1,
    HI_ERR_CORE_MUX_INVALID_ID                      = 0x800020C2,

    /*
    * 4 Component (0x8000_3000 - 0x8000_4000)
    */
    /* NV */
    HI_ERR_NV_NO_ENOUGH_SPACE                       = 0x80003000,
    HI_ERR_NV_INIT_FAILED                           = 0x80003001,
    HI_ERR_NV_INIT_MUTEX_ERR                        = 0x80003002,
    HI_ERR_NV_INIT_SEM_ERR                          = 0x80003003,
    HI_ERR_NV_NOT_SUPPORT                           = 0x80003004,
    HI_ERR_NV_HEAP_EXHAUSTED                        = 0x80003005,
    HI_ERR_NV_INVALID_PARAMS                        = 0x80003006,
    HI_ERR_NV_NOT_FOUND                             = 0x80003007,
    HI_ERR_NV_PAGE_HANDLER_ERROR                    = 0x80003008,
    HI_ERR_NV_PAGE_INIT_ERROR                       = 0x80003009,
    HI_ERR_NV_PAGE_UNINITIALIZED                    = 0x8000300A,
    HI_ERR_NV_KEY_NOT_FOUND                         = 0x8000300B,
    HI_ERR_NV_ILLEGAL_OPERATION                     = 0x8000300C,
    HI_ERR_NV_CACHE_IN_PROGRESS                     = 0x8000300D,
    HI_ERR_NV_CACHE_ALL_WRITTEN                     = 0x8000300E,
    HI_ERR_NV_DEFRAG_STATE_INVALID                  = 0x8000300F,
    HI_ERR_NV_DEFRAG_PAGE_LOC_ERR                   = 0x80003010,
    HI_ERR_NV_DEFRAG_PAGE_IDX_INVALID               = 0x80003011,
    HI_ERR_NV_DEFRAG_PAGE_UNCLEAN                   = 0x80003012,
    HI_ERR_NV_INVALID_STORE                         = 0x80003013,
    HI_ERR_NV_PAGES_ALREADY_ASSIGNED                = 0x80003014,
    HI_ERR_NV_BUFFER_TOO_SMALL                      = 0x80003015,
    HI_ERR_NV_RPC_ERROR                             = 0x80003016,
    HI_ERR_NV_INIT_PAGE_ID_ERR                      = 0x80003017,
    HI_ERR_NV_NO_KEY_DATA                           = 0x80003018,
    HI_ERR_NV_AES_CONFIG_FAIL                       = 0x80003019,
    HI_ERR_NV_AES_ENCRYPT_FAIL                      = 0x8000301A,
    HI_ERR_NV_AES_CLEAR_FAIL                        = 0x8000301B,
    HI_ERR_NV_HASH_INIT_FAIL                        = 0x8000301C,
    HI_ERR_NV_HASH_UPDATE_FAIL                      = 0x8000301D,
    HI_ERR_NV_HASH_FINAL_FAIL                       = 0x8000301E,
    HI_ERR_NV_STATE_ERROR                           = 0x8000301F,
    HI_ERR_NV_NOTIFY_LIST_FULL                      = 0x80003020,
    HI_ERR_NV_NOTIFY_SEGMENT_ERR                    = 0x80003021,
    HI_ERR_NV_READ_FLASH_ERR                        = 0x80003022,
    HI_ERR_NV_SEM_WAIT_ERR                          = 0x80003023,
    HI_ERR_KV_NOT_INITIALISED                      = 0x80003F00,
    HI_ERR_KV_ERROR_IN_PARAMETERS                  = 0x80003F01,
    HI_ERR_KV_ERROR_ERASING_PAGE                   = 0x80003F02,
    HI_ERR_KV_NOT_ENOUGH_SPACE                     = 0x80003F03,
    HI_ERR_KV_KEY_NOT_FOUND                        = 0x80003F04,
    HI_ERR_KV_TRYING_TO_MODIFY_A_PERMANENT_KEY     = 0x80003F05,
    HI_ERR_KV_WRITE_VETOED                         = 0x80003F06,
    HI_ERR_KV_DEFRAGMENTATION_NEEDED               = 0x80003F07,
    HI_ERR_KV_INVALID_STORE                        = 0x80003F08,
    HI_ERR_KV_WRITE_BUFFER_NOT_ALLOCATED           = 0x80003F09,
    HI_ERR_KV_KEY_HEADER_BUFFER_NOT_ALLOCATED      = 0x80003F0A,
    HI_ERR_KV_KEY_DATA_BUFFER_NOT_ALLOCATED        = 0x80003F0B,
    HI_ERR_KV_KEY_HASH_BUFFER_NOT_ALLOCATED        = 0x80003F0C,
    HI_ERR_KV_WRITE_BUFFER_TOO_SMALL               = 0x80003F0D,
    HI_ERR_KV_KEY_HASH_BUFFER_TOO_SMALL            = 0x80003F0E,
    HI_ERR_KV_GET_BUFFER_TOO_SMALL                 = 0x80003F0F,
    HI_ERR_KV_AES_UNAVAILABLE                      = 0x80003F10,
    HI_ERR_KV_HASH_UNAVAILABLE                     = 0x80003F11,
    HI_ERR_KV_INVALID_PAGE                         = 0x80003F12,
    HI_ERR_KV_ZERO_LENGTH_COPY                     = 0x80003F13,
    HI_ERR_KV_LENGTH_MISMATCH                      = 0x80003F14,
    HI_ERR_KV_HASH_MISMATCH                        = 0x80003F15,
    HI_ERR_KV_DATA_MISMATCH                        = 0x80003F16,
    HI_ERR_KV_SEARCH_PATTERN_MISMATCH              = 0x80003F17,
    HI_ERR_KV_SEARCH_KEY_TYPE_MISMATCH             = 0x80003F18,
    HI_ERR_KV_SEARCH_KEY_STATE_MISMATCH            = 0x80003F19,
    HI_ERR_KV_INVALID_KEY_HEADER                   = 0x80003F1A,
    HI_ERR_KV_PAGE_NOT_FOUND                       = 0x80003F1B,
    HI_ERR_KV_STATE_INVALID                        = 0x80003F1C,
    HI_ERR_KV_KEY_NOT_IN_WRITE_QUEUE               = 0x80003F1D,
    HI_ERR_KV_BUFFER_PRIMED_PREMATURELY            = 0x80003F1E,
    HI_ERR_KV_FAILURE                              = 0x80003F1F,

    /* HCC MODULE */
    HI_ERR_HCC_INIT_ERR                             = 0x80003030,
    HI_ERR_HCC_PARAM_ERR                            = 0x80003031,
    HI_ERR_HCC_STATE_OFF                            = 0x80003032,
    HI_ERR_HCC_STATE_EXCEPTION                      = 0x80003033,
    HI_ERR_HCC_BUILD_TX_BUF_ERR                     = 0x80003034,
    HI_ERR_HCC_TX_BUF_ERR                           = 0x80003035,
    HI_ERR_HCC_BUS_ERR                              = 0x80003036,
    HI_ERR_HCC_FC_PRE_PROC_ERR                      = 0x80003037,
    HI_ERR_HCC_HANDLER_ERR                          = 0x80003038,
    HI_ERR_HCC_FC_PROC_UNBLOCK                      = 0x80003039,
    HI_ERR_HCC_FC_PROC_BLOCK                        = 0x8000303A,
    HI_ERR_HCC_HANDLER_REPEAT                       = 0x8000303B,
    HI_ERR_HCC_SERVICE_REGISTER_REPEAT              = 0x8000303C,

    /**/
    HI_ERR_CYCBUF_NOT_ENOUGH_HANDLE                 = 0x80003041,
    HI_ERR_CYCBUF_INVALID_HANDLE                    = 0x80003042,
    HI_ERR_CYCBUF_MALLOC_FAIL                       = 0x80003043,
    HI_ERR_CYCBUF_BUSY                              = 0x80003044,
    HI_ERR_CYCBUF_NO_DATA                           = 0x80003045,
    HI_ERR_CYCBUF_MEMOPT_FAIL                       = 0x80003046,
    HI_ERR_CYCBUF_SUB_INVALID_FREE_LEN              = 0x80003047,
    HI_ERR_CYCBUF_SUB_INVALID_USED_LEN              = 0x80003048,

    HI_ERR_IRQ_REQUEST_FAIL                         = 0x80003081,
    HI_ERR_IRQ_FREE_FAIL                            = 0x80003082,

    HI_ERR_RAND_NOT_SUPPORT_CRYPT_RAND              = 0x800030c1,

    HI_ERR_CORE_MUTEX_INVALID_ID                    = 0x80003101,
    HI_ERR_CORE_MUTEX_GET_FAIL                      = 0x80003102,
    HI_ERR_CORE_MUTEX_GET_TIME_OUT                  = 0x80003103,

    HI_ERR_TRACK_INIT_FAIL                          = 0x80003141,
    HI_ERR_TRACK_START_FAIL                         = 0x80003142,
    HI_ERR_TRACK_DEINIT_FAIL                        = 0x80003143,
    HI_ERR_TRACK_GET_INFO_FAIL                      = 0x80003144,
    HI_ERR_TRACK_RECORD_FAIL                        = 0x80003145,

    /* PARTITION */
    HI_ERR_PARTITION_INIT_ERR                       = 0x80003160,
    HI_ERR_PARTITION_NOT_SUPPORT                    = 0x80003161,
    HI_ERR_PARTITION_INVALID_PARAMS                 = 0x80003162,
    HI_ERR_PARTITION_CONFIG_NOT_FOUND               = 0x80003163,
    HI_ERR_IMAGE_CONFIG_NOT_FOUND                   = 0x80003164,

    /* AT */
    HI_ERR_AT_RECVING                               = 0x80003180,
    HI_ERR_AT_INVALID_PARAMETER                     = 0x80003181,
    HI_ERR_AT_NAME_OR_FUNC_REPEAT_REGISTERED        = 0x80003182,
    HI_ERR_AT_NO_SLEEP                                  = 0x80003183,
    HI_ERR_AT_DEEP_SLEEP                                = 0x80003184,

    /* diag */
    HI_ERR_DIAG_STAT_INVALID_ID                     = 0x800031A1, /* stat id is invalid */
    HI_ERR_DIAG_NO_INITILIZATION                    = 0x800031A2, /* diag not initilizate */
    HI_ERR_DIAG_NOT_FOUND                           = 0x800031A3, /* diag id not found */
    HI_ERR_DIAG_INVALID_PARAMETER                   = 0x800031A4, /* parameter invalid */
    /* If the return value is HI_ERR_DIAG_CONSUMED, it indicates that the user sends a response (local connection)
    to the host through the diag_send_ack_packet_prv API. The DIAG framework does not automatically */
    HI_ERR_DIAG_CONSUMED                            = 0x800031A5,
    HI_ERR_DIAG_TOO_SMALL_BUFFER                    = 0x800031A6, /* too small buffer */
    HI_ERR_DIAG_NO_MORE_DATA                        = 0x800031A7, /* uart receive no more data */
    HI_ERR_DIAG_NOT_ENOUGH_MEMORY                   = 0x800031A8, /* malloc memory fail */
    HI_ERR_DIAG_INVALID_HEAP_ADDR                   = 0x800031A9, /* addr out of heap range */
    HI_ERR_DIAG_NOT_CONNECT                         = 0x800031Aa, /* diag not connect */
    HI_ERR_DIAG_BUSY                                = 0x800031Ab, /* channel is busy */
    HI_ERR_DIAG_TOO_LARGE_FRAME                     = 0x800031Ac, /* too large frame */
    HI_ERR_DIAG_RAM_ALIGN                           = 0x800031Ad, /* ram addr not align */
    HI_ERR_DIAG_NOT_SUPPORT                         = 0x800031Ae, /* not support */
    HI_ERR_DIAG_QUEUE_FULL                          = 0x800031Af, /* queue full */
    HI_ERR_DIAG_MSG_CFG_NOT_ALLOW                   = 0x800031B0, /* msg config allow send msg to pc */
    HI_ERR_DIAG_TOO_LARGE_MSG                       = 0x800031B1, /* too large msg */
    HI_ERR_DIAG_INVALID_CODE_ADDR                   = 0x800031B2, /* invalid code addr */
    HI_ERR_DIAG_OBJ_NOT_FOUND                       = 0x800031B3, /* object not found */
    HI_ERR_DIAG_BLOCK_EMPTY                         = 0x800031B4, /* buffer list is empty */
    HI_ERR_DIAG_NOT_ENOUGH_SHARE_SPACE              = 0x800031B5, /* share space not enough */
    HI_ERR_DIAG_CORRUPT_SHARED_MEMORY               = 0x800031B6, /* share space ctrl is corrupt */
    HI_ERR_DIAG_NO_NEXT_MESSAGE                     = 0x800031B7, /* no more msg in share mempry */
    HI_ERR_DIAG_KV_NOT_SUPPORT_ID                   = 0x800031B8, /* not support kv id */
    HI_ERR_DIAG_BAD_DATA                            = 0x800031B9, /* bad data */

    /* HCC */
    HI_ERR_HCC_INVALID_PARAMETER                    = 0x80003281,
    HI_ERR_HCC_NO_SUCH_DEVICE                       = 0x80003282,
    HI_ERR_HCC_BUS_HANDLE_INVALID                   = 0x80003283,

    /* system status */
    HI_ERR_PROTOCOL_FLASH_VETO                      = 0x800032C1,
    HI_ERR_APPS_FLASH_VETO                          = 0x800032C2,
    HI_ERR_IOMCU_FLASH_VETO                         = 0x800032C3,

    /* system cpup_dfx */
    HI_ERR_CPUP_INVALID_PARAMETER                   = 0x80003300,

    /* GFX */
    HI_ERR_GFX_INVALID_PARAM                        = 0x80004000,
    HI_ERR_GFX_NULL_POINTER                         = 0x80004001,
    HI_ERR_GFX_NO_MEM                               = 0x80004002,
    HI_ERR_GFX_UNSUPPOTRED_OPERATION                = 0x80004003,
    HI_ERR_GFX_OPERATION_TIMEOUT                    = 0x80004004,
    HI_ERR_GFX_OPERATION_INTERRUPT                  = 0x80004005,
    HI_ERR_GFX_INVALID_RESO                         = 0x80004006,
    HI_ERR_GFX_INVALID_FMT                          = 0x80004007,
    HI_ERR_GFX_INVALID_ADDR                         = 0x80004008,
    HI_ERR_GFX_INVALID_STRIDE                       = 0x80004009,
    HI_ERR_GFX_INVALID_RECT                         = 0x8000400a,
    HI_ERR_GFX_INVALID_MODE                         = 0x8000400b,

    /* Cache */
    HI_ERR_CACHE_INVALID_PARAM                      = 0x80004101,

    /*
    * 5 Protocol stack (0x8000_4000 - 0x8000_5000)
    */

    /*
    * 6 Reserved (0x8000_5000 - 0x8000_7000)
    */

    /*
    * 7 Reserved by customers (0x8000_7000 - 0x8000_8000)
    */
    /* PLC */
    HI_ERR_PLC_FULL = 0x80008001,
    HI_ERR_MALLOC_FAILUE = 0x80008002,
    HI_ERR_TF_IDENTIFY_DOING = 0x80008003,
    HI_ERR_NOT_SUPPORT = 0x80008004,
    HI_ERR_BUSY = 0x80008005,
    HI_ERR_NOT_EXIST = 0x80008006,
    HI_ERR_BAD_DATA = 0x80008007,
    HI_ERR_FEATURE_NOT_SUPPORT = 0x80008008,
    HI_ERR_NO_INITILIZATION = 0x80008009,
    HI_ERR_NOT_FOUND_NETWORK = 0x8000800a,
    HI_ERR_CONTINUE = 0x8000800b,
    HI_ERR_MEMCPY_FAIL = 0x8000800c,
    HI_ERR_BAD_FRAME = 0x8000800d,
    HI_ERR_EXIST = 0x8000800e,
    HI_ERR_SKIP = 0x8000800f,
    HI_ERR_INVALID_PARAMETER = 0x80008010,
    HI_ERR_NOT_ENOUGH_MEMORY  = 0x80008011,
    HI_ERR_TOO_SMALL_BUFFER = 0x80008012,
    HI_ERR_CONSUMED = 0x80008013,
    HI_ERR_QUEUE_FULL = 0x80008014,
    HI_ERR_ACCESS_DENIED = 0x80008015,
    HI_ERR_NO_MORE_MEMORY = 0x80008016,
    HI_ERR_QUEUE_EMPTY = 0x80008017,
    HI_ERR_NO_MORE_DATA = 0x80008018,
    /* Touch panel (0x8000_8040 - 0x8000_80C0) */
    HI_ERR_TP_DEV_INFO_NOT_REGISTER                 = 0x80008040,
    HI_ERR_TP_I2C_DEV_NOT_INIT                      = 0x80008041,
    HI_ERR_TP_GPIO_DEV_NOT_INIT                     = 0x80008042,
    HI_ERR_TP_ZTW523_INIT_DATA                      = 0x80008043,
    HI_ERR_TP_ZTW523_INIT_RESET                     = 0x80008044,
    HI_ERR_TP_ZTW523_INIT_POWERUP                   = 0x80008045,
    HI_ERR_TP_ZTW523_INIT_CONFIG                    = 0x80008046,
    HI_ERR_TP_ZTW523_INIT_GETINFO                   = 0x80008047,
    HI_ERR_TP_ZTW523_POWERUP_ERROR                  = 0x80008048,
    HI_ERR_TP_ZTW523_POWERUP_VENDOR_CMD_ENABLE      = 0x80008049,
    HI_ERR_TP_ZTW523_POWERUP_READ_CHIPID            = 0x8000804A,
    HI_ERR_TP_ZTW523_POWERUP_INTN_CLEAR             = 0x8000804B,
    HI_ERR_TP_ZTW523_POWERUP_NVM_INIT               = 0x8000804C,
    HI_ERR_TP_ZTW523_POWERUP_PROGRAM_START          = 0x8000804D,
    HI_ERR_TP_ZTW523_POWERUP_REPORT_RATE            = 0x8000804E,
    HI_ERR_TP_ZTW523_POWERUP_PIXEL_THRESHOLD        = 0x8000804F,
    HI_ERR_TP_ZTW523_POWERUP_SW_RESET               = 0x80008050,
    HI_ERR_TP_ZTW523_HWCALIB_TOUCHMODE              = 0x80008051,
    HI_ERR_TP_ZTW523_HWCALIB_CMD1                   = 0x80008052,
    HI_ERR_TP_ZTW523_HWCALIB_CLEARINT1              = 0x80008053,
    HI_ERR_TP_ZTW523_HWCALIB_READE2P                = 0x80008054,
    HI_ERR_TP_ZTW523_HWCALIB_CMD2                   = 0x80008055,
    HI_ERR_TP_ZTW523_HWCALIB_CLEARINT2              = 0x80008056,
    HI_ERR_TP_ZTW523_HWCALIB_OVERTIME               = 0x80008057,
    HI_ERR_TP_ZTW523_HWCALIB_INITTOUCHMODE          = 0x80008058,
    HI_ERR_TP_ZTW523_HWCALIB_WRITETOUCHMODE         = 0x80008059,
    HI_ERR_TP_ZTW523_HWCALIB_WRITEINT               = 0x8000805A,
    HI_ERR_TP_ZTW523_HWCALIB_WRITECTL1_EN           = 0x8000805B,
    HI_ERR_TP_ZTW523_HWCALIB_WRITECTL2_EN           = 0x8000805C,
    HI_ERR_TP_ZTW523_HWCALIB_SAVE                   = 0x8000805D,
    HI_ERR_TP_ZTW523_HWCALIB_WRITECTL1_DIS          = 0x8000805E,
    HI_ERR_TP_ZTW523_HWCALIB_WRITECTL2_DIS          = 0x8000805F,
    HI_ERR_TP_ZTW523_POWERUP_FW_CMD_ENABLE          = 0x80008060,
    HI_ERR_TP_ZTW523_POWERUP_FW_READ_CHIPCODE       = 0x80008061,
    HI_ERR_TP_ZTW523_POWERUP_FW_INTN_CLEAR          = 0x80008062,
    HI_ERR_TP_ZTW523_POWERUP_FW_NVM_INIT            = 0x80008063,
    HI_ERR_TP_ZTW523_POWERUP_FW_NVM_VPPON           = 0x80008064,
    HI_ERR_TP_ZTW523_POWERUP_FW_NVM_WPDIS           = 0x80008065,
    HI_ERR_TP_ZTW523_POWERUP_FW_INIT_FLASH          = 0x80008066,
    HI_ERR_TP_ZTW523_FW_VERIFY_NVM_VPPON            = 0x80008067,
    HI_ERR_TP_ZTW523_FW_VERIFY_NVM_WPDIS            = 0x80008068,
    HI_ERR_TP_ZTW523_FW_VERIFY_INIT_FLASH           = 0x80008069,
    HI_ERR_TP_ZTW523_FW_VERIFY_OPENFILE             = 0x8000806A,
    HI_ERR_TP_ZTW523_FW_VERIFY_FSTAT                = 0x8000806B,
    HI_ERR_TP_ZTW523_FW_VERIFY_PAGESIZE             = 0x8000806C,
    HI_ERR_TP_ZTW523_FW_VERIFY_LSEEK                = 0x8000806D,
    HI_ERR_TP_ZTW523_FW_VERIFY_READSIZE             = 0x8000806E,
    HI_ERR_TP_ZTW523_FW_VERIFY_READ                 = 0x8000806F,
    HI_ERR_TP_ZTW523_FW_VERIFY_COMPARE              = 0x80008070,
    HI_ERR_TP_ZTW523_FW_UPGRADE_OPENFILE            = 0x80008071,
    HI_ERR_TP_ZTW523_FW_UPGRADE_FSTAT               = 0x80008072,
    HI_ERR_TP_ZTW523_FW_UPGRADE_PAGESIZE            = 0x80008073,
    HI_ERR_TP_ZTW523_FW_UPGRADE_FWSIZE              = 0x80008074,
    HI_ERR_TP_ZTW523_FW_UPGRADE_LSEEK               = 0x80008075,
    HI_ERR_TP_ZTW523_FW_UPGRADE_READSIZE            = 0x80008076,
    HI_ERR_TP_ZTW523_FW_UPGRADE_WRITE               = 0x80008077,
    HI_ERR_TP_ZTW523_FW_CHECK_RESET                 = 0x80008078,
    HI_ERR_TP_ZTW523_FW_CHECK_POWERUP               = 0x80008079,
    HI_ERR_TP_ZTW523_FW_CHECK_IRQDIS                = 0x8000807A,
    HI_ERR_TP_ZTW523_FW_CHECK_READFW_VERSION        = 0x8000807B,
    HI_ERR_TP_ZTW523_FW_CHECK_READMINORFW_VERSION   = 0x8000807C,
    HI_ERR_TP_ZTW523_FW_CHECK_READ_DATAREG          = 0x8000807D,
    HI_ERR_TP_ZTW523_FW_CHECK_FAIL                  = 0x8000807E,
    HI_ERR_TP_ZTW523_FW_CHECK_INIT_TOUCHMODE        = 0x8000807F,
    HI_ERR_TP_ZTW523_FW_CHECK_WRITE_TOUCHMODE       = 0x80008080,
    HI_ERR_TP_ZTW523_FW_CHECK_WRITE_FINGERNUM       = 0x80008081,
    HI_ERR_TP_ZTW523_FW_CHECK_WRITE_XRES            = 0x80008082,
    HI_ERR_TP_ZTW523_FW_CHECK_WRITE_YRES            = 0x80008083,
    HI_ERR_TP_ZTW523_FW_CHECK_WRITE_CALIBCMD        = 0x80008084,
    HI_ERR_TP_ZTW523_FW_CHECK_WRITE_INTMASK         = 0x80008085,
    HI_ERR_TP_ZTW523_FW_CHECK_OVERTIMES             = 0x80008086,
    HI_ERR_TP_ZTW523_CHECKID_RESET                  = 0x80008087,
    HI_ERR_TP_ZTW523_CHECKID_POWERUP                = 0x80008088,
    HI_ERR_TP_ZTW523_CHECKID_READ_CHIPCODE          = 0x80008089,
    HI_ERR_TP_ZTW523_CHECKID_READ_VENDORID          = 0x8000808A,
    HI_ERR_TP_ZTW523_GETFWVER_RESET                 = 0x8000808B,
    HI_ERR_TP_ZTW523_GETFWVER_POWERUP               = 0x8000808C,
    HI_ERR_TP_ZTW523_GETFWVER_READFWVER             = 0x8000808D,
    HI_ERR_TP_ZTW523_GETFWVER_READMINORFWVER        = 0x8000808E,
    HI_ERR_TP_ZTW523_GETFWVER_READDATAREG           = 0x8000808F,
    HI_ERR_TP_ZTW523_CONFIG_RESET                   = 0x80008090,
    HI_ERR_TP_ZTW523_CONFIG_READ_FWVERSION          = 0x80008091,
    HI_ERR_TP_ZTW523_CONFIG_READ_FWMINORVERSION     = 0x80008092,
    HI_ERR_TP_ZTW523_CONFIG_READ_DATAREG            = 0x80008093,
    HI_ERR_TP_ZTW523_CONFIG_INIT_TOUCHMODE          = 0x80008094,
    HI_ERR_TP_ZTW523_CONFIG_WRITE_TOUCHMODE         = 0x80008095,
    HI_ERR_TP_ZTW523_CONFIG_WRITE_FINGER_NUM        = 0x80008096,
    HI_ERR_TP_ZTW523_CONFIG_WRITE_XRES              = 0x80008097,
    HI_ERR_TP_ZTW523_CONFIG_WRITE_YRES              = 0x80008098,
    HI_ERR_TP_ZTW523_CONFIG_WRITE_CALIBCMD          = 0x80008099,
    HI_ERR_TP_ZTW523_CONFIG_WRITE_INTMASK           = 0x8000809A,
    HI_ERR_TP_ZTW523_CONFIG_WRITE_GESWAKEUP         = 0x8000809B,
    HI_ERR_TP_ZTW523_CONFIG_CLEAR_INT               = 0x8000809C,
    HI_ERR_TP_ZTW523_SETMODE_SWRESET                = 0x8000809D,
    HI_ERR_TP_ZTW523_SETMODE_SETNCOUNT              = 0x8000809E,
    HI_ERR_TP_ZTW523_SETMODE_SETNAFE                = 0x8000809F,
    HI_ERR_TP_ZTW523_SETMODE_SETUCOUNT              = 0x800080A0,
    HI_ERR_TP_ZTW523_SETMODE_SETDELAY               = 0x800080A1,
    HI_ERR_TP_ZTW523_SETMODE_WRITEMODE              = 0x800080A2,
    HI_ERR_TP_ZTW523_SETMODE_CLEAR_INT              = 0x800080A3,
    HI_ERR_TP_ZTW523_GET_RAWDATA_CLEAR_INT1         = 0x800080A4,
    HI_ERR_TP_ZTW523_GET_RAWDATA_READDATA           = 0x800080A5,
    HI_ERR_TP_ZTW523_GET_RAWDATA_CLEAR_INT2         = 0x800080A6,
    HI_ERR_TP_ZTW523_GET_DND_SETMODE                = 0x800080A7,
    HI_ERR_TP_ZTW523_GET_DND_DATA                   = 0x800080A8,
    HI_ERR_TP_ZTW523_GET_DND_SET_POINTMODE          = 0x800080A9,
    HI_ERR_TP_ZTW523_GET_SELFDND_SETMODE            = 0x800080AA,
    HI_ERR_TP_ZTW523_GET_SELFDND_DATA               = 0x800080AB,
    HI_ERR_TP_ZTW523_GET_SELFDND_SET_POINTMODE      = 0x800080AC,
    HI_ERR_TP_ZTW523_GET_HWID_RESET                 = 0x800080AD,
    HI_ERR_TP_ZTW523_GET_HWID_POWERUP               = 0x800080AE,
    HI_ERR_TP_ZTW523_GET_HWID_READ_ID               = 0x800080AF,
} hi_errno;

#ifndef EOK
#define EOK HI_ERR_SUCCESS
#endif
#endif /* HI_ERRNO_H_ */

