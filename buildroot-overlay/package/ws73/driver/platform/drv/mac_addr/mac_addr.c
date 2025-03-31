/*
 * Copyright (c) CompanyNameMagicTag 2021-2022. All rights reserved.
 * Description: wal common msg api.
 * Author: Huanghe
 * Create: 2021-08-16
 */
#include "mac_addr.h"
#include "oal_types.h"
#include "efuse_opt.h"
#include "oal_ext_util.h"
#include "soc_osal.h"
#if defined(_PRE_OS_VERSION_LITEOS) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
#else
#include <linux/nl80211.h>
#endif
#include "customize_wifi.h"
#include "ini.h"
#include "customize.h"
#include "oal_debug.h"
#include "osal_types.h"

#if defined(_PRE_OS_VERSION_LITEOS) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
enum nl80211_iftype {
    NL80211_IFTYPE_UNSPECIFIED,
    NL80211_IFTYPE_ADHOC,
    NL80211_IFTYPE_STATION,
    NL80211_IFTYPE_AP,
    NL80211_IFTYPE_AP_VLAN,
    NL80211_IFTYPE_WDS,
    NL80211_IFTYPE_MONITOR,
    NL80211_IFTYPE_MESH_POINT,
    NL80211_IFTYPE_P2P_CLIENT,
    NL80211_IFTYPE_P2P_GO,
    NL80211_IFTYPE_P2P_DEVICE,
    /* keep last */
    NUM_NL80211_IFTYPES,
    NL80211_IFTYPE_MAX = NUM_NL80211_IFTYPES - 1
};
#endif

typedef struct {
    osal_u8   ac_addr[WLAN_MAC_ADDR_LEN];
    osal_u16  us_status;
}dev_addr_stru;

dev_addr_stru g_mac_addr = {0};
dev_addr_stru g_ap_dev_addr = {0};
dev_addr_stru g_p2p_dev_addr = {0};

osal_u32 set_dev_addr(const osal_u8 *pc_addr, osal_u8 mac_len, osal_u8 type)
{
    if (type == NL80211_IFTYPE_AP) {
        if (memcpy_s(g_ap_dev_addr.ac_addr, WLAN_MAC_ADDR_LEN, pc_addr, mac_len) != EOK) {
            return OAL_FAIL;
        } else {
            return OAL_SUCC;
        }
    } else if ((type == NL80211_IFTYPE_MESH_POINT) || (type == NL80211_IFTYPE_P2P_CLIENT) ||
        (type == NL80211_IFTYPE_P2P_GO) || (type == NL80211_IFTYPE_P2P_DEVICE)) {
        if (memcpy_s(g_p2p_dev_addr.ac_addr, WLAN_MAC_ADDR_LEN, pc_addr, mac_len) != EOK) {
            return OAL_FAIL;
        } else {
            return OAL_SUCC;
        }
    } else {
        if (memcpy_s(g_mac_addr.ac_addr, WLAN_MAC_ADDR_LEN, pc_addr, mac_len) != EOK) {
            return OAL_FAIL;
        } else {
            return OAL_SUCC;
        }
    }
}

osal_u8 mac_addr_is_zero(const osal_u8 *mac_addr)
{
    osal_u8 zero_mac_addr[WLAN_MAC_ADDR_LEN] = {0};
    return (memcmp(zero_mac_addr, mac_addr, WLAN_MAC_ADDR_LEN) == 0);
}

osal_u32 mac_addr_check(const osal_u8 *mac_addr)
{
    if ((OSAL_TRUE == mac_addr_is_zero(mac_addr)) || ((mac_addr[0] & 0x1) == 0x1)) {
        return OAL_FAIL;
    }

    return OAL_SUCC;
}

#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#define INPUT_MAC_HEX 16
osal_u32 plat_parse_mac_addr(osal_u8 *cust_param_info, osal_u8 *coe_params,
    osal_u8 *param_num_val, osal_u8 max_idx)
{
    osal_s8       *pc_token;
    osal_char      *pc_ctx;
    osal_s8       *pc_end = ";";
    osal_s8       *pc_sep = ",";
    osal_u8        param_num = 0;
    osal_u8       cust_param[CUS_PARAMS_LEN_MAX];
    osal_u32      ret = OAL_FAIL;

    ret = (osal_u32)memcpy_s(cust_param, CUS_PARAMS_LEN_MAX, cust_param_info, strlen(cust_param_info));
    if (ret != OAL_SUCC) {
        oal_print_err("parse_mac_addr memcpy_s failed\n");
        return ret;
    }

    pc_token = strtok_s(cust_param, pc_end, &pc_ctx);
    if (pc_token == OAL_PTR_NULL) {
        oal_print_err("hwifi_config_sepa_coefficient_from_param read get null value check!\r\n");
        return OAL_PTR_NULL;
    }
    pc_token = strtok_s(pc_token, pc_sep, &pc_ctx);
    /* 获取定制化系数 */
    while (pc_token) {
        if (param_num == max_idx) {
            oal_print_err("hwifi_config_sepa_coefficient_from_param::nv or ini param is too many idx[%d] Max[%d]\n",
                param_num, max_idx);
            return OAL_FAIL;
        }
        *(coe_params + param_num) = (osal_u8)osal_strtoll(pc_token, OAL_PTR_NULL, INPUT_MAC_HEX);
        pc_token = strtok_s(OAL_PTR_NULL, pc_sep, &pc_ctx);
        param_num++;
    }

    *param_num_val = param_num;
    return OAL_SUCC;
}
osal_module_export(plat_parse_mac_addr);
#endif

#ifndef CONFIG_NO_SUPPORT_INI
#define MAC_ADDR_STR_LEN 64
osal_u32 get_mac_from_ini(osal_u8 mac[], osal_u8 mac_len)
{
    osal_u32 ret = OAL_FAIL;
#if defined(_PRE_OS_VERSION_LITEOS) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
    return ret;
#else
    osal_u8 mac_str[MAC_ADDR_STR_LEN] = {0};
    osal_u8 data_len;

    ret = (osal_u32)get_cust_conf_string_etc(INI_MODU_PLAT, "mac_addr", mac_str, MAC_ADDR_STR_LEN);
    if (ret != OAL_SUCC) {
        oal_print_err("ini get mac_addr failed\n");
        return ret;
    }
    if ((plat_parse_mac_addr(mac_str, mac, &data_len, WLAN_MAC_ADDR_LEN) == OAL_SUCC) &&
    (data_len == WLAN_MAC_ADDR_LEN)) {
        return OAL_SUCC;
    }
    return ret;
#endif
}
#endif

osal_u32 get_mac_from_efuse(osal_u8 mac[], osal_u8 mac_len)
{
    osal_u8 index;

    for (index = EFUSE_MAC_NUM; index > 0; index--) {
        if (uapi_efuse_read((EXT_EFUSE_IPV4_MAC_ADDR_01_ID + index - 1), mac, mac_len) == OAL_SUCC) {
            if (mac_addr_is_zero(mac) == OAL_SUCC) {
                return OAL_SUCC;
            }
        }
    }
    return OAL_FAIL;
}
#if defined(_PRE_OS_VERSION_LITEOS) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
struct timeval {
    long    tv_sec;         /* seconds */
    long    tv_usec;        /* and microseconds */
};
static void eth_random_addr(osal_u8 *addr)
{
    struct timeval tv1;
    struct timeval tv2;

    /* 获取随机种子 */
    gettimeofday(&tv1, NULL);

    /* 防止秒级种子为0 */
    tv1.tv_sec += 2; /* 加2 */

    tv2.tv_sec = (td_u32)((td_u32)((td_u64)tv1.tv_sec * tv1.tv_sec) * (td_u64)tv1.tv_usec);
    tv2.tv_usec = (td_u32)((td_u32)((td_u64)tv1.tv_sec * tv1.tv_usec) * (td_u64)tv1.tv_usec);

    /* 生成随机的mac地址 */
    addr[0] = ((td_u32)tv2.tv_sec & 0xff) & 0xfe;
    addr[1] = (td_u32)tv2.tv_usec & 0xff;
    addr[2] = ((td_u32)tv2.tv_sec & 0xff0) >> 4;   /* mac_addr[2]右移4 bit */
    addr[3] = ((td_u32)tv2.tv_usec & 0xff0) >> 4;  /* mac_addr[3]右移4 bit */
    addr[4] = ((td_u32)tv2.tv_sec & 0xff00) >> 8;  /* mac_addr[4]右移8 bit */
    addr[5] = ((td_u32)tv2.tv_usec & 0xff00) >> 8; /* mac_addr[5]右移8 bit */
}
#endif
/*****************************************************************************
 功能描述  : 随机化初始mac地址 让单板启动时携带默认mac
 获取优先级：配置文件-->读efuse-->随机生成
*****************************************************************************/
#define RANDOM_DEFOURT_MAC1 0x00
#define RANDOM_DEFOURT_MAC2 0x73
osal_void init_dev_addr(osal_void)
{
    if (OSAL_TRUE != mac_addr_is_zero(&g_mac_addr.ac_addr[0])) {
        return;
    }

// #ifndef CONFIG_NO_SUPPORT_INI
//     if (get_mac_from_ini(&g_mac_addr.ac_addr[0], WLAN_MAC_ADDR_LEN) == OAL_SUCC) {
//         if (mac_addr_check(&g_mac_addr.ac_addr[0]) == OAL_SUCC) {
//             return;
//         }
//     }
// #endif

    if (get_mac_from_efuse(&g_mac_addr.ac_addr[0], WLAN_MAC_ADDR_LEN) == OAL_SUCC) {
        if (mac_addr_check(&g_mac_addr.ac_addr[0]) == OAL_SUCC) {
            return;
        }
    }

    eth_random_addr(g_mac_addr.ac_addr);
    g_mac_addr.ac_addr[1] = RANDOM_DEFOURT_MAC1; /* 1 地址第2位 0x00     */
    g_mac_addr.ac_addr[2] = RANDOM_DEFOURT_MAC2; /* 2 地址第3位 0x73 */
    g_mac_addr.us_status = 0;
}

typedef enum {
    ADDR_IDX_STA = 0,
    ADDR_IDX_BLE = 1,
    ADDR_IDX_AP = 2,
    ADDR_IDX_P2P = 3,
    ADDR_IDX_SLE = 4,
    ADDR_IDX_BUTT
}addr_idx;

addr_idx get_dev_addr_idx(osal_u8 type)
{
    addr_idx addr_idx = ADDR_IDX_BUTT;

    switch (type) {
        case NL80211_IFTYPE_STATION:
            addr_idx = ADDR_IDX_STA;
            break;
        case NL80211_IFTYPE_AP:
            addr_idx = ADDR_IDX_AP;
            break;
        case NL80211_IFTYPE_P2P_CLIENT:
        case NL80211_IFTYPE_P2P_GO:
        case NL80211_IFTYPE_P2P_DEVICE:
        case NL80211_IFTYPE_MESH_POINT:
            addr_idx = ADDR_IDX_P2P;
            break;
        default:
            oal_print_err("get_dev_addr_idx iftype error!!!\n");
            break;
    }

    return addr_idx;
}

mac_derivation_ptr g_mac_derivation_ptr = OSAL_NULL;

osal_void set_mac_derivation_ptr(mac_derivation_ptr ptr)
{
    g_mac_derivation_ptr = ptr;
}
osal_module_export(set_mac_derivation_ptr);
#define BIT_8_MASK 0x100
#define LOW_8_BITS_MASK 0xff
#define HIGH_MAC_MASK 0xfe

osal_void mac_carry(osal_u16 *mac_low, osal_u16 *mac_high)
{
    (*mac_high) += (((*mac_low) & BIT_8_MASK)  >> SIZE_8_BITS);
    (*mac_low) = (*mac_low) & LOW_8_BITS_MASK;
}
#define WIFI_MAC_ADDR_DERIVE_BIT 4
#define BLE_MAC_ADDR_DERIVE_BIT 5
#define DERIVE_START_BYTE 3
osal_u32 get_derived_mac(osal_u8 mac[], osal_u8 mac_len, addr_idx idx, osal_u8 derive_bit)
{
    osal_u16 us_addr[WLAN_MAC_ADDR_LEN];
    osal_u32 tmp;

    for (tmp = 0; tmp < WLAN_MAC_ADDR_LEN; tmp++) {
        us_addr[tmp] = (osal_u16)g_mac_addr.ac_addr[tmp];
    }
    us_addr[derive_bit] += idx;  // 4 字节增加idx
    for (tmp = derive_bit; tmp > DERIVE_START_BYTE; tmp--) {
        mac_carry(&(us_addr[tmp]), &(us_addr[tmp - 1]));
    }
    if (us_addr[DERIVE_START_BYTE] > LOW_8_BITS_MASK) {
        us_addr[DERIVE_START_BYTE] = 0;
    }
    if (idx != ADDR_IDX_BLE) {
        us_addr[0] &= HIGH_MAC_MASK;
    }
    for (tmp = 0; tmp < WLAN_MAC_ADDR_LEN; tmp++) {
        mac[tmp] = (osal_u8)us_addr[tmp];
    }
    return OAL_SUCC;
}

osal_u32 get_dev_addr(osal_u8 *pc_addr, osal_u8 addr_len, osal_u8 type)
{
    osal_u32 tmp;
    osal_u8  zero_mac[WLAN_MAC_ADDR_LEN] = {0};
    addr_idx addr_idx;
    if (pc_addr == NULL) {
        oal_print_err("pc_addr == NULL)!!!\n");
        return OAL_FAIL;
    }

    if ((type == NL80211_IFTYPE_AP) && (memcmp(g_ap_dev_addr.ac_addr, zero_mac, WLAN_MAC_ADDR_LEN) != 0)) {
        for (tmp = 0; tmp < WLAN_MAC_ADDR_LEN; tmp++) {
            pc_addr[tmp] = g_ap_dev_addr.ac_addr[tmp];
        }
        return OAL_SUCC;
    }
    if (((type == NL80211_IFTYPE_MESH_POINT) || (type == NL80211_IFTYPE_P2P_CLIENT) ||
        (type == NL80211_IFTYPE_P2P_GO) || (type == NL80211_IFTYPE_P2P_DEVICE)) &&
        (memcmp(g_p2p_dev_addr.ac_addr, zero_mac, WLAN_MAC_ADDR_LEN) != 0)) {
        for (tmp = 0; tmp < WLAN_MAC_ADDR_LEN; tmp++) {
            pc_addr[tmp] = g_p2p_dev_addr.ac_addr[tmp];
        }
        return OAL_SUCC;
    }
    if (type == IFTYPE_BLE) {
        for (tmp = 0; tmp < WLAN_MAC_ADDR_LEN; tmp++) {
            pc_addr[tmp] = g_mac_addr.ac_addr[tmp];
        }
        get_derived_mac(pc_addr, addr_len, ADDR_IDX_BLE, BLE_MAC_ADDR_DERIVE_BIT);
        return OAL_SUCC;
    }
    addr_idx = get_dev_addr_idx(type);
    if (addr_idx >= ADDR_IDX_BUTT) {
        oal_print_err("type=%d,error!!!\n", type);
        return OAL_FAIL;
    }
    if (g_mac_derivation_ptr != NULL) {
        return g_mac_derivation_ptr(g_mac_addr.ac_addr, WLAN_MAC_ADDR_LEN, type, pc_addr, WLAN_MAC_ADDR_LEN);
    } else {
        return get_derived_mac(pc_addr, addr_len, addr_idx, WIFI_MAC_ADDR_DERIVE_BIT);
    }
}

osal_module_export(get_dev_addr);
osal_module_export(set_dev_addr);