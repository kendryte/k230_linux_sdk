/*
 * Copyright (c) CompanyNameMagicTag 2014-2023. All rights reserved.
 * Description: pm wlan
 */
#ifndef __PM_WLAN_H__
#define __PM_WLAN_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/* interface for wlan */

#if defined(CONFIG_SUPPORT_UART_DEVICE_DETECT) && (CONFIG_SUPPORT_UART_DEVICE_DETECT != 0)
/**
 * @brief  Detect if device is connectable.
 * @retval 0        Success.
 * @retval Other    Failure.
 */
osal_s32 uapi_pm_device_detect(osal_void);
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of pm_extern.h */





