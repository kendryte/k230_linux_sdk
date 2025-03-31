/*
 * Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
 * Description: thruput test file.
 */
#ifdef _PRE_WLAN_FEATURE_DAQ
#include "hmac_sample_daq.h"
#include "td_base.h"
#include "frw_hmac.h"
#include "hal_ext_if_device.h"
#include "hal_ext_if.h"
#include "hal_mac.h"
#include "hmac_config.h"
#include "dmac_ext_if_type_rom.h"
#include "common_error_code_rom.h"
#include "diag_sample_data.h"
#include "hmac_ccpriv.h"
#include "plat_firmware.h"
#include "hal_soc_reg.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_HMAC_SAMPLE_DAQ_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_DEVICE

#define HMAC_SAMPLE_DAQ_TIMEOUT 1000 /* 数采默认时间*ms */
#define HMAC_SAMPLE_DAQ_MAX_SIZE 8 /* ws53数采最大空间8k */

hal_to_dmac_device_mac_test_stru g_hal_mac_device;
osal_u32 g_pkt_sample_data_cnt = 256; /* 默认get 256次 */
static frw_timeout_stru g_hmac_diag_timer;
hal_to_dmac_device_mac_test_stru g_hal_mac_test_device;

diag_sample_sub_mode_stru g_sub_mode_list_test4[HMAC_SAMPLE_DAQ_TEST4_CNT] = {
    {0x80, 2, SOC_DIAG_MSG_ID_WIFI_SAMPLE_DBAC_INFO}, {0x81, 2, SOC_DIAG_MSG_ID_WIFI_SAMPLE_TX_FSM_INFO},
    {0x82, 4, SOC_DIAG_MSG_ID_WIFI_SAMPLE_TX_AMPDU0_INFO}, {0x83, 4, SOC_DIAG_MSG_ID_WIFI_SAMPLE_TX_AMPDU1_INFO},
    {0x84, 4, SOC_DIAG_MSG_ID_WIFI_SAMPLE_TX_AMPDU2_INFO}, {0x85, 4, SOC_DIAG_MSG_ID_WIFI_SAMPLE_TX_AMPDU3_INFO},
    {0x86, 4, SOC_DIAG_MSG_ID_WIFI_SAMPLE_RX_AMPDU0_INFO}, {0x87, 4, SOC_DIAG_MSG_ID_WIFI_SAMPLE_RX_AMPDU1_INFO},
    {0x88, 4, SOC_DIAG_MSG_ID_WIFI_SAMPLE_RX_AMPDU2_INFO}, {0x89, 4, SOC_DIAG_MSG_ID_WIFI_SAMPLE_RX_AMPDU3_INFO},
    {0x8A, 2, SOC_DIAG_MSG_ID_WIFI_SAMPLE_TX_ENCR_RC4_INFO}, {0x8B, 2, SOC_DIAG_MSG_ID_WIFI_SAMPLE_RX_DECR_RC4_INFO},
    {0x8C, 2, SOC_DIAG_MSG_ID_WIFI_SAMPLE_TX_ENCR_AES_INFO}, {0x8D, 2, SOC_DIAG_MSG_ID_WIFI_SAMPLE_RX_DECR_AES_INFO},
    {0x8E, 2, SOC_DIAG_MSG_ID_WIFI_SAMPLE_MULTI_BSS_INFO}, {0x8F, 2, SOC_DIAG_MSG_ID_WIFI_SAMPLE_HE_ROM_INFO},
    {0x90, 2, SOC_DIAG_MSG_ID_WIFI_SAMPLE_TRIGGER_NFRP_INFO}, {0x91, 2, SOC_DIAG_MSG_ID_WIFI_SAMPLE_COEX_INFO},
    {0x92, 2, SOC_DIAG_MSG_ID_WIFI_SAMPLE_P2P_INFO}, {0x93, 2, SOC_DIAG_MSG_ID_WIFI_SAMPLE_TX_BCN_INFO},
    {0x94, 2, SOC_DIAG_MSG_ID_WIFI_SAMPLE_TSF_VAP0_INFO}, {0x95, 2, SOC_DIAG_MSG_ID_WIFI_SAMPLE_TSF_VAP1_INFO},
    {0x96, 2, SOC_DIAG_MSG_ID_WIFI_SAMPLE_INTR_STATUS_INFO}, {0x97, 2, SOC_DIAG_MSG_ID_WIFI_SAMPLE_SOUNDING_INFO},
    {0x98, 2, SOC_DIAG_MSG_ID_WIFI_SAMPLE_CSI_INFO}, {0x99, 2, SOC_DIAG_MSG_ID_WIFI_SAMPLE_MU_EDCA_INFO},
    {0x9A, 2, SOC_DIAG_MSG_ID_WIFI_SAMPLE_TRIG_PADDING_INFO}, {0x9B, 2, SOC_DIAG_MSG_ID_WIFI_SAMPLE_RX_VECTOR_INFO},
    {0x9C, 2, SOC_DIAG_MSG_ID_WIFI_SAMPLE_ERR_INTR_PADDING_INFO},
    {0x9D, 2, SOC_DIAG_MSG_ID_WIFI_SAMPLE_ERR2_INTR_PADDING_INFO},
};

diag_sample_sub_mode_stru g_sub_mode_list_test8[HMAC_SAMPLE_DAQ_TEST8_CNT] = {
    {0xED, 4, SOC_DIAG_MSG_ID_WIFI_SAMPLE_SOUDING_CSI_INFO},
};

/*****************************************************************************
 功能描述  : wifi数采开启 （HSO下发命令）
*****************************************************************************/
OAL_STATIC osal_void hmac_sample_set_param(mac_cfg_mct_set_diag_stru *event_set_diag, diag_wlan_sample_cmd *sample_cmd,
    hal_to_dmac_device_mac_test_stru *hal_mac_test_device)
{
    event_set_diag->function_index = (sample_cmd->flag == 1) ? 0 : 12; /* 0: 数采初始化，12: 数采停止 */
    event_set_diag->value = (osal_s32)sample_cmd->sample_size;
    event_set_diag->mode = (osal_s32)sample_cmd->sample_type;

    if (event_set_diag->value >= 8) { /* 命令配置值8及以上 */
        g_pkt_sample_data_cnt = 32 * HMAC_SAMPLE_DAQ_CNT; /* 其他 对应get 32*1024次 */
    } else {
        /* 分别对应get 2k/4k/6k/8k/10k/12k/14k/16k次 */
        g_pkt_sample_data_cnt = 2 * HMAC_SAMPLE_DAQ_CNT * (event_set_diag->value + 1);
    }

    hal_set_mac_diag_param(0, event_set_diag, hal_mac_test_device);
}

OAL_STATIC osal_void hmac_sample_set_mode(mac_cfg_mct_set_diag_stru *event_set_diag, diag_wlan_sample_cmd *sample_cmd,
    hal_to_dmac_device_mac_test_stru *hal_mac_test_device)
{
    event_set_diag->function_index = (osal_s32)sample_cmd->mode;
    event_set_diag->value = (osal_s32)sample_cmd->sub_mode;

    if (event_set_diag->function_index != 1 && event_set_diag->function_index != 4 && /* 1、4: 分别对应数采维测模式1和4 */
        event_set_diag->function_index != 8) { /* 8: 数采维测模式8 */
        return;
    }

    hal_set_mac_diag_mode(event_set_diag, hal_mac_test_device);
}

OAL_STATIC osal_void hmac_report_sample_data(osal_u32 sample_type_cnt, osal_u32 *index, osal_u32 msg_id)
{
    osal_u32 i;
    osal_u32 *buf = OSAL_NULL;
    hal_to_dmac_device_mac_test_stru *hal_mac_test_device = &g_hal_mac_device;
    osal_u32 data;
    osal_u32 sample_msg_id = msg_id;

    buf = (osal_u32 *)osal_kmalloc(HMAC_SAMPLE_DAQ_BUF_LEN, 0);
    if (buf == OSAL_NULL) {
        oam_warning_log0(0, OAM_SF_CFG, "hmac_report_sample_data, oal_mem_alloc failed.");
        return;
    }

    if (sample_type_cnt == HMAC_SAMPLE_DAQ_APB_CNT) {
        sample_msg_id = SOC_DIAG_MSG_ID_WIFI_SAMPLE_MAC_APB_INFO;
        *index += HMAC_SAMPLE_DAQ_APB_CNT - 1;
    } else if (sample_type_cnt == HMAC_SAMPLE_DAQ_RSSI_CNT) {
        sample_msg_id = SOC_DIAG_MSG_ID_WIFI_SAMPLE_RSSI_INFO;
    } else if (sample_type_cnt == HMAC_SAMPLE_DAQ_TX_RX_CNT) {
        *index += HMAC_SAMPLE_DAQ_TX_RX_CNT - 1;
    } else {
        /* 维测4和维测8 */
        *index += sample_type_cnt - 1;
    }

    for (i = 0; i < sample_type_cnt; i++) {
        data = hal_sample_daq_get_data(hal_mac_test_device->diag_param.diag_read_addr);

        /* 判断子特性tx、rx */
        if (i == 1 && sample_type_cnt == HMAC_SAMPLE_DAQ_TX_RX_CNT) {
            /* 取bit0的数据 */
            sample_msg_id = ((data & 0x1) == 0) ? SOC_DIAG_MSG_ID_WIFI_SAMPLE_MAC_TX_INFO
                : SOC_DIAG_MSG_ID_WIFI_SAMPLE_MAC_RX_INFO;
        }
        if (memcpy_s(buf + HMAC_SAMPLE_DAQ_LEN * i, HMAC_SAMPLE_DAQ_LEN, &data,
            HMAC_SAMPLE_DAQ_LEN) != EOK) {
            oam_error_log0(0, OAM_SF_CFG, "{hmac_report_sample_data memcpy_s: failed!}");
            osal_kfree(buf);
            return;
        }

        if (hal_mac_test_device->diag_param.diag_read_addr >= hal_mac_test_device->diag_param.diag_basic_end_addr) {
            break;
        }

        hal_mac_test_device->diag_param.diag_read_addr = hal_mac_test_device->diag_param.diag_read_addr + 0x4;
    }

    /* 上报数采结果到HSO */
    if (hal_mac_test_device->diag_param.diag_read_addr < hal_mac_test_device->diag_param.diag_basic_end_addr) {
        diag_report_wlan_sample_data((TD_CONST td_u8 *)buf, HMAC_SAMPLE_DAQ_LEN * sample_type_cnt, sample_msg_id);
    }

    if (buf != OSAL_NULL) {
        osal_kfree(buf);
    }
}

OAL_STATIC osal_u32 hmac_sample_wlan_get_data_diag1_test(hal_to_dmac_device_mac_test_stru *hal_mac_test_device)
{
    osal_u32 i;
    osal_u32 data;
    osal_u32 msg_id = 0;

    for (i = 0; i < g_pkt_sample_data_cnt; i++) {
        data = hal_sample_daq_get_data(hal_mac_test_device->diag_param.diag_read_addr);
        switch (((data >> 24) & 0xff)) { /* 24：右移24位 */
            case TX_RX_SAMPLE_ADDR:
                hmac_report_sample_data(HMAC_SAMPLE_DAQ_TX_RX_CNT, &i, msg_id); // 报文上报HSO
                break;
            case APB_SAMPLE_ADDR:
                hmac_report_sample_data(HMAC_SAMPLE_DAQ_APB_CNT, &i, msg_id); // 报文上报HSO
                break;
            case RSSI_SAMPLE_ADDR0:
            case RSSI_SAMPLE_ADDR1:
            case RSSI_SAMPLE_ADDR2:
            case RSSI_SAMPLE_ADDR3:
                hmac_report_sample_data(HMAC_SAMPLE_DAQ_RSSI_CNT, &i, msg_id); // 报文上报HSO
                break;
            default:
                hal_mac_test_device->diag_param.diag_read_addr = hal_mac_test_device->diag_param.diag_read_addr + 0x4;
                break;
        }
    }

    diag_report_wlan_sample_data(OSAL_NULL, 0, 0); /* 0: 数采上报结束信号 */
    return OAL_SUCC;
}
OAL_STATIC osal_void hmac_report_sample_data_test(diag_sample_sub_mode_stru sub_mode_list[], osal_u32 list_len,
    osal_u32 sample_addr, osal_u32 *index)
{
    osal_u32 i;
    osal_u32 sample_type_cnt = 0;
    osal_u32 msg_id = 0;

    hal_to_dmac_device_mac_test_stru *hal_mac_test_device = &g_hal_mac_device;

    for (i = 0; i < list_len; i++) {
        if (sub_mode_list[i].sub_mode_flag == sample_addr) {
            sample_type_cnt = sub_mode_list[i].sub_mode_type_cnt;
            msg_id = sub_mode_list[i].msg_id;
            hmac_report_sample_data(sample_type_cnt, index, msg_id);
            return;
        }
    }
    hal_mac_test_device->diag_param.diag_read_addr = hal_mac_test_device->diag_param.diag_read_addr + 0x4;
}

OAL_STATIC osal_void hmac_sample_wlan_get_data_diag_test(hal_to_dmac_device_mac_test_stru *hal_mac_test_device,
    diag_sample_sub_mode_stru sub_mode_list[], osal_u32 list_len)
{
    osal_u32 i;
    osal_u32 data;

    for (i = 0; i < g_pkt_sample_data_cnt; i++) {
        data = hal_sample_daq_get_data(hal_mac_test_device->diag_param.diag_read_addr);
        /* 获取维测4和维测8子特性的msg id */
        hmac_report_sample_data_test(sub_mode_list, list_len, (data >> 24) & 0xff, &i); /* 24：右移24位 */
    }

    diag_report_wlan_sample_data(OSAL_NULL, 0, 0); /* 0: 数采上报结束信号 */
}

OAL_STATIC osal_u32 hmac_sample_wlan_get_data(osal_void)
{
    hal_to_dmac_device_mac_test_stru *hal_mac_test_device = &g_hal_mac_device;

    switch (hal_mac_test_device->diag_param.diag_mode) {
        case HAL_MAC_DIAG_TEST1: // 1对应维测1
            hmac_sample_wlan_get_data_diag1_test(hal_mac_test_device);
            break;
        case HAL_MAC_DIAG_TEST4: // 4对应维测4
            hmac_sample_wlan_get_data_diag_test(hal_mac_test_device, g_sub_mode_list_test4, HMAC_SAMPLE_DAQ_TEST4_CNT);
            break;
        case HAL_MAC_DIAG_TEST8: // 8对应维测8
            hmac_sample_wlan_get_data_diag_test(hal_mac_test_device, g_sub_mode_list_test8, HMAC_SAMPLE_DAQ_TEST8_CNT);
            break;
        default:
            break;
    }

    return OAL_SUCC;
}

OAL_STATIC osal_u32 hmac_sample_soc_get_data(osal_void)
{
    hal_to_dmac_device_mac_test_stru *hal_mac_test_device = &g_hal_mac_device;
    osal_u8 *buf = OSAL_NULL;
    osal_u32 value = 0;
    osal_u32 offset = 0;
    osal_u32 i;

    buf = (osal_u8 *)osal_kmalloc(HMAC_SAMPLE_DAQ_BUF_LEN, 0);
    if (buf == OSAL_NULL) {
        oam_warning_log0(0, OAM_SF_CFG, "hmac_sample_soc_get_data, osal_kmalloc failed.");
        return OAL_PTR_NULL;
    }

    for (i = 0; i < g_pkt_sample_data_cnt; i++) {
        value = hal_sample_daq_get_data(hal_mac_test_device->diag_param.diag_read_addr);
        hal_mac_test_device->diag_param.diag_read_addr += HMAC_SAMPLE_DAQ_LEN;
        offset = (i * HMAC_SAMPLE_DAQ_LEN) % HMAC_SAMPLE_DAQ_BUF_LEN;

        if (memcpy_s(buf + offset, HMAC_SAMPLE_DAQ_BUF_LEN - offset, &value, HMAC_SAMPLE_DAQ_LEN) != EOK) {
            oam_error_log0(0, OAM_SF_CFG, "{hmac_report_soc_sample_data memcpy_s: failed!}");
            break;
        }

        /* 装满512字节, 发送一次 */
        if (offset + HMAC_SAMPLE_DAQ_LEN == HMAC_SAMPLE_DAQ_BUF_LEN) {
            diag_cmd_report_sample_data(buf, HMAC_SAMPLE_DAQ_BUF_LEN);
        }
    }
    osal_kfree(buf);
    diag_report_wlan_sample_data(OSAL_NULL, 0, 0); /* 0: 数采上报结束信号 */
    return OAL_SUCC;
}

OAL_STATIC osal_u32 hmac_sample_get_data(const hal_to_dmac_device_mac_test_stru *hal_mac_test_device)
{
    if (hal_mac_test_device->diag_param.diag_source == HMAC_SAMPLE_SOURCE_WLAN) {
        return hmac_sample_wlan_get_data();
    } else if (hal_mac_test_device->diag_param.diag_source == HMAC_SAMPLE_SOURCE_SOC) {
        return hmac_sample_soc_get_data();
    }
    return OAL_FAIL;
}

