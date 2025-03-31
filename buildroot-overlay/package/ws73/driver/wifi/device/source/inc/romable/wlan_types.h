/*
 * Copyright (c) CompanyNameMagicTag. 2021-2024. All rights reserved.
 * Description: Header files shared by wlan_types.h and wlan_types_rom.h.
 * Create: 2021-09-18
 */

#ifndef __WLAN_TYPES_HCM_H__
#define __WLAN_TYPES_HCM_H__

#include <osal_types.h>

#include "oal_types_device_rom.h"
#include "wlan_types_base_rom.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*****************************************************************************
   基本宏定义
*****************************************************************************/
#define WLAN_2G_SUB_BAND_NUM        14

/* 空间流定义 */
#define WLAN_SINGLE_NSS                 0
#define WLAN_DOUBLE_NSS                 1
#define WLAN_TRIPLE_NSS                 2
#define WLAN_FOUR_NSS                   3
#define WLAN_NSS_LIMIT                  2

#define WLAN_USER_HT_NSS_LIMIT          4
#define WLAN_USER_VHT_NSS_LIMIT         8
/* 芯片最大空间流数目 */
#if (_PRE_WLAN_FEATURE_NSS_MODE == _PRE_WLAN_SUPPORT_SINGLE_NSS)
#define WLAN_MAX_NSS_NUM           (WLAN_SINGLE_NSS)
#elif (_PRE_WLAN_FEATURE_NSS_MODE == _PRE_WLAN_SUPPORT_DOUBLE_NSS)
#define WLAN_MAX_NSS_NUM           (WLAN_DOUBLE_NSS)
#else
#define WLAN_MAX_NSS_NUM           (WLAN_SINGLE_NSS)
#endif

#define WLAN_NSS_MAX_NUM_LIMIT     (WLAN_MAX_NSS_NUM + 1)  /* 最大空间流数目，考虑数组大小需要+1 */


#define MAC_BYTE_ALIGN_VALUE                4           /* 4字节对齐 */

#define WLAN_MAC_ADDR_LEN                   6           /* MAC地址长度宏 */
#define WLAN_MAX_FRAME_HEADER_LEN           36          /* 最大的MAC帧头长度，数据帧36，管理帧为28 */
#define WLAN_MIN_FRAME_HEADER_LEN           10          /* ack与cts的帧头长度为10 */
#define WLAN_MAX_FRAME_LEN                  1600        /* 维测用，防止越界 */
#define WLAN_MGMT_FRAME_HEADER_LEN          24          /* 管理帧的MAC帧头长度，数据帧36，管理帧为28 */
#define WLAN_IWPRIV_MAX_BUFF_LEN            100         /* iwpriv上传的字符串最大长度 */
/* SSID最大长度, +1为\0预留空间 */
#define WLAN_SSID_MAX_LEN                   (32 + 1)

/* 80211MAC帧头FC字段宏定义 */
#define WLAN_PROTOCOL_VERSION               0x00        /* 协议版本 */
#define WLAN_FC0_TYPE_MGT                   0x00        /* 管理帧 */
#define WLAN_FC0_TYPE_CTL                   0x04        /* 控制帧 */
#define WLAN_FC0_TYPE_DATA                  0x08        /* 数据帧 */

/* 管理帧subtype */
#define WLAN_FC0_SUBTYPE_ASSOC_REQ          0x00
#define WLAN_FC0_SUBTYPE_ASSOC_RSP          0x10
#define WLAN_FC0_SUBTYPE_REASSOC_REQ        0x20
#define WLAN_FC0_SUBTYPE_REASSOC_RSP        0x30
#define WLAN_FC0_SUBTYPE_PROBE_REQ          0x40
#define WLAN_FC0_SUBTYPE_PROBE_RSP          0x50
#define WLAN_FC0_SUBTYPE_BEACON             0x80
#define WLAN_FC0_SUBTYPE_ATIM               0x90
#define WLAN_FC0_SUBTYPE_DISASSOC           0xa0
#define WLAN_FC0_SUBTYPE_AUTH               0xb0
#define WLAN_FC0_SUBTYPE_DEAUTH             0xc0
#define WLAN_FC0_SUBTYPE_ACTION             0xd0
#define WLAN_FC0_SUBTYPE_ACTION_NO_ACK      0xe0

/* 控制帧subtype */
#define WLAN_FC0_SUBTYPE_NDPA               0x50
#define WLAN_FC0_SUBTYPE_CONTROL_WRAPPER    0x70        /* For TxBF RC */
#define WLAN_FC0_SUBTYPE_BAR                0x80
#define WLAN_FC0_SUBTYPE_BA                 0x90

#define WLAN_FC0_SUBTYPE_PS_POLL            0xa0
#define WLAN_FC0_SUBTYPE_RTS                0xb0
#define WLAN_FC0_SUBTYPE_CTS                0xc0
#define WLAN_FC0_SUBTYPE_ACK                0xd0
#define WLAN_FC0_SUBTYPE_CF_END             0xe0
#define WLAN_FC0_SUBTYPE_CF_END_ACK         0xf0

/* 数据帧subtype */
#define WLAN_FC0_SUBTYPE_DATA               0x00
#define WLAN_FC0_SUBTYPE_CF_ACK             0x10
#define WLAN_FC0_SUBTYPE_CF_POLL            0x20
#define WLAN_FC0_SUBTYPE_CF_ACPL            0x30
#define WLAN_FC0_SUBTYPE_NODATA             0x40
#define WLAN_FC0_SUBTYPE_CFACK              0x50
#define WLAN_FC0_SUBTYPE_CFPOLL             0x60
#define WLAN_FC0_SUBTYPE_CF_ACK_CF_ACK      0x70
#define WLAN_FC0_SUBTYPE_QOS                0x80
#define WLAN_FC0_SUBTYPE_QOS_NULL           0xc0

