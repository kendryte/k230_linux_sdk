/*
 * Copyright (c) CompanyNameMagicTag. 2023-2023. All rights reserved.
 * Description: source file.
 * Author: CompanyName
 * Create: 2023-01-07
 */

#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include <linux/version.h>
#include <linux/module.h>
#include <linux/kernel.h>
#ifdef CONFIG_OF
#include <linux/of.h>
#endif
#include <linux/time.h>
#include <linux/fs.h>
#endif /* end of (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION) */

#include "ini.h"
#include "plat_pm_board.h"
#include "oal_schedule.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_EXT_INI_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

/*
 * 2 Global Variable Definition
 */
#define CUST_PATH_INI_CONN             "/data/vendor/cust_conn/ini_cfg"     /* 某运营商在不同产品的差异配置 */
/* mutex for open ini file */
#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
struct mutex file_mutex_etc;
#elif defined(_PRE_OS_VERSION_LITEOS) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
osal_mutex file_mutex_etc;
#endif
#if defined(HOST_BOARD) && defined(BOARD_SHAOLINAXE) && (HOST_BOARD == BOARD_SHAOLINAXE)
osal_char g_ini_file_name_etc[INI_FILE_PATH_LEN] = "/vendor/etc/wifi/ws73_cfg.ini";
#else
osal_char g_ini_file_name_etc[INI_FILE_PATH_LEN] = CONFIG_INI_FILE_PATH;
#endif
osal_char g_ini_conn_file_name_etc[INI_FILE_PATH_LEN] = {0};
#define INI_FILE_PATH           (g_ini_file_name_etc)

ini_board_version_stru g_board_version_etc = {{0}};
ini_param_version_stru g_param_version_etc = {{0}};

osal_slong g_ini_file_time_sec = 0;

static osal_s32 ko_read_line(ini_file *fp, char *addr)
{
    osal_s32 ret;
    osal_char  tmp[MAX_READ_LINE_NUM] = {0};
    osal_s32 cnt = 0;
#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
    loff_t last_pos = fp->f_pos;
    ret = kernel_read(fp, tmp, MAX_READ_LINE_NUM, &fp->f_pos);
#else
    ret = kernel_read(fp, fp->f_pos, tmp, MAX_READ_LINE_NUM);
#endif
#elif defined(_PRE_OS_VERSION_LITEOS) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
    ret = fread(tmp, sizeof(char), MAX_READ_LINE_NUM, fp);
#endif
    if (ret < 0) {
        oal_print_err("kernel_line read ret < 0");
        return INI_FAILED;
    } else if (ret == 0) {
        /* end of file */
        return 0;
    }

    cnt = 0;
    while ((tmp[cnt] != '\n') && (cnt < MAX_READ_LINE_NUM - 1)) {
        *addr++ = tmp[cnt++];
    }

    if (cnt <= (MAX_READ_LINE_NUM - 1)) {
        *addr = '\n';
    } else {
        oal_print_err("ko read_line is unexpected");
        return INI_FAILED;
    }

    /* change file pos to next line */
#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
    fp->f_pos = (cnt + 1) + last_pos;
#else
    fp->f_pos += (cnt + 1);
#endif
#endif
    return ret;
}

static ini_file *ini_file_open(osal_char *filename, osal_char *para)
{
    ini_file *fp;

    uapi_unused(para);
#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    fp = (ini_file *)filp_open(filename, O_RDONLY, 0);
    if (OAL_IS_ERR_OR_NULL(fp)) {
#elif defined(_PRE_OS_VERSION_LITEOS) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
    fp = (ini_file *)fopen(filename, "r");
    if (fp == NULL) {
#endif
        fp = NULL;
    }

    return fp;
}

static osal_s32 ini_file_close(ini_file *fp)
{
#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    filp_close(fp, NULL);
#elif defined(_PRE_OS_VERSION_LITEOS) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
    int fclose_ret = fclose(fp);
    if (fclose_ret != 0) {
        return INI_FAILED;
    }
#endif
    fp = NULL;
    return INI_SUCC;
}

static bool ini_file_exist(osal_char *file_path)
{
    ini_file *fp = NULL;

    if (file_path == NULL) {
        oal_print_err("para file_path is NULL\n");
        return false;
    }

    fp = ini_file_open(file_path, "rt");
    if (fp == NULL) {
        oal_print_dbg("%s not exist\n", file_path);
        return false;
    }

    ini_file_close(fp);

    return true;
}

/* Note:symbol "fp" not accessed */
#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
static osal_s32 ini_file_seek(ini_file *fp, osal_s16 fp_pos)
#else
static osal_s32 ini_file_seek(ini_file *fp, long fp_pos)
#endif
#elif defined(_PRE_OS_VERSION_LITEOS) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
static osal_s32 ini_file_seek(ini_file *fp, off_t fp_pos)
#endif
{
#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    fp->f_pos += fp_pos;
#elif defined(_PRE_OS_VERSION_LITEOS) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
    lseek(fp, fp_pos, SEEK_CUR);
#endif
    return INI_SUCC;
}

static osal_s32 ini_readline_func(ini_file *fp, osal_char *rd_buf)
{
    osal_char tmp[MAX_READ_LINE_NUM];
    osal_s32 ret;

    (osal_void)memset_s(tmp, MAX_READ_LINE_NUM, 0, MAX_READ_LINE_NUM);
    ret = ko_read_line(fp, tmp);
    if (ret == INI_FAILED) {
        oal_print_err("ko_read_line failed!!!");
        return INI_FAILED;
    } else if (ret == 0) {
        oal_print_err("end of .ini file!!!");
        return INI_FAILED;
    }

    ret = strncpy_s(rd_buf, MAX_READ_LINE_NUM, tmp, MAX_READ_LINE_NUM - 1);
    if (ret != EOK) {
        return INI_FAILED;
    }

    return INI_SUCC;
}

osal_s32 ini_check_str_etc(ini_file *fp, osal_char *tmp, osal_char *var)
{
    osal_u16 len;
    osal_u16 curr_var_len;
    osal_u16 search_var_len;

    if ((fp == NULL) || (var == NULL) || (var[0] == '\0')) {
        oal_print_err("check if var is NULL or blank");
        return INI_FAILED;
    }

    do {
        len = (osal_u16)strlen(tmp);
        curr_var_len = 0;

        while ((curr_var_len < MAX_READ_LINE_NUM) && (tmp[curr_var_len] != '\r') && (tmp[curr_var_len] != '\n') &&
                (tmp[curr_var_len] != 0)) {
            curr_var_len++;
        }

        if ((tmp[0] == '#') || (tmp[0] == ' ') || (tmp[0] == '\n') || (tmp[0] == '\r')) {
            break;
        }
        search_var_len = (osal_u16)strlen(var);
        if (search_var_len > curr_var_len) {
            break;
        }
        if (strncmp(tmp, var, search_var_len) == 0) {
            return INI_SUCC;
        }
    } while (0);

    if (ini_file_seek(fp, -len) == INI_FAILED) {
        oal_print_err("file seek failed!!!");
        return INI_FAILED;
    }
    if (ini_file_seek(fp, curr_var_len + 1) == INI_FAILED) {
        oal_print_err("file seek failed!!!");
        return INI_FAILED;
    }
    if (((curr_var_len + 1) < MAX_READ_LINE_NUM) && (tmp[curr_var_len + 1] == '\n')) {
        if (ini_file_seek(fp, 1) == INI_FAILED) {
            oal_print_err("file seek failed!!!");
            return INI_FAILED;
        }
    }
    return INI_FAILED;
}

static osal_s32 ini_check_value(osal_char *value)
{
    size_t len = 0x00;

    len = strlen(value);
    if (len < 2) { /* The minimum length of value is 2 */
        oal_print_err("ini_check_value fail, value length %zu < 2(min len)\n", len);
        return INI_FAILED;
    }

    if (value[0] == ' ' || value[len - 1] == ' ' || value[0] == '\n') {
        value[0] = '\0';
        oal_print_err("::%s has blank space or is blank::", value);
        return INI_FAILED;
    }

    /* check \n of line */
    if (value[len - 1] == '\n') {
        value[len - 1] = '\0';
    }

    /* check \r of line */
    if (value[len - 2] == '\r') { /* 2 penultimate character */
        value[len - 2] = '\0'; /* 2 penultimate character */
    }

    return INI_SUCC;
}

static osal_void ini_set_modu_str(osal_s32 tag_index, osal_s8 *modu_str, osal_u32 modu_str_len)
{
    osal_s32 ret = EINVAL;
    switch (tag_index) {
        case INI_MODU_WIFI:
            ret = strncpy_s(modu_str, modu_str_len, INI_STR_WIFI_NORMAL, modu_str_len - 1);
            break;
        case INI_MODU_BT:
            ret = strncpy_s(modu_str, modu_str_len, INI_STR_BT_NORMAL, modu_str_len - 1);
            break;
        case INI_MODU_PLAT:
            ret = strncpy_s(modu_str, modu_str_len, INI_STR_PLAT, modu_str_len - 1);
            break;
        case INI_MODU_HOST_VERSION:
            ret = strncpy_s(modu_str, modu_str_len, INT_STR_HOST_VERSION, modu_str_len - 1);
            break;
        case INI_MODU_WIFI_MAC:
            ret = strncpy_s(modu_str, modu_str_len, INI_STR_WIFI_MAC, modu_str_len - 1);
            break;
        case INI_MODU_COEXIST:
            ret = strncpy_s(modu_str, modu_str_len, INI_STR_COEXIST, modu_str_len - 1);
            break;
        case INI_MODU_DEV_WIFI:
            ret = strncpy_s(modu_str, modu_str_len, INI_STR_DEVICE_WIFI, modu_str_len - 1);
            break;
        case INI_MODU_DEV_BT:
            ret = strncpy_s(modu_str, modu_str_len, INI_STR_DEVICE_BT, modu_str_len - 1);
            break;
        case INI_MODU_DEV_BFG_PLAT:
            ret = strncpy_s(modu_str, modu_str_len, INI_STR_DEVICE_BFG_PLAT, modu_str_len - 1);
            break;
        case INI_MODU_FCC_NVRAM:
            ret = strncpy_s(modu_str, modu_str_len, INI_STR_WIFI_FCC_NVRAM, modu_str_len - 1);
            break;
        case INI_MODU_CE_NVRAM:
            ret = strncpy_s(modu_str, modu_str_len, INI_STR_WIFI_CE_NVRAM, modu_str_len - 1);
            break;
        case INI_MODU_JP_NVRAM:
            ret = strncpy_s(modu_str, modu_str_len, INI_STR_WIFI_JP_NVRAM, modu_str_len - 1);
            break;
        default :
            oal_print_err("not suport tag type:%x!!!", tag_index);
            return;
    }
    if (ret != EOK) {
        oal_print_err("ini_set_modu_str strncpy_s failed!!");
    }
}

static osal_s32 ini_find_modu(ini_file *fp, osal_s32 tag_index, osal_char *var, osal_char *value)
{
    osal_char tmp[MAX_READ_LINE_NUM] = {0};
    osal_s8 modu_str[INI_STR_MODU_LEN] = {0};
    osal_s32 ret;

    uapi_unused(var);
    uapi_unused(value);

    ini_set_modu_str(tag_index, modu_str, INI_STR_MODU_LEN);
    /* find the value of mode var, such as ini_wifi_mode
     * every mode except PLAT mode has only one mode var */
    for (;;) {
        ret = ini_readline_func(fp, tmp);
        if (ret == INI_FAILED) {
            oal_print_err("have end of .ini file!!!");
            return INI_FAILED;
        }

        if (strstr(tmp, INI_STR_DEVICE_BFG_PLAT) != NULL) {
            oal_print_err("not find %s!!!", modu_str);
            return INI_FAILED;
        }

        ret = ini_check_str_etc(fp, tmp, modu_str);
        if (ret == INI_SUCC) {
            break;
        } else {
            continue;
        }
    }

    return INI_SUCC;
}

static osal_s32 ini_find_var(ini_file *fp, osal_s32 tag_index, osal_char * var, osal_char *value,
                            osal_u32 size)
{
    osal_s32 ret;
    osal_char tmp[MAX_READ_LINE_NUM + 1] = {0};
    size_t search_var_len;

    /* find the modu of var, such as [HOST_WIFI_NORMAL] of wifi moduler */
    ret = ini_find_modu(fp, tag_index, var, value);
    if (ret == INI_FAILED) {
        return INI_FAILED;
    }

    /* find the var in modu, such as [HOST_WIFI_NORMAL] of wifi moduler */
    for (;;) {
        ret = ini_readline_func(fp, tmp);
        if (ret == INI_FAILED) {
            oal_print_err("have end of .ini file!!!");
            return INI_FAILED;
        }

        if (tmp[0] == '[') {
            return INI_FAILED;
        }

        search_var_len = strlen(var);
        ret = ini_check_str_etc(fp, tmp, var);
        if ((ret == INI_SUCC) && (tmp[search_var_len] == '=')) {
            ret = strncpy_s(value, size, &tmp[search_var_len+1], size - 1);
            if (ret != EOK) {
                return INI_FAILED;
            }
            break;
        } else {
            continue;
        }
    }

    return INI_SUCC;
}

osal_s32 ini_find_var_value_by_path_etc(osal_char *path, osal_s32 tag_index, osal_char *var,
                                        osal_char *value, osal_u32 size)
{
    ini_file *fp = NULL;

#ifdef INI_TIME_TEST
    struct timeval tv[2];
#endif

    osal_s32 ret;

    if (var == NULL || var[0] == '\0' || value == NULL) {
        oal_print_err("check if var and value is NULL or blank");
        return INI_FAILED;
    }

#ifdef INI_TIME_TEST
    do_gettimeofday(&tv[0]);
#endif

    ini_mutex_lock(&file_mutex_etc);

    fp = ini_file_open(path, "rt");
    if (fp == 0) {
        oal_print_err("open %s failed!!!", path);
        ini_mutex_unlock(&file_mutex_etc);
        return INI_FAILED;
    }

    /* find var in .ini return value */
    ret = ini_find_var(fp, tag_index, var, value, size);
    if (ret == INI_FAILED) {
        value[0] = '\0';
        ini_file_close(fp);
        ini_mutex_unlock(&file_mutex_etc);
        return INI_FAILED;
    }

#ifdef INI_TIME_TEST
    do_gettimeofday(&tv[1]);
    /* 1000 for usec to nsec */
    oal_print_dbg("time take = %ld", (tv[1].tv_sec - tv[0].tv_sec) * 1000 + (tv[1].tv_usec - tv[0].tv_usec) / 1000);
#endif

    ini_file_close(fp);
    ini_mutex_unlock(&file_mutex_etc);

    /* check blank space of value */
    if (ini_check_value(value) == INI_SUCC) {
        return INI_SUCC;
    }

    return INI_FAILED;
}

osal_s32 ini_find_var_value_etc(osal_s32 tag_index, osal_char *var, osal_char *value, osal_u32 size)
{
    if (ini_file_exist(INI_FILE_PATH) == 0) {
        oal_print_err(" %s not exist!!!", INI_FILE_PATH);
        return INI_FAILED;
    }

    return ini_find_var_value_by_path_etc(INI_FILE_PATH, tag_index, var, value, size);
}

osal_s32 get_cust_conf_string_etc(osal_s32 tag_index, osal_char *var, osal_char *value, osal_u32 size)
{
    osal_s32 ret;
    ret = memset_s(value, size, 0, size);
    if (ret != EOK) {
        oal_print_err("memset_s fail!\r\n");
    }
    return ini_find_var_value_etc(tag_index, var, value, size);
}

osal_s32 set_cust_conf_string_etc(osal_s32 tag_index, osal_char *name, osal_char *var)
{
    osal_s32 ret = INI_FAILED;

    if (tag_index != CUST_MODU_NVRAM) {
        oal_print_err("NOT SUPPORT MODU TO WRITE");
        return INI_FAILED;
    }
    return ret;
}

osal_s32 get_cust_conf_int32_etc(osal_s32 tag_index, osal_char *var, osal_s32 *value)
{
    osal_s32 ret = 0;
    osal_char  str[INI_READ_VALUE_LEN] = {0};

    ret = ini_find_var_value_etc(tag_index, var, str, sizeof(str));
    if (ret < 0) {
        /* ini_find_var_value_etc has error log, delete this log */
        oal_print_dbg("cust modu didn't get var of %s.", var);
        return INI_FAILED;
    }

    if (!strncmp(str, "0x", strlen("0x")) || !strncmp(str, "0X", strlen("0X"))) {
        oal_print_dbg("get hex of:%s.", var);
        ret = sscanf_s(str, "%x", value);
    } else {
        ret = sscanf_s(str, "%d", value);
    }

    if (ret < 0) {
        oal_print_err("%s trans to int failed", var);
        return INI_FAILED;
    }

    oal_print_dbg("conf %s get vale:%d", var, *value);

    return INI_SUCC;
}

static osal_s32 get_ini_file(osal_char *file_path, ini_file **fp)
{
    if (file_path == NULL) {
        oal_print_info("para file_path is NULL\n");
        return INI_FAILED;
    }

    *fp = ini_file_open(file_path, "rt");
    if (*fp == NULL) {
        oal_print_info("inifile %s not exist\n", file_path);
        return INI_FAILED;
    }

    return INI_SUCC;
}

static osal_s32 ini_file_check_timespec(ini_file *fp)
{
    if (fp == NULL) {
        oal_print_err("para file is NULL\n");
        return INI_FILE_TIMESPEC_UNRECONFIG;
    }

#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    if (fp->f_path.dentry == NULL) {
        oal_print_err("file dentry is NULL\n");
        return INI_FILE_TIMESPEC_UNRECONFIG;
    }

    if (g_ini_file_time_sec != inf_file_get_ctime(fp->f_path.dentry)) {
        oal_print_info("ini_file time_secs changed from [%ld]to[%ld]\n", g_ini_file_time_sec,
            (osal_slong)inf_file_get_ctime(fp->f_path.dentry));
        g_ini_file_time_sec = inf_file_get_ctime(fp->f_path.dentry);

        return INI_FILE_TIMESPEC_RECONFIG;
    } else {
        oal_print_info("ini file is not upadted time_secs[%ld]\n", g_ini_file_time_sec);
        return INI_FILE_TIMESPEC_UNRECONFIG;
    }
#endif
    return INI_FILE_TIMESPEC_UNRECONFIG;
}

