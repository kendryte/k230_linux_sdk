/*
 * Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
 * Description: hal_vap.c.
 * Create: 2023-01-11
 */

#include "hal_vap.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HAL_VAP_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

#ifdef _PRE_WLAN_FEATURE_P2P
#define P2P_PERIOD_NOA_COUNT 255

osal_u16 g_noa_count = P2P_PERIOD_NOA_COUNT;
#endif

/*****************************************************************************
 函 数 名  : hh503_set_11ax_vap_idx
*****************************************************************************/
osal_void hh503_set_11ax_vap_idx(osal_u8 hal_vap_idx)
{
    u_tx_param_val_pilot2 tx;
    tx.u32 = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x118);
    tx.bits.xCt2Orrz8OsRzOPzGOlo4m83_ = hal_vap_idx;

    hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x118, tx.u32);
}

/*****************************************************************************
  VAP管理相关处理函数
*****************************************************************************/
osal_void hh503_del_vap(hal_to_dmac_device_stru *hal_device, osal_u8 vap_id)
{
    hal_device_stru *device = (hal_device_stru *)hal_device;
    hal_to_dmac_vap_stru *hal_to_dmac_vap = OSAL_NULL;
    osal_u8 mac_addr[WLAN_MAC_ADDR_LEN] = {0};
    osal_u32 int_save;

    /* 清除删除设备的MAC地址 */
    hh503_get_hal_vap(hal_device, vap_id, &hal_to_dmac_vap);
    if (hal_to_dmac_vap != OSAL_NULL) {
        hh503_vap_set_macaddr(hal_to_dmac_vap, mac_addr, WLAN_MAC_ADDR_LEN);
    } else {
        oam_warning_log1(0, OAM_SF_CFG, "{hh503_del_vap::hal_to_dmac_vap[%d] is NULL!}", vap_id);
        return;
    }

    /* 中断上半部打断hal vap删除过程，hal vap指针还没有置空，但是成员值不可信 */
    /* 关中断 */
    int_save = frw_osal_irq_lock();

#ifdef _PRE_PM_TBTT_OFFSET_PROBE
    hh503_tbtt_offset_probe_destroy(hal_to_dmac_vap);
#endif

    if (device->vap_list[vap_id] != OSAL_NULL) {
        oal_mem_free(device->vap_list[vap_id], OAL_TRUE);
        device->vap_list[vap_id] = OSAL_NULL;
    }
    device->vap_num--;

    /* 删除为sta的业务vap时，设置device下的sta_hal_vap_idx 为无效索引 */
    if (hal_device->sta_hal_vap_idx == vap_id) {
        hal_device->sta_hal_vap_idx = 0xff;
    }

    /* 删除为ap的业务vap时，设置device下的ap_hal_vap_idx 为无效索引 */
    if (hal_device->ap_hal_vap_idx == vap_id) {
        hal_device->ap_hal_vap_idx = 0xff;
    }

#ifdef _PRE_WLAN_FEATURE_P2P
    /* 删除为P2P的业务vap时，设置device下的p2p_hal_vap_idx 为无效索引 */
    if (hal_device->p2p_hal_vap_idx == vap_id) {
        hal_device->p2p_hal_vap_idx = 0xff;
    }
#endif /* _PRE_WLAN_FEATURE_P2P */

    /* 开中断 */
    frw_osal_irq_restore(int_save);
}

/*****************************************************************************
 功能描述 : 分配vap 内存
*****************************************************************************/
osal_u32 hh503_alloc_vap(hal_vap_stru **hal_vap)
{
    hal_vap_stru *vap;

    vap = (hal_vap_stru *)oal_mem_alloc(OAL_MEM_POOL_ID_LOCAL, sizeof(hal_vap_stru), OAL_TRUE);
    if (vap == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{hh503_alloc_vap::create hal vap failed.}\r\n");
        return OAL_ERR_CODE_ALLOC_MEM_FAIL;
    }

    (osal_void)memset_s(vap, sizeof(hal_vap_stru), 0, sizeof(hal_vap_stru));
    *hal_vap = vap;
    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 初始化VAP，包括VAP公共变量赋值，以及函数挂接
*****************************************************************************/
osal_void hh503_vap_init(hal_vap_stru *hal_vap, osal_u8 vap_id, wlan_vap_mode_enum_uint8 vap_mode)
{
    /* 为vap赋值 */
    hal_vap->vap_base.vap_id = vap_id;
    hal_vap->vap_base.vap_mode = vap_mode;
    hal_vap->vap_base.hal_vap_state = HAL_VAP_STATE_INIT;
}

/*****************************************************************************
 功能描述  : 根据输入的模式，分配相应的VAP ID
 固定VAP ID应用：WALN0 - ID:0   P2P_DEV - ID:1 GC/GO/AP - ID:2
*****************************************************************************/
osal_void hh503_add_vap(hal_to_dmac_device_stru *hal_device_base, wlan_vap_mode_enum_uint8 vap_mode,
    osal_u8 hal_vap_id, hal_to_dmac_vap_stru **hal_vap)
{
    hal_vap_stru *vap = OSAL_NULL;
    hal_device_stru *hal_device = (hal_device_stru *)hal_device_base;
#ifdef _PRE_WLAN_FEATURE_P2P
    wlan_p2p_mode_enum_uint8 p2p_mode = vap_mode >> 4; /* 右移4位 */
    vap_mode &= 0x0f;
#endif /* _PRE_WLAN_FEATURE_P2P */

    if ((hal_device == OSAL_NULL) || (hal_vap == OSAL_NULL)) {
        return;
    }

    if (hal_device->vap_num >= HAL_MAX_VAP_NUM) {
        oam_error_log1(0, OAM_SF_CFG,
            "{hh503_add_vap::vap num[%d] execeed spec[3], add failed.}", hal_device->vap_num);
        return;
    }

    if (hh503_alloc_vap(&vap) != OAL_SUCC) {
        return;
    }

    /* 保存hal层的sta vap id */
    if (vap_mode == WLAN_VAP_MODE_BSS_STA
#ifdef _PRE_WLAN_FEATURE_P2P
    && (p2p_mode == WLAN_LEGACY_VAP_MODE)
#endif
    ) {
        hal_device_base->sta_hal_vap_idx = hal_vap_id;
    }

    /* 保存hal层的ap vap id */
    if (vap_mode == WLAN_VAP_MODE_BSS_AP
#ifdef _PRE_WLAN_FEATURE_P2P
    && (p2p_mode == WLAN_LEGACY_VAP_MODE)
#endif
    ) {
        hal_device_base->ap_hal_vap_idx = hal_vap_id;
    }

#ifdef _PRE_WLAN_FEATURE_P2P
    /* 保存hal层的p2p vap id */
    if ((p2p_mode == WLAN_P2P_CL_MODE) || (p2p_mode == WLAN_P2P_GO_MODE)) {
        hal_device_base->p2p_hal_vap_idx = hal_vap_id;
    }

    vap->vap_base.p2p_mode = p2p_mode;
#endif
    /* vap初始化 */
    hh503_vap_init(vap, hal_vap_id, vap_mode);

    /* 将全局结构中的vap指针，指向要添加的vap */
    hal_device->vap_list[hal_vap_id] = vap;
    hal_device->vap_num++;
    *hal_vap = &vap->vap_base;
}

/*****************************************************************************
 函 数 名  : hh503_get_hal_vap
*****************************************************************************/
osal_void hh503_get_hal_vap(const hal_to_dmac_device_stru *hal_device_base, osal_u8 vap_id,
    hal_to_dmac_vap_stru **hal_vap)
{
    hal_device_stru *hal_device;

    hal_device = (hal_device_stru *)hal_device_base;

    hh503_get_vap_internal(hal_device, vap_id, hal_vap);
}

/*****************************************************************************
 功能描述  : 读取TBTT计数器
*****************************************************************************/
osal_void hh503_vap_read_tbtt_timer(const hal_to_dmac_vap_stru *hal_vap, osal_u32 *value)
{
    osal_u8 vap_id = hal_vap->vap_id;
    osal_u32 addr[] = {
        HH503_MAC_RD0_BANK_BASE_0x60, HH503_MAC_RD0_BANK_BASE_0x6C,  HH503_MAC_RD1_BANK_BASE_0x1B0
    };

    if (vap_id >= HAL_MAX_VAP_NUM) {
        oam_warning_log3(0, OAM_SF_ANY, "hh503_vap_read_tbtt_timer::vap mode [%d]hal vap id=%d >= max vap=%d!",
            hal_vap->vap_mode, vap_id, HAL_MAX_VAP_NUM);
        return;
    }
    if (value != OSAL_NULL) {
        *value = hal_reg_read(addr[vap_id]);
    }
}


/*****************************************************************************
 功能描述 : 设置vap的beacon周期,单位TU(1024us),不能配0
*****************************************************************************/
osal_void hh503_vap_set_machw_beacon_period(const hal_to_dmac_vap_stru *hal_vap, osal_u16 beacon_period)
{
    osal_u8 vap_id = hal_vap->vap_id;
    osal_u32 reg[] = {
        HH503_MAC_VLD_BANK_BASE_0x8, HH503_MAC_VLD_BANK_BASE_0x20, HH503_MAC_VLD_BANK_BASE_0x90
    };

    if ((vap_id >= HAL_MAX_VAP_NUM) || (hal_vap->vap_mode != WLAN_VAP_MODE_BSS_AP)) {
        oam_warning_log3(0, OAM_SF_ANY,
            "hh503_vap_set_machw_beacon_period::vap mode [%d] != WLAN_VAP_MODE_BSS_AP,or hal vap id=%d >= max vap=%d!",
            hal_vap->vap_mode, vap_id, HAL_MAX_VAP_NUM);
        return;
    }

    if (beacon_period != 0) {
        hal_reg_write(reg[vap_id], (osal_u32)beacon_period);
    } else {
        oam_warning_log0(0, OAM_SF_ANY, "hh503_vap_set_machw_beacon_period::wrong beacon period 0");
    }
}


/*****************************************************************************
 功能描述  : 挂起beacon帧发送
*****************************************************************************/
osal_void hh503_vap_beacon_suspend(const hal_to_dmac_vap_stru *hal_vap)
{
    osal_u32 value;
    osal_u8 vap_id = hal_vap->vap_id;
    osal_u32 addr[] = {
        HH503_MAC_CTRL2_BANK_BASE_0xC, HH503_MAC_CTRL2_BANK_BASE_0x34, HH503_MAC_CTRL2_BANK_BASE_0x12C
    };

    /* 只有ap模式才能挂起beacon发送 */
    if ((hal_vap->vap_mode != WLAN_VAP_MODE_BSS_AP) || (vap_id >= HAL_MAX_VAP_NUM)) {
        return;
    }

    /* 挂起beacon发送 */
    value = hal_reg_read(addr[vap_id]);
    hal_reg_write(addr[vap_id], (value | HH503_MAC_CFG_VAP0_TX_BCN_SUSPEND_MASK));
}

/*****************************************************************************
 功能描述  : 恢复beacon发送
*****************************************************************************/
osal_void hh503_vap_beacon_resume(const hal_to_dmac_vap_stru *hal_vap)
{
    osal_u32 value;
    osal_u8 vap_id = hal_vap->vap_id;
    osal_u32 addr[] = {
        HH503_MAC_CTRL2_BANK_BASE_0xC, HH503_MAC_CTRL2_BANK_BASE_0x34, HH503_MAC_CTRL2_BANK_BASE_0x12C
    };

    osal_u32 offset[] = {
        HH503_MAC_RPT_INTR_VAP0_TBTT_OFFSET, HH503_MAC_RPT_INTR_VAP1_TBTT_OFFSET, HH503_MAC_RPT_INTR_VAP2_TBTT_OFFSET
    }; // 中断状态寄存器

    /* 只有ap模式才能恢复beacon发送 */
    if ((hal_vap->vap_mode != WLAN_VAP_MODE_BSS_AP) || (vap_id >= HAL_MAX_VAP_NUM)) {
        return;
    }

    /* 恢复beacon发送 */
    value = hal_reg_read(addr[vap_id]);
    hal_reg_write(addr[vap_id], (value & (~HH503_MAC_CFG_VAP0_TX_BCN_SUSPEND_MASK)));

    /* unmask tbtt中断 */
    hal_reg_write_bits_by_msg(HH503_MAC_CTRL0_BANK_BASE_0x54, offset[vap_id], 1, 0, OSAL_TRUE);
}

/*****************************************************************************
 功能描述 : 设置vap的mac地址
*****************************************************************************/
osal_void hh503_vap_set_macaddr(const hal_to_dmac_vap_stru *hal_vap, const osal_u8 *mac_addr,
    osal_u16 mac_addr_len)
{
    osal_u8 vap_id = hal_vap->vap_id;
    osal_u32 mac_addr_msb; /* MAC地址高16位 */
    osal_u32 mac_addr_lsb; /* MAC地址低32位 */
    osal_u32 value;
    /* HAL vap index 0 1 2 3 4 */
    osal_u32 addr[][2] = {
        {HH503_MAC_CTRL2_BANK_BASE_0x4, HH503_MAC_CTRL2_BANK_BASE_0x0},
        {HH503_MAC_CTRL2_BANK_BASE_0x2C, HH503_MAC_CTRL2_BANK_BASE_0x28},
        {HH503_MAC_CTRL2_BANK_BASE_0x124, HH503_MAC_CTRL2_BANK_BASE_0x120}
    };
    unref_param(mac_addr_len);

    if (vap_id >= HAL_MAX_VAP_NUM) {
        oam_error_log2(0, OAM_SF_CFG, "{hh503_vap_set_macaddr::hal vap id=%d >= max vap=%d]}", vap_id,
            HAL_MAX_VAP_NUM);
        return;
    }

    mac_addr_msb = mac_addr[0];
    mac_addr_msb = (mac_addr_msb << 8) | mac_addr[1]; /* mac addr:8-15bit */

    mac_addr_lsb = mac_addr[2]; /* 数组元素2 */
    mac_addr_lsb = (mac_addr_lsb << 8) | mac_addr[3]; /* 左移8位，数组元素3 */
    mac_addr_lsb = (mac_addr_lsb << 8) | mac_addr[4]; /* 左移8位，数组元素4 */
    mac_addr_lsb = (mac_addr_lsb << 8) | mac_addr[5]; /* 左移8位，数组元素5 */

    value = hal_reg_read(addr[vap_id][0]);
    value &= ~(HH503_MAC_CFG_VAP0_MACADDR_H_MASK);
    hal_reg_write(addr[vap_id][0], value | (mac_addr_msb << HH503_MAC_CFG_VAP0_MACADDR_H_OFFSET));
    hal_reg_write(addr[vap_id][1], mac_addr_lsb);

    oam_warning_log4(0, OAM_SF_CFG, "{hh503_vap_set_macaddr_patch:: MAC ADDR:: %02X:%02X:%02X:%02X:XX:XX.}",
        (osal_u8)(hal_reg_read(addr[vap_id][0]) >> 24), /* 右移24位 */
        (osal_u8)((hal_reg_read(addr[vap_id][0]) & 0xFF0000) >> 16), /* 右移16位 */
        (osal_u8)(hal_reg_read(addr[vap_id][1]) >> 24), /* 右移24位 */
        (osal_u8)((hal_reg_read(addr[vap_id][1]) & 0xFF0000) >> 16)); /* 右移16位 */

    return;
}

/*****************************************************************************
 函 数 名  : hh503_vap_clr_opmode
*****************************************************************************/
osal_void hh503_vap_clr_opmode(const hal_to_dmac_vap_stru *hal_vap, wlan_vap_mode_enum_uint8 vap_mode)
{
    osal_u8  vap_id = hal_vap->vap_id;
    u_pa_control2 pa_ctrl2;
    u_vap2_mode_set0 vap2_mode_set0;
    unref_param(vap_mode);

    if (vap_id >= HAL_MAX_VAP_NUM) {
        oam_error_log2(0, OAM_SF_CFG, "{hh503_vap_clr_opmode::hal vap id=%d >= max vap=%d]}", vap_id, HAL_MAX_VAP_NUM);
        return;
    }

    /* 解决VAP DOWN操作时，清除PA CONTROL寄存器对应bit错误 */
    if (vap_id == HAL_VAP_VAP0) {
        pa_ctrl2.u32 = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x4);
        pa_ctrl2.bits.x6NgwqGuywSiFW_ = 0;
        hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x4, pa_ctrl2.u32);
    } else if (vap_id == HAL_VAP_VAP1) {
        pa_ctrl2.u32 = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x4);
        pa_ctrl2.bits.xCt2OPzGrOIv4m_ = 0;
        hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x4, pa_ctrl2.u32);
    } else {
        vap2_mode_set0.u32 =  hal_reg_read(HH503_MAC_CTRL2_BANK_BASE_0x118);
        vap2_mode_set0.bits.x6NgwqGuVwSiFW_ = 0;
        hal_reg_write(HH503_MAC_CTRL2_BANK_BASE_0x118, vap2_mode_set0.u32);
    }
}

