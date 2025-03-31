/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: algorithm anti interference
 */

#ifdef _PRE_WLAN_FEATURE_ANTI_INTERF

/******************************************************************************
  1 其他头文件包含
******************************************************************************/
#include "alg_anti_interference.h"
#include "hmac_alg_notify.h"
#include "hal_ext_if.h"
#include "alg_transplant.h"
#include "alg_main.h"
#include "oal_mem_hcm.h"
#include "alg_gla.h"
#ifdef _PRE_WLAN_FEATURE_POWERSAVE
#include "hmac_sta_pm.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_ALG_ANTI_INTERFERENCE_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST
/******************************************************************************
  2 宏定义
******************************************************************************/
#define ALG_ANTI_INF_CCA_THR_NUM                 3     /* 协议CCA检测门限寄存器的个数 */
/* 滑窗统计相关配置 */
#define ALG_ANTI_INF_STAT_RSSI_MODE              1     /* 0:滑窗平均;1:周期求rssi平均值 */
#define ALG_ANTI_INF_SLIDE_WIN                   4     /* rssi统计滑窗长度 */
#define ALG_ANTI_INF_RSSI_INVALID               127    /* RSSI的初始值 */
#define ALG_ANTI_INF_SERIAL_DEBUG                2     /* debug串口日志 */
#define ALG_ANTI_INF_HSO_DEBUG                   1     /* debug HSO日志 */
#define ALG_ANTI_INF_SMTH_SHIFT_BIT              2     /* 平滑系数shift bit */
/* 弱干扰免疫参数 */
#define ALG_ANTI_INF_UNLOCK_TX_TH_MAX_VAL      (-72)   /* 动态agc unlock tx门限最大值 */
#define ALG_ANTI_INF_UNLOCK_RX_TH_MAX_VAL      (-72)   /* 动态agc unlock rx门限最大值 */
#define ALG_ANTI_INF_UNLOCK_RSSI_BACKOFF_40M     3     /* AP在40M模式下，unlock门限相对统计RSSI回退3dB */
#define ALG_ANTI_INF_BEACON_LOSS_NUM_TH          7     /* 连续X个beacon帧没有收到，重置agc门限 */

/* 定时器周期 */
#define ALG_ANTI_INF_RSSI_STAT_CYCLE_MS       1500     /* 遍历和统计STA RSSI的定时器周期 */
#define ALG_ANTI_INF_DYN_UNLOCK_OFF_CYCLE_MS   300     /* 关闭动态agc unlock的定时器周期 */
#define ALG_ANTI_INF_DYN_UNLOCK_OFF_DUR_MS_AP   75     /* ap模式下动态agc unlock关闭时间定时器周期 */
#define ALG_ANTI_INF_DYN_UNLOCK_OFF_DUR_MS_STA  50     /* sta模式下动态agc unlock关闭时间定时器周期 */

/* 负增益场景优化相关 */
#define ALG_ANTI_INF_KEEP_CYC_NUM_MIN            4     /* 算法保持on/off的最小周期个数 */
#define ALG_ANTI_INF_PER_FALL_TH                 2     /* per下降判别门限 */
#define ALG_ANTI_INF_KEEP_CYC_NUM_MAX           64     /* 算法保持on/off的最大周期个数 */
#define ALG_ANTI_INF_GOODPUT_FALL_TH             8     /* goodput下降比值判别门限(百分之X) */
#define ALG_ANTI_INF_TX_TIME_FALL_TH            16     /* tx time下降比值判别门限(百分之X) */
#define ALG_ANTI_INF_GOODPUT_JITTER_TH           2     /* goodput抖动门限 */
#define ALG_ANTI_INF_UNLOCK_TH_RSSI_BK          25     /* 根据rssi设置unlock门限的回退值 */
#define ALG_ANTI_INF_NON_DIR_RSSI_TH_OFFSET     15     /* 根据rssi设置non-dir帧过滤rssi门限的抬高值 */
/******************************************************************************
  3 STRUCT定义
******************************************************************************/
/* device下抗干扰特性对应的信息结构体 */
typedef struct {
    /* 抗干扰特性开关参数 */
    oal_bool_enum_uint8 inf_immune_enable : 2, /* 是否使能自适应干扰免疫算法(不使能时，配置mac寄存器仍有效) */
        unlock_config_enable : 2,              /* 整个agc unlock的使能信号 */
        per_probe_en : 1,                      /* 是否使能tx time信息识别 */
        scan_agculk_status : 1,                /* 扫描时需要关闭算法，防止结束扫描时候算法永远都被打开 */
        is_auth : 1,                           /* 判断当前是否是auth帧处理流程 */
        resv : 1;               /* 指示干扰检测是否正在测量信道 */
    oal_bool_enum_uint8 dyn_unlock_enable;     /* agc unlock动态使能标识(定时器相关) */
    osal_u8 tx_time_fall_th;                   /* tx time下降比值门限(百分之X) */
    osal_u8 goodput_fall_th;                   /* goodput下降比值门限(百分之X) */

    oal_bool_enum_uint8 recv_frm_flag;         /* 是否已经收到第一个认证帧 */
    osal_u8 probe_keep_cyc_cnt[2];             /* 2:数组大小 算法on/off探测保持周期个数计数器 */
    osal_u8 debug_mode;                        /* debug开关 */

    osal_u16 target_user_bitmap;               /* 标记当前所有关联用户 */
    osal_u16 user_bitmap;                      /* 标记T1周期用户是否收到包 */

    osal_u8 user_num;                          /* 用户数 */
    oal_bool_enum_uint8 probe_direct;          /* 当前算法探测的方向 */
    osal_u8 probe_keep_cyc_num[2];             /* 2:数组大小 算法on/off探测保持周期个数 */

    osal_s8 rssi_min;                          /* 记录最小的RSSI */
    osal_s8 non_dir_rssi_th;                   /* non-direct包丢弃的RSSI门限 */
    osal_s8 dyn_unlock_tx_th;                  /* 动态agc unlock中的tx门限 */
    osal_s8 dyn_unlock_rx_th;                  /* 动态agc unlock中的rx门限 */

    osal_u32 curr_tx_time;                     /* 保存最新tx_time */
    osal_u32 tx_time_last;                     /* 保存前一周期tx_time */
    osal_u32 tx_time_avg[2];                   /* 2:数组大小 探测周期内的平均tx time */

    osal_s8 dyn_unlock_txth_limit;             /* agc unlock tx门限最大限制值(考虑cca) */
    osal_s8 dyn_unlock_rxth_limit;             /* agc unlock rx门限最大限制值(考虑cca) */
    oal_bool_enum_uint8 dyn_extlna_off_bypass; /* 动态LNA bypass */
    osal_s8 rssi_min_last;                     /* 正在使用的RSSI */

    osal_u8 keep_cyc_max_num;                  /* 探测保持的最大周期个数 */
    osal_s8 agc_unlock_last;                   /* 记录上次设置的AGC unlock门限值 */
    osal_s8 agc_unlock_last_non_defult;        /* 记录上次设置的非默认值的AGC unlock门限值 */
    osal_s8 non_dir_rssi_last;                 /* 记录上次设置的non direct门限 */
    osal_s8 res;                               /* 对齐 */

    osal_u8 keep_cyc_min_num;                  /* 探测保持的最小周期个数 */
    osal_u8 per_fall_th;                       /* per下降门限 */
    osal_u8 goodput_jitter_th;                 /* goodput抖动门限 */
    osal_u8 dyn_unlock_start_timeout_sta;      /* STA对应AGC unlock门限动态调整关闭时间 */
    /* 弱干扰免疫相关参数 */
    osal_s8 agc_unlock_def_tx_th;              /* 默认agc unlock中的tx门限 */
    osal_s8 agc_unlock_def_rx_th;              /* 默认agc unlock中的rx门限 */
    osal_s8 agc_unlock_max_tx_th;              /* agc unlock中的tx最大门限 */
    osal_s8 agc_unlock_max_rx_th;              /* agc unlock中的rx最大门限 */

    osal_s8 non_dir_rssi_def_th;               /* 默认non direct帧丢包门限 */
    osal_s8 non_dir_rssi_max_th;               /* 默认non direct帧丢包最大门限 */
    osal_s8 rssi_bk;                           /* 设定的回退值 */
    osal_s8 agc_unlock_bk_40m;                 /* 40M时多回退的dBm数 */

    osal_u32 timeout_t1;                       /* T1周期 */
    osal_u32 timeout_t2;                       /* T2周期 */
    osal_u32 timeout_t3;                       /* T3周期 */

    hal_to_dmac_device_stru *hal_device;       /* hal device实体 */

    /* 相关定时器 */
#if ALG_ANTI_INF_STAT_RSSI_MODE
    frw_timeout_stru rssi_stat_timer;          /* 遍历和统计RSSI周期定时器 */
#endif
    frw_timeout_stru dyn_unlock_off_timer;     /* 关闭动态agc unlock的周期定时器 */
    frw_timeout_stru dyn_unlock_start_timer;   /* 动态agc unlock关闭持续时间定时器 */
} alg_anti_interference_stru;

/* 用户RSSI统计信息结构体 */
typedef struct {
#if (ALG_ANTI_INF_STAT_RSSI_MODE == 0)
    osal_s8 tx_rssi_buf[ALG_ANTI_INF_SLIDE_WIN]; /* 当前最近的4个ack的rssi */
    osal_s8 rx_rssi_buf[ALG_ANTI_INF_SLIDE_WIN]; /* 当前最近的4个data的rssi */
#endif
    osal_u32 goodput_kbps_last[2]; /* 2:数组大小 算法处于off/on的上个goodput值 */
    osal_u16 per_avg_last[2];       /* 2:数组大小 算法处于off/on的平均per */
    osal_u16 per_sum_last; /* 算法处于off/on的per之和 */
    osal_u8 res[2]; /* 对齐 */
} alg_anti_intf_user_info_stru;

/******************************************************************************
  4 函数声明
******************************************************************************/
OSAL_STATIC osal_u32 alg_anti_intf_rssi_stat_timer_handler(osal_void *void_code);
OSAL_STATIC osal_u32 alg_anti_intf_dyn_unlock_off_timer_handler(osal_void *void_code);
OSAL_STATIC osal_u32 alg_anti_intf_dyn_unlock_start_timer_handler(osal_void *void_code);
OSAL_STATIC oal_bool_enum_uint8 alg_anti_intf_is_all_vap_mode_sta(const hal_to_dmac_device_stru *hal_device);
OSAL_STATIC osal_u32 alg_anti_intf_tbtt_isr(hmac_vap_stru *hmac_vap, hal_to_dmac_device_stru *hal_device);
OSAL_STATIC osal_void alg_anti_intf_record_rx_rssi_info(alg_anti_interference_stru *anti_intf,
    const hal_rx_statistic_stru *rx_stats);
OSAL_STATIC osal_void alg_anti_intf_next_probe_proc(alg_anti_interference_stru *anti_intf,
    oal_bool_enum_uint8 next_dir);
OSAL_STATIC osal_void alg_anti_intf_calc_goodput_diff(const alg_anti_interference_stru *anti_intf,
    const alg_anti_intf_user_info_stru *user_info, oal_bool_enum_uint8 *has_negative);
#if (defined(_PRE_WLAN_FEATURE_EDCA_OPT) && defined(_PRE_WLAN_FEATURE_INTF_DET))
OSAL_STATIC osal_void alg_anti_intf_calc_tx_time_diff(alg_anti_interference_stru *anti_intf,
    oal_bool_enum_uint8 *has_negative);
#endif
OSAL_STATIC osal_void alg_anti_intf_calc_unlock_th_limit(const hal_to_dmac_device_stru *hal_device,
    alg_anti_interference_stru *anti_intf);
OSAL_STATIC osal_u32 alg_anti_intf_switch(hal_to_dmac_device_stru *hal_device, oal_bool_enum_uint8 alg_enable);
osal_void alg_anti_intf_set_dyn_unlock_th_reg(hal_to_dmac_device_stru *hal_device,
    alg_anti_interference_stru *anti_inf);
OSAL_STATIC osal_u32 alg_anti_intf_set_rssi_th_reg(alg_anti_interference_stru *anti_inf);
OSAL_STATIC osal_u32 alg_anti_intf_init_user_info(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user);
OSAL_STATIC osal_u32 alg_anti_intf_exit_user_info(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user);
OSAL_STATIC osal_u32 alg_anti_intf_rx_process(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user, oal_netbuf_stru *buf,
    hal_rx_statistic_stru *rx_stats);
OSAL_STATIC osal_u32 alg_anti_intf_rx_mgmt_process(const hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    oal_netbuf_stru *buf);
#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
OSAL_STATIC osal_u32 alg_anti_intf_config_param(hmac_vap_stru *hmac_vap, frw_msg *msg);
#endif
osal_s32 hmac_anti_intf_get_tx_comp_info(hmac_vap_stru *hmac_vap, frw_msg *msg);
/******************************************************************************
  5 全局变量定义
******************************************************************************/
/**********************************************
    device级anti_i结构体
**********************************************/
OSAL_STATIC alg_anti_interference_stru g_anti_intf;
/******************************************************************************
  6 函数定义
******************************************************************************/
/******************************************************************************
 功能描述  : 根据真实RSSI回退一定数值确认弱干扰门限
******************************************************************************/
WIFI_TCM_TEXT osal_s8 alg_anti_intf_get_rssi_bk_val(osal_s8 rssi, alg_anti_interference_stru *anti_intf)
{
    return rssi - anti_intf->rssi_bk;
}

