/*
 * Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
 * Description: 产测相关操作接口函数实现
 */
#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
#ifdef _PRE_WLAN_FEATURE_MFG_TEST
/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "hal_mfg.h"
#include "reg/c_union_define_cmu_ctl_rb.h"
#include "reg/cmu_ctl_rb_reg_offset.h"
#include "efuse_opt.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*****************************************************************************
功能描述  : 赋初始值到寄存器
*****************************************************************************/
osal_void hal_rf_set_default_cmu_xo_trim(osal_void)
{
    u_cmu_xo_sig val;

    val.u32 = hal_reg_read(CMU_CTL_RB_CMU_XO_SIG_REG);
    val.bits.rg_cmu_xo_trim_coarse = XO_TRIM_COARSE;
    val.bits.rg_cmu_xo_trim_fine = XO_TRIM_FINE;
    val.bits.rg_cmu_xo_trim_fine_sel = XO_TRIM_ENABLE;
    val.bits.rg_cmu_xo_trim_coarse_sel = XO_TRIM_ENABLE;
    hal_reg_write(CMU_CTL_RB_CMU_XO_SIG_REG, val.u32);
}

osal_void hal_set_mfg_mode(osal_u8 mode)
{
    if (mode == 1) {
        hal_rf_set_default_cmu_xo_trim();
    }
}

osal_void hal_set_xo_trim_fine(osal_u32 xo_trim_fine, osal_u32 *fine_reg)
{
    u_cmu_xo_sig val;

    val.u32 = hal_reg_read(CMU_CTL_RB_CMU_XO_SIG_REG);
    val.bits.rg_cmu_xo_trim_fine = xo_trim_fine;
    val.bits.rg_cmu_xo_trim_fine_sel = XO_TRIM_ENABLE;
    hal_reg_write(CMU_CTL_RB_CMU_XO_SIG_REG, val.u32);
    val.u32 = hal_reg_read(CMU_CTL_RB_CMU_XO_SIG_REG);
    *fine_reg = val.bits.rg_cmu_xo_trim_fine;
}

osal_void hal_set_xo_trim_coarse(osal_u32 xo_trim_coarse, osal_u32 *coarse_reg)
{
    u_cmu_xo_sig val;

    val.u32 = hal_reg_read(CMU_CTL_RB_CMU_XO_SIG_REG);
    val.bits.rg_cmu_xo_trim_coarse = xo_trim_coarse;
    val.bits.rg_cmu_xo_trim_coarse_sel = XO_TRIM_ENABLE;
    hal_reg_write(CMU_CTL_RB_CMU_XO_SIG_REG, val.u32);
    val.u32 = hal_reg_read(CMU_CTL_RB_CMU_XO_SIG_REG);
    *coarse_reg = val.bits.rg_cmu_xo_trim_coarse;
}

osal_void hal_get_xo_trim(osal_u32 *xo_trim_coarse, osal_u32 *xo_trim_fine)
{
    u_cmu_xo_sig val;

    val.u32 = hal_reg_read(CMU_CTL_RB_CMU_XO_SIG_REG);
    *xo_trim_coarse = val.bits.rg_cmu_xo_trim_coarse;
    *xo_trim_fine = val.bits.rg_cmu_xo_trim_fine;
}

osal_void hal_get_temp(osal_void)
{
    osal_s32 ret;
    osal_s16 temperature = 0;

    ret = (osal_s32)uapi_tsensor_read_temperature(&temperature);
    if (ret != OAL_SUCC) {
        wifi_printf_always("ERROR\r\n");
        return;
    }
    wifi_printf_always("OK\r\n");
    wifi_printf_always("temperature %d\r\n", temperature);
    return;
}

osal_s32 hal_efuse_cmu_xo_trim_write(osal_u8 xo_trim_coarse, osal_u8 xo_trim_fine)
{
    osal_s32 ret;
    osal_u64 state = 0;

    ret = uapi_efuse_get_lockstat(&state);
    if (ret != OAL_SUCC) {
        return ret;
    }

    if ((((osal_u64)1 << (EXT_EFUSE_LOCK_CMU_XO_TRIM_COARSE_ID - EXT_EFUSE_LOCK_CHIP_ID)) & state) == 0) {
        ret = (osal_s32)uapi_efuse_write(EXT_EFUSE_CMU_XO_TRIM_COARSE_ID, &xo_trim_coarse, sizeof(xo_trim_coarse));
        ret |= (osal_s32)uapi_efuse_write(EXT_EFUSE_CMU_XO_TRIM_FINE_ID, &xo_trim_fine, sizeof(xo_trim_fine));
    } else if ((((osal_u64)1 << (EXT_EFUSE_LOCK_XO_PPM_1_ID - EXT_EFUSE_LOCK_CHIP_ID)) & state) == 0) {
        ret = (osal_s32)uapi_efuse_write(EXT_EFUSE_XO_PPM_1_ID, &xo_trim_coarse, sizeof(xo_trim_coarse));
        ret |= (osal_s32)uapi_efuse_write(EXT_EFUSE_XO_PPM_2_ID, &xo_trim_fine, sizeof(xo_trim_fine));
    } else {
        oam_warning_log0(0, OAM_SF_ANY, "{hal_efuse_cmu_xo_trim_write::no left space!}");
        return OAL_FAIL;
    }

    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{hal_efuse_cmu_xo_trim_write::return err code [%d]!}", ret);
        return ret;
    }
    return OAL_SUCC;
}

