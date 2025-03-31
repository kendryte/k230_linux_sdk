/**
 * Copyright (c) CompanyNameMagicTag 2020-2021. All rights reserved.\n
 *
 * Description: This file contains power management api definition. \n
 * Author: CompanyName \n
 * History: \n
 * 2022-02-25, 更新文件注释 \n
 * 2020-12-31, 初始化该文件 \n
 */

#ifndef __SOC_LOWPOWER_H__
#define __SOC_LOWPOWER_H__

#include <td_base.h>

/**
* @file soc_lowpower.h
*/

/** @defgroup iot_pm pm api */

/**
 * @ingroup iot_pm
 * Pm errno
 */
typedef enum {
    PM_SUCCESS = 0,
    PM_FAILURE = 0xFFFFFFFF,
} ext_pm_errno;

/**
 * @ingroup iot_pm
 * Power management subsystem type enumeration.
 */
typedef enum {
    PM_SUBSYS_ID_LPMCU = 0,
    PM_SUBSYS_ID_ACPU,
    PM_SUBSYS_ID_PCPU,
    PM_SUBSYS_ID_SCPU,
    PM_SUBSYS_ID_IOCPU,
    PM_SUBSYS_ID_DSP,
    PM_SUBSYS_ID_CGRA,
    PM_SUBSYS_ID_INVALID
} ext_pm_subsys_id;

/**
* @ingroup iot_pm
* Wakeup interrupt source type enumeration.
*/
typedef enum {
    APP_WKUP_TYPE_ACPU_RTC,             /**< A Core wakeup int type */
    APP_WKUP_TYPE_ACPU_GPIO,            /**< A Core wakeup int type */
    APP_WKUP_TYPE_ACPU_LPUART,          /**< A Core wakeup int type */
    APP_WKUP_TYPE_CAPSENSOR,            /**< A Core wakeup int type */
    APP_WKUP_TYPE_ACPU_IPC,             /**< A Core wakeup int type */

    PROT_WKUP_TYPE_RTC,                 /**< P Core wakeup int type */
    PROT_WKUP_TYPE_PROT_GPIO,           /**< P Core wakeup int type */
    PROT_WKUP_TYPE_EXT0_TBTT,           /**< P Core wakeup int type */
    PROT_WKUP_TYPE_EXT1_TBTT,           /**< P Core wakeup int type */
    PROT_WKUP_TYPE_EXT2_TBTT,           /**< P Core wakeup int type */
    PROT_WKUP_TYPE_EXT_P2P,             /**< P Core wakeup int type */
    PROT_WKUP_TYPE_EXT_WUR_WAKE,        /**< P Core wakeup int type */
    PROT_WKUP_TYPE_EXT_WUR_RX,          /**< P Core wakeup int type */
    PROT_WKUP_TYPE_EXT0_TSF_SLP,        /**< P Core wakeup int type */
    PROT_WKUP_TYPE_EXT1_TSF_SLP,        /**< P Core wakeup int type */
    PROT_WKUP_TYPE_EXT2_TSF_SLP,        /**< P Core wakeup int type */
    PROT_WKUP_TYPE_EXT_WUR_SLP,         /**< P Core wakeup int type */
    PROT_WKUP_TYPE_TIMING_GEN_WKUP,     /**< P Core wakeup int type */
    PROT_WKUP_TYPE_TIMING_GEN_SLEEP,    /**< P Core wakeup int type */
    PROT_WKUP_TYPE_PROT_BT_RADIO_EN,    /**< P Core wakeup int type */
    PROT_WKUP_TYPE_PROT_IPC_WKUP,       /**< P Core wakeup int type */

    IOMCU_WKUP_TYPE_IOCPU_RTC,          /**< IOMCU wakeup int type */
    IOMCU_WKUP_TYPE_SENSOR_GPIO,        /**< IOMCU wakeup int type */
    IOMCU_WKUP_TYPE_IOCPU_LPUART,       /**< IOMCU wakeup int type */
    IOMCU_WKUP_TYPE_IOCPU_IPC,          /**< IOMCU wakeup int type */

    SEC_WKUP_TYPE_SEC_RTC,              /**< S Core wakeup int type */
    SEC_WKUP_TYPE_SEC_GPIO,             /**< S Core wakeup int type */
    SEC_WKUP_TYPE_SEC_IPC               /**< S Core wakeup int type */
} ext_pm_wakeup_type;

/**
* @ingroup iot_pm
* Power management level init type
*/
typedef struct {
    ext_pm_subsys_id subsys_id;
    td_u32 min_lsleep_ticks;  /**< Min light sleep ticks */
    td_u32 min_dsleep_ticks;  /**< Min deep sleep ticks */
    td_u32 delta_ticks;       /**< Delta ticks, e.g. wakeup earlier */
} ext_pm_init_param;

/**
 * @ingroup iot_pm
 * Power management level enumeration.
 */
typedef enum {
    PM_NO_SLEEP,    /**< no sleep type.CNcomment: CNend */
    PM_LIGHT_SLEEP, /**< light sleep type.CNcomment: CNend */
    PM_DEEP_SLEEP,  /**< deep sleep type.CNcomment: CNend */
} ext_pm_type;

/**
 * @ingroup iot_pm
 * Service module enumeration.
 */
typedef enum {
    PM_MOD_DIAG = 0, /**< diag uart Id.CNcomment: Id */
    PM_MOD_SHELL,    /**< shell Id.CNcomment: CNend */
    PM_MOD_WIFI,     /**< WIFI Id.CNcomment: CNend */
    PM_MOD_BT,       /**< WUR Id.CNcomment: CNend */
    PM_MOD_DEMO,     /**< demo Id.CNcomment: CNend */
    PM_MOD_MAX
} ext_pm_mod_id;

/**
 * @ingroup iot_pm
 * Peripheral module enumeration
 */
typedef enum {
    PM_PERI_ID_WIFI = 0,
    PM_PERI_ID_WUR,
    PM_PERI_ID_MAX
} ext_pm_peri_id;

/**
 * @ingroup iot_pm
 * Power management structure.
 */
typedef struct {
    ext_pm_subsys_id cur_subsys_id;
    td_u32           wakeup_times;       /**< wakeup times.CNcomment: CNend */
    td_u32           dsleep_fail_times;  /**< the times of power off fail during
                                              deepsleep.CNcomment: CNend */
    ext_pm_type      type;               /**< power management level.CNcomment:
                                              CNend */
} ext_pm_info;

/**
 * @ingroup iot_pm
 * Power management device.
 */
struct ext_pm_device {
    const char *name;
    td_void *data;
};

/**
 * @ingroup iot_pm
 * Power management driver ops.
 */
struct ext_pm_driver_ops {
    td_u32 (*prepare)(struct ext_pm_device *device, ext_pm_type mode);
    td_u32 (*complete)(struct ext_pm_device *device, ext_pm_type mode);
    td_u32 (*suspend)(struct ext_pm_device *device, ext_pm_type mode);
    td_u32 (*resume)(struct ext_pm_device *device, ext_pm_type mode);
};

/**
 * @ingroup  iot_pm
 * @brief  Statistics related to power saving.
 *
 * @par Description:
 *           Statistics related to power saving.CNcomment: CNend
 *
 * @param  None
 *
 * @retval ext_pm_info Pointer to the status sturcture. CNcomment: CNend
 * @par Dependency:
 *            @li soc_lowpower.h: Describes power saving management APIs.
 * @see  None
 */
ext_pm_info *uapi_pm_get_info(td_void);

/**
 * @ingroup  iot_pm
 * @brief  Low power initialization.
 *
 * @par Description:
 *           Initializes power saving management. CNcomment: CNend
 *
 * @attention This is an initialization API and does not support multi-task
 *            calling or repeated calling.
 must call after uapi_sal_init.
 CNcomment: CNend
 * @param  None
 *
 * @retval #PM_SUCCESS   Success.
 * @retval #PM_FAILURE   Failure.
 * @par Dependency:
 *            @li soc_lowpower.h: Describes power saving management APIs.
 * @see  None
 */
td_u32 uapi_pm_init(ext_pm_init_param *init_param);

/**
 * @ingroup  iot_pm
 * @brief  Power management enable.
 *
 * @par Description:
 *           Enable power saving management. CNcomment: CNend
 *
 * @retval none
 *
 * @par Dependency:
 *            @li soc_lowpower.h: Describes power saving management APIs.
 * @see  None
 */
td_void uapi_pm_enable(td_void);

/**
 * @ingroup  iot_pm
 * @brief  Power management disable.
 *
 * @par Description:
 *           Disable power saving management. CNcomment: CNend
 *
 * @retval none
 *
 * @par Dependency:
 *            @li soc_lowpower.h: Describes power saving management APIs.
 * @see  None
 */
td_void uapi_pm_disable(td_void);

/**
 * @ingroup  iot_pm
 * @brief  vote against system entering sleep.
 *         CNcomment:投反对票阻止系统进入睡眠。CNend
 *
 * @par Description: vote against system entering sleep, automatically
 *      remove veto upon timeout.
 *      CNcomment: 投反对票阻止系统进入睡眠, 超时自动去反对票。CNend
 *
 * @param  pm_type  [in] power management type.
 * @param  id  [in] module id.
 * @param  timeout  [in] ms, the interval of timeout.
 * @attention If the interval of timeout is 0, veto exists until user call
 *            uapi_pm_unvote_against_sleep.
 *            CNcomment: 如果timeout值为0，用户需要主动调用
 *            uapi_pm_unvote_against_sleep去除反对票。CNend
 *
 * @retval #PM_SUCCESS   Success.
 * @retval #PM_FAILURE   Failure.
 * @par Dependency:
 *            @li soc_lowpower.h: Describes power saving management APIs.
 * @see  None
 */
td_u32 uapi_pm_vote_against_sleep(ext_pm_type pm_type, ext_pm_mod_id id, td_u32 timeout);

/**
 * @ingroup  iot_pm
 * @brief  remove veto, allow system entering sleep.
 *         CNcomment:去除反对票允许系统进入睡眠。CNend
 *
 * @par Description: see brief
 *
 * @param  pm_type  [in] power management type.
 * @param  id  [in] module id.
 *
 * @retval #PM_SUCCESS   Success.
 * @retval #PM_FAILURE   Failure.
 * @attention none
 * @par Dependency:
 *            @li soc_lowpower.h: Describes power saving management APIs.
 * @see  None
 */
td_u32 uapi_pm_unvote_against_sleep(ext_pm_type pm_type, ext_pm_mod_id id);

/**
 * @ingroup  iot_pm
 * @brief  register hook function for driver power management.
 *         CNcomment:注册驱动功耗管理钩子函数。CNend
 *
 * @par Description: see brief
 *
 * @param  dev  [in] driver deivce.
 * @param  drv_ops  [in] pointer to hook function structure.
 *
 * @retval #PM_SUCCESS   Success.
 * @retval #PM_FAILURE   Failure.
 * @attention none
 * @par Dependency:
 *            @li soc_lowpower.h: Describes power saving management APIs.
 * @see  None
 */
td_u32 uapi_pm_drv_register(struct ext_pm_device *dev, struct ext_pm_driver_ops *drv_ops);

/**
 * @ingroup  iot_pm
 * @brief  unregister hook function for driver power management.
 *         CNcomment:解注册驱动功耗管理钩子函数。CNend
 *
 * @par Description: see brief
 *
 * @param  dev  [in] driver deivce.
 *
 * @retval #PM_SUCCESS   Success.
 * @retval #PM_FAILURE   Failure.
 * @attention none
 * @par Dependency:
 *            @li soc_lowpower.h: Describes power saving management APIs.
 * @see  None
 */
td_u32 uapi_pm_drv_unregister(struct ext_pm_device *device);


/**
 * @ingroup  iot_pm
 * @brief  获取睡眠次数. CNcomment:获取睡眠次数 CNend
 *
 * @par Description: see brief
 *
 * @param  pm_mode  [in] 睡眠模式.
 *
 * @retval 睡眠次数
 * @attention none
 * @par Dependency:
 *            @li soc_lowpower.h: Describes power saving management APIs.
 * @see  None
 */
td_u32 uapi_pm_get_sleep_cnt(td_u32 pm_mode);

// -打桩接口，待实现

/**
 * @ingroup  iot_pm
 * @brief  判断外置32k晶体是否稳定. CNcomment:判断外置32k晶体是否稳定 CNend
 *
 * @par Description: see brief
 *
 * @retval 0: 不稳定，其他，稳定。
 * @attention none
 * @par Dependency:
 *            @li soc_lowpower.h: Describes power saving management APIs.
 * @see  None
 */
td_u32 uapi_pm_is_ext_32k_stable(void);

/**
 * @ingroup  iot_pm
 * @brief  获取外置32k晶体频率. CNcomment:获取外置32k晶体频率 CNend
 *
 * @par Description: see brief
 *
 * @param  frep  [out] 时钟频率
 *
 * @retval #PM_SUCCESS   Success.
 * @retval #PM_FAILURE   Failure.
 * @attention none
 * @par Dependency:
 *            @li soc_lowpower.h: Describes power saving management APIs.
 * @see  None
 */
td_u32 uapi_pm_get_ext_32k_freq(td_u32 *frep);

extern td_u32 pm_crg_svc_vote(td_u8 sid, td_u8 freq_lvl);

#endif /* __SOC_LOWPOWER_H__ */
