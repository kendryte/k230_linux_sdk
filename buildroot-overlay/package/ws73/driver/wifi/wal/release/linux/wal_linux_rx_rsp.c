/*
 * Copyright (c) CompanyNameMagicTag 2001-2023. All rights reserved.
 * Description: 接收驱动上报过来的消息，上报给内核
 */

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "oal_ext_if.h"
#include "wlan_types_common.h"
#include "frw_ext_if.h"
#include "mac_device_ext.h"
#include "mac_resource_ext.h"
#include "mac_vap_ext.h"
#include "mac_regdomain.h"
#include "mac_ie.h"
#include "hmac_ext_if.h"
#include "wal_ext_if.h"
#include "wal_main.h"
#include "wal_config.h"
#include "wal_linux_util.h"
#include "wal_linux_scan.h"
#include "wal_linux_cfg80211.h"
#include "wal_linux_ioctl.h"
#include "oal_cfg80211.h"
#include "oal_netbuf.h"
#include "hmac_resource.h"
#include "hmac_device.h"
#include "hmac_scan.h"
#include "hmac_11r.h"

#include "hmac_user.h"
#include "plat_pm_wlan.h"
#include "hmac_feature_dft.h"

#ifdef _PRE_WLAN_DFT_STAT
#include "hmac_dfx.h"
#endif
#include "common_dft.h"
#ifdef CONTROLLER_CUSTOMIZATION
#include "plat_pm_dfr.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_WAL_LINUX_RX_RSP_C
#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/

/*****************************************************************************
  3 函数实现
*****************************************************************************/

/*****************************************************************************
 函 数 名  : wal_scan_report
 功能描述  : 上报扫描完成事件处理
 输入参数  : scan_mgmt
 输出参数  : 无

 修改历史      :
  1.日    期   : 2016年7月29日
    修改内容   : 新生成函数

*****************************************************************************/
OAL_STATIC osal_void wal_scan_report(hmac_scan_stru *scan_mgmt, oal_bool_enum is_aborted)
{
    /* 通知 kernel scan 已经结束 */
    oal_cfg80211_scan_done_etc(scan_mgmt->request, is_aborted);

    scan_mgmt->request = OAL_PTR_NULL;
    scan_mgmt->complete = OAL_TRUE;

    oam_warning_log1(0, OAM_SF_SCAN,
        "{wal_scan_report::scan complete, release scan_requset, abort flag[%d]!}", is_aborted);

    /* 让编译器优化时保证OAL_WAIT_QUEUE_WAKE_UP在最后执行 */
    oal_smp_mb();
    osal_wait_wakeup(&scan_mgmt->wait_queue);
}

#ifdef _PRE_WLAN_FEATURE_PNO_SCAN
/*****************************************************************************
 函 数 名  : wal_schedule_scan_report
 功能描述  : 上报PNO扫描完成事件处理
 输入参数  : wiphy,scan_mgmt
 输出参数  : 无

 修改历史      :
  1.日    期   : 2016年7月29日
    修改内容   : 新生成函数

*****************************************************************************/
OAL_STATIC osal_void wal_schedule_scan_report(oal_wiphy_stru *wiphy, hmac_scan_stru *scan_mgmt)
{
    /* 上报调度扫描结果 */
    oal_cfg80211_sched_scan_result_etc(wiphy);

    scan_mgmt->sched_scan_req     = OAL_PTR_NULL;
    scan_mgmt->sched_scan_complete = OAL_TRUE;

    oam_warning_log0(0, OAM_SF_SCAN, "{wal_schedule_scan_report::sched scan complete.!}");
}
#endif

OAL_STATIC osal_u32 wal_scan_comp_proc_report(hmac_scan_stru *scan_mgmt,
    const hmac_scan_rsp_stru *scan_rsp, oal_wiphy_stru *wiphy)
{
    oal_bool_enum is_aborted;

    /* 对于内核下发的扫描request资源加锁 */
    osal_mutex_lock(&(scan_mgmt->scan_request_mutexlock));

    /* 没有未释放的扫描资源，直接返回 */
    if ((scan_mgmt->request == OAL_PTR_NULL) && (scan_mgmt->sched_scan_req == OAL_PTR_NULL)) {
        /* 通知完内核，释放资源后解锁 */
        osal_mutex_unlock(&(scan_mgmt->scan_request_mutexlock));
        oam_warning_log0(0, OAM_SF_SCAN, "{wal_scan_comp_proc_sta_etc::legacy scan and pno scan are complete!}");

        return OAL_SUCC;
    }

    if ((scan_mgmt->request != OAL_PTR_NULL) && (scan_mgmt->sched_scan_req != OAL_PTR_NULL)) {
        /* 一般情况下,2个扫描同时存在是一种异常情况,在此添加打印,暂不做异常处理 */
        oam_warning_log0(0, OAM_SF_SCAN, "{wal_scan_comp_proc_sta_etc::legacy scan and pno scan are all started!!!}");
    }

    /* 上层下发的普通扫描进行对应处理 */
#ifdef _PRE_WLAN_FEATURE_PNO_SCAN
    if (scan_rsp->result_code == MAC_SCAN_PNO) {
        /* PNO扫描结束事件 */
        if (scan_mgmt->sched_scan_req != OAL_PTR_NULL) {
            /* 存在PNO扫描结束上报但驱动PNO扫描已经结束的情况,
            * 即普通scan打断PNO scan,此时device正巧上报PNO扫描结束事件
            */
            wal_schedule_scan_report(wiphy, scan_mgmt);
        } else {
            oam_warning_log0(0, OAM_SF_SCAN, "{wal_scan_comp_proc_sta_etc::sched scan already complete!}");
        }
    } else {
#endif
        /* 普通扫描结束事件 */
        if (scan_mgmt->request != OAL_PTR_NULL) {
            /* 是scan abort的话告知内核 */
            is_aborted = (MAC_SCAN_ABORT == scan_rsp->result_code) ? OAL_TRUE : OAL_FALSE;
            wal_scan_report(scan_mgmt, is_aborted);
        } else {
            oam_warning_log0(0, OAM_SF_SCAN, "{wal_scan_comp_proc_sta_etc::scan already complete!}");
        }
#ifdef _PRE_WLAN_FEATURE_PNO_SCAN
    }
#endif
    /* 通知完内核，释放资源后解锁 */
    osal_mutex_unlock(&(scan_mgmt->scan_request_mutexlock));

    return OAL_SUCC;
}
/*****************************************************************************
 函 数 名  : wal_scan_comp_proc_sta_etc
 功能描述  : STA上报扫描完成事件处理
 输入参数  : event_mem: 事件内存
 输出参数  : 无

 修改历史      :
  1.日    期   : 2013年7月5日
    修改内容   : 新生成函数
  2.日    期   : 2013年9月9日
    修改内容   : 增加上报内核部分函数

*****************************************************************************/
osal_s32 wal_scan_comp_proc_sta_etc(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    hmac_scan_rsp_stru *scan_rsp = OAL_PTR_NULL;
    hmac_device_stru *hmac_device = OAL_PTR_NULL;
    hmac_bss_mgmt_stru *bss_mgmt = OAL_PTR_NULL;
    hmac_scan_stru *scan_mgmt = OAL_PTR_NULL;
    oal_wiphy_stru *wiphy = OAL_PTR_NULL;

    /* 删除等待扫描超时定时器 */
    if (hmac_vap->scan_timeout.is_registerd == OAL_TRUE) {
        frw_destroy_timer_entry(&(hmac_vap->scan_timeout));
    }

    /* 获取hmac device 指针 */
    hmac_device = hmac_res_get_mac_dev_etc(0);
    if (hmac_device == OAL_PTR_NULL) {
        oam_error_log1(0, OAM_SF_SCAN,
            "vap_id[%d] {wal_scan_comp_proc_sta_etc::hmac_device is null!}", hmac_vap->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }
    scan_mgmt = &(hmac_device->scan_mgmt);
    wiphy      = hmac_device->wiphy;

    /* 获取扫描结果的管理结构地址 */
    bss_mgmt = &(hmac_device->scan_mgmt.scan_record_mgmt.bss_mgmt);

    /* 获取驱动上报的扫描结果结构体指针 */
    scan_rsp = (hmac_scan_rsp_stru *)msg->data;

    /* 如果扫描返回结果的非成功，打印维测信息 */
    if ((scan_rsp->result_code != MAC_SCAN_SUCCESS) && (scan_rsp->result_code != MAC_SCAN_PNO)) {
        oam_warning_log2(0, OAM_SF_SCAN, "vap_id[%d] {wal_scan_comp_proc_sta_etc::scan not succ, err_code[%d]!}",
                         hmac_vap->vap_id, scan_rsp->result_code);
    }

    /* 上报所有扫描到的bss, 无论扫描结果成功与否，统一上报扫描结果，有几个上报几个 */
    wal_inform_all_bss_etc(wiphy, bss_mgmt, hmac_vap->vap_id);

    return (osal_s32)wal_scan_comp_proc_report(scan_mgmt, scan_rsp, wiphy);
}

static inline osal_void wal_asoc_comp_proc_setup_result(hmac_asoc_rsp_stru *asoc_rsp,
    oal_connet_result_stru *result)
{
    if (memcpy_s(result->bssid, sizeof(result->bssid), asoc_rsp->addr_ap, WLAN_MAC_ADDR_LEN) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_asoc_comp_proc_setup_result::memcpy_s error}");
    }
    result->req_ie = asoc_rsp->asoc_req_ie_buff;
    result->req_ie_len = asoc_rsp->asoc_req_ie_len;
    result->rsp_ie = asoc_rsp->asoc_rsp_ie_buff;
    result->rsp_ie_len = asoc_rsp->asoc_rsp_ie_len;
    result->status_code = asoc_rsp->status_code;
    return;
}

static osal_u32 wal_asoc_comp_proc_succ(oal_net_device_stru *netdev, hmac_asoc_rsp_stru *asoc_rsp,
    oal_connet_result_stru *result, osal_u8 vap_id)
{
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,34))
    osal_u32 ret;
#endif
    wal_asoc_comp_proc_setup_result(asoc_rsp, result);

    /* 关联成功，上报关联结果前，先更新对应bss 信息，
     * 避免由于关联前不扫描，内核将bss 老化，4次握手完成后下发秘钥提示失败情况 */
    if (result->status_code == MAC_SUCCESSFUL_STATUSCODE) {
        hmac_device_stru    *hmac_device = OAL_PTR_NULL;
        oal_wiphy_stru      *wiphy = OAL_PTR_NULL;
        hmac_bss_mgmt_stru  *bss_mgmt = OAL_PTR_NULL;
        hmac_device = hmac_res_get_mac_dev_etc(0);
        if ((hmac_device == OAL_PTR_NULL) ||
            (hmac_device->wiphy == OAL_PTR_NULL)) {
            oam_error_log1(0, OAM_SF_ASSOC,
                "vap_id[%d] {wal_asoc_comp_proc_sta_etc::get ptr is null!}", vap_id);
            return OAL_ERR_CODE_PTR_NULL;
        }
        wiphy       = hmac_device->wiphy;
        bss_mgmt    = &(hmac_device->scan_mgmt.scan_record_mgmt.bss_mgmt);

        wal_update_bss_etc(wiphy, bss_mgmt, result->bssid);
    }
    /* 关联成功，上报关联结果前，先更新对应bss 信息，
     * 避免由于关联前不扫描，内核将bss 老化，4次握手完成后下发秘钥提示失败情况 */
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,34))
    oal_cfg80211_connect_result_etc(netdev, result->bssid, result->req_ie, result->req_ie_len,
        result->rsp_ie, result->rsp_ie_len, result->status_code, GFP_ATOMIC);
#else
    ret = oal_cfg80211_connect_result_etc(netdev, result->bssid, result->req_ie,
        result->req_ie_len, result->rsp_ie, result->rsp_ie_len, result->status_code, GFP_ATOMIC);
    if (ret != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_ASSOC,
                         "vap_id[%d] {wal_asoc_comp_proc_sta_etc::oal_cfg80211_connect_result_etc fail[%d]!}",
                         vap_id, ret);
    }
#endif
    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_11D