/*******************************************************************************
 功能描述  : 弱干扰免疫模块参数初始化
******************************************************************************/
OSAL_STATIC osal_void alg_anti_intf_init_fill_str(osal_void)
{
    alg_anti_interference_stru *anti_intf = &g_anti_intf;
    hal_to_dmac_device_stru *hal_device = hal_chip_get_hal_device();
    memset_s(anti_intf, sizeof(alg_anti_interference_stru), 0, sizeof(alg_anti_interference_stru));
    /* 初始化device的私有信息 */
    anti_intf->hal_device = hal_device;
    anti_intf->rssi_min_last = ALG_ANTI_INF_RSSI_INVALID;
    anti_intf->probe_direct = OSAL_TRUE;
    anti_intf->probe_keep_cyc_num[0] = ALG_ANTI_INF_KEEP_CYC_NUM_MIN;
    anti_intf->probe_keep_cyc_num[1] = ALG_ANTI_INF_KEEP_CYC_NUM_MIN;
    anti_intf->rssi_min = ALG_ANTI_INF_RSSI_INVALID;
    anti_intf->non_dir_rssi_th = ALG_ANTI_INF_RSSI_TH_DEFAULT_VAL;
    anti_intf->dyn_unlock_tx_th = ALG_ANTI_INF_UNLOCK_TX_TH_DEFAULT_VAL;
    anti_intf->dyn_unlock_rx_th = ALG_ANTI_INF_UNLOCK_RX_TH_DEFAULT_VAL;
    anti_intf->inf_immune_enable = WLAN_ANTI_INTF_WORK_MODE;
    anti_intf->unlock_config_enable = WLAN_ANTI_INTF_WORK_MODE;
    anti_intf->dyn_unlock_enable = OSAL_TRUE;
    anti_intf->scan_agculk_status = OSAL_TRUE;
    anti_intf->is_auth = OSAL_FALSE;

    /* 负增益优化参数 */
    anti_intf->per_probe_en = OSAL_TRUE;
    anti_intf->goodput_fall_th = ALG_ANTI_INF_GOODPUT_FALL_TH;
    anti_intf->keep_cyc_max_num = ALG_ANTI_INF_KEEP_CYC_NUM_MAX;
    anti_intf->keep_cyc_min_num = ALG_ANTI_INF_KEEP_CYC_NUM_MIN;
    anti_intf->tx_time_fall_th = ALG_ANTI_INF_TX_TIME_FALL_TH;
    anti_intf->per_fall_th = ALG_ANTI_INF_PER_FALL_TH;
    anti_intf->goodput_jitter_th = ALG_ANTI_INF_GOODPUT_JITTER_TH;
    anti_intf->timeout_t1 = ALG_ANTI_INF_RSSI_STAT_CYCLE_MS;
    anti_intf->timeout_t2 = ALG_ANTI_INF_DYN_UNLOCK_OFF_CYCLE_MS;
    anti_intf->timeout_t3 = ALG_ANTI_INF_DYN_UNLOCK_OFF_DUR_MS_AP;
    anti_intf->dyn_unlock_start_timeout_sta = ALG_ANTI_INF_DYN_UNLOCK_OFF_DUR_MS_STA;
    anti_intf->agc_unlock_def_tx_th = ALG_ANTI_INF_UNLOCK_TX_TH_DEFAULT_VAL;
    anti_intf->agc_unlock_def_rx_th = ALG_ANTI_INF_UNLOCK_RX_TH_DEFAULT_VAL;
    anti_intf->non_dir_rssi_def_th = ALG_ANTI_INF_RSSI_TH_DEFAULT_VAL;
    anti_intf->non_dir_rssi_max_th = ALG_ANTI_INF_RSSI_TH_MAX_VAL;
    anti_intf->rssi_bk = ALG_ANTI_INF_UNLOCK_TH_RSSI_BK;
    anti_intf->agc_unlock_bk_40m = ALG_ANTI_INF_UNLOCK_RSSI_BACKOFF_40M;
    anti_intf->agc_unlock_max_tx_th = ALG_ANTI_INF_UNLOCK_TX_TH_MAX_VAL;
    anti_intf->agc_unlock_max_rx_th = ALG_ANTI_INF_UNLOCK_RX_TH_MAX_VAL;
    anti_intf->agc_unlock_last_non_defult = ALG_ANTI_INF_UNLOCK_TX_TH_DEFAULT_VAL;
}

#ifdef _PRE_WLAN_FEATURE_INTF_DET
/******************************************************************************
 功能描述  : 获取tx time
******************************************************************************/
OSAL_STATIC osal_u32 alg_anti_intf_tx_time_notify_process(hal_to_dmac_device_stru *hal_device)
{
    alg_anti_interference_stru *anti_intf = OSAL_NULL;
    osal_u32 ret;
    ret = hmac_alg_get_device_priv_stru(hal_device, HAL_ALG_DEVICE_STRU_ID_ANTI_INTF, (osal_void **)&anti_intf);
    if (osal_unlikely(ret != OAL_SUCC)) {
        return OAL_FAIL;
    }

    anti_intf->curr_tx_time = hal_device->mac_ch_stats.tx_time;
    return OAL_SUCC;
}
#endif

/******************************************************************************
 功能描述  :注册钩子
******************************************************************************/
OSAL_STATIC osal_void alg_anti_intf_register_func(osal_void)
{
    /* 注册钩子函数 */
    hmac_alg_register_add_user_notify_func(HMAC_ALG_ADD_USER_NOTIFY_ANTI_INTF, alg_anti_intf_init_user_info);
    hmac_alg_register_del_user_notify_func(HMAC_ALG_DEL_USER_NOTIFY_ANTI_INTF, alg_anti_intf_exit_user_info);
    hmac_alg_register_rx_notify_func(HMAC_ALG_RX_ANTI_INTF, alg_anti_intf_rx_process);
    hmac_alg_register_rx_mgmt_notify_func(HMAC_ALG_RX_MGMT_ANTI_INTF, alg_anti_intf_rx_mgmt_process);
#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
    hmac_alg_register_para_cfg_notify_func(ALG_PARAM_CFG_ANTI_INTF, alg_anti_intf_config_param);
#endif
    hmac_alg_register_anti_intf_tx_time_notify(alg_anti_intf_tx_time_notify_process);

    hmac_alg_register_anti_intf_switch_notify(alg_anti_intf_switch);
    hmac_alg_register_tbtt_notify_func(alg_anti_intf_tbtt_isr);

    /* 注册消息通知 */
    frw_msg_hook_register(WLAN_MSG_D2H_ANTI_INTF_INFO_REPORT, hmac_anti_intf_get_tx_comp_info);
    return;
}

/*******************************************************************************
 功能描述  : 弱干扰免疫模块初始化
******************************************************************************/
osal_s32 alg_anti_intf_init(osal_void)
{
    osal_u32 ret;
    alg_anti_interference_stru *anti_intf = &g_anti_intf;
    hal_to_dmac_device_stru *hal_device = hal_chip_get_hal_device();

    ret = hmac_alg_register(HMAC_ALG_ID_ANTI_INTF);
    if (osal_unlikely(ret != OAL_SUCC)) {
        return (osal_s32)ret;
    }

    /* 设置nav免疫寄存器 */
    hal_set_nav_max_duration(ALG_ANTI_INF_MAX_DUR_VAL, ALG_ANTI_INF_OBSS_MAX_DUR_VAL);
    alg_anti_intf_init_fill_str();

    /* 初始化agc unlock门限 */
    alg_anti_intf_calc_unlock_th_limit(hal_device, anti_intf);

    /* 注册相关定时器，启动其中2个 */
#if ALG_ANTI_INF_STAT_RSSI_MODE
    frw_create_timer_entry(&(anti_intf->rssi_stat_timer), alg_anti_intf_rssi_stat_timer_handler, anti_intf->timeout_t1,
        (osal_void *)(anti_intf), OSAL_TRUE);
#endif
    frw_create_timer_entry(&(anti_intf->dyn_unlock_off_timer), alg_anti_intf_dyn_unlock_off_timer_handler,
        anti_intf->timeout_t2, (osal_void *)(anti_intf), OSAL_TRUE);

    /* 注册device私有信息 */
    ret = hmac_alg_register_device_priv_stru(hal_device, HAL_ALG_DEVICE_STRU_ID_ANTI_INTF,
        (osal_void *)anti_intf);
    if (ret != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_ANY, "{alg_anti_intf_init_timer_and_nav:: register_device FAIL!.}");
    }

    alg_anti_intf_register_func();

    return OAL_SUCC;
}
/******************************************************************************
 功能描述  : 弱干扰免疫模块退出函数,释放用户信息
******************************************************************************/
OSAL_STATIC osal_void alg_anti_intf_exit_release_user_info(const hal_to_dmac_device_stru *hal_device)
{
    hmac_user_stru *hmac_user = OSAL_NULL;
    osal_u8 mac_vap_id[WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE] = {0};
    osal_u32 up_vap_num, vap_index;
    struct osal_list_head *list_pos = OSAL_NULL;
    alg_anti_intf_user_info_stru *user_info = OSAL_NULL;
    up_vap_num = hal_device_find_all_up_vap(hal_device, mac_vap_id, WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE);
    for (vap_index = 0; vap_index < up_vap_num; vap_index++) {
        hmac_vap_stru *hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap_id[vap_index]);
        if (osal_unlikely(hmac_vap == OSAL_NULL)) {
            oam_error_log1(0, OAM_SF_ANTI_INTF, "alg_anti_intf_exit_release_user_info::hmac_vap[%d] NULL",
                mac_vap_id[vap_index]);
            continue;
        }

        alg_vap_foreach_user(hmac_user, hmac_vap, list_pos) {
            /* 组播用户不做处理 */
            if (osal_unlikely(hmac_user->is_multi_user == OSAL_TRUE)) {
                continue;
            }

            hmac_alg_get_user_priv_stru(hmac_user, HMAC_ALG_USER_STRU_ID_ANTI_INTF, (osal_void **)&user_info);
            if (osal_unlikely(user_info == OSAL_NULL)) {
                continue;
            }

            hmac_alg_unregister_user_priv_stru(hmac_user, HMAC_ALG_USER_STRU_ID_ANTI_INTF);
            oal_mem_free(user_info, OSAL_TRUE);
        }
    }
    return;
}
/******************************************************************************
 功能描述  : 弱干扰免疫模块退出函数
******************************************************************************/
osal_void alg_anti_intf_exit(osal_void)
{
    hal_to_dmac_device_stru *hal_device = OSAL_NULL;
    alg_anti_interference_stru *anti_intf = OSAL_NULL;

    if (osal_unlikely(hmac_alg_unregister(HMAC_ALG_ID_ANTI_INTF) != OAL_SUCC)) {
        return;
    }

    /* 注销钩子函数 */
    hmac_alg_unregister_add_user_notify_func(HMAC_ALG_ADD_USER_NOTIFY_ANTI_INTF);
    hmac_alg_unregister_del_user_notify_func(HMAC_ALG_DEL_USER_NOTIFY_ANTI_INTF);
    hmac_alg_unregister_rx_notify_func(HMAC_ALG_RX_ANTI_INTF);
    hmac_alg_unregister_rx_mgmt_notify_func(HMAC_ALG_RX_MGMT_ANTI_INTF);
#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
    hmac_alg_unregister_para_cfg_notify_func(ALG_PARAM_CFG_ANTI_INTF);
#endif
    hmac_alg_unregister_anti_intf_switch_notify();
    hmac_alg_unregister_anti_intf_tx_time_notify();
    hmac_alg_unregister_tbtt_notify_func();

    hal_device = hal_chip_get_hal_device();
    if (osal_unlikely(hmac_alg_get_device_priv_stru(hal_device, HAL_ALG_DEVICE_STRU_ID_ANTI_INTF,
        (osal_void **)&anti_intf) != OAL_SUCC)) {
        oam_warning_log0(0, OAM_SF_ANTI_INTF, "{alg_anti_intf_exit: get priv stru failed!}");
        return;
    }

    /* 停止并注销相关定时器 */
#if ALG_ANTI_INF_STAT_RSSI_MODE
    frw_destroy_timer_entry(&(anti_intf->rssi_stat_timer));
#endif
    frw_destroy_timer_entry(&(anti_intf->dyn_unlock_off_timer));
    frw_destroy_timer_entry(&(anti_intf->dyn_unlock_start_timer));

    /* 注销并释放device级别下的结构体 */
    hmac_alg_unregister_device_priv_stru(hal_device, HAL_ALG_DEVICE_STRU_ID_ANTI_INTF);
    /* 释放user信息 */
    alg_anti_intf_exit_release_user_info(hal_device);
    /* 注销消息处理 */
    frw_msg_hook_unregister(WLAN_MSG_D2H_ANTI_INTF_INFO_REPORT);
}

