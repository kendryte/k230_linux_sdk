/*
 * Copyright (c) CompanyNameMagicTag 2012-2021. All rights reserved.
 */

#include <linux/workqueue.h>
#include <linux/slab.h>
#include <linux/module.h>
#include "soc_osal.h"
#include "osal_inner.h"

static osal_mutex g_mutex_workqueue;
OSAL_LIST_HEAD(wq_list);
struct wq_node {
    osal_workqueue *osal_work;
    struct work_struct *work;
    struct osal_list_head node;
};

static osal_workqueue *osal_find_work(const struct work_struct *work)
{
    struct osal_list_head *this = NULL;

    if (work == NULL) {
        osal_log("parameter invalid!\n");
        return NULL;
    }

    if (osal_list_empty(&wq_list)) {
        osal_log("find work failed! wq_list is empty!\n");
        return NULL;
    }
    osal_list_for_each(this, &wq_list) {
        struct wq_node *ws = osal_list_entry(this, struct wq_node, node);
        if (ws->work == work) {
            return ws->osal_work;
        }
    }
    osal_log("find work failed!\n");
    return NULL;
}

static int osal_del_work(const struct work_struct *work)
{
    struct osal_list_head *this = NULL;
    struct osal_list_head *next = NULL;

    if (work == NULL) {
        osal_log("parameter invalid!\n");
        return OSAL_FAILURE;
    }

    if (osal_list_empty(&wq_list)) {
        osal_log("find work failed! wq_list is empty!\n");
        return OSAL_FAILURE;
    }
    osal_list_for_each_safe(this, next, &wq_list) {
        struct wq_node *ws = osal_list_entry(this, struct wq_node, node);
        if (ws->work == work) {
            osal_list_del(this);
            kfree(ws);
            return 0;
        }
    }
    osal_log("del work failed!\n");
    return OSAL_FAILURE;
}

static void osal_work_handler(struct work_struct *work)
{
    osal_workqueue *ow = osal_find_work(work);
    if ((ow != NULL) && (ow->handler != NULL)) {
        ow->handler(ow);
    }
}

int osal_workqueue_init(osal_workqueue *work, osal_workqueue_handler handler)
{
    struct work_struct *w = NULL;
    struct wq_node *w_node = NULL;

    if (work == NULL) {
        osal_log("parameter invalid!\n");
        return OSAL_FAILURE;
    }

    w = kmalloc(sizeof(struct work_struct), GFP_ATOMIC);
    if (w == NULL) {
        osal_log("osal_init_work kmalloc failed!\n");
        return OSAL_FAILURE;
    }

    w_node = kmalloc(sizeof(struct wq_node), GFP_ATOMIC);
    if (w_node == NULL) {
        osal_log("osal_init_work kmalloc failed!\n");
        kfree(w);
        return OSAL_FAILURE;
    }
    INIT_WORK(w, osal_work_handler);
    work->work = w;
    work->handler = handler;
    w_node->osal_work = work;
    w_node->work = w;

    if (osal_list_empty(&wq_list)) {
        int ret = osal_mutex_init(&g_mutex_workqueue);
        if (ret != 0) {
            osal_log("workqueue mutex init failed! ret = %#x.\n", ret);
            kfree(w);
            kfree(w_node);
            return OSAL_FAILURE;
        }
    }

    osal_mutex_lock(&g_mutex_workqueue);
    osal_list_add(&(w_node->node), &wq_list);
    osal_mutex_unlock(&g_mutex_workqueue);
    return OSAL_SUCCESS;
}
EXPORT_SYMBOL(osal_workqueue_init);

int osal_workqueue_schedule(osal_workqueue *work)
{
    if ((work != NULL) && (work->work != NULL)) {
        return (td_bool)schedule_work(work->work);
    } else {
        return (td_bool)false;
    }
}
EXPORT_SYMBOL(osal_workqueue_schedule);

void osal_workqueue_destroy(osal_workqueue *work)
{
    if ((work != NULL) && (work->work != NULL)) {
        osal_del_work(work->work);
        kfree((struct work_struct *)work->work);
        work->work = NULL;
        /* when the wq_list is empty delete the mutex resource */
        osal_mutex_lock(&g_mutex_workqueue);
        if (osal_list_empty(&wq_list)) {
            osal_mutex_unlock(&g_mutex_workqueue);
            osal_mutex_destroy(&g_mutex_workqueue);
            osal_printk("release the workqueue recource...\n");
            return;
        }
        osal_mutex_unlock(&g_mutex_workqueue);
    }
}
EXPORT_SYMBOL(osal_workqueue_destroy);

int osal_workqueue_flush(osal_workqueue *work)
{
    if ((work != NULL) && (work->work != NULL)) {
        return (int)flush_work(work->work);
    } else {
        return -1;
    }
}
EXPORT_SYMBOL(osal_workqueue_flush);
