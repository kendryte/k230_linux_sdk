#ifndef _DISPLAY_LOGO_H
#define _DISPLAY_LOGO_H

#include "stdio.h"

#define VO_SOFT_RST_CTL                                             0x00
#define VO_REG_LOAD_CTL                                             0x04
#define VO_DMA_SW_CTL                                               0x08
#define VO_DMA_RD_CTL_OUT                                           0x0c
#define VO_DMA_ARB_MODE                                             0x10
#define VO_DMA_WEIGHT_RD0                                           0x24
#define VO_DMA_WEIGHT_RD1                                           0x28
#define VO_DMA_WEIGHT_RD2                                           0x2c
#define VO_DMA_WEIGHT_RD3                                           0x30

#define VO_DMA_PRIORITY_RD_1                                        0x40
#define VO_DMA_ID_RD_0                                              0x4c
#define VO_DMA_ID_RD_1                                              0x50

#define VO_CONFIG_LINE_BASE                                         0x0600
#define VO_HSCALE_BASE                                              0x8000
#define VO_VSCALE_BASE                                              0x8C00
#define VO_GAMMA_BASE                                               0xC000

#define VO_LAYER0_LINE0_BD_CTL                                      0x80
#define VO_LAYER0_LINE1_BD_CTL                                      0x84
#define VO_LAYER0_LINE2_BD_CTL                                      0x88
#define VO_LAYER0_LINE3_BD_CTL                                      0x8C
#define VO_LAYER1_BD_CTL                                            0x90
#define VO_LAYER2_BD_CTL                                            0x94
#define VO_LAYER3_BD_CTL                                            0x98
#define VO_LAYER4_BD_CTL                                            0x9c
#define VO_LAYER5_BD_CTL                                            0xa0
#define VO_LAYER6_BD_CTL                                            0xa4
//TIMING
#define VO_DISP_HSYNC_CTL                                           0x100
#define VO_DISP_HSYNC1_CTL                                          0x104
#define VO_DISP_VSYNC1_CTL                                          0x108
#define VO_DISP_VSYNC2_CTL                                          0x110
#define VO_DISP_HSYNC2_CTL                                          0x10c
#define VO_DISP_TOTAL_SIZE                                          0x11C
#define VO_DISP_XZONE_CTL                                           0x0C0
#define VO_DISP_YZONE_CTL                                           0x0C4

// OFFSET + ACTIVE
#define VO_DISP_LAYER0_XCTL                                         0x0C8
#define VO_DISP_LAYER0_YCTL                                         0x0CC
#define VO_DISP_LAYER1_XCTL                                         0x0D0
#define VO_DISP_LAYER1_YCTL                                         0x0D4
#define VO_DISP_LAYER2_XCTL                                         0x0D8
#define VO_DISP_LAYER2_YCTL                                         0x0DC
#define VO_DISP_LAYER3_XCTL                                         0x0E0
#define VO_DISP_LAYER3_YCTL                                         0x0E4
// offset
#define VO_LAYER0_IMG_IN_OFFSET                                     0x16C
#define VO_LAYER1_IMG_IN_OFFSET                                     0x1D4
#define VO_LAYER2_IMG_IN_OFFSET                                     0x214
#define VO_LAYER3_IMG_IN_OFFSET                                     0x254
// VO ACTIVE
#define VO_DISP_LAYER0_IN_ACT_SIZE                                  0x15C
#define VO_DISP_LAYER0_OUT_ACT_SIZE                                 0x160
#define VO_DISP_LAYER1_ACT_SIZE                                     0x1E0
#define VO_DISP_LAYER2_ACT_SIZE                                     0x220
#define VO_DISP_LAYER3_ACT_SIZE                                     0x260
//ADDR
#define VO_DISP_LAYER0_Y_ADDR0                                      0xa0c
#define VO_DISP_LAYER0_Y_ADDR1                                      0xa14
#define VO_DISP_LAYER0_UV_ADDR0                                     0xa10
#define VO_DISP_LAYER0_UV_ADDR1                                     0xa18
#define VO_DISP_LAYER1_Y_ADDR0                                      0xa2c
#define VO_DISP_LAYER1_Y_ADDR1                                      0xa34
#define VO_DISP_LAYER1_UV_ADDR0                                     0xa30
#define VO_DISP_LAYER1_UV_ADDR1                                     0xa38
#define VO_DISP_LAYER2_Y_ADDR0                                      0x204
#define VO_DISP_LAYER2_Y_ADDR1                                      0x20c
#define VO_DISP_LAYER2_UV_ADDR0                                     0x208
#define VO_DISP_LAYER2_UV_ADDR1                                     0x210
#define VO_DISP_LAYER3_Y_ADDR0                                      0x244
#define VO_DISP_LAYER3_Y_ADDR1                                      0x24c
#define VO_DISP_LAYER3_UV_ADDR0                                     0x248
#define VO_DISP_LAYER3_UV_ADDR1                                     0x250
//VO LAYER CRTL
#define VO_LAYER0_CTL                                               0x140
#define VO_LAYER0_IMG_IN_DAT_MODE                                   0x148
#define VO_LAYER1_CTL                                               0x1C0
#define VO_LAYER2_CTL                                               0x200
#define VO_LAYER3_CTL                                               0x240
#define VO_DISP_ENABLE                                              0x118
#define VO_DISP_CTL                                                 0x114
//VO LAYER STRIDE
#define VO_LAYER0_IMG_IN_STRIDE                                     0x164
#define VO_LAYER1_IMG_IN_STRIDE                                     0x1DC
#define VO_LAYER2_IMG_IN_STRIDE                                     0x21C
#define VO_LAYER3_IMG_IN_STRIDE                                     0x25C
//dma burst length
#define VO_LAYER0_SCALE_BLENTH                                      0x144
#define VO_LAYER1_IMG_IN_BLENTH                                     0x1D8
#define VO_LAYER2_IMG_IN_BLENTH                                     0x218
#define VO_LAYER3_IMG_IN_BLENTH                                     0x258
//VO PINPANG MODE SELSET
#define VO_LAYER0_PINGPANG_SEL_MODE                                 0x184
#define VO_LAYER1_PINGPANG_SEL_MODE                                 0x1E4
#define VO_LAYER2_PINGPANG_SEL_MODE                                 0x224
#define VO_LAYER3_PINGPANG_SEL_MODE                                 0x264

#define VO_DISP_BACKGROUND                                          0x3D0
#define VO_DISP_MIX_LAYER_GLB_EN                                    0x3c0
#define VO_DISP_MIX_LAYER_GLB_ALPHA0                                0x3c4
#define VO_DISP_MIX_LAYER_GLB_ALPHA1                                0x3c8
#define VO_DISP_MIX_SEL                                             0x3cc

#define VO_DISP_YUV2RGB_CTL                                         0x380
#define VO_DISP_CLUT_CTL                                            0x3d8
#define VO_DISP_DITH_CTL                                            0x3d4
#define VO_OSD_RGB2YUV_CTL                                          0x340

#define VO_DISP_IRQ0_CTL                                            0x3e0
#define VO_DISP_IRQ1_CTL                                            0x3e4
#define VO_DISP_IRQ2_CTL                                            0x3e8
#define VO_DISP_IRQ_STATUS                                          0x3ec

#define VO_DISP_DRAW_FRAME                                          0x600
#define VO_DISP_DRAW_FRAME_CMD                                      0x780
// layer0 new
#define VO_LAYER0_CONFIG                                            0xA00
#define VO_LAYER0_SIZE                                              0xA04
#define VO_LAYER0_STRIDE                                            0xA08
#define VO_LAYER0_ADDR_SEL                                          0xA1C

#define VO_LAYER0_VSCALE_STEP                                       0x174
#define VO_LAYER0_VSCALE_ST_PSTEP                                   0x178
#define VO_LAYER0_HSCALE_STEP                                       0x17c
#define VO_LAYER0_HSCALE_ST_PSTEP                                   0x180
// layer1 new
#define VO_LAYER1_CONFIG                                            0xA20
#define VO_LAYER1_SIZE                                              0xA24
#define VO_LAYER1_STRIDE                                            0xA28
#define VO_LAYER1_ADDR_SEL                                          0xA3c
// osd
#define VO_DISP_OSD0_XCTL                                           0x0E8
#define VO_DISP_OSD0_YCTL                                           0x0EC
#define VO_DISP_OSD1_XCTL                                           0x0F0
#define VO_DISP_OSD1_YCTL                                           0x0F4
#define VO_DISP_OSD2_XCTL                                           0x0F8
#define VO_DISP_OSD2_YCTL                                           0x0FC
#define VO_DISP_OSD3_XCTL                                           0x820
#define VO_DISP_OSD3_YCTL                                           0x824
#define VO_DISP_OSD4_XCTL                                           0x828
#define VO_DISP_OSD4_YCTL                                           0x82C
#define VO_DISP_OSD5_XCTL                                           0x830
#define VO_DISP_OSD5_YCTL                                           0x834
#define VO_DISP_OSD6_XCTL                                           0x838
#define VO_DISP_OSD6_YCTL                                           0x83C
#define VO_DISP_OSD7_XCTL                                           0x840
#define VO_DISP_OSD7_YCTL                                           0x844
// osd0
#define VO_OSD0_INFO                                                0x280
#define VO_OSD0_DMA_CTRL                                            0x298
#define VO_OSD0_STRIDE                                              0x29C
#define VO_OSD0_VLU_ADDR0                                           0x288
#define VO_OSD0_ALP_ADDR0                                           0x28C
#define VO_OSD0_VLU_ADDR1                                           0x290
#define VO_OSD0_ALP_ADDR1                                           0x294
#define VO_OSD0_ACT_SIZE                                            0x284
#define VO_OSD0_ADDR_SEL_MODE                                       0x2A0
// osd1
#define VO_OSD1_INFO                                                0x2C0
#define VO_OSD1_ACT_SIZE                                            0x2C4
#define VO_OSD1_DMA_CTRL                                            0x2D8
#define VO_OSD1_STRIDE                                              0x2DC
#define VO_OSD1_VLU_ADDR0                                           0x2C8
#define VO_OSD1_ALP_ADDR0                                           0x2CC
#define VO_OSD1_VLU_ADDR1                                           0x2D0
#define VO_OSD1_ALP_ADDR1                                           0x2D4
#define VO_OSD1_ADDR_SEL_MODE                                       0x2E0
//osd2
#define VO_OSD2_INFO                                                0x300
#define VO_OSD2_ACT_SIZE                                            0x304
#define VO_OSD2_DMA_CTRL                                            0x318
#define VO_OSD2_STRIDE                                              0x31C
#define VO_OSD2_VLU_ADDR0                                           0x308
#define VO_OSD2_ALP_ADDR0                                           0x30C
#define VO_OSD2_VLU_ADDR1                                           0x310
#define VO_OSD2_ALP_ADDR1                                           0x314
#define VO_OSD2_ADDR_SEL_MODE                                       0x320
// osd3
#define VO_OSD3_INFO                                                0x850
#define VO_OSD3_SIZE                                                0x854
#define VO_OSD3_VLU_ADDR0                                           0x858
#define VO_OSD3_ALP_ADDR0                                           0x85c
#define VO_OSD3_VLU_ADDR1                                           0x860
#define VO_OSD3_ALP_ADDR1                                           0x864
#define VO_OSD3_DMA_CTRL                                            0x868
#define VO_OSD3_STRIDE                                              0x86c
#define VO_OSD3_ADDR_SEL_MODE                                       0x870
// osd4
#define VO_OSD4_INFO                                                0x880
#define VO_OSD4_SIZE                                                0x884
#define VO_OSD4_VLU_ADDR0                                           0x888
#define VO_OSD4_ALP_ADDR0                                           0x88c
#define VO_OSD4_VLU_ADDR1                                           0x890
#define VO_OSD4_ALP_ADDR1                                           0x894
#define VO_OSD4_DMA_CTRL                                            0x898
#define VO_OSD4_STRIDE                                              0x89c
#define VO_OSD4_ADDR_SEL_MODE                                       0x8a0
// osd5
#define VO_OSD5_INFO                                                0x8b0
#define VO_OSD5_SIZE                                                0x8b4
#define VO_OSD5_VLU_ADDR0                                           0x8b8
#define VO_OSD5_ALP_ADDR0                                           0x8bc
#define VO_OSD5_VLU_ADDR1                                           0x8c0
#define VO_OSD5_ALP_ADDR1                                           0x8c4
#define VO_OSD5_DMA_CTRL                                            0x8c8
#define VO_OSD5_STRIDE                                              0x8cc
#define VO_OSD5_ADDR_SEL_MODE                                       0x8d0
// osd6
#define VO_OSD6_INFO                                                0x8e0
#define VO_OSD6_SIZE                                                0x8e4
#define VO_OSD6_VLU_ADDR0                                           0x8e8
#define VO_OSD6_ALP_ADDR0                                           0x8ec
#define VO_OSD6_VLU_ADDR1                                           0x8f0
#define VO_OSD6_ALP_ADDR1                                           0x8f4
#define VO_OSD6_DMA_CTRL                                            0x8f8
#define VO_OSD6_STRIDE                                              0x8fc
#define VO_OSD6_ADDR_SEL_MODE                                       0x900
// osd4
#define VO_OSD7_INFO                                                0x910
#define VO_OSD7_SIZE                                                0x914
#define VO_OSD7_VLU_ADDR0                                           0x918
#define VO_OSD7_ALP_ADDR0                                           0x91c
#define VO_OSD7_VLU_ADDR1                                           0x920
#define VO_OSD7_ALP_ADDR1                                           0x924
#define VO_OSD7_DMA_CTRL                                            0x928
#define VO_OSD7_STRIDE                                              0x92c
#define VO_OSD7_ADDR_SEL_MODE                                       0x930
// write back
#define VO_WB_DMACH                                                 (0x400) /* read/write */
#define VO_WB_DMAMODE                                               (0x404) /* read/write */
#define VO_WB_WBPIXEL                                               (0x408) /* read/write */
#define VO_WB_OUTADDR0DATAY                                         (0x40c) /* read/write */
#define VO_WB_OUTADDR1DATAY                                         (0x410) /* read/write */
#define VO_WB_OUTADDR0DATAUV                                        (0x414) /* read/write */
#define VO_WB_OUTADDR1DATAUV                                        (0x418) /* read/write */
#define VO_WB_HSTRIDEY                                              (0x41C) /* read/write */
#define VO_WB_HSTRIDEUV                                             (0x420) /* read/write */
#define VO_WB_BUFSIZE                                               (0x424) /* read/write */
#define VO_WB_VSIZE                                                 (0x428) /* read/write */
#define VO_WB_HSIZE                                                 (0x42c) /* read/write */
#define VO_WB_WBCH0DMA                                              (0x440) /* read/write */
#define VO_WB_WBCH0ERRUNIT                                          (0x444) /* read/write */
#define VO_WB_WBCH0ERRTH                                            (0x448) /* read/write */
#define VO_WB_WBCH0INFO                                             (0x44c) /* read/write */
#define VO_WB_WBCH0RST                                              (0x450) /* read/write */
#define VO_WB_WBCH1DMA                                              (0x430) /* read/write */
#define VO_WB_WBCH1ERRUNIT                                          (0x464) /* read/write */
#define VO_WB_WBCH1ERRTH                                            (0x468) /* read/write */
#define VO_WB_WBCH1INFO                                             (0x46c) /* read/write */
#define VO_WB_WBCH1RST                                              (0x470) /* read/write */
#define VO_WB_WBYOUTIDLE                                            (0x480) /* read */
#define VO_WB_WBUVOUTIDLE                                           (0x484) /* read */
#define VO_WB_WBYOUTINT                                             (0x488) /* read */
#define VO_WB_WBUVOUTINT                                            (0x48C) /* read */
#define VO_WB_WBYOUTFUNC0                                           (0x490) /* read */
#define VO_WB_WBYOUTFUNC1                                           (0x494) /* read */
#define VO_WB_WBUVOUTFUNC0                                          (0x498) /* read */
#define VO_WB_WBUVOUTFUNC1                                          (0x49c) /* read */
// osd
#define VO_OSD_INFOR(x)                                             ((x <= 2) ? (VO_OSD0_INFO + x * 0x40)          : ( VO_OSD3_INFO + (x - 3) * 0x30 ))
#define VO_OSD_SIZE(x)                                              ((x <= 2) ? (VO_OSD0_ACT_SIZE + x * 0x40)      : ( VO_OSD3_SIZE + (x - 3) * 0x30 ))

