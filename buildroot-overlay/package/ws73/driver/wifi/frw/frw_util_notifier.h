/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: wlan util notifier header
 * Create: 2022-05-20
 */
// 支持优先级，返回值，支持携带参数
#ifndef __FRW_UTIL_NOTIFIER_H__
#define __FRW_UTIL_NOTIFIER_H__
#include "osal_adapt.h"
#include "osal_types.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/* notifier 事件类型 */
typedef enum {
    WLAN_UTIL_NOTIFIER_EVENT_SYS_INIT,
    WLAN_UTIL_NOTIFIER_EVENT_ADD_VAP,
    WLAN_UTIL_NOTIFIER_EVENT_DEL_VAP,
    WLAN_UTIL_NOTIFIER_EVENT_DEL_VAP_FEATURE,
    WLAN_UTIL_NOTIFIER_EVENT_START_VAP,
    WLAN_UTIL_NOTIFIER_EVENT_START_VAP_EXIT,
    WLAN_UTIL_NOTIFIER_EVENT_DOWN_VAP,
    WLAN_UTIL_NOTIFIER_EVENT_ADD_USER,
    WLAN_UTIL_NOTIFIER_EVENT_DEL_USER,
    WLAN_UTIL_NOTIFIER_EVENT_DEL_USER_FEATURE,
    WLAN_UTIL_NOTIFIER_EVENT_DEL_USER_RESET,
    WLAN_UTIL_NOTIFIER_EVENT_TBTT_HANDLE,
    WLAN_UTIL_NOTIFIER_EVENT_TBTT_AP,
    WLAN_UTIL_NOTIFIER_EVENT_VAP_STATE_CHANGE,
    WLAN_UTIL_NOTIFIER_EVENT_SCAN_BEGIN,
    WLAN_UTIL_NOTIFIER_EVENT_SCAN_END,
    WLAN_UTIL_NOTIFIER_EVENT_ROAM_ACTIVE,
    WLAN_UTIL_NOTIFIER_EVENT_ROAM_DEACTIVE,
    WLAN_UTIL_NOTIFIER_EVENT_INETADDR_NOTIFIER_UP,
    WLAN_UTIL_NOTIFIER_EVENT_SUSPEND_CHANGE,
    WLAN_UTIL_NOTIFIER_EVENT_ADD_GTK,
    WLAN_UTIL_NOTIFIER_EVENT_DEL_GTK,
    WLAN_UTIL_NOTIFIER_EVENT_LINKLOSS_INIT,
    WLAN_UTIL_NOTIFIER_EVENT_RX_ASSOC_RSP,
    WLAN_UTIL_NOTIFIER_EVENT_11V_NEW_BSS_LIST,
    WLAN_UTIL_NOTIFIER_EVENT_11V_FREE_BSS_LIST,
    WLAN_UTIL_NOTIFIER_EVENT_11V_VAP_ROAM_INFO_INIT,
    WLAN_UTIL_NOTIFIER_EVENT_11V_VAP_ROAM_INFO_DEINIT,

    WLAN_UTIL_NOTIFIER_EVENT_MAX,
} wlan_util_notifier_event_en;

/* notifier事件优先级，值越大，优先级越高 */
typedef enum {
    WLAN_UTIL_NOTIFIER_PRIORITY_DEFAULT,

    WLAN_UTIL_NOTIFIER_PRIORITY_MAX,
} wlan_util_notifier_priority_en;


typedef osal_bool (*wlan_util_notifier_hook_func)(osal_void *notify_data);

typedef struct {
    struct osal_list_head list_entry;
    osal_u32 priority;
    wlan_util_notifier_hook_func hook_func;
} notifier_node_stru;

osal_void frw_util_notifier_init(osal_void);
osal_void frw_util_notifier_exit(osal_void);
osal_u32 frw_util_notifier_notify(wlan_util_notifier_event_en event, osal_void *notify_data);

osal_bool frw_util_notifier_unregister(wlan_util_notifier_event_en event, wlan_util_notifier_hook_func hook_func);
osal_bool frw_util_notifier_unregister_with_priority(wlan_util_notifier_event_en event,
    wlan_util_notifier_hook_func hook_func, osal_u32 priority);

osal_bool frw_util_notifier_register(wlan_util_notifier_event_en event, wlan_util_notifier_hook_func hook_func);
osal_bool frw_util_notifier_register_with_priority(wlan_util_notifier_event_en event,
    wlan_util_notifier_hook_func hook_func, osal_u32 priority);

typedef osal_u32 (*frw_wifi_module_func_init)(osal_void);
typedef osal_void (*frw_wifi_module_func_exit)(osal_void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