const efuse_id_info_stru g_efuse_id_info[] = {
    {"jtag",             2,   {EXT_EFUSE_JTM_ID, EXT_EFUSE_MAX}, {EXT_EFUSE_LOCK_JTM_ID, EXT_EFUSE_LOCK_MAX}},
    {"ssi",              2,   {EXT_EFUSE_SSI_SPI_MASK_ID, EXT_EFUSE_MAX},
                              {EXT_EFUSE_LOCK_SSI_SPI_MASK_ID, EXT_EFUSE_LOCK_MAX}},
    {"temp",             4,   {EXT_EFUSE_XO_PPM_TEMPERATURE_1_ID, EXT_EFUSE_XO_PPM_TEMPERATURE_2_ID},
                              {EXT_EFUSE_LOCK_CMU_XO_TRIM_COARSE_ID, EXT_EFUSE_LOCK_XO_PPM_1_ID}},
    {"curve_factor_h",   4,   {EXT_EFUSE_CURV_FACTOR_HIGN_1_ID, EXT_EFUSE_CURV_FACTOR_HIGN_2_ID},
                              {EXT_EFUSE_LOCK_CURV_FACTOR_HIGN_1_ID, EXT_EFUSE_LOCK_CURV_FACTOR_HIGN_2_ID}},
    {"curve_factor_l",   4,   {EXT_EFUSE_CURV_FACTOR_LOW_1_ID, EXT_EFUSE_CURV_FACTOR_LOW_2_ID},
                              {EXT_EFUSE_LOCK_CURV_FACTOR_LOW_1_ID, EXT_EFUSE_LOCK_CURV_FACTOR_LOW_2_ID}},
    {"11b_hign",         16,  {EXT_EFUSE_11B_HIGN_1_ID, EXT_EFUSE_11B_HIGN_2_ID},
                              {EXT_EFUSE_LOCK_11B_HIGN_1_ID, EXT_EFUSE_LOCK_11B_HIGN_2_ID}},
    {"11b_low",          16,  {EXT_EFUSE_11B_LOW_1_ID, EXT_EFUSE_11B_LOW_2_ID},
                              {EXT_EFUSE_LOCK_11B_LOW_1_ID, EXT_EFUSE_LOCK_11B_LOW_2_ID}},
    {"ofdm_20M_hign",    16,  {EXT_EFUSE_OFDM_20M_HIGN_1_ID, EXT_EFUSE_OFDM_20M_HIGN_2_ID},
                              {EXT_EFUSE_LOCK_OFDM_20M_HIGN_1_ID, EXT_EFUSE_LOCK_OFDM_20M_HIGN_2_ID}},
    {"ofdm_20M_low",     16,  {EXT_EFUSE_OFDM_20M_LOW_1_ID, EXT_EFUSE_OFDM_20M_LOW_2_ID},
                              {EXT_EFUSE_LOCK_OFDM_20M_LOW_1_ID, EXT_EFUSE_LOCK_OFDM_20M_LOW_2_ID}},
    {"ofdm_40M_hign",    16,  {EXT_EFUSE_OFDM_40M_HIGN_1_ID, EXT_EFUSE_OFDM_40M_HIGN_2_ID},
                              {EXT_EFUSE_LOCK_OFDM_40M_HIGN_1_ID, EXT_EFUSE_LOCK_OFDM_40M_HIGN_2_ID}},
    {"ofdm_40M_low",     16,  {EXT_EFUSE_OFDM_40M_LOW_1_ID, EXT_EFUSE_OFDM_40M_LOW_2_ID},
                              {EXT_EFUSE_LOCK_OFDM_40M_LOW_1_ID, EXT_EFUSE_LOCK_OFDM_40M_LOW_2_ID}},
    {"rssi_offset_0",    5,   {EXT_EFUSE_RSSI_BAND1_1_ID, EXT_EFUSE_RSSI_BAND1_2_ID},
                              {EXT_EFUSE_LOCK_RSSI_BAND1_1_ID, EXT_EFUSE_LOCK_RSSI_BAND1_2_ID}},
    {"rssi_offset_1",    5,   {EXT_EFUSE_RSSI_BAND2_1_ID, EXT_EFUSE_RSSI_BAND2_2_ID},
                              {EXT_EFUSE_LOCK_RSSI_BAND2_1_ID, EXT_EFUSE_LOCK_RSSI_BAND2_2_ID}},
    {"rssi_offset_2",    5,   {EXT_EFUSE_RSSI_BAND3_1_ID, EXT_EFUSE_RSSI_BAND3_2_ID},
                              {EXT_EFUSE_LOCK_RSSI_BAND3_1_ID, EXT_EFUSE_LOCK_RSSI_BAND3_2_ID}},
    {"used_flag",        1,   {EXT_EFUSE_GROUP_USED_FLAG1_ID, EXT_EFUSE_GROUP_USED_FLAG2_ID},
                              {EXT_EFUSE_LOCK_MAX, EXT_EFUSE_LOCK_MAX}},
};

/*****************************************************************************
 函 数 名  : hal_efuse_read_parse_rssi_offset
 功能描述  : 获取efuse rssi校准参数
*****************************************************************************/
static osal_s32 hal_efuse_read_parse_rssi_offset(efuse_operate_stru *info)
{
    if (osal_strcmp("rssi_offset_0", info->efuse_name) == 0) {
        wifi_printf_always("OK\r\n");
        wifi_printf_always("%4d ", ((osal_s8)(info->val[0] << SIZE_3_BITS) >> SIZE_3_BITS));
    } else if (osal_strcmp("rssi_offset_1", info->efuse_name) == 0) {
        wifi_printf_always("%4d ", ((osal_s8)(info->val[0] << SIZE_3_BITS) >> SIZE_3_BITS));
    } else if (osal_strcmp("rssi_offset_2", info->efuse_name) == 0) {
        wifi_printf_always("%4d \r\n", ((osal_s8)(info->val[0] << SIZE_3_BITS) >> SIZE_3_BITS));
    } else {
        wifi_printf_always("FAIL\r\n");
        return OAL_FAIL;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hal_efuse_read_parse_temp
 功能描述  : 获取efuse产测温度
*****************************************************************************/
static osal_s32 hal_efuse_read_parse_temp(efuse_operate_stru *info)
{
    osal_u8 temperature = 0;

    temperature = (info->val[0] & 0xF);
    wifi_printf_always("OK\n");
    wifi_printf_always("temperature:%d\n", temperature);
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hal_efuse_read_parse_pwr_curve_factor
 功能描述  : 获取efuse功率放大系数
*****************************************************************************/
static osal_s32 hal_efuse_read_parse_pwr_curve_factor(efuse_operate_stru *info)
{
    osal_s8 curve_factor = 0;

    curve_factor = (osal_s8)(info->val[0] & 0xF);
    if (osal_strcmp("curve_factor_h", info->efuse_name) == 0) {
        wifi_printf_always("OK\n");
        wifi_printf_always("%4d ", curve_factor);
    } else if (osal_strcmp("curve_factor_l", info->efuse_name) == 0) {
        wifi_printf_always("%4d ", curve_factor);
    } else {
        return OAL_FAIL;
    }
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hal_efuse_read_parse_pwr_offset
 功能描述  : 获取efuse功率校准参数
*****************************************************************************/
static osal_s32 hal_efuse_read_parse_pwr_offset(efuse_operate_stru *info)
{
    osal_u8 index;
    osal_s16 constant_offset;
    osal_char *pwr_str[] = {"11b_hign", "11b_low", "ofdm_20M_hign", "ofdm_20M_low", "ofdm_40M_hign", "ofdm_40M_low"};

    constant_offset = (osal_s16)((info->val[1] << SIZE_8_BITS) + info->val[0]);
    for (index = 0; index < (sizeof(pwr_str) / sizeof(pwr_str[0])); index++) {
        if (osal_strcmp(pwr_str[index], info->efuse_name) == 0) {
            wifi_printf_always("%4d ", constant_offset);
        }
    }
    return OAL_SUCC;
}

static osal_s32 hal_efuse_read_jtag_enable(efuse_operate_stru *info)
{
    osal_u8 jtag_mask;

    jtag_mask = (osal_u8)(info->val[0] >> 1);
    wifi_printf_always("jtag_mask: %4d \n", jtag_mask);

    return OAL_SUCC;
}

static osal_s32 hal_efuse_read_ssi_enable(efuse_operate_stru *info)
{
    osal_u8 ssi_spi_mask;

    ssi_spi_mask = (osal_u8)(info->val[0] >> 1);
    wifi_printf_always("ssi_spi_mask: %4d \n", ssi_spi_mask);

    return OAL_SUCC;
}

const efuse_id_info_back_stru g_efuse_id_info_back[] = {
    {"jtag", 2, hal_efuse_read_jtag_enable},
    {"ssi", 2, hal_efuse_read_ssi_enable},
    {"temp", 4, hal_efuse_read_parse_temp},
    {"curve_factor_h", 4, hal_efuse_read_parse_pwr_curve_factor},
    {"curve_factor_l", 4, hal_efuse_read_parse_pwr_curve_factor},
    {"11b_hign", 16, hal_efuse_read_parse_pwr_offset},
    {"11b_low", 16, hal_efuse_read_parse_pwr_offset},
    {"ofdm_20M_hign", 16, hal_efuse_read_parse_pwr_offset},
    {"ofdm_20M_low", 16, hal_efuse_read_parse_pwr_offset},
    {"ofdm_40M_hign", 16, hal_efuse_read_parse_pwr_offset},
    {"ofdm_40M_low", 16, hal_efuse_read_parse_pwr_offset},
    {"rssi_offset_0", 5, hal_efuse_read_parse_rssi_offset},
    {"rssi_offset_1", 5, hal_efuse_read_parse_rssi_offset},
    {"rssi_offset_2", 5, hal_efuse_read_parse_rssi_offset},
};

/*****************************************************************************
 函 数 名  : hal_efuse_report_read_parse
 功能描述  : 调用efuse读取命令对应的结果处理函数
*****************************************************************************/
static osal_s32 hal_efuse_report_read_parse(efuse_operate_stru *info)
{
    osal_u8 idex;

    if (info == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_CFG, "{hal_efuse_report_read_parse::info is NULL.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    for (idex = 0; idex < sizeof(g_efuse_id_info_back) / sizeof(efuse_id_info_back_stru); idex++) {
        if (g_efuse_id_info_back[idex].func != NULL &&
            osal_strcmp(info->efuse_name, g_efuse_id_info_back[idex].efuse_name) == 0) {
            if (g_efuse_id_info_back[idex].func(info) == OAL_SUCC) {
                return OAL_SUCC;
            }
            return OAL_FAIL;
        }
    }
    return OAL_FAIL;
}

static const osal_u8 *hal_efuse_get_groupid_by_name(const osal_char *name, osal_u16 len)
{
    osal_u8 i;

    unref_param(len);
    for (i = 0; i < sizeof(g_efuse_id_info) / sizeof(efuse_id_info_stru); i++) {
        if (osal_strcmp(name, g_efuse_id_info[i].efuse_name) == 0) {
            return g_efuse_id_info[i].group_id;
        }
    }
    return OSAL_NULL;
}

static const osal_u8 *hal_efuse_get_lockid_by_name(const osal_char *name, osal_u16 len)
{
    osal_u8 i;

    unref_param(len);
    for (i = 0; i < sizeof(g_efuse_id_info) / sizeof(efuse_id_info_stru); i++) {
        if (osal_strcmp(name, g_efuse_id_info[i].efuse_name) == 0) {
            return g_efuse_id_info[i].lock_id;
        }
    }
    return OSAL_NULL;
}

static const osal_u32 hal_efuse_get_size_by_name(const osal_char *name, osal_u16 len)
{
    osal_u8 i;

    unref_param(len);
    for (i = 0; i < sizeof(g_efuse_id_info) / sizeof(efuse_id_info_stru); i++) {
        if (osal_strcmp(name, g_efuse_id_info[i].efuse_name) == 0) {
            return g_efuse_id_info[i].size;
        }
    }
    return 0;
}

/*****************************************************************************
功能描述  : 获取efuse剩余校准数据组数
*****************************************************************************/
osal_s32 hal_efuse_get_left_count(osal_u8 *left)
{
    osal_u8 loop;
    osal_s32 ret;
    osal_u8 count = 0;
    const osal_u8 *group_id = OSAL_NULL;
    osal_u8 flag = 0;

    group_id = hal_efuse_get_groupid_by_name("used_flag", EFUSE_NAME_LEN);
    if (group_id == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{hal_efuse_get_left_count, group_id invalid}");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    for (loop = 0; loop < EFUSE_GROUP_MAX; loop++) {
        ret = uapi_efuse_read(group_id[loop], &flag, sizeof(flag));
        if (ret != OAL_SUCC) {
            oam_error_log0(0, OAM_SF_ANY, "{hal_efuse_get_left_count, osal_efuse_usr_read fail}");
            return OAL_FAIL;
        }
        if (flag == 0) {
            count++;
        }
    }

    *left = count;
    return OAL_SUCC;
}

/*****************************************************************************
功能描述  : 获取efuse对应块剩余组数
*****************************************************************************/
osal_s32 hal_efuse_get_group_left_count(const osal_char *name, osal_u16 len, osal_u8 *left)
{
    osal_u8 i;
    osal_s32 ret;
    osal_u8 count = 0;
    const osal_u8 *group_id = OSAL_NULL;
    const osal_u8 *lock_id = OSAL_NULL;
    osal_u64 state = 0;

    group_id = hal_efuse_get_groupid_by_name(name, len);
    if (group_id == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{hal_efuse_get_group_left_count, group_id invalid}");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    lock_id = hal_efuse_get_lockid_by_name(name, len);
    if (lock_id == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{hal_efuse_get_group_left_count, lock_id invalid}");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    ret = uapi_efuse_get_lockstat(&state);
    if (ret != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_ANY, "{hal_efuse_get_group_left_count, get efuse stat fail}");
        return ret;
    }
    for (i = 0; i < EFUSE_GROUP_MAX; i++) {
        if (group_id[i] == EXT_EFUSE_MAX) {
            continue;
        }
        if (((1 << lock_id[i]) & state) == 0) {
            count++; /* 遍历两组PG，统计未锁定位的数量 */
        }
    }

    *left = count;
    return OAL_SUCC;
}

/*****************************************************************************
功能描述  : 获取合法efuse_id
*****************************************************************************/
static osal_s32 hal_efuse_get_valid_id(const osal_char *name, osal_u16 len, osal_u8 op, osal_u8 *id, osal_u8 *lc_id)
{
    osal_u8 i;
    osal_s32 ret;
    osal_u8 r_id = EXT_EFUSE_MAX;
    osal_u8 w_id = EXT_EFUSE_MAX;
    const osal_u8 *group_id = OSAL_NULL;
    const osal_u8 *lock_id = OSAL_NULL;
    osal_u8 stat = 0;
    osal_u8 is_zero = 0;

    group_id = hal_efuse_get_groupid_by_name(name, len);
    if (group_id == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{hal_efuse_get_valid_id, group_id invalid}");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    lock_id = hal_efuse_get_lockid_by_name(name, len);
    if (lock_id == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{hal_efuse_get_valid_id, lock_id invalid}");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    /* 遍历两组PG位，找出此次efuse读写操作对应的id */
    for (i = 0; i < EFUSE_GROUP_MAX; i++) {
        if (group_id[i] == EXT_EFUSE_MAX) {
            continue;
        }

        ret = uapi_efuse_read(lock_id[i], &stat, sizeof(osal_u8));
        if (ret != OAL_SUCC) {
            oam_error_log0(0, OAM_SF_ANY, "{hal_efuse_get_group_left_count, get efuse stat fail}");
            return ret;
        }
        if ((stat & 0x1) != 0) {
            r_id = group_id[i]; /* 记录读操作id，遍历两组，取最后一组 */
            is_zero++;
        } else if ((stat & 0x1) == 0) {
            if (w_id == EXT_EFUSE_MAX) {
                w_id = group_id[i]; /* 记录写操作id，遍历两组，取第一组 */
                *lc_id = lock_id[i];
            }
        }
    }
    if ((is_zero == 0) || (op == EFUSE_OP_WRITE)) {
        *id = w_id;  /* 如果两组都没锁定，或者是写操作，则操作id为写id的值 */
    } else {
        *id = r_id;
    }
    if (*id == EXT_EFUSE_MAX) {
        return OAL_FAIL;
    }
    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 读取efuse频偏产测校准信息
*****************************************************************************/
osal_s32 hal_read_efuse_cmu_xo_trim(osal_u8 *coarse_reg, osal_u8 *fine_reg)
{
    osal_s32 ret;
    osal_u64 state = 0;

    ret = uapi_efuse_get_lockstat(&state);
    if (ret != OAL_SUCC) {
        return ret;
    }

    if ((((osal_u64)1 << (EXT_EFUSE_LOCK_XO_PPM_1_ID  - EXT_EFUSE_LOCK_CHIP_ID)) & state) != 0) {
        ret = uapi_efuse_read(EXT_EFUSE_XO_PPM_1_ID, coarse_reg, sizeof(osal_u8));
        ret |= uapi_efuse_read(EXT_EFUSE_XO_PPM_2_ID, fine_reg, sizeof(osal_u8));
    } else if ((((osal_u64)1 << (EXT_EFUSE_LOCK_CMU_XO_TRIM_COARSE_ID - EXT_EFUSE_LOCK_CHIP_ID)) & state) != 0) {
        ret = uapi_efuse_read(EXT_EFUSE_CMU_XO_TRIM_COARSE_ID, coarse_reg, sizeof(osal_u8));
        ret |= uapi_efuse_read(EXT_EFUSE_CMU_XO_TRIM_FINE_ID, fine_reg, sizeof(osal_u8));
    }

    *coarse_reg = *coarse_reg & 0xF;
    if (ret != OAL_SUCC) {
        wifi_printf_always("ERROR\n");
        return ret;
    }

    return OAL_SUCC;
}

/*****************************************************************************
功能描述  : efuse写操作
*****************************************************************************/
osal_s32 hal_efuse_write_cmd(efuse_operate_stru *info, efuse_operate_stru *resp)
{
    osal_s32 ret;
    osal_u8 efuse_id = EXT_EFUSE_MAX;
    osal_u8 lock_id = EXT_EFUSE_LOCK_MAX;

    ret = hal_efuse_get_valid_id(info->efuse_name, sizeof(info->efuse_name), EFUSE_OP_WRITE, &efuse_id, &lock_id);
    if (ret != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_ANY, "{hal_efuse_write_cmd, invalid efuse id}");
        return OAL_FAIL;
    }
    ret = (osal_s32)uapi_efuse_write(efuse_id, info->val, EFUSE_MAX_LEN);
    if (ret != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_ANY, "{hal_efuse_write_cmd, efuse write fail}");
        return ret;
    }
    ret = uapi_efuse_lock(lock_id);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_ANY, "hal_efuse_write_cmd, lock efuse %d fail", efuse_id);
        return ret;
    }
    resp->val[0] = 'O';
    resp->val[1] = 'K';
    resp->len = 2; /* 长度为2 */
    return OAL_SUCC;
}

/*****************************************************************************
功能描述  : efuse读操作
*****************************************************************************/
osal_s32 hal_efuse_read_cmd(efuse_operate_stru *info, efuse_operate_stru *resp)
{
    osal_s32 ret;
    osal_u8 efuse_id = EXT_EFUSE_MAX;
    osal_u8 lock_id = EXT_EFUSE_LOCK_MAX;
    ret = hal_efuse_get_valid_id(info->efuse_name, sizeof(info->efuse_name), EFUSE_OP_READ, &efuse_id, &lock_id);
    if (ret != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_ANY, "{hal_efuse_read_cmd, invalid efuse id}");
        return ret;
    }
    ret = uapi_efuse_read(efuse_id, resp->val, EFUSE_MAX_LEN);
    if (ret != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_ANY, "{hal_efuse_read_cmd, efuse read fail}");
        return ret;
    }
    resp->len = hal_efuse_get_size_by_name(info->efuse_name, sizeof(info->efuse_name));
    if (resp->len == 0) {
        oam_error_log0(0, OAM_SF_ANY, "{hal_efuse_read_cmd, group_size fail}");
        return OAL_FAIL;
    }
    return OAL_SUCC;
}

/*****************************************************************************
功能描述  : 获取efuse对应块剩余组数
*****************************************************************************/
osal_s32 hal_efuse_remain_cmd(efuse_operate_stru *info, efuse_operate_stru *resp)
{
    osal_s32 ret = OAL_FAIL;
    osal_u8 lock_remain = 0;

    ret = hal_efuse_get_group_left_count(info->efuse_name, sizeof(info->efuse_name), &lock_remain);
    if (ret != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_ANY, "{hal_efuse_remain_cmd, group_left_count invalid}");
        return ret;
    }
    resp->val[0] = lock_remain;
    resp->len = sizeof(lock_remain);
    return OAL_SUCC;
}

osal_s32 hal_ccpriv_get_valid_mac(osal_void)
{
    osal_s32 ret;
    osal_u8 index;
    osal_u8 mac_idx = EXT_EFUSE_IPV4_MAC_ADDR_03_ID;
    osal_u8 mac_zero[OAL_MAC_ADDR_LEN] = {0};
    osal_u8 mac4_addr[OAL_MAC_ADDR_LEN] = {0};

    for (index = 0; index < WLAN_CFG_MAC_GROUP_NUM; index++) {
        if (mac_idx > EXT_EFUSE_IPV4_MAC_ADDR_03_ID || mac_idx < EXT_EFUSE_IPV4_MAC_ADDR_01_ID) {
            oam_error_log0(0, OAM_SF_ANY, "{hal_ccpriv_get_valid_mac, mac_idx error}");
            return OAL_FAIL;
        }
        ret = uapi_efuse_read(mac_idx, mac4_addr, sizeof(mac4_addr));
        if (ret != OAL_SUCC) {
            oam_error_log0(0, OAM_SF_ANY, "{hal_ccpriv_get_valid_mac, uapi_efuse_read_mac fail}");
            return OAL_FAIL;
        }
        if (osal_memcmp(mac_zero, mac4_addr, OAL_MAC_ADDR_LEN) == 0) {
            mac_idx--;
            continue;
        }
        break;
    }
    /* 1表示mac地址的第二位，2表示mac地址的第三位，3表示mac地址的第四位 */
    wifi_printf_always("Mac Addr: %02x:%02x:%02x:%02x:**:**\n", mac4_addr[0], mac4_addr[1], mac4_addr[2], mac4_addr[3]);
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hal_efuse_read_parse_all_mfg_data
 功能描述  : 输出所有校准数据
*****************************************************************************/
static osal_s32 hal_efuse_read_parse_all_mfg_data(efuse_operate_stru *info)
{
    osal_s32 ret;
    osal_u8 remain;
    efuse_mfg_data_stru *mfg_data = OSAL_NULL;
    osal_u8 coarse_reg = 0;
    osal_u8 fine_reg = 0;

    if (info == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_CFG, "{hal_efuse_read_parse_all_mfg_data::info is NULL.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    mfg_data = (efuse_mfg_data_stru *)info->val;
    wifi_printf_always("OK\n");
    ret = hal_efuse_get_left_count(&remain);
    if (ret != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_ANY, "{hal_efuse_read_parse_all_mfg_data, group_left_count invalid}");
        return OAL_FAIL;
    }

    ret = hal_read_efuse_cmu_xo_trim(&coarse_reg, &fine_reg);
    if (ret != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_ANY, "{hal_efuse_read_parse_all_mfg_data, freq_offset read fail!}");
        return OAL_FAIL;
    }

    wifi_printf_always("Group left count: %d\n", remain);
    wifi_printf_always("Power Calibration Param:\n");
    wifi_printf_always("Curve_factor:%4d %4d\n",
        (mfg_data->pwr_offset.curve_factor_hig & 0xF), mfg_data->pwr_offset.curve_factor_low & 0xF);
    wifi_printf_always("11b_constant_offset: %4d %4d\n",
        (osal_s16)(mfg_data->pwr_offset.band1_hign[0] + (mfg_data->pwr_offset.band1_hign[1] << SIZE_8_BITS)),
        (osal_s16)(mfg_data->pwr_offset.band1_low[0] + (mfg_data->pwr_offset.band1_low[1] << SIZE_8_BITS)));
    wifi_printf_always("ofdm_20M_constant_offset: %4d %4d\n",
        (osal_s16)(mfg_data->pwr_offset.ofdm_20m_hign[0] + (mfg_data->pwr_offset.ofdm_20m_hign[1] << SIZE_8_BITS)),
        (osal_s16)(mfg_data->pwr_offset.ofdm_20m_low[0] + (mfg_data->pwr_offset.ofdm_20m_low[1] << SIZE_8_BITS)));
    wifi_printf_always("ofdm_40M_constant_offset: %4d %4d\n",
        (osal_s16)(mfg_data->pwr_offset.ofdm_40m_hign[0] + (mfg_data->pwr_offset.ofdm_40m_hign[1] << SIZE_8_BITS)),
        (osal_s16)(mfg_data->pwr_offset.ofdm_40m_low[0] + (mfg_data->pwr_offset.ofdm_40m_low[1] << SIZE_8_BITS)));
    wifi_printf_always("Temp: %4d\n", (mfg_data->temp & 0xF));
    wifi_printf_always("Freq Calibration Param: %4d %4d\n", coarse_reg, fine_reg);
    wifi_printf_always("Rssi Calibration Param: %4d %4d %4d\n",
        ((osal_s8)(mfg_data->rssi_offset.rssi_offset_0 << SIZE_3_BITS) >> SIZE_3_BITS),
        ((osal_s8)(mfg_data->rssi_offset.rssi_offset_1 << SIZE_3_BITS) >> SIZE_3_BITS),
        ((osal_s8)(mfg_data->rssi_offset.rssi_offset_2 << SIZE_3_BITS) >> SIZE_3_BITS));

    ret = hal_ccpriv_get_valid_mac();
    if (ret != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_ANY, "{hal_efuse_read_parse_all_mfg_data, uapi_efuse_read_mac fail}");
        return OAL_FAIL;
    }
    return OAL_SUCC;
}

