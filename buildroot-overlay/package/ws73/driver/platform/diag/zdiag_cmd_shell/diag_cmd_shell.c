/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: diag cmd for mocked shell.
 */
#include "diag_cmd_shell.h"
#ifdef CONFIG_SUPPORT_MOCKED_SHELL
#include "td_base.h"
#include "oal_ext_util.h"
#include "efuse_opt.h"
#include "mac_addr.h"
#include "soc_zdiag.h"
#include "soc_osal.h"
#include "soc_diag_cmd_id.h"
#include "diag_cmd_mem_read_write.h"
#include "diag_cmd_mem_read_write_st.h"
#ifdef CONFIG_PLAT_TRNG_TRIG_RPT_TEST
#include "plat_misc.h"
#endif
#include "zdiag_adapt_layer.h"

#define ZDIGA_SHELL_CMD_EFUSE_PARAM 128

ext_errno diag_cmd_shell_read_reg(td_u16 cmd_id, td_pvoid cmd_param, td_u16 cmd_param_size, diag_option *option);
ext_errno diag_cmd_shell_write_reg(td_u16 cmd_id, td_pvoid cmd_param, td_u16 cmd_param_size, diag_option *option);
ext_errno diag_cmd_shell_read_efuse(td_u16 cmd_id, td_pvoid cmd_param, td_u16 cmd_param_size, diag_option *option);
ext_errno diag_cmd_shell_write_efuse(td_u16 cmd_id, td_pvoid cmd_param, td_u16 cmd_param_size, diag_option *option);
#ifdef CONFIG_PLAT_TRNG_TRIG_RPT_TEST
ext_errno diag_cmd_shell_trig_trng(td_u16 cmd_id, td_pvoid cmd_param, td_u16 cmd_param_size, diag_option *option);
#endif

typedef struct {
    osal_s8 *cmd_name;     /* 命令字符串 */
    td_zdiag_cmd_f func;   /* 命令对应处理函数 */
} zdiag_cmd_entry_stru;

typedef struct {
    osal_u16 efuse_id;
    osal_u16 bit_cnt;
    osal_u8  data[ZDIGA_SHELL_CMD_EFUSE_PARAM];
    osal_u32 len;
} zdiag_cmd_efuse_stru;

#ifdef CONFIG_PLAT_TRNG_TRIG_RPT_TEST
typedef struct {
    osal_u8  data[PLAT_TRNG_RANDOM_BYTES_MAX];
    osal_s32 cnt;
} trig_trng_t;
#endif

OSAL_STATIC zdiag_cmd_entry_stru  g_zdiag_shell_cmd[] = {
    {"read_reg",           diag_cmd_shell_read_reg},
    {"write_reg",          diag_cmd_shell_write_reg},
    {"read_efuse",         diag_cmd_shell_read_efuse},
    {"write_efuse",        diag_cmd_shell_write_efuse},
#ifdef CONFIG_PLAT_TRNG_TRIG_RPT_TEST
    {"trig_trng",           diag_cmd_shell_trig_trng},
#endif
};

diag_shell_api g_diag_shell_api;

ext_errno diag_shell_run_cmd(td_u16 cmd_id, td_pvoid cmd_param, td_u16 cmd_param_size, diag_option *option)
{
    if (g_diag_shell_api.run_sh != TD_NULL) {
        return g_diag_shell_api.run_sh(cmd_id, cmd_param, cmd_param_size, option);
    }
    return EXT_ERR_DIAG_NOT_SUPPORT;
}

td_void diag_shell_api_register(diag_shell_api *api)
{
    g_diag_shell_api = *api;
}

td_void diag_get_cmd_one_arg(td_s8 *cmd, td_s8 *arg, td_u32 arg_len, td_u32 *offset)
{
    td_s8 *cmd_copy = cmd;
    osal_u32 pos = 0;

    while (*cmd_copy == ' ') {
        ++cmd_copy;
    }

    while ((*cmd_copy != ' ') && (*cmd_copy != '\0') && pos < arg_len) {
        arg[pos] = *cmd_copy;
        ++pos;
        ++cmd_copy;
    }

    arg[pos]  = '\0';
    *offset = (osal_u32)(cmd_copy - cmd);
}

td_u32 diag_hex_to_char(osal_u8 *dest, td_u32 size, td_s8 *src, td_u32 cnt)
{
    td_u32 len = 0;
    td_u32 idx = 0;
    len += (td_u32)snprintf_s(dest, size, size, "%02x", src[idx]);
    for (idx = 1; idx < cnt; idx++) {
        len += (td_u32)snprintf_s(dest + len, size - len, size - len, ",%02x", src[idx]);
    }
    len += (td_u32)snprintf_s(dest + len, size - len, size - len, "\0\n");
    return len;
}

#define REPORT_DATA_PER_SIZE 128
ext_errno diag_cmd_read_reg(td_u16 cmd_id, diag_option *option, uintptr_t start_addr, td_u32 cnt)
{
    uintptr_t addr = start_addr;
    td_u32 value = 0xFFFFFFFF;
    td_u32 reg_cnt = cnt > REPORT_DATA_PER_SIZE ? REPORT_DATA_PER_SIZE : cnt; /* 限制单次读取寄存器个数 */
    td_u32 per_size = (td_u32)sizeof(td_u32);
    td_u8  buf[REPORT_DATA_PER_SIZE] = {0};
    td_u32 len;
    td_u32 rsv = REPORT_DATA_PER_SIZE;
    while (reg_cnt > 0) {
        len = 0;
        if (diag_reg_read(addr, &value) != EXT_ERR_SUCCESS) {
            len += (td_u32)snprintf_s(buf, rsv - len, rsv - len,
                "can't diag_reg_read, range isn't regaddr_whitelist.\n");
            uapi_zdiag_report_packet(cmd_id, option, buf, len, TD_TRUE);
            break;
        }
        len += (td_u32)snprintf_s(buf, rsv - len, rsv - len, "0x%08x:", addr);
        len += (td_u32)snprintf_s(buf + len, rsv - len, rsv - len, "0x%08x\n", value);
        uapi_zdiag_report_packet(cmd_id, option, buf, len, TD_TRUE);
        addr += per_size;
        reg_cnt--;
    }
    return EXT_ERR_SUCCESS;
}

ext_errno diag_cmd_write_reg(td_u16 cmd_id, diag_option *option, uintptr_t addr, td_u32 value)
{
    td_u8  buf[REPORT_DATA_PER_SIZE] = {0};
    ext_errno ret;
    td_u32 len = 0;

    ret = diag_reg_write(addr, value);
    if (ret == EXT_ERR_SUCCESS) {
        len += (td_u32)snprintf_s(buf, REPORT_DATA_PER_SIZE, REPORT_DATA_PER_SIZE, "succ.\n");
    } else {
        len += (td_u32)snprintf_s(buf, REPORT_DATA_PER_SIZE, REPORT_DATA_PER_SIZE,
            "can't diag_reg_write, range isn't regaddr_whitelist.\n");
    }

    uapi_zdiag_report_packet(cmd_id, option, buf, len, TD_TRUE);
    return EXT_ERR_SUCCESS;
}

#define ZDIGA_SHELL_CMD_REG_PARAM 128
ext_errno diag_cmd_shell_read_reg(td_u16 cmd_id, td_pvoid cmd_param, td_u16 cmd_param_size, diag_option *option)
{
    mem_read_cmd_t cmd = {0};
    td_s8 param[ZDIGA_SHELL_CMD_REG_PARAM] = {0};
    td_u32 offset;
    td_s8 *end = OSAL_NULL;

    diag_get_cmd_one_arg((td_s8 *)cmd_param, param, ZDIGA_SHELL_CMD_REG_PARAM, &offset);
    cmd.start_addr = (uintptr_t)oal_strtol(param, (td_s8 **)&end, 16);  // 16进制

    diag_get_cmd_one_arg((td_s8 *)cmd_param + offset, param, ZDIGA_SHELL_CMD_REG_PARAM, &offset);
    cmd.cnt = (td_u32)oal_atoi((osal_s8 *)param);

    diag_cmd_read_reg(cmd_id, option, cmd.start_addr, cmd.cnt);
    return EXT_ERR_SUCCESS;
}

ext_errno diag_cmd_shell_write_reg(td_u16 cmd_id, td_pvoid cmd_param, td_u16 cmd_param_size, diag_option *option)
{
    mem_read_cmd_t cmd = {0};
    td_s8 param[ZDIGA_SHELL_CMD_REG_PARAM] = {0};
    td_u32 offset;
    td_s8 *end = OSAL_NULL;

    diag_get_cmd_one_arg((td_s8 *)cmd_param, param, ZDIGA_SHELL_CMD_REG_PARAM, &offset);
    cmd.start_addr = (uintptr_t)oal_strtol(param, (td_s8 **)&end, 16);  // 16进制

    diag_get_cmd_one_arg((td_s8 *)cmd_param + offset, param, ZDIGA_SHELL_CMD_REG_PARAM, &offset);
    cmd.cnt = (td_u32)oal_strtol(param, (td_s8 **)&end, 16);   // 16进制

    diag_cmd_write_reg(cmd_id, option, cmd.start_addr, cmd.cnt);
    return EXT_ERR_SUCCESS;
}

#define BYTE_BIT_SIZE 8
ext_errno diag_cmd_shell_read_efuse(td_u16 cmd_id, td_pvoid cmd_param, td_u16 cmd_param_size, diag_option *option)
{
    zdiag_cmd_efuse_stru cmd = {0, 0, {0}, 0};
    td_s8 param[ZDIGA_SHELL_CMD_EFUSE_PARAM] = {0};
    td_u8  rsp_buf[REPORT_DATA_PER_SIZE] = {0};
    td_u32 len = 0;
    td_u32 offset;
    ext_errno ret = EXT_ERR_SUCCESS;

    diag_get_cmd_one_arg((td_s8 *)cmd_param, param, ZDIGA_SHELL_CMD_REG_PARAM, &offset);
    cmd.efuse_id = (td_u16)oal_atoi((osal_s8 *)param);

    diag_get_cmd_one_arg((td_s8 *)cmd_param + offset, param, ZDIGA_SHELL_CMD_REG_PARAM, &offset);
    cmd.bit_cnt = (td_u16)oal_atoi((osal_s8 *)param);

    cmd.len = ((cmd.bit_cnt % BYTE_BIT_SIZE != 0) ?
        (cmd.bit_cnt / BYTE_BIT_SIZE + 1) : (cmd.bit_cnt / BYTE_BIT_SIZE));
    ret = uapi_efuse_read(cmd.efuse_id, cmd.data, cmd.len);
    if (ret == EXT_ERR_SUCCESS) {
        len += diag_hex_to_char(rsp_buf, REPORT_DATA_PER_SIZE, cmd.data, cmd.len);
    } else {
        len += (td_u32)snprintf_s(rsp_buf, REPORT_DATA_PER_SIZE, REPORT_DATA_PER_SIZE, "uapi_efuse_read:fail[%d].\n",
            ret);
    }
    uapi_zdiag_report_packet(cmd_id, option, rsp_buf, len, TD_TRUE);
    return EXT_ERR_SUCCESS;
}

ext_errno diag_cmd_shell_write_efuse(td_u16 cmd_id, td_pvoid cmd_param, td_u16 cmd_param_size, diag_option *option)
{
    zdiag_cmd_efuse_stru cmd = {0, 0, {0}, 0};
    td_s8 param[ZDIGA_SHELL_CMD_EFUSE_PARAM] = {0};
    td_u8  rsp_buf[REPORT_DATA_PER_SIZE] = {0};
    td_u8  data_len = 0;
    td_u32 len = 0;
    td_u32 offset;
    ext_errno ret = EXT_ERR_SUCCESS;

    diag_get_cmd_one_arg((td_s8 *)cmd_param, param, ZDIGA_SHELL_CMD_REG_PARAM, &offset);
    cmd.efuse_id = (td_u16)oal_atoi((osal_s8 *)param);
    cmd_param += offset;

    diag_get_cmd_one_arg((td_s8 *)cmd_param, param, ZDIGA_SHELL_CMD_REG_PARAM, &offset);
    cmd.bit_cnt = (td_u16)oal_atoi((osal_s8 *)param);
    cmd_param += offset;

    diag_get_cmd_one_arg((td_s8 *)cmd_param, param, ZDIGA_SHELL_CMD_REG_PARAM, &offset);
    plat_parse_mac_addr(param, cmd.data, &data_len, ZDIGA_SHELL_CMD_EFUSE_PARAM);
    cmd_param += offset;

    diag_get_cmd_one_arg((td_s8 *)cmd_param, param, ZDIGA_SHELL_CMD_REG_PARAM, &offset);
    cmd.len = (td_u16)oal_atoi((osal_s8 *)param);

    ret = uapi_efuse_write(cmd.efuse_id, cmd.data, data_len);
    if (ret == EXT_ERR_SUCCESS) {
        len += (td_u32)snprintf_s(rsp_buf, REPORT_DATA_PER_SIZE, REPORT_DATA_PER_SIZE, "succ.\n");
    } else {
        len += (td_u32)snprintf_s(rsp_buf, REPORT_DATA_PER_SIZE, REPORT_DATA_PER_SIZE, "uapi_efuse_write:fail[0x%x].\n",
            ret);
    }
    uapi_zdiag_report_packet(cmd_id, option, rsp_buf, len, TD_TRUE);
    return EXT_ERR_SUCCESS;
}

#ifdef CONFIG_PLAT_TRNG_TRIG_RPT_TEST
#define ZDIGA_SHELL_CMD_TRIG_TRNG 128
ext_errno diag_cmd_shell_trig_trng(td_u16 cmd_id, td_pvoid cmd_param, td_u16 cmd_param_size, diag_option *option)
{
    ext_errno ret = EXT_ERR_SUCCESS;
    trig_trng_t cmd = {0};
    td_s8 param[ZDIGA_SHELL_CMD_TRIG_TRNG] = {0};
    td_u32 offset = 0;
    td_s8 *end = OSAL_NULL;
    td_u8  rsp_buf[REPORT_DATA_PER_SIZE] = {0};
    td_u32 buf_len = REPORT_DATA_PER_SIZE;
    td_u32 len = 0, get_size = 0;
    td_u32 res = 0;
    td_s32 i;

    zdiag_printf("cmd_id 0x%x\n", cmd_id);
    diag_get_cmd_one_arg((td_s8 *)cmd_param + offset, param, ZDIGA_SHELL_CMD_REG_PARAM, &offset);
    cmd.cnt = (td_s32)oal_strtol(param, (td_s8 **)&end, 16);   // 16进制
    if (cmd.cnt % 4) { // Check whether c is a multiple of 4.
        len += snprintf_s(rsp_buf + len, buf_len - len, buf_len - len - 1, "cnt is not 4 multiple.\n");
        uapi_zdiag_report_packet(cmd_id, option, rsp_buf, len, TD_TRUE);
        return EXT_ERR_DIAG_INVALID_PARAMETER;
    }
    zdiag_printf("cmd.cnt %d\n", cmd.cnt);
    while (cmd.cnt > 0) {
        res = plat_trng_get_random_bytes((td_u8 *)cmd.data, cmd.cnt, &get_size);
        if (res == EXT_ERR_SUCCESS) {
            len += snprintf_s(rsp_buf + len, buf_len - len, buf_len - len - 1, "succ get bytes[%d].\n", get_size);
        } else {
            len += snprintf_s(rsp_buf + len, buf_len - len, buf_len - len - 1, "plat_trng_get:fail[0x%x].\n", res);
            ret = res;
            break;
        }
        zdiag_printf("cmd.cnt %d, get_size %d\n", cmd.cnt, get_size);
        cmd.cnt -= get_size;
        for (i = 0; i < get_size; ++i) {
            len += snprintf_s(rsp_buf + len, buf_len - len, buf_len - len - 1, "[0x%02x].", cmd.data[i]);
        }
    }
    zdiag_printf("cmd.cnt %d, get_size %d, len %d\n", cmd.cnt, get_size, len);
    len += snprintf_s(rsp_buf + len, buf_len - len, buf_len - len - 1, "data end.\n");
    uapi_zdiag_report_packet(cmd_id, option, rsp_buf, len, TD_TRUE);
    zdiag_printf("len %d\n", len);
    return ret;
}
#endif

td_bool diag_find_shell_cmd(const td_s8 *l, const td_s8 *r, td_u32 len)
{
    td_u32 i;
    for (i = 0; i < len && *l == *r; i++, l++, r++);
    return i == len ? OAL_TRUE : OAL_FALSE;
}

ext_errno diag_cmd_mocked_shell(td_u16 cmd_id, td_pvoid cmd_param, td_u16 cmd_param_size, diag_option *option)
{
    td_u32 i = 0;
    td_u32 offset = 0x4; // hso shell固定数据偏移
    td_u32 len;
    uapi_unused(cmd_id);

    for (i = 0; i < sizeof(g_zdiag_shell_cmd) / sizeof(g_zdiag_shell_cmd[0]); i++) {
        len = (td_u32)osal_strlen(g_zdiag_shell_cmd[i].cmd_name);
        if (diag_find_shell_cmd(g_zdiag_shell_cmd[i].cmd_name, cmd_param + offset, len)) {
            offset += len;
            g_zdiag_shell_cmd[i].func(DIAG_CMD_ID_SHELL_CMD, cmd_param + offset, cmd_param_size - offset, option);
            return EXT_ERR_SUCCESS;
        }
    }

#if defined(CONFIG_DFX_SUPPORT_SHELL_PROC)
    zdiag_printf("diag cmd shell: size: %d, cmd_param: %s\n", cmd_param_size - offset, (td_char *)(cmd_param + offset));
    if (i == sizeof(g_zdiag_shell_cmd) / sizeof(g_zdiag_shell_cmd[0])) {
        diag_shell_run_cmd(DIAG_CMD_ID_SHELL_CMD, cmd_param + offset, cmd_param_size - offset, option);
    }
#endif
    return EXT_ERR_SUCCESS;
}
#endif
