/*
 * Copyright (c) CompanyNameMagicTag 2012-2021. All rights reserved.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/printk.h>
#include <linux/interrupt.h>
#include <linux/version.h>
#include <linux/slab.h>
#include "soc_osal.h"
#include "securec.h"
#include "osal_inner.h"
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 4, 0) && LINUX_VERSION_CODE <= KERNEL_VERSION(4, 14, 0))
#define OSAL_IRQ_COMPATIBLE
#endif

#ifdef ON_CHIP_LINUX_SDK
#ifdef OSAL_IRQ_COMPATIBLE
#include <linux/module.h>
#include <linux/of_irq.h>
#include <linux/platform_device.h>
#endif
#endif

#define MAX_IRQ_NAME_LEN 32

#ifdef OSAL_IRQ_COMPATIBLE
static DEFINE_SPINLOCK(interrupt_lock);
static OSAL_LIST_HEAD(g_irq_list);
#endif

typedef struct irq_info_ {
    unsigned int irq;
    unsigned int _irq;
    char irq_name[MAX_IRQ_NAME_LEN];
    struct osal_list_head node;
}irq_info;

#ifdef OSAL_IRQ_COMPATIBLE
#ifdef ON_CHIP_LINUX_SDK
static struct platform_device *g_osal_virt_dev = NULL;

int osal_get_irq_by_name(const char *name)
{
    struct resource *res = NULL;

    if (g_osal_virt_dev == NULL) {
        osal_log("osal_virt_dev is not registered!\n");
        return -1;
    }

    if (name == NULL) {
        osal_log("input module name is NULL!\n");
        return -1;
    }

    res = platform_get_resource_byname(g_osal_virt_dev, IORESOURCE_IRQ, name);
    if (res == NULL) {
        osal_log("module %s platform_get_resource_byname fail!\n", name);
        return -1;
    }

    return (int)res->start;
}
EXPORT_SYMBOL(osal_get_irq_by_name);

__attribute__((unused)) static int drv_osal_virt_probe(struct platform_device *dev)
{
    osal_log("virtual device initialization.\n");
    g_osal_virt_dev = dev;
    return 0;
}
#endif

static irq_info *osal_irq_find_node(unsigned int irq, const struct osal_list_head *list)
{
    irq_info *irq_node = NULL;

    if (list == NULL) {
        return NULL;
    }

    osal_list_for_each_entry(irq_node, list, node) {
        if (irq_node->irq == irq) {
            return irq_node;
        }
    }

    return NULL;
}

static irq_info *is_in_irq_list(unsigned int irq, const char *name)
{
    irq_info *irq_node = NULL;

    irq_node = osal_irq_find_node(irq, &g_irq_list);
    if (irq_node != NULL  &&
        (osal_strncmp(irq_node->irq_name, strlen(irq_node->irq_name), name, strlen(name)) == 0)) {
        return irq_node;
    }

    return NULL;
}

static unsigned int irq_add_request(unsigned int irq, const char *name)
{
    int ret;
    unsigned int _irq;
    irq_info *irq_node = NULL;

    irq_node = is_in_irq_list(irq, name);
    if (irq_node != NULL) {
        printk("irq %s, irq_num =%dr repeat!!!\n", name, irq);
        return -1;
    }

    irq_node = kmalloc(sizeof(irq_info), __GFP_ZERO);
    if (irq_node == NULL) {
        printk("kmalloc irq_node:%s failed.\n", name);
        return -1;
    }

    ret = osal_get_irq_by_name((char *)name);
    if (ret == -1) {
        printk("ext_drv_sys_get_irq_by_name:%s failed.\n", name);
        kfree(irq_node);
        return -1;
    }
    _irq = (unsigned int)ret;
    irq_node->_irq = _irq;
    irq_node->irq = irq;
    ret = snprintf_s(irq_node->irq_name, MAX_IRQ_NAME_LEN, MAX_IRQ_NAME_LEN - 1, "%s", name);
    if (ret < 0) {
        kfree(irq_node);
        return -1;
    }

    osal_list_add_tail(&irq_node->node, &g_irq_list);

    return _irq;
}
#endif

int osal_irq_request(unsigned int irq, osal_irq_handler handler, osal_irq_handler thread_fn,
    const char *name, void *dev)
{
    int ret;
#ifdef OSAL_IRQ_COMPATIBLE
    unsigned long lock_flags;
#endif
    unsigned long flags = (dev == NULL) ? 0 : IRQF_SHARED;

    if (name == NULL) {
        printk("name is NULL ! \n");
        return OSAL_FAILURE;
    }

#ifdef OSAL_IRQ_COMPATIBLE
    spin_lock_irqsave(&interrupt_lock, lock_flags);
    ret = irq_add_request(irq, name);
    if (ret == -1) {
        printk("irq_add_request irq name :%s, irq :%d failed.\n ", name, ret);
        spin_unlock_irqrestore(&interrupt_lock, lock_flags);
        osal_dump_stack();
        return OSAL_FAILURE;
    }
    irq = (unsigned int)ret;
    spin_unlock_irqrestore(&interrupt_lock, lock_flags);
#endif

    ret = request_threaded_irq(irq, (irq_handler_t)handler, (irq_handler_t)thread_fn, flags, name, dev);
    if (ret != OSAL_SUCCESS) {
        osal_log("request_threaded_irq failed! ret = %#x.\n", ret);
        return OSAL_FAILURE;
    }
    return OSAL_SUCCESS;
}
EXPORT_SYMBOL(osal_irq_request);

void osal_irq_free(unsigned int irq, void *dev)
{
    unsigned int _irq = irq;

#ifdef OSAL_IRQ_COMPATIBLE
    unsigned long flags;
    irq_info *irq_node = NULL;
    spin_lock_irqsave(&interrupt_lock, flags);
    irq_node = osal_irq_find_node(irq, &g_irq_list);
    if (irq_node != NULL) {
        _irq = irq_node->_irq;
        osal_list_del(&irq_node->node);
        kfree(irq_node);
        if (osal_list_empty(&g_irq_list)) {
            spin_unlock_irqrestore(&interrupt_lock, flags);
            printk("release irq mutex resource .. \n");
            free_irq(_irq, dev);
            return;
        }
    } else {
        printk("osal_irq_free irq=%d, is not exsit\n", irq);
        osal_dump_stack();
        spin_unlock_irqrestore(&interrupt_lock, flags);
        return;
    }
    spin_unlock_irqrestore(&interrupt_lock, flags);
#endif

    free_irq(_irq, dev);
}
EXPORT_SYMBOL(osal_irq_free);

int osal_irq_set_affinity(unsigned int irq, const char *name, int cpu_mask)
{
    struct cpumask cpumask_set;

    cpumask_clear(&cpumask_set);

    ((OSAL_CPU_0 & (unsigned int)cpu_mask) == 0) ?
        0 : cpumask_set_cpu(0, &cpumask_set); // cpu0

    ((OSAL_CPU_1 & (unsigned int)cpu_mask) == 0) ?
        0 : cpumask_set_cpu(1, &cpumask_set); // cpu1

    ((OSAL_CPU_2 & (unsigned int)cpu_mask) == 0) ?
        0 : cpumask_set_cpu(2, &cpumask_set); // cpu2

    ((OSAL_CPU_3 & (unsigned int)cpu_mask) == 0) ?
        0 : cpumask_set_cpu(3, &cpumask_set); // cpu 3

#ifdef OSAL_IRQ_COMPATIBLE
    irq = (unsigned int)osal_get_irq_by_name((char *)name); // 依赖外部函数,暂无实现
#endif
    if ((unsigned int)cpu_mask == OSAL_CPU_ALL) {
        return irq_set_affinity_hint(irq, NULL);
    }

    return irq_set_affinity_hint(irq, &cpumask_set);
}
EXPORT_SYMBOL(osal_irq_set_affinity);

int osal_in_interrupt(void)
{
    return in_interrupt();
}
EXPORT_SYMBOL(osal_in_interrupt);

/*
 * enable_irq - enable handling of an irq
 * @irq: Interrupt to enable
 *
 * Undoes the effect of one call to disable_irq().  If this
 * matches the last disable, processing of interrupts on this
 * IRQ line is re-enabled.
 * This function may be called from IRQ context only when
 * desc->irq_data.chip->bus_lock and desc->chip->bus_sync_unlock
 * are NULL !
 */
void osal_irq_enable(unsigned int irq)
{
    unsigned int _irq = irq;

#ifdef OSAL_IRQ_COMPATIBLE
    irq_info *irq_node = NULL;

    irq_node = osal_irq_find_node(irq, &g_irq_list);
    if (irq_node != NULL) {
        _irq = irq_node->_irq;
    } else {
        printk("osal_irq_enable irq=%d, is not exsit\n", irq);
        return;
    }
#endif

    enable_irq(_irq);

    return;
}
EXPORT_SYMBOL(osal_irq_enable);

/*
 * disable_irq - disable an irq and wait for completion
 * irq: Interrupt to disable
 *
 * Disable the selected interrupt line.  Enables and Disables are
 * nested.
 * This function waits for any pending IRQ handlers for this interrupt
 * to complete before returning. If you use this function while
 * holding a resource the IRQ handler may need you will deadlock.
 * This function may be called - with care - from IRQ context.
 **/
void osal_irq_disable(unsigned int irq)
{
    unsigned int _irq = irq;

#ifdef OSAL_IRQ_COMPATIBLE
    irq_info *irq_node = NULL;

    irq_node = osal_irq_find_node(irq, &g_irq_list);
    if (irq_node != NULL) {
        _irq = irq_node->_irq;
    } else {
        printk("osal_irq_disable irq=%d, is not exsit\n", irq);
        return;
    }
#endif

    disable_irq(_irq);

    return;
}
EXPORT_SYMBOL(osal_irq_disable);

/* tasklet is running only on one CPU simultaneously */
int osal_tasklet_init(osal_tasklet *tasklet)
{
    struct tasklet_struct *tasklet_local = NULL;

    if (tasklet == NULL || tasklet->tasklet != NULL) {
        printk("init tasklet is NULL ! \n");
        return OSAL_FAILURE;
    }

    tasklet_local = (struct tasklet_struct *)kmalloc(sizeof(struct tasklet_struct), GFP_KERNEL);
    if (tasklet_local == NULL) {
        printk("Tasklet initialize when malloc memory failed\n");
        return OSAL_FAILURE;
    }

    tasklet_local->next  = NULL;
    tasklet_local->state = 0;
    atomic_set(&tasklet_local->count, 0);
    tasklet_local->func = tasklet->handler;
    tasklet_local->data = tasklet->data;

    tasklet->tasklet = (void*)tasklet_local;

    return OSAL_SUCCESS;
}
EXPORT_SYMBOL(osal_tasklet_init);

/*
 * tasklet update should be called after tasklet init
 **/
int osal_tasklet_update(osal_tasklet *tasklet)
{
    struct tasklet_struct *tasklet_local = NULL;

    if (tasklet == NULL || tasklet->tasklet == NULL) {
        printk("tasklet setdata input is NULL\n");
        return OSAL_FAILURE;
    }

    tasklet_local = (struct tasklet_struct *)(tasklet->tasklet);
    tasklet_local->data = tasklet->data;
    tasklet_local->func = tasklet->handler;

    return OSAL_SUCCESS;
}
EXPORT_SYMBOL(osal_tasklet_update);

/*
 * add tasklet to tasklet_soc_vec and start tasklet
 **/
int osal_tasklet_schedule(osal_tasklet *tasklet)
{
    if (tasklet == NULL || tasklet->tasklet == NULL) {
        printk("schedule tasklet is NULL ! \n");
        return OSAL_FAILURE;
    }

    tasklet_schedule((struct tasklet_struct *)tasklet->tasklet);

    return OSAL_SUCCESS;
}
EXPORT_SYMBOL(osal_tasklet_schedule);

/* kill tasklet */
int osal_tasklet_kill(osal_tasklet *tasklet)
{
    if (tasklet == NULL || tasklet->tasklet == NULL) {
        printk("kill tasklet is NULL \n");
        return OSAL_FAILURE;
    }

    tasklet_kill((struct tasklet_struct *)tasklet->tasklet);
    kfree(tasklet->tasklet);
    tasklet->tasklet = NULL;

    return OSAL_SUCCESS;
}
EXPORT_SYMBOL(osal_tasklet_kill);