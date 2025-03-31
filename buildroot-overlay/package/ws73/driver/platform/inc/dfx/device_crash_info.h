/*
 * Copyright (c) CompanyNameMagicTag 2021-2023. All rights reserved.
 * Description: show device crash info header file
 * Author: Huanghe
 * Create: 2021-06-08
 */

#include "osal_types.h"
#include "oal_ext_util.h"
#include "td_base.h"
#ifndef _DEVICE_CRASH_INFO_H_
#define _DEVICE_CRASH_INFO_H_
/***********************************************************
 * WCPU CRASH INFO
 ***********************************************************/
#define DEV_STACK_START_ADDR 0x20000000
#define DEV_STACK_LEN        0x700

typedef struct {
    osal_u32  reserve;
    osal_u32  reg_sp;
    osal_u32  reg_lr;
    osal_u32  reg_cpsr;
    osal_u32  reg0;
    osal_u32  reg1;
    osal_u32  reg2;
    osal_u32  reg3;
    osal_u32  reg4;
    osal_u32  reg5;
    osal_u32  reg6;
    osal_u32  reg7;
    osal_u32  reg8;
    osal_u32  reg9;
    osal_u32  reg10;
    osal_u32  reg11;
    osal_u32  reg12;
    osal_u32  reg_pc;
} wl_syserr_exc_reg;

typedef enum {
    HARD_FAULT_DEBUG_EVENT,
    HARD_FAULT_OTHER_FAULT,
    HARD_FAULT_FETCHING_VECTOR,
    MEM_FAULT_ENTER_STACK,
    MEM_FAULT_QUIT_STACK,
    MEM_FAULT_DATA_ACCESS,
    MEM_FAULT_INSTRUCTION_ACCESS,
    BUS_FAULT_ENTER_STACK,
    BUS_FAULT_QUIT_STACK,
    BUS_FAULT_NON_EXACT_DATA_ACCESS,
    BUS_FAULT_DATA_ACCESS,
    BUS_FAULT_INSTRUCTION_ACCESS,
    UASGE_FAULT_DIV_ZERO,
    UASGE_FAULT_UNALIGNED_ACCESS,
    UASGE_FAULT_TRY_EXE_CO_PRO_INSTR,
    UASGE_FAULT_INVALID_EXC_RET_PC,
    UASGE_FAULT_TRY_ENTER_ARM,
    UASGE_FAULT_UNDEFINED_INSTRUCTION
} wl_crash_type;

typedef struct {
    osal_u32  int_id;
    osal_u32  fault_type;
    osal_u32  hard_fault_reason;
    osal_u32  usage_fault_reason;
    osal_u32  mem_bus_fault_reason;     /* all align to osal_u32 */
    osal_u32  address;
    osal_u32  crash_type;
} wl_status_reg_info;

typedef struct {
    osal_u32  stack_top;
    osal_u32  stack_buttom;
} wl_stack_info;

typedef struct {
    osal_u32  ram_text_start;
    osal_u32  ram_text_end;
    osal_u32  patch_text_start;
    osal_u32  patch_text_end;
    osal_u32  rom_text_start;
    osal_u32  rom_text_end;
} wl_text_info;

struct  wcpu_memdump_hdr_stru {
    osal_u32 panic_type;            /* abort,wdt or other */
    osal_u32 version;               /* for script parse */
    osal_u32 reg_store_addr;        /* reference to g_ulRegStoreAddr */
    osal_u32 memdump_addr;
    osal_u32 memdump_size;
    osal_u32 overflow_flag;         /* if deadbeaf means the mem overflow */
};

struct wcpu_panic_memdump_stru {
    struct wcpu_memdump_hdr_stru hdr;         /* 24B */
    wl_syserr_exc_reg reg;                    /* 18 */
    wl_status_reg_info    stat_reg_info;     /* 7 */
    wl_stack_info         stack_info;       /* 2 */
    wl_text_info          text_info;        /* 6 */
};

osal_void print_wcpu_device_crash_info(osal_u32 *pst_buf, osal_u32 ul_size);
osal_void print_wcpu_code_trace_info(osal_u32 *pst_buf, osal_u32 ul_size);

/***********************************************************
 * BCPU CRASH INFO
 ***********************************************************/
#define BT_DEV_SYSERR_INFO_LEN      224
#define BT_DEV_SYSERR_STACK_LEN     2048
#define BT_STACK_PART_1             1500
#define BT_STACK_PART_2             BT_DEV_SYSERR_STACK_LEN - BT_STACK_PART_1

#define CCAUSE_EXC_TYPE_MAX     12
#define EXC_STAGE_INIT       0
#define EXC_STAGE_TASK       1
#define EXC_STAGE_IRQ        2
#define EXC_STAGE_EXC        3

#define EXC_STAGE_STATUS_MAX     4
#define EXC_STAGE_STATUS_MAX_LEN 11
#define CCAUSE_EXC_TYPE_MAX     12
#define CCAUSE_EXC_INFO_MAX_LEN 36

#define SYSERR_EXC_TRACE_DEPTH 10       /* 异常前调度轨迹深度 */
#define SYSERR_EXC_TYPE_NAME_SIZE 8     /* 异常类型的名字字符串长度限制 */
#define SYSERR_EXC_KERNEL_VER_LEN_MAX 60
#define SYSERR_EXC_TASK_NAME_SIZE 8        /* 异常存储的任务名字字符串长度限制 */
#define SYSERR_ESP_PRE_DATA_MAX_NUM 8    /* 异常栈sp向栈底扩展深度(word为单位) */
#define SYSERR_ESP_DATA_MAX_NUM 36      /* 异常栈信息深度(word为单位) */
#define EXC_INVALID_STACK_DATA   0x11223344
#define STACK_OFFSET           144

typedef enum {
    INSTRU_ADDR_NOT_ALIGNED,
    INSTRU_ACCESS_FAULT,
    INVALID_INSTRU,
    BREAK_POINT,
    LOAD_ADDR_NOT_ALIGNED,
    LOAD_ACCESS_FAULT,
    STORE_ADDR_NOT_ALIGNED,
    STORE_ACCESS_FAULT,
    ENVIR_CALL_FROM_U_MODE,
    ENVIR_CALL_FROM_S_MODE,
    ENVIR_CALL_FROM_H_MODE,
    ENVIR_CALL_FROM_M_MODE,
    EXEC_CODE_BUTT
} syserr_exception_code;

/* using for hcc xfer */
typedef enum {
    XFER_NOT_START = 0,
    CRASH_INFO_OK,
    STACK_1_OK,
    STACK_2_OK
} oam_hcc_bt_syserr_xfer_type;

/* using for hcc xfer */
typedef struct {
    osal_u32 crash_info[BT_DEV_SYSERR_INFO_LEN];
    osal_u16 len;
} oam_bt_crash_stru;

/* using for hcc xfer */
typedef struct {
    osal_u32 stack_val[BT_STACK_PART_1 / WORD_WIDTH];
    osal_u32 stack_num;
    osal_u16 len;
} oam_bt_stack_stru;

typedef struct {
    osal_char name[SYSERR_EXC_TASK_NAME_SIZE];
    osal_u32 id;  /* task id.:任务ID. */
    osal_u32 status;
    osal_u32 stack_peak;
    osal_u32 stack_size;
    uintptr_t sp;
    uintptr_t stack_top;
    uintptr_t stack_bottom;
    osal_u32 real_sp;
    osal_u32 overflow_flag;
} syserr_task_info;

typedef struct {
    osal_u32 ram_text_start_addr;
    osal_u32 ram_text_end_addr;
    osal_u32 rom_text_start_addr;
    osal_u32 rom_text_end_addr;
    syserr_task_info task;         /* task information.:任务信息. */
} bt_syserr_stack_info;

typedef struct {
    osal_u32 ccause;
    osal_u32 mcause;
    osal_u32 mtval;
    osal_u32 gp;
    osal_u32 mstatus;
    osal_u32 mepc;
    osal_u32 tp;
    osal_u32 sp;
    osal_u32 s11;
    osal_u32 s10;
    osal_u32 s9;
    osal_u32 s8;
    osal_u32 s7;
    osal_u32 s6;
    osal_u32 s5;
    osal_u32 s4;
    osal_u32 s3;
    osal_u32 s2;
    osal_u32 s1;
    osal_u32 s0;
    osal_u32 t6;
    osal_u32 t5;
    osal_u32 t4;
    osal_u32 t3;
    osal_u32 a7;
    osal_u32 a6;
    osal_u32 a5;
    osal_u32 a4;
    osal_u32 a3;
    osal_u32 a2;
    osal_u32 a1;
    osal_u32 a0;
    osal_u32 t2;
    osal_u32 t1;
    osal_u32 t0;
    osal_u32 ra;
} syserr_reg_info;

typedef struct {
    osal_u32 mstatus;     /* CPU mstatus register value.:CPU mstatus寄存器值. */
    osal_u32 mtval;       /* CPU mtval register value.:CPU mtval寄存器值. */
    osal_u32 mcause;      /* CPU mcause register value.:CPU mcause寄存器值. */
    osal_u32 ccause;      /* CPU ccause register value.:CPU ccause寄存器值. */
    osal_u16 cur_task_id; /* current task id.:当前任务ID. */
    osal_u16 exc_stage;
} syserr_core_info;

typedef struct {
    syserr_reg_info reg_info;     /* CPU registers of crash information.:死机CPU寄存器信息. */
    syserr_core_info core_info;   /* CPU CSR registers of crash information.:死机CPU状态寄存器信息. */
    bt_syserr_stack_info stack_info;
} bt_syserr_info;

bt_syserr_info* bt_syserr_get_ptr(void);
osal_void print_bcpu_code_crash_info(bt_syserr_info* info);
osal_void print_bcpu_code_trace_info(bt_syserr_info* info, oal_uint8* stack);

#endif