#define VO_OSD_VLU_ADDR0(x)                                         ((x <= 2) ? (VO_OSD0_VLU_ADDR0 + x * 0x40)     : ( VO_OSD3_VLU_ADDR0 + (x - 3) * 0x30 ))
#define VO_OSD_ALP_ADDR0(x)                                         ((x <= 2) ? (VO_OSD0_ALP_ADDR0 + x * 0x40)     : ( VO_OSD3_ALP_ADDR0 + (x - 3) * 0x30 ))
#define VO_OSD_VLU_ADDR1(x)                                         ((x <= 2) ? (VO_OSD0_VLU_ADDR1 + x * 0x40)     : ( VO_OSD3_VLU_ADDR1 + (x - 3) * 0x30 ))
#define VO_OSD_ALP_ADDR1(x)                                         ((x <= 2) ? (VO_OSD0_ALP_ADDR1 + x * 0x40)     : ( VO_OSD3_ALP_ADDR1 + (x - 3) * 0x30 ))

#define VO_OSD_DMA_CTRL(x)                                          ((x <= 2) ? (VO_OSD0_DMA_CTRL + x * 0x40)      : ( VO_OSD3_DMA_CTRL + (x - 3) * 0x30 ))
#define VO_OSD_STRIDE(x)                                            ((x <= 2) ? (VO_OSD0_STRIDE + x * 0x40)        : ( VO_OSD3_STRIDE + (x - 3) * 0x30 ))
#define VO_OSD_ADDR_SEL_MODE(x)                                     ((x <= 2) ? (VO_OSD0_ADDR_SEL_MODE + x * 0x40) : ( VO_OSD3_ADDR_SEL_MODE + (x - 3) * 0x30 ))
#define VO_DISP_OSD_XCTL(x)                                         ((x <= 2) ? (VO_DISP_OSD0_XCTL + (x * 0x8))    : ( VO_DISP_OSD3_XCTL + (x - 3) * 0x8 ))
#define VO_DISP_OSD_YCTL(x)                                         ((x <= 2) ? (VO_DISP_OSD0_YCTL + (x * 0x8))    : ( VO_DISP_OSD3_YCTL + (x - 3) * 0x8 ))
// layer
#define VO_DISP_LAYER_Y_ADDR0(x)                                    ((x <= 1) ? (VO_DISP_LAYER0_Y_ADDR0 + (x * 0x20)) : ( VO_DISP_LAYER2_Y_ADDR0 + (x - 2) * 0x40 ))
#define VO_DISP_LAYER_UV_ADDR0(x)                                   ((x <= 1) ? (VO_DISP_LAYER0_UV_ADDR0 + (x * 0x20)) : ( VO_DISP_LAYER2_UV_ADDR0 + (x - 2) * 0x40 ))
#define VO_DISP_LAYER_Y_ADDR1(x)                                    ((x <= 1) ? (VO_DISP_LAYER0_Y_ADDR1 + (x * 0x20)) : ( VO_DISP_LAYER2_Y_ADDR1 + (x - 2) * 0x40 ))
#define VO_DISP_LAYER_UV_ADDR1(x)                                   ((x <= 1) ? (VO_DISP_LAYER0_UV_ADDR1 + (x * 0x20)) : ( VO_DISP_LAYER2_UV_ADDR1 + (x - 2) * 0x40 ))
#define VO_DISP_LAYER_CFG(x)                                        ((x <= 1) ? (VO_LAYER0_CONFIG + (x * 0x20)) : ( VO_LAYER2_CTL + (x - 2) * 0x40 ))
#define VO_DISP_LAYER_ADDR_SEL_MODE(x)                              ((x <= 1) ? (VO_LAYER0_ADDR_SEL + (x * 0x20)) : ( VO_LAYER2_PINGPANG_SEL_MODE + (x - 2) * 0x40 ))
#define VO_DISP_LAYER_XCTL(x)                                       (VO_DISP_LAYER0_XCTL + (x * 0x8))
#define VO_DISP_LAYER_YCTL(x)                                       (VO_DISP_LAYER0_YCTL + (x * 0x8))
#define VO_DISP_IN_SIZE(x)                                          (VO_LAYER0_CONFIG + (x * 0x20))
#define VO_DISP_OUT_SIZE(x)                                         ((x <= 1) ? (VO_LAYER0_CONFIG + (x * 0x20)) : ( VO_DISP_LAYER2_ACT_SIZE + (x - 2) * 0x40 ))
#define VO_DISP_LAYER_IMG_BLENTH(x)                                 ((x <= 1) ? (VO_LAYER0_SCALE_BLENTH + (x * 0x94)) : ( VO_LAYER2_IMG_IN_BLENTH + (x - 2) * 0x40 ))


