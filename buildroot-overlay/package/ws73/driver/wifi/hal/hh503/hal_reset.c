/*
 * Copyright (c) CompanyNameMagicTag 2020-2020. All rights reserved.
 * Description: chip reset.
 * Create: 2020-7-3
 */

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "hal_reset.h"
#include "hal_mac.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HAL_RESET_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

#define DMA_REG_BANK_NUM            7       // PHY BANK + PHY_CTRL
#define MAC_PHY_DMA_WIDTH           4       /* mac phy寄存器dmac搬运位宽均为4bytes */
#define MAC_REG_SAVE_NUM            6       /* BANK0 BANK1 BANK2 BANK3 BANK4 WLMAC */
#define PHY_REG_SAVE_NUM            4       /* BANK0 BANK1 BANK2 BANK3 */
/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
oal_netbuf_stru g_pm_mac_ptr;      /* 低功耗保存的mac寄存器mem地址管理体 */
oal_netbuf_stru g_pm_phy_ptr;      /* 低功耗保存的phy寄存器mem地址管理体 */
/* DMA映射表 */


/*****************************************************************************
  3 函数实现
*****************************************************************************/
/*****************************************************************************
 功能描述  : 释放用于保存寄存器的netbuf
*****************************************************************************/
static inline osal_void hh503_reset_free_reg_netbuf(oal_netbuf_stru **netbuf)
{
    oal_netbuf_stru *cur_netbuf = *netbuf;
    oal_netbuf_stru *next_buf = OSAL_NULL;
    while (cur_netbuf != OSAL_NULL) {
        next_buf = cur_netbuf->next;
        oal_netbuf_free(cur_netbuf);
        cur_netbuf = next_buf;
    }
    *netbuf = OSAL_NULL;
}

/*****************************************************************************
 功能描述  : 下电前MAC内容保存
*****************************************************************************/
osal_void hh503_reset_save_mac_reg(osal_void)
{
    oal_netbuf_stru *cur_netbuf = &g_pm_mac_ptr;
    osal_u8 idx;
    osal_u8 *mac_addr[MAC_REG_SAVE_NUM] = {
        (osal_u8 *)HH503_MAC_CTRL0_BANK_BASE, (osal_u8 *)HH503_MAC_CTRL1_BANK_BASE,
        (osal_u8 *)HH503_MAC_CTRL2_BANK_BASE, (osal_u8 *)HH503_MAC_WLMAC_CTRL_BASE
    };
    osal_u32 reg_len[MAC_REG_SAVE_NUM] = {
        PS_MAC_CTRL0_REG_LENGTH, PS_MAC_CTRL1_REG_LENGTH, PS_MAC_CTRL2_REG_LENGTH,
        /* wlmac有1个寄存器地址共4bytes需要单独保存 */
        (PS_MAC_WLMAC_CTRL_REG_LENGTH + 4)
    };
    hal_reg32_cfg_stru tx_fifo[] = {
        {HH503_MAC_VLD_BANK_BASE_0x30, 0}, {HH503_MAC_VLD_BANK_BASE_0x34, 0},
        {HH503_MAC_VLD_BANK_BASE_0x38, 0}, {HH503_MAC_VLD_BANK_BASE_0x3C, 0},
        {HH503_MAC_VLD_BANK_BASE_0x40, 0}, {HH503_MAC_VLD_BANK_BASE_0x44, 0},
    };

    if (g_pm_mac_ptr.next != OSAL_NULL) {  /* 逻辑异常::存在已保存寄存器值 */
        oam_error_log0(0, OAM_SF_ANY, "hh503_reset_mac_reg_save::exist saved reg value.");
        return;
    }
    /* 清除发送FIFO地址 */
    hal_reg_write_tbl(tx_fifo, sizeof(tx_fifo) / sizeof(hal_reg32_cfg_stru));
    /* 保存mac寄存器表 */
    for (idx = MAC_REG_SAVE_NUM; idx > 0; idx--) {
        oal_netbuf_stru *temp = oal_netbuf_alloc_ext(OAL_NORMAL_NETBUF, reg_len[idx - 1], OAL_NETBUF_PRIORITY_HIGH);
        if (temp == OSAL_NULL) {
            hh503_reset_free_reg_netbuf(&g_pm_mac_ptr.next);
            oam_error_log0(0, OAM_SF_ANY, "hh503_reset_mac_reg_save::netbuff alloc error.");
            return;
        }
        cur_netbuf->next = temp;
         /* 保存对应的寄存器 */
        (osal_void)memcpy_s(oal_netbuf_data(temp), reg_len[idx - 1], mac_addr[idx - 1], reg_len[idx - 1]);
        cur_netbuf = temp;

        if (idx == MAC_REG_SAVE_NUM) {
            /* 保存wlmac后面1个寄存器 */
            osal_u8 *payload = oal_netbuf_data(cur_netbuf);
            *(osal_u32 *)(&payload[PS_MAC_WLMAC_CTRL_REG_LENGTH]) = hal_reg_read(HH503_MAC_WLMAC_CTRL_BASE_0x110);
        }
    }
}

/*****************************************************************************
 功能描述  : phy寄存器保存并复位phy
*****************************************************************************/
osal_void hh503_reset_phy(osal_u8 reset_reg)
{
    osal_u16 reset_reg_mask = HH503_PHY_SOFT_RST_WP0_PHY_N_MASK;

    if (reset_reg) {
        /* 复位PHY寄存器 */
    }
    /* 复位PHY逻辑 */
    hal_reg_write(HH503_PHY0_CTL_BASE_0x20, reset_reg_mask); /* BIT10 don't reset */

     /* 需要延10us */
    osal_udelay(10);

    /* PHY0软复位解 */
    hal_reg_write(HH503_PHY0_CTL_BASE_0x20, 0x25FF); /* initial value */
}

/*****************************************************************************
 功能描述  : 复位MAC寄存器
*****************************************************************************/
osal_void hh503_reset_mac_soft_reset(osal_void)
{
    osal_u16 reset_reg_mask = (HH503_MAC_SOFT_RST_WL0_MAC_SLAVE_N_MASK | HH503_MAC_SOFT_RST_WL0_MAC_AON_N_MASK);
    /* MAC软复位 */
    hal_reg_write16(HH503_MAC_WLMAC_CTRL_BASE_0x20, reset_reg_mask);

    /* 需要延10us */
    osal_udelay(10);

    /* MAC软复位解8bit全部 */
    hal_reg_write16(HH503_MAC_WLMAC_CTRL_BASE_0x20, 0x3FF); /* initial value */
    return;
}

/*****************************************************************************
 功能描述  : 复位MAC所有模块
*****************************************************************************/
osal_void hh503_reset_mac_logic_all(osal_void)
{
    osal_u8 len;
    osal_u8 index;

    u_rpt_vap0_rx_bcn_param rpt_vap0_rx_bcn_param;
    u_vap0_dtim_count_update vap0_dtim_count_update = {0};
    u_vap0_listen_interval_timer_update vap0_listen_interval_timer_update = {0};

    /* mac重启时，会把定时器也重启，导致所有vap的beacon周期重新初始化成0从而引起beaconmiss
           需要在mac重启前，读取所有vap的tbtt status 及tsf status，并在重启后再写入 */
    /* VAP模式可配,这里需要5个vap都操作 */
    hal_reg32_cfg_stru mac_vap_reg_tbl[] = {
        {HH503_MAC_RD0_BANK_BASE_0x60, 0x0},
        {HH503_MAC_RD0_BANK_BASE_0x6C, 0x0},
        {HH503_MAC_RD1_BANK_BASE_0x1B0, 0x0},
        {HH503_MAC_RD0_BANK_BASE_0x5C, 0x0},
        {HH503_MAC_RD0_BANK_BASE_0x68, 0x0},
        {HH503_MAC_RD1_BANK_BASE_0x1AC, 0x0},
        {HH503_MAC_RD0_BANK_BASE_0x58, 0x0},
        {HH503_MAC_RD0_BANK_BASE_0x64, 0x0},
        {HH503_MAC_RD1_BANK_BASE_0x1A8, 0x0},
    };
    osal_u32 mac_vap_reg_add_tbl[] = {
        HH503_MAC_VLD_BANK_BASE_0xC,
        HH503_MAC_VLD_BANK_BASE_0x24,
        HH503_MAC_VLD_BANK_BASE_0x94,
        HH503_MAC_VLD_BANK_BASE_0x4,
        HH503_MAC_VLD_BANK_BASE_0x1C,
        HH503_MAC_VLD_BANK_BASE_0x8C,
        HH503_MAC_VLD_BANK_BASE_0x0,
        HH503_MAC_VLD_BANK_BASE_0x18,
        HH503_MAC_VLD_BANK_BASE_0x88,
    };

    osal_u32 mac_ptr_vap_rx_bcn_param[] = {
        HH503_MAC_RD0_BANK_BASE_0x54,
        HH503_MAC_RD0_BANK_BASE_0x70,
        HH503_MAC_RD1_BANK_BASE_0x1B4,
    };
    osal_u32 mac_vap_dtim_count_update[] = {
        HH503_MAC_VLD_BANK_BASE_0x10,
        HH503_MAC_VLD_BANK_BASE_0x28,
        HH503_MAC_VLD_BANK_BASE_0x98,
    };
    osal_u32 mac_vap_listen_interval_timer_update[] = {
        HH503_MAC_VLD_BANK_BASE_0x14,
        HH503_MAC_VLD_BANK_BASE_0x2C,
        HH503_MAC_VLD_BANK_BASE_0x9C,
    };
    len = sizeof(mac_vap_reg_tbl) / sizeof(hal_reg32_cfg_stru);

    hal_reg_read_tbl(mac_vap_reg_tbl, len);

    for (index = 0; index < len; index++) {
        mac_vap_reg_tbl[index].address = mac_vap_reg_add_tbl[index];
    }

    hh503_reset_mac_soft_reset();
    hal_reg_write_tbl(mac_vap_reg_tbl, len);
    for (index = 0; index < HAL_MAX_VAP_NUM; index++) {
        rpt_vap0_rx_bcn_param.u32 = hal_reg_read(mac_ptr_vap_rx_bcn_param[index]);
        vap0_dtim_count_update.bits.x6NgwqGuywFLBSwTwqGo_ = rpt_vap0_rx_bcn_param.bits.xqGuywFLBSw6d__w6hL_;
        hal_reg_write(mac_vap_dtim_count_update[index], vap0_dtim_count_update.u32);
        vap0_listen_interval_timer_update.bits.xCt2OPzG3OxlsRmoOqOPzx_ = rpt_vap0_rx_bcn_param.bits.xPzG3OxlsRmoOCoR_;
        hal_reg_write(mac_vap_listen_interval_timer_update[index], vap0_listen_interval_timer_update.u32);
    }
}

/*****************************************************************************
 功能描述  : mac复位操作
*****************************************************************************/
osal_void hh503_reset_mac(osal_u8 sub_mod, osal_u8 reset_reg)
{
    if (reset_reg) {
        /* 复位寄存器，同时也复位逻辑，操作soc的整体复位寄存器 */
        hh503_reset_save_mac_reg();
        /* MAC软复位触发 */
        hal_reg_write(HH503_MAC_WLMAC_CTRL_BASE_0x20, 0);
        /* MAC软复位解除 */
        hal_reg_write(HH503_MAC_WLMAC_CTRL_BASE_0x20, 0x3FF);
    } else {
        /* 单独复位mac的所有或者部分logic，操作mac复位寄存器 */
        switch (sub_mod) {
            case HAL_RESET_MAC_ALL:
                /* 目前MAC不支持单模块复位，只能全部复位 */
                hh503_reset_mac_logic_all();
                break;

            case HAL_RESET_MAC_LOGIC:
                hh503_reset_mac_soft_reset();
                break;
            default:
                break;
        }
    }
}

/*****************************************************************************
 功能描述  : 内存拷贝方式保存/恢复寄存器
*****************************************************************************/
static inline osal_void hh503_reset_handle_reg_tbl(osal_void)
{
}

/*****************************************************************************
 功能描述  : mac/phy复位对外接口函数
*****************************************************************************/
osal_void hh503_reset_phy_machw(hal_reset_hw_type_enum_uint8 type, osal_u8 sub_mod, osal_u8 reset_phy_reg,
    osal_u8 reset_mac_reg)
{
    switch (type) {
        case HAL_RESET_HW_TYPE_PHY:
            hh503_reset_phy(reset_phy_reg);
            break;
        case HAL_RESET_HW_TYPE_MAC:
            hh503_reset_mac(sub_mod, reset_mac_reg);
            break;
        case HAL_RESET_HW_TYPE_MAC_PHY:
            hh503_reset_phy(reset_phy_reg);
            hh503_reset_mac(sub_mod, reset_mac_reg);
            break;
        case HAL_RESET_HW_TYPE_ALL:
            hh503_reset_mac(sub_mod, reset_mac_reg);
            hh503_reset_phy(reset_phy_reg);
            break;
        default:
            break;
    }

    return;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
