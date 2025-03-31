/*
 * Copyright (c) CompanyNameMagicTag 2020-2020. All rights reserved.
 * Description: HAL Layer Specifications.
 * Create: 2020-7-3
 */

#ifndef __HAL_OPS_COMMON_ROM_H__
#define __HAL_OPS_COMMON_ROM_H__

/*****************************************************************************
    1 头文件包含
*****************************************************************************/
#include "frw_list_rom.h"
#include "frw_util_common_rom.h"
#include "oal_fsm.h"
#include "hal_commom_ops_type_rom.h"
#include "hal_common_ops_device_rom.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define INI_SUCC                (0)
#define INI_FAILED              (-1)

/*****************************************************************************
    加密相关枚举定义
*****************************************************************************/
typedef struct {
    hal_fcs_protect_type_enum_uint8 protect_type;
    hal_fcs_protect_cnt_enum_uint8  protect_cnt;
    osal_u16                          protect_coex_pri : 2;     /* btcoex下使用，one pkt发送优先级 */
    osal_u16                          cfg_one_pkt_tx_vap_index : 4;
    osal_u16                          cfg_one_pkt_tx_peer_index : 5;
    osal_u16                          bit_rsv : 5;

    osal_u32                          tx_mode;
    osal_u32                          tx_data_rate;
    osal_u16                          duration;    /* 单位 us */
    osal_u16                          timeout;

    osal_u16                          wait_timeout;     /* 软件定时器超时时间 */
    osal_u8                           rsv[2];      /* 保留2字节对齐 */
    osal_u8                             protect_frame[24]; // 24 todo mac_vap_ext hmac_only
} hal_one_packet_cfg_stru;

typedef struct {
    hal_fcs_protect_type_enum_uint8 protect_type;
    osal_u8 vap_id;    /* hal vap id */
    osal_u8 status;    /* 0:成功 1:接收响应帧失败 2:接收响应帧超时 3:异常结束 */
    osal_u8 timeout;   /* 最后一次是否发送超时 */
    osal_u8 tx_cnt;    /* 发送次数 */
    osal_u8 resv;
    osal_u16 tx_time;  /* 硬件发送时间 */
} hal_one_packet_debug_stru;

/*****************************************************************************
  结构名  : hmac_tx_dscr_queue_header_stru
  结构说明: 发送描述符队列头的结构体
*****************************************************************************/
typedef struct {
    struct osal_list_head                     header;          /* 发送描述符队列头结点 */
    hal_dscr_queue_status_enum_uint8        queue_status;    /* 发送描述符队列状态 */
    osal_u8                               ppdu_cnt;     /* 发送描述符队列中元素的个数 */
    osal_u8                               mpdu_cnt;     /* 发送描述符队列中mpdu总数，聚合场景，多个mpdu组成一个ppdu */
    osal_u8                               auc_resv[1];   /* 保留2字节对齐 */
#ifdef _PRE_WLAN_DFR_STAT
    osal_u32                              enqueue_total_cnt;   /* 统计周期内入队的数量 */
#endif
} hal_tx_dscr_queue_header_stru;
/*****************************************************************************/
/*****************************************************************************/
/*                        hh503 产品宏定义、枚举                            */
/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************
    2 宏定义
*****************************************************************************/
#define hal_public_hook_func(_func)     hh503##_func

#define HAL_COEX_SW_IRQ_BT BIT0
/*****************************************************************************
  3 枚举
*****************************************************************************/
/* 算法描述符中RTS开关枚举 */
typedef enum {
    WLAN_PROTECT_MODE_NONE = 0,
    WLAN_PROTECT_MODE_RTS_CTS = 1,
    WLAN_PROTECT_MODE_SELF_CTS = 2,
    WLAN_PROTECT_MODE_BUTT
} wlan_protect_mode_enum;
typedef osal_u8 wlan_protect_mode_enum_uint8;

/*****************************************************************************/
/*                        公共宏定义、枚举、结构体                            */
/*****************************************************************************/
#define hal_pow_lpf_base_idx(_en_band)      (((_en_band) == WLAN_BAND_2G) ? HAL_POW_2G_LPF_BASE_IDX : \
    HAL_POW_5G_LPF_BASE_IDX)  /* 基准LPF Index */
#define hal_pow_dac_base_idx(_en_band)      (((_en_band) == WLAN_BAND_2G) ? HAL_POW_2G_DAC_BASE_IDX : \
    HAL_POW_5G_DAC_BASE_IDX)  /* 基准DAC Index */

/* Bit[22:22]: DPD enable; Bit[21:12]: delt dbb scaling; Bit[11:10]: DAC gain; Bit[9:8]: PA;
   Bit[7:5]: LPF gain ; Bit[4:4]: UPC gain ; Bit[3:2]: CFR index; Bit[1:0]: dpd_tpc_lv */
#define hal_pow_code_comb(_en_dpd_enable, _uc_upc_idx, _uc_pa_idx, _uc_dac_idx, _uc_lpf_idx, \
    _uc_delta_dbb_scaling, _uc_dpd_tpc_lv, _uc_cfr_index) \
    ((((osal_u32)(_uc_dpd_tpc_lv) & 0x3) | (((osal_u32)(_uc_cfr_index) & 0x3) << 2) | \
      (((osal_u32)(_uc_upc_idx) & 0x1) << 4) | (((osal_u32)(_uc_lpf_idx) & 0x7) << 5) | \
      (((osal_u32)(_uc_pa_idx) & 0x3) << 8) | (((osal_u32)(_uc_dac_idx) & 0x3) << 10) | \
      (((osal_u32)(_uc_delta_dbb_scaling) & 0x3ff) << 12) | (((osal_u32)(_en_dpd_enable) & 0x1) << 22)) & 0x7fffff)

#define hal_ini_pow_code(_en_band)    (hal_pow_code_comb(OSAL_TRUE, HAL_POW_UPC_LOW_START_IDX, HAL_POW_PA_BASE_IDX, \
    hal_pow_dac_base_idx(_en_band), hal_pow_lpf_base_idx(_en_band), HAL_POW_DELTA_DBB_SCAL_BASE_IDX, \
    HAL_POW_DPD_TPC_BASE_IDX, HAL_POW_CFR_BASE_IDX))

#define hal_pow_fill_tx_four_same_code(_uc_code)    \
    ((((osal_u32)(_uc_code)) << 24) | (((osal_u32)(_uc_code)) << 16) | \
     (((osal_u32)(_uc_code)) << 8) | ((osal_u32)(_uc_code)))

#define hal_pow_fill_upc_data_reg(_uc_data1, _uc_data2, _uc_data3, _uc_data4)    \
    (((osal_u32)((_uc_data1)<<(osal_u32)24)) | ((osal_u32)((_uc_data2)<<(osal_u32)16)) | \
    ((osal_u32)(_uc_data3)<<(osal_u32)8) | (_uc_data4))

#define hal_abs(_a)  (((_a) > 0) ? (_a) : (-(_a)))

#define hal_pow_find_near_idx(_uc_data1, _uc_data2, _uc_idx1, _uc_idx2, _uc_input)    \
    ((hal_abs((_uc_data1) - (_uc_input))) > (hal_abs((_uc_data2)-(_uc_input))) ? (_uc_idx2) : (_uc_idx1))

#define get_rssi_min(_c_rssi_ant0, _c_rssi_ant1)  \
    (((_c_rssi_ant0) == OAL_RSSI_INIT_VALUE) ? (_c_rssi_ant1) : \
     ((_c_rssi_ant1) == OAL_RSSI_INIT_VALUE) ? (_c_rssi_ant0) : \
     osal_min((_c_rssi_ant0), (_c_rssi_ant1)))

#define get_rssi(_c_rssi_dbm, _c_rssi_ant0, _c_rssi_ant1) get_rssi_min(_c_rssi_ant0, _c_rssi_ant1)

/* hal device下挂接收描述符控制相关宏定义 */
#define get_dev_rx_dscr_q_empty(_pst_hal_dev, _uc_queue_id) \
    (((alg_rx_dscr_ctl_alg_info_stru *)((_pst_hal_dev)->rx_dscr_ctl.rx_dscr_ctl_alg_info + \
    (_uc_queue_id)))->rx_dscr_q_empty)
#define get_dev_rx_dscr_max_used_cnt(_pst_hal_dev, _uc_queue_id) \
    (((alg_rx_dscr_ctl_alg_info_stru *)((_pst_hal_dev)->rx_dscr_ctl.rx_dscr_ctl_alg_info + \
    (_uc_queue_id)))->rx_max_dscr_used_cnt)
#define get_dev_rx_dscr_event_cnt(_pst_hal_dev, _uc_queue_id)    \
    (((alg_rx_dscr_ctl_alg_info_stru *)((_pst_hal_dev)->rx_dscr_ctl.rx_dscr_ctl_alg_info + \
    (_uc_queue_id)))->rx_event_cnt)
#define get_dev_rx_dscr_event_pkts(_pst_hal_dev, _uc_queue_id)   \
    (((alg_rx_dscr_ctl_alg_info_stru *)((_pst_hal_dev)->rx_dscr_ctl.rx_dscr_ctl_alg_info + \
    (_uc_queue_id)))->rx_event_pkts_sum)
#define get_dev_rx_dscr_avrage_pkts(_pst_hal_dev, _uc_queue_id)  \
    ((get_dev_rx_dscr_event_pkts(_pst_hal_dev, _uc_queue_id)) / (get_dev_rx_dscr_event_cnt(_pst_hal_dev, _uc_queue_id)))
#define get_dev_rx_dscr_avrg_pkts(_pst_hal_dev, _uc_queue_id)    \
    ((get_dev_rx_dscr_event_cnt(_pst_hal_dev, _uc_queue_id) == 0) ? 0 : \
    (get_dev_rx_dscr_avrage_pkts(_pst_hal_dev, _uc_queue_id)))
#define get_dev_rx_dscr_smooth_dscr_pkts(_pst_hal_dev, _uc_queue_id) \
    (((alg_rx_dscr_ctl_alg_info_stru *)((_pst_hal_dev)->rx_dscr_ctl.rx_dscr_ctl_alg_info + \
    (_uc_queue_id)))->rx_dscr_smooth_event_pkts)
#define get_dev_rx_dscr_smooth_avr_dscr(_pst_hal_dev, _uc_queue_id) \
    (((alg_rx_dscr_ctl_alg_info_stru *)((_pst_hal_dev)->rx_dscr_ctl.rx_dscr_ctl_alg_info + \
    (_uc_queue_id)))->rx_dscr_smooth_avr_dscr)

#define get_dev_max(_us_ualue, _us_thres)        \
    ((_us_ualue) = ((_us_ualue) < (_us_thres)) ? (_us_thres) : (_us_ualue))

#define get_dev_rx_dscr_reset_static(_pst_hal_dev, _uc_queue_id) \
    do { \
        get_dev_rx_dscr_max_used_cnt(_pst_hal_dev, _uc_queue_id) = 0; \
        get_dev_rx_dscr_event_cnt(_pst_hal_dev, _uc_queue_id) = 0; \
        get_dev_rx_dscr_event_pkts(_pst_hal_dev, _uc_queue_id) = 0; \
    } while (0)
#define get_dev_rx_dscr_reset_smooth(_pst_hal_dev, _uc_queue_id) \
    do { \
        get_dev_rx_dscr_smooth_avr_dscr(_pst_hal_dev, _uc_queue_id) = 0; \
        get_dev_rx_dscr_smooth_dscr_pkts(_pst_hal_dev, _uc_queue_id) = 0; \
    } while (0)
/* 获取tpc档位对应的目标功率增益 */
#define hal_dev_get_idx_target_pow_gain(_pst_hal_dev, _uc_pow_idx, _en_freq_band) \
    (((hal_to_dmac_device_stru *)(_pst_hal_dev))->tx_pwr_ctrl.tpc_lvl_vs_gain_ctrl->as_pow_level_table[(_uc_pow_idx)] \
        [(_en_freq_band)])

#define ops_min(a, b)           (((a) < (b)) ? (a) : (b))

/* 计算PAPA=5.5时，对应的功率补偿值 单位0.1dBm */
/* 根据实验室测试数据，一次拟合得到计算功率偏差公式0.25+0.03*delt_power，推导对应6
    M&MCS0有如下功率补偿值计算公式，小于-0.5dB时不需要补偿 */
#define hal_dev_cfr_get_compsen_by_deltpow(_c_delt_pow) ((osal_u8)(((_c_delt_pow) < -5) ? 0 : \
    (((3*(_c_delt_pow) + 25) + 5) / 10)))


/* 检查是否为11b 1M速率 */
#define hal_phy_11b_1m_rate(_a, _b)  (((_a) == WLAN_11B_PHY_PROTOCOL_MODE) && ((_b) == 0))

#ifdef _PRE_WLAN_FEATURE_NEGTIVE_DET
/* 干扰检测模块PK模式探测阶段枚举 */
typedef enum {
    HAL_ALG_INTF_PKADJ_STAGE_INIT      = 0,     /* 用于统计跑流的初始干扰 */
    HAL_ALG_INTF_PKADJ_STAGE_TXOP      = 1,
    HAL_ALG_INTF_PKADJ_STAGE_AIFSN     = 2,
    HAL_ALG_INTF_PKADJ_STAGE_CW        = 3,

    HAL_ALG_INTF_PK_STAGE_BUTT
} hal_alg_intf_pk_stage_enum;
typedef osal_u8 hal_alg_pk_intf_stage_enum_uint8;
#endif

/*****************************************************************************
    7.1 基准发送描述符定义
*****************************************************************************/
typedef struct tag_hal_tx_dscr_stru {
    struct osal_list_head                 entry;
    osal_void                         *skb_start_addr;         /* Sub MSDU 0 Skb Address */
    osal_u16                          original_mpdu_len;       /* mpdu长度 含帧头 */
    hal_tx_queue_type_enum_uint8        q_num;                   /* 发送队列队列号 */
    osal_u8                           is_retried         : 1; /* 是不是重传包 */
    osal_u8                           is_ampdu           : 1; /* 是不是ampdu */
    osal_u8                           is_rifs            : 1; /* 是不是rifs发送 */
    osal_u8                           is_first           : 1; /* 标志是否是第一个描述符 */
    osal_u8                           bit_resv           : 4;
    osal_u8                           data[0];
} hal_tx_dscr_stru;

/*****************************************************************************
  7.2 基准接收描述符定义
*****************************************************************************/
typedef struct tag_hal_rx_dscr_stru {
    struct osal_list_head         entry;
    osal_void                 *skb_start_addr;          /* 描述符中保存的netbuf的首地址 */
    osal_u8                   data[0];
} hal_rx_dscr_stru;

typedef struct {
    osal_u32                              mac_hdr_start_addr;
    osal_void                             *skb_start_addr;
} hal_tx_mpdu_address_params_stru;

typedef struct {
    /* tx dscr中算法填写的参数 */
    hal_tx_phy_mode_one_stru           phy_info;                            /* 第13行(HY TX MODE 1) */
    hal_tx_ctrl_desc_rate_stru           tx_dsrc[HAL_TX_RATE_MAX_NUM];  /* 第14(PHY TX RATA 1) 19 20 21 行 */
    hal_tx_txop_tpc_stru                 tpc;                             /* 第24 25 26 27行 */
    osal_u8                              rts_rate[HAL_TX_RATE_MAX_NUM];
    osal_u32                             self_cts_protect : 1;
    osal_u32                             rts_tpc_rank : 2;
    osal_u32                             res_ : 29;
#if defined _PRE_WLAN_PLAT_URANUS && !defined _PRE_WLAN_PLAT_URANUS_CS
    hal_tx_txop_tx_power_stru            tx_power;
#endif
}hal_tx_txop_alg_stru;

typedef struct {
    osal_u8                            retry_num;
    osal_u8                            rts_succ;
    osal_u8                            cts_failure;
    osal_s8                             last_ack_rssi;
    osal_u8                            mpdu_num;
    osal_u8                            error_mpdu_num;
    osal_u8                            last_rate_rank;
    osal_u8                            tid;
    hal_tx_queue_type_enum_uint8         ac;
    osal_u16                           mpdu_len;
    osal_u8                            is_retried;
    osal_u8                            bandwidth;
    osal_u8                            sounding_mode;           /* 表示该帧sounding类型 */
    osal_u8                            status;                  /* 该帧的发送结果 */
    osal_u8                            ampdu_enable;            /* 表示该帧是否为AMPDU聚合帧 */
    osal_u16                           origin_mpdu_lenth;       /* mpdu长度 */
    osal_u8                            last_tb_flag;            /* tb发送的标记 */

    osal_u64                           ampdu_result;
    hal_channel_matrix_dsc_stru          tx_dsc_chnl_matrix;      /* 发送描述符中的信道矩阵信息 */
    hal_tx_ctrl_desc_rate_stru    per_rate[HAL_TX_RATE_MAX_NUM];
    osal_u32                           ampdu_length;
#if defined _PRE_WLAN_PLAT_URANUS && !defined _PRE_WLAN_PLAT_URANUS_CS
    hal_tx_txop_tx_power_stru        tx_power;
#endif
    hal_tx_txop_tpc_stru             tpc;

    osal_u32                           now_time_ms;
    osal_u8                            normal_pkt_num;
    osal_u8                            tx_desc_rate_rank; /* 发送成功时选择的速率等级，status=1时有效 */
    osal_u8                            first_succ_cnt;    /* 第一次发送速率级别成功的帧数 */
    osal_u8                            first_fail_cnt;    /* 第一次发送速率级别失败的帧数 */
} hal_tx_dscr_ctrl_one_param;

/*****************************************************************************
  结构名  : hal_user_pow_info_stru
  结构说明: HAL模块USER级别功率结构
*****************************************************************************/
typedef struct {
    hal_rate_pow_code_gain_table_stru *rate_pow_table; /* EVM功率表单结构体指针 */
    hal_tx_txop_alg_stru              *txop_param;     /* 用户速率描述符信息结构体 */
} hal_user_pow_info_stru;

/* Beacon帧发送参数 */
typedef struct {
    uintptr_t               pkt_ptr;  /* 64位OS上用osal_u32会导致pkt_ptr截断 用uintptr_t则不会 */
    osal_u32                pkt_len;
    hal_tx_txop_alg_stru   *tx_param;
    osal_u32                tx_chain_mask;
} hal_beacon_tx_params_stru;

typedef struct {
    osal_u16 partial_aid : 9,   /* partial_aid */
             ax_bss_color : 6,
             txop_ps_not_allowed : 1;
    osal_u8  tx_vap_idx : 2,
             ra_lut_index : 3,
             peer_ps_chk_disable : 1,
             lsig_txop : 1,
             obss_pd_tx_disable : 1;
    osal_u8 retry_flag_hw_bypass : 1,
            duration_hw_bypass : 1,
            timestamp_hw_bypass : 1,
            tx_pn_hw_bypass : 1,
            tx_addba_ssn_bypass : 1,
            long_nav_enable : 1,
            tx_fcs_gen_err_en : 1,
            pdet_en : 1;

    osal_u8 cipher_key_type;      /* 密钥ID/类型 */
    osal_u8 cipher_protocol_type;
    osal_u8 cipher_key_id;
    osal_u8 seq_ctl_hw_bypass : 1,
            resrv_1 : 7;

    osal_u8 ftm_protocol_mode : 4,     /* ftm */
            ftm_bandwidth : 4;
    osal_u8 ftm_cali_en : 1,
            is_ftm : 2,
            ftm_chain : 2,
            resrv_2 : 3;
} hal_tx_mpdu_ctl;

/*****************************************************************************
  结构名  : hal_tx_mpdu_stru
  结构说明: DMAC模块MPDU发送控制结构
*****************************************************************************/
typedef struct {
    /* 从11MAC帧头中获取 针对MPDU */
    hal_wmm_txop_params_stru              wmm;
    hal_tx_mpdu_mac_hdr_params_stru       mpdu_mac_hdr;                          /* 第12 17行(PHY TX MODE 2) */
    hal_tx_mpdu_address_params_stru       mpdu_addr;                             /* 第18行(MAC TX MODE 2) */
    hal_tx_msdu_address_params            msdu_addr[WLAN_DSCR_SUBTABEL_MAX_NUM]; /* 第24,25...行 */
    hal_tx_mpdu_ctl                     mpdu_ctl;
    osal_u16                            mpdu_len;                              /* 发送帧的长度,包括head */
    osal_u16                            resv;
} hal_tx_mpdu_stru;

/*****************************************************************************
  结构名  : hal_rx_dscr_queue_header_stru
  结构说明: 接收描述符队列头的结构体
*****************************************************************************/
typedef struct {
    struct osal_list_head                  header;             /* 发送描述符队列头结点 */
    osal_u16                              element_cnt;        /* 接收描述符队列中元素的个数 */
    hal_dscr_queue_status_enum_uint8    queue_status;       /* 接收描述符队列的状态 */
    osal_u8                               available_cnt;      /* 当前队列中硬件可用描述符个数 */
} hal_rx_dscr_queue_header_stru;

/*****************************************************************************
    结构名  : hal_cfg_rts_tx_param_stru
    结构说明: RTS设置发送参数
*****************************************************************************/
typedef struct {
    wlan_legacy_rate_value_enum_uint8   rate[HAL_TX_RATE_MAX_NUM];      /* 发送速率，单位mpbs */

    /* 协议模式, 取值参见wlan_phy_protocol_enum_uint8 */
    wlan_phy_protocol_enum_uint8        protocol_mode[HAL_TX_RATE_MAX_NUM];
    wlan_channel_band_enum_uint8        band;
    osal_u8                             recv[3];                        /* 保留3字节对齐 */
} hal_cfg_rts_tx_param_stru;

/*****************************************************************************
  7.7 对外部保留的设备级接口列表，建议外部不做直接调用，而是调用对应的内联函数
*****************************************************************************/
typedef osal_void (* p_hal_alg_isr_func)(osal_u8 vap_id, osal_void *p_hal_to_dmac_device);
typedef osal_void (* p_hal_gap_isr_func)(osal_u8 vap_id, osal_void *p_hal_to_dmac_device);
typedef osal_u32 (*p_tbtt_ap_isr_func)(osal_u8 mac_vap_id);

/* 保存硬件上报接收中断信息结构 */
typedef struct {
    struct osal_list_head         dlist_head;
    osal_u32                 *base_dscr;      /* 本次中断描述符地址 */
    osal_u16                  dscr_num;        /* 接收到的描述符的个数 */
    osal_u8                   channel_num;     /* 本次中断时，所在的信道号 */
    osal_u8                   queue_id;
} hal_hw_rx_dscr_info_stru;

typedef struct {
    frw_timeout_stru                 tx_prot_timer;        /* 检测无发送完成中断定时器 */
    hal_dfr_timer_step_enum_uint8    tx_prot_timer_step;   /* 超时标志，表明第几次超时 */
    osal_u16                         tx_prot_time;         /* 超时时间 */
    osal_u8                          auc_resv[1];
} hal_dfr_tx_prot_stru;

/*****************************************************************************
  结构名  : hal_phy_pow_param_stru
  结构说明: PHY 功率相关寄存器参数, 在2.4G/5G频点切换时使用
*****************************************************************************/
typedef struct {
    /* 不同帧的tpc code */
    osal_u32       ack_cts_pow_code[WLAN_OFDM_ACK_CTS_TYPE_BUTT][WLAN_2G_SUB_BAND_NUM + WLAN_5G_SUB_BAND_NUM];
    osal_u32       aul_2g_dsss_ack_cts_pow_code[WLAN_2G_SUB_BAND_NUM];
    osal_u32       rts_pow_code[WLAN_2G_SUB_BAND_NUM + WLAN_5G_SUB_BAND_NUM];
    osal_u32       one_pkt_pow_code[WLAN_2G_SUB_BAND_NUM + WLAN_5G_SUB_BAND_NUM];
    osal_u32       bar_pow_code[WLAN_2G_SUB_BAND_NUM + WLAN_5G_SUB_BAND_NUM];
    osal_u32       cfend_pow_code[WLAN_2G_SUB_BAND_NUM + WLAN_5G_SUB_BAND_NUM];
    osal_u32       vht_report_pow_code[WLAN_5G_SUB_BAND_NUM];
    /* 读取不同帧格式的速率 */
    osal_u8        rate_bar;
    osal_u8        rate_rts;
    osal_u8        rate_one_pkt;
    osal_u8        rate_abort_selfcts;
    osal_u8        rate_ofdm_ack_cts[WLAN_OFDM_ACK_CTS_TYPE_BUTT];
    osal_u8        rate_cfend;
    osal_u8        rate_vht_report;
    osal_u8        resv_2[2];
} hal_phy_pow_param_stru;

typedef struct {
    oal_bool_enum_uint8     aen_dyn_cali_chain_comp_flag[WLAN_RF_CHANNEL_NUMS];
    osal_u8                 tx_chain_sel;
    osal_u8                 auc_resv[1];
} hal_dyn_cali_chain_status_stru;
typedef struct hal_dyn_cali_val {
    osal_u16              aus_cali_en_cnt[WLAN_BAND_BUTT];
    osal_u16              aus_cali_en_interval[WLAN_BAND_BUTT];      /* 两次动态校准间隔描述符个数  */

    osal_s32              pdet_val;     /* 上报pdet值 */

    osal_u8               tpc_code;     /* 上报tpc code */
    osal_u8               pdet_chain   : 4;     /* 上报pdet的通道 */
    osal_u8               protocol     : 4;     /* 校准帧的协议模式 wlan_phy_protocol_enum */
    osal_u16              resv;
    hal_dyn_cali_chain_status_stru cali_chain_status_info;
} hal_dyn_cali_val_stru;

typedef struct {
    frw_timeout_stru             scan_timer;                             /* 扫描定时器用于切换信道 */
    mac_channel_stru             home_channel;                           /* 记录home channel */

    osal_u8                    original_mac_addr[WLAN_MAC_ADDR_LEN];     /* 扫描开始前保存原始的MAC地址 */
    wlan_scan_mode_enum_uint8    scan_mode;                              /* 扫描模式 */
    osal_u8                    scan_pause_bitmap;                        /* 扫描是否被暂停 */

    osal_u8                    start_chan_idx;                         /* 扫描起始idx */
    osal_u8                    scan_chan_idx;                          /* 当前扫描信道索引累加值 */
    osal_u8                    channel_nums;                           /* 此hal扫描的信道个数 */
    osal_u8                    scan_ap_num_in_2p4;                     /* 在2.4g上扫到ap的信道个数 */

    osal_u8                    last_channel_band;                      /* 上一个channel的频段 */

    /* 记录当前信道的扫描次数，第一次发送广播ssid的probe req帧，后面发送指
       定ssid的probe req帧 */
    osal_u8                    curr_channel_scan_count;
    osal_u8                    max_scan_count_per_channel;             /* 每个信道的扫描次数 */

    /* 间隔n个信道，切回工作信道工作一段时间 */
    osal_u8                    scan_channel_interval;

    /* 扫描在某一信道停留此时间后，扫描结束, ms，必须配置为MAC负载统计周期的整数倍 */
    osal_u16                   scan_time;
    osal_u16                   work_time_on_home_channel;  /* 背景扫描时，返回工作信道工作的时间 */

    osal_u32                   scan_timestamp;                         /* 记录最新一次扫描开始的时间 */

    osal_u8                    scan_ap_num_in_5p0;                     /* 在5g上扫到ap的信道个数 */
    osal_u8                    curr_scan_state               : 2; /* 此hal扫描状态 */
    osal_u8                    scan_curr_chan_find_bss_flag  : 1; /* 本信道是否扫到bss */

    /* 背景扫描时，扫描完一个信道，判断是否需要切回工作信道工作 */
    osal_u8                    need_switch_back_home_channel : 1;
    osal_u8                    working_in_home_chan          : 1; /* 是否工作在工作信道 */
    osal_u8                    scan_band                     : 2; /* 扫描的频段,支持fast scan使用 */
    osal_u8                    resv0                         : 1;
    hal_scan_tx_dscr_dbg_ctl_enum_uint8 tx_dscr_debug_ctl; /* 控制扫描时打印部分tx dscr */
    osal_u8                    resv1;
} hal_scan_params_stru;

struct tag_hal_to_dmac_device_stru;

typedef osal_void (*p_dmac_scan_one_channel_start_cb)(struct tag_hal_to_dmac_device_stru *hal_device,
    oal_bool_enum_uint8 is_scan_start);