OAL_STATIC osal_void hmac_sample_stop_period_task(osal_void)
{
    if (g_hmac_diag_timer.is_registerd == OSAL_TRUE) {
        frw_destroy_timer_entry(&g_hmac_diag_timer);
    }
}

OAL_STATIC osal_u32 hmac_sample_query(osal_void *arg)
{
    osal_u32 ret;
    hal_to_dmac_device_mac_test_stru *hal_mac_test_device = OSAL_NULL;

    if (arg == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_CFG, "{hmac_sample_query::p_arg null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    hal_mac_test_device = (hal_to_dmac_device_mac_test_stru *)arg;

    /* 查询数采进度 */
    hal_set_mac_diag_test_query(hal_mac_test_device);

    if (hal_sample_daq_done() == OSAL_TRUE) {
        hal_sample_daq_prepare_data();
        hal_show_mac_daq_reg_cfg_info(); /* HSO输出mac数采相关寄存器配置信息 */
        ret = hmac_sample_get_data((const hal_to_dmac_device_mac_test_stru *)hal_mac_test_device);
        if (ret != OAL_SUCC) {
            oam_error_log0(0, OAM_SF_CFG, "{hmac_get_sample_data::fail }");
        }
        hal_show_daq_done_reg_info(); /* HSO输出mac/soc数采结束时，相关寄存器信息 */
        hmac_sample_stop_period_task(); /* 删除定时器 */
        hal_pkt_ram_sample_deinit();
    }
    return OAL_SUCC;
}

OAL_STATIC osal_void hmac_sample_start_period_task(hal_to_dmac_device_mac_test_stru *hal_mac_test_device)
{
    /* 启动周期检测维测定时器 */
    if (g_hmac_diag_timer.is_registerd == OSAL_TRUE) {
        frw_destroy_timer_entry(&g_hmac_diag_timer);
    }
    /* 启动定时器,查询数采结果 */
    frw_create_timer_entry(&g_hmac_diag_timer, hmac_sample_query, HMAC_SAMPLE_DAQ_TIMEOUT,
        (osal_void *)hal_mac_test_device, OAL_TRUE);
}

osal_u32 hmac_sample_wlan_data_enable(diag_wlan_sample_cmd *sample_cmd, osal_u32 len)
{
    mac_cfg_mct_set_diag_stru event_set_diag = {0};
    hal_to_dmac_device_mac_test_stru *hal_mac_test_device = &g_hal_mac_device;
    unref_param(len);
    if (sample_cmd == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_CFG, "{hmac_sample_data_enable::sample_cmd null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 设置param参数 */
    hal_mac_test_device->diag_param.diag_source = HMAC_SAMPLE_SOURCE_WLAN; // wlan
    hmac_sample_set_param(&event_set_diag, sample_cmd, hal_mac_test_device);

    /* 设置mode参数 */
    hmac_sample_set_mode(&event_set_diag, sample_cmd, hal_mac_test_device);

    /* 启动定时器,查询数采结果 */
    hmac_sample_start_period_task(hal_mac_test_device);

    return OAL_SUCC;
}

/* soc=> coex数采 */
osal_u32 hmac_sample_soc_data_enable(diag_wlan_sample_cmd *sample_cmd, osal_u32 len)
{
    mac_cfg_mct_set_diag_stru event_set_diag = {0};
    hal_to_dmac_device_mac_test_stru *hal_mac_test_device = &g_hal_mac_device;
    unref_param(len);
    if (sample_cmd == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_CFG, "{hmac_sample_data_enable::sample_cmd null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 设置param参数 */
    hal_mac_test_device->diag_param.diag_source = HMAC_SAMPLE_SOURCE_SOC; // soc => coex 数采
    hmac_sample_set_param(&event_set_diag, sample_cmd, hal_mac_test_device);

    /* 启动定时器,查询数采结果 */
    hmac_sample_start_period_task(hal_mac_test_device);

    return OAL_SUCC;
}

osal_u32 hmac_sample_daq_init(osal_void)
{
    /* 消息注册 */
    return OAL_SUCC;
}

osal_void hmac_sample_daq_deinit(osal_void)
{
    /* 消息去注册 */
    return;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif