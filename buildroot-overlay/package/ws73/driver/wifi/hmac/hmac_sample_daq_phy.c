/*
 * Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
 * Description: thruput test file.
 */
#ifdef _PRE_WLAN_FEATURE_DAQ
#include "hmac_sample_daq_phy.h"
#include "fe_hal_phy_daq.h"
#include "common_error_code_rom.h"
#include "diag_sample_data.h"
#include "hal_soc_reg.h"
#include "dmac_common_inc_rom.h"
#include "mac_vap_ext.h"
#include "frw_msg_rom.h"
#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include "plat_firmware.h"
#endif
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_HMAC_SAMPLE_DAQ_PHY_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

#define HMAC_SAMPLE_PHY_DAQ_TIMEOUT 1000 /* 数采默认时间*ms */
#define HMAC_SAMPLE_PHY_DAQ_SIZE_UNIT 512 /* 数采HSO下发长度的单位 也作为每次取数据的大小 */
#define HMAC_SAMPLE_DAQ_TRIGGER_DEFAULT 0xFF /* 数采HSO下发子模式默认值 */
#define HMAC_PHY_EVENT_NODE_SEL    19      /* phy事件上报节点数 */
#define HMAC_PHY_EVENT_SIZE_UNIT   1024    /* phy事件上报每次上报数据大小 */
#define HMAC_PHY_EVENT_TIMES       1024    /* 中断上报1024次 */

hal_to_dmac_device_mac_test_stru g_hal_mac_device_test;
static frw_timeout_stru g_hmac_sample_phy_timer;
OSAL_STATIC osal_u32 hmac_sample_wlan_phy_get_data(osal_void);

OSAL_STATIC osal_void hmac_sample_phy_stop_period_task(osal_void)
{
    if (g_hmac_sample_phy_timer.is_registerd == OSAL_TRUE) {
        frw_destroy_timer_entry(&g_hmac_sample_phy_timer);
    }
}

OSAL_STATIC osal_u32 hmac_sample_phy_query(osal_void *arg)
{
    unref_param(arg);
    /* 查询数采进度 */
    if (hmac_config_chip_test_set_diag_param_phy_query()) {
        if (hmac_sample_wlan_phy_get_data() != OAL_SUCC) {
            oam_error_log0(0, OAM_SF_CFG, "{hmac_get_sample_data::fail }");
        }
        hmac_sample_phy_stop_period_task(); /* 删除定时器 */
        hal_sample_free_msg();
        hmac_config_chip_test_set_diag_param_phy_recovery();    // 清理配置
    }
    return OAL_SUCC;
}

OSAL_STATIC osal_void hmac_sample_phy_start_period_task(hal_to_dmac_device_mac_test_stru *hal_mac_test_device)
{
    unref_param(hal_mac_test_device);
    /* 启动周期检测维测定时器 */
    if (g_hmac_sample_phy_timer.is_registerd == OSAL_TRUE) {
        frw_destroy_timer_entry(&g_hmac_sample_phy_timer);
    }
    /* 启动定时器,查询数采结果 */
    frw_create_timer_entry(&g_hmac_sample_phy_timer, hmac_sample_phy_query, HMAC_SAMPLE_PHY_DAQ_TIMEOUT,
        OSAL_NULL, OAL_TRUE);
}

/* ============== PHY 数采接口 起始 ============ */
void hmac_config_chip_test_set_diag_param_phy_recovery(void)
{
    hal_to_dmac_device_mac_test_stru *hal_mac_test_device = &g_hal_mac_device_test;
    wifi_printf("{hmac_config_chip_test_set_diag_param_phy_recovery enter, diagmode:%d}\r\n",
        hal_mac_test_device->diag_param.diag_mode);
    // 触发模式需要配置PHY寄存器
    hal_config_chip_test_set_diag_param_phy_recovery(hal_mac_test_device);
}

void hmac_config_chip_test_set_diag_param_phy_begin(hal_to_phy_test_diag_stru *param)
{
    hal_to_dmac_device_mac_test_stru *hal_mac_test_device = &g_hal_mac_device_test;

    hal_config_chip_test_set_diag_param_phy_begin(param, hal_mac_test_device);
    wifi_printf("{diag_param node:%u, len:%u, start addr:0x%x end addr:0x%x event_rpt_addr: 0x%x.}\r\n",
        param->cfg_sample_node_sel, param->cfg_sample_length, hal_mac_test_device->diag_param.diag_basic_start_addr,
        hal_mac_test_device->diag_param.diag_basic_end_addr, param->event_rpt_addr);
    if (param->cfg_sample_mode != 0) {
        wifi_printf("{diag_param debug by len:%u, start mode:%u, end mode:%u len:%u tsf:%u mac:%u.}\r\n",
            param->cfg_debug_sample_len, param->cfg_debug_sample_start_mode, param->cfg_debug_sample_end_mode,
            param->cfg_debug_sample_delay, param->cfg_with_mac_tsf_en, param->cfg_with_mac_info_en);
    }
}

oal_bool_enum_uint8 hmac_config_chip_test_set_diag_param_phy_query(void)
{
    osal_u32 sample_done;
    hal_to_dmac_device_mac_test_stru *hal_mac_test_device = &g_hal_mac_device_test;

    sample_done = hal_config_chip_test_set_diag_param_phy_query(hal_mac_test_device);
    if (sample_done == OSAL_TRUE) {
        wifi_printf("hmac_config_chip_test_set_diag_param_phy_query sample is done, read from:0x%x, end:0x%x!\n",
            hal_mac_test_device->diag_param.diag_read_addr, hal_mac_test_device->diag_param.diag_sample_addr);
    } else {
        wifi_printf("hmac_config_chip_test_set_diag_param_phy_query sample is doing, cur addr:0x%x!\n",
            hal_mac_test_device->diag_param.diag_sample_addr);
    }
    return sample_done;
}

/*
 * 函 数 名   : hmac_complete_phy_event_rpt
 * 功能描述   : PHY事件上报完成，分析事件信息
 */
osal_s32 hmac_complete_phy_event_rpt(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    osal_u8 *buf = OSAL_NULL;
    static osal_u32 diag_cnt = 0;
    static osal_u32 event_rpt_cnt = HMAC_PHY_EVENT_TIMES - 1; /* 中断上报次数-1 */
    hal_to_dmac_device_mac_test_stru *hal_mac_test_device = &g_hal_mac_device_test;

    unref_param(hmac_vap);

    if (msg == OSAL_NULL || msg->data == OSAL_NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{hmac_complete_phy_event_rpt::msg is null }");
        return OAL_FAIL;
    }

    buf = (osal_u8 *)msg->data;
    if (diag_cnt < hal_mac_test_device->diag_param.diag_phy_event_cnt) {
        diag_cmd_report_sample_data(buf, HMAC_PHY_EVENT_SIZE_UNIT);
        diag_cnt++;
    }

    wifi_printf("{hmac_complete_phy_event_rpt event_rpt_cnt=[%d] diag_cnt=[%d].}\r\n", event_rpt_cnt, diag_cnt);
    if ((event_rpt_cnt == 0) && (diag_cnt >= hal_mac_test_device->diag_param.diag_phy_event_cnt)) {
        diag_report_wlan_sample_data(OSAL_NULL, 0, 0); /* 0: 事件上报结束信号 */
        hmac_config_chip_test_set_diag_param_phy_recovery(); /* 清理配置 */
        hal_config_diag_int_switch(OAL_FALSE);
        diag_cnt = 0;
        event_rpt_cnt = HMAC_PHY_EVENT_TIMES - 1;
        return OAL_SUCC;
    }

    if (diag_cnt >= hal_mac_test_device->diag_param.diag_phy_event_cnt) {
        hal_config_diag_int_switch(OAL_TRUE);
        event_rpt_cnt--;
        diag_cnt = 0;
    }

    return OAL_SUCC;
}

// hso 数采 启动接口
osal_u32 hmac_sample_wlan_phy_data_enable(diag_wlan_phy_sample_cmd *sample_cmd, osal_u32 len)
{
    hal_to_dmac_device_mac_test_stru *hal_mac_test_device = &g_hal_mac_device_test;
    hal_to_phy_test_diag_stru phy_diag_info = {0};
    unref_param(len);
    if (sample_cmd == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_CFG, "{hmac_sample_data_enable::sample_cmd null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    wifi_printf("{hmac_sample_wlan_phy_data_enable diag_cmd flag:%u, node:%u, sample_size:%u sample_type:%u "
                "trigger_start:%u trigger_end:%u.}\r\n",
        sample_cmd->flag, sample_cmd->node, sample_cmd->sample_size, sample_cmd->sample_type,
        sample_cmd->trigger_start, sample_cmd->trigger_end);
    /* 设置param参数 */
    hal_mac_test_device->diag_param.diag_source = HMAC_SAMPLE_SOURCE_WLAN_PHY; // soc => coex 数采
    // 关闭数采 如果不能自动停止 则通过接口下发关闭 来进行恢复
    if (sample_cmd->flag == 0) {
        hmac_config_chip_test_set_diag_param_phy_recovery();
        if (g_hmac_sample_phy_timer.is_registerd == OSAL_TRUE) {
            frw_destroy_timer_entry(&g_hmac_sample_phy_timer);
        }
        hal_sample_free_msg();
        return OAL_SUCC;
    }
    // 开启数采
    // cfg_sample_node_sel 选择PHY内部节点 参考节点信号说明
    phy_diag_info.cfg_sample_node_sel = sample_cmd->node;
    // cfg_sample_length 配置数采长度 单位: 4Byte size 2k 下发 2 对应长度 2k/4=512
    phy_diag_info.cfg_sample_length = (sample_cmd->sample_size >> 1) * HMAC_SAMPLE_PHY_DAQ_SIZE_UNIT;
    // cfg_sample_mode 数采模式 0: 定长 1: 触发即刻停止 2: 触发延时停止 3: 触发定长
    phy_diag_info.cfg_sample_mode = (sample_cmd->sample_type != 0) ? HAL_TEST_DIAG_PHY_SAMPLE_MODE_DEBUG_WITHOUT_DELAY :
        (sample_cmd->trigger_start != HMAC_SAMPLE_DAQ_TRIGGER_DEFAULT) ? HAL_TEST_DIAG_PHY_SAMPLE_MODE_DEBUG_FIX_LEN :
        HAL_TEST_DIAG_PHY_SAMPLE_MODE_FIX_LEN;
    /* phy事件上报开关和次数 */
    phy_diag_info.event_rpt_addr = sample_cmd->event_rpt_addr;
    hal_mac_test_device->diag_param.diag_phy_event_cnt = sample_cmd->sample_size;
    // 触发模式刷新触发信号
    if (phy_diag_info.cfg_sample_mode != HAL_TEST_DIAG_PHY_SAMPLE_MODE_FIX_LEN) {
        phy_diag_info.cfg_debug_sample_start_mode = sample_cmd->trigger_start;
        phy_diag_info.cfg_debug_sample_end_mode = sample_cmd->trigger_end;
    }
    // 循环定长
    if (phy_diag_info.cfg_sample_mode == HAL_TEST_DIAG_PHY_SAMPLE_MODE_DEBUG_FIX_LEN) {
        phy_diag_info.cfg_debug_sample_len = OSAL_TRUE;
    }
    if (hal_sample_alloc_msg((osal_u16)(sample_cmd->sample_size << 1) * HMAC_SAMPLE_PHY_DAQ_SIZE_UNIT) == OAL_FAIL) {
        oam_error_log0(0, OAM_SF_CFG, "hmac_sample_wlan_phy_data_enable::osal_kmalloc pkt_ram failed.");
        return OAL_FAIL;
    }
    hmac_config_chip_test_set_diag_param_phy_begin(&phy_diag_info);
    if ((phy_diag_info.cfg_sample_node_sel == HMAC_PHY_EVENT_NODE_SEL) &&
        ((phy_diag_info.event_rpt_addr & 0x1) == 0x1)) {
        oam_warning_log0(0, OAM_SF_CFG, "hmac_sample_wlan_phy_data_enable::phy event enable.");
        return OAL_SUCC;
    }
    /* 启动定时器,查询数采结果 */
    hmac_sample_phy_start_period_task(hal_mac_test_device);
    return OAL_SUCC;
}

OSAL_STATIC osal_u32 hmac_sample_wlan_phy_get_data(osal_void)
{
    hal_to_dmac_device_mac_test_stru *hal_mac_test_device = &g_hal_mac_device_test;
    osal_u8 buf[HMAC_SAMPLE_PHY_DAQ_SIZE_UNIT] = {0};
    osal_u32 offset = 0;

    while ((offset = hal_config_chip_test_set_diag_param_phy_save_data(buf, HMAC_SAMPLE_PHY_DAQ_SIZE_UNIT,
        hal_mac_test_device)) > 0) {
        diag_cmd_report_sample_data(buf, offset);
    }

    diag_report_wlan_sample_data(OSAL_NULL, 0, 0); /* 0: 数采上报结束信号 */
    return OAL_SUCC;
}
/* ============== PHY 数采接口 结束 ============ */
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif