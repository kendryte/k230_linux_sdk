/*
 * Copyright (c) CompanyNameMagicTag 2021-2021. All rights reserved.
 * Description: OSAL Error Definition.
 */

#ifndef OSAL_ERRNO_H_
#define OSAL_ERRNO_H_

#include "soc_module.h"

#define OSAL_EINTR        (-4)  /* Interrupted system call */
#define OSAL_ETIME       (-62)  /* Timer expired */
#define OSAL_EOVERFLOW   (-75)  /* Value too large for defined data type */


#define _OSAL_ERRNO_BEGIN_ ERR_NUMBER(OSAL_MODULE_MIN, OSAL_ERR_TYPE_BEGIN, 0)
#define _OSAL_ERRNO_END_   ERR_NUMBER(OSAL_MODULE_MAX, OSAL_ERR_TYPE_BOTTOM, 0xFF)

typedef enum {
    OSAL_ERR_SUCCESS = 0,
    OSAL_ERR_FAILURE = 0xFFFFFFFF,
} osal_errno;

#ifndef EOK
#define EOK 0
#endif

#endif /* OSAL_ERRNO_H_ */
