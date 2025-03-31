/*
 * Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
 * Description: tsensor driver implementatioin.
 * Author: CompanyName
 * Create: 2023-02-15
 */

#if defined(CONFIG_SUPPORT_TSENSOR)

#include "tsensor.h"
#include <soc_tsensor.h>
#include "soc_resource_id.h"
#include "soc_osal.h"
#include "plat_misc.h"

#define LOW_CODE 114
#define HIGH_CODE 896
#define LOW_TEMP (-40)
#define HIGH_TEMP 125
#define TMP_BASE_ADDR 0x40018800
#define TIME_S_TO_MS 1000

td_s16 g_temperature_record;

td_u32 uapi_tsensor_read_temperature(td_s16 *temperature)
{
#ifdef CONFIG_HCC_SUPPORT_UART
    osal_u32 ret;
    osal_u8 in_data = 0;
    plat_msg_ctl_struct *temp_ctl_ptr = plat_get_temp_ctl_ptr();
    plat_data_len_struct input_data = {.data = &in_data, .data_len = sizeof(osal_u8)};
    plat_data_len_struct output_data = {.data = (td_u8 *)temperature, .data_len = sizeof(td_s16)};
 
    if (temperature == TD_NULL) {
        return EXT_ERR_TSENSOR_INVALID_PARAMETER;
    }
 
    ret = send_message_to_device(&input_data, H2D_PLAT_CFG_MSG_GET_TEMP, temp_ctl_ptr,
        &output_data);
    if (ret != EXT_ERR_SUCCESS) {
        oal_print_err("{uapi_ccpriv_get_temp failed!}\r\n");
        return ret;
    }
 
    return EXT_ERR_SUCCESS;
#else
    td_u16 temperature_code;
    td_u32 tmp_val = 0;

    if (temperature == TD_NULL) {
        return EXT_ERR_TSENSOR_INVALID_PARAMETER;
    }

    if (hcc_read_reg(HCC_CHANNEL_AP, TMP_BASE_ADDR + TSENSOR_AUTO_STS, &tmp_val) != EXT_ERR_SUCCESS) {
        return EXT_ERR_FAILURE;
    }
    if (tmp_val & (1 << TSENSOR_READY_POS)) {
        temperature_code = (td_u16)(tmp_val >> TSENSOR_READY_DATA_OFFSET_2BITS);
        *temperature = ((temperature_code - LOW_CODE) * (HIGH_TEMP - LOW_TEMP) / (HIGH_CODE - LOW_CODE) + LOW_TEMP);
        g_temperature_record = *temperature;
        return EXT_ERR_SUCCESS;
    }
    *temperature = g_temperature_record;
    return EXT_ERR_SUCCESS;
#endif
}

osal_module_export(uapi_tsensor_read_temperature);

#endif
