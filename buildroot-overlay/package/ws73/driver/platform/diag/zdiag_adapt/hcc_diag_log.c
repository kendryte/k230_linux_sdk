/*
 * Copyright (c) CompanyNameMagicTag 2021-2021. All rights reserved.
 * Description: hcc diag log
 * This file should be changed only infrequently and with great care.
 */

#include "hcc_diag_log.h"
#include "soc_osal.h" // for osal_module_export, osal_module_license
#include "soc_diag_cmd_id.h"
#include "oam_ext_if.h"

#include "hcc_if.h"
#include "hcc_comm.h" // for enum hcc_queue_type
#include "zdiag_common.h" // for struct msp_mux_packet_head_stru
#include "diag_adapt_layer.h"
#include "zdiag_adapt_layer.h"
#include "zdiag_tx_proc.h"

#include "plat_hcc_srv.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

static td_void hcc_diag_log_mem_free(hcc_queue_type queue_id, td_u8 *buf, td_u8 *user_param);
static td_u32 hcc_diag_log_mem_alloc(hcc_queue_type queue_id, td_u32 len, td_u8 **buf, td_u8 **user_param);

#if defined(WSCFG_PLAT_DIAG_LOG_OUT)
#define HCC_DATA_DUMP_LINE_SIZE 16
static td_void hcc_send_device_data_dump(td_u8 *hcc_buf, td_u16 len)
{
    int i;

    zdiag_print("hcc_send_device_data_dump content len = %d\r\n", len);
    for (i = 0; i < len; ++i) {
        if (i % HCC_DATA_DUMP_LINE_SIZE == 0) {
            zdiag_print("\n");
        }
        zdiag_print("%02x ", hcc_buf[i]);
    }
    zdiag_print("\n");
}

#define DIAG_MSG_ID_MASK 0x3FFF
#define DIAG_MSG_IS_SHIFT 18

/* adjust module id, based hcc diag msg, msg_id, on host side */
static td_void hcc_diag_log_get_module_id_adjusted(td_u8 *buf, td_u16 length)
{
    msp_mux_packet_head_stru_device *layer1;
    msp_diag_head_ind_stru *layer2;
    diag_cmd_log_layer_ind_stru *layer3;
    td_u32 msg_id, file_id, *module_id;

    if (length <
        (sizeof(msp_mux_packet_head_stru_device) + sizeof(msp_diag_head_ind_stru) + sizeof(msp_diag_head_ind_stru))) {
        return;
    }

    layer1 = (msp_mux_packet_head_stru_device *)buf;
    layer2 = (msp_diag_head_ind_stru *)(layer1 + 1);
    layer3 = (diag_cmd_log_layer_ind_stru *)(layer2 + 1);

    zdiag_print("layer1->cmd_id: %d, layer2->cmd_id: 0x%x.\n", layer1->cmd_id, layer2->cmd_id);
    if ((layer1->cmd_id != DIAG_CMD_MSG_RPT_SYS) || (layer2->cmd_id != DIAG_CMD_MSG_RPT_SYS)) {
        return;
    }

    module_id = &layer3->module;
    msg_id = layer3->id;
    file_id = ((msg_id & ((td_u32)DIAG_MSG_ID_MASK << DIAG_MSG_IS_SHIFT)) >> DIAG_MSG_IS_SHIFT);
    zdiag_print("module_id: %d, msg_id: 0x%x, file_id: 0x%x.\n", *module_id, msg_id, file_id);
    if ((file_id >= DIAG_FILE_ID_BT_MIN) && (file_id <= DIAG_FILE_ID_BT_MAX)) {
        *module_id = DIAG_MOD_ID_BSLE_DEVICE;
        layer1->crc16 = diag_adapt_crc16(0, layer1->puc_packet_data, layer1->packet_data_size);
    }
    zdiag_print("module_id: %d, file_id: %d.\n", *module_id, file_id);
}

#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
td_u32 hcc_send_device_data2diag_etc(td_u8 *hcc_buf, td_u16 len)
{
    msp_mux_packet_head_stru_device *mem;
    td_u16 length;
    td_u8 *buf = TD_NULL;

    if (hcc_buf == NULL) {
        return EXT_ERR_FAILURE;
    }

    mem = (msp_mux_packet_head_stru_device *)hcc_buf;
    length = mem->packet_data_size + (td_u16)sizeof(msp_mux_packet_head_stru_device);
    hcc_send_device_data_dump(hcc_buf, len);

    buf = (td_u8 *)osal_kmalloc(length, OSAL_GFP_KERNEL);
    if (buf == TD_NULL) {
        return EXT_ERR_FAILURE;
    }

    if (memcpy_s(buf, length, hcc_buf, length) != EOK) {
        oal_print_err("memcpy_s fail\n");
        osal_kfree(buf);
        return EXT_ERR_FAILURE;
    }
    mem = (msp_mux_packet_head_stru_device *)buf;
    mem->start_flag = MUX_START_FLAG; // 0xDEADBEEF
    zdiag_print("hcc send device data size %d\n", mem->packet_data_size);
    hcc_diag_log_get_module_id_adjusted((td_u8 *)buf, length);
    zdiag_send_oam_log((td_u8 *)buf, length, ID_MSG_ZDIAG_CHIP_LOG_TX);

    return EXT_ERR_SUCCESS;
}

static td_u32 hcc_diag_log_rx_proc_func_etc(
    hcc_queue_type queue_id, td_u8 sub_type, td_u8 *buf, td_u32 len, td_u8 *user_param)
{
    td_u32 ret;

    if (OAL_WARN_ON(buf == NULL)) {
        oal_print_err("%s error: buf is null\n", __func__);
        return EXT_ERR_FAILURE;
    }

    // here no need to calc real len, len only be 32\64\...
    if (len < HCC_DATA_LEN_ALIGN) {
        oal_print_err("%s error: hcc pkt is too short\n", __func__);
        hcc_diag_log_mem_free(queue_id, buf, user_param);
        return EXT_ERR_FAILURE;
    }

    if ((len % HCC_DATA_LEN_ALIGN) != 0) {
        oal_print_err("%s error: hcc pkt len is invalid\n", __func__);
        hcc_diag_log_mem_free(queue_id, buf, user_param);
        return EXT_ERR_FAILURE;
    }

    /* Call diag tx Interface */
    ret = hcc_send_device_data2diag_etc(buf, len);
    if (ret != EXT_ERR_SUCCESS) {
        hcc_diag_log_mem_free(queue_id, buf, user_param);
        return EXT_ERR_FAILURE;
    }

    /* free hcc mem */
    hcc_diag_log_mem_free(queue_id, buf, user_param);

    return EXT_ERR_SUCCESS;
}
#endif

static td_u32 hcc_oam_rx_proc(hcc_queue_type queue_id, td_u8 subtype,
    td_u8 *buf, td_u32 len, td_u8 *user_param)
{
    if (subtype == PLAT_MSG_TRANSFER) {
        return hcc_plat_host_rx_proc(queue_id, subtype, buf, len, user_param);
#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    } else {
        return hcc_diag_log_rx_proc_func_etc(queue_id, subtype, buf, len, user_param);
#endif
    }
}
#endif

static td_void hcc_diag_log_mem_free(hcc_queue_type queue_id, td_u8 *buf, td_u8 *user_param)
{
    osal_kfree(buf);
}

static td_u32 hcc_diag_log_mem_alloc(hcc_queue_type queue_id, td_u32 len, td_u8 **buf, td_u8 **user_param)
{
    *buf = (td_u8 *)osal_kmalloc(len, OSAL_GFP_ATOMIC);
    if (*buf == TD_NULL) {
        oal_print_err("hcc_diag_log_mem_alloc - len %d fail\r\n", len);
        return EXT_ERR_FAILURE;
    }
    *user_param = TD_NULL;
    return EXT_ERR_SUCCESS;
}

hcc_adapt_ops g_hcc_diag_adapt_ops = {
    .free = hcc_diag_log_mem_free,
    .alloc = hcc_diag_log_mem_alloc,
    .start_subq = TD_NULL,
    .stop_subq = TD_NULL,
#if defined(WSCFG_PLAT_DIAG_LOG_OUT)
    .rx_proc = hcc_oam_rx_proc,
#else
    .rx_proc = hcc_plat_host_rx_proc,
#endif
};

td_s32 hcc_diag_log_init(td_void)
{
    td_u32 ret;

    ret = hcc_service_init(HCC_CHANNEL_AP, HCC_ACTION_TYPE_OAM, &g_hcc_diag_adapt_ops);
    if (ret != EXT_ERR_SUCCESS) {
        oal_print_err("hcc diag log service ini failed, ret %d\n", ret);
        return EXT_ERR_FAILURE;
    }
    oal_print_err("hcc diag log service ini ok\n");

    return EXT_ERR_SUCCESS;
}

td_void hcc_diag_log_exit(td_void)
{
    hcc_service_deinit(HCC_CHANNEL_AP, HCC_ACTION_TYPE_OAM);
    return;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
