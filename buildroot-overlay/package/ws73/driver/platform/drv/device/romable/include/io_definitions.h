/*
 * Copyright (c) CompanyNameMagicTag 2020-2021. All rights reserved.
 * Description: IO Definitions and Multiplexor.
 * Author: CompanyName
 * Create: 2021-9-30
 */


#ifndef __IO_DEFINITIONS_H__
#define __IO_DEFINITIONS_H__

#define IO_FUNC_DEFAULT               0
#define IO_FUNC_MAX                   5         /* Maximum number of FUNC supported by each pin */
/* Schmitt enable mask */
#define IO_TRIGGER_MASK               0x0008

/* Drive strength */
#define IO_DRIVE_MASK                 0x0030
#define IO_DRIVE_SHIFT                4
#define EXT_IO_DRIVE_STRENGTH_MAX     (EXT_IO_DRIVE_STRENGTH_3)

#define IO_PULL_UP_MASK               0x0200
#define IO_PULL_DOWN_MASK             0x0400

/* Slew rate */
#define IO_SLEW_MASK                  0x0100

/* input enable */
#define IO_IE_MASK                    0x0800

/* IO PIN resources */
typedef enum {
    EXT_IO_PIN_0,
    EXT_IO_PIN_1,
    EXT_IO_PIN_2,
    EXT_IO_PIN_3,
    EXT_IO_PIN_4,
    EXT_IO_PIN_5,
    EXT_IO_PIN_6,
    EXT_IO_PIN_7,
    EXT_IO_PIN_8,
    EXT_IO_PIN_9,
    EXT_IO_PIN_10,
    EXT_IO_PIN_11,
    EXT_IO_PIN_XO,
    EXT_IO_TYPE_ID, /* not supprot I/O multiplexing */
    EXT_IO_PIN_MAX,
    EXT_IO_PIN_NONE = EXT_IO_PIN_MAX,
} ext_io_pin;

typedef enum {
    PIN_0_SSI_CLK               = 0,
    PIN_0_GPIO_0                = 1,
    PIN_0_DEBUG_MODE_0          = 3,
    PIN_0_DIAG_7                = 5
} ext_io_pin_0_func;

typedef enum {
    PIN_1_GPIO_1                = 0,
    PIN_1_UART_CTS_N            = 2,
    PIN_1_JTAG_TMS              = 4,
    PIN_1_DIAG_6                = 5
} ext_io_pin_1_func;

typedef enum {
    PIN_2_UART_RXD              = 0,
    PIN_2_GPIO_2                = 1,
    PIN_2_DIAG_5                = 5
} ext_io_pin_2_func;

typedef enum {
    PIN_3_UART_TXD              = 0,
    PIN_3_GPIO_3                = 1,
    PIN_3_CLK_24M_COEX          = 3,
    PIN_3_DIAG_4                = 5,
} ext_io_pin_3_func;

typedef enum {
    PIN_4_PMU_32K_TEST          = 0,
    PIN_4_GPIO_4                = 1,
    PIN_4_UART_RTS_N            = 2,
    PIN_4_REFCLK_FREQ_STATUS    = 3,
    PIN_4_JTAG_TCK              = 4,
    PIN_4_DIAG_3                = 5,
} ext_io_pin_4_func;

typedef enum {
    PIN_5_SDIO_DATA1            = 0,
    PIN_5_GPIO_5                = 1,
    PIN_5_GSPI_INT              = 2,
} ext_io_pin_5_func;

typedef enum {
    PIN_6_SDIO_DATA0            = 0,
    PIN_6_GPIO_6                = 1,
    PIN_6_GSPI_DO               = 2,
    PIN_6_DSPI_TXD              = 4,
} ext_io_pin_6_func;

typedef enum {
    PIN_7_SDIO_CLK              = 0,
    PIN_7_GPIO_7                = 1,
    PIN_7_GSPI_CK               = 2,
    PIN_7_DSPI_CK               = 4
} ext_io_pin_7_func;

typedef enum {
    PIN_8_SDIO_CMD              = 0,
    PIN_8_GPIO_8                = 1,
    PIN_8_GSPI_DI               = 2,
    PIN_8_DSPI_RXD              = 4,
} ext_io_pin_8_func;

typedef enum {
    PIN_9_SDIO_DATA3            = 0,
    PIN_9_GPIO_9                = 1,
    PIN_9_GSPI_CSN              = 2,
    PIN_9_DSPI_CSN              = 4,
    PIN_9_DIAG_2                = 5,
} ext_io_pin_9_func;

typedef enum {
    PIN_10_SDIO_DATA2           = 0,
    PIN_10_GPIO_10              = 1,
    PIN_10_USB_VBUS             = 2,
    PIN_10_INTERFACE_SELECTION  = 3,
    PIN_10_DIAG_1               = 5,
} ext_io_pin_10_func;

typedef enum {
    PIN_11_GPIO_11              = 0,
    PIN_11_DIAG_0               = 5,
} ext_io_pin_11_func;

typedef enum {
    PIN_XO_SSI_DATA             = 0,
    PIN_XO_GPIO_XO              = 1,
    PIN_XO_DEBUG_MODE_1         = 3,
} ext_io_pin_xo_func;

#endif /* __IO_DEFINITIONS_H__ */
