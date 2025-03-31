/*
 * Copyright (c) CompanyNameMagicTag 2020-2022. All rights reserved.
 * Description: wpi数据加解密.
 */

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "hmac_wapi_wpi.h"
#include "hmac_wapi_sms4.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_HMAC_WAPI_WPI_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/

/*****************************************************************************
3 函数实现
*****************************************************************************/
/*****************************************************************************
 函 数 名  : hmac_wpi_encrypt_etc
 功能描述  : ofb encrypt
 输入参数  : osal_u8 *iv    为IV存储区起始地址
             osal_u8 *bufin    为明文存储区起始地址
             osal_u32 buflen    为明文（以Byte为单位）长度
             osal_u8 *key      为会话密钥存储区起始地址
             osal_u8* bufout   为密文存储区起始地址，
                                密文存储区空间与明文存储区空间大小相同
*****************************************************************************/
osal_u32  hmac_wpi_encrypt_etc(osal_u8 *iv, osal_u8 *bufin, osal_u32 buflen,
    osal_u8 *key, osal_u8 *bufout)
{
    osal_u32       iv_out[4];
    osal_u32      *pul_in;
    osal_u32      *pul_out;
    osal_u8       *out;
    osal_u8       *in;
    osal_u32       counter;
    osal_u32       comp;
    osal_u32       loop;
    osal_u32       pr_keyin[32];

    if (buflen < 1) {
        return OAL_FAIL;
    }

    hmac_sms4_keyext_etc(key,  (osal_u32 *)pr_keyin);

    counter = buflen / 16; /* 16换算 */
    comp    = buflen % 16; /* 16换算 */

    /* get the iv */
    hmac_sms4_crypt_etc(iv, (osal_u8 *)iv_out, (const osal_u32 *)pr_keyin);
    pul_in  = (osal_u32 *)bufin;
    pul_out = (osal_u32 *)bufout;

    for (loop = 0; loop < counter; loop++) {
        pul_out[0] = pul_in[0] ^ iv_out[0];
        pul_out[1] = pul_in[1] ^ iv_out[1];
        pul_out[2] = pul_in[2] ^ iv_out[2]; /* 2索引值 */
        pul_out[3] = pul_in[3] ^ iv_out[3]; /* 3索引值 */
        hmac_sms4_crypt_etc((osal_u8 *)iv_out, (osal_u8 *)iv_out, (const osal_u32 *)pr_keyin);
        pul_in  += 4; /* 4长度累加 */
        pul_out += 4; /* 4长度累加 */
    }

    in  = (osal_u8 *)pul_in;
    out = (osal_u8 *)pul_out;
    iv = (osal_u8 *)iv_out;

    for (loop = 0; loop < comp; loop++) {
        out[loop] = in[loop] ^ iv[loop];
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : WPI_Decrypt1
 功能描述  : ofb decrypt
 输入参数  : osal_u8* iv    为IV存储区起始地址
             osal_u8* bufin    为密文存储区起始地址
             osal_u32 buflen    为密文（以Byte为单位）长度
             osal_u8* key      为会话密钥存储区起始地址
             osal_u8* bufout   为明文存储区起始地址
*****************************************************************************/
osal_u32  hmac_wpi_decrypt_etc(osal_u8 *iv, osal_u8 *bufin, osal_u32 buflen,
    osal_u8 *key, osal_u8 *bufout)
{
    return hmac_wpi_encrypt_etc(iv, bufin, buflen, key, bufout);
}

/*****************************************************************************
 函 数 名  : hmac_wpi_swap_pn_etc
 功能描述  : 将PN倒转一下
 输入参数  : UWORD8 *pucPn - PN值数组存储的地方
             UWORD8 ucLen - PN值的长度
*****************************************************************************/
void hmac_wpi_swap_pn_etc(osal_u8 *pn, osal_u8 len)
{
    osal_u8       index;
    osal_u8       temp;
    osal_u8       len_tmp;

    len_tmp = len / 2; /* 2换算 */
    for (index = 0; index < len_tmp; index++) {
        temp = pn[index];
        pn[index] = pn[len - 1 - index];
        pn[len - 1 - index] = temp;
    }
}

/*****************************************************************************
 函 数 名  : hmac_wpi_pmac_etc
 功能描述  : 计算mic
 输入参数  : osal_u8* iv    为IV存储区起始地址
             osal_u8* pucBuf      为text存储区起始地址
             osal_u32 ulPamclen   为text长度（以Byte为单位，且应为16Byte的整数倍）除以16的倍数
             osal_u8* pucKey      为用于计算MIC的密钥KEY存储区起始地址
             osal_u8* pucMic      为MIC存储区起始地址
*****************************************************************************/
osal_u32 hmac_wpi_pmac_etc(const osal_u8 *iv, osal_u8 *buf, osal_u32 pamclen,
    osal_u8 *key, osal_u8 *mic)
{
    osal_u32  mic_tmp[4];
    osal_u32  loop;
    osal_u32 *pul_in;
    osal_u32  mac_keyin[32];

    if ((pamclen < 1) || (pamclen > 4096)) { /* 4096长度判断 */
        return OAL_FAIL;
    }

    hmac_sms4_keyext_etc(key, (osal_u32 *)mac_keyin);
    pul_in = (osal_u32 *)buf;
    hmac_sms4_crypt_etc(iv, (osal_u8 *)mic_tmp, (const osal_u32 *)mac_keyin);

    for (loop = 0; loop < pamclen; loop++) {
        mic_tmp[0] ^= pul_in[0];
        mic_tmp[1] ^= pul_in[1];
        mic_tmp[2] ^= pul_in[2]; /* 2索引值 */
        mic_tmp[3] ^= pul_in[3]; /* 3索引值 */
        pul_in += 4; /* 4长度 */
        hmac_sms4_crypt_etc((osal_u8 *)mic_tmp, (osal_u8 *)mic_tmp, (const osal_u32 *)mac_keyin);
    }

    pul_in    = (osal_u32 *)mic;
    pul_in[0] = mic_tmp[0];
    pul_in[1] = mic_tmp[1];
    pul_in[2] = mic_tmp[2]; /* 2索引值 */
    pul_in[3] = mic_tmp[3]; /* 3索引值 */

    return OAL_SUCC;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

