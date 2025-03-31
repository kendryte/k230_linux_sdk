/*
 * Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
 * Description: hal_ce.c
 * Create: 2023-01-16
 */

#include "hal_common_ops.h"
#include "hal_mac.h"
#include "mac_resource_ext.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_HAL_CE_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

#define HH503_LUT_RETRY_INTERVAL 10
#define HH503_LUT_OPER_READ  0
#define HH503_LUT_OPER_WRITE 1
#define HH503_LUT_OPER_DEL   2
#define HH503_LUT_OPER_CLR   3

#define HH503_PN_FRM_TYPE_MCBC_DATA  0
#define HH503_PN_FRM_TYPE_UC_QOS     1
#define HH503_PN_FRM_TYPE_UC_NQOS    2
#define HH503_PN_FRM_TYPE_UC_MGMT    3
#define HH503_PN_FRM_TYPE_MCBC_MGMT  4

#define HH503_MAX_USR_NUM  8

#define HH503_KEY_IGTK 0
#define HH503_KEY_PTK  1
#define HH503_KEY_GTK  2

#ifdef _PRE_WLAN_FEATURE_PMF
/*****************************************************************************
函 数 名  : hh503_set_bip_decrypto
功能描述  : 配置vap级别的广播管理帧解密开关
*****************************************************************************/
osal_void hh503_set_bip_decrypto(const hal_to_dmac_vap_stru *hal_vap, oal_bool_enum_uint8 crypto)
{
    /* 开启/关闭pmf相关开关 */
    u_ce_control ce;
    u_vap2_mode_set0 vap2_mode_set0;

    if (hal_vap->vap_id == HAL_VAP_VAP0) {
        ce.u32 = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x8);
        ce.bits.xCt2OPzG3OI2IROyCO4mCwOmo_ = crypto;
        hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x8, ce.u32);
    } else if (hal_vap->vap_id == HAL_VAP_VAP1) {
        ce.u32 = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x8);
        ce.bits.x6NgwqGu0wSgSLwj6wFW6_wWh_ = crypto;
        hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x8, ce.u32);
    } else {
        vap2_mode_set0.u32 = hal_reg_read(HH503_MAC_CTRL2_BANK_BASE_0x118);
        vap2_mode_set0.bits.xCt2OPzGZOI2IROyCO4mCwOmo_ = crypto;
        hal_reg_write(HH503_MAC_CTRL2_BANK_BASE_0x118, vap2_mode_set0.u32);
    }
}

/*****************************************************************************
功能描述  : 配置pmf加解密寄存器
*****************************************************************************/
osal_void hh503_control_pmf_reg(hal_device_stru *hal_device, oal_bool_enum_uint8 enable_pmf)
{
    u_ce_control ce;
    /* 操作PMF相关寄存器时需要提前tx suspend，然后关PA再开始操作 */
    hh503_set_machw_tx_suspend();
    hal_save_machw_phy_pa_status(&hal_device->hal_device_base);
    hh503_disable_machw_phy_and_pa();

    ce.u32 = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x8);
    ce.bits.x6NgwuSNwFW6wWh_ = enable_pmf;
    hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x8, ce.u32);

    hh503_recover_machw_phy_and_pa(&hal_device->hal_device_base);
    hal_set_machw_tx_resume();
}

/*****************************************************************************
功能描述  : 配置pmf加解密开关
*****************************************************************************/
osal_void hh503_set_pmf_crypto(hal_to_dmac_vap_stru *hal_vap, oal_bool_enum_uint8 crypto)
{
    u_ce_control ce;
    hal_device_stru *hal_device = OSAL_NULL;

    hal_device = hh503_chip_get_device();

    /* 开启/关闭pmf相关开关 */
    /* 加密寄存器常开,不参与配置(与王浩确认无问题),解决去关联时删用户过快导致帧未加密问题 */
        /* 1. Red CE enable bit */
    ce.u32 = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x8);
    if (ce.bits.x6NgwuSNwFW6wWh_ == crypto) {
        /* BIT19:管理帧加密功能使能位. BIT20:管理帧解密功能使能位 */
        hh503_set_bip_decrypto(hal_vap, crypto);
        return;
    }
    hh503_control_pmf_reg(hal_device, crypto);
}
#endif

OSAL_STATIC osal_u32 hh503_is_ce_lut_done(osal_void)
{
    osal_u32 retry = 0;

    u_ce_lut_config ce_lut;
    while (retry < HH503_PA_LUT_UPDATE_TIMEOUT) {
        ce_lut.u32 = hal_reg_read(HH503_MAC_LUT0_BANK_BASE_0x24);
        if (ce_lut.bits.xCt2OdmQOxSROvGmwOmo_ == 0) {
            return OSAL_TRUE;
        }

        // busy now
        osal_udelay(HH503_LUT_RETRY_INTERVAL); /* 延时10us */
        retry += HH503_LUT_RETRY_INTERVAL;
        continue;
    }
    return OSAL_FALSE;
}

/*****************************************************************************
 功能描述  : 使能ce功能
*****************************************************************************/
osal_u32 hh503_enable_ce(osal_void)
{
    u_ce_control ce;

    ce.u32 = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x8);
    ce.bits.xCt2O4zRzOmoCwOmo_ = 1;
    ce.bits.x6NgwFGLGwFW6_wWh_ = 1;
    ce.bits.x6NgwuSNwWh6_wWh_ = 1;
    ce.bits.x6NgwuSNwFW6wWh_ = 1;
    ce.bits.xCt2OPzG3OI2IROyCO4mCwOmo_ = 1;
    ce.bits.x6NgwqGu0wSgSLwj6wFW6_wWh_ = 1;
    hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x8, ce.u32);
    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : ce_ctrl寄存器初始化
 目的：减少 hal_initialize_machw()函数代码行
*****************************************************************************/
osal_void hh503_mac_ce_ctr_init(osal_void)
{
    u_ce_control ce_ctrl;
    u_vap2_mode_set0 vap2_mode_set0;

    /* CE相关的Lut表资源主路辅路共用，辅路不再重复配置 */
    /* 修改为默认关闭管理帧加解密，与vap能力保持一致，防止能力未对齐出现广播帧接收异常 */
    /* 注意:修改该寄存器默认值 须同步修改vap能力位默认值:cap_info.pmf_active */
    ce_ctrl.u32 = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x8);
    vap2_mode_set0.u32 = hal_reg_read(HH503_MAC_CTRL2_BANK_BASE_0x118);
    /* 加密寄存器常开(与王浩确认无问题),解决去关联时删用户过快导致帧未加密问题 */

    /* vap0-1 的解密能力位对应BIT8 BIT9         vap2对应vap2_mode_set0寄存器BIT3 */

    /* cfg_11n_tkip_bypass_en需要是bypass  */
    ce_ctrl.bits.x6NgwuSNwFW6wWh_ = 0;
    ce_ctrl.bits.x6NgwuSNwWh6_wWh_ = 1;
    ce_ctrl.bits.xCt2OPzG3OI2IROyCO4mCwOmo_ = 0;
    ce_ctrl.bits.x6NgwqGu0wSgSLwj6wFW6_wWh_ = 0;
    ce_ctrl.bits.x6Ngw00hwLKBuwjtuGCCwWh_ = 1;
    hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x8, ce_ctrl.u32);

    vap2_mode_set0.bits.xCt2OPzGZOI2IROyCO4mCwOmo_ = 0;
    hal_reg_write(HH503_MAC_CTRL2_BANK_BASE_0x118, vap2_mode_set0.u32);
}

/*****************************************************************************
 功能描述 : 清除lut中的密钥
*****************************************************************************/
osal_void hh503_ce_clear_all(osal_void)
{
    u_ce_lut_config ce_lut = {.u32 = 0};
    u_peer_addr_lut_config peer_addr = {.u32 = 0};
    osal_u32 oper_sel = 0;
    /* 将0值写入key mickey */
    hal_reg_write(HH503_MAC_LUT0_BANK_BASE_0x14, 0x0);
    hal_reg_write(HH503_MAC_LUT0_BANK_BASE_0x18, 0x0);
    hal_reg_write(HH503_MAC_LUT0_BANK_BASE_0x1C, 0x0);
    hal_reg_write(HH503_MAC_LUT0_BANK_BASE_0x20, 0x0);

    /* 清空ce_key */
    ce_lut.bits.xCt2OdmQOxSROvGmwOmo_ = 1;
    ce_lut.bits.xCt2OdmQOxSROvGmw_ = HH503_LUT_OPER_CLR;
    ce_lut.bits.x6NgwKWtwodLwiuW_wSiFW_ = 1; // 【0】：表示配置模式为ce_key还是rx_pn：1表示ce_key，0表示rx_pn。

    hal_reg_write(HH503_MAC_LUT0_BANK_BASE_0x24, ce_lut.u32);
    if (hh503_is_ce_lut_done() != OSAL_TRUE) {
        oam_error_log0(0, OAM_SF_ANY, "{hh503_ce_clear_all::wait lut ce set done failed.}");
        return;
    }

    /* 清空rx_pn */
    ce_lut.bits.x6NgwKWtwodLwiuW_wSiFW_ = 0; // 【0】：表示配置模式为ce_key还是rx_pn：1表示ce_key，0表示rx_pn。
    hal_reg_write(HH503_MAC_LUT0_BANK_BASE_0x24, ce_lut.u32);
    if (hh503_is_ce_lut_done() != OSAL_TRUE) {
        oam_error_log0(0, OAM_SF_ANY, "{hh503_ce_clear_all::wait lut ce set done failed.}");
        return;
    }

    /* Peer ADDR表清零 */
    for (oper_sel = 0; oper_sel <= 3; oper_sel++) { /* oper_sel 取值范围0~3，对应四张表 */
        peer_addr.bits.x6NgwuWW_wGFF_wodLwiuW__ = HH503_LUT_OPER_CLR;
        peer_addr.bits.x6NgwuWW_wGFF_wodLwiuW_wWh_ = OSAL_TRUE;
        peer_addr.bits.x6NgwuWW_wGFF_wodLwiuW_wCWo_ = oper_sel;
        hal_reg_write(HH503_MAC_LUT0_BANK_BASE_0x8, peer_addr.u32);

        /* 检查 LUT_EN Bit 以确认硬件写入成功 */
        if (hh503_wait_mac_done_timeout(HH503_MAC_LUT0_BANK_BASE_0x8, BIT0, BIT0,
            HAL_CE_LUT_UPDATE_TIMEOUT) != OAL_SUCC) {
            oam_error_log0(0, OAM_SF_ANY, "{hh503_ce_clear_all::waiting lut done timeout!}\r\n");
        }
    }
}

/*****************************************************************************
功能描述 : 添加一个cipher key到lut 表
*****************************************************************************/
osal_void hh503_ce_add_mic_key_process(const hal_security_key_stru *security_key)
{
    u_ce_lut_config ce_lut = {.u32 = 0};
    osal_u32 reg_value;
    osal_u8 *mic_key = security_key->mic_key;
    osal_u16 x6NgwKWtwodLwGFF__ = 0;

    /* 将TKIP MIC 密钥地址写入TKIP_MIC_KEY */
    if (mic_key == OSAL_NULL) {
        return;
    }

    if (security_key->cipher_type == WLAN_80211_CIPHER_SUITE_TKIP) {
        /* key第3、2、1、0字节，左移8、16、24位 */
        reg_value = mic_key[0] | (mic_key[1] << 8) | (mic_key[2] << 16) | ((osal_u32)mic_key[3] << 24);
        mic_key += 4; /* 索引位置增加4 */
        hal_reg_write(HH503_MAC_LUT0_BANK_BASE_0x20, reg_value);
        /* key第3、2、1、0字节，左移8、16、24位 */
        reg_value = mic_key[0] | (mic_key[1] << 8) | (mic_key[2] << 16) | ((osal_u32)mic_key[3] << 24);
        mic_key += 4; /* 索引位置增加4 */
        hal_reg_write(HH503_MAC_LUT0_BANK_BASE_0x1C, reg_value);
        /* key第3、2、1、0字节，左移8、16、24位 */
        reg_value = mic_key[0] | (mic_key[1] << 8) | (mic_key[2] << 16) | ((osal_u32)mic_key[3] << 24);
        mic_key += 4; /* 索引位置增加4 */
        hal_reg_write(HH503_MAC_LUT0_BANK_BASE_0x18, reg_value);
        /* key第3、2、1、0字节，左移8、16、24位 */
        reg_value = mic_key[0] | (mic_key[1] << 8) | (mic_key[2] << 16) | ((osal_u32)mic_key[3] << 24);
        hal_reg_write(HH503_MAC_LUT0_BANK_BASE_0x14, reg_value);
    } else {
        /* ccmp256、gcmp256类写密钥方式与cipher_key顺序保持一致，key第3、2、1、0字节，左移8、16、24位 */
        reg_value = mic_key[3] | (mic_key[2] << 8) | (mic_key[1] << 16) | ((osal_u32)mic_key[0] << 24);
        mic_key += 4; /* 索引加4 */
        hal_reg_write(HH503_MAC_LUT0_BANK_BASE_0x14, reg_value);
        /* key第3、2、1、0字节，左移8、16、24位 */
        reg_value = mic_key[3] | (mic_key[2] << 8) | (mic_key[1] << 16) | ((osal_u32)mic_key[0] << 24);
        mic_key += 4; /* 索引加4 */
        hal_reg_write(HH503_MAC_LUT0_BANK_BASE_0x18, reg_value);
        /* key第3、2、1、0字节，左移8、16、24位 */
        reg_value = mic_key[3] | (mic_key[2] << 8) | (mic_key[1] << 16) | ((osal_u32)mic_key[0] << 24);
        mic_key += 4; /* 索引加4 */
        hal_reg_write(HH503_MAC_LUT0_BANK_BASE_0x1C, reg_value);
        /* key第3、2、1、0字节，左移8、16、24位 */
        reg_value = mic_key[3] | (mic_key[2] << 8) | (mic_key[1] << 16) | ((osal_u32)mic_key[0] << 24);
        hal_reg_write(HH503_MAC_LUT0_BANK_BASE_0x20, reg_value);
    }

    /* 2.2 将密钥写入MAC内部LUT表 */
    /* 更新 LUT Operation 为写入 */
    ce_lut.bits.xCt2OdmQOxSROvGmw_ = HH503_LUT_OPER_WRITE;
    ce_lut.bits.xCt2OdmQOxSROvGmwOmo_ = OSAL_TRUE;
    ce_lut.bits.x6NgwKWtwodLwiuW_wSiFW_ = 3; // 3: 11b: ce_key: 1; mic_key: 1

    /*
     *  0: peer_idx: 0- 2; bit16-18; 0000_0000_0000_0111b
     *  3: key_type: 3- 6; bit19-22; 0000_0000_0111_1000b
     *  7:   key_id: 7-10; bit23-26; 0000_0111_1000_0000b
     * 11: enc_type:11-14; bit27-30; 0111_1000_0000_0000b
     * 15: vap_type:15-15; bit31-31; 1000_0000_0000_0000b
     */
    x6NgwKWtwodLwGFF__ = ((osal_u16)security_key->lut_idx << 0) |
                       ((osal_u16)security_key->key_type << 3) |
                       ((osal_u16)security_key->key_id << 7) |
                       ((osal_u16)hal_cipher_suite_to_ctype(security_key->cipher_type) << 11) |
                       ((osal_u16)security_key->key_origin << 15); // 左移 3 7 11 15
    ce_lut.bits.x6NgwKWtwodLwGFF__ = x6NgwKWtwodLwGFF__;
    hal_reg_write(HH503_MAC_LUT0_BANK_BASE_0x24, ce_lut.u32);
}

/*****************************************************************************
功能描述  : 将指定的密钥写入硬件寄存器
*****************************************************************************/
oal_bool_enum_uint8 hh503_ce_add_cipher_key(const hal_security_key_stru *security_key)
{
    osal_u8 key_wep[16] = {0}; /* 16表示数组大小 */
    osal_u32 reg_value;
    u_ce_lut_config ce_lut = {.u32 = 0};
    osal_u8 *cipher_key = security_key->cipher_key;
    osal_u16 x6NgwKWtwodLwGFF__ = 0;

    if (cipher_key == OSAL_NULL) {
        return OSAL_TRUE;
    }

    /* 1. 等待Mac Address Lut空闲 BIT0 --LUT操作使能, 0:不使能 1:使能 */
    if (hh503_is_ce_lut_done() != OSAL_TRUE) {
        oam_error_log0(0, OAM_SF_TX, "{hh503_ce_add_cipher_key::wait lut ce idle failed before add.}");
        return OSAL_FALSE;
    }
    /* 2.1 wep密钥需要向后对齐 */
    if (security_key->cipher_type == WLAN_80211_CIPHER_SUITE_WEP_40) {
        /* 偏移16个字节 */
        (osal_void)memmove_s(key_wep + 16 - WLAN_WEP40_KEY_LEN, WLAN_WEP40_KEY_LEN, cipher_key, WLAN_WEP40_KEY_LEN);
        cipher_key = key_wep;
    } else if (security_key->cipher_type == WLAN_80211_CIPHER_SUITE_WEP_104) {
        /* 偏移16个字节 */
        (osal_void)memmove_s(key_wep + 16 - WLAN_WEP104_KEY_LEN, WLAN_WEP104_KEY_LEN, cipher_key, WLAN_WEP104_KEY_LEN);
        cipher_key = key_wep;
    } else if (security_key->cipher_type == WLAN_80211_CIPHER_SUITE_BIP) {
        /* 偏移16个字节 */
        (osal_void)memmove_s(key_wep + 16 - WLAN_BIP_KEY_LEN, WLAN_BIP_KEY_LEN, cipher_key, WLAN_BIP_KEY_LEN);
        cipher_key = key_wep;
    }

    /* 2.2 将密钥地址写入KEY_QTR */
    /* key第3、2、1、0字节，左移8、16、24位 */
    reg_value = cipher_key[3] | (cipher_key[2] << 8) | (cipher_key[1] << 16) | ((osal_u32)cipher_key[0] << 24);
    cipher_key += 4; /* 索引加4 */
    hal_reg_write(HH503_MAC_LUT0_BANK_BASE_0x14, reg_value);
    /* key第3、2、1、0字节，左移8、16、24位 */
    reg_value = cipher_key[3] | (cipher_key[2] << 8) | (cipher_key[1] << 16) | ((osal_u32)cipher_key[0] << 24);
    cipher_key += 4; /* 索引加4 */
    hal_reg_write(HH503_MAC_LUT0_BANK_BASE_0x18, reg_value);
    /* key第3、2、1、0字节，左移8、16、24位 */
    reg_value = cipher_key[3] | (cipher_key[2] << 8) | (cipher_key[1] << 16) | ((osal_u32)cipher_key[0] << 24);
    cipher_key += 4; /* 索引加4 */
    hal_reg_write(HH503_MAC_LUT0_BANK_BASE_0x1C, reg_value);
    /* key第3、2、1、0字节，左移8、16、24位 */
    reg_value = cipher_key[3] | (cipher_key[2] << 8) | (cipher_key[1] << 16) | ((osal_u32)cipher_key[0] << 24);
    hal_reg_write(HH503_MAC_LUT0_BANK_BASE_0x20, reg_value);

    ce_lut.bits.xCt2OdmQOxSROvGmw_ = HH503_LUT_OPER_WRITE;
    ce_lut.bits.xCt2OdmQOxSROvGmwOmo_ = OSAL_TRUE;
    ce_lut.bits.x6NgwKWtwodLwiuW_wSiFW_ = 1; // 1: 01b: ce_key: 1; mic_key: 0

    /*
     *  0: peer_idx: 0- 2; bit16-18; 0000_0000_0000_0111b
     *  3: key_type: 3- 6; bit19-22; 0000_0000_0111_1000b
     *  7:   key_id: 7-10; bit23-26; 0000_0111_1000_0000b
     * 11: enc_type:11-14; bit27-30; 0111_1000_0000_0000b
     * 15: vap_type:15-15; bit31-31; 1000_0000_0000_0000b
     */
    x6NgwKWtwodLwGFF__ = ((osal_u16)security_key->lut_idx << 0) |
                       ((osal_u16)security_key->key_type << 3) |
                       ((osal_u16)security_key->key_id << 7) |
                       ((osal_u16)hal_cipher_suite_to_ctype(security_key->cipher_type) << 11) |
                       ((osal_u16)security_key->key_origin << 15); // 左移 3 7 11 15
    ce_lut.bits.x6NgwKWtwodLwGFF__ = x6NgwKWtwodLwGFF__;
    hal_reg_write(HH503_MAC_LUT0_BANK_BASE_0x24, ce_lut.u32);
    if (hh503_is_ce_lut_done() != OSAL_TRUE) {
        oam_error_log0(0, OAM_SF_TX, "{hh503_ce_add_cipher_key::wait lut ce idle failed when add key }");
        return OSAL_FALSE;
    }

    /* 写密钥 */
    hh503_ce_add_mic_key_process(security_key); // lut控制寄存器

    /* 检查 LUT_EN Bit 以确认硬件写入成功 */
    return hh503_is_ce_lut_done();
}

/*****************************************************************************
功能描述 : 比较cipher key
*****************************************************************************/
osal_u32 hh503_compare_cipher_key(const osal_u8 *cipher_key)
{
    osal_u32 reg_value;
    osal_u8 qtr_key[16]; /* wep_key数组长度16 */

    /* 2.2 将密钥地址写入KEY_QTR */
    reg_value = hal_reg_read(HH503_MAC_LUT0_BANK_BASE_0x14);
    qtr_key[0] = ((reg_value & 0xFF000000) >> 24); /* 右移24位 */
    qtr_key[1] = ((reg_value & 0x00FF0000) >> 16); /* 右移16位 */
    qtr_key[2] = ((reg_value & 0x0000FF00) >> 8);  /* 数组元素2，右移8位 */
    qtr_key[3] = (reg_value & 0x000000FF); /* 数组元素3 */

    reg_value = hal_reg_read(HH503_MAC_LUT0_BANK_BASE_0x18);
    qtr_key[4] = ((reg_value & 0xFF000000) >> 24); /* 数组元素4，右移24位 */
    qtr_key[5] = ((reg_value & 0x00FF0000) >> 16); /* 数组元素5，右移16位 */
    qtr_key[6] = ((reg_value & 0x0000FF00) >> 8);  /* 数组元素6，右移8位 */
    qtr_key[7] = (reg_value & 0x000000FF); /* 数组元素7 */

    reg_value = hal_reg_read(HH503_MAC_LUT0_BANK_BASE_0x1C);
    qtr_key[8] = ((reg_value & 0xFF000000) >> 24); /* 数组元素8，右移24位 */
    qtr_key[9] = ((reg_value & 0x00FF0000) >> 16); /* 数组元素9，右移16位 */
    qtr_key[10] = ((reg_value & 0x0000FF00) >> 8); /* 数组元素10，右移8位 */
    qtr_key[11] = (reg_value & 0x000000FF); /* 数组元素11 */

    reg_value = hal_reg_read(HH503_MAC_LUT0_BANK_BASE_0x20);
    qtr_key[12] = ((reg_value & 0xFF000000) >> 24); /* 数组元素12，右移24位 */
    qtr_key[13] = ((reg_value & 0x00FF0000) >> 16); /* 数组元素13，右移16位 */
    qtr_key[14] = ((reg_value & 0x0000FF00) >> 8);  /* 数组元素14，右移8位 */
    qtr_key[15] = (reg_value & 0x000000FF); /* 数组元素15 */

    if (memcmp(qtr_key, cipher_key, 16)) { /* 长度16字节 */
        oam_error_log0(0, OAM_SF_CRYPTO, "{hh503_compare_cipher_key::KEY is not matched the target!!}\r\n");
        return OAL_ERR_CODE_CFG_REG_ERROR;
    }
    return OAL_SUCC;
}

OAL_STATIC osal_u32 hh503_compare_tkip_mic_key(const hal_security_key_stru *security_key)
{
    osal_u32 reg_value;
    osal_u8 qtr_key[16] = {0}; /* wep_key数组长度16 */
    osal_u8 *mic_key = security_key->mic_key;

    reg_value = hal_reg_read(HH503_MAC_LUT0_BANK_BASE_0x20);
    qtr_key[0] = (reg_value & 0x000000FF);
    qtr_key[1] = ((reg_value & 0x0000FF00) >> 8);  /* 右移8位 */
    qtr_key[2] = ((reg_value & 0x00FF0000) >> 16); /* 数组元素2，右移16位 */
    qtr_key[3] = ((reg_value & 0xFF000000) >> 24); /* 数组元素3，右移24位 */

    reg_value = hal_reg_read(HH503_MAC_LUT0_BANK_BASE_0x1C);
    qtr_key[4] = (reg_value & 0x000000FF); /* 数组元素4 */
    qtr_key[5] = ((reg_value & 0x0000FF00) >> 8);  /* 数组元素5，右移8位 */
    qtr_key[6] = ((reg_value & 0x00FF0000) >> 16); /* 数组元素6，右移16位 */
    qtr_key[7] = ((reg_value & 0xFF000000) >> 24); /* 数组元素7，右移24位 */

    reg_value = hal_reg_read(HH503_MAC_LUT0_BANK_BASE_0x18);
    qtr_key[8] = (reg_value & 0x000000FF); /* 数组元素8 */
    qtr_key[9] = ((reg_value & 0x0000FF00) >> 8);  /* 数组元素9，右移8位 */
    qtr_key[10] = ((reg_value & 0x00FF0000) >> 16); /* 数组元素10，右移16位 */
    qtr_key[11] = ((reg_value & 0xFF000000) >> 24); /* 数组元素11，右移24位 */

    reg_value = hal_reg_read(HH503_MAC_LUT0_BANK_BASE_0x14);
    qtr_key[12] = (reg_value & 0x000000FF); /* 数组元素12 */
    qtr_key[13] = ((reg_value & 0x0000FF00) >> 8);  /* 数组元素13，右移8位 */
    qtr_key[14] = ((reg_value & 0x00FF0000) >> 16); /* 数组元素14，右移16位 */
    qtr_key[15] = ((reg_value & 0xFF000000) >> 24); /* 数组元素15，右移24位 */

    if (memcmp(qtr_key, mic_key, 16) != 0) { /* 长度16字节 */
        oam_error_log0(0, OAM_SF_CRYPTO,
            "{hh503_compare_tkip_mic_key::TKIP_MIC_KEY is not matched the target!!!}\r\n");
        return OAL_ERR_CODE_CFG_REG_ERROR;
    }
    return OAL_SUCC;
}

/*****************************************************************************
功能描述 : 比较mic key
*****************************************************************************/
osal_u32 hh503_compare_mic_key(const hal_security_key_stru *security_key)
{
    osal_u32 reg_value;
    osal_u8 qtr_key[16] = {0}; /* wep_key数组长度16 */
    osal_u8 *mic_key = security_key->mic_key;

    /* 3 将TKIP MIC 密钥地址写入TKIP_MIC_KEY */
    if (mic_key == OSAL_NULL) {
        return OAL_SUCC;
    }
    if (security_key->cipher_type == WLAN_80211_CIPHER_SUITE_TKIP) {
        return hh503_compare_tkip_mic_key(security_key);
    }
    reg_value = hal_reg_read(HH503_MAC_LUT0_BANK_BASE_0x14);
    qtr_key[0] = ((reg_value & 0xFF000000) >> 24); /* 右移24位 */
    qtr_key[1] = ((reg_value & 0x00FF0000) >> 16); /* 右移16位 */
    qtr_key[2] = ((reg_value & 0x0000FF00) >> 8);  /* 数组元素2，右移8位 */
    qtr_key[3] = (reg_value & 0x000000FF); /* 数组元素3 */

    reg_value = hal_reg_read(HH503_MAC_LUT0_BANK_BASE_0x18);
    qtr_key[4] = ((reg_value & 0xFF000000) >> 24); /* 数组元素4，右移24位 */
    qtr_key[5] = ((reg_value & 0x00FF0000) >> 16); /* 数组元素5，右移16位 */
    qtr_key[6] = ((reg_value & 0x0000FF00) >> 8);  /* 数组元素6，右移8位 */
    qtr_key[7] = (reg_value & 0x000000FF); /* 数组元素7 */

    reg_value = hal_reg_read(HH503_MAC_LUT0_BANK_BASE_0x1C);
    qtr_key[8] = ((reg_value & 0xFF000000) >> 24); /* 数组元素8，右移24位 */
    qtr_key[9] = ((reg_value & 0x00FF0000) >> 16); /* 数组元素9，右移16位 */
    qtr_key[10] = ((reg_value & 0x0000FF00) >> 8); /* 数组元素10，右移8位 */
    qtr_key[11] = (reg_value & 0x000000FF); /* 数组元素11 */

    reg_value = hal_reg_read(HH503_MAC_LUT0_BANK_BASE_0x20);
    qtr_key[12] = ((reg_value & 0xFF000000) >> 24); /* 数组元素12，右移24位 */
    qtr_key[13] = ((reg_value & 0x00FF0000) >> 16); /* 数组元素13，右移16位 */
    qtr_key[14] = ((reg_value & 0x0000FF00) >> 8);  /* 数组元素14，右移8位 */
    qtr_key[15] = (reg_value & 0x000000FF); /* 数组元素15 */

    if (memcmp(qtr_key, mic_key, 16)) { /* 长度16字节 */
        oam_error_log0(0, OAM_SF_CRYPTO,
            "{hh503_compare_mic_key::TKIP_MIC_KEY is not matched the target!!!}\r\n");
        return OAL_ERR_CODE_CFG_REG_ERROR;
    }
    return OAL_SUCC;
}

/*****************************************************************************
功能描述  : 读取LUT表
*****************************************************************************/
osal_u32 hh503_ce_get_cipher_key(const hal_security_key_stru *security_key)
{
    osal_u8 *cipher_key = OSAL_NULL;
    osal_u8 wep_key[16] = {0}; /* wep_key数组长度16 */
    u_ce_lut_config ce_lut = {.u32 = 0};
    osal_u16 x6NgwKWtwodLwGFF__ = 0;

    if (security_key == OSAL_NULL || security_key->cipher_key == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{hh503_ce_get_cipher_key:: security_key is null}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }
    cipher_key = security_key->cipher_key;

    if (cipher_key == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{hh503_ce_get_cipher_key:: key is null}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 1. 等待Mac Address Lut空闲 BIT0 --LUT操作使能, 0:不使能 1:使能 */
    if (hh503_is_ce_lut_done() != OSAL_TRUE) {
        oam_error_log0(0, OAM_SF_ANY, "{hh503_ce_get_cipher_key:: wait lut ce idle timeout!!!}\r\n");
        return OAL_ERR_CODE_CFG_REG_TIMEOUT;
    }

    ce_lut.bits.xCt2OdmQOxSROvGmw_ = HH503_LUT_OPER_READ;
    ce_lut.bits.xCt2OdmQOxSROvGmwOmo_ = OSAL_TRUE;

    /*
     *  0: peer_idx: 0- 2; bit16-18; 0000_0000_0000_0111b
     *  3: key_type: 3- 6; bit19-22; 0000_0000_0111_1000b
     *  7:   key_id: 7-10; bit23-26; 0000_0111_1000_0000b
     * 11: enc_type:11-14; bit27-30; 0111_1000_0000_0000b
     * 15: vap_type:15-15; bit31-31; 1000_0000_0000_0000b
     */
    x6NgwKWtwodLwGFF__ = ((osal_u16)security_key->lut_idx << 0) |
                       ((osal_u16)security_key->key_type << 3) |
                       ((osal_u16)security_key->key_id << 7) |
                       ((osal_u16)hal_cipher_suite_to_ctype(security_key->cipher_type) << 11) |
                       ((osal_u16)security_key->key_origin << 15); // 左移 3 7 11 15
    ce_lut.bits.x6NgwKWtwodLwGFF__ = x6NgwKWtwodLwGFF__;

    /* 检查 LUT_EN Bit 以确认硬件写入成功 */
    if (hh503_is_ce_lut_done() != OSAL_TRUE) {
        oam_error_log0(0, OAM_SF_ANY, "{hh503_ce_get_cipher_key:: wait lut ce set done timeout!!!}\r\n");
        return OAL_ERR_CODE_CFG_REG_TIMEOUT;
    }
    /* 2.1 wep密钥需要向后对齐 */
    if (security_key->cipher_type == WLAN_80211_CIPHER_SUITE_WEP_40) {
        /* 偏移16字节 */
        (osal_void)memmove_s(wep_key + 16 - WLAN_WEP40_KEY_LEN, WLAN_WEP40_KEY_LEN, cipher_key, WLAN_WEP40_KEY_LEN);
        cipher_key = wep_key;
    } else if (security_key->cipher_type == WLAN_80211_CIPHER_SUITE_WEP_104) {
        /* 偏移16字节 */
        (osal_void)memmove_s(wep_key + 16 - WLAN_WEP104_KEY_LEN, WLAN_WEP104_KEY_LEN, cipher_key, WLAN_WEP104_KEY_LEN);
        cipher_key = wep_key;
    } else if (security_key->cipher_type == WLAN_80211_CIPHER_SUITE_BIP) {
        /* 偏移16字节 */
        (osal_void)memmove_s(wep_key + 16 - WLAN_BIP_KEY_LEN, WLAN_BIP_KEY_LEN, cipher_key, WLAN_BIP_KEY_LEN);
        cipher_key = wep_key;
    }

    ce_lut.bits.x6NgwKWtwodLwiuW_wSiFW_ = 1; // 1: 01b: ce_key: 1; mic_key: 0
    hal_reg_write(HH503_MAC_LUT0_BANK_BASE_0x24, ce_lut.u32);
    if (hh503_is_ce_lut_done() != OSAL_TRUE) {
        oam_error_log0(0, OAM_SF_ANY, "{hh503_ce_get_cipher_key:wait lut ce set done timeout for cipher key read}\r\n");
        return OAL_ERR_CODE_CFG_REG_TIMEOUT;
    }
    if (hh503_compare_cipher_key(cipher_key) != OAL_SUCC) {
        wifi_printf("\n func[%s] line[%d] cipher_type[%d] fail\n", __func__, __LINE__, security_key->cipher_type);
        return OAL_ERR_CODE_CFG_REG_ERROR;
    }

    ce_lut.bits.x6NgwKWtwodLwiuW_wSiFW_ = 3; // 3: 11b: ce_key: 1; mic_key: 1
    hal_reg_write(HH503_MAC_LUT0_BANK_BASE_0x24, ce_lut.u32);
    if (hh503_compare_mic_key(security_key) != OAL_SUCC) {
        wifi_printf("\n func[%s] line[%d] cipher_type[%d] fail\n", __func__, __LINE__, security_key->cipher_type);
        return OAL_ERR_CODE_CFG_REG_ERROR;
    }

    return OAL_SUCC;
}

/*****************************************************************************
功能描述  : 清除TX PN
*****************************************************************************/
osal_void hh503_clear_tx_pn(const hal_pn_lut_cfg_stru *pn_lut_cfg)
{
    u_peer_addr_lut_config peer_addr_lut_config = {.u32 = 0};

    if (hh503_wait_mac_done_timeout(HH503_MAC_LUT0_BANK_BASE_0x8,
        HH503_MAC_CFG_PEER_ADDR_LUT_OPER_EN_MASK, HH503_MAC_CFG_PEER_ADDR_LUT_OPER_EN_MASK,
        HH503_PA_LUT_UPDATE_TIMEOUT) != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_ANY, "{uapi102_clear_tx_pn::waiting lut pn idle timeout!}\r\n");
    }

    /* LUT操作使能 0：不使能 1：使能 */
    peer_addr_lut_config.bits.x6NgwuWW_wGFF_wodLwiuW_wWh_ = 1;

    peer_addr_lut_config.bits.x6NgwuWW_wGFF_wodLwiuW_wCWo_ = 2; // 类型选择为tx_pn:2
    /* PN/TSC类型 0：组播 1：单播 */
    peer_addr_lut_config.bits.x6NgwuWW_wGFF_wodLwLPwuhwLtuW_ = pn_lut_cfg->pn_key_type;

    if (peer_addr_lut_config.bits.x6NgwuWW_wGFF_wodLwLPwuhwLtuW_ != 1) { // gtk，igtk对端索引
        peer_addr_lut_config.bits.x6NgwuWW_wGFF_wodLwBhFWP_ = 0;
    } else { // ptk时 对端索引
        peer_addr_lut_config.bits.x6NgwuWW_wGFF_wodLwBhFWP_ = pn_lut_cfg->pn_peer_idx;
    }

    /* 对LUT表的操作类型 0：读操作 1：写操作 2：删除操作 */
    peer_addr_lut_config.bits.x6NgwuWW_wGFF_wodLwiuW__ = 2;

    hal_reg_write(HH503_MAC_LUT0_BANK_BASE_0x8, peer_addr_lut_config.u32);
}

osal_void hh503_clear_rx_pn(const hal_pn_lut_cfg_stru *pn_lut_cfg)
{
    osal_u8 tid;
    osal_u8 peer_idx;
    u_ce_lut_config ce_lut = {.u32 = 0};
    osal_u16 x6NgwKWtwodLwGFF__ = 0;

    if (hh503_is_ce_lut_done() != OSAL_TRUE) {
        return;
    }

    ce_lut.bits.xCt2OdmQOxSROvGmw_ = HH503_LUT_OPER_DEL;
    ce_lut.bits.xCt2OdmQOxSROvGmwOmo_ = OSAL_TRUE;

    ce_lut.bits.x6NgwKWtwodLwiuW_wSiFW_ = 0; // 0: 00b: ce_key: 0; mic_key: 0

    /*
     *  0: peer_idx: 0- 2; bit16-18; 0000_0000_0000_0111b
     *  3: key_type: 3- 6; bit19-22; 0000_0000_0111_1000b
     *  7:   key_id: 7-10; bit23-26; 0000_0111_1000_0000b
     * 11: enc_type:11-14; bit27-30; 0111_1000_0000_0000b
     * 15: vap_type:15-15; bit31-31; 1000_0000_0000_0000b
     */
    x6NgwKWtwodLwGFF__ = ((osal_u16)pn_lut_cfg->pn_peer_idx << 0) |
                       ((osal_u16)HH503_KEY_PTK << 3) |
                       ((osal_u16)pn_lut_cfg->pn_key_type << 11);

    /* 清除单播Qos帧所有TID的RX PN */
    if ((pn_lut_cfg->all_tid == OSAL_TRUE) && (pn_lut_cfg->pn_key_type == HH503_PN_FRM_TYPE_UC_QOS)) {
        for (tid = 0; tid < 8; tid++) { /* 8表示界限大小 */
            x6NgwKWtwodLwGFF__ |= ((osal_u16)tid << 7) & 0x0780; // 7: key_id; 0x0780: mask
            ce_lut.bits.x6NgwKWtwodLwGFF__ = x6NgwKWtwodLwGFF__;
            hal_reg_write(HH503_MAC_LUT0_BANK_BASE_0x24, ce_lut.u32);
            x6NgwKWtwodLwGFF__ &= ~(0x0780); // 0x0780: mask
            ce_lut.bits.x6NgwKWtwodLwGFF__ = x6NgwKWtwodLwGFF__;
            if (hh503_is_ce_lut_done() != OSAL_TRUE) {
                wifi_printf("\n func[%s] line[%d] tid[%d]\n", __func__, __LINE__, tid);
            }
        }
        return;
    }

    /* 清除单播非Qos帧和单播管理帧的rx            _pn */
    if (pn_lut_cfg->pn_key_type == HH503_PN_FRM_TYPE_UC_NQOS || pn_lut_cfg->pn_key_type == HH503_PN_FRM_TYPE_UC_MGMT) {
        hal_reg_write(HH503_MAC_LUT0_BANK_BASE_0x24, ce_lut.u32);
        if (hh503_is_ce_lut_done() != OSAL_TRUE) {
            wifi_printf("\n func[%s] line[%d] \n", __func__, __LINE__);
        }
        return;
    }

    // 更新组播秘钥，清除所有用户RX PN
    // pn_lut_cfg->pn_key_type 为 HH503_PN_FRM_TYPE_UC_MCBC_DATA HH503_PN_FRM_TYPE_UC_MCBC_MGMT
    for (peer_idx = 0; peer_idx < HH503_MAX_USR_NUM; peer_idx++) {
        ce_lut.bits.x6NgwKWtwodLwGFF__ = x6NgwKWtwodLwGFF__;
        hal_reg_write(HH503_MAC_LUT0_BANK_BASE_0x24, ce_lut.u32);
        if (hh503_is_ce_lut_done() != OSAL_TRUE) {
            wifi_printf("\n func[%s] line[%d] tid[%d]\n", __func__, __LINE__, peer_idx);
            return;
        }
    }
}

/*****************************************************************************
功能描述  : 清除TX PN
*****************************************************************************/
osal_void hh503_clear_rx_tx_pn(const hal_security_key_stru *security_key)
{
    hal_pn_lut_cfg_stru pn_lut_cfg = {0};

    /* 单播密钥配置 */
    if (security_key->key_type == WLAN_KEY_TYPE_PTK) {
        pn_lut_cfg.pn_key_type = 1;
        pn_lut_cfg.pn_peer_idx = security_key->lut_idx;
        hh503_clear_tx_pn(&pn_lut_cfg);

        /* 单播Qos帧 */
        pn_lut_cfg.pn_key_type = HH503_PN_FRM_TYPE_UC_QOS;
        pn_lut_cfg.all_tid = OSAL_TRUE;
        hh503_clear_rx_pn(&pn_lut_cfg);
        /* 单播非Qos帧 */
        pn_lut_cfg.pn_key_type = HH503_PN_FRM_TYPE_UC_NQOS; /* key类型为2 */
        pn_lut_cfg.all_tid = OSAL_FALSE;
        hh503_clear_rx_pn(&pn_lut_cfg);

        pn_lut_cfg.pn_key_type = HH503_PN_FRM_TYPE_UC_MGMT; /* 3: 单播管理帧 */
        pn_lut_cfg.all_tid = OSAL_FALSE;
        hh503_clear_rx_pn(&pn_lut_cfg);
    } else { /* 组播配置 */
        pn_lut_cfg.pn_key_type = HH503_PN_FRM_TYPE_MCBC_DATA;
        pn_lut_cfg.pn_peer_idx = security_key->lut_idx;
        hh503_clear_tx_pn(&pn_lut_cfg);
        hh503_clear_rx_pn(&pn_lut_cfg);
        pn_lut_cfg.pn_key_type = HH503_PN_FRM_TYPE_MCBC_MGMT;   /* 4: 组播/广播管理帧 */
        pn_lut_cfg.pn_peer_idx = security_key->lut_idx;
        hh503_clear_rx_pn(&pn_lut_cfg);
    }
}
/* key和mic key复用同一个寄存器，所以读取前要将lut config寄存器配成不同的值 */
osal_void hh503_ce_get_key_process(osal_u8 *cipher_key, osal_u8 *mic_key, const hal_security_key_stru *security_key)
{
    osal_u32 reg_value;
    osal_u8 temp_key[16] = {0}; /* 16表示数组大小 */
    u_ce_lut_config ce_lut = {.u32 = 0};
    osal_u16 x6NgwKWtwodLwGFF__ = 0;

    /* 1. 等待Mac Address Lut空闲 BIT0 --LUT操作使能, 0:不使能 1:使能 */
    if (hh503_is_ce_lut_done() != OSAL_TRUE) {
        oam_error_log0(0, OAM_SF_ANY, "{hh503_ce_get_key_process:: wait lut ce idle timeout!!!}\r\n");
        return;
    }
    /* 2.2 将密钥写入MAC内部LUT表 */
    /* 更新 LUT Operation 为读取 */
    ce_lut.bits.xCt2OdmQOxSROvGmw_ = HH503_LUT_OPER_READ;

    /*
     *  0: peer_idx: 0- 2; bit16-18; 0000_0000_0000_0111b
     *  3: key_type: 3- 6; bit19-22; 0000_0000_0111_1000b
     *  7:   key_id: 7-10; bit23-26; 0000_0111_1000_0000b
     * 11: enc_type:11-14; bit27-30; 0111_1000_0000_0000b
     * 15: vap_type:15-15; bit31-31; 1000_0000_0000_0000b
     */
    x6NgwKWtwodLwGFF__ = ((osal_u16)security_key->lut_idx << 0) |
                       ((osal_u16)security_key->key_type << 3) |
                       ((osal_u16)security_key->key_id << 7) |
                       ((osal_u16)hal_cipher_suite_to_ctype(security_key->cipher_type) << 11) |
                       ((osal_u16)security_key->key_origin << 15); // 左移 3 7 11 15
    ce_lut.bits.x6NgwKWtwodLwGFF__ = x6NgwKWtwodLwGFF__;

    /* 更新 LUT操作使能 */
    ce_lut.bits.x6NgwKWtwodLwiuW_wSiFW_ = 1; // 1: 01b: ce_key: 1; mic_key: 0
    hal_reg_write(HH503_MAC_LUT0_BANK_BASE_0x24, ce_lut.u32);

    /* 检查 LUT_EN Bit 以确认硬件写入成功 */
    if (hh503_is_ce_lut_done() != OSAL_TRUE) {
        oam_error_log0(0, OAM_SF_ANY, "{hh503_ce_get_key_process:: wait lut ce set done timeout!!!}\r\n");
        return;
    }

    /* 2.1 将密钥地址写入KEY_QTR */
    reg_value = hal_reg_read(HH503_MAC_LUT0_BANK_BASE_0x14);
    temp_key[0] = ((reg_value & 0xFF000000) >> 24); /* 右移24位 */
    temp_key[1] = ((reg_value & 0x00FF0000) >> 16); /* 右移16位 */
    temp_key[2] = ((reg_value & 0x0000FF00) >> 8);  /* 第2个数组元素，右移8位 */
    temp_key[3] = (reg_value & 0x000000FF); /* 第3个数组元素 */

    reg_value = hal_reg_read(HH503_MAC_LUT0_BANK_BASE_0x18);
    temp_key[4] = ((reg_value & 0xFF000000) >> 24); /* 第4个数组元素，右移24位 */
    temp_key[5] = ((reg_value & 0x00FF0000) >> 16); /* 第5个数组元素，右移16位 */
    temp_key[6] = ((reg_value & 0x0000FF00) >> 8);  /* 第6个数组元素，右移8位 */
    temp_key[7] = (reg_value & 0x000000FF); /* 第7个数组元素 */

    reg_value = hal_reg_read(HH503_MAC_LUT0_BANK_BASE_0x1C);
    temp_key[8] = ((reg_value & 0xFF000000) >> 24); /* 第8个数组元素，右移24位 */
    temp_key[9] = ((reg_value & 0x00FF0000) >> 16); /* 第9个数组元素，右移16位 */
    temp_key[10] = ((reg_value & 0x0000FF00) >> 8); /* 第10个数组元素，右移8位 */
    temp_key[11] = (reg_value & 0x000000FF); /* 第11个数组元素 */

    reg_value = hal_reg_read(HH503_MAC_LUT0_BANK_BASE_0x20);
    temp_key[12] = ((reg_value & 0xFF000000) >> 24); /* 第12个数组元素，右移24位 */
    temp_key[13] = ((reg_value & 0x00FF0000) >> 16); /* 第13个数组元素，右移16位 */
    temp_key[14] = ((reg_value & 0x0000FF00) >> 8);  /* 第14个数组元素，右移8位 */
    temp_key[15] = (reg_value & 0x000000FF); /* 第15个数组元素 */
    (osal_void)memcpy_s(cipher_key, WLAN_CIPHER_KEY_LEN, temp_key, WLAN_CIPHER_KEY_LEN); /* 长度16字节 */

    ce_lut.bits.x6NgwKWtwodLwiuW_wSiFW_ = 3; // 3: 11b: ce_key: 1; mic_key: 1
    hal_reg_write(HH503_MAC_LUT0_BANK_BASE_0x24, ce_lut.u32);
    /* 3 将TKIP MIC 密钥地址写入TKIP_MIC_KEY */
    if (mic_key != OSAL_NULL) {
        if (hh503_is_ce_lut_done() != OSAL_TRUE) {
            oam_error_log0(0, OAM_SF_ANY,
                "{hh503_ce_get_key_process:: wait lut ce set done timeout for mic key read !!!}\r\n");
            return;
        }

        reg_value = hal_reg_read(HH503_MAC_LUT0_BANK_BASE_0x20);
        temp_key[0] = (reg_value & 0x000000FF);
        temp_key[1] = ((reg_value & 0x0000FF00) >> 8);  /* 右移8位 */
        temp_key[2] = ((reg_value & 0x00FF0000) >> 16); /* 第2个数组元素，右移16位 */
        temp_key[3] = ((reg_value & 0xFF000000) >> 24); /* 第3个数组元素，右移24位 */

        reg_value = hal_reg_read(HH503_MAC_LUT0_BANK_BASE_0x1C);
        temp_key[4] = (reg_value & 0x000000FF); /* 第4个数组元素 */
        temp_key[5] = ((reg_value & 0x0000FF00) >> 8);  /* 第5个数组元素，右移8位 */
        temp_key[6] = ((reg_value & 0x00FF0000) >> 16); /* 第6个数组元素，右移16位 */
        temp_key[7] = ((reg_value & 0xFF000000) >> 24); /* 第7个数组元素，右移24位 */

        reg_value = hal_reg_read(HH503_MAC_LUT0_BANK_BASE_0x18);
        temp_key[8] = (reg_value & 0x000000FF); /* 第8个数组元素 */
        temp_key[9] = ((reg_value & 0x0000FF00) >> 8);  /* 第9个数组元素，右移8位 */
        temp_key[10] = ((reg_value & 0x00FF0000) >> 16); /* 第10个数组元素，右移16位 */
        temp_key[11] = ((reg_value & 0xFF000000) >> 24); /* 第11个数组元素，右移24位 */

        reg_value = hal_reg_read(HH503_MAC_LUT0_BANK_BASE_0x14);
        temp_key[12] = (reg_value & 0x000000FF); /* 第12个数组元素 */
        temp_key[13] = ((reg_value & 0x0000FF00) >> 8); /* 第13个数组元素，右移8位 */
        temp_key[14] = ((reg_value & 0x00FF0000) >> 16); /* 第14个数组元素，右移16位 */
        temp_key[15] = ((reg_value & 0xFF000000) >> 24); /* 第15个数组元素，右移24位 */
        (osal_void)memcpy_s(mic_key, WLAN_TEMPORAL_KEY_LENGTH, temp_key, WLAN_TEMPORAL_KEY_LENGTH); /* 长度16字节 */
    }
}

/*****************************************************************************
函 数 名  : hh503_ce_get_key
功能描述  : 读取指定的cipher key
*****************************************************************************/
osal_void hh503_ce_get_key(const hal_security_key_stru *security_key)
{
    osal_u8 *cipher_key = OSAL_NULL;
    osal_u8 *mic_key = OSAL_NULL;

    cipher_key = security_key->cipher_key;
    mic_key = security_key->mic_key;

    if (cipher_key == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{hh503_ce_get_key:: key is null}\r\n");
        return;
    }
    hh503_ce_get_key_process(cipher_key, mic_key, security_key);
}

/*****************************************************************************
 功能描述  : 使能ce功能
*****************************************************************************/
OSAL_STATIC oal_bool_enum_uint8 hh503_config_ce_control(osal_void)
{
    u_ce_control ce;

    ce.u32 = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x8);
    ce.bits.x6NgwLKBuwSB6wWh_ = 1;
    ce.bits.xCt2O4voROloCOGoOwmRwQ_ = 1;
    hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x8, ce.u32);
    return OSAL_TRUE;
}

/*****************************************************************************
功能描述  : 将指定的密钥写入硬件寄存器
*****************************************************************************/
osal_void hh503_ce_add_key(hal_security_key_stru *security_key)
{
    /* 关联过程中更新密钥不清除PN */
    if (security_key->update_key == OSAL_FALSE) {
        hh503_clear_rx_tx_pn(security_key);
    }
    /* 1. 使能加密 */
    if (hh503_enable_ce() != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_ANY, "{hh503_ce_add_key::config_pa_ce_control failed.}\r\n");
        return;
    }

    /* 2. 配置加密模块功能 */
    if (hh503_config_ce_control() != OSAL_TRUE) {
        oam_error_log0(0, OAM_SF_ANY, "{hh503_ce_add_key::config_ce_ce_control failed.}\r\n");
        return;
    }

    /* 4. 更新密钥 */
    if (hh503_ce_add_cipher_key(security_key) != OSAL_TRUE) {
        oam_error_log0(0, OAM_SF_ANY, "{hh503_ce_add_key::add_cipher_key failed.}\r\n");
        return;
    }
    if (hh503_ce_get_cipher_key(security_key) != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_ANY, "{hh503_ce_add_key::get_cipher_key failed.ul_ret =[%d]}\r\n");
        return;
    }
}

osal_void hh503_ce_del_key_process(const hal_security_key_stru *security_key)
{
    u_ce_lut_config ce_lut = {.u32 = 0};
    osal_u16 x6NgwKWtwodLwGFF__ = 0;

    if (hh503_is_ce_lut_done() != OSAL_TRUE) {
        wifi_printf("\n func[%s] line[%d] \n", __func__, __LINE__);
        return;
    }

    ce_lut.bits.xCt2OdmQOxSROvGmwOmo_ = 1;
    ce_lut.bits.xCt2OdmQOxSROvGmw_ = HH503_LUT_OPER_DEL;

    oam_warning_log2(0, OAM_SF_CRYPTO, "{hh503_ce_del_key_process::lut_idx=%d, reg_value=0x%08x.}",
        security_key->lut_idx, ce_lut.u32);

    /*
     *  0: peer_idx: 0- 2; bit16-18; 0000_0000_0000_0111b
     *  3: key_type: 3- 6; bit19-22; 0000_0000_0111_1000b
     *  7:   key_id: 7-10; bit23-26; 0000_0111_1000_0000b
     * 11: enc_type:11-14; bit27-30; 0111_1000_0000_0000b
     * 15: vap_type:15-15; bit31-31; 1000_0000_0000_0000b
     */
    x6NgwKWtwodLwGFF__ = ((osal_u16)security_key->lut_idx << 0) |
                       ((osal_u16)security_key->key_type << 3) |
                       ((osal_u16)security_key->key_id << 7) |
                       ((osal_u16)hal_cipher_suite_to_ctype(WLAN_80211_CIPHER_SUITE_TKIP) << 11) |
                       ((osal_u16)security_key->key_origin << 15); // 左移 3 7 11 15
    ce_lut.bits.x6NgwKWtwodLwGFF__ = x6NgwKWtwodLwGFF__;

    /* 更新 LUT操作模式 */
    ce_lut.bits.x6NgwKWtwodLwiuW_wSiFW_ = 1; // 1: 01b: ce_key: 1; mic_key: 0
    hal_reg_write(HH503_MAC_LUT0_BANK_BASE_0x24, ce_lut.u32);

    if (hh503_is_ce_lut_done() != OSAL_TRUE) {
        wifi_printf("\n func[%s] line[%d] \n", __func__, __LINE__);
        oam_error_log0(0, OAM_SF_RX, "{hh503_ce_del_key_process::wait lut ce setting done timeout.}\r\n");
    }
    return;
}

/*****************************************************************************
功能描述  : 将指定的密钥从硬件寄存器删除
*****************************************************************************/
osal_void hh503_ce_del_key(const hal_security_key_stru *security_key)
{
    u_ce_lut_config ce_lut = {.u32 = 0};
    osal_u16 x6NgwKWtwodLwGFF__ = 0;

    if (hh503_is_ce_lut_done() != OSAL_TRUE) {
        oam_error_log0(0, OAM_SF_TX, "{hh503_ce_del_key::wait lut ce idle failed.}");
        return;
    }

    /* 先将key清0 */
    /* 将0值写入key mickey */
    hal_reg_write(HH503_MAC_LUT0_BANK_BASE_0x14, 0x0);
    hal_reg_write(HH503_MAC_LUT0_BANK_BASE_0x18, 0x0);
    hal_reg_write(HH503_MAC_LUT0_BANK_BASE_0x1C, 0x0);
    hal_reg_write(HH503_MAC_LUT0_BANK_BASE_0x20, 0x0);
    /* 2.2 将密钥写入MAC内部LUT表 */

    /* 更新 LUT Operation 为写入 */
    ce_lut.bits.xCt2OdmQOxSROvGmwOmo_ = 1;
    ce_lut.bits.xCt2OdmQOxSROvGmw_ = HH503_LUT_OPER_WRITE;

    /*
     *  0: peer_idx: 0- 2; bit16-18; 0000_0000_0000_0111b
     *  3: key_type: 3- 6; bit19-22; 0000_0000_0111_1000b
     *  7:   key_id: 7-10; bit23-26; 0000_0111_1000_0000b
     * 11: enc_type:11-14; bit27-30; 0111_1000_0000_0000b
     * 15: vap_type:15-15; bit31-31; 1000_0000_0000_0000b
     */
    x6NgwKWtwodLwGFF__ = ((osal_u16)security_key->lut_idx << 0) |
                       ((osal_u16)security_key->key_type << 3) |
                       ((osal_u16)security_key->key_id << 7) |
                       ((osal_u16)hal_cipher_suite_to_ctype(WLAN_80211_CIPHER_SUITE_TKIP) << 11) |
                       ((osal_u16)security_key->key_origin << 15); // 左移 3 7 11 15

    /* 更新 LUT操作模式 */
    ce_lut.bits.x6NgwKWtwodLwiuW_wSiFW_ = 1; // 1: 01b: ce_key: 1; mic_key: 0
    hal_reg_write(HH503_MAC_LUT0_BANK_BASE_0x24, ce_lut.u32);

    /* 检查 LUT_EN Bit 以确认硬件写入成功 */
    if (hh503_is_ce_lut_done() != OSAL_TRUE) {
        oam_error_log0(0, OAM_SF_TX, "{hh503_ce_del_key::wait lut ce set done failed.}");
        return;
    }

    hal_reg_write(HH503_MAC_LUT0_BANK_BASE_0x14, 0x0);
    hal_reg_write(HH503_MAC_LUT0_BANK_BASE_0x18, 0x0);
    hal_reg_write(HH503_MAC_LUT0_BANK_BASE_0x1C, 0x0);
    hal_reg_write(HH503_MAC_LUT0_BANK_BASE_0x20, 0x0);
    ce_lut.bits.x6NgwKWtwodLwiuW_wSiFW_ = 3; // 3: 11b: ce_key: 1; mic_key: 1
    hal_reg_write(HH503_MAC_LUT0_BANK_BASE_0x24, ce_lut.u32);

    /* 检查 LUT_EN Bit 以确认硬件写入成功 */
    if (hh503_is_ce_lut_done() != OSAL_TRUE) {
        oam_error_log0(0, OAM_SF_TX, "{hh503_ce_del_key::wait lut ce set done failed.}");
        return;
    }

    /* 再删除key */
    /* 寄存器操作最多尝试两次 */
    hh503_ce_del_key_process(security_key);
}

/*****************************************************************************
功能描述  : 将指定的MAC 地址写入硬件寄存器
*****************************************************************************/
osal_void hh503_ce_add_peer_macaddr(osal_u8 lut_idx, const osal_u8 *addr)
{
    osal_u32 reg_value;
    u_peer_addr_lut_config peer_addr_lut_config = {.u32 = 0};
    if (addr == OSAL_NULL) {
        /* 空地址表面不需要配置，返回成功 */
        return;
    }

    /* 0 - 31 为合法,软件目前只是使用16个用户资源，按照WLAN_ASSOC_USER_MAX_NUM来即可 */
    if (lut_idx >= mac_chip_get_max_asoc_user(0)) {
        oam_error_log1(0, OAM_SF_ANY, "{hh503_ce_add_peer_macaddr::invalid lut idx [%d]!}\r\n", lut_idx);
        return;
    }

    /* 1. 等待Mac Address Lut空闲 */
    if (hh503_wait_mac_done_timeout(HH503_MAC_LUT0_BANK_BASE_0x8, HH503_MAC_CFG_PEER_ADDR_LUT_OPER_EN_MASK,
        HH503_MAC_CFG_PEER_ADDR_LUT_OPER_EN_MASK, HAL_CE_LUT_UPDATE_TIMEOUT) != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_ANY, "{hh503_ce_add_peer_macaddr::waiting lut ce idle timeout!}\r\n");
        return;
    }

    /* 2. 将mac地址写入PA_PEER_ADDRESS */
    reg_value = addr[1] | (addr[0] << 8); /* 左移8位 */
    hal_reg_write(HH503_MAC_LUT0_BANK_BASE_0x0, reg_value);
    reg_value = (addr[5] | ((addr[4] << 8) & 0xFF00) | /* 数组元素5、4、左移8位 */
        ((addr[3] << 16) & 0xFF0000) | /* 数组元素3、左移16位 */
        (((osal_u32)addr[2] << 24) & 0xFF000000)); /* 数组元素2，左移24位 */
    hal_reg_write(HH503_MAC_LUT0_BANK_BASE_0x4, reg_value);

    /* 3. 将地址写入MAC内部LUT表 */
    peer_addr_lut_config.bits.x6NgwuWW_wGFF_wodLwiuW__ = 1;
    peer_addr_lut_config.bits.x6NgwuWW_wGFF_wodLwBhFWP_ = lut_idx;
    peer_addr_lut_config.bits.x6NgwuWW_wGFF_wodLwiuW_wCWo_ = 0;
    peer_addr_lut_config.bits.x6NgwuWW_wGFF_wodLwiuW_wWh_ = 1;
    hal_reg_write(HH503_MAC_LUT0_BANK_BASE_0x8, peer_addr_lut_config.u32);

    /* 4. 等待Mac Address Lut空闲,以确认硬件写入成功 */
    if (hh503_wait_mac_done_timeout(HH503_MAC_LUT0_BANK_BASE_0x8, BIT0, BIT0,
        HAL_CE_LUT_UPDATE_TIMEOUT) != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_ANY, "{hh503_ce_add_peer_macaddr::waiting lut ce idle timeout!}\r\n");
    }

    return;
}

/*****************************************************************************
功能描述  : 将指定的MAC 地址从硬件Lut表删除
*****************************************************************************/
osal_void hh503_ce_del_peer_macaddr(osal_u8 lut_idx)
{
    u_peer_addr_lut_config peer_addr = {.u32 = 0};
    /* 在上一次操作结束条件下写入用户Mac Address */
    if (hh503_wait_mac_done_timeout(HH503_MAC_LUT0_BANK_BASE_0x8,
        HH503_MAC_CFG_PEER_ADDR_LUT_OPER_EN_MASK, HH503_MAC_CFG_PEER_ADDR_LUT_OPER_EN_MASK,
        HAL_CE_LUT_UPDATE_TIMEOUT) != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_ANY, "{hh503_ce_del_peer_macaddr::waiting lut idle timeout!}\r\n");
        return;
    }

    peer_addr.bits.x6NgwuWW_wGFF_wodLwiuW__ = HH503_LUT_OPER_DEL;
    peer_addr.bits.x6NgwuWW_wGFF_wodLwBhFWP_ = lut_idx;
    peer_addr.bits.x6NgwuWW_wGFF_wodLwiuW_wWh_ = OSAL_TRUE;
    peer_addr.bits.x6NgwuWW_wGFF_wodLwiuW_wCWo_ = 0;
    hal_reg_write(HH503_MAC_LUT0_BANK_BASE_0x8, peer_addr.u32);

    /* 检查 LUT_EN Bit 以确认硬件写入成功 */
    if (hh503_wait_mac_done_timeout(HH503_MAC_LUT0_BANK_BASE_0x8, BIT0, BIT0,
        HAL_CE_LUT_UPDATE_TIMEOUT) != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_ANY, "{hh503_ce_del_peer_macaddr::waiting lut done timeout!}\r\n");
    }

    return;
}


/*****************************************************************************
 功能描述  : 去使能ce功能
*****************************************************************************/
osal_void hh503_disable_ce(osal_void)
{
    u_ce_control ce_ctrl;

    ce_ctrl.u32 = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x8);
    ce_ctrl.bits.x6NgwFGLGwFW6_wWh_ = 0;
    ce_ctrl.bits.xCt2O4zRzOmoCwOmo_ = 0;
    ce_ctrl.bits.x6NgwuSNwWh6_wWh_ = 0;
    ce_ctrl.bits.x6NgwuSNwFW6wWh_ = 0;
    hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x8, ce_ctrl.u32);
}

osal_void hh503_set_replay_detect_en(osal_u8 enable)
{
    u_ce_control ce = {0};
 
    ce.u32 = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x8);
    ce.bits.xCt2OwmGxzQO4mRmCROmo_ = enable;
    hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x8, ce.u32);
}
 
osal_u8 hh503_get_replay_detect_en(osal_void)
{
    u_ce_control ce = {0};
 
    ce.u32 = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x8);
    return ce.bits.xCt2OwmGxzQO4mRmCROmo_;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
