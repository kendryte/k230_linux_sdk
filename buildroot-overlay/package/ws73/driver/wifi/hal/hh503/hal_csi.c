/*
 * Copyright (c) CompanyNameMagicTag 2021-2023. All rights reserved.
 * Description: MAC/CSI initialization and adapt interface implementation.
 * Create: 2021-11-29
 */

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "hal_csi.h"
#include "hal_ext_if.h"
#include "oal_ext_if.h"
#include "oam_ext_if.h"

#include "wlan_msg.h"

#include "hal_mac_reg.h"
#include "dmac_ext_if_type.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HAL_CSI_C

#define STATIC static

STATIC osal_void hh503_csi_set_enable(osal_u8 usr_idx, oal_bool_enum_uint8 switch_flag);
STATIC osal_void hh503_csi_set_mac_frm_type_filter(
    osal_u8 usr_idx, wlan_frame_type_enum frame_type, oal_bool_enum_uint8 flag);
STATIC osal_void hh503_csi_set_whitelist(osal_u8 usr_idx, const osal_u8 *mac_addr);
STATIC osal_void hh503_csi_set_usr_ppdu_format(osal_u8 usr_idx, osal_u8 ppdu_type, oal_bool_enum_uint8 flag);
STATIC osal_void hh503_csi_set_usr_frm_subtype(osal_u8 usr_idx, osal_u16 subtype, oal_bool_enum_uint8 flag);
STATIC osal_void hh503_csi_set_sample_period(osal_u8 usr_idx, osal_u16 period);

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
osal_u16 g_csi_max_len = CSI_DATA_MAX_LEN;      /* 初值默认配置为762字节，HE最大子载波上报所需内存 */
osal_u8 g_csi_blk_num = CSI_DATA_BLK_NUM;       /* 内存块数默认配置为2块，NETBUF内存池最大1600左右，与单块大小的乘积不能超过 */

/*****************************************************************************
  3 函数实现
*****************************************************************************/
/*****************************************************************************
 功能描述  : CSI配置类接口
 CSI接口设计分两大类：配置类和上报类
*****************************************************************************/

/*****************************************************************************
 功能描述  : 设置各白名单用户对应的CSI上报使能
*****************************************************************************/
STATIC osal_void hh503_csi_set_enable(osal_u8 user_index, oal_bool_enum_uint8 switch_flag)
{
    osal_u32 switch_bitmap = 0x1 << user_index;
    u_csi_en reg_val = {{0}};

    reg_val.u32 = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x14C);
    if (switch_flag == OSAL_TRUE) {
        reg_val.bits.cfg_csi_en |= switch_bitmap;
    } else {
        reg_val.bits.cfg_csi_en &= (osal_u32)(~switch_bitmap);
    }
    hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x14C, reg_val.u32);
}

/*****************************************************************************
 功能描述  : 获取各白名单用户对应的CSI上报使能
*****************************************************************************/
STATIC osal_void hh503_csi_get_enable(osal_u8 user_index, oal_bool_enum_uint8 *switch_flag)
{
    osal_u32 switch_bitmap = 0x1 << user_index;
    u_csi_en reg_val = {{0}};

    reg_val.u32 = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x14C);
    if (reg_val.bits.cfg_csi_en & switch_bitmap) {
        *switch_flag = OSAL_TRUE;
    } else {
        *switch_flag = OSAL_FALSE;
    }
}

/*****************************************************************************
 功能描述  : 获取各白名单用户对应的CSI上报使能
*****************************************************************************/
osal_bool hal_csi_vap_is_close(osal_void)
{
    oal_bool_enum_uint8 flag;
    int usr_id;

    for (usr_id = 0; usr_id < HAL_CSI_MAX_USER_NUM; ++usr_id) {
        hh503_csi_get_enable(usr_id, &flag);
        if (flag) {
            return OAL_FALSE;
        }
    }
    return OAL_TRUE;
}


/*****************************************************************************
 功能描述  : 设置CSI各白名单TA地址匹配功能bypass开关
*****************************************************************************/
STATIC osal_void hh503_csi_set_addr_match_bypass(osal_u8 user_index, oal_bool_enum_uint8 bypass_flag)
{
    osal_u32 switch_bitmap = 0x1 << user_index;
    u_csi_en reg_val = {{0}};

    reg_val.u32 = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x14C);
    if (bypass_flag == OSAL_TRUE) {
        reg_val.bits.xCt2OCslOz44wOIzRCAOyQGzss_ |= switch_bitmap;
    } else {
        reg_val.bits.xCt2OCslOz44wOIzRCAOyQGzss_ &= (~switch_bitmap);
    }
    hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x14C, reg_val.u32);
}

/*****************************************************************************
功能描述  : 获取CSI各白名单TA地址匹配功能bypass开关
*****************************************************************************/
STATIC osal_void hh503_csi_get_addr_match_bypass(osal_u8 user_index, oal_bool_enum_uint8 *bypass_flag)
{
    osal_u32 switch_bitmap = 0x1 << user_index;
    u_csi_en reg_val = {{0}};

    reg_val.u32 = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x14C);
    if (reg_val.bits.xCt2OCslOz44wOIzRCAOyQGzss_ & switch_bitmap) {
        *bypass_flag = OSAL_TRUE;
    } else {
        *bypass_flag = OSAL_FALSE;
    }
}

/*****************************************************************************
 功能描述  : 设置CSI白名单地址过滤类型 0 RA 1 TA
*****************************************************************************/
STATIC osal_void hh503_csi_set_addr_filter_type(osal_u8 user_index, oal_bool_enum_uint8 addr_filter_type)
{
    osal_u32 switch_bitmap = 0x1 << user_index;
    u_csi_user_frm_subtype reg_val = {{0}};

    reg_val.u32 = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x150);
    if (addr_filter_type == OSAL_TRUE) {
        reg_val.bits.xCt2OCslOSswOIzRCAORzOwzOsmx_ |= switch_bitmap;
    } else {
        reg_val.bits.xCt2OCslOSswOIzRCAORzOwzOsmx_ &= (~switch_bitmap);
    }
    hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x150, reg_val.u32);
}

/*****************************************************************************
 功能描述  : 获取CSI白名单地址过滤类型 0 RA 1 TA
*****************************************************************************/
STATIC osal_void hh503_csi_get_addr_filter_type(osal_u8 user_index, oal_bool_enum_uint8 *addr_filter_type)
{
    osal_u32 switch_bitmap = 0x1 << user_index;
    u_csi_user_frm_subtype reg_val = {{0}};

    reg_val.u32 = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x150);
    if (reg_val.bits.xCt2OCslOSswOIzRCAORzOwzOsmx_ & switch_bitmap) {
        *addr_filter_type = OSAL_TRUE;
    } else {
        *addr_filter_type = OSAL_FALSE;
    }
}


/* 依据条件表达式将一个原始数和开关比特位按位进行操作, if 条件 ture, 按位或, else 按位与 */
#define toggle_bitmap(condition_expr, orig_bits, switch_bitmap) ({ \
        osal_u32 _orig_bits = (osal_u32)(orig_bits); \
        osal_u32 _switch_bitmap = (osal_u32)(switch_bitmap); \
        oal_bool_enum_uint8 _condition_expr = (oal_bool_enum_uint8)(condition_expr); \
        _condition_expr ? (_orig_bits | _switch_bitmap) : (_orig_bits & (~_switch_bitmap)); \
    })

/*****************************************************************************
 功能描述  : 设置CSI白名单用户的MAC帧类型过滤使能
             注:写 cfg_csi_usr[0~5]_frm_type 寄存器
*****************************************************************************/
STATIC osal_void hh503_csi_set_mac_frm_type_filter(
    osal_u8 user_index, wlan_frame_type_enum frame_type, oal_bool_enum_uint8 flag)
{
    osal_u32 bitmap = 0x1 << frame_type;
    u_csi_en reg_val = {{0}};

    reg_val.u32 = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x14C);
    if (user_index == HAL_CSI_USR_0) {
        reg_val.bits.x6Ngw6CBwdC_ywN_SwLtuW_ = toggle_bitmap(flag, reg_val.bits.x6Ngw6CBwdC_ywN_SwLtuW_, bitmap);
    } else if (user_index == HAL_CSI_USR_1) {
        reg_val.bits.xCt2OCslOSswrOtwIORQGm_ = toggle_bitmap(flag, reg_val.bits.xCt2OCslOSswrOtwIORQGm_, bitmap);
    } else if (user_index == HAL_CSI_USR_2) {
        reg_val.bits.x6Ngw6CBwdC_VwN_SwLtuW_ = toggle_bitmap(flag, reg_val.bits.x6Ngw6CBwdC_VwN_SwLtuW_, bitmap);
    } else if (user_index == HAL_CSI_USR_3) {
        reg_val.bits.x6Ngw6CBwdC_kwN_SwLtuW_ = toggle_bitmap(flag, reg_val.bits.x6Ngw6CBwdC_kwN_SwLtuW_, bitmap);
    } else {
    }
    hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x14C, reg_val.u32);
}

/*****************************************************************************
 功能描述  : 获取CSI白名单用户的MAC帧类型过滤使能
*****************************************************************************/
STATIC osal_void hh503_csi_get_mac_frm_type_filter(
    osal_u8 user_index, wlan_frame_type_enum frame_type, oal_bool_enum_uint8 *flag)
{
    osal_u32 bitmap = 0x1 << frame_type;
    osal_u32 frm_type_en_flag = 0;
    u_csi_en reg_val = {{0}};

    reg_val.u32 = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x14C);
    if (user_index == HAL_CSI_USR_0) {
        frm_type_en_flag = reg_val.bits.x6Ngw6CBwdC_ywN_SwLtuW_;
    } else if (user_index == HAL_CSI_USR_1) {
        frm_type_en_flag = reg_val.bits.xCt2OCslOSswrOtwIORQGm_;
    } else if (user_index == HAL_CSI_USR_2) {
        frm_type_en_flag = reg_val.bits.x6Ngw6CBwdC_VwN_SwLtuW_;
    } else if (user_index == HAL_CSI_USR_3) {
        frm_type_en_flag = reg_val.bits.x6Ngw6CBwdC_kwN_SwLtuW_;
    } else {
    }

    *flag = (frm_type_en_flag & bitmap) ? OSAL_TRUE : OSAL_FALSE;
}

/*****************************************************************************
 功能描述  : 设置CSI白名单用户的TA匹配地址
             注:写 CSI_USER[0~5]_ADDR 寄存器
                mac_addr 为!NULL 表示增加该白名单用户;
                         为 NULL 表示删除该白名单用户;
*****************************************************************************/
STATIC osal_void hh503_csi_set_whitelist(osal_u8 user_index, const osal_u8 *mac_addr)
{
    osal_u32 usr_addr_lsb; /* MAC地址低32位 */
    osal_u32 idx;
    if (mac_addr != OSAL_NULL) {
        usr_addr_lsb = mac_addr[2];                       /* 数组元素2 */
        usr_addr_lsb = (usr_addr_lsb << 8) | mac_addr[3]; /* 左移8位，数组元素3 */
        usr_addr_lsb = (usr_addr_lsb << 8) | mac_addr[4]; /* 左移8位，数组元素4 */
        usr_addr_lsb = (usr_addr_lsb << 8) | mac_addr[5]; /* 左移8位，数组元素5 */
    } else {
        usr_addr_lsb = 0;
    }

    if (user_index == HAL_CSI_USR_0) {
        hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x154, usr_addr_lsb);
    } else if (user_index == HAL_CSI_USR_1) {
        hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x158, usr_addr_lsb);
    } else if (user_index == HAL_CSI_USR_2) {
        hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x15C, usr_addr_lsb);
    } else if (user_index == HAL_CSI_USR_3) {
        hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x1D0, usr_addr_lsb);
    }
    for (idx = WLAN_MAC_ADDR_LEN - 1; idx >= HAL_CSI_USR_ADDR_NUM; --idx) {
        wifi_printf("\r\n  mac_addr[%d]: 0x%02x\n", idx, mac_addr[idx]);
    }
}

/*****************************************************************************
 功能描述  : 获取CSI白名单用户的TA匹配地址
*****************************************************************************/
STATIC osal_u32 hh503_csi_get_whitelist(osal_u8 user_index, osal_u8 *mac_addr)
{
    osal_u32 usr_addr_lsb = 0; /* MAC地址低32位 */

    if (mac_addr == OSAL_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (user_index == HAL_CSI_USR_0) {
        usr_addr_lsb = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x154);
    } else if (user_index == HAL_CSI_USR_1) {
        usr_addr_lsb = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x158);
    } else if (user_index == HAL_CSI_USR_2) {
        usr_addr_lsb = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x15C);
    } else if (user_index == HAL_CSI_USR_3) {
        usr_addr_lsb = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x1D0);
    }

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 设置CSI用户的PPDU格式
*****************************************************************************/
STATIC osal_void hh503_csi_set_usr_ppdu_format(osal_u8 user_index, osal_u8 ppdu_type, oal_bool_enum_uint8 flag)
{
    osal_u32 bitmap = 0x1 << ppdu_type;
    u_csi_ppdu_format reg_val = {{0}};

    reg_val.u32 = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x160);
    if (user_index == HAL_CSI_USR_0) {
        reg_val.bits.xCt2OCslOSsw3OGG4SOtvwIzR_ = toggle_bitmap(flag, reg_val.bits.xCt2OCslOSsw3OGG4SOtvwIzR_, bitmap);
    } else if (user_index == HAL_CSI_USR_1) {
        reg_val.bits.x6Ngw6CBwdC_0wuuFdwNi_SGL_ = toggle_bitmap(flag, reg_val.bits.x6Ngw6CBwdC_0wuuFdwNi_SGL_, bitmap);
    } else if (user_index == HAL_CSI_USR_2) {
        reg_val.bits.xCt2OCslOSswZOGG4SOtvwIzR_ = toggle_bitmap(flag, reg_val.bits.xCt2OCslOSswZOGG4SOtvwIzR_, bitmap);
    } else if (user_index == HAL_CSI_USR_3) {
        reg_val.bits.xCt2OCslOSswiOGG4SOtvwIzR_ = toggle_bitmap(flag, reg_val.bits.xCt2OCslOSswiOGG4SOtvwIzR_, bitmap);
    } else {
    }
    hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x160, reg_val.u32);
}

/*****************************************************************************
 功能描述  : 设置CSI用户的PPDU格式
*****************************************************************************/
STATIC osal_void hh503_csi_get_usr_ppdu_format(osal_u8 user_index, osal_u8 ppdu_type, oal_bool_enum_uint8 *flag)
{
    osal_u32 bitmap;
    u_csi_ppdu_format reg_val = {{0}};

    bitmap = 0x1 << ppdu_type;
    reg_val.u32 = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x160);
    if (user_index == HAL_CSI_USR_0) {
        *flag = (reg_val.bits.xCt2OCslOSsw3OGG4SOtvwIzR_ & bitmap) ? OSAL_TRUE : OSAL_FALSE;
    } else if (user_index == HAL_CSI_USR_1) {
        *flag = (reg_val.bits.x6Ngw6CBwdC_0wuuFdwNi_SGL_ & bitmap) ? OSAL_TRUE : OSAL_FALSE;
    } else if (user_index == HAL_CSI_USR_2) {
        *flag = (reg_val.bits.xCt2OCslOSswZOGG4SOtvwIzR_ & bitmap) ? OSAL_TRUE : OSAL_FALSE;
    } else if (user_index == HAL_CSI_USR_3) {
        *flag = (reg_val.bits.xCt2OCslOSswiOGG4SOtvwIzR_ & bitmap) ? OSAL_TRUE : OSAL_FALSE;
    } else {
    }
}

/*****************************************************************************
 功能描述  : 设置CSI白名单各用户匹配的MAC帧子类型及是否使能
             注:写 CSI_USER[0~3]_ADDR 寄存器, 入参user_index由外部hal调用者保证, hh503为内部接口
*****************************************************************************/
STATIC osal_void hh503_csi_set_usr_frm_subtype(osal_u8 user_index, osal_u16 subtype, oal_bool_enum_uint8 flag)
{
    u_csi_user_frm_subtype reg_val = {{0}};
    osal_u32 switch_bitmap;

    switch_bitmap = 0x1 << user_index;
    reg_val.u32 = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x150);
    if (flag == OSAL_TRUE) {
        reg_val.bits.x6Ngw6CBwN_SwCdjLtuWwSGL6QwWh_ |= switch_bitmap;
    } else {
        reg_val.bits.x6Ngw6CBwN_SwCdjLtuWwSGL6QwWh_ &= (~switch_bitmap);
    }

    if (user_index < HAL_CSI_USR_2) {
        if (user_index == HAL_CSI_USR_0) {
            reg_val.bits.x6Ngw6CBwdC_ywN_SwCdjLtuW_ = subtype;
        }
        if (user_index == HAL_CSI_USR_1) {
            reg_val.bits.xCt2OCslOSswrOtwIOsSyRQGm_ = subtype;
        }
    } else {
        if (user_index == HAL_CSI_USR_2) {
            reg_val.bits.x6Ngw6CBwdC_VwN_SwCdjLtuW_ = subtype;
        }
        if (user_index == HAL_CSI_USR_3) {
            reg_val.bits.x6Ngw6CBwdC_kwN_SwCdjLtuW_ = subtype;
        }
    }
    hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x150, reg_val.u32);
}

/*****************************************************************************
 功能描述  : 获取CSI白名单各用户匹配的MAC帧子类型及是否使能
*****************************************************************************/
STATIC osal_void hh503_csi_get_usr_frm_subtype(osal_u8 usr_id, osal_u8 *subtype, oal_bool_enum_uint8 *flag)
{
    u_csi_user_frm_subtype reg_val = {{0}};
    osal_u32 bitmap;

    bitmap = 0x1 << usr_id;
    reg_val.u32 = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x150);
    *flag = (reg_val.bits.x6Ngw6CBwN_SwCdjLtuWwSGL6QwWh_ & bitmap) ? OSAL_TRUE : OSAL_FALSE;

    if (usr_id <= HAL_CSI_USR_1) {
        *subtype = (usr_id == HAL_CSI_USR_0) ? reg_val.bits.x6Ngw6CBwdC_ywN_SwCdjLtuW_
                                             : reg_val.bits.xCt2OCslOSswrOtwIOsSyRQGm_;
    } else if (usr_id <= HAL_CSI_USR_3) {
        *subtype = (usr_id == HAL_CSI_USR_2) ? reg_val.bits.x6Ngw6CBwdC_VwN_SwCdjLtuW_
                                             : reg_val.bits.x6Ngw6CBwdC_kwN_SwCdjLtuW_;
    }
}

/*****************************************************************************
 功能描述  : 设置CSI用户的上报周期
            period (0~4095)ms
*****************************************************************************/
STATIC osal_void hh503_csi_set_sample_period(osal_u8 user_index, osal_u16 period)
{
    if (user_index < HAL_CSI_USR_2) {
        u_csi_usr01_sample_period sample_period = {{0}};
        sample_period.u32 = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x164);
        if (user_index == HAL_CSI_USR_0) {
            sample_period.bits.xCt2OCslOSsw3OszIGxmOGmwlv4_ = period;
        }
        if (user_index == HAL_CSI_USR_1) {
            sample_period.bits.x6Ngw6CBwdC_0wCGSuoWwuW_BiF_ = period;
        }
        hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x164, sample_period.u32);
    } else {
        u_csi_usr23_sample_period sample_period = {{0}};
        sample_period.u32 = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x168);
        if (user_index == HAL_CSI_USR_2) {
            sample_period.bits.xCt2OCslOSswZOszIGxmOGmwlv4_ = period;
        }
        if (user_index == HAL_CSI_USR_3) {
            sample_period.bits.xCt2OCslOSswiOszIGxmOGmwlv4_ = period;
        }
        hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x168, sample_period.u32);
    }
}

/*****************************************************************************
 功能描述  : 获取CSI用户的上报周期
*****************************************************************************/
STATIC osal_void hh503_csi_get_sample_period(osal_u8 user_index, osal_u16 *period)
{
    if (user_index < HAL_CSI_USR_2) {
        u_csi_usr01_sample_period sample_period = {{0}};
        sample_period.u32 = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x164);
        if (user_index == HAL_CSI_USR_0) {
            *period = sample_period.bits.xCt2OCslOSsw3OszIGxmOGmwlv4_;
        }
        if (user_index == HAL_CSI_USR_1) {
            *period = sample_period.bits.x6Ngw6CBwdC_0wCGSuoWwuW_BiF_;
        }
    } else {
        u_csi_usr23_sample_period sample_period = {{0}};
        sample_period.u32 = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x168);
        if (user_index == HAL_CSI_USR_2) {
            *period = sample_period.bits.xCt2OCslOSswZOszIGxmOGmwlv4_;
        }
        if (user_index == HAL_CSI_USR_3) {
            *period = sample_period.bits.xCt2OCslOSswiOszIGxmOGmwlv4_;
        }
    }
    wifi_printf("\r\n hh503_csi_get_sample_period user_index: 0x%x, *period: 0x%x\n", user_index, *period);
}

/*****************************************************************************
 功能描述  : 设置CSI计数清零
*****************************************************************************/
STATIC osal_void hh503_csi_clr_rpt_cnt(hal_csi_rpt_cnt_enum err_type)
{
    u_csi_buff_set reg_val0 = {{0}};
    u_csi_buff_set reg_val1 = {{0}};
    osal_u32 switch_bitmap;

    reg_val0.u32 = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x16C);
    reg_val1.u32 = reg_val0.u32;

    // note that hal_csi_rpt_cnt_enum is as same as bits.cfg_csi_rpt_cnt_clr[0~4]
    switch_bitmap = 0x1 << err_type;
    reg_val0.bits.xCt2OCslOwGROCoROCxw_ |= switch_bitmap;
    reg_val1.bits.xCt2OCslOwGROCoROCxw_ &= (~switch_bitmap);

    // first write bits.cfg_csi_rpt_cnt_clr[err_type] 1 then write 0, to clear the count
    hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x16C, reg_val0.u32);
    hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x16C, reg_val1.u32);
}


/*****************************************************************************
 功能描述  : 获取逻辑CSI上报计数
*****************************************************************************/
osal_u32 hh503_csi_get_blk_num_cnt(osal_void)
{
    u_rpt_csi_cnt reg_val = {{0}};
    reg_val.u32 = hal_reg_read(HH503_MAC_RD1_BANK_BASE_0x190);
    return reg_val.bits.x_uLw6CBwjoKwhdSw6hL_;
}

/*****************************************************************************
 功能描述  : 配置逻辑CSI PHY通道
*****************************************************************************/
osal_void hh503_csi_phy_open_channel(osal_void)
{
    osal_u32 reg_val;
    reg_val = hal_reg_read(HH503_PHY_BANK5_BASE_0x2EC);
    reg_val |= 0x02;
    hal_reg_write(HH503_PHY_BANK5_BASE_0x2EC, reg_val);
}

STATIC osal_void hh503_csi_set_tsf(osal_u8 hal_vap_id)
{
    u_diag_control1 reg_val = {{0}};
    reg_val.u32 = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x18C);
    reg_val.bits.xCt2O4lz2ORstOsmx_ = hal_vap_id;
    hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x18C, reg_val.u32);
}

/*****************************************************************************
 功能描述  : 关闭逻辑CSI PHY通道
*****************************************************************************/
STATIC osal_void hal_csi_phy_close_channel(osal_void)
{
    u_chn_csi reg_val;
    reg_val.u32 = hal_reg_read(HH503_PHY_BANK5_BASE_0x2EC);
    reg_val.bits.xCt2OCslOxxRtOmo_ = 0;
    reg_val.bits.cfg_csi_en = 0;
    hal_reg_write(HH503_PHY_BANK5_BASE_0x2EC, reg_val.u32);
}


/*****************************************************************************
  Hal层接口，封装hh503接口
*****************************************************************************/

/*****************************************************************************
 功能描述  : 配置发送消息给device执行接口
*****************************************************************************/
STATIC osal_u32 hal_csi_send_msg_to_device(osal_u8 cmd)
{
    osal_s32 ret;
    frw_msg msg_info = {0};
    hal_csi_cfg_info info = {0};
    osal_u32 msg_cfg_result = 0;

    info.cmd_id = cmd;
    info.csi_data_blk_num = g_csi_blk_num;
    info.csi_data_max_len = g_csi_max_len;

    frw_msg_init((osal_u8 *)&info, sizeof(info), (osal_u8 *)&msg_cfg_result, sizeof(osal_u32), &msg_info);

    ret = frw_send_msg_to_device(0, WLAN_MSG_H2D_C_CFG_CSI_HAL, &msg_info, OSAL_TRUE);
    if (ret != OAL_SUCC) {
        return ret;
    }

    return msg_cfg_result;
}

/*****************************************************************************
 功能描述  : csi init接口
            注: call directly on device, send msg on host
*****************************************************************************/
STATIC osal_u32 hal_csi_init(osal_void)
{
    oal_bool_enum_uint8 flag;
    int usr_id;

    for (usr_id = 0; usr_id < HAL_CSI_MAX_USER_NUM; ++usr_id) {
        hh503_csi_get_enable(usr_id, &flag);
        if (flag) {
            break;
        }
    }
    wifi_printf("\r\n hal_csi_init usr_id: %d\n", usr_id);
    if (usr_id == HAL_CSI_MAX_USER_NUM) {
        wifi_printf("\r\n hal_csi_enable call msg id: %d\n", CSI_MSG_H2D_CMD_INIT);
        hal_csi_send_msg_to_device(CSI_MSG_H2D_CMD_INIT); // this cmd is to init csi memory on device
        return OAL_SUCC;
    }
    return OAL_CONTINUE;
}

/*****************************************************************************
 功能描述  : csi exit接口
*****************************************************************************/
STATIC osal_u32 hal_csi_exit(osal_void)
{
    oal_bool_enum_uint8 flag;
    int usr_id;

    for (usr_id = 0; usr_id < HAL_CSI_MAX_USER_NUM; ++usr_id) {
        hh503_csi_get_enable(usr_id, &flag);
        if (flag) {
            break;
        }
    }
    oam_info_log1(0, OAM_SF_ANY, "hal_csi_exit usr_id: %d", usr_id);
    if (usr_id == HAL_CSI_MAX_USER_NUM) {
        oam_info_log1(0, OAM_SF_ANY, "hal_csi_enable call msg id: %d", CSI_MSG_H2D_CMD_EXIT);
        hal_csi_send_msg_to_device(CSI_MSG_H2D_CMD_EXIT); // this cmd is to exit csi memory on device

        /* 关闭逻辑CSI PHY通道 */
        hal_csi_phy_close_channel();
    }

    return OAL_SUCC;
}

// csi使能禁用接口
/*****************************************************************************
 函 数 名  : hal_csi_enable
 功能描述  : csi使能接口，usr_id对应白名单用户，错误码通OSAL
*****************************************************************************/
osal_u32 hal_csi_enable(osal_u8 usr_id)
{
    if (usr_id >= HAL_CSI_MAX_USER_NUM) {
        return OAL_ERR_CODE_CONFIG_EXCEED_SPEC;
    }

    wifi_printf("\r\n hal_csi_enable usr_id: %d\n", usr_id);
    hal_csi_init(); /* do csi init before call csi enable interface */
    hal_csi_stat_clr(); /* 先清零CSI统计计数, 再使能CSI启动上报数据 */

    hh503_csi_set_enable(usr_id, OAL_TRUE);

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hal_csi_disable
 功能描述  : csi禁用接口，usr_id对应白名单用户，错误码通OSAL
*****************************************************************************/
osal_u32 hal_csi_disable(osal_u8 usr_id)
{
    if (usr_id >= HAL_CSI_MAX_USER_NUM) {
        return OAL_ERR_CODE_CONFIG_EXCEED_SPEC;
    }

    oam_info_log1(0, OAM_SF_ANY, "hal_csi_disable usr_id: %d\n", usr_id);
    hh503_csi_set_enable(usr_id, OAL_FALSE);
    hal_csi_exit(); /* do csi exit after all csi usr been disabled */
    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 设置csi用户配置接口，usr_id对应白名单用户
            注: 用户上报使能配置字段cfg_csi_en在设置时忽略, 由专门使能禁用接口控制
*****************************************************************************/
osal_u32 hal_csi_set_usr_attr(osal_u8 usr_id, const hal_csi_usr_attr *attr)
{
    if (!attr) {
        return OAL_ERR_CODE_PTR_NULL;
    }
    if (usr_id >= HAL_CSI_MAX_USER_NUM) {
        return OAL_ERR_CODE_CONFIG_EXCEED_SPEC;
    }

    hh503_csi_set_whitelist(usr_id, attr->usr_addr);

    hh503_csi_set_addr_match_bypass(usr_id, attr->cfg_addr_match_bypass);
    hh503_csi_set_addr_filter_type(usr_id, attr->cfg_match_ta_ra_sel);
    hh503_csi_set_mac_frm_type_filter(usr_id, WLAN_MANAGEMENT, attr->cfg_mgmt_frame_en);
    hh503_csi_set_mac_frm_type_filter(usr_id, WLAN_CONTROL, attr->cfg_ctrl_frame_en);
    hh503_csi_set_mac_frm_type_filter(usr_id, WLAN_DATA_BASICTYPE, attr->cfg_data_frame_en);

    hh503_csi_set_usr_frm_subtype(usr_id, attr->match_frame_subtype, attr->frm_subtype_match_en);

    hh503_csi_set_usr_ppdu_format(usr_id, HAL_CSI_PPDU_NONE_HT, attr->ppdu_non_ht_en);
    hh503_csi_set_usr_ppdu_format(usr_id, HAL_CSI_PPDU_HE_ER_SU, attr->ppdu_he_er_su_en);
    hh503_csi_set_usr_ppdu_format(usr_id, HAL_CSI_PPDU_HE_MU_MIMO, attr->ppdu_he_mu_mimo_en);
    hh503_csi_set_usr_ppdu_format(usr_id, HAL_CSI_PPDU_HE_MU_OFDMA, attr->ppdu_he_mu_ofdma_en);
    hh503_csi_set_usr_ppdu_format(usr_id, HAL_CSI_PPDU_HT, attr->ppdu_ht_en);
    hh503_csi_set_usr_ppdu_format(usr_id, HAL_CSI_PPDU_VHT, attr->ppdu_vht_en);

    hh503_csi_set_sample_period(usr_id, attr->sample_period_ms);

    if (attr->cfg_csi_en) {
        hal_csi_enable(usr_id);
    } else {
        hal_csi_disable(usr_id);
    }
    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 设置csi buffer地址块数及buffer大小
*****************************************************************************/
osal_u32 hal_csi_set_buffer_config(hal_csi_buffer_stru *csi_buffer)
{
    if (!csi_buffer) {
        return OAL_ERR_CODE_PTR_NULL;
    }
    g_csi_max_len = csi_buffer->csi_data_max_len;
    g_csi_blk_num = csi_buffer->csi_data_blk_num;
    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 获取csi用户配置接口，usr_id对应白名单用户
*****************************************************************************/
osal_u32 hal_csi_get_usr_attr(osal_u8 usr_id, hal_csi_usr_attr *attr)
{
    oal_bool_enum_uint8 flag;
    if (!attr) {
        return OAL_ERR_CODE_PTR_NULL;
    }
    if (usr_id >= HAL_CSI_MAX_USER_NUM) {
        return OAL_ERR_CODE_CONFIG_EXCEED_SPEC;
    }

    hh503_csi_get_whitelist(usr_id, attr->usr_addr);
    hh503_csi_get_enable(usr_id, &flag);
    attr->cfg_csi_en = flag;
    hh503_csi_get_addr_filter_type(usr_id, &flag);
    attr->cfg_match_ta_ra_sel = flag;
    hh503_csi_get_addr_match_bypass(usr_id, &flag);
    attr->cfg_addr_match_bypass = flag;
    hh503_csi_get_mac_frm_type_filter(usr_id, WLAN_MANAGEMENT, &flag);
    attr->cfg_mgmt_frame_en = flag;
    hh503_csi_get_mac_frm_type_filter(usr_id, WLAN_CONTROL, &flag);
    attr->cfg_ctrl_frame_en = flag;
    hh503_csi_get_mac_frm_type_filter(usr_id, WLAN_DATA_BASICTYPE, &flag);
    attr->cfg_data_frame_en = flag;

    hh503_csi_get_usr_frm_subtype(usr_id, &attr->match_frame_subtype, &flag);
    attr->frm_subtype_match_en = flag;

    hh503_csi_get_usr_ppdu_format(usr_id, HAL_CSI_PPDU_NONE_HT, &flag);
    attr->ppdu_non_ht_en = flag;
    hh503_csi_get_usr_ppdu_format(usr_id, HAL_CSI_PPDU_HE_ER_SU, &flag);
    attr->ppdu_he_er_su_en = flag;
    hh503_csi_get_usr_ppdu_format(usr_id, HAL_CSI_PPDU_HE_MU_MIMO, &flag);
    attr->ppdu_he_mu_mimo_en = flag;
    hh503_csi_get_usr_ppdu_format(usr_id, HAL_CSI_PPDU_HE_MU_OFDMA, &flag);
    attr->ppdu_he_mu_ofdma_en = flag;
    hh503_csi_get_usr_ppdu_format(usr_id, HAL_CSI_PPDU_HT, &flag);
    attr->ppdu_ht_en = flag;
    hh503_csi_get_usr_ppdu_format(usr_id, HAL_CSI_PPDU_VHT, &flag);
    attr->ppdu_vht_en = flag;

    hh503_csi_get_sample_period(usr_id, &attr->sample_period_ms);
    return OAL_SUCC;
}

// csi其他配置接口
osal_u32 hal_csi_stat_clr(osal_void)
{
    hh503_csi_clr_rpt_cnt(HAL_CSI_CNT_H_NUM_ERR);
    hh503_csi_clr_rpt_cnt(HAL_CSI_CNT_H_TIMMING_ERR);
    hh503_csi_clr_rpt_cnt(HAL_CSI_CNT_TIMEOUT_ERR);
    hh503_csi_clr_rpt_cnt(HAL_CSI_CNT_SW_BLK_ERR);
    hh503_csi_clr_rpt_cnt(HAL_CSI_CNT_RPT_SUCC);
    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 设置csi上报的tsf选择，STA为0，其他为1
*****************************************************************************/
osal_u32 hal_csi_set_tsf(osal_u8 hal_vap_id)
{
    hh503_csi_set_tsf(hal_vap_id); /* AP/p2p固定使用ID1 */
    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  :   关闭csi ack帧默认上报通道
*****************************************************************************/
osal_void hal_csi_set_ack_resp_flt(void)
{
    u_csi_buff_set reg_val = {{0}};
    reg_val.u32 = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x16C);
    /* csi ack默认上报通道关闭，将寄存器值写为1 */
    reg_val.bits.xCt2OCslO4lwmCROwmsGOCRwxOtxROmo_ = 1;
    hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x16C, reg_val.u32);
}

/*****************************************************************************
 功能描述  : 判断CSI PHY通道是否打开
*****************************************************************************/
osal_bool hal_csi_is_open(void)
{
    u_chn_csi reg_val;
    osal_bool is_open;
    reg_val.u32 = hal_reg_read(HH503_PHY_BANK5_BASE_0x2EC);
    is_open = (reg_val.bits.cfg_csi_en == 1) ? OSAL_TRUE : OSAL_FALSE;
    return is_open;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
