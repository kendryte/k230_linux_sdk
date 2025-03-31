/*
 * Copyright (c) CompanyNameMagicTag 2022-2023. All rights reserved.
 * Description: power control common
 */
#include "cali_online_common.h"
#include "fe_hal_gp_if.h"
#ifdef _PRE_PRODUCT_ID_HOST
#include "fe_extern_if_host.h"
#else
#include "hal_ext_if_rom.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*
 * 函 数 名   : cali_wait_ppa_lut_free
 * 功能描述   : 等待ppa lut寄存器表空闲
 */
osal_u32 cali_wait_ppa_lut_free(osal_void)
{
    osal_u16 delay = 0;
    osal_u32 reg_val = 0;

    for (;;) {
        /* 增加延时 */
        osal_udelay(2); /* 延时2us */
        reg_val = fe_hal_gp_get_ppa_lut_lock_state();
        /* 累加完成 */
        if ((reg_val & BIT0) == 0) {
            return OAL_SUCC;
        }

        /* 等待超时 */
        delay++;
        if (delay > CALI_WAIT_PPA_LUT_TIMEOUT) {
#ifdef _PRE_WLAN_RF_CALI_DEBUG
            OAM_ERROR_LOG1(0, OAM_SF_CALIBRATE,
                "{cali_wait_ppa_lut_free::wait ppa lut timeout reg[%d]!}\n", reg_val);
#endif
            return OAL_FAIL;
        }
    }
    return OAL_FAIL;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
