// ******************************************************************************
// Copyright     :  Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
// File name     :  diag_ctl_rb_reg_offset.h
// Project line  :
// Department    :
// Author        :  HH503 SoC
// Version       :  V100
// Date          :  2020/7/27
// Description   :  ws73 SERIES
// Others        :  Generated automatically by nManager V4.1
// History       :  HH503 SoC 2022/07/08 15:00:45 Create file
// ******************************************************************************

#ifndef __DIAG_CTL_RB_REG_OFFSET_H__
#define __DIAG_CTL_RB_REG_OFFSET_H__

/* DIAG_CTL_RB Base address of Module's Register */
#define DIAG_CTL_RB_BASE (0x40018400)

/* **************************************************************************** */
/*                      HH503 DIAG_CTL_RB Registers' Definitions                            */
/* **************************************************************************** */

#define DIAG_CTL_RB_CFG_HIGH_CLK_MODE_REG (DIAG_CTL_RB_BASE + 0x100)       /* 维测读写跑高频 */
#define DIAG_CTL_RB_CFG_PKT_RAM_EN_DEL_REG (DIAG_CTL_RB_BASE + 0x104)      /* PacketRAM配置 */
#define DIAG_CTL_RB_CFG_PKT_WR_RD_SEL_REG (DIAG_CTL_RB_BASE + 0x108)       /* PacketRAM配置 */
#define DIAG_CTL_RB_CFG_PKT_RAM_CLK_EN_REG (DIAG_CTL_RB_BASE + 0x10C)      /* PacketRAM配置 */
#define DIAG_CTL_RB_CFG_PC_RA_SEL_REG (DIAG_CTL_RB_BASE + 0x110)           /* CPU_PC_RA配置 */
#define DIAG_CTL_RB_CFG_APB_DIAG_EN_REG (DIAG_CTL_RB_BASE + 0x114)         /* APB监测使能 */
#define DIAG_CTL_RB_CFG_SAMPLE_SEL_REG (DIAG_CTL_RB_BASE + 0x210)          /* 数据采集源选择 */
#define DIAG_CTL_RB_CFG_SAMPLE_NODE_SEL_REG (DIAG_CTL_RB_BASE + 0x214)     /* 数据采集源节点选择 */
#define DIAG_CTL_RB_CFG_SAMPLE_LENGTH_REG (DIAG_CTL_RB_BASE + 0x218)       /* 数据采集长度配置 */
#define DIAG_CTL_RB_CFG_SAMPLE_START_ADDR_REG (DIAG_CTL_RB_BASE + 0x21C)   /* 数据采集起始地址配置 */
#define DIAG_CTL_RB_CFG_SAMPLE_END_ADDR_REG (DIAG_CTL_RB_BASE + 0x220)     /* 数据采集结束地址配置 */
#define DIAG_CTL_RB_CFG_SAMPLE_MODE_REG (DIAG_CTL_RB_BASE + 0x224)         /* 数据采集源模式选择 */
#define DIAG_CTL_RB_CFG_SAMPLE_GEN_SYNC_REG (DIAG_CTL_RB_BASE + 0x228)     /* 数据采集模式同步使能 */
#define DIAG_CTL_RB_CFG_SAMPLE_EN_REG (DIAG_CTL_RB_BASE + 0x22C)           /* 数据采集使能 */
#define DIAG_CTL_RB_SAMPLE_DONE_REG (DIAG_CTL_RB_BASE + 0x230)             /* 数据完成指示 */
#define DIAG_CTL_RB_SAMPLE_DONE_ADDR_REG (DIAG_CTL_RB_BASE + 0x234)        /* 数采完成上报地址 */
#define DIAG_CTL_RB_PKE_MEM_OBS_SAMPLE_ADDR_REG (DIAG_CTL_RB_BASE + 0x238) /* 片内地址实时观测 */
#define DIAG_CTL_RB_OBS_DIAG_CLK_DBG_REG (DIAG_CTL_RB_BASE + 0x23C)        /* 片内时钟上报 */
#define DIAG_CTL_RB_LOOP_INT_DEPTH_REG (DIAG_CTL_RB_BASE + 0x240)          /* 循环数采中断深度 */
#define DIAG_CTL_RB_DIAG_INT_EN_REG (DIAG_CTL_RB_BASE + 0x244)             /* 维测中断上报使能寄存器 */
#define DIAG_CTL_RB_DIAG_INT_REG (DIAG_CTL_RB_BASE + 0x248)                /* 维测中断上报寄存器 */
#define DIAG_CTL_RB_DIAG_DLY_REG (DIAG_CTL_RB_BASE + 0x24C)                /* 维测延迟停止配置寄存器 */
#define DIAG_CTL_RB_DIAG_BUS_MONITOR_REG (DIAG_CTL_RB_BASE + 0x250)        /* 维测中断监测使能 */
#define DIAG_CTL_RB_DIAG_BUS_MONITOR_ADDR_REG (DIAG_CTL_RB_BASE + 0x254)   /* 维测地址监测 */
#define DIAG_CTL_RB_DIAG_BUS_MONITOR_MASK_REG (DIAG_CTL_RB_BASE + 0x258)   /* 维测数据掩位 */
#define DIAG_CTL_RB_DIAG_BUS_MONITOR_DATA_REG (DIAG_CTL_RB_BASE + 0x25C)   /* 维测数据掩位 */
#define DIAG_CTL_RB_CFG_DIAG_JUMP_REG (DIAG_CTL_RB_BASE + 0x260)           /* 维测跳步长配置 */
#define DIAG_CTL_RB_MONITOR_FIFO_STATUS_REG (DIAG_CTL_RB_BASE + 0x264)     /* 采数异常状态指示 */
#define DIAG_CTL_RB_PKE_MEM_OBS_CPU_RADDR_REG (DIAG_CTL_RB_BASE + 0x270)   /* CPU读RAM地址指针监测 */
#define DIAG_CTL_RB_CFG_TEST_GEN_SEL_REG (DIAG_CTL_RB_BASE + 0x300)        /* 数据生成选择 */
#define DIAG_CTL_RB_CFG_TEST_GEN_MODE_REG (DIAG_CTL_RB_BASE + 0x304)       /* 数据生成模式 */
#define DIAG_CTL_RB_CFG_TEST_GEN_NODE_REG (DIAG_CTL_RB_BASE + 0x308)       /* 数据生成模式 */
#define DIAG_CTL_RB_CFG_TEST_GEN_LENGTH_REG (DIAG_CTL_RB_BASE + 0x30C)     /* 数据生成长度配置 */
#define DIAG_CTL_RB_CFG_TEST_GEN_START_ADDR_REG (DIAG_CTL_RB_BASE + 0x310) /* 数据生成起始地址配置 */
#define DIAG_CTL_RB_CFG_TEST_GEN_END_ADDR_REG (DIAG_CTL_RB_BASE + 0x314)   /* 数据生成结束地址配置 */
#define DIAG_CTL_RB_CFG_TEST_GEN_OFFSET_REG (DIAG_CTL_RB_BASE + 0x318)     /* 数据生成偏移地址配置 */
#define DIAG_CTL_RB_CFG_TEST_GEN_DELAY_NUM_REG (DIAG_CTL_RB_BASE + 0x31C)  /* 生成数据延迟配置寄存器 */
#define DIAG_CTL_RB_CFG_GEN_EN_REG (DIAG_CTL_RB_BASE + 0x320)              /* 数据生成使能 */
#define DIAG_CTL_RB_GEN_DONE_REG (DIAG_CTL_RB_BASE + 0x324)                /* 数据生成完成指示 */
#define DIAG_CTL_RB_GEN_DONE_ADDR_REG (DIAG_CTL_RB_BASE + 0x328)           /* 数据生成完成上报地址 */
#define DIAG_CTL_RB_CFG_MONITOR_CLOCK_REG (DIAG_CTL_RB_BASE + 0x330)       /* 数采时钟配置 */
#define DIAG_CTL_RB_OBS_GEN_ADDR_REG (DIAG_CTL_RB_BASE + 0x334)            /* GEN地址观测 */
#define DIAG_CTL_RB_OBS_GEN_CLK_DBG_REG (DIAG_CTL_RB_BASE + 0x338)         /* GEN 时钟观测调试 */
#define DIAG_CTL_RB_CFG_DIAG_MUX_REG (DIAG_CTL_RB_BASE + 0x350)            /* DIAG选择寄存器 */
#define DIAG_CTL_RB_CFG_GROUP0_PIN_SEL_0_7_REG (DIAG_CTL_RB_BASE + 0x354)  /* GROUP0 DIAG PIN选择寄存器 \
                                                                                          */
#define DIAG_CTL_RB_CFG_CLOCK_TEST_SEL_REG (DIAG_CTL_RB_BASE + 0x358)      /* 观测时钟选择配置寄存器 */
#define DIAG_CTL_RB_CFG_CLOCK_TEST_DIV_REG (DIAG_CTL_RB_BASE + 0x35C)      /* 观测时钟分频配置寄存器 */
#define DIAG_CTL_RB_CFG_CLOCK_TEST_GATE_EN_REG (DIAG_CTL_RB_BASE + 0x360)  /* 观测时钟输出使能配置寄存器 \
                                                                                          */
#define DIAG_CTL_RB_CFG_CLOCK_CNT_REG (DIAG_CTL_RB_BASE + 0x364) /* 观测时钟选择计数周期配置寄存器 */
#define DIAG_CTL_RB_CFG_CLOCK_CNT_EN_REG \
    (DIAG_CTL_RB_BASE + 0x368)                                          /* 观测时钟选择计数周期使能配置寄存器 */
#define DIAG_CTL_RB_RPT_CLOCK_CNT_REG (DIAG_CTL_RB_BASE + 0x36C) /* 观测时钟选择计数周期上报 */
#define DIAG_CTL_RB_PCPU_LOAD_REG (DIAG_CTL_RB_BASE + 0x370)     /* WCPU_LOAD */
#define DIAG_CTL_RB_RPT_PCPU_PC_REG (DIAG_CTL_RB_BASE + 0x374)   /* PCPU_PC */
#define DIAG_CTL_RB_RPT_PCPU_LR_REG (DIAG_CTL_RB_BASE + 0x378)   /* PCPU_LR */
#define DIAG_CTL_RB_RPT_PCPU_SP_REG (DIAG_CTL_RB_BASE + 0x37C)   /* PCPU_SP */
#define DIAG_CTL_RB_PCPU_PCLRSP_EN_REG (DIAG_CTL_RB_BASE + 0x380) /* PCLRSP_EN */

#endif // __DIAG_CTL_RB_REG_OFFSET_H__
