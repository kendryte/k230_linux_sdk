/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: hmac_csi
 * Author:
 * Create: 2022-09-20
 */

#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "hmac_csi.h"
#include "hmac_ccpriv.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_HMAC_CSI_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
osal_u32 g_vap_id = INT_MAX;

/*****************************************************************************
  3 函数实现
*****************************************************************************/
OAL_STATIC osal_s32 hmac_config_rx_csi(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    osal_s32 ret;

    if ((hmac_vap->init_flag == MAC_VAP_INVAILD) || (msg->data == OAL_PTR_NULL)) {
        oam_warning_log2(0, OAM_SF_M2S,
            "{hmac_config_rx_csi::hmac_vap->init_flag[%d], param[%p]!}", hmac_vap->init_flag, (uintptr_t)msg->data);
        return OAL_ERR_CODE_PTR_NULL;
    }
    ret = frw_asyn_host_post_msg(WLAN_MSG_H2W_RX_CSI, FRW_POST_PRI_LOW, hmac_vap->vap_id, msg);
    if (ret != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_M2S,
            "{hmac_config_rx_csi::hmac_vap->vap_id[%d], param[%p]!}", hmac_vap->vap_id, (uintptr_t)msg->data);
        return ret;
    }

    return OAL_SUCC;
}

OAL_STATIC osal_s32 hmac_csi_set_reg_config(hmac_vap_stru *hmac_vap)
{
    hal_csi_set_tsf(hmac_vap->hal_vap->vap_id);

    /* 关闭csi ack帧默认上报通道 */
    hal_csi_set_ack_resp_flt();

    /* 使能csi时，打开mac和phy的PA         */
    hal_enable_machw_phy_and_pa(hmac_vap->hal_device);

    return OAL_SUCC;
}

OAL_STATIC osal_s32 hmac_config_csi_set_config(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    hal_csi_usr_attr user_attr;
    mac_csi_usr_config_stru *csi_config = OAL_PTR_NULL;
    osal_s32 ret;

    /* 不允许在多个vap_id下发csi相关配置，只能用同一个vap */
    if (g_vap_id != INT_MAX && hmac_vap->vap_id != g_vap_id) {
        oam_error_log1(0, OAM_SF_CFG, "vap_id[%d] {hmac_config_csi_set_config:VAPs cannot coexist.}",
            hmac_vap->vap_id);
        return OAL_ERR_CODE_CONFIG_UNSUPPORT;
    }
    g_vap_id = hmac_vap->vap_id;

    csi_config = (mac_csi_usr_config_stru *)msg->data;
    hal_csi_get_usr_attr(csi_config->user_idx, &user_attr);
    /* enable为0表示关闭对应user idx的csi信息上报,user全关闭，重置g_vap_id */
    if (csi_config->enable == 0) {
        user_attr.cfg_csi_en = 0;
        hal_csi_set_usr_attr(csi_config->user_idx, &user_attr);
        if (hal_csi_vap_is_close() == OAL_TRUE) {
            g_vap_id = INT_MAX;
        }
        return OAL_SUCC;
    }

    /* 剩下处理修改user idx的配置的情况，如果现在就是开启状态，则不允许修改 */
    if (user_attr.cfg_csi_en == 1) {
        oam_error_log1(0, OAM_SF_CFG, "user_idx[%d] {hmac_config_csi_set_config:user exists.}",
            csi_config->user_idx);
        return OAL_ERR_CODE_CONFIG_UNSUPPORT;
    }

    /* 寄存器相关配置设置 */
    ret = hmac_csi_set_reg_config(hmac_vap);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_CFG, "{hmac_config_csi_set_config::hmac_vap->vap_id[%d]!}", hmac_vap->vap_id);
        return ret;
    }

    /* 白名单总开关 */
    user_attr.cfg_csi_en = 1;
    /* MAC地址方向, 0 RA,1 TA */
    user_attr.cfg_match_ta_ra_sel = csi_config->cfg_match_ta_ra_sel;

    /* 帧类型过滤，管理 控制 数据 */
    memcpy_s(user_attr.usr_addr, WLAN_MAC_ADDR_LEN, csi_config->mac_addr, WLAN_MAC_ADDR_LEN);
    user_attr.cfg_mgmt_frame_en = ((csi_config->frame_type_filter_bitmap >> WLAN_MANAGEMENT) & 0x1);
    user_attr.cfg_ctrl_frame_en = ((csi_config->frame_type_filter_bitmap >> WLAN_CONTROL) & 0x1);
    user_attr.cfg_data_frame_en = ((csi_config->frame_type_filter_bitmap >> WLAN_DATA_BASICTYPE) & 0x1);
    /* 帧子类型过滤开关 */
    user_attr.frm_subtype_match_en = csi_config->sub_type_filter_enable;
    /* 帧子类型过滤类型 */
    user_attr.match_frame_subtype = csi_config->sub_type_filter;
    /* ppdu format类型 */
    user_attr.ppdu_non_ht_en = ((csi_config->ppdu_filter_bitmap  >> HAL_CSI_PPDU_NONE_HT) & 0x1);
    user_attr.ppdu_he_er_su_en = ((csi_config->ppdu_filter_bitmap  >> HAL_CSI_PPDU_HE_ER_SU) & 0x1);
    user_attr.ppdu_he_mu_mimo_en = ((csi_config->ppdu_filter_bitmap  >> HAL_CSI_PPDU_HE_MU_MIMO) & 0x1);
    user_attr.ppdu_he_mu_ofdma_en = ((csi_config->ppdu_filter_bitmap  >> HAL_CSI_PPDU_HE_MU_OFDMA) & 0x1);
    user_attr.ppdu_ht_en = ((csi_config->ppdu_filter_bitmap  >> HAL_CSI_PPDU_HT) & 0x1);
    user_attr.ppdu_vht_en = ((csi_config->ppdu_filter_bitmap  >> HAL_CSI_PPDU_VHT) & 0x1);
    /* period间隔时间 */
    user_attr.sample_period_ms = csi_config->period;

    wifi_printf("csi_set_config:frame type[%d %d %d] \r\n",
        user_attr.cfg_mgmt_frame_en, user_attr.cfg_ctrl_frame_en, user_attr.cfg_data_frame_en);

    hal_csi_set_usr_attr(csi_config->user_idx, &user_attr);
    return OAL_SUCC;
}

