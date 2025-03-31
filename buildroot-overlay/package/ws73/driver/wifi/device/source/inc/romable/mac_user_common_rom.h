/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: netbuf enum type head file.
 * Create: 2022-07-04
 */

#ifndef MAC_USER_COMMON_ROM_H
#define MAC_USER_COMMON_ROM_H

#include "osal_types.h"
#include "wlan_types_base_rom.h"

#ifdef __cplusplus
#if __cplusplus
    extern "C" {
#endif
#endif

/* 用户与AP的关联状态枚举 */
typedef enum {
    MAC_USER_STATE_AUTH_COMPLETE = 1,
    MAC_USER_STATE_AUTH_KEY_SEQ1 = 2,
    MAC_USER_STATE_ASSOC = 3,

    MAC_USER_STATE_BUTT = 4
} mac_user_asoc_state_enum;
typedef osal_u8 mac_user_asoc_state_enum_uint8;

typedef enum {
    MAC_USER_TYPE_DEFAULT,
    MAC_USER_TYPE_TDLS,

    MAC_USER_TYPE_BUTT
} mac_user_type_enum;
typedef osal_u8 mac_user_type_enum_uint8;

typedef struct mac_key_params_tag {
    osal_u8 key[OAL_WPA_KEY_LEN];
    osal_u8 seq[OAL_WPA_SEQ_LEN];
    osal_s32 key_len;
    osal_s32 seq_len;
    osal_u32 cipher;
} mac_key_params_stru;

typedef struct {
    mac_frame_he_cap_ie_stru he_cap_ie; /* HE Cap ie */
    mac_frame_he_oper_ie_stru he_oper_ie; /* HE Operation */
    wlan_mib_vht_op_width_enum_uint8 channel_width; /* 解析VHT Operation IE */
    osal_u16 max_mpdu_length;

    osal_u8 bss_color       : 6; /* 保存bss color change announcement ie数据 */
    osal_u8 bss_color_exist : 1;
    osal_u8 he_capable       : 1; /* HE capable */
} mac_he_hdl_stru;

typedef struct {
osal_u16                max_mcs_1ss : 2,                             /* 一个空间流的MCS最大支持MAP */
                        max_mcs_2ss : 2,                             /* 一个空间流的MCS最大支持MAP */
                        max_mcs_3ss : 2,                             /* 一个空间流的MCS最大支持MAP */
                        max_mcs_4ss : 2,                             /* 一个空间流的MCS最大支持MAP */
                        max_mcs_5ss : 2,                             /* 一个空间流的MCS最大支持MAP */
                        max_mcs_6ss : 2,                             /* 一个空间流的MCS最大支持MAP */
                        max_mcs_7ss : 2,                             /* 一个空间流的MCS最大支持MAP */
                        max_mcs_8ss : 2;                             /* 一个空间流的MCS最大支持MAP */
} mac_max_mcs_map_stru;

typedef mac_max_mcs_map_stru mac_tx_max_mcs_map_stru;
typedef mac_max_mcs_map_stru mac_rx_max_mcs_map_stru;

typedef struct {
    osal_u16  max_mpdu_length;
    osal_u16  basic_mcs_set;

    osal_u32  max_mpdu_length_flag         : 2,
              supported_channel_width : 2,
              rx_ldpc                 : 1,
              short_gi_80mhz          : 1,
              short_gi_160mhz         : 1,
              tx_stbc                 : 1,
              rx_stbc                 : 3,
              su_beamformer_cap       : 1,  /* SU bfer能力，要过AP认证，必须填1 */
              su_beamformee_cap       : 1,  /* SU bfee能力，要过STA认证，必须填1 */
              num_bf_ant_supported    : 3,  /* SU时，最大接收NDP的Nsts，最小是1 */
              num_sounding_dim        : 3,  /* SU时，表示Nsts最大值，最小是1 */
              mu_beamformer_cap       : 1,  /* 不支持，set to 0 */
              mu_beamformee_cap       : 1,  /* 不支持，set to 0 */
              vht_txop_ps             : 1,
              htc_vht_capable         : 1,
              max_ampdu_len_exp       : 3,
              vht_link_adaptation     : 2,
              rx_ant_pattern          : 1,
              tx_ant_pattern          : 1,
              resv1                   : 2;  /* 解析vht Capabilities IE: VHT Capabilities Info field */

    mac_tx_max_mcs_map_stru     tx_max_mcs_map;
    mac_rx_max_mcs_map_stru     rx_max_mcs_map;

    osal_u16  rx_highest_rate : 13,
              resv2           : 3;
    osal_u16  tx_highest_rate : 13,
              resv3           : 3;          /* 解析vht Capabilities IE: VHT Supported MCS Set field */

    oal_bool_enum_uint8 vht_capable;         /* VHT capable */

    /* vht operation只有是ap的情况下有 */
    wlan_mib_vht_op_width_enum_uint8  channel_width; /* 解析VHT Operation IE. 0 for 20/40M, 1 for 80M, 2 for 160M */
    osal_u8                           channel_center_freq_seg0;
    osal_u8                           channel_center_freq_seg1;
} mac_vht_hdl_stru;

/* user tx参数，从架构分析文档获得成员 */
typedef struct {
    wlan_security_txop_params_stru security;
} mac_user_tx_param_stru;

typedef struct {
    osal_u16 tid_pkt_num[WLAN_TID_MAX_NUM];
    osal_u32 tid_pkt_len[WLAN_TID_MAX_NUM];
} mac_user_tid_pkt_stat_stru;

typedef struct {
    osal_u8 rs_nrates;                      /* 个数 */
    osal_u8 rs_rates[WLAN_MAX_SUPP_RATES]; /* 速率 */
} mac_rate_stru;

/* 密钥管理结构体 */
typedef struct {
    wlan_ciper_protocol_type_enum_uint8   cipher_type;
    osal_u8                               default_index;                       /* 默认索引 */
    osal_u8                               igtk_key_index;                      /* igtk索引 */
    osal_u8                               gtk             : 1; /* 指示RX GTK的槽位，02使用 */
    osal_u8                               last_gtk_key_idx : 7; /* igtk索引 */
    wlan_priv_key_param_stru              key[WLAN_NUM_TK + WLAN_NUM_IGTK];   /* key缓存 */
} mac_key_mgmt_stru;

/* user下ht相关信息 */
typedef struct {
    /* ht cap */
    oal_bool_enum_uint8 ht_capable;                  /* HT capable */
    osal_u8           max_rx_ampdu_factor;           /* Max AMPDU Rx Factor */
    osal_u8           min_mpdu_start_spacing;        /* Min AMPDU Start Spacing */
    osal_u8           htc_support;                   /* HTC 域支持 */

    osal_u16          ldpc_coding_cap         : 1,  /* LDPC 编码 capability */
                      supported_channel_width : 1,  /* STA 支持的带宽 0: 20Mhz, 1: 20/40Mhz */
                      sm_power_save           : 2,  /* SM 省电模式 */
                      ht_green_field          : 1,  /* 绿野模式 */
                      short_gi_20mhz          : 1,  /* 20M下短保护间隔 */
                      short_gi_40mhz          : 1,  /* 40M下短保护间隔 */
                      tx_stbc                 : 1,  /* Indicates support for the transmission of PPDUs using STBC */
                      rx_stbc                 : 2,  /* 支持 Rx STBC */
                      ht_delayed_block_ack    : 1,  /* Indicates support for HT-delayed Block Ack opera-tion. */
                      max_amsdu_length        : 1,  /* Indicates maximum A-MSDU length. */
                      dsss_cck_mode_40mhz     : 1,  /* 40M下 DSSS/CCK 模式 0:不使用 40M dsss/cck, 1: 使用 40M dsss/cck */
                      bit_resv                : 1,
                      forty_mhz_intolerant    : 1,
                      lsig_txop_protection    : 1;  /* 支持 L-SIG TXOP 保护 */

    osal_u8           rx_mcs_bitmask[WLAN_HT_MCS_BITMASK_LEN];   /* Rx MCS bitmask */

    /* ht operation, VAP是STA, user是AP独有 */
    osal_u8           primary_channel;

    osal_u8           secondary_chan_offset             : 2,
                        sta_chan_width                    : 1,
                        rifs_mode                         : 1,
                        ht_protection                     : 2,
                        nongf_sta_present                 : 1,
                        obss_nonht_sta_present            : 1;

    osal_u8           dual_beacon                       : 1,
                        dual_cts_protection               : 1,
                        secondary_beacon                  : 1,
                        lsig_txop_protection_full_support : 1,
                        pco_active                        : 1,
                        pco_phase                         : 1,
                        resv6                             : 2;

    osal_u8           auc_resv[1];

    osal_u8           basic_mcs_set[16];

    osal_u32          imbf_receive_cap                :  1, /* 隐式TxBf接收能力 */
                      receive_staggered_sounding_cap  :  1, /* 接收交错探测帧的能力 */
                      transmit_staggered_sounding_cap :  1, /* 发送交错探测帧的能力 */
                      receive_ndp_cap                 :  1, /* 接收NDP能力 */
                      transmit_ndp_cap                :  1, /* 发送NDP能力 */
                      imbf_cap                        :  1, /* 隐式TxBf能力 */
                      calibration                     :  2, /* 0=不支持 1=可以报告不能发起 2=保留 3=可以发起，可以响应 */
                      exp_csi_txbf_cap                :  1, /* 应用CSI反馈进行TxBf的能力 */
                      exp_noncomp_txbf_cap            :  1, /* 应用非压缩矩阵进行TxBf的能力 */
                      exp_comp_txbf_cap               :  1, /* 应用压缩矩阵进行TxBf的能力 */
                      exp_csi_feedback                :  2, /* 0=不支持.1=延迟反馈.2=立即反馈.3=延迟和立即反馈 */
                      exp_noncomp_feedback            :  2, /* 0=不支持.1=延迟反馈.2=立即反馈.3=延迟和立即反馈 */
                      exp_comp_feedback               :  2, /* 0=不支持.1=延迟反馈.2=立即反馈.3=延迟和立即反馈 */
                      min_grouping                    :  2, /* 0=不分组.1=1,2分组. 2=1,4分组. 3=1,2,4分组 */
                      csi_bfer_ant_number             :  2, /* CSI反馈时，bfee最多支持的beamformer天线数 */
                      noncomp_bfer_ant_number         :  2, /* 非压缩矩阵反馈时，bfee最多支持的beamformer天线数 */
                      comp_bfer_ant_number            :  2, /* 压缩矩阵反馈时，bfee最多支持的beamformer天线数 */
                      csi_bfee_max_rows               :  2, /* bfer支持的来自bfee的CSI显示反馈的最大行数 */
                      channel_est_cap                 :  2, /* 信道估计的能力，0=1空时流，依次递增 */
                      reserved                        :  3;
}mac_user_ht_hdl_stru;

#ifdef __cplusplus
#if __cplusplus
    }
#endif
#endif

#endif /* end of mac_user_common_rom.h */