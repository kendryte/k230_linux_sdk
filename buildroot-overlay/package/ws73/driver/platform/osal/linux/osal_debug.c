/*
 * Copyright (c) CompanyNameMagicTag 2012-2021. All rights reserved.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/printk.h>
#include "soc_osal.h"


void osal_panic(const char *fmt, const char *fun, int line, const char *cond)
{
    panic(fmt, fun, line, cond);
}
EXPORT_SYMBOL(osal_panic);

void osal_dump_stack(void)
{
    dump_stack();
}
EXPORT_SYMBOL(osal_dump_stack);

void osal_printk(const char *fmt, ...)
{
    va_list args;

    if (fmt == NULL) {
        return;
    }

    va_start(args, fmt);
    vprintk(fmt, args);
    va_end(args);
}
EXPORT_SYMBOL(osal_printk);
