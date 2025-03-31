/*
 * Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
 * Description: hmac bsrp nfrp
 * Create: 2023-2-17
 */

#include "hmac_bsrp_nfrp.h"
#include "hmac_user.h"
#include "mac_resource_ext.h"
#include "diag_log_common.h"
#include "frw_util_notifier.h"
#include "frw_hmac.h"
#include "hmac_feature_interface.h"
#include "hmac_ccpriv.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

mac_bsrp_info_stru g_bsrp_info;
mac_nfrp_info_stru g_nfrp_info;

OSAL_STATIC mac_bsrp_info_stru *hmac_get_bsrp_info(osal_void)
{
    return &g_bsrp_info;
}

OSAL_STATIC mac_nfrp_info_stru *hmac_get_nfrp_info(osal_void)
{
    return &g_nfrp_info;
}

#ifdef _PRE_WLAN_FEATURE_CENTRALIZE
static osal_void hmac_get_device_user_tid_pkt_stat(hmac_vap_stru *hmac_vap,
    osal_u16 user_id, mac_user_tid_pkt_stat_stru *tid_pkt_stat)
{
    frw_msg msg = {0};
    osal_s32 ret;

    frw_msg_init((osal_u8 *)&user_id, sizeof(user_id), (osal_u8 *)tid_pkt_stat,
        sizeof(mac_user_tid_pkt_stat_stru), &msg);
    ret = frw_send_msg_to_device(hmac_vap->vap_id, WLAN_MSG_H2D_C_CFG_GET_TID_PKT_STAT, &msg, OSAL_TRUE);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_EDCA, "{hmac_get_device_user_tid_pkt_stat:: send msg to device fail=%d!", ret);
    }
    return;
}
#endif

/*****************************************************************************
 功能描述  : 统计tid队列中数据字节数并配置ndp反馈状态
*****************************************************************************/
OSAL_STATIC osal_void hmac_nfrp_set_ndp_feedback_status(hmac_vap_stru *hmac_vap,
    hmac_user_stru *hmac_user, mac_user_tid_pkt_stat_stru *tid_pkt_stat)
{
    osal_u32 buffer_len = 0;
    osal_u8 tid;
    osal_u8 ndp_status;
    mac_nfrp_info_stru *nfrp_info = hmac_get_nfrp_info();

    if (mac_mib_get_support_response_nfrp_trigger(hmac_vap) == OSAL_TRUE) {
        return;
    }

    for (tid = WLAN_TIDNO_BEST_EFFORT; tid < WLAN_TID_MAX_NUM; tid++) {
        buffer_len += (hmac_user->tx_tid_queue[tid].tid_buffer_frame_len + tid_pkt_stat->tid_pkt_len[tid] +
            MAC_80211_QOS_FRAME_LEN * (hmac_user->tx_tid_queue[tid].mpdu_num + tid_pkt_stat->tid_pkt_num[tid]));
    }
    /* x入参为2 */
    ndp_status = (buffer_len > (osal_u32)(oal_pow(2, nfrp_info->res_req_buf_threshold_exponent))) ? 1 : 0;
    if (ndp_status != nfrp_info->ndp_status) {
        hal_set_ndp_status(ndp_status);
        nfrp_info->ndp_status = ndp_status;
    }
}

OSAL_STATIC osal_void hmac_nfrp_sta_update_param(osal_u8 *payload, osal_u16 msg_len, hmac_vap_stru *hmac_vap,
    osal_u8 frame_sub_type, hmac_user_stru *hmac_user)
{
    osal_u8 req_buf_threshold = 0;
    osal_u8 *ie = OSAL_NULL;
    mac_nfrp_info_stru *nfrp_info = hmac_get_nfrp_info();

    if (hmac_vap == OAL_PTR_NULL || hmac_user == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_ASSOC, "{hmac_sta_update_nfr_param::ptr is NULL!}");
        return;
    }

    if (mac_mib_get_he_option_implemented(hmac_vap) == OAL_FALSE ||
        mac_mib_get_support_response_nfrp_trigger(hmac_vap) == OAL_FALSE) {
        return;
    }

    /************ NDP Feedback Report Parameter Set Element ****************/
    /* ------------------------------------------------------------------- */
    /* | EID | LEN | Ext EID|Resource Request Buffer Threshold Exponent |  */
    /* ------------------------------------------------------------------- */
    /* |  1  |  1  |   1    |                     1                     |  */
    /* ------------------------------------------------------------------- */
    ie = hmac_find_ie_ext_ie(MAC_EID_HE, MAC_EID_EXT_NFR_PARAM_SET, payload, msg_len);
    if (ie != OSAL_NULL) {
        if (ie[1] != MAC_HE_NFR_PARAM_PAYLOAD_LEN) {
            oam_error_log0(0, OAM_SF_ASSOC, "{hmac_sta_update_nfr_param::parse nfr fail!}");
            return;
        }
        req_buf_threshold = *(ie + MAC_HE_NFR_PARAM_HEADER_LEN);
    }

    /* beacon帧只在threshold变化时才更新 */
    if ((frame_sub_type == WLAN_FC0_SUBTYPE_BEACON) &&
        (nfrp_info->res_req_buf_threshold_exponent == req_buf_threshold)) {
        return;
    }
    /* assoc rsp帧不包含threshold信息, 则使用默认值 256 = 2^8 */
    if (frame_sub_type == WLAN_FC0_SUBTYPE_ASSOC_RSP && ie == NULL) {
        nfrp_info->res_req_buf_threshold_exponent = MAC_HE_NFR_PARAM_DEFAULT_VAL;
    } else {
        nfrp_info->res_req_buf_threshold_exponent = req_buf_threshold;
    }

    return;
}

/*****************************************************************************
 函 数 名  : hmac_tid_queue_size_calc
 功能描述  : 通过协议9-0a公式，根据缓存帧总长度计算出Queue Size subfield
*****************************************************************************/
OSAL_STATIC osal_u8 hmac_bsrp_calc_queue_size_by_frame_len(osal_u32 ac_frame_len)
{
    osal_u8 scaling_factor;
    osal_u8 unscaled_value;
    osal_u8 queue_size_subfield;

    /* QS等于0情况 */
    if (ac_frame_len == SF0_MIN_SIZE) {
        scaling_factor = SCALING_FACTOR0;
        unscaled_value = UNSCALED_VALUE0;
        /* QS大于0或小于等于1008(1024-16)情况 */
    } else if (ac_frame_len > SF0_MIN_SIZE && ac_frame_len <= (SF1_MIN_SIZE - SF0_SIZE_UNITS)) {
        scaling_factor = SCALING_FACTOR0;
        unscaled_value = (osal_u8)((osal_round_up(ac_frame_len, SF0_SIZE_UNITS)) / SF0_SIZE_UNITS);
        /* QS大于1008或小于等于1024情况 */
    } else if (ac_frame_len > (SF1_MIN_SIZE - SF0_SIZE_UNITS) && ac_frame_len <= SF1_MIN_SIZE) {
        scaling_factor = SCALING_FACTOR1;
        unscaled_value = UNSCALED_VALUE0;
        /* QS大于1024或小于等于17152(17408-256)情况 */
    } else if (ac_frame_len > SF1_MIN_SIZE && ac_frame_len <= (SF2_MIN_SIZE - SF1_SIZE_UNITS)) {
        scaling_factor = SCALING_FACTOR1;
        unscaled_value = (osal_u8)((osal_round_up(ac_frame_len - SF1_MIN_SIZE, SF1_SIZE_UNITS)) / SF1_SIZE_UNITS);
        /* QS大于17152或小于等于17408情况 */
    } else if (ac_frame_len > (SF2_MIN_SIZE - SF1_SIZE_UNITS) && ac_frame_len <= SF2_MIN_SIZE) {
        scaling_factor = SCALING_FACTOR2;
        unscaled_value = UNSCALED_VALUE0;
        /* QS大于17408或小于等于146432(148480-2048)情况 */
    } else if (ac_frame_len > SF2_MIN_SIZE && ac_frame_len <= (SF3_MIN_SIZE - SF2_SIZE_UNITS)) {
        scaling_factor = SCALING_FACTOR2;
        unscaled_value = (osal_u8)((osal_round_up(ac_frame_len - SF2_MIN_SIZE, SF2_SIZE_UNITS)) / SF2_SIZE_UNITS);
        /* QS大于146432或小于等于148480情况 */
    } else if (ac_frame_len > (SF3_MIN_SIZE - SF2_SIZE_UNITS) && ac_frame_len <= SF3_MIN_SIZE) {
        scaling_factor = SCALING_FACTOR3;
        unscaled_value = UNSCALED_VALUE0;
        /* QS大于148480或小于等于2147328情况 */
    } else if (ac_frame_len > SF3_MIN_SIZE && ac_frame_len <= SF3_MAX_SIZE) {
        scaling_factor = SCALING_FACTOR3;
        unscaled_value = (osal_u8)((osal_round_up(ac_frame_len - SF3_MIN_SIZE, SF3_SIZE_UNITS)) / SF3_SIZE_UNITS);
        /* QS大于2147328情况 */
    } else if (ac_frame_len > SF3_MAX_SIZE) {
        scaling_factor = SCALING_FACTOR3;
        unscaled_value = UNSCALED_VALUE_MAX;
        /* Unspecified or unknown */
    } else {
        wifi_printf("should never here, ac_frame_len = %u\r\n", ac_frame_len);
        scaling_factor = SCALING_FACTOR3;
        unscaled_value = UNSCALED_VALUE_UNKNOWN;
    }
    queue_size_subfield = ((scaling_factor << SF_OFFSET) | unscaled_value);
    return queue_size_subfield;
}

/*****************************************************************************
 函 数 名  : hmac_ac_buffer_status_calc
 功能描述  : 将AC队列帧长通过公式计算得到Queue Size， 并写入MAC寄存器
*****************************************************************************/
OSAL_STATIC osal_u8 hmac_bsrp_calc_queue_size(const hmac_user_stru *hmac_user,
    wlan_wme_ac_type_enum_uint8 ac, mac_user_tid_pkt_stat_stru *tid_pkt_stat)
{
    osal_u32 ac_frame_len;
    osal_u32 dbg_size;
    osal_u8 queue_size;    /* Queue Size子字段 */
    osal_u8 bsrp_tid;      /* BSR反馈的QoS Control字段中的tid: 与认证平台一致 0 1 5 7 */
    osal_u8 tid;           /* 某个AC业务对应的两个TID之一 */
    mac_bsrp_info_stru *bsrp_info = hmac_get_bsrp_info();

    /* 统计BE BK VI VO业务缓存帧长(将业务对应的TID队列帧长相加) */
    switch (ac) {
        case WLAN_WME_AC_BE:
            bsrp_tid = WLAN_TIDNO_BEST_EFFORT;
            tid = WLAN_TIDNO_ANT_TRAINING_LOW_PRIO;
            break;
        case WLAN_WME_AC_BK:
            bsrp_tid = WLAN_TIDNO_BACKGROUND;
            tid = WLAN_TIDNO_UAPSD;
            break;
        case WLAN_WME_AC_VI:
            bsrp_tid = WLAN_TIDNO_VIDEO;
            tid = WLAN_TIDNO_ANT_TRAINING_HIGH_PRIO;
            break;
        case WLAN_WME_AC_VO:
            bsrp_tid = WLAN_TIDNO_VOICE;
            tid = WLAN_TIDNO_BCAST;
            break;
        default:
            oam_error_log1(0, OAM_SF_ANY, "hmac_ac_buffer_status_calc::invalie ac=%d!", ac);
            return 0;
    }

    ac_frame_len = hmac_user->tx_tid_queue[bsrp_tid].tid_buffer_frame_len + tid_pkt_stat->tid_pkt_len[bsrp_tid] +
         MAC_80211_QOS_FRAME_LEN * (hmac_user->tx_tid_queue[bsrp_tid].mpdu_num + tid_pkt_stat->tid_pkt_num[bsrp_tid]);

    ac_frame_len += hmac_user->tx_tid_queue[tid].tid_buffer_frame_len + tid_pkt_stat->tid_pkt_len[tid] +
        MAC_80211_QOS_FRAME_LEN * (hmac_user->tx_tid_queue[tid].mpdu_num + tid_pkt_stat->tid_pkt_num[tid]);

    /* 如果g_tid_dbg_size中的size不为0，说明此时为调试模式，使用g_tid_dbg_size里面配置的size */
    dbg_size = bsrp_info->tid_dbg_size[bsrp_tid] + bsrp_info->tid_dbg_size[tid];
    if (dbg_size != 0) {
        ac_frame_len = dbg_size;
    }

    queue_size = hmac_bsrp_calc_queue_size_by_frame_len(ac_frame_len);

    return queue_size;
}

OSAL_STATIC wlan_tidno_enum_uint8 hmac_bsrp_ac_to_tid(osal_s8 ac)
{
    if (ac == WLAN_WME_AC_VO) {
        return WLAN_TIDNO_BCAST;
    } else if (ac == WLAN_WME_AC_VI) {
        return WLAN_TIDNO_VIDEO;
    } else if (ac == WLAN_WME_AC_BK) {
        return WLAN_TIDNO_BACKGROUND;
    } else {
        return WLAN_TIDNO_BEST_EFFORT;
    }
}

OSAL_STATIC osal_void hmac_bsrp_set_queue_size(hmac_user_stru *hmac_user, mac_user_tid_pkt_stat_stru *tid_pkt_stat)
{
    osal_s8 ac;
    osal_u8 offset = 0;
    osal_u8 ac_queue_size[WLAN_WME_AC_BUTT] = {0};
    osal_u8 ac_offset[WLAN_WME_AC_BUTT] = {BIT_OFFSET_24, BIT_OFFSET_16, BIT_OFFSET_8, BIT_OFFSET_0};
    osal_u8 ac_queue_tid[WLAN_WME_AC_BUTT] = {0};
    osal_u32 queue_size = 0;
    osal_u16 ac_order;
    mac_bsrp_info_stru *bsrp_info = hmac_get_bsrp_info();

    for (ac = WLAN_WME_AC_BE; ac < WLAN_WME_AC_BUTT; ac++) {
        ac_queue_size[ac] = hmac_bsrp_calc_queue_size(hmac_user, (wlan_wme_ac_type_enum_uint8)ac, tid_pkt_stat);
    }

    /* 先填充不为0的AC */
    for (ac = WLAN_WME_AC_VO; ac >= WLAN_WME_AC_BE; ac--) {
        /* BE的优先级高于BK，BK需要最后填充 */
        if (ac_queue_size[ac] == 0 || ac == WLAN_WME_AC_BK) {
            continue;
        }
        queue_size |=  (ac_queue_size[ac] << ac_offset[offset]);
        ac_queue_tid[offset] = hmac_bsrp_ac_to_tid(ac);
        offset++;
    }
    if (ac_queue_size[WLAN_WME_AC_BK] != 0) {
        queue_size |=  (ac_queue_size[WLAN_WME_AC_BK] << ac_offset[offset]);
        ac_queue_tid[offset] = hmac_bsrp_ac_to_tid(WLAN_WME_AC_BK);
        offset++;
    }
    /* 再填充为0的AC */
    for (ac = WLAN_WME_AC_VO; ac >= WLAN_WME_AC_BE; ac--) {
        if (ac_queue_size[ac] != 0 || ac == WLAN_WME_AC_BK) {
            continue;
        }
        ac_queue_tid[offset] = hmac_bsrp_ac_to_tid(ac);
        offset++;
    }
    if (ac_queue_size[WLAN_WME_AC_BK] == 0) {
        ac_queue_tid[offset] = hmac_bsrp_ac_to_tid(WLAN_WME_AC_BK);
    }
    /* 将AC 的queue_size和对应的TID写入寄存器 */
    ac_order = ac_queue_tid[3] |             /* 3: 最低优先级 */
        (ac_queue_tid[2] << BIT_OFFSET_4) |  /* 2: 次低优先级 */
        (ac_queue_tid[1] << BIT_OFFSET_8) |  /* 1: 次高优先级 */
        (ac_queue_tid[0] << BIT_OFFSET_12);  /* 0: 最高优先级 */

    if (queue_size != bsrp_info->ac_queue_size) {
        hal_set_queue_size_in_ac(queue_size, 0, ac_order);
        bsrp_info->ac_queue_size = queue_size;
    }
}

/*****************************************************************************
 函 数 名  : hmac_buffer_status_statistics
 功能描述  : 每200ms调用该函数统计各TID队列缓存帧，并更新给MAC硬件
*****************************************************************************/
OSAL_STATIC osal_u32 hmac_bsrp_buffer_status_statistics(osal_void *prg)
{
    hmac_vap_stru *hmac_vap = OSAL_NULL;
    hmac_user_stru *hmac_user = OSAL_NULL;
    mac_bsrp_info_stru *bsrp_info = hmac_get_bsrp_info();
    mac_user_tid_pkt_stat_stru tid_pkt_stat;

    unref_param(prg);

    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(bsrp_info->sta_vap_id);
    if (hmac_vap == OSAL_NULL) {
        return OAL_FAIL;
    }

    if (!is_legacy_sta(hmac_vap)) {
        return OAL_SUCC;
    }

    hmac_user = (hmac_user_stru *)mac_res_get_hmac_user_etc(hmac_vap->assoc_vap_id);
    if (hmac_user == OSAL_NULL) {
        return OAL_FAIL;
    }

    (osal_void)memset_s(&tid_pkt_stat, sizeof(tid_pkt_stat), 0, sizeof(tid_pkt_stat));
#ifdef _PRE_WLAN_FEATURE_CENTRALIZE
    hmac_get_device_user_tid_pkt_stat(hmac_vap, hmac_vap->assoc_vap_id, &tid_pkt_stat);
#endif

    /* 设置ndp feedback status */
    hmac_nfrp_set_ndp_feedback_status(hmac_vap, hmac_user, &tid_pkt_stat);

    /* 设置tid buffer   status */
    hmac_bsrp_set_queue_size(hmac_user, &tid_pkt_stat);

    return OAL_SUCC;
}

OSAL_STATIC osal_void hmac_bsrp_tid_buffer_status_init(osal_void)
{
    osal_u8 ac_queue_tid[WLAN_WME_AC_BUTT] = {0};
    osal_u8 offset = 0;
    osal_s8 ac;
    osal_u16 ac_order;

    for (ac = WLAN_WME_AC_VO; ac >= WLAN_WME_AC_BE; ac--) {
        ac_queue_tid[offset] = hmac_bsrp_ac_to_tid(ac);
        offset++;
    }

    ac_order = ac_queue_tid[3] |             /* 3: 最低优先级 */
        (ac_queue_tid[2] << BIT_OFFSET_4) |  /* 2: 次低优先级 */
        (ac_queue_tid[1] << BIT_OFFSET_8) |  /* 1: 次高优先级 */
        (ac_queue_tid[0] << BIT_OFFSET_12);  /* 0: 最高优先级 */

    hal_set_queue_size_in_ac(0, 0, ac_order);
}

OSAL_STATIC osal_bool hmac_bsrp_nfrp_add_user(osal_void *notify_data)
{
    hmac_vap_stru *hmac_vap = OSAL_NULL;
    hmac_user_stru *hmac_user = (hmac_user_stru *)notify_data;
    mac_bsrp_info_stru *bsrp_info = hmac_get_bsrp_info();

    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(hmac_user->vap_id);
    if (hmac_vap == OSAL_NULL) {
        oam_warning_log1(0, 0, "{hmac_bsrp_add_user:vap[%d] is null.}", hmac_user->vap_id);
        return OSAL_FALSE;
    }

    if (!is_legacy_sta(hmac_vap)) {
        return OSAL_TRUE;
    }

    if (bsrp_info->buffer_status_timer.is_registerd == OSAL_TRUE) {
        return OSAL_TRUE;
    }

    bsrp_info->sta_vap_id = hmac_user->vap_id;
    hmac_bsrp_tid_buffer_status_init();
    frw_create_timer_entry(&bsrp_info->buffer_status_timer, hmac_bsrp_buffer_status_statistics, BSRP_CALC_TIMEOUT,
        OSAL_NULL, OSAL_TRUE);
    /* 初始化tid调试变量，默认值为0，表示调试功能关闭 */
    memset_s(&bsrp_info->tid_dbg_size, sizeof(osal_u32) * WLAN_TID_MAX_NUM, 0, sizeof(osal_u32) * WLAN_TID_MAX_NUM);

    return OSAL_TRUE;
}

OSAL_STATIC osal_bool hmac_bsrp_nfrp_del_user(osal_void *notify_data)
{
    hmac_vap_stru *hmac_vap = OSAL_NULL;
    hmac_user_stru *hmac_user = (hmac_user_stru *)notify_data;
    mac_bsrp_info_stru *bsrp_info = hmac_get_bsrp_info();

    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(hmac_user->vap_id);
    if (hmac_vap == OSAL_NULL) {
        oam_warning_log1(0, 0, "{hmac_bsrp_nfrp_del_user:vap[%d] is null.}", hmac_user->vap_id);
        return OSAL_FALSE;
    }

    if (!is_legacy_sta(hmac_vap)) {
        return OSAL_TRUE;
    }

    /* 关闭BSRP定时器 */
    if (bsrp_info->buffer_status_timer.is_registerd == OSAL_TRUE) {
        frw_destroy_timer_entry(&bsrp_info->buffer_status_timer);
    }

    bsrp_info->sta_vap_id = WLAN_VAP_SUPPORT_MAX_NUM_LIMIT;

    return OSAL_TRUE;
}

#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
/*****************************************************************************
 函 数 名  : hmac_ccpriv_tid_dbg
 功能描述  : 配置各TID的缓存报文的大小,仅调试使用,tid_dbg [tid][buff size]
*****************************************************************************/
OSAL_STATIC osal_s32 hmac_ccpriv_tid_dbg(hmac_vap_stru *hmac_vap, const osal_s8 *param)
{
    osal_s32 ret;
    osal_u32 value;
    osal_u32 tid;
    mac_bsrp_info_stru *bsrp_info = hmac_get_bsrp_info();
    mac_tid_dbg_stru tid_dbg;
    memset_s(&tid_dbg, OAL_SIZEOF(tid_dbg), 0, OAL_SIZEOF(tid_dbg));

    unref_param(hmac_vap);

    /* 解析tid num  : param num should be 0~7 */
    ret = hmac_ccpriv_get_digit_with_check_max(&param, 7, &value);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_CFG, "{hmac_ccpriv_tid_dbg::tid num invalid.}");
        return ret;
    }
    if (value >= WLAN_TID_MAX_NUM) {
        oam_warning_log1(0, OAM_SF_CFG, "{hmac_ccpriv_tid_dbg::tid[%u] is error!}", value);
        return OAL_FAIL;
    }
    tid = value;

    /* 解析buffer size: param num should be 0~0x7FFFFFFF */
    ret = hmac_ccpriv_get_digit_with_check_max(&param, 0x7FFFFFFF, &value);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_CFG, "{hmac_ccpriv_tid_dbg::tid buffer size invalid.}");
        return ret;
    }
    bsrp_info->tid_dbg_size[tid] = value;

    return OAL_SUCC;
}
#endif

osal_u32 hmac_bsrp_nfrp_init(osal_void)
{
    /* 通知注册 */
    frw_util_notifier_register(WLAN_UTIL_NOTIFIER_EVENT_ADD_USER, hmac_bsrp_nfrp_add_user);
    frw_util_notifier_register(WLAN_UTIL_NOTIFIER_EVENT_DEL_USER, hmac_bsrp_nfrp_del_user);
#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
    /* ccpriv命令注册 */
    hmac_ccpriv_register((const osal_s8 *)"tid_dbg", hmac_ccpriv_tid_dbg);
#endif
    /* 对外接口注册 */
    hmac_feature_hook_register(HMAC_FHOOK_NFRP_UPDATE_PARAMS, hmac_nfrp_sta_update_param);

    return OAL_SUCC;
}

osal_void hmac_bsrp_nfrp_deinit(osal_void)
{
    /* 通知去注册 */
    frw_util_notifier_register(WLAN_UTIL_NOTIFIER_EVENT_ADD_USER, hmac_bsrp_nfrp_add_user);
    frw_util_notifier_register(WLAN_UTIL_NOTIFIER_EVENT_DEL_USER, hmac_bsrp_nfrp_del_user);
#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
    /* ccpriv命令去注册 */
    hmac_ccpriv_unregister((const osal_s8 *)"tid_dbg");
#endif
    /* 对外去接口注册 */
    hmac_feature_hook_unregister(HMAC_FHOOK_NFRP_UPDATE_PARAMS);
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
