/*
 * Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
 * Description: tsensor driver implementatioin's inner head file.
 * Author: CompanyName
 * Create: 2023-02-15
 */

#ifndef __TSENSOR_H__
#define __TSENSOR_H__

#if defined(CONFIG_SUPPORT_TSENSOR)

#include <td_base.h>
#include <reg_table.h>
#include "soc_osal.h"
#include "ints_table.h"
#include "hcc_if.h"
#include "oal_debug.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_INT_CALLBACK_NUM                3

#define TSENSOR_IRQ                         (NUM_INTERRUPT_TSENSOR)

#define TSENSOR_START                       0x0000    /* TSENSOR 启动 */
#define TSENSOR_CTRL                        0x0004    /* TSENSOR 控制寄存器 */
#define TSENSOR_MAN_STS                     0x0008    /* TSENSOR 手动控制状态寄存器 */
#define TSENSOR_AUTO_STS                    0x000C    /* TSENSOR 自动控制状态寄存器 */
#define TSENSOR_CALIB_CTRL                  0x0010    /* TSENSOR CALIB配置寄存器(校准配置?) */
#define TSENSOR_TEMP_HIGH_LIMIT             0x0014    /* TSENSOR 温度门限上限 */
#define TSENSOR_TEMP_LOW_LIMIT              0x0018    /* TSENSOR 温度门限下限 */
#define TSENSOR_TEMP_OVER_LIMIT             0x001C    /* TSENSOR 过温(超高温)门限 */
#define TSENSOR_TEMP_INT_EN                 0x0020    /* 中断配置寄存器 增加了多档温度读取中断 */
#define TSENSOR_TEMP_INT_CLR                0x0024    /* 清中断寄存器 */
#define TSENSOR_TEMP_INT_STATE              0x0028    /* 中断状态寄存器 */
#define TSENSOR_AUTO_REFRESH_PERIOD         0x002C    /* TSENSOR 自动检测控制寄存器 */
#define TSENSOR_AUTO_REFRESH_CFG            0x0030    /* TSENSOR 自动检测控制寄存器 */

#define TSENSOR_C0_OVER_TEMP_CTL            0x0034    /* TSENSOR过热手动控制 */
#define TSENSOR_C0_MULTILEVE_TEMP0          0x0038    /* TSENSOR多档温度梯度0 */
#define TSENSOR_C0_MULTILEVE_TEMP1          0x003C    /* TSENSOR多档温度梯度1 */
#define TSENSOR_C0_MULTILEVE_TEMP2          0x0040    /* TSENSOR多档温度梯度2 */
#define TSENSOR_C0_MULTILEVE_TEMP3          0x0044    /* TSENSOR多档温度梯度3 */
#define SCPU_LOCK_STATUS                    0x0048    /* SCPU挂死状态寄存器 */

#define TSENSOR_START_ENABLE                1         /* Tsensor启动寄存器使能 */

#define TSENSOR_CTRL_ENABLE_POS             0         /* Tsensor控制寄存器使能位 */
#define TSENSOR_CTRL_MODE_POS               1         /* Tsensor模式寄存器选择位 */
#define TSENSOR_CTRL_MODE_BITS              2         /* Tsensor模式寄存器总位数 */

#define TSENSOR_AUTO_CLR_POS                0         /* 清除16点平均单次上报模式或16点平均循环上报模式的状态和数据 */
#define TSENSOR_MAN_CLR_POS                 0         /* 清除单点循环上报模式的状态 */
#define TSENSOR_INT_CLR_POS                 0         /* 清中断 */
#define TSENSOR_INT_CTRL_POS                0         /* 关中断 */

#define TSENSOR_AVDD_PG_AUTO_CTL            (~(1<<1)) /* VDDIO power gating 模式控制信号：自动控制 */
#define TSENSOR_AVDD_PG_MAN_CTL             7         /* VDDIO power gating 模式控制信号：手动控制 */
#define TSENSOR_PERIOD_CLOCK_VALUE          32        /* TSENSOR自动检测周期，32K时钟周期数 */
#define TSENSOR_AUTO_REFRESH_ENABLE_POS     0         /* Tsensor自动刷新使能位 */
#define TSENSOR_MODE_16_POINTS_SINGLE       0         /* 模式：16点平均单次上报 */
#define TSENSOR_MODE_16_POINTS_LOOP         1         /* 模式：16点平均循环上报 */
#define TSENSOR_MODE_1_POINT_LOOP           2         /* 模式：单点循环上报 */
#define TSENSOR_READY_POS                   1         /* Tsensor ready?:0: 检测未启动或手动检测中
                                                                        1: tsensor_data_man值为有效的温度值 */
#define TSENSOR_READY_DATA_OFFSET_2BITS     2         /* Tsensor温度有效位的偏移位 */
#define TSENSOR_INT_MOD_DONE_EN_POS         0         /* Tsensor温度采集完毕中断使能 0：Disable；1：Enable */
#define TSENSOR_INT_MOD_THRESHOLD_EN_POS    1         /* Tsensor温度过阈值温度中断使能 */
#define TSENSOR_INT_MOD_PROTECT_EN_POS      2         /* Tsensor温度过温中断使能 */
#define TSENSOR_INT_MOD_LTO0_EN_POS         3         /* Tsensor多档温度 0档位中断使能 */
#define TSENSOR_INT_MOD_0TO1_EN_POS         4         /* Tsensor多档温度 1档位中断使能 */
#define TSENSOR_INT_MOD_1TO2_EN_POS         5         /* Tsensor多档温度 2档位中断使能 */
#define TSENSOR_INT_MOD_2TO3_EN_POS         6         /* Tsensor多档温度 3档位中断使能 */
#define TSENSOR_INT_MOD_3TOH_EN_POS         7         /* Tsensor多档温度 4档位中断使能 */
#define TSENSOR_INT_MOD_DONE_STATUS         0         /* Tsensor温度采集完毕中断 */
#define TSENSOR_INT_MOD_OUTTHRESH_STATUS    1         /* Tsensor温度超门限范围中断 */
#define TSENSOR_INT_MOD_OVERTEMP_STATUS     2         /* Tsensor温度过温中断 */
#define TSENSOR_INT_MOD_LTO0_STATUS         3         /* Tsensor多档温度上报: ml0 > temp */
#define TSENSOR_INT_MOD_0TO1_STATUS         4         /* Tsensor多档温度上报: ml1 > temp >= ml0 */
#define TSENSOR_INT_MOD_1TO2_STATUS         5         /* Tsensor多档温度上报: ml2 > temp >= ml1 */
#define TSENSOR_INT_MOD_2TO3_STATUS         6         /* Tsensor多档温度上报: ml3 > temp >= ml2 */
#define TSENSOR_INT_MOD_3TOH_STATUS         7         /* Tsensor多档温度上报:       temp >= ml3 */
#define TSENSOR_INT_DISABLE_STATUS          0         /* Tsensor去使能所有中断 */
#define TSENSOR_INTERRUPT_BITS              8         /* Tsensor中断使能总位数 */
#define TSENSOR_OVERTEMP_THRESH_ENABLE_POS  10        /* Tsensor超高温PA保护使能 */
#define TSENSOR_PDTEMP_THRESH_ENABLE_POS    10        /* Tsensor过温下电上报使能 */
#define TSENSOR_CALIB_TYPE_POS              1         /* calib type: bit 1:2 */
#define TSENSOR_CALIB_SEL_POS               0         /* calib SEL: 0:enable; 1:disable */
#define TSENSOR_CALIB_TYPE_BITS             2         /* calib type位数 */

#define TSENSOR_OVER_TEMP_PRT_MODE_POS      0         /* Tsensor过热手动控制模式选择:0自动,1手动 */
#define TSENSOR_OVER_TEMP_PRT_MAN_POS       1         /* 手动模式控制RF状态：0关闭RF,1开启RF */
#define TSENSOR_OVER_TEMP_PRT_POLAR_POS     2         /* 自动模式下控制RF状态：0过热时OVER_TEMP_PRT输出0,1过热时OVER_TEMP_PRT输出1 */
#define TSENSOR_OVER_TEMP_PRT_STS_POS       3         /* OVER_TEMP_PRT状态查询 */

#define TSENSOR_TEMPERATURE_HIGH            125       /* 温度校准大值 */
#define TSENSOR_TEMPERATURE_LOW             (-40)     /* 温度校准小值 */
#define TSENSOR_CODE_HIGH                   896       /* 温度校准大值对应的码值 */
#define TSENSOR_CODE_LOW                    114       /* 温度校准小值对应的码值 */

#define TSENSOR_TEMPERATURE_MAX             146     /* 温度最大值 */
#define TSENSOR_TEMPERATURE_MIN             (-67)     /* 温度最小值 */
#define TSENSOR_CODE_MAX                    1023     /* 温度最大值对应的码值 */
#define TSENSOR_CODE_MIN                    0     /* 温度最小值对应的码值 */

#define TSENSOR_1M_FREQ                     1000000

typedef struct {
    td_s16 high_temp;
    td_u16 high_code;
    td_s16 low_temp;
    td_u16 low_code;
} tsensor_calibration_t;

typedef struct {
    uintptr_t               base_addr;
    td_u32                  irq_num;
    tsensor_calibration_t   calibration;
} tsensor_cfg_t;

td_u32 uapi_tsensor_read_temperature(td_s16 *temperature);

#ifdef __cplusplus
}
#endif
#endif

#endif