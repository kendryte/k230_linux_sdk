/*
 * Copyright (c) CompanyNameMagicTag 2020-2023. All rights reserved.
 * Description: cali Data Processing.
 */

#if defined(_PRE_PRODUCT_ID_HOST)
#ifndef CONFIG_NO_SUPPORT_INI
#include "hmac_cali_mgmt.h"

#include "oam_struct.h"
#include "plat_cali.h"
#include "plat_pm_wlan.h"
#include "wlan_msg.h"
#include "diag_log_common.h"
#include "common_dft.h"
#include "hal_ext_if_device.h"
#include "hal_rf.h"
#if defined(CONFIG_MIDDLEWARE_SUPPORT_NV)
#include "nv.h"
#include "nv_common_cfg.h"
#endif
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
#include "soc_customize_wifi.h"
#endif
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_HMAC_CALI_MGMT_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST
osal_u8 *g_cali_data_buffer = OAL_PTR_NULL;
OSAL_STATIC osal_u8* hmac_get_cali_data_buffer_addr(osal_void)
{
    // 校准数据还没有上报 那么不能进行下发 获取数据为空 +++ 后续要从nv中获取校准数据
    if (g_cali_data_buffer == OAL_PTR_NULL) {
        oam_warning_log0(0, OAM_SF_CFG, "hmac_get_cali_data_buffer_addr::buffer not init!\n");
        return OAL_PTR_NULL;
    }
    return g_cali_data_buffer;
}
OSAL_STATIC osal_u32 hmac_init_cali_data_buffer_addr(osal_u16 buffer_len)
{
    osal_u16 buffer_mem_len = buffer_len;
    if (g_cali_data_buffer != OAL_PTR_NULL) {
        return OAL_SUCC;
    }
    if (buffer_len == 0) {
        oam_error_log0(0, OAM_SF_CFG, "hmac_init_cali_data_buffer_addr::buffer init len error!\n");
        return OAL_FAIL;
    }

    g_cali_data_buffer = (osal_u8 *)oal_mem_alloc(OAL_MEM_POOL_ID_LOCAL, buffer_mem_len, OAL_TRUE);
    if (g_cali_data_buffer == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_CFG, "hmac_init_cali_data_buffer_addr::buffer init fail!\n");
        return OAL_FAIL;
    }
    memset_s(g_cali_data_buffer, buffer_mem_len, 0, buffer_mem_len);
    return OAL_SUCC;
}

osal_void hmac_init_cali_data_buffer_exit(osal_void)
{
    if (g_cali_data_buffer != OSAL_NULL) {
        oal_mem_free(g_cali_data_buffer, OAL_TRUE);
        g_cali_data_buffer = OSAL_NULL;
    }
}
#if defined(CONFIG_MIDDLEWARE_SUPPORT_NV)
OSAL_STATIC osal_u8* hmac_get_send_cali_data_from_nv(osal_void)
{
    osal_u16 nv_cali_data_length = 0;
    osal_u32 nv_ret = OAL_FAIL;
    osal_u8 *data = OAL_PTR_NULL;

    if (hmac_init_cali_data_buffer_addr(WLAN_RF_FE_CALI_DATA_STRUCT_SIZE) == OAL_FAIL) {
        return OAL_PTR_NULL;
    }
    data = hmac_get_cali_data_buffer_addr();
    if (data == OAL_PTR_NULL) {
        return OAL_PTR_NULL;
    }
    nv_ret = uapi_nv_read(NV_ID_FE_CALI_DATA, WLAN_RF_FE_CALI_DATA_STRUCT_SIZE, &nv_cali_data_length, data);
    if (nv_ret == OAL_SUCC) {
        return data;
    }

    return OAL_PTR_NULL;
}
OSAL_STATIC osal_u32 hmac_save_cali_data_to_nv(osal_u8 *data, osal_u16 data_len)
{
    osal_u32 nv_ret = OAL_FAIL;

    if (data_len <= WLAN_RF_FE_CALI_DATA_STRUCT_SIZE) {
        nv_ret = uapi_nv_write(NV_ID_FE_CALI_DATA, data, data_len);
        if (nv_ret == OAL_SUCC) {
            return OAL_SUCC;
        }
    }
    return OAL_FAIL;
}
#endif
osal_u32 hmac_send_cali_data_etc(hmac_vap_stru *hmac_vap)
{
    frw_msg msg = {0};
    osal_u32 ret = OAL_SUCC;
    osal_u8 *buffer = OAL_PTR_NULL;
    hmac_cali_data_header_stru *cali_msg = OAL_PTR_NULL;
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    osal_s32 cali_data_mask = 0;

    if (hwifi_get_init_priv_value(WLAN_CFG_PRIV_CALI_DATA_MASK, &cali_data_mask) != OAL_SUCC ||
        (cali_data_mask & WLAN_RF_FE_CALI_DATA_MASK_DOWNLOAD) != WLAN_RF_FE_CALI_DATA_MASK_DOWNLOAD) {
        return OAL_SUCC;
    }
#endif
#if defined(CONFIG_MIDDLEWARE_SUPPORT_NV)
    buffer = hmac_get_send_cali_data_from_nv();
#else
    buffer = hmac_get_cali_data_buffer_addr();
#endif
    cali_msg = (hmac_cali_data_header_stru *)buffer;
    // 已经收到device上报的校准数据 可以进行数据下发
    if (cali_msg == OAL_PTR_NULL) {
        oam_warning_log0(0, OAM_SF_CFG, "hmac_send_cali_data_etc::cali data none!\n");
        return OAL_FAIL;
    }
#if defined(CONFIG_MIDDLEWARE_SUPPORT_NV)
    // NV 下申请的内存大小是 WLAN_RF_FE_CALI_DATA_LEN 检查下数据长度
    if ((cali_msg->cali_data_received == 0) || (cali_msg->cali_data_len == 0) ||
        (cali_msg->cali_data_len > WLAN_RF_FE_CALI_DATA_LEN)) {
#else
    if ((cali_msg->cali_data_received == 0) || (cali_msg->cali_data_len == 0)) {
#endif
        oam_error_log2(0, OAM_SF_CFG, "hmac_send_cali_data_etc::need calibrate!received:%u len:%u\n",
            cali_msg->cali_data_received, cali_msg->cali_data_len);
        ret = OAL_FAIL;
    }
    if (ret == OAL_SUCC) {
        // 只需要将校准的内容发送到device 用于管理的结构不需要
        msg.data = (osal_u8 *)(buffer + sizeof(hmac_cali_data_header_stru));
        msg.data_len = cali_msg->cali_data_len;
        ret = (osal_u32)frw_send_msg_to_device(hmac_vap->vap_id, WLAN_MSG_H2D_C_CFG_CALI_DATA_SYNC, &msg, OSAL_FALSE);
        if (ret == OAL_SUCC) {
            return OAL_SUCC;
        }
    }
    oam_error_log1(0, OAM_SF_CFG, "hmac_send_cali_data_etc:send cali data fail[%d]", ret);
#if defined(CONFIG_MIDDLEWARE_SUPPORT_NV)
    hmac_init_cali_data_buffer_exit();
#endif
    return OAL_FAIL;
}

osal_s32 hmac_save_cali_event_etc(osal_u8 *data, osal_u16 data_len)
{
    osal_s32 ret = OAL_SUCC;
    osal_u32 mem_offset = sizeof(hmac_cali_data_header_stru);
    hmac_cali_data_header_stru *data_msg = (hmac_cali_data_header_stru *)data;
    // 保存校准数据长度以及校准数据接收标志
#if defined(CONFIG_MIDDLEWARE_SUPPORT_NV)
    data_msg->cali_data_received = OAL_TRUE;
    ret = hmac_save_cali_data_to_nv(data, data_msg->cali_data_len + mem_offset);
#else
    osal_u8 *buffer = OAL_PTR_NULL;
    buffer = hmac_get_cali_data_buffer_addr();
    // 尚未初始化说明还没有接收到上报数据 这是首次接收校准数据 需要申请空间
    if (buffer == OAL_PTR_NULL) {
        ret = (osal_s32)hmac_init_cali_data_buffer_addr(data_msg->cali_data_len + mem_offset);
        buffer = hmac_get_cali_data_buffer_addr();
    }
    if (ret != OAL_SUCC) {
        wifi_printf("hmac_save_cali_event_etc::init fail, len:%u\r\n", data_msg->cali_data_len);
        return ret;
    }
    data_msg->cali_data_received = OAL_TRUE;
    ret = memcpy_s(buffer, data_msg->cali_data_len + mem_offset, data, data_msg->cali_data_len + mem_offset);
#endif
    wifi_printf("save cali data len:%u ret:%d.\r\n", data_len, ret);
    return ret;
}
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif  // CONFIG_NO_SUPPORT_INI
#endif