typedef struct {
    p_dmac_scan_one_channel_start_cb         hmac_scan_one_channel_start;
} hal_device_fsm_cb;

/* hal device状态机结构体 */
typedef struct _hal_device_handle_handler {
    osal_u8                        cur_state;
    osal_u8                        prev_state;
    oal_fsm_stru                   oal_fsm;                    /* 状态机 */
    osal_u8                        is_fsm_attached;            /* 状态机是否已经注册 */
    frw_timeout_stru               check_timer;                /* 定时器 */
    osal_u8                        wait_hw_txq_empty    : 1;   /* 标记是否等待硬件队列为空后关闭mac pa */
    osal_u8                        mac_pa_switch_en     : 1;
    osal_u8                        bit_resv             : 6;
    osal_u8                        pm_vap_id;                  /* 执行休眠唤醒的hal vap id */
    hal_work_sub_state_uint8     sub_work_state;               /* pm状态,用于双rf下各rf状态同步 */

    osal_u32                       vap_pm_state_bitmap[HAL_WORK_SUB_STATE_NUM];     /* vap ps state bitmap */
    hal_device_fsm_cb            hal_device_fsm_cb;
} hal_dev_fsm_stru;

/* hal device m2s状态机结构体 */
typedef struct {
    oal_fsm_stru                     oal_fsm;          /* 状态机 */
    osal_u8                        is_fsm_attached;    /* 状态机是否已经注册 */
    wlan_m2s_type_enum_uint8         m2s_type;         /* 0:软切换 1:硬切换 */
    wlan_m2s_mode_stru               m2s_mode;         /* m2s业务申请占用bitmap，用户优先级管理 */
    oal_bool_enum_uint8              command_scan_on;  /* command下优化扫描是否在执行，防止miso时误end */
    osal_u8                        _rom[4];            /* 4字节储存rom地址 */
} hal_m2s_fsm_stru;

#ifdef _PRE_WLAN_FEATURE_M2S
typedef osal_void (*p_m2s_back_to_mimo_check_cb)(struct tag_hal_to_dmac_device_stru *hal_device);

typedef struct {
    p_m2s_back_to_mimo_check_cb    m2s_back_to_mimo_check;
} hal_device_m2s_mgr_cb;

typedef struct {
    /* 处于管理用户时，需要调整action方案 */
    wlan_m2s_mgr_vap_stru     m2s_blacklist[WLAN_M2S_BLACKLIST_MAX_NUM];
    osal_u8                     blacklist_bss_index;                       /* 黑名单MAC地址的数组下标 */
    osal_u8                     blacklist_bss_cnt;                         /* 黑名单用户个数 */
    osal_u8                     m2s_switch_protect  : 1; /* 是否使能m2s切换保护，针对sta模式 */

    /* 存在关联ap用户是blacklist none的，需要刷新标记为切换保持miso稳定态 */
    osal_u8                     delay_swi_miso_hold : 1;
    osal_u8                     mss_on              : 1; /* 是否使能mss上报和下发功能 */
    osal_u8                     resv                : 5;

    /* 最近一次cur rssi mgmt逻辑学习到的合适single_txchain */
    osal_u8                     rssi_mgmt_single_txchain;
    hal_device_m2s_mgr_cb     hal_device_m2s_mgr_cb;
} hal_device_m2s_mgr_stru;
#endif

#ifdef _PRE_WLAN_FEATURE_BTCOEX
typedef struct {
    frw_timeout_stru s2m_resume_timer;          /* 切回mimo状态超时等待定时器 */

    /* siso ap切换回mimo探测时间，防止音乐切歌造成太过频繁 */
    frw_timeout_stru bt_coex_s2m_siso_ap_timer;

    hal_coex_m2s_mode_bitmap_stru m2s_mode_bitmap; /* 申请siso切换的业务bitmap，目前是ldac=1 和6slot=2 */

    /* 申请切换回mimo的业务bitmap，目前是ldac=0 a2dp=0 和6slot=0 3者都满足 */
    hal_coex_s2m_mode_bitmap_stru s2m_mode_bitmap;

    /* 需要切回mimo的超时处理函数里面才用此bit来清m2s标记，防止l
       dac结束时，6slot来了又切siso流程 */
    hal_coex_s2m_mode_bitmap_stru s2m_wait_bitmap;

    osal_u8 siso_ap_excute_on : 1; /* siso ap切换siso执行标志 */
    osal_u8 m2s_6slot         : 1; /* 0: m2s; 1:开m2s */
    osal_u8 m2s_ldac          : 1;
    osal_u8 m2s_resv          : 5;
} hal_device_btcoex_mgr_stru;
#endif

typedef struct tag_hal_to_dmac_device_stru {
    hal_cfg_cap_info_stru  *cfg_cap_info;

    hal_rx_dscr_queue_header_stru rx_dscr_queue[HAL_RX_QUEUE_NUM];
    hal_tx_dscr_queue_header_stru tx_dscr_queue[HAL_TX_QUEUE_NUM];

    osal_u32 rx_machw_dscr_addr[HAL_RX_DSCR_QUEUE_ID_BUTT]; /* 接收硬件队列状态寄存器 */

    osal_u16 rx_normal_dscr_cnt;
    osal_u16 rx_small_dscr_cnt;

    hal_alg_stat_info_stru hal_alg_stat;  /* 算法使用的统计信息结构体 */

    /* 0: 关闭常发; 1:保留给RF测试; 2: ampdu聚合帧常发; 3:非聚合帧常发 */
    osal_u8 al_tx_flag            : 3;

    /* 0: 关闭常收; 1:保留给RF测试；2: ampdu聚合帧常收; 3:非聚合帧常收 */
    osal_u8 al_rx_flag            : 3;
    osal_u8 one_packet_st         : 1; /* 0表示DBC结束 1表示DBAC正在执行 */
    osal_u8 clear_fifo_st         : 1; /* 0表示无clear fifo状态，1表示clear fifo状态 */

    osal_u8 al_txrx_ampdu_num     : 7; /* 指示用于常发常收的聚合帧数目 */
    osal_u8 current_rx_list_index : 1;

    /* 分两次打印寄存器信息，0:上报soc寄存器的值,1:上报rf寄存器的值, */
    /* 2:上报MAC寄存器的值,3:补充 ABB_CALI_WL_CTRL 信息,4:上报phy寄存器的值 */
    osal_u8 reg_info_flag         : 5;
    osal_u8 sdp_init_flag         : 1; /* 0表示 wifi aware未初始化，1表示 wifi aware已初始化 */
    osal_u8 rsv                   : 2; /* 已删除, 算法用的是vap中的,当前创建的P2P 是处于CL/GO 模式 */

    osal_u8 current_chan_number;

    osal_u32                     al_tx_pkt_len;
    osal_void                    *alrx_netbuf; /* 记录常收时，描述符所共用的内存 */
    osal_u32                     rx_normal_mdpu_succ_num;
    osal_u32                     rx_ampdu_succ_num;
    osal_u32                     rx_ppdu_fail_num;
    struct osal_list_head       rx_isr_info_res_list; /* 接收中断信息存储资源链表 */

    osal_u8   p2p_hal_vap_idx;         /* 用于P2P 中断产生后指示对应设备的hal vap */
    osal_u8   sta_hal_vap_idx;         /* 用于STA 中断产生后指示对应设备的hal vap */
    osal_u8   ap_hal_vap_idx;          /* 用于softAP 中断产生后指示对应设备的hal vap */
    osal_u8   full_phy_freq_user_cnt;  /* device下需要满频的vap(ap)/sta(user) 个数 */
    osal_u8   trig_downlink_delta_pwr; /* AP侧到STA侧空口功率损耗 */
    osal_u8   ax_vap_cnt; /* 记录当前存在ax设备的数目 */
    osal_char resv[2]; // 2字节预留

    hal_dyn_cali_val_stru dyn_cali_val;

#ifdef _PRE_WLAN_FEATURE_DFS
    hal_dfs_radar_filter_stru dfs_radar_filter;
#endif

    /* hh503重构出来了rf dev，校准参数全部都从rf dev中获取 */
    /* 功率相关PHY参数 */
    hal_phy_pow_param_stru          phy_pow_param;

    /* 字节对齐 */
    p_hal_alg_isr_func  pa_hal_alg_isr_func_table[HAL_ISR_TYPE_BUTT][HAL_ALG_ISR_NOTIFY_BUTT];

    osal_s16                     always_rx_rssi;

    mac_channel_stru           wifi_channel_status;

    hal_rssi_stru              rssi;

    osal_char                    rx_last_rssi; /* 保存上次rssi */
    osal_u8                      adc_4t01_flag : 1; /* ADC使用4路80M合成为320M */
    osal_u8                      resv0         : 7;
    osal_u8                      adc_type : 4,      /* hal_clk_adc_dac_enum */
                               dac_type : 4;      /* hal_clk_adc_dac_enum */

    osal_u8                      current_11ax_working : 1, /* 当前hal dev是否有工作的11ax vap */
                               ampdu_tx_hw          : 1, /* AMPDU TX硬化开关 */
                               ampdu_partial_resnd  : 1, /* 硬件聚合时,MAC是否做部分帧重传 */
                               intf_det_invalid     : 1, /* 指示当前的检测结果是否有效 */
                               dpd_cali_doing       : 1,
                               dpd_clk_on           : 1,

                               /* 功率RF寄存器控使能时，固定功率配置不生效 */
                               pow_rf_reg_ctl_flag  : 1,
                               m2s_excute_flag      : 1; /* m2s正在执行的标记 */

    wlan_chan_ratio_stru            chan_ratio;                       /* 信道繁忙度相关统计量 */
    wlan_scan_chan_stats_stru       chan_result;                      /* 扫描时 一个信道的信道测量记录 */

    hal_dev_fsm_stru               hal_dev_fsm;                       /* hal device 状态机结构体 */
    osal_u32                         work_vap_bitmap;                   /* 需要在work状态工作的vap的个数 */

    hal_m2s_fsm_stru               hal_m2s_fsm;                       /* hal device m2s状态机 */

    /* hal device上所有vap上用户数总和，方便统计本hal device上用户数 */
    osal_u8                      assoc_user_nums;
    osal_u8                      fix_power_level;                    /* 固定功率等级(取值为0, 1, 2, 3) */
    osal_u8                      mag_mcast_frm_power_level;          /* 管理帧的功率等级 */
    osal_u8                      control_frm_power_level;            /* 控制帧的功率等级 */

    osal_void                    *alg_priv;                       /* 算法私有结构体 */
    hal_scan_params_stru           hal_scan_params;

    hal_device_tx_pwr_ctrl_stru    tx_pwr_ctrl;
#ifdef _PRE_WLAN_FEATURE_BTCOEX
    frw_timeout_stru                    btcoex_powersave_timer;
    hal_device_btcoex_sw_preempt_stru   btcoex_sw_preempt;
#endif

    osal_u8                   srb_switch         : 1,
                              mac_parse_tim      : 1,
                              fix_sifs_resp_rate : 1, /* 是否限制响应帧速率 */
                              is_mac_pa_enabled  : 1, /* pa是否使能 */
                              sideband_flag      : 1, /* 0:表示非边带认证国家，1:表示边带认证国家 */
                              resv4              : 1,
                              promis_switch      : 1, /* 混杂模式开关 */
                              is_temp_protect_pa_off  : 1; /* 过温保护是否关闭PA */
    osal_u8                   reserve[2];

    p_tbtt_ap_isr_func                  tbtt_update_beacon_func;

    alg_rx_dscr_ctl_device_info_stru    rx_dscr_ctl;
#ifdef _PRE_WLAN_FEATURE_FTM
    osal_u64              t1;
    osal_u64              t4;
#endif
    /* 同频干扰算法 */
    osal_u32 duty_ratio;          /* 占空比统计 */

    /* MAC测量信息 */
    hal_ch_mac_statics_stru  mac_ch_stats;
    hal_ch_intf_statics_stru intf_statics_stru;

    frw_timeout_stru                dyn_cali_per_frame_timer; /* 动态校准每帧的定时器 */
    hal_dyn_cali_pa_ppa_asjust_stru dyn_cali_vdet_stru;

#ifdef _PRE_WLAN_FEATURE_BTCOEX
    hal_device_btcoex_mgr_stru device_btcoex_mgr;
#endif

    osal_u8 twt_session_enable             : 1;
    osal_u8 psd_enable : 1;
    osal_u8 priv_freq_enable : 1;
    osal_u8 resv1 : 5;
    osal_u8 cali_data[WLAN_RF_CHANNEL_NUMS];

    osal_u8 resv2[2]; /* 2字节resv */
    oal_bool_enum_uint8 ps_mode_ever; /* 标记干扰检测期间是否进过低功耗 */

    osal_u8 tx_ba_index_table[MAC_TX_BA_LUT_BMAP_LEN]; /* 发送端BA 软件维护LUT表位图 大小4 */
    osal_u8 resv3[2]; // 4字节对齐
    osal_u16 foc_err_cnt;
    osal_u32 last_trigger_time;
    osal_s32 cfo_phase_avg;
    osal_u32 phase_det_cnt;
    /* ROM化后资源扩展指针 */
    osal_void             *_rom;
    osal_u16              tx_time;
    osal_u8               resv5[2];
    osal_u32              backoff_done_timestamp;
    osal_u32              tx_done_timestamp;
} hal_to_dmac_device_stru;

/* 自动调频的 pps门限 */
typedef struct {
    osal_u32 pps_value_0;
    osal_u32 pps_value_1;
    osal_u32 pps_value_2;
} hmac_pps_value_stru;

/* HAL模块和DMAC模块共用的WLAN RX结构体 */
typedef struct {
    osal_u32                  *base_dscr;      /* 描述符基地址 */
    hal_to_dmac_device_stru *hal_device;
    osal_u16                  dscr_num;        /* 接收到的描述符的个数 */
    osal_u8                   queue_id;        /* 队列同步标识 */
    osal_u8                   channel_num;
} hal_wlan_rx_event_stru;

typedef struct {
    hal_tx_dscr_stru        *base_dscr; /* 发送完成中断硬件所上报的描述符指针 */
    hal_to_dmac_device_stru *hal_device;
    osal_u8                dscr_num; /* 硬件一次发送所发出的描述符个数 */

    osal_s16                pdet_val;
    osal_u8                 tpc_code;

    osal_u64              t1;
    osal_u64              t4;
    osal_u16              tx_time;
    oal_bool_enum_uint8      pdet_enable : 1;
    osal_u8                chain_num       : 1;
    osal_u8                resv            : 6;
    osal_u8               resv1[1];
    osal_u32              backoff_done_timestamp;
    osal_u32              tx_done_timestamp;
} hal_tx_complete_event_stru;

typedef struct {
    hal_error_state_stru            error_state;
    hal_to_dmac_device_stru        *hal_device;
} hal_error_irq_event_stru;

typedef struct {
    hal_to_dmac_device_stru        *hal_device;
} hal_common_irq_event_stru;

typedef struct {
    osal_u8 ampdu_enable;
    osal_u8 mpdu_num;
}hal_tx_mpdu_info_mod;

typedef struct {
    wlan_scan_chan_stats_stru      chan_result_total; /* 时间、功率检测结果 */
    hal_ch_mac_statics_stru        mac_ch_stats_total; /* 发送、接收时间检测结果 */
} alg_intf_det_ch_stats_result;

typedef struct {
    osal_u8 vap_id;                               /* 下发扫描请求的vap id */
    wlan_scan_type_enum_uint8 bss_type; /* 要扫描的bss类型 */
    wlan_scan_type_enum_uint8 scan_type;          /* 主动/被动 */
    osal_u8 probe_delay;                          /* 主动扫描发probe request帧之前的等待时间 */

    osal_u8 scan_func;                            /* DMAC SCANNER 扫描模式 */
    osal_u8 max_scan_count_per_channel;           /* 每个信道的扫描次数 */
    osal_u16 scan_time;                           /* 扫描在某一信道停留此时间后，扫描结束, ms，必须是10的整数倍 */

    wlan_scan_mode_enum_uint8 scan_mode;          /* 扫描模式:前景扫描 or 背景扫描 */
    osal_u8 channel_nums;                         /* 信道列表中信道的个数 */
    osal_u8 resv[2];                              /* 对齐 */

    hal_chan_measure_stru measure_period;          /* 检测周期 */
    mac_channel_stru channel_list[WLAN_MAX_CHANNEL_NUM];
} alg_intf_det_ch_stats_param;
/*****************************************************************************
  4 函数实现
*****************************************************************************/
static inline osal_u32*  hal_rx_dscr_get_real_addr(osal_u32 *rx_dscr)
{
    if (rx_dscr == OAL_PTR_NULL) {
        return rx_dscr;
    }
    return (osal_u32 *)((osal_u8 *)rx_dscr + sizeof(hal_rx_dscr_stru));
}

static inline osal_u32*  hal_rx_dscr_get_sw_addr(osal_u32 *rx_dscr)
{
    if (rx_dscr == OAL_PTR_NULL) {
        return rx_dscr;
    }
    return (osal_u32 *)((osal_u8 *)rx_dscr - sizeof(hal_rx_dscr_stru));
}

/*****************************************************************************
    10.2 对外暴露的配置接口
*****************************************************************************/
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
