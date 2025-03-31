/*
 * Copyright (c) CompanyNameMagicTag 2021-2021. All rights reserved.
 * Description: ACORE GPIO ID.
 */

#ifndef __GPIO_ID_H__
#define __GPIO_ID_H__

#define GPIO_NUM_IN_BANK     8
#define GPIO_BANK_MAX        (((EXT_GPIO_MAX - 1) / (GPIO_NUM_IN_BANK)) + 1)

#define GPIO_SHARE_IQR_NUM   8
#define GPIO_IRQ_NUM         (((EXT_GPIO_MAX - 1) / (GPIO_SHARE_IQR_NUM)) + 1)

#define GPIO_NUM_IN_GROUP    32
#define GPIO_GROUP_MAX       (((EXT_GPIO_MAX - 1) / (GPIO_NUM_IN_GROUP)) + 1)

typedef enum {
    EXT_GPIO_0,
    EXT_GPIO_1,
    EXT_GPIO_2,
    EXT_GPIO_3,
    EXT_GPIO_4,
    EXT_GPIO_5,
    EXT_GPIO_6,
    EXT_GPIO_7,
    EXT_GPIO_8,
    EXT_GPIO_9,
    EXT_GPIO_10,
    EXT_GPIO_11,
    EXT_GPIO_12,
    EXT_GPIO_13,
    EXT_GPIO_14,
    EXT_GPIO_MAX
} ext_gpio_id;
#endif