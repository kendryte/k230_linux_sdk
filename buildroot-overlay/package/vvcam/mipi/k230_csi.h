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
#ifndef __DW_CSI_H__
#define __DW_CSI_H__

#ifdef __cplusplus
extern "C" {
#endif /* end of #ifdef __cplusplus */

#define  CSI_REG_BASE_ADDR 0x90009000
#define  CSI_REG_MEM_SIZE  0x10000

#define DWC_LANES_NUM                           (0x04)
#define DWC_CSI2_RESETN                         (0x08)
#define DWC_INT_ST_MAIN                         (0x0c)
#define DWC_DATA_IDS_1                          (0x10)
#define DWC_DATA_IDS_2                          (0x14)
#define DWC_PHY_CFG                             (0x18)
#define DWC_PHY_MODE                            (0x1c)
#define DWC_DATA_IDS_VC_1                       (0x30)
#define DWC_DATA_IDS_VC_2                       (0x34)
#define DWC_PHY_SHUTDOWNZ                       (0x40)
#define DWC_DPHY_RSTZ                           (0x44)
#define DWC_PHY_RX                              (0x48)
#define DWC_PHY_STOPSTATE                       (0x4c)
#define DWC_PHY_TEST_CTRL0                      (0x50)
#define DWC_PHY_TEST_CTRL1                      (0x54)
#define DWC_PHY2_TEST_CTRL0                     (0x58)
#define DWC_PHY2_TEST_CTRL1                     (0x5c)
#define DWC_PPI_PG_PATTERN_VRES                 (0x60)
#define DWC_PPI_PG_PATTERN_HRES                 (0x64)
#define DWC_PPI_PG_CONFIG                       (0x68)
#define DWC_PPI_PG_ENABLE                       (0x6c)
#define DWC_PPI_PG_STATUS                       (0x70)
#define DWC_IPI_MODE                            (0x80)
#define DWC_IPI_VCID                            (0x84)
#define DWC_IPI_DATA_TYPE                       (0x88)
#define DWC_IPI_MEM_FLUSH                       (0x8c)
#define DWC_IPI_HSA_TIME                        (0x90)
#define DWC_IPI_HBP_TIME                        (0x94)
#define DWC_IPI_HSD_TIME                        (0x98)
#define DWC_IPI_HLINE_TIME                      (0x9c)
#define DWC_IPI_SOFTRSTN                        (0xa0)
#define DWC_IPI_ADV_FEATURES                    (0x0c)
#define DWC_IPI_VSA_LINES                       (0xb0)
#define DWC_IPI_VBP_LINES                       (0xb4)
#define DWC_IPI_VFP_LINES                       (0xb8)
#define DWC_IPI_VACTIVE_LINES                   (0xbc)
#define DWC_VC_EXTENSION                        (0xc8)
#define DWC_PHY_CAL                             (0xcc)
#define DWC_INT_ST_PHY_FATAL                    (0xe0)
#define DWC_INT_MSK_PHY_FATAL                   (0xe4)
#define DWC_INT_FORCE_PHY_FATAL                 (0xe8)
#define DWC_INT_ST_PKT_FATAL                    (0xf0)
#define INT_MSK_PKT_FATAL                       (0xf4)
#define DWC_INT_FORCE_PKT_FATAL                 (0x08)
#define DWC_IPI2_MODE                           (0x200)
#define DWC_IPI2_VCID                           (0x204)
#define DWC_IPI2_DATA_TYPE                      (0x208)
#define DWC_IPI2_MEM_FLUSH                      (0x20c)
#define DWC_IPI2_HSA_TIME                       (0x210)
#define DWC_IPI2_HBP_TIME                       (0x214)
#define DWC_IPI2_HSD_TIME                       (0x218)
#define DWC_IPI2_ADV_FEATURES                   (0x21c)
#define DWC_IPI3_MODE                           (0x220)
#define DWC_IPI3_VCID                           (0x224)
#define DWC_IPI3_DATA_TYPE                      (0x228)
#define DWC_IPI3_MEM_FLUSH                      (0x22c)
#define DWC_IPI3_HSA_TIME                       (0x230)
#define DWC_IPI3_HBP_TIME                       (0x234)
#define DWC_IPI3_HSD_TIME                       (0x238)
#define DWC_IPI3_ADV_FEATURES                   (0x23c)

/**
 * @brief Defines the CSI NUM
 *
 */
enum csi_num {
    CSI0 = 1,
    CSI1 = 2,
    CSI2 = 3,
};

/**
 * @brief Defines the MIPI LANEs
 *
 */
enum mipi_lanes {
    MIPI_1LANE = 0,
    MIPI_2LANE = 1,
    MIPI_4LANE = 3,
};

/**
 * @brief Defines the MIPI CSI PHY freq
 *
 */
enum mipi_phy_freq {
	MIPI_PHY_800M  = 1,
	MIPI_PHY_1200M = 2,
	MIPI_PHY_1600M = 3,
};

int dwc_csi_phy_init(enum csi_num csi, enum mipi_phy_freq freq, enum mipi_lanes lan_num);
int csi_device_init(void);

#ifdef __cplusplus
}
#endif /* end of #ifdef __cplusplus */

#endif

