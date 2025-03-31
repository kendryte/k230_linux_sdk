/*
 * Copyright (c) CompanyNameMagicTag 2020-2022. All rights reserved.
 * Description: STA侧管理帧处理.
 */

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "hmac_11i.h"
#include "oal_ext_if.h"
#include "oal_types.h"
#include "oal_netbuf_ext.h"
#include "frw_ext_if.h"
#include "wlan_spec.h"
#include "wlan_types_common.h"
#include "mac_resource_ext.h"
#include "mac_frame.h"
#include "mac_device_ext.h"
#include "mac_vap_ext.h"
#include "hmac_wapi.h"
#include "hmac_main.h"
#include "hmac_crypto_tkip.h"
#include "hmac_config.h"
#include "hmac_mgmt_bss_comm.h"
#include "hmac_roam_if.h"
#include "hmac_frag.h"
#include "frw_util_notifier.h"
#include "wlan_msg.h"
#include "hmac_feature_interface.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_HMAC_11I_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
frw_timeout_stru g_update_gtk_timer;

/*****************************************************************************
  3 函数实现
*****************************************************************************/
OAL_STATIC oal_bool_enum_uint8 hmac_11i_is_ptk(oal_bool_enum_uint8 macaddr_is_zero, oal_bool_enum_uint8 pairwise)
{
    return (macaddr_is_zero != OAL_TRUE) && (pairwise == OAL_TRUE);
}
/*****************************************************************************
 函 数 名  : hmac_check_igtk_exist
 功能描述  : 通过igtk index检查igtk是否存在
*****************************************************************************/
osal_u32 hmac_check_igtk_exist(osal_u8 igtk_index)
{
    /* igtk的key index 为4或5 */
    if ((igtk_index > WLAN_MAX_IGTK_KEY_INDEX) ||
        ((igtk_index <= WLAN_MAX_IGTK_KEY_INDEX - WLAN_NUM_IGTK))) {
        return OAL_ERR_CODE_PMF_IGTK_NOT_EXIST;
    }

    return OAL_SUCC;
}

OSAL_STATIC osal_u32 hmac_11i_update_gtk_timeout_fn(osal_void *arg)
{
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)(arg);
 
    oam_warning_log1(0, OAM_SF_ANY, "vap_id[%d] {hmac_11i_update_gtk_timeout_fn:: enable replay detect}",
        hmac_vap->vap_id);
    hal_set_replay_detect_en(OSAL_TRUE);
    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 调用增加密钥写寄存器接口，将一个密钥写入硬件Lut表
*****************************************************************************/
static osal_u32 hmac_11i_update_key_to_ce(const hmac_vap_stru *hmac_vap, hal_security_key_stru *key)
{
    hal_security_key_stru        key_update;
    osal_u8                        key_cipher[WLAN_CIPHER_KEY_LEN] = { 0 };
    osal_u8                        zero[WLAN_CIPHER_KEY_LEN] = { 0 };
    osal_u8                        key_id;

    hal_to_dmac_device_stru *hal_device = hmac_vap->hal_device;
    if (hal_device == OSAL_NULL) {
        oam_error_log1(0, OAM_SF_WPA, "vap_id[%d] {hmac_11i_update_key_to_ce::hal_device null.}", hmac_vap->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 重要信息打印warning */
    oam_warning_log4(0, OAM_SF_WPA, "{hmac_11i_update_key_to_ce::id=%u,type=%u,lutidx=%u,cipher=%u}",
        key->key_id, key->key_type, key->lut_idx, key->cipher_type);
    oam_warning_log3(0, 0, "vap_id[%d] {hmac_11i_update_key_to_ce::update_key=%u, key_origin=%u}", hmac_vap->vap_id,
        key->update_key, key->key_origin);
    /* 非加密不用add key */
    if (key->cipher_type == WLAN_80211_CIPHER_SUITE_NO_ENCRYP) {
        return OAL_SUCC;
    }

    /* 组播密钥更新关闭重放攻击检测 超时后打开 防止更新期间pn误刷新 */
    if ((key->key_type != WLAN_KEY_TYPE_PTK) && (hal_get_replay_detect_en() == OSAL_TRUE)) {
        hal_set_replay_detect_en(OSAL_FALSE);
        hal_ce_add_key(key);
        frw_create_timer_entry(&g_update_gtk_timer, hmac_11i_update_gtk_timeout_fn, HMAC_UPDATE_GTK_TIMEOUT,
            (hmac_vap_stru *)hmac_vap, OAL_FALSE);
        return OAL_SUCC;
    }

    /* 3.1 写硬件寄存器   */
    hal_ce_add_key(key);

    if ((key->cipher_type == WLAN_80211_CIPHER_SUITE_CCMP) && (key->key_type == WLAN_KEY_TYPE_PTK)) {
        /* 有的终端会使用错误的key index发包，所以在更新key时，读取其他key id的密钥值，若为0，则一同更新 */
        (osal_void)memcpy_s(&key_update, sizeof(hal_security_key_stru), key, sizeof(hal_security_key_stru));
        key_update.cipher_key = key_cipher;

        for (key_id = 0; key_id < WLAN_NUM_TK; key_id++) {
            if (key_id == key->key_id) {
                continue;
            }
            key_update.key_id = key_id;
            (osal_void)memset_s(key_update.cipher_key, WLAN_CIPHER_KEY_LEN, 0, WLAN_CIPHER_KEY_LEN);
            hal_ce_get_key(&key_update);
            if (oal_memcmp(key_update.cipher_key, zero, WLAN_CIPHER_KEY_LEN) != 0) {
                continue;
            }
            /* 4. 更新其他key id的密钥 */
            if (memcpy_s(key_update.cipher_key, WLAN_CIPHER_KEY_LEN, key->cipher_key, WLAN_CIPHER_KEY_LEN) != EOK) {
                oam_error_log0(0, OAM_SF_ANY, "{hmac_11i_update_key_to_ce::memcpy_s error}");
            }
            hal_ce_add_key(&key_update);
        }
    }
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_11i_del_key_to_ce
 功能描述  : 将一个密钥从硬件LUT表中删除

*****************************************************************************/
static osal_u32 hmac_11i_del_key_to_ce(const hmac_vap_stru *hmac_vap, osal_u8 key_id,
    hal_cipher_key_type_enum_uint8 key_type, osal_u8 lut_index, wlan_ciper_protocol_type_enum_uint8 cipher_type)
{
    hal_security_key_stru    security_key = {0};
    hal_to_dmac_device_stru  *hal_device = OSAL_NULL;
    hal_security_key_stru    key = {0};
    osal_u8                key_cipher[WLAN_CIPHER_KEY_LEN] = { 0 };
    osal_u8                crt_key[WLAN_CIPHER_KEY_LEN] = { 0 };
    osal_u8                other_key_id;

    security_key.key_id      = key_id;
    security_key.key_type    = key_type;
    security_key.lut_idx     = lut_index;
    security_key.update_key  = OSAL_TRUE;
    security_key.cipher_type = cipher_type;
    security_key.cipher_key = OSAL_NULL;
    security_key.mic_key    = OSAL_NULL;

    hal_device = hmac_vap->hal_device;
    if (hal_device == OSAL_NULL) {
        oam_error_log1(0, OAM_SF_WPA, "vap_id[%d] {hmac_11i_del_key_to_ce::hal_device null.}", hmac_vap->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }
    /* 重要信息打印warning */
    oam_warning_log3(0, OAM_SF_WPA,
        "{hmac_11i_del_key_to_ce::keyid=%u, keytype=%u,lutidx=%u}", key_id, key_type, lut_index);
    /* 与dmac_11i_update_key_to_ce对应,同步删除 */
    if ((cipher_type == WLAN_80211_CIPHER_SUITE_CCMP) && (key_type == WLAN_KEY_TYPE_PTK)) {
        (osal_void)memcpy_s(&key, sizeof(hal_security_key_stru), &security_key, sizeof(hal_security_key_stru));
        key.cipher_key = key_cipher;
        hal_ce_get_key(&key);
        (osal_void)memcpy_s(crt_key, WLAN_CIPHER_KEY_LEN, key.cipher_key, WLAN_CIPHER_KEY_LEN);

        for (other_key_id = 0; other_key_id < WLAN_NUM_TK; other_key_id++) {
            if (other_key_id == key_id) {
                continue;
            }
            key.key_id = other_key_id;
            (osal_void)memset_s(key.cipher_key, WLAN_CIPHER_KEY_LEN, 0, WLAN_CIPHER_KEY_LEN);
            hal_ce_get_key(&key);
            if (oal_memcmp(key.cipher_key, crt_key, WLAN_CIPHER_KEY_LEN) == 0) {
                /* 4. 删除其他key id的密钥 */
                hal_ce_del_key(&key);
            }
        }
    }

    /* 删除本key id硬件寄存器秘钥 */
    hal_ce_del_key(&security_key);

    return OAL_SUCC;
}
/*****************************************************************************
 函 数 名  : hmac_11i_get_auth_type
 功能描述  : 根据vap，获取认证类型

*****************************************************************************/
static hal_key_origin_enum_uint8 hmac_11i_get_auth_type(const hmac_vap_stru *hmac_vap)
{
    if (is_ap(hmac_vap)) {
        return HAL_AUTH_KEY;
    }
    return HAL_SUPP_KEY;
}
/*****************************************************************************
 函 数 名  : dmac_11i_get_key_type
 功能描述  : 根据vap，获取密钥类型

*****************************************************************************/
static hal_cipher_key_type_enum_uint8 hmac_11i_get_gtk_key_type(const hmac_vap_stru *hmac_vap,
    wlan_ciper_protocol_type_enum_uint8 cipher_type)
{
    osal_u8           rx_gtk        = HAL_KEY_TYPE_RX_GTK;
    hmac_user_stru      *multi_user   = OSAL_NULL;

    if (is_ap(hmac_vap) && (cipher_type != WLAN_80211_CIPHER_SUITE_BIP)) {
        return  HAL_KEY_TYPE_RX_GTK;
    }

    /* 不分gtk1，gtk2，gtk和ptk一样配置，根据keyid区分 */
    multi_user = (hmac_user_stru *)mac_res_get_hmac_user_etc(hmac_vap->multi_user_idx);
    if (multi_user == OSAL_NULL) {
        oam_warning_log2(0, OAM_SF_WPA, "vap_id[%d] {hmac_11i_get_gtk_key_type::multi_user[%d] null.}",
            hmac_vap->vap_id, hmac_vap->multi_user_idx);
        return HAL_KEY_TYPE_TX_GTK;
    }

    if (cipher_type == WLAN_80211_CIPHER_SUITE_BIP) {
        rx_gtk = HAL_KEY_TYPE_TX_GTK;
    } else if (multi_user->key_info.gtk == 0) {
        rx_gtk = HAL_KEY_TYPE_RX_GTK;
    }
    return rx_gtk;
}

/*****************************************************************************
 函 数 名  : hmac_reset_gtk_token
 功能描述  : 复位gtk乒乓位

*****************************************************************************/
osal_u32 hmac_reset_gtk_token(const hmac_vap_stru *hmac_vap)
{
    hmac_user_stru *multi_user = OSAL_NULL;
    if (hmac_vap == OSAL_NULL) {
        oam_warning_log0(0, OAM_SF_WPA, "{hmac_11i_add_gtk_key::hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    multi_user = (hmac_user_stru *)mac_res_get_hmac_user_etc(hmac_vap->multi_user_idx);
    if (multi_user == OSAL_NULL) {
        oam_warning_log2(0, OAM_SF_WPA, "vap_id[%d] {hmac_11i_add_gtk_key::multi_user[%d] null.}", hmac_vap->vap_id,
            hmac_vap->multi_user_idx);
        return OAL_ERR_CODE_SECR_USER_INVAILD;
    }
    multi_user->key_info.gtk = 0;

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 设置单播密钥
*****************************************************************************/
static osal_u32 hmac_11i_add_ptk_key(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user, osal_u8 key_index)
{
    osal_u8 mic_key[WLAN_TEMPORAL_KEY_LENGTH]  = {0};
    hal_security_key_stru security_key;
    wlan_priv_key_param_stru *key = OSAL_NULL;
    osal_u8 cipher;
    osal_u32 ret;

    /* 1.0 ptk index 检查 */
    if (key_index >= WLAN_NUM_TK) {
        return OAL_ERR_CODE_SECR_KEY_ID;
    }
    /* 2.1 参数准备 */
    key = &hmac_user->key_info.key[key_index];
    cipher = (osal_u8)key->cipher;
    if ((cipher != WLAN_80211_CIPHER_SUITE_TKIP) && (cipher != WLAN_80211_CIPHER_SUITE_CCMP) &&
        (cipher != WLAN_80211_CIPHER_SUITE_GCMP) && (cipher != WLAN_80211_CIPHER_SUITE_GCMP_256) &&
        (cipher != WLAN_80211_CIPHER_SUITE_CCMP_256)) {
        oam_error_log3(0, OAM_SF_WPA, "vap_id[%d] {hmac_11i_add_ptk_key::invalid chiper type[%d], key_index=%d.}",
            hmac_vap->vap_id, key_index, key->cipher);
        return OAL_ERR_CODE_SECR_CHIPER_TYPE;
    }
    security_key.key_id      = key_index;
    security_key.key_type    = HAL_KEY_TYPE_PTK;
    security_key.key_origin  = hmac_11i_get_auth_type(hmac_vap);
    security_key.update_key  = ((hmac_user->is_rx_eapol_key_open == OAL_FALSE) ? OAL_TRUE : OAL_FALSE);
    security_key.lut_idx     = hmac_user->lut_index;
    security_key.cipher_type = hmac_user->key_info.cipher_type;
    security_key.cipher_key = key->key;
    security_key.mic_key    = OSAL_NULL;

    if (key->key_len > WLAN_TEMPORAL_KEY_LENGTH) {
        if (security_key.cipher_type == WLAN_80211_CIPHER_SUITE_TKIP) {
            security_key.mic_key = key->key + WLAN_TEMPORAL_KEY_LENGTH;
            /* 对于TKIP模式，MIC存在txrx交换bit顺序的情况，需要转换顺序 */
            if (is_sta(hmac_vap)) {
                security_key.mic_key = mic_key;
                (osal_void)memcpy_s(mic_key, sizeof(mic_key),
                    key->key + WLAN_TEMPORAL_KEY_LENGTH + WLAN_MIC_KEY_LENGTH, WLAN_MIC_KEY_LENGTH);
                (osal_void)memcpy_s(mic_key + WLAN_MIC_KEY_LENGTH, sizeof(mic_key) - WLAN_MIC_KEY_LENGTH,
                    key->key + WLAN_TEMPORAL_KEY_LENGTH, WLAN_MIC_KEY_LENGTH);
            }
        } else {
            /* ccmp256、gcmp256等32字节密钥，高16字节放在mic_key，低16放在cipher_key */
            security_key.mic_key = key->key;
            security_key.cipher_key = key->key + WLAN_TEMPORAL_KEY_LENGTH;
        }
    }

    /* 4.1 将加密方式和加密密钥写入CE中, 同时增加激活用户 */
    ret = hmac_11i_update_key_to_ce(hmac_vap, &security_key);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, 0, "{hmac_11i_update_key_to_ce::update_key_to_ce failed[%d].}", ret);
        return ret;
    }

    /* 寄存器写入成功后，更新单播用户密钥 */
    hmac_user_set_key_etc(hmac_user, security_key.key_type, security_key.cipher_type, security_key.key_id);

    /* 5.1 打开1X端口认证状态 */
    hmac_user_set_port_etc(hmac_user, OSAL_TRUE);

    return OAL_SUCC;
}

static oal_bool_enum is_key_cipher_valid(const hmac_vap_stru *hmac_vap, const osal_u8 key_cipher)
{
    if ((key_cipher != WLAN_80211_CIPHER_SUITE_TKIP) && (key_cipher != WLAN_80211_CIPHER_SUITE_CCMP) &&
        (key_cipher != WLAN_80211_CIPHER_SUITE_BIP) && (key_cipher != WLAN_80211_CIPHER_SUITE_GCMP) &&
        (key_cipher != WLAN_80211_CIPHER_SUITE_GCMP_256) && (key_cipher != WLAN_80211_CIPHER_SUITE_CCMP_256) &&
        (key_cipher != WLAN_80211_CIPHER_SUITE_BIP_CMAC_256)) {
        oam_error_log2(0, 0, "vap_id[%d] {hmac_11i_add_gtk_key::invalid chip type[%d].}", hmac_vap->vap_id, key_cipher);
        return OAL_FALSE;
    }
    return OAL_TRUE;
}

/*****************************************************************************
 函 数 名  : hmac_11i_add_gtk_key
 功能描述  : 设置组播密钥
*****************************************************************************/
static osal_u32 hmac_11i_add_gtk_key(const hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user, osal_u8 key_index)
{
    osal_u8 mic_key[WLAN_MIC_KEY_LENGTH] = {0};
    hal_security_key_stru security_key;
    wlan_priv_key_param_stru *key = OSAL_NULL;
    osal_u32 ret;
    hmac_add_gtk_notify_stru add_gtk_data = {0};

    /* 2.1 参数准备 */
    key = &hmac_user->key_info.key[key_index];

    if (is_key_cipher_valid(hmac_vap, (osal_u8)key->cipher) == 0) {
        return OAL_ERR_CODE_SECR_CHIPER_TYPE;
    }

    /* 规避AP更新组播密钥使用的是相同的keyid，导致组播密钥槽的keyid一致，组播解密失败
       keyid一致，采用原有密钥槽重新配置密钥 */
    hmac_user->key_info.gtk ^= (key_index == hmac_user->key_info.last_gtk_key_idx) ? BIT0 : 0;

    security_key.cipher_type = (osal_u8)key->cipher;
    security_key.key_id = key_index;
    security_key.key_type = hmac_11i_get_gtk_key_type(hmac_vap, security_key.cipher_type);
    hmac_user->key_info.gtk ^= BIT0; /* GTK 槽位乒乓使用 */
    hmac_user->key_info.last_gtk_key_idx = key_index;

    oam_warning_log3(0, OAM_SF_WPA, "vap_id[%d] {hmac_11i_add_gtk_key::new gtk=%u, keyidx = %u.}",
        hmac_vap->vap_id, hmac_user->key_info.gtk, hmac_user->key_info.last_gtk_key_idx);

    security_key.key_origin  = hmac_11i_get_auth_type(hmac_vap);
    security_key.update_key  = OSAL_FALSE; /* 为TURE时表示关联状态中途更新PTK，此时不清除PN值,FALSE时需要清除PN值 */
    /* 获取对应组播秘钥对应的rx lut idx，因不同产品获取方式不同，统一封装函数获取 */
    hal_vap_get_gtk_rx_lut_idx(hmac_vap->hal_vap, &security_key.lut_idx);

    security_key.cipher_key = key->key;
    security_key.mic_key = OSAL_NULL;
    if (key->key_len > WLAN_TEMPORAL_KEY_LENGTH) {
        if (security_key.cipher_type == WLAN_80211_CIPHER_SUITE_TKIP) {
            security_key.mic_key = key->key + WLAN_TEMPORAL_KEY_LENGTH;
            /* 对于TKIP模式，MIC存在txrx交换bit顺序的情况，需要转换顺序 */
            if (is_sta(hmac_vap)) {
                (osal_void)memcpy_s(mic_key, WLAN_MIC_KEY_LENGTH, security_key.mic_key, WLAN_MIC_KEY_LENGTH);
                (osal_void)memcpy_s(security_key.mic_key, WLAN_MIC_KEY_LENGTH,
                    security_key.mic_key + WLAN_MIC_KEY_LENGTH, WLAN_MIC_KEY_LENGTH);
                (osal_void)memcpy_s(security_key.mic_key + WLAN_MIC_KEY_LENGTH, WLAN_MIC_KEY_LENGTH,
                    mic_key, WLAN_MIC_KEY_LENGTH);
            }
        } else {
            /* ccmp256、gcmp256等32字节密钥，高16字节放在mic_key，低16放在cipher_key */
            security_key.mic_key = key->key;
            security_key.cipher_key = key->key + WLAN_TEMPORAL_KEY_LENGTH;
        }
    }

    /* 3.1 将加密方式和加密密钥写入CE中 */
    ret = hmac_11i_update_key_to_ce(hmac_vap, &security_key);
    if (ret != OAL_SUCC) {
        return ret;
    }

    /* 设置成功后，更新一下multiuser中的安全信息,目前只有keyid在发送组播帧时会使用到 */
    hmac_user_set_key_etc(hmac_user, security_key.key_type, security_key.cipher_type, security_key.key_id);

    /* 4.1 打开1X端口认证状态 */
    hmac_user_set_port_etc(hmac_user, OSAL_TRUE);

    add_gtk_data.cipher_type = security_key.cipher_type;
    add_gtk_data.vap_id = hmac_vap->vap_id;
    add_gtk_data.key_idx = key_index;

    frw_util_notifier_notify(WLAN_UTIL_NOTIFIER_EVENT_ADD_GTK, (osal_void *)&add_gtk_data);
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_11i_add_wep_key
 功能描述  : 设置组播密钥
*****************************************************************************/
static osal_u32 hmac_11i_add_wep_key(const hmac_vap_stru *hmac_vap, osal_u8 key_index)
{
    hmac_user_stru                      *multi_user = OSAL_NULL;
    wlan_priv_key_param_stru           *key                   = OSAL_NULL;
    hal_security_key_stru               security_key;

    if (hmac_vap == OSAL_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (mac_is_wep_enabled(hmac_vap) != OSAL_TRUE) {
        return OAL_SUCC;
    }

    /* 1.1 根据索引找到组播user内存区域 */
    multi_user = mac_res_get_hmac_user_etc(hmac_vap->multi_user_idx);
    if (multi_user == OSAL_NULL) {
        return OAL_ERR_CODE_SECR_USER_INVAILD;
    }

    if (multi_user->key_info.cipher_type != WLAN_80211_CIPHER_SUITE_WEP_104 &&
        multi_user->key_info.cipher_type != WLAN_80211_CIPHER_SUITE_WEP_40) {
        oam_error_log2(0, OAM_SF_WPA, "vap_id[%d] {hmac_11i_add_wep_key::invalid cipher type[%d].}",
                       hmac_vap->vap_id, multi_user->key_info.cipher_type);

        return OAL_ERR_CODE_SECR_CHIPER_TYPE;
    }
    key = &multi_user->key_info.key[key_index];
    /* 2.1 参数准备 */
    /* 未覆盖 */
    security_key.key_id      = key_index;
    security_key.cipher_type = multi_user->key_info.cipher_type;

    security_key.key_type    = hmac_11i_get_gtk_key_type(hmac_vap, security_key.cipher_type);
    security_key.key_origin  = hmac_11i_get_auth_type(hmac_vap);
    security_key.update_key  = OSAL_FALSE;

    if (is_ap(hmac_vap) && (security_key.key_type == WLAN_KEY_TYPE_TX_GTK)) {
        hal_vap_get_gtk_rx_lut_idx(hmac_vap->hal_vap, &security_key.lut_idx);
    } else {
        security_key.lut_idx = hmac_vap->hal_vap->vap_id;
    }

    security_key.cipher_key = key->key;
    security_key.mic_key    = OSAL_NULL;

    /* 3.1 将加密方式和加密密钥写入CE中 */
    if (hmac_11i_update_key_to_ce(hmac_vap, &security_key) != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_WPA,
                       "vap_id[%d] {hmac_11i_add_wep_key::hmac_11i_update_key_to_ce invalid.}", hmac_vap->vap_id);

        return OAL_FAIL;
    }

    /* 5.1 打开发送描述符的加密属性 */
    if (security_key.key_type == WLAN_KEY_TYPE_TX_GTK) {
        multi_user->user_tx_info.security.cipher_key_type = HAL_KEY_TYPE_TX_GTK;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_11i_del_ptk_key
 功能描述  : 删除单播密钥
*****************************************************************************/
static osal_u32 hmac_11i_del_ptk_key(hmac_vap_stru *hmac_vap, osal_u8 key_index, const osal_u8 *mac_addr)
{
    osal_u32                          ret;
    hmac_user_stru                 *current_dmac_user     = OSAL_NULL;
    osal_u8                           key_id;
    osal_u8                           ce_lut_index;
    hal_cipher_key_type_enum_uint8  key_type;
    wlan_ciper_protocol_type_enum_uint8 cipher_type;

    /* 1.1 根据mac地址找到user索引 */
    current_dmac_user = mac_vap_get_hmac_user_by_addr_etc(hmac_vap, mac_addr);
    if (current_dmac_user == OSAL_NULL) {
        oam_warning_log0(0, OAM_SF_WPA, "{hmac_11i_del_ptk_key::mac_vap_get_hmac_user_by_addr_etc failed.}");
        return OAL_ERR_CODE_SECR_USER_INVAILD;
    }

    /* 2.1 参数准备 */
    key_type     = HAL_KEY_TYPE_PTK;
    key_id       = key_index;
    ce_lut_index = current_dmac_user->lut_index;
    cipher_type  = current_dmac_user->key_info.cipher_type;

    /* 3.2 删除CE中的对应密钥 */
    ret = hmac_11i_del_key_to_ce(hmac_vap, key_id, key_type, ce_lut_index, cipher_type);
    if (ret != OAL_SUCC) {
        // weifugai
        oam_error_log4(0, OAM_SF_WPA,
            "{hmac_11i_del_ptk_key::dmac_11i_del_key_to_cefailed[%d],key_id=%d key_type=%d ce_lut_index=%d.}",
            ret, key_id, key_type, ce_lut_index);

        return ret;
    }

    /* 4.1 关闭1X端口认证状态 */
    hmac_user_set_port_etc(current_dmac_user, OSAL_FALSE);
    /* 初始化用户的密钥信息 */
    hmac_user_init_key_etc(current_dmac_user);

    if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_STA && hmac_vap->vap_state != MAC_VAP_STATE_STA_FAKE_UP) {
        hmac_user_set_pmf_active_etc(current_dmac_user, hmac_vap->user_pmf_cap);
    }
    /* 5.1 关闭发送描述符的加密属性 */
    current_dmac_user->user_tx_info.security.cipher_key_type = HAL_KEY_TYPE_BUTT;

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_11i_del_gtk_key
 功能描述  : 删除组播密钥
*****************************************************************************/
static osal_u32 hmac_11i_del_gtk_key(const hmac_vap_stru *hmac_vap, osal_u8 key_index)
{
    osal_u32                          ret;
    hmac_user_stru                  *current_mac_user = OSAL_NULL;
    osal_u8                           key_id;
    osal_u8                           ce_lut_index;
    hal_cipher_key_type_enum_uint8      key_type;
    wlan_ciper_protocol_type_enum_uint8 cipher_type;
    hmac_del_gtk_notify_stru del_gtk_data = {0};

    if (hmac_vap == OSAL_NULL || hmac_vap->hal_vap == OSAL_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }
    unref_param(key_index);

    /* 1.1 根据索引找到组播user内存区域 */
    current_mac_user = (hmac_user_stru *)mac_res_get_hmac_user_etc(hmac_vap->multi_user_idx);
    if (current_mac_user == OSAL_NULL) {
        return OAL_ERR_CODE_SECR_USER_INVAILD;
    }

    /* 2.1 参数准备 */
    key_id = 0;
    key_type = hmac_11i_get_gtk_key_type(hmac_vap, current_mac_user->key_info.cipher_type);
    /* 获取对应组播秘钥对应的rx lut idx，因不同产品获取方式不同，统一封装函数获取 */
    hal_vap_get_gtk_rx_lut_idx(hmac_vap->hal_vap, &ce_lut_index);
    cipher_type  = current_mac_user->key_info.cipher_type;

    /* 3.1 删除CE中的对应密钥 */
    ret = hmac_11i_del_key_to_ce(hmac_vap, key_id, key_type, ce_lut_index, cipher_type);
    if (ret != OAL_SUCC) {
        oam_error_log4(0, OAM_SF_WPA,
                       "{hmac_11i_del_gtk_key::hmac_11i_del_key_to_ce[%d],key_id=%d key_type=%d ce_lut_index=%d.}",
                       ret, key_id, key_type, ce_lut_index);

        return ret;
    }

    /* 4.1 关闭1X端口认证状态 */
    hmac_user_set_port_etc(current_mac_user, OSAL_FALSE);
    hmac_user_init_key_etc(current_mac_user);
    if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_STA && hmac_vap->vap_state != MAC_VAP_STATE_STA_FAKE_UP) {
        hmac_user_set_pmf_active_etc(current_mac_user, hmac_vap->user_pmf_cap);
    }
    /* 5.1 关闭发送描述符的加密属性 */
    if (key_type == HAL_KEY_TYPE_TX_GTK) {
        current_mac_user->user_tx_info.security.cipher_key_type = WLAN_KEY_TYPE_BUTT;
    }

    del_gtk_data.cipher_type = cipher_type;
    del_gtk_data.vap_id = hmac_vap->vap_id;

    frw_util_notifier_notify(WLAN_UTIL_NOTIFIER_EVENT_ADD_GTK, (osal_void *)&del_gtk_data);

    if (g_update_gtk_timer.is_registerd == OSAL_TRUE) {
        frw_destroy_timer_entry(&g_update_gtk_timer);
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_11i_add_key_from_user
 功能描述  : 设置用户的加密套件
*****************************************************************************/
osal_u32 hmac_11i_add_key_from_user(const hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user)
{
    osal_u8                           key_id = 0;
    osal_u8                          *cipkey = OSAL_NULL;
    osal_u8                          *mickey = OSAL_NULL;
    osal_u8                           auth_supp;
    osal_u32                          ret;
    hal_security_key_stru               security_key;

    if (hmac_vap == OSAL_NULL || hmac_user == OSAL_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_AP) {
        auth_supp = HAL_AUTH_KEY;
    } else {
        auth_supp = HAL_SUPP_KEY;
    }

    /* 在多用户扩展情况下 可能存在同一个chip下 多个vap 每个vap加密方式不同
       wep加密方式下 PTK秘钥也需要替换
    */
    /* wep加密时不需要恢复PTK密钥 */
    if (mac_is_wep_allowed(hmac_vap) == OSAL_TRUE) {
        return OAL_SUCC;
    }
    cipkey = hmac_user->key_info.key[key_id].key;        /* 前16字节是cipherkey */
    mickey = hmac_user->key_info.key[key_id].key + 16;   /* 后16字节是mickey */

    security_key.key_id      = key_id;
    security_key.key_type    = HAL_KEY_TYPE_PTK;
    security_key.key_origin  = auth_supp;
    security_key.update_key  = OSAL_FALSE;
    security_key.lut_idx     = hmac_user->lut_index;
    security_key.cipher_type = hmac_user->key_info.cipher_type;
    security_key.cipher_key = cipkey;
    security_key.mic_key    = mickey;

    ret = hmac_11i_update_key_to_ce(hmac_vap, &security_key);
    if (ret != OAL_SUCC) {
        oam_error_log2(0, OAM_SF_WPA, "vap_id[%d] {hmac_11i_add_key_from_user::update_key_to_ce failed[%d].}",
            hmac_vap->vap_id, ret);
        return ret;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_config_11i_add_key_set_reg
 功能描述  : add key 处理
*****************************************************************************/
osal_u32 hmac_config_11i_add_key_set_reg(hmac_vap_stru *hmac_vap, osal_u8 pairwise,
    osal_u8 key_index, hmac_user_stru *hmac_user)
{
    osal_u32 ret;

    /* 2.1 如果是单播，需要设置PTK */
    if (pairwise == OSAL_TRUE) {
        ret = hmac_11i_add_ptk_key(hmac_vap, hmac_user, key_index);
    /* 2.2 如果是组播，需要设置GTK */
    } else {
        if (hmac_vap->hal_vap == OSAL_NULL) {
            return OAL_ERR_CODE_PTR_NULL;
        }
        ret = hmac_11i_add_gtk_key(hmac_vap, hmac_user, key_index);
    }
    if (ret != OAL_SUCC) {
        return ret;
    }

    return OAL_SUCC;
}

OAL_STATIC osal_u32  hmac_config_11i_add_key_ext(hmac_vap_stru *hmac_vap, osal_u8 pairwise,
    osal_u8 key_index, hmac_user_stru *hmac_user)
{
    osal_u32 ret;

    if (pairwise == OSAL_TRUE) {
        hmac_user->ptk_need_install = OSAL_TRUE;

        /* For the first time connection, first set key then send 4/4 EAPOL frame;
         * for the second rekey connection, first send 4/4 EAPOL frame then set key. */
        if ((hmac_user->is_rx_eapol_key_open == OSAL_FALSE) && (hmac_user->eapol_key_4_4_tx_succ == OSAL_FALSE)) {
            hmac_user->ptk_key_idx = key_index;
            oam_warning_log2(0, OAM_SF_WPA, "vap_id[%d] {hmac_config_11i_add_key_ext::user id %d, key_idx %d}",
                hmac_vap->vap_id, key_index);
            return OAL_SUCC;
        }
    }

    /* 设置硬件寄存器 */
    ret = hmac_config_11i_add_key_set_reg(hmac_vap, pairwise, key_index, hmac_user);
    if (ret != OAL_SUCC) {
        oam_error_log2(0, 0, "vap_id[%d] {hmac_config_11i_add_key_ext::set_reg fail[%d].}", hmac_vap->vap_id, ret);
        return ret;
    }

    if (pairwise == OSAL_TRUE) {
        hmac_user->ptk_need_install      = OSAL_FALSE;
        hmac_user->eapol_key_4_4_tx_succ = OSAL_FALSE;
        hmac_user->ptk_key_idx           = 0;
    }
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_config_11i_remove_key
 功能描述  : remove key
*****************************************************************************/
static osal_s32 hmac_config_11i_remove_key(hmac_vap_stru *hmac_vap,
    mac_removekey_param_stru *removekey_params)
{
    osal_u32 ret;
    osal_u8 key_index;
    oal_bool_enum_uint8 pairwise;
    osal_u8 *mac_addr = OSAL_NULL;

    /* 1.1 入参检查 */
    if ((hmac_vap == OSAL_NULL) || (removekey_params == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_WPA, "{hmac_config_11i_remove_key::param null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 2.1 获取参数 */
    key_index = removekey_params->key_index;
    pairwise  = removekey_params->pairwise;
    mac_addr = (osal_u8*)removekey_params->mac_addr;

    /* 3.1 如果是单播 */
    if ((hmac_addr_is_zero_etc(mac_addr) != OSAL_TRUE) && (pairwise == OSAL_TRUE)) {
        ret = hmac_11i_del_ptk_key(hmac_vap, key_index, mac_addr);
        if (ret != OAL_SUCC) {
            oam_error_log2(0, OAM_SF_WPA,
                "vap_id[%d] {hmac_config_11i_remove_key::hmac_11i_del_ptk_key failed[%d].}", hmac_vap->vap_id, ret);
            return (osal_s32)ret;
        }
    /* 3.2 如果是组播 */
    } else {
        ret = hmac_11i_del_gtk_key(hmac_vap, key_index);
        if (ret != OAL_SUCC) {
            oam_error_log2(0, OAM_SF_WPA,
                "vap_id[%d] {hmac_config_11i_remove_key::hmac_11i_del_gtk_key failed[%d].}", hmac_vap->vap_id, ret);

            return (osal_s32)ret;
        }
    }
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_11i_tkip_mic_failure_handler
 功能描述  : 将mic事件上报到hmac
*****************************************************************************/
osal_void hmac_11i_tkip_mic_failure_handler(const hmac_vap_stru *hmac_vap, const osal_u8 *user_mac,
    oal_nl80211_key_type key_type)
{
    hmac_mic_event_stru mic_event = {0};
    osal_s32 ret;
    frw_msg msg = {0};

    if ((hmac_vap == OSAL_NULL) || (user_mac == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_WPA, "{hmac_11i_tkip_mic_failure_handler::param null.}");
        return;
    }

    /* 将mic信息上报给hmac */
    (osal_void)memcpy_s(mic_event.user_mac, WLAN_MAC_ADDR_LEN, user_mac, WLAN_MAC_ADDR_LEN);
    mic_event.key_type = key_type;
    mic_event.key_id = 0; /* tkip 只支持1个密钥，写死0 */

    msg.data = (osal_u8 *)&mic_event;
    msg.data_len = OAL_SIZEOF(hmac_mic_event_stru);
    ret = frw_asyn_host_post_msg(WLAN_MSG_H2W_MIC_FAILURE, FRW_POST_PRI_LOW, hmac_vap->vap_id, &msg);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_WPA, "{hmac_11i_tkip_mic_failure_handler::} post msg to wal failed ret[%d]", ret);
    }
    return;
}

/*****************************************************************************
 函 数 名  : hmac_get_key_info
 功能描述  : 获取key
 输出参数  : osal_u32
 返 回 值  : 0:成功,其他:失败
 调用函数  : 无
 被调函数  : 无
*****************************************************************************/
OAL_STATIC wlan_priv_key_param_stru *hmac_get_key_info(hmac_vap_stru *hmac_vap, osal_u8 *mac_addr,
    oal_bool_enum_uint8 pairwise, osal_u8 key_index, osal_u16 *pus_user_idx)
{
    oal_bool_enum_uint8         macaddr_is_zero;
    hmac_user_stru             *hmac_user = OAL_PTR_NULL;
    osal_u32                  ul_ret;

    /* 1.1 根据mac addr 找到对应sta索引号 */
    macaddr_is_zero = hmac_addr_is_zero_etc(mac_addr);
    if (hmac_11i_is_ptk(macaddr_is_zero, pairwise) == OAL_FALSE) {
        /* 如果是组播用户，不能使用mac地址来查找 */

        /* 根据索引找到组播user内存区域 */
        *pus_user_idx = hmac_vap->multi_user_idx;
    } else { /* 单播用户 */
        ul_ret = hmac_vap_find_user_by_macaddr_etc(hmac_vap, mac_addr, pus_user_idx);
        if (ul_ret != OAL_SUCC) {
            oam_warning_log2(0, OAM_SF_WPA,
                "vap_id[%d] {hmac_get_key_info::hmac_vap_find_user_by_macaddr_etc failed[%d]}",
                hmac_vap->vap_id, ul_ret);
            return OAL_PTR_NULL;
        }
    }

    hmac_user = (hmac_user_stru *)mac_res_get_hmac_user_etc(*pus_user_idx);
    if (hmac_user == OAL_PTR_NULL) {
        oam_warning_log2(0, OAM_SF_WPA, "vap_id[%d] {hmac_get_key_info::hmac_user[%d] null.}",
            hmac_vap->vap_id, *pus_user_idx);
        return OAL_PTR_NULL;
    }

    /* LOG */
    oam_info_log3(0, OAM_SF_WPA,
        "vap_id[%d] {hmac_get_key_info::key_index=%d,pairwise=%d.}", hmac_vap->vap_id, key_index, pairwise);

    if (mac_addr != OAL_PTR_NULL) {
        oam_info_log1(0, OAM_SF_WPA, "{hmac_get_key_info::user[%d]", *pus_user_idx);
        oam_info_log4(0, OAM_SF_WPA, "{hmac_get_key_info::mac_addr = %02X:%02X:%02X:%02X:XX:XX.}",
            /* 0:1:2:3:数组下标 */
            mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3]);
    }

    return hmac_user_get_key_etc(hmac_user, key_index);
}

OAL_STATIC osal_u32 hmac_config_11i_add_key_get_user(mac_addkey_param_stru *param, hmac_vap_stru *hmac_vap,
    hmac_user_stru **hmac_user)
{
    osal_u8 key_index;
    oal_bool_enum_uint8 pairwise;
    osal_u16 user_idx = MAC_INVALID_USER_ID;
    osal_u32 ret;
    osal_u8 *mac_addr = OAL_PTR_NULL;
    mac_key_params_stru *key = OAL_PTR_NULL;

    key_index = param->key_index;
    pairwise  = param->pairwise;
    mac_addr = (osal_u8 *)param->mac_addr;
    key      = &(param->key);

    if (key_index >= WLAN_NUM_TK + WLAN_NUM_IGTK) {
        oam_error_log2(0, OAM_SF_WPA, "vap_id[%d]{hmac_config_11i_add_key_get_user::invalid key_index[%d]}",
            hmac_vap->vap_id, key_index);
        return OAL_ERR_CODE_SECURITY_KEY_ID;
    }

    oam_info_log3(0, OAM_SF_WPA, "vap_id[%d] {hmac_config_11i_add_key_get_user::key_index=%d, pairwise=%d.}",
        hmac_vap->vap_id, key_index, pairwise);
    oam_info_log4(0, OAM_SF_WPA,
        "vap_id[%d] {hmac_config_11i_add_key_get_user::params cipher=0x%08x, keylen=%d, seqlen=%d.}",
        hmac_vap->vap_id, key->cipher, key->key_len, key->seq_len);

    oam_info_log4(0, OAM_SF_WPA, "{hmac_config_11i_add_key_get_user::mac addr=%02X:%02X:%02X:%02X:XX:XX}",
        mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3]); /* 0、1、2、3:打印MAC */

    if (pairwise == OAL_TRUE) {
        /* 单播密钥存放在单播用户中 */
        ret = hmac_vap_find_user_by_macaddr_etc(hmac_vap, mac_addr, &user_idx);
        if (ret != OAL_SUCC) {
            // 驱动删用户与hostapd删用户在时序上无法保证原子过程，可能出现二者同时删除的情形
            oam_warning_log3(0, OAM_SF_WPA, "vap[%d]{hmac_config_11i_add_key_get_user::user[%d] find_user fail[%d]}",
                hmac_vap->vap_id, user_idx, ret);
            return ret;
        }
    } else {
#ifdef _PRE_WLAN_FEATURE_DFR
        /* DFR记录组播秘钥,用于自愈时恢复 */
        hmac_dfr_fill_ap_recovery_info(WLAN_MSG_W2H_CFG_ADD_KEY, param, hmac_vap);
#endif
        /* 组播密钥存放在组播用户中 */
        user_idx = hmac_vap->multi_user_idx;
    }
    oam_info_log2(0, OAM_SF_WPA, "vap_id[%d] {hmac_config_11i_add_key_get_user::user_idx=%d.}",
        hmac_vap->vap_id, user_idx);

    *hmac_user = (hmac_user_stru *)mac_res_get_hmac_user_etc(user_idx);
    if (*hmac_user == OAL_PTR_NULL) {
        oam_error_log2(0, OAM_SF_WPA, "vap[%d]{hmac_config_11i_add_key_get_user::get_mac_user null.idx:%u}",
            hmac_vap->vap_id, user_idx);
        return OAL_ERR_CODE_SECURITY_USER_INVAILD;
    }
    return OAL_CONTINUE;
}

OAL_STATIC osal_u32 hmac_config_11i_set_key(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    mac_addkey_param_stru *param)
{
    osal_u32 ret;
    mac_key_params_stru *key = &(param->key);
    osal_void *fhook;

    ret = hmac_vap_add_key_etc(hmac_vap,  hmac_user, param->key_index, key);
    if (ret != OAL_SUCC) {
        oam_error_log2(0, OAM_SF_WPA, "vap_id[%d] {hmac_config_11i_set_key::mac_11i_add_key fail[%d].}",
            hmac_vap->vap_id, ret);
        return ret;
    }

    if (param->pairwise == OAL_TRUE) {
        hmac_user_set_key_etc(hmac_user, WLAN_KEY_TYPE_PTK, (osal_u8)key->cipher, param->key_index);
    } else {
        hmac_user_set_key_etc(hmac_user, WLAN_KEY_TYPE_RX_GTK, (osal_u8)key->cipher, param->key_index);
    }

    /* 设置用户8021x端口合法性的状态为合法 */
    hmac_user_set_port_etc(hmac_user, OAL_TRUE);

    fhook = hmac_get_feature_fhook(HMAC_FHOOK_ROAM_ADD_KEY_DONE);
    if (fhook != OSAL_NULL) {
        ((hmac_roam_add_key_done_etc_cb)fhook)(hmac_vap, param->pairwise);
    }

    return OAL_CONTINUE;
}

/*****************************************************************************
 函 数 名  : hmac_config_11i_add_key_etc
 功能描述  : add key 逻辑，抛事件到DMAC
 输入参数  : frw_event_mem_stru *event_mem
 输出参数  : osal_u32
 返 回 值  : 0:成功,其他:失败
 调用函数  : 无
 被调函数  : 无
*****************************************************************************/
osal_s32 hmac_config_11i_add_key_etc(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    hmac_user_stru *hmac_user = OAL_PTR_NULL;
    mac_addkey_param_stru *addkey_param = OSAL_NULL;
    osal_u32 ret;
    osal_void *wapi_add_key = hmac_get_feature_fhook(HMAC_FHOOK_WAPI_ADD_KEY);
    osal_void *wapi_reset = hmac_get_feature_fhook(HMAC_FHOOK_WAPI_RESET);
    osal_void *fhook = hmac_get_feature_fhook(HMAC_FHOOK_FRAG_DERAG_CLEAR);

    /* 1.1 入参检查 */
    if ((hmac_vap == OAL_PTR_NULL) || (msg->data == OAL_PTR_NULL)) {
        oam_error_log2(0, OAM_SF_WPA, "{hmac_config_11i_add_key_etc::ptr null,mac_vap=%p,param=%p}",
            (uintptr_t)hmac_vap, (uintptr_t)msg->data);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 2.1 获取参数 */
    addkey_param = (mac_addkey_param_stru *)msg->data;

    if (wapi_add_key != OSAL_NULL) {
        ret = ((hmac_wapi_add_key_and_sync_etc_cb)wapi_add_key)(hmac_vap, addkey_param->key.cipher, addkey_param);
        if (ret != OAL_CONTINUE) {
            return (osal_s32)ret;
        }
    }

    /* 2.2 索引值最大值检查 */
    ret = hmac_config_11i_add_key_get_user(addkey_param, hmac_vap, &hmac_user);
    if (ret != OAL_CONTINUE) {
        return (osal_s32)ret;
    }

    /* add_key 流程中清除user下的分片缓存，防止重关联或者rekey流程报文重组attack */
    if (fhook != OSAL_NULL) {
        ((hmac_user_clear_defrag_res_cb)fhook)(hmac_user);
    }

    if ((wapi_reset != OSAL_NULL) && (((hmac_11i_reset_wapi_cb)wapi_reset)(hmac_vap, hmac_user) != OAL_CONTINUE)) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 3.1 将加密属性更新到用户中 */
    ret = hmac_config_11i_set_key(hmac_vap, hmac_user, addkey_param);
    if (ret != OAL_CONTINUE) {
        return (osal_s32)ret;
    }

    /* WEP模式下不需要在addkey流程设置寄存器 */
    if (mac_is_wep_enabled(hmac_vap) != OSAL_TRUE) {
        ret = hmac_config_11i_add_key_ext(hmac_vap, addkey_param->pairwise, addkey_param->key_index, hmac_user);
        oam_info_log2(0, 0, "vap_id[%d]{hmac_config_11i_add_key_etc::11i_add_key %d}", hmac_vap->vap_id, ret);
    } else {
        ret = OAL_SUCC;
        /* set default key接口未适配，此处写死使用index 0，将组播密钥写入寄存器 */
        if ((addkey_param->pairwise == OSAL_FALSE) && (addkey_param->key_index == 0)) {
            ret = hmac_11i_add_wep_key(hmac_vap, addkey_param->key_index);
            oam_info_log2(0, 0, "vap_id[%d]{hmac_config_11i_add_key_etc::add wep multi key:%u}", hmac_vap->vap_id, ret);
        }
    }

    if (ret == OAL_SUCC) {
        /* 组播密钥更新 则同步至device */
        hmac_user_sync(hmac_user);
    } else {
        oam_warning_log2(0, 0, "vap_id[%d]{hmac_config_11i_add_key_etc::add key fail[%u]}", hmac_vap->vap_id, ret);
    }

    return (osal_s32)ret;
}

/*****************************************************************************
函 数 名  : hmac_config_11i_get_key_etc
功能描述  : 获取密钥，不需要抛事件到DMAC,直接从hmac数据区中拿数据
输入参数  : frw_event_mem_stru *event_mem
输出参数  : osal_u32
返 回 值  : 0:成功,其他:失败
调用函数  : 无
被调函数  : 无
*****************************************************************************/
osal_s32 hmac_config_11i_get_key_etc(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    wlan_priv_key_param_stru     *priv_key = OAL_PTR_NULL;
    oal_key_params_stru           key;
    osal_u8                     key_index;
    osal_u16                    user_idx  = MAC_INVALID_USER_ID;
    oal_bool_enum_uint8           pairwise;
    osal_u8                    *mac_addr = OAL_PTR_NULL;
    void                         *cookie;
    void                        (*callback)(void*, oal_key_params_stru*);
    mac_getkey_param_stru        *payload_getkey_param = OAL_PTR_NULL;

    /* 1.1 ~{Hk2N<l2i~} */
    if ((hmac_vap == OAL_PTR_NULL) || (msg->data == OAL_PTR_NULL)) {
        oam_error_log2(0, OAM_SF_WPA, "{hmac_config_11i_get_key_etc::param null, hmac_vap=%p, param=%p.}",
            (uintptr_t)hmac_vap, (uintptr_t)msg->data);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 2.1 ~{;qH!2NJ}~} */
    payload_getkey_param = (mac_getkey_param_stru *)msg->data;

    key_index = payload_getkey_param->key_index;
    pairwise  = payload_getkey_param->pairwise;
    mac_addr = payload_getkey_param->mac_addr;
    cookie       = payload_getkey_param->cookie;
    callback     = payload_getkey_param->callback;

    /* 2.2 索引值最大值检查 */
    if (key_index >= WLAN_NUM_TK + WLAN_NUM_IGTK) {
        oam_error_log2(0, OAM_SF_WPA, "vap_id[%d] {hmac_config_11i_get_key_etc::key_index invalid[%d].}",
            hmac_vap->vap_id, key_index);
        return OAL_ERR_CODE_SECURITY_KEY_ID;
    }

    /* 3.1 获取密钥 */
    priv_key = hmac_get_key_info(hmac_vap, mac_addr, pairwise, key_index, &user_idx);
    /* begin: 四次握手获取密钥信息长度为0 是正常值,不应该为error 级别打印 */
    if (priv_key == OAL_PTR_NULL) {
        oam_error_log3(0, OAM_SF_WPA, "vap_id[%d] {hmac_config_11i_get_key_etc::key is null.pairwise[%d], key_idx[%d]}",
            hmac_vap->vap_id, pairwise, key_index);
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (priv_key->key_len == 0) {
        oam_info_log3(0, OAM_SF_WPA, "vap_id[%d] {hmac_config_11i_get_key_etc::key len = 0.pairwise[%d], key_idx[%d]}",
            hmac_vap->vap_id, pairwise, key_index);
        return OAL_ERR_CODE_SECURITY_KEY_LEN;
    }
    /* end: 四次握手获取密钥信息长度为0 是正常值,不应该为error 级别打印 */

    /* 4.1 密钥赋值转换 */
    memset_s(&key, sizeof(key), 0, sizeof(key));
    key.key     = priv_key->key;
    key.key_len = (osal_s32)priv_key->key_len;
    key.seq     = priv_key->seq;
    key.seq_len = (osal_s32)priv_key->seq_len;
    key.cipher  = priv_key->cipher;

    /* 5.1 调用回调函数 */
    if (callback) {
        callback(cookie, &key);
    }

    return OAL_SUCC;
}

OAL_STATIC osal_u32 hmac_config_11i_get_key_info(hmac_vap_stru *hmac_vap, osal_u8 *mac_addr,
    oal_bool_enum_uint8 pairwise, osal_u8 key_index, wlan_priv_key_param_stru **priv_key)
{
    osal_u16 user_idx = MAC_INVALID_USER_ID;

    *priv_key = hmac_get_key_info(hmac_vap, mac_addr, pairwise, key_index, &user_idx);
    if (*priv_key == OAL_PTR_NULL) {
        if (user_idx == MAC_INVALID_USER_ID) {
            oam_warning_log1(0, OAM_SF_WPA, "vap_id[%d]{hmac_config_11i_remove_key_etc::user del}", hmac_vap->vap_id);
            return OAL_SUCC;
        } else {
            oam_warning_log1(0, OAM_SF_WPA, "vap_id[%d] {hmac_config_11i_remove_key_etc::priv_key null.}",
                hmac_vap->vap_id);
            return OAL_ERR_CODE_SECURITY_USER_INVAILD;
        }
    }

    if ((*priv_key)->key_len == 0) {
        /* 如果检测到密钥没有使用， 则直接返回正确 */
        oam_info_log1(0, OAM_SF_WPA, "vap_id[%d] {hmac_config_11i_remove_key_etc::key_len=0.}", hmac_vap->vap_id);
        return OAL_SUCC;
    }
    return OAL_CONTINUE;
}

OAL_STATIC osal_u32 hmac_config_11i_set_priv_key(osal_u8 *mac_addr, oal_bool_enum_uint8 pairwise,
    hmac_vap_stru *hmac_vap)
{
    oal_bool_enum_uint8 macaddr_is_zero = hmac_addr_is_zero_etc(mac_addr);
    hmac_user_stru *hmac_user;

    if (hmac_11i_is_ptk(macaddr_is_zero, pairwise) == OAL_TRUE) {
        hmac_user = hmac_vap_get_user_by_addr_etc(hmac_vap, mac_addr, WLAN_MAC_ADDR_LEN);
        if (hmac_user == OAL_PTR_NULL) {
            return OAL_ERR_CODE_SECURITY_USER_INVAILD;
        }
        hmac_user->user_tx_info.security.cipher_key_type = HAL_KEY_TYPE_BUTT;
    } else {
        hmac_user = (hmac_user_stru *)mac_res_get_hmac_user_etc(hmac_vap->multi_user_idx);
        if (hmac_user == OAL_PTR_NULL) {
            return OAL_ERR_CODE_SECURITY_USER_INVAILD;
        }
    }

    hmac_user_set_port_etc(hmac_user, OAL_FALSE);
    hmac_user_init_key_etc(hmac_user);

    if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_STA && hmac_vap->vap_state != MAC_VAP_STATE_STA_FAKE_UP) {
        hmac_user_set_pmf_active_etc(hmac_user, hmac_vap->user_pmf_cap);
    }
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_wep_remove_key
 功能描述  : 将一个wep密钥从硬件LUT表中删除
*****************************************************************************/
static osal_void hmac_wep_remove_key(const hmac_vap_stru *hmac_vap, osal_u8 key_id)
{
    hal_security_key_stru wep_key = {0};

    wep_key.key_id = key_id;
    wep_key.key_type = WLAN_KEY_TYPE_TX_GTK;
    if (hmac_vap->hal_vap != OSAL_NULL) {
        wep_key.lut_idx = hmac_vap->hal_vap->vap_id;
    }

    /* 写硬件寄存器   */
    hal_ce_del_key(&wep_key);
}

/*****************************************************************************
 函 数 名  : hmac_config_wep_remove_key
 功能描述  : 删除 wep密钥key信息
*****************************************************************************/
static osal_s32 hmac_config_wep_remove_key(hmac_vap_stru *hmac_vap, osal_u8 key_id)
{
    /* 从硬件Lut表删除 wep key */
    if (is_sta(hmac_vap)) {
        hmac_wep_remove_key(hmac_vap, key_id);
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_config_11i_remove_key_etc
 功能描述  : 处理remove key事件，抛事件到DMAC
 输入参数  : hmac_vap_stru *hmac_vap, osal_u16 len, osal_u8 *param
 输出参数  : osal_u32
 返 回 值  : 0:成功,其他:失败
 调用函数  : 无
 被调函数  : 无
*****************************************************************************/
osal_s32 hmac_config_11i_remove_key_etc(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    wlan_priv_key_param_stru  *priv_key = OAL_PTR_NULL;
    osal_s32 ret;
    osal_u8 key_index;
    osal_u8 vap_id;
    oal_bool_enum_uint8 pairwise;
    osal_u8 *mac_addr;
    mac_removekey_param_stru *removekey_params = OAL_PTR_NULL;

    /* 1.1 入参检查 */
    if ((hmac_vap == OAL_PTR_NULL) || (msg->data == OAL_PTR_NULL)) {
        oam_error_log2(0, OAM_SF_WPA, "{hmac_config_11i_remove_key_etc::param null,hmac_vap=%p, param=%p.}",
            (uintptr_t)hmac_vap, (uintptr_t)msg->data);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 2.1 获取参数 */
    removekey_params = (mac_removekey_param_stru *)msg->data;
    key_index = removekey_params->key_index;
    pairwise  = removekey_params->pairwise;
    mac_addr = removekey_params->mac_addr;
    vap_id = hmac_vap->vap_id;

    oam_info_log3(0, OAM_SF_WPA, "vap_id[%d] {hmac_config_11i_remove_key_etc::key_index=%d, pairwise=%d.}",
        vap_id, key_index, pairwise);

    /* 2.2 索引值最大值检查 */
    if (key_index >= WLAN_NUM_TK + WLAN_NUM_IGTK) {
        /* 内核会下发删除6 个组播密钥，驱动现有6个组播密钥保存空间 */
        /* 对于检测到key idx > 最大密钥数，不做处理 */
        oam_info_log2(0, OAM_SF_WPA, "vap[%d]{hmac_config_11i_remove_key_etc::key_index[%d]}", vap_id, key_index);
        return OAL_SUCC;
    }

    /* 3.1 获取本地密钥信息 */
    ret = (osal_s32)hmac_config_11i_get_key_info(hmac_vap, mac_addr, pairwise, key_index, &priv_key);
    if (ret != OAL_CONTINUE) {
        return ret;
    }

    /* 4.1 区分是wep还是wpa */
    if ((priv_key->cipher == WITP_WLAN_CIPHER_SUITE_WEP40) || (priv_key->cipher == WITP_WLAN_CIPHER_SUITE_WEP104)) {
        mac_mib_set_wep_etc(hmac_vap, key_index, WLAN_WEP_40_KEY_SIZE);
        ret = hmac_config_wep_remove_key(hmac_vap, key_index);
    } else {
        ret = (osal_s32)hmac_config_11i_set_priv_key(mac_addr, pairwise, hmac_vap);
        if (ret != OAL_SUCC) {
            return ret;
        }
        /* 4.2 抛事件到dmac层处理 */
        ret = hmac_config_11i_remove_key(hmac_vap, removekey_params);
    }
    if (OAL_UNLIKELY(ret != OAL_SUCC)) {
        oam_error_log2(0, OAM_SF_WPA, "vap[%d]{hmac_config_11i_remove_key_etc::send_event fail[%d]}",
            vap_id, ret);
        return ret;
    }

    /* 5.1 删除密钥成功，设置密钥长度为0 */
    priv_key->key_len = 0;

    return ret;
}

static osal_void hmac_config_wep_add_entry(hmac_user_stru *hmac_user)
{
    hal_security_key_stru wep_key = { 0 };

    wep_key.key_id = hmac_user->key_info.default_index;
    wep_key.cipher_type = hmac_user->key_info.cipher_type;
    wep_key.key_type = WLAN_KEY_TYPE_PTK;
    wep_key.lut_idx = hmac_user->lut_index;
    wep_key.update_key = OSAL_FALSE;
    wep_key.key_origin = WLAN_AUTH_KEY;

    /* 密钥已经添加过，不需要重复添加，只需要激活用户即可 */
    wep_key.cipher_key = hmac_user->key_info.key[wep_key.key_id].key;
    wep_key.mic_key = OSAL_NULL;

    /* 写硬件寄存器   */
    hal_ce_add_key(&wep_key);

    return;
}

/*****************************************************************************
 函 数 名  : hmac_config_11i_add_wep_entry_etc
 功能描述  : add wep加密，抛事件到DMAC
 输入参数  : hmac_vap_stru *hmac_vap, osal_u16 len, osal_u8 *param
 输出参数  : osal_u32
 返 回 值  : 0:成功,其他:失败
 调用函数  : 无
 被调函数  : 无
*****************************************************************************/
osal_u32 hmac_config_11i_add_wep_entry_etc(hmac_vap_stru *hmac_vap, const osal_u8 *param)
{
    hmac_user_stru *hmac_user = OSAL_NULL;
    osal_u32 ret;

    if (hmac_vap == OAL_PTR_NULL || param == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_WPA, "{hmac_config_11i_add_wep_entry_etc::PARMA NULL}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_user = (hmac_user_stru *)hmac_vap_get_user_by_addr_etc(hmac_vap, param, WLAN_MAC_ADDR_LEN);
    if (hmac_user == OAL_PTR_NULL) {
        oam_error_log1(0, OAM_SF_WPA, "vap_id[%d] {hmac_config_11i_add_wep_entry_etc::mac_user NULL}",
            hmac_vap->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    ret = hmac_user_update_wep_key_etc(hmac_user, hmac_vap->multi_user_idx);
    if (ret != OAL_SUCC) {
        oam_error_log2(0, OAM_SF_WPA,
            "vap_id[%d] {hmac_config_11i_add_wep_entry_etc::mac_wep_add_usr_key failed[%d].}",
            hmac_vap->vap_id, ret);
        return ret;
    }

    hmac_config_wep_add_entry(hmac_user);

    /* 设置用户的发送加密套件 */
    oam_info_log2(0, OAM_SF_WPA,
        "vap_id[%d] {hmac_config_11i_add_wep_entry_etc:: usridx[%d] OK.}", hmac_vap->vap_id,
        hmac_user->assoc_id);

    return ret;
}

/*****************************************************************************
 函 数 名  : hmac_init_security_etc
 功能描述  : 初始化加密数据。
 输入参数  : hmac_user,addr
 输出参数  : 无
 返 回 值  : mac_status_code_enum_uint16
*****************************************************************************/
osal_u32 hmac_init_security_etc(hmac_vap_stru *hmac_vap, const osal_u8 *addr)
{
    osal_u32 ul_ret = OAL_SUCC;

    if (hmac_vap == OAL_PTR_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (mac_is_wep_enabled(hmac_vap) == OAL_TRUE) {
        ul_ret = hmac_config_11i_add_wep_entry_etc(hmac_vap, addr);
    }
    return ul_ret;
}

/*****************************************************************************
 函 数 名  : hmac_check_capability_mac_phy_supplicant_etc
 功能描述  : 检测期望join 的AP能力信息是否匹配
 输入参数  : hmac_vap_stru hmac_vap         STA 自己
             mac_bss_dscr_stru *bss_dscr  AP bss 信息
 输出参数  : 无
 返 回 值  : osal_u32 OAL_SUCC 匹配成功
                        OAL_FAIL 匹配失败
*****************************************************************************/
osal_u32 hmac_check_capability_mac_phy_supplicant_etc(hmac_vap_stru *hmac_vap, mac_bss_dscr_stru  *bss_dscr)
{
    osal_u32 ul_ret;

    if ((hmac_vap == OAL_PTR_NULL) || (bss_dscr == OAL_PTR_NULL)) {
        oam_warning_log2(0, OAM_SF_WPA, "{hmac_check_capability_mac_phy_supplicant_etc::input null %p %p.}",
            (uintptr_t)hmac_vap, (uintptr_t)bss_dscr);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 根据协议模式重新初始化STA HT/VHT mib值 */
    hmac_vap_config_vht_ht_mib_by_protocol_etc(hmac_vap);

    ul_ret = hmac_check_bss_cap_info_etc(bss_dscr->us_cap_info, hmac_vap);
    if (ul_ret != OAL_TRUE) {
        /* MAC能力不做严格检查 */
        oam_warning_log2(0, OAM_SF_WPA,
            "vap_id[%d] {hmac_check_capability_mac_phy_supplicant_etc::hmac_check_bss_cap_info_etc failed[%d].}",
            hmac_vap->vap_id, ul_ret);
    }

#ifdef _PRE_WIFI_DEBUG
    /* check bss capability info PHY,忽略PHY能力不匹配的AP */
    mac_vap_check_bss_cap_info_phy_ap_etc(bss_dscr->us_cap_info, hmac_vap);
#endif

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_sta_protocol_down_by_chipher
 功能描述  : 更新STA加密的mib 信息
 输出参数  : 无
 返 回 值  : osal_u32
*****************************************************************************/
osal_void hmac_sta_protocol_down_by_chipher(hmac_vap_stru *hmac_vap, mac_bss_dscr_stru *bss_dscr)
{
    mac_cfg_mode_param_stru         cfg_mode;
    osal_u32                      pair_suite[WLAN_PAIRWISE_CIPHER_SUITES] = {0};
    oal_bool_enum_uint8             legcy_only     = OAL_FALSE;
    osal_void *fhook = hmac_get_feature_fhook(HMAC_FHOOK_WAPI_UPDATE_LEGCY_ONLY);

    if (hmac_vap->protocol >= WLAN_HT_MODE) {
        /* 在WEP / TKIP 加密模式下，不能工作在HT MODE */
        if (mac_mib_get_privacyinvoked(hmac_vap) == OAL_TRUE && mac_mib_get_rsnaactivated(hmac_vap) == OAL_FALSE) {
            legcy_only = OAL_TRUE;
        }

        if (hmac_vap->cap_flag.wpa == OAL_TRUE) {
            pair_suite[0] = MAC_WPA_CHIPER_TKIP;

            if (mac_mib_wpa_pair_match_suites(hmac_vap, (osal_u32 *)pair_suite) != 0) {
                legcy_only = OAL_TRUE;
            }
        }

        if (hmac_vap->cap_flag.wpa2 == OAL_TRUE) {
            pair_suite[0] = MAC_RSN_CHIPER_TKIP;

            if (mac_mib_rsn_pair_match_suites(hmac_vap, (osal_u32 *)pair_suite) != 0) {
                legcy_only = OAL_TRUE;
            }
        }

        if (fhook != OSAL_NULL) {
            ((hmac_wapi_update_legcy_only_cb)fhook)(bss_dscr, &legcy_only);
        }
    }

    oam_warning_log4(0, OAM_SF_WPA, "vap_id[%d] hmac_sta_protocol_down_by_chipher::legacy_only[%d],wpa[%d]wpa2[%d]",
        hmac_vap->vap_id, legcy_only, hmac_vap->cap_flag.wpa, hmac_vap->cap_flag.wpa2);

    cfg_mode.protocol = hmac_vap->protocol;

    if (legcy_only == OAL_TRUE) {
        if (hmac_vap->channel.band == WLAN_BAND_2G) {
            cfg_mode.protocol = WLAN_MIXED_ONE_11G_MODE;
        } else if (hmac_vap->channel.band == WLAN_BAND_5G) {
            cfg_mode.protocol = WLAN_LEGACY_11A_MODE;
        }
        hmac_vap->channel.en_bandwidth = WLAN_BAND_WIDTH_20M;
    }

    if (cfg_mode.protocol >= WLAN_HT_MODE) {
        mac_mib_set_TxAggregateActived(hmac_vap, OAL_TRUE);
        mac_mib_set_AmsduAggregateAtive(hmac_vap, OAL_TRUE);
    } else {
        mac_mib_set_TxAggregateActived(hmac_vap, OAL_FALSE);
        mac_mib_set_AmsduAggregateAtive(hmac_vap, OAL_FALSE);
    }

#ifdef _PRE_WIFI_DMT
    hmac_config_sta_update_rates_etc(hmac_vap, &cfg_mode, OAL_PTR_NULL);
#endif

    hmac_vap_init_by_protocol_etc(hmac_vap, cfg_mode.protocol);

    return;
}

/*****************************************************************************
 函 数 名  : hmac_en_mic_etc
 功能描述  : 增加mic校验码
 输出参数  : iv头的长度
*****************************************************************************/
osal_u32 hmac_en_mic_etc(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user, oal_netbuf_stru *pst_netbuf,
    osal_u8 *iv_len)
{
    wlan_priv_key_param_stru             *pst_key        = OAL_PTR_NULL;
    osal_u32                            ul_ret;
    wlan_ciper_protocol_type_enum_uint8   cipher_type;
    wlan_cipher_key_type_enum_uint8       key_type;

    /* 1.1 入参检查 */
    if ((hmac_vap == OAL_PTR_NULL) || (hmac_user == OAL_PTR_NULL) || (pst_netbuf == OAL_PTR_NULL) ||
        (iv_len == OAL_PTR_NULL)) {
        oam_error_log4(0, OAM_SF_WPA, "{hmac_en_mic_etc::hmac_vap=%p hmac_user=%p pst_netbuf=%p iv_len=%p.}",
            (uintptr_t)hmac_vap, (uintptr_t)hmac_user, (uintptr_t)pst_netbuf, (uintptr_t)iv_len);
        return OAL_ERR_CODE_PTR_NULL;
    }

    *iv_len    = 0;
    key_type    = hmac_user->user_tx_info.security.cipher_key_type;
    cipher_type = hmac_user->key_info.cipher_type;
    pst_key = hmac_user_get_key_etc(hmac_user, key_type - 1);
    if (pst_key == OAL_PTR_NULL) {
        oam_error_log1(0, OAM_SF_WPA, "{hmac_en_mic_etc::hmac_user_get_key_etc FAIL. key_type[%d]}", key_type);
        return OAL_ERR_CODE_SECURITY_KEY_ID;
    }

    switch (cipher_type) {
        case WLAN_80211_CIPHER_SUITE_TKIP:
            if (key_type  == 0 || key_type > 5) { /* 5 加密类型 */
                return OAL_ERR_CODE_SECURITY_KEY_TYPE;
            }
            ul_ret = hmac_crypto_tkip_enmic_etc(pst_key, pst_netbuf);
            if (ul_ret != OAL_SUCC) {
                oam_error_log2(0, OAM_SF_WPA, "vap_id[%d] {hmac_en_mic_etc::hmac_crypto_tkip_enmic_etc failed[%d].}",
                    hmac_vap->vap_id, ul_ret);
                return ul_ret;
            }
            *iv_len = WEP_IV_FIELD_SIZE + EXT_IV_FIELD_SIZE;
            break;
        case WLAN_80211_CIPHER_SUITE_CCMP:
            *iv_len = WEP_IV_FIELD_SIZE + EXT_IV_FIELD_SIZE;
            break;
        default:
            break;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_de_mic_etc
 功能描述  : mic码校验
*****************************************************************************/
osal_u32 hmac_de_mic_etc(hmac_user_stru *hmac_user, oal_netbuf_stru *pst_netbuf)
{
    wlan_priv_key_param_stru             *pst_key        = OAL_PTR_NULL;
    osal_u32                            ul_ret;
    wlan_ciper_protocol_type_enum_uint8   cipher_type;
    wlan_cipher_key_type_enum_uint8       key_type;

    /* 1.1 入参检查 */
    if ((hmac_user == OAL_PTR_NULL) ||
        (pst_netbuf == OAL_PTR_NULL)) {
        oam_warning_log0(0, OAM_SF_WPA, "{hmac_de_mic_etc::param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    key_type    = hmac_user->user_tx_info.security.cipher_key_type;
    cipher_type = hmac_user->key_info.cipher_type;
    pst_key = hmac_user_get_key_etc(hmac_user, key_type - 1);
    if (pst_key == OAL_PTR_NULL) {
        oam_warning_log1(0, OAM_SF_WPA, "{hmac_de_mic_etc::hmac_user_get_key_etc FAIL. key_type[%d]}", key_type);
        return OAL_ERR_CODE_SECURITY_KEY_ID;
    }

    switch (cipher_type) {
        case WLAN_80211_CIPHER_SUITE_TKIP:
            if (key_type  == 0 || key_type > 5) { /* 5 加密类型 */
                return OAL_ERR_CODE_SECURITY_KEY_TYPE;
            }
            ul_ret = hmac_crypto_tkip_demic_etc(pst_key, pst_netbuf);
            if (ul_ret != OAL_SUCC) {
                oam_warning_log2(0, OAM_SF_WPA,
                    "vap_id[%d] {hmac_de_mic_etc::hmac_crypto_tkip_demic_etc failed[%d].}",
                    hmac_user->vap_id, ul_ret);
                return ul_ret;
            }
            break;
        default:
            break;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_11i_port_filter
 功能描述  : 接收数据，安全相关过滤
 输入参数  : (1)vap
             (2)mac地址
             (3)接收数据类型
 输出参数  : 无
 返 回 值  : 成功或者失败
*****************************************************************************/
WIFI_HMAC_TCM_TEXT WIFI_TCM_TEXT osal_u32 hmac_11i_ether_type_filter_etc(hmac_vap_stru *hmac_vap,
    const hmac_user_stru *hmac_user, osal_u16 ether_type)
{
    osal_bool wapi_type = OSAL_TRUE;
    osal_void *fhook = hmac_get_feature_fhook(HMAC_FHOOK_WAPI_ETHER_TYPE_CHECK);
    if (fhook != OSAL_NULL) {
        wapi_type = ((hmac_wapi_ether_type_check_cb)fhook)(ether_type);
    }
    if (hmac_user->port_valid == OAL_FALSE) { /* 判断端口是否打开 */
        /* 接收数据时，针对非EAPOL 的数据帧做过滤 */
        if (oal_byteorder_host_to_net_uint16(ETHER_TYPE_PAE) != ether_type && wapi_type) {
            oam_warning_log2(0, OAM_SF_WPA,
                "vap_id[%d] {hmac_11i_ether_type_filter_etc::TYPE 0x%04x not permission.}",
                hmac_vap->vap_id, ether_type);
            return OAL_ERR_CODE_SECURITY_PORT_INVALID;
        }
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_config_set_pmksa_etc
 功能描述  : 增加一条pmk缓存记录
 输入参数  : hmac_vap_stru *hmac_vap
             osal_u16 len
             osal_u8 *param
 输出参数  : 无
*****************************************************************************/
osal_s32 hmac_config_set_pmksa_etc(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    hmac_pmksa_cache_stru              *pmksa_cache;
    struct osal_list_head                *pmksa_entry;
    struct osal_list_head                *pmksa_entry_tmp;
    osal_u32                          pmksa_count = 0;
    mac_cfg_pmksa_param_stru           *cfg_pmksa = OAL_PTR_NULL;

    if ((hmac_vap == OAL_PTR_NULL) || (msg == OAL_PTR_NULL) || (msg->data == OAL_PTR_NULL)) {
        oam_error_log0(0, OAM_SF_CFG, "{hmac_config_set_pmksa_etc param null}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    oam_warning_log1(0, OAM_SF_CFG, "hmac_config_set_pmksa_etc::vap_id[%d]", hmac_vap->vap_id);

    cfg_pmksa = (mac_cfg_pmksa_param_stru *)msg->data;
    if (osal_list_empty(&(hmac_vap->pmksa_list_head)) != 0) {
        OSAL_INIT_LIST_HEAD(&(hmac_vap->pmksa_list_head));
    }

    osal_list_for_each_safe(pmksa_entry, pmksa_entry_tmp, &(hmac_vap->pmksa_list_head)) {
        pmksa_cache = osal_list_entry(pmksa_entry, hmac_pmksa_cache_stru, entry);
        /* 已存在时，先删除，保证最新的pmk在dlist头部 */
        if (oal_compare_mac_addr(cfg_pmksa->bssid, pmksa_cache->bssid) == 0) {
            osal_list_del(pmksa_entry);
            oal_mem_free(pmksa_cache, OAL_TRUE);
            oam_warning_log4(0, OAM_SF_CFG, "hmac_config_set_pmksa_etc::DEL first[%02X:%02X:%02X:%02X:XX:XX]",
                /* 0:1:2:3:数组下标 */
                cfg_pmksa->bssid[0], cfg_pmksa->bssid[1], cfg_pmksa->bssid[2], cfg_pmksa->bssid[3]);
        }
        pmksa_count++;
    }

    if (pmksa_count > WLAN_PMKID_CACHE_SIZE) {
        /* 超过最大个数时，先队列尾，保证最新的pmk在dlist头部 */
        oam_warning_log4(0, OAM_SF_CFG, "hmac_config_set_pmksa_etc::store more pmksa [%02X:%02X:%02X:%02X:XX:XX] fail",
            /* 0:1:2:3:数组下标 */
            cfg_pmksa->bssid[0], cfg_pmksa->bssid[1], cfg_pmksa->bssid[2], cfg_pmksa->bssid[3]);
        pmksa_entry = hmac_vap->pmksa_list_head.prev; // list tail entry
        osal_list_del(pmksa_entry);
        pmksa_cache  = osal_list_entry(pmksa_entry, hmac_pmksa_cache_stru, entry);
        oal_mem_free(pmksa_cache, OAL_TRUE);
    }

    pmksa_cache = oal_mem_alloc(OAL_MEM_POOL_ID_LOCAL, OAL_SIZEOF(hmac_pmksa_cache_stru), OAL_TRUE);
    if (pmksa_cache == OAL_PTR_NULL) {
        oam_error_log4(0, OAM_SF_CFG, "hmac_config_set_pmksa_etc:: oal_mem_alloc fail [%02X:%02X:%02X:%02X:XX:XX]",
            /* 0:1:2:3:数组下标 */
            cfg_pmksa->bssid[0], cfg_pmksa->bssid[1], cfg_pmksa->bssid[2], cfg_pmksa->bssid[3]);
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (memcpy_s(pmksa_cache->bssid, sizeof(pmksa_cache->bssid), cfg_pmksa->bssid, OAL_MAC_ADDR_LEN) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_config_set_pmksa_etc::memcpy_s bssid error}");
    }
    if (memcpy_s(pmksa_cache->pmkid, sizeof(pmksa_cache->pmkid), cfg_pmksa->pmkid, WLAN_PMKID_LEN) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_config_set_pmksa_etc::memcpy_s pmkid error}");
    }

    osal_list_add(&(pmksa_cache->entry), &(hmac_vap->pmksa_list_head));

    oam_warning_log4(0, OAM_SF_CFG, "hmac_config_set_pmksa_etc::SET pmksa for[%02X:%02X:%02X:%02X:XX:XX] OK",
        /* 0:1:2:3:数组下标 */
        cfg_pmksa->bssid[0], cfg_pmksa->bssid[1], cfg_pmksa->bssid[2], cfg_pmksa->bssid[3]);

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_config_del_pmksa_etc
 功能描述  : 删除一条pmk缓存记录
 输入参数  : hmac_vap_stru *hmac_vap
             osal_u16 len
             osal_u8 *param
 输出参数  : 无
*****************************************************************************/
osal_s32 hmac_config_del_pmksa_etc(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    mac_cfg_pmksa_param_stru           *cfg_pmksa;
    hmac_pmksa_cache_stru              *pmksa_cache;
    struct osal_list_head                *pmksa_entry;
    struct osal_list_head                *pmksa_entry_tmp;

    if ((hmac_vap == OAL_PTR_NULL) || (msg->data == OAL_PTR_NULL)) {
        oam_error_log0(0, OAM_SF_CFG, "{hmac_config_del_pmksa_etc param null}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    cfg_pmksa    = (mac_cfg_pmksa_param_stru *)msg->data;

    if (osal_list_empty(&(hmac_vap->pmksa_list_head)) != 0) {
        oam_warning_log1(0, OAM_SF_CFG, "vap_id[%d]", hmac_vap->vap_id);
        oam_warning_log4(0, OAM_SF_CFG, "hmac_config_del_pmksa_etc:: pmksa dlist is null [%02X:%02X:%02X:%02X:XX:XX]",
                         /* 0:1:2:3:数组下标 */
                         cfg_pmksa->bssid[0], cfg_pmksa->bssid[1], cfg_pmksa->bssid[2], cfg_pmksa->bssid[3]);
    }

    osal_list_for_each_safe(pmksa_entry, pmksa_entry_tmp, &(hmac_vap->pmksa_list_head)) {
        pmksa_cache = osal_list_entry(pmksa_entry, hmac_pmksa_cache_stru, entry);
        if (oal_compare_mac_addr(cfg_pmksa->bssid, pmksa_cache->bssid) == 0) {
            osal_list_del(pmksa_entry);
            oal_mem_free(pmksa_cache, OAL_TRUE);
            oam_warning_log1(0, OAM_SF_CFG, "vap_id[%d]", hmac_vap->vap_id);
            oam_warning_log4(0, OAM_SF_CFG, "hmac_config_del_pmksa_etc:: DEL pmksa of [%02X:%02X:%02X:%02X:XX:XX]",
                /* 0:1:2:3:数组下标 */
                cfg_pmksa->bssid[0], cfg_pmksa->bssid[1], cfg_pmksa->bssid[2], cfg_pmksa->bssid[3]);
            return OAL_SUCC;
        }
    }

    oam_warning_log1(0, OAM_SF_CFG, "vap_id[%d]", hmac_vap->vap_id);
    oam_warning_log4(0, OAM_SF_CFG, "hmac_config_del_pmksa_etc:: NO pmksa of [%02X:%02X:%02X:%02X:XX:XX]",
        /* 0:1:2:3:数组下标 */
        cfg_pmksa->bssid[0], cfg_pmksa->bssid[1], cfg_pmksa->bssid[2], cfg_pmksa->bssid[3]);
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_config_flush_pmksa_etc
 功能描述  : 清除pmk缓存记录
 输入参数  : hmac_vap_stru *hmac_vap
             osal_u16 len
             osal_u8 *param
 输出参数  : 无
*****************************************************************************/
osal_s32 hmac_config_flush_pmksa_etc(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    hmac_pmksa_cache_stru              *pmksa_cache;
    struct osal_list_head                *pmksa_entry;
    struct osal_list_head                *pmksa_entry_tmp;

    unref_param(msg);

    if (hmac_vap == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_CFG, "{hmac_config_flush_pmksa_etc param null}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (osal_list_empty(&(hmac_vap->pmksa_list_head)) != 0) {
        return OAL_SUCC;
    }

    osal_list_for_each_safe(pmksa_entry, pmksa_entry_tmp, &(hmac_vap->pmksa_list_head)) {
        pmksa_cache = osal_list_entry(pmksa_entry, hmac_pmksa_cache_stru, entry);

        oam_warning_log1(0, OAM_SF_CFG, "vap_id[%d]", hmac_vap->vap_id);
        oam_warning_log4(0, OAM_SF_CFG, "hmac_config_flush_pmksa_etc:: DEL pmksa of [%02X:%02X:%02X:%02X:XX:XX]",
            /* 0:1:2:3:数组下标 */
            pmksa_cache->bssid[0], pmksa_cache->bssid[1], pmksa_cache->bssid[2], pmksa_cache->bssid[3]);

        osal_list_del(pmksa_entry);
        oal_mem_free(pmksa_cache, OAL_TRUE);
    }

    return OAL_SUCC;
}

oal_module_symbol(hmac_config_11i_remove_key_etc);
oal_module_symbol(hmac_config_11i_get_key_etc);
oal_module_symbol(hmac_config_11i_add_key_etc);
oal_module_symbol(hmac_config_11i_add_wep_entry_etc);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

