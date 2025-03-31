/*
 * Copyright (c) CompanyNameMagicTag 2020-2020. All rights reserved.
 * Description: Header file of mac_regdomain_rom.c.
 * Create: 2020-7-8
 */

#ifndef __MAC_REGDOMAIN_H__
#define __MAC_REGDOMAIN_H__

/*****************************************************************************
    其他头文件包含
*****************************************************************************/
#include "oal_ext_if.h"
#include "wlan_spec.h"
#include "wlan_types_common.h"
#include "frw_util.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*****************************************************************************
    宏定义
*****************************************************************************/
#define MAC_MAX_SUPP_CHANNEL (osal_u8)(OAL_MAX((osal_u8)MAC_CHANNEL_FREQ_2_BUTT, (osal_u8)MAC_CHANNEL_FREQ_5_BUTT))
#define mac_get_ch_bit(_val) (1 << (_val))

/* 默认管制域最大发送功率 */
#define MAC_RC_DEFAULT_MAX_TX_PWR 30

#define MAC_INVALID_RC 255     /* 无效的管制类索引值 */

#define MAC_SEC_CHAN_INDEX_OFFSET_START_FREQ_5 1
#define MAC_AFFECTED_CHAN_OFFSET_START_FREQ_5 0
#define MAC_SEC_CHAN_INDEX_OFFSET_START_FREQ_2 4
#define MAC_AFFECTED_CHAN_OFFSET_START_FREQ_2 3

#ifdef _PRE_WLAN_FEATURE_015CHANNEL_EXP
#define MAX_CHANNEL_NUM_FREQ_2G 16     /* 2G频段最大的信道号 */
#else
#define MAX_CHANNEL_NUM_FREQ_2G 14     /* 2G频段最大的信道号 */
#endif

#define MAC_MAX_20M_SUB_CH 8      /* VHT160中，20MHz信道总个数 */

/*****************************************************************************
    枚举定义
*****************************************************************************/
/* 一个管制类的起始频率枚举 */
typedef enum {
    MAC_RC_START_FREQ_2  = WLAN_BAND_2G,  /* 2.407 */
    MAC_RC_START_FREQ_5  = WLAN_BAND_5G,  /* 5 */

    MAC_RC_START_FREQ_BUTT
} mac_rc_start_freq_enum;
typedef osal_u8 mac_rc_start_freq_enum_uint8;

/* 管制类信道间距枚举 */
typedef enum {
    MAC_CH_SPACING_5MHZ = 0,
    MAC_CH_SPACING_10MHZ,
    MAC_CH_SPACING_20MHZ,
    MAC_CH_SPACING_25MHZ,
    MAC_CH_SPACING_40MHZ,
    MAC_CH_SPACING_80MHZ,

    MAC_CH_SPACING_BUTT
} mac_ch_spacing_enum;
typedef osal_u8 mac_ch_spacing_enum_uint8;

/* 雷达认证标准枚举 */
typedef enum {
    MAC_DFS_DOMAIN_NULL = 0,
    MAC_DFS_DOMAIN_FCC = 1,
    MAC_DFS_DOMAIN_ETSI = 2,
    MAC_DFS_DOMAIN_MKK = 3,
    MAC_DFS_DOMAIN_KOREA = 4,

    MAC_DFS_DOMAIN_BUTT
} mac_dfs_domain_enum;
typedef osal_u8 mac_dfs_domain_enum_uint8;

/* 5GHz频段枚举: 信道号对应的信道索引值 */
typedef enum {
    MAC_CHANNEL36 = 0,
    MAC_CHANNEL40 = 1,
    MAC_CHANNEL44 = 2,
    MAC_CHANNEL48 = 3,
    MAC_CHANNEL52 = 4,
    MAC_CHANNEL56 = 5,
    MAC_CHANNEL60 = 6,
    MAC_CHANNEL64 = 7,
    MAC_CHANNEL100 = 8,
    MAC_CHANNEL104 = 9,
    MAC_CHANNEL108 = 10,
    MAC_CHANNEL112 = 11,
    MAC_CHANNEL116 = 12,
    MAC_CHANNEL120 = 13,
    MAC_CHANNEL124 = 14,
    MAC_CHANNEL128 = 15,
    MAC_CHANNEL132 = 16,
    MAC_CHANNEL136 = 17,
    MAC_CHANNEL140 = 18,
    MAC_CHANNEL144 = 19,
    MAC_CHANNEL149 = 20,
    MAC_CHANNEL153 = 21,
    MAC_CHANNEL157 = 22,
    MAC_CHANNEL161 = 23,
    MAC_CHANNEL165 = 24,
    MAC_CHANNEL184 = 25,
    MAC_CHANNEL188 = 26,
    MAC_CHANNEL192 = 27,
    MAC_CHANNEL196 = 28,
    MAC_CHANNEL_FREQ_5_BUTT = 29
} mac_channel_freq_5_enum;
typedef osal_u8 mac_channel_freq_5_enum_uint8;

/* 2.4GHz频段枚举: 信道号对应的信道索引值 */
typedef enum {
    MAC_CHANNEL1 = 0,
    MAC_CHANNEL2 = 1,
    MAC_CHANNEL3 = 2,
    MAC_CHANNEL4 = 3,
    MAC_CHANNEL5 = 4,
    MAC_CHANNEL6 = 5,
    MAC_CHANNEL7 = 6,
    MAC_CHANNEL8 = 7,
    MAC_CHANNEL9 = 8,
    MAC_CHANNEL10 = 9,
    MAC_CHANNEL11 = 10,
    MAC_CHANNEL12 = 11,
    MAC_CHANNEL13 = 12,
    MAC_CHANNEL14 = 13,
#ifdef _PRE_WLAN_FEATURE_015CHANNEL_EXP
    MAC_CHANNEL15 = 14,
    MAC_CHANNEL16 = 15, // MAC_CHANNEL16代替新增的0信道
    MAC_CHANNEL_FREQ_2_BUTT = 16
#else
    MAC_CHANNEL_FREQ_2_BUTT = 14
#endif
} mac_channel_freq_2_enum;
typedef osal_u8 mac_channel_freq_2_enum_uint8;

/* note:change alg/acs/acs_cmd_resp.h as while */
typedef enum {
    MAC_RC_DFS = BIT0,
    MAC_RC_NO_OUTDOOR = BIT1,
    MAC_RC_NO_INDOOR = BIT2
} mac_behaviour_bmap_enum;

/* 信道的channel */
enum {
    CHANNEL1 = 1,
    CHANNEL2 = 2,
    CHANNEL3 = 3,
    CHANNEL4 = 4,
    CHANNEL5 = 5,
    CHANNEL6 = 6,
    CHANNEL7 = 7,
    CHANNEL8 = 8,
    CHANNEL9 = 9,
    CHANNEL10 = 10,
    CHANNEL14 = 14,
#ifdef _PRE_WLAN_FEATURE_015CHANNEL_EXP
    CHANNEL15 = 15,
    CHANNEL16 = 16,
#endif
    CHANNEL20 = 20,
    CHANNEL25 = 25,
    CHANNEL36 = 36,
    CHANNEL64 = 64,
    CHANNEL100 = 100,
    CHANNEL144 = 144,
    CHANNEL149 = 149,
    CHANNEL165 = 165,
    CHANNEL184 = 184,
    CHANNEL196 = 196
};

/* 移位bit */
#define SHITF_BIT2 2

enum {
    CHANNEL_IDX_COUNT1 = 1,
    CHANNEL_IDX_COUNT2 = 2,
    CHANNEL_IDX_COUNT3 = 3,
    CHANNEL_IDX_COUNT4 = 4,
    CHANNEL_IDX_COUNT5 = 5,
    CHANNEL_IDX_COUNT6 = 6,
    CHANNEL_IDX_COUNT7 = 7,
    CHANNEL_IDX_COUNT8 = 8,
};

/*****************************************************************************
    STRUCT定义
*****************************************************************************/
#define MAC_RD_INFO_LEN                 12  /* mac_regdomain_info_stru去掉mac_regclass_info_stru的长度 */

/* channel info结构体 */
typedef struct {
    osal_u8 chan_number;     /* 信道号 */
    osal_u8 reg_class;       /* 管制类在管制域中的索引号 */
    osal_u8 auc_resv[2];
} mac_channel_info_stru;

typedef struct {
    osal_u16 freq;        /* 中心频率，单位MHz */
    osal_u8 number;      /* 信道号 */
    osal_u8 idx;         /* 信道索引(软件用) */
} mac_freq_channel_map_stru;

typedef struct {
    osal_u32 channel;
    mac_freq_channel_map_stru channels[MAC_MAX_20M_SUB_CH];
} mac_channel_list_stru;

typedef struct {
    osal_u8   cnt;
    wlan_channel_bandwidth_enum_uint8 aen_supp_bw[WLAN_BW_CAP_BUTT];
} mac_supp_mode_table_stru;

/*****************************************************************************
    STRUCT定义
*****************************************************************************/
/* 管制域配置命令结构体 */
typedef struct {
    osal_void *mac_regdom;
} mac_cfg_country_stru;

/* 管制类结构体: 每个管制类保存的信息 */
typedef struct {
    mac_rc_start_freq_enum_uint8 start_freq;          /* 起始频率 */
    mac_ch_spacing_enum_uint8 ch_spacing;          /* 信道间距 */
    osal_u8 behaviour_bmap;      /* 允许的行为位图 位图定义见mac_behaviour_bmap_enum */
    osal_u8 coverage_class;      /* 覆盖类 */
    osal_u8 max_reg_tx_pwr;      /* 管制类规定的最大发送功率, 单位dBm */
    osal_u16 max_tx_pwr;          /* 实际使用的最大发送功率,扩大了10倍用于计算, 单位dBm，可以比管制域规定功率大 */
    osal_u8 resv[1];
    osal_u32 channel_bmap;        /* 支持信道位图，例 0011表示支持的信道的index为0 1 */
} mac_regclass_info_stru;

/* 管制域信息结构体 */
/*
 * 管制类值、管制类位图与管制类信息 数组下表的关系
 * 管制类取值        : .... 7  6  5  4  3  2  1  0
 * 管制类位图        : .... 1  1  0  1  1  1  0  1
 * 管制类信息数组下标: .... 5  4  x  3  2  1  x  0
 */
typedef struct {
    osal_s8 ac_country[WLAN_COUNTRY_STR_LEN];       /* 国家字符串 */
    mac_dfs_domain_enum_uint8 dfs_domain;                          /* DFS 雷达标准 */
    osal_u8 regclass_num;                        /* 管制类个数 */
    oal_bool_enum_uint8 sideband_country;
    osal_u8 resv[2];
    mac_regclass_info_stru regclass[WLAN_MAX_RC_NUM];          /* 管制域包含的管制类信息，注意 此成员只能放在最后一项! */
}  mac_regdomain_info_stru;

/*****************************************************************************
    函数声明
*****************************************************************************/
const mac_freq_channel_map_stru *hmac_regdomain_get_freq_map_2g_etc(osal_void);
const mac_freq_channel_map_stru *hmac_regdomain_get_freq_map_5g_etc(osal_void);
extern void hmac_get_regdomain_info_etc(mac_regdomain_info_stru **rd_info);
extern void hmac_init_regdomain_etc(void);
extern void hmac_init_channel_list_etc(void);
extern osal_u32 hmac_get_channel_num_from_idx_etc(osal_u8 band, osal_u8 idx, osal_u8 *channel_num);
extern osal_u32 hmac_get_channel_idx_from_num_etc(osal_u8 band, osal_u8 channel_num, osal_u8 *channel_idx);
extern osal_u32 hmac_is_channel_idx_valid_etc(osal_u8 band, osal_u8 ch_idx);
extern osal_u32 hmac_is_channel_num_valid_etc(osal_u8 band, osal_u8 ch_num);

extern osal_u32 hmac_regdomain_set_country_etc(osal_u16 len, osal_u8 *param);
extern osal_s8* hmac_regdomain_get_country_etc(void);
extern mac_regclass_info_stru* hmac_get_channel_idx_rc_info_etc(osal_u8 band, osal_u8 ch_idx);
extern mac_regclass_info_stru* hmac_get_channel_num_rc_info_etc(osal_u8 band, osal_u8 ch_num);
extern void hmac_get_ext_chan_info(osal_u8 pri20_channel_idx, wlan_channel_bandwidth_enum_uint8 en_bandwidth,
    mac_channel_list_stru *chan_info);
extern oal_bool_enum_uint8 hmac_is_cover_dfs_channel(osal_u8 band,
    wlan_channel_bandwidth_enum_uint8 en_bandwidth, osal_u8 channel_num);
extern wlan_channel_bandwidth_enum_uint8 hmac_regdomain_get_support_bw_mode(
    wlan_channel_bandwidth_enum_uint8 cfg_bw, osal_u8 channel);
osal_bool hmac_regdomain_channel_is_support_bw(wlan_channel_bandwidth_enum_uint8 cfg_bw, osal_u8 channel);

#ifdef _PRE_WLAN_FEATURE_11D
osal_u32 mac_set_country_ie_2g_etc(const mac_regdomain_info_stru *rd_info, osal_u8 *buffer, osal_u8 *len);

osal_u32 mac_set_country_ie_5g_etc(const mac_regdomain_info_stru *rd_info, const osal_u8 *buffer, osal_u8 *len);
#endif

/*****************************************************************************
 函 数 名  : mac_is_rc_dfs_req
 功能描述  : 根据国家码和管制域判断该信道是否为DFS信道
*****************************************************************************/
static INLINE__ oal_bool_enum_uint8  mac_is_rc_dfs_req(const mac_regclass_info_stru *rc_info)
{
    if ((rc_info->behaviour_bmap & MAC_RC_DFS) == 0) {
        return OSAL_FALSE;
    }

    return OSAL_TRUE;
}

/*****************************************************************************
 函 数 名  : mac_is_ch_in_radar_band
 功能描述  : 判断某条信道是否为DFS信道
*****************************************************************************/
static INLINE__ oal_bool_enum_uint8  mac_is_ch_in_radar_band(mac_rc_start_freq_enum_uint8 band, osal_u8 chan_idx)
{
    mac_regclass_info_stru   *rc_info;

    rc_info = hmac_get_channel_idx_rc_info_etc(band, chan_idx);
    if (osal_unlikely(rc_info == OSAL_NULL)) {
        return OSAL_FALSE;
    }

    return mac_is_rc_dfs_req(rc_info);
}

/*****************************************************************************
 函 数 名  : mac_is_dfs_channel
 功能描述  : 判断信道是否是DFS信道
*****************************************************************************/
static INLINE__ oal_bool_enum_uint8 mac_is_dfs_channel(osal_u8 band, osal_u8 channel_num)
{
    osal_u8 channel_idx = 0xff;

    if (hmac_get_channel_idx_from_num_etc(band, channel_num, &channel_idx) != OAL_SUCC) {
        return OSAL_FALSE;
    }

    if (mac_is_ch_in_radar_band(band, channel_idx) == OSAL_FALSE) {
        return OSAL_FALSE;
    }

    return OSAL_TRUE;
}

/*****************************************************************************
 函 数 名  : mac_get_band_by_channel_num
 功能描述  : 通过信道号找到频段，由调用者保证输入的信道号合法，从而降低本接口的复杂性，提高效率
*****************************************************************************/
static INLINE__ wlan_channel_band_enum_uint8 mac_get_band_by_channel_num(osal_u8 channel_num)
{
    return ((channel_num > MAX_CHANNEL_NUM_FREQ_2G) ? WLAN_BAND_5G : WLAN_BAND_2G);
}

/*****************************************************************************
 函 数 名  : mac_is_ch_supp_in_regclass
 功能描述  : 判断信道是否被管制类支持
*****************************************************************************/
static INLINE__ oal_bool_enum_uint8 mac_is_ch_supp_in_regclass(const mac_regclass_info_stru *rc_info, osal_u8 freq,
    osal_u8 ch_idx)
{
    if (rc_info == OSAL_NULL) {
        return OSAL_FALSE;
    }

    if (rc_info->start_freq != freq) {
        return OSAL_FALSE;
    }

    if ((rc_info->channel_bmap & mac_get_ch_bit(ch_idx)) != 0) {
        return OSAL_TRUE;
    }

    return OSAL_FALSE;
}

/*****************************************************************************
 函 数 名  : mac_get_num_supp_channel
 功能描述  : 获取当前频段所支持的最大信道数目
*****************************************************************************/
static INLINE__ osal_u8  mac_get_num_supp_channel(wlan_channel_band_enum_uint8 band)
{
    switch (band) {
        case WLAN_BAND_2G: /* 2.4GHz */
            return (osal_u8)MAC_CHANNEL_FREQ_2_BUTT;

        case WLAN_BAND_5G:   /* 5GHz */
            return (osal_u8)MAC_CHANNEL_FREQ_5_BUTT;

        default:
            return 0;
    }
}

/*****************************************************************************
 函 数 名  : mac_get_sec_ch_idx_offset
 功能描述  :
*****************************************************************************/
static INLINE__ osal_u8 mac_get_sec_ch_idx_offset(wlan_channel_band_enum_uint8 band)
{
    switch (band) {
        case WLAN_BAND_2G: /* 2.4GHz */
            return (osal_u8)MAC_SEC_CHAN_INDEX_OFFSET_START_FREQ_2;

        case WLAN_BAND_5G:   /* 5GHz */
            return (osal_u8)MAC_SEC_CHAN_INDEX_OFFSET_START_FREQ_5;

        default:
            return 0;
    }
}

/*****************************************************************************
 函 数 名  : mac_get_affected_ch_idx_offset
 功能描述  :
*****************************************************************************/
static INLINE__ osal_u8 mac_get_affected_ch_idx_offset(wlan_channel_band_enum_uint8 band)
{
    switch (band) {
        case WLAN_BAND_2G: /* 2.4GHz */
            return (osal_u8)MAC_AFFECTED_CHAN_OFFSET_START_FREQ_2;

        case WLAN_BAND_5G:   /* 5GHz */
            return (osal_u8)MAC_AFFECTED_CHAN_OFFSET_START_FREQ_5;

        default:
            return 0;
    }
}

#ifdef __cplusplus
#if __cplusplus
        }
#endif
#endif

#endif /* end of mac_regdomain.h */