static osal_s32 ini_file_check_valid_timespec(osal_char *file_name)
{
    ini_file *fp = NULL;
    osal_s32 ret;

    ret = get_ini_file(g_ini_conn_file_name_etc, &fp);
    if (ret != INI_SUCC) {
        return INI_FAILED;
    }

    if (ini_file_check_timespec(fp) != (osal_s32)INI_FILE_TIMESPEC_RECONFIG) {
        ini_file_close(fp);
        return INI_FAILED;
    }

    ini_file_close(fp);
    return INI_SUCC;
}

osal_s32 ini_file_check_conf_update(void)
{
    if (ini_file_check_valid_timespec(g_ini_conn_file_name_etc) == INI_SUCC) {
        oal_print_info("%s ini file is updated\n", g_ini_conn_file_name_etc);
        return INI_FILE_TIMESPEC_RECONFIG;
    } else if (ini_file_check_valid_timespec(INI_FILE_PATH) == INI_SUCC) {
        oal_print_info("%s ini file is updated\n", INI_FILE_PATH);
        return INI_FILE_TIMESPEC_RECONFIG;
    } else {
        oal_print_info("no ini file is updated\n");
        return INI_FILE_TIMESPEC_UNRECONFIG;
    }
}

int ini_cfg_init_etc(void)
{
    osal_s32 ret;

    ret = snprintf_s(g_ini_conn_file_name_etc, sizeof(g_ini_file_name_etc), sizeof(g_ini_file_name_etc) - 1,
        "%s", g_ini_file_name_etc);
    if (ret < 0) {
        return INI_FAILED;
    }
    get_pm_board_info()->ini_file_name = g_ini_file_name_etc;
    ret = strncpy_s((osal_char*)(g_board_version_etc.board_version), INI_VERSION_STR_LEN,
        "ws73", INI_VERSION_STR_LEN - 1);
    if (ret != EOK) {
        return INI_FAILED;
    }
    ret = strncpy_s((osal_char*)(g_param_version_etc.param_version), INI_VERSION_STR_LEN,
        "ws73", INI_VERSION_STR_LEN - 1);
    if (ret != EOK) {
        return INI_FAILED;
    }
    ini_init_mutex(&file_mutex_etc);
    oal_print_warning("ini_file_name@%s\n", g_ini_file_name_etc);
    return INI_SUCC;
}

void ini_cfg_exit_etc(void)
{
    ini_destroy_mutex(&file_mutex_etc);
    oal_print_warning("ini config search exit!\n");
}

/*
 * 功能描述  : 从字符串中分割定制化项列表值 支持32bit
 * 参数  :  cust_param 字符串参数
 * param_list 定制化参数列表 出参 实际长度*list_len 传入长度max_len
 * param_len 参数字节长 每个出参的长度 字节 INI_PARAM_BYTE_ONE INI_PARAM_BYTE_TWO INI_PARAM_BYTE_FOUR
 * list_len 参数列表长度 数量 实际占用长度根据param_len计算
 * max_len  列表可用最大长度 字节数
 */
osal_u32 ini_get_cust_item_list(const char *cust_param, osal_u8 *param_list, osal_u8 param_len, osal_u8 *list_len,
    osal_u8 max_len)
{
    char *pc_token = NULL;
    char *pc_ctx = NULL;
    const char *pc_end = ";";
    const char *pc_sep = ",";
    osal_u8 param_num = 0;
    osal_s32 param = 0;
    char cust_param_str[INI_READ_VALUE_LEN] = {0};
    osal_s32 ret;
    *list_len = 0; // 初始化为0

    if (memcpy_s(cust_param_str, INI_READ_VALUE_LEN, cust_param, strlen(cust_param)) != EOK) {
        oal_print_err("ini_get_cust_item_list input param too long!\r\n");
        return OAL_FAIL;
    }
    pc_token = strtok_s(cust_param_str, pc_end, &pc_ctx);
    if (pc_token == NULL) {
        oal_print_err("ini_get_cust_item_list read get null value check!\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }
    pc_token = strtok_s(pc_token, pc_sep, &pc_ctx);
    /* 获取定制化系数 */
    while (pc_token != NULL) {
        // 考虑即将占用的数据长度 不能超过总长
        if (param_num * param_len + param_len > max_len) {
            oal_print_warning("ini_get_cust_item_list::param num:%d len:%d over max%d\r\n",
                param_num, param_len, max_len);
            break;
        }
        /* 将字符串转换成数字 */
        if (!strncmp(pc_token, "0x", strlen("0x")) || !strncmp(pc_token, "0X", strlen("0X"))) {
            ret = sscanf_s(pc_token, "%x", &param);
        } else {
            ret = sscanf_s(pc_token, "%d", &param);
        }
        if (ret <= 0) {
            oal_print_err("ini_get_cust_item_list::sscanf_s fail\r\n");
            return OAL_FAIL;
        }
        // 根据字节长度拷贝数据
        switch (param_len) {
            case INI_PARAM_BYTE_ONE:
            case INI_PARAM_BYTE_TWO:
            case INI_PARAM_BYTE_FOUR:
                memcpy_s(param_list + param_num * param_len, param_len, &param, param_len);
                break;
            default:
                oal_print_err("ini_get_cust_item_list param len error:%d!\r\n", param_len);
                return OAL_FAIL;
        }
        pc_token = strtok_s(NULL, pc_sep, &pc_ctx);
        param_num++;
    }
    *list_len = param_num;
    return OAL_SUCC;
}
#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
oal_module_param_string(g_ini_file_name_etc, g_ini_file_name_etc, INI_FILE_PATH_LEN, OAL_S_IRUGO);
#endif

EXPORT_SYMBOL_GPL(get_cust_conf_int32_etc);
EXPORT_SYMBOL_GPL(ini_file_check_conf_update);
EXPORT_SYMBOL_GPL(get_cust_conf_string_etc);
EXPORT_SYMBOL_GPL(ini_get_cust_item_list);
