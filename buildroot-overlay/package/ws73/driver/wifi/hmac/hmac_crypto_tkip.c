/*
 * Copyright (c) CompanyNameMagicTag 2020-2022. All rights reserved.
 * Description: tkip加解密.
 */

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "hmac_crypto_tkip.h"
#include "oam_ext_if.h"
#include "wlan_spec.h"
#include "mac_frame.h"
#include "dmac_ext_if_hcm.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
#define     MAX_TKIP_PN_GAP_ERR    1  /* Max. gap in TKIP PN before doing MIC sanity check */


/*****************************************************************************
  3 函数实现
*****************************************************************************/
OAL_STATIC osal_void michael_block(osal_u32 *msb, osal_u32 *lsb)
{
    (*lsb) ^= rotl((*msb), 17); /* 17:循环移动位数 */
    (*msb) += (*lsb);
    (*lsb) ^= xswap(*msb);
    (*msb) += (*lsb);
    (*lsb) ^= rotl((*msb), 3); /* 3:循环移动位数 */
    (*msb) += (*lsb);
    (*lsb) ^= rotr((*msb), 2); /* 2:循环移动位数 */
    (*msb) += (*lsb);
}

/*****************************************************************************
 函 数 名  : hmac_crypto_tkip_michael_hdr
 功能描述  : mic所需的源mac，目的mac，及TID信息构造
*****************************************************************************/
OAL_STATIC osal_void hmac_crypto_tkip_michael_hdr(mac_ieee80211_frame_stru *frame_header,
    osal_u8 hdr[16])
{
    mac_ieee80211_frame_addr4_stru *frame_4addr_hdr = OAL_PTR_NULL;
    osal_u8 frame_dir;

    frame_4addr_hdr = (mac_ieee80211_frame_addr4_stru *)(uintptr_t)frame_header;

    frame_dir  = (frame_4addr_hdr->frame_control.to_ds != 0) ? 1 : 0;
    frame_dir += (frame_4addr_hdr->frame_control.from_ds != 0) ? 2 : 0; /* 报文方向，2表示from ds */

    switch (frame_dir) {
        case IEEE80211_FC1_DIR_NODS:
            if (memcpy_s(hdr, WLAN_MAC_ADDR_LEN, frame_4addr_hdr->address1, WLAN_MAC_ADDR_LEN) != EOK) {
                oam_error_log0(0, OAM_SF_ANY, "{hmac_crypto_tkip_michael_hdr::memcpy_s NODS addr1 error}");
            }
            if (memcpy_s(hdr + WLAN_MAC_ADDR_LEN, WLAN_MAC_ADDR_LEN,
                frame_4addr_hdr->address2, WLAN_MAC_ADDR_LEN) != EOK) {
                oam_error_log0(0, OAM_SF_ANY, "{hmac_crypto_tkip_michael_hdr::memcpy_s NODS addr2 error}");
            }
            break;
        case IEEE80211_FC1_DIR_TODS:
            if (memcpy_s(hdr, WLAN_MAC_ADDR_LEN, frame_4addr_hdr->address3, WLAN_MAC_ADDR_LEN) != EOK) {
                oam_error_log0(0, OAM_SF_ANY, "{hmac_crypto_tkip_michael_hdr::memcpy_s TODS addr3 error}");
            }
            if (memcpy_s(hdr + WLAN_MAC_ADDR_LEN, WLAN_MAC_ADDR_LEN,
                frame_4addr_hdr->address2, WLAN_MAC_ADDR_LEN) != EOK) {
                oam_error_log0(0, OAM_SF_ANY, "{hmac_crypto_tkip_michael_hdr::memcpy_s TODS addr2 error}");
            }
            break;
        case IEEE80211_FC1_DIR_FROMDS:
            if (memcpy_s(hdr, WLAN_MAC_ADDR_LEN, frame_4addr_hdr->address1, WLAN_MAC_ADDR_LEN) != EOK) {
                oam_error_log0(0, OAM_SF_ANY, "{hmac_crypto_tkip_michael_hdr::memcpy_s FROMDS addr1 error}");
            }
            if (memcpy_s(hdr + WLAN_MAC_ADDR_LEN, WLAN_MAC_ADDR_LEN,
                frame_4addr_hdr->address3, WLAN_MAC_ADDR_LEN) != EOK) {
                oam_error_log0(0, OAM_SF_ANY, "{hmac_crypto_tkip_michael_hdr::memcpy_s FROMDS addr3 error}");
            }
            break;
        default:
            if (memcpy_s(hdr, WLAN_MAC_ADDR_LEN, frame_4addr_hdr->address3, WLAN_MAC_ADDR_LEN) != EOK) {
                oam_error_log0(0, OAM_SF_ANY, "{hmac_crypto_tkip_michael_hdr::memcpy_s default addr3 error}");
            }
            if (memcpy_s(hdr + WLAN_MAC_ADDR_LEN, WLAN_MAC_ADDR_LEN,
                frame_4addr_hdr->address4, WLAN_MAC_ADDR_LEN) != EOK) {
                oam_error_log0(0, OAM_SF_ANY, "{hmac_crypto_tkip_michael_hdr::memcpy_s default addr4 error}");
            }
            break;
    }
    hdr[12] = 0;    /* hdr第12位，表示qc_tid */

    if (frame_4addr_hdr->frame_control.sub_type == WLAN_QOS_DATA) {
        if (frame_dir == IEEE80211_FC1_DIR_DSTODS) {
            hdr[12] =                                                                   /* hdr第12位，表示qc_tid */
                ((mac_ieee80211_qos_frame_addr4_stru *)(uintptr_t)frame_4addr_hdr)->qc_tid;
        } else {
            hdr[12] = ((mac_ieee80211_qos_frame_stru *)(uintptr_t)frame_4addr_hdr)->qc_tid; /* hdr第12位，表示qc_tid */
        }
    }
    hdr[13] = hdr[14] = hdr[15] = 0; /* 13、14、15位reserved */
}

OAL_STATIC OAL_INLINE osal_void hmac_crypto_tkip_michael_mic_update_msb(oal_netbuf_stru *pst_netbuf, osal_u32 *space,
    osal_u8 **data, osal_u8 *data_next, osal_u32 *msb)
{
    switch (*space) {
        case 1:
            (*msb) ^= get_le32_split((*data)[0], data_next[0],
                data_next[1], data_next[2]);    /* 根据space取到data_next的第2位 */
            *data = data_next + 3;               /* 根据space偏移3位 */
            *space = OAL_NETBUF_LEN(pst_netbuf) - 3;  /* 根据space偏移3位 */
            break;
        case 2: /* space为2 */
            (*msb) ^= get_le32_split((*data)[0], (*data)[1], data_next[0], data_next[1]);
            *data = data_next + 2;               /* 根据space偏移2位 */
            *space = OAL_NETBUF_LEN(pst_netbuf) - 2;  /* 根据space偏移2位 */
            break;
        case 3: /* space为3 */
            (*msb) ^= get_le32_split((*data)[0], (*data)[1],
                (*data)[2], data_next[0]);         /* 根据space取到data的第2位 */
            *data = data_next + 1;
            *space = OAL_NETBUF_LEN(pst_netbuf) - 1;
            break;
        default:
            break;
    }
}

OAL_STATIC OAL_INLINE osal_void hmac_crypto_tkip_michael_mic_prepare_last_block(const osal_u8 *data,
    osal_u32 data_len, osal_u32 *msb)
{
    /* Last block and padding (0x5a, 4..7 x 0) */
    switch (data_len) {
        case 0:
            (*msb) ^= get_le32_split(0x5a, 0, 0, 0);
            break;
        case 1:
            (*msb) ^= get_le32_split(data[0], 0x5a, 0, 0);
            break;
        case 2: /* data_len为2 */
            (*msb) ^= get_le32_split(data[0], data[1], 0x5a, 0);
            break;
        case 3: /* data_len为3 */
            (*msb) ^= get_le32_split(data[0], data[1], data[2], 0x5a);  /* 2 索引 */
            break;
        default:
            break;
    }
}

/*****************************************************************************
 函 数 名  : hmac_crypto_tkip_michael_mic
 功能描述  : 生成mic校验码
*****************************************************************************/
OAL_STATIC osal_u32 hmac_crypto_tkip_michael_mic(osal_u8 *key, oal_netbuf_stru *pst_netbuf,
    mac_ieee80211_frame_stru *frame_header, osal_u32 offset, osal_u32 data_len, osal_u8 mic[IEEE80211_WEP_MICLEN])
{
    osal_u8                  hdr[16]    = {0};
    osal_u32                 lsb, msb, space, i;
    const osal_u8           *data = OAL_PTR_NULL;
    const osal_u8           *data_next = OAL_PTR_NULL;

    hmac_crypto_tkip_michael_hdr(frame_header, (osal_u8 *)hdr);

    msb = get_le32(key);
    lsb = get_le32(key + 4); /* 偏移4位取lsb */

    /* Michael MIC pseudo header: DA, SA, 3 x 0, Priority */
    for (i = 0; i < 4; i++) {                /* 偏移0/4/8/12位异或 */
        msb ^= get_le32(&hdr[4 * i]); /* 偏移0/4/8/12位异或 */
        michael_block(&msb, &lsb);
    }

    /* first buffer has special handling */
    data = OAL_NETBUF_DATA(pst_netbuf) + offset;
    space = OAL_NETBUF_LEN(pst_netbuf) - offset;
    for (;;) {
        space = (space > data_len) ? data_len : space;

        /* collect 32-bit blocks from current buffer */
        while (space >= sizeof(osal_u32)) {
            msb ^= get_le32(data);
            michael_block(&msb, &lsb);
            data  += sizeof(osal_u32);
            space -= (osal_u32)sizeof(osal_u32);
            data_len -= (osal_u32)sizeof(osal_u32);
        }

        if (data_len < sizeof(osal_u32)) {
            break;
        }

        pst_netbuf = oal_netbuf_list_next(pst_netbuf);
        if (pst_netbuf == NULL) {
            return OAL_ERR_CODE_SECURITY_BUFF_NUM;
        }
        if (space != 0) { /* Block straddles buffers, split references */
            data_next = OAL_NETBUF_DATA(pst_netbuf);
            if (OAL_NETBUF_LEN(pst_netbuf) < sizeof(osal_u32) - space) {
                return OAL_ERR_CODE_SECURITY_BUFF_LEN;
            }
            hmac_crypto_tkip_michael_mic_update_msb(pst_netbuf, &space, (osal_u8 **)&data, (osal_u8 *)data_next, &msb);
            michael_block(&msb, &lsb);
            data_len -= (osal_u32)sizeof(osal_u32);
        } else { /* Setup for next buffer */
            data = OAL_NETBUF_DATA(pst_netbuf);
            space = OAL_NETBUF_LEN(pst_netbuf);
        }
    }

    hmac_crypto_tkip_michael_mic_prepare_last_block(data, data_len, &msb);

    michael_block(&msb, &lsb);
    michael_block(&msb, &lsb);

    put_le32(mic, msb);
    put_le32(mic + 4, lsb);  /* mic高4位拼装 */

    return OAL_SUCC;
}
/*****************************************************************************
 函 数 名  : hmac_crypto_tkip_enmic_etc
 功能描述  : 添加tikip mic校验
*****************************************************************************/
osal_u32 hmac_crypto_tkip_enmic_etc(wlan_priv_key_param_stru *pst_key, oal_netbuf_stru *pst_netbuf)
{
    osal_u32                     hdrlen      = 0;  /* 发送时，pktlen里不含80211mac头 */
    osal_u32                     pktlen;
    osal_u32                     ul_ret;
    osal_u8                     *mic_tail   = OAL_PTR_NULL;
    osal_u8                     *tx_mic_key = OAL_PTR_NULL;
    osal_u8                      mic[IEEE80211_WEP_MICLEN] = {0};
    mac_tx_ctl_stru               *cb         = OAL_PTR_NULL;
    mac_ieee80211_frame_stru    *frame_new_header = OAL_PTR_NULL;

    if (((osal_u8)pst_key->cipher) != WLAN_80211_CIPHER_SUITE_TKIP) {
        oam_warning_log1(0, OAM_SF_WPA, "hmac_crypto_tkip_enmic_etc::pst_key->cipher=0x%x", pst_key->cipher);
        return OAL_ERR_CODE_SECURITY_CHIPER_TYPE;
    }

    /* 1.1 计算整个报文的长度，不处理存在多个netbuf的情况 */
    pktlen = OAL_NETBUF_LEN(pst_netbuf);
    if (OAL_NETBUF_NEXT(pst_netbuf) != OAL_PTR_NULL) {
        return OAL_ERR_CODE_SECURITY_BUFF_NUM;
    }

    /* 2.1 在netbuf上增加mic空间 */
    if (oal_netbuf_tailroom(pst_netbuf) < IEEE80211_WEP_MICLEN) {
        /* 2.2 如果原来的netbuf长度不够，需要重新申请 */
        oal_netbuf_expand_head(pst_netbuf, 0, IEEE80211_WEP_MICLEN, OSAL_GFP_KERNEL); /* 最后这个参数未使用 */
    }

    /* 3.1 获取mic及密钥 */
    mic_tail   = (osal_u8 *)OAL_NETBUF_TAIL(pst_netbuf);
    tx_mic_key = pst_key->key + WLAN_TEMPORAL_KEY_LENGTH;

    oal_netbuf_put(pst_netbuf, IEEE80211_WEP_MICLEN);

    cb    = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_netbuf);
    frame_new_header = (mac_ieee80211_frame_stru *)(pst_netbuf->data - mac_get_cb_frame_header_length(cb));
    mac_get_cb_frame_header_addr(cb) = frame_new_header;

    /* 4.1 计算mic */
    ul_ret = hmac_crypto_tkip_michael_mic(tx_mic_key, pst_netbuf, mac_get_cb_frame_header_addr(cb), hdrlen,
                                          pktlen - hdrlen, (osal_u8 *)mic);
    if (ul_ret != OAL_SUCC) {
        return ul_ret;
    }

    /* 4.1 拷贝mic到帧尾部 */
    if (memcpy_s(mic_tail, IEEE80211_WEP_MICLEN, mic, IEEE80211_WEP_MICLEN) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_crypto_tkip_enmic_etc::memcpy_s error}");
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_crypto_tkip_demic_etc
 功能描述  : 比较tkip mic校验，并剥掉mic尾
*****************************************************************************/
osal_u32 hmac_crypto_tkip_demic_etc(wlan_priv_key_param_stru *pst_key, oal_netbuf_stru *pst_netbuf)
{
    mac_rx_ctl_stru          *cb                              = OAL_PTR_NULL;
    osal_u32                hdrlen;                    /* 接收时，pktlen里包含80211mac头 */
    osal_u8                *rx_mic_key                      = OAL_PTR_NULL;
    osal_u32                ul_ret;
    osal_u32                pktlen;
    osal_u8                 mic[IEEE80211_WEP_MICLEN]       = {0};
    osal_u8                 mic_peer[IEEE80211_WEP_MICLEN]  = {0};

    if (((osal_u8)pst_key->cipher) != WLAN_80211_CIPHER_SUITE_TKIP) {
        oam_warning_log1(0, OAM_SF_WPA, "hmac_crypto_tkip_demic_etc::pst_key->cipher=0x%x", pst_key->cipher);
        return OAL_ERR_CODE_SECURITY_CHIPER_TYPE;
    }

    /* 1.1 计算整个报文的长度，不处理存在多个netbuf的情况 */
    pktlen         = OAL_NETBUF_LEN(pst_netbuf);
    if (oal_netbuf_list_next(pst_netbuf) != OAL_PTR_NULL) {
        return OAL_ERR_CODE_SECURITY_BUFF_NUM;
    }

    /* 2.1 从CB中获取80211头长度 */
    cb    = (mac_rx_ctl_stru *)oal_netbuf_cb(pst_netbuf);
    hdrlen = cb->mac_header_len;

    /* 3.1 只处理数据帧 */

    /* 4.1 获取解密密钥，接收密钥需要偏移8个字节 */
    rx_mic_key = pst_key->key + WLAN_TEMPORAL_KEY_LENGTH + WLAN_MIC_KEY_LENGTH;

    /* 5.1 计算mic */
    ul_ret = hmac_crypto_tkip_michael_mic(rx_mic_key, pst_netbuf,
        (mac_ieee80211_frame_stru *)(uintptr_t)mac_get_rx_cb_mac_header_addr(cb), hdrlen,
        pktlen - (hdrlen + IEEE80211_WEP_MICLEN), mic);
    if (ul_ret != OAL_SUCC) {
        return ul_ret;
    }

    /* 6.1 获取对端的mic并跟本地计算的mic进行比较 */
    oal_netbuf_copydata(pst_netbuf, pktlen - IEEE80211_WEP_MICLEN, (osal_void *)mic_peer, IEEE80211_WEP_MICLEN,
        IEEE80211_WEP_MICLEN);
    if (osal_memcmp(mic, mic_peer, IEEE80211_WEP_MICLEN) != 0) {
        /* NB: 802.11 layer handles statistic and debug msg
        ieee80211_notify_michael_failure(vap, wh, k->wk_keyix); */
        return OAL_ERR_CODE_SECURITY_WRONG_KEY;
    }

    /* 7.1 去掉mic尾部 */
    oal_netbuf_trim(pst_netbuf, IEEE80211_WEP_MICLEN);
    return OAL_SUCC;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

