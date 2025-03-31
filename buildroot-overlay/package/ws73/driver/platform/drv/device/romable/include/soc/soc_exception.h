/**
* @file soc_exception.h
*
* Copyright (c) CompanyNameMagicTag 2020-2020. All rights reserved.  \n
* Description: exception interfaces open-source.   \n
* Author: CompanyName   \n
* Create: 2020-10-15
*/

#ifndef __SOC_EXCEPTION_H__
#define __SOC_EXCEPTION_H__

#include <td_base.h>
#define TRACK_FRAME_PARAMS_MAX_NUM      2

typedef enum {
    EXCEPTION_PHASE_INITIALIZATION,
    EXCEPTION_PHASE_TASK,
    EXCEPTION_PHASE_INTERRUPT,
    EXCEPTION_PHASE_MAX_TYPE
} ext_exception_phase_type;

/* basic info for location exception */
typedef struct {
    td_u32 mepc;
    td_u32 ra;
    td_u32 sp;
} ext_exception_cpu_basic_info;

typedef struct {
    td_u32 mcause;
    td_u32 mstatus;
    td_u32 ccause;
    td_u32 mtval;
    td_u32 gp;
    td_u32 tp;
    td_u32 t0;
    td_u32 t1;
    td_u32 t2;
    td_u32 s0;
    td_u32 s1;
    td_u32 a0;
    td_u32 a1;
    td_u32 a2;
    td_u32 a3;
    td_u32 a4;
    td_u32 a5;
    td_u32 a6;
    td_u32 a7;
    td_u32 s2;
    td_u32 s3;
    td_u32 s4;
    td_u32 s5;
    td_u32 s6;
    td_u32 s7;
    td_u32 s8;
    td_u32 s9;
    td_u32 s10;
    td_u32 s11;
    td_u32 t3;
    td_u32 t4;
    td_u32 t5;
    td_u32 t6;
} ext_exception_cpu_extend_info;

typedef struct {
    td_u32  event_type;                           /* event type */
    td_u32  task;                                 /* current running task */
    td_u64  time;                                 /* current timestamp */
    td_uintptr_t identity;                        /* subject of the event description */
    td_uintptr_t params[TRACK_FRAME_PARAMS_MAX_NUM];    /* event frame's params */
} ext_exception_track_info;

typedef struct {
    ext_exception_cpu_basic_info basic_info;
    ext_exception_cpu_extend_info extend_info;
} ext_exception_cpu_register;

typedef struct {
    td_u16 is_irq;
    td_u16 type;
    td_u32 faultaddr;
    td_u32 thrdpid;
    td_u32 stack_top;
    td_u32 stack_bottom;
} ext_exception_schedule_info;

typedef struct {
    ext_exception_cpu_register exc_cpu_context;
    ext_exception_schedule_info schedule_info;
} ext_exception_context;

#endif