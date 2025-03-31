/*
 * Copyright (c) CompanyNameMagicTag 2021-2021. All rights reserved.
 * Description: wal ccpriv api.
 * Create: 2021-07-19
 */

#ifndef __WAL_LINUX_CCPRIV_H__
#define __WAL_LINUX_CCPRIV_H__

#include "oal_ext_if.h"
#include "wlan_types_common.h"
#include "wlan_spec.h"
#include "wal_linux_ioctl.h"
#include "wal_ccpriv_common.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_WAL_LINUX_CCPRIV_H
#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#ifdef _PRE_WLAN_CFGID_DEBUG
extern OAL_CONST wal_ccpriv_cmd_entry_stru g_ast_ccpriv_cmd_debug_etc[];
#endif

extern OAL_CONST wal_ioctl_mode_map_stru g_ast_mode_map_etc[];

#define WAL_HIPRIV_CMD_MAX_LEN       (WLAN_HMEM_LOCAL_SIZE2 - 4)     /* 私有配置命令字符串最大长度，对应本地内存池一级大小 */

#define WAL_HIPRIV_CMD_NAME_MAX_LEN     80                          /* 字符串中每个单词的最大长度(原20) */
#define WAL_HIPRIV_CMD_VALUE_MAX_LEN    10                          /* 字符串中某个对应变量取值的最大位数 */

#define WAL_HIPRIV_PROC_ENTRY_NAME   "ccpriv"

#define WAL_HIPRIV_HT_MCS_MIN           0
#define WAL_HIPRIV_HT_MCS_MAX           32
#define WAL_HIPRIV_VHT_MCS_MIN          0
#define WAL_HIPRIV_VHT_MCS_MAX          11
#define WAL_HIPRIV_HE_MCS_MIN          0
#define WAL_HIPRIV_HE_MCS_MAX          11
#define WAL_HIPRIV_NSS_MIN              1
#define WAL_HIPRIV_NSS_MAX              4
#define WAL_HIPRIV_CH_NUM               4

#ifdef _PRE_WLAN_FEATURE_ALWAYS_TX
#define WAL_HIPRIV_MAX_DUTY_RATIO       9
#define WAL_HIPRIV_MAX_LONG_SLOT        31
#define WAL_HIPRIV_SHORT_SLOT           9
#define WAL_HIPRIV_MAX_BACK_OFF_TIME    255
#define WAL_HIPRIV_SET_LONG_SLOT_ORDER " 32 0x4001006C 0x019F503C"
#define WAL_HIPRIV_SET_SHORT_SLOT_ORDER " 32 0x4001006C 0x0149503C"
#endif

#define WAL_HIPRIV_BOOL_NIM             0
#define WAL_HIPRIV_BOOL_MAX             1
#define WAL_HIPRIV_FREQ_SKEW_ARG_NUM    8
#define WAL_PHY_DEBUG_TEST_WORD_CNT     5           /* trailer上报个数 */

#define WAL_HIPRIV_MS_TO_S                   1000   /* ms和s之间倍数差 */
#define WAL_HIPRIV_KEEPALIVE_INTERVAL_MIN    5000   /* 受默认老化计数器出发时间所限制 */
#define WAL_HIPRIV_KEEPALIVE_INTERVAL_MAX    0xffff /* timer间隔时间限制所致(oal_uin16) */

#define WAL_IWPRIV_CAP_NUM              14          /* ???vap????????? */
#define WAL_IWPRIV_IGMP_MIN_LEN         50

/*****************************************************************************
 函 数 名  : wal_ccpriv_unreg_free_netdev
 功能描述  : 创建net_dev失败释放内存
*****************************************************************************/
static INLINE__ osal_void wal_ccpriv_unreg_free_netdev(oal_net_device_stru *net_dev, oal_wireless_dev_stru *wdev)
{
    /* unregister will call dev destructor to free net_dev && need before wdev free */
    oal_net_unregister_netdev(net_dev);
    oal_mem_free(wdev, OAL_FALSE);
}

osal_u32  wal_ccpriv_get_mac_addr(osal_s8 *pc_param, osal_u8 auc_mac_addr[], osal_u32 *pul_total_offset);
osal_u32 uapi_ccpriv_ampdu_tx_on(oal_net_device_stru *net_dev, osal_s8 *param);
osal_u32  uapi_ccpriv_del_vap_etc(oal_net_device_stru *net_dev, osal_s8 *pc_param);

#ifdef _PRE_WLAN_CFGID_DEBUG
osal_u32 wal_ccpriv_get_debug_cmd_size_etc(osal_void);
#endif

#ifdef _PRE_WLAN_FEATURE_M2S
osal_u32  uapi_ccpriv_set_m2s_switch(oal_net_device_stru *net_dev, osal_s8 *pc_param);
#endif

osal_u32 uapi_ccpriv_set_mode(oal_net_device_stru *net_dev, osal_s8 *param);
osal_void wal_register_debug_cmd(wal_ccpriv_debug_cmd_func cb);
osal_void wal_unregister_debug_cmd(osal_void);

extern osal_void set_wlan_flag_close(osal_void);
extern osal_u32 plat_parse_mac_addr(osal_u8 *cust_param_info, osal_u8 *coe_params,
    osal_u8 *param_num_val, osal_u8 max_idx);
extern osal_u32 efuse_write_dev_addr(osal_u8 *pc_addr, osal_u8 addr_len);
extern osal_u32 efuse_write_pid_vid(osal_u16 pid, osal_u16 vid);
osal_void wlan_set_dfr_recovery_flag(osal_u8 dfr_flag);
osal_u32 uapi_ccpriv_tx_power(oal_net_device_stru *net_dev, osal_s8 *param, osal_u32 msg_id);


#ifdef CONTROLLER_CUSTOMIZATION
ssize_t uapi_ccpriv_sys_write(struct kobject *kobj, struct kobj_attribute *attr, const char *pc_buffer, size_t count);
ssize_t uapi_ccpriv_sys_read(struct kobject *kobj, struct kobj_attribute *attr, char *pc_buffer);
#else
oal_ssize_t uapi_ccpriv_sys_write(oal_device_stru *dev, oal_device_attribute_stru *attr,
    const char *pc_buffer, oal_size_t count);
oal_ssize_t uapi_ccpriv_sys_read(oal_device_stru *dev, oal_device_attribute_stru *attr, char *pc_buffer);
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
