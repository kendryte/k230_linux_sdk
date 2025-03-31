/*
 * Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
 * Description: zdiag adapt os
 * This file should be changed only infrequently and with great care.
 */
#include "zdiag_adapt_os.h"
#include <oal_ext_util.h>
#include "hcc_if.h"
#include "securec.h"
#include "soc_osal.h"
#include "soc_module.h"
#include "soc_zdiag.h"
#include "soc_crc.h"
#include "soc_diag_wdk.h"
#include "diag_rom_api.h"
#include "diag_cmd_default_tbl.h"
#include "zdiag_linux_socket.h"
#include "zdiag_linux_uart.h"
#include "zdiag_tx_proc.h"
#include "zdiag_log_adapt.h"
#include "zdiag_lib_dbg.h"
#include "zdiag_adapt_hcc_comm.h"
#include "diag_adapt_layer.h"
#include "oal_debug.h"
#if defined(CONFIG_HCC_SUPPORT_UART)
#include "hcc_adapt_uart.h"
#endif
#if defined(_PRE_OS_VERSION) && defined(_PRE_OS_VERSION_LINUX) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include "zdiag_local_log.h"
#include "zdiag_shell_proc.h"
#endif

#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include "ini.h"
#endif

#define INI_DIAG_LOG_MODE     "oam_log_mode" /* Default ini name */

#define OS_TIME_MULTIPLE 1000

/* rewrite adapt code */
typedef struct {
    zdiag_addr addr;
    td_u8 attribute;
    diag_channel_id channel_id;
} diag_addr_item;

diag_addr_item g_addr_items[] = {
    {ACORE_ADDR, DIAG_ADDR_ATTRIBUTE_VALID, DIAG_SUPPORT_CHANNEL_CNT},
    {ACORE_HSO_ADDR, DIAG_ADDR_ATTRIBUTE_VALID|DIAG_ADDR_ATTRIBUTE_HSO_CONNECT, DIAG_CHANNEL_ID_0},
    {ACORE_HSOB_ADDR, DIAG_ADDR_ATTRIBUTE_VALID|DIAG_ADDR_ATTRIBUTE_HSO_CONNECT, DIAG_CHANNEL_ID_1},
    {PCORE_ADDR, DIAG_ADDR_ATTRIBUTE_VALID|DIAG_ADDR_ATTRIBUTE_HCC, DIAG_CHANNEL_ID_1},
    {PCORE_HSO_ADDR, DIAG_ADDR_ATTRIBUTE_VALID|DIAG_ADDR_ATTRIBUTE_HCC, DIAG_CHANNEL_ID_1},
};

#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
volatile td_bool g_diag_hcc_conn_state = TD_FALSE;

td_bool diag_hcc_conn_state_get(td_void)
{
    return g_diag_hcc_conn_state;
}

td_void diag_hcc_conn_state_set(td_bool state)
{
    g_diag_hcc_conn_state = state;
}

/* diag log mode, judge which output be selected by ini, hso or local file */
td_u8 g_dlog_mode = DIAG_LOG_TO_HSO; /* update by ini oam_log_mode */

td_u8 zdiag_log_mode_get(void)
{
    return g_dlog_mode;
}

void zdiag_log_mode_set(td_u8 dlog_mode)
{
    g_dlog_mode = dlog_mode;
}

#endif

diag_hso_conn_stat g_diag_hso_conn_state = DIAG_HSO_CONN_NONE;

diag_hso_conn_stat diag_hso_conn_state_get(td_void)
{
    return g_diag_hso_conn_state;
}

td_void diag_hso_conn_state_set(diag_hso_conn_stat state)
{
    g_diag_hso_conn_state = (diag_hso_conn_stat)state;
}

#if defined(_PRE_OS_VERSION_LITEOS) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
td_u32 diag_adapt_get_msg_time(td_void)
{
    time_t sec = time(TD_NULL);
    return (td_u32)sec;
}