/* sta关联成功后，更新自己的管制域信息 */
STATIC osal_u32  wal_regdomain_update_sta_etc(osal_u8 vap_id)
{
    osal_s8 *desired_country;
    oal_net_device_stru *net_dev;
    osal_s32 ret;
    oal_bool_enum_uint8 updata_rd_by_ie_switch;

    hmac_vap_get_updata_rd_by_ie_switch_etc(vap_id, &updata_rd_by_ie_switch);

    if (OAL_TRUE == updata_rd_by_ie_switch) {
        desired_country = hmac_vap_get_desired_country_etc(vap_id);
        if (OAL_UNLIKELY(OAL_PTR_NULL == desired_country)) {
            oam_error_log1(0, OAM_SF_ANY, "vap_id[%d] {wal_regdomain_update_sta_etc::desired_country is null ptr!}",
                           vap_id);
            return OAL_ERR_CODE_PTR_NULL;
        }
        /* 期望的国家码全为0，表示对端AP的国家码不存在，采用sta当前默认的国家码 */
        if ((desired_country[0] == 0) && (desired_country[1] == 0)) {
            oam_info_log1(0, OAM_SF_ANY,
                "vap_id[%d] {wal_regdomain_update_sta_etc::ap does not have country ie, use default!}", vap_id);
            return OAL_SUCC;
        }
        net_dev = hmac_vap_get_net_device_etc(vap_id);
        if (OAL_UNLIKELY(OAL_PTR_NULL == net_dev)) {
            oam_error_log1(0, OAM_SF_ANY, "vap_id[%d] {wal_regdomain_update_sta_etc::net_dev is null ptr!}",
                           vap_id);
            return OAL_ERR_CODE_PTR_NULL;
        }
        ret = wal_util_set_country_code(net_dev, desired_country);
        if (ret != OAL_SUCC) {
            oam_warning_log2(0, OAM_SF_ANY,
                "vap_id[%d] {wal_regdomain_update_sta_etc::wal_util_set_country_code err code %d!}", vap_id, ret);
            return OAL_FAIL;
        }
        oam_info_log3(0, OAM_SF_ANY, "vap_id[%d] {wal_util_set_country_code::country is %u, %u!}", vap_id,
                      (osal_u8)desired_country[0], (osal_u8)desired_country[1]);
    } else {
        oam_info_log1(0, OAM_SF_ANY,
            "vap_id[%d] {wal_regdomain_update_sta_etc::updata_rd_by_ie_switch is OAL_FALSE!}", vap_id);
    }
    return OAL_SUCC;
}
#endif

/*****************************************************************************
 STA上报关联完成事件处理
*****************************************************************************/
osal_s32  wal_asoc_comp_proc_sta_etc(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    oal_connet_result_stru result;
    oal_net_device_stru *netdev;
    hmac_asoc_rsp_stru *assoc_rsp;
    osal_u32 ul_ret = OAL_SUCC;

    assoc_rsp  = (hmac_asoc_rsp_stru *)msg->data;

    /* 获取net_device */
    netdev = hmac_vap_get_net_device_etc(hmac_vap->vap_id);
    if (netdev == OAL_PTR_NULL) {
        oam_error_log1(0, OAM_SF_ASSOC, "vap_id[%d] {wal_asoc_comp_proc_sta_etc::get net device ptr is null!}",
                       hmac_vap->vap_id);
        oal_free(assoc_rsp->asoc_rsp_ie_buff);
        assoc_rsp->asoc_rsp_ie_buff = OAL_PTR_NULL;
        return OAL_ERR_CODE_PTR_NULL;
    }

    (osal_void)memset_s(&result, sizeof(oal_connet_result_stru), 0, sizeof(oal_connet_result_stru));
    ul_ret = wal_asoc_comp_proc_succ(netdev, assoc_rsp, &result, hmac_vap->vap_id);
    oal_free(assoc_rsp->asoc_rsp_ie_buff);
    assoc_rsp->asoc_rsp_ie_buff = OAL_PTR_NULL;
    if (ul_ret != OAL_SUCC) {
        return ul_ret;
    }

#ifdef _PRE_WLAN_FEATURE_11D
    /* 如果关联成功，sta根据AP的国家码设置自己的管制域 */
    if (assoc_rsp->result_code == HMAC_MGMT_SUCCESS) {
        wal_regdomain_update_sta_etc(hmac_vap->vap_id);
    }
#endif

    /* 启动发送队列，防止发送队列被漫游关闭后无法恢复 */
    oal_net_tx_wake_all_queues(netdev);
    oam_warning_log2(0, OAM_SF_ASSOC, "vap_id[%d] {wal_asoc_comp_proc_sta_etc status_code[%d] OK!}",
                     hmac_vap->vap_id, result.status_code);
    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_ROAM
/*****************************************************************************
 函 数 名  : wal_roam_comp_proc_sta_etc
 功能描述  : STA上报漫游完成事件处理
 输入参数  : event_mem: 事件内存
 输出参数  : 无

 修改历史      :
  1.日    期   : 2015年6月18日
    修改内容   : 新生成函数

*****************************************************************************/
osal_s32  wal_roam_comp_proc_sta_etc(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    oal_net_device_stru *net_device;
    hmac_device_stru *hmac_device;
    hmac_roam_rsp_stru *roam_rsp;
    struct ieee80211_channel *channel;
    oal_ieee80211_band_enum_uint8 band = OAL_IEEE80211_NUM_BANDS;
    osal_slong freq;

    roam_rsp  = (hmac_roam_rsp_stru *)msg->data;

    /* 获取net_device */
    net_device = hmac_vap_get_net_device_etc(hmac_vap->vap_id);
    if (net_device == OAL_PTR_NULL) {
        oam_error_log1(0, OAM_SF_ROAM, "vap_id[%d] {wal_asoc_comp_proc_sta_etc::get net device ptr is null!}",
                       hmac_vap->vap_id);
        oal_free(roam_rsp->asoc_rsp_ie_buff);
        roam_rsp->asoc_rsp_ie_buff = OAL_PTR_NULL;
        return OAL_ERR_CODE_PTR_NULL;
    }
    /* 获取device id 指针 */
    hmac_device = hmac_res_get_mac_dev_etc(0);
    if (hmac_device == OAL_PTR_NULL) {
        oam_warning_log0(0, OAM_SF_SCAN, "{wal_asoc_comp_proc_sta_etc:: dev[0] is null.}");
        oal_free(roam_rsp->asoc_rsp_ie_buff);
        roam_rsp->asoc_rsp_ie_buff = OAL_PTR_NULL;
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (roam_rsp->st_channel.band >= WLAN_BAND_BUTT) {
        oam_error_log2(0, OAM_SF_ROAM, "vap_id[%d] {wal_asoc_comp_proc_sta_etc::unexpected band[%d]!}",
                       hmac_vap->vap_id, roam_rsp->st_channel.band);
        oal_free(roam_rsp->asoc_rsp_ie_buff);
        roam_rsp->asoc_rsp_ie_buff = OAL_PTR_NULL;
        return OAL_FAIL;
    }

    band = (roam_rsp->st_channel.band == WLAN_BAND_2G) ? OAL_IEEE80211_BAND_2GHZ : OAL_IEEE80211_BAND_5GHZ;

    freq = oal_ieee80211_channel_to_frequency(roam_rsp->st_channel.chan_number, band);

    channel = (struct ieee80211_channel *)oal_ieee80211_get_channel(hmac_device->wiphy, freq);

    /* 调用内核接口，上报关联结果 */
    oal_cfg80211_roamed_etc(net_device, channel, roam_rsp->bssid, roam_rsp->asoc_req_ie_buff,
                            roam_rsp->asoc_req_ie_len, roam_rsp->asoc_rsp_ie_buff,
                            roam_rsp->asoc_rsp_ie_len, GFP_ATOMIC);
    oam_warning_log4(0, OAM_SF_ASSOC,
        "{wal_roam_comp_proc_sta_etc::oal_cfg80211_roamed_etc OK asoc_req_ie[%p] len[%d] asoc_rsp_ie[%p] len[%d]!}",
        (uintptr_t)(roam_rsp->asoc_req_ie_buff), roam_rsp->asoc_req_ie_len, (uintptr_t)(roam_rsp->asoc_rsp_ie_buff),
        roam_rsp->asoc_rsp_ie_len);
    oal_free(roam_rsp->asoc_rsp_ie_buff);
    roam_rsp->asoc_rsp_ie_buff = OAL_PTR_NULL;

    return OAL_SUCC;
}
#endif /* _PRE_WLAN_FEATURE_ROAM */
#ifdef _PRE_WLAN_FEATURE_11R
/*****************************************************************************
 函 数 名  : wal_ft_event_proc_sta_etc
 功能描述  : STA上报FT事件处理
*****************************************************************************/
osal_s32 wal_ft_event_proc_sta_etc(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    osal_u8 *ie_buf = OSAL_NULL;
    oal_net_device_stru *net_device = OSAL_NULL;
    hmac_roam_ft_stru *ft_event = OSAL_NULL;
    oal_cfg80211_ft_event_stru cfg_ft_event = {0};
    ft_event = (hmac_roam_ft_stru *)msg->data;
    ie_buf = msg->data + sizeof(hmac_roam_ft_stru);

    /* 获取net_device */
    net_device = hmac_vap_get_net_device_etc(hmac_vap->vap_id);
    if (OAL_PTR_NULL == net_device) {
        oam_error_log1(0, OAM_SF_ROAM, "vap_id[%d] {wal_ft_event_proc_sta_etc::get net device ptr is null!}",
                       hmac_vap->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    cfg_ft_event.ies           = ie_buf;
    cfg_ft_event.ies_len       = ft_event->ft_ie_len;
    cfg_ft_event.target_ap     = ft_event->bssid;
    cfg_ft_event.ric_ies       = OAL_PTR_NULL;
    cfg_ft_event.ric_ies_len   = 0;

    /* 调用内核接口，上报关联结果 */
    oal_cfg80211_ft_event_etc(net_device, &cfg_ft_event);
    return OAL_SUCC;
}
#endif /* _PRE_WLAN_FEATURE_11R */

/*****************************************************************************
 函 数 名  : wal_disasoc_comp_event_proc
 功能描述  : STA上报去关联完成事件处理
*****************************************************************************/
osal_s32 wal_disasoc_comp_proc_sta_etc(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    oal_disconnect_result_stru disconnect_result;
    oal_net_device_stru *net_device;
    osal_u16 *pus_disasoc_reason_code;
    osal_bool locally_generated = 0;
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,34))
    osal_u32 ret;
#endif

    /* 获取net_device */
    net_device = hmac_vap_get_net_device_etc(hmac_vap->vap_id);
    if (net_device == OAL_PTR_NULL) {
        oam_error_log1(0, OAM_SF_ASSOC, "vap_id[%d] {wal_disasoc_comp_proc_sta_etc::get net device ptr is null!}",
                       hmac_vap->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取去关联原因码指针 */
    pus_disasoc_reason_code = (osal_u16 *)msg->data;
    /* 去关联reason = 6强制触发wpa进行扫描 */
    if (*pus_disasoc_reason_code == MAC_NOT_AUTHED) {
        *pus_disasoc_reason_code = MAC_DEAUTH_LV_SS;
    }

    (osal_void)memset_s(&disconnect_result, sizeof(oal_disconnect_result_stru), 0,
        sizeof(oal_disconnect_result_stru));

    /* 准备上报内核的关联结果结构体 */
    disconnect_result.reason_code = *pus_disasoc_reason_code;

    /* 调用内核接口，上报去关联结果 */
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,34))
#ifdef CONTROLLER_CUSTOMIZATION
    /* Android suspend休眠使用dfr流程, 上报断连需为locally_generated */
    if ((plat_is_device_in_recovery() == OSAL_TRUE) && (*pus_disasoc_reason_code == MAC_UNSPEC_REASON)) {
        locally_generated = 1;
    }
#endif
    oal_cfg80211_disconnected_etc(net_device, disconnect_result.reason_code, disconnect_result.pus_disconn_ie,
        disconnect_result.disconn_ie_len, locally_generated, GFP_ATOMIC);
#else
    ret = oal_cfg80211_disconnected_etc(net_device, disconnect_result.reason_code, disconnect_result.pus_disconn_ie,
        disconnect_result.disconn_ie_len, locally_generated, GFP_ATOMIC);
    if (ret != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_ASSOC,
            "vap_id[%d] {wal_disasoc_comp_proc_sta_etc::oal_cfg80211_disconnected_etc fail[%d]!}",
            hmac_vap->vap_id, ret);
        return ret;
    }
#endif
    /* sta模式下，不需要传mac地址 */
    oam_warning_log2(0, OAM_SF_ASSOC, "vap_id[%d] {wal_disasoc_comp_proc_sta_etc reason_code[%d] OK!}",
                     hmac_vap->vap_id, disconnect_result.reason_code);
    return OAL_SUCC;
}

OAL_STATIC osal_u32 wal_connect_new_sta_report(oal_net_device_stru *net_device,
    osal_u8 connect_user_addr[WLAN_MAC_ADDR_LEN], oal_station_info_stru *station_info, osal_u8 vap_id)
{
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,34))
    osal_u32 rc;
#endif

/* 调用内核接口，上报STA关联结果 */
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,34))
    oal_cfg80211_new_sta_etc(net_device, connect_user_addr, station_info, GFP_ATOMIC);
#else
    rc = oal_cfg80211_new_sta_etc(net_device, connect_user_addr, station_info, GFP_ATOMIC);
    if (rc != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_ASSOC,
            "vap_id[%d] {wal_connect_new_sta_proc_ap_etc::oal_cfg80211_new_sta_etc fail[%d]!}",
            vap_id, rc);
        return rc;
    }
#endif

    oam_warning_log4(0, OAM_SF_ASSOC, "{wal_connect_new_sta_proc_ap_etc mac[%02X:%02X:%02X:%02X:XX:XX] OK!}",
        /* 0:1:2:3:数组下标 */
        connect_user_addr[0], connect_user_addr[1], connect_user_addr[2], connect_user_addr[3]);

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : wal_connect_new_sta_proc_ap_etc
 功能描述  : 驱动上报内核bss网络中新加入了一个STA
 输入参数  : 无
 输出参数  : 无

 修改历史      :
  1.日    期   : 2013年9月9日
    修改内容   : 新生成函数

*****************************************************************************/
osal_s32 wal_connect_new_sta_proc_ap_etc(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    oal_net_device_stru *net_device = OAL_PTR_NULL;
    osal_u8 connect_user_addr[WLAN_MAC_ADDR_LEN];
    oal_station_info_stru station_info;
    hmac_asoc_user_req_ie_stru  *asoc_user_req_info = OAL_PTR_NULL;

    /* 获取net_device */
    net_device = hmac_vap_get_net_device_etc(hmac_vap->vap_id);
    if (net_device == OAL_PTR_NULL) {
        oam_error_log1(0, OAM_SF_ASSOC, "vap_id[%d] {wal_connect_new_sta_proc_ap_etc::get net device ptr is null!}",
                       hmac_vap->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    (osal_void)memset_s(&station_info, sizeof(oal_station_info_stru), 0, sizeof(oal_station_info_stru));
    asoc_user_req_info = (hmac_asoc_user_req_ie_stru *)(msg->data);

#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 2, 0))
    /* 向内核标记填充了关联请求帧的ie信息 */
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0))
    /* Linux 4.0 版本不需要STATION_INFO_ASSOC_REQ_IES 标识 */
#else
    station_info.filled |=  STATION_INFO_ASSOC_REQ_IES;
#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0)) */
    station_info.assoc_req_ies = asoc_user_req_info->assoc_req_ie_buff;
    if (station_info.assoc_req_ies == OAL_PTR_NULL) {
        oam_error_log1(0, OAM_SF_ASSOC, "vap_id[%d] {wal_connect_new_sta_proc_ap_etc::asoc ie is null!}",
                       hmac_vap->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }
    station_info.assoc_req_ies_len = asoc_user_req_info->assoc_req_ie_len;
#endif

    /* 获取关联user mac addr */
    if (memcpy_s(connect_user_addr, sizeof(connect_user_addr),
        (osal_u8 *)asoc_user_req_info->user_mac_addr, WLAN_MAC_ADDR_LEN) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_connect_new_sta_proc_ap_etc::memcpy_s mac error}");
    }

    /* 上报关联结果 */
    return (osal_s32)wal_connect_new_sta_report(net_device, (osal_u8 *)connect_user_addr, &station_info,
        hmac_vap->vap_id);
}

/*****************************************************************************
 函 数 名  : wal_disconnect_sta_proc_ap_etc
 功能描述  : 驱动上报内核bss网络中删除了一个STA
 输入参数  : 无
 输出参数  : 无

 修改历史      :
  1.日    期   : 2013年9月9日
    修改内容   : 新生成函数

*****************************************************************************/
osal_s32 wal_disconnect_sta_proc_ap_etc(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    oal_net_device_stru *net_device;
    osal_u8 disconn_user_addr[WLAN_MAC_ADDR_LEN];
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE < KERNEL_VERSION(3,10,44))
    osal_s32 ret;
#endif

    /* 获取net_device */
    net_device = hmac_vap_get_net_device_etc(hmac_vap->vap_id);
    if (net_device == OAL_PTR_NULL) {
        oam_error_log1(0, OAM_SF_ASSOC, "vap_id[%d] {wal_disconnect_sta_proc_ap_etc::get net device ptr is null!}\r\n",
                       hmac_vap->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取去关联user mac addr */
    if (memcpy_s(disconn_user_addr, sizeof(disconn_user_addr), (osal_u8 *)msg->data, WLAN_MAC_ADDR_LEN) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_disconnect_sta_proc_ap_etc::memcpy_s error}");
    }

    /* 调用内核接口，上报STA去关联结果 */
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,44))
    oal_cfg80211_del_sta_etc(net_device, disconn_user_addr, GFP_ATOMIC);
#else
    ret = oal_cfg80211_del_sta_etc(net_device, (osal_u8 *)disconn_user_addr, GFP_ATOMIC);
    if (ret != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_ASSOC,
                         "vap_id[%d] {wal_disconnect_sta_proc_ap_etc::oal_cfg80211_del_sta_etc return fail[%d]!}",
                         hmac_vap->vap_id, ret);
        return OAL_FAIL;
    }
#endif
    oam_warning_log1(0, OAM_SF_ASSOC, "vap_id[%d]", hmac_vap->vap_id);
    oam_warning_log4(0, OAM_SF_ASSOC, "{wal_disconnect_sta_proc_ap_etc mac[%02X:%02X:%02X:%02X:XX:XX] OK!}",
        /* 0:1:2:3:数组下标 */
        disconn_user_addr[0], disconn_user_addr[1], disconn_user_addr[2], disconn_user_addr[3]);

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : wal_mic_failure_proc_etc
 功能描述  : 驱动上报内核mic错误