osal_s32 hal_config_efuse_report(const hmac_vap_stru *hmac_vap, efuse_operate_stru *read_result)
{
    osal_s32 ret;

    switch (read_result->op) {
        case EFUSE_OP_READ:
            ret = hal_efuse_report_read_parse(read_result);
            if (ret != OAL_SUCC) {
                return OAL_FAIL;
            }
            break;
        case EFUSE_OP_WRITE:
            if (osal_strcmp((const osal_char *)read_result->val, "OK") == 0) {
                return OAL_SUCC;
            } else {
                return OAL_FAIL;
            }
            break;
        case EFUSE_OP_REMAIN:
            wifi_printf_always("OK\n");
            wifi_printf_always("Group left count : %d\n", read_result->val[0]);
            break;
        case EFUSE_OP_READ_ALL_MFG_DATA:
            ret = hal_efuse_read_parse_all_mfg_data(read_result);
            if (ret != OAL_SUCC) {
                return OAL_FAIL;
            }
            break;
        default:
            oam_error_log1(0, OAM_SF_ANY, "{hal_config_efuse_report, unknown efuse operation %d}", read_result->op);
            return OAL_FAIL;
    }
    return OAL_SUCC;
}

/*****************************************************************************
功能描述  : 获取efuse所有校准数据
*****************************************************************************/
osal_s32 hal_efuse_get_all_mfg_data(efuse_operate_stru *info)
{
    efuse_mfg_data_stru *mfg_data = (efuse_mfg_data_stru *)info->val;
    osal_s32 ret;
    osal_u8 idex;
    osal_u8 efuse_id = EXT_EFUSE_MAX;
    osal_u8 lock_id = EXT_EFUSE_LOCK_MAX;
    const efuse_mfg_data_status ptr[] = {
        {NULL, 0}, {NULL, 0}, {(osal_u8 *)&mfg_data->temp, sizeof(mfg_data->temp)},
        {(osal_u8 *)&mfg_data->pwr_offset.curve_factor_hig, sizeof(mfg_data->pwr_offset.curve_factor_hig)},
        {(osal_u8 *)&mfg_data->pwr_offset.curve_factor_low, sizeof(mfg_data->pwr_offset.curve_factor_low)},
        {(osal_u8 *)mfg_data->pwr_offset.band1_hign, sizeof(mfg_data->pwr_offset.band1_hign)},
        {(osal_u8 *)mfg_data->pwr_offset.band1_low, sizeof(mfg_data->pwr_offset.band1_low)},
        {(osal_u8 *)mfg_data->pwr_offset.ofdm_20m_hign, sizeof(mfg_data->pwr_offset.ofdm_20m_hign)},
        {(osal_u8 *)mfg_data->pwr_offset.ofdm_20m_low, sizeof(mfg_data->pwr_offset.ofdm_20m_low)},
        {(osal_u8 *)mfg_data->pwr_offset.ofdm_40m_hign, sizeof(mfg_data->pwr_offset.ofdm_40m_hign)},
        {(osal_u8 *)mfg_data->pwr_offset.ofdm_40m_low, sizeof(mfg_data->pwr_offset.ofdm_40m_low)},
        {(osal_u8 *)&mfg_data->rssi_offset.rssi_offset_0, sizeof(mfg_data->rssi_offset.rssi_offset_0)},
        {(osal_u8 *)&mfg_data->rssi_offset.rssi_offset_1, sizeof(mfg_data->rssi_offset.rssi_offset_1)},
        {(osal_u8 *)&mfg_data->rssi_offset.rssi_offset_2, sizeof(mfg_data->rssi_offset.rssi_offset_2)}, {NULL, 0}
    };

    for (idex = 0; idex < sizeof(g_efuse_id_info) / sizeof(g_efuse_id_info[0]); idex++) {
        if (ptr[idex].data == NULL) {
            continue;
        }
        ret = hal_efuse_get_valid_id(g_efuse_id_info[idex].efuse_name,
            EFUSE_NAME_LEN, EFUSE_OP_READ, &efuse_id, &lock_id);
        if (ret != OAL_SUCC) {
            oam_error_log0(0, OAM_SF_ANY, "{hal_efuse_get_all_mfg_data, invalid efuse id}");
            return OAL_FAIL;
        }
        ret = uapi_efuse_read(efuse_id, ptr[idex].data, ptr[idex].len);
        if (ret != OAL_SUCC) {
            oam_error_log0(0, OAM_SF_ANY, "{hal_efuse_get_all_mfg_data, efuse read fail}");
            return ret;
        }
        ret = hal_efuse_get_group_left_count(g_efuse_id_info[idex].efuse_name,
            EFUSE_NAME_LEN, &mfg_data->group_left[idex]);
        if (ret != OAL_SUCC) {
            oam_error_log0(0, OAM_SF_ANY, "{hal_efuse_get_all_mfg_data, group_left_count invalid}");
            return OAL_FAIL;
        }
    }
    return OAL_SUCC;
}

/*****************************************************************************
功能描述  : efuse操作处理
*****************************************************************************/
osal_s32 hal_config_efuse_operate(hmac_vap_stru *hmac_vap, efuse_operate_stru *param)
{
    osal_s32 ret = OAL_FAIL;
    efuse_operate_stru read_report;
    osal_bool need_report = OAL_FALSE;

    (osal_void)memset_s(&read_report, sizeof(read_report), 0, sizeof(read_report));
    if (hmac_vap == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_RRM, "{hal_config_efuse_operate::hmac_vap NULL!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    switch (param->op) {
        case EFUSE_OP_WRITE:
            ret = hal_efuse_write_cmd(param, &read_report);
            need_report = OAL_TRUE;
            break;
        case EFUSE_OP_READ:
            ret = hal_efuse_read_cmd(param, &read_report);
            need_report = OAL_TRUE;
            break;
        case EFUSE_OP_REMAIN:
            ret = hal_efuse_remain_cmd(param, &read_report);
            need_report = OAL_TRUE;
            break;
        case EFUSE_OP_READ_ALL_MFG_DATA:
            ret = hal_efuse_get_all_mfg_data(&read_report);
            need_report = OAL_TRUE;
            break;
        default:
            oam_error_log1(0, OAM_SF_ANY, "{hal_config_efuse_operate, unkown efuse operate %d}", param->op);
            return OAL_FAIL;
    }
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_ANY, "{hal_config_efuse_operate, efuse op %d fail}", param->op);
        return ret;
    }
    if (need_report == OAL_TRUE) {
        (osal_void)memcpy_s(read_report.efuse_name, sizeof(read_report.efuse_name),
            param->efuse_name, sizeof(param->efuse_name));
        read_report.op = param->op;
        ret = hal_config_efuse_report(hmac_vap, &read_report);
        if (ret != OAL_SUCC) {
            oam_warning_log2(0, OAM_SF_CFG,
                "vap_id[%d] {hal_config_efuse_operate::hal_config_efuse_report failed[%d].}", hmac_vap->vap_id, ret);
            return ret;
        }
    }
    return OAL_SUCC;
}

osal_s32 hal_efuse_write_mfg_flag(osal_void)
{
    osal_s32 ret;
    osal_u8 index;
    osal_u8 flag = 0;
    const osal_u8 *group_id = OSAL_NULL;

    group_id = hal_efuse_get_groupid_by_name("used_flag", EFUSE_MAX_LEN);
    if (group_id == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_ccpriv_efuse_write_mfg_flag, group_id invalid}");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }
    for (index = 0; index < EFUSE_GROUP_MAX; index++) {
        ret = (osal_s32)uapi_efuse_read(group_id[index], &flag, sizeof(flag));
        if (ret == OAL_SUCC && flag == 0) {
            flag = 1;
            ret = (osal_s32)uapi_efuse_write(group_id[index], &flag, sizeof(flag));
            if (ret != OAL_SUCC) {
                return ret;
            }
            break;
        }
    }
    return OAL_SUCC;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif
#endif