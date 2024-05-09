/****************************************************************************
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2023 VeriSilicon Holdings Co., Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 *****************************************************************************
 *
 * The GPL License (GPL)
 *
 * Copyright (c) 2023 VeriSilicon Holdings Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program;
 *
 *****************************************************************************
 *
 * Note: This software is released under dual MIT and GPL licenses. A
 * recipient may use this file under the terms of either the MIT license or
 * GPL License. If you wish to use only one license not the other, you can
 * indicate your decision by deleting one of the above license notices in your
 * version of this file.
 *
 *****************************************************************************/

#include <linux/io.h>
#include <linux/spinlock.h>
#include "vvcam_isp_driver.h"
#include "vvcam_isp_hal.h"

extern void vvcam_isp_proc_stat(unsigned long pde,
                    const uint32_t *irq_mis, const int len);

void vvcam_isp_irq_stat_tasklet(unsigned long dev)
{
    struct vvcam_isp_dev *isp_dev = (struct vvcam_isp_dev *)dev;
    uint32_t irq_mis[VVCAM_EID_MAX];

    memset(irq_mis, 0, sizeof(irq_mis));

    memcpy(irq_mis, isp_dev->irq_mis, sizeof(irq_mis));
    memset(isp_dev->irq_mis, 0, sizeof(isp_dev->irq_mis));

    vvcam_isp_proc_stat(isp_dev->pde, irq_mis, VVCAM_EID_MAX);
}

static void vvcam_isp_hal_write_reg(void __iomem *base,
			uint32_t addr, uint32_t value)
{
    writel(value, base + addr);
}

static int vvcam_isp_hal_read_reg(void __iomem *base, uint32_t addr)
{
    return readl(base + addr);
}

int vvcam_isp_reset(struct vvcam_isp_dev *isp_dev, uint32_t reset)
{
    if (IS_ERR(isp_dev->reset)) {
        // ???
        return -1;
    }
    return reset_control_reset(isp_dev->reset);
}

int vvcam_isp_write_reg(struct vvcam_isp_dev *isp_dev, vvcam_isp_reg_t isp_reg)
{
    vvcam_isp_hal_write_reg(isp_dev->base, isp_reg.addr, isp_reg.value);
    return 0;
}

int vvcam_isp_read_reg(struct vvcam_isp_dev *isp_dev, vvcam_isp_reg_t *isp_reg)
{
    uint32_t reg_value = 0;

    reg_value = vvcam_isp_hal_read_reg(isp_dev->base, isp_reg->addr);

    isp_reg->value = reg_value;

    return 0;
}


irqreturn_t vvcam_isp_irq_process(struct vvcam_isp_dev *isp_dev)
{
    uint32_t isp_mis = 0;
    uint32_t isp_fe_ctrl = 0;
    uint64_t timestamp;

    vvcam_event_t event;

    if (!isp_dev->refcnt)
        return IRQ_NONE;

    isp_fe_ctrl = vvcam_isp_hal_read_reg(isp_dev->base, ISP_FE_CTL);
    if ((isp_fe_ctrl & ISP_FE_CFG_SEL_MASK) == ISP_FE_SEL_CMDBUF) {
        return IRQ_HANDLED;
    }

    isp_mis = vvcam_isp_hal_read_reg(isp_dev->base, ISP_MIS);
    if (isp_mis) {
        // printk("isp_mis mis 0x%08x\n", isp_mis);
        vvcam_isp_hal_write_reg(isp_dev->base, ISP_ICR, isp_mis);
    } else {
        return IRQ_NONE;
    }

    timestamp = ktime_get_ns();

    if (isp_mis) {
        event.type = VVCAM_EVENT_IRQ_TYPE;
        event.id   = VVCAM_EID_ISP_MIS;
        event.timestamp = timestamp;
        event.irqevent.irq_value = isp_mis;
        vvcam_event_queue(&isp_dev->event_dev, &event);
        isp_dev->irq_mis[VVCAM_EID_ISP_MIS] = isp_mis;
        tasklet_schedule(&isp_dev->stat_tasklet);
    }

    return IRQ_HANDLED;
}

irqreturn_t vvcam_isp_mi_irq_process(struct vvcam_isp_dev *isp_dev)
{
    uint32_t miv2_mis  = 0;
    uint32_t miv2_mis1 = 0;
    uint32_t miv2_mis2 = 0;
    uint32_t miv2_mis3 = 0;
    uint32_t mi_mis_hdr1 = 0;
    uint32_t isp_fe_ctrl = 0;
    uint64_t timestamp;

    vvcam_event_t event;
    bool irq = false;

    if (!isp_dev->refcnt)
        return IRQ_NONE;

    isp_fe_ctrl = vvcam_isp_hal_read_reg(isp_dev->base, ISP_FE_CTL);
    if ((isp_fe_ctrl & ISP_FE_CFG_SEL_MASK) == ISP_FE_SEL_CMDBUF) {
        return IRQ_HANDLED;
    }

    miv2_mis = vvcam_isp_hal_read_reg(isp_dev->base, MIV2_MIS);
    if (miv2_mis) {
        //printk("miv2 mis 0x%08x\n", miv2_mis);
        vvcam_isp_hal_write_reg(isp_dev->base, MIV2_ICR, miv2_mis);
        irq = true;
    }

    miv2_mis1 = vvcam_isp_hal_read_reg(isp_dev->base, MIV2_MIS1);
    if (miv2_mis1) {
        vvcam_isp_hal_write_reg(isp_dev->base, MIV2_ICR1, miv2_mis1);
        irq = true;
    }

    miv2_mis2 = vvcam_isp_hal_read_reg(isp_dev->base, MIV2_MIS2);
    if (miv2_mis2) {
        vvcam_isp_hal_write_reg(isp_dev->base, MIV2_ICR2, miv2_mis2);
        irq = true;
    }

    miv2_mis3 = vvcam_isp_hal_read_reg(isp_dev->base, MIV2_MIS3);
    if (miv2_mis3) {
        vvcam_isp_hal_write_reg(isp_dev->base, MIV2_ICR3, miv2_mis3);
        irq = true;
    }

    mi_mis_hdr1 = vvcam_isp_hal_read_reg(isp_dev->base, MI_MIS_HDR1);
    if (mi_mis_hdr1) {
        vvcam_isp_hal_write_reg(isp_dev->base, MI_ICR_HDR1, mi_mis_hdr1);
        irq = true;
    }

    timestamp = ktime_get_ns();

    if (miv2_mis & (MIV2_MIS_MCM_RAW_RADY_MASK | MIV2_MIS_FRAME_END_MASK)) {
        event.type = VVCAM_EVENT_IRQ_TYPE;
        event.id   = VVCAM_EID_RDMA_MIS;
        event.timestamp = timestamp;
        event.irqevent.irq_value = miv2_mis &
            (MIV2_MIS_MCM_RAW_RADY_MASK | MIV2_MIS_FRAME_END_MASK);
        miv2_mis &= ~MIV2_MIS_MCM_RAW_RADY_MASK;
        vvcam_event_queue(&isp_dev->event_dev, &event);
        isp_dev->irq_mis[VVCAM_EID_RDMA_MIS] = event.irqevent.irq_value;
    }

    if (miv2_mis & MIV2_MIS_JPD_FRAME_END_MASK) {
        event.type = VVCAM_EVENT_IRQ_TYPE;
        event.id   = VVCAM_EID_MIV2_JDP;
        event.timestamp = timestamp;
        event.irqevent.irq_value = miv2_mis & MIV2_MIS_JPD_FRAME_END_MASK;
        miv2_mis &= ~MIV2_MIS_JPD_FRAME_END_MASK;
        vvcam_event_queue(&isp_dev->event_dev, &event);
        isp_dev->irq_mis[VVCAM_EID_MIV2_JDP] = event.irqevent.irq_value;
    }

    if (miv2_mis & MIV2_MIS_MCM_RAW0_FRAME_END_MASK) {
        event.type = VVCAM_EVENT_IRQ_TYPE;
        event.id   = VVCAM_EID_MCM_WR_RAW0_MIS;
        event.timestamp = timestamp;
        event.irqevent.irq_value = miv2_mis & MIV2_MIS_MCM_RAW0_FRAME_END_MASK;
        miv2_mis &= ~MIV2_MIS_MCM_RAW0_FRAME_END_MASK;
        vvcam_event_queue(&isp_dev->event_dev, &event);
        isp_dev->irq_mis[VVCAM_EID_MCM_WR_RAW0_MIS] =  event.irqevent.irq_value;
    }

    if (miv2_mis & MIV2_MIS_MCM_RAW1_FRAME_END_MASK) {
        event.type = VVCAM_EVENT_IRQ_TYPE;
        event.id   = VVCAM_EID_MCM_WR_RAW1_MIS;
        event.timestamp = timestamp;
        event.irqevent.irq_value = miv2_mis & MIV2_MIS_MCM_RAW1_FRAME_END_MASK;
        miv2_mis &= ~MIV2_MIS_MCM_RAW1_FRAME_END_MASK;
        vvcam_event_queue(&isp_dev->event_dev, &event);
        isp_dev->irq_mis[VVCAM_EID_MCM_WR_RAW1_MIS] = event.irqevent.irq_value;
    }


    if (miv2_mis3 & MIV2_MIS3_MCM_G2RAW0_FRAME_END_MASK) {
        event.type = VVCAM_EVENT_IRQ_TYPE;
        event.id   = VVCAM_EID_MCM_WR_G2_RAW0_MIS;
        event.timestamp = timestamp;
        event.irqevent.irq_value = miv2_mis3 & MIV2_MIS3_MCM_G2RAW0_FRAME_END_MASK;
        miv2_mis3 &=  ~MIV2_MIS3_MCM_G2RAW0_FRAME_END_MASK;
        vvcam_event_queue(&isp_dev->event_dev, &event);
        isp_dev->irq_mis[VVCAM_EID_MCM_WR_G2_RAW0_MIS] = event.irqevent.irq_value;
    }

    if (miv2_mis3 & MIV2_MIS3_MCM_G2RAW1_FRAME_END_MASK) {
        event.type = VVCAM_EVENT_IRQ_TYPE;
        event.id   = VVCAM_EID_MCM_WR_G2_RAW1_MIS;
        event.timestamp = timestamp;
        event.irqevent.irq_value = miv2_mis3 & MIV2_MIS3_MCM_G2RAW1_FRAME_END_MASK;
        miv2_mis3 &=  ~MIV2_MIS3_MCM_G2RAW1_FRAME_END_MASK;
        vvcam_event_queue(&isp_dev->event_dev, &event);
        isp_dev->irq_mis[VVCAM_EID_MCM_WR_G2_RAW1_MIS] = event.irqevent.irq_value;
    }

    if (miv2_mis) {
        event.type = VVCAM_EVENT_IRQ_TYPE;
        event.id   = VVCAM_EID_MIV2_MIS;
        event.timestamp = timestamp;
        event.irqevent.irq_value = miv2_mis;
        vvcam_event_queue(&isp_dev->event_dev, &event);
    }

    if (miv2_mis1) {
        event.type = VVCAM_EVENT_IRQ_TYPE;
        event.id   = VVCAM_EID_MIV2_MIS1;
        event.timestamp = timestamp;
        event.irqevent.irq_value = miv2_mis1;
        vvcam_event_queue(&isp_dev->event_dev, &event);
    }

    if ((miv2_mis2 & MIV2_MIS2_HDR_RDMA_READY_MASK)
       || (miv2_mis & MIV2_MIS_FRAME_END_MASK)) {
        event.type = VVCAM_EVENT_IRQ_TYPE;
        event.id   = VVCAM_EID_MIV2_MIS2_HDR;
        event.timestamp = timestamp;
        event.irqevent.irq_value = (miv2_mis2 & MIV2_MIS2_HDR_RDMA_READY_MASK)
                                   | (miv2_mis & MIV2_MIS_FRAME_END_MASK);
        miv2_mis2 &= ~MIV2_MIS2_HDR_RDMA_READY_MASK;
        vvcam_event_queue(&isp_dev->event_dev, &event);
    }

    if (miv2_mis2) {
        event.type = VVCAM_EVENT_IRQ_TYPE;
        event.id   = VVCAM_EID_MIV2_MIS2;
        event.timestamp = timestamp;
        event.irqevent.irq_value = miv2_mis2;
        vvcam_event_queue(&isp_dev->event_dev, &event);
    }

    if (miv2_mis3) {
        event.type = VVCAM_EVENT_IRQ_TYPE;
        event.id   = VVCAM_EID_MIV2_MIS3;
        event.timestamp = timestamp;
        event.irqevent.irq_value = miv2_mis3;
        vvcam_event_queue(&isp_dev->event_dev, &event);
    }

    if (mi_mis_hdr1) {
        event.type = VVCAM_EVENT_IRQ_TYPE;
        event.id   = VVCAM_EID_MI_MIS_HDR1;
        event.timestamp = timestamp;
        event.irqevent.irq_value = mi_mis_hdr1;
        vvcam_event_queue(&isp_dev->event_dev, &event);
    }

    if (irq) {
        tasklet_schedule(&isp_dev->stat_tasklet);
        return IRQ_HANDLED;
    }

    return IRQ_NONE;
}