*****************************************************************************/
osal_s32 wal_mic_failure_proc_etc(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    oal_net_device_stru *net_device = OSAL_NULL;
    hmac_mic_event_stru *mic_event = OSAL_NULL;

    mic_event = (hmac_mic_event_stru *)(msg->data);

    /* 获取net_device */
    net_device = hmac_vap_get_net_device_etc(hmac_vap->vap_id);
    if (net_device == OAL_PTR_NULL) {
        oam_error_log1(0, OAM_SF_CRYPTO, "vap_id[%d] {wal_mic_failure_proc_etc::get net device ptr is null!}",
                       hmac_vap->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 调用内核接口，上报mic攻击 */
    oal_cfg80211_mic_failure_etc(net_device, mic_event->user_mac, mic_event->key_type,
                                 mic_event->key_id, NULL, GFP_ATOMIC);

    oam_warning_log1(0, OAM_SF_CRYPTO, "vap_id[%d]", hmac_vap->vap_id);
    oam_warning_log4(0, OAM_SF_CRYPTO, "{wal_mic_failure_proc_etc::mac[%02X:%02X:%02X:%02X:XX:XX] OK!}",
        /* 0:1:2:3:数组下标 */
        mic_event->user_mac[0], mic_event->user_mac[1], mic_event->user_mac[2], mic_event->user_mac[3]);
    return OAL_SUCC;
}

#ifdef _PRE_WLAN_DFT_STAT
/*****************************************************************************
 功能描述  : vendor IE管理帧上报, 打印第一个匹配的vendor IE完整信息
*****************************************************************************/
osal_void wal_dump_vendor_ie_info(hmac_vap_stru *hmac_vap, osal_u8 frame_type, osal_u16 frame_len,
    osal_u8 *ie, osal_u8 ie_len)
{
    osal_u16 i;
    osal_char *buf = OSAL_NULL;
    const osal_s32 buf_len = 1024;
    osal_s32 ret = 0;

    buf = (osal_char *)oal_mem_alloc(OAL_MEM_POOL_ID_LOCAL, buf_len, OAL_TRUE);
    if (buf == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_RX, "wal_dump_vendor_ie_info::malloc() FAILED");
        return;
    }

    (osal_void)memset_s(buf, buf_len, 0, buf_len);
    wifi_printf("vendor IE frame info\n");
    wifi_printf("vap_id=[%d] frame_type=[%s], frame_len=[%d], ie_len=[%d], payload= \n",
        hmac_vap->vap_id, (frame_type == WLAN_FC0_SUBTYPE_PROBE_REQ) ? "PROBE_REQ" : "ACTION", frame_len, ie_len);
    for (i = 0; i < ie_len; i++) {
        ret += (osal_s32)snprintf_s(buf + ret, buf_len - ret, buf_len - ret, "%02x ", ie[i]);
    }
    wifi_printf("%s\n", buf);
    oal_mem_free(buf, OAL_TRUE);
}

/*****************************************************************************
 功能描述  : vendor IE管理帧上报, 携带 vendor IE 的 probe req/action 帧信息打印
*****************************************************************************/
osal_void wal_dump_mgmt_with_vendor_ie(hmac_vap_stru *hmac_vap, osal_u8 *payload,
    osal_u16 frame_len, oal_ieee80211_mgmt *mgmt)
{
    osal_u16 ie_len;
    mac_app_ie_stru *app_ie = OSAL_NULL;
    mac_ieee80211_vendor_ie_stru *ie_req = OSAL_NULL;
    mac_ieee80211_vendor_ie_stru *vendor_ie = OSAL_NULL;
    osal_u32 ie_oui;
    osal_u8 frame_type;
    osal_u8 *ie_tmp = OSAL_NULL;

    if (osal_unlikely(hmac_vap == OSAL_NULL || payload == OSAL_NULL || mgmt == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_RX, "{wal_dump_mgmt_with_vendor_ie::null pointer}");
        return;
    }

    frame_type = mgmt->frame_control & 0xFC;
    if ((frame_type != WLAN_FC0_SUBTYPE_PROBE_REQ) && (frame_type != WLAN_FC0_SUBTYPE_ACTION)) {
        oam_info_log1(0, OAM_SF_RX,
            "wal_dump_mgmt_with_vendor_ie::frame_type[%d] is neither probe req nor action", frame_type);
        return;
    }

    app_ie = &hmac_vap->app_ie[OAL_APP_VENDOR_IE];
    if ((hmac_vap->mgmt_report != OAL_TRUE) ||
        (app_ie->ie == OSAL_NULL) || (app_ie->ie_len < (MAC_IE_HDR_LEN + MAC_OUI_LEN))) {
        return;
    }

    ie_req = (mac_ieee80211_vendor_ie_stru *)app_ie->ie;
    /* 16、8：表示左移位数，2：表示下标 */
    ie_oui = (ie_req->oui[0] << 16) | (ie_req->oui[1] << 8) | ie_req->oui[2];
    ie_len = frame_len - MAC_80211_FRAME_LEN;

#ifdef _PRE_WLAN_FEATURE_P2P
    /* P2P Action帧需要偏移8字节(Fixed Para占8字节)到IE的起始位置,即:Tagged Para起始处 */
    /* 帧结构: MAC HEADER(24 Bytes) + Fixed Para(8 Bytes) + Tagged Para(IEs) */
    if (frame_type == WLAN_FC0_SUBTYPE_ACTION &&
        hmac_ie_check_p2p_action_etc(payload + MAC_ACTION_CATEGORY_AND_CODE_LEN) == OAL_TRUE) {
        payload += P2P_PUB_ACT_TAG_PARAM_OFF;
        ie_len -= P2P_PUB_ACT_TAG_PARAM_OFF;
    }
#endif
    ie_tmp = hmac_find_vendor_ie_etc(ie_oui, ie_req->oui_type, payload, ie_len);
    if (ie_tmp == OSAL_NULL) {
        oam_info_log0(0, OAM_SF_RX, "wal_dump_mgmt_with_vendor_ie::cannot find vendor ie");
        return;
    }

    vendor_ie = (mac_ieee80211_vendor_ie_stru *)ie_tmp;
    wal_dump_vendor_ie_info(hmac_vap, frame_type, frame_len, ie_tmp, vendor_ie->len + MAC_IE_HDR_LEN);
}
#endif

/*****************************************************************************
 函 数 名  : wal_send_mgmt_to_host_etc
 功能描述  : 驱动上报内核接收到管理帧
*****************************************************************************/
osal_s32 wal_send_mgmt_to_host_etc(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    oal_net_device_stru *net_dev = OAL_PTR_NULL;
    osal_s32 l_freq;
    osal_u8 rssi;
    osal_u16 len;
    osal_s32 ret = OAL_SUCC;
    hmac_rx_mgmt_event_stru *mgmt_frame = OAL_PTR_NULL;
    oal_ieee80211_mgmt *mgmt = OAL_PTR_NULL;

    mgmt_frame  = (hmac_rx_mgmt_event_stru *)msg->data;

    /* 获取net_device */
    net_dev = oal_dev_get_by_name((const osal_s8 *)mgmt_frame->ac_name);
    if (net_dev == OAL_PTR_NULL) {
        /* 管理帧丢弃 */
        hmac_dft_print_drop_frame_info(THIS_FILE_ID, __LINE__, 1, (oal_netbuf_stru *)mgmt_frame->buf);
        oam_error_log1(0, OAM_SF_ANY, "vap_id[%d] {wal_send_mgmt_to_host_etc::get net device ptr is null!}",
                       hmac_vap->vap_id);
        oal_free(mgmt_frame->buf);
        return OAL_ERR_CODE_PTR_NULL;
    }
    oal_dev_put(net_dev);

    len  = mgmt_frame->len;
    l_freq  = mgmt_frame->l_freq;
    rssi = mgmt_frame->rssi;

    mgmt = (oal_ieee80211_mgmt *)mgmt_frame->buf;

#ifdef _PRE_WLAN_DFT_STAT
    /* vendor IE报文上报. 打印携带指定vendor IE的 probe req/action 帧信息 */
    wal_dump_mgmt_with_vendor_ie(hmac_vap, mgmt_frame->buf + WLAN_MGMT_FRAME_HEADER_LEN, mgmt_frame->len, mgmt);
#endif

    /**********************************************************************************************
        NOTICE:  AUTH ASSOC DEAUTH DEASSOC 这几个帧上报给host的时候 可能给ioctl调用死锁
                 需要添加这些帧上报的时候 请使用workqueue来上报
    ***********************************************************************************************/

    /* 调用内核接口，上报接收到管理帧 */
    if (oal_cfg80211_rx_mgmt_etc(net_dev, l_freq, rssi, mgmt_frame->buf, len, GFP_ATOMIC) != OAL_SUCC) {
        oam_warning_log3(0, OAM_SF_ANY, "vap_id[%d] {wal_send_mgmt_to_host_etc::fc[0x%04x], if_type[%d]!}",
                         hmac_vap->vap_id, mgmt->frame_control, net_dev->ieee80211_ptr->iftype);
        oam_warning_log4(0, OAM_SF_ANY,
            "vap_id[%d] {wal_send_mgmt_to_host_etc::oal_cfg80211_rx_mgmt_etc fail[%d]!len[%d], freq[%d]}",
            hmac_vap->vap_id, OAL_FAIL, len, l_freq);
        ret = OAL_FAIL;
    }
    oal_free(mgmt_frame->buf);
#ifdef _PRE_WLAN_DFT_STAT
    if (ret == OAL_SUCC) {
        /* wal发送管理帧给内核成功 */
        hmac_pkts_rx_stat(hmac_vap->vap_id, 1, RX_M_SEND_LAN_SUCC_PKTS);
    } else {
        /* wal发送管理帧给内核失败 */
        hmac_pkts_rx_stat(hmac_vap->vap_id, 1, RX_M_SEND_LAN_FAIL_PKTS);
    }
#endif

    return ret;
}

#ifdef _PRE_WLAN_FEATURE_P2P
/*****************************************************************************
 函 数 名  : wal_p2p_listen_timeout_etc
 功能描述  : HMAC上报监听超时
*****************************************************************************/
osal_s32 wal_p2p_listen_timeout_etc(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    oal_wireless_dev_stru *wdev = OSAL_NULL;
    osal_u64 ull_cookie;
    oal_ieee80211_channel_stru st_listen_channel;
    hmac_p2p_listen_expired_stru *p2p_listen_expired = OSAL_NULL;
    hmac_device_stru *hmac_device;

    /* 获取hmac_device_stru */
    hmac_device = hmac_res_get_mac_dev_etc(0);
    if (hmac_device == OAL_PTR_NULL) {
        oam_error_log1(0, OAM_SF_PROXYSTA,
            "vap_id[%d] {wal_p2p_listen_timeout_etc::get mac_device ptr is null.}", hmac_vap->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }
    p2p_listen_expired  = (hmac_p2p_listen_expired_stru *)(msg->data);

    wdev                = p2p_listen_expired->wdev;
    ull_cookie              = hmac_device->st_p2p_info.ull_last_roc_id;
    st_listen_channel       = p2p_listen_expired->st_listen_channel;
    oal_cfg80211_remain_on_channel_expired_etc(wdev, ull_cookie, &st_listen_channel, GFP_ATOMIC);
    oam_warning_log1(0, OAM_SF_P2P, "{wal_p2p_listen_timeout_etc::END!cookie [%x]}", ull_cookie);
    return OAL_SUCC;
}
#endif

/* HMAC上报channel switch处理 */
osal_s32 wal_report_channel_switch(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    oal_net_device_stru *net_device = NULL;
    oal_channel_switch_stru *channel_switch = NULL;
    oal_cfg80211_chan_def chandef;
    hmac_device_stru *hmac_device = NULL;
    oal_wiphy_stru *wiphy = NULL;

    /* 获取net_device */
    net_device = hmac_vap_get_net_device_etc(hmac_vap->vap_id);
    if (net_device == NULL) {
        oam_error_log1(0, OAM_SF_CHAN, "{wal_report_channel_switch::get net device ptr is null! vap_id %d}",
            hmac_vap->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取hmac_device */
    hmac_device = hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    if (hmac_device == NULL) {
        oam_error_log1(0, OAM_SF_CHAN, "{wal_report_channel_switch::get hmac_device is null! device_id %d}",
            hmac_vap->device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取wiphy */
    wiphy = hmac_device->wiphy;
    if (wiphy == NULL) {
        oam_error_log0(0, OAM_SF_CHAN, "{wal_report_channel_switch:: pst_wiphy is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    channel_switch = (oal_channel_switch_stru *)(msg->data);

    (void)memset_s(&chandef, sizeof(chandef), 0, sizeof(chandef));
    chandef.chan = oal_ieee80211_get_channel(wiphy, channel_switch->center_freq);
    if (chandef.chan == NULL) {
        oam_error_log1(0, OAM_SF_CHAN, "{wal_report_channel_switch::can't get chan pointer from wiphy(freq=%d)!}",
            channel_switch->center_freq);
        return OAL_FAIL;
    }
    chandef.chan->center_freq = channel_switch->center_freq;
    chandef.width = channel_switch->width;
    chandef.center_freq1 = channel_switch->center_freq1;
    chandef.center_freq2 = channel_switch->center_freq2;
    oal_cfg80211_ch_switch_notify(net_device, &chandef);

    oam_warning_log4(hmac_vap->vap_id, OAM_SF_CHAN,
                     "{wal_report_channel_switch::center_freq=%d, width=%d, center_freq1=%d, center_freq2=%d}",
                     chandef.chan->center_freq, chandef.width, chandef.center_freq1, chandef.center_freq2);

    return OAL_SUCC;
}

/* 从频点和带宽信息计算得到center_freq1 */
osal_s32 wal_get_center_freq1_from_freq_and_band(osal_u16 freq, osal_u8 band,
    wlan_channel_bandwidth_enum_uint8 bandwidth, osal_u32 *center_freq1)
{
    /******************************************************
    channel number to center channel idx map
    BAND WIDTH                            CENTER CHAN INDEX:

    WLAN_BAND_WIDTH_20M                   chan_num
    WLAN_BAND_WIDTH_40PLUS                chan_num + 2
    WLAN_BAND_WIDTH_40MINUS               chan_num - 2
    WLAN_BAND_WIDTH_80PLUSPLUS            chan_num + 6 暂不支持
    WLAN_BAND_WIDTH_80PLUSMINUS           chan_num - 2 暂不支持
    WLAN_BAND_WIDTH_80MINUSPLUS           chan_num + 2 暂不支持
    WLAN_BAND_WIDTH_80MINUSMINUS          chan_num - 6 暂不支持
    WLAN_BAND_WIDTH_5M                    暂不支持
    WLAN_BAND_WIDTH_10M                   暂不支持
    WLAN_BAND_WIDTH_40M                   暂不支持
    WLAN_BAND_WIDTH_80M                   暂不支持
    ********************************************************/

    /* 中心信道的偏移量, 现在只支持3种场景, 初始化3个值 */
    osal_s8 center_chan_offset[WLAN_BAND_WIDTH_BUTT] = {
        0, 2, -2 /* 偏移2个信道 */
    };

    /* 目前只支持2G, 不支持80M频宽, 2412 2482 为2.4G频点; 5G 及其他场景 待重新适配 */
    if ((band != OAL_IEEE80211_BAND_2GHZ) || (freq < 2412) || (freq > 2482) ||
        (bandwidth >= WLAN_BAND_WIDTH_80PLUSPLUS)) {
        oam_warning_log3(0, OAM_SF_CSA, "hmac_get_center_freq1_from_freq_and_band fail:band %d, freq %d, bandwidth %d.",
            band, freq, bandwidth);
        return OAL_FAIL;
    }

    if (bandwidth == WLAN_BAND_WIDTH_40PLUS) {
        if (freq > 2452) { /* 40M 向上扩频 只支持 1-9信道, 2452为9信道中心频点 */
            return OAL_FAIL;
        }
    } else if (bandwidth == WLAN_BAND_WIDTH_40MINUS) {
        if (freq < 2432) { /* 40M 向下扩频 只支持 5-13信道, 2432为5信道中心频点 */
            return OAL_FAIL;
        }
    }

    /* 偏移5M */
    *center_freq1 = freq + center_chan_offset[bandwidth] * 5;

    return OAL_SUCC;
}

osal_u8 wlan_bandwidth_to_ieee_chan_width(osal_u8 bandwidth, osal_u8 ht_support_flag)
{
    if (bandwidth == WLAN_BAND_WIDTH_20M) {
        if (ht_support_flag == 0) {
            return NL80211_CHAN_WIDTH_20_NOHT;
        }
        return NL80211_CHAN_WIDTH_20;
    }

    if ((bandwidth == WLAN_BAND_WIDTH_40PLUS) || (bandwidth == WLAN_BAND_WIDTH_40MINUS)) {
        return NL80211_CHAN_WIDTH_40;
    }

    if ((bandwidth >= WLAN_BAND_WIDTH_80PLUSPLUS) && (bandwidth <= WLAN_BAND_WIDTH_80MINUSMINUS)) {
        return NL80211_CHAN_WIDTH_80;
    }

    return WLAN_BW_CAP_BUTT;
}

osal_s32 wal_report_channel_switch_csa(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    osal_s32 ret;
    frw_msg msg_report;
    oal_ieee80211_band_enum_uint8 band;
    oal_channel_switch_stru chan_switch_report;
    hmac_csa_chan_switch_done_stru *chan_switch_done = (hmac_csa_chan_switch_done_stru *)(msg->data);

    if (chan_switch_done->band == WLAN_BAND_2G) {
        band = OAL_IEEE80211_BAND_2GHZ;
    } else if (chan_switch_done->band == WLAN_BAND_5G) {
        band = OAL_IEEE80211_BAND_5GHZ;
    } else {
        band = OAL_IEEE80211_NUM_BANDS;
    }

    (osal_void)memset_s(&chan_switch_report, sizeof(chan_switch_report), 0, sizeof(chan_switch_report));
    chan_switch_report.center_freq = oal_ieee80211_channel_to_frequency(chan_switch_done->chan_number, band);
    if (chan_switch_report.center_freq == 0) {
        oam_warning_log2(0, OAM_SF_CSA, "wal_report_channel_switch_csa fail:freq=0,chan_number%d,band%d!",
            chan_switch_done->chan_number, band);
        return OAL_FAIL;
    }

    chan_switch_report.width = (enum nl80211_chan_width)wlan_bandwidth_to_ieee_chan_width(chan_switch_done->bandwidth,
        mac_mib_get_high_throughput_option_implemented(hmac_vap));

    ret = wal_get_center_freq1_from_freq_and_band(chan_switch_report.center_freq, band,
        chan_switch_done->bandwidth, &chan_switch_report.center_freq1);
    if (ret != OAL_SUCC) {
        oam_warning_log4(0, OAM_SF_CSA, "hmac_chan_switch_comp_notify_to_wal fail:freq %d,chan_number %d,band %d,bw %d",
            chan_switch_report.center_freq, chan_switch_done->chan_number, band, chan_switch_done->bandwidth);
        return OAL_FAIL;
    }

    (void)memset_s(&msg_report, sizeof(msg_report), 0, sizeof(msg_report));
    msg_report.data = (osal_u8 *)&chan_switch_report;
    msg_report.data_len = OAL_SIZEOF(chan_switch_report);

    return wal_report_channel_switch(hmac_vap, &msg_report);
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

