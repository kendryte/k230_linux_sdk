/*
 * Copyright (c) CompanyNameMagicTag 2020-2022. All rights reserved.
 * Description: wpi数据加解密.
 */

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "hmac_wapi.h"
#include "oal_ext_if.h"
#include "oal_types.h"
#include "mac_frame.h"
#include "hmac_wapi_sms4.h"
#include "hmac_wapi_wpi.h"
#include "mac_data.h"
#include "hmac_feature_dft.h"
#include "hmac_tx_data.h"
#include "oam_struct.h"
#include "frw_util_notifier.h"
#include "hmac_feature_interface.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_HMAC_WAPI_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
OAL_CONST osal_u8 g_auc_wapi_oui_etc[WAPI_IE_OUI_SIZE] = {0x00, 0x14, 0x72};

OAL_STATIC hmac_wapi_stru *g_wapi_userinfo[WLAN_USER_MAX_USER_LIMIT];

WIFI_HMAC_TCM_TEXT WIFI_TCM_TEXT OAL_STATIC hmac_wapi_stru *hmac_wapi_get_user_info(osal_u16 user_id)
{
    if (user_id >= WLAN_USER_MAX_USER_LIMIT) {
        return OAL_PTR_NULL;
    }
    return g_wapi_userinfo[user_id];
}
/*****************************************************************************
  3 函数实现
*****************************************************************************/

/*****************************************************************************
 功能描述  : 判断是否为qos帧
*****************************************************************************/
OAL_STATIC oal_bool_enum_uint8  hmac_wapi_is_qos_etc(const mac_ieee80211_frame_stru *pst_mac_hdr)
{
    if ((pst_mac_hdr->frame_control.type == WLAN_DATA_BASICTYPE) &&
        ((WLAN_QOS_DATA & pst_mac_hdr->frame_control.sub_type) != 0)) {
        return OAL_TRUE;
    }

    return OAL_FALSE;
}

/*****************************************************************************
 功能描述  : 计算mic数据，作为计算mic之用
*****************************************************************************/
OAL_STATIC osal_u32 hmac_wapi_calc_mic_data_etc(mac_ieee80211_frame_stru *pst_mac_hdr, osal_u8 mac_hdr_len,
    osal_u8 keyidx, osal_u8 *payload, osal_u16 pdu_len, osal_u8 *mic, osal_u16 mic_len)
{
    osal_u8       is_qos;
    osal_u8      *mic_oringin;
    osal_u32 frag_num_len = 2; /* 2:frag_num长度值 */

    unref_param(mac_hdr_len);

    memset_s(mic, mic_len, 0, mic_len);

    mic_oringin = mic;

    /* frame control */
    if (memcpy_s(mic, mic_len, (osal_u8 *)&(pst_mac_hdr->frame_control),
        OAL_SIZEOF(pst_mac_hdr->frame_control)) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_wapi_calc_mic_data_etc::memcpy_s frame_control error}");
    }
    mic[0] &= ~(BIT4 | BIT5 | BIT6);                 /* sub type */
    mic[1] &= ~(BIT3 | BIT4 | BIT5);                 /* retry, pwr Mgmt, more data */
    mic[1] |= BIT6;

    mic += OAL_SIZEOF(pst_mac_hdr->frame_control);

    /* addr1 */
    mac_get_address1((osal_u8 *)pst_mac_hdr, mic);
    mic += OAL_MAC_ADDR_LEN;

    /* addr2 */
    mac_get_address2((osal_u8 *)pst_mac_hdr, mic);
    mic += OAL_MAC_ADDR_LEN;

    /* 序列控制 */
    memset_s(mic, frag_num_len, 0, frag_num_len);
    mic[0] = (osal_u8)(pst_mac_hdr->frag_num);
    mic += frag_num_len; /* 2长度值 */

    /* addr3 */
    mac_get_address3((osal_u8 *)pst_mac_hdr, mic);
    mic += OAL_MAC_ADDR_LEN;

    /* 跳过addr4 */
    mic += OAL_MAC_ADDR_LEN;

    /* qos ctrl */
    is_qos = hmac_wapi_is_qos_etc(pst_mac_hdr);
    if (is_qos == OAL_TRUE) {
        mac_get_qos_ctrl((osal_u8 *)pst_mac_hdr, mic);
        mic += MAC_QOS_CTL_LEN;
    }

    /* keyidx + reserve总共2个字节 */
    *mic = keyidx;
    mic += 2; /* 2长度值 */

    /* 填充pdulen 协议写明大端字节序 */
    *mic = (osal_u8)((pdu_len & 0xff00) >> 8); /* 8偏移值 */
    *(mic + 1) = (osal_u8)(pdu_len & 0x00ff);
    mic += 2; /* 2长度值 */

    mic_oringin +=
        (hmac_wapi_is_qos_etc(pst_mac_hdr) == OAL_TRUE) ? SMS4_MIC_PART1_QOS_LEN : SMS4_MIC_PART1_NO_QOS_LEN;
    if (memcpy_s(mic_oringin, pdu_len, payload, pdu_len) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_wapi_calc_mic_data_etc::memcpy_s payload error}");
    }

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 申请空间，用于存放mic
*****************************************************************************/
OAL_STATIC osal_u8 *hmac_wapi_mic_alloc_etc(osal_u8 is_qos, osal_u16 pdu_len, osal_u16 *pus_mic_len)
{
    osal_u16      mic_part1_len;
    osal_u16      mic_part2_len;
    osal_u16      mic_len;

    mic_part1_len = (is_qos == OAL_TRUE) ? SMS4_MIC_PART1_QOS_LEN : SMS4_MIC_PART1_NO_QOS_LEN;

    /* 按照协议，补齐不足位，16字节对齐 */
    mic_part2_len = (osal_u16)((pdu_len + SMS4_PADDING_LEN - 1) & (~(SMS4_PADDING_LEN - 1)));

    mic_len = mic_part1_len + mic_part2_len;

    *pus_mic_len = mic_len;

    return oal_mem_alloc(OAL_MEM_POOL_ID_LOCAL, mic_len, OAL_TRUE);
}

/*****************************************************************************
 功能描述  : 释放申请的mic空间
*****************************************************************************/
OAL_STATIC osal_u32 hmac_wapi_mic_free_etc(osal_u8 *mic)
{
    if (mic != OAL_PTR_NULL) {
        oal_mem_free(mic, OAL_TRUE);
        return OAL_SUCC;
    }

    return OAL_FAIL;
}

/*****************************************************************************
 功能描述  : 判断keyidx是否合法
*****************************************************************************/
OAL_STATIC osal_u8 hmac_wapi_is_keyidx_valid_etc(hmac_wapi_stru *wapi, osal_u8 keyidx_rx)
{
    if ((wapi->keyidx != keyidx_rx) &&
        (wapi->keyupdate_flg != OAL_TRUE)) {
        oam_warning_log3(0, OAM_SF_ANY, "{hmac_wapi_is_keyidx_valid_etc::keyidx==%u, keyidx_rx==%u, update==%u.}",
                         wapi->keyidx, keyidx_rx, wapi->keyupdate_flg);
        return OAL_FALSE;
    }

    wapi->keyupdate_flg = OAL_FALSE; /* 更新完成取消标志 */

    /* key没有启用 */
    if (wapi->wapi_key[keyidx_rx].key_en != OAL_TRUE) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_wapi_is_keyidx_valid_etc::keyen==%u.}",
                         wapi->wapi_key[keyidx_rx].key_en);
        return OAL_FALSE;
    }

    return OAL_TRUE;
}

/*****************************************************************************
 功能描述  : 单播帧判断数据奇偶正确性
*****************************************************************************/
OAL_STATIC oal_bool_enum_uint8 hmac_wapi_is_pn_odd_ucast_etc(const osal_u8 *pn)
{
    return (oal_bool_enum_uint8)(((*pn & BIT0) == 0) ? OAL_FALSE : OAL_TRUE);
}

/*****************************************************************************
 功能描述  : 组播帧判断数据奇偶性
*****************************************************************************/
OAL_STATIC osal_u8 hmac_wapi_is_pn_odd_bcast_etc(const osal_u8 *pn)
{
    unref_param(pn);
    return OAL_TRUE;
}

/*****************************************************************************
 功能描述  : 每收到一个帧，更新pn
*****************************************************************************/
OAL_STATIC osal_u8 hmac_wapi_pn_update_etc(osal_u8 *pn, osal_u8  inc)
{
    osal_u32          loop;
    osal_u32          loop_num;
    osal_u32          overlow;     /* 进位 */
    osal_u32         *pul_pn;

    pul_pn = (osal_u32 *)pn;
    loop_num = (osal_u32)(WAPI_PN_LEN / OAL_SIZEOF(osal_u32));
    overlow = inc;

    for (loop = 0; loop < loop_num; loop++) {
        if (*pul_pn > (*pul_pn + overlow)) {
            *pul_pn    += overlow;
            overlow = 1;     /* 溢出高位加1 */
        } else {
            *pul_pn    += overlow;
            break;
        }
        pul_pn++;
    }

    return OAL_TRUE;
}

/*****************************************************************************
 功能描述  : 处理已经分片或者不需分片的netbuff链
             1)如果已经分片，则处理这个链，将链上所有netbuff上的数据进行加密处理
             2)如果没有分片，则处理单个netbuff,将这个netbuff上的数据进行加密处理
*****************************************************************************/
OAL_STATIC oal_netbuf_stru  *hmac_wapi_netbuff_tx_handle_etc(hmac_wapi_stru *wapi, oal_netbuf_stru  *buf)
{
    osal_u32           ul_ret;
    oal_netbuf_stru     *netbuf_tmp;    /* 指向需要释放的netbuff */
    oal_netbuf_stru     *netbuf_prev;   /* 指向已经加密的netbuff */
    oal_netbuf_stru     *buf_first;     /* 指向还未加密的netbuff */

    /* buf的初始位置在snap头的llc处 */
    if (hmac_get_data_type_from_80211_etc(buf, 0) == MAC_DATA_WAPI) {
        oam_warning_log0(0, OAM_SF_WAPI, "{hmac_wapi_netbuff_tx_handle_etc::wapi, dont encrypt!.}");
        return buf;
    }

    ul_ret = wapi->wapi_encrypt(wapi, buf);
    if (ul_ret != OAL_SUCC) {
        hmac_free_netbuf_list_etc(buf);
        return OAL_PTR_NULL;
    }

    netbuf_tmp = buf;

    /* 使netbuff指针指向下一个需要加密的分片帧 */
    netbuf_prev = OAL_NETBUF_NEXT(buf);
    if (netbuf_prev == OAL_PTR_NULL) {
        return OAL_PTR_NULL;
    }
    buf_first = netbuf_prev;
    buf = OAL_NETBUF_NEXT(netbuf_prev);

    oal_netbuf_free(netbuf_tmp);

    while (buf != OAL_PTR_NULL) {
        ul_ret = wapi->wapi_encrypt(wapi, buf);
        if (ul_ret != OAL_SUCC) {
            hmac_free_netbuf_list_etc(buf_first);
            return OAL_PTR_NULL;
        }
        OAL_NETBUF_NEXT(netbuf_prev) = OAL_NETBUF_NEXT(buf);
        netbuf_tmp = buf;
        netbuf_prev = OAL_NETBUF_NEXT(buf);
        if (netbuf_prev == OAL_PTR_NULL) {
            return OAL_PTR_NULL;
        }
        buf = OAL_NETBUF_NEXT(netbuf_prev);

        oal_netbuf_free(netbuf_tmp);
    }
    return buf_first;
}

/*****************************************************************************
 功能描述  : 接收处理比发送要简单，因为每次只会有一个netbuff需要处理
             假设加密的netbuff为1，解密后的为0，那么输入为1,解密完成后变为
             1->0 本函数将1删掉，然后将已经解密的0往上送
*****************************************************************************/
OAL_STATIC oal_netbuf_stru *hmac_wapi_netbuff_rx_handle_etc(hmac_wapi_stru *wapi, oal_netbuf_stru *buf)
{
    osal_u32                    ul_ret;
    oal_netbuf_stru              *netbuf_tmp;    /* 指向需要释放的netbuff */

    /* 非加密帧，不进行解密 */
    if (((oal_netbuf_data(buf))[1] & 0x40) == 0) {
        return buf;
    }

    ul_ret = wapi->wapi_decrypt(wapi, buf);
    if (ul_ret != OAL_SUCC) {
        return OAL_PTR_NULL;
    }

    netbuf_tmp = buf;
    buf = OAL_NETBUF_NEXT(buf);
    oal_netbuf_free(netbuf_tmp);

    return buf;
}

/*****************************************************************************
 功能描述  : 增加/更新 key
*****************************************************************************/
OAL_STATIC osal_u32 hmac_wapi_add_key_etc(hmac_wapi_stru *wapi, osal_u8 key_index, osal_u8 *key)
{
    hmac_wapi_key_stru *pst_key;
    OAL_CONST osal_u8 wapi_pn_init_etc[WAPI_PN_LEN] = {
        0x36, 0x5c, 0x36, 0x5c, 0x36, 0x5c, 0x36, 0x5c, 0x36, 0x5c, 0x36, 0x5c, 0x36, 0x5c, 0x36, 0x5c
    };

    wapi->port_valid = OAL_TRUE;
    wapi->keyidx = key_index;
    wapi->keyupdate_flg = OAL_TRUE;
    pst_key = &(wapi->wapi_key[key_index]);

    if (memcpy_s(pst_key->wpi_ek, sizeof(pst_key->wpi_ek), key, WAPI_KEY_LEN) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_wapi_add_key_etc::memcpy_s wpi_ek error}");
    }
    if (memcpy_s(pst_key->wpi_ck, sizeof(pst_key->wpi_ck), key + WAPI_KEY_LEN, WAPI_KEY_LEN) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_wapi_add_key_etc::memcpy_s wpi_ck error}");
    }
    pst_key->key_en = OAL_TRUE;

    /* 重置PN */
    if (memcpy_s(pst_key->pn_rx, sizeof(pst_key->pn_rx), wapi_pn_init_etc, WAPI_PN_LEN) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_wapi_add_key_etc::memcpy_s pn_rx error}");
    }
    if (memcpy_s(pst_key->pn_tx, sizeof(pst_key->pn_tx), wapi_pn_init_etc, WAPI_PN_LEN) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_wapi_add_key_etc::memcpy_s pn_tx error}");
    }

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 检验wpi头的合法性
*****************************************************************************/
OAL_STATIC osal_u8 hmac_wapi_is_wpihdr_valid_etc(hmac_wapi_stru *wapi, osal_u8  *wapi_hdr)
{
    osal_u8 keyidx_rx;
    osal_u8 *pn_rx;

    keyidx_rx = *wapi_hdr;

    if (hmac_wapi_is_keyidx_valid_etc(wapi, keyidx_rx) != OAL_TRUE) {
        return OAL_FALSE;
    }

    pn_rx = wapi_hdr + SMS4_KEY_IDX + SMS4_WAPI_HDR_RESERVE;
    if (wapi->wapi_is_pn_odd(pn_rx) != OAL_TRUE) {
        oam_warning_log0(0, OAM_SF_ANY, "{hmac_wapi_is_wpihdr_valid_etc::wapi_is_pn_odd==false.}");
        return OAL_FALSE;
    }

    return OAL_TRUE;
}

/*****************************************************************************
 功能描述  : 对数据进行解密
 其    他  : 此函数如果返回错误，需要将netbuff释放掉，不需要再送到后面进行处理
*****************************************************************************/
OAL_STATIC osal_u32 hmac_wapi_decrypt_etc(hmac_wapi_stru *wapi, oal_netbuf_stru *pst_netbuf)
{
    oal_netbuf_stru             *netbuff_des    = NULL;
    osal_u8                   *key_ek         = NULL;
    osal_u8                   *key_ck         = NULL;
    osal_u8                    key_index;
    osal_u16                   pdu_len;
    osal_u8                    calc_mic[SMS4_MIC_LEN];
    osal_u32                   wapi_result;

    mac_ieee80211_frame_stru    *pst_mac_hdr;
    osal_u8                   *netbuff;
    osal_u16                   netbuff_len;
    osal_u8                   *wapi_hdr;
    osal_u8                    mac_hdr_len;
    osal_u8                   *pn;
    osal_u8                   *pdu;
    osal_u8                   *mic_data;        /* 按照协议，构造mic所需要的数据，见 wapi实施指南 图51 */
    osal_u8                   *mic;
    osal_u16                   mic_len;
    mac_rx_ctl_stru             *rx_ctl;
    mac_rx_ctl_stru             *rx_ctl_in;

    /************ 1. 解密前的数据准备,获取各头指针和内容长度 ************/
    netbuff = OAL_NETBUF_DATA(pst_netbuf);        // for ut,del temprarily
    netbuff_len = (osal_u16)OAL_NETBUF_LEN(pst_netbuf);

    /* 获取mac头 */
    pst_mac_hdr = (mac_ieee80211_frame_stru *)netbuff;

    /* wapi的数据帧一般为QOS帧  */
    rx_ctl_in = (mac_rx_ctl_stru *)OAL_NETBUF_CB(pst_netbuf);
    mac_hdr_len = rx_ctl_in->mac_header_len;
    wapi_hdr = (osal_u8 *)pst_mac_hdr + mac_hdr_len;
    pn = wapi_hdr + SMS4_KEY_IDX + SMS4_WAPI_HDR_RESERVE;
    pdu = pn + SMS4_PN_LEN;

    oam_warning_log1(0, OAM_SF_ANY, "{hmac_wpi_decrypt_etc::mac_hdr_len %u!.}", mac_hdr_len);

    if (netbuff_len < (osal_u16)(mac_hdr_len + HMAC_WAPI_HDR_LEN + SMS4_MIC_LEN)) {
        oam_error_log2(0, OAM_SF_ANY, "{hmac_wpi_decrypt_etc::netbuff_len %u, machdr len %u err!.}",
                       netbuff_len, mac_hdr_len);
        oal_netbuf_free(pst_netbuf);
        return OAL_FAIL;
    }
    pdu_len = netbuff_len - mac_hdr_len - HMAC_WAPI_HDR_LEN - SMS4_MIC_LEN;

    key_index = *wapi_hdr;

    if (key_index >= HMAC_WAPI_MAX_KEYID) {
        oam_error_log1(0, OAM_SF_ANY, "{hmac_wpi_decrypt_etc::key_index %u err!.}", key_index);
        oal_netbuf_free(pst_netbuf);
        return OAL_FAIL;
    }

    if (hmac_wapi_is_wpihdr_valid_etc(wapi, wapi_hdr) != OAL_TRUE) {
        oam_warning_log0(0, OAM_SF_ANY, "{hmac_wpi_decrypt_etc::hmac_wapi_is_wpihdr_valid_etc err!.}");
        oal_netbuf_free(pst_netbuf);
        return OAL_FAIL;
    }

    /************ 2. 准备新的netbuff,用来存放解密后的数据, 填写cb字段 ************/
#ifdef _PRE_LWIP_ZERO_COPY
    netbuff_des = oal_pbuf_netbuf_alloc(WLAN_MEM_NETBUF_SIZE2);
#else
    netbuff_des = OAL_MEM_NETBUF_ALLOC(OAL_NORMAL_NETBUF, WLAN_MEM_NETBUF_SIZE2, OAL_NETBUF_PRIORITY_MID);
#endif
    if (netbuff_des == NULL) {
        oal_netbuf_free(pst_netbuf);
        return OAL_ERR_CODE_ALLOC_MEM_FAIL;
    }

#ifdef _PRE_LWIP_ZERO_COPY
    skb_reserve(netbuff_des, PBUF_ZERO_COPY_RESERVE);
#endif

    /* 先拷贝mac头 */
    oal_netbuf_init(netbuff_des, mac_hdr_len);
#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    oal_netbuf_copydata(pst_netbuf, 0, oal_netbuf_data(netbuff_des), mac_hdr_len, mac_hdr_len);
#elif defined(_PRE_OS_VERSION_LITEOS) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
    (osal_void)memcpy_s(oal_netbuf_data(netbuff_des), mac_hdr_len,
        OAL_NETBUF_DATA(pst_netbuf), mac_hdr_len);
#endif

    /* 拷贝cb */
    rx_ctl = (mac_rx_ctl_stru *)OAL_NETBUF_CB(netbuff_des);
#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    if (memcpy_s(rx_ctl, MAC_TX_CTL_SIZE, OAL_NETBUF_CB(pst_netbuf), MAC_TX_CTL_SIZE) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_wpi_decrypt_etc::memcpy_s error}");
    }
#elif defined(_PRE_OS_VERSION_LITEOS) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
    (osal_void)memcpy_s(rx_ctl, MAC_TX_CTL_SIZE, OAL_NETBUF_CB(pst_netbuf), MAC_TX_CTL_SIZE);
#endif

    /************ 3. 解密前的密钥准备和PN准备 ************/
    key_ek = wapi->wapi_key[key_index].wpi_ek;
    hmac_wpi_swap_pn_etc(pn, SMS4_PN_LEN);

    /******************** 4. 解密**************************/
    wapi_result = hmac_wpi_decrypt_etc(pn,
        pdu,
        (pdu_len + SMS4_MIC_LEN),            /* 需要解密的数据长度 */
        key_ek,
        (oal_netbuf_data(netbuff_des) + mac_hdr_len));
    if (wapi_result != OAL_SUCC) {
        oal_netbuf_free(netbuff_des);
        /* 返回之前注意入参netbuff是否在外面被释放 */
        oal_netbuf_free(pst_netbuf);
        return OAL_ERR_CODE_WAPI_DECRYPT_FAIL;
    }

    /* mic作为校验数，不需要put */
    oal_netbuf_put(netbuff_des, pdu_len);

    /************ 5. 计算mic，并进行校验 ************/
    mic_data = hmac_wapi_mic_alloc_etc(hmac_wapi_is_qos_etc(pst_mac_hdr), pdu_len, &mic_len);
    if (mic_data == OAL_PTR_NULL) {
        oal_netbuf_free(netbuff_des);
        /* 注意netbuff后续是否有释放处理 */
        oal_netbuf_free(pst_netbuf);
        return OAL_ERR_CODE_ALLOC_MEM_FAIL;
    }

    /* 计算mic预备数据 */
    hmac_wapi_calc_mic_data_etc(pst_mac_hdr, mac_hdr_len, key_index,
        oal_netbuf_data(netbuff_des) + mac_hdr_len, pdu_len, mic_data, mic_len);
    key_ck = wapi->wapi_key[key_index].wpi_ck;
    wapi_result = hmac_wpi_pmac_etc(pn,
        mic_data,
        (mic_len >> 4), /* 4偏移值 */
        key_ck,
        (osal_u8 *)calc_mic);

    /* 计算完mic后，释放mic data */
    hmac_wapi_mic_free_etc(mic_data);
    if (wapi_result != OAL_SUCC) {
        oal_netbuf_free(netbuff_des);
        oal_netbuf_free(pst_netbuf);
        return OAL_ERR_CODE_WAPI_MIC_CALC_FAIL;
    }

    mic = oal_netbuf_data(netbuff_des) + mac_hdr_len + pdu_len;
    if (osal_memcmp(mic, calc_mic, SMS4_MIC_LEN) != 0) {            /* 比较MIC */
        oam_warning_log0(0, OAM_SF_ANY, "{hmac_wpi_decrypt_etc::mic check fail!.}");
        oal_netbuf_free(netbuff_des);
        oal_netbuf_free(pst_netbuf);
        return OAL_ERR_CODE_WAPI_MIC_CMP_FAIL;
    }

    /* 返回前清protected */
    (oal_netbuf_data(netbuff_des))[1] &= ~0x40;

    /* 填写cb */
    rx_ctl->mac_header_len  = mac_hdr_len;
    rx_ctl->frame_len = (osal_u16)OAL_NETBUF_LEN(netbuff_des);

    OAL_NETBUF_NEXT(netbuff_des) = OAL_NETBUF_NEXT(pst_netbuf);
    OAL_NETBUF_NEXT(pst_netbuf) = netbuff_des;

    hmac_wapi_pn_update_etc(wapi->wapi_key[wapi->keyidx].pn_rx, wapi->pn_inc);

    oam_warning_log0(0, OAM_SF_ANY, "{hmac_wpi_decrypt_etc::OK!.}");
    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 对数据进行加密,处理完成之后，无论是否分片，
            第一个netbuff为处理前，没有加密的netbuff，
            后面挂的netbuff为加密过的netbuff,请注意!
*****************************************************************************/
OAL_STATIC osal_u32 hmac_wapi_encrypt_etc(hmac_wapi_stru *wapi, oal_netbuf_stru  *netbuf)
{
    mac_ieee80211_frame_stru        *mac_hdr;
    osal_u8                        mac_hdr_len;
    osal_u8                       *mic_data;
    osal_u16                       pdu_len;
    osal_u16                       mic_len;
    osal_u8                        key_index;
    osal_u8                       *key_ck;
    osal_u8                       *key_ek;
    osal_u8                        calc_mic[SMS4_MIC_LEN];
    osal_u8                        pn_swap[SMS4_PN_LEN]; /* 保存变换后的pn,用来计算mic和加密 */
    oal_netbuf_stru                 *netbuff_des;
    osal_u8                       *wapi_hdr;
    osal_u8                       *datain;
    osal_u32                       result;
    osal_u8                       *payload;
    mac_tx_ctl_stru                 *tx_ctl;

    /************ 1. 加密前的数据准备,获取各头指针和内容长度 ************/
    /* 获取mac头 */
    tx_ctl = (mac_tx_ctl_stru *)OAL_NETBUF_CB(netbuf);
    mac_hdr = mac_get_cb_frame_header_addr(tx_ctl);
    mac_hdr_len = mac_get_cb_frame_header_length(tx_ctl);

    /* 设置加密位 注意，mac头涉及加密，所以需要在最开始设置 */
    mac_set_protectedframe((osal_u8 *)mac_hdr);

    oam_warning_log2(0, OAM_SF_ANY, "{hmac_wapi_encrypt_etc::mac_hdr_len %u! more==%u.}", mac_hdr_len,
        mac_hdr->frame_control.more_frag);
    pdu_len = mac_get_cb_mpdu_len(tx_ctl);

    /* payload的起始位置在snap头的llc处 */
    payload = OAL_NETBUF_DATA(netbuf);

    /************ 2. 计算mic,wapi的数据帧一般为QOS帧  ************/
    mic_data = hmac_wapi_mic_alloc_etc(hmac_wapi_is_qos_etc(mac_hdr), pdu_len, &mic_len);
    if (mic_data == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_wapi_encrypt_etc::hmac_wapi_mic_alloc_etc err!");
        return OAL_ERR_CODE_ALLOC_MEM_FAIL;
    }

    /* 计算mic预备数据 */
    key_index = wapi->keyidx;
    hmac_wapi_calc_mic_data_etc(mac_hdr, mac_hdr_len, key_index, payload, pdu_len, mic_data, mic_len);
    key_ck = wapi->wapi_key[key_index].wpi_ck;
    if (memcpy_s(pn_swap, sizeof(pn_swap), wapi->wapi_key[key_index].pn_tx, SMS4_PN_LEN) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_wapi_encrypt_etc::memcpy_s pn_tx error}");
        return OAL_FAIL;
    }
    hmac_wpi_swap_pn_etc(pn_swap, SMS4_PN_LEN);
    /* 计算mic */
    result = hmac_wpi_pmac_etc((osal_u8 *)pn_swap, mic_data, (mic_len >> 4), key_ck, (osal_u8 *)calc_mic); /* 4偏移值 */

    hmac_wapi_mic_free_etc(mic_data);
    if (result == OAL_FAIL) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_wapi_encrypt_etc::hmac_wpi_pmac_etc mic calc err!");
        return OAL_ERR_CODE_WAPI_MIC_CALC_FAIL;
    }

    /************ 3. 准备新的netbuff,用来存放加密后的数据, 填写cb,并准备加密前的数据 ************/
    netbuff_des = OAL_MEM_NETBUF_ALLOC(OAL_NORMAL_NETBUF, WLAN_MEM_NETBUF_SIZE2, OAL_NETBUF_PRIORITY_MID);
    if (netbuff_des == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_wapi_encrypt_etc::netbuff_des alloc err!");
        return OAL_ERR_CODE_ALLOC_MEM_FAIL;
    }

    /* 填写cb */
    if (memcpy_s(OAL_NETBUF_CB(netbuff_des), MAC_TX_CTL_SIZE, OAL_NETBUF_CB(netbuf), MAC_TX_CTL_SIZE) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_wapi_encrypt_etc::memcpy_s cb error}");
        return OAL_FAIL;
    }

    /* 先拷贝mac头,为了后续hcc处理，此处填写最大的空间 */
    oal_netbuf_init(netbuff_des, MAC_80211_QOS_HTC_4ADDR_FRAME_LEN);
    /* 跟其他host报文保持统一，将预留空间放在mac header之前，以qos data为例 |10字节空闲|26字节mac header|XX字节payload| */
    if (memcpy_s(OAL_NETBUF_DATA(netbuff_des) + MAC_80211_QOS_HTC_4ADDR_FRAME_LEN - mac_hdr_len,
        mac_hdr_len, mac_hdr, mac_hdr_len) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_wapi_encrypt_etc::memcpy_s mac_hdr error}");
        return OAL_FAIL;
    }

    datain = oal_mem_alloc(OAL_MEM_POOL_ID_LOCAL, pdu_len + SMS4_MIC_LEN, OAL_TRUE);
    if (datain == OAL_PTR_NULL) {
        oal_netbuf_free(netbuff_des);
        oam_error_log0(0, OAM_SF_ANY, "{hmac_wapi_encrypt_etc::datain alloc err!");
        return OAL_ERR_CODE_ALLOC_MEM_FAIL;
    }
    if (memcpy_s(datain, pdu_len, payload, pdu_len) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_wapi_encrypt_etc::memcpy_s payload error}");
        return OAL_FAIL;
    }
    /* 拷贝mic */
    if (memcpy_s(datain + pdu_len, SMS4_MIC_LEN, calc_mic, SMS4_MIC_LEN) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_wapi_encrypt_etc::memcpy_s calc_mic error}");
        return OAL_FAIL;
    }

    key_ek = wapi->wapi_key[key_index].wpi_ek;
    /************************ 4. 加密 ************************/
    result = hmac_wpi_encrypt_etc(pn_swap, datain, pdu_len + SMS4_MIC_LEN, key_ek,
        OAL_NETBUF_DATA(netbuff_des) + HMAC_WAPI_HDR_LEN + MAC_80211_QOS_HTC_4ADDR_FRAME_LEN);

    oal_mem_free(datain, OAL_TRUE);
    if (result != OAL_SUCC) {
        oal_netbuf_free(netbuff_des);
        oam_error_log1(0, OAM_SF_ANY, "{hmac_wapi_encrypt_etc::hmac_wpi_encrypt_etc err==%u!", result);
        return OAL_ERR_CODE_WAPI_ENRYPT_FAIL;
    }
    /* 此处put完之后，netbuff的len为mac头+pdulen+sms4+wapi的长度 */
    oal_netbuf_put(netbuff_des, pdu_len + SMS4_MIC_LEN + HMAC_WAPI_HDR_LEN);

    /***************** 5. 填写wapi头 *****************/
    wapi_hdr = OAL_NETBUF_DATA(netbuff_des) + MAC_80211_QOS_HTC_4ADDR_FRAME_LEN;

    /* 填写WPI头 -- keyIndex */
    *(wapi_hdr)  = key_index;
    /* 保留位清零 */
    *(wapi_hdr + SMS4_KEY_IDX)  = 0;
    /* 填写PN */
    if (memcpy_s((wapi_hdr + SMS4_KEY_IDX + SMS4_WAPI_HDR_RESERVE), SMS4_PN_LEN, wapi->wapi_key[key_index].pn_tx,
        SMS4_PN_LEN) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_wapi_encrypt_etc::memcpy_s wapi_key pn_tx error}");
        return OAL_FAIL;
    }

    /* 再次填写cb */
    tx_ctl = (mac_tx_ctl_stru *)OAL_NETBUF_CB(netbuff_des);
    mac_get_cb_frame_header_addr(tx_ctl) = (mac_ieee80211_frame_stru *)((osal_u8 *)OAL_NETBUF_DATA(netbuff_des) +
        MAC_80211_QOS_HTC_4ADDR_FRAME_LEN - mac_hdr_len);
    /* netbuf 的data指针指向payload */
    oal_netbuf_pull(netbuff_des, MAC_80211_QOS_HTC_4ADDR_FRAME_LEN);

    /* 不包括mac hdr */
    mac_get_cb_mpdu_len(tx_ctl) = (osal_u16)(HMAC_WAPI_HDR_LEN + pdu_len + SMS4_MIC_LEN);
    OAL_NETBUF_NEXT(netbuff_des) = OAL_NETBUF_NEXT(netbuf);
    OAL_NETBUF_NEXT(netbuf) = netbuff_des;

    /* 更新pn */
    hmac_wapi_pn_update_etc(wapi->wapi_key[wapi->keyidx].pn_tx, wapi->pn_inc);
    oam_warning_log0(0, OAM_SF_ANY, "{hmac_wapi_encrypt_etc::hmac_wpi_encrypt_etc OK!");

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 将port重置
*****************************************************************************/
OAL_STATIC osal_u32 hmac_wapi_reset_port_etc(hmac_wapi_stru *wapi)
{
    wapi->port_valid = OAL_FALSE;
    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 去初始化wapi对象
*****************************************************************************/
OAL_STATIC osal_void hmac_wapi_deinit_etc(osal_u16 user_idx)
{
    hmac_wapi_stru *wapi_user_info = hmac_wapi_get_user_info(user_idx);
    if (wapi_user_info != OSAL_NULL) {
        oal_mem_free(wapi_user_info, OAL_TRUE);
        g_wapi_userinfo[user_idx] = OSAL_NULL;
    }
    return;
}

OAL_STATIC osal_bool hmac_wapi_del_user_exit(osal_void *notify_data)
{
    hmac_vap_stru *hmac_vap = OSAL_NULL;
    hmac_wapi_stru *wapi_multi_user_info = OSAL_NULL;
    hmac_device_stru *hmac_device = OSAL_NULL;
    hmac_user_stru *hmac_user = (hmac_user_stru *)notify_data;

    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(hmac_user->vap_id);
    if (hmac_vap == OSAL_NULL) {
        oam_warning_log1(0, 0, "{hmac_wapi_del_user_exit:vap[%d] is null.}", hmac_user->vap_id);
        return OSAL_FALSE;
    }

    hmac_wapi_deinit_etc(hmac_user->assoc_id);
    /* STA模式下，清组播wapi加密端口 */
    wapi_multi_user_info = hmac_wapi_get_user_info(hmac_vap->multi_user_idx);
    if (wapi_multi_user_info == OSAL_NULL) {
        oam_error_log2(0, OAM_SF_ANY, "vap_id[%d] {hmac_wapi_del_user_exit::hmac_wapi_get_user_info fail user_idx[%u]}",
                       hmac_vap->vap_id, hmac_vap->multi_user_idx);
        return OSAL_FALSE;
    }
    hmac_wapi_reset_port_etc(wapi_multi_user_info);
    hmac_device = hmac_res_get_mac_dev_etc(hmac_user->device_id);
    hmac_device->wapi = OAL_FALSE;
    return OSAL_TRUE;
}

/*****************************************************************************
 功能描述  : 初始化wapi对象
*****************************************************************************/
OAL_STATIC osal_u32 hmac_wapi_init_etc(osal_u16 user_idx, osal_u8 pairwise)
{
    osal_u32 loop;
    hmac_wapi_stru *wapi_user_info = hmac_wapi_get_user_info(user_idx);
    if (wapi_user_info == OSAL_NULL) {
        g_wapi_userinfo[user_idx] = oal_mem_alloc(OAL_MEM_POOL_ID_LOCAL, sizeof(hmac_wapi_stru), OAL_TRUE);
        if (g_wapi_userinfo[user_idx] == OSAL_NULL) {
            oam_error_log1(0, OAM_SF_ANY, "user_idx[%d] hmac_wapi_stru malloc null!", user_idx);
            return OAL_FAIL;
        }
        wapi_user_info = g_wapi_userinfo[user_idx];
    }
    memset_s(wapi_user_info, OAL_SIZEOF(hmac_wapi_stru), 0, OAL_SIZEOF(hmac_wapi_stru));
    if (pairwise == OAL_TRUE) {
        wapi_user_info->pn_inc = WAPI_UCAST_INC;
        wapi_user_info->wapi_is_pn_odd = hmac_wapi_is_pn_odd_ucast_etc;
    } else {
        wapi_user_info->pn_inc = WAPI_BCAST_INC;
        wapi_user_info->wapi_is_pn_odd = hmac_wapi_is_pn_odd_bcast_etc;
    }

    for (loop = 0; loop < HMAC_WAPI_MAX_KEYID; loop++) {
        wapi_user_info->wapi_key[loop].key_en = OAL_FALSE;
    }

    wapi_user_info->port_valid = OAL_FALSE;
    wapi_user_info->wapi_decrypt = hmac_wapi_decrypt_etc;
    wapi_user_info->wapi_encrypt = hmac_wapi_encrypt_etc;
    wapi_user_info->wapi_netbuff_txhandle = hmac_wapi_netbuff_tx_handle_etc;
    wapi_user_info->wapi_netbuff_rxhandle = hmac_wapi_netbuff_rx_handle_etc;
    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 获取用户的wapi对象指针
*****************************************************************************/
WIFI_TCM_TEXT OAL_STATIC hmac_wapi_stru *hmac_user_get_wapi_ptr_etc(hmac_vap_stru *hmac_vap,
    oal_bool_enum_uint8 pairwise, osal_u16 pairwise_idx)
{
    osal_u16 user_index;
    hmac_wapi_stru *wapi_user_info;

    if (pairwise == OAL_TRUE) {
        user_index = pairwise_idx;
    } else {
        user_index = hmac_vap->multi_user_idx;
    }

    wapi_user_info = hmac_wapi_get_user_info(user_index);
    return wapi_user_info;
}

OAL_STATIC osal_bool hmac_wapi_add_user_init(osal_void *notify_data)
{
    hmac_user_stru *hmac_user = (hmac_user_stru *)notify_data;
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(hmac_user->device_id);
    hmac_wapi_init_etc(hmac_user->assoc_id, OAL_TRUE);
    hmac_device->wapi = OAL_FALSE;
    return OSAL_TRUE;
}

OAL_STATIC oal_netbuf_stru *hmac_wapi_rx_netbuf(hmac_vap_stru *hmac_vap, oal_netbuf_stru *netbuf,
    hmac_wapi_stru *wapi, hmac_user_stru *hmac_user, mac_rx_ctl_stru **rx_ctrl)
{
    netbuf = wapi->wapi_netbuff_rxhandle(wapi, netbuf);
    if (netbuf == OAL_PTR_NULL) {
        oam_warning_log1(0, OAM_SF_RX,
            "vap_id[%d] {hmac_rx_lan_frame_classify_wapi:: wapi decrypt FAIL!}", hmac_vap->vap_id);
        hmac_dft_print_drop_frame_info(THIS_FILE_ID, __LINE__, 1, OAL_PTR_NULL);
        hmac_user_stats_pkt_incr(hmac_user->rx_pkt_drop, 1);
        return OAL_PTR_NULL;
    }

    /* 重新获取该MPDU的控制信息 */
    *rx_ctrl = (mac_rx_ctl_stru *)oal_netbuf_cb(netbuf);
    return netbuf;
}

WIFI_HMAC_TCM_TEXT WIFI_TCM_TEXT OAL_STATIC oal_netbuf_stru *hmac_rx_lan_frame_classify_wapi(hmac_vap_stru *hmac_vap,
    oal_netbuf_stru *netbuf, const mac_ieee80211_frame_stru *frame_hdr,
    hmac_user_stru *hmac_user, mac_rx_ctl_stru **rx_ctrl)
{
    hmac_wapi_stru *wapi = OAL_PTR_NULL;
    oal_bool_enum_uint8 is_mcast;
    oal_bool_enum_uint8 pairwise = OAL_TRUE;

    is_mcast = ETHER_IS_MULTICAST(frame_hdr->address1);
    if (is_mcast == OAL_TRUE) {
        pairwise = OAL_FALSE;
    }
    wapi = hmac_user_get_wapi_ptr_etc(hmac_vap, pairwise, hmac_user->assoc_id);
    if (wapi == OAL_PTR_NULL) {
        oam_warning_log0(0, OAM_SF_WPA, "{hmac_rx_lan_frame_classify_wapi:: get wapi Err!.}");
        hmac_user_stats_pkt_incr(hmac_user->rx_pkt_drop, 1);
        hmac_dft_print_drop_frame_info(THIS_FILE_ID, __LINE__, 1, netbuf);
        oal_netbuf_free(netbuf);
        return OAL_PTR_NULL;
    }
    if ((wapi->port_valid != OAL_TRUE) || (wapi->wapi_netbuff_rxhandle == OAL_PTR_NULL)) {
        return netbuf;
    }
    return hmac_wapi_rx_netbuf(hmac_vap, netbuf, wapi, hmac_user, rx_ctrl);
}

WIFI_HMAC_TCM_TEXT WIFI_TCM_TEXT OAL_STATIC osal_u32 hmac_wapi_tx_classify_special(hmac_vap_stru *hmac_vap,
    osal_u16 type, mac_tx_ctl_stru *tx_ctl, osal_u8 *tid)
{
    if (type == oal_host2net_short(ETHER_TYPE_WAI)) {
        oam_info_log1(0, OAM_SF_TX, "vap_id[%d] {hmac_wapi_tx_classify_special::ETHER_TYPE_WAI.}", hmac_vap->vap_id);
        hmac_tx_set_vip_tid(tx_ctl, WLAN_CB_FRAME_TYPE_DATA, MAC_DATA_WAPI, OAL_TRUE, tid);
        return OSAL_TRUE;
    }
    return OSAL_FALSE;
}

WIFI_TCM_TEXT OAL_STATIC osal_u32 hmac_tx_lan_to_wlan_wapi(hmac_vap_stru *hmac_vap, oal_netbuf_stru **netbuf)
{
    hmac_wapi_stru *wapi = OAL_PTR_NULL;
    mac_ieee80211_frame_stru *pst_mac_hdr = OAL_PTR_NULL;
    oal_bool_enum_uint8 is_mcast;
    oal_bool_enum_uint8 pairwise = OAL_TRUE;

    /* 获取wapi对象 组播/单播 */
    pst_mac_hdr = mac_get_cb_frame_header_addr((mac_tx_ctl_stru *)oal_netbuf_cb(*netbuf));
    is_mcast = ETHER_IS_MULTICAST(pst_mac_hdr->address1);
    if (is_mcast == OAL_TRUE) {
        pairwise = OAL_FALSE;
    }
    wapi = hmac_user_get_wapi_ptr_etc(hmac_vap, pairwise, hmac_vap->assoc_vap_id);
    if (wapi == OAL_PTR_NULL) {
        OAM_STAT_VAP_INCR(hmac_vap->vap_id, tx_abnormal_msdu_dropped, 1);
        oam_warning_log1(0, OAM_SF_ANY,
            "hmac_tx_lan_to_wlan_wapi:hmac_user_get_wapi_ptr_etc fail!assoc_id[%u]", hmac_vap->assoc_vap_id);
        hmac_dft_print_drop_frame_info(THIS_FILE_ID, __LINE__, 1, *netbuf);
        return OAL_ERR_CODE_PTR_NULL;
    }

    if ((wapi->port_valid == OAL_TRUE) && (wapi->wapi_netbuff_txhandle != OAL_PTR_NULL)) {
        *netbuf = wapi->wapi_netbuff_txhandle(wapi, *netbuf);
    }

    return OAL_CONTINUE;
}

/*****************************************************************************
 功能描述  : 配置wapi key并且同步
*****************************************************************************/
OAL_STATIC osal_u32 hmac_config_wapi_add_key_etc(hmac_vap_stru *hmac_vap, mac_addkey_param_stru *payload_addkey_param)
{
    osal_u8                        key_index;
    oal_bool_enum_uint8              pairwise;
    osal_u8                       *mac_addr;
    mac_key_params_stru             *key_param;
    hmac_wapi_stru                  *wapi;
    osal_u32                       ul_ret;
    osal_u16                       user_index = 0;
    hmac_device_stru                 *hmac_device;

    key_index = payload_addkey_param->key_index;
    if (key_index >= HMAC_WAPI_MAX_KEYID) {
        oam_error_log1(0, OAM_SF_WPA, "{hmac_config_wapi_add_key_etc::keyid==%u Err!.}", key_index);
        return OAL_FAIL;
    }

    pairwise  = payload_addkey_param->pairwise;
    mac_addr = (osal_u8 *)payload_addkey_param->mac_addr;
    key_param   = &payload_addkey_param->key;

    if (key_param->key_len != (WAPI_KEY_LEN * 2)) { /* 2倍长度 */
        oam_error_log1(0, OAM_SF_WPA, "{hmac_config_wapi_add_key_etc:: key_len %u Err!.}", key_param->key_len);
        return OAL_FAIL;
    }

    if (pairwise) {
        ul_ret = hmac_vap_find_user_by_macaddr_etc(hmac_vap, mac_addr, &user_index);
        if (ul_ret != OAL_SUCC) {
            oam_error_log2(0, OAM_SF_ANY,
                "vap_id[%d] {hmac_config_wapi_add_key_etc::hmac_vap_find_user_by_macaddr_etc failed. %u}",
                hmac_vap->vap_id, ul_ret);
            return OAL_FAIL;
        }
    }

    wapi = hmac_user_get_wapi_ptr_etc(hmac_vap, pairwise, user_index);
    if (wapi == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_WPA, "{hmac_config_wapi_add_key_etc:: get wapi  Err!.}");
        return OAL_FAIL;
    }

    hmac_wapi_add_key_etc(wapi, key_index, key_param->key);

    hmac_device = hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    if (hmac_device == OAL_PTR_NULL) {
        oam_error_log1(0, OAM_SF_CFG, "vap_id[%d] {hmac_config_wapi_add_key_etc::device null.}",
            hmac_vap->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }
    hmac_device->wapi = OAL_TRUE;

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 保存wapi key并且同步
*****************************************************************************/
OAL_STATIC osal_u32 hmac_wapi_add_key_and_sync_etc(hmac_vap_stru *hmac_vap, osal_u32 type,
    mac_addkey_param_stru *payload_addkey_param)
{
    osal_u32 ret;

    if (type != WITP_WLAN_CIPHER_SUITE_SMS4) {
        return OAL_CONTINUE;
    }
    oam_warning_log2(0, OAM_SF_WPA, "{hmac_wapi_add_key_and_sync_etc:: key idx==%u, pairwise==%u}",
        payload_addkey_param->key_index, payload_addkey_param->pairwise);

    ret = hmac_config_wapi_add_key_etc(hmac_vap, payload_addkey_param);
    if (ret != OAL_SUCC) {
        oam_error_log2(0, OAM_SF_WPA,
            "vap_id[%d] {hmac_wapi_add_key_and_sync_etc::hmac_config_wapi_add_key_etc fail[%d].}", hmac_vap->vap_id,
            ret);
        return ret;
    }

    /* 关掉硬件的加解密功能 */
    hal_disable_ce();

    return ret;
}

/*****************************************************************************
 功能描述  : 判断wapi设备是否关连
*****************************************************************************/
OAL_STATIC osal_u8  hmac_wapi_connected_etc(osal_u8 device_id)
{
    osal_u8 vap_idx;
    hmac_device_stru *hmac_device = OSAL_NULL;
    hmac_vap_stru *hmac_vap = OSAL_NULL;
    hmac_wapi_stru *wapi_user_info = OSAL_NULL;

    hmac_device = hmac_res_get_mac_dev_etc(device_id);
    if (OAL_UNLIKELY(hmac_device == OAL_PTR_NULL)) {
        oam_error_log1(0, OAM_SF_UM, "{hmac_wapi_connected_etc::hmac_device null.id %u}", device_id);
        return OAL_FALSE;
    }

    for (vap_idx = 0; vap_idx < hmac_device->vap_num; vap_idx++) {
        hmac_vap = mac_res_get_hmac_vap(hmac_device->vap_id[vap_idx]);
        if (OAL_UNLIKELY(hmac_vap == OAL_PTR_NULL)) {
            oam_warning_log1(0, OAM_SF_CFG, "vap is null! vap id is %d", hmac_device->vap_id[vap_idx]);
            continue;
        }

        if (!is_sta(hmac_vap)) {
            continue;
        }

        wapi_user_info = hmac_wapi_get_user_info(hmac_vap->multi_user_idx);
        if ((wapi_user_info != OAL_PTR_NULL) && (wapi_user_info->port_valid == OAL_TRUE)) {
            return OAL_TRUE;
        }
    }

    return OAL_FALSE;
}

OAL_STATIC osal_u32 hmac_11i_reset_wapi(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user)
{
    hmac_device_stru *hmac_device = OSAL_NULL;
    hmac_wapi_stru *wapi_user_info = hmac_wapi_get_user_info(hmac_user->assoc_id);

    if (wapi_user_info == OSAL_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 11i的情况下，关掉wapi端口 */
    hmac_wapi_reset_port_etc(wapi_user_info);
    hmac_device = hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    if (hmac_device == OSAL_NULL) {
        oam_error_log1(0, 0, "vap_id[%d] {hmac_config_11i_add_key_etc::device null.}", hmac_vap->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }
    hmac_device->wapi = OAL_FALSE;
    return OAL_CONTINUE;
}

OAL_STATIC osal_void hmac_wapi_update_legcy_only(const mac_bss_dscr_stru *bss_dscr, oal_bool_enum_uint8 *legcy_only)
{
    if ((bss_dscr != OSAL_NULL) && (bss_dscr->wapi != 0)) {
        *legcy_only = OAL_TRUE;
    }
}

WIFI_HMAC_TCM_TEXT WIFI_TCM_TEXT OAL_STATIC osal_bool hmac_wapi_ether_type_check(osal_u16 ether_type)
{
    return (oal_byteorder_host_to_net_uint16(ETHER_TYPE_WAI) != ether_type);
}

OAL_STATIC osal_bool hmac_wapi_disable(osal_u8 wapi)
{
    return (wapi != OAL_TRUE);
}

WIFI_HMAC_TCM_TEXT WIFI_TCM_TEXT OAL_STATIC osal_bool hmac_wapi_get_ether_type(osal_u16 ether_type)
{
    return (ether_type == oal_host2net_short(ETHER_TYPE_WAI));
}

OAL_STATIC osal_u32 hmac_wapi_connect_check_bss_dscr(mac_bss_dscr_stru *bss_dscr,
    hmac_vap_stru *hmac_vap, mac_conn_param_stru *connect_param)
{
    bss_dscr->wapi = connect_param->wapi;
    if (bss_dscr->wapi != 0) {
        hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(hmac_vap->device_id);
        if (hmac_device == OAL_PTR_NULL) {
            oam_error_log2(0, OAM_SF_CFG,
                "vap_id[%d] {hmac_wapi_connect_check_bss_dscr::connect failed, mac_device null! device_id[%d]}",
                hmac_vap->vap_id, hmac_vap->device_id);
            return OAL_ERR_CODE_MAC_DEVICE_NULL;
        }
#ifdef _PRE_WLAN_FEATURE_P2P
        if (mac_device_is_p2p_connected_etc(hmac_device) == OAL_SUCC) {
            oam_warning_log1(0, OAM_SF_CFG,
                "vap_id[%d] {hmac_wapi_connect_check_bss_dscr:: wapi connect failed for p2p having been connected!.}",
                hmac_vap->vap_id);
            return OAL_FAIL;
        }
#endif
    }
    return OAL_CONTINUE;
}

OAL_STATIC osal_s32 hmac_wapi_update_roam(hmac_vap_stru *hmac_vap, osal_u8 *roming_now)
{
    /* wapi下，将roam标志置为1，防止arp探测 */
    hmac_wapi_stru *wapi_user_info = hmac_wapi_get_user_info(hmac_vap->multi_user_idx);
    if (wapi_user_info == OAL_PTR_NULL) {
        return OAL_PTR_NULL;
    }

    if (wapi_user_info->port_valid == OAL_TRUE) {
        *roming_now = 1;
        return OAL_FAIL;
    }
    return OAL_SUCC;
}

OAL_STATIC osal_u32 hmac_wapi_user_add_check(osal_u8 device_id)
{
    if (hmac_res_get_mac_dev_etc(device_id) == OAL_PTR_NULL) {
        oam_warning_log0(0, OAM_SF_UM, "{hmac_wapi_user_add_check::hmac_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    return OAL_CONTINUE;
}

OAL_STATIC osal_u8 hmac_wapi_update_connect_param(osal_u32 wpa_versions)
{
    if (wpa_versions == WITP_WAPI_VERSION) {
        oam_warning_log0(0, OAM_SF_ANY, "hmac_wapi_update_connect_param::crypt ver is wapi!");
        return OAL_TRUE;
    }
    return OAL_FALSE;
}

osal_u32 hmac_wapi_init(osal_void)
{
    osal_u16 user_id;
    for (user_id = 0; user_id < WLAN_USER_MAX_USER_LIMIT; user_id++) {
        g_wapi_userinfo[user_id] = OSAL_NULL;
    }
    /* 通知注册 */
    frw_util_notifier_register(WLAN_UTIL_NOTIFIER_EVENT_ADD_USER, hmac_wapi_add_user_init);
    frw_util_notifier_register(WLAN_UTIL_NOTIFIER_EVENT_DEL_USER, hmac_wapi_del_user_exit);
    /* 对外接口注册 */
    hmac_feature_hook_register(HMAC_FHOOK_WAPI_INIT, hmac_wapi_init_etc);
    hmac_feature_hook_register(HMAC_FHOOK_WAPI_RX_FRAME, hmac_rx_lan_frame_classify_wapi);
    hmac_feature_hook_register(HMAC_FHOOK_WAPI_TX_FRAME, hmac_wapi_tx_classify_special);
    hmac_feature_hook_register(HMAC_FHOOK_WAPI_TX_LAN2WLAN, hmac_tx_lan_to_wlan_wapi);
    hmac_feature_hook_register(HMAC_FHOOK_WAPI_RESET, hmac_11i_reset_wapi);
    hmac_feature_hook_register(HMAC_FHOOK_WAPI_ADD_KEY, hmac_wapi_add_key_and_sync_etc);
    hmac_feature_hook_register(HMAC_FHOOK_WAPI_UPDATE_LEGCY_ONLY, hmac_wapi_update_legcy_only);
    hmac_feature_hook_register(HMAC_FHOOK_WAPI_ETHER_TYPE_CHECK, hmac_wapi_ether_type_check);
    hmac_feature_hook_register(HMAC_FHOOK_WAPI_CONNECT_BSS_DSCR, hmac_wapi_connect_check_bss_dscr);
    hmac_feature_hook_register(HMAC_FHOOK_WAPI_DISABLE, hmac_wapi_disable);
    hmac_feature_hook_register(HMAC_FHOOK_WAPI_UPDATE_ROAM, hmac_wapi_update_roam);
    hmac_feature_hook_register(HMAC_FHOOK_WAPI_USER_ADD_CHECK, hmac_wapi_user_add_check);
    hmac_feature_hook_register(HMAC_FHOOK_WAPI_DEINIT, hmac_wapi_deinit_etc);
    hmac_feature_hook_register(HMAC_FHOOK_WAPI_GET_ETHER_TYPE, hmac_wapi_get_ether_type);
    hmac_feature_hook_register(HMAC_FHOOK_WAPI_CONNECT_CHECK, hmac_wapi_connected_etc);
    hmac_feature_hook_register(HMAC_FHOOK_WAPI_UPDATE_CONNECT_PARAM, hmac_wapi_update_connect_param);
    return OAL_SUCC;
}

osal_void hmac_wapi_deinit(osal_void)
{
    /* 通知去注册 */
    frw_util_notifier_unregister(WLAN_UTIL_NOTIFIER_EVENT_ADD_USER, hmac_wapi_add_user_init);
    frw_util_notifier_unregister(WLAN_UTIL_NOTIFIER_EVENT_DEL_USER, hmac_wapi_del_user_exit);
    /* 对外接口去注册 */
    hmac_feature_hook_unregister(HMAC_FHOOK_WAPI_INIT);
    hmac_feature_hook_unregister(HMAC_FHOOK_WAPI_RX_FRAME);
    hmac_feature_hook_unregister(HMAC_FHOOK_WAPI_TX_FRAME);
    hmac_feature_hook_unregister(HMAC_FHOOK_WAPI_TX_LAN2WLAN);
    hmac_feature_hook_unregister(HMAC_FHOOK_WAPI_RESET);
    hmac_feature_hook_unregister(HMAC_FHOOK_WAPI_ADD_KEY);
    hmac_feature_hook_unregister(HMAC_FHOOK_WAPI_UPDATE_LEGCY_ONLY);
    hmac_feature_hook_unregister(HMAC_FHOOK_WAPI_ETHER_TYPE_CHECK);
    hmac_feature_hook_unregister(HMAC_FHOOK_WAPI_CONNECT_BSS_DSCR);
    hmac_feature_hook_unregister(HMAC_FHOOK_WAPI_DISABLE);
    hmac_feature_hook_unregister(HMAC_FHOOK_WAPI_UPDATE_ROAM);
    hmac_feature_hook_unregister(HMAC_FHOOK_WAPI_USER_ADD_CHECK);
    hmac_feature_hook_unregister(HMAC_FHOOK_WAPI_DEINIT);
    hmac_feature_hook_unregister(HMAC_FHOOK_WAPI_GET_ETHER_TYPE);
    hmac_feature_hook_unregister(HMAC_FHOOK_WAPI_CONNECT_CHECK);
    hmac_feature_hook_unregister(HMAC_FHOOK_WAPI_UPDATE_CONNECT_PARAM);
    return;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

