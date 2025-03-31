/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: diag cmd mem read/write.
 * Author:
 * Create: 2022-07-19
 */

#include "diag_cmd_mem_read_write.h"
#include "securec.h"
#include "diag_cmd_mem_read_write_st.h"
#include "soc_diag_cmd_id.h"
#include "hcc_if.h"
#include "hcc_cfg.h"
#include "oal_ext_util.h"
#include "oam_ext_if.h"
#include "oal_debug.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_DIAG_CMD_MEM_READ_WRITE_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_OAM_HOST

#define REPORT_DATA_PER_SIZE 64
#define REG_BIT_NUM 4

#ifdef _PRE_WLAN_REG_RW_WHITELIST
typedef struct {
    osal_u32 start;
    osal_u32 end;
} addr_items;

/*
 * 有交集需要合并, 否则查表结果可能错误
 * 前闭后闭，以字节为单位
 */
addr_items g_regaddr_whitelist[] = {
    /*
     * 0x40010000: PA_CONTROL1
     */
    {0x40010000, 0x40010003}, //  0x40010003: 0x40010004 - 1

    /*
     * 0x40010060: DURATION_ADJUST_VAL
     * 0x40010064: MAX_DURATION_CTRL
     * 0x40010068: OBSS_MAX_DURATION_VALUE
     */
    {0x40010060, 0x4001006B}, //  0x4001006B: 0x4001006C - 1

    /*
     * 0x400100A8: EDCA_TXOPLIMIT_ACBKBE
     * 0x400100AC: EDCA_TXOPLIMIT_ACBKBE
     * 0x400100B0: EDCA_TXOPLIMIT_ACBKBE
     * 0x400100B4: EDCA_TXOPLIMIT_ACVIVO
     */
    {0x400100A8, 0x400100B7}, //  0x400100B7: 0x400100B8 - 1

    /*
     * 0x40010138: BYPASS_SAMPLE_CONTROL
     */
    {0x40010138, 0x4001013B}, //  0x4001013B: 0x4001013C - 1

    /*
     * 0x4001e818: PAD_GPIO_06_CTRL
     * 0x4001e81c: PAD_GPIO_07_CTRL
     * 0x4001e820: PAD_GPIO_08_CTRL
     */
    {0x4001e818, 0x4001e823}, //  0x4001e823: 0x4001e824 - 1

    /*
     * 0x4001e830: PAD_GPIO_12_CTRL
     * 0x4001e834: PAD_GPIO_13_CTRL
     * 0x4001e838: PAD_GPIO_14_CTRL
     */
    {0x4001e830, 0x4001e83B}, //  0x4001e83B: 0x4001e83C - 1
};

ext_errno reg_rw_check_addr(td_u32 start_addr, td_u32 bytes_cnt)
{
    td_u32 ret = OAL_FAIL;
    td_u32 cnt;
    td_u32 i;
    td_u32 end_addr;

    end_addr = start_addr + bytes_cnt - 1;
    cnt = (td_u32)sizeof(g_regaddr_whitelist) / (td_u32)sizeof(addr_items);

    for (i = 0; i < cnt; i++) {
        // 根据起始地址判断在哪个白名单区间内
        if (start_addr > g_regaddr_whitelist[i].end) {
            continue;
        } else {
            if ((start_addr >= g_regaddr_whitelist[i].start) &&
                (end_addr <= g_regaddr_whitelist[i].end)) {
                ret = EXT_ERR_SUCCESS; // 在地址白名单区间内
            } else {
                ret = EXT_ERR_FAILURE;
            }
            break;
        }
    }

    return ret;
}
#endif

typedef struct {
    mem_read_ind_head_t head;
    td_u8 data[REPORT_DATA_PER_SIZE];
} mem_read_ind_common_t;

ext_errno diag_reg_read(td_u32 addr, td_u32 *read_val)
{
#ifdef _PRE_WLAN_REG_RW_WHITELIST
    if (reg_rw_check_addr(addr, REG_BIT_NUM) == EXT_ERR_SUCCESS) {
        return hcc_read_reg(HCC_CHANNEL_AP, addr, read_val);
    }
    return EXT_ERR_FAILURE;
#else
    return hcc_read_reg(HCC_CHANNEL_AP, addr, read_val);
#endif
}

ext_errno diag_reg_write(td_u32 addr, td_u32 val)
{
#ifdef _PRE_WLAN_REG_RW_WHITELIST
    if (reg_rw_check_addr(addr, REG_BIT_NUM) == EXT_ERR_SUCCESS) {
        return hcc_write_reg(HCC_CHANNEL_AP, addr, val);
    }
    return EXT_ERR_FAILURE;
#else
    return hcc_write_reg(HCC_CHANNEL_AP, addr, val);
#endif
}

static ext_errno diag_report_data(td_u16 cmd_id, diag_option *option, uintptr_t start_addr, uintptr_t end_addr)
{
    mem_read_ind_common_t ind;
    td_u32 value = 0xFFFFFFFF;
    uintptr_t addr = start_addr;
    td_u32 per_size = (td_u32)sizeof(td_u32);
    while (addr != end_addr) {
        ind.head.start_addr = addr;
        diag_reg_read(addr, &value);
        memcpy_s(ind.data, REPORT_DATA_PER_SIZE, (void *)&value, per_size);
        ind.head.size = per_size;
        uapi_zdiag_report_packet(cmd_id, option, (td_pbyte)&ind,
            (td_u16)sizeof(mem_read_ind_head_t) + per_size, TD_TRUE);
        addr += per_size;
    }
    return EXT_ERR_SUCCESS;
}

ext_errno diag_cmd_mem32(td_u16 cmd_id, td_pvoid cmd_param, td_u16 cmd_param_size, diag_option *option)
{
    mem_read_cmd_t *cmd =  cmd_param;
    uintptr_t end_addr = cmd->start_addr + (cmd->cnt * (td_u32)sizeof(td_u32));
    uapi_unused(cmd_param_size);
    return diag_report_data(cmd_id, option, cmd->start_addr, end_addr);
}

ext_errno diag_cmd_w4(td_u16 cmd_id, td_pvoid cmd_param, td_u16 cmd_param_size, diag_option *option)
{
    mem_write_cmd_t *cmd =  cmd_param;
    mem_write_ind_t ind;
    uapi_unused(cmd_param_size);
    diag_reg_write(cmd->start_addr, cmd->val);
    ind.ret = EXT_ERR_SUCCESS;
    uapi_zdiag_report_packet(cmd_id, option, (td_pbyte)&ind, (td_u16)sizeof(mem_write_ind_t), TD_TRUE);
    return EXT_ERR_SUCCESS;
}

ext_errno diag_cmd_mem_read_and_write(td_u16 cmd_id, td_pvoid cmd_param, td_u16 cmd_param_size, diag_option *option)
{
    switch (cmd_id) {
        case DIAG_CMD_MEM_MEM32:
            return diag_cmd_mem32(cmd_id, cmd_param, cmd_param_size, option);
        case DIAG_CMD_MEM_W4:
            return diag_cmd_w4(cmd_id, cmd_param, cmd_param_size, option);
        default:
            return EXT_ERR_NOT_SUPPORT;
    }
}
#ifdef _PRE_WLAN_REG_RW_WHITELIST
EXPORT_SYMBOL(reg_rw_check_addr);
#endif
