/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: 维测功能开关控制
 * Date: 2022-03-08
 */

#include "common_dft_rom.h"
#include "securec.h"
#include "diag_log_common.h"
#include "common_error_code_rom.h"
#if defined (_PRE_WLAN_PLAT_URANUS) || defined (_PRE_WLAN_PLAT_WS83)
#include "soc_zdiag.h"
#endif
#include "soc_osal.h"
#include "wlan_types_base_rom.h"
#include "wlan_types.h"
#include "eth_ip_common_rom.h"
#include "wlan_util_common_rom.h"
#include "mac_frame_common_rom.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_COMMON_DFT_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

#define DFT_MAX_FRAME_LEN 1600 /* 维测用，防止越界 */
/* OAM模块统一使用的全局操作变量上下文，包括OAM其他子模块全局上下文 */
dft_mng_ctx g_dft_mng_ctx = {0};
#if defined (_PRE_WLAN_PLAT_URANUS) || defined (_PRE_WLAN_PLAT_WS83)
osal_u32 g_report_num = 0;
#endif
osal_void dft_set_log_level(osal_u8 level)
{
    g_dft_mng_ctx.log_level = level;
}

/* 获取当前上报日志级别 */
oam_log_level_enum_uint8 dft_get_log_level_switch(osal_void)
{
    return g_dft_mng_ctx.log_level;
}

/* 获取所有帧上报总开关 */
osal_u8 dft_get_global_frame_switch(osal_void)
{
    return g_dft_mng_ctx.global_frame_switch;
}

/* 设置所有帧上报总开关 */
osal_u32 dft_set_global_frame_switch(osal_u8 log_switch)
{
    if (log_switch >= DFT_SWITCH_BUTT) {
        return OAL_FAIL;
    }
    g_dft_mng_ctx.global_frame_switch = log_switch;
    oam_warning_log1(0, OAM_SF_DFT, "{dft_set_global_frame_switch: switch[%d].}", g_dft_mng_ctx.global_frame_switch);
    return OAL_SUCC;
}

/* 获取帧上报开关 */
osal_u32 dft_report_80211_frame_get_switch(osal_u8 frame_mode, osal_u8 direction, osal_u8 track_frame_type,
    dft_80211_frame_ctx *frame_ctx)
{
    if (frame_mode >= FRAME_MODE_BUTT) {
        return OAL_ERR_CODE_CONFIG_EXCEED_SPEC;
    }

    if (direction >= OAM_OTA_FRAME_DIRECTION_TYPE_BUTT) {
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    if (track_frame_type >= OAM_USER_TRACK_FRAME_TYPE_BUTT) {
        return OAL_ERR_CODE_CONFIG_UNSUPPORT;
    }

    if (frame_ctx == OSAL_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (memcpy_s(frame_ctx, sizeof(dft_80211_frame_ctx),
        &g_dft_mng_ctx.user_track_ctx.frame_ctx[frame_mode].frame_type_ctx[direction][track_frame_type],
        sizeof(dft_80211_frame_ctx)) != EOK) {
        return OAL_FAIL;
    }
    return OAL_SUCC;
}

/* 设置帧上报开关 */
osal_u32 dft_report_80211_frame_set_switch(osal_u8 frame_mode, osal_u8 direction, osal_u8 track_frame_type,
    dft_80211_frame_ctx *frame_ctx)
{
    if (frame_mode >= FRAME_MODE_BUTT) {
        return OAL_ERR_CODE_CONFIG_EXCEED_SPEC;
    }

    if (direction >= OAM_OTA_FRAME_DIRECTION_TYPE_BUTT) {
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    if (track_frame_type >= OAM_USER_TRACK_FRAME_TYPE_BUTT) {
        return OAL_ERR_CODE_CONFIG_UNSUPPORT;
    }

    if ((frame_ctx->frame_content_switch >= DFT_SWITCH_BUTT) || (frame_ctx->frame_cb_switch >= DFT_SWITCH_BUTT) ||
        (frame_ctx->frame_dscr_switch >= DFT_SWITCH_BUTT)) {
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    if (memcpy_s(&g_dft_mng_ctx.user_track_ctx.frame_ctx[frame_mode].frame_type_ctx[direction][track_frame_type],
        sizeof(dft_80211_frame_ctx), frame_ctx, sizeof(dft_80211_frame_ctx)) != EOK) {
        return OAL_FAIL;
    }
    return OAL_SUCC;
}

osal_void dft_report_to_hso(const osal_u8 *data, osal_u16 len, osal_u32 msg_id)
{
#if defined (_PRE_WLAN_PLAT_URANUS) || defined (_PRE_WLAN_PLAT_WS83)
    osal_s32 ret;
    ret = (osal_s32)uapi_diag_report_sys_msg(THIS_MOD_ID, msg_id, data, len, DIAG_LOG_LEVEL_WARN);
    if (ret != 0) {
        oam_warning_log3(0, OAM_SF_DFT, "{dft_report_to_hso:: msg_id[0x%x] len[%d] ret[%d]}", msg_id, len, ret);
    } else {
        if (g_report_num == 0xFFFFFFFF) { /* 0xFFFFFFFF is osal_u32 max value */
            g_report_num = 0;
        } else {
            g_report_num++;
        }
    }
#endif
    return;
}

osal_void dft_report_signal_addr(const osal_u8 *data, osal_u16 len, osal_u32 msg_id)
{
    osal_u8 *buf = OSAL_NULL;
    osal_u16 dst_len = DFT_MAX_FRAME_LEN;

    if (data == OSAL_NULL) {
        oam_warning_log0(0, OAM_SF_DFT, "{dft_report_signal_addr:data is NULL.}");
        return;
    }
    if (msg_id <= SOC_DIAG_MSG_ID_WIFI_FRAME_MAX) {
        if (dft_get_global_frame_switch() != DFT_SWITCH_ON) {
            return;
        }
    }

    buf = osal_kmalloc(DFT_MAX_FRAME_LEN, 0);
    if (buf == OSAL_NULL) {
        oam_warning_log0(0, OAM_SF_DFT, "{dft_report_signal_addr:buf is NULL.}");
        return;
    }
    (osal_void)memset_s(buf, DFT_MAX_FRAME_LEN, 0, DFT_MAX_FRAME_LEN);

    len = (len > DFT_MAX_FRAME_LEN) ? DFT_MAX_FRAME_LEN : len;
    if (memcpy_s(buf, dst_len, data, len) != EOK) {
        oam_warning_log0(0, OAM_SF_DFT, "{dft_report_signal_addr:memcpy_s data error.}");
        osal_kfree(buf);
        return;
    }
    dft_report_to_hso(buf, len, msg_id);
    osal_kfree(buf);
}

osal_void dft_report_two_addr(const osal_u8 *hdr_addr, osal_u8 hdr_len,
    const osal_u8 *frame_addr, osal_u16 frame_len, adjust_hso_param_stru *adjust_hso_param)
{
    osal_u8 *buf = OSAL_NULL;
    osal_u16 dst_len = DFT_MAX_FRAME_LEN;

    if ((hdr_addr == OSAL_NULL) || (frame_addr == OSAL_NULL) || (hdr_len >= DFT_MAX_FRAME_LEN) ||
        (adjust_hso_param == OSAL_NULL) || (frame_len < hdr_len)) {
        oam_warning_log1(0, OAM_SF_DFT,
            "{dft_report_two_addr:hdr_add or frame_addr or adjust_hso_param = NULL or hdr_len[%d] error.}", hdr_len);
        return;
    }
    if (adjust_hso_param->msg_id <= SOC_DIAG_MSG_ID_WIFI_FRAME_MAX) {
        if (dft_get_global_frame_switch() != DFT_SWITCH_ON) {
            return;
        }
    }

    buf = osal_kmalloc(DFT_MAX_FRAME_LEN, 0);
    if (buf == OSAL_NULL) {
        oam_warning_log0(0, OAM_SF_DFT, "{dft_report_two_addr:buf is NULL.}");
        return;
    }
    (osal_void)memset_s(buf, DFT_MAX_FRAME_LEN, 0, DFT_MAX_FRAME_LEN);

    /* 检查mac帧总长度合法性 */
    frame_len = (frame_len > DFT_MAX_FRAME_LEN) ? DFT_MAX_FRAME_LEN : frame_len;

    if (memcpy_s(buf, dst_len, hdr_addr, hdr_len) != EOK) {
        oam_warning_log0(0, OAM_SF_DFT, "{dft_report_two_addr:memcpy_s hdr_addr error.}");
        osal_kfree(buf);
        return;
    }
    /* 判断是否需要填充rssi 和 channel */
    if (adjust_hso_param->is_probe_rsp_or_beacon) {
        buf[2] = adjust_hso_param->rssi; /* mac头第2个字节填充rssi */
        buf[3] = adjust_hso_param->channel; /* mac头第3个字节填充channel */
    }
    if (memcpy_s(buf + hdr_len, dst_len - hdr_len, frame_addr, frame_len - hdr_len) != EOK) {
        oam_warning_log0(0, OAM_SF_DFT, "{dft_report_two_addr:memcpy_s hdr_addr error.}");
        osal_kfree(buf);
        return;
    }
    dft_report_to_hso(buf, frame_len, adjust_hso_param->msg_id);
    osal_kfree(buf);
}

/* 上报80211 frame中的CB字段 */
osal_void dft_report_netbuf_cb_etc(osal_u8 *netbuf_cb, osal_u16 len, osal_u32 msg_id)
{
    dft_report_signal_addr(netbuf_cb, len, msg_id);
}

/* 上报80211 frame */
osal_void dft_report_80211_frame(osal_u8 *hdr_addr, osal_u8 hdr_len, const osal_u8 *frame_addr, osal_u16 frame_len,
    adjust_hso_param_stru *adjust_hso_param)
{
    dft_report_two_addr(hdr_addr, hdr_len, frame_addr, frame_len, adjust_hso_param);
}

/*
 * 向SDT上报描述符，包括发送描述符和接收描述符。
 * rx描述符的最大长度为64字节，tx描述符的最大长度为256字节。
 * 如果dscr_len大于256，则入参错误，直接返回
 */
osal_void dft_report_dscr_etc(osal_u8 *dscr_addr, osal_u16 dscr_len, osal_u32 msg_id)
{
    dft_report_signal_addr(dscr_addr, dscr_len, msg_id);
}

osal_void dft_report_params_etc(osal_u8 *param, osal_u16 param_len, osal_u32 msg_id)
{
    dft_report_signal_addr(param, param_len, msg_id);
}

osal_void dft_report_eth_frame_etc(osal_u8 *eth_frame, osal_u16 eth_frame_len, osal_u32 msg_id)
{
    dft_report_signal_addr(eth_frame, eth_frame_len, msg_id);
}

osal_u32 dft_get_frame_mode(osal_u8 *hdr)
{
    struct mac_ieee80211_frame *mac_hdr = OSAL_NULL;
    osal_u8 *address = OSAL_NULL;
    osal_u8 is_tods;
    osal_u8 is_fromds;

    if (hdr == OSAL_NULL) {
        return FRAME_MODE_BUTT;
    }
    mac_hdr = (struct mac_ieee80211_frame *)hdr;
    is_tods = mac_hdr_get_to_ds(hdr);
    is_fromds = mac_hdr_get_from_ds(hdr);
    if (!is_tods && !is_fromds) {
        /* IBSS */
        address = mac_hdr->address1;
    } else if (is_tods) {
        /* to AP */
        address = mac_hdr->address3;
    } else if (is_fromds) {
        /* from AP */
        address = mac_hdr->address1;
    }

    return (address[0] & 1) != 0 ? BROADCAST_FRAME : UNICAST_FRAME;
}

osal_void dft_get_frame_report_switch(osal_u8 *hdr, osal_u8 direction, dft_80211_frame_ctx *frame_ctx)
{
    osal_u8 frame_type;
    osal_u32 frame_mode;
    osal_u32 frame_sub_type;
    osal_u32 ret;

    if (hdr == OSAL_NULL) {
        oam_warning_log0(0, OAM_SF_DFT, "{dft_get_frame_report_switch:hdr is NULL.}");
        return;
    }

    if (direction >= OAM_OTA_FRAME_DIRECTION_TYPE_BUTT) {
        oam_warning_log0(0, OAM_SF_DFT, "{dft_get_frame_report_switch:direction is invalid.}");
        return;
    }

    if (frame_ctx == OSAL_NULL) {
        oam_warning_log0(0, OAM_SF_DFT, "{dft_get_frame_report_switch:frame_ctx is NULL.}");
        return;
    }

    frame_type = mac_get_frame_type(hdr);
    frame_mode = dft_get_frame_mode(hdr);
    frame_sub_type = mac_get_frame_sub_type(hdr);
    /* 屏蔽beacon帧 */
    if (frame_type == WLAN_FC0_TYPE_MGT && frame_sub_type == WLAN_FC0_SUBTYPE_BEACON) {
        return;
    }

    frame_type = (frame_type == WLAN_FC0_TYPE_MGT) ? OAM_USER_TRACK_FRAME_TYPE_MGMT : OAM_USER_TRACK_FRAME_TYPE_DATA;
    ret = dft_report_80211_frame_get_switch(frame_mode, direction, frame_type, frame_ctx);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_DFT, "{dft_get_frame_report_switch:get frame_ctx fail[%d].}", ret);
        return;
    }
}
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#ifdef CONFIG_ENABLE_ROM_PATCH
#include "ram_malloc.h"
#include "patch_section.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define DFT_REPORT_MEM_WATER_LINE 0x2000
PATCH_T osal_void dft_report_to_hso_patch(const osal_u8 *data, osal_u16 len, osal_u32 msg_id)
{
#if defined (_PRE_WLAN_PLAT_URANUS) || defined (_PRE_WLAN_PLAT_WS83)
    osal_s32 ret;

    if (get_free_size() <= DFT_REPORT_MEM_WATER_LINE) {
        g_report_num++; // 统计达水线丢包数
        return;
    }

    ret = (osal_s32)uapi_diag_report_sys_msg(THIS_MOD_ID, msg_id, data, len, DIAG_LOG_LEVEL_WARN);
    if (ret != 0) {
        oam_warning_log3(0, OAM_SF_DFT, "{dft_report_to_hso_patch:: msg_id[0x%x] len[%d] ret[%d]}", msg_id, len, ret);
    }
#endif
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif // CONFIG_ENABLE_ROM_PATCH