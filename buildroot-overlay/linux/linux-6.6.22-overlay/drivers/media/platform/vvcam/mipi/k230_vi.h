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
#ifndef __VI_H__
#define __VI_H__

#ifdef __cplusplus
        extern "C" {
#endif /* end of #ifdef __cplusplus */

#include "k230_csi.h"
#include <linux/types.h>

#define VI_REG_BASE_ADDR                        0x90009000
#define VI_REG_MEM_SIZE                         0x10000
#define SYSCTL_CLK_ADDR                         (0X91108000)
#define SYSCTL_REG_MEM_SIZE                     0x1000

#define VI_RXDPHY0_CFG0                         (0x0)
#define VI_RXDPHY0_CFG1                         (0x4)
#define VI_RXDPHY1_CFG0                         (0x8)
#define VI_RXDPHY1_CFG1                         (0xc)
#define VI_RXDPHY2_CFG0                         (0x10)
#define VI_RXDPHY2_CFG1                         (0x14)
#define VI_CSI_IPI_HALT_CFG                     (0x18)
#define VI_CSI_EMAEDDED_STA                     (0x1C)
#define VI_ISP_CFG                              (0x20)
#define VI_SLAVE_MODE_CFG                       (0x24)
#define VI_SLAVE_MODE_SEN0_VS_CFG               (0x28)
#define VI_SLAVE_MODE_SEN0_HS_CFG               (0x2C)
#define VI_SLAVE_MODE_SEN1_VS_CFG               (0x30)
#define VI_SLAVE_MODE_SEN1_HS_CFG               (0x34)
#define VI_SLAVE_MODE_SEN0_VS_HIGH              (0x38)
#define VI_SLAVE_MODE_SEN0_HS_HIGH              (0x3C)
#define VI_SLAVE_MODE_SEN1_VS_HIGH              (0x40)
#define VI_SLAVE_MODE_SEN1_HS_HIGH              (0x44)
#define VI_3D_MODE_LIGHT_CTL                    (0x48)
#define VI_FLASH_TRIGGER_0_CFG_0                (0x4c)
#define VI_FLASH_TRIGGER_0_CFG_1                (0x50)
#define VI_FLASH_TRIGGER_1_CFG_0                (0x54)
#define VI_FLASH_TRIGGER_1_CFG_1                (0x58)
#define VI_FLASH_TRIGGER_CFG                    (0x5c)
#define VI_IMG_DVP_SELECT_CTL                   (0x60)
#define VI_CAP_STC_VI_CFG                       (0x64)
#define VI_INTR_SET                             (0x68)
#define VI_INTR_CLR                             (0x6C)
#define VI_INTR_MASK_REG                        (0x70)
#define VI_INTR_STATUS                          (0x74)
#define VI_IPI_POL_CFG                          (0x78)
#define VI_CSI0_DIS_FRAME_M                     (0x7C)
#define VI_CSI0_DIS_FRAME_N                     (0x80)
#define VI_CSI1_DIS_FRAME_M                     (0x84)
#define VI_CSI1_DIS_FRAME_N                     (0x88)
#define VI_CSI2_DIS_FRAME_M                     (0x8C)
#define VI_CSI2_DIS_FRAME_N                     (0x90)
#define VI_DIS_FRAME_EN                         (0x94)
#define VI_SONY_CONFIG                          (0x98)
#define VI_VI_PHY_STA_CLR                       (0x9C)
#define VI_FLASH_TRIGGER_CFG_1                  (0xA0)
#define VI_SONY_CONFIG_1                        (0xA4)
#define VI_SONY_CONFIG_2                        (0xA8)
#define VI_CAP_STC_VI_HDR                       (0xAC)
#define VI_PHY_ENABLE_CLK                       (0xB0)

// bit ops
#define BITS_PER_LONG                           64
// #define BIT_MASK(nr)                            (1ul << ((nr) % BITS_PER_LONG))
// #define GENMASK(h, l)                           (((~0UL) << (l)) & (~0UL >> (BITS_PER_LONG - 1 - (h))))


typedef struct {
    bool err_phy0_control_lan0;
    bool err_phy0_control_lan1;
    bool err_phy0_lp1_contention;
    bool err_phy0_lp0_contention;

    bool err_phy1_control_lan0;
    bool err_phy1_control_lan1;
    bool err_phy1_lp1_contention;
    bool err_phy1_lp0_contention;

    bool err_phy2_control_lan0;
    bool err_phy2_control_lan1;
    bool err_phy2_lp1_contention;
    bool err_phy2_lp0_contention;

    bool flash_trigger0_strobe_miss;
    bool flash_trigger1_strobe_miss;
    bool flash_trigger0_frame_miss;
    bool flash_trigger1_frame_miss; 
} k_vi_err_status;

/**
 * @brief Defines the MIPI CSI data type
 *
 */
enum csi_data_type {
    CSI_DATA_TYPE_RAW8   = 0x2A,
    CSI_DATA_TYPE_RAW10 = 0x2B,
    CSI_DATA_TYPE_RAW12 = 0x2C,
    CSI_DATA_TYPE_RAW16 = 0x2E,
    CSI_DATA_TYPE_YUV422_8 = 0x1E,
};

/**
 * @brief Defines the HDR mode of VICAP
 *
 */
enum hdr_mode {
    VCID_HDR_2FRAME = 0,
    VCID_HDR_3FRAME = 1,
    SONY_HDR_3FRAME = 2,
    SONY_HDR_2FRAME = 3,
    LINERA_MODE     = 4,
};

/**
 * @brief Defines the dvp port of VICAP
 *
 */
enum vi_dvp_port {
    VI_DVP_PORT0 = 0,
    VI_DVP_PORT1 = 1,
    VI_DVP_PORT2 = 2,
    VI_DVP_PORT_MAX,
};

/**
 * @brief Defines the MIPI CSI work mode
 *
 */
enum csi_work_mode {
    CSI_CAMERA_MODE  = 0,
    CSI_CONTROL_MODE = 1,
};

enum vi_ipi {
    IPI1 = 1,
    IPI2 = 2,
    IPI3 = 3,
};

/**
 * @brief Defines the vi ipi attr of VICAP
 *
 */
struct vi_ipi_attr {
    enum csi_work_mode work_mode;
    enum csi_data_type data_type;
    bool is_csi_sync_event;
    u32 hsa;
    u32 hbp;
};

/**
 * @brief Defines vi attributes of VICAP
 *
 */
struct vi_attr {
    enum csi_num csi_num;
    enum mipi_lanes mipi_lanes;
    enum csi_data_type data_type;
    enum mipi_phy_freq phy_freq;
    enum hdr_mode hdr_mode;
    enum vi_dvp_port dvp_port;
};

int vi_device_init(void);
int kd_vi_set_config(struct vi_attr *attr);

#ifdef __cplusplus
}
#endif /* end of #ifdef __cplusplus */

#endif

