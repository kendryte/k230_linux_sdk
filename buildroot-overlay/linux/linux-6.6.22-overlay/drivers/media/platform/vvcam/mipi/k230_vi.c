/* Copyright (c) 2023, Canaan Bright Sight Co., Ltd
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

#include "k230_vi.h"
#include <asm/mmio.h>
#include <asm/io.h>
#include <linux/delay.h>

static u8 *g_vi_addr = NULL;

static void dwc_rst_csi(enum csi_num csi_num)
{
    writel(0x01, (g_vi_addr + (csi_num * 0x800))  + DWC_CSI2_RESETN);
}

static void dwc_set_lan_num(enum csi_num csi_num, u32 lan_num, u32 freq)
{
    u32 phy_freq[] = {0, 0x14 ,0x0b, 0x0d};

    dwc_rst_csi(csi_num);

    if(lan_num == MIPI_4LANE)
    {
        writel(0x00000051 | (phy_freq[freq] << 25), g_vi_addr + ((csi_num - 1) * 0x8));  //h9000_9010
        writel(0x00000051 | (phy_freq[freq] << 25), g_vi_addr + (csi_num * 0x8));  //h9000_9010
    }
    else
        writel(0x00000051 | (phy_freq[freq] << 25), g_vi_addr + ((csi_num - 1) * 0x8));  //h9000_9010

    writel(lan_num, (g_vi_addr + (csi_num * 0x800))  + DWC_LANES_NUM);
}

static int kd_vi_set_ipi_attr(enum csi_num csi, enum vi_ipi ipi, struct vi_ipi_attr *attr)
{
    u32 reg = 0;

    if ((csi > CSI2) || (!attr) || (ipi > IPI3)) {
        printk("%s, invalid param, csi(%d), ipi(%d)\n", __func__, csi, ipi);
        return -1;
    }

    switch(ipi)
    {
        case IPI1 :
            //set hsa
            writel(0x0, (g_vi_addr + (csi * 0x800)) + DWC_IPI_VCID);

            writel(attr->hsa, (g_vi_addr + (csi * 0x800)) + DWC_IPI_HSA_TIME);
            //set hbp
            writel(attr->hbp, (g_vi_addr + (csi * 0x800)) + DWC_IPI_HBP_TIME);
            //set csi sync
            if(attr->is_csi_sync_event == 1)
                writel(0x1030000, (g_vi_addr + (csi * 0x800)) + 0xac);
            // set work mode
            reg = readl((g_vi_addr + (csi * 0x800)) + DWC_IPI_MODE);
            reg = (reg & ~(BIT_MASK(0))) | (0 << 0);
            writel(reg, (g_vi_addr + (csi * 0x800)) + DWC_IPI_MODE);
            // set data type
            writel(attr->data_type, (g_vi_addr + (csi * 0x800)) + DWC_IPI_DATA_TYPE);
            // set cut through
            reg = readl((g_vi_addr + (csi * 0x800)) + DWC_IPI_MODE);
            reg = (reg & ~(BIT_MASK(16))) | (0x1 << 16);
            writel(reg, (g_vi_addr + (csi * 0x800)) + DWC_IPI_MODE);
            // set interface = 16
            reg = readl((g_vi_addr + (csi * 0x800)) + DWC_IPI_MODE);
            reg = (reg & ~(BIT_MASK(8))) | (0x1 << 8);
            writel(reg, (g_vi_addr + (csi * 0x800)) + DWC_IPI_MODE);
            //enable ipi
            reg = readl((g_vi_addr + (csi * 0x800)) + DWC_IPI_MODE);
            reg = (reg & ~(BIT_MASK(24))) | (0x1 << 24);
            writel(reg, (g_vi_addr + (csi * 0x800)) + DWC_IPI_MODE);
            break;

        case IPI2 :
            writel(0x1, (g_vi_addr + (csi * 0x800)) + DWC_IPI2_VCID);

            //set hsa
            writel(attr->hsa, (g_vi_addr + (csi * 0x800)) + DWC_IPI2_HSA_TIME);
            //set hbp
            writel(attr->hbp, (g_vi_addr + (csi * 0x800)) + DWC_IPI2_HBP_TIME);
            //set csi sync
            if(attr->is_csi_sync_event == 1)
                writel(0x1030000, (g_vi_addr + (csi * 0x800)) + 0x21c);
            // set work mode
            reg = readl((g_vi_addr + (csi * 0x800)) + DWC_IPI2_MODE);
            reg = (reg & ~(BIT_MASK(0))) | (0 << 0);
            writel(reg, (g_vi_addr + (csi * 0x800)) + DWC_IPI2_MODE);
            // set data type
            writel(attr->data_type, (g_vi_addr + (csi * 0x800)) + DWC_IPI2_DATA_TYPE);
            // set cut through
            reg = readl((g_vi_addr + (csi * 0x800)) + DWC_IPI2_MODE);
            reg = (reg & ~(BIT_MASK(16))) | (0x1 << 16);
            writel(reg, (g_vi_addr + (csi * 0x800)) + DWC_IPI2_MODE);
            // set interface = 16
            reg = readl((g_vi_addr + (csi * 0x800)) + DWC_IPI2_MODE);
            reg = (reg & ~(BIT_MASK(8))) | (0x1 << 8);
            writel(reg, (g_vi_addr + (csi * 0x800)) + DWC_IPI2_MODE);
            //enable ipi
            reg = readl((g_vi_addr + (csi * 0x800)) + DWC_IPI2_MODE);
            reg = (reg & ~(BIT_MASK(24))) | (0x1 << 24);
            writel(reg, (g_vi_addr + (csi * 0x800)) + DWC_IPI2_MODE);

            break;

        case IPI3 :
            writel(0x2, (g_vi_addr + (csi * 0x800))  + DWC_IPI3_VCID);

            //set hsa
            writel(attr->hsa, (g_vi_addr + (csi * 0x800)) + DWC_IPI3_HSA_TIME);
            //set hbp
            writel(attr->hbp, (g_vi_addr + (csi * 0x800)) + DWC_IPI3_HBP_TIME);
            //set csi sync
            if(attr->is_csi_sync_event == 1)
                writel(0x1030000, (g_vi_addr + (csi * 0x800)) + 0x23c);
            // set work mode
            reg = readl((g_vi_addr + (csi * 0x800)) + DWC_IPI3_MODE);
            reg = (reg & ~(BIT_MASK(0))) | (0 << 0);
            writel(reg, (g_vi_addr + (csi * 0x800)) + DWC_IPI3_MODE);
            // set data type
            writel(attr->data_type, (g_vi_addr + (csi * 0x800)) + DWC_IPI3_DATA_TYPE);
            // set cut through
            reg = readl((g_vi_addr + (csi * 0x800)) + DWC_IPI3_MODE);
            reg = (reg & ~(BIT_MASK(16))) | (0x1 << 16);
            writel(reg, (g_vi_addr + (csi * 0x800)) + DWC_IPI3_MODE);
            // set interface = 16
            reg = readl((g_vi_addr + (csi * 0x800)) + DWC_IPI3_MODE);
            reg = (reg & ~(BIT_MASK(8))) | (0x1 << 8);
            writel(reg, (g_vi_addr + (csi * 0x800)) + DWC_IPI3_MODE);
            //enable ipi
            reg = readl((g_vi_addr + (csi * 0x800)) + DWC_IPI3_MODE);
            reg = (reg & ~(BIT_MASK(24))) | (0x1 << 24);
            writel(reg, (g_vi_addr + (csi * 0x800)) + DWC_IPI3_MODE);
            break;

        default :
            break;
    }

    return 0;
}

int kd_vi_set_config(struct vi_attr *attr)
{
    struct vi_ipi_attr ipi;

    if(!attr) {
        printk("%s attr is null\n", __func__);
        return -1;
    }

    // set lan num
    dwc_set_lan_num(attr->csi_num, attr->mipi_lanes, attr->phy_freq);
    // set csi
    switch(attr->hdr_mode)
    {
        case LINERA_MODE:
            //set ipi csi
            ipi.work_mode = CSI_CAMERA_MODE;
            ipi.data_type = attr->data_type;
            ipi.is_csi_sync_event = 1;
            ipi.hsa = 10;
            ipi.hbp = 10;
            kd_vi_set_ipi_attr(CSI0, IPI1, &ipi);
            break;

        case VCID_HDR_2FRAME:
            //set ipi csi
            ipi.work_mode = CSI_CAMERA_MODE;
            ipi.data_type = attr->data_type;
            ipi.is_csi_sync_event = 1;
            ipi.hsa = 10;
            ipi.hbp = 10;

            kd_vi_set_ipi_attr(CSI0, IPI1, &ipi);
            kd_vi_set_ipi_attr(CSI0, IPI2, &ipi);
            break;

        case VCID_HDR_3FRAME:
            //set ipi csi
            ipi.work_mode = CSI_CAMERA_MODE;
            ipi.data_type = attr->data_type;
            ipi.is_csi_sync_event = 1;
            ipi.hsa = 10;
            ipi.hbp = 10;

            kd_vi_set_ipi_attr(CSI0, IPI1, &ipi);
            kd_vi_set_ipi_attr(CSI0, IPI2, &ipi);
            kd_vi_set_ipi_attr(CSI0, IPI3, &ipi);
            break;

        case SONY_HDR_3FRAME:
            break;

        case SONY_HDR_2FRAME:
            break;
    }

    return 0;
}

void kd_vi_3d_mode_crtl(bool enable)
{
    u32 reg = 0;
    reg = readl(g_vi_addr + VI_3D_MODE_LIGHT_CTL);
    reg = (reg & ~(BIT_MASK(31))) | (enable << 31);
    writel(reg, g_vi_addr + VI_3D_MODE_LIGHT_CTL);
    
}

int vi_device_init(void)
{
    g_vi_addr = ioremap(VI_REG_BASE_ADDR, VI_REG_MEM_SIZE);
    if (!g_vi_addr) {
        printk("vi_init g_vi_addr ioremap err.\n");
        return -1;
    }
    return 0;
}

