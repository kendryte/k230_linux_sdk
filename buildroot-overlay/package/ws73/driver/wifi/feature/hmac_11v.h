/*
 * Copyright (c) CompanyNameMagicTag. 2018-2024. All rights reserved.
 * 文 件 名   : hmac_11v.h
 * 生成日期   : 2018年4月7日
 * 功能描述   : 11v 宏、枚举及结构定义
 */

#ifndef __HMAC_11V_H__
#define __HMAC_11V_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "mac_vap_ext.h"
#include "hmac_vap.h"
#include "hmac_roam_alg.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_11V_H

/*****************************************************************************
  2 宏定义
*****************************************************************************/
/* 目前管理帧申请内存800字节 帧自带最大长度19(frame boady)+50(url)+ N*(15(neighbor)+3(sub)+12(sub))
  不存在超出 修改数量限制时须注意 */
#define HMAC_MAX_BSS_NEIGHBOR_LIST  20          /* BSS Transition 帧中允许发送的最大候选AP列表集数量 */

// 11v等待帧返回的超时时间
#define HMAC_11V_WAIT_STATUS_TIMEOUT        5000         // 5000ms
#define HMAC_11V_MAX_URL_LENGTH             50          /* 携带URL字符的最大长度限制为50 */
#define HMAC_11V_QUERY_FRAME_BODY_FIX       4           /* query帧帧体固定头长度 */
#define HMAC_11V_REQUEST_FRAME_BODY_FIX     7           /* query帧帧体固定头长度 */
#define HMAC_11V_RESPONSE_FRAME_BODY_FIX    5           /* response帧帧体固定头长度 */
#define HMAC_11V_PERFERMANCE_ELEMENT_LEN    1           /* perfermance ie length */
#define HMAC_11V_TERMINATION_ELEMENT_LEN    10          /* termination ie length */
#define HMAC_11V_TOKEN_MAX_VALUE            255         /* 帧发送信令的最大值 */
#define HMAC_11V_SUBELEMENT_ID_RESV         0           /* SUBELEMENT预留 ID */
#define MAC_11V_ROAM_SCAN_ONE_CHANNEL_LIMIT 2
#define MAC_11V_ROAM_SCAN_FULL_CHANNEL_LIMIT (MAC_11V_ROAM_SCAN_ONE_CHANNEL_LIMIT + 1)
#define HMAC_11V_TERMINATION_TSF_LENGTH     8           /* termination_tsf时间域字节长度 */
#define WAL_WIFI_FEATURE_SUPPORT_11V        1

/*****************************************************************************
  3 枚举定义
*****************************************************************************/
/* IEEE Std 802.11-2012 - Table 8-253 */
enum bss_trans_mgmt_status_code {
    WNM_BSS_TM_ACCEPT = 0,
    WNM_BSS_TM_REJECT_UNSPECIFIED = 1,
    WNM_BSS_TM_REJECT_INSUFFICIENT_BEACON = 2,
    WNM_BSS_TM_REJECT_INSUFFICIENT_CAPABITY = 3,
    WNM_BSS_TM_REJECT_UNDESIRED = 4,
    WNM_BSS_TM_REJECT_DELAY_REQUEST = 5,
    WNM_BSS_TM_REJECT_STA_CANDIDATE_LIST_PROVIDED = 6,
    WNM_BSS_TM_REJECT_NO_SUITABLE_CANDIDATES = 7,
    WNM_BSS_TM_REJECT_LEAVING_ESS = 8
};

/*****************************************************************************
  Neighbor Report Element子信息元素(Subelement)的 ID
  820.11-2012协议583页，Table 8-115—SubElement IDs
*****************************************************************************/
typedef enum {
    HMAC_NEIGH_SUB_ID_RESERVED              = 0,
    HMAC_NEIGH_SUB_ID_TFS_INFO,
    HMAC_NEIGH_SUB_ID_COND_COUNTRY,
    HMAC_NEIGH_SUB_ID_BSS_CANDIDATE_PERF,
    HMAC_NEIGH_SUB_ID_TERM_DURATION,
    HMAC_NEIGH_SUB_ID_VENDOR_SPECIFIC = 221,

    HMAC_NEIGH_SUB_ID_BUTT
} hmac_neighbor_sub_eid_enum;
typedef osal_u8 hmac_neighbor_sub_eid_enum_uint8;


/*****************************************************************************
  4 STRUCT定义
*****************************************************************************/
/* 此文件中结构体与协议一致，要求1字节对齐 */
#pragma pack(push, 1)
struct hmac_bss_term_duration {
    osal_u8       sub_ie_id;                                        /* subelement ID，将ID置成0 表示不存在该元素 */
    osal_u8       termination_tsf[HMAC_11V_TERMINATION_TSF_LENGTH];       /* BSS终止时间: TSF */
    osal_u16      duration_min;                                            /* BSS消失时间 time: 分钟 */
    osal_u8       resv;                                                    /* 四字节对齐用 */
};
typedef struct hmac_bss_term_duration hmac_bss_term_duration_stru;

/* 候选BSS列表集的Neighbor Report IE结构体 由于只需要用到subelement 3 4 故定义两个subelement */
struct hmac_neighbor_bss_info {
    osal_u8       auc_mac_addr[WLAN_MAC_ADDR_LEN];                      /* BSSID MAC地址 */
    osal_u8       opt_class;                                         /* Operation Class */
    osal_u8       chl_num;                                           /* Channel number */
    osal_u8       phy_type;                                          /* PHY type */
    osal_u8       candidate_perf;                                    /* perference data BSSID偏好值 */
    osal_u16      resv;                                              /* 四字节对齐 */
    oal_bssid_infomation_stru   bssid_info;                       /* BSSID information */
    hmac_bss_term_duration_stru term_duration;                   /* 子元素Termination duration */
};
typedef struct hmac_neighbor_bss_info hmac_neighbor_bss_info_stru;

/* bss transition query帧体信息结构体 */
struct hmac_bsst_query_info {
    osal_u8 reason;
    osal_u8 bss_list_num; /* bss list的数量 最大不允许超过50个 */
    osal_u16 resv;        /* 四字节对齐 */
    hmac_neighbor_bss_info_stru *neighbor_bss_list;
};
typedef struct hmac_bsst_query_info hmac_bsst_query_info_stru;

struct hmac_bsst_req_mode {
    osal_u8   candidate_list_include : 1,                  /* 是否包含邻居AP列表 */
                abridged : 1,                                /* 1:表示没有包含在邻居列表的AP优先级设置为0 */
                bss_disassoc_imminent : 1,                   /* 是否即将断开STA 置0 帧的时间域全为FF FF */
                termination_include : 1,               /* BSS终结时间  置0则帧体中不包含BSS Termination Duration字段 */
                ess_disassoc_imminent : 1,                   /* EES终止时间 */
                rev : 3;
};
typedef struct hmac_bsst_req_mode hmac_bsst_req_mode_stru;

/* bss transition request帧体信息结构体 */
struct hmac_bsst_req_info {
    osal_u8               validity_interval;                              /* 邻居列表有效时间 TBTTs */
    osal_u16              disassoc_time;                                  /* AP取关联STA时间 TBTTs */
    osal_u8               *session_url;                                  /* 要求传入字符串 最大限制为100个字符 */
    hmac_bsst_req_mode_stru request_mode;                                   /* request mode */
    osal_u8               bss_list_num;
    osal_u8               resv;                                           /* 4字节对齐 */
    hmac_neighbor_bss_info_stru *neighbor_bss_list;                        /* bss list的数量 最大不允许超过50个 */
    hmac_bss_term_duration_stru term_duration;                              /* 子元素Termination duration */
};
typedef struct hmac_bsst_req_info hmac_bsst_req_info_stru;

/* bss transition response帧体信息结构体 */
struct hmac_bsst_rsp_info {
    osal_u8       status_code;                         /* 状态码 接收或者拒绝 */
    osal_u8       termination_delay;                   /* 要求AP延后终止时间:分钟 */
    osal_u8       target_bss_addr[WLAN_MAC_ADDR_LEN]; /* 接收切换的目标BSSID */
    osal_u8       chl_num;                             /* Channel number */
    osal_u8       bss_list_num;                        /* bss list的数量最大限制为50个 */
    osal_u16      resv;                                /* 四字节对齐 */
    hmac_neighbor_bss_info_stru *neighbor_bss_list;
};
typedef struct hmac_bsst_rsp_info hmac_bsst_rsp_info_stru;

/* 通过11v_tx_query命令下发的消息内容 */
struct hmac_trigger_11v_msg {
    osal_u8       mac_addr_list_0[WLAN_MAC_ADDR_LEN];        /* 第1个邻居的BSSID MAC地址 */
    osal_u8       mac_addr_list_1[WLAN_MAC_ADDR_LEN];        /* 第2个邻居的BSSID MAC地址 */
    osal_u8       chl_num_list_0;                            /* 第1个邻居的信道 */
    osal_u8       chl_num_list_1;                            /* 第2个邻居的信道 */
};
typedef struct hmac_trigger_11v_msg hmac_trigger_11v_msg_stru;
#pragma pack(pop)

typedef struct {
    hmac_bsst_rsp_info_stru bsst_rsp_info;
    osal_u32                roam_11v_scan_fail;
} hmac_11v_vap_roam_info_stru;

typedef struct {
    hmac_11v_vap_roam_info_stru *roam_info_11v;
    osal_u8                 enable_11v;
} hmac_11v_vap_info_stru;

typedef osal_u32(*mac_user_callback_func_11v)(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    hmac_bsst_rsp_info_stru *bsst_rsp_info);
/* 11v 控制信息结构体 */
typedef struct {
    osal_u8                            user_bsst_token;                     /* 用户发送bss transition 帧的信令 */
    osal_u8                            user_status;                         /* 用户11V状态 */
    osal_u8                            uc_11v_roam_scan_times;                 /* 单信道11v漫游扫描次数  */
    osal_u8                            resv;                                /* 4字节对齐用 */
    frw_timeout_stru                     status_wait_timer;                   /* 等待用户回应帧的计时器 */
    mac_user_callback_func_11v           mac_11v_callback_fn;                    /* 回调函数指针 */
} hmac_user_11v_ctrl_stru;
/*****************************************************************************
  5 全局变量声明
*****************************************************************************/


/*****************************************************************************
  6 消息头定义
*****************************************************************************/


/*****************************************************************************
  7 消息定义
*****************************************************************************/


/*****************************************************************************
  8 UNION定义
*****************************************************************************/


/*****************************************************************************
  9 OTHERS定义
*****************************************************************************/


/*****************************************************************************
  10 函数声明
*****************************************************************************/
static osal_u32 hmac_11v_init_weakref(osal_void) __attribute__ ((weakref("hmac_11v_init"), used));
static osal_void hmac_11v_deinit_weakref(osal_void) __attribute__ ((weakref("hmac_11v_deinit"), used));

typedef osal_void (*hmac_11v_set_bss_transition_cb)(mac_user_cap_info_stru *cap_info, const osal_u8 *cap);
typedef osal_void (*hmac_11v_set_ext_cap_bss_transition_cb)(hmac_vap_stru *hmac_vap, mac_ext_cap_ie_stru *ext_cap);
typedef osal_void (*hmac_11v_set_scan_params_cb)(mac_scan_req_stru *scan_params, hmac_roam_info_stru *roam_info);
typedef osal_bool (*hmac_11v_check_bss_dscr_cb)(hmac_roam_info_stru *roam_info, mac_bss_dscr_stru *bss_dscr);
typedef osal_bool (*hmac_11v_check_scan_result_cb)(hmac_roam_info_stru *roam_info,
    oal_bool_enum_uint8 flag_in_scan_rslts);
typedef osal_void (*hmac_11v_trigger_roam_check_cb)(const hmac_roam_info_stru *roam_info, hmac_vap_stru *hmac_vap);
typedef osal_void (*hmac_get_11v_cap_cb)(hmac_vap_stru *hmac_vap, osal_s32 *pl_value);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of hmac_11v.h */
