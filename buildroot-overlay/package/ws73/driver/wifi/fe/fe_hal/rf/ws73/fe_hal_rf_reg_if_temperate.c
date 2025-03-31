/*
 * Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
 * Description: ws73 rf reg interface and struct adaption for temperate comp
*/
#include "fe_hal_rf_reg_if_temperate.h"
#include "hal_rf_reg.h"
#include "hal_reg_opt.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif
/*
* Function    : hal_rf_set_abb127_d_wb_rf_temp_code_bank_sel
* Description : Set the value of the member u_rf_abb127_union.d_wb_rf_temp_code_bank_sel
* Input       : osal_u16 d_wb_rf_temp_code_bank_sel
* Return      : osal_void
*/
osal_void hal_rf_set_abb127_d_wb_rf_temp_code_bank_sel(osal_u16 x4OqyOwtORmIGOCv4mOyzodOsmx_)
{
    u_rf_abb127_union val;

    val.u16 = hal_reg_read16(HH503_RF_ABB_REG_BASE_0x1FC);
    val.bits.x4OqyOwtORmIGOCv4mOyzodOsmx_ = x4OqyOwtORmIGOCv4mOyzodOsmx_;
    hal_reg_write16(HH503_RF_ABB_REG_BASE_0x1FC, val.u16);
}

/*
* Function    : hal_rf_get_abb127_d_wb_rf_temp_code_bank_sel
* Description : Get the value of the member u_rf_abb127_union.d_wb_rf_temp_code_bank_sel
* Input       : osal_void
* Return      : osal_u16 d_wb_rf_temp_code_bank_sel
*/
osal_u16 hal_rf_get_abb127_d_wb_rf_temp_code_bank_sel(osal_void)
{
    u_rf_abb127_union val;

    val.u16 = hal_reg_read16(HH503_RF_ABB_REG_BASE_0x1FC);
    return val.bits.x4OqyOwtORmIGOCv4mOyzodOsmx_;
}
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif