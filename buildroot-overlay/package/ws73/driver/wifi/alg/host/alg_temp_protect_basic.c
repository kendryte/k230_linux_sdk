/*
 * Copyright (c) @CompanyNameMagicTag 2020-2022. All rights reserved.
 * Description: algorithm temperature protection
 */

#ifdef _PRE_WLAN_FEATURE_TEMP_PROTECT
/******************************************************************************
  1 其他头文件包含
******************************************************************************/
#include "alg_temp_protect.h"
#include "hmac_device.h"
#include "hal_common_ops.h"
#include "hmac_alg_notify.h"
#include "hal_ext_if.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_ALG_TEMP_PROTECT_BASIC_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST
/******************************************************************************
  2 宏定义
******************************************************************************/
#define ALG_TEMP_PROTECT_STATUS_PA_OFF 1
#define ALG_TEMP_PROTECT_STATUS_SAFE 0
/******************************************************************************
3 STRUCT定义
******************************************************************************/
typedef struct {
    frw_timeout_stru temp_protect_timer;   /* 温度检测定时器 */
    osal_s32 accumulate_temp;              /* 温度累加值 */
    osal_s16 ave_temp;                     /* 单周期内温度的平均值 */
    osal_s16 pa_off_th;                    /* PA_off门限值 */
    osal_u8 times;                         /* 采样（定时器）的次数 */
    osal_u8 times_th;                      /* 采样次数门限 */
    osal_u8 last_cycle_status;             /* 记录上一周期的状态 */
    osal_u8 resv;                          /* 对齐 */
} alg_temp_protect_basic_stru;
/******************************************************************************
  4 全局变量定义
******************************************************************************/
alg_temp_protect_basic_stru g_hmac_temp_protect_basic;

/******************************************************************************
  5 函数实现
******************************************************************************/
OAL_STATIC osal_u32 alg_temp_protect_basic_timer_handler(osal_void *arg);

/******************************************************************************
 功能描述  : 同步PA状态到device侧
******************************************************************************/
OAL_STATIC osal_void alg_temp_protect_switch_pa_off(alg_temp_protect_basic_stru *temp_protect, osal_u8 is_pa_off)
{
    osal_s32 ret = OAL_FAIL;
    frw_msg msg = {0};
    msg.data = &is_pa_off;
    msg.data_len = (osal_u16)sizeof(is_pa_off);

    if (is_pa_off == OSAL_TRUE) {
        temp_protect->times_th = ALG_TEMP_PROTECT_TIMES_TH_PA_OFF;
        temp_protect->last_cycle_status = ALG_TEMP_PROTECT_STATUS_PA_OFF;
    } else {
        temp_protect->times_th = ALG_TEMP_PROTECT_TIMES_TH_SAFE;
        temp_protect->last_cycle_status = ALG_TEMP_PROTECT_STATUS_SAFE;
    }

    ret = frw_send_msg_to_device(0, WLAN_MSG_H2D_C_CFG_DEVICE_TEMP_PA_OFF, &msg, OSAL_TRUE);
    if (osal_unlikely(ret != OAL_SUCC)) {
        oam_error_log1(0, OAM_SF_ANY, "{alg_temp_protect_switch_pa_off:: temp send fail [%d].}", ret);
    }
    return;
}

/******************************************************************************
 函 数 名  : alg_temp_protect_basic_cur_temp_get
 功能描述  : 过温保护获取当前温度
******************************************************************************/
OAL_STATIC osal_u32 alg_temp_protect_basic_cur_temp_get(alg_temp_protect_basic_stru *temp_protect,
    osal_s16 *temperature)
{
    unref_param(temp_protect);
    /* Tsensor 接口暂未适配 */
    if (osal_unlikely(hal_read_max_temperature(temperature) != OAL_SUCC)) {
        return OAL_FAIL;
    }
    return OAL_SUCC;
}

/******************************************************************************
 函 数 名  : alg_temp_protect_get_pa_off_th
 功能描述  : 过温保护获得当前PA_OFF门限值
******************************************************************************/
OAL_STATIC osal_s16 alg_temp_protect_get_pa_off_th(osal_void)
{
    oal_bool_enum_uint8 is_chip_cldo_mode = OSAL_FALSE;
    osal_s16 pa_off_th = ALG_TEMP_PROTECT_PA_OFF_TH;

#ifdef _PRE_WLAN_FEATURE_TEMP_PROTECT_CLDO_MODE
    is_chip_cldo_mode = hal_chip_is_cldo_mode();
#endif
    if (is_chip_cldo_mode == OSAL_TRUE) {
        pa_off_th -= ALG_TEMP_PROTECT_CUSTOM_TH_ADJUST;
    }

    return pa_off_th;
}

/******************************************************************************
 函 数 名  : alg_temp_protect_init
 功能描述  : 过温保护模块初始化
******************************************************************************/
__attribute__((weak)) osal_void alg_temp_protect_init(osal_void)
{
    alg_temp_protect_basic_stru *temp_protect = &g_hmac_temp_protect_basic;
    hal_to_dmac_device_stru *hal_device = hal_chip_get_hal_device();
    /* 算法注册 */
    if (hmac_alg_register(HMAC_ALG_ID_TEMP_PROTECT) != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_ANY, "{alg_temp_protect_init:: alg temp registered before.}");
        return;
    }
    memset_s(temp_protect, sizeof(alg_temp_protect_basic_stru), 0, sizeof(alg_temp_protect_basic_stru));

    /* 私有结构体赋值 */
    temp_protect->times_th = ALG_TEMP_PROTECT_TIMES_TH_SAFE;
    temp_protect->last_cycle_status = ALG_TEMP_PROTECT_STATUS_SAFE;
    temp_protect->pa_off_th = alg_temp_protect_get_pa_off_th();

    /* 创建定时器，定时器采样周期为1s */
    frw_create_timer_entry(&temp_protect->temp_protect_timer, alg_temp_protect_basic_timer_handler,
        ALG_TEMP_PROTECT_TIMER_CYCLE, (osal_void *)(hal_device), OSAL_TRUE);
}

/******************************************************************************
 函 数 名  : alg_temp_protect_exit
 功能描述  : 过温保护模块退出函数
******************************************************************************/
__attribute__((weak)) osal_void alg_temp_protect_exit(osal_void)
{
    alg_temp_protect_basic_stru *temp_protect = &g_hmac_temp_protect_basic;
    osal_u32 ret;
    /* 算法去注册 */
    ret = hmac_alg_unregister(HMAC_ALG_ID_TEMP_PROTECT);
    if (ret != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_ANY, "{alg_temp_protect_exit::unregister fail}");
        return;
    }
    /* 删定时器 */
    frw_destroy_timer_entry(&temp_protect->temp_protect_timer);
}

/******************************************************************************
 函 数 名  : alg_temp_protect_basic_timer_handler
 功能描述  : 过温保护模块定时器到期处理函数
******************************************************************************/
osal_u32 alg_temp_protect_basic_timer_handler(osal_void *arg)
{
    osal_s16 cur_temp = 0;
    alg_temp_protect_basic_stru *temp_protect = &g_hmac_temp_protect_basic;

    unref_param(arg);

    oam_info_log0(0, OAM_SF_ANY, "alg_temp_protect_basic_timer_handler");

    if (temp_protect->times < (temp_protect->times_th)) {
        if (alg_temp_protect_basic_cur_temp_get(temp_protect, &cur_temp) != OAL_SUCC) {
            return OAL_FAIL;
        }
        temp_protect->accumulate_temp = temp_protect->accumulate_temp + cur_temp;
        temp_protect->times += 1;
    } else {
        /* times_th 0 校验 */
        if (osal_unlikely(temp_protect->times_th == 0)) {
            oam_warning_log0(0, OAM_SF_ANY, "{alg_temp_protect_basic_timer_handler::times_th equals to 0.}");
            return OAL_FAIL;
        }
        temp_protect->ave_temp = (osal_s16)(temp_protect->accumulate_temp / (temp_protect->times_th));

        /* 超过PA_OFF门限，进入PA_OFF保护 */
        if ((temp_protect->ave_temp >= temp_protect->pa_off_th) &&
            (temp_protect->last_cycle_status != ALG_TEMP_PROTECT_STATUS_PA_OFF)) {
            oam_error_log1(0, OAM_SF_ANY, "{alg_temp_protect_basic_timer_handler::temperature PA off %d}",
                temp_protect->ave_temp);
            alg_temp_protect_switch_pa_off(temp_protect, OSAL_TRUE);
        } else if (temp_protect->last_cycle_status != ALG_TEMP_PROTECT_STATUS_SAFE) {
            /* 低于PA_OFF门限，返回SAFE状态 */
            alg_temp_protect_switch_pa_off(temp_protect, OSAL_FALSE);
        }

        temp_protect->accumulate_temp = 0;
        temp_protect->times = 0;
    }
    return OAL_SUCC;
}

#endif /* #ifdef _PRE_WLAN_FEATURE_TEMP_PRO */
