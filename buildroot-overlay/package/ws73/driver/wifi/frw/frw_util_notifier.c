/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: wlan util notifier
 * Create: 2022-05-20
 */
#include "frw_util_notifier.h"
#include "oal_mem_hcm.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

struct osal_list_head g_event_list_head[WLAN_UTIL_NOTIFIER_EVENT_MAX];
osal_bool g_inited_flag = OSAL_FALSE;

osal_u32 frw_util_notifier_notify(wlan_util_notifier_event_en event, osal_void *notify_data)
{
    struct osal_list_head *entry = OSAL_NULL;
    notifier_node_stru *notifier_node = OSAL_NULL;

    if (event < 0 || event >= WLAN_UTIL_NOTIFIER_EVENT_MAX) {
        return OSAL_FAILURE;
    }

    osal_list_for_each(entry, &g_event_list_head[event])
    {
        notifier_node = osal_list_entry(entry, notifier_node_stru, list_entry);
        if (notifier_node->hook_func != OSAL_NULL) {
            if (notifier_node->hook_func(notify_data) != OSAL_TRUE) {
                wifi_printf("frw_util_notifier_notify,failed[%d %p]\r\n", event, notifier_node->hook_func);
            }
        }
    }

    return OSAL_SUCCESS;
}

osal_bool frw_util_notifier_register_with_priority(wlan_util_notifier_event_en event,
    wlan_util_notifier_hook_func hook_func, osal_u32 priority)
{
    notifier_node_stru *notifier_node_new = OSAL_NULL;
    struct osal_list_head *entry = OSAL_NULL;
    struct osal_list_head *entry_next = OSAL_NULL;
    notifier_node_stru *notifier_node = OSAL_NULL;

    if (event < 0 || event >= WLAN_UTIL_NOTIFIER_EVENT_MAX) {
        return OSAL_FALSE;
    }

    notifier_node_new = osal_kmalloc(OAL_SIZEOF(notifier_node_stru), OSAL_GFP_KERNEL);
    if (notifier_node_new == NULL) {
        wifi_printf("frw_util_notifier_register_with_priority failed\r\n");
        return OSAL_FALSE;
    }

    notifier_node_new->hook_func = hook_func;
    notifier_node_new->priority = priority;

    if (osal_list_empty(&g_event_list_head[event]) > 0) {
        osal_list_add_tail(&notifier_node_new->list_entry, &g_event_list_head[event]);
        return OSAL_TRUE;
    }

    /* 在队列中找到了一个节点的优先级小于新节点，则插入到该节点前面即可 */
    osal_list_for_each_safe(entry, entry_next, &g_event_list_head[event])
    {
        notifier_node = osal_list_entry(entry, notifier_node_stru, list_entry);
        if (notifier_node_new->priority > notifier_node->priority) {
            osal_list_add_tail(&notifier_node_new->list_entry, &notifier_node->list_entry);
            return OSAL_TRUE;
        }
    }

    /* 所有节点的优先级都大于新插入的，则将改节点插入队尾 */
    osal_list_add_tail(&notifier_node_new->list_entry, &g_event_list_head[event]);
    return OSAL_TRUE;
}

osal_bool frw_util_notifier_register(wlan_util_notifier_event_en event, wlan_util_notifier_hook_func hook_func)
{
    return frw_util_notifier_register_with_priority(event, hook_func, WLAN_UTIL_NOTIFIER_PRIORITY_DEFAULT);
}

osal_bool frw_util_notifier_unregister_with_priority(wlan_util_notifier_event_en event,
    wlan_util_notifier_hook_func hook_func, osal_u32 priority)
{
    struct osal_list_head *entry = OSAL_NULL;
    struct osal_list_head *entry_next = OSAL_NULL;
    notifier_node_stru *notifier_node = OSAL_NULL;

    if (event < 0 || event >= WLAN_UTIL_NOTIFIER_EVENT_MAX) {
        return OSAL_FALSE;
    }

    osal_list_for_each_safe(entry, entry_next, &g_event_list_head[event])
    {
        notifier_node = osal_list_entry(entry, notifier_node_stru, list_entry);
        if (notifier_node->hook_func == hook_func && notifier_node->priority == priority) {
            osal_list_del(entry);
            osal_kfree(notifier_node);
            return OSAL_TRUE;
        }
    }

    return OSAL_FALSE;
}

osal_bool frw_util_notifier_unregister(wlan_util_notifier_event_en event, wlan_util_notifier_hook_func hook_func)
{
    return frw_util_notifier_unregister_with_priority(event, hook_func, WLAN_UTIL_NOTIFIER_PRIORITY_DEFAULT);
}

osal_void frw_util_notifier_init(osal_void)
{
    osal_u32 i = 0;

    if (g_inited_flag == OSAL_TRUE) {
        return;
    }

    g_inited_flag = OSAL_TRUE;
    for (i = 0; i < sizeof(g_event_list_head) / sizeof(g_event_list_head[0]); i++) {
        OSAL_INIT_LIST_HEAD(&g_event_list_head[i]);
    }
}

osal_void frw_util_notifier_exit(osal_void)
{
    osal_u32 i = 0;
    struct osal_list_head *entry = OSAL_NULL;
    struct osal_list_head *entry_next = OSAL_NULL;
    notifier_node_stru *notifier_node = OSAL_NULL;

    if (g_inited_flag == OSAL_FALSE) {
        return;
    }

    g_inited_flag = OSAL_FALSE;
    for (i = 0; i < sizeof(g_event_list_head) / sizeof(g_event_list_head[0]); i++) {
        osal_list_for_each_safe(entry, entry_next, &g_event_list_head[i]) {
            notifier_node = osal_list_entry(entry, notifier_node_stru, list_entry);
            osal_list_del(entry);
            osal_kfree(notifier_node);
        }
    }
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
