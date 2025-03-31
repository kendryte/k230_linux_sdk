/*
 * Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
 * Description: osal adapt timer file
 */

#ifdef _OSAL_LITEOS_SDK_
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/timer.h>
#endif
#include "osal_adapt.h"


/*
 * adapt timer 结构体与 cfbb 存在差异 为屏蔽timer结构体差异 将timer赋值&init合并
 * adapt SDK timer_init 只对timer->flag 进行赋值 并未执行LOS_SwtmrCreate 故将 init&set 合并
*/
int osal_adapt_timer_init(osal_timer *timer, void *func, unsigned long data, unsigned int interval)
{
#ifdef _OSAL_LITEOS_SDK_
    int ret;
    timer->timer    = NULL;
    timer->handler  = func;
    timer->data     = data;
    timer->interval = interval;
    ret = osal_timer_init(timer);
    if (ret == OSAL_FAILURE) {
        return OSAL_FAILURE;
    }
    return osal_set_timer(timer, interval);
#else /* _OSAL_LITEOS_CFBB_ */
    timer->timer    = NULL;
    timer->handler  = func;
    timer->data     = data;
    timer->interval = interval;
    return osal_timer_init(timer);
#endif
}

unsigned int osal_adapt_jiffies_to_msecs(const unsigned int n)
{
#ifdef _OSAL_LITEOS_SDK_
    return LOS_Tick2MS(n);
#else /* _OSAL_LITEOS_CFBB_ */
    return osal_jiffies_to_msecs(n);
#endif
}

int osal_adapt_timer_destroy(osal_timer *timer)
{
#ifdef _OSAL_LITEOS_SDK_
    return osal_timer_destory(timer);
#else /* _OSAL_LITEOS_CFBB_ */
    return osal_timer_destroy(timer);
#endif
}

unsigned long long osal_adapt_get_jiffies(void)
{
#ifdef _OSAL_LITEOS_SDK_
    return LOS_TickCountGet();
#else /* _OSAL_LITEOS_CFBB_ */
    return osal_get_jiffies();
#endif
}

int osal_adapt_timer_mod(osal_timer *timer, unsigned int interval)
{
#ifdef _OSAL_LITEOS_SDK_
    // adapt:osal_set_timer 启动定时器  cfbb:osal_timer_mod 修改定时器参数 并重新启动定时器
    return osal_set_timer(timer, interval);
#else /* _OSAL_LITEOS_CFBB_ */
    return osal_timer_mod(timer, interval);
#endif
}

#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
EXPORT_SYMBOL(osal_adapt_timer_init);
EXPORT_SYMBOL(osal_adapt_jiffies_to_msecs);
EXPORT_SYMBOL(osal_adapt_timer_destroy);
EXPORT_SYMBOL(osal_adapt_get_jiffies);
EXPORT_SYMBOL(osal_adapt_timer_mod);
#endif