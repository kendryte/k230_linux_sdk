/*
 * Copyright (c) CompanyNameMagicTag 2021-2022. All rights reserved.
 * Description: diag filter
 * This file should be changed only infrequently and with great care.
 */
#include "zdiag_filter.h"
#include "securec.h"
#include "zdiag_adapt_layer.h"

#define ZDIAG_FILTER_MAX_LEVEL 8
#define ZDIAG_STATE_CONNECT_NOTIFY_CNT 2

#if defined DIAG_FILTER_BSS_SECTION
TD_PRV DIAG_FILTER_BSS_SECTION zdiag_filter_ctrl g_zdiag_filter_ctrl;
#else
TD_PRV zdiag_filter_ctrl g_zdiag_filter_ctrl;
#endif

TD_PRV zdiag_filter_notify_hook g_zdiag_notify_process[ZDIAG_STATE_CONNECT_NOTIFY_CNT];

zdiag_filter_ctrl *zdiag_get_filter_ctrl(td_void)
{
    return &g_zdiag_filter_ctrl;
}

zdiag_addr zdiag_get_connect_tool_addr(td_void)
{
    zdiag_filter_ctrl *ctrl = zdiag_get_filter_ctrl();
    return ctrl->tool_addr;
}

td_void zdiag_set_enable(td_bool enable, zdiag_addr addr)
{
    int i;
    zdiag_filter_ctrl *ctrl = zdiag_get_filter_ctrl();
    zdiag_printf("zdiag_set_enable %d\r\n", enable);
    memset_s(ctrl, sizeof(zdiag_filter_ctrl), 0x0, sizeof(zdiag_filter_ctrl));
    if (enable) {
        ctrl->enable = enable;
        ctrl->tool_addr = addr;
        g_zdiag_filter_ctrl.last_rev_time = diag_adapt_get_cur_second();
    } else {
        memset_s(ctrl, sizeof(zdiag_filter_ctrl), 0x0, sizeof(zdiag_filter_ctrl));
    }
    for (i = 0; i < ZDIAG_STATE_CONNECT_NOTIFY_CNT; i++) {
        if (g_zdiag_notify_process[i] != TD_NULL) {
            (td_void)g_zdiag_notify_process[i](enable);
        }
    }
}

td_bool zdiag_is_enable(td_void)
{
    zdiag_filter_ctrl *ctrl = zdiag_get_filter_ctrl();
    return ctrl->enable;
}

td_void zdiag_set_level_enable(td_u8 level, td_bool enable)
{
    zdiag_filter_ctrl *ctrl = zdiag_get_filter_ctrl();
    zdiag_printf("utils zdiag_set_level_enable %d %d\r\n", level, enable);
    if (level >= ZDIAG_FILTER_MAX_LEVEL) {
        return;
    }

    if (enable) {
        ctrl->level |= (td_u8)(1 << level);
    } else {
        ctrl->level &= (td_u8)(~(1 << level));
    }
}

td_void zdiag_set_id_enable(td_u32 id, td_bool enable)
{
    td_u32 i;
    td_s32 free_idx = -1;
    td_s32 match_idx = -1;

    zdiag_filter_ctrl *ctrl = zdiag_get_filter_ctrl();
    for (i = 0; i < ZDIAG_FILTER_GROUP_NUM; i++) {
        if (ctrl->enable_id[i] == id) {
            match_idx = (td_s32)i;
            break;
        }
        if (ctrl->enable_id[i] == 0 && free_idx == -1) {
            free_idx = (td_s32)i;
        }
    }
    zdiag_printf("zdiag_set_id_enable %d %d %d %d\r\n", id, enable, match_idx, free_idx);

    if (match_idx != -1 && enable) {
        return;
    }

    if (match_idx != -1 && !enable) {
        ctrl->enable_id[match_idx] = 0;
        return;
    }

    if (free_idx != -1 && enable) {
        ctrl->enable_id[free_idx] = (td_u16)id;
    }
    return;
}

td_bool zdiag_log_enable(td_u8 level, td_u32 id)
{
    td_u32 i;
    zdiag_filter_ctrl *ctrl = zdiag_get_filter_ctrl();
    if (!ctrl->enable) {
        zdiag_printf("un_enable_a %d %d\r\n", level, id);
        return TD_FALSE;
    }

    if (level >= ZDIAG_FILTER_MAX_LEVEL) {
        zdiag_printf("un_enable_b %d %d\r\n", level, id);
        return TD_FALSE;
    }

    if (!((1 << level) & ctrl->level)) {
        zdiag_printf("un_enable_c %d %d ctrl->level %d\r\n", level, id, ctrl->level);
        return TD_FALSE;
    }

    for (i = 0; i < ZDIAG_FILTER_GROUP_NUM; i++) {
        if (ctrl->enable_id[i] == id) {
            zdiag_printf("en_enable_d %d %d\r\n", level, id);
            return TD_TRUE;
        }
    }
    zdiag_printf("un_enable_e %d %d\r\n", level, id);
    return TD_FALSE;
}

td_void zdiag_filter_register_notify_hook(zdiag_filter_notify_hook hook)
{
    int i;
    for (i = 0; i < ZDIAG_STATE_CONNECT_NOTIFY_CNT; i++) {
        if (g_zdiag_notify_process[i] == TD_NULL) {
            g_zdiag_notify_process[i] = hook;
            break;
        }
    }
}

td_void zdiag_state_beat_heart_pkt_recv(zdiag_addr peer_addr)
{
    if (g_zdiag_filter_ctrl.tool_addr == peer_addr) {
        g_zdiag_filter_ctrl.last_rev_time = diag_adapt_get_cur_second();
    }
}

td_u32 zdiag_state_get_last_recv_time(zdiag_addr peer_addr)
{
    if (g_zdiag_filter_ctrl.tool_addr == peer_addr) {
        return g_zdiag_filter_ctrl.last_rev_time;
    }
    return 0;
}

td_void zdiag_filter_init(td_void)
{
    memset_s(&g_zdiag_filter_ctrl, sizeof(zdiag_filter_ctrl), 0, sizeof(zdiag_filter_ctrl));
}
