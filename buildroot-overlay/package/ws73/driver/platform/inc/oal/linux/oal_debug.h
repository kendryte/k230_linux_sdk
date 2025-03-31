/*
 * Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
 * Description: oal linux debug header file.
 * Author: Huanghe
 * Create: 2023-01-01
 */

#ifndef __OAL_DEBUG_H__
#define __OAL_DEBUG_H__

#include "oal_debug_base.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#ifdef CONFIG_PLAT_SUPPORT_OAL_PRINT
#ifdef CONTROLLER_CUSTOMIZATION
#define oal_print_emerg(s, args...)          do { \
        printk(KERN_EMERG KBUILD_MODNAME ":EMERG]" s, ##args); \
    } while (0)

#define oal_print_alert(s, args...)          do { \
        printk(KERN_EMERG KBUILD_MODNAME ":ALERT]" s, ##args); \
    } while (0)

#define oal_print_crit(s, args...)           do { \
        printk(KERN_EMERG KBUILD_MODNAME ":CRIT]" s, ##args); \
    } while (0)

#define oal_print_err(s, args...)            do { \
        printk(KERN_EMERG KBUILD_MODNAME ":E]" s, ##args); \
    } while (0)

#define oal_print_production_info(s, args...) do { \
        printk(KERN_EMERG KBUILD_MODNAME  s, ##args); \
    } while (0)

#define oal_print_warning(s, args...)        do { \
        printk(KERN_EMERG KBUILD_MODNAME ":W]" s, ##args); \
    } while (0)
#else
#define oal_print_emerg(s, args...)          do { \
        if (PLAT_LOG_EMERG <= g_plat_loglevel_etc) { \
            printk(KERN_EMERG KBUILD_MODNAME ":EMERG]" s, ##args); \
        } \
    } while (0)

#define oal_print_alert(s, args...)          do { \
        if (PLAT_LOG_ALERT <= g_plat_loglevel_etc) { \
            printk(KERN_ALERT KBUILD_MODNAME ":ALERT]" s, ##args); \
        } \
    } while (0)

#define oal_print_crit(s, args...)                                            \
    do {                                                                      \
        if (PLAT_LOG_CRIT <= g_plat_loglevel_etc) {                           \
            printk(KERN_CRIT KBUILD_MODNAME ":CRIT]" s, ##args); \
        }                                                                     \
    } while (0)

#define oal_print_err(s, args...)            do { \
        if (PLAT_LOG_ERR <= g_plat_loglevel_etc) { \
            printk(KERN_ERR KBUILD_MODNAME ":E]" s, ##args); \
        } \
    } while (0)

#define oal_print_production_info(s, args...)            do { \
        if (PLAT_LOG_ERR <= g_plat_loglevel_etc) { \
            printk(KERN_ERR s, ##args); \
        } \
    } while (0)

#define oal_print_warning(s, args...)        do { \
        if (PLAT_LOG_WARNING <= g_plat_loglevel_etc) { \
            printk(KERN_WARNING KBUILD_MODNAME ":W]" s, ##args); \
        } \
    } while (0)
#endif
#else
#define oal_print_emerg(s, args...)
#define oal_print_alert(s, args...)
#define oal_print_crit(s, args...)
#define oal_print_err(s, args...)
#define oal_print_warning(s, args...)
#define oal_print_production_info(s, args...)
#endif

#define oal_print_notice(s, args...)

#define oal_print_info(s, args...)

#define oal_print_dbg(s, args...)

#define oal_print_succ(s, args...)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of oal_debug.h */

