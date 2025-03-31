/*
 * Copyright (c) CompanyNameMagicTag 2021-2021. All rights reserved.
 * Description: wal 11d api.
 * Create: 2021-07-16
 */

#include "wal_linux_11d.h"
#include "mac_vap_ext.h"
#include "mac_resource_ext.h"
#include "mac_regdomain.h"
#include "wal_regdb.h"
#include "wal_linux_util.h"
#include "wal_linux_cfg80211.h"
#include "wal_linux_ioctl.h"

#include "hcc_host_if.h"
#include "plat_cali.h"
#include "plat_pm_wlan.h"
#include "plat_firmware.h"

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
#include "oal_netbuf.h"
#include "soc_customize_wifi.h"
#endif /* #ifdef _PRE_PLAT_FEATURE_CUSTOMIZE */

#include "wlan_msg.h"
#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_WAL_LINUX_11D_C
#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#ifdef _PRE_WLAN_FEATURE_11D
typedef struct {
    osal_s8                    *pc_country;          /* 国家字符串 */
    mac_dfs_domain_enum_uint8    dfs_domain;       /* DFS 雷达标准 */
} wal_dfs_domain_entry_stru;

STATIC OAL_CONST wal_dfs_domain_entry_stru g_ast_dfs_domain_table_etc[] = {
    {"AE", MAC_DFS_DOMAIN_ETSI},
    {"AL", MAC_DFS_DOMAIN_NULL},
    {"AM", MAC_DFS_DOMAIN_ETSI},
    {"AN", MAC_DFS_DOMAIN_ETSI},
    {"AR", MAC_DFS_DOMAIN_FCC},
    {"AT", MAC_DFS_DOMAIN_ETSI},
    {"AU", MAC_DFS_DOMAIN_FCC},
    {"AZ", MAC_DFS_DOMAIN_ETSI},
    {"BA", MAC_DFS_DOMAIN_ETSI},
    {"BE", MAC_DFS_DOMAIN_ETSI},
    {"BG", MAC_DFS_DOMAIN_ETSI},
    {"BH", MAC_DFS_DOMAIN_ETSI},
    {"BL", MAC_DFS_DOMAIN_NULL},
    {"BN", MAC_DFS_DOMAIN_ETSI},
    {"BO", MAC_DFS_DOMAIN_ETSI},
    {"BR", MAC_DFS_DOMAIN_FCC},
    {"BY", MAC_DFS_DOMAIN_ETSI},
    {"BZ", MAC_DFS_DOMAIN_ETSI},
    {"CA", MAC_DFS_DOMAIN_FCC},
    {"CH", MAC_DFS_DOMAIN_ETSI},
    {"CL", MAC_DFS_DOMAIN_NULL},
    {"CN", MAC_DFS_DOMAIN_NULL},
    {"CO", MAC_DFS_DOMAIN_FCC},
    {"CR", MAC_DFS_DOMAIN_FCC},
    {"CS", MAC_DFS_DOMAIN_ETSI},
    {"CY", MAC_DFS_DOMAIN_ETSI},
    {"CZ", MAC_DFS_DOMAIN_ETSI},
    {"DE", MAC_DFS_DOMAIN_ETSI},
    {"DK", MAC_DFS_DOMAIN_ETSI},
    {"DO", MAC_DFS_DOMAIN_FCC},
    {"DZ", MAC_DFS_DOMAIN_NULL},
    {"EC", MAC_DFS_DOMAIN_FCC},
    {"EE", MAC_DFS_DOMAIN_ETSI},
    {"EG", MAC_DFS_DOMAIN_ETSI},
    {"ES", MAC_DFS_DOMAIN_ETSI},
    {"FI", MAC_DFS_DOMAIN_ETSI},
    {"FR", MAC_DFS_DOMAIN_ETSI},
    {"GB", MAC_DFS_DOMAIN_ETSI},
    {"GE", MAC_DFS_DOMAIN_ETSI},
    {"GR", MAC_DFS_DOMAIN_ETSI},
    {"GT", MAC_DFS_DOMAIN_FCC},
    {"HK", MAC_DFS_DOMAIN_FCC},
    {"HN", MAC_DFS_DOMAIN_FCC},
    {"HR", MAC_DFS_DOMAIN_ETSI},
    {"HU", MAC_DFS_DOMAIN_ETSI},
    {"ID", MAC_DFS_DOMAIN_NULL},
    {"IE", MAC_DFS_DOMAIN_ETSI},
    {"IL", MAC_DFS_DOMAIN_ETSI},
    {"IN", MAC_DFS_DOMAIN_NULL},
    {"IQ", MAC_DFS_DOMAIN_NULL},
    {"IR", MAC_DFS_DOMAIN_NULL},
    {"IS", MAC_DFS_DOMAIN_ETSI},
    {"IT", MAC_DFS_DOMAIN_ETSI},
    {"JM", MAC_DFS_DOMAIN_FCC},
    {"JO", MAC_DFS_DOMAIN_ETSI},
    {"JP", MAC_DFS_DOMAIN_MKK},
    {"KP", MAC_DFS_DOMAIN_NULL},
    {"KR", MAC_DFS_DOMAIN_KOREA},
    {"KW", MAC_DFS_DOMAIN_ETSI},
    {"KZ", MAC_DFS_DOMAIN_NULL},
    {"LB", MAC_DFS_DOMAIN_NULL},
    {"LI", MAC_DFS_DOMAIN_ETSI},
    {"LK", MAC_DFS_DOMAIN_FCC},
    {"LT", MAC_DFS_DOMAIN_ETSI},
    {"LU", MAC_DFS_DOMAIN_ETSI},
    {"LV", MAC_DFS_DOMAIN_ETSI},
    {"MA", MAC_DFS_DOMAIN_NULL},
    {"MC", MAC_DFS_DOMAIN_ETSI},
    {"MK", MAC_DFS_DOMAIN_ETSI},
    {"MO", MAC_DFS_DOMAIN_FCC},
    {"MT", MAC_DFS_DOMAIN_ETSI},
    {"MX", MAC_DFS_DOMAIN_FCC},
    {"MY", MAC_DFS_DOMAIN_FCC},
    {"NG", MAC_DFS_DOMAIN_NULL},
    {"NL", MAC_DFS_DOMAIN_ETSI},
    {"NO", MAC_DFS_DOMAIN_ETSI},
    {"NP", MAC_DFS_DOMAIN_NULL},
    {"NZ", MAC_DFS_DOMAIN_FCC},
    {"OM", MAC_DFS_DOMAIN_FCC},
    {"PA", MAC_DFS_DOMAIN_FCC},
    {"PE", MAC_DFS_DOMAIN_FCC},
    {"PG", MAC_DFS_DOMAIN_FCC},
    {"PH", MAC_DFS_DOMAIN_FCC},
    {"PK", MAC_DFS_DOMAIN_NULL},
    {"PL", MAC_DFS_DOMAIN_ETSI},
    {"PR", MAC_DFS_DOMAIN_FCC},
    {"PT", MAC_DFS_DOMAIN_ETSI},
    {"QA", MAC_DFS_DOMAIN_NULL},
    {"RO", MAC_DFS_DOMAIN_ETSI},
    {"RU", MAC_DFS_DOMAIN_FCC},
    {"SA", MAC_DFS_DOMAIN_FCC},
    {"SE", MAC_DFS_DOMAIN_ETSI},
    {"SG", MAC_DFS_DOMAIN_NULL},
    {"SI", MAC_DFS_DOMAIN_ETSI},
    {"SK", MAC_DFS_DOMAIN_ETSI},
    {"SV", MAC_DFS_DOMAIN_FCC},
    {"SY", MAC_DFS_DOMAIN_NULL},
    {"TH", MAC_DFS_DOMAIN_FCC},
    {"TN", MAC_DFS_DOMAIN_ETSI},
    {"TR", MAC_DFS_DOMAIN_ETSI},
    {"TT", MAC_DFS_DOMAIN_FCC},
    {"TW", MAC_DFS_DOMAIN_NULL},
    {"UA", MAC_DFS_DOMAIN_NULL},
    {"US", MAC_DFS_DOMAIN_FCC},
    {"UY", MAC_DFS_DOMAIN_FCC},
    {"UZ", MAC_DFS_DOMAIN_FCC},
    {"VE", MAC_DFS_DOMAIN_FCC},
    {"VN", MAC_DFS_DOMAIN_ETSI},
    {"YE", MAC_DFS_DOMAIN_NULL},
    {"ZA", MAC_DFS_DOMAIN_FCC},
    {"ZW", MAC_DFS_DOMAIN_NULL}
};

/*****************************************************************************
 功能描述  : 判断是否是大写字母
 输入参数  : c_letter: 国家字符串字母
 输出参数  : 无

 修改历史      :
  1.日    期   : 2013年10月16日
    修改内容   : 新生成函数

*****************************************************************************/
OAL_STATIC oal_bool_enum_uint8  wal_is_alpha_upper(osal_s8 c_letter)
{
    if (c_letter >= 'A' && c_letter <= 'Z') {
        return OAL_TRUE;
    }

    return OAL_FALSE;
}

/*****************************************************************************
 功能描述  : 获取一个管制类的起始频带
 输入参数  : start_freq: 起始频率
             end_freq  : 结束频率
 输出参数  : 无

 修改历史      :
  1.日    期   : 2013年10月16日
    修改内容   : 新生成函数

*****************************************************************************/
osal_u8  wal_regdomain_get_band_etc(osal_u32 start_freq, osal_u32 end_freq)
{
#ifdef _PRE_WLAN_FEATURE_015CHANNEL_EXP
    if (start_freq > 2372 && end_freq < 2524) {           /* 2372、2524,2.4G信道频率上下限 */
#else
    if (start_freq > 2400 && end_freq < 2500) {           /* 2400、2500,2.4G信道频率上下限 */
#endif
        return MAC_RC_START_FREQ_2;
    } else if (start_freq > 5000 && end_freq < 5870) {    /* 5000、5870,5G信道频率上下限 */
        return MAC_RC_START_FREQ_5;
    } else if (start_freq > 4900 && end_freq < 4999) {    /* 4900、4999,4.9G信道频率上下限 */
        return MAC_RC_START_FREQ_5;
    } else {
        return MAC_RC_START_FREQ_BUTT;
    }
}

/*****************************************************************************
 功能描述  : 获取一个管制类的带宽
 输入参数  : bw: linux管制类中的带宽值
 输出参数  : 无

 修改历史      :
  1.日    期   : 2013年10月17日
    修改内容   : 新生成函数

*****************************************************************************/
osal_u8  wal_regdomain_get_bw_etc(osal_u8 bw)
{
    osal_u8 bw_map;

    switch (bw) {
        case 80:    /* 80M频宽 */
            bw_map = MAC_CH_SPACING_80MHZ;
            break;
        case 40:    /* 40M频宽 */
            bw_map = MAC_CH_SPACING_40MHZ;
            break;
        case 20:    /* 20M频宽 */
            bw_map = MAC_CH_SPACING_20MHZ;
            break;
        default:
            bw_map = MAC_CH_SPACING_BUTT;
            break;
    };

    return bw_map;
}

/*****************************************************************************
 功能描述  : 获取管制类信道位图，信道在2g频段上
 输入参数  : start_freq: 起始频率
             end_freq  : 结束频率
 输出参数  : 无

 修改历史      :
  1.日    期   : 2013年10月17日
    修改内容   : 新生成函数

*****************************************************************************/
osal_u32  wal_regdomain_get_channel_2g_etc(osal_u32 start_freq, osal_u32 end_freq)
{
    osal_u32 freq;
    osal_u32 i;
    osal_u32 ch_bmap = 0;

    for (freq = start_freq + 10; freq <= (end_freq - 10); freq++) { /* 信道频率偏移10M */
        for (i = 0; i < MAC_CHANNEL_FREQ_2_BUTT; i++) {
            if (freq == hmac_regdomain_get_freq_map_2g_etc()[i].freq) {
                ch_bmap |= (1 << i);
            }
        }
    }

    return ch_bmap;
}

#ifdef _PRE_WLAN_SUPPORT_5G
/*****************************************************************************
 功能描述  : 获取管制类信道位图，信道在5g频段上
 输入参数  : start_freq: 起始频率
             end_freq  : 结束频率
 输出参数  : 无

 修改历史      :
  1.日    期   : 2013年10月17日
    修改内容   : 新生成函数

*****************************************************************************/
osal_u32  wal_regdomain_get_channel_5g_etc(osal_u32 start_freq, osal_u32 end_freq)
{
    osal_u32 freq;
    osal_u32 i;
    osal_u32 ch_bmap = 0;

    for (freq = start_freq + 10; freq <= (end_freq - 10); freq += 5) { /* 信道频率偏移10M,每次间隔5M */
        for (i = 0; i < MAC_CHANNEL_FREQ_5_BUTT; i++) {
            if (freq == hmac_regdomain_get_freq_map_5g_etc()[i].freq) {
                ch_bmap |= (1 << i);
            }
        }
    }
    return ch_bmap;
}
#endif

/*****************************************************************************
 功能描述  : 获取1个管制类的信道位图
 输入参数  : band      : 频段
             start_freq: 起始频率
             end_freq  : 中止频率
 输出参数  : 无

 修改历史      :
  1.日    期   : 2013年10月17日
    修改内容   : 新生成函数

*****************************************************************************/
osal_u32  wal_regdomain_get_channel_etc(osal_u8 band, osal_u32 start_freq, osal_u32 end_freq)
{
    osal_u32 ch_bmap = 0;

    switch (band) {
        case MAC_RC_START_FREQ_2:
            ch_bmap = wal_regdomain_get_channel_2g_etc(start_freq, end_freq);
            break;

#ifdef _PRE_WLAN_SUPPORT_5G
        case MAC_RC_START_FREQ_5:
            ch_bmap = wal_regdomain_get_channel_5g_etc(start_freq, end_freq);
            break;
#endif

        default:
            break;
    }

    return ch_bmap;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,3,0))
osal_u32 wal_linux_update_wiphy_channel_list_num_etc(oal_net_device_stru *net_dev, oal_wiphy_stru *wiphy)
{
#ifdef KERNEL_4_9_ADAP
    osal_u16 len;
    osal_u32 ul_ret;
    mac_vendor_cmd_channel_list_stru channel_list;
    oal_ieee80211_supported_band *supported_band_2ghz_info = wal_cfg80211_get_ieee80211_supported_band();
#endif
    hmac_vap_stru                    *hmac_vap;

    if (wiphy == OAL_PTR_NULL || net_dev == OAL_PTR_NULL) {
        oam_error_log2(0, OAM_SF_ANY, "{wal_linux_update_wiphy_channel_list_num_etc::wiphy %p, net_dev %p}",
            (uintptr_t)wiphy, (uintptr_t)net_dev);
        return OAL_PTR_NULL;
    }

    hmac_vap = (hmac_vap_stru *)net_dev->ml_priv;
    if (hmac_vap == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_linux_update_wiphy_channel_list_num_etc::NET_DEV_PRIV is NULL.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
#ifdef KERNEL_4_9_ADAP
    ul_ret = hmac_config_vendor_cmd_get_channel_list_etc(hmac_vap, &len, (osal_u8 *)(&channel_list));
    if (ul_ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_linux_update_wiphy_channel_list_num_etc::get_channel_list fail. %d}",
                         ul_ret);
        return ul_ret;
    }

    /* 只更新2G信道个数，5G信道由于存在DFS 区域，且带宽计算并无问题,不需要修改 */
    supported_band_2ghz_info->n_channels = channel_list.channel_num_2g;

    oam_warning_log2(0, OAM_SF_ANY,
                     "{wal_linux_update_wiphy_channel_list_num_etc::2g_channel_num = %d, 5g_channel_num = %d}",
                     channel_list.channel_num_2g, channel_list.channel_num_5g);
#endif
    return OAL_SUCC;
}
#endif

/*****************************************************************************
 功能描述  : 根据国家码，获取对应的雷达检测标准
 输入参数  : mac_regdom: 管制域指针
             pc_country    : 国家码
 输出参数  : mac_regdom: 管制域指针
 返 回 值  : 无

 修改历史      :
  1.日    期   : 2014年10月31日
    修改内容   : 新生成函数

*****************************************************************************/
static inline osal_void  wal_get_dfs_domain(mac_regdomain_info_stru *mac_regdom,
    OAL_CONST osal_s8 *pc_country)
{
    osal_u32    u_idx;
    osal_u32 size = (osal_u32)OAL_ARRAY_SIZE(g_ast_dfs_domain_table_etc);

    for (u_idx = 0; u_idx < size; u_idx++) {
        if (0 == osal_strcmp(g_ast_dfs_domain_table_etc[u_idx].pc_country, pc_country)) {
            mac_regdom->dfs_domain = g_ast_dfs_domain_table_etc[u_idx].dfs_domain;

            return;
        }
    }

    mac_regdom->dfs_domain = MAC_DFS_DOMAIN_NULL;
}

/*****************************************************************************
 功能描述  : 填充管制下发的管制域信息
 输入参数  : regdom    : 指向linux的管制域信息
             mac_regdom: 指向要下发的管制域信息
 输出参数  : 无

 修改历史      :
  1.日    期   : 2013年10月16日
    修改内容   : 新生成函数

*****************************************************************************/
OAL_STATIC osal_void  wal_regdomain_fill_info(OAL_CONST oal_ieee80211_regdomain_stru *regdom,
    mac_regdomain_info_stru *mac_regdom)
{
    osal_u32  i;
    osal_u32  start;
    osal_u32  end;
    osal_u8   band;
    osal_u8   bw;

    /* 复制国家字符串 */
    mac_regdom->ac_country[0] = regdom->alpha2[0];
    mac_regdom->ac_country[1] = regdom->alpha2[1];
    mac_regdom->ac_country[2] = 0;

    /* 获取DFS认证标准类型 */
    wal_get_dfs_domain(mac_regdom, regdom->alpha2);

    /* 填充管制类个数 */
    mac_regdom->regclass_num = (osal_u8)regdom->n_reg_rules;

    /* 填充管制类信息 */
    for (i = 0; i < regdom->n_reg_rules; i++) {
        /* 填写管制类的频段(2.4G或5G) */
        start = regdom->reg_rules[i].freq_range.start_freq_khz / 1000;
        end   = regdom->reg_rules[i].freq_range.end_freq_khz / 1000;
        band  = wal_regdomain_get_band_etc(start, end);
        mac_regdom->regclass[i].start_freq = band;

        /* 填写管制类允许的最大带宽 */
        bw = (osal_u8)(regdom->reg_rules[i].freq_range.max_bandwidth_khz / 1000);
        mac_regdom->regclass[i].ch_spacing = wal_regdomain_get_bw_etc(bw);

        /* 填写管制类信道位图 */
        mac_regdom->regclass[i].channel_bmap = wal_regdomain_get_channel_etc(band, start, end);

        /* 标记管制类行为 */
        mac_regdom->regclass[i].behaviour_bmap = 0;

        if (regdom->reg_rules[i].flags & NL80211_RRF_DFS) {
            mac_regdom->regclass[i].behaviour_bmap |= MAC_RC_DFS;
        }

        if (regdom->reg_rules[i].flags & NL80211_RRF_NO_INDOOR) {
            mac_regdom->regclass[i].behaviour_bmap |= MAC_RC_NO_INDOOR;
        }

        if (regdom->reg_rules[i].flags & NL80211_RRF_NO_OUTDOOR) {
            mac_regdom->regclass[i].behaviour_bmap |= MAC_RC_NO_OUTDOOR;
        }

        /* 填充覆盖类和最大发送功率 */
        mac_regdom->regclass[i].coverage_class = 0;
        mac_regdom->regclass[i].max_reg_tx_pwr =
            (osal_u8)(regdom->reg_rules[i].power_rule.max_eirp / 100);
        mac_regdom->regclass[i].max_tx_pwr     =
            (osal_u16)(regdom->reg_rules[i].power_rule.max_eirp / 10);
    }
}

