/*
 * Copyright (c) CompanyNameMagicTag 2022-2023. All rights reserved.
 * Description: diag offline log utils for os: linux
 * This file should be changed only infrequently and with great care.
 */

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "zdiag_local_log.h"
#include <linux/path.h>
#include <linux/namei.h>
#include "oal_debug.h"
#include "soc_osal.h"
#include "osal_types.h"
#include "osal_errno.h"
#include "oal_kernel_file.h"
#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include "ini.h"
#endif
#include "diag_cmd_connect.h"
#include "diag_cmd_filter.h"
#include "zdiag_adapt_os.h"
#include "zdiag_tx_proc.h"
#include "zdiag_lib_dbg.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#if defined(CONFIG_DIAG_SUPPORT_LOCAL_LOG)

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
#define OAM_TIME_BUF_LEN        30 /* time buf len */

#define OAM_GET_CFG_FAIL        0
#define OAM_FILE_MAX_CNT        20 /* save file count max num */
#define OAM_FILE_MAX_NAME_LEN   128 /* save file name max len */

// Use byte unit, avoid cases: 1024 (10) * 1024 (10) * 4096 (12) = (32), overflow
// if Use 1K unit, file_max_size = OAM_FILE_K_SIZE * OAM_FILE_SIZE_K_UNIT;
#define OAM_FILE_SIZE_MIN       (32) /* save file name max len */
#define OAM_FILE_MAX_SIZE       (1024 * 1024 * 2) /* save file name max len */
#define OAM_FILE_SIZE_K_UNIT    (1024) /* unit 1 K */

#define OAM_FILE_CNT_DEFAULT    5 /* Default number of files cnt 5 */
#define OAM_FILE_SIZE_DEFAULT   (128 * OAM_FILE_SIZE_K_UNIT) /* Default File Size 128K */

#define OAM_LOG_LEVEL_MIN       (0) /* log level min */
#define OAM_LOG_LEVEL_DEFAULT   (PLAT_LOG_ERR) /* log level default */
#define OAM_LOG_LEVEL_MAX       (8) /* log level max */

#define KLIB_MODE_T (S_IFREG | S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR)

typedef struct {
    char file_dir[OAM_FILE_MAX_NAME_LEN]; /* local log dir */
} file_manage_comm_stru;

typedef struct {
    td_u8 file_max_cnt;
    td_u8 file_cur_pos;
    td_u16 resv;
    td_s32 file_max_size;
    char file_dir[OAM_FILE_MAX_NAME_LEN]; /* local log dir */
    char file_path[OAM_FILE_MAX_CNT][OAM_FILE_MAX_NAME_LEN];
} oam_file_manage_stru;

static oam_file_manage_stru g_file_mng; // refactored based on g_diag_local_log.filemng

td_u8 g_local_log_dbg_level = PLAT_LOG_ERR;

/*****************************************************************************
  3 函数实现
*****************************************************************************/
static void diag_log_ini_head(diag_local_file_head *file_head)
{
    file_head->start_flag  = 0xaabbccdd;
    file_head->version     = 0;
    file_head->file_type   = 0x01; /* msg:0x01, cmd:0x02 */
    file_head->msg_version = 0xfb; /* Chip Version, Need to consult */
}

/* Description: get sys time from klib rtc
 * ptr: osal_rtc_time(klib ds) vs struct tm *(uapi)
 */
static void osal_klib_get_sys_time(osal_rtc_time* ptr)
{
    osal_timeval tv; // same as (linux)struct timeval (liteos)osal_timeval

    // no need to check osal_rtc_time* ptr whether invalid, caller guarantee ptr valid.
    osal_gettimeofday(&tv); // do_gettimeofday included
    osal_rtc_time_to_tm(tv.tv_sec, ptr); // rtc_time_to_tm included
    oam_warn("sec:%ld-UTC:%d-%d-%d %d:%d:%d\n", tv.tv_sec,
        ptr->tm_year + 1900, ptr->tm_mon + 1, ptr->tm_mday, ptr->tm_hour, ptr->tm_min, ptr->tm_sec); /* year + 1900 */
}

/* Description: get system time
 *  refactored based on oam_get_sys_time
 */
void zdiag_local_log_get_sys_time(char *buf, unsigned int buf_len)
{
    osal_rtc_time tm_rtc; // struct rtc_time(linux) included
    int ret;

    /* get local time */
    osal_klib_get_sys_time(&tm_rtc);
    ret = sprintf_s(buf, buf_len, "_%4d_%02d_%02d_%02d_%02d_%02d",
                    tm_rtc.tm_year + 1900,   /* year plus 1900 */
                    tm_rtc.tm_mon + 1,       /* mon  */
                    tm_rtc.tm_mday,          /* day  */
                    tm_rtc.tm_hour,          /* hour */
                    tm_rtc.tm_min,           /* min  */
                    tm_rtc.tm_sec);          /* sec  */
    if (ret <= OSAL_SUCCESS) {
        oam_error("sprintf_s failed ret = %d", ret);
    }
    return;
}

static int osal_klib_mkdir(const char *name, umode_t mode)
{
    struct dentry *dentry;
    struct path path;
    int error;
    unsigned int lookup_flags = LOOKUP_DIRECTORY;

    dentry = kern_path_create(AT_FDCWD, name, &path, lookup_flags);
    if (IS_ERR(dentry)) {
        oam_error(" kern_path_create: path_name[%s] fail!! ret=%d\n", name, (int)IS_ERR(dentry));
        return PTR_ERR(dentry);
    }

    if (!IS_POSIXACL(path.dentry->d_inode)) {
        oam_error(" dir [%s] mode=%d\n", name, (int)mode);
        mode = (umode_t)((unsigned int)mode & (~(unsigned int)current_umask()));
        oam_error(" dir [%s] mode=%d\n", name, (int)mode);
    }

    error = security_path_mkdir(&path, dentry, mode);
    if (!error) {
        oam_error(" dir [%s] error=%d\n", name, (int)error);
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE <= KERNEL_VERSION(5, 15, 0))
        error = vfs_mkdir(path.dentry->d_inode, dentry, mode);
#else
        struct mnt_idmap *idmap = &nop_mnt_idmap;
        error = vfs_mkdir(idmap, path.dentry->d_inode, dentry, mode);
#endif
        oam_error(" dir [%s] error=%d\n", name, (int)error);
    }
    done_path_create(&path, dentry); // no need use LOOKUP_REVAL
    oam_info(" dir [%s] error=%d\n", name, (int)error);
    return error;
}

static void *diag_local_log_file_create(td_u16 file_pos)
{
    int ret; // unsigned long f_ret;
    void *fp; // vs: FILE *file = NULL;
    char *file_path;

    file_path = g_file_mng.file_path[file_pos];

    memset_s(file_path, OAM_FILE_MAX_NAME_LEN, 0, OAM_FILE_MAX_NAME_LEN);
    ret = sprintf_s(file_path,
        OAM_FILE_MAX_NAME_LEN, "%s/log_%02d.bin", g_file_mng.file_dir, file_pos);
    if (ret < 0) {
        oam_error("sprintf_s fail!\n");
        return NULL;
    }

    fp = osal_klib_fopen(file_path, O_CREAT | O_RDWR, KLIB_MODE_T);
    if (fp == NULL) {
        oam_error("filp_open [%s] failed! fp=%p! \n", file_path, fp);
        return NULL;
    }
    oam_info("file_pos = %d, open %s success\n", file_pos, file_path);

    return fp;
}

static void *diag_local_log_file_alter(td_u16 file_pos)
{
    void *fp; // vs: FILE *file = NULL;
    int file_len;
    const char *file_path;
    file_path = g_file_mng.file_path[file_pos];

    if (file_path[0] == '\0') {
        return diag_local_log_file_create(file_pos);
    } else {
        fp = osal_klib_fopen(file_path, O_APPEND | O_RDWR, KLIB_MODE_T);
        if (fp == NULL) {
            oam_error("filp_open [%s] failed! fp=%p! \n", file_path, fp);
            return NULL;
        }
        oam_info("filp_open [%s] ok! fp=%p! \n", file_path, fp);

        // get size of file
        file_len = osal_klib_fseek(0, SEEK_END, fp);
        oam_info("file[%s], file_len: %d\n", file_path, file_len);

        // move fp->f_pos to the head of file
        osal_klib_fseek(0, SEEK_SET, fp);
        oam_info("file[%s], close file before unlink.\n", file_path);
        osal_klib_fclose(fp); // first close, then unlink
        fp = NULL;
        if (file_len < 0) {
            oam_error("file[%s] size[%d]\n", file_path, file_len);
            return NULL;
        } else if (file_len > 0) {
            oam_info("unlink file[%s] size[%d]\n", file_path, file_len);
            osal_klib_unlink(file_path);
        }
    }

    // internel call osal_klib_fopen(file_path, O_CREAT | O_RDWR, KLIB_MODE_T)
    fp = diag_local_log_file_create(file_pos);
    if (fp == NULL) {
        oam_error("filp_open [%s] failed! fp=%p! \n", g_file_mng.file_path[file_pos], fp);
        return NULL;
    }
    file_path = g_file_mng.file_path[file_pos];
    oam_info("file_pos = %d, open %s success\n", file_pos, file_path);

    return fp;
}

void zdiag_local_file_add_header(struct file *fp)
{
    int ret;
    diag_local_file_head file_head;

    diag_log_ini_head(&file_head);
    ret = osal_klib_fwrite((const char *)&file_head, sizeof(diag_local_file_head), (void *)fp); // add local file head
    if (ret <= 0) {
        oam_error("diag log wr head fail: %d\n", ret);
    }
}

osal_s32 zdiag_local_log_get_stat(osal_char *file_name)
{
    osal_s32 ret = OAL_SUCC;
    struct kstat file_stat = {0};
#if defined(LINUX_VERSION_CODE) && defined(KERNEL_VERSION) && (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
    struct path file_path;
    ret = user_path_at_empty(AT_FDCWD, file_name, LOOKUP_FOLLOW, &file_path, NULL);
    if (ret != 0) {
        return ret;
    }
    vfs_getattr(&file_path, &file_stat, STATX_BASIC_STATS, AT_NO_AUTOMOUNT);
#elif (defined(LINUX_VERSION_CODE) && defined(KERNEL_VERSION) && (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 9, 0)))
    ret = vfs_stat(file_name, &file_stat);
#else
    uapi_unused(file_stat);
#endif
    return ret;
}

void zdiag_local_log_scan(void)
{
    int ret;
    td_u8 i;
    char *file_path;

    for (i = 0; i < g_file_mng.file_max_cnt; i++) {
        file_path = g_file_mng.file_path[i];
        ret = sprintf_s(file_path,
            OAM_FILE_MAX_NAME_LEN, "%s/log_%02d.bin", g_file_mng.file_dir, i);
        if ((ret < 0) || (zdiag_local_log_get_stat(file_path) != 0)) {
            g_file_mng.file_cur_pos = i;
            memset_s(file_path, OAM_FILE_MAX_NAME_LEN, 0, OAM_FILE_MAX_NAME_LEN);
            break;
        }
    }
}

static void zdiag_local_log_dir_init(void)
{
    int ret;
    char *dir_name = NULL;
    td_u8 file_num;
    td_u8 i;
    void *fp;
    umode_t dir_mode = KLIB_MODE_T;

    g_file_mng.file_cur_pos = 0;
    file_num = g_file_mng.file_max_cnt; // parsed from ini file
    for (i = 0; i < file_num; ++i) {
        g_file_mng.file_path[i][0] = '\0';
    }

    dir_name = g_file_mng.file_dir; // simplify dir_name operations
    oam_info("dir_name: %s\n", dir_name);
    ret = osal_klib_mkdir(dir_name, dir_mode);
    if (ret != 0) {
        oam_error("open dir [%s] fail!! ret=%d\n", dir_name, ret); // need check dir opmod
    }
    /* scan */
    zdiag_local_log_scan();

    fp = diag_local_log_file_create(g_file_mng.file_cur_pos);
    if (fp == NULL) {
        oam_error("null fp param. \r\n");
        return;
    }
    zdiag_local_file_add_header(fp);
}

#ifdef CONFIG_NO_SUPPORT_INI
static td_s32 zdiag_local_log_cfg_init(void)
{
    osal_s32 ret;
    osal_s32 ini_value = (enum DIAG_LOG_MODE)WSCFG_PLAT_DIAG_LOG_OUT;
    if (ini_value != DIAG_LOG_TO_FILE) {
        return EXT_ERR_DIAG_NOT_SUPPORT;
    }

    ret = sprintf_s(g_file_mng.file_dir, OAM_FILE_MAX_NAME_LEN, "%s", CONFIG_INI_ZDIAG_FILE_DIR);
    if (ret < 0) {
        return EXT_ERR_DIAG_INVALID_PARAMETER;
    }

    g_file_mng.file_max_cnt = CONFIG_INI_ZDIAG_FILE_MAX_CNT;
    g_file_mng.file_max_size = CONFIG_INI_ZDIAG_FILE_MAX_SIZE;

    return OSAL_SUCCESS;
}
#else
static td_s32 zdiag_local_log_cfg_init(void)
{
    int ret = OSAL_SUCCESS;
    osal_s32 ini_value = DIAG_LOG_TO_FILE;
    char log_dir[OAM_FILE_MAX_NAME_LEN] = { '\0' };

    /* Get ini custom config datas, and Check */
    /* 1, log enable */
    ret = get_cust_conf_int32_etc(INI_MODU_PLAT, OAM_LOG_ENABLE, &ini_value);
    if (ret != INI_SUCC) {
        oam_warn("get ini diag_log_enable res: %d\n", ret);
        ret = EXT_ERR_DIAG_NO_INITILIZATION;
    }
    if (ini_value != DIAG_LOG_TO_FILE) {
        return EXT_ERR_DIAG_NOT_SUPPORT;
    }

    /* 2, dir str */
    if (get_cust_conf_string_etc(INI_MODU_PLAT, OAM_LOG_PATH, log_dir, OAM_FILE_MAX_NAME_LEN) != INI_SUCC) {
        oam_error("get log_dir custom config failed, use default dir\n");
        log_dir[0] = '\0';
        ret = sprintf_s(g_file_mng.file_dir, OAM_FILE_MAX_NAME_LEN, "%s", OAM_FILE_PATH);
    }
    if (strlen(log_dir) != 0) {
        ret = sprintf_s(g_file_mng.file_dir, OAM_FILE_MAX_NAME_LEN, "%s", log_dir);
    }
    if (ret < 0) {
        return EXT_ERR_DIAG_INVALID_PARAMETER;
    }

    /* 3, log file number count */
    ini_value = OAM_GET_CFG_FAIL;
    ret = get_cust_conf_int32_etc(INI_MODU_PLAT, OAM_LOG_COUNT, &ini_value);
    if (ret != INI_SUCC || ini_value <= OAM_GET_CFG_FAIL || ini_value > OAM_FILE_MAX_CNT) {
        oam_error("ini]file_max_count get fail, use default %d\n", OAM_FILE_CNT_DEFAULT);
        ini_value = OAM_FILE_CNT_DEFAULT;
    }
    g_file_mng.file_max_cnt = (td_u8)ini_value;

    /* 4, log file max size */
    ini_value = OAM_FILE_SIZE_DEFAULT;
    ret = get_cust_conf_int32_etc(INI_MODU_PLAT, OAM_LOG_SIZE, &ini_value);
    if (ret != INI_SUCC || ini_value <= OAM_FILE_SIZE_MIN || ini_value > OAM_FILE_MAX_SIZE) {
        oam_error("ini]file_max_size get fail, use default %d\n", OAM_FILE_SIZE_DEFAULT);
        ini_value = OAM_FILE_SIZE_DEFAULT;
    }
    g_file_mng.file_max_size = ini_value;

    /* 5, log level */
    ret = get_cust_conf_int32_etc(INI_MODU_PLAT, OAM_LOG_LEVEL, &ini_value);
    if (ret != INI_SUCC || ini_value <= OAM_LOG_LEVEL_MIN || ini_value > OAM_LOG_LEVEL_MAX) {
        oam_warn("get ini diag_log_level res: %d\n", ret);
        ini_value = OAM_LOG_LEVEL_DEFAULT;
    }
    g_local_log_dbg_level = ini_value;

    oam_debug("diag log dir[%s],cnt:%d,size:%d\n", g_file_mng.file_dir,
        g_file_mng.file_max_cnt, g_file_mng.file_max_size);

    return EXT_SUCCESS;
}
#endif

void zdiag_local_log_cb_for_dev_bsp_ready(void)
{
    // get local log stat
    if (zdiag_log_mode_get() == DIAG_LOG_TO_FILE) {
        // when local log enable, enable device zdiag filter
        // when local log disable, no need to diasable host and device zdiag filter
        diag_sync_cfg_cmd_to_dev((td_u32)g_local_log_dbg_level);
    }
}

void zdiag_local_log_init(void)
{
    td_s32 ret;
    ret = zdiag_local_log_cfg_init();
    if (ret != EXT_SUCCESS) {
        oal_print_err("zdiag local log, init res: %d!\n", ret);
    }

    zdiag_local_log_dir_init();

    if (zdiag_log_mode_get() == (td_u8)DIAG_LOG_TO_FILE) {
        // when local log enable, disable diag disconnect cmd when use both hso-conn and local-log
        diag_dis_cmd_support_or_not(TD_FALSE);
        // enable host zdiag filter, device filter leave to be sent when dev bsp ready
        diag_enable_level_switch((td_u32)g_local_log_dbg_level);
    }
}

td_s32 osal_klib_unlink(const char *file_path)
{
    td_s32 ret = OSAL_SUCCESS;
    struct file *fp = NULL;
    struct dentry *dentry = NULL;

    if (file_path == NULL) {
        oam_error("file_path NULL!\n");
        return OSAL_FAILURE;
    }

    fp = osal_klib_fopen(file_path, O_APPEND | O_RDWR, KLIB_MODE_T);
    if (OAL_IS_ERR_OR_NULL(fp)) {
        oam_error("filp_open[%s] fail!!, fp=%p\n", file_path, fp);
        fp = NULL;
        return EXT_ERR_FAILURE;
    }
    dentry = fp->f_path.dentry;
    if (OAL_IS_ERR_OR_NULL(dentry)) {
        oam_error("osal_klib_unlink[%s] fp[%p] dentry[%p] err\n", file_path, fp, dentry);
        return EXT_ERR_FAILURE;
    }
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE > KERNEL_VERSION(5, 15, 0))
    // ret = vfs_unlink(&init_user_ns, dentry->d_parent->d_inode, dentry, NULL);
    struct mnt_idmap *idmap = &nop_mnt_idmap;
    ret = vfs_unlink(idmap, dentry->d_parent->d_inode, dentry, NULL);
#elif defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 13, 0))
    ret = vfs_unlink(dentry->d_parent->d_inode, dentry, NULL);
#else
    ret = vfs_unlink(dentry->d_parent->d_inode, dentry);
#endif
    if (ret != OSAL_SUCCESS) {
        oam_error("vfs_unlink fail, ret=0x%x\n", ret);
    }
    return ret;
}

/* before called this, make sure that zdiag_log_mode_get() returns DIAG_LOG_TO_FILE */
td_u32 zdiag_offline_log_output(td_u8 *hcc_buf, td_u16 len)
{
    void *fp;
    osal_s32 file_len = 0;
    zdiag_ctx_stru *ctx = zdiag_get_ctx();
    td_u8 file_pos = g_file_mng.file_cur_pos;
    const char *file_path = g_file_mng.file_path[file_pos];

    if (hcc_buf == NULL || file_path[0] == '\0') {
        return EXT_ERR_PTR_NULL;
    }

    // here need append mode to add log at log file tail
    fp = osal_klib_fopen(file_path, O_APPEND | O_RDWR, KLIB_MODE_T);
    if (fp == NULL) {
        oam_error("file[%s] fp is null. \r\n", file_path);
        return EXT_ERR_DIAG_OBJ_NOT_FOUND;
    }

    file_len = osal_klib_fseek(0, SEEK_END, fp);
    if (file_len <= 0) {
        osal_klib_fclose(fp);
        return EXT_ERR_FS_IO_ERROR;
    }
    if (((osal_u32)file_len + len) > (unsigned long)g_file_mng.file_max_size) {
        // alter file when cur log file full
        oam_warn("exp wr: %u, cur len: %lu, exceed!\n", ((osal_u32)file_len + len),
            (unsigned long)g_file_mng.file_max_size);
        osal_klib_fclose(fp);
        g_file_mng.file_cur_pos++;
        if (g_file_mng.file_cur_pos >= g_file_mng.file_max_cnt) {
            g_file_mng.file_cur_pos = 0;
        }
        file_pos = g_file_mng.file_cur_pos;
        fp = diag_local_log_file_alter(file_pos);
        if (fp == NULL) {
            oam_error("null fp param. \r\n");
            return EXT_ERR_FS_NO_DEVICE;
        }
        file_path = g_file_mng.file_path[file_pos];
        oam_info("new log file_path: %s \n", file_path);
        zdiag_local_file_add_header(fp);
    }

    if (osal_klib_fwrite(hcc_buf, len, fp) <= 0) {
        osal_klib_fclose(fp);
        return EXT_ERR_FS_IO_ERROR;
    }

    osal_klib_fclose(fp);
    ctx->tx_succ_cnt++;
    return EXT_SUCCESS;
}

#define ZDIAG_TEST_LOCAL_LOG_OUTPUT_LEN 512
#define ZDIAG_TEST_LOCAL_LOG_CHAR_COUNT 16

void test_zdiag_local_log_output(void)
{
    char buf[ZDIAG_TEST_LOCAL_LOG_OUTPUT_LEN];
    int i;
    td_u32 ret;

    for (i = 0; i < ZDIAG_TEST_LOCAL_LOG_OUTPUT_LEN; ++i) {
        buf[i] = 'a' + i / ZDIAG_TEST_LOCAL_LOG_CHAR_COUNT;
    }

    ret = zdiag_offline_log_output(buf, sizeof(buf));
    if (ret == EXT_ERR_FS_IO_ERROR) {
        oam_error("diag offline write failed! \n");
    } else if (ret == EXT_ERR_PTR_NULL) {
        oam_error("diag offline null ptr! \n");
    } else if (ret != EXT_SUCCESS) {
        oam_error("diag offline log ret:0x%x \n", ret);
    }
}

#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE > KERNEL_VERSION(5, 10, 0))
MODULE_IMPORT_NS(VFS_internal_I_am_really_a_filesystem_and_am_NOT_a_driver);
#endif
#endif
#endif

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif
