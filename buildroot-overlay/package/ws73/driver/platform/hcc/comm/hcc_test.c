/*
 * Copyright (c) CompanyNameMagicTag 2021-2023. All rights reserved.
 * Description: HCC TEST
 * Author: CompanyName
 */

#include "td_base.h"
#include "hcc_comm.h"
#include "hcc_if.h"
#include "hcc_dfx.h"
#include "hcc.h"
#include "hcc_bus.h"
#include "soc_osal.h"
#include "hcc_channel.h"
#include "hcc_adapt.h"
#include "hcc_service.h"
#include "securec.h"
#include "hcc_test.h"

#ifdef CONFIG_HCC_SUPPORT_TEST
TD_PRV hcc_test_info g_test_info = {
    .test_para.loop_cnt = 0,
    .test_para.frame_size = 0,
    .test_para.expect_rx_cnt = 0,
    .test_para.test_queue_id = TEST_SDIO_SINGLE_QUEUE,
    .test_para.channel_name = HCC_CHANNEL_AP,
    .test_para.test_data = 0x01, // 测试数据  0x5a
    .test_para.test_service = HCC_ACTION_TYPE_TEST,

    .test_stat = {
        .test_stop = TD_FALSE,
    },
};

hcc_test_info *hcc_test_get_test_info(td_void)
{
    return &g_test_info;
}

td_void hcc_test_set_trans_rate(td_u32 rate)
{
    g_test_info.test_data.trans_rate = rate;
}

td_void hcc_test_set_trans_time_us(td_u64 us)
{
    g_test_info.test_data.trans_time = us;
}

td_u64 hcc_test_get_start_time(td_void)
{
    return g_test_info.start_time;
}

td_void hcc_test_set_start_time(td_u64 us)
{
    hcc_debug("set start_time us: %d us\n", (td_u32)us);
    g_test_info.start_time = us;
}

#if defined(CONFIG_HCC_SUPPORT_UART)
td_u64 hcc_test_get_init_time(td_void)
{
    return g_test_info.init_time;
}

td_void hcc_test_set_init_time(td_u64 us)
{
    hcc_debug("set init_time us: %d us\n", (td_u32)us);
    g_test_info.init_time = us;
}
#endif

td_u64 hcc_test_get_stop_time(td_void)
{
    return g_test_info.stop_time;
}

td_void hcc_test_set_stop_time(td_u64 us)
{
    hcc_debug("set stop_time us: %d us\n", (td_u32)us);
    g_test_info.stop_time = us;
}

td_u64 hcc_test_get_trans_time_us(td_void)
{
    return g_test_info.test_data.trans_time;
}

td_u32 hcc_test_get_trans_rate(td_void)
{
    return g_test_info.test_data.trans_rate;
}

td_u32 hcc_test_get_trans_bits(td_void)
{
    return g_test_info.test_data.trans_bits;
}

td_void hcc_test_set_trans_bits(td_u32 bits)
{
    g_test_info.test_data.trans_bits = bits;
}

td_u32 hcc_test_get_exp_rx_cnt(td_void)
{
    return g_test_info.test_para.expect_rx_cnt;
}

td_u32 hcc_test_get_loop_cnt(td_void)
{
    return g_test_info.test_para.loop_cnt;
}

td_u16 hcc_test_get_frame_size(td_void)
{
    return g_test_info.test_para.frame_size;
}

td_u32 hcc_test_get_test_service(td_void)
{
    return g_test_info.test_para.test_service;
}

td_bool hcc_test_is_test_stop(td_void)
{
    return g_test_info.test_stat.test_stop;
}

td_void hcc_test_set_test_stop(td_bool stop)
{
    g_test_info.test_stat.test_stop = stop;
}

hcc_queue_type hcc_test_get_test_queue_id(td_void)
{
    return g_test_info.test_para.test_queue_id;
}

td_void hcc_test_set_test_queue_id(hcc_queue_type queue_id)
{
    g_test_info.test_para.test_queue_id = queue_id;
}

td_u16 hcc_test_get_test_channel(td_void)
{
    return g_test_info.test_para.channel_name;
}

td_u32 hcc_test_get_test_data(td_void)
{
    return g_test_info.test_para.test_data++;
}

TD_PRV td_void hcc_test_set_test_channel(hcc_channel_name channel_name)
{
    g_test_info.test_para.channel_name = channel_name;
}

TD_PRV td_void hcc_test_set_exp_rx_cnt(td_u32 cnt)
{
    g_test_info.test_para.expect_rx_cnt = cnt;
}

TD_PRV td_void hcc_test_set_loop_cnt(td_u32 cnt)
{
    g_test_info.test_para.loop_cnt = cnt;
}

TD_PRV td_void hcc_test_set_frame_size(td_u16 size)
{
    g_test_info.test_para.frame_size = size;
}

TD_PRV td_void hcc_test_set_test_data(td_u32 data)
{
    g_test_info.test_para.test_data = data;
}

TD_PRV td_void hcc_test_msg(hcc_tx_msg_type msg)
{
    hcc_send_message(HCC_CHANNEL_AP, msg, HCC_ACTION_TYPE_TEST);
}

TD_PRV td_void hcc_test_cmd_init_all_param(td_u32 data1, td_u32 data2, td_u32 data3)
{
    td_u32 trans_bits = 0;
    hcc_test_set_loop_cnt(data1);
    hcc_test_set_frame_size(data2);
    hcc_test_set_exp_rx_cnt(data3);
    hcc_test_reset_rx_stat();
    trans_bits = hcc_test_get_frame_size() * 8; /* 8 : 位数 */
    trans_bits *= (hcc_test_get_loop_cnt() > 0 ? hcc_test_get_loop_cnt() : hcc_test_get_exp_rx_cnt());
    hcc_test_set_trans_bits(trans_bits);
    hcc_test_set_trans_time_us(0);
    hcc_test_set_trans_rate(0);
#if defined(CONFIG_HCC_SUPPORT_UART)
    hcc_test_set_init_time(hcc_get_timestamp());
#endif
    hcc_test_print_initial_result();
}

TD_PRV ext_errno hcc_test_command_proc(td_u32 command)
{
    switch (command) {
        case HCC_TEST_CMD_START_TEST:
            hcc_debug("HCC TEST 1 DIR\r\n");
            hcc_test();
            break;

        case HCC_TEST_CMD_PRINT_RESULT:
            hcc_test_print_test_result();
            break;

        case HCC_TEST_CMD_TEST_INIT:
            hcc_test_init();
            break;

        case HCC_TEST_GET_CREDIT:
            hcc_test_get_credit();
            break;

        default:
            hcc_printf_err_log("cmd[%d]\r\n", command);
            return EXT_ERR_FAILURE;
    }
    return EXT_ERR_SUCCESS;
}

TD_PRV ext_errno hcc_test_command_proc_p1(td_u32 command, td_u32 data)
{
    switch (command) {
        case HCC_TEST_CMD_ENABLE_RX_THREAD:
            hcc_enable_rx_thread(HCC_CHANNEL_AP, (td_bool)data);
            break;

        case HCC_TEST_CMD_ENABLE_TX_THREAD:
            hcc_enable_tx_thread(HCC_CHANNEL_AP, (td_bool)data);
            break;

        case HCC_TEST_SET_QUEUE_ID:
            hcc_test_set_test_queue_id(data);
            break;

        case HCC_TEST_CMD_TEST_MSG:
            hcc_test_msg((hcc_tx_msg_type)data);
            break;

        case HCC_TEST_PRINT_SERVICE_INFO:
            hcc_dfx_service_info_print((hcc_service_type)data);
            break;

        case HCC_TEST_PRINT_BUS_INFO:
            hcc_bus_dfx_statics_print((hcc_channel_name)data);
            break;

        case HCC_TEST_UPDATE_CREDIT:
            hcc_bus_update_credit(hcc_get_handler(HCC_CHANNEL_AP)->bus, data);
            break;

        case HCC_TEST_CMD_SET_TX_DATA:
            hcc_test_set_test_data(data);
            break;

        case HCC_TEST_CMD_SET_TX_CHANNEL:
            hcc_test_set_test_channel(data);
            break;

        case HCC_TEST_CMD_TEST_REG_OPT:
            hcc_test_register_opt(data);
            break;

        default:
            hcc_printf_err_log("\r\n");
            return EXT_ERR_FAILURE;
    }
    return EXT_ERR_SUCCESS;
}

TD_PRV ext_errno hcc_test_command_proc_p2(td_u32 command, td_u32 data1, td_u32 data2)
{
    switch (command) {
        case HCC_TEST_PRINT_QUEUE_INFO:
            hcc_dfx_queue_info_print(HCC_CHANNEL_AP, data1, data2);
            break;
        default:
            hcc_printf_err_log("\r\n");
            return EXT_ERR_FAILURE;
    }
    return EXT_ERR_SUCCESS;
}

TD_PRV ext_errno hcc_test_command_proc_p3(td_u32 command, td_u32 data1, td_u32 data2, td_u32 data3)
{
    switch (command) {
        case HCC_TEST_CMD_INIT_ALL_PARAM:
            hcc_test_cmd_init_all_param(data1, data2, data3);
            break;

        case HCC_TEST_CMD_INIT_ALL_PARAM_AND_START_TEST:
            hcc_debug("hcc test start:\r\n");
            hcc_test_cmd_init_all_param(data1, data2, data3);
            /* 设置完参数启动测试流程 */
            hcc_test_start_test_after_init_paras();
            /* 收到测试停止后计算速率并打印 */
            break;

        default:
            hcc_printf_err_log("\r\n");
            return EXT_ERR_FAILURE;
    }
    return EXT_ERR_SUCCESS;
}

hcc_test_proc g_hcc_test_proc = TD_NULL;
td_void hcc_test_cmd_register(hcc_test_proc test_proc)
{
    g_hcc_test_proc = test_proc;
}

td_s32 hcc_test_proc_local_cmd(td_u32 *argv, td_u32 argc)
{
    ext_errno ret = EXT_ERR_FAILURE;
    if (argv == TD_NULL) {
        return EXT_ERR_FAILURE;
    }
    switch (argc) {
        case 1:
            ret = hcc_test_command_proc(argv[0]);
            break;

        case 2: /* 2: 参数数量 */
            ret = hcc_test_command_proc_p1(argv[0], argv[1]);
            break;

        case 3: /* 3: 参数数量 */
            ret = hcc_test_command_proc_p2(argv[0], argv[1], argv[2]); /* 2 : index of parameters */
            break;

        case 4: /* 4: 参数数量 */
            /* 2 : index of parameters 3 : index of parameters */
            ret = hcc_test_command_proc_p3(argv[0], argv[1], argv[2], argv[3]);
            break;

        default:
            hcc_printf_err_log("argc err\n");
            break;
    }
    if (g_hcc_test_proc != TD_NULL) {
        return g_hcc_test_proc(argv, argc);
    }
    return ret;
}
osal_module_export(hcc_test_proc_local_cmd);
#endif
