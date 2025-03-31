/*
 * Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
 * Description: zdiag for sample data.
 */
#ifdef CONFIG_SUPPORT_SAMPLE_DATA
#include "diag_sample_data.h"
#include "td_base.h"
#include "oal_ext_util.h"
#include "soc_zdiag.h"
#include "soc_osal.h"
#include "oal_debug.h"
#include "oam_ext_if.h"
#include "zdiag_log_util.h"
#include "zdiag_adapt_os.h"
#include "soc_diag_cmd_id.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_DIAG_SAMPLE_DATA_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_OAM_HOST

#if defined(WSCFG_PLAT_DIAG_LOG_OUT)
#define DIAG_SAMPLE_TRANSMIT_FINISH 11

diag_sample_func_cb g_diag_sample_cb[ZDIAG_SAMPLE_CB_END] = {TD_NULL};
diag_sample_record g_sample_running = {0};

ext_errno diag_cmd_start_sample_data(td_u16 cmd_id, td_pvoid cmd_param, td_u16 cmd_param_size, diag_option *option)
{
    diag_btsoc_sample_cmd *cmd = (diag_btsoc_sample_cmd *)cmd_param;
    td_u32 idx = cmd_id - DIAG_CMD_ID_BSLE_SAMPLE;
    diag_sample_ack ack = {EXT_ERR_FAILURE, 0, 0};
    diag_sample_func_cb func_cb = (diag_sample_func_cb)g_diag_sample_cb[idx];

    if (func_cb == TD_NULL) {
        oam_error_log1(0, 0, "diag_cmd_start_sample_data:sample func_cb is NULL[%d].", idx);
        goto report_ack;
    }

    oal_print_err("diag_cmd_start_sample_data:flag:%d, transmit_id:%d, sample_size:%d, sample_type:%d, msg_cnt:%d.\n",
        cmd->flag, cmd->transmit_id, cmd->sample_size, cmd->sample_type, g_sample_running.msg_cnt);

    if (g_sample_running.running == OSAL_FALSE && cmd->flag == ZDIAG_SAMPLE_START) { /* 启动数采 */
        g_sample_running.running = OSAL_TRUE;
        ack.ret = func_cb(cmd_param, cmd_param_size);
    } else if (g_sample_running.running == OSAL_TRUE && cmd->flag == ZDIAG_SAMPLE_STOP && /* 停止数采 */
        g_sample_running.msg_cnt == 0) {
        g_sample_running.running = OSAL_FALSE;
        ack.ret = func_cb(cmd_param, cmd_param_size);
    } else {
        oam_warning_log3(0, 0, "diag_cmd_start_sample_data:sample_running:%d, sample_start:%d, msg_cnt:%d.\n",
            g_sample_running.running, cmd->flag, g_sample_running.msg_cnt);
        goto report_ack;
    }

    g_sample_running.cmd_id = cmd_id;
    g_sample_running.transmit_id = cmd->transmit_id;
    g_sample_running.offset = 0;
    g_sample_running.msg_cnt = 0;

report_ack:
    ack.flag = cmd->flag;
    ack.transmit_id = cmd->transmit_id;
    uapi_zdiag_report_packet(cmd_id, option, (td_u8 *)&ack, sizeof(diag_sample_ack), TD_TRUE);
    return EXT_ERR_SUCCESS;
}

ext_errno diag_cmd_report_data(td_u8 *buf, td_u32 len)
{
    diag_sample_reply_pkt *reply = TD_NULL;
    diag_option option = {0};
    td_u32 reply_size = (td_u32)sizeof(diag_sample_reply_pkt) + len;
    reply = (diag_sample_reply_pkt *)diag_adapt_malloc(reply_size);
    if (reply == TD_NULL || buf == TD_NULL) {
        return EXT_ERR_FAILURE;
    }
    g_sample_running.offset += len;
    reply->transmit_id = g_sample_running.transmit_id;
    reply->offset = g_sample_running.offset;
    reply->size = len;
    reply->ret = EXT_ERR_SUCCESS;
    if (memcpy_s(reply->data, len, buf, len) != EOK) {
        reply->ret = EXT_ERR_FAILURE;
        oam_error_log1(0, 0, "diag_cmd_report_data:memcpy_s error, len:%d.\n", len);
    }
    uapi_zdiag_report_packet(DIAG_CMD_ID_SAMPLE_DATA, &option, (td_u8 *)reply, reply_size, TD_TRUE);
    diag_adapt_free(reply);
    return EXT_ERR_SUCCESS;
}

ext_errno diag_cmd_report_finish(td_void)
{
    diag_option option = {0};
    diag_sample_notify finish = {0};
    g_sample_running.running = OSAL_FALSE;
    finish.transmit_id = g_sample_running.transmit_id;
    finish.state_code = DIAG_SAMPLE_TRANSMIT_FINISH;
    oal_print_err("diag_cmd_report_finish:transmit_id:%d, report_msg:%d.\n",
        finish.transmit_id, g_sample_running.msg_cnt);
    uapi_zdiag_report_packet(DIAG_CMD_ID_SAMPLE_FINISH, &option, (td_u8 *)&finish, sizeof(diag_sample_notify), TD_TRUE);
    return EXT_ERR_SUCCESS;
}

/**************************************************************
 功能描述: 上报wlan数采数据到hso的message显示界面，可点击查看
 buf：上报wlan数采数据
 len: 数据对应字节长度
***************************************************************/
ext_errno diag_report_wlan_sample_data(TD_CONST td_u8 *buf, td_u32 len, td_u32 msg_id)
{
    if (len == 0) {
        return diag_cmd_report_finish();
    }
    g_sample_running.msg_cnt++;
    return uapi_diag_report_sys_msg(THIS_MOD_ID, msg_id, buf, len, DIAG_LOG_LEVEL_WARN);
}

/**************************************************************
 功能描述: 上报bsle/soc数采数据到hso写bin文件
 buf：上报数采数据
 len: 数据对应字节长度；为0时表示数采上报结束信号
***************************************************************/
ext_errno diag_cmd_report_sample_data(td_u8 *buf, td_u32 len)
{
    if (len == 0) {
        return diag_cmd_report_finish();
    }
    g_sample_running.msg_cnt++;
    return diag_cmd_report_data(buf, len);
}

/**************************************************************
 功能描述: 注册bsle/soc/wlan数采配置启动命令接口
 idx：  0：bsle数采；1：wifi数采；2：soc数采
 func: 数采配置参数命令下发回调函数
***************************************************************/
td_void diag_sample_data_register(diag_sample_cb_enum idx, diag_sample_func_cb func)
{
    if (idx >= ZDIAG_SAMPLE_CB_END) {
        return;
    }
    g_diag_sample_cb[idx] = func;
}

#else
ext_errno diag_cmd_report_sample_data(td_u8 *buf, td_u32 len)
{
    uapi_unused(buf);
    uapi_unused(len);
    return EXT_ERR_SUCCESS;
}

td_void diag_sample_data_register(diag_sample_cb_enum idx, diag_sample_func_cb func)
{
    uapi_unused(idx);
    uapi_unused(func);
}

ext_errno diag_report_wlan_sample_data(TD_CONST td_u8 *buf, td_u32 len, td_u32 msg_id)
{
    uapi_unused(buf);
    uapi_unused(len);
    uapi_unused(msg_id);
    return EXT_ERR_SUCCESS;
}
#endif

EXPORT_SYMBOL(diag_report_wlan_sample_data);
EXPORT_SYMBOL(diag_cmd_report_sample_data);
EXPORT_SYMBOL(diag_sample_data_register);
#endif
