#include <linux/clk.h>
#include <linux/component.h>
#include <linux/crc-ccitt.h>
#include <linux/module.h>
#include <linux/of_address.h>
#include <linux/phy/phy-mipi-dphy.h>
#include <linux/phy/phy.h>
#include <linux/platform_device.h>
#include <linux/regmap.h>
#include <linux/regulator/consumer.h>
#include <linux/reset.h>
#include <linux/slab.h>

#include <drm/drm_atomic_helper.h>
#include <drm/drm_mipi_dsi.h>
#include <drm/drm_panel.h>
#include <drm/drm_print.h>
#include <drm/drm_probe_helper.h>
#include <drm/drm_simple_kms_helper.h>
#include "canaan_dsi.h"
#include "linux/export.h"
#include <video/mipi_display.h>


#define TXDPHY_PLL_CFG1         0x10
#define TXDPHY_PLL_CFG0         0x08
#define PHY_TST_CTRL0           0xb4
#define PHY_TST_CTRL1           0xb8
#define TXDPHY_CFG1             0x4
#define PHY_RSTZ                0xa0
#define TXDPHY_CFG0             0x0
#define PHY_STATUS              (0xb0)
#define LPCLK_CTRL              (0x94)

static void k230_dsi_write_phy_reg(struct canaan_dsi *dsi, uint8_t addr, uint8_t val)
{
    uint32_t reg = 0;

    reg = 0x10000 + addr;
    writel(reg, PHY_TST_CTRL1 + dsi->base);
    writel(0x2, PHY_TST_CTRL0 + dsi->base);
    writel(0x0, PHY_TST_CTRL0 + dsi->base);

    reg = val;
    writel(reg, PHY_TST_CTRL1 + dsi->base);
    writel(0x2, PHY_TST_CTRL0 + dsi->base);
    writel(0x0, PHY_TST_CTRL0 + dsi->base);
}

static void k230_dsi_phy_pll_config(struct canaan_dsi *dsi, uint32_t m, uint32_t n, uint8_t vco)
{
    uint32_t reg = 0;

    reg = readl(dsi->base + 0x400 + TXDPHY_PLL_CFG1);
    reg = (reg & ~(BIT_MASK(9))) | (1 << 9);             // shadow_clear field
    writel(reg, dsi->base + 0x400 + TXDPHY_PLL_CFG1);
    reg = (reg & ~(BIT_MASK(9))) | (0 << 9);            // shadow_clear field
    writel(reg, dsi->base + 0x400 + TXDPHY_PLL_CFG1);

    reg = readl(dsi->base + 0x400 + TXDPHY_PLL_CFG1);
    reg = (reg & ~(GENMASK(1, 0))) | (0x1 << 0);            // clksel
    writel(reg, dsi->base + 0x400 + TXDPHY_PLL_CFG1);

    reg = readl(dsi->base + 0x400 + TXDPHY_PLL_CFG0);         // TODO case by case
    reg = (reg & ~(GENMASK(26, 17))) | (m << 17);           // M=m+2   M=125
    reg = (reg & ~(GENMASK(30, 27))) | (n << 27);           // N=n+1   N=6
    writel(reg, dsi->base + 0x400 + TXDPHY_PLL_CFG0);         // expect fout = 125 = (1M/4N)*24;  m=125,n=6

    reg = readl(dsi->base + 0x400 + TXDPHY_PLL_CFG1);
    reg = (reg & ~(GENMASK(16, 11))) | (vco << 11);         // vco_cntrl
    writel(reg, dsi->base + 0x400 + TXDPHY_PLL_CFG1);

    reg = readl(dsi->base + 0x400 + TXDPHY_PLL_CFG0);
    reg = (reg & ~(GENMASK(6, 0))) | (0x10 << 0);             // cpbias_cntrl
    writel(reg, dsi->base + 0x400 + TXDPHY_PLL_CFG0);

    reg = readl(dsi->base + 0x400 + TXDPHY_PLL_CFG0);
    reg = (reg & ~(GENMASK(9, 8))) | (0x00 << 8);             // gmp_cntrl
    writel(reg, dsi->base + 0x400 + TXDPHY_PLL_CFG0);

    reg = readl(dsi->base + 0x400 + TXDPHY_PLL_CFG0);
    reg = (reg & ~(GENMASK(16, 11))) | (0x00 << 11);             // int_cntrl
    writel(reg, dsi->base + 0x400 + TXDPHY_PLL_CFG0);

    reg = readl(dsi->base + 0x400 + TXDPHY_PLL_CFG1);
    reg = (reg & ~(GENMASK(8, 3))) | (0x8 << 3);         // prop_cntrl
    writel(reg, dsi->base + 0x400 + TXDPHY_PLL_CFG1);

    k230_dsi_write_phy_reg(dsi, 0x14, 0x2);    // pll1_th1
    k230_dsi_write_phy_reg(dsi, 0x15, 0x60);    // pll1_th2
    k230_dsi_write_phy_reg(dsi, 0x16, 0x3);    // pll1_th3
    k230_dsi_write_phy_reg(dsi, 0x1D, 0x1);    // pll_lock_sel

    reg = readl(dsi->base + 0x400 + TXDPHY_PLL_CFG1);
    reg = (reg & ~(BIT_MASK(10))) | (1 << 10);
    writel(reg, dsi->base + 0x400 + TXDPHY_PLL_CFG1);

    msleep(1);

    reg = (reg & ~(BIT_MASK(10))) | (0 << 10);
    writel(reg, dsi->base + 0x400 + TXDPHY_PLL_CFG1);

}

static void k230_dsi_phy0_config(struct canaan_dsi *dsi, uint32_t m, uint32_t n, uint8_t vco, uint8_t hsfreq)
{
    uint32_t reg = 0;

    // select phy0
    writel(0x0, dsi->base + 0x400 + TXDPHY_CFG1);
    // set rstz
    writel(0xc, dsi->base + PHY_RSTZ);
    // Set test clr to 1
    writel(0x1, dsi->base + PHY_TST_CTRL0);
    // Set test clr to 0
    writel(0x0, dsi->base + PHY_TST_CTRL0);
    // set mastermacro=1, prototyping_env =1 ??
    k230_dsi_write_phy_reg(dsi, 0x0c, 0x03);
    //set hsfreqrange
    // k230_dsi_write_phy_reg(0x44, 0x96);             // set 445.5  hsfreqrange = 0010110  ??为什么补了个高位为1 啊 （b10010110）
    k230_dsi_write_phy_reg(dsi, 0x44, hsfreq);             // set 445.5  hsfreqrange = 0010110  ??为什么补了个高位为1 啊 （b10010110）
    // for < 1.5Gbps data rate set Slew
    k230_dsi_write_phy_reg(dsi, 0xa0, 0x40);           // set Slew rate vs DDL sr_range      *TODO case by case
    k230_dsi_write_phy_reg(dsi, 0xa4, 0x11);           // set Slew sr_osc_freq_target[6:0]   *TODO case by case
    k230_dsi_write_phy_reg(dsi, 0xa4, 0x85);           // set Slew sr_osc_freq_target[11:7]  *TODO case by case
    k230_dsi_write_phy_reg(dsi, 0xa3, 0x1);           // enable Slew rate calibration  (sr_sel_tester)

    k230_dsi_write_phy_reg(dsi, 0x1f, 0x1);               // mpll_prog[0] = 1'b1
    k230_dsi_write_phy_reg(dsi, 0x4a, 0x40);               // prg_on_lane0 = 1'b1

    reg = readl(dsi->base + 0x400 + TXDPHY_CFG0);
    reg = (reg & ~(GENMASK(7, 2))) | (0x28 << 2);            //phy0 cfgclkfreqrange     6'b101000;
    reg = (reg & ~(GENMASK(13, 8))) | (0x28 << 8);          //phy1 cfgclkfreqrange      6'b101000;
    writel(reg, dsi->base + 0x400 + TXDPHY_CFG0);

    // config pll
    // k230_dsi_phy_pll_config(295, 15, 0x17);                  // 222.75 M   phy clk = pll x 2
    k230_dsi_phy_pll_config(dsi, m, n, vco);                  // 222.75 M   phy clk = pll x 2

    writel(0x28a0, dsi->base + 0x400 + TXDPHY_CFG0);
    writel(0xc, dsi->base + PHY_RSTZ);
}


void k230_dsi_phy1_config(struct canaan_dsi *dsi, uint8_t hsfreq)
{
    uint32_t reg = 0;

    // select1
    writel(0x400000, dsi->base + 0x400 + TXDPHY_CFG1);

    // printf("0x400000 is %x \n", readl(dsi->base + 0x400 + TXDPHY_CFG1));

    writel(0x1, dsi->base + PHY_TST_CTRL0);
    msleep(1);
    writel(0x0, dsi->base + PHY_TST_CTRL0);

    // SET TEST CLR TO LOW
    k230_dsi_write_phy_reg(dsi, 0xc, 0x0);

    // select1
    writel(0x400000, dsi->base + 0x400 + TXDPHY_CFG1);


    //set hsfreqrange 445.5Mbps
    // k230_dsi_write_phy_reg(0x44, 0x96);         // set 445.5  hsfreqrange = 0010110  ??为什么补了个高位为1 啊 （b10010110）
    k230_dsi_write_phy_reg(dsi, 0x44, hsfreq);         // set 445.5  hsfreqrange = 0010110  ??为什么补了个高位为1 啊 （b10010110）

    // try slave phy hs clk lane
    k230_dsi_write_phy_reg(dsi, 0x30, 0xff);

    // for < 1.5Gbps data rate set Slew
    k230_dsi_write_phy_reg(dsi, 0xa0, 0x40);           // set Slew rate vs DDL sr_range      *TODO case by cas

    k230_dsi_write_phy_reg(dsi, 0xa4, 0x11);           // set Slew sr_osc_freq_target[6:0]   *TODO case by case

    k230_dsi_write_phy_reg(dsi, 0xa4, 0x85);           // set Slew sr_osc_freq_target[11:7]  *TODO case by case
    k230_dsi_write_phy_reg(dsi, 0xa3, 0x1);           // enable Slew rate calibration  (sr_sel_tester)
    k230_dsi_write_phy_reg(dsi, 0x1f, 0x1);               // mpll_prog[0] = 1'b1

    k230_dsi_write_phy_reg(dsi, 0x4a, 0x40);               // prg_on_lane0 = 1'b1


    reg = readl(dsi->base + 0x400 + TXDPHY_CFG0);
    reg = (reg & ~(GENMASK(7, 2))) | (0x28 << 2);            //phy0 cfgclkfreqrange     6'b101000;
    reg = (reg & ~(GENMASK(13, 8))) | (0x28 << 8);          //phy1 cfgclkfreqrange      6'b101000;
    writel(reg, dsi->base + 0x400 + TXDPHY_CFG0);

    reg = readl(dsi->base + 0x400 + TXDPHY_CFG0);
    reg = (reg & ~(BIT_MASK(1))) | (0 << 1);
    writel(reg, dsi->base + 0x400 + TXDPHY_CFG0);            //phy1 basedir

    writel(0x4, dsi->base + PHY_RSTZ);               //phy enableclk
    writel(0xd, dsi->base + PHY_RSTZ);               // redundant
    writel(0xf, dsi->base + PHY_RSTZ);

    k230_dsi_write_phy_reg(dsi, 0x3, 0x80);              // monitor phy fsm

    while (readl(dsi->base + PHY_TST_CTRL1) != 0x580)  //0x580
    {
        k230_dsi_write_phy_reg(dsi, 0x03, 0x80);
        msleep(1);
    }

    // printf("phy1 config done \n");
}

void k230_dsi_config_4lan_phy(struct canaan_dsi *dsi, uint32_t m, uint32_t n, uint8_t vco, uint8_t hsfreq)
{
    uint32_t reg = 0;

    k230_dsi_phy0_config(dsi, m, n, vco, hsfreq);
    k230_dsi_phy1_config(dsi, hsfreq);


    reg = readl(dsi->base + 0x400 + TXDPHY_CFG1); // rdata = 0x0
    writel(0x0, dsi->base + 0x400 + TXDPHY_CFG1);
    writel(0xd, dsi->base + PHY_RSTZ);
    writel(0xf, dsi->base + PHY_RSTZ);

    while (readl(dsi->base + PHY_STATUS) != 0x1fbd);

    msleep(1);
    /* To postpone HS request,  for MIPI spec: First STOP_STATE time should be greater than T_INIT time at least 100us */
    writel(0x1, dsi->base + LPCLK_CTRL);

    //-------- display_dut_cfg_seq_dsi::wait_for_PHY1_PWRUP() -------//
    // printf("=== wait_for_PHY1_PWRUP() begin=== \n");
    reg = readl(dsi->base + 0x400 + TXDPHY_CFG1); // rdata = 0x0
    writel(0x400000, dsi->base + 0x400 + TXDPHY_CFG1);

    while (readl(dsi->base + PHY_STATUS) != 0x1fbd);

    reg = readl(dsi->base + PHY_STATUS); // rdata = 0x1fbd

}