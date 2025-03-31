/*
 * Copyright (c) CompanyNameMagicTag 2020-2023. All rights reserved.
 * Description: pm extern head.
 * Create: 2020-04-06
 */

#ifndef __MP13_PM_EXTERN_H__
#define __MP13_PM_EXTERN_H__

#define PM_FREQ_MAX     PM_640MHZ
#define CPU_40MHZ       40000000UL
#define CPU_48MHZ       48000000UL
#define CPU_80MHZ       80000000UL
#define CPU_160MHZ      160000000UL
#define CPU_240MHZ      240000000UL
#define CPU_320MHZ      320000000UL
#define CPU_480MHZ      480000000UL
#define CPU_560MHZ      560000000UL
#define CPU_640MHZ      640000000UL

enum pm_cpu_freq_ex {
    PM_CG                                       = 0x0UL,    /* Clock Gating not support */
    PM_TCXO                                     = 0x1UL,    /* Tcxo */
    PM_48MHZ                                    = 0x8UL,    /* 48MHz */
    PM_80MHZ                                    = 0x9UL,    /* 80MHz */
    PM_160MHZ                                   = 0xBUL,    /* 160MHz */
    PM_320MHZ                                   = 0xFUL,    /* 320MHz */
    PM_480MHZ                                   = 0x13UL,   /* 480MHz */
    PM_640MHZ                                   = 0x1EUL,   /* pilot 640MHz, mpw 480MHz  */
    PM_WIFI_MAXFREQ                             = PM_640MHZ,
};
static inline td_u32 uapi_pm_is_host_sleep(osal_void)
{
    return 0;
}

static inline osal_void crg_wlan_enable_chg_cpu_freq(osal_bool enable)
{
    return;
}
#endif /* end of pm_extern.h */