/*****************************************************************************
 函 数 名  : hh503_vap_set_opmode
*****************************************************************************/
osal_void hh503_vap_set_opmode(hal_to_dmac_vap_stru *hal_vap, wlan_vap_mode_enum_uint8 vap_mode)
{
    osal_u8 vap_id = hal_vap->vap_id;
    osal_u8 vap_mode_value;
    u_pa_control2 pa_ctrl2;
    u_vap2_mode_set0 vap2_mode_set0;

    if (vap_id >= HAL_MAX_VAP_NUM) {
        oam_error_log2(0, OAM_SF_CFG, "{hh503_vap_set_opmode::hal sta id=%d >= max vap=%d]}", vap_id, HAL_MAX_VAP_NUM);
        return;
    }

    if (vap_mode == WLAN_VAP_MODE_BSS_STA) {
        vap_mode_value = BIT0;
    } else if (vap_mode == WLAN_VAP_MODE_BSS_AP) {
        vap_mode_value = BIT1;
    } else {
        oam_error_log2(0, OAM_SF_CFG, "{hh503_vap_set_opmode::hal sta id=%d,wrong vap mode[%d]}", vap_id, vap_mode);
        return;
    }

    /* 两bit,设置模式前先clear */
    hal_vap_clr_opmode(hal_vap, vap_mode);

    if (vap_id == HAL_VAP_VAP0) {
        pa_ctrl2.u32 = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x4);
        pa_ctrl2.bits.x6NgwqGuywSiFW_ = vap_mode_value;
        hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x4, pa_ctrl2.u32);
    } else if (vap_id == HAL_VAP_VAP1) {
        pa_ctrl2.u32 = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x4);
        pa_ctrl2.bits.xCt2OPzGrOIv4m_ = vap_mode_value;
        hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x4, pa_ctrl2.u32);
    } else {
        vap2_mode_set0.u32 =  hal_reg_read(HH503_MAC_CTRL2_BANK_BASE_0x118);
        vap2_mode_set0.bits.x6NgwqGuVwSiFW_ = vap_mode_value;
        hal_reg_write(HH503_MAC_CTRL2_BANK_BASE_0x118, vap2_mode_set0.u32);
    }
}

/*****************************************************************************
 功能描述 : 设置vap发送beacon帧的速率
*****************************************************************************/
osal_void hh503_vap_set_beacon_rate(const hal_to_dmac_vap_stru *hal_vap, osal_u32 beacon_rate)
{
    osal_u8 vap_id = hal_vap->vap_id;
    osal_u32 rate_reg[] = {
        HH503_MAC_CTRL2_BANK_BASE_0x20, HH503_MAC_CTRL2_BANK_BASE_0x48, HH503_MAC_CTRL2_BANK_BASE_0x140
    };

    if ((vap_id >= HAL_MAX_VAP_NUM) || (hal_vap->vap_mode != WLAN_VAP_MODE_BSS_AP)) {
        oam_warning_log3(0, OAM_SF_ANY,
            "hh503_vap_set_beacon_rate::vap mode [%d]!= WLAN_VAP_MODE_BSS_AP,or hal vap id=%d >= max vap=%d]}",
            hal_vap->vap_mode, vap_id, HAL_MAX_VAP_NUM);
        return;
    }

    hal_reg_write(rate_reg[vap_id], beacon_rate);
}

#ifdef _PRE_WLAN_FEATURE_P2P
/*****************************************************************************
 功能描述  : 设置OPS寄存器
*****************************************************************************/
osal_void hal_vap_set_ops(const hal_to_dmac_vap_stru *hal_vap, osal_u8 ops_ctrl, osal_u8 ct_window)
{
    u_p2p_ctrl3 reg;
    u_p2p_ctrl1 p2p_ctrl1;
    if (ops_ctrl == 0) {
        p2p_ctrl1.u32 = hal_reg_read(HH503_MAC_CTRL1_BANK_BASE_0x18);
        /* 取消P2P ops 节能 */
        p2p_ctrl1.bits.xCt2OGZGOzGGOGsOmo_ = 0;
        hal_reg_write(HH503_MAC_CTRL1_BANK_BASE_0x18, p2p_ctrl1.u32);
    } else {
        u_pa_control1 pa_ctrl;
        pa_ctrl.u32 = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x0);
        pa_ctrl.bits.xCt2OGZGOGsOmo_ = 1;
        hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x0, pa_ctrl.u32);

        p2p_ctrl1.u32 = hal_reg_read(HH503_MAC_CTRL1_BANK_BASE_0x18);
        /* 设置P2P机会节能使能 */
        p2p_ctrl1.bits.xCt2OGZGOzGGOGsOmo_ = 1;
        /* mode软件配对应的vap ID */
        p2p_ctrl1.bits.xCt2OGZGOPzGOsmx_ = hal_vap->vap_id;
        hal_reg_write(HH503_MAC_CTRL1_BANK_BASE_0x18, p2p_ctrl1.u32);

        /* 设置OPS 节能CT_WINDOW 参数 */
        reg.u32 = hal_reg_read(HH503_MAC_CTRL1_BANK_BASE_0x20);
        reg.bits.xCt2OGZGOCROqlo4vqOPzx_ = ct_window;
        hal_reg_write(HH503_MAC_CTRL1_BANK_BASE_0x20, reg.u32);
    }
}

/*****************************************************************************
 功能描述  : 设置NoA寄存器
*****************************************************************************/
osal_void hal_vap_set_noa(const hal_to_dmac_vap_stru *hal_vap, osal_u32 start_tsf,
    osal_u32 duration, osal_u32 interval, osal_u8 count)
{
    u_p2p_ctrl2 reg_ctr2;
    u_pa_control1 pa_ctrl;
    u_p2p_ctrl1 p2p_ctrl;
    if (count == 0) {
        /* 取消noa */
        reg_ctr2.u32 = hal_reg_read(HH503_MAC_CTRL1_BANK_BASE_0x1C);
        reg_ctr2.bits.xCt2OGZGOovzOvGmw_ = 2; /* 2:10 */
        hal_reg_write(HH503_MAC_CTRL1_BANK_BASE_0x1C, reg_ctr2.u32); /* 左移2位 */
    } else {
        g_noa_count = count;

        /* 设置P2P 节能使能 */
        /* 设置P2P 控制寄存器VAP mode bit */
        p2p_ctrl.u32 = hal_reg_read(HH503_MAC_CTRL1_BANK_BASE_0x18);
        /* mode软件配对应的vap ID */
        // reg = (reg & ~HH503_MAC_CFG_P2P_VAP_SEL_MASK) | (hal_vap->vap_id << HH503_MAC_CFG_P2P_VAP_SEL_OFFSET);
        p2p_ctrl.bits.xCt2OGZGOPzGOsmx_ = hal_vap->vap_id;
        /* 设置P2P  cfg_p2p_absent_suspend_en absent 不使能suspend */
        hal_reg_write(HH503_MAC_CTRL1_BANK_BASE_0x18, p2p_ctrl.u32);

        pa_ctrl.u32 = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x0);
        pa_ctrl.bits.xCt2OGZGOGsOmo_ = 1;
        hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x0, pa_ctrl.u32);

        /* 设置noa 参数 */
        hal_reg_write(HH503_MAC_CTRL1_BANK_BASE_0x24, start_tsf);
        hal_reg_write(HH503_MAC_CTRL1_BANK_BASE_0x2C, duration);
        hal_reg_write(HH503_MAC_CTRL1_BANK_BASE_0x30, interval);

        /* 设置P2P NOA 使能和noa count 参数 ，清空twt相关寄存器 */
        reg_ctr2.u32 = hal_reg_read(HH503_MAC_CTRL1_BANK_BASE_0x1C);
        reg_ctr2.bits.xCt2OGZGOovzOvGmw_ = 2; /* 2:10 */
        hal_reg_write(HH503_MAC_CTRL1_BANK_BASE_0x1C, reg_ctr2.u32); /* 左移2位 */

        reg_ctr2.bits.xCt2OGZGOovzOCvSoR_ = count;
        reg_ctr2.bits.xCt2OGZGOovzOvGmw_ = 3;  /* 3表示开启自校准功能 */
        reg_ctr2.bits.x6NgwLTLwWh_ = 0;
        reg_ctr2.bits.xCt2ORqROqzdmOR8Omo_ = 0;
        hal_reg_write(HH503_MAC_CTRL1_BANK_BASE_0x1C, reg_ctr2.u32);
    }

    oam_info_log4(0, OAM_SF_ANY, "hal_vap_set_noa, start_tsf:%u, dura:%d, intval:%d, cnt:%d",
        start_tsf, duration, interval, count);
}

#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