// dsi reg
#define MODE_CFG                                                    (0x34)
#define CLKMGR_CFG                                                  (0x08)
#define DPI_VCID                                                    (0x0c)
#define DPI_COLOR_CODING                                            (0x10)
#define DPI_CFG_POL                                                 (0x14)
#define DPI_LP_CMD_TIM                                              (0x18)
#define INT_ST0                                                     (0xbc)
#define INT_ST1                                                     (0xc0)
#define INT_MSK0                                                    (0xc4)
#define INT_MSK1                                                    (0xc8)
#define GEN_VCID                                                    (0x30)
#define LPCLK_CTRL                                                  (0x94)
// dpi timing
#define VID_MODE_CFG                                                (0x38)
#define VID_PKT_SIZE                                                (0x3c)
#define VID_NUM_CHUNKS                                              (0x40)
#define VID_NULL_SIZE                                               (0x44)
#define VID_HSA_TIME                                                (0x48)
#define VID_HBP_TIME                                                (0x4c)
#define VID_HLINE_TIME                                              (0x50)
#define VID_VSA_LINES                                               (0x54)
#define VID_VBP_LINES                                               (0x58)
#define VID_VFP_LINES                                               (0x5c)
#define VID_VACTIVE_LINES                                           (0x60)
// lpdt
#define PWR_UP                                                      (0x04)
#define PCKHDL_CFG                                                  (0x2c)
#define CMD_MODE_CFG                                                (0x68)
#define GEN_HDR                                                     (0x6c)
#define GEN_PLD_DATA                                                (0x70)
#define LPCLK_CTRL                                                  (0x94)
#define PHY_STATUS                                                  (0xb0)
#define CMD_PKT_STATUS                                              (0x74)
// phy
#define PHY_IF_CFG                                                  (0xa4)

#define PHY_TST_CTRL0                                               0xb4
#define PHY_TST_CTRL1                                               0xb8
#define TXDPHY_CFG1                                                 0x4
#define TXDPHY_CFG0                                                 0x0
#define PHY_RSTZ                                                    0xa0
#define TXDPHY_PLL_CFG1                                             0x10
#define TXDPHY_PLL_CFG0                                             0x08


// bit ops
#define BITS_PER_LONG                                               64
//#define BIT_MASK(nr)                                                (1ul << ((nr) % BITS_PER_LONG))
#define GENMASK(h, l)                                               (((~0UL) << (l)) & (~0UL >> (BITS_PER_LONG - 1 - (h))))


// addr && intr
#define VO_BASE_ADDR                                                ((volatile void __iomem* )0x90840000)
#define DSI_BASE_ADDR                                               ((volatile void __iomem* )0x90850000)
#define RST_BASE_ADDR                                               ((volatile void __iomem* )0x91101000)
#define TXPHY_BASE_ADDR                                             ((volatile void __iomem* )0x90850400)
#define CLK_BASE_ADDR                                               ((volatile void __iomem* )0x91100000)
#define PWR_BASE_ADDR                                               ((volatile void __iomem* )0x91103000)
#define STC_BASE_ADDR                                               ((volatile void __iomem* )0x91108000)
#define IRQN_VO_DISP_INTERRUPT                                      ((volatile void __iomem* )16+117)
#define IRQN_DSI_INTERRUPT                                          ((volatile void __iomem* )16+118)


typedef enum
{
    K_VO_LAYER_Y_ENDIAN_DODE0 = 0,              // 45670123
    K_VO_LAYER_Y_ENDIAN_DODE1,                  // 76543210
    K_VO_LAYER_Y_ENDIAN_DODE2,                  // 01234567
    K_VO_LAYER_Y_ENDIAN_DODE3,                  // 32107654

} k_vo_layer_y_endian_mode;

typedef enum
{
    K_VO_LAYER_UV_ENDIAN_DODE0 = 0,             //U2 V2 U3 V3 U0 V0 U1 V1
    K_VO_LAYER_UV_ENDIAN_DODE1,
    K_VO_LAYER_UV_ENDIAN_DODE2,
    K_VO_LAYER_UV_ENDIAN_DODE3,
    K_VO_LAYER_UV_ENDIAN_DODE4,
    K_VO_LAYER_UV_ENDIAN_DODE5,
    K_VO_LAYER_UV_ENDIAN_DODE6,
    K_VO_LAYER_UV_ENDIAN_DODE7,

} k_vo_layer_uv_endian_mode;


typedef struct
{
    uint32_t pclk;
    uint32_t phyclk;
    uint32_t htotal;
    uint32_t hdisplay;
    uint32_t hsync_len;
    uint32_t hback_porch;
    uint32_t hfront_porch;
    uint32_t vtotal;
    uint32_t vdisplay;
    uint32_t vsync_len;
    uint32_t vback_porch;
    uint32_t vfront_porch;
} k_vo_display_resolution;


typedef enum
{
    K_DSI_1LAN = 0,
    K_DSI_2LAN = 1,
    K_DSI_4LAN = 3,
} k_dsi_lan_num;


typedef enum
{
    K_VO_LP_MODE,
    K_VO_HS_MODE,
} k_vo_dsi_cmd_mode;

typedef enum
{
    K_BURST_MODE = 0,
    K_NON_BURST_MODE_WITH_SYNC_EVENT = 1,
    K_NON_BURST_MODE_WITH_PULSES = 2,
} k_dsi_work_mode;

typedef struct
{
    k_vo_display_resolution resolution;
    k_dsi_lan_num lan_num;
    k_vo_dsi_cmd_mode cmd_mode;
    k_dsi_work_mode work_mode;
    uint32_t lp_div;
} k_vo_dsi_attr;


typedef struct
{
    uint32_t n;
    uint32_t m;
    uint32_t voc;
    uint32_t hs_freq;
} k_connectori_phy_attr;

typedef struct {
    uint32_t dsi_test_mode;
    uint32_t bg_color;
    uint32_t intr_line;
    uint32_t pixclk_div;
    uint32_t buff_num;
    k_dsi_lan_num lan_num;
    k_dsi_work_mode work_mode;
    k_vo_dsi_cmd_mode cmd_mode;
    k_connectori_phy_attr phy_attr;
    k_vo_display_resolution resolution;

    uint32_t w_start;
    uint32_t h_start;
} k_connector_info;


typedef enum
{
    K_VO_INTF_MIPI = 0,
} k_vo_intf_type;

typedef enum
{
    K_VO_OUT_1080P30,
    K_VO_OUT_1080P60,
} k_vo_intf_sync;

typedef struct
{
    uint32_t  bg_color;        //yuv
    k_vo_intf_type intf_type;
    k_vo_intf_sync intf_sync;
    k_vo_display_resolution *sync_info;          //

} k_vo_pub_attr;

typedef enum
{
    K_VO_LAYER0 = 0,
    K_VO_LAYER1 = 1,
    K_VO_LAYER2 = 2,
    K_MAX_VO_LAYER_NUM,
} k_vo_layer;

typedef enum
{
    K_VO_OSD0 = 0,
    K_VO_OSD1 = 1,
    K_VO_OSD2 = 2,
    K_VO_OSD3 = 3,
    K_MAX_VO_OSD_NUM,
} k_vo_osd;


typedef enum
{
    K_VO_OSD_MAP_ORDER = 0,
    K_VO_OSD_MAP_1234_TO_2143 = 2,
    K_VO_OSD_MAP_1234_TO_4321 = 3,

} k_vo_osd_dma_map;


typedef struct
{
    uint32_t n;
    uint32_t m;
    uint32_t voc;
    uint32_t phy_lan_num;
    uint32_t hs_freq;
} k_vo_mipi_phy_attr;

typedef enum
{
    K_ROTATION_0 = (0x01L << 0),
    K_ROTATION_90 = (0x01L << 1),
    K_ROTATION_180 = (0x01L << 2),
    K_ROTATION_270 = (0x01L << 3),
} k_vo_rotation;

// vo
void kd_vo_enable(void);
int kd_vo_set_dev_param(k_vo_pub_attr *attr);
void kd_vo_set_timing(k_vo_display_resolution *resolution);
uint32_t vo_init(void);
void kd_vo_set_layer(k_connector_info *info, uint32_t width, uint32_t height, uint32_t pos_x , uint32_t pos_y , k_vo_rotation rotation, uint32_t addr);
void vo_layer1_test(k_connector_info *info);


// dsi
uint32_t kd_dsi_set_attr(k_vo_dsi_attr *attr);
uint32_t kd_dwc_lpdt_send_pkg(uint8_t *buf, uint32_t cmd_len);

// clk
void k230_set_pixclk(uint32_t div);

// phy
uint32_t dwc_mipi_phy_config(k_vo_mipi_phy_attr *phy);

//reset
void k230_display_rst(void);
int st7701_init(k_connector_info *info);
uint32_t dwc_dsi_enable(uint32_t enable);
void kd_vo_set_vtth_intr(bool status, uint32_t vpos);
uint32_t kd_dwc_lpdt_send_pkg(uint8_t *buf, uint32_t cmd_len);
unsigned long k230_display_logo_get_pic_mem_addr(void);
#endif