irqreturn_t vvcam_isp_fe_irq_process(struct vvcam_isp_dev *isp_dev)
{
    uint32_t isp_fe_mis = 0;
    uint32_t isp_fe_ctrl = 0;
    vvcam_event_t event;
    uint64_t timestamp;

    if (!isp_dev->refcnt)
        return IRQ_NONE;

    isp_fe_mis = vvcam_isp_hal_read_reg(isp_dev->base, ISP_FE_MIS);
    if (isp_fe_mis) {
        vvcam_isp_hal_write_reg(isp_dev->base, ISP_FE_ICR, isp_fe_mis);
    } else {
        return IRQ_NONE;
    }

    isp_fe_ctrl = vvcam_isp_hal_read_reg(isp_dev->base, ISP_FE_CTL);
    if ((isp_fe_ctrl & ISP_FE_CFG_SEL_MASK) == ISP_FE_SEL_CMDBUF) {
        isp_fe_ctrl &= ~(ISP_FE_CFG_SEL_MASK | ISP_FE_AHB_WRITE_MASK);
        isp_fe_ctrl |= (ISP_FE_SEL_AHBBUF) << ISP_FE_CFG_SEL_SHIFT;
        isp_fe_ctrl |= (ISP_FE_AHB_WR_ENABLE) << ISP_FE_AHB_WRITE_SHIFT;
        vvcam_isp_hal_write_reg(isp_dev->base, ISP_FE_CTL, isp_fe_ctrl);

    }

    timestamp = ktime_get_ns();

    if (isp_fe_mis) {
        event.type = VVCAM_EVENT_IRQ_TYPE;
        event.id   = VVCAM_EID_FE_MIS;
        event.timestamp = timestamp;
        event.irqevent.irq_value = isp_fe_mis;
        vvcam_event_queue(&isp_dev->event_dev, &event);
        isp_dev->irq_mis[VVCAM_EID_FE_MIS] = isp_fe_mis;
        tasklet_schedule(&isp_dev->stat_tasklet);
    }

    return IRQ_HANDLED;
}

irqreturn_t vvcam_isp_fusa_irq_process(struct vvcam_isp_dev *isp_dev)
{
    uint32_t fusa_ecc_mis1 = 0;
    uint32_t fusa_ecc_mis2 = 0;
    uint32_t fusa_ecc_mis3 = 0;
    uint32_t fusa_ecc_mis4 = 0;
    uint32_t fusa_ecc_mis5 = 0;
    uint32_t fusa_ecc_mis6 = 0;
    uint32_t fusa_dup_mis  = 0;
    uint32_t fusa_parity_mis = 0;
    uint32_t fusa_lv1_mis1 = 0;
    uint64_t timestamp;

    vvcam_event_t event;
    bool irq = false;
    
    if (!isp_dev->refcnt)
        return IRQ_NONE;

    printk("enter fusa_irq_process\n");
    
    fusa_ecc_mis1 = vvcam_isp_hal_read_reg(isp_dev->base, FUSA_ECC_MIS1);
    printk("FUSA_ECC_MIS1 : 0x%08x \n", fusa_ecc_mis1);
    if (fusa_ecc_mis1) {
        vvcam_isp_hal_write_reg(isp_dev->base, FUSA_ECC_ICR1, fusa_ecc_mis1);
        irq = true;
    }

    fusa_ecc_mis2 = vvcam_isp_hal_read_reg(isp_dev->base, FUSA_ECC_MIS2);
    printk("FUSA_ECC_MIS2 : 0x%08x \n", fusa_ecc_mis2);
    if (fusa_ecc_mis2) {
        vvcam_isp_hal_write_reg(isp_dev->base, FUSA_ECC_ICR2, fusa_ecc_mis2);
        irq = true;
    }

    fusa_ecc_mis3 = vvcam_isp_hal_read_reg(isp_dev->base, FUSA_ECC_MIS3);
    printk("FUSA_ECC_MIS3 : 0x%08x \n", fusa_ecc_mis3);
    if (fusa_ecc_mis3) {
        vvcam_isp_hal_write_reg(isp_dev->base, FUSA_ECC_ICR3, fusa_ecc_mis3);
        irq = true;
    }

    fusa_ecc_mis4 = vvcam_isp_hal_read_reg(isp_dev->base, FUSA_ECC_MIS4);
    printk("FUSA_ECC_MIS4 : 0x%08x \n", fusa_ecc_mis4);
    if (fusa_ecc_mis4) {
        vvcam_isp_hal_write_reg(isp_dev->base, FUSA_ECC_ICR4, fusa_ecc_mis4);
        irq = true;
    }

    fusa_ecc_mis5 = vvcam_isp_hal_read_reg(isp_dev->base, FUSA_ECC_MIS5);
    printk("FUSA_ECC_MIS5 : 0x%08x \n", fusa_ecc_mis5);
    if (fusa_ecc_mis5) {
        vvcam_isp_hal_write_reg(isp_dev->base, FUSA_ECC_ICR5, fusa_ecc_mis5);
        irq = true;
    }

    fusa_ecc_mis6 = vvcam_isp_hal_read_reg(isp_dev->base, FUSA_ECC_MIS6);
    printk("FUSA_ECC_MIS6 : 0x%08x \n", fusa_ecc_mis6);
    if (fusa_ecc_mis6) {
        vvcam_isp_hal_write_reg(isp_dev->base, FUSA_ECC_ICR6, fusa_ecc_mis6);
        irq = true;
    }

    fusa_dup_mis = vvcam_isp_hal_read_reg(isp_dev->base, FUSA_DUP_MIS);
    printk("FUSA_DUP_MIS : 0x%08x \n", fusa_dup_mis);
    if (fusa_dup_mis) {
        vvcam_isp_hal_write_reg(isp_dev->base, FUSA_DUP_ICR, fusa_dup_mis);
        irq = true;
    }

    fusa_parity_mis = vvcam_isp_hal_read_reg(isp_dev->base, FUSA_PARITY_MIS);
    printk("FUSA_PARITY_MIS : 0x%08x \n", fusa_parity_mis);
    if (fusa_parity_mis) {
        vvcam_isp_hal_write_reg(isp_dev->base, FUSA_PARITY_ICR, fusa_parity_mis);
        irq = true;
    }

    fusa_lv1_mis1 = vvcam_isp_hal_read_reg(isp_dev->base, FUSA_LV1_MIS1);
    printk("FUSA_LV1_MIS1 : 0x%08x \n", fusa_lv1_mis1);
    if (fusa_lv1_mis1) {
        vvcam_isp_hal_write_reg(isp_dev->base, FUSA_LV1_ICR1, fusa_lv1_mis1);
        irq = true;
    }

    timestamp = ktime_get_ns();

    if (fusa_ecc_mis1) {
        event.type = VVCAM_EVENT_IRQ_TYPE;
        event.id   = VVCAM_EID_FUSA_ECC_IMSC1;
        event.timestamp = timestamp;
        event.irqevent.irq_value = fusa_ecc_mis1;

        vvcam_event_queue(&isp_dev->event_dev, &event);
        isp_dev->irq_mis[VVCAM_EID_FUSA_ECC_IMSC1] = event.irqevent.irq_value;
    }
    
    if (fusa_ecc_mis2) {
        event.type = VVCAM_EVENT_IRQ_TYPE;
        event.id   = VVCAM_EID_FUSA_ECC_IMSC2;
        event.timestamp = timestamp;
        event.irqevent.irq_value = fusa_ecc_mis2;

        vvcam_event_queue(&isp_dev->event_dev, &event);
        isp_dev->irq_mis[VVCAM_EID_FUSA_ECC_IMSC2] = event.irqevent.irq_value;
    }

    if (fusa_ecc_mis3) {
        event.type = VVCAM_EVENT_IRQ_TYPE;
        event.id   = VVCAM_EID_FUSA_ECC_IMSC3;
        event.timestamp = timestamp;
        event.irqevent.irq_value = fusa_ecc_mis3;

        vvcam_event_queue(&isp_dev->event_dev, &event);
        isp_dev->irq_mis[VVCAM_EID_FUSA_ECC_IMSC3] = event.irqevent.irq_value;
    }

    if (fusa_ecc_mis4) {
        event.type = VVCAM_EVENT_IRQ_TYPE;
        event.id   = VVCAM_EID_FUSA_ECC_IMSC4;
        event.timestamp = timestamp;
        event.irqevent.irq_value = fusa_ecc_mis4;

        vvcam_event_queue(&isp_dev->event_dev, &event);
        isp_dev->irq_mis[VVCAM_EID_FUSA_ECC_IMSC4] = event.irqevent.irq_value;
    }

    if (fusa_ecc_mis5) {
        event.type = VVCAM_EVENT_IRQ_TYPE;
        event.id   = VVCAM_EID_FUSA_ECC_IMSC5;
        event.timestamp = timestamp;
        event.irqevent.irq_value = fusa_ecc_mis5;

        vvcam_event_queue(&isp_dev->event_dev, &event);
        isp_dev->irq_mis[VVCAM_EID_FUSA_ECC_IMSC5] = event.irqevent.irq_value;
    }

    if (fusa_ecc_mis6) {
        event.type = VVCAM_EVENT_IRQ_TYPE;
        event.id   = VVCAM_EID_FUSA_ECC_IMSC6;
        event.timestamp = timestamp;
        event.irqevent.irq_value = fusa_ecc_mis6;

        vvcam_event_queue(&isp_dev->event_dev, &event);
        isp_dev->irq_mis[VVCAM_EID_FUSA_ECC_IMSC6] = event.irqevent.irq_value;
    }

    if (fusa_dup_mis) {
        event.type = VVCAM_EVENT_IRQ_TYPE;
        event.id   = VVCAM_EID_FUSA_DUP_IMSC;
        event.timestamp = timestamp;
        event.irqevent.irq_value = fusa_dup_mis;

        vvcam_event_queue(&isp_dev->event_dev, &event);
        isp_dev->irq_mis[VVCAM_EID_FUSA_DUP_IMSC] = event.irqevent.irq_value;
    }

    if (fusa_parity_mis) {
        event.type = VVCAM_EVENT_IRQ_TYPE;
        event.id   = VVCAM_EID_FUSA_PARITY_IMSC;
        event.timestamp = timestamp;
        event.irqevent.irq_value = fusa_parity_mis;

        vvcam_event_queue(&isp_dev->event_dev, &event);
        isp_dev->irq_mis[VVCAM_EID_FUSA_PARITY_IMSC] = event.irqevent.irq_value;
    }

    if (fusa_lv1_mis1) {
        event.type = VVCAM_EVENT_IRQ_TYPE;
        event.id   = VVCAM_EID_FUSA_LV1_IMSC1;
        event.timestamp = timestamp;
        event.irqevent.irq_value = fusa_lv1_mis1;

        vvcam_event_queue(&isp_dev->event_dev, &event);
        isp_dev->irq_mis[VVCAM_EID_FUSA_LV1_IMSC1] = event.irqevent.irq_value;
    }

    if (irq) {
        tasklet_schedule(&isp_dev->stat_tasklet);
        return IRQ_HANDLED;
    }

    return IRQ_HANDLED;
}