/******************************************************************************
 功能描述  : 用户关联时，初始化STA统计信息
******************************************************************************/
osal_u32 alg_anti_intf_init_user_info(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user)
{
    alg_anti_intf_user_info_stru *user_info = OSAL_NULL;
    alg_anti_interference_stru *anti_intf = OSAL_NULL;
    hal_to_dmac_device_stru *hal_device = OSAL_NULL;
    osal_u32 ret;

    oal_bool_enum_uint8 is_registered = hmac_alg_is_registered(HMAC_ALG_ID_ANTI_INTF);
    /* 算法未注册，或者组播用户不处理 */
    if (osal_unlikely((is_registered == OSAL_FALSE) || (hmac_user->is_multi_user == OSAL_TRUE))) {
        return OAL_SUCC;
    }

    hal_device = hmac_vap->hal_device;
    if (osal_unlikely(hal_device == OSAL_NULL)) {
        oam_error_log1(0, OAM_SF_ANTI_INTF,
                       "{alg_anti_intf_init_user_info:[%d]get hal_device pointer fail!}", hmac_vap->vap_id);
        return OAL_ERR_CODE_ALLOC_MEM_FAIL;
    }

    ret = hmac_alg_get_device_priv_stru(hal_device, HAL_ALG_DEVICE_STRU_ID_ANTI_INTF, (osal_void **)&anti_intf);
    if (ret != OAL_SUCC) {
        return OAL_FAIL;
    }

    /* user私有信息结构体并初始化 */
    user_info = (alg_anti_intf_user_info_stru *)alg_mem_alloc(ALG_MEM_ANTI_INTF, sizeof(alg_anti_intf_user_info_stru));
    if (osal_unlikely(user_info == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_TPC, "{alg_anti_intf_init_user_info: mem alloc fail!}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    memset_s(user_info, sizeof(alg_anti_intf_user_info_stru), 0, sizeof(alg_anti_intf_user_info_stru));

    /* 挂载用户级别的统计信息结构体 */
    ret = hmac_alg_register_user_priv_stru(hmac_user, HMAC_ALG_USER_STRU_ID_ANTI_INTF, (osal_void *)user_info);
    if (osal_unlikely(ret != OAL_SUCC)) {
        oal_mem_free(user_info, OSAL_TRUE);
        return ret;
    }
    anti_intf->target_user_bitmap |= (0x1 << hmac_user->lut_index);

    /* 关联一个用户时，将会通知算法并重新设定agc unlock门限 */
    alg_anti_intf_calc_unlock_th_limit(hal_device, anti_intf);
    if (anti_intf->debug_mode == ALG_ANTI_INF_HSO_DEBUG) {
        oam_warning_log0(0, OAM_SF_ANTI_INTF, "[ANTI_INTF_DBG]init_user_info::update max agc unlock thr!");
    } else if (anti_intf->debug_mode == ALG_ANTI_INF_SERIAL_DEBUG) {
#ifdef _PRE_WLAN_ALG_UART_PRINT
        wifi_printf("[ANTI_INTF_DBG]init_user_info::update max agc unlock thr!\r\n");
#endif
    }

    return OAL_SUCC;
}

/******************************************************************************
 功能描述  : 去关联时，释放用户级信息结构体
******************************************************************************/
osal_u32 alg_anti_intf_exit_user_info(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user)
{
    alg_anti_intf_user_info_stru *user_info = OSAL_NULL;
    osal_u32 ret;
    oal_bool_enum is_registered;
    alg_anti_interference_stru *anti_intf = OSAL_NULL;
    hal_to_dmac_device_stru *hal_device = OSAL_NULL;

    /* 如果算法没被使能，直接返回 */
    is_registered = hmac_alg_is_registered(HMAC_ALG_ID_ANTI_INTF);
    if (osal_unlikely((is_registered == OSAL_FALSE) || (hmac_user->is_multi_user == OSAL_TRUE))) {
        return OAL_SUCC;
    }
    hal_device = hmac_vap->hal_device;
    if (osal_unlikely(hal_device == OSAL_NULL)) {
        oam_error_log1(0, OAM_SF_ANTI_INTF,
                       "{alg_anti_intf_exit_user_info:[%d]get hal_device pointer fail!}", hmac_vap->vap_id);
        return OAL_ERR_CODE_ALLOC_MEM_FAIL;
    }
    ret = hmac_alg_get_device_priv_stru(hal_device, HAL_ALG_DEVICE_STRU_ID_ANTI_INTF, (osal_void **)&anti_intf);
    if (osal_unlikely(ret != OAL_SUCC)) {
        return OAL_FAIL;
    }
    /* 获取并释放用户级别的私有信息结构体 */
    ret = hmac_alg_get_user_priv_stru(hmac_user, HMAC_ALG_USER_STRU_ID_ANTI_INTF, (osal_void **)&user_info);
    if (osal_unlikely(ret != OAL_SUCC)) {
        return ret;
    }
    anti_intf->target_user_bitmap &= ~(0x1 << hmac_user->lut_index);
    /* 注销结构体初始化时有清0去注册时不再清0操作 */
    (osal_void)hmac_alg_unregister_user_priv_stru(hmac_user, HMAC_ALG_USER_STRU_ID_ANTI_INTF);
    oal_mem_free(user_info, OSAL_TRUE);
    return OAL_FAIL;
}

/******************************************************************************
 功能描述  : 帧接收的处理函数，设置门限
******************************************************************************/
WIFI_TCM_TEXT osal_void alg_anti_intf_set_thr(hal_to_dmac_device_stru *hal_device,
    alg_anti_interference_stru *anti_intf)
{
    if ((anti_intf->rssi_min < (anti_intf->rssi_min_last - ALG_RSSI_MARGIN_DB)) &&
        (anti_intf->rssi_min > (anti_intf->agc_unlock_def_tx_th + anti_intf->rssi_bk)) &&
        (anti_intf->user_bitmap == anti_intf->target_user_bitmap)) {
        anti_intf->rssi_min_last = anti_intf->rssi_min;
        anti_intf->dyn_unlock_tx_th = alg_anti_intf_get_rssi_bk_val(anti_intf->rssi_min, anti_intf);
        anti_intf->dyn_unlock_rx_th = anti_intf->dyn_unlock_tx_th;
        if (anti_intf->debug_mode == ALG_ANTI_INF_HSO_DEBUG) {
            oam_warning_log1(0, OAM_SF_ANTI_INTF, "{[ANTI_INTF_DBG]process:enter reg! dyn_agc_unlock[%d]}",
                anti_intf->dyn_unlock_tx_th);
        } else if (anti_intf->debug_mode == ALG_ANTI_INF_SERIAL_DEBUG) {
#ifdef _PRE_WLAN_ALG_UART_PRINT
            wifi_printf("[ANTI_INTF_DBG]process:enter set reg! dyn_agc_unlock[%d].\r\n",
                anti_intf->dyn_unlock_tx_th);
#endif
        }
        alg_anti_intf_set_dyn_unlock_th_reg(hal_device, anti_intf);
    }
    return;
}

/******************************************************************************
 功能描述  : 获取发送完成相关信息
******************************************************************************/
WIFI_TCM_TEXT osal_s32 hmac_anti_intf_get_tx_comp_info(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    alg_anti_intf_tx_comp_info_stru *tx_comp_info = (alg_anti_intf_tx_comp_info_stru *)msg->data;
    hal_to_dmac_device_stru *hal_device = hal_chip_get_hal_device();
    alg_anti_interference_stru *anti_intf = OSAL_NULL;
    oal_bool_enum_uint8 switch_enable = alg_host_get_gla_switch_enable(ALG_GLA_ID_WEAK_IMMUNE, ALG_GLA_OPTIONAL_SWITCH);
    osal_u32 ret;

    unref_param(hmac_vap);

    ret = hmac_alg_get_device_priv_stru(hal_device, HAL_ALG_DEVICE_STRU_ID_ANTI_INTF, (osal_void **)&anti_intf);
    if (osal_unlikely(ret != OAL_SUCC)) {
        return OAL_ERR_CODE_PTR_NULL;
    }
    anti_intf->rssi_min = get_rssi_min(anti_intf->rssi_min, tx_comp_info->rssi_min);
    anti_intf->user_bitmap |= tx_comp_info->user_bitmap;
    if ((anti_intf->debug_mode == ALG_ANTI_INF_HSO_DEBUG) || (switch_enable == OSAL_TRUE)) {
        oam_warning_log2(0, OAM_SF_ANTI_INTF, "[GLA][ALG][WEAK_IMMUNE]:rssi_min=%d,user_bitmap=%d",
            anti_intf->rssi_min, anti_intf->user_bitmap);
    } else if (anti_intf->debug_mode == ALG_ANTI_INF_SERIAL_DEBUG) {
#ifdef _PRE_WLAN_ALG_UART_PRINT
        wifi_printf("[ANTI_INTF_DBG]tx_complete:host: rssi_min[%d], user_bitmap = [%d], device: rssi_min[%d], \
            user_bitmap = [%d]}", anti_intf->rssi_min,
            anti_intf->user_bitmap, tx_comp_info->rssi_min, tx_comp_info->user_bitmap);
#endif
    }
    alg_anti_intf_set_thr(hal_device, anti_intf);
    return OAL_SUCC;
}

/******************************************************************************
 功能描述  : 帧接收处理函数，获取上报的RSSI
******************************************************************************/
osal_u32 alg_anti_intf_rx_process(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user, oal_netbuf_stru *buf,
    hal_rx_statistic_stru *rx_stats)
{
    osal_u32 ret;
    mac_ieee80211_qos_htc_frame_stru *frame_hdr = OSAL_NULL;
    dmac_rx_ctl_stru *rx_ctl = OSAL_NULL;
    alg_anti_interference_stru *anti_intf = OSAL_NULL;
    hal_to_dmac_device_stru *hal_device = OSAL_NULL;

    /* 多播用户无须处理 */
    if (hmac_user->is_multi_user == OSAL_TRUE) {
        return OAL_SUCC;
    }

    /* 获取hal device */
    hal_device = hmac_vap->hal_device;
    if (osal_unlikely(hal_device == OSAL_NULL)) {
        oam_warning_log0(0, OAM_SF_ANTI_INTF, "{alg_anti_intf_rx_process::hal_device pointer is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    ret = hmac_alg_get_device_priv_stru(hal_device, HAL_ALG_DEVICE_STRU_ID_ANTI_INTF, (osal_void **)&anti_intf);
    if (osal_unlikely(ret != OAL_SUCC)) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取接收控制结构体 */
    rx_ctl = (dmac_rx_ctl_stru *)oal_netbuf_cb(buf);

    /* 获取mac帧头 */
    frame_hdr = (mac_ieee80211_qos_htc_frame_stru *)(mac_get_rx_cb_mac_hdr(&rx_ctl->rx_info));

    /* 调用rx RSSI检测函数 */
    anti_intf->user_bitmap |= (0x1 << hmac_user->lut_index);
    alg_anti_intf_record_rx_rssi_info(anti_intf, rx_stats);

    if (anti_intf->debug_mode == ALG_ANTI_INF_HSO_DEBUG) {
        oam_warning_log4(0, OAM_SF_ANTI_INTF, "{[ANTI_INTF_DBG]rx_process:min rssi[%d] last rssi[%d] \
            received user bitmap[%d] total user bitmap[%d]}", anti_intf->rssi_min,
            anti_intf->rssi_min_last, anti_intf->user_bitmap, anti_intf->target_user_bitmap);
    } else if (anti_intf->debug_mode == ALG_ANTI_INF_SERIAL_DEBUG) {
#ifdef _PRE_WLAN_ALG_UART_PRINT
        wifi_printf("[ANTI_INTF_DBG]rx_process:min rssi[%d] last rssi[%d] received user bitmap[%d] \
            total user bitmap[%d].\r\n", anti_intf->rssi_min, anti_intf->rssi_min_last,
            anti_intf->user_bitmap, anti_intf->target_user_bitmap);
#endif
    }
    if (frame_hdr->frame_control.type == WLAN_DATA_BASICTYPE) {
        alg_anti_intf_set_thr(hal_device, anti_intf);
    }

    return OAL_SUCC;
}
/******************************************************************************
 功能描述  : 管理帧处理函数，设置门限
******************************************************************************/
OSAL_STATIC osal_void alg_anti_intf_rx_mgmt_process_set_thr(hal_to_dmac_device_stru *hal_device,
    alg_anti_interference_stru *anti_intf, osal_s8 auth_rssi)
{
    osal_s32 rssi_unlock_tx_th;
    osal_s32 rssi_unlock_rx_th;
    if ((auth_rssi != ALG_ANTI_INF_RSSI_INVALID) &&
            (auth_rssi > (anti_intf->agc_unlock_def_tx_th + anti_intf->rssi_bk))) {
            rssi_unlock_tx_th = alg_anti_intf_get_rssi_bk_val(auth_rssi, anti_intf);
            rssi_unlock_rx_th = rssi_unlock_tx_th;
            anti_intf->is_auth = OSAL_TRUE;
            /* 判断是否为第一个认证帧;是则直接更新为其rssi，否则取与当前门限的最小值 */
            if (anti_intf->recv_frm_flag == OSAL_FALSE) {
                anti_intf->dyn_unlock_tx_th = (osal_s8)rssi_unlock_tx_th;
                anti_intf->dyn_unlock_rx_th = (osal_s8)rssi_unlock_rx_th;
                anti_intf->recv_frm_flag = OSAL_TRUE;
            } else {
                anti_intf->dyn_unlock_tx_th =
                    (osal_s8)osal_min(anti_intf->dyn_unlock_tx_th, rssi_unlock_tx_th);
                anti_intf->dyn_unlock_rx_th =
                    (osal_s8)osal_min(anti_intf->dyn_unlock_rx_th, rssi_unlock_rx_th);
            }
            /* 更新noise floor寄存器 */
            if (anti_intf->debug_mode == ALG_ANTI_INF_HSO_DEBUG) {
                oam_warning_log4(0, OAM_SF_ANTI_INTF, "{[ANTI_INTF_DBG]rx_mgmt_process:enter reg! \
                    dyn_agc_unlock[%d] user bitmap[%d] auth_rssi[%d] is_auth[%d]}",
                    anti_intf->dyn_unlock_tx_th, anti_intf->user_bitmap, auth_rssi, anti_intf->is_auth);
            } else if (anti_intf->debug_mode == ALG_ANTI_INF_SERIAL_DEBUG) {
#ifdef _PRE_WLAN_ALG_UART_PRINT
                wifi_printf("[ANTI_INTF_DBG]rx_mgmt_process:enter set reg! dyn_agc_unlock[%d] user bitmap[%d] \
                    auth_rssi[%d].\r\n", anti_intf->dyn_unlock_tx_th, anti_intf->user_bitmap, auth_rssi);
#endif
            }
            alg_anti_intf_set_dyn_unlock_th_reg(hal_device, anti_intf);
            anti_intf->is_auth = OSAL_FALSE;
    }
    return;
}
/******************************************************************************
 功能描述  : 管理帧处理函数，只处理认证帧
******************************************************************************/
osal_u32 alg_anti_intf_rx_mgmt_process(const hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user, oal_netbuf_stru *buf)
{
    alg_anti_interference_stru *anti_intf = OSAL_NULL;
    mac_ieee80211_qos_htc_frame_stru *frame_hdr = OSAL_NULL;
    osal_u32 ret;
    oal_bool_enum is_registered;
    hal_to_dmac_device_stru *hal_device = OSAL_NULL;
    dmac_rx_ctl_stru *rx_ctl = OSAL_NULL;
    hal_rx_statistic_stru *rx_stats = OSAL_NULL;
    unref_param(hmac_user);
    /* 如果算法没被使能，直接返回 */
    is_registered = hmac_alg_is_registered(HMAC_ALG_ID_ANTI_INTF);
    if (osal_unlikely(is_registered == OSAL_FALSE)) {
        return OAL_SUCC;
    }

    hal_device = hmac_vap->hal_device;
    if (osal_unlikely(hal_device == OSAL_NULL)) {
        oam_error_log1(0, OAM_SF_ANTI_INTF, "{alg_anti_intf_rx_mgmt_process:[%d]dev pointer null!}", hmac_vap->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }
    /* 获取接收控制结构体 */
    rx_ctl = (dmac_rx_ctl_stru *)oal_netbuf_cb(buf);
    frame_hdr = (mac_ieee80211_qos_htc_frame_stru *)(mac_get_rx_cb_mac_hdr(&rx_ctl->rx_info));
    /* 判断是否是认证帧 */
    if (frame_hdr->frame_control.sub_type == WLAN_AUTH) {
        osal_s8 auth_rssi;
        ret = hmac_alg_get_device_priv_stru(hal_device, HAL_ALG_DEVICE_STRU_ID_ANTI_INTF, (osal_void **)&anti_intf);
        if (osal_unlikely(ret != OAL_SUCC)) {
            return ret;
        }
        rx_stats = &(rx_ctl->rx_statistic);
        /* 获取接收描述符结构体的rssi */
        auth_rssi = rx_stats->rssi_dbm;
        /* 设置门限 */
        alg_anti_intf_rx_mgmt_process_set_thr(hal_device, anti_intf, auth_rssi);
    }

    return OAL_SUCC;
}

/******************************************************************************
 功能描述  :  上行RSSI统计函数
******************************************************************************/
osal_void alg_anti_intf_record_rx_rssi_info(alg_anti_interference_stru *anti_intf,
    const hal_rx_statistic_stru *rx_stats)
{
    osal_s32 rssi_dbm = rx_stats->rssi_dbm;
    oal_bool_enum_uint8 switch_enable = alg_host_get_gla_switch_enable(ALG_GLA_ID_WEAK_IMMUNE, ALG_GLA_OPTIONAL_SWITCH);

    anti_intf->rssi_min = (osal_s8)get_rssi_min(rssi_dbm, (osal_s32)anti_intf->rssi_min);
    if ((anti_intf->debug_mode == ALG_ANTI_INF_HSO_DEBUG) || (switch_enable == OSAL_TRUE)) {
        oam_warning_log2(0, OAM_SF_ANTI_INTF, "[GLA][ALG][WEAK_IMMUNE]:rssi_min=%d,user_bitmap=%d",
            anti_intf->rssi_min, anti_intf->user_bitmap);
    }
}

/******************************************************************************
 功能描述  : 设置non-direct RSSI阈值
******************************************************************************/
OSAL_STATIC osal_void alg_anti_intf_set_non_dir_rssi(alg_anti_interference_stru *anti_inf, osal_s8 non_dir_rssi_val)
{
    oal_bool_enum_uint8 switch_enable = alg_host_get_gla_switch_enable(ALG_GLA_ID_WEAK_IMMUNE, ALG_GLA_USUAL_SWITCH);

    if ((anti_inf->non_dir_rssi_last != non_dir_rssi_val)) {
        anti_inf->non_dir_rssi_last = non_dir_rssi_val;
        if ((anti_inf->debug_mode == ALG_ANTI_INF_HSO_DEBUG) || (switch_enable == OSAL_TRUE)) {
            oam_warning_log1(0, OAM_SF_ANTI_INTF, "[GLA][ALG][WEAK_IMMUNE]:weak_rssi=%d", non_dir_rssi_val);
        }
        hal_set_weak_intf_rssi_th(non_dir_rssi_val);
    }
    return;
}

/******************************************************************************
 功能描述  : 设置non-direct RSSI阈值,并写入对应mac寄存器
******************************************************************************/
osal_u32 alg_anti_intf_set_rssi_th_reg(alg_anti_interference_stru *anti_inf)
{
    /* 不超过设定的最大值和最小值 */
    anti_inf->non_dir_rssi_th =
        osal_min(anti_inf->non_dir_rssi_th, anti_inf->non_dir_rssi_max_th);
    anti_inf->non_dir_rssi_th =
        osal_max(anti_inf->non_dir_rssi_th, anti_inf->non_dir_rssi_def_th);
    /* 算法未开启,不设置寄存器 */
    if (anti_inf->inf_immune_enable == WLAN_ANTI_INTF_EN_OFF) {
        return OAL_SUCC;
    }

    /* 负增益优化:探测方向为off(默认值)时，或者没有收全所有用户的包，设置为最小值  */
    if (((anti_inf->inf_immune_enable == WLAN_ANTI_INTF_EN_PROBE) && (anti_inf->probe_direct == OSAL_FALSE)) ||
        anti_inf->user_bitmap != anti_inf->target_user_bitmap) {
        if (anti_inf->debug_mode == ALG_ANTI_INF_HSO_DEBUG) {
            oam_warning_log4(0, OAM_SF_ANTI_INTF, "{[ANTI_INTF_DBG]set min non_direct th [-95]: inf_immune_enable[%d], \
                probe_direct[%d], user bitmap[%d], total user bitmap[%d]}", anti_inf->inf_immune_enable,
                anti_inf->probe_direct, anti_inf->user_bitmap, anti_inf->target_user_bitmap);
        } else if (anti_inf->debug_mode == ALG_ANTI_INF_SERIAL_DEBUG) {
#ifdef _PRE_WLAN_ALG_UART_PRINT
            wifi_printf("[ANTI_INTF_DBG]set min non_direct th [-95]: inf_immune_enable[%d], probe_direct[%d], \
                user bitmap[%d], total user bitmap[%d]\r\n", anti_inf->inf_immune_enable,
                anti_inf->probe_direct,
                anti_inf->user_bitmap, anti_inf->target_user_bitmap);
#endif
        }
        alg_anti_intf_set_non_dir_rssi(anti_inf, anti_inf->non_dir_rssi_def_th);
        return OAL_SUCC;
    }

    alg_anti_intf_set_non_dir_rssi(anti_inf, anti_inf->non_dir_rssi_th);
    return OAL_SUCC;
}

/******************************************************************************
 功能描述  : 设置agc unlock门限
******************************************************************************/
WIFI_TCM_TEXT osal_void alg_anti_intf_set_agc_unlock(alg_anti_interference_stru *anti_inf, osal_s8 agc_unlock_val)
{
    oal_bool_enum_uint8 switch_enable_usual =
        alg_host_get_gla_switch_enable(ALG_GLA_ID_WEAK_IMMUNE, ALG_GLA_USUAL_SWITCH);
    oal_bool_enum_uint8 switch_enable_optional =
        alg_host_get_gla_switch_enable(ALG_GLA_ID_WEAK_IMMUNE, ALG_GLA_OPTIONAL_SWITCH);

#ifdef _PRE_WLAN_FEATURE_POWERSAVE
    if (hmac_sta_pm_get_wlan_dev_state() == HMAC_ALLOW_SLEEP) {
        return;
    }
#endif
    if (anti_inf->agc_unlock_last != agc_unlock_val) {
        anti_inf->agc_unlock_last = agc_unlock_val;
        if ((anti_inf->debug_mode == ALG_ANTI_INF_HSO_DEBUG) || (switch_enable_optional == OSAL_TRUE) ||
            ((switch_enable_usual == OSAL_TRUE) && (agc_unlock_val != ALG_ANTI_INF_UNLOCK_TX_TH_DEFAULT_VAL) &&
            (anti_inf->agc_unlock_last_non_defult != agc_unlock_val))) {
            oam_warning_log1(0, OAM_SF_ANTI_INTF, "[GLA][ALG][WEAK_IMMUNE]:agc_unlock=%d", agc_unlock_val);
            anti_inf->agc_unlock_last_non_defult = agc_unlock_val;
        }
        hal_set_agc_unlock_min_th(agc_unlock_val, agc_unlock_val);
    }
    return;
}

/******************************************************************************
 功能描述  : 负增益优化:探测方向为off(默认值)时，设置为最小值
******************************************************************************/
WIFI_TCM_TEXT osal_u32 alg_anti_intf_negtive_det_alg_off_set_min_thr(alg_anti_interference_stru *anti_inf)
{
    oal_bool_enum_uint8 bitmap_is_match = ((anti_inf->user_bitmap == anti_inf->target_user_bitmap) ||
        (anti_inf->is_auth == OSAL_TRUE));
    if (((anti_inf->unlock_config_enable == WLAN_ANTI_INTF_EN_PROBE) &&
        (anti_inf->probe_direct == OSAL_FALSE)) || (bitmap_is_match == OSAL_FALSE)) {
        if (anti_inf->debug_mode == ALG_ANTI_INF_HSO_DEBUG) {
            oam_warning_log4(0, OAM_SF_ANTI_INTF, "{[ANTI_INTF_DBG]set min agc unlock th [-107]: \
                unlock_config_enable[%d], probe_direct[%d], received user bitmap[%d], target user bitmap[%d]}",
                anti_inf->unlock_config_enable, anti_inf->probe_direct, anti_inf->user_bitmap,
                anti_inf->target_user_bitmap);
        } else if (anti_inf->debug_mode == ALG_ANTI_INF_SERIAL_DEBUG) {
#ifdef _PRE_WLAN_ALG_UART_PRINT
            wifi_printf("[ANTI_INTF_DBG]set min agc unlock th [-107]:unlock_config_enable[%d] probe_direct[%d], \
                received user bitmap[%d], target user bitmap[%d]\r\n",
                anti_inf->unlock_config_enable, anti_inf->probe_direct,
                anti_inf->user_bitmap, anti_inf->target_user_bitmap);
#endif
        }

        alg_anti_intf_set_agc_unlock(anti_inf, anti_inf->agc_unlock_def_rx_th);
        return OAL_FAIL;
    }
    return OAL_SUCC;
}
#ifdef _PRE_WLAN_FEATURE_DBAC
/******************************************************************************
 功能描述  : dbac运行时，设置为最小值
******************************************************************************/
OAL_STATIC osal_u32 alg_anti_intf_dbac_is_running_set_min_thr(alg_anti_interference_stru *anti_inf)
{
    hmac_device_stru *hmac_device = OSAL_NULL;
    hmac_device = hmac_res_get_mac_dev_etc(0);
    if (mac_is_dbac_running(hmac_device)) {
        if (anti_inf->debug_mode == ALG_ANTI_INF_HSO_DEBUG) {
            oam_warning_log0(0, OAM_SF_ANTI_INTF, "{[ANTI]DBAC is running}");
        } else if (anti_inf->debug_mode == ALG_ANTI_INF_SERIAL_DEBUG) {
#ifdef _PRE_WLAN_ALG_UART_PRINT
            wifi_printf("[ANTI]DBAC is running\r\n");
#endif
        }
        alg_anti_intf_set_agc_unlock(anti_inf, anti_inf->agc_unlock_def_rx_th);
        return OAL_FAIL;
    }
    return OAL_SUCC;
}
#endif
/******************************************************************************
 功能描述  : 调整有效门限
******************************************************************************/
WIFI_TCM_TEXT osal_void alg_anti_intf_adjust_and_set_thr(hal_to_dmac_device_stru *hal_device,
    alg_anti_interference_stru *anti_inf)
{
    osal_s32 tx_th_reg_val = anti_inf->dyn_unlock_tx_th;
    osal_s32 rx_th_reg_val = anti_inf->dyn_unlock_rx_th;
    /* 根据device工作mode调整门限 */
    wlan_bw_cap_enum_uint8 device_mode = hmac_get_device_bw_mode(hal_device);
    if (device_mode == WLAN_BW_CAP_40M) {
        tx_th_reg_val -= anti_inf->agc_unlock_bk_40m;
        rx_th_reg_val -= anti_inf->agc_unlock_bk_40m;
    }

    /* 门限范围[-95,-72],门限最大值从th_limit中获取 */
    tx_th_reg_val = osal_min(tx_th_reg_val, anti_inf->dyn_unlock_txth_limit);
    tx_th_reg_val = osal_max(tx_th_reg_val, anti_inf->agc_unlock_def_tx_th);
    rx_th_reg_val = osal_min(rx_th_reg_val, anti_inf->dyn_unlock_rxth_limit);
    rx_th_reg_val = osal_max(rx_th_reg_val, anti_inf->agc_unlock_def_rx_th);

    alg_anti_intf_set_agc_unlock(anti_inf, (osal_s8)rx_th_reg_val);
    return;
}

/******************************************************************************
 功能描述  : 设置agc tx/rx unlock门限,并写入对应phy寄存器
******************************************************************************/
WIFI_TCM_TEXT osal_void alg_anti_intf_set_dyn_unlock_th_reg(hal_to_dmac_device_stru *hal_device,
    alg_anti_interference_stru *anti_inf)
{
    /* 算法未开启,不设置寄存器 */
    if (anti_inf->unlock_config_enable == WLAN_ANTI_INTF_EN_OFF) {
        return;
    }

    if (alg_anti_intf_negtive_det_alg_off_set_min_thr(anti_inf) != OAL_SUCC) {
        return;
    }
#ifdef _PRE_WLAN_FEATURE_DBAC
    if (alg_anti_intf_dbac_is_running_set_min_thr(anti_inf) != OAL_SUCC) {
        return;
    }
#endif

    /* 动态agc unlock的tx/rx门限写入被定时器禁止 */
    if (anti_inf->dyn_unlock_enable == OSAL_FALSE) {
        return;
    }
    alg_anti_intf_adjust_and_set_thr(hal_device, anti_inf);

    return;
}

/******************************************************************************
 功能描述  : 获取对应用户AR信息
******************************************************************************/
OSAL_STATIC osal_u32 alg_anti_intf_get_user_ar_info(alg_anti_intf_info_stru *user_info_tmp,
    const hmac_user_stru *hmac_user, alg_anti_intf_user_info_stru *user_info,
    const alg_anti_interference_stru *anti_inf, oal_bool_enum_uint8 probe_dir)
{
    osal_u32 id_index;
    for (id_index = 0; id_index < user_info_tmp->user_num; id_index++) {
        if (user_info_tmp->user_ar_info[id_index].user_lut_idx == hmac_user->lut_index) {
            user_info->per_sum_last += user_info_tmp->user_ar_info[id_index].short_per;

            if (anti_inf->probe_keep_cyc_cnt[probe_dir] != anti_inf->probe_keep_cyc_num[probe_dir]) {
                return OAL_FAIL;
            }

            user_info->goodput_kbps_last[probe_dir] = user_info_tmp->user_ar_info[id_index].goodput;

            /* 计算平均per */
            user_info->per_avg_last[probe_dir] =
                user_info->per_sum_last / anti_inf->probe_keep_cyc_num[probe_dir];
        }
    }
    return OAL_SUCC;
}

OSAL_STATIC osal_void alg_anti_intf_refresh_user_stat_info(const hmac_vap_stru *hmac_vap,
    alg_anti_interference_stru *anti_inf, alg_anti_intf_info_stru *user_info_tmp, oal_bool_enum_uint8 probe_dir,
    oal_bool_enum_uint8 *has_negative)
{
    const hmac_user_stru *hmac_user = OSAL_NULL;
    struct osal_list_head *list_pos = OSAL_NULL;
    alg_anti_intf_user_info_stru *user_info = OSAL_NULL;
    osal_u32 ret;

    /* 负增益优化 */
    if ((anti_inf->inf_immune_enable != WLAN_ANTI_INTF_EN_PROBE) &&
        (anti_inf->unlock_config_enable != WLAN_ANTI_INTF_EN_PROBE)) {
        return;
    }

    alg_vap_foreach_user(hmac_user, hmac_vap, list_pos) {
        /* 如果用户尚未关联成功，则不访问私有信息结构体 */
        if (hmac_user->user_asoc_state != MAC_USER_STATE_ASSOC) {
            continue;
        }
        hmac_alg_get_user_priv_stru(hmac_user, HMAC_ALG_USER_STRU_ID_ANTI_INTF, (osal_void **)&user_info);
        if (osal_unlikely(user_info == OSAL_NULL)) {
            continue;
        }

        /* 获取goodput、per等统计信息 */
        ret = alg_anti_intf_get_user_ar_info(user_info_tmp, hmac_user, user_info, anti_inf, probe_dir);
        if (ret != OAL_SUCC) {
            continue;
        }

        if (anti_inf->probe_keep_cyc_cnt[1 - probe_dir] == anti_inf->probe_keep_cyc_num[1 - probe_dir]) {
            alg_anti_intf_calc_goodput_diff(anti_inf, user_info, has_negative);
        }
        user_info->per_sum_last = 0;
    }
}

OSAL_STATIC osal_void alg_anti_intf_refresh_vap_stat_info(alg_anti_interference_stru *anti_inf,
    oal_bool_enum_uint8 probe_dir, oal_bool_enum_uint8 *has_negative, hal_to_dmac_device_stru *hal_device)
{
    osal_u32 vap_index, vap_num;
    hmac_vap_stru *hmac_vap = OSAL_NULL;
    osal_u8 mac_vap_id[WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE] = {0};
    frw_msg msg_to_device = {0};
    alg_anti_intf_info_stru user_info_tmp = {0};
    osal_s32 ret;

    /* 遍历所有vap，并记录最大工作带宽 */
    vap_num = hal_device_find_all_up_vap(hal_device, mac_vap_id, WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE);
    /* 读取device侧碰撞率和goodput的接口 */
    memset_s(&user_info_tmp, sizeof(alg_anti_intf_info_stru), 0, sizeof(alg_anti_intf_info_stru));
    frw_msg_init(OSAL_NULL, 0, (osal_u8 *)&user_info_tmp, sizeof(alg_anti_intf_info_stru), &msg_to_device);
    ret = frw_send_msg_to_device(0, WLAN_MSG_H2D_C_CFG_GET_ANTI_INTF_STAT, &msg_to_device, OSAL_TRUE);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_ANTI_INTF, "{alg_anti_intf_refresh_user_stat_info:: send msg to device fail!");
        return;
    }

    for (vap_index = 0; vap_index < vap_num; vap_index++) {
        hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap_id[vap_index]);
        if (osal_unlikely(hmac_vap == OSAL_NULL)) {
            oam_warning_log1(0, OAM_SF_ANTI_INTF, "alg_anti_intf_rssi_stat_timer_handler::hmac_vap[%d] IS NULL.",
                mac_vap_id[vap_index]);
            continue;
        }

        /* 非STA单用户, 或DBDC下, 则不作动态外置LNA开关方案 */
        if (((is_legacy_sta(hmac_vap) == OSAL_FALSE) || (hmac_vap->user_nums != 1)) &&
            (anti_inf->dyn_extlna_off_bypass == OSAL_FALSE)) {
            anti_inf->dyn_extlna_off_bypass = OSAL_TRUE;
        }

        /* p2p device下不做处理 */
        if (is_p2p_dev(hmac_vap)) {
            continue;
        }
        alg_anti_intf_refresh_user_stat_info(hmac_vap, anti_inf, &user_info_tmp, probe_dir, has_negative);
    }
}

/******************************************************************************
 功能描述  : 清除统计信息
******************************************************************************/
OSAL_STATIC osal_void alg_anti_intf_clean_dmac_info(alg_anti_interference_stru *anti_inf)
{
    frw_msg msg_to_device = {0};
    osal_s32 ret;

    unref_param(anti_inf);

    /* 清除device侧数据统计 */
    frw_msg_init(OSAL_NULL, 0, OSAL_NULL, 0, &msg_to_device);
    ret = frw_send_msg_to_device(0, WLAN_MSG_H2D_C_CFG_CLEAR_ANTI_INTF_STAT, &msg_to_device, OSAL_TRUE);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANTI_INTF, "{alg_anti_intf_clean_probe_pkt_info:: send msg to device fail=%d!", ret);
        return;
    }
}
OSAL_STATIC osal_void alg_anti_intf_set_reg_val(alg_anti_interference_stru *anti_inf,
    hal_to_dmac_device_stru *hal_device)
{
    unref_param(hal_device);

    /* 设置门限并更新寄存器 */
    if ((anti_inf->rssi_min != ALG_ANTI_INF_RSSI_INVALID) &&
        (anti_inf->rssi_min > (anti_inf->agc_unlock_def_tx_th + anti_inf->rssi_bk))) {
        anti_inf->non_dir_rssi_th = anti_inf->rssi_min + ALG_ANTI_INF_NON_DIR_RSSI_TH_OFFSET;
        anti_inf->dyn_unlock_tx_th = alg_anti_intf_get_rssi_bk_val(anti_inf->rssi_min, anti_inf);
        anti_inf->dyn_unlock_rx_th = anti_inf->dyn_unlock_tx_th;
    } else {
        anti_inf->non_dir_rssi_th = anti_inf->non_dir_rssi_def_th;
        anti_inf->dyn_unlock_tx_th = anti_inf->agc_unlock_def_rx_th;
        anti_inf->dyn_unlock_rx_th = anti_inf->dyn_unlock_tx_th;
    }
    if (anti_inf->debug_mode == ALG_ANTI_INF_HSO_DEBUG) {
        oam_warning_log3(0, OAM_SF_ANTI_INTF, "{[ANTI_INTF_DBG]alg_anti_intf_set_reg_val:rssi_min[%d], \
            dyn_unlock_tx_th[%d], non_dir_rssi_th [%d]}",
            anti_inf->rssi_min, anti_inf->dyn_unlock_tx_th,
            anti_inf->non_dir_rssi_th);
    } else if (anti_inf->debug_mode == ALG_ANTI_INF_SERIAL_DEBUG) {
#ifdef _PRE_WLAN_ALG_UART_PRINT
        wifi_printf("[ANTI_INTF_DBG]alg_anti_intf_set_reg_val:rssi_min[%d], dyn_unlock_tx_th[%d], \
            non_dir_rssi_th [%d]\r\n", anti_inf->rssi_min,
            anti_inf->dyn_unlock_tx_th, anti_inf->non_dir_rssi_th);
#endif
    }
    if (osal_unlikely(alg_anti_intf_set_rssi_th_reg(anti_inf) != OAL_SUCC)) {
        oam_error_log0(0, OAM_SF_ANTI_INTF, "{alg_anti_intf_dyn_unlock_start_timer_handler: set_rssi_th_reg err!}");
    }
    alg_anti_intf_set_dyn_unlock_th_reg(anti_inf->hal_device, anti_inf);

    /* 门限参数生效后将实时参数值复位 */
    alg_anti_intf_clean_dmac_info(anti_inf);
    anti_inf->rssi_min = ALG_ANTI_INF_RSSI_INVALID;
    anti_inf->rssi_min_last = ALG_ANTI_INF_RSSI_INVALID;
    anti_inf->user_bitmap = 0; /* 每个周期都要收到所有用户的包才动态调弱干扰免疫门限 */
}
#if (defined(_PRE_WLAN_FEATURE_EDCA_OPT) && defined(_PRE_WLAN_FEATURE_INTF_DET))
/******************************************************************************
 功能描述  : 平滑前后两个周期的tx time
******************************************************************************/
OSAL_STATIC osal_u32 alg_anti_det_duty_cyc_smth(osal_u32 x, osal_u32 y, osal_u32 a)
{
    return (((x) >> (a)) + (y) - ((y) >> (a)));
}

