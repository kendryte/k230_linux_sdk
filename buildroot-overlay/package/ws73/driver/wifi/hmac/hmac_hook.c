/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: hmac侧Hook模块
 * Create: 2022-07-01
 */

/*****************************************************************************
  1 头文件包吿
*****************************************************************************/
#include "hmac_hook.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

struct osal_list_head g_hmac_hooks[HMAC_FRAME_HOOK_END];

static WIFI_HMAC_TCM_TEXT WIFI_TCM_TEXT osal_u32 hmac_call_hooks(oal_netbuf_stru **netbuf, hmac_vap_stru *hmac_vap,
    osal_u8 hooknum)
{
    struct osal_list_head *entry = OSAL_NULL;
    hmac_netbuf_hook_stru *hook_node = OSAL_NULL;
    osal_u32 ret = OAL_CONTINUE;

    osal_list_for_each(entry, &g_hmac_hooks[hooknum])
    {
        hook_node = osal_list_entry(entry, hmac_netbuf_hook_stru, list_entry);
        if (hook_node->hook_func != OSAL_NULL) {
            ret = hook_node->hook_func(netbuf, hmac_vap);
            if (ret != OAL_CONTINUE) {
                return ret;
            }
        }
    }

    return ret;
}

/* api */
osal_u32 hmac_register_netbuf_hook(hmac_netbuf_hook_stru *netbuf_hook_new)
{
    struct osal_list_head *entry = OSAL_NULL;
    struct osal_list_head *entry_next = OSAL_NULL;
    hmac_netbuf_hook_stru *hook_node = OSAL_NULL;
    osal_u16 hooknum = netbuf_hook_new->hooknum;

    if ((hooknum < HMAC_FRAME_HOOK_START) || (hooknum >= HMAC_FRAME_HOOK_END)) {
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    if (netbuf_hook_new->priority > HMAC_HOOK_PRI_NUM) {
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    if (osal_list_empty(&g_hmac_hooks[hooknum]) != 0) {
        osal_list_add_tail(&netbuf_hook_new->list_entry, &g_hmac_hooks[hooknum]);
        return OAL_SUCC;
    }

    osal_list_for_each_safe(entry, entry_next, &g_hmac_hooks[hooknum])
    {
        hook_node = osal_list_entry(entry, hmac_netbuf_hook_stru, list_entry);
        /* 已经添加的函数不允许再次添加 */
        if (netbuf_hook_new->hook_func == hook_node->hook_func) {
            return OAL_SUCC;
        }

        /* 在队列中找到了一个节点的优先级大于新节点，则插入到该节点前面即可 */
        if (netbuf_hook_new->priority < hook_node->priority) {
            osal_list_add_tail(&netbuf_hook_new->list_entry, &hook_node->list_entry);
            return OAL_SUCC;
        }
    }
    
    /* 所有节点的优先级都大于新插入的，则将改节点插入队尾 */
    osal_list_add_tail(&netbuf_hook_new->list_entry, &g_hmac_hooks[hooknum]);

    return OAL_SUCC;
}

osal_void hmac_unregister_netbuf_hook(hmac_netbuf_hook_stru *netbuf_hook)
{
    struct osal_list_head *entry = OSAL_NULL;
    struct osal_list_head *entry_next = OSAL_NULL;
    hmac_netbuf_hook_stru *hook_node = OSAL_NULL;
    osal_u16 hooknum = netbuf_hook->hooknum;

    if ((hooknum < HMAC_FRAME_HOOK_START) || (hooknum >= HMAC_FRAME_HOOK_END)) {
        return;
    }

    if (netbuf_hook->priority > HMAC_HOOK_PRI_NUM) {
        return;
    }

    osal_list_for_each_safe(entry, entry_next, &g_hmac_hooks[hooknum])
    {
        hook_node = osal_list_entry(entry, hmac_netbuf_hook_stru, list_entry);
        if (hook_node == netbuf_hook) {
            osal_list_del(&netbuf_hook->list_entry);
            break;
        }
    }
}

WIFI_HMAC_TCM_TEXT WIFI_TCM_TEXT osal_u32 hmac_call_netbuf_hooks(oal_netbuf_stru **netbuf,
    hmac_vap_stru *hmac_vap, osal_u8 hooknum)
{
    if ((hooknum < HMAC_FRAME_HOOK_START) || (hooknum >= HMAC_FRAME_HOOK_END)) {
        return OAL_CONTINUE;
    }

    return hmac_call_hooks(netbuf, hmac_vap, hooknum);
}

osal_void hmac_hook_init(osal_void)
{
    osal_u16 hooknum;

    for (hooknum = HMAC_FRAME_HOOK_START; hooknum < HMAC_FRAME_HOOK_END; hooknum++) {
        OSAL_INIT_LIST_HEAD(&g_hmac_hooks[hooknum]);
    }
}

osal_void hmac_hook_exit(osal_void)
{
    hmac_hook_init();
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif