/**
 * Copyright (c) CompanyNameMagicTag 2019-2022. All rights reserved. \n
 *
 * Description: Provides tsensor driver api \n
 * Author: CompanyName \n
 * History: \n
 * 2022-02-26， 更新文件注释 \n
 * 2019-05-30， 初始化该文件 \n
 */

/** @defgroup iot_tsensor iot_tsensor */

#ifndef __SOC_TSENSOR_H__
#define __SOC_TSENSOR_H__
#include <td_base.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * @ingroup  iot_tsensor
 * @brief Callback function of Tsensor interruption. CNcomment:Tsensor中断回调函数。CNend
 * @par Description:
 *           Callback function of Tsensor interruption. CNcomment:Tsensor回调函数。CNend
 * @attention None
 * @param  data [IN] type #td_s16，Parameter transfer of the callback function, indicating the temperature when the
 *              interrupt is reported.CNcomment:回调函数参数传递，表示中断上报时的温度值。CNend
 * @retval None
 * @par Dependency:
 *            @li soc_tsensor.h：Tsensor APIs.CNcomment:文件用于描述获取Tsensor相关接口。CNend
 * @see None
 */
typedef td_void (*ext_tsensor_callback_func)(td_s16);

/*
 * @ingroup iot_tsensor
 * Tsensor report mode.CNcomment:Tsensor上报模式。CNend
 */
typedef enum {
    EXT_TSENSOR_MODE_16_POINTS_SINGLE = 0, /* CNcomment:模式：16点平均单次上报 CNend */
    EXT_TSENSOR_MODE_16_POINTS_LOOP = 1,   /* CNcomment:模式：16点平均循环上报 CNend */
    EXT_TSENSOR_MODE_1_POINT_LOOP = 2,     /* CNcomment:模式：单点循环上报 CNend */
    EXT_TSENSOR_MODE_MAX,                  /* CNcomment:错误入参，不可使用 CNend */
} ext_tsensor_mode;

/*
 * @ingroup iot_tsensor
 * Tsensor multilevel interrupt enable level.CNcomment:Tsensor 多档温度中断使能档位。CNend
 */
typedef enum {
    TSENSOR_MULTILEVEL_EN_LTO0 = 0,  /* CNcomment:当温度落在L - 0档位时，触发此回调函数 */
    TSENSOR_MULTILEVEL_EN_0TO1,      /* CNcomment:当温度落在0 - 1档位时，触发此回调函数 */
    TSENSOR_MULTILEVEL_EN_1TO2,      /* CNcomment:当温度落在1 - 2档位时，触发此回调函数 */
    TSENSOR_MULTILEVEL_EN_2TO3,      /* CNcomment:当温度落在2 - 3档位时，触发此回调函数 */
    TSENSOR_MULTILEVEL_EN_3TOH,      /* CNcomment:当温度落在3 - L档位时，触发此回调函数 */
    TSENSOR_MULTILEVEL_EN_MAX,
} ext_tsensor_multilevel_en;

/*
 * @ingroup iot_tsensor
 * Tsensor multilevel threshold value level.CNcomment:Tsensor 多档温度阈值设置档位。CNend
 */
typedef enum {
    TSENSOR_MULTILEVEL_VAL_0 = 0,  /* CNcomment:设置0档位的温度阈值 */
    TSENSOR_MULTILEVEL_VAL_1,      /* CNcomment:设置1档位的温度阈值 */
    TSENSOR_MULTILEVEL_VAL_2,      /* CNcomment:设置2档位的温度阈值 */
    TSENSOR_MULTILEVEL_VAL_3,      /* CNcomment:设置3档位的温度阈值 */
    TSENSOR_MULTILEVEL_VAL_MAX,
} ext_tsensor_multilevel_value;

#define EXT_TSENSOR_INVALID_CODE 0xFFFF   /* CNcomment:无效温度码，有效温度码为(0-255) CNend */
#define EXT_TSENSOR_INVALID_TEMP (-1000)  /* CNcomment:无效温度，有效温度码为(-40-140) CNend */
#define EXT_TSENSOR_AUTO_REFRESH_PERIOD_100MS           0x7D00

/*
 * @ingroup  iot_tsensor
 * @brief Tsensor module start.CNcomment:Tsensor模块启动。CNend
 * @par Description:
 *           Tsensor module start.CNcomment:Tsensor启动温度采集。CNend
 * @attention Period is valid when mode = #EXT_TSENSOR_MODE_16_POINTS_LOOP or EXT_TSENSOR_MODE_1_POINT_LOOP.
 *            By default, the Tsensor EXT_TSENSOR_MODE_16_POINTS_SINGLE temperature collection mode is enabled for the
 *            Wi-Fi module. If this function is repeatedly called, the default mode used by the Wi-Fi module will be
 *            overwritten. If you want to use the Tsensor module for other services, you can register the
 *            corresponding interrupt callback function in the service.
 * CNcomment:period仅当mode为EXT_TSENSOR_MODE_16_POINTS_SINGLE有效, period范围: period * (1 / 32768)us > 16 * 512us;
 * WiFi模块默认启用Tsensor EXT_TSENSOR_MODE_16_POINTS_SINGLE温度采集模式，重复调用此函数会覆盖WiFi模块使
 * 用的默认模式， 如要将Tsensor模块用于其他业务，可直接在该业务内注册对应的中断回调函数。CNend
 * @param  mode             [IN] type #ext_tsensor_mode，Mode of Tsensor temperature acquisition.
 *                                    CNcomment:Tsensor温度采集模式。CNend
 * @param  period           [IN] type #td_u16，Acquisition period, it is the multiple of 2 ms.
 *                                    CNcomment:温度自动检测周期，为32K时钟数。CNend
 * @retval #EXT_ERR_SUCCESS     Success.
 * @retval #Other               Failure. For details, see soc_errno.h.
 * @par Dependency:
 *            @li soc_tsensor.h：for Tsensor APIs.CNcomment:文件用于描述Tsensor相关接口。CNend
 * @see None
 */
td_u32 uapi_tsensor_start(ext_tsensor_mode mode, td_u16 period);

/*
 * @ingroup  iot_tsensor
 * @brief Read temperature data.CNcomment:读取温度值。CNend
 * @par Description:
 *          Read temperature data.CNcomment:在启动Tsensor温度采集后，读取温度值。CNend
 * @attention None
 * @param  temperature      [IN] type #td_s16*，address wait to be writed the temperature value.
 *                                    CNcomment:待写入温度值的地址。CNend
 * @retval #EXT_ERR_SUCCESS     Success.
 * @retval #Other               Failure. For details, see soc_errno.h.
 * @par Dependency:
 *            @li soc_tsensor.h：for Tsensor APIs.CNcomment:文件用于描述Tsensor相关接口。CNend
 * @see None
 */
td_u32 uapi_tsensor_read_temperature(td_s16* temperature);

/*
 * @ingroup  iot_tsensor
 * @brief Tsensor module stop.CNcomment:停止Tsensor温度采集。CNend
 * @par Description:
 *          Tsensor module stop.CNcomment:停止Tsensor温度采集。CNend
 * @attention
 *     1. By default, the Tsensor function is enabled for the Wi-Fi module. If this function is invoked, the
 *     temperature threshold protection function of the Wi-Fi module registration will be affected. \n
 *     CNcomment:1. WiFi模块默认启用Tsensor，调用此函数将会影响WiFi本身注册的温度阈值保护等功能。CNend \n
 *     2. The Tsensor function is strongly coupled with the chip temperature protection service. Tsensor enable status
 *     is checked every 100s. If the Tsensor is disabled or destroyed, it will be restarted. Users are not advised
 *     to disable the Tsensor by calling ext_tsensor_stop. \n
 *     CNcomment:2. Tsensor功能和芯片温度保护业务强耦合，系统定时100s周期检查Tsenor开关状态，如果Tsensor被关闭或销毁，
 *     则会重启Tsensor，如非必要，建议用户不要调用ext_tsensor_stop接口关闭Tesnsor。CNend
 * @param  None
 * @retval #EXT_ERR_SUCCESS     Success.
 * @retval #Other               Failure. For details, see soc_errno.h.
 * @par Dependency:
 *            @li soc_tsensor.h：for Tsensor APIs.CNcomment:文件用于描述Tsensor相关接口。CNend
 * @see None
 */
td_u32 uapi_tsensor_stop(td_void);

/*
 * @ingroup  iot_tsensor
 * @brief Destroy Tsensor module.CNcomment:销毁Tsensor模块。CNend
 * @par Description:
 *          On the basis of ext_tsensor_stop, this interface disables the reporting of Tsensor interrupts and clears
 *          the callback functions registered by users.
 *          CNcomment:在ext_tsensor_stop的基础上，此接口同时禁用了Tsensor的中断上报，并清空用户注册的回调函数。CNend
 * @attention
 *     1. After this API is called, if the interrupt callback function is used, you need to invoke the corresponding
 *     interface to set the interrupt function before starting the interrupt, and enable the interrupt reporting
 *     function.By default, the Tsensor function is enabled for the Wi-Fi module. If this function is invoked,
 *     the temperature threshold protection function of the Wi-Fi module registration will be affected. \n
 *     CNcomment:1. 调用此接口后，对于使用中断回调的场景，在start之前，需再次调用相应接口设置中断函数，开启中断上报；
 *     WiFi模块默认启用Tsensor，调用此函数将会影响WiFi本身注册的温度阈值保护等功能。CNend \n
 *     2. The Tsensor function is strongly coupled with the chip temperature protection service. Tsensor enable status
 *     is checked every 100s. If the Tsensor is disabled or destroyed, it will be restarted. Users are not advised
 *     to destroy the Tsensor by calling uapi_tsensor_destroy. \n
 *     CNcomment:2. Tsensor功能和芯片温度保护业务强耦合，系统定时100s周期检查Tsenor开关状态，如果Tsensor被关闭或销毁，
 *     则会重启Tsensor，如非必要，建议用户不要调用uapi_tsensor_destroy接口销毁Tesnsor。CNend
 * @param None
 * @retval #EXT_ERR_SUCCESS     Success.
 * @retval #Other               Failure. For details, see soc_errno.h.
 * @par Dependency:
 *            @li soc_tsensor.h：for Tsensor APIs.CNcomment:文件用于描述Tsensor相关接口。CNend
 * @see None
 */
td_u32 uapi_tsensor_destroy(td_void);

/*
 * @ingroup  iot_tsensor
 * @brief Sets the temperature calibration.CNcomment:温度校准设置。CNend
 * @par Description:
 *          Sets the temperature calibration.CNcomment:温度校准设置。CNend
 * @attention None
 * @param  trim_code          [IN] type #td_u8，Low 4bit valid. The corresponding temperature calibration values are
 *                            as follows:CNcomment:低4bit有效，对应的温度校准值如下所示:CNend \n
 *                            0000 --  0.000 ℃ \n
 *                            0001 --  1.410 ℃ \n
 *                            0010 --  2.820 ℃ \n
 *                            0011 --  4.230 ℃ \n
 *                            0100 --  5.640 ℃ \n
 *                            0101 --  7.050 ℃ \n
 *                            0110 --  8.460 ℃ \n
 *                            0111 --  9.870 ℃ \n
 *                            1000 --  0.000 ℃ \n
 *                            1001 -- -1.410 ℃ \n
 *                            1010 -- -2.820 ℃ \n
 *                            1011 -- -4.230 ℃ \n
 *                            1100 -- -5.640 ℃ \n
 *                            1101 -- -7.050 ℃ \n
 *                            1110 -- -8.460 ℃ \n
 *                            1111 -- -9.870 ℃ \n
 * @param  trim_sel           [IN] type #td_bool，0：The temp_trim of the Tsensor IP is directly loaded by the efuse.
 *                                                CNcomment:选择Tsensor IP的temp_trim由efuse直接加载，CNend
 *                                                1：The temp_trim of the Tsensor IP is configured by the register.
 *                                                CNcomment:选择Tsensor IP的temp_trim由寄存器配置。CNend
 * @retval #EXT_ERR_SUCCESS     Success.
 * @retval #Other               Failure. For details, see soc_errno.h.
 * @par Dependency:
 *            @li soc_tsensor.h：for Tsensor APIs.CNcomment:文件用于描述Tsensor相关接口。CNend
 * @see None
 */
td_u32 uapi_tsensor_set_temp_trim(td_u8 trim_code, td_bool trim_sel);

/*
 * @ingroup  iot_tsensor
 * @brief Sets the temperature threshold.CNcomment:设置温度阈值门限。CNend
 * @par Description:
 *          Sets the temperature threshold.CNcomment:设置温度阈值门限。CNend
 * @attention This function is invoked before Tsensor temperature collection is started. This function is invoked to
 *            enable the function of reporting the temperature threshold interrupt. A maximum of three temperature
 *            threshold interrupt callback functions can be registered at the same time. When the Wi-Fi service is
 *            started. \n
 *            CNcomment:在启动Tsensor温度采集之前调用，调用此函数将使能温度阈值门限中断上报, 最多可同时注册
 *            3个温度阈值门限中断回调函数。CNend
 * @param  low_temp            [IN] type #td_s16，Low Temperature Threshold.CNcomment:低温门限。CNend
 * @param  high_temp           [IN] type #td_s16，High Temperature Threshold.CNcomment:高温门限。CNend
 * @param  callback_func       [IN] type #ext_tsensor_callback_func，Indicates the callback function when the temperature
 *                             exceeds the threshold.CNcomment:温度超阈值中断回调函数。CNend
 * @retval #EXT_ERR_SUCCESS     Success.
 * @retval #Other               Failure. For details, see soc_errno.h.
 * @par Dependency:
 *            @li soc_tsensor.h：for Tsensor APIs.CNcomment:文件用于描述Tsensor相关接口。CNend
 * @see None
 */
td_u32 uapi_tsensor_set_outtemp_threshold(td_s16 low_temp, td_s16 high_temp,
                                          ext_tsensor_callback_func callback_func);

/*
 * @ingroup  iot_tsensor
 * @brief To set the threshold for the over-high temperature threshold.CNcomment:设置超高温阈值门限。CNend
 * @par Description:
 *          To set the threshold for the over-high temperature threshold.CNcomment:设置超高温阈值门限。CNend
 * @attention This function is invoked before Tsensor temperature collection is started. This function is invoked to
 *            enable the function of reporting the temperature threshold interrupt. A maximum of three temperature
 *            threshold interrupt callback functions can be registered at the same time. When the Wi-Fi service is
 *            started. \n
 *            CNcomment:在启动Tsensor温度采集之前调用，调用此函数将使能温度阈值门限中断上报, 最多可同时注册
 *            3个温度阈值门限中断回调函数。CNend
 * @param  over_temp           [IN] type #td_s16，Ultra-high temperature threshold.CNcomment:超高温门限。CNend
 * @param  callback_func       [IN] type #ext_tsensor_callback_func，Interrupt callback function when the temperature
 *                             exceeds the upper temperature threshold. CNcomment:温度超过超高温门限中断回调函数。CNend
 * @retval #EXT_ERR_SUCCESS     Success.
 * @retval #Other               Failure. For details, see soc_errno.h.
 * @par Dependency:
 *            @li soc_tsensor.h：for Tsensor APIs.CNcomment:文件用于描述Tsensor相关接口。CNend
 * @see None
 */
td_u32 uapi_tsensor_set_overtemp_threshold(td_s16 over_temp, ext_tsensor_callback_func callback_func);

/*
 * @ingroup  iot_tsensor
 * @brief Registers the callback function for the temperature collection completion interrupt.
 *        CNcomment:注册温度采集完成中断回调函数。CNend
 * @par Description:
 *          Registers the callback function for the temperature collection completion interrupt.
 *          CNcomment:注册温度采集完成中断回调函数。CNend
 * @attention This function is invoked before Tsensor temperature collection is started. This function is invoked to
 *            enable the function of reporting the temperature threshold interrupt. A maximum of three temperature
 *            threshold interrupt callback functions can be registered at the same time. The Tsensor temperature
 *            collection period is short. In EXT_TSENSOR_MODE_16_POINTS_LOOP and EXT_TSENSOR_MODE_1_POINT_LOOP modes,
 *            the collection completion interrupt is frequently triggered, occupying a large number of CPU resources.
 *            As a result, other services may fail to be scheduled. In EXT_TSENSOR_MODE_16_POINTS_SINGLE mode, a large
 *            period can be set to avoid this problem, however, if the value of period is too large, the temperature
 *            monitoring density decreases. Therefore, it is recommended that you do not read the temperature by
 *            collecting data. \n
 *            CNcomment:在启动Tsensor温度采集之前调用，调用此函数将使能温度采集完成中断上报,
 *            最多可同时注册3个温度采集完成中断回调函数;Tsensor温度采集周期很短，EXT_TSENSOR_MODE_16_POINTS_LOOP和
 *            EXT_TSENSOR_MODE_1_POINT_LOOP模式下采集完成中断会频繁触发，占用大量cpu资源，可能会导致其他业务得不到调度，
 *            EXT_TSENSOR_MODE_16_POINTS_SINGLE模式虽然可以通过设置较大的period来避免这个问题，但过大的period值将会导致温度监控密度
 *            的下降，因此建议尽量不要通过采集完成中断的方式读取温度。CNend
 * @param  callback_func       [IN] type #ext_tsensor_callback_func，Temperature collection completion interrupt callback
 *                             function.CNcomment:温度采集完成中断回调函数。CNend
 * @retval #EXT_ERR_SUCCESS     Success.
 * @retval #Other               Failure. For details, see soc_errno.h.
 * @par Dependency:
 *            @li soc_tsensor.h：for Tsensor APIs.CNcomment:文件用于描述Tsensor相关接口。CNend
 * @see None
 */
td_u32 uapi_tsensor_register_temp_collect_finish_int_callback(ext_tsensor_callback_func callback_func);

/*
 * @ingroup  iot_tsensor
 * @brief Registers the callback function for the temperature collection completion interrupt.
 *        CNcomment:设置多档温度阈值。CNend
 * @par Description:
 *          Registers the callback function for the temperature collection completion interrupt.
 *          CNcomment:设置多档温度阈值。CNend
 * @attention This function is invoked before Tsensor temperature collection is started.
 *            CNcomment:在启动Tsensor温度采集之前调用。 CNend
 * @param  level       [IN] type #ext_tsensor_multilevel_value，Multi-step temperature range value
 *                               CNcomment:多档温度档位值。CNend
 * @param  temp        [IN] type #td_s16, multi-step temperature threshold. CNcomment:多档温度阈值。CNend
 * @retval #EXT_ERR_SUCCESS     Success.
 * @retval #Other               Failure. For details, see soc_errno.h.
 * @par Dependency:
 *            @li soc_tsensor.h：for Tsensor APIs.CNcomment:文件用于描述Tsensor相关接口。CNend
 * @see None
 */
td_u32 uapi_tsensor_set_multilevel_threshold_value(ext_tsensor_multilevel_value level, td_s16 temp);

/*
 * @ingroup  iot_tsensor
 * @brief To set the threshold for the multi-step temperature threshold.CNcomment:设置多档温度回调。CNend
 * @par Description:
 *          To set the threshold for the multi-step temperature threshold.CNcomment:设置多档温度回调。CNend
 * @attention This function is invoked before Tsensor temperature collection is started. This function is invoked to
 *            enable the function of reporting the multi-step temperature threshold interrupt.
 *            CNcomment: 在启动Tsensor温度采集之前调用，调用此函数将使能多档温度门限中断上报。 CNend
 * @param  level         [IN] type #ext_tsensor_multilevel_en，multi-step temperature threshold.CNcomment:多档温度使能档位.
 * @param  callback_func [IN] type #ext_tsensor_callback_func，Interrupt callback function when the temperature
 *                             exceeds the setting multi-step temperature threshold. CNcomment:多档温度门限中断回调函数.
 * @retval #EXT_ERR_SUCCESS     Success.
 * @retval #Other               Failure. For details, see soc_errno.h.
 * @par Dependency:
 *            @li soc_tsensor.h：for Tsensor APIs.CNcomment:文件用于描述Tsensor相关接口。CNend
 * @see None
 */
td_u32 uapi_tsensor_set_multilevel_threshold_en(ext_tsensor_multilevel_en level,
    ext_tsensor_callback_func callback_func);

/*
 * @ingroup  iot_tsensor
 * @brief Check whether the temperature is a valid value.CNcomment:检查温度是否为有效值。CNend
 * @par Description:
 *          Check whether the temperature is a valid value.CNcomment:检查温度是否为有效值。CNend
 * @attention After Init
 *            CNcomment: tsensor初始化之后检查温度是否有更新。 CNend
 * @param  None
 * @retval None
 * @par Dependency:
 *            @li soc_tsensor.h：for Tsensor APIs.CNcomment:文件用于描述Tsensor相关接口。CNend
 * @see None
 */
td_void uapi_tsensor_wait_temperature_ready(td_void);

#ifdef __cplusplus
}
#endif

#endif