/******************************************************************************
 功能描述  : tx time的负增益检测
******************************************************************************/
OSAL_STATIC osal_void alg_anti_intf_neg_det_of_tx_time(hal_to_dmac_device_stru *hal_device,
    alg_anti_interference_stru *anti_inf,
    oal_bool_enum_uint8 probe_dir, oal_bool_enum_uint8 *has_negative)
{
    unref_param(hal_device);

    if (anti_inf->probe_keep_cyc_cnt[probe_dir] == anti_inf->probe_keep_cyc_num[probe_dir]) {
        /* 增加tx time判别信息 */
        anti_inf->tx_time_avg[probe_dir] = alg_anti_det_duty_cyc_smth(anti_inf->tx_time_last,
            anti_inf->curr_tx_time, ALG_ANTI_INF_SMTH_SHIFT_BIT);
        anti_inf->tx_time_last = anti_inf->curr_tx_time;
        if (anti_inf->probe_keep_cyc_cnt[1 - probe_dir] == anti_inf->probe_keep_cyc_num[1 - probe_dir]) {
            alg_anti_intf_calc_tx_time_diff(anti_inf, has_negative);
        }
    }
    return;
}
#endif
/******************************************************************************
 功能描述  : 确定探测方向
******************************************************************************/
OSAL_STATIC osal_void alg_anti_intf_probe_direct_confirm(alg_anti_interference_stru *anti_inf,
    oal_bool_enum_uint8 probe_dir, oal_bool_enum_uint8 has_negative)
{
    if (anti_inf->probe_keep_cyc_cnt[probe_dir] == anti_inf->probe_keep_cyc_num[probe_dir]) {
        if (anti_inf->probe_keep_cyc_cnt[1 - probe_dir] == anti_inf->probe_keep_cyc_num[1 - probe_dir]) {
            /* 根据识别结果判断下次探测方向 */
            alg_anti_intf_next_probe_proc(anti_inf, (1 - has_negative));
        } else {
            anti_inf->probe_direct = 1 - probe_dir;
        }
    }
    return;
}

/******************************************************************************
 功能描述  : RSSI统计定时器到期的处理函数(T1)
******************************************************************************/
osal_u32 alg_anti_intf_rssi_stat_timer_handler(osal_void *void_code)
{
    oal_bool_enum_uint8 has_negative = OSAL_FALSE;
    alg_anti_interference_stru *anti_inf = (alg_anti_interference_stru *)void_code;
    hal_to_dmac_device_stru *hal_device = OSAL_NULL;
    oal_bool_enum_uint8 probe_dir;
    oal_bool_enum_uint8 switch_enable = alg_host_get_gla_switch_enable(ALG_GLA_ID_WEAK_IMMUNE, ALG_GLA_OPTIONAL_SWITCH);
    if (osal_unlikely((anti_inf == OSAL_NULL) || (anti_inf->hal_device == OSAL_NULL))) {
        oam_error_log1(0, OAM_SF_ANY, "{alg_tx_mgmt_notify:: ptr is null!%d}", anti_inf == OSAL_NULL);
        return OAL_ERR_CODE_PTR_NULL;
    }
#ifdef _PRE_WLAN_FEATURE_POWERSAVE
    if (hmac_sta_pm_get_wlan_dev_state() == HMAC_ALLOW_SLEEP) {
        return OAL_FAIL;
    }
#endif
    hal_device = anti_inf->hal_device;

    anti_inf->dyn_extlna_off_bypass = OSAL_FALSE;
    probe_dir = anti_inf->probe_direct;
    if (probe_dir >= 2) { /* 2:判断dir */
        oam_error_log1(0, OAM_SF_ANTI_INTF, "{alg_anti_intf_rssi_stat_timer_handler: dir = %u, invalid!}", probe_dir);
        return OAL_FAIL;
    }
    anti_inf->probe_keep_cyc_cnt[probe_dir] += ((anti_inf->inf_immune_enable == WLAN_ANTI_INTF_EN_PROBE) ||
        (anti_inf->unlock_config_enable == WLAN_ANTI_INTF_EN_PROBE)) ? 1 : 0;
#if (defined(_PRE_WLAN_FEATURE_EDCA_OPT) && defined(_PRE_WLAN_FEATURE_INTF_DET))
    alg_anti_intf_neg_det_of_tx_time(hal_device, anti_inf, probe_dir, &has_negative);
#endif

    /* 遍历所有vap，并记录最大工作带宽 */
    alg_anti_intf_refresh_vap_stat_info(anti_inf, probe_dir, &has_negative, hal_device);

    alg_anti_intf_probe_direct_confirm(anti_inf, probe_dir, has_negative);

    if ((anti_inf->debug_mode == ALG_ANTI_INF_HSO_DEBUG) || (switch_enable == OSAL_TRUE)) {
        oam_warning_log1(0, OAM_SF_ANTI_INTF, "[GLA][ALG][WEAK_IMMUNE]:inf_immune_enable=%d", probe_dir);
    } else if (anti_inf->debug_mode == ALG_ANTI_INF_SERIAL_DEBUG) {
#ifdef _PRE_WLAN_ALG_UART_PRINT
        wifi_printf("[ANTI_INTF_DBG]T1 set weak immune th!\r\n");
#endif
    }
    alg_anti_intf_set_reg_val(anti_inf, hal_device);

    return OAL_SUCC;
}

/******************************************************************************
 功能描述  : 关闭动态agc unlock周期定时器到期的处理函数(T2)
******************************************************************************/
osal_u32 alg_anti_intf_dyn_unlock_off_timer_handler(osal_void *void_code)
{
    alg_anti_interference_stru *anti_inf = OSAL_NULL;
    oal_bool_enum_uint8 switch_enable = alg_host_get_gla_switch_enable(ALG_GLA_ID_WEAK_IMMUNE, ALG_GLA_OPTIONAL_SWITCH);

    /* 判断入参合法性 */
    if (osal_unlikely(void_code == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_ANTI_INTF, "{alg_anti_intf_dyn_unlock_off_timer_handler: input pointer is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    anti_inf = (alg_anti_interference_stru *)void_code;
    /* 关闭动态unlock，将reg设为较小值 */
    anti_inf->dyn_unlock_enable = OSAL_FALSE;

    if ((anti_inf->unlock_config_enable == WLAN_ANTI_INTF_EN_ON) ||
        (anti_inf->unlock_config_enable == WLAN_ANTI_INTF_EN_PROBE)) {
        alg_anti_intf_set_agc_unlock(anti_inf, anti_inf->agc_unlock_def_rx_th);
    }
    if ((anti_inf->debug_mode == ALG_ANTI_INF_HSO_DEBUG) || (switch_enable == OSAL_TRUE)) {
        oam_warning_log1(0, OAM_SF_ANTI_INTF, "[GLA][ALG][WEAK_IMMUNE]:dyn_unlock_enable=%d",
            anti_inf->dyn_unlock_enable);
    } else if (anti_inf->debug_mode == ALG_ANTI_INF_SERIAL_DEBUG) {
#ifdef _PRE_WLAN_ALG_UART_PRINT
        wifi_printf("[ANTI_INTF_DBG]T2 turn off set dyn agc unlock th!\r\n");
#endif
    }

    /* 所有vap都是sta模式，按照sta周期；否则按ap周期 */
    if (alg_anti_intf_is_all_vap_mode_sta(anti_inf->hal_device)) {
        anti_inf->timeout_t3 = anti_inf->dyn_unlock_start_timeout_sta;
    } else {
        anti_inf->timeout_t3 = ALG_ANTI_INF_DYN_UNLOCK_OFF_DUR_MS_AP;
    }

    /* 启动agc unlock关闭时间定时器 */
    frw_create_timer_entry(&(anti_inf->dyn_unlock_start_timer), alg_anti_intf_dyn_unlock_start_timer_handler,
        anti_inf->timeout_t3, (osal_void *)(anti_inf), OSAL_FALSE);

    return OAL_SUCC;
}

/******************************************************************************
 功能描述  : 动态unlock off持续时间定时器到期的处理函数(T3)
******************************************************************************/
osal_u32 alg_anti_intf_dyn_unlock_start_timer_handler(osal_void *void_code)
{
    alg_anti_interference_stru *anti_inf = (alg_anti_interference_stru *)void_code;
    oal_bool_enum_uint8 switch_enable = alg_host_get_gla_switch_enable(ALG_GLA_ID_WEAK_IMMUNE, ALG_GLA_OPTIONAL_SWITCH);

    if (osal_unlikely(anti_inf == OSAL_NULL || anti_inf->hal_device == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_ANTI_INTF, "{alg_anti_intf_dyn_unlock_start_timer_handler: param null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 使能动态agc unlock功能, 并设置寄存器为当前计算值 */
    anti_inf->dyn_unlock_enable = OSAL_TRUE;
    if ((anti_inf->debug_mode == ALG_ANTI_INF_HSO_DEBUG) || (switch_enable == OSAL_TRUE)) {
        oam_warning_log1(0, OAM_SF_ANTI_INTF, "[GLA][ALG][WEAK_IMMUNE]:dyn_unlock_enable=%d",
            anti_inf->dyn_unlock_enable);
    } else if (anti_inf->debug_mode == ALG_ANTI_INF_SERIAL_DEBUG) {
#ifdef _PRE_WLAN_ALG_UART_PRINT
        wifi_printf("[ANTI_INTF_DBG]T3 turn on set dyn agc unlock th!\r\n");
#endif
    }
    alg_anti_intf_set_dyn_unlock_th_reg(anti_inf->hal_device, anti_inf);

    return OAL_SUCC;
}
#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
/******************************************************************************
 功能描述  : non direct丢包门限参数配置
******************************************************************************/
OSAL_STATIC osal_void alg_anti_intf_config_inf_immune_enable(alg_anti_interference_stru *anti_inf, osal_u8 value)
{
    anti_inf->inf_immune_enable = value;
    anti_inf->probe_keep_cyc_cnt[0] = 0;
    anti_inf->probe_keep_cyc_cnt[1] = 0;
    anti_inf->probe_keep_cyc_num[0] = anti_inf->keep_cyc_min_num;
    anti_inf->probe_keep_cyc_num[1] = anti_inf->keep_cyc_min_num;

    /* 配置命令简化 */
    if (anti_inf->inf_immune_enable == WLAN_ANTI_INTF_EN_OFF) {
        if (anti_inf->debug_mode == ALG_ANTI_INF_HSO_DEBUG) {
            oam_warning_log0(0, OAM_SF_ANTI_INTF, "{[ANTI_INTF_DBG]config turn off non direct th!}");
        } else if (anti_inf->debug_mode == ALG_ANTI_INF_SERIAL_DEBUG) {
#ifdef _PRE_WLAN_ALG_UART_PRINT
            wifi_printf("[ANTI_INTF_DBG]config turn off non direct th!\r\n");
#endif
        }

        alg_anti_intf_set_non_dir_rssi(anti_inf, anti_inf->non_dir_rssi_def_th);
    }
    return;
}
/******************************************************************************
 功能描述  : AGC unlock门限参数配置
******************************************************************************/
OSAL_STATIC osal_void alg_anti_intf_config_agc_unlock_enable(alg_anti_interference_stru *anti_inf, osal_u8 value)
{
    anti_inf->unlock_config_enable = value;

    anti_inf->probe_keep_cyc_cnt[0] = 0;
    anti_inf->probe_keep_cyc_cnt[1] = 0;
    anti_inf->probe_keep_cyc_num[0] = anti_inf->keep_cyc_min_num;
    anti_inf->probe_keep_cyc_num[1] = anti_inf->keep_cyc_min_num;

    /* 配置命令简化 */
    if (anti_inf->unlock_config_enable == WLAN_ANTI_INTF_EN_OFF) {
        if (anti_inf->debug_mode == ALG_ANTI_INF_HSO_DEBUG) {
            oam_warning_log0(0, OAM_SF_ANTI_INTF, "{[ANTI_INTF_DBG]config turn off agc unlock th!}");
        } else if (anti_inf->debug_mode == ALG_ANTI_INF_SERIAL_DEBUG) {
#ifdef _PRE_WLAN_ALG_UART_PRINT
            wifi_printf("[ANTI_INTF_DBG]config turn off agc unlock th!\r\n");
#endif
        }
        alg_anti_intf_set_agc_unlock(anti_inf, anti_inf->agc_unlock_def_rx_th);
    }
    return;
}
/******************************************************************************
 功能描述  : 单个参数配置函数补充
******************************************************************************/
OSAL_STATIC osal_u32 alg_anti_intf_config_param_resume_second(alg_anti_interference_stru *anti_inf,
    const mac_ioctl_alg_param_stru *alg_param)
{
    oal_bool_enum_uint8 tmp = OSAL_FALSE;
    osal_u32 times = 0;
    switch (alg_param->alg_cfg) {
        case MAC_ALG_CFG_ANTI_INTF_GOODPUT_JITTER_TH:
            anti_inf->goodput_jitter_th = (osal_u8)alg_param->value;
            break;
        case MAC_ALG_CFG_ANTI_INTF_KEEP_CYC_MAX_NUM:
            anti_inf->keep_cyc_max_num = (osal_u8)alg_param->value;
            break;
        case MAC_ALG_CFG_ANTI_INTF_KEEP_CYC_MIN_NUM:
            anti_inf->keep_cyc_min_num = (osal_u8)alg_param->value;
            /* 探测保持的最小周期个数不可小于或等于0 */
            if (anti_inf->keep_cyc_min_num <= 0) {
                oam_warning_log1(0, OAM_SF_ANTI_INTF, "{alg_anti_intf_config_param:param error! cyc_min_num = %d",
                                 anti_inf->keep_cyc_min_num);
                return OAL_FAIL;
            }
            break;
        case MAC_ALG_CFG_ANTI_INTF_DEBUG_MODE:
            anti_inf->debug_mode = (osal_u8)alg_param->value;
            break;
        case MAC_ALG_CFG_ANTI_INTF_NAV_IMM_ENABLE:
            tmp = (oal_bool_enum_uint8)alg_param->value;
            if (tmp == OSAL_TRUE) {
                hal_set_nav_max_duration(ALG_ANTI_INF_MAX_DUR_VAL, ALG_ANTI_INF_OBSS_MAX_DUR_VAL);
            } else {
                hal_set_nav_max_duration(ALG_ANTI_INF_MAX_DUR_VAL, ALG_ANTI_INF_MAX_DUR_VAL);
            }
            oam_warning_log1(0, OAM_SF_ANTI_INTF, "{alg_anti_intf_config_param succ! anti_inf_nav_enable = %d", tmp);
#ifdef _PRE_WLAN_ALG_UART_PRINT
            wifi_printf("alg_anti_intf_config_param succ! anti_inf_nav_enable = %d.\r\n", tmp);
#endif
            break;
        case MAC_ALG_CFG_ANTI_INTF_NAV_IMM_ENQUIRY:
            tmp = (oal_bool_enum_uint8)alg_param->value;
            if (tmp == OSAL_TRUE) {
                hal_get_nav_protect_times(&times);
                oam_warning_log1(0, OAM_SF_ANTI_INTF, "{alg_anti_intf_config_param succ! anti_inf_nav_protect_times = \
                    %d.", times);
#ifdef _PRE_WLAN_ALG_UART_PRINT
                wifi_printf("{alg_anti_intf_config_param succ! anti_inf_nav_protect_times = %d.\r\n", times);
#endif
            }
            break;
        default:
            oam_warning_log0(0, OAM_SF_ANTI_INTF, "{alg_anti_intf_config_param:config param error!}");
            return OAL_FAIL;
    }
    return OAL_SUCC;
}
/******************************************************************************
 功能描述  : 单个参数配置函数补充
******************************************************************************/
OSAL_STATIC osal_u32 alg_anti_intf_config_param_resume_first(alg_anti_interference_stru *anti_inf,
    const mac_ioctl_alg_param_stru *alg_param)
{
    osal_u32 value = alg_param->value;
    mac_alg_cfg_enum alg_cfg = alg_param->alg_cfg;
    switch (alg_cfg) {
        case MAC_ALG_CFG_ANTI_INTF_UNLOCK_CYCLE:
            frw_create_timer_entry(&(anti_inf->dyn_unlock_off_timer), alg_anti_intf_dyn_unlock_off_timer_handler,
                value, (osal_void *)(anti_inf), OSAL_TRUE);
            break;
        case MAC_ALG_CFG_ANTI_INTF_UNLOCK_DUR_TIME:
            anti_inf->timeout_t3 = (osal_u16)alg_param->value;
            frw_destroy_timer_entry(&(anti_inf->dyn_unlock_start_timer));
            break;
        case MAC_ALG_CFG_ANTI_INTF_PER_PROBE_EN:
            anti_inf->per_probe_en = (osal_u8)alg_param->value;
            break;
        case MAC_ALG_CFG_ANTI_INTF_GOODPUT_FALL_TH:
            anti_inf->goodput_fall_th = (osal_u8)alg_param->value;
            break;
        case MAC_ALG_CFG_ANTI_INTF_TX_TIME_FALL_TH:
            anti_inf->tx_time_fall_th = (osal_u8)alg_param->value;
            break;
        case MAC_ALG_CFG_ANTI_INTF_PER_FALL_TH:
            anti_inf->per_fall_th = (osal_u8)alg_param->value;
            break;
        default:
            if (alg_anti_intf_config_param_resume_second(anti_inf, alg_param) != OAL_SUCC) {
                return OAL_FAIL;
            }
            return OAL_SUCC;
    }
    oam_warning_log2(0, OAM_SF_ANTI_INTF, "alg_anti_intf_config_param_resume,type=%d,value=%d",
        alg_cfg, value);
    return OAL_SUCC;
}

/******************************************************************************
 功能描述  : 单个参数配置函数
******************************************************************************/
osal_u32 alg_anti_intf_config_param(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    alg_anti_interference_stru *anti_inf = OSAL_NULL;
    hal_to_dmac_device_stru *hal_device = OSAL_NULL;
    mac_ioctl_alg_param_stru *alg_param = (mac_ioctl_alg_param_stru *)msg->data;
    osal_u32 ret;

    if ((alg_param->alg_cfg <= MAC_ALG_CFG_ANTI_INTF_START) || (alg_param->alg_cfg >= MAC_ALG_CFG_ANTI_INTF_END)) {
        return OAL_SUCC;
    }
    hal_device = hmac_vap->hal_device;
    if (osal_unlikely(hal_device == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_ANTI_INTF, "{alg_anti_intf_config_param: hal device NULL!}");
    }

    ret = hmac_alg_get_device_priv_stru(hal_device, HAL_ALG_DEVICE_STRU_ID_ANTI_INTF, (osal_void **)&anti_inf);
    if (osal_unlikely(ret != OAL_SUCC)) {
        return ret;
    }
#ifdef _PRE_WLAN_ALG_UART_PRINT
    wifi_printf("alg_anti_intf_config_param %d %d\r\n", alg_param->alg_cfg, alg_param->value);
#endif
    oam_warning_log2(0, OAM_SF_ANY, "{alg_anti_intf_config_param: alg_anti_intf_config ID[%d] value[%d]!}",
        alg_param->alg_cfg, alg_param->value);
    /* 配置对应参数 */
    switch (alg_param->alg_cfg) {
        case MAC_ALG_CFG_ANTI_INTF_IMM_ENABLE:
            if (anti_inf->inf_immune_enable != alg_param->value) {
                alg_anti_intf_config_inf_immune_enable(anti_inf, (osal_u8)alg_param->value);
            }
            break;
        case MAC_ALG_CFG_ANTI_INTF_UNLOCK_ENABLE:
            if (anti_inf->unlock_config_enable != alg_param->value) {
                alg_anti_intf_config_agc_unlock_enable(anti_inf, (osal_u8)alg_param->value);
            }
            anti_inf->scan_agculk_status = (alg_param->value == 0) ? OSAL_FALSE : OSAL_TRUE;
            break;
        case MAC_ALG_CFG_ANTI_INTF_RSSI_STAT_CYCLE:
            frw_create_timer_entry(&(anti_inf->rssi_stat_timer), alg_anti_intf_rssi_stat_timer_handler,
                alg_param->value, (osal_void *)(anti_inf), OSAL_TRUE);
            break;
        default:
            if (alg_anti_intf_config_param_resume_first(anti_inf, alg_param) != OAL_SUCC) {
                oam_warning_log0(0, OAM_SF_ANTI_INTF, "{alg_anti_intf_config_param:config param error!}");
                return OAL_FAIL;
            }
    }

    if ((alg_param->alg_cfg == MAC_ALG_CFG_ANTI_INTF_DEBUG_MODE) &&
        (frw_send_msg_to_device(hmac_vap->vap_id, WLAN_MSG_H2D_C_CFG_ALG_PARAM, msg, OSAL_FALSE) != OAL_SUCC)) {
        oam_warning_log0(0, OAM_SF_ANY, "{alg_anti_intf_config_param: frw_send_msg_to_device failed!}");
        return OAL_FAIL;
    }

    return OAL_SUCC;
}
#endif

/******************************************************************************
 功能描述  : 判断device的所有vap是否都是sta模式
******************************************************************************/
oal_bool_enum_uint8 alg_anti_intf_is_all_vap_mode_sta(const hal_to_dmac_device_stru *hal_device)
{
    hmac_vap_stru *hmac_vap = OSAL_NULL;
    osal_u32 vap_index, up_vap_num;
    osal_u8 mac_vap_id[WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE] = {0};

    up_vap_num = hal_device_find_all_up_vap(hal_device, mac_vap_id, WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE);
    for (vap_index = 0; vap_index < up_vap_num; vap_index++) {
        hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap_id[vap_index]);
        if (osal_unlikely(hmac_vap == OSAL_NULL)) {
            oam_error_log1(0, OAM_SF_ANTI_INTF, "alg_intf_det_clean_user_pkt_info::hmac_vap[%d] IS NULL.",
                           mac_vap_id[vap_index]);
            continue;
        }

        /* 非STA模式都按ap模式设置 */
        if (hmac_vap->hal_vap->vap_mode != WLAN_VAP_MODE_BSS_STA) {
            return OSAL_FALSE;
        }
    }

    return OSAL_TRUE;
}

/******************************************************************************
 功能描述  : TBTT中断钩子函数
******************************************************************************/
osal_u32 alg_anti_intf_tbtt_isr(hmac_vap_stru *hmac_vap, hal_to_dmac_device_stru *hal_device)
{
    alg_anti_interference_stru *anti_intf = OSAL_NULL;
    osal_u32 ret;

    if (osal_unlikely(hmac_vap == NULL)) {
        oam_error_log0(0, OAM_SF_ANTI_INTF, "{alg_anti_intf_tbtt_isr::hmac_vap == NULL}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    ret = hmac_alg_get_device_priv_stru(hal_device, HAL_ALG_DEVICE_STRU_ID_ANTI_INTF, (osal_void **)&anti_intf);
    if (osal_unlikely((ret != OAL_SUCC) || (anti_intf == OSAL_NULL))) {
        oam_warning_log1(0, OAM_SF_ANTI_INTF, "{tbtt_isr:[%d]get_device_priv_stru fail!}", hmac_vap->assoc_vap_id);
        return ret;
    }

    /* 连续8次没有收到beacon帧，将agc寄存器设为最小值 */
    if ((hmac_vap->hal_vap->vap_mode == WLAN_VAP_MODE_BSS_STA) &&
        (hmac_vap->vap_state == MAC_VAP_STATE_UP)) {
        if ((hmac_vap->linkloss_info.linkloss_rate > LINKLOSS_CNT_UNDER_8) &&
            (anti_intf->agc_unlock_last != anti_intf->agc_unlock_def_rx_th)) {
            if ((anti_intf->unlock_config_enable == WLAN_ANTI_INTF_EN_ON) ||
                (anti_intf->unlock_config_enable == WLAN_ANTI_INTF_EN_PROBE)) {
                alg_anti_intf_set_agc_unlock(anti_intf, anti_intf->agc_unlock_def_rx_th);
                oam_warning_log0(0, OAM_SF_ANTI_INTF, "{[ANTI_INTF_DBG]tbtt isr linkloss_rate > 8!}");
            }
        }
    }
    return OAL_SUCC;
}

/******************************************************************************
 功能描述  : 根据场景识别结果，下次探测的处理函数
******************************************************************************/
osal_void alg_anti_intf_next_probe_proc(alg_anti_interference_stru *anti_intf, oal_bool_enum_uint8 next_dir)
{
    anti_intf->probe_direct = next_dir;
    anti_intf->probe_keep_cyc_cnt[0] = 0;
    anti_intf->probe_keep_cyc_cnt[1] = 0;
    anti_intf->probe_keep_cyc_num[next_dir] *= 2; /* 2:自乘2 */
    anti_intf->probe_keep_cyc_num[next_dir] =
        osal_min(anti_intf->probe_keep_cyc_num[next_dir], anti_intf->keep_cyc_max_num);
    anti_intf->probe_keep_cyc_num[1 - next_dir] = anti_intf->keep_cyc_min_num;
    if (anti_intf->debug_mode == ALG_ANTI_INF_HSO_DEBUG) {
        oam_warning_log3(0, OAM_SF_ANTI_INTF, "{[ANTI_INTF_DBG]next probe direct[%d], alg_on_cyc_cnt[%d], \
        alg_off_cyc_cnt[%d]!}", next_dir, anti_intf->probe_keep_cyc_num[1], anti_intf->probe_keep_cyc_num[0]);
    } else if (anti_intf->debug_mode == ALG_ANTI_INF_SERIAL_DEBUG) {
#ifdef _PRE_WLAN_ALG_UART_PRINT
        wifi_printf("[ANTI_INTF_DBG]next probe direct[%d]!\r\n", next_dir);
#endif
    }
}

/******************************************************************************
 功能描述  : 计算一轮探测goodput的判别结果
******************************************************************************/
osal_void alg_anti_intf_calc_goodput_diff(const alg_anti_interference_stru *anti_intf,
    const alg_anti_intf_user_info_stru *user_info, oal_bool_enum_uint8 *has_negative)
{
    osal_s32 goodput_diff;
    osal_s32 goodput_fall_ratio;
    osal_u32 divisior = 1;

    /* 计算goodput变化比值 */
    divisior = osal_max(divisior, user_info->goodput_kbps_last[0]);
    goodput_diff = (osal_s32)(user_info->goodput_kbps_last[0] - user_info->goodput_kbps_last[1]);
    goodput_fall_ratio = 100 * goodput_diff / (osal_s32)divisior; /* 100:相乘 */

    if (anti_intf->debug_mode == ALG_ANTI_INF_HSO_DEBUG) {
        oam_warning_log3(0, OAM_SF_ANTI_INTF, "{ratio = %d, gp[0] = %d, gp[1] = %d}", goodput_fall_ratio,
            user_info->goodput_kbps_last[0], user_info->goodput_kbps_last[1]);
    } else if (anti_intf->debug_mode == ALG_ANTI_INF_SERIAL_DEBUG) {
#ifdef _PRE_WLAN_ALG_UART_PRINT
        wifi_printf("[ANTI_INTF_DBG]ratio = %d, gp[0] = %d, gp[1] = %d.\r\n", goodput_fall_ratio,
            user_info->goodput_kbps_last[0], user_info->goodput_kbps_last[1]);
#endif
    }

    if (goodput_fall_ratio >= (osal_s32)anti_intf->goodput_fall_th) {
        *has_negative = OSAL_TRUE;

        return;
    }
    goodput_fall_ratio = (goodput_fall_ratio > 0) ? goodput_fall_ratio : (-goodput_fall_ratio);
    /* 计算per变化值 */
    if (anti_intf->per_probe_en == OSAL_TRUE) {
        /* goodput抖动小于2%, 并且per增大2%以上，认为负增益 */
        if ((anti_intf->goodput_jitter_th >= goodput_fall_ratio) &&
            ((user_info->per_avg_last[1] - user_info->per_avg_last[0]) > (osal_s32)anti_intf->per_fall_th)) {
            *has_negative = OSAL_TRUE;
        }

        if (anti_intf->debug_mode == ALG_ANTI_INF_HSO_DEBUG) {
            oam_warning_log3(0, OAM_SF_ANTI_INTF, "[ANTI_INTF_DBG]per[0] = %d, per[1] = %d, sum_cur = %d",
                user_info->per_avg_last[0], user_info->per_avg_last[1], user_info->per_sum_last);
        }
    }
}

#if (defined(_PRE_WLAN_FEATURE_EDCA_OPT) && defined(_PRE_WLAN_FEATURE_INTF_DET))

/******************************************************************************
 功能描述  : 计算一轮探测tx time的判别结果
******************************************************************************/
osal_void alg_anti_intf_calc_tx_time_diff(alg_anti_interference_stru *anti_intf,
    oal_bool_enum_uint8 *has_negative)
{
    osal_s32 tx_time_diff;
    osal_s32 tx_time_diff_ratio = 0;

    /* 计算tx time变化比 */
    if ((anti_intf->tx_time_avg[0] > 0) && (anti_intf->tx_time_avg[1] > 0)) {
        tx_time_diff = (osal_s32)(anti_intf->tx_time_avg[0] - anti_intf->tx_time_avg[1]);
        tx_time_diff_ratio = 100 * tx_time_diff / (osal_s32)anti_intf->tx_time_avg[0]; /* 100:倍数 */

        if (tx_time_diff_ratio >= (osal_s32)anti_intf->tx_time_fall_th) {
            *has_negative = OSAL_TRUE;
        }
    }

    if (anti_intf->debug_mode == ALG_ANTI_INF_HSO_DEBUG) {
        oam_warning_log4(0, OAM_SF_ANTI_INTF, "{tx_time: dir = %d, ratio = %d, avg[0] = %d, avg[1] = %d}",
            anti_intf->probe_direct, tx_time_diff_ratio,
            anti_intf->tx_time_avg[0], anti_intf->tx_time_avg[1]);
    } else if (anti_intf->debug_mode == ALG_ANTI_INF_SERIAL_DEBUG) {
#ifdef _PRE_WLAN_ALG_UART_PRINT
        wifi_printf("[ANTI_INTF_DBG]tx_time: dir = %d, ratio = %d, avg[0] = %d, avg[1] = %d.\r\n",
            anti_intf->probe_direct, tx_time_diff_ratio, anti_intf->tx_time_avg[0], anti_intf->tx_time_avg[1]);
#endif
    }

    anti_intf->tx_time_avg[0] = 0;
    anti_intf->tx_time_avg[1] = 0;
}

#endif

OSAL_STATIC osal_u32 alg_anti_intf_calc_unlock_th_process(alg_anti_interference_stru *anti_intf, osal_u8 up_vap_num,
    const osal_u8 *mac_vap_id, osal_u16 mac_vap_size, wlan_channel_band_enum_uint8 *band)
{
    hmac_vap_stru *hmac_vap = OSAL_NULL;
    osal_u32 vap_index;
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);
    unref_param(mac_vap_size);

    for (vap_index = 0; vap_index < up_vap_num; vap_index++) {
        hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap_id[vap_index]);
        if (osal_unlikely(hmac_vap == OSAL_NULL)) {
            oam_error_log1(0, OAM_SF_ANTI_INTF, "alg_intf_det_clean_user_pkt_info::hmac_vap[%d] IS NULL.",
                           mac_vap_id[vap_index]);
            continue;
        }

        /* p2p dev,非业务vap不处理 */
        if (is_p2p_dev(hmac_vap)) {
            continue;
        }

        /* device下有ap(GO)则默认使用若干扰门限值 */
        if (hmac_vap->hal_vap->vap_mode == WLAN_VAP_MODE_BSS_AP || mac_is_dbac_running(hmac_device) == OSAL_TRUE) {
            anti_intf->dyn_unlock_txth_limit = anti_intf->agc_unlock_max_tx_th;
            anti_intf->dyn_unlock_rxth_limit = anti_intf->agc_unlock_max_rx_th;
            if (anti_intf->debug_mode == ALG_ANTI_INF_HSO_DEBUG) {
                oam_warning_log1(0, OAM_SF_ANTI_INTF, "{[ANTI_INTF_DBG]max dyn agc unlock th[%d]!}",
                    anti_intf->dyn_unlock_txth_limit);
            } else if (anti_intf->debug_mode == ALG_ANTI_INF_SERIAL_DEBUG) {
#ifdef _PRE_WLAN_ALG_UART_PRINT
                wifi_printf("[ANTI_INTF_DBG]max dyn agc unlock th[%d]!\r\n", anti_intf->dyn_unlock_txth_limit);
#endif
            }
            return OAL_FAIL;
        }

        if (hmac_vap->channel.band == WLAN_BAND_5G) {
            *band = WLAN_BAND_5G;
        }
    }
    return OAL_SUCC;
}

/******************************************************************************
 功能描述  : 计算保存的agc unlock最大阈值
******************************************************************************/
osal_void alg_anti_intf_calc_unlock_th_limit(const hal_to_dmac_device_stru *hal_device,
    alg_anti_interference_stru *anti_intf)
{
#ifdef _PRE_WLAN_FEATURE_CCA_OPT
    osal_s8                      ac_acc_limit[ALG_ANTI_INF_CCA_THR_NUM] = {0};
    osal_u32                     index;
    osal_u8                      up_vap_num;
    wlan_channel_band_enum_uint8 band = WLAN_BAND_2G;
    osal_u8                      mac_vap_id[WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE] = {0};
    osal_s32                     dyn_unlock_th_limit = 0;

    /* 获取cca协议门限 */
    hal_get_cca_reg_th(ac_acc_limit, ALG_ANTI_INF_CCA_THR_NUM);

    up_vap_num = hal_device_find_all_up_vap(hal_device, mac_vap_id, WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE);
    if (alg_anti_intf_calc_unlock_th_process(anti_intf, up_vap_num, mac_vap_id,
        WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE, &band) != OAL_SUCC) {
        return;
    }

    /* 2.4Gmode 不考虑vht cca; 5Gmode 不考虑11b cca */
    if (band == WLAN_BAND_2G) {
        ac_acc_limit[2] = 0; /* 2:下标 */
    } else {
        ac_acc_limit[0] = 0;
    }
    /* 获取cca寄存器最小门限值 */
    for (index = 0; index < ALG_ANTI_INF_CCA_THR_NUM; index++) {
        dyn_unlock_th_limit = osal_min(dyn_unlock_th_limit, ac_acc_limit[index]);
    }
#else
    const osal_s32 dyn_unlock_th_limit = 0;
#endif
    /* 保存agc unlock门限最大阈值 */
    anti_intf->dyn_unlock_txth_limit = osal_min((osal_s8)dyn_unlock_th_limit, anti_intf->agc_unlock_max_tx_th);
    anti_intf->dyn_unlock_rxth_limit = osal_min((osal_s8)dyn_unlock_th_limit, anti_intf->agc_unlock_max_rx_th);
    if (anti_intf->debug_mode == ALG_ANTI_INF_HSO_DEBUG) {
        oam_warning_log1(0, OAM_SF_ANTI_INTF, "{[ANTI_INTF_DBG]max dyn agc unlock th[%d]!}",
            anti_intf->dyn_unlock_txth_limit);
    } else if (anti_intf->debug_mode == ALG_ANTI_INF_SERIAL_DEBUG) {
#ifdef _PRE_WLAN_ALG_UART_PRINT
        wifi_printf("[ANTI_INTF_DBG]max dyn agc unlock th[%d]!\r\n", anti_intf->dyn_unlock_txth_limit);
#endif
    }
}

/******************************************************************************
 功能描述  : 算法开关配置处理函数
******************************************************************************/
osal_u32 alg_anti_intf_switch(hal_to_dmac_device_stru *hal_device, oal_bool_enum_uint8 alg_enable)
{
    alg_anti_interference_stru *anti_intf = OSAL_NULL;
    oal_bool_enum_uint8 switch_enable = alg_host_get_gla_switch_enable(ALG_GLA_ID_WEAK_IMMUNE, ALG_GLA_OPTIONAL_SWITCH);
    osal_u32 ret;

    if (osal_unlikely((hal_device == OSAL_NULL) || (alg_enable > WLAN_ANTI_INTF_EN_PROBE))) {
        /* CCA扫描逻辑需要移出普通扫描流程，不然这里会出现vap删除之后hal device指针为空，仍进入该逻辑 */
        oam_warning_log2(0, OAM_SF_ANTI_INTF,
            "{alg_anti_intf_switch: check input para fail, hal_device[%p],alg_enable[%d]!}",
            (uintptr_t)hal_device, alg_enable);
        return OAL_FAIL;
    }

    ret = hmac_alg_get_device_priv_stru(hal_device, HAL_ALG_DEVICE_STRU_ID_ANTI_INTF, (osal_void **)&anti_intf);
    if (osal_unlikely(ret != OAL_SUCC)) {
        return ret;
    }
    if (anti_intf->debug_mode == ALG_ANTI_INF_SERIAL_DEBUG) {
#ifdef _PRE_WLAN_ALG_UART_PRINT
        wifi_printf("[ANTI_INTF_DBG]alg_anti_intf_switch: alg_enable[%d]\r\n", alg_enable);
#endif
    } else if ((anti_intf->debug_mode == ALG_ANTI_INF_HSO_DEBUG) || (switch_enable == OSAL_TRUE)) {
        oam_warning_log1(0, OAM_SF_ANTI_INTF, "[GLA][ALG][WEAK_IMMUNE]:dyn_unlock_enable=%d", alg_enable);
    }

    if (alg_enable == OSAL_FALSE) {
        frw_timer_stop_timer(&(anti_intf->dyn_unlock_off_timer));
        frw_timer_stop_timer(&(anti_intf->dyn_unlock_start_timer));
        frw_timer_stop_timer(&(anti_intf->rssi_stat_timer));
        anti_intf->dyn_unlock_enable = alg_enable;

        /* 配置算法关闭，默认值为最小 */
        alg_anti_intf_set_agc_unlock(anti_intf, anti_intf->agc_unlock_def_rx_th);
    } else if (anti_intf->scan_agculk_status == OSAL_TRUE) {
        frw_create_timer_entry(&(anti_intf->dyn_unlock_off_timer), alg_anti_intf_dyn_unlock_off_timer_handler,
            anti_intf->timeout_t2, (osal_void *)(anti_intf), OSAL_TRUE);
        frw_create_timer_entry(&(anti_intf->rssi_stat_timer), alg_anti_intf_rssi_stat_timer_handler,
            anti_intf->timeout_t1, (osal_void *)(anti_intf), OSAL_TRUE);

        anti_intf->dyn_unlock_enable = alg_enable;
    } else {
        return OAL_SUCC;
    }

    return OAL_SUCC;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif /* #ifdef _PRE_WLAN_FEATURE_ANTI_INTERF */