/*****************************************************************************
 功能描述  : 下发配置country事件
*****************************************************************************/
static inline osal_s32 wal_regdomain_send_country_event(oal_net_device_stru *net_dev, osal_s8 *pc_country,
    mac_regdomain_info_stru *mac_regdom)
{
    mac_cfg_country_stru                   *param;
    osal_s32                               l_ret;
    regdomain_enum                          regdomain;

    mac_cfg_country_stru country_info;
    (osal_void)memset_s(&country_info, OAL_SIZEOF(country_info), 0, OAL_SIZEOF(country_info));
    param = &country_info;

    /* 填写WID对应的参数 */
    regdomain = hwifi_get_regdomain_from_country_code(pc_country);
    mac_regdom->sideband_country = (regdomain == REGDOMAIN_ETSI) ? OAL_TRUE :
        (regdomain == REGDOMAIN_FCC) ? OAL_TRUE : OAL_FALSE;
    param->mac_regdom = mac_regdom;

    l_ret = wal_async_post2hmac_no_rsp(wal_util_get_vap_id(net_dev), WLAN_MSG_W2H_CFG_COUNTRY,
        (osal_u8 *)&country_info, OAL_SIZEOF(country_info), FRW_POST_PRI_LOW);
    if (OAL_UNLIKELY(OAL_SUCC != l_ret)) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_regdomain_send_country_event::wal_sync_post2hmac_no_rsp return err code %d!}", l_ret);
        return l_ret;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 更新regdomain时,不支持ACS特性的处理
*****************************************************************************/
static inline osal_s32 wal_regdomain_no_acs_proc(oal_net_device_stru *net_dev, osal_s8 *country,
    const oal_ieee80211_regdomain_stru *regdom)
{
    /* 驱动不支持ACS时，更新hostapd管制域信息; 如果驱动支持ACS，则不需要更新，否则hostapd无法配置DFS/14信道 */
#ifndef _PRE_SUPPORT_ACS
    osal_u8 dev_id;
    hmac_device_stru *hmac_device;
    hmac_vap_stru *hmac_vap = OSAL_NULL;
    hmac_vap = (hmac_vap_stru *)net_dev->ml_priv;
    if (OAL_UNLIKELY(hmac_vap == OSAL_NULL)) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_regdomain_no_acs_proc::net_dev->ml_priv is null ptr.}");
        return OAL_FAIL;
    }

    dev_id = hmac_vap->device_id;
    hmac_device = hmac_res_get_mac_dev_etc(dev_id);
    if ((hmac_device != OSAL_NULL) && (hmac_device->wiphy != OSAL_NULL)) {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,3,0))
        /* Linux 内核4.4版本增加10M带宽，将channel 12添加到US信道列表，修改       wifi 驱动刷新当前管制域支持信道列表    */
        wal_linux_update_wiphy_channel_list_num_etc(net_dev, hmac_device->wiphy);
#endif
        oam_warning_log2(0, OAM_SF_ANY, "{wal_regdomain_no_acs_proc::update kernel info:%c%c}", country[0], country[1]);
        wal_cfg80211_reset_bands_etc(dev_id);
        oal_wiphy_apply_custom_regulatory(hmac_device->wiphy, regdom);
        /* 更新管制域后，将每个信道flag 信息保存到对应orig_flag，避免去关联时恢复管制域信息错误，导致某些信道不能工作        */
        wal_cfg80211_save_bands_etc(dev_id);
    }
#endif
    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 下发配置管制域信息
 输入参数  : net_dev: net_device
             pc_country : 要设置的国家字符串
 输出参数  : 无
 修改历史      :
  1.日    期   : 2013年10月22日
    修改内容   : 新生成函数

