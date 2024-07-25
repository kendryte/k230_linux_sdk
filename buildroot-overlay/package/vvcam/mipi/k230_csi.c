/* Copyright (c) 2024, Canaan Bright Sight Co., Ltd
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "k230_csi.h"
#include <asm/mmio.h>
#include <asm/io.h>
#include <linux/delay.h>

static u8 *g_csi_addr = NULL;

static void k230_csi0_rxphy0_reg_write(u16 phy_addr, u8 code_data)
{
    u8 code_addr_msb, code_addr_lsb, phy_testdin;
    u8 phy_testclr, phy_testclk,phy_testen;
    u32 reg = 0;

    code_addr_msb = (phy_addr >> 8) & 0xff;
    code_addr_lsb = phy_addr & 0xff;

    //1. writing the 4-bit test code MSBs
    //a. set testclk & testen to low

    phy_testclr = 0;
    phy_testclk = 0;
    phy_testen = 0;
    phy_testdin = 0;
    writel(0x0, g_csi_addr + 0x800 + DWC_PHY_TEST_CTRL0);
    writel(0x0, g_csi_addr + 0x800 + DWC_PHY_TEST_CTRL1);

    // #10ns;
    // udelay(1);
    udelay(1);
    //b.set tesen to high, set clk to high, any delay needed?
    phy_testen = 1;
    reg = 0;
    reg = (reg & ~(GENMASK(7, 0))) | (phy_testdin << 0);
    reg = (reg & ~(BIT_MASK(16))) | (phy_testen << 16);
    writel(reg, g_csi_addr + 0x800 + DWC_PHY_TEST_CTRL1);

    // #10ns;
    // udelay(1);
    udelay(1);
    phy_testclk = 1;
    reg = 0;
    reg = (reg & ~(BIT_MASK(0))) | (phy_testclr << 0);
    reg = (reg & ~(BIT_MASK(1))) | (phy_testclk << 1);
    writel(reg, g_csi_addr + 0x800 + DWC_PHY_TEST_CTRL0);

    //c. place 0x00 in testdin
    phy_testdin = 0;
    reg = 0;
    reg = (reg & ~(GENMASK(7, 0))) | (phy_testdin << 0);
    reg = (reg & ~(BIT_MASK(16))) | (phy_testen << 16);
    writel(reg, g_csi_addr + 0x800 + DWC_PHY_TEST_CTRL1);
    // reg_write(32'h9000_a854, {15'd0, phy_testen, 8'd0, phy_testdin});

    //d. set testclk to low, set testen to low
    phy_testclk = 0;
    reg = 0;
    reg = (reg & ~(BIT_MASK(0))) | (phy_testclr << 0);
    reg = (reg & ~(BIT_MASK(1))) | (phy_testclk << 1);
    writel(reg, g_csi_addr + 0x800 + DWC_PHY_TEST_CTRL0);
    // reg_write(32'h9000_a850, {30'd0, phy_testclk, phy_testclr});
    phy_testen = 0;
    reg = 0;
    reg = (reg & ~(GENMASK(7, 0))) | (phy_testdin << 0);
    reg = (reg & ~(BIT_MASK(16))) | (phy_testen << 16);
    writel(reg, g_csi_addr + 0x800 + DWC_PHY_TEST_CTRL1);
    // reg_write(32'h9000_a854, {15'd0, phy_testen, 8'd0, phy_testdin});

    //e. place the 8-bit word corresponding to the testcode MSBs in testdin
    phy_testdin = code_addr_msb;
    reg = 0;
    reg = (reg & ~(GENMASK(7, 0))) | (phy_testdin << 0);
    reg = (reg & ~(BIT_MASK(16))) | (phy_testen << 16);
    writel(reg, g_csi_addr + 0x800 + DWC_PHY_TEST_CTRL1);
    // reg_write(32'h9000_a854, {15'd0, phy_testen, 8'd0, phy_testdin});

    //f. set testclk to high
    phy_testclk = 1;
    reg = 0;
    reg = (reg & ~(BIT_MASK(0))) | (phy_testclr << 0);
    reg = (reg & ~(BIT_MASK(1))) | (phy_testclk << 1);
    writel(reg, g_csi_addr + 0x800 + DWC_PHY_TEST_CTRL0);
    // reg_write(32'h9000_a850, {30'd0, phy_testclk, phy_testclr});
    // #10ns;
    // udelay(1);
    udelay(1);
    //2. writing the 8-bit test code LSBs
    //a. set tesclk to low
    phy_testclk = 0;
    reg = 0;
    reg = (reg & ~(BIT_MASK(0))) | (phy_testclr << 0);
    reg = (reg & ~(BIT_MASK(1))) | (phy_testclk << 1);
    writel(reg, g_csi_addr + 0x800 + DWC_PHY_TEST_CTRL0);
    // reg_write(32'h9000_a850, {30'd0, phy_testclk, phy_testclr});
    //b. set testen to high
    //c. set testclk to high
    //d. place the 8-bit word test data in testdin
    phy_testen = 1;
    phy_testclk = 1;
    reg = 0;
    reg = (reg & ~(GENMASK(7, 0))) | (phy_testdin << 0);
    reg = (reg & ~(BIT_MASK(16))) | (phy_testen << 16);
    writel(reg, g_csi_addr + 0x800 + DWC_PHY_TEST_CTRL1);
    // reg_write(32'h9000_a854, {15'd0, phy_testen, 8'd0, phy_testdin});

    reg = 0;
    reg = (reg & ~(BIT_MASK(0))) | (phy_testclr << 0);
    reg = (reg & ~(BIT_MASK(1))) | (phy_testclk << 1);
    writel(reg, g_csi_addr + 0x800 + DWC_PHY_TEST_CTRL0);
    // reg_write(32'h9000_a850, {30'd0, phy_testclk, phy_testclr});
    phy_testdin = code_addr_lsb;
    reg = 0;
    reg = (reg & ~(GENMASK(7, 0))) | (phy_testdin << 0);
    reg = (reg & ~(BIT_MASK(16))) | (phy_testen << 16);
    writel(reg, g_csi_addr + 0x800 + DWC_PHY_TEST_CTRL1);
    // reg_write(32'h9000_a854, {15'd0, phy_testen, 8'd0, phy_testdin});

    // #10ns;
    // udelay(1);
    udelay(1);
    //e. set testclk to low
    phy_testclk = 0;
    reg = 0;
    reg = (reg & ~(BIT_MASK(0))) | (phy_testclr << 0);
    reg = (reg & ~(BIT_MASK(1))) | (phy_testclk << 1);
    writel(reg, g_csi_addr + 0x800 + DWC_PHY_TEST_CTRL0);
    // reg_write(32'h9000_a850, {30'd0, phy_testclk, phy_testclr});
    //f. set testen to low
    //3. writing data
    //a. place the 8-bit data
    //b. set testclk to high
    phy_testclk = 0;
    phy_testen = 0;
    reg = 0;
    reg = (reg & ~(GENMASK(7, 0))) | (phy_testdin << 0);
    reg = (reg & ~(BIT_MASK(16))) | (phy_testen << 16);
    writel(reg, g_csi_addr + 0x800 + DWC_PHY_TEST_CTRL1);
    // reg_write(32'h9000_a854, {15'd0, phy_testen, 8'd0, phy_testdin});
    phy_testdin = code_data;
    reg = 0;
    reg = (reg & ~(GENMASK(7, 0))) | (phy_testdin << 0);
    reg = (reg & ~(BIT_MASK(16))) | (phy_testen << 16);
    writel(reg, g_csi_addr + 0x800 + DWC_PHY_TEST_CTRL1);
    // reg_write(32'h9000_a854, {15'd0, phy_testen, 8'd0, phy_testdin});
    reg = 0;
    reg = (reg & ~(BIT_MASK(0))) | (phy_testclr << 0);
    reg = (reg & ~(BIT_MASK(1))) | (phy_testclk << 1);
    writel(reg, g_csi_addr + 0x800 + DWC_PHY_TEST_CTRL0);
    // reg_write(32'h9000_a850, {30'd0, phy_testclk, phy_testclr});
    // #10ns;
    // udelay(1);
    udelay(1);
    phy_testclk = 1;
    reg = 0;
    reg = (reg & ~(BIT_MASK(0))) | (phy_testclr << 0);
    reg = (reg & ~(BIT_MASK(1))) | (phy_testclk << 1);
    writel(reg, g_csi_addr + 0x800 + DWC_PHY_TEST_CTRL0);
    // reg_write(32'h9000_a850, {30'd0, phy_testclk, phy_testclr});
    // #10ns;
    // udelay(1);
    udelay(1);
    phy_testclk = 0;
    reg = 0;
    reg = (reg & ~(BIT_MASK(0))) | (phy_testclr << 0);
    reg = (reg & ~(BIT_MASK(1))) | (phy_testclk << 1);
    writel(reg, g_csi_addr + 0x800 + DWC_PHY_TEST_CTRL0);
    // reg_write(32'h9000_a850, {30'd0, phy_testclk, phy_testclr});
}


static void k230_csi1_rxphy1_reg_write(u16 phy_addr, u8 code_data)
{
    u8 code_addr_msb, code_addr_lsb, phy_testdin;
    u8 phy_testclr, phy_testclk,phy_testen;
    u32 reg = 0;

    code_addr_msb = (phy_addr >> 8) & 0xff;
    code_addr_lsb = phy_addr & 0xff;

    //1. writing the 4-bit test code MSBs
    //a. set testclk & testen to low

    phy_testclr = 0;
    phy_testclk = 0;
    phy_testen = 0;
    phy_testdin = 0;
    writel(0x0, g_csi_addr + 0x1000 + DWC_PHY_TEST_CTRL0);
    writel(0x0, g_csi_addr + 0x1000 + DWC_PHY_TEST_CTRL1);

    // #10ns;
    // udelay(1);
    udelay(1);
    //b.set tesen to high, set clk to high, any delay needed?
    phy_testen = 1;
    reg = 0;
    reg = (reg & ~(GENMASK(7, 0))) | (phy_testdin << 0);
    reg = (reg & ~(BIT_MASK(16))) | (phy_testen << 16);
    writel(reg, g_csi_addr + 0x1000 + DWC_PHY_TEST_CTRL1);

    // #10ns;
    // udelay(1);
    udelay(1);
    phy_testclk = 1;
    reg = 0;
    reg = (reg & ~(BIT_MASK(0))) | (phy_testclr << 0);
    reg = (reg & ~(BIT_MASK(1))) | (phy_testclk << 1);
    writel(reg, g_csi_addr + 0x1000 + DWC_PHY_TEST_CTRL0);

    //c. place 0x00 in testdin
    phy_testdin = 0;
    reg = 0;
    reg = (reg & ~(GENMASK(7, 0))) | (phy_testdin << 0);
    reg = (reg & ~(BIT_MASK(16))) | (phy_testen << 16);
    writel(reg, g_csi_addr + 0x1000 + DWC_PHY_TEST_CTRL1);
    // reg_write(32'h9000_a854, {15'd0, phy_testen, 8'd0, phy_testdin});

    //d. set testclk to low, set testen to low
    phy_testclk = 0;
    reg = 0;
    reg = (reg & ~(BIT_MASK(0))) | (phy_testclr << 0);
    reg = (reg & ~(BIT_MASK(1))) | (phy_testclk << 1);
    writel(reg, g_csi_addr + 0x1000 + DWC_PHY_TEST_CTRL0);
    // reg_write(32'h9000_a850, {30'd0, phy_testclk, phy_testclr});
    phy_testen = 0;
    reg = 0;
    reg = (reg & ~(GENMASK(7, 0))) | (phy_testdin << 0);
    reg = (reg & ~(BIT_MASK(16))) | (phy_testen << 16);
    writel(reg, g_csi_addr + 0x1000 + DWC_PHY_TEST_CTRL1);
    // reg_write(32'h9000_a854, {15'd0, phy_testen, 8'd0, phy_testdin});

    //e. place the 8-bit word corresponding to the testcode MSBs in testdin
    phy_testdin = code_addr_msb;
    reg = 0;
    reg = (reg & ~(GENMASK(7, 0))) | (phy_testdin << 0);
    reg = (reg & ~(BIT_MASK(16))) | (phy_testen << 16);
    writel(reg, g_csi_addr + 0x1000 + DWC_PHY_TEST_CTRL1);
    // reg_write(32'h9000_a854, {15'd0, phy_testen, 8'd0, phy_testdin});

    //f. set testclk to high
    phy_testclk = 1;
    reg = 0;
    reg = (reg & ~(BIT_MASK(0))) | (phy_testclr << 0);
    reg = (reg & ~(BIT_MASK(1))) | (phy_testclk << 1);
    writel(reg, g_csi_addr + 0x1000 + DWC_PHY_TEST_CTRL0);
    // reg_write(32'h9000_a850, {30'd0, phy_testclk, phy_testclr});
    // #10ns;
    // udelay(1);
    udelay(1);
    //2. writing the 8-bit test code LSBs
    //a. set tesclk to low
    phy_testclk = 0;
    reg = 0;
    reg = (reg & ~(BIT_MASK(0))) | (phy_testclr << 0);
    reg = (reg & ~(BIT_MASK(1))) | (phy_testclk << 1);
    writel(reg, g_csi_addr + 0x1000 + DWC_PHY_TEST_CTRL0);
    // reg_write(32'h9000_a850, {30'd0, phy_testclk, phy_testclr});
    //b. set testen to high
    //c. set testclk to high
    //d. place the 8-bit word test data in testdin
    phy_testen = 1;
    phy_testclk = 1;
    reg = 0;
    reg = (reg & ~(GENMASK(7, 0))) | (phy_testdin << 0);
    reg = (reg & ~(BIT_MASK(16))) | (phy_testen << 16);
    writel(reg, g_csi_addr + 0x1000 + DWC_PHY_TEST_CTRL1);
    // reg_write(32'h9000_a854, {15'd0, phy_testen, 8'd0, phy_testdin});

    reg = 0;
    reg = (reg & ~(BIT_MASK(0))) | (phy_testclr << 0);
    reg = (reg & ~(BIT_MASK(1))) | (phy_testclk << 1);
    writel(reg, g_csi_addr + 0x1000 + DWC_PHY_TEST_CTRL0);
    // reg_write(32'h9000_a850, {30'd0, phy_testclk, phy_testclr});
    phy_testdin = code_addr_lsb;
    reg = 0;
    reg = (reg & ~(GENMASK(7, 0))) | (phy_testdin << 0);
    reg = (reg & ~(BIT_MASK(16))) | (phy_testen << 16);
    writel(reg, g_csi_addr + 0x1000 + DWC_PHY_TEST_CTRL1);
    // reg_write(32'h9000_a854, {15'd0, phy_testen, 8'd0, phy_testdin});

    // #10ns;
    // udelay(1);
    udelay(1);
    //e. set testclk to low
    phy_testclk = 0;
    reg = 0;
    reg = (reg & ~(BIT_MASK(0))) | (phy_testclr << 0);
    reg = (reg & ~(BIT_MASK(1))) | (phy_testclk << 1);
    writel(reg, g_csi_addr + 0x1000 + DWC_PHY_TEST_CTRL0);
    // reg_write(32'h9000_a850, {30'd0, phy_testclk, phy_testclr});
    //f. set testen to low
    //3. writing data
    //a. place the 8-bit data
    //b. set testclk to high
    phy_testclk = 0;
    phy_testen = 0;
    reg = 0;
    reg = (reg & ~(GENMASK(7, 0))) | (phy_testdin << 0);
    reg = (reg & ~(BIT_MASK(16))) | (phy_testen << 16);
    writel(reg, g_csi_addr + 0x1000 + DWC_PHY_TEST_CTRL1);
    // reg_write(32'h9000_a854, {15'd0, phy_testen, 8'd0, phy_testdin});
    phy_testdin = code_data;
    reg = 0;
    reg = (reg & ~(GENMASK(7, 0))) | (phy_testdin << 0);
    reg = (reg & ~(BIT_MASK(16))) | (phy_testen << 16);
    writel(reg, g_csi_addr + 0x1000 + DWC_PHY_TEST_CTRL1);
    // reg_write(32'h9000_a854, {15'd0, phy_testen, 8'd0, phy_testdin});
    reg = 0;
    reg = (reg & ~(BIT_MASK(0))) | (phy_testclr << 0);
    reg = (reg & ~(BIT_MASK(1))) | (phy_testclk << 1);
    writel(reg, g_csi_addr + 0x1000 + DWC_PHY_TEST_CTRL0);
    // reg_write(32'h9000_a850, {30'd0, phy_testclk, phy_testclr});
    // #10ns;
    // udelay(1);
    udelay(1);
    phy_testclk = 1;
    reg = 0;
    reg = (reg & ~(BIT_MASK(0))) | (phy_testclr << 0);
    reg = (reg & ~(BIT_MASK(1))) | (phy_testclk << 1);
    writel(reg, g_csi_addr + 0x1000 + DWC_PHY_TEST_CTRL0);
    // reg_write(32'h9000_a850, {30'd0, phy_testclk, phy_testclr});
    // #10ns;
    // udelay(1);
    udelay(1);
    phy_testclk = 0;
    reg = 0;
    reg = (reg & ~(BIT_MASK(0))) | (phy_testclr << 0);
    reg = (reg & ~(BIT_MASK(1))) | (phy_testclk << 1);
    writel(reg, g_csi_addr + 0x1000 + DWC_PHY_TEST_CTRL0);
    // reg_write(32'h9000_a850, {30'd0, phy_testclk, phy_testclr});
}


static void k230_csi2_rxphy2_reg_write(u16 phy_addr, u8 code_data)
{

    u8 code_addr_msb, code_addr_lsb, phy_testdin;
    u8 phy_testclr, phy_testclk,phy_testen;
    u32 reg = 0;

    code_addr_msb = (phy_addr >> 8) & 0xff;
    code_addr_lsb = phy_addr & 0xff;

    //1. writing the 4-bit test code MSBs
    //a. set testclk & testen to low

    phy_testclr = 0;
    phy_testclk = 0;
    phy_testen = 0;
    phy_testdin = 0;
    writel(0x0, g_csi_addr + 0x1800 + DWC_PHY_TEST_CTRL0);
    writel(0x0, g_csi_addr + 0x1800 + DWC_PHY_TEST_CTRL1);

    // #10ns;
    // udelay(1);
    udelay(1);
    //b.set tesen to high, set clk to high, any delay needed?
    phy_testen = 1;
    reg = 0;
    reg = (reg & ~(GENMASK(7, 0))) | (phy_testdin << 0);
    reg = (reg & ~(BIT_MASK(16))) | (phy_testen << 16);
    writel(reg, g_csi_addr + 0x1800 + DWC_PHY_TEST_CTRL1);

    // #10ns;
    // udelay(1);
    udelay(1);
    phy_testclk = 1;
    reg = 0;
    reg = (reg & ~(BIT_MASK(0))) | (phy_testclr << 0);
    reg = (reg & ~(BIT_MASK(1))) | (phy_testclk << 1);
    writel(reg, g_csi_addr + 0x1800 + DWC_PHY_TEST_CTRL0);

    //c. place 0x00 in testdin
    phy_testdin = 0;
    reg = 0;
    reg = (reg & ~(GENMASK(7, 0))) | (phy_testdin << 0);
    reg = (reg & ~(BIT_MASK(16))) | (phy_testen << 16);
    writel(reg, g_csi_addr + 0x1800 + DWC_PHY_TEST_CTRL1);
    // reg_write(32'h9000_a854, {15'd0, phy_testen, 8'd0, phy_testdin});

    //d. set testclk to low, set testen to low
    phy_testclk = 0;
    reg = 0;
    reg = (reg & ~(BIT_MASK(0))) | (phy_testclr << 0);
    reg = (reg & ~(BIT_MASK(1))) | (phy_testclk << 1);
    writel(reg, g_csi_addr + 0x1800 + DWC_PHY_TEST_CTRL0);
    // reg_write(32'h9000_a850, {30'd0, phy_testclk, phy_testclr});
    phy_testen = 0;
    reg = 0;
    reg = (reg & ~(GENMASK(7, 0))) | (phy_testdin << 0);
    reg = (reg & ~(BIT_MASK(16))) | (phy_testen << 16);
    writel(reg, g_csi_addr + 0x1800 + DWC_PHY_TEST_CTRL1);
    // reg_write(32'h9000_a854, {15'd0, phy_testen, 8'd0, phy_testdin});

    //e. place the 8-bit word corresponding to the testcode MSBs in testdin
    phy_testdin = code_addr_msb;
    reg = 0;
    reg = (reg & ~(GENMASK(7, 0))) | (phy_testdin << 0);
    reg = (reg & ~(BIT_MASK(16))) | (phy_testen << 16);
    writel(reg, g_csi_addr + 0x1800 + DWC_PHY_TEST_CTRL1);
    // reg_write(32'h9000_a854, {15'd0, phy_testen, 8'd0, phy_testdin});

    //f. set testclk to high
    phy_testclk = 1;
    reg = 0;
    reg = (reg & ~(BIT_MASK(0))) | (phy_testclr << 0);
    reg = (reg & ~(BIT_MASK(1))) | (phy_testclk << 1);
    writel(reg, g_csi_addr + 0x1800 + DWC_PHY_TEST_CTRL0);
    // reg_write(32'h9000_a850, {30'd0, phy_testclk, phy_testclr});
    // #10ns;
    // udelay(1);
    udelay(1);
    //2. writing the 8-bit test code LSBs
    //a. set tesclk to low
    phy_testclk = 0;
    reg = 0;
    reg = (reg & ~(BIT_MASK(0))) | (phy_testclr << 0);
    reg = (reg & ~(BIT_MASK(1))) | (phy_testclk << 1);
    writel(reg, g_csi_addr + 0x1800 + DWC_PHY_TEST_CTRL0);
    // reg_write(32'h9000_a850, {30'd0, phy_testclk, phy_testclr});
    //b. set testen to high
    //c. set testclk to high
    //d. place the 8-bit word test data in testdin
    phy_testen = 1;
    phy_testclk = 1;
    reg = 0;
    reg = (reg & ~(GENMASK(7, 0))) | (phy_testdin << 0);
    reg = (reg & ~(BIT_MASK(16))) | (phy_testen << 16);
    writel(reg, g_csi_addr + 0x1800 + DWC_PHY_TEST_CTRL1);
    // reg_write(32'h9000_a854, {15'd0, phy_testen, 8'd0, phy_testdin});

    reg = 0;
    reg = (reg & ~(BIT_MASK(0))) | (phy_testclr << 0);
    reg = (reg & ~(BIT_MASK(1))) | (phy_testclk << 1);
    writel(reg, g_csi_addr + 0x1800 + DWC_PHY_TEST_CTRL0);
    // reg_write(32'h9000_a850, {30'd0, phy_testclk, phy_testclr});
    phy_testdin = code_addr_lsb;
    reg = 0;
    reg = (reg & ~(GENMASK(7, 0))) | (phy_testdin << 0);
    reg = (reg & ~(BIT_MASK(16))) | (phy_testen << 16);
    writel(reg, g_csi_addr + 0x1800 + DWC_PHY_TEST_CTRL1);
    // reg_write(32'h9000_a854, {15'd0, phy_testen, 8'd0, phy_testdin});

    // #10ns;
    // udelay(1);
    udelay(1);
    //e. set testclk to low
    phy_testclk = 0;
    reg = 0;
    reg = (reg & ~(BIT_MASK(0))) | (phy_testclr << 0);
    reg = (reg & ~(BIT_MASK(1))) | (phy_testclk << 1);
    writel(reg, g_csi_addr + 0x1800 + DWC_PHY_TEST_CTRL0);
    // reg_write(32'h9000_a850, {30'd0, phy_testclk, phy_testclr});
    //f. set testen to low
    //3. writing data
    //a. place the 8-bit data
    //b. set testclk to high
    phy_testclk = 0;
    phy_testen = 0;
    reg = 0;
    reg = (reg & ~(GENMASK(7, 0))) | (phy_testdin << 0);
    reg = (reg & ~(BIT_MASK(16))) | (phy_testen << 16);
    writel(reg, g_csi_addr + 0x1800 + DWC_PHY_TEST_CTRL1);
    // reg_write(32'h9000_a854, {15'd0, phy_testen, 8'd0, phy_testdin});
    phy_testdin = code_data;
    reg = 0;
    reg = (reg & ~(GENMASK(7, 0))) | (phy_testdin << 0);
    reg = (reg & ~(BIT_MASK(16))) | (phy_testen << 16);
    writel(reg, g_csi_addr + 0x1800 + DWC_PHY_TEST_CTRL1);
    // reg_write(32'h9000_a854, {15'd0, phy_testen, 8'd0, phy_testdin});
    reg = 0;
    reg = (reg & ~(BIT_MASK(0))) | (phy_testclr << 0);
    reg = (reg & ~(BIT_MASK(1))) | (phy_testclk << 1);
    writel(reg, g_csi_addr + 0x1800 + DWC_PHY_TEST_CTRL0);
    // reg_write(32'h9000_a850, {30'd0, phy_testclk, phy_testclr});
    // #10ns;
    // udelay(1);
    udelay(1);
    phy_testclk = 1;
    reg = 0;
    reg = (reg & ~(BIT_MASK(0))) | (phy_testclr << 0);
    reg = (reg & ~(BIT_MASK(1))) | (phy_testclk << 1);
    writel(reg, g_csi_addr + 0x1800 + DWC_PHY_TEST_CTRL0);
    // reg_write(32'h9000_a850, {30'd0, phy_testclk, phy_testclr});
    // #10ns;
    // udelay(1);
    udelay(1);
    phy_testclk = 0;
    reg = 0;
    reg = (reg & ~(BIT_MASK(0))) | (phy_testclr << 0);
    reg = (reg & ~(BIT_MASK(1))) | (phy_testclk << 1);
    writel(reg, g_csi_addr + 0x1800 + DWC_PHY_TEST_CTRL0);
    // reg_write(32'h9000_a850, {30'd0, phy_testclk, phy_testclr});
}


static void k230_csi0_rxphy1_reg_write(u16 phy_addr, u8 code_data)
{
    u8 code_addr_msb, code_addr_lsb, phy_testdin;
    u8 phy_testclr, phy_testclk,phy_testen;
    u32 reg = 0;

    code_addr_msb = (phy_addr >> 8) & 0xff;
    code_addr_lsb = phy_addr & 0xff;

    //1. writing the 4-bit test code MSBs
    //a. set testclk & testen to low

    phy_testclr = 0;
    phy_testclk = 0;
    phy_testen = 0;
    phy_testdin = 0;
    writel(0x0, g_csi_addr + 0x800 + DWC_PHY2_TEST_CTRL0);
    writel(0x0, g_csi_addr + 0x800 + DWC_PHY2_TEST_CTRL1);

    // #10ns;
    // udelay(1);
    udelay(1);
    //b.set tesen to high, set clk to high, any delay needed?
    phy_testen = 1;
    reg = 0;
    reg = (reg & ~(GENMASK(7, 0))) | (phy_testdin << 0);
    reg = (reg & ~(BIT_MASK(16))) | (phy_testen << 16);
    writel(reg, g_csi_addr + 0x800 + DWC_PHY2_TEST_CTRL1);

    // #10ns;
    // udelay(1);
    udelay(1);
    phy_testclk = 1;
    reg = 0;
    reg = (reg & ~(BIT_MASK(0))) | (phy_testclr << 0);
    reg = (reg & ~(BIT_MASK(1))) | (phy_testclk << 1);
    writel(reg, g_csi_addr + 0x800 + DWC_PHY2_TEST_CTRL0);

    //c. place 0x00 in testdin
    phy_testdin = 0;
    reg = 0;
    reg = (reg & ~(GENMASK(7, 0))) | (phy_testdin << 0);
    reg = (reg & ~(BIT_MASK(16))) | (phy_testen << 16);
    writel(reg, g_csi_addr + 0x800 + DWC_PHY2_TEST_CTRL1);
    // reg_write(32'h9000_a854, {15'd0, phy_testen, 8'd0, phy_testdin});

    //d. set testclk to low, set testen to low
    phy_testclk = 0;
    reg = 0;
    reg = (reg & ~(BIT_MASK(0))) | (phy_testclr << 0);
    reg = (reg & ~(BIT_MASK(1))) | (phy_testclk << 1);
    writel(reg, g_csi_addr + 0x800 + DWC_PHY2_TEST_CTRL0);
    // reg_write(32'h9000_a850, {30'd0, phy_testclk, phy_testclr});
    phy_testen = 0;
    reg = 0;
    reg = (reg & ~(GENMASK(7, 0))) | (phy_testdin << 0);
    reg = (reg & ~(BIT_MASK(16))) | (phy_testen << 16);
    writel(reg, g_csi_addr + 0x800 + DWC_PHY2_TEST_CTRL1);
    // reg_write(32'h9000_a854, {15'd0, phy_testen, 8'd0, phy_testdin});

    //e. place the 8-bit word corresponding to the testcode MSBs in testdin
    phy_testdin = code_addr_msb;
    reg = 0;
    reg = (reg & ~(GENMASK(7, 0))) | (phy_testdin << 0);
    reg = (reg & ~(BIT_MASK(16))) | (phy_testen << 16);
    writel(reg, g_csi_addr + 0x800 + DWC_PHY2_TEST_CTRL1);
    // reg_write(32'h9000_a854, {15'd0, phy_testen, 8'd0, phy_testdin});

    //f. set testclk to high
    phy_testclk = 1;
    reg = 0;
    reg = (reg & ~(BIT_MASK(0))) | (phy_testclr << 0);
    reg = (reg & ~(BIT_MASK(1))) | (phy_testclk << 1);
    writel(reg, g_csi_addr + 0x800 + DWC_PHY2_TEST_CTRL0);
    // reg_write(32'h9000_a850, {30'd0, phy_testclk, phy_testclr});
    // #10ns;
    // udelay(1);
    udelay(1);
    //2. writing the 8-bit test code LSBs
    //a. set tesclk to low
    phy_testclk = 0;
    reg = 0;
    reg = (reg & ~(BIT_MASK(0))) | (phy_testclr << 0);
    reg = (reg & ~(BIT_MASK(1))) | (phy_testclk << 1);
    writel(reg, g_csi_addr + 0x800 + DWC_PHY2_TEST_CTRL0);
    // reg_write(32'h9000_a850, {30'd0, phy_testclk, phy_testclr});
    //b. set testen to high
    //c. set testclk to high
    //d. place the 8-bit word test data in testdin
    phy_testen = 1;
    phy_testclk = 1;
    reg = 0;
    reg = (reg & ~(GENMASK(7, 0))) | (phy_testdin << 0);
    reg = (reg & ~(BIT_MASK(16))) | (phy_testen << 16);
    writel(reg, g_csi_addr + 0x800 + DWC_PHY2_TEST_CTRL1);
    // reg_write(32'h9000_a854, {15'd0, phy_testen, 8'd0, phy_testdin});

    reg = 0;
    reg = (reg & ~(BIT_MASK(0))) | (phy_testclr << 0);
    reg = (reg & ~(BIT_MASK(1))) | (phy_testclk << 1);
    writel(reg, g_csi_addr + 0x800 + DWC_PHY2_TEST_CTRL0);
    // reg_write(32'h9000_a850, {30'd0, phy_testclk, phy_testclr});
    phy_testdin = code_addr_lsb;
    reg = 0;
    reg = (reg & ~(GENMASK(7, 0))) | (phy_testdin << 0);
    reg = (reg & ~(BIT_MASK(16))) | (phy_testen << 16);
    writel(reg, g_csi_addr + 0x800 + DWC_PHY2_TEST_CTRL1);
    // reg_write(32'h9000_a854, {15'd0, phy_testen, 8'd0, phy_testdin});

    // #10ns;
    // udelay(1);
    udelay(1);
    //e. set testclk to low
    phy_testclk = 0;
    reg = 0;
    reg = (reg & ~(BIT_MASK(0))) | (phy_testclr << 0);
    reg = (reg & ~(BIT_MASK(1))) | (phy_testclk << 1);
    writel(reg, g_csi_addr + 0x800 + DWC_PHY2_TEST_CTRL0);
    // reg_write(32'h9000_a850, {30'd0, phy_testclk, phy_testclr});
    //f. set testen to low
    //3. writing data
    //a. place the 8-bit data
    //b. set testclk to high
    phy_testclk = 0;
    phy_testen = 0;
    reg = 0;
    reg = (reg & ~(GENMASK(7, 0))) | (phy_testdin << 0);
    reg = (reg & ~(BIT_MASK(16))) | (phy_testen << 16);
    writel(reg, g_csi_addr + 0x800 + DWC_PHY2_TEST_CTRL1);
    // reg_write(32'h9000_a854, {15'd0, phy_testen, 8'd0, phy_testdin});
    phy_testdin = code_data;
    reg = 0;
    reg = (reg & ~(GENMASK(7, 0))) | (phy_testdin << 0);
    reg = (reg & ~(BIT_MASK(16))) | (phy_testen << 16);
    writel(reg, g_csi_addr + 0x800 + DWC_PHY2_TEST_CTRL1);
    // reg_write(32'h9000_a854, {15'd0, phy_testen, 8'd0, phy_testdin});
    reg = 0;
    reg = (reg & ~(BIT_MASK(0))) | (phy_testclr << 0);
    reg = (reg & ~(BIT_MASK(1))) | (phy_testclk << 1);
    writel(reg, g_csi_addr + 0x800 + DWC_PHY2_TEST_CTRL0);
    // reg_write(32'h9000_a850, {30'd0, phy_testclk, phy_testclr});
    // #10ns;
    // udelay(1);
    udelay(1);
    phy_testclk = 1;
    reg = 0;
    reg = (reg & ~(BIT_MASK(0))) | (phy_testclr << 0);
    reg = (reg & ~(BIT_MASK(1))) | (phy_testclk << 1);
    writel(reg, g_csi_addr + 0x800 + DWC_PHY2_TEST_CTRL0);
    // reg_write(32'h9000_a850, {30'd0, phy_testclk, phy_testclr});
    // #10ns;
    // udelay(1);
    udelay(1);
    phy_testclk = 0;
    reg = 0;
    reg = (reg & ~(BIT_MASK(0))) | (phy_testclr << 0);
    reg = (reg & ~(BIT_MASK(1))) | (phy_testclk << 1);
    writel(reg, g_csi_addr + 0x800 + DWC_PHY2_TEST_CTRL0);
    // reg_write(32'h9000_a850, {30'd0, phy_testclk, phy_testclr});
}

#if 0
static u8 k230_csi0_rxphy0_reg_read(u16 phy_addr)
{
    u8 code_addr_msb, code_addr_lsb, phy_testdin;
    u8 phy_testclr, phy_testclk,phy_testen;
    u32 rdata, reg;

    code_addr_msb = (phy_addr >> 8) & 0xff ;
    code_addr_lsb = phy_addr  & 0xff;

    //1. writing the 4-bit test code MSBs
    //a. set testclk & testen to low
    phy_testclr = 0;
    phy_testclk = 0;
    phy_testen = 0;
    phy_testdin = 0;

    writel(0x0, g_csi_addr + 0x800 + DWC_PHY_TEST_CTRL0);
    writel(0x0, g_csi_addr + 0x800 + DWC_PHY_TEST_CTRL1);
    // reg_write(32'h9000_a850, {30'd0, phy_testclk, phy_testclr});
    // reg_write(32'h9000_a854, {15'd0, phy_testen, 8'd0, phy_testdin});
    // #10ns;
    // udelay(1);
    udelay(1);
    //b.set tesen to high, set clk to high, any delay needed?
    phy_testen = 1;
    reg = 0;
    reg = (reg & ~(GENMASK(7, 0))) | (phy_testdin << 0);
    reg = (reg & ~(BIT_MASK(16))) | (phy_testen << 16);
    writel(reg, g_csi_addr + 0x800 + DWC_PHY_TEST_CTRL1);
    // reg_write(32'h9000_a854, {15'd0, phy_testen, 8'd0, phy_testdin});
    // #10ns;
    // udelay(1);
    udelay(1);
    phy_testclk = 1;
    reg = 0;
    reg = (reg & ~(BIT_MASK(0))) | (phy_testclr << 0);
    reg = (reg & ~(BIT_MASK(1))) | (phy_testclk << 1);
    writel(reg, g_csi_addr + 0x800 + DWC_PHY_TEST_CTRL0);
    // reg_write(32'h9000_a850, {30'd0, phy_testclk, phy_testclr});

    //c. place 0x00 in testdin
    phy_testdin = 0;
    reg = 0;
    reg = (reg & ~(GENMASK(7, 0))) | (phy_testdin << 0);
    reg = (reg & ~(BIT_MASK(16))) | (phy_testen << 16);
    writel(reg, g_csi_addr + 0x800 + DWC_PHY_TEST_CTRL1);
    // reg_write(32'h9000_a854, {15'd0, phy_testen, 8'd0, phy_testdin});

    //d. set testclk to low, set testen to low
    phy_testclk = 0;
    reg = 0;
    reg = (reg & ~(BIT_MASK(0))) | (phy_testclr << 0);
    reg = (reg & ~(BIT_MASK(1))) | (phy_testclk << 1);
    writel(reg, g_csi_addr + 0x800 + DWC_PHY_TEST_CTRL0);
    // reg_write(32'h9000_a850, {30'd0, phy_testclk, phy_testclr});
    phy_testen = 0;
    reg = 0;
    reg = (reg & ~(GENMASK(7, 0))) | (phy_testdin << 0);
    reg = (reg & ~(BIT_MASK(16))) | (phy_testen << 16);
    writel(reg, g_csi_addr + 0x800 + DWC_PHY_TEST_CTRL1);
    // reg_write(32'h9000_a854, {15'd0, phy_testen, 8'd0, phy_testdin});

    //e. place the 8-bit word corresponding to the testcode MSBs in testdin
    phy_testdin = code_addr_msb;
    reg = 0;
    reg = (reg & ~(GENMASK(7, 0))) | (phy_testdin << 0);
    reg = (reg & ~(BIT_MASK(16))) | (phy_testen << 16);
    writel(reg, g_csi_addr + 0x800 + DWC_PHY_TEST_CTRL1);
    // reg_write(32'h9000_a854, {15'd0, phy_testen, 8'd0, phy_testdin});

    //f. set testclk to high
    phy_testclk = 1;
    reg = 0;
    reg = (reg & ~(BIT_MASK(0))) | (phy_testclr << 0);
    reg = (reg & ~(BIT_MASK(1))) | (phy_testclk << 1);
    writel(reg, g_csi_addr + 0x800 + DWC_PHY_TEST_CTRL0);
    // reg_write(32'h9000_a850, {30'd0, phy_testclk, phy_testclr});
    // #10ns;
    // udelay(1);
    udelay(1);
    //2. writing the 8-bit test code LSBs
    //a. set tesclk to low
    phy_testclk = 0;
    reg = 0;
    reg = (reg & ~(BIT_MASK(0))) | (phy_testclr << 0);
    reg = (reg & ~(BIT_MASK(1))) | (phy_testclk << 1);
    writel(reg, g_csi_addr + 0x800 + DWC_PHY_TEST_CTRL0);
    // reg_write(32'h9000_a850, {30'd0, phy_testclk, phy_testclr});
    //b. set testen to high
    //c. set testclk to high
    //d. place the 8-bit word test data in testdin
    phy_testen = 1;
    phy_testclk = 1;
    reg = 0;
    reg = (reg & ~(GENMASK(7, 0))) | (phy_testdin << 0);
    reg = (reg & ~(BIT_MASK(16))) | (phy_testen << 16);
    writel(reg, g_csi_addr + 0x800 + DWC_PHY_TEST_CTRL1);
    // reg_write(32'h9000_a854, {15'd0, phy_testen, 8'd0, phy_testdin});

    reg = 0;
    reg = (reg & ~(BIT_MASK(0))) | (phy_testclr << 0);
    reg = (reg & ~(BIT_MASK(1))) | (phy_testclk << 1);
    writel(reg, g_csi_addr + 0x800 + DWC_PHY_TEST_CTRL0);
    // reg_write(32'h9000_a850, {30'd0, phy_testclk, phy_testclr});
    phy_testdin = code_addr_lsb;
    reg = 0;
    reg = (reg & ~(GENMASK(7, 0))) | (phy_testdin << 0);
    reg = (reg & ~(BIT_MASK(16))) | (phy_testen << 16);
    writel(reg, g_csi_addr + 0x800 + DWC_PHY_TEST_CTRL1);
    // reg_write(32'h9000_a854, {15'd0, phy_testen, 8'd0, phy_testdin});
    // #10ns;
    // udelay(1);
    udelay(1);
    //e. set testclk to low
    phy_testclk = 0;
    reg = 0;
    reg = (reg & ~(BIT_MASK(0))) | (phy_testclr << 0);
    reg = (reg & ~(BIT_MASK(1))) | (phy_testclk << 1);
    writel(reg, g_csi_addr + 0x800 + DWC_PHY_TEST_CTRL0);
    // reg_write(32'h9000_a850, {30'd0, phy_testclk, phy_testclr});
    //f. set testen to low
    phy_testclk = 0;
    phy_testen = 0;
    reg = 0;
    reg = (reg & ~(GENMASK(7, 0))) | (phy_testdin << 0);
    reg = (reg & ~(BIT_MASK(16))) | (phy_testen << 16);
    writel(reg, g_csi_addr + 0x800 + DWC_PHY_TEST_CTRL1);
    // reg_write(32'h9000_a854, {15'd0, phy_testen, 8'd0, phy_testdin});
    //g. get phy data (read testout)
    rdata = readl(g_csi_addr + 0x800 + DWC_PHY_TEST_CTRL1);
    // reg_read(32'h9000_a854, rdata);

    rdata = (rdata >> 8) & 0xff;
    // code_data = rdata[15:8];

    return rdata;
}

static u8 k230_csi0_rxphy1_reg_read(u16 phy_addr)
{
    u8 code_addr_msb, code_addr_lsb, phy_testdin;
    u8 phy_testclr, phy_testclk,phy_testen;
    u32 rdata, reg;

    code_addr_msb = (phy_addr >> 8) & 0xff ;
    code_addr_lsb = phy_addr  & 0xff;

    //1. writing the 4-bit test code MSBs
    //a. set testclk & testen to low
    phy_testclr = 0;
    phy_testclk = 0;
    phy_testen = 0;
    phy_testdin = 0;

    writel(0x0, g_csi_addr + 0x800 + DWC_PHY2_TEST_CTRL0);
    writel(0x0, g_csi_addr + 0x800 + DWC_PHY2_TEST_CTRL1);
    // reg_write(32'h9000_a850, {30'd0, phy_testclk, phy_testclr});
    // reg_write(32'h9000_a854, {15'd0, phy_testen, 8'd0, phy_testdin});
    // #10ns;
    // udelay(1);
    udelay(1);
    //b.set tesen to high, set clk to high, any delay needed?
    phy_testen = 1;
    reg = 0;
    reg = (reg & ~(GENMASK(7, 0))) | (phy_testdin << 0);
    reg = (reg & ~(BIT_MASK(16))) | (phy_testen << 16);
    writel(reg, g_csi_addr + 0x800 + DWC_PHY2_TEST_CTRL1);
    // reg_write(32'h9000_a854, {15'd0, phy_testen, 8'd0, phy_testdin});
    // #10ns;
    // udelay(1);
    udelay(1);
    phy_testclk = 1;
    reg = 0;
    reg = (reg & ~(BIT_MASK(0))) | (phy_testclr << 0);
    reg = (reg & ~(BIT_MASK(1))) | (phy_testclk << 1);
    writel(reg, g_csi_addr + 0x800 + DWC_PHY2_TEST_CTRL0);
    // reg_write(32'h9000_a850, {30'd0, phy_testclk, phy_testclr});

    //c. place 0x00 in testdin
    phy_testdin = 0;
    reg = 0;
    reg = (reg & ~(GENMASK(7, 0))) | (phy_testdin << 0);
    reg = (reg & ~(BIT_MASK(16))) | (phy_testen << 16);
    writel(reg, g_csi_addr + 0x800 + DWC_PHY2_TEST_CTRL1);
    // reg_write(32'h9000_a854, {15'd0, phy_testen, 8'd0, phy_testdin});

    //d. set testclk to low, set testen to low
    phy_testclk = 0;
    reg = 0;
    reg = (reg & ~(BIT_MASK(0))) | (phy_testclr << 0);
    reg = (reg & ~(BIT_MASK(1))) | (phy_testclk << 1);
    writel(reg, g_csi_addr + 0x800 + DWC_PHY2_TEST_CTRL0);
    // reg_write(32'h9000_a850, {30'd0, phy_testclk, phy_testclr});
    phy_testen = 0;
    reg = 0;
    reg = (reg & ~(GENMASK(7, 0))) | (phy_testdin << 0);
    reg = (reg & ~(BIT_MASK(16))) | (phy_testen << 16);
    writel(reg, g_csi_addr + 0x800 + DWC_PHY2_TEST_CTRL1);
    // reg_write(32'h9000_a854, {15'd0, phy_testen, 8'd0, phy_testdin});

    //e. place the 8-bit word corresponding to the testcode MSBs in testdin
    phy_testdin = code_addr_msb;
    reg = 0;
    reg = (reg & ~(GENMASK(7, 0))) | (phy_testdin << 0);
    reg = (reg & ~(BIT_MASK(16))) | (phy_testen << 16);
    writel(reg, g_csi_addr + 0x800 + DWC_PHY2_TEST_CTRL1);
    // reg_write(32'h9000_a854, {15'd0, phy_testen, 8'd0, phy_testdin});

    //f. set testclk to high
    phy_testclk = 1;
    reg = 0;
    reg = (reg & ~(BIT_MASK(0))) | (phy_testclr << 0);
    reg = (reg & ~(BIT_MASK(1))) | (phy_testclk << 1);
    writel(reg, g_csi_addr + 0x800 + DWC_PHY2_TEST_CTRL0);
    // reg_write(32'h9000_a850, {30'd0, phy_testclk, phy_testclr});
    // #10ns;
    // udelay(1);
    udelay(1);
    //2. writing the 8-bit test code LSBs
    //a. set tesclk to low
    phy_testclk = 0;
    reg = 0;
    reg = (reg & ~(BIT_MASK(0))) | (phy_testclr << 0);
    reg = (reg & ~(BIT_MASK(1))) | (phy_testclk << 1);
    writel(reg, g_csi_addr + 0x800 + DWC_PHY2_TEST_CTRL0);
    // reg_write(32'h9000_a850, {30'd0, phy_testclk, phy_testclr});
    //b. set testen to high
    //c. set testclk to high
    //d. place the 8-bit word test data in testdin
    phy_testen = 1;
    phy_testclk = 1;
    reg = 0;
    reg = (reg & ~(GENMASK(7, 0))) | (phy_testdin << 0);
    reg = (reg & ~(BIT_MASK(16))) | (phy_testen << 16);
    writel(reg, g_csi_addr + 0x800 + DWC_PHY2_TEST_CTRL1);
    // reg_write(32'h9000_a854, {15'd0, phy_testen, 8'd0, phy_testdin});

    reg = 0;
    reg = (reg & ~(BIT_MASK(0))) | (phy_testclr << 0);
    reg = (reg & ~(BIT_MASK(1))) | (phy_testclk << 1);
    writel(reg, g_csi_addr + 0x800 + DWC_PHY2_TEST_CTRL0);
    // reg_write(32'h9000_a850, {30'd0, phy_testclk, phy_testclr});
    phy_testdin = code_addr_lsb;
    reg = 0;
    reg = (reg & ~(GENMASK(7, 0))) | (phy_testdin << 0);
    reg = (reg & ~(BIT_MASK(16))) | (phy_testen << 16);
    writel(reg, g_csi_addr + 0x800 + DWC_PHY2_TEST_CTRL1);
    // reg_write(32'h9000_a854, {15'd0, phy_testen, 8'd0, phy_testdin});
    // #10ns;
    // udelay(1);
    udelay(1);
    //e. set testclk to low
    phy_testclk = 0;
    reg = 0;
    reg = (reg & ~(BIT_MASK(0))) | (phy_testclr << 0);
    reg = (reg & ~(BIT_MASK(1))) | (phy_testclk << 1);
    writel(reg, g_csi_addr + 0x800 + DWC_PHY2_TEST_CTRL0);
    // reg_write(32'h9000_a850, {30'd0, phy_testclk, phy_testclr});
    //f. set testen to low
    phy_testclk = 0;
    phy_testen = 0;
    reg = 0;
    reg = (reg & ~(GENMASK(7, 0))) | (phy_testdin << 0);
    reg = (reg & ~(BIT_MASK(16))) | (phy_testen << 16);
    writel(reg, g_csi_addr + 0x800 + DWC_PHY2_TEST_CTRL1);
    // reg_write(32'h9000_a854, {15'd0, phy_testen, 8'd0, phy_testdin});
    //g. get phy data (read testout)
    rdata = readl(g_csi_addr + 0x800 + DWC_PHY2_TEST_CTRL1);
    // reg_read(32'h9000_a854, rdata);

    rdata = (rdata >> 8) & 0xff;
    // code_data = rdata[15:8];

    return rdata;
}

static u8 k230_csi1_rxphy1_reg_read(u16 phy_addr)
{
    u8 code_addr_msb, code_addr_lsb, phy_testdin;
    u8 phy_testclr, phy_testclk,phy_testen;
    u32 rdata, reg;

    code_addr_msb = (phy_addr >> 8) & 0xff ;
    code_addr_lsb = phy_addr  & 0xff;

    //1. writing the 4-bit test code MSBs
    //a. set testclk & testen to low
    phy_testclr = 0;
    phy_testclk = 0;
    phy_testen = 0;
    phy_testdin = 0;

    writel(0x0, g_csi_addr + 0x1000 + DWC_PHY_TEST_CTRL0);
    writel(0x0, g_csi_addr + 0x1000 + DWC_PHY_TEST_CTRL1);
    // reg_write(32'h9000_a850, {30'd0, phy_testclk, phy_testclr});
    // reg_write(32'h9000_a854, {15'd0, phy_testen, 8'd0, phy_testdin});
    // #10ns;
    // udelay(1);
    udelay(1);
    //b.set tesen to high, set clk to high, any delay needed?
    phy_testen = 1;
    reg = 0;
    reg = (reg & ~(GENMASK(7, 0))) | (phy_testdin << 0);
    reg = (reg & ~(BIT_MASK(16))) | (phy_testen << 16);
    writel(reg, g_csi_addr + 0x1000 + DWC_PHY_TEST_CTRL1);
    // reg_write(32'h9000_a854, {15'd0, phy_testen, 8'd0, phy_testdin});
    // #10ns;
    // udelay(1);
    udelay(1);
    phy_testclk = 1;
    reg = 0;
    reg = (reg & ~(BIT_MASK(0))) | (phy_testclr << 0);
    reg = (reg & ~(BIT_MASK(1))) | (phy_testclk << 1);
    writel(reg, g_csi_addr + 0x1000 + DWC_PHY_TEST_CTRL0);
    // reg_write(32'h9000_a850, {30'd0, phy_testclk, phy_testclr});

    //c. place 0x00 in testdin
    phy_testdin = 0;
    reg = 0;
    reg = (reg & ~(GENMASK(7, 0))) | (phy_testdin << 0);
    reg = (reg & ~(BIT_MASK(16))) | (phy_testen << 16);
    writel(reg, g_csi_addr + 0x1000 + DWC_PHY_TEST_CTRL1);
    // reg_write(32'h9000_a854, {15'd0, phy_testen, 8'd0, phy_testdin});

    //d. set testclk to low, set testen to low
    phy_testclk = 0;
    reg = 0;
    reg = (reg & ~(BIT_MASK(0))) | (phy_testclr << 0);
    reg = (reg & ~(BIT_MASK(1))) | (phy_testclk << 1);
    writel(reg, g_csi_addr + 0x1000 + DWC_PHY_TEST_CTRL0);
    // reg_write(32'h9000_a850, {30'd0, phy_testclk, phy_testclr});
    phy_testen = 0;
    reg = 0;
    reg = (reg & ~(GENMASK(7, 0))) | (phy_testdin << 0);
    reg = (reg & ~(BIT_MASK(16))) | (phy_testen << 16);
    writel(reg, g_csi_addr + 0x1000 + DWC_PHY_TEST_CTRL1);
    // reg_write(32'h9000_a854, {15'd0, phy_testen, 8'd0, phy_testdin});

    //e. place the 8-bit word corresponding to the testcode MSBs in testdin
    phy_testdin = code_addr_msb;
    reg = 0;
    reg = (reg & ~(GENMASK(7, 0))) | (phy_testdin << 0);
    reg = (reg & ~(BIT_MASK(16))) | (phy_testen << 16);
    writel(reg, g_csi_addr + 0x1000 + DWC_PHY_TEST_CTRL1);
    // reg_write(32'h9000_a854, {15'd0, phy_testen, 8'd0, phy_testdin});

    //f. set testclk to high
    phy_testclk = 1;
    reg = 0;
    reg = (reg & ~(BIT_MASK(0))) | (phy_testclr << 0);
    reg = (reg & ~(BIT_MASK(1))) | (phy_testclk << 1);
    writel(reg, g_csi_addr + 0x1000 + DWC_PHY_TEST_CTRL0);
    // reg_write(32'h9000_a850, {30'd0, phy_testclk, phy_testclr});
    // #10ns;
    // udelay(1);
    udelay(1);
    //2. writing the 8-bit test code LSBs
    //a. set tesclk to low
    phy_testclk = 0;
    reg = 0;
    reg = (reg & ~(BIT_MASK(0))) | (phy_testclr << 0);
    reg = (reg & ~(BIT_MASK(1))) | (phy_testclk << 1);
    writel(reg, g_csi_addr + 0x1000 + DWC_PHY_TEST_CTRL0);
    // reg_write(32'h9000_a850, {30'd0, phy_testclk, phy_testclr});
    //b. set testen to high
    //c. set testclk to high
    //d. place the 8-bit word test data in testdin
    phy_testen = 1;
    phy_testclk = 1;
    reg = 0;
    reg = (reg & ~(GENMASK(7, 0))) | (phy_testdin << 0);
    reg = (reg & ~(BIT_MASK(16))) | (phy_testen << 16);
    writel(reg, g_csi_addr + 0x1000 + DWC_PHY_TEST_CTRL1);
    // reg_write(32'h9000_a854, {15'd0, phy_testen, 8'd0, phy_testdin});

    reg = 0;
    reg = (reg & ~(BIT_MASK(0))) | (phy_testclr << 0);
    reg = (reg & ~(BIT_MASK(1))) | (phy_testclk << 1);
    writel(reg, g_csi_addr + 0x1000 + DWC_PHY_TEST_CTRL0);
    // reg_write(32'h9000_a850, {30'd0, phy_testclk, phy_testclr});
    phy_testdin = code_addr_lsb;
    reg = 0;
    reg = (reg & ~(GENMASK(7, 0))) | (phy_testdin << 0);
    reg = (reg & ~(BIT_MASK(16))) | (phy_testen << 16);
    writel(reg, g_csi_addr + 0x1000 + DWC_PHY_TEST_CTRL1);
    // reg_write(32'h9000_a854, {15'd0, phy_testen, 8'd0, phy_testdin});
    // #10ns;
    // udelay(1);
    udelay(1);
    //e. set testclk to low
    phy_testclk = 0;
    reg = 0;
    reg = (reg & ~(BIT_MASK(0))) | (phy_testclr << 0);
    reg = (reg & ~(BIT_MASK(1))) | (phy_testclk << 1);
    writel(reg, g_csi_addr + 0x1000 + DWC_PHY_TEST_CTRL0);
    // reg_write(32'h9000_a850, {30'd0, phy_testclk, phy_testclr});
    //f. set testen to low
    phy_testclk = 0;
    phy_testen = 0;
    reg = 0;
    reg = (reg & ~(GENMASK(7, 0))) | (phy_testdin << 0);
    reg = (reg & ~(BIT_MASK(16))) | (phy_testen << 16);
    writel(reg, g_csi_addr + 0x1000 + DWC_PHY_TEST_CTRL1);
    // reg_write(32'h9000_a854, {15'd0, phy_testen, 8'd0, phy_testdin});
    //g. get phy data (read testout)
    rdata = readl(g_csi_addr + 0x1000 + DWC_PHY_TEST_CTRL1);
    // reg_read(32'h9000_a854, rdata);

    rdata = (rdata >> 8) & 0xffff;
    // code_data = rdata[15:8];

    return rdata;
}

static u32 k230_csi2_rxphy2_reg_read(u16 phy_addr)
{
    u8 code_addr_msb, code_addr_lsb, phy_testdin;
    u8 phy_testclr, phy_testclk,phy_testen;
    u32 rdata, reg;

    code_addr_msb = (phy_addr >> 8) & 0xff ;
    code_addr_lsb = phy_addr  & 0xff;

    //1. writing the 4-bit test code MSBs
    //a. set testclk & testen to low
    phy_testclr = 0;
    phy_testclk = 0;
    phy_testen = 0;
    phy_testdin = 0;

    writel(0x0, g_csi_addr + 0x1800 + DWC_PHY_TEST_CTRL0);
    writel(0x0, g_csi_addr + 0x1800 + DWC_PHY_TEST_CTRL1);
    // reg_write(32'h9000_a850, {30'd0, phy_testclk, phy_testclr});
    // reg_write(32'h9000_a854, {15'd0, phy_testen, 8'd0, phy_testdin});
    // #10ns;
    // udelay(1);
    udelay(1);
    //b.set tesen to high, set clk to high, any delay needed?
    phy_testen = 1;
    reg = 0;
    reg = (reg & ~(GENMASK(7, 0))) | (phy_testdin << 0);
    reg = (reg & ~(BIT_MASK(16))) | (phy_testen << 16);
    writel(reg, g_csi_addr + 0x1800 + DWC_PHY_TEST_CTRL1);
    // reg_write(32'h9000_a854, {15'd0, phy_testen, 8'd0, phy_testdin});
    // #10ns;
    // udelay(1);
    udelay(1);
    phy_testclk = 1;
    reg = 0;
    reg = (reg & ~(BIT_MASK(0))) | (phy_testclr << 0);
    reg = (reg & ~(BIT_MASK(1))) | (phy_testclk << 1);
    writel(reg, g_csi_addr + 0x1800 + DWC_PHY_TEST_CTRL0);
    // reg_write(32'h9000_a850, {30'd0, phy_testclk, phy_testclr});

    //c. place 0x00 in testdin
    phy_testdin = 0;
    reg = 0;
    reg = (reg & ~(GENMASK(7, 0))) | (phy_testdin << 0);
    reg = (reg & ~(BIT_MASK(16))) | (phy_testen << 16);
    writel(reg, g_csi_addr + 0x1800 + DWC_PHY_TEST_CTRL1);
    // reg_write(32'h9000_a854, {15'd0, phy_testen, 8'd0, phy_testdin});

    //d. set testclk to low, set testen to low
    phy_testclk = 0;
    reg = 0;
    reg = (reg & ~(BIT_MASK(0))) | (phy_testclr << 0);
    reg = (reg & ~(BIT_MASK(1))) | (phy_testclk << 1);
    writel(reg, g_csi_addr + 0x1800 + DWC_PHY_TEST_CTRL0);
    // reg_write(32'h9000_a850, {30'd0, phy_testclk, phy_testclr});
    phy_testen = 0;
    reg = 0;
    reg = (reg & ~(GENMASK(7, 0))) | (phy_testdin << 0);
    reg = (reg & ~(BIT_MASK(16))) | (phy_testen << 16);
    writel(reg, g_csi_addr + 0x1800 + DWC_PHY_TEST_CTRL1);
    // reg_write(32'h9000_a854, {15'd0, phy_testen, 8'd0, phy_testdin});

    //e. place the 8-bit word corresponding to the testcode MSBs in testdin
    phy_testdin = code_addr_msb;
    reg = 0;
    reg = (reg & ~(GENMASK(7, 0))) | (phy_testdin << 0);
    reg = (reg & ~(BIT_MASK(16))) | (phy_testen << 16);
    writel(reg, g_csi_addr + 0x1800 + DWC_PHY_TEST_CTRL1);
    // reg_write(32'h9000_a854, {15'd0, phy_testen, 8'd0, phy_testdin});

    //f. set testclk to high
    phy_testclk = 1;
    reg = 0;
    reg = (reg & ~(BIT_MASK(0))) | (phy_testclr << 0);
    reg = (reg & ~(BIT_MASK(1))) | (phy_testclk << 1);
    writel(reg, g_csi_addr + 0x1800 + DWC_PHY_TEST_CTRL0);
    // reg_write(32'h9000_a850, {30'd0, phy_testclk, phy_testclr});
    // #10ns;
    // udelay(1);
    udelay(1);
    //2. writing the 8-bit test code LSBs
    //a. set tesclk to low
    phy_testclk = 0;
    reg = 0;
    reg = (reg & ~(BIT_MASK(0))) | (phy_testclr << 0);
    reg = (reg & ~(BIT_MASK(1))) | (phy_testclk << 1);
    writel(reg, g_csi_addr + 0x1800 + DWC_PHY_TEST_CTRL0);
    // reg_write(32'h9000_a850, {30'd0, phy_testclk, phy_testclr});
    //b. set testen to high
    //c. set testclk to high
    //d. place the 8-bit word test data in testdin
    phy_testen = 1;
    phy_testclk = 1;
    reg = 0;
    reg = (reg & ~(GENMASK(7, 0))) | (phy_testdin << 0);
    reg = (reg & ~(BIT_MASK(16))) | (phy_testen << 16);
    writel(reg, g_csi_addr + 0x1800 + DWC_PHY_TEST_CTRL1);
    // reg_write(32'h9000_a854, {15'd0, phy_testen, 8'd0, phy_testdin});

    reg = 0;
    reg = (reg & ~(BIT_MASK(0))) | (phy_testclr << 0);
    reg = (reg & ~(BIT_MASK(1))) | (phy_testclk << 1);
    writel(reg, g_csi_addr + 0x1800 + DWC_PHY_TEST_CTRL0);
    // reg_write(32'h9000_a850, {30'd0, phy_testclk, phy_testclr});
    phy_testdin = code_addr_lsb;
    reg = 0;
    reg = (reg & ~(GENMASK(7, 0))) | (phy_testdin << 0);
    reg = (reg & ~(BIT_MASK(16))) | (phy_testen << 16);
    writel(reg, g_csi_addr + 0x1800 + DWC_PHY_TEST_CTRL1);
    // reg_write(32'h9000_a854, {15'd0, phy_testen, 8'd0, phy_testdin});
    // #10ns;
    // udelay(1);
    udelay(1);
    //e. set testclk to low
    phy_testclk = 0;
    reg = 0;
    reg = (reg & ~(BIT_MASK(0))) | (phy_testclr << 0);
    reg = (reg & ~(BIT_MASK(1))) | (phy_testclk << 1);
    writel(reg, g_csi_addr + 0x1800 + DWC_PHY_TEST_CTRL0);
    // reg_write(32'h9000_a850, {30'd0, phy_testclk, phy_testclr});
    //f. set testen to low
    phy_testclk = 0;
    phy_testen = 0;
    reg = 0;
    reg = (reg & ~(GENMASK(7, 0))) | (phy_testdin << 0);
    reg = (reg & ~(BIT_MASK(16))) | (phy_testen << 16);
    writel(reg, g_csi_addr + 0x1800 + DWC_PHY_TEST_CTRL1);
    // reg_write(32'h9000_a854, {15'd0, phy_testen, 8'd0, phy_testdin});
    //g. get phy data (read testout)
    rdata = readl(g_csi_addr + 0x1800 + DWC_PHY_TEST_CTRL1);
    // reg_read(32'h9000_a854, rdata);

    rdata = (rdata >> 8) & 0xffff;
    // code_data = rdata[15:8];

    return rdata;
}
#endif
static void k230_csi0_phy0_config(u32 freq)
{
    //1. set rstz = 1'b0
    writel(0x0, g_csi_addr + 0x800 + DWC_DPHY_RSTZ);
    //2. set shutdownz=1'b0
    writel(0x0, g_csi_addr + 0x800 + DWC_PHY_SHUTDOWNZ);
    //3. set testclr=1'b1
    writel(0x1, g_csi_addr + 0x800 + DWC_PHY_TEST_CTRL0);
    //4. wait for 15ns
    // #15ns;
    // udelay(1);
    udelay(1);
    //5. set testclr =1'b0
    writel(0x0, g_csi_addr + 0x800 + DWC_PHY_TEST_CTRL0);
    //6. set hsfreqrange[6:0]=7'b0001010
    //phy_reg_write(16'h2, 7'b0001010);

    // writel(0x26 << 25, g_csi_addr );    //h9000_9010
    writel(freq << 25, g_csi_addr );

    //7. config register 0xe5 to set counter_for_des_enbypass(bit0) to 1'b1
    k230_csi0_rxphy0_reg_write(0xe5, 0x1);
    //8. config 0xe4(bits7:4) to 2 (27M cflclk) for counter_for_des_en_config_if_fw
    k230_csi0_rxphy0_reg_write(0xe4, 0x20); //8'b00100000
    //9. config 0x1ab to set cb_sel_vref_lprx_fw(bits1:0) to 2'b10
    k230_csi0_rxphy0_reg_write(0x1ab, 0x06);  //8'b00000110
    //10. config 0x1ac to set rx_rxlp_bias_prog_rw(bit6) to 1'b1
    k230_csi0_rxphy0_reg_write(0x1ac, 0x4b);  // 8'b01001011
    //11. config 0x8 to set deskew_pol_rw signal (bit 5) to 1'b1
    k230_csi0_rxphy0_reg_write(0x08, 0x38);  //8'b00111000
    //12. config 0x307 to set rxclk_rxhs_pull_long_channel_if_rw signal(bit 7) to 1'b1
    k230_csi0_rxphy0_reg_write(0x307, 0x80);  //8'b10000000

    //13. config 0x607 to set rxclk_rxhs_ddl_tune_ovr_en_rw(bit 5) to 1'b1 and set rx0_rxhs_ddl_tune_ovr_rw[4:0] (bits 4:0) to 5'b11111
    k230_csi0_rxphy0_reg_write(0x607, 0x3f);  //8'b00111111
    //14. config 0x807 to set rx1_rxhs_ddl_tune_ovr_en_rw(bit 5) to 1'b1 and set rx1_rxhs_ddl_tune_ovr_rw[4:0](bits 4:0) to 5'b11111
    k230_csi0_rxphy0_reg_write(0x807, 0x3f);  //8'b00111111

    //15. set cfgclkfreqrange[5:0] = round[Fcfg_clk(MHz)-17)*4] = 6'b101000
    //16. Apply cfg_clk signal with the appropriate frequency with 27Mhz frequency

    // writel(0x00000050 | (0x26 << 25), g_csi_addr );  //h9000_9010  hsfreqrange_0 = 0x26

    writel(0x00000050 | (freq << 25), g_csi_addr );  //h9000_9010  hsfreqrange_0 = 0x26

    //17. Set basedir_0 = 1'b1
    // writel(0x00000051 | (0x26 << 25), g_csi_addr );  //h9000_9010
    writel(0x00000051 | (freq << 25), g_csi_addr );  //h9000_9010
    //18. Set forcerxm
    writel(0x00c00051 | (freq << 25), g_csi_addr );  //h9000_9010
    // writel(0x00c00051 | (0x26 << 25), g_csi_addr );  //h9000_9010
    //19. Set all requests inputs to zero;
    //20. Wait for 15 ns;
    // #15ns;
    // udelay(1);
    udelay(1);
    //21. Set enable_n and enableclk=1'b1;
    //tie 1 in maix3
    //22. Wait 5ns;
    // #5ns;

    //23. Set shutdownz=1'b1;
    writel(0x1, g_csi_addr + 0x800 + DWC_PHY_SHUTDOWNZ);
    //24. Wait 5ns;
    // #5ns;
    //25. Set rstz=1'b1;
    writel(0x1, g_csi_addr + 0x800 + DWC_DPHY_RSTZ);
    //26. Wait until stopstatedata_n and stopstateclk outputs are asserted indicating PHY is receiving LP11 in enabled datalanes and clocklanes;
    // #1000ns;
    // udelay(1);
    // udelay(1);
    // int count = 0;
    // while((readl(g_csi_addr + 0x800 + DWC_PHY_STOPSTATE) & 0x10000) != 0x10000)
    // {
    //     // udelay(1);
    //     udelay(1);
    //     count++;
    //     if (count >1000000) {
    //         pr_err("wait stop timeout\n");
    //         break;
    //     }
    // }
    // udelay(1);
    // udelay(1);

    // writel(0x00000051 | (0x14 << 25), g_csi_addr );  //h9000_9010
}

static void k230_csi1_phy1_config(u32 freq)
{
    int count = 0;
    //1. set rstz = 1'b0
    writel(0x0, g_csi_addr + 0x1000 + DWC_DPHY_RSTZ);
    //2. set shutdownz=1'b0
    writel(0x0, g_csi_addr + 0x1000 + DWC_PHY_SHUTDOWNZ);
    //3. set testclr=1'b1
    writel(0x1, g_csi_addr + 0x1000 + DWC_PHY_TEST_CTRL0);
    //4. wait for 15ns
    // #15ns;
    // udelay(1);
    udelay(1);
    //5. set testclr =1'b0
    writel(0x0, g_csi_addr + 0x1000 + DWC_PHY_TEST_CTRL0);
    //6. set hsfreqrange[6:0]=7'b0001010
    //phy_reg_write(16'h2, 7'b0001010);
    writel(freq << 25, g_csi_addr + 0x08);                 //h9000_9010

    //7. config register 0xe5 to set counter_for_des_enbypass(bit0) to 1'b1
    k230_csi1_rxphy1_reg_write(0xe5, 0x1);
    //8. config 0xe4(bits7:4) to 2 (27M cflclk) for counter_for_des_en_config_if_fw
    k230_csi1_rxphy1_reg_write(0xe4, 0x20); //8'b00100000
    //9. config 0x1ab to set cb_sel_vref_lprx_fw(bits1:0) to 2'b10
    k230_csi1_rxphy1_reg_write(0x1ab, 0x06);  //8'b00000110
    //10. config 0x1ac to set rx_rxlp_bias_prog_rw(bit6) to 1'b1
    k230_csi1_rxphy1_reg_write(0x1ac, 0x4b);  // 8'b01001011
    //11. config 0x8 to set deskew_pol_rw signal (bit 5) to 1'b1
    k230_csi1_rxphy1_reg_write(0x08, 0x38);  //8'b00111000
    //12. config 0x307 to set rxclk_rxhs_pull_long_channel_if_rw signal(bit 7) to 1'b1
    k230_csi1_rxphy1_reg_write(0x307, 0x80);  //8'b10000000

    //13. config 0x607 to set rxclk_rxhs_ddl_tune_ovr_en_rw(bit 5) to 1'b1 and set rx0_rxhs_ddl_tune_ovr_rw[4:0] (bits 4:0) to 5'b11111
    k230_csi1_rxphy1_reg_write(0x607, 0x3f);  //8'b00111111
    //14. config 0x807 to set rx1_rxhs_ddl_tune_ovr_en_rw(bit 5) to 1'b1 and set rx1_rxhs_ddl_tune_ovr_rw[4:0](bits 4:0) to 5'b11111
    k230_csi1_rxphy1_reg_write(0x807, 0x3f);  //8'b00111111

    //15. set cfgclkfreqrange[5:0] = round[Fcfg_clk(MHz)-17)*4] = 6'b101000
    //16. Apply cfg_clk signal with the appropriate frequency with 27Mhz frequency
    writel(0x00000050 | (freq << 25), g_csi_addr + 0x08);  //h9000_9010  hsfreqrange_0 = 0x26

    //17. Set basedir_0 = 1'b1
    writel(0x00000051 | (freq << 25), g_csi_addr + 0x08);  //h9000_9010
    //18. Set forcerxm
    writel(0x00c00051 | (freq << 25), g_csi_addr + 0x08);  //h9000_9010
    //19. Set all requests inputs to zero;
    //20. Wait for 15 ns;
    // #15ns;
    // udelay(1);
    udelay(1);
    //21. Set enable_n and enableclk=1'b1;
    //tie 1 in maix3
    //22. Wait 5ns;
    // #5ns;
    //23. Set shutdownz=1'b1;
    writel(0x1, g_csi_addr + 0x1000 + DWC_PHY_SHUTDOWNZ);
    //24. Wait 5ns;
    // #5ns;
    // udelay(1);
    udelay(1);
    //25. Set rstz=1'b1;
    writel(0x1, g_csi_addr + 0x1000 + DWC_DPHY_RSTZ);
    //26. Wait until stopstatedata_n and stopstateclk outputs are asserted indicating PHY is receiving LP11 in enabled datalanes and clocklanes;
    // #1000ns;
    // udelay(1);
    udelay(1);
    while((readl(g_csi_addr + 0x1000 + DWC_PHY_STOPSTATE) & 0x10000) != 0x10000)
    {
        // udelay(1);
        udelay(1);
        count++;
        if (count >1000000) {
            break;
        }
    }

    // udelay(1);
    udelay(1);
    // writel(0x00000051 | (0x14 << 25), g_csi_addr + 0x08);  //h9000_9010
}


static void k230_csi2_phy2_config(u32 freq)
{
    int count = 0;
    //1. set rstz = 1'b0
    writel(0x0, g_csi_addr + 0x1800 + DWC_DPHY_RSTZ);
    //2. set shutdownz=1'b0
    writel(0x0, g_csi_addr + 0x1800 + DWC_PHY_SHUTDOWNZ);
    //3. set testclr=1'b1
    writel(0x1, g_csi_addr + 0x1800 + DWC_PHY_TEST_CTRL0);
    //4. wait for 15ns
    // #15ns;
    // udelay(1);
    udelay(1);
    //5. set testclr =1'b0
    writel(0x0, g_csi_addr + 0x1800 + DWC_PHY_TEST_CTRL0);
    //6. set hsfreqrange[6:0]=7'b0001010
    //phy_reg_write(16'h2, 7'b0001010);
    writel(freq << 25, g_csi_addr + 0x10);                 //h9000_9010

    //7. config register 0xe5 to set counter_for_des_enbypass(bit0) to 1'b1
    k230_csi2_rxphy2_reg_write(0xe5, 0x1);
    //8. config 0xe4(bits7:4) to 2 (27M cflclk) for counter_for_des_en_config_if_fw
    k230_csi2_rxphy2_reg_write(0xe4, 0x20); //8'b00100000
    //9. config 0x1ab to set cb_sel_vref_lprx_fw(bits1:0) to 2'b10
    k230_csi2_rxphy2_reg_write(0x1ab, 0x06);  //8'b00000110
    //10. config 0x1ac to set rx_rxlp_bias_prog_rw(bit6) to 1'b1
    k230_csi2_rxphy2_reg_write(0x1ac, 0x4b);  // 8'b01001011
    //11. config 0x8 to set deskew_pol_rw signal (bit 5) to 1'b1
    k230_csi2_rxphy2_reg_write(0x08, 0x38);  //8'b00111000
    //12. config 0x307 to set rxclk_rxhs_pull_long_channel_if_rw signal(bit 7) to 1'b1
    k230_csi2_rxphy2_reg_write(0x307, 0x80);  //8'b10000000

    //13. config 0x607 to set rxclk_rxhs_ddl_tune_ovr_en_rw(bit 5) to 1'b1 and set rx0_rxhs_ddl_tune_ovr_rw[4:0] (bits 4:0) to 5'b11111
    k230_csi2_rxphy2_reg_write(0x607, 0x3f);  //8'b00111111
    //14. config 0x807 to set rx1_rxhs_ddl_tune_ovr_en_rw(bit 5) to 1'b1 and set rx1_rxhs_ddl_tune_ovr_rw[4:0](bits 4:0) to 5'b11111
    k230_csi2_rxphy2_reg_write(0x807, 0x3f);  //8'b00111111

    //15. set cfgclkfreqrange[5:0] = round[Fcfg_clk(MHz)-17)*4] = 6'b101000
    //16. Apply cfg_clk signal with the appropriate frequency with 27Mhz frequency
    writel(0x00000050 | (freq << 25), g_csi_addr + 0x10);  //h9000_9010  hsfreqrange_0 = 0x26

    //17. Set basedir_0 = 1'b1
    writel(0x00000051 | (freq << 25), g_csi_addr + 0x10);  //h9000_9010
    //18. Set forcerxm
    writel(0x00c00051 | (freq << 25), g_csi_addr + 0x10);  //h9000_9010
    //19. Set all requests inputs to zero;
    //20. Wait for 15 ns;
    // #15ns;
    // udelay(1);
    udelay(1);
    //21. Set enable_n and enableclk=1'b1;
    //tie 1 in maix3
    //22. Wait 5ns;
    // #5ns;
    //23. Set shutdownz=1'b1;
    writel(0x1, g_csi_addr + 0x1800 + DWC_PHY_SHUTDOWNZ);
    //24. Wait 5ns;
    // #5ns;
    // udelay(1);
    udelay(1);
    //25. Set rstz=1'b1;
    writel(0x1, g_csi_addr + 0x1800 + DWC_DPHY_RSTZ);
    //26. Wait until stopstatedata_n and stopstateclk outputs are asserted indicating PHY is receiving LP11 in enabled datalanes and clocklanes;
    // #1000ns;
    // udelay(1);
    udelay(1);
    while((readl(g_csi_addr + 0x1800 + DWC_PHY_STOPSTATE) & 0x10000) != 0x10000)
    {
        udelay(1);
        count++;
        if (count >1000000) {
            pr_err("wait stop timeout\n");
            break;
        }
    }
    // udelay(1);
    udelay(1);

    // writel(0x00000051 | (0x14 << 25), g_csi_addr + 0x10);  //h9000_9010
}


void k230_csi0_phy0_4lane_config(u32 freq)
{
    // config 4 lan
    writel(0x0, g_csi_addr + 0x18 ); //0x90009018
    //1. set rstz = 1'b0
    writel(0x0, g_csi_addr + 0x800 + DWC_DPHY_RSTZ);
    //2. set shutdownz=1'b0
    writel(0x0, g_csi_addr + 0x800 + DWC_PHY_SHUTDOWNZ);
    //3. set testclr=1'b1
    writel(0x1, g_csi_addr + 0x800 + DWC_PHY_TEST_CTRL0);
    //4. wait for 15ns
    // #15ns;
    udelay(1);
    //5. set testclr =1'b0
    writel(0x0, g_csi_addr + 0x800 + DWC_PHY_TEST_CTRL0);
    //6. set hsfreqrange[6:0]=7'b0001010
    //phy_reg_write(16'h2, 7'b0001010);
    
    // writel csi 4 lan
    writel(0x3, g_csi_addr + 0x800 + 0x4);

    // writel(0x26 << 25, 0x90009000);    //h9000_9010
    // writel(0x14 << 25, 0x90009000);
    writel(0x1 | (freq << 25), g_csi_addr + 0x0);

    //7. config register 0xe5 to set counter_for_des_enbypass(bit0) to 1'b1
    k230_csi0_rxphy0_reg_write(0xe5, 0x1);
    //8. config 0xe4(bits7:4) to 2 (27M cflclk) for counter_for_des_en_config_if_fw
    k230_csi0_rxphy0_reg_write(0xe4, 0x20); //8'b00100000
    //9. config 0x1ab to set cb_sel_vref_lprx_fw(bits1:0) to 2'b10
    k230_csi0_rxphy0_reg_write(0x1ab, 0x06);  //8'b00000110
    //10. config 0x1ac to set rx_rxlp_bias_prog_rw(bit6) to 1'b1
    k230_csi0_rxphy0_reg_write(0x1ac, 0x4b);  // 8'b01001011
    //11. config 0x8 to set deskew_pol_rw signal (bit 5) to 1'b1
    k230_csi0_rxphy0_reg_write(0x08, 0x38);  //8'b00111000
    //12. config 0x307 to set rxclk_rxhs_pull_long_channel_if_rw signal(bit 7) to 1'b1
    //k230_csi0_rxphy0_reg_write(0x307, 0x80);  //8'b10000000   //del by wangchao @20230215

    //13. config 0x607 to set rxclk_rxhs_ddl_tune_ovr_en_rw(bit 5) to 1'b1 and set rx0_rxhs_ddl_tune_ovr_rw[4:0] (bits 4:0) to 5'b11111
    k230_csi0_rxphy0_reg_write(0x607, 0x3f);  //8'b00111111
    //14. config 0x807 to set rx1_rxhs_ddl_tune_ovr_en_rw(bit 5) to 1'b1 and set rx1_rxhs_ddl_tune_ovr_rw[4:0](bits 4:0) to 5'b11111
    k230_csi0_rxphy0_reg_write(0x807, 0x3f);  //8'b00111111

    //15. set cfgclkfreqrange[5:0] = round[Fcfg_clk(MHz)-17)*4] = 6'b101000
    //16. Apply cfg_clk signal with the appropriate frequency with 27Mhz frequency

    // writel(0x00000050 | (0x26 << 25), 0x90009000
    // writel(0x00000050 | (0x14 << 25), 0x90009000);  //h9000_9010  hsfreqrange_0 = 0x26

    //17. Set basedir_0 = 1'b1
    // writel(0x00000051 | (0x26 << 25), 0x90009000);  //h9000_9010
    writel(0x00000051 | (freq << 25), g_csi_addr + 0x0);  //h9000_9010
    //18. Set forcerxm
    writel(0x00c00051 | (freq << 25), g_csi_addr + 0x0);  //h9000_9010
    // writel(0x00c00051 | (0x26 << 25), 0x90009000);  //h9000_9010
    //19. Set all requests inputs to zero;
    //20. Wait for 15 ns;
    // #15ns;
    udelay(1);
    //21. Set enable_n and enableclk=1'b1;
    //tie 1 in maix3
    writel(0x1, g_csi_addr + 0xb0);  
    udelay(1);                                                                   
}

void k230_csi0_phy1_4lane_config(u32 freq)
{
    //1. set rstz = 1'b0
    writel(0x0, g_csi_addr + 0x1000 + DWC_DPHY_RSTZ);
    //2. set shutdownz=1'b0
    writel(0x0, g_csi_addr + 0x1000 + DWC_PHY_SHUTDOWNZ);
    //3. set testclr=1'b1
    writel(0x1, g_csi_addr + 0x1000 + DWC_PHY_TEST_CTRL0);
    //4. wait for 15ns
    // #15ns;
    udelay(1);
    //5. set testclr =1'b0
    writel(0x0, g_csi_addr + 0x1000 + DWC_PHY_TEST_CTRL0);
    //6. set hsfreqrange[6:0]=7'b0001010
    //phy_reg_write(16'h2, 7'b0001010);

    // writel(0x26 << 25, 0x90009000);    //h9000_9010
    // writel(0x14 << 25, 0x90009008);
     writel(0x1 | (freq << 25), g_csi_addr + 0x8);

    //7. config register 0xe5 to set counter_for_des_enbypass(bit0) to 1'b1
    k230_csi0_rxphy1_reg_write(0xe5, 0x1);
    //8. config 0xe4(bits7:4) to 2 (27M cflclk) for counter_for_des_en_config_if_fw
    k230_csi0_rxphy1_reg_write(0xe4, 0x20); //8'b00100000
    //9. config 0x1ab to set cb_sel_vref_lprx_fw(bits1:0) to 2'b10
    k230_csi0_rxphy1_reg_write(0x1ab, 0x06);  //8'b00000110
    //10. config 0x1ac to set rx_rxlp_bias_prog_rw(bit6) to 1'b1
    k230_csi0_rxphy1_reg_write(0x1ac, 0x4b);  // 8'b01001011
    //11. config 0x8 to set deskew_pol_rw signal (bit 5) to 1'b1
    k230_csi0_rxphy1_reg_write(0x08, 0x38);  //8'b00111000
    //12. config 0x307 to set rxclk_rxhs_pull_long_channel_if_rw signal(bit 7) to 1'b1
    //k230_csi0_rxphy1_reg_write(0x307, 0x80);  //8'b10000000   //del by wangchao @20230215

    //13. config 0x607 to set rxclk_rxhs_ddl_tune_ovr_en_rw(bit 5) to 1'b1 and set rx0_rxhs_ddl_tune_ovr_rw[4:0] (bits 4:0) to 5'b11111
    k230_csi0_rxphy1_reg_write(0x607, 0x3f);  //8'b00111111
    //14. config 0x807 to set rx1_rxhs_ddl_tune_ovr_en_rw(bit 5) to 1'b1 and set rx1_rxhs_ddl_tune_ovr_rw[4:0](bits 4:0) to 5'b11111
    k230_csi0_rxphy1_reg_write(0x807, 0x3f);  //8'b00111111

    //15. set cfgclkfreqrange[5:0] = round[Fcfg_clk(MHz)-17)*4] = 6'b101000
    //16. Apply cfg_clk signal with the appropriate frequency with 27Mhz frequency

    // writel(0x00000050 | (0x26 << 25), 0x90009000);  //h9000_9010  hsfreqrange_0 = 0x26

    // writel(0x00000050 | (0x14 << 25), 0x90009008);  //h9000_9010  hsfreqrange_0 = 0x26

    //17. Set basedir_0 = 1'b1
    // writel(0x00000051 | (0x26 << 25), 0x90009000);  //h9000_9010
    writel(0x00000051 | (freq << 25), g_csi_addr + 0x8);  //h9000_9010
    //18. Set forcerxm
    writel(0x00c00051 | (freq << 25), g_csi_addr + 0x8);  //h9000_9010
    // writel(0x00c00051 | (0x26 << 25), 0x90009000);  //h9000_9010
    //19. Set all requests inputs to zero;
    //20. Wait for 15 ns;
    udelay(1);

    // writel(0x00000051 | (0x14 << 25), 0x90009000);  //h9000_9010
}

void k230_phy0_master_phy1_slave_config(void)
{
    int count = 0;
    //Set bit [0] (mastermacro -> set as master PHY) of register 0x133 of the master PHY to 1?��b1
    k230_csi0_rxphy0_reg_write(0x133, 0x1);
    //Set bit [0] (mastermacro -> set as slave PHY) of register 0x133 of the slave PHY to 1?��b0
    k230_csi0_rxphy1_reg_write(0x133, 0x0);
    //Set bit [2](rxclk_rxhs_clk_to_long_channel_if -> enable the hs clock to long channel) of register 0x307 of the master PHY to 1?��b1
    k230_csi0_rxphy0_reg_write(0x307, 0x4);
    //Set bit [2] (rxclk_rxhs_clk_to_long_channel_if -> disable the hs clock to long channel) of register 0x307 of the slave PHY to 1?��b0
    k230_csi0_rxphy1_reg_write(0x307, 0x0);
    //Set bit [5](rxhs_clk_from_long_channel_if_rw -> enable the hs clock to long channel of lane0) of register 0x508 of the master/slave PHYs to 1?��b1
    k230_csi0_rxphy0_reg_write(0x508, 0x20);
    k230_csi0_rxphy1_reg_write(0x508, 0x20);
    //Set bit [5] (rxhs_clk_from_long_channel_if_rw of lane1 -> enable the hs clock to long channel of lane0) of register 0x708 of the master/slave PHYs to 1?��b1
    k230_csi0_rxphy0_reg_write(0x708, 0x20);
    k230_csi0_rxphy1_reg_write(0x708, 0x20);
    //Set bit [3](rxclk_rxhs_ddr_clk_sel_if -> disable the DDR clock source) of register 0x308 of the master PHY to 1?��b0 ( use short channel to generate DDR)
    k230_csi0_rxphy0_reg_write(0x308, 0x0);
    //Set bit [3](rxclk_rxhs_ddr_clk_sel_if -> enable the DDR clock source) of register 0x308 of the slave PHY to 1?��b1 ( use long channel to generate DDR)
    k230_csi0_rxphy1_reg_write(0x308, 0x8);
    //Set bits [1:0](clk_en_lanes_tester,clk_en_lanes_bypass -> enable the clock for tester and bypass the clock lane) of register 0x0E0 of the slave PHY to 2?��b11
    k230_csi0_rxphy1_reg_write(0xe0, 0x3);
    //Set bit [1]( rxclk_rxhs_ddr_clk_en_bypass ��C> enable DDR clock bypass) of register 0x0E1 of the slave PHY to 1?��b1
    k230_csi0_rxphy1_reg_write(0xe1, 0x2);
    //Set bit [3]( rxclk_rxhs_ddr_cl(rxclk_rxhs_ddr_clk_sel_if -> disable the DDR clock source) of register 0x308 of the master PHY to 1?��b0 ( use short channel to generate DDR)
    k230_csi0_rxphy0_reg_write(0x308, 0x0);
    //Set bit [3](rxclk_rxhs_ddr_clk_sel_if -> enable the DDR clock source) of register 0x308 of the slave PHY to 1?��b1 ( use long channel to generate DDR)
    k230_csi0_rxphy1_reg_write(0x308, 0x8);
    //Set bits [1:0](clk_en_lanes_tester,clk_en_lanes_bypass -> enable the clock for tester and bypass the clock lane) of register 0x0E0 of the slave PHY to 2?��b11
    k230_csi0_rxphy1_reg_write(0xe0, 0x3);
    //Set bit [1]( rxclk_rxhs_ddr_clk_en_bypass ��C> enable DDR clock bypass) of register 0x0E1 of the slave PHY to 1?��b1
    k230_csi0_rxphy1_reg_write(0xe1, 0x2);
    //Set bit [3]( rxclk_rxhs_ddr_clk_en ��C> enable the HS DDR clock) of register 0x307 of the slave PHY to 1?��b1 (this configuration can be done when setting slave PHY clock lane to don?��t drive long channel clock)
    k230_csi0_rxphy1_reg_write(0x307, 0x8);
    //Set bit [7]( lprxponcd_bypass_clklane -> enable the LP-CD bypass) of register 0x304 of the slave PHY to 1?��b1
    k230_csi0_rxphy1_reg_write(0x304, 0x80);
    k230_csi0_rxphy1_reg_write(0x305, 0xa);
    //#5ns;
    udelay(1);
    //Set shutdownz=1'b1;
    writel(0x1, g_csi_addr + 0x800 + DWC_PHY_SHUTDOWNZ);
    //#5ns;
    udelay(1);
    //Set rstz=1'b1;
    writel(0x1, g_csi_addr + 0x800 + DWC_DPHY_RSTZ);

    //Wait until stopstatedata_n and stopstateclk outputs are asserted indicating PHY is receiving LP11 in enabled datalanes and clocklanes;
    udelay(1);
    while((readl(g_csi_addr + 0x800 + DWC_PHY_STOPSTATE) & 0x10003) != 0x10003)
    {
        // udelay(1);
        udelay(1);
        count++;
        if (count >1000000) {
            pr_err("4lan wait stop timeout\n");
            break;
        }
    }
    udelay(1);
    //#5ns;
    udelay(1);
    writel(0x400, g_csi_addr + 0x18);//0x90009018);
    //Wait until stopstatedata_n and stopstateclk outputs are asserted indicating PHY is receiving LP11 in enabled datalanes and data lan2 3 ;
    udelay(1);
    while((readl(g_csi_addr + 0x800 + DWC_PHY_STOPSTATE) & 0xc) != 0xc)
    {
        // udelay(1);
        udelay(1);
        count++;
        if (count >1000000) {
            pr_err("4lan wait stop timeout\n");
            break;
        }
    }
    udelay(1);
}

static void k230_csi0_config_4lan_phy(u32 freq)
{
    k230_csi0_phy0_4lane_config(freq);
    k230_csi0_phy1_4lane_config(freq);
    k230_phy0_master_phy1_slave_config();
}

int dwc_csi_phy_init(enum csi_num csi, enum mipi_phy_freq freq, enum mipi_lanes lan_num)
{
    u32 phy_freq;

    if (csi > CSI2) {
        pr_err("%s invalid param csi %d.\n", __func__, csi);
        return -EINVAL;
    }
    // enable clk
    writel(0b111, g_csi_addr + 0xb0);

    switch(freq)
    {
        case MIPI_PHY_800M :
            //795.625 - 905.625    0011001 = 0x19
            phy_freq = 0x19;
            break;
        case MIPI_PHY_1200M :
            //1128.125 - 1273.125   0001011 = 0x0b
            phy_freq = 0x0b;
            break;
        case MIPI_PHY_1600M :
            // 1508.125 - 1693.125   0001101 = 0x0d
            phy_freq = 0x0d;
            break;
        default :
            pr_err("%s invalid param phy_freq %d.\n", __func__, phy_freq);
            return -EINVAL;
    }

    //set phy attr
    switch(csi)
    {
        case CSI0 :
            if(lan_num == MIPI_4LANE)
            {
                k230_csi0_config_4lan_phy(phy_freq);
            }
            else
            {
                k230_csi0_phy0_config(phy_freq);
            }
            break;

        case CSI1 :
            k230_csi1_phy1_config(phy_freq);
            break;

        case CSI2 :
            k230_csi2_phy2_config(phy_freq);
            break;
        default :
            break;
    }

    return 0;
}


int csi_device_init(void)
{
    if (g_csi_addr != NULL) {
        iounmap(g_csi_addr);
        g_csi_addr = NULL;
    }
    g_csi_addr = ioremap(CSI_REG_BASE_ADDR, CSI_REG_MEM_SIZE);
    if (g_csi_addr == NULL) {
        pr_err("%s ioremap error\n", __func__);
        return -EINVAL;
    }
    return 0;
}

