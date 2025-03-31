/*
 * Copyright (c) @CompanyNameMagicTag 2020-2022. All rights reserved.
 * Description: Header file
 */

#ifndef MSG_BTCOEX_ROM_H
#define MSG_BTCOEX_ROM_H

#include "hal_common_ops_device_rom.h"

#ifdef _PRE_WLAN_FEATURE_BTCOEX

typedef struct {
    osal_u8 dis_abort : 1;  /* ps期间是否disable abort */
    osal_u8 reply_cts : 1;  /* ps期间是否回复CTS */
    osal_u8 rsp_frame_ps : 1; /* ps期间响应帧是否设置ps bit */
    osal_u8 one_pkt_en : 1;   /* 是否发送one pkt节能帧 */
    osal_u8 custom_option : 1;    /* 自定义开关选项 */
    osal_u8 close_rts : 1;        /* 关闭rts功能 */
    osal_u8 resv_bits : 2;
    osal_u8 resv_bytes[3];
} mac_btcoex_ps_flag_info_stru;

typedef struct {
    /* 字段参考hal_one_packet_cfg_stru 部分字段需要在device侧自行获取 */
    hal_fcs_protect_type_enum_uint8   protect_type;
    hal_fcs_protect_cnt_enum_uint8    protect_cnt;
    osal_u16                          protect_coex_pri : 2;     /* btcoex下使用，one pkt发送优先级 */
    osal_u16                          cfg_one_pkt_tx_vap_index : 4;
    osal_u16                          cfg_one_pkt_tx_peer_index : 5;
    osal_u16                          bit_rsv : 5;
    osal_u32                          tx_mode;
    osal_u32                          tx_data_rate;
    osal_u16                          duration;        /* 单位 us */
    osal_u16                          timeout;         /* 硬件超时时间 */
    osal_u16                          wait_timeout;    /* 软件定时器超时时间 */

    osal_u8                           is_valid  : 1;
    osal_u8                           is_vap_param : 1;   /* vap参数标志 */
    osal_u8                           is_dev_param : 1;   /* dev参数标志 */
    osal_u8                           bit_rsv2 : 5;      /* 字节对齐 */
    osal_u8                           rsv[1];      /* 字节对齐 */
} mac_btcoex_ps_frame_info_stru;

#endif
#endif