OAL_STATIC osal_s32 hmac_csi_set_param(const osal_s8 *param, mac_csi_usr_config_stru *csi_config)
{
    osal_u32 val;
    osal_s32 ret;

    /* 获取参数 白名单地址过滤类型 0 RA 1 TA */
    ret = hmac_ccpriv_get_digit_with_check_max(&param, 1, &val);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_ccpriv_csi_set_config::get addr_filter_type err[%d]!}", ret);
        return ret;
    }
    csi_config->cfg_match_ta_ra_sel = (osal_u8)val;

    /* 获取参数 user mac地址 */
    ret = (osal_s32)hmac_ccpriv_get_mac_addr_etc(&param, csi_config->mac_addr);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_ccpriv_csi_set_config::get mac err[%d]!}", ret);
        return ret;
    }

    /* 获取参数 user frame type,取值范围0~7,bit0管理帧 bit1控制帧 bit2数据帧 */
    ret = hmac_ccpriv_get_digit_with_check_max(&param, 7, &val);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_ccpriv_csi_set_config::get frame_type err[%d]!}", ret);
        return ret;
    }
    csi_config->frame_type_filter_bitmap = (osal_u8)val;

    /* 获取参数子帧过滤开关,取值0关闭,1打开 */
    ret = hmac_ccpriv_get_digit_with_check_max(&param, 1, &val);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_ccpriv_csi_set_config::get frame sub type enable err[%d]!}", ret);
        return ret;
    }
    csi_config->sub_type_filter_enable = (osal_u8)val;

    /* 获取参数 user frame sub type,取值范围0~15,由4个bit组成的子帧类型对应的十进制数,1111对应15 */
    ret = hmac_ccpriv_get_digit_with_check_max(&param, 15, &val);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_ccpriv_csi_set_config::get frame sub type filter err[%d]!}", ret);
        return ret;
    }
    csi_config->sub_type_filter = (osal_u8)val;

    /* 获取ppdu format过滤具体参数,取值范围0~63,共6位bit,详见结构体定义 */
    ret = hmac_ccpriv_get_digit_with_check_max(&param, 63, &val);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_ccpriv_csi_set_config::frame ppdu_filter_bitmap err[%d]!}", ret);
        return ret;
    }
    csi_config->ppdu_filter_bitmap = (osal_u8)val;

    /* 获取csi 上报时间间隔,取值范围0~4095（单位ms）,共12位bit */
    ret = hmac_ccpriv_get_digit_with_check_max(&param, 4095, &val);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_ccpriv_csi_set_config::get period err[%d]!}", ret);
        return ret;
    }
    csi_config->period = (osal_u16)val;

    return OAL_SUCC;
}

OAL_STATIC osal_s32 hmac_ccpriv_csi_set_config(hmac_vap_stru *hmac_vap, const osal_s8 *param)
{
    mac_csi_usr_config_stru csi_config;
    osal_u32 val;
    osal_s32 ret;
    frw_msg msg;

    (osal_void)memset_s(&csi_config, OAL_SIZEOF(csi_config), 0, OAL_SIZEOF(csi_config));
    (osal_void)memset_s(&msg, OAL_SIZEOF(msg), 0, OAL_SIZEOF(msg));

    /* 获取参数 user_idx,取值范围0~3,共4个用户 */
    ret = hmac_ccpriv_get_digit_with_check_max(&param, 3, &val);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_ccpriv_csi_set_config::get user_idx err[%d]!}", ret);
        return ret;
    }
    csi_config.user_idx = (osal_u8)val;

    /* 获取参数 user开关 0表示删除,1表示添加 */
    ret = hmac_ccpriv_get_digit_with_check_max(&param, 1, &val);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_ccpriv_csi_set_config::get enable err[%d]!}", ret);
        return ret;
    }
    csi_config.enable = (osal_u8)val;

    /* 只有当获取的enable参数为1时，才接着获取接下来的其他参数 */
    if (csi_config.enable == 1) {
        ret = hmac_csi_set_param(param, &csi_config);
        if (ret != OAL_SUCC) {
            oam_warning_log1(0, OAM_SF_ANY, "{hmac_ccpriv_csi_set_config::hmac_csi_set_param err[%d]!}", ret);
            return ret;
        }
    }

    msg.data = (osal_u8 *)&csi_config;
    ret = hmac_config_csi_set_config(hmac_vap, &msg);
    if (OAL_UNLIKELY(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_ccpriv_csi_set_config::return err code [%d]!}", ret);
    }
    return ret;
}

/* 获取用户0配置信息 echo "wlan0 csi_get_config 0"  > /sys/ccsys/ccpriv */
OAL_STATIC osal_s32 hmac_ccpriv_csi_get_config(hmac_vap_stru *hmac_vap, const osal_s8 *param)
{
    hal_csi_usr_attr user_attr;
    osal_s32 ret;
    osal_u8 user_id;

    unref_param(hmac_vap);

    (osal_void)memset_s(&user_attr, OAL_SIZEOF(user_attr), 0, OAL_SIZEOF(user_attr));

    /* 获取参数 user_idx,取值范围0~3,共4个用户 */
    ret = hmac_ccpriv_get_u8_with_check_max(&param, 3, &user_id);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_ccpriv_csi_get_config::get user_idx err[%d]!}", ret);
        return ret;
    }

    hal_csi_get_usr_attr(user_id, &user_attr);
    /* 如果该用户现在是关闭状态，则不进行打印 */
    if (user_attr.cfg_csi_en == 0) {
        return OAL_ERR_CODE_CONFIG_UNSUPPORT;
    }

    /* 打印所获取的用户配置信息 */
    wifi_printf("hmac_ccpriv_csi_get_config:usr_id: %d addr_filter_type: %d mac_addr[%02X:%02X:**:**:**:**] \r\n",
        user_id, user_attr.cfg_match_ta_ra_sel, user_attr.usr_addr[0], user_attr.usr_addr[1]);
    wifi_printf("hmac_ccpriv_csi_get_config:frame type[%d %d %d] sub_frame_enable[%d] sub_frame[%d]\r\n",
        user_attr.cfg_mgmt_frame_en, user_attr.cfg_ctrl_frame_en, user_attr.cfg_data_frame_en,
        user_attr.frm_subtype_match_en, user_attr.match_frame_subtype);
    wifi_printf("hmac_ccpriv_csi_get_config:ppdu[%d %d %d %d %d %d] period[%d]\r\n",
        user_attr.ppdu_non_ht_en, user_attr.ppdu_he_er_su_en, user_attr.ppdu_he_mu_mimo_en,
        user_attr.ppdu_he_mu_ofdma_en, user_attr.ppdu_ht_en, user_attr.ppdu_vht_en, user_attr.sample_period_ms);

    return OAL_SUCC;
}

/* 设置buffer个数和大小，默认值为2,762 */
OAL_STATIC osal_s32 hmac_ccpriv_csi_set_buffer(hmac_vap_stru *hmac_vap, const osal_s8 *param)
{
    hal_csi_buffer_stru csi_buffer;
    osal_s32 ret;
    osal_u32 val;
    osal_u32 buflen = 762; /* 762 默认buffer长度 */

    unref_param(hmac_vap);

    (osal_void)memset_s(&csi_buffer, OAL_SIZEOF(csi_buffer), 0, OAL_SIZEOF(csi_buffer));

    /* 获取参数 csi_data_blk_num,取值范围1~4,buffer个数 */
    ret = hmac_ccpriv_get_digit_with_check_max(&param, 4, &val);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_ccpriv_csi_set_buffer::get csi_data_blk_num err[%d]!}", ret);
        return ret;
    }
    csi_buffer.csi_data_blk_num = (osal_u8)val;

    /* 获取参数 csi_data_max_len,取值范围378~762,单个buffer长度 */
    ret = hmac_ccpriv_get_digit_with_check_max(&param, buflen, &val);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_ccpriv_csi_set_buffer::get csi_data_max_len err[%d]!}", ret);
        return ret;
    }
    csi_buffer.csi_data_max_len = (osal_u16)val;

    /* 判断buffer总长度是否超限，因为申请的是netbuf内存池，所以乘积不能超过netbuf最大的size */
    if (csi_buffer.csi_data_blk_num > WLAN_LARGE_NETBUF_SIZE / csi_buffer.csi_data_max_len) {
        oam_warning_log0(0, OAM_SF_ANY, "{hmac_ccpriv_csi_set_buffer::buffer exceeds the threshold!}");
        return OAL_ERR_CODE_CONFIG_EXCEED_SPEC;
    }

    ret = (osal_s32)hal_csi_set_buffer_config(&csi_buffer);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_CFG, "hmac_ccpriv_csi_set_buffer:set buffer failed! buffer len[%u] user is null.",
            csi_buffer.csi_data_max_len);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }
    return OAL_SUCC;
}

OAL_STATIC osal_void hmac_csi_exit(osal_void)
{
    osal_u8 index;

    /* 禁用所有用户,退出CSI，重置g_vap_id */
    for (index = 0; index < HAL_CSI_MAX_USER_NUM; index++) {
        hal_csi_disable(index);
    }
    g_vap_id = INT_MAX;
}

OAL_STATIC osal_s32 hmac_config_csi_switch(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    oal_bool_enum_uint8 csi_switch_on;

    unref_param(hmac_vap);

    csi_switch_on = (oal_bool_enum_uint8)*msg->data;
    if (csi_switch_on == OAL_TRUE) {
        hh503_csi_phy_open_channel();
    } else if (csi_switch_on == OAL_FALSE) {
        hmac_csi_exit();
    } else {
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    return OAL_SUCC;
}

OAL_STATIC osal_s32 hmac_ccpriv_csi_switch(hmac_vap_stru *hmac_vap, const osal_s8 *param)
{
    osal_u8 val;
    osal_s32 ret;
    oal_bool_enum_uint8 csi_switch_on;
    frw_msg msg;

    (osal_void)memset_s(&msg, OAL_SIZEOF(msg), 0, OAL_SIZEOF(msg));

    ret = hmac_ccpriv_get_u8_with_check_max(&param, 1, &val);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_ccpriv_csi_switch::get csi_data_blk_num err[%d]!}", ret);
        return ret;
    }
    csi_switch_on = val;

    msg.data = (osal_u8 *)&csi_switch_on;
    ret = hmac_config_csi_switch(hmac_vap, &msg);
    if (OAL_UNLIKELY(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_ccpriv_csi_switch::return err code [%d]!}", ret);
    }
    return ret;
}

osal_u32 hmac_csi_init(osal_void)
{
    /* ccpriv命令注册 */
    hmac_ccpriv_register((const osal_s8 *)"csi_set_config", hmac_ccpriv_csi_set_config);
    hmac_ccpriv_register((const osal_s8 *)"csi_get_config", hmac_ccpriv_csi_get_config);
    hmac_ccpriv_register((const osal_s8 *)"csi_set_buffer", hmac_ccpriv_csi_set_buffer);
    hmac_ccpriv_register((const osal_s8 *)"csi_switch", hmac_ccpriv_csi_switch);

    /* 消息注册 */
    frw_msg_hook_register(WLAN_MSG_W2H_C_CFG_SET_CSI_PARAM, hmac_config_csi_set_config);
    frw_msg_hook_register(WLAN_MSG_W2H_C_CFG_CSI_SWITCH, hmac_config_csi_switch);
    frw_msg_hook_register(WLAN_MSG_D2H_C_CFG_RX_CSI, hmac_config_rx_csi);

    /* 对外接口注册 */
    hmac_feature_hook_register(HMAC_FHOOK_CSI_EXIT, hmac_csi_exit);
    return OAL_SUCC;
}

osal_void hmac_csi_deinit(osal_void)
{
    /* ccpriv命令去注册 */
    hmac_ccpriv_unregister((const osal_s8 *)"csi_set_config");
    hmac_ccpriv_unregister((const osal_s8 *)"csi_get_config");
    hmac_ccpriv_unregister((const osal_s8 *)"csi_set_buffer");
    hmac_ccpriv_unregister((const osal_s8 *)"csi_switch");

    /* 消息去注册 */
    frw_msg_hook_unregister(WLAN_MSG_W2H_C_CFG_SET_CSI_PARAM);
    frw_msg_hook_unregister(WLAN_MSG_W2H_C_CFG_CSI_SWITCH);
    frw_msg_hook_unregister(WLAN_MSG_D2H_C_CFG_RX_CSI);

    /* 对外接口去注册 */
    hmac_feature_hook_unregister(HMAC_FHOOK_CSI_EXIT);
    return;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif /* #ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD */