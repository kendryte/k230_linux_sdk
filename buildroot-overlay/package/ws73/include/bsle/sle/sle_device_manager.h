/**
 * Copyright (c) @CompanyNameMagicTag 2024. All rights reserved.
 * Description: SLE Device Manager, module.
 */

/**
 * @defgroup sle_device_manager Device Manager API
 * @ingroup  SLE
 * @{
 */

#ifndef SLE_DEVICE_MANAGER
#define SLE_DEVICE_MANAGER

#include <stdint.h>
#include "errcode.h"
#include "sle_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @if Eng
 * @brief  Registering the Path for Storing SLE Device Persistence Information.
 * @par    Registering the Path for Storing SLE Device Persistence Information.
 * @attention Applicable to the Linux system.
 * @param [in] path file store path.
 * @retval error code.
 * @else
 * @brief  注册SLE设备持久化信息存储路径。
 * @par    注册SLE设备持久化信息存储路径。
 * @attention 适用于linux系统。
 * @param [in] path 文件持久化路径。
 * @retval 执行结果错误码。
 * @endif
 */
errcode_t sle_dev_manager_register_file_path(const uint8_t *path);

#ifdef __cplusplus
}
#endif
#endif /* SLE_DEVICE_MANAGER */
/**
 * @}
 */