*****************************************************************************/
osal_s32  wal_regdomain_update_etc(oal_net_device_stru *net_dev, osal_s8 *country)
{
    OAL_CONST oal_ieee80211_regdomain_stru *regdom;
    osal_u16                              size;
    mac_regdomain_info_stru                *mac_regdom;
    osal_s32                               ret;
    osal_s8 old_region = hwifi_get_region(hmac_regdomain_get_country_etc());
    osal_s8 new_region = hwifi_get_region(country);

    regdom = wal_regdb_find_db_etc(country);
    if (regdom == OSAL_NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_regdomain_update_etc::no regdomain db was found!}");
        return -OAL_EINVAL;
    }

    if (old_region != new_region) {
        hwifi_set_nvram_tag_by_region_index(new_region);
        hwifi_force_refresh_rf_params(net_dev);
    }

    size = (osal_u16)(OAL_SIZEOF(mac_regclass_info_stru) * regdom->n_reg_rules + MAC_RD_INFO_LEN);

    /* 申请内存存放管制域信息，将内存指针作为事件payload抛下去 */
    /* 此处申请的内存在事件处理函数释放(hmac_config_set_country_etc) */
    mac_regdom = oal_mem_alloc(OAL_MEM_POOL_ID_LOCAL, size, OAL_TRUE);
    if (mac_regdom == OSAL_NULL) {
        oam_error_log1(0, OAM_SF_ANY, "{wal_regdomain_update_etc::alloc regdom mem fail(size:%d)}", size);
        return -OAL_ENOMEM;
    }

    wal_regdomain_fill_info(regdom, mac_regdom);

    ret = wal_regdomain_send_country_event(net_dev, country, mac_regdom);
    if (ret != OAL_SUCC) {
        oal_mem_free(mac_regdom, OAL_TRUE);
        return ret;
    }
    return wal_regdomain_no_acs_proc(net_dev, country, regdom);
}
/*****************************************************************************
 功能描述  : 下发配置管制域信息
 输入参数  : net_dev: net_device
             pc_country : 要设置的国家字符串
 输出参数  : 无

 修改历史      :
  1.日    期   : 2015年1月19日
    修改内容   : 新生成函数

*****************************************************************************/
osal_s32  wal_regdomain_update_for_dfs_etc(oal_net_device_stru *net_dev, osal_s8 *pc_country)
{
    OAL_CONST oal_ieee80211_regdomain_stru *regdom;
    osal_u16                              size;
    mac_regdomain_info_stru                *mac_regdom;
    osal_s32                               l_ret;

    if (!wal_is_alpha_upper(pc_country[0]) || !wal_is_alpha_upper(pc_country[1])) {
        if (('9' == pc_country[0]) && ('9' == pc_country[1])) {
            oam_info_log0(0, OAM_SF_ANY, "{wal_regdomain_update_for_dfs_etc::set regdomain to 99!}");
        } else {
            oam_warning_log0(0, OAM_SF_ANY, "{wal_regdomain_update_for_dfs_etc::country str is invalid!}");
            return -OAL_EINVAL;
        }
    }

    regdom = wal_regdb_find_db_etc(pc_country);
    if (OAL_PTR_NULL == regdom) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_regdomain_update_for_dfs_etc::no regdomain db was found!}");
        return -OAL_EINVAL;
    }

    size = (osal_u16)(OAL_SIZEOF(mac_regclass_info_stru) * regdom->n_reg_rules + MAC_RD_INFO_LEN);

    /* 申请内存存放管制域信息,在本函数结束后释放 */
    mac_regdom = oal_mem_alloc(OAL_MEM_POOL_ID_LOCAL, size, OAL_TRUE);
    if (OAL_PTR_NULL == mac_regdom) {
        oam_error_log1(0, OAM_SF_ANY,
            "{wal_regdomain_update_for_dfs_etc::alloc regdom mem fail, return null ptr!size[%d]}", size);
        return -OAL_ENOMEM;
    }

    wal_regdomain_fill_info(regdom, mac_regdom);
    l_ret = wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(net_dev), WLAN_MSG_W2H_C_CFG_COUNTRY_FOR_DFS,
        &(mac_regdom->dfs_domain), OAL_SIZEOF(mac_dfs_domain_enum_uint8));
    if (OAL_UNLIKELY(OAL_SUCC != l_ret)) {
        /* mac_regdom内存，此处释放 */
        oal_mem_free(mac_regdom, OAL_TRUE);
        oam_warning_log1(0, OAM_SF_ANY, "{wal_regdomain_update_for_dfs_etc::return err code %d!}", l_ret);
        return l_ret;
    }
    /* mac_regdom内存，此处释放 */
    oal_mem_free(mac_regdom, OAL_TRUE);

    return OAL_SUCC;
}
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