td_u32 diag_adapt_get_cur_second(td_void)
{
    time_t sec = time(TD_NULL);
    return (td_u32)sec;
}

#else

td_u32 diag_adapt_get_msg_time(td_void)
{
    td_u64 msec_cnt;
    td_u64 usec_cnt;

#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 20, 0))
    struct timespec64 tv;
    ktime_get_real_ts64(&tv);
    usec_cnt = (td_u64)(tv.tv_nsec / OS_TIME_MULTIPLE); // 1000 is time unit transform multiple
#else
    struct timeval tv;
    do_gettimeofday(&tv);
    usec_cnt = (td_u64)tv.tv_usec;
#endif

    msec_cnt = (td_u64)(tv.tv_sec * OS_TIME_MULTIPLE * OS_TIME_MULTIPLE) + usec_cnt;
    return (td_u32)msec_cnt;
}

td_u32 diag_adapt_get_cur_second(td_void)
{
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 20, 0))
    struct timespec64 tv;
    ktime_get_real_ts64(&tv);
#else
    struct timeval tv;
    do_gettimeofday(&tv);
#endif

    return (td_u32)tv.tv_sec;
}
#endif

zdiag_addr diag_adapt_get_emmc_dst(td_void)
{
    return ACORE_ADDR;
}

zdiag_addr diag_adapt_get_local_addr(td_void)
{
    return ACORE_ADDR;
}

zdiag_addr diag_adapt_get_cache_pkt_dst(td_void)
{
    return ACORE_ADDR;
}

zdiag_addr diag_adapt_get_module_id(td_void)
{
    return DIAG_MOD_ID_WIFI_HOST;
}

/* get module id, based module_id, msg_id, on host side */
zdiag_addr diag_adapt_get_module_id_adjusted(td_u32 module_id, td_u32 msg_id)
{
    td_u32 file_id = ((msg_id & ((td_u32)0x3FFF << 18)) >> 18);
    zdiag_print("host module_id: %d, msg_id: 0x%x, file_id: %d\n", module_id, msg_id, file_id);
    if ((file_id >= DIAG_FILE_ID_BT_MIN) && (file_id <= DIAG_FILE_ID_BT_MAX)) {
        return (module_id == DIAG_MOD_ID_WIFI_HOST) ? DIAG_MOD_ID_BSLE_HOST : DIAG_MOD_ID_OAM_HOST;
    }
    if (module_id == DIAG_MOD_ID_WIFI_HOST) {
        return module_id;
    } else if (module_id == DIAG_MOD_ID_WIFI_DEVICE) {
        return DIAG_MOD_ID_WIFI_DEVICE;
    } else {
        return DIAG_MOD_ID_WIFI_HOST;
    }
}

diag_channel_id diag_adapt_dst_2_channel_id(zdiag_addr addr)
{
    unsigned int i;
    for (i = 0; i < sizeof(g_addr_items) / sizeof(g_addr_items[0]); i++) {
        diag_addr_item *item = &g_addr_items[i];
        if (item->addr == addr) {
            return item->channel_id;
        }
    }
    return DIAG_CHANNEL_ID_0;
}

diag_addr_attribute diag_adapt_addr_2_attribute(zdiag_addr addr)
{
    unsigned int i;
    for (i = 0; i < sizeof(g_addr_items) / sizeof(g_addr_items[0]); i++) {
        diag_addr_item *item = &g_addr_items[i];
        if (item->addr == addr) {
            return item->attribute;
        }
    }
    return 0;
}

td_u32 diag_adapt_int_lock(td_void)
{
    return 0;
}

td_void diag_adapt_int_restore(td_u32 lock_stat)
{
    return;
}

td_u16 diag_adapt_crc16(td_u16 crc_start, TD_CONST td_u8 *buf, td_u32 len)
{
    return uapi_crc16(crc_start, buf, len);
}

td_void *diag_adapt_malloc(unsigned size)
{
    return osal_kmalloc(size, OSAL_GFP_KERNEL);
}

td_void diag_adapt_free(td_void *p)
{
    return osal_kfree(p);
}

static ext_errno zdiag_cmd_ind_init(td_void)
{
    ext_errno ret;
    ret = uapi_diag_default_cmd_init();
    if (ret != EXT_ERR_SUCCESS) {
        return ret;
    }

    return EXT_ERR_SUCCESS;
}

static ext_errno zdiag_channel_init(td_void)
{
    ext_errno ret;

#if defined(CONFIG_DIAG_SUPPORT_UART) && (CONFIG_DIAG_SUPPORT_UART == 1)
    ret = zdiag_uart_channel_init();
    if (ret != EXT_ERR_SUCCESS) {
        return ret;
    }
#endif // for CONFIG_DIAG_SUPPORT_UART

#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#if defined(CONFIG_DIAG_SUPPORT_SOCKET) && (CONFIG_DIAG_SUPPORT_SOCKET == 1)
    ret = zdiag_socket_channel_init();
    if (ret != EXT_ERR_SUCCESS) {
        return ret;
    }
#endif // for CONFIG_DIAG_SUPPORT_SOCKET

    ret = zdiag_tx_proc_init();
    if (ret != EXT_ERR_SUCCESS) {
        return ret;
    }

#if defined(CONFIG_DFX_SUPPORT_SYSFS) && (CONFIG_DFX_SUPPORT_SYSFS == 1)
    ret = zdiag_dbg_sysfs_init();
    if (ret != EXT_ERR_SUCCESS) {
        return ret;
    }
#endif // for CONFIG_DFX_SUPPORT_SYSFS
#endif
    return EXT_ERR_SUCCESS;
}

static ext_errno zdiag_channel_exit(td_void)
{
    ext_errno ret;

#if defined(CONFIG_DIAG_SUPPORT_UART) && (CONFIG_DIAG_SUPPORT_UART == 1)
    ret = zdiag_uart_channel_exit();
    if (ret != EXT_ERR_SUCCESS) {
        oal_print_err("diag uart kernel exit, absent! \r\n");
    }
#endif // for CONFIG_DIAG_SUPPORT_UART

#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#if defined(CONFIG_DIAG_SUPPORT_SOCKET) && (CONFIG_DIAG_SUPPORT_SOCKET == 1)
    ret = zdiag_socket_channel_exit();
    if (ret != EXT_ERR_SUCCESS) {
        return ret;
    }
#endif // for CONFIG_DIAG_SUPPORT_SOCKET
    ret = zdiag_tx_proc_exit();
    if (ret != EXT_ERR_SUCCESS) {
        return ret;
    }

#if defined(CONFIG_DFX_SUPPORT_SYSFS) && (CONFIG_DFX_SUPPORT_SYSFS == 1)
    ret = zdiag_dbg_sysfs_exit();
    if (ret != EXT_ERR_SUCCESS) {
        return ret;
    }
#endif // for CONFIG_DFX_SUPPORT_SYSFS
#endif
    return EXT_ERR_SUCCESS;
}

#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
ext_errno zdiag_adapt_log_mode_init(td_void)
{
#ifdef CONFIG_NO_SUPPORT_INI
    return EXT_ERR_SUCCESS;
#else
    /* Get ini custom config datas, and Check */
    osal_s32 ini_value;
    ext_errno ret;

    ret = get_cust_conf_int32_etc(INI_MODU_PLAT, INI_DIAG_LOG_MODE, &ini_value);
    if (ret != INI_SUCC) {
        oal_print_err("get diag log_mode custom config failed\n");
        return EXT_ERR_DIAG_INVALID_PARAMETER;
    }
    if (ini_value < DIAG_LOG_TO_HSO || ini_value >= DIAG_LOG_TO_NULL) {
        return EXT_ERR_DIAG_NOT_SUPPORT;
    }
    g_dlog_mode = (td_u8)ini_value;
    return EXT_ERR_SUCCESS;
#endif
}
#endif

ext_errno zdiag_adapt_init(td_void)
{
    ext_errno ret = EXT_ERR_SUCCESS;
    diag_rom_api rom_api;

#if defined(CONFIG_DFX_SUPPORT_SHELL_PROC)
#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    zdiag_shell_proc_init();
#endif
#endif

    ret = zdiag_cmd_ind_init();
    if (ret != EXT_ERR_SUCCESS) {
        oal_print_err("zdiag_adapt_init::zdiag_cmd_ind_init err[%d].\n", ret);
        return ret;
    }

    ret = zdiag_channel_init();
    if (ret != EXT_ERR_SUCCESS) {
        oal_print_err("zdiag_adapt_init::zdiag_channel_init err[%d].\n", ret);
        return ret;
    }

    rom_api.report_sys_msg = zdiag_log_report_sys_msg_instance;
    diag_rom_api_register(&rom_api);

#if defined(_PRE_OS_VERSION) && defined(_PRE_OS_VERSION_LINUX) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    ret = zdiag_adapt_log_mode_init();
    if (ret != EXT_ERR_SUCCESS) {
        oal_print_err("zdiag_adapt_init::zdiag_adapt_log_mode_init err[%d].\n", ret);
        return ret;
    }
#if defined(CONFIG_DIAG_SUPPORT_LOCAL_LOG)
    if (zdiag_log_mode_get() == DIAG_LOG_TO_FILE) {
        zdiag_local_log_init();
    }
#endif
#endif

    oal_print_info("zdiag_adapt_init::succ.");
    return EXT_ERR_SUCCESS;
}

ext_errno zdiag_adapt_exit(td_void)
{
#if defined(CONFIG_DFX_SUPPORT_SHELL_PROC)
#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    zdiag_shell_proc_exit();
#endif
#endif
    zdiag_channel_exit();
    return EXT_ERR_SUCCESS;
}

static td_void diag_pkt_malloc_cmn(diag_pkt_malloc_param *param, diag_pkt_malloc_result *result)
{
    result->basic.type = param->basic.type;
    result->basic.size = param->basic.size;
    result->basic.buf = diag_adapt_malloc(result->basic.size);
}

td_void diag_adapt_pkt_malloc(diag_pkt_malloc_param *param, diag_pkt_malloc_result *result)
{
    if (param->basic.type == DIAG_PKT_MALLOC_TYPE_IPC) {
        result->basic.buf = TD_NULL;
        return;
    } else {
        diag_pkt_malloc_cmn(param, result);
    }
}

static td_void diag_pkt_free_cmn(diag_pkt_malloc_result *result)
{
    diag_adapt_free(result->basic.buf);
    result->basic.buf = TD_NULL;
}

td_void diag_adapt_pkt_free(diag_pkt_malloc_result *result)
{
    if (result->basic.type == DIAG_PKT_MALLOC_TYPE_IPC) {
        return;
    } else if (result->basic.type == DIAG_PKT_MALLOC_TYPE_CMN) {
        diag_pkt_free_cmn(result);
    }
}

osal_u32 diag_hcc_adapt_alloc(hcc_queue_type queue_id, osal_u32 len, osal_u8 **buf, osal_u8 **user_param)
{
    *buf = (osal_u8 *)osal_kmalloc(len, OSAL_GFP_KERNEL);
    if (*buf == OSAL_NULL) {
        return EXT_ERR_MALLOC_FAILURE;
    }
    zdiag_print("diag_hcc_adapt_alloc - %d\r\n", len);
    *user_param = OSAL_NULL;
    return EXT_ERR_SUCCESS;
}

static inline osal_void diag_hcc_service_init_hdr(hcc_transfer_param *param, osal_u32 main_type, osal_u32 sub_type,
    osal_u8 queue_id, osal_u16 fc_flag)
{
    param->service_type = main_type;
    param->sub_type = sub_type;
    param->queue_id = queue_id;
    param->fc_flag = fc_flag;
}

static td_u32 diag_hcc_sync_cmd_send(osal_u8 *data, zdiag_hcc_oam_data_stru *sync, hcc_transfer_param *param,
                                     td_pvoid cmd_param, osal_u32 data_size)
{
    errno_t ret;
#ifdef CONFIG_HCC_SUPPORT_UART
    hcc_handler *hcc = hcc_get_handler(HCC_CHANNEL_AP);
#endif
    osal_u16 fc_flag = HCC_FC_NONE;  // alternative selection HCC_FC_DROP
    ret = memcpy_s(data + hcc_get_head_len(), data_size - hcc_get_head_len(), (osal_u8 *)sync,
                   sizeof(zdiag_hcc_oam_data_stru));
    if (ret != EOK) {
        goto CMD_MEMCPY_FAIL;
    }
    ret = memcpy_s(data + hcc_get_head_len() + sizeof(zdiag_hcc_oam_data_stru),
                   data_size - hcc_get_head_len() - sizeof(zdiag_hcc_oam_data_stru), (osal_u8 *)cmd_param, sync->len);
    if (ret != EOK) {
        goto CMD_MEMCPY_FAIL;
    }
    diag_hcc_service_init_hdr(param, HCC_ACTION_TYPE_OAM, OAM_HCC_DEAULT_VALUE, HCC_QUEUE_INTERNAL, fc_flag);
#ifdef CONFIG_HCC_SUPPORT_UART
    if (hcc->bus->bus_type == HCC_BUS_UART) {
        data -= sizeof(uart_packet_head);
        data_size += sizeof(uart_packet_head);
    }
#endif
    if (hcc_tx_data(HCC_CHANNEL_AP, data, data_size, param) != EXT_ERR_SUCCESS) {
        osal_kfree(data);
        return EXT_ERR_HCC_TX_BUF_ERR;
    }
    return EXT_ERR_SUCCESS;
CMD_MEMCPY_FAIL:
#ifdef CONFIG_HCC_SUPPORT_UART
    if (hcc->bus->bus_type == HCC_BUS_UART) {
        data -= sizeof(uart_packet_head);
    }
#endif
    osal_kfree(data);
    return EXT_ERR_MEMCPY_FAIL;
}

ext_errno diag_hcc_sync_cmd_to_dev(td_u16 cmd_id, td_pvoid cmd_param, td_u16 cmd_param_size, diag_option *option)
{
    hcc_transfer_param param;
    zdiag_hcc_oam_data_stru sync = {0};
    osal_u8 *data = OSAL_NULL;
    osal_u8 *user_param = OSAL_NULL;
    osal_u32 data_size, ret;
#ifdef CONFIG_HCC_SUPPORT_UART
    hcc_handler *hcc = hcc_get_handler(HCC_CHANNEL_AP);
    if (hcc == TD_NULL) {
        return EXT_ERR_FAILURE;
    }
#endif

    sync.sync_id = cmd_id;
    sync.len = cmd_param_size;

#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    if (!g_diag_hcc_conn_state) {
        return EXT_ERR_HCC_STATE_EXCEPTION;
    }
#endif
    data_size = hcc_get_head_len() + (osal_u32)sizeof(zdiag_hcc_oam_data_stru) + sync.len;
#ifdef CONFIG_HCC_SUPPORT_UART
    if (hcc->bus->bus_type == HCC_BUS_UART) {
        data_size += sizeof(uart_packet_head) + 1;
    }
#endif
    data_size = (data_size + HCC_DATA_LEN_ALIGN - 1) / HCC_DATA_LEN_ALIGN * HCC_DATA_LEN_ALIGN;
    ret = diag_hcc_adapt_alloc(HCC_QUEUE_INTERNAL, data_size, &data, &user_param);
    if (ret != EXT_ERR_SUCCESS) {
        oal_print_err("diag_hcc_sync_hso_conn_disc failed\r\n");
        return EXT_ERR_HCC_BUILD_TX_BUF_ERR;
    }
#ifdef CONFIG_HCC_SUPPORT_UART
    if (hcc->bus->bus_type == HCC_BUS_UART) {
        data += sizeof(uart_packet_head);
        data_size -= sizeof(uart_packet_head);
    }
#endif
    param.user_param = user_param;
    return diag_hcc_sync_cmd_send(data, &sync, &param, cmd_param, data_size);
}

#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#define EXT_DIAG_MODULE_SYS 5    /* Number of types that can be configured for the SYS MSG */

#define ZDIAG_FILTER_MAX_LEVEL 8
#include "soc_diag_cmd.h"
#include "soc_diag_cmd_id.h"

ext_errno diag_sync_cfg_cmd_to_dev(td_u32 level)
{
    diag_option option = DIAG_OPTION_INIT_VAL;
    td_u8 msp_diag_data[64];
    td_u8 diag_module_sys[EXT_DIAG_MODULE_SYS] = {
        DIAG_MOD_ID_WIFI_HOST, DIAG_MOD_ID_WIFI_DEVICE, DIAG_MOD_ID_OAM_HOST, SOC_ID_BSP, SOC_ID_SYS};
    td_u16 data_len;
    td_u16 i; // item
    diag_cmd_msg_cfg_req_stru *msg_cfg_req = (diag_cmd_msg_cfg_req_stru *)msp_diag_data; // outer msg: msp_diag_head

    data_len = 0;
    diag_hcc_sync_cmd_to_dev(DIAG_CMD_HOST_CONNECT, msg_cfg_req, data_len, &option);
    msg_cfg_req = (diag_cmd_msg_cfg_req_stru *)msp_diag_data;

    for (data_len = 0, i = 0; i < EXT_DIAG_MODULE_SYS; ++i) {
        if ((data_len + sizeof(diag_cmd_msg_cfg_req_stru)) > sizeof(msp_diag_data)) {
            oal_print_err("buffer length not support!\n");
            return EXT_ERR_FAILURE;
        }
        msg_cfg_req->module_id = diag_module_sys[i];
        msg_cfg_req->is_dest_module = 0;
        msg_cfg_req->switch_code = 1;
        msg_cfg_req += 1;
        data_len += (td_u16)sizeof(diag_cmd_msg_cfg_req_stru);
    }
    diag_hcc_sync_cmd_to_dev(DIAG_CMD_MSG_CFG_SET_SYS, (td_pvoid)msp_diag_data, data_len, &option);
    msg_cfg_req = (diag_cmd_msg_cfg_req_stru *)msp_diag_data;

    for (data_len = 0, i = 0; i < uapi_min(level, ZDIAG_FILTER_MAX_LEVEL); ++i) {
        if ((data_len + sizeof(diag_cmd_msg_cfg_req_stru)) > sizeof(msp_diag_data)) {
            oal_print_err("buffer length not support!\n");
            return EXT_ERR_FAILURE;
        }
        msg_cfg_req->module_id = i;
        msg_cfg_req->is_dest_module = 0;
        msg_cfg_req->switch_code = 1;
        msg_cfg_req += 1;
        data_len += (td_u16)sizeof(diag_cmd_msg_cfg_req_stru);
    }
    diag_hcc_sync_cmd_to_dev(DIAG_CMD_MSG_CFG_SET_LEVEL, (td_pvoid)msp_diag_data, data_len, &option);
    oal_print_err("diag sync cfg to dev ok\r\n");

    return EXT_ERR_SUCCESS;
}
#endif