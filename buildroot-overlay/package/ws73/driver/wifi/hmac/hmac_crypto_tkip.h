/*
 * Copyright (c) CompanyNameMagicTag 2020-2022. All rights reserved.
 * Description: tkip crypto head file.
 */

#ifndef __HMAC_CRYPTO_TKIP_H__
#define __HMAC_CRYPTO_TKIP_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "oal_net.h"
#include "wlan_types_common.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*****************************************************************************
  2 宏定义
*****************************************************************************/
#define IEEE80211_WEP_MICLEN        8   /* trailing MIC */

#define IEEE80211_FC1_DIR_NODS              0x00    /* STA->STA */
#define IEEE80211_FC1_DIR_TODS              0x01    /* STA->AP  */
#define IEEE80211_FC1_DIR_FROMDS            0x02    /* AP ->STA */
#define IEEE80211_FC1_DIR_DSTODS            0x03    /* AP ->AP  */

#define IEEE80211_NON_QOS_SEQ               16      /* index for non-QoS (including management) sequence number space */
#define IEEE80211_FC0_TYPE_MASK             0x0c
#define IEEE80211_FC0_SUBTYPE_QOS           0x80
#define IEEE80211_FC0_TYPE_DATA             0x08


#define WEP_IV_FIELD_SIZE       4       /* wep IV field size */
#define EXT_IV_FIELD_SIZE       4       /* ext IV field size */


/*****************************************************************************
  3 枚举定义
*****************************************************************************/


/*****************************************************************************
  4 全局变量声明
*****************************************************************************/


/*****************************************************************************
  5 消息头定义
*****************************************************************************/


/*****************************************************************************
  6 消息定义
*****************************************************************************/


/*****************************************************************************
  7 STRUCT定义
*****************************************************************************/


/*****************************************************************************
  8 UNION定义
*****************************************************************************/


/*****************************************************************************
  9 OTHERS定义
*****************************************************************************/


/*****************************************************************************
  10 函数声明
*****************************************************************************/

OAL_STATIC OAL_INLINE osal_u32 rotl(osal_u32 val, osal_s32 bits)
{
    return (val << (osal_u32)bits) | (val >> (osal_u32)(32 - bits));    /* 4*8bit共计32位 */
}


OAL_STATIC OAL_INLINE osal_u32 rotr(osal_u32 val, osal_s32 bits)
{
    return (val >> (osal_u32)bits) | (val << (osal_u32)(32 - bits));    /* 4*8bit共计32位 */
}


OAL_STATIC OAL_INLINE osal_u32 xswap(osal_u32 val)
{
    return ((val & 0x00ff00ff) << 8) | ((val & 0xff00ff00) >> 8);           /* 左移8位、右移8位 */
}


OAL_STATIC OAL_INLINE osal_u32 get_le32_split(osal_u8 b0, osal_u8 b1, osal_u8 b2, osal_u8 b3)
{
    return b0 | (b1 << 8) | (b2 << 16) | (b3 << 24);    /* 左移8、16、24位 */
}

OAL_STATIC OAL_INLINE osal_u32 get_le32(const osal_u8 *p)
{
    return get_le32_split(p[0], p[1], p[2], p[3]);      /* uint8的地址第0、1、2、3位得到uint32 */
}
OAL_STATIC OAL_INLINE osal_void put_le32(osal_u8 *p, osal_u32 v)
{
    p[0] = (osal_u8)v;
    p[1] = (osal_u8)(v >> 8);         /* 第1位右移8位 */
    p[2] = (osal_u8)(v >> 16);        /* 第2位右移16位 */
    p[3] = (osal_u8)(v >> 24);        /* 第3位右移24位 */
}

osal_u32 hmac_crypto_tkip_enmic_etc(wlan_priv_key_param_stru *pst_key, oal_netbuf_stru *pst_netbuf);
osal_u32 hmac_crypto_tkip_demic_etc(wlan_priv_key_param_stru *pst_key, oal_netbuf_stru *pst_netbuf);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of hmac_crypto_tkip.h */