#define WLAN_FC1_DIR_MASK                   0x03
#define WLAN_FC1_DIR_NODS                   0x00        /* STA->STA */
#define WLAN_FC1_DIR_TODS                   0x01        /* STA->AP  */
#define WLAN_FC1_DIR_FROMDS                 0x02        /* AP ->STA */
#define WLAN_FC1_DIR_DSTODS                 0x03        /* AP ->AP  */

#define WLAN_FC1_MORE_FRAG                  0x04
#define WLAN_FC1_RETRY                      0x08
#define WLAN_FC1_PWR_MGT                    0x10
#define WLAN_FC1_MORE_DATA                  0x20
#define WLAN_FC1_WEP                        0x40
#define WLAN_FC1_ORDER                      0x80

#define WLAN_HDR_DUR_OFFSET                 2           /* duartion相对于mac头的字节偏移 */
#define WLAN_HDR_ADDR1_OFFSET               4           /* addr1相对于mac头的字节偏移 */
#define WLAN_HDR_ADDR2_OFFSET               10          /* addr1相对于mac头的字节偏移 */
#define WLAN_HDR_ADDR3_OFFSET               16          /* addr1相对于mac头的字节偏移 */
#define WLAN_HDR_FRAG_OFFSET                22          /* 分片序号相对于mac的字节偏移 */

#define WLAN_REASON_CODE_LEN                2

/* 帧头DS位 */
#define WLAN_FRAME_TO_AP                   0x0100
#define WLAN_FRAME_FROM_AP                 0x0200
/* FCS长度(4字节) */
#define WLAN_HDR_FCS_LENGTH                 4

#define WLAN_RANDOM_MAC_OUI_LEN            3            /* 随机mac地址OUI长度 */

#define WLAN_MAX_BAR_DATA_LEN               20  /* BAR帧的最大长度 */
#define WLAN_CHTXT_SIZE                     128 /* challenge text的长度 */

#define WLAN_SEQ_SHIFT                      4
/* AMPDU Delimeter长度(4字节) */
#define WLAN_DELIMETER_LENGTH               4

/* 取绝对值 */
#define get_abs(val)                        ((val) > 0 ? (val) : -(val))

/* 配置命令最大长度: 从算法名称开始算起，不包括"alg" */
#define DMAC_ALG_CONFIG_MAX_ARG     7

/* 信道切换计数 */
#define WLAN_CHAN_SWITCH_DEFAULT_CNT        6
#define WLAN_CHAN_SWITCH_DETECT_RADAR_CNT   1

/* 默认信道配置 */
#define WLAN_DEFAULT_CHAN_NUM        6

/* 5G子频段数目 */
#define WLAN_5G_SUB_BAND_NUM        7
#define WLAN_5G_20M_SUB_BAND_NUM    7
#define WLAN_5G_80M_SUB_BAND_NUM    7
#define WLAN_5G_CALI_SUB_BAND_NUM   (WLAN_5G_20M_SUB_BAND_NUM + WLAN_5G_80M_SUB_BAND_NUM)

#define WLAN_DIEID_MAX_LEN   40

#define WLAN_FIELD_TYPE_AID            0xC000

#define WLAN_SIFS_OFDM_POWLVL_NUM   4

#define WLAN_ACTION_BODY_CATEGORY_OFFSET      0          /* Action帧的category字段相对于frame body的字节偏移 */
#define WLAN_ACTION_BODY_ELEMENT_OFFSET       6          /* Action帧的element字段相对于frame body的字节偏移 */
#define WLAN_ACTION_BODY_OFFSET               24         /* Action帧的frame body相对于帧起始地址的字节偏移 */

#define WLAN_ACTION_CATEGORY_VENDOR_SPECIFIC  127         /* Action帧的类别为厂商自定义 */
#define WLAN_ACTION_TYPE_NOA                  1           /* Action帧的子类别为NOA */

/* 管理帧包含的固定长度信息元素的总长度 */
#define WLAN_FC0_ASSOC_REQ_MIN_LEN          4
#define WLAN_FC0_REASSOC_REQ_MIN_LEN        8
#define WLAN_FC0_BEACON_MIN_LEN             12
#define WLAN_FC0_DISASSOC_MIN_LEN           2
#define WLAN_FC0_AUTH_MIN_LEN               6
#define WLAN_FC0_DEAUTH_MIN_LEN             2
#define WLAN_FC0_ACTION_MIN_LEN             1

#define WLAN_INVALD_VHT_MCS     0xff
#define WLAN_HT_MAX_BITMAP_LEN 8                /* ht速率的bitmap最大位数 */
/*****************************************************************************
  2.2 WME宏定义
*****************************************************************************/

static inline osal_u8 wlan_wme_ac_to_tid(osal_u8 ac)
{
    /* 数字6，5，1，0表示tid */
    return ((ac == WLAN_WME_AC_VO) ? 6 : (ac == WLAN_WME_AC_VI) ? 5 : (ac == WLAN_WME_AC_BK) ? 1 : 0);
}

static inline osal_u8 wlan_wme_tid_to_ac(osal_u8 tid)
{
    return (((tid == 0) || (tid == 3)) ? WLAN_WME_AC_BE : /* 数字0,3表示tid */
        ((tid == 1) || (tid == 2)) ? WLAN_WME_AC_BK :     /* 数字1,2表示tid */
        ((tid == 4) || (tid == 5)) ? WLAN_WME_AC_VI : WLAN_WME_AC_VO); /* 数字4,5表示tid */
}

/*****************************************************************************
    HT宏定义
*****************************************************************************/
/* 11n: Maximum A-MSDU Length Indicates maximum A-MSDU length.See 9.11.
Set to 0 for 3839 octetsSet to 1 for 7935 octets */
/* 11AC(9.11): A VHT STA that sets the Maximum MPDU Length in the VHT Capabilities element to indicate 3895 octets
   shall set the Maximum A-MSDU Length in the HT Capabilities element to indicate 3839 octets. A VHT STA
   that sets the Maximum MPDU Length in the VHT Capabilities element to indicate 7991 octets or 11 454 oc-
   tets shall set the Maximum A-MSDU Length in the HT Capabilities element to indicate 7935 octets. */
#define WLAN_AMSDU_FRAME_MAX_LEN_SHORT      3839
#define WLAN_AMSDU_FRAME_MAX_LEN_LONG       7935

/* RSSI统计滤波，RSSI范围是-128~127, 一般不会等于127这么大，所以将127设置为MARKER,即初始值 */
#define WLAN_RSSI_DUMMY_MARKER              0x7F

/*****************************************************************************
    安全相关宏定义
*****************************************************************************/
/* cipher suite selectors */
#define WITP_WLAN_CIPHER_SUITE_USE_GROUP 0x000FAC00
#define WITP_WLAN_CIPHER_SUITE_WEP40     0x000FAC01
#define WITP_WLAN_CIPHER_SUITE_TKIP      0x000FAC02
/* reserved:                0x000FAC03 */
#define WITP_WLAN_CIPHER_SUITE_CCMP      0x000FAC04
#define WITP_WLAN_CIPHER_SUITE_WEP104    0x000FAC05
#define WITP_WLAN_CIPHER_SUITE_AES_CMAC  0x000FAC06
#define WITP_WLAN_CIPHER_SUITE_GCMP      0x000FAC08
#define WITP_WLAN_CIPHER_SUITE_GCMP_256  0x000FAC09
#define WITP_WLAN_CIPHER_SUITE_CCMP_256  0x000FAC0A
#define WITP_WLAN_CIPHER_SUITE_BIP_GMAC_128  0x000FAC0B
#define WITP_WLAN_CIPHER_SUITE_BIP_GMAC_256  0x000FAC0C
#define WITP_WLAN_CIPHER_SUITE_BIP_CMAC_256  0x000FAC0D

#define WITP_WLAN_CIPHER_SUITE_SMS4      0x00147201

/* AKM suite selectors */
#define WITP_WLAN_AKM_SUITE_8021X        0x000FAC01
#define WITP_WLAN_AKM_SUITE_PSK          0x000FAC02
#define WITP_WLAN_AKM_SUITE_WAPI_PSK     0x000FAC04
#define WITP_WLAN_AKM_SUITE_WAPI_CERT    0x000FAC12

#define WLAN_PMKID_LEN           16
#define WLAN_PMKID_CACHE_SIZE    16
#define WLAN_NONCE_LEN           32
#define WLAN_PTK_PREFIX_LEN      22
#define WLAN_GTK_PREFIX_LEN      19
#define WLAN_GTK_DATA_LEN        (NONCE_LEN + WLAN_MAC_ADDR_LEN)
#define WLAN_PTK_DATA_LEN        (2 * NONCE_LEN + 2 * WLAN_MAC_ADDR_LEN)

#define WLAN_KCK_LENGTH          16
#define WLAN_KEK_LENGTH          16
#define WLAN_TEMPORAL_KEY_LENGTH 16
#define WLAN_MIC_KEY_LENGTH      8

#define WLAN_PMK_SIZE        32 /* In Bytes */
#define WLAN_PTK_SIZE        64 /* In Bytes */
#define WLAN_GTK_SIZE        32 /* In Bytes */
#define WLAN_GMK_SIZE        32 /* In Bytes */

#define WLAN_WEP40_KEY_LEN              5
#define WLAN_WEP104_KEY_LEN             13
#define WLAN_TKIP_KEY_LEN               32      /* TKIP KEY length 256 BIT */
#define WLAN_CCMP_KEY_LEN               16      /* CCMP KEY length 128 BIT */
#define WLAN_BIP_KEY_LEN                16      /* BIP KEY length 128 BIT */

#define WLAN_NUM_DOT11WEPDEFAULTKEYVALUE       4
#define WLAN_MAX_WEP_STR_SIZE                  27 /* 5 for WEP 40; 13 for WEP 104 */
#define WLAN_WEP_SIZE_OFFSET                   0
#define WLAN_WEP_KEY_VALUE_OFFSET              1
#define WLAN_WEP_40_KEY_SIZE                  40
#define WLAN_WEP_104_KEY_SIZE                104

#define WLAN_COUNTRY_STR_LEN        3

/* crypto status */
#define WLAN_ENCRYPT_BIT        (1 << 0)
#define WLAN_WEP_BIT            (1 << 1)
#define WLAN_WEP104_BIT         ((1 << 2) | (1 << 1))
#define WLAN_WPA_BIT            (1 << 3)
#define WLAN_WPA2_BIT           (1 << 4)
#define WLAN_CCMP_BIT           (1 << 5)
#define WLAN_TKIP_BIT           (1 << 6)
#ifdef _PRE_WLAN_FEATURE_WPA3
#define WLAN_WPA3_BIT           (1 << 7)
#endif

#define WLAN_WAPI_BIT           (1 << 5)

/* 11i参数 */
/* WPA 密钥长度 */
#define WLAN_WPA_KEY_LEN                    32
#define WLAN_CIPHER_KEY_LEN                 16
#define WLAN_TKIP_MIC_LEN                   16
/* WPA 序号长度 */
#define WLAN_WPA_SEQ_LEN                    16

/* auth */
#define WLAN_OPEN_SYS_BIT       (1 << 0)
#define WLAN_SHARED_KEY_BIT     (1 << 1)
#define WLAN_8021X_BIT          (1 << 2)

#define WLAN_WITP_CAPABILITY_PRIVACY BIT4

#define WLAN_NUM_TK             4
#define WLAN_NUM_IGTK           2
#define WLAN_MAX_IGTK_KEY_INDEX 5
#define WLAN_MAX_WEP_KEY_COUNT  4

/*****************************************************************************
    过滤命令宏定义
*****************************************************************************/
#define  WLAN_BIP_REPLAY_FAIL_FLT  BIT0               /* BIP重放攻击过滤 */
#define  WLAN_CCMP_REPLAY_FAIL_FLT  BIT1              /* CCMP重放攻击过滤 */
#define  WLAN_OTHER_CTRL_FRAME_FLT BIT2               /* direct控制帧过滤 */
#define  WLAN_BCMC_MGMT_OTHER_BSS_FLT BIT3            /* 其他BSS网络的组播管理帧过滤 */

#define  WLAN_UCAST_MGMT_OTHER_BSS_FLT BIT4           /* 其他BSS网络的单播管理帧过滤 */

#define  WLAN_UCAST_DATA_OTHER_BSS_FLT BIT5           /* 其他BSS网络的单播数据帧过滤 */

/*****************************************************************************
    TX & RX Chain Macro
*****************************************************************************/
/* RF通道数规格 */
/* RF0 */
#define WLAN_RF_CHANNEL_ZERO        0
/* RF1 */
#define WLAN_RF_CHANNEL_ONE         1

/* PHY通道选择 */
/* 通道0 */
#define WLAN_PHY_CHAIN_ZERO_IDX     0
/* 通道1 */
#define WLAN_PHY_CHAIN_ONE_IDX      1

/* 双通道掩码 */
#define WLAN_RF_CHAIN_DOUBLE        3
#define WLAN_RF_CHAIN_ONE           2
#define WLAN_RF_CHAIN_ZERO          1

/* 双通道掩码 */
#define WLAN_PHY_CHAIN_DOUBLE       3
#define WLAN_TX_CHAIN_DOUBLE        WLAN_PHY_CHAIN_DOUBLE
#define WLAN_RX_CHAIN_DOUBLE        WLAN_PHY_CHAIN_DOUBLE
/*  通道0 掩码 */
#define WLAN_PHY_CHAIN_ZERO         1
#define WLAN_TX_CHAIN_ZERO          WLAN_PHY_CHAIN_ZERO
#define WLAN_RX_CHAIN_ZERO          WLAN_PHY_CHAIN_ZERO
/*  通道1 掩码 */
#define WLAN_PHY_CHAIN_ONE          2
#define WLAN_TX_CHAIN_ONE           WLAN_PHY_CHAIN_ONE
#define WLAN_RX_CHAIN_ONE           WLAN_PHY_CHAIN_ONE

#define WLAN_2G_CHANNEL_NUM         14
#define WLAN_5G_CHANNEL_NUM         29
/* wifi 5G 2.4G全部信道个数 */
#define WLAN_MAX_CHANNEL_NUM        (WLAN_2G_CHANNEL_NUM + WLAN_5G_CHANNEL_NUM)

/* PLL0 */
#define WLAN_RF_PLL_ID_0            0
/* PLL1 */
#define WLAN_RF_PLL_ID_1            1

/*****************************************************************************
  linkloss
*****************************************************************************/
#define WLAN_LINKLOSS_REPORT            10 /* linkloss每10次打印一次维测 */
#define WLAN_LINKLOSS_MIN_THRESHOLD     10 /* linkloss门限最小最低值 */

/* 11AX MCS相关的内容 */
#define MAC_MAX_SUP_MCS7_11AX_EACH_NSS             0   /* 11AX各空间流支持的最大MCS序号，支持0-7 */
#define MAC_MAX_SUP_MCS9_11AX_EACH_NSS             1   /* 11AX各空间流支持的最大MCS序号，支持0-9 */
#define MAC_MAX_SUP_MCS11_11AX_EACH_NSS            2   /* 11AX各空间流支持的最大MCS序号，支持0-11 */
#define MAC_MAX_SUP_INVALID_11AX_EACH_NSS          3   /* 不支持 */


#define MAC_MAX_RATE_SINGLE_NSS_20M_11AX           106  /* 1个空间流20MHz的最大速率-见ax协议28.5-HE-MCSs */
#define MAC_MAX_RATE_SINGLE_NSS_40M_11AX           212 /* 1个空间流40MHz的最大Long GI速率 */
#define MAC_MAX_RATE_SINGLE_NSS_80M_11AX           212 /* 1个空间流40MHz的最大Long GI速率 */

#define MAC_MAX_RATE_DOUBLE_NSS_20M_11AX           211 /* 2个空间流20MHz的最大Long GI速率 */
#define MAC_MAX_RATE_DOUBLE_NSS_40M_11AX           423 /* 2个空间流20MHz的最大Long GI速率 */
#define MAC_MAX_RATE_DOUBLE_NSS_80M_11AX           869 /* 2个空间流20MHz的最大Long GI速率 */

#define MAC_TRIGGER_FRAME_PADDING_DURATION0US       0
#define MAC_TRIGGER_FRAME_PADDING_DURATION8US       1
#define MAC_TRIGGER_FRAME_PADDING_DURATION16US      2

/*****************************************************************************
  3 枚举定义
*****************************************************************************/
/*****************************************************************************
  3.1 基本枚举类型
*****************************************************************************/
/* TID个数为8,0~7 */
#define WLAN_TID_MAX_NUM                    WLAN_TIDNO_BUTT

/* 按照80211-2012 协议 Table 8-101 AKM suite selectors 定义 */
#define WLAN_AUTH_SUITE_RSV              0
#define WLAN_AUTH_SUITE_1X               1
#define WLAN_AUTH_SUITE_PSK              2
#define WLAN_AUTH_SUITE_FT_1X            3
#define WLAN_AUTH_SUITE_FT_PSK           4
#define WLAN_AUTH_SUITE_1X_SHA256        5
#define WLAN_AUTH_SUITE_PSK_SHA256       6
#define WLAN_AUTH_SUITE_TDLS             7
#define WLAN_AUTH_SUITE_SAE_SHA256       8
#define WLAN_AUTH_SUITE_FT_SHA256        9
#define WLAN_AUTH_SUITE_OWE              18

#define MAC_OWE_GROUP_SUPPORTED_NUM 3
#define MAC_OWE_GROUP_19 19
#define MAC_OWE_GROUP_20 20
#define MAC_OWE_GROUP_21 21

/* 协议能力枚举 */
typedef enum {
    WLAN_PROTOCOL_CAP_LEGACY,
    WLAN_PROTOCOL_CAP_HT,
    WLAN_PROTOCOL_CAP_VHT,
    WLAN_PROTOCOL_CAP_HE,
    WLAN_PROTOCOL_CAP_BUTT,
}wlan_protocol_cap_enum;
typedef td_u8 wlan_protocol_cap_enum_uint8;

/* 数据帧子类型枚举 */
typedef enum {
    MAC_DATA_DHCP                 = 0,
    MAC_DATA_EAPOL,
    MAC_DATA_ARP_RSP,
    MAC_DATA_ARP_REQ,  /* 注意: 前4个vip帧类型顺序勿变 */
    MAC_DATA_DHCPV6,
    MAC_DATA_PPPOE,
    MAC_DATA_WAPI,
    MAC_DATA_HS20,
    MAC_DATA_CHARIOT_SIG,  /* chariot 信令报文 */
    MAC_DATA_VIP_FRAME            = MAC_DATA_CHARIOT_SIG, /* 以上为VIP DATA FRAME */
    MAC_DATA_TDLS,
    MAC_DATA_VLAN,
    MAC_DATA_ND,
    MAC_DATA_ICMP_REQ,
    MAC_DATA_ICMP_RSP,
    MAC_DATA_ICMP_OTH,

    MAC_DATA_BUTT
} mac_data_type_enum_uint8;

/* diff_define wg. 以下数据结构dmac/device使用相同定义，hmac定义不同。待特性确认 */
/* 带宽能力枚举 */
typedef enum {
    WLAN_BW_CAP_20M,
    WLAN_BW_CAP_40M,
    WLAN_BW_CAP_80M,
    WLAN_BW_CAP_ER_SU_106_TONE,
    WLAN_STD_BW_CAP_BUTT = (WLAN_BW_CAP_ER_SU_106_TONE + 1), /* 主流带宽(20/40/80M)的分界 */
    WLAN_BW_CAP_160M,
    WLAN_BW_CAP_80PLUS80,    /* 工作在80+80 */
    WLAN_BW_CAP_5M,
    WLAN_BW_CAP_10M,

    WLAN_BW_CAP_BUTT
} wlan_bw_cap_enum;
typedef osal_u8 wlan_bw_cap_enum_uint8;

/* monitor类型枚举 */
typedef enum {
    WLAN_MONITOR_OFF,                    /* 关闭报文上报 */
    WLAN_MONITOR_MCAST_DATA,             /* 上报组播(广播)数据包 */
    WLAN_MONITOR_UCAST_DATA,             /* 上报单播数据包 */
    WLAN_MONITOR_MCAST_MANAGEMENT,       /* 上报组播(广播)管理包 */
    WLAN_MONITOR_UCAST_MANAGEMENT,       /* 上报单播管理包 */
    WLAN_MONITOR_BUTT
} wlan_monitor_enum;
typedef td_u8 wlan_monitor_enum_uint8;

/*****************************************************************************
  3.4 VHT枚举类型
*****************************************************************************/
/* WIFI协议类型定义 */
/* Note: 此定义若修改, 需要相应更新g_auc_default_mac_to_phy_protocol_mapping数组值 */
typedef enum {
    WLAN_LEGACY_11A_MODE            = 0,    /* 11a, 5G, OFDM */
    WLAN_LEGACY_11B_MODE            = 1,    /* 11b, 2.4G */
    WLAN_LEGACY_11G_MODE            = 2,    /* 旧的11g only已废弃, 2.4G, OFDM */
    WLAN_MIXED_ONE_11G_MODE         = 3,    /* 11bg, 2.4G */
    WLAN_MIXED_TWO_11G_MODE         = 4,    /* 11g only, 2.4G */
    WLAN_HT_MODE                    = 5,    /* 11n(11bgn或者11an，根据频段判断) */
    WLAN_VHT_MODE                   = 6,    /* 11ac */
    WLAN_HT_ONLY_MODE               = 7,    /* 11n only mode,只有带HT的设备才可以接入 */
    WLAN_VHT_ONLY_MODE              = 8,    /* 11ac only mode 只有带VHT的设备才可以接入 */
    WLAN_HT_11G_MODE                = 9,    /* 11ng,不包括11b */
    WLAN_HE_MODE                    = 10,   /* 11ax */

    WLAN_PROTOCOL_BUTT
} wlan_protocol_enum;
typedef td_u8 wlan_protocol_enum_uint8;

/* vap协议最大能力，供上层接口调用 */
typedef enum {
    WLAN_PROTOCOL_LEGACY_11A_MODE,
    WLAN_PROTOCOL_LEGACY_11B_MODE,
    WLAN_PROTOCOL_LEGACY_11G_MODE,
    WLAN_PROTOCOL_HT_MODE,
    WLAN_PROTOCOL_VHT_MODE,
    WLAN_PROTOCOL_HE_MODE,

    WLAN_PROTOCOL_MODE_BUTT
} wlan_protocol_mode_enum;
typedef td_u8 wlan_protocol_mode_enum_uint8;

/* 重要:仅限描述符接口使用(表示发送该帧使用的pramble类型)，0表示short preamble; 1表示long preamble */
typedef enum {
    WLAN_LEGACY_11B_DSCR_SHORT_PREAMBLE  = 0,
    WLAN_LEGACY_11B_DSCR_LONG_PREAMBLE   = 1,

    WLAN_LEGACY_11B_PREAMBLE_BUTT
}wlan_11b_dscr_preamble_enum;
typedef td_u8 wlan_11b_dscr_preamble_enum_uint8;

typedef enum {
    WLAN_11B_1_M_BPS = 0,
    WLAN_11B_2_M_BPS = 1,
    WLAN_11B_5_HALF_M_BPS = 2,
    WLAN_11B_11_M_BPS = 3,
    WLAN_11B_BUTT,
} wlan_11b_rate_index_enum;
typedef osal_u8 wlan_11b_rate_index_enum_uint8;

typedef enum {
    WLAN_11AG_6M_BPS = 0,
    WLAN_11AG_9M_BPS = 1,
    WLAN_11AG_12M_BPS = 2,
    WLAN_11AG_18M_BPS = 3,
    WLAN_11AG_24M_BPS = 4,
    WLAN_11AG_36M_BPS = 5,
    WLAN_11AG_48M_BPS = 6,
    WLAN_11AG_54M_BPS = 7,
    WLAN_11AG_BUTT,
} wlan_11ag_rate_value_enum;
typedef osal_u8 wlan_legacy_ofdm_mcs_enum_uint8;

typedef enum {
#if (WLAN_SINGLE_NSS <= WLAN_MAX_NSS_NUM)
    WLAN_HT_SINGLE_NSS_START = 0,
    WLAN_HT_MCS0 = WLAN_HT_SINGLE_NSS_START,
    WLAN_HT_MCS1,
    WLAN_HT_MCS2,
    WLAN_HT_MCS3,
    WLAN_HT_MCS4,
    WLAN_HT_MCS5,
    WLAN_HT_MCS6,
    WLAN_HT_MCS7,
    WLAN_HT_SINGLE_NSS_END = WLAN_HT_MCS7,
#endif
#if (WLAN_DOUBLE_NSS <= WLAN_MAX_NSS_NUM)
    WLAN_HT_DOUBLE_NSS_START,
    WLAN_HT_MCS8 = WLAN_HT_DOUBLE_NSS_START,
    WLAN_HT_MCS9,
    WLAN_HT_MCS10,
    WLAN_HT_MCS11,
    WLAN_HT_MCS12,
    WLAN_HT_MCS13,
    WLAN_HT_MCS14,
    WLAN_HT_MCS15,
    WLAN_HT_DOUBLE_NSS_END = WLAN_HT_MCS15,
#endif
    WLAN_HT_MCS_BUTT
} wlan_ht_mcs_enum;
typedef osal_u8 wlan_ht_mcs_enum_uint8;

#define WLAN_HT_MCS32                       32
#define WLAN_MIN_MPDU_LEN_FOR_MCS32         12
#define WLAN_MIN_MPDU_LEN_FOR_MCS32_SHORTGI 13
#define WLAN_MIN_MPDU_LEN_FOR_HE            80


typedef enum {
    WLAN_NO_SOUNDING                = 0,
    WLAN_NDP_SOUNDING               = 1,
    WLAN_STAGGERED_SOUNDING         = 2,
    WLAN_LEGACY_SOUNDING            = 3,

    WLAN_SOUNDING_BUTT
} wlan_sounding_enum;
typedef td_u8 wlan_sounding_enum_uint8;

#ifdef _PRE_WLAN_FEATURE_DFX_CHR
typedef enum {
    CHR_ERR_CODE_PLL = 0,
    CHR_ERR_CODE_RX_SARADC_OFFSET,
    CHR_ERR_CODE_RX_SARADC_DELAY,
    CHR_ERR_CODE_RX_LPF_WIFI,
    CHR_ERR_CODE_RX_LPF_BLE,
    CHR_ERR_CODE_RX_RFDC_WIFI,
    CHR_ERR_CODE_RX_RFDC_BLE,
    CHR_ERR_CODE_RX_DIGDC,
    CHR_ERR_CODE_RX_DIGDC_TRACK,
    CHR_ERR_CODE_RX_DC_OFFSET,
    CHR_ERR_CODE_RX_IQ_CALI,
    CHR_ERR_CODE_RX_IQ_MANUAL,
    CHR_ERR_CODE_RX_LCTUNE,
    CHR_ERR_CODE_TX_LPF_WIFI,
    CHR_ERR_CODE_TX_LPF_BLE,
    CHR_ERR_CODE_TX_PA,
    CHR_ERR_CODE_TX_IQDC,
    CHR_ERR_CODE_TX_PADIG,
    CHR_ERR_CODE_TX_PADIGPOWDB,
    CHR_ERR_CODE_BUTT
} chr_cali_err_code_enum;
#endif

/*****************************************************************************
  3.4 算法宏,枚举类型
*****************************************************************************/

/* CCA_OPT工作模式 */
#define WLAN_CCA_OPT_DISABLE                0   /* 不做任何优化 */
#define WLAN_CCA_OPT_ENABLE                 1   /* 不做同频识别的EDCA优化 */

/* EDCA_OPT STA模式下工作模式 */
#define WLAN_EDCA_OPT_STA_DISABLE           0   /* 不做任何优化 */
#define WLAN_EDCA_OPT_STA_ENABLE            1   /* 不做同频识别的EDCA优化 */

/* EDCA_OPT AP模式下工作模式 */
#define WLAN_EDCA_OPT_AP_EN_DISABLE         0   /* 不做任何优化 */
#define WLAN_EDCA_OPT_AP_EN_DEFAULT         1   /* 不做同频识别的EDCA优化 */
#define WLAN_EDCA_OPT_AP_EN_WITH_COCH       2   /* 带有同频识别的优化 */

/* 弱干扰免疫算法使能模式 */
#define WLAN_ANTI_INTF_EN_OFF               0   /* 算法关闭 */
#define WLAN_ANTI_INTF_EN_ON                1   /* 算法打开 */
#define WLAN_ANTI_INTF_EN_PROBE             2   /* 探测机制进行算法开/关 */

/* 动态byass外置LNA使能模式 */
#define WLAN_EXTLNA_BYPASS_EN_OFF               0   /* 动态bypass外置LNA关 */
#define WLAN_EXTLNA_BYPASS_EN_ON                1   /* 动态bypass外置LNA开 */
#define WLAN_EXTLNA_BYPASS_EN_FORCE             2   /* 动态bypass外置LNA强制开 */

/* DFS使能模式 */
#define WLAN_DFS_EN_OFF               0   /* 算法关闭 */
#define WLAN_DFS_EN_ON                1   /* 算法打开,检测到雷达切换信道 */

/*****************************************************************************
  2.5 过滤命令宏定义
*****************************************************************************/
#define  WLAN_CFG_OTHER_BSS_BEACON_FLT_EN BIT25       /* 其他BSS网络的Beacon帧过滤 */
#define  WLAN_CFG_WDS_FLT_EN BIT24                    /* WDS帧是否进行过滤 */
#define  WLAN_CFG_WDS_BCMC_FLT_EN BIT23               /* 广播WDS帧是否过滤开关 */
#define  WLAN_CFG_WDS_NON_DIRECT_FLT_EN BIT22         /* 非直接单播WDS帧是否过滤开关 */


/*****************************************************************************
  3.7 报文类型枚举
*****************************************************************************/
typedef enum {
    WLAN_PKT_TYPE_BEACON,
    WLAN_PKT_TYPE_PROBE_REQ,
    WLAN_PKT_TYPE_PROBE_RESP,
    WLAN_PKT_TYPE_ASSOC_REQ,
    WLAN_PKT_TYPE_ASSOC_RESP,
    WLAN_PKT_TYPE_BUTT
} wlan_pkt_type;
typedef td_u8 wlan_pkt_type_uint8;

/*****************************************************************************
  3.12 HE 枚举
*****************************************************************************/
/* 在host不具备 he 能力时，可以扫描支持11ax的设备 */
typedef enum {
    WLAN_GI_LONG = 0,  /* 11ax:3.2us,非11ax:0.8us */
    WLAN_GI_SHORT,     /* 11ax:0.8us,非11ax:0.4us */
    WLAN_GI_MID,       /* only 11ax:1.6us */
    WLAN_NON_HE_GI_TYPE_BUTT = WLAN_GI_MID,
    WLAN_HE_GI_TYPE_BUTT
} wlan_gi_type_enum;
typedef osal_u8 wlan_gi_type_enum_uint8;

typedef enum {
    WLAN_HE_MCS0,
    WLAN_HE_MCS1,
    WLAN_HE_MCS2,
    WLAN_HE_MCS3,
    WLAN_HE_MCS4,
    WLAN_HE_MCS5,
    WLAN_HE_MCS6,
    WLAN_HE_MCS7,
    WLAN_HE_MCS8,
    WLAN_HE_MCS9,
#ifdef _PRE_WLAN_FEATURE_1024QAM
    WLAN_HE_MCS10,
    WLAN_HE_MCS11,
#endif
    WLAN_HE_MCS_BUTT
} wlan_he_mcs_enum;
typedef osal_u8 wlan_he_mcs_enum_uint8;

typedef enum {
    WLAN_HE_SU_PPDU_TYPE       = 4,
    WLAN_HE_MU_PPDU_TYPE       = 5,
    WLAN_HE_EXT_SU_PPDU_TPYE   = 6,
    WLAN_HE_TRIG_PPDU_TYPE     = 7,
    WLAN_HE_PPDU_TYPE_BUTT
} wlan_he_protocol_mode;
typedef osal_u8 wlan_he_protocol_mode_uint8;

typedef enum {
    WLAN_HE_ER_MCS0 = WLAN_HE_MCS0,
    WLAN_HE_ER_MCS1,
    WLAN_HE_ER_MCS2,
    WLAN_HE_ER_DCM_MCS0,
    WLAN_HE_ER_BUTT
}wlan_he_er_mcs_enum;
typedef osal_u8 wlan_he_er_mcs_enum_uint8;

typedef enum {
    WLAN_HE_LTF_1X = 0,
    WLAN_HE_LTF_2X,
    WLAN_HE_LTF_4X,
    WLAN_HE_LTF_TYPE_BUTT
} wlan_he_ltf_type_enum;
typedef osal_u8 wlan_he_ltf_type_enum_uint8;

/*****************************************************************************
  3.13 11ax trigger帧类型
*****************************************************************************/
typedef enum {
    WLAN_HE_TRIG_TYPE_BASIC      = 0,
    WLAN_HE_TRIG_TYPE_BFRP       = 1,
    WLAN_HE_TRIG_TYPE_MU_BAR     = 2,
    WLAN_HE_TRIG_TYPE_MU_RTS     = 3,
    WLAN_HE_TRIG_TYPE_BSRP       = 4,
    WLAN_HE_TRIG_TYPE_GCR_MU_BAR = 5,
    WLAN_HE_TRIG_TYPE_BQRP       = 6,
    WLAN_HE_TRIG_TYPE_NFRP       = 7,
    WLAN_HE_TRIG_TYPE_MAX        = 8,
} wlan_he_trig_type;

typedef enum {
    WLAN_HE_RU_26,
    WLAN_HE_RU_52,
    WLAN_HE_RU_106,
    WLAN_HE_RU_242,
    WLAN_HE_RU_484,
    WLAN_HE_RU_996,
    /* WLAN_HE_RU_1992,160M */
    WLAN_HE_RU_BUTT
} wlan_he_ru_enum;
typedef osal_u8 wlan_he_ru_enum_uint8;
/*****************************************************************************
  3.14 A-Control类型
*****************************************************************************/
typedef enum {
    WLAN_HTC_A_CONTROL_TYPE_TRS = 0,
    WLAN_HTC_A_CONTROL_TYPE_OM  = 1,
    WLAN_HTC_A_CONTROL_TYPE_HLA = 2,
    WLAN_HTC_A_CONTROL_TYPE_BSR = 3,
    WLAN_HTC_A_CONTROL_TYPE_UPH = 4,
    WLAN_HTC_A_CONTROL_TYPE_BQR = 5,
    WLAN_HTC_A_CONTROL_TYPE_CAS = 6,
    WLAN_HTC_A_CONTROL_TYPE_ONES = 15
} wlan_he_a_control_type;

/*****************************************************************************
    STRUCT定义
*****************************************************************************/
typedef struct {
    td_u32                      cipher;                      /* 加密类型 */
    td_u32                      key_len;                     /* 密钥长 */
    td_u32                      seq_len;                     /* sequnece长 */
    td_u8                       key[WLAN_WPA_KEY_LEN];      /* 密钥 */
    td_u8                       seq[WLAN_WPA_SEQ_LEN];      /* sequence */
} wlan_priv_key_param_stru;

typedef struct {
    osal_u16  chan_ratio_20m;
    osal_u16  chan_ratio_40m;
    osal_u16  chan_ratio_80m;
    osal_char    free_power_20m;
    osal_char    free_power_40m;
    osal_char    free_power_80m;
    osal_u8   auc_resv[3];         /* 预留3字节对齐 */
    osal_u8   _rom[4];            /* 4字节rom地址 */
} wlan_chan_ratio_stru;

typedef struct {
    osal_u8                          user_mac_addr[WLAN_MAC_ADDR_LEN];     /* 切换对应的AP MAC地址 */
    wlan_m2s_action_type_enum_uint8  action_type;                          /* 切换对应需要发送的action帧类型，需要和vap下ori交互处理 */
    oal_bool_enum_uint8              m2s_result;                           /* 切换是否符合初始action帧，作为vap切换成功失败标记 */
} wlan_m2s_mgr_vap_stru;

/* DMAC SCAN 信道统计测量结果结构体 */
typedef struct {
    td_u8   channel_number;      /* 信道号 */
    td_u8   stats_valid;
    td_u8   stats_cnt;           /* 信道繁忙度统计次数 */
    td_u8   free_power_cnt;      /* 信道空闲功率 */

    /* PHY信道测量统计 */
    td_u8   bandwidth_mode;
    td_u8   auc_resv[1];
    td_s16   free_power_stats_20m;
    td_s16   free_power_stats_40m;
    td_s16   free_power_stats_80m;

    /* MAC信道测量统计 */
    td_u32  total_stats_time_us;
    td_u32  total_free_time_20m_us;
    td_u32  total_free_time_40m_us;
    td_u32  total_free_time_80m_us;
    td_u32  total_free_time_sec20m_us;
    td_u32  total_send_time_us;
    td_u32  total_recv_time_us;

    td_u8   radar_detected;
    td_u8   radar_bw;
    td_u8   radar_type;
    td_u8   radar_freq_offset;

    td_s16  s_free_power_stats_160m;
    td_u16  phy_total_stats_time_ms;
} wlan_scan_chan_stats_stru;

/*****************************************************************************
    OTHERS定义
*****************************************************************************/
/*****************************************************************************
 功能描述  : 获取帧类型
*****************************************************************************/
static inline td_u8  wlan_hdr_get_frame_type(const td_u8 *header)
{
    return ((header[0] & (0x0c)) >> 2); /* 2: 右移2位 */
}

static inline wlan_vht_mcs_enum_uint8 wlan_get_vht_max_support_mcs(osal_u16 vht_mcs_map)
{
    return ((vht_mcs_map == 3) ? WLAN_INVALD_VHT_MCS : /* 11ac 3: 速率档位 */
        (vht_mcs_map == 2) ? WLAN_VHT_MCS9 : (vht_mcs_map == 1) ? WLAN_VHT_MCS8 : WLAN_VHT_MCS7); /* 11ac 2: 速率档位 */
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of wlan_types.h */
