/*
 * Copyright (c) CompanyNameMagicTag 2021-2021. All rights reserved.
 * Description: bt vendor header
 * Author: AuthorNameMagicTag
 * Create: 2021-11-10
 */

#ifndef __BT_VENDOR_SOC_H__
#define __BT_VENDOR_SOC_H__

/* Other Header File Including */
#include <utils/Log.h>
#include <cutils/properties.h>

#include "bt_vendor_lib.h"

#ifndef SUCCESS
#define SUCCESS 0
#endif

#ifndef FAILED
#define FAILED (-1)
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#define BT_UART_CTL  "/dev/hwbt"


#define alog_debug(fmt, arg...)                                     \
    do {                                                            \
        ALOGD("[%s:%d]" fmt "\n", __FUNCTION__, __LINE__, ##arg);   \
    } while (0)

#define alog_info(fmt, arg...)                                      \
    do {                                                            \
        ALOGI("[%s:%d]" fmt "\n", __FUNCTION__, __LINE__, ##arg);   \
    } while (0)

#define alog_warning(fmt, arg...)                                   \
    do {                                                            \
        ALOGW("[%s:%d]" fmt "\n", __FUNCTION__, __LINE__, ##arg);   \
    } while (0)

#define alog_error(fmt, arg...)                                     \
    do {                                                            \
        ALOGE("[%s:%d]" fmt "\n", __FUNCTION__, __LINE__, ##arg);   \
    } while (0)

#endif /* end of bt_vendor_soc.h */
