/*
 * Copyright (c) CompanyNameMagicTag 2021-2021. All rights reserved.
 * Description: diag oam log
 * This file should be changed only infrequently and with great care.
 */
#include "diag_oam_log.h"
#include "soc_zdiag.h"

#define OAM_LOG_LEVEL_MASK 0x0F
TD_PRV ext_errno oam_log_msg_print(td_u32 msg_id, td_u32 mod_id, td_u8 *buf, td_u32 size)
{
    td_u32 level = msg_id & OAM_LOG_LEVEL_MASK;
    /* does't check if the buffer is null. because when param count is 0,the buffer is null. */
    return uapi_diag_report_sys_msg(mod_id, msg_id, buf, (td_u16)size, (td_u8)level);
}

ext_errno oam_log_print0_press_prv(td_u32 msg_id, td_u32 mod_id)
{
    return oam_log_msg_print(msg_id, mod_id, TD_NULL, 0);
}

ext_errno oam_log_print1_press_prv(td_u32 msg_id, td_u32 mod_id, td_u32 param_1)
{
    zdiag_log_msg1 msg;
    msg.data0 = param_1;
    return oam_log_msg_print(msg_id, mod_id, (td_u8 *)&msg, sizeof(zdiag_log_msg1));
}

ext_errno oam_log_print2_press_prv(td_u32 msg_id, td_u32 mod_id, td_u32 param_1, td_u32 param_2)
{
    zdiag_log_msg2 msg;
    msg.data0 = param_1;
    msg.data1 = param_2;
    return oam_log_msg_print(msg_id, mod_id, (td_u8 *)&msg, sizeof(zdiag_log_msg2));
}

ext_errno oam_log_print3_press_prv(td_u32 msg_id, td_u32 mod_id, zdiag_log_msg3 *olm)
{
    zdiag_log_msg3 msg;
    msg.data0 = olm->data0;
    msg.data1 = olm->data1;
    msg.data2 = olm->data2;
    return oam_log_msg_print(msg_id, mod_id, (td_u8 *)&msg, sizeof(zdiag_log_msg3));
}

ext_errno oam_log_print4_press_prv(td_u32 msg_id, td_u32 mod_id, zdiag_log_msg4 *olm)
{
    zdiag_log_msg4 msg;
    msg.data0 = olm->data0;
    msg.data1 = olm->data1;
    msg.data2 = olm->data2;
    msg.data3 = olm->data3;
    return oam_log_msg_print(msg_id, mod_id, (td_u8 *)&msg, sizeof(zdiag_log_msg4));
}

ext_errno oam_log_print_buff_press_prv(td_u32 msg_id, td_u32 mod_id, td_u8 *data, td_u32 data_size)
{
    return oam_log_msg_print(msg_id, mod_id, data, data_size);
}
