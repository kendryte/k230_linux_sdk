/*
 * Copyright (c) CompanyNameMagicTag 2022-2023. All rights reserved.
 * Description: Header file of HAL external public interfaces .
 * Create: 2022-2-19
 */

#ifndef __HAL_EXT_IF_DEVICE_H__
#define __HAL_EXT_IF_DEVICE_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "hal_ext_if_device_rom.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*****************************************************************************
  2 宏定义
*****************************************************************************/

#define HAL_TX_QEUEU_MAX_PPDU_NUM   2 /* DMAC也有一套，建议合并放入SPEC中 */


#define HAL_ANT_SWITCH_RSSI_HIGH_CNT             1000 /* 维持10帧都超过阈值则切换 */
#define HAL_ANT_SWITCH_RSSI_LOW_CNT              10

#define HAL_ANT_SWITCH_RSSI_MGMT_STRONG_TH       10  /* 强信号下管理帧ant0与ant1相差10dB时切换到SISO */
#define HAL_ANT_SWITCH_RSSI_MGMT_WEAK_TH         3   /* 弱信号下管理帧ant0与ant1相差3dB时切换到SISO */

#define HAL_ANT_SWITCH_RSSI_TBTT_CNT_TH          50   /* 50个tbtt中断触发一次探测 */
#define HAL_ANT_SWITCH_RSSI_MIMO_TBTT_OPEN_TH    50   /* 50个tbtt中断触发一次探测 */
#define HAL_ANT_SWITCH_RSSI_MIMO_TBTT_CLOSE_TH   10   /* 50个tbtt中断触发一次探测 */
#define HAL_ANT_SWITCH_RSSI_HT_DIFF_TH           10   /* HT协议下20M的差值门限，MCS12灵敏度-MCS7灵敏度 */
#define HAL_ANT_SWITCH_RSSI_VHT_DIFF_TH          12   /* VHT协议下20M的差值门限，双流MCS4灵敏度-单流MCS9灵敏度 */
#define HAL_ANT_SWITCH_RSSI_HT_MIN_TH           (-86)  /* HT协议下的最小值门限，MCS11的灵敏度+3db */
#define HAL_ANT_SWITCH_RSSI_VHT_MIN_TH          (-82)  /* VHT协议下的最小值门限，双流MCS4的灵敏度+3db */

#define HAL_CCA_OPT_ED_LOW_TH_DSSS_DEF     (-76) /* CCA DSSS 检测门限寄存器默认值 */
#define HAL_CCA_OPT_ED_LOW_TH_DSSS_MIN     (-88) /* CCA DSSS 检测门限寄存器最小值 */
#define HAL_CCA_OPT_ED_LOW_TH_OFDM_DEF     (-82) /* CCA OFDM 检测门限寄存器默认值 */
#define HAL_CCA_OPT_ED_LOW_TH_OFDM_MIN     (-88) /* CCA OFDM 检测门限寄存器最小值 */

#define HAL_CCA_OPT_ED_HYST_20TH_DEF       (-62)        /* CCA 20M 空闲概率检测门限 */
#define HAL_CCA_OPT_ED_HYST_40TH_DEF       (-59)        /* CCA 40M 空闲概率检测门限 */

#define HAL_CCA_OPT_ED_HYST_STEP_20TH_DEF       (2)        /* CCA 20M 空闲概率检测安全门限 */
#define HAL_CCA_OPT_ED_HYST_STEP_40TH_DEF       (2)        /* CCA 40M 空闲概率检测安全门限 */

/*****************************************************************************
  8 UNION定义
*****************************************************************************/

/* MIMO/SISO状态机状态枚举 */
typedef enum {
    HAL_M2S_STATE_IDLE = 0,         /* 初始运行状态 */
    HAL_M2S_STATE_SISO = 1,         /* SISO状态,软件和硬件都是单通道 */
    HAL_M2S_STATE_MIMO = 2,         /* MIMO状态,软件和硬件都是双通道 */
    HAL_M2S_STATE_MISO = 3,         /* MISO探测态,软件单通道，硬件双通道 */
    HAL_M2S_STATE_SIMO = 4,         /* SIMO状态,软件双通道，硬件单通道 */

    HAL_M2S_STATE_BUTT              /* 最大状态 */
} hal_m2s_state;
typedef osal_u8 hal_m2s_state_uint8;

/* MIMO/SISO状态机事件枚举 */
typedef enum {
    /* 1.RSSI模块 */
    HAL_M2S_EVENT_ANT_RSSI_MIMO_TO_MISO_C0,
    HAL_M2S_EVENT_ANT_RSSI_MIMO_TO_MISO_C1,
    HAL_M2S_EVENT_ANT_RSSI_MISO_C1_TO_MISO_C0,
    HAL_M2S_EVENT_ANT_RSSI_MISO_C0_TO_MISO_C1,
    HAL_M2S_EVENT_ANT_RSSI_MISO_TO_MIMO,

    /* 2.test模块 */
    HAL_M2S_EVENT_TEST_MIMO_TO_SISO_C0 = 10,
    HAL_M2S_EVENT_TEST_MIMO_TO_SISO_C1,
    HAL_M2S_EVENT_TEST_MIMO_TO_MISO_C0,    /* MISO硬件是mimo的，miso要切分c0还是c1，便于切siso时区分出c0还是c1 */
    HAL_M2S_EVENT_TEST_MIMO_TO_MISO_C1,
    HAL_M2S_EVENT_TEST_MISO_TO_MIMO,       /* miso此时不区分c0或者c1 */
    HAL_M2S_EVENT_TEST_SISO_TO_MIMO,
    HAL_M2S_EVENT_TEST_SISO_TO_MISO_C0,
    HAL_M2S_EVENT_TEST_SISO_TO_MISO_C1,
    HAL_M2S_EVENT_TEST_SISO_C0_TO_SISO_C1,
    HAL_M2S_EVENT_TEST_SISO_C1_TO_SISO_C0,
    HAL_M2S_EVENT_TEST_MISO_TO_SISO_C0 = 20,
    HAL_M2S_EVENT_TEST_MISO_TO_SISO_C1,
    HAL_M2S_EVENT_TEST_MISO_C0_TO_MISO_C1,
    HAL_M2S_EVENT_TEST_MISO_C1_TO_MISO_C0,
    HAL_M2S_EVENT_TEST_IDLE_TO_SISO_C0,
    HAL_M2S_EVENT_TEST_IDLE_TO_SISO_C1,
    HAL_M2S_EVENT_TEST_IDLE_TO_MIMO,
    HAL_M2S_EVENT_TEST_IDLE_TO_MISO_C0,
    HAL_M2S_EVENT_TEST_IDLE_TO_MISO_C1,

    /* 3.BT模块 */
    HAL_M2S_EVENT_BT_MIMO_TO_SISO_C1 = 40,
    HAL_M2S_EVENT_BT_SISO_TO_MIMO,
    HAL_M2S_EVENT_BT_SISO_C0_TO_SISO_C1,
    HAL_M2S_EVENT_BT_MISO_TO_SISO_C1,

    /* 4.hal device抛来的同步事件 */
    HAL_M2S_EVENT_IDLE_BEGIN = 50,
    HAL_M2S_EVENT_WORK_BEGIN,
    HAL_M2S_EVENT_SCAN_BEGIN,
    HAL_M2S_EVENT_SCAN_PREPARE,
    HAL_M2S_EVENT_SCAN_CHANNEL_BACK, // scan切回home channel事件
    HAL_M2S_EVENT_SCAN_END,

    /* 6.上层模块 */
    HAL_M2S_EVENT_COMMAND_MIMO_TO_SISO_C0 = 70,
    HAL_M2S_EVENT_COMMAND_MIMO_TO_SISO_C1,
    HAL_M2S_EVENT_COMMAND_MIMO_TO_MISO_C0,    /* MISO硬件是mimo的，miso要切分c0还是c1，便于切siso时区分出c0还是c1 */
    HAL_M2S_EVENT_COMMAND_MIMO_TO_MISO_C1,
    HAL_M2S_EVENT_COMMAND_MISO_TO_MIMO,       /* miso此时不区分c0或者c1 */
    HAL_M2S_EVENT_COMMAND_SISO_TO_MIMO,       /* siso此时不区分c0或者c1 */
    HAL_M2S_EVENT_COMMAND_SISO_TO_MISO_SCAN_BEGIN,  /* 专门用于并发扫描未开，mss下的特殊扫描,当前是c0 siso mss */
    HAL_M2S_EVENT_COMMAND_MISO_TO_SISO_SCAN_END,  /* 专门用于并发扫描未开，mss下的特殊扫描,当前是c0 siso mss */
    HAL_M2S_EVENT_COMMAND_SISO_C0_TO_SISO_C1,
    HAL_M2S_EVENT_COMMAND_SISO_C1_TO_SISO_C0,
    HAL_M2S_EVENT_COMMAND_MISO_TO_SISO_C0,
    HAL_M2S_EVENT_COMMAND_MISO_TO_SISO_C1,
    HAL_M2S_EVENT_COMMAND_MISO_C0_TO_MISO_C1,
    HAL_M2S_EVENT_COMMAND_MISO_C1_TO_MISO_C0,
    HAL_M2S_EVENT_COMMAND_IDLE_TO_SISO_C0,
    HAL_M2S_EVENT_COMMAND_IDLE_TO_SISO_C1,
    HAL_M2S_EVENT_COMMAND_IDLE_TO_MIMO,
    HAL_M2S_EVENT_COMMAND_IDLE_TO_MISO_C0,
    HAL_M2S_EVENT_COMMAND_IDLE_TO_MISO_C1,

    /* 7.SISO静态启动模块 */
    HAL_M2S_EVENT_CUSTOM_MIMO_TO_SISO_C0 = 100,   /* 当前蓝牙使用主天线，静态启动默认启动到C0上;开关wifi才能恢复 */
    /* 优先级最高，AP模式下使用，不涉及dbdc，mss和test返回，rssi不涉及，并发扫描返回；蓝牙来的话，切C1siso */
    HAL_M2S_EVENT_CUSTOM_SISO_C0_TO_SISO_C1,
    HAL_M2S_EVENT_CUSTOM_SISO_C1_TO_SISO_C0,      /* 保持custom优先级最高，根据蓝牙业务触发切c0还是c1 siso */

    /* 8.SISO硬件规格静态启动模块, 不允许蓝牙等业务切换 */
    HAL_M2S_EVENT_SPEC_MIMO_TO_SISO_C0 = 110,     /* 硬件规格,从双天线工作到单天线工作 */
    HAL_M2S_EVENT_SPEC_MIMO_TO_SISO_C1,           /* 硬件规格,从双天线工作到单天线工作 */
    HAL_M2S_EVENT_SPEC_SISO_TO_MIMO,              /* 硬件规格,从单天线工作到双天线工作 */
    HAL_M2S_EVENT_SPEC_MISO_TO_SISO_C0,           /* 硬件规格,从双天线工作到单天线工作 */
    HAL_M2S_EVENT_SPEC_SISO_C1_TO_SISO_C0,   /* 2g c1 siso切换到5g c0 siso，异频dbac的处理方式，其他场景去关联要先回mimo */

    /* 初始化 */
    HAL_M2S_EVENT_FSM_INIT = 255,

    HAL_M2S_EVENT_BUTT
} hal_m2s_event_tpye;
typedef osal_u16 hal_m2s_event_tpye_uint16;

#ifdef _PRE_WLAN_FEATURE_DAQ
typedef struct {
    osal_u32 diag_basic_start_addr;
    osal_u32 diag_basic_end_addr;
    osal_u32 diag_sample_addr;
    osal_u32 diag_mode;
    osal_u32 diag_num;
    osal_u32 diag_cycle;
    osal_u32 diag_read_addr;
    osal_u32 diag_source;
    osal_u32 diag_phy_event_en;
    osal_u32 diag_phy_event_cnt;
} hal_mac_test_diag_stru;

typedef struct  {
    hal_mac_test_diag_stru diag_param;
} hal_to_dmac_device_mac_test_stru;

/* 设置用户配置参数 */
typedef struct {
    osal_s32 function_index;
    osal_s32 value;
    osal_s32 mode;
} mac_cfg_mct_set_diag_stru;
enum {
    HAL_DIAG_TEST_FUNCTION_INDEX0 = 0,
    HAL_DIAG_TEST_FUNCTION_INDEX1,
    HAL_DIAG_TEST_FUNCTION_INDEX2,
    HAL_DIAG_TEST_FUNCTION_INDEX12 = 12, // 0-12 MAC数采
    HAL_DIAG_TEST_FUNCTION_INDEX20 = 20,
    HAL_DIAG_TEST_FUNCTION_INDEX50 = 50, // 20-50 ADC数采 待定
    HAL_DIAG_TEST_FUNCTION_INDEX100 = 100,
    HAL_DIAG_TEST_FUNCTION_INDEX500 = 500
};
enum {
    HAL_TEST_DIAG_PHY_SAMPLE_MODE_FIX_LEN,
    HAL_TEST_DIAG_PHY_SAMPLE_MODE_DEBUG_WITHOUT_DELAY,
    HAL_TEST_DIAG_PHY_SAMPLE_MODE_DEBUG_WITH_DELAY,
    HAL_TEST_DIAG_PHY_SAMPLE_MODE_DEBUG_FIX_LEN,
    HAL_TEST_DIAG_PHY_SAMPLE_MODE_BUTT
};
enum {
    HAL_TEST_DIAG_PHY_SAMPLE_WITHOUT_MAC,
    HAL_TEST_DIAG_PHY_SAMPLE_WITH_MAC_TSF_ONLY,
    HAL_TEST_DIAG_PHY_SAMPLE_WITH_MAC_INFO_ALL,
    HAL_TEST_DIAG_PHY_SAMPLE_MAC_BUTT
};
enum {
    HAL_TEST_DIAG_PHY_SAMPLE_CFG_MODE_RECOVERY,
    HAL_TEST_DIAG_PHY_SAMPLE_CFG_MODE_BEGIN,
    HAL_TEST_DIAG_PHY_SAMPLE_CFG_MODE_QUERY,
    HAL_TEST_DIAG_PHY_SAMPLE_CFG_MODE_SAVE,
    HAL_TEST_DIAG_PHY_SAMPLE_CFG_MODE_BUTT
};
typedef struct {
    osal_u32 cfg_sample_node_sel : 16;          // 选择PHY内部节点 参考节点信号说明
    osal_u32 cfg_sample_length : 16;            // 配置数采长度 单位: 4Byte
    osal_u32 cfg_debug_sample_len : 1;          // 配置触发定长数采模式 1: 触发定长 可以不需要
    osal_u32 cfg_with_sample_delay_en : 1;      // 配置触发延时终止
    osal_u32 cfg_debug_sample_start_mode : 6;   // debug模式数采开始条件选择
    osal_u32 cfg_debug_sample_end_mode : 6;     // debug模式数采结束条件选择
    osal_u32 cfg_debug_sample_delay : 8;        // 配置延时样点个数
    osal_u32 cfg_with_mac_tsf_en : 1;           // 配置同时采样mac tsf信息 1: 开启
    osal_u32 cfg_with_mac_info_en : 1;          // 配置同时采样mac信息 1: 开启
    osal_u32 cfg_sample_mode : 3;               // 数采模式 0: 定长 1: 触发即刻停止 2: 触发延时停止 3: 触发定长
    osal_u32 cfg_op_mode : 2;                   // 数采操作模式 0:执行关闭 1:执行开启 2:查询执行情况
    osal_u32 extend_mode : 1;                   // 拓展模式 支持tsf mac以及延时配置 0: 不支持(liteos)  1: 支持(linux)
    osal_u32 resv : 2;
    osal_u32 event_rpt_addr;                    // phy事件上报地址
    osal_u32 save_msg_addr;                     // 保存消息的地址
} hal_to_phy_test_diag_stru;
#endif

typedef struct  {
    /* tx dscr tx_count0/1/2/3 */
    osal_u16 tx_count0 : 3,
           tx_count1 : 3,
           tx_count2 : 3,
           tx_count3 : 3,
           rsv : 4;
}hal_hw_retry_num_stru;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of hal_ext_if_device.h */
