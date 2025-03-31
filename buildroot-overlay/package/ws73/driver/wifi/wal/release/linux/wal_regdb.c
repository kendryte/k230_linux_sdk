/*
 * Copyright (c) CompanyNameMagicTag 2013-2023. All rights reserved.
 * Description: wal regdb define.
 */

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "wal_regdb.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_WAL_REGDB_C
#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
#define NL80211_RRF_NO_INDOOR (1 << 2)
#define NUM_2_REG_RULE  2
#define NUM_3_REG_RULE  3
#define NUM_4_REG_RULE  4
#define NUM_5_REG_RULE  5
#define NUM_6_REG_RULE  6

/* 国家码/管制域规则表 */
OAL_CONST wal_country_reg_list_stru g_country_reg_list[] = {
    {
        .country_list = "AI,BS,KW,UG",
        .reg_rule = REG_RULE(2402, 2472, 40, 6, 20, 0)
    },
    {
        .country_list = "AR,BM",
        .reg_rule = REG_RULE(2402, 2472, 40, 6, 30, 0)
    },
    {
        .country_list = "BO,BY,GB,GU,MH,NI,NP,VI",
        .reg_rule = REG_RULE(2402, 2482, 40, 6, 30, 0)
    },
    {
        .country_list = "BZ,PR,TW,US,CA",
        .reg_rule = REG_RULE(2402, 2472, 40, 3, 30, 0)
    },
    {
        .country_list = "CO,DO,GT,MX,PA,UZ",
        .reg_rule = REG_RULE(2402, 2482, 40, 3, 30, 0)
    },
    {
        .country_list = "CN",
        .reg_rule = REG_RULE(2402, 2482, 40, 6, 27, 0)
    },
    {
        .country_list = "UA",
        .reg_rule = REG_RULE(2402, 2482, 40, 6, 20, NL80211_RRF_NO_OUTDOOR | 0)
    },
    {
        .country_list = "AL,KR",
        .reg_rule = REG_RULE(2402, 2482, 20, 6, 20, 0)
    },
    {
        .country_list = "AE,AF,AG,AM,AN,AO,AS,AT,AU,AW,AZ,BA,BD,BE,BG,BH,BL,BN,BR,CH,CL,CR,CS,CU,CY,CZ,DE,DK,DZ,EC,"
            "EE,EG,ES,ET,FI,FR,GD,GE,GF,GL,GR,GU,HK,HN,HR,HU,ID,IE,JP,IL,IN,IQ,IR,IS,IT,JM,JO,KE,KH,KP,KY,KZ,LA,LB,LI,"
            "LK,LS,LT,LU,LV,MA,MC,MD,ME,MK,MN,MO,MR,MT,MU,MV,MW,MY,NG,NL,NO,NZ,OM,PE,PG,PH,PK,PL,PT,PY,QA,RE,RO,RS,RU,"
            "SA,SD,SE,SG,SI,SK,SN,SV,SY,TH,TN,TR,TT,UY,VA,VE,VN,YE,YT,ZA,ZM,ZW,ZZ",
        .reg_rule = REG_RULE(2402, 2482, 40, 6, 20, 0)
    }
};
/* 默认管制域信息 */
oal_ieee80211_regdomain_stru g_default_regdom_etc = {
    .n_reg_rules = 1,
    .alpha2 =  "99",
    .reg_rules = {
        /* IEEE 802.11b/g, 信道 1..13 */
        REG_RULE(2412 - 10, 2472 + 10, 40, 6, 25, 0),
#ifdef _PRE_WLAN_FEATURE_015CHANNEL_EXP
        REG_RULE(0, 0, 0, 0, 0, 0),
        REG_RULE(0, 0, 0, 0, 0, 0),
#endif
        REG_RULE(0, 0, 0, 0, 0, 0)
    }
};

oal_ieee80211_regdomain_stru *wal_regdb_get_default_regdom(void)
{
    return &g_default_regdom_etc;
}

/*****************************************************************************
  3 函数实现
*****************************************************************************/
#ifdef _PRE_WLAN_FEATURE_11D
static void wal_regdb_set_cur_regdom(const osal_s8 *country_code, struct ieee80211_reg_rule reg_rule)
{
    g_default_regdom_etc.reg_rules[0] = reg_rule;
    g_default_regdom_etc.alpha2[0] = country_code[0];
    g_default_regdom_etc.alpha2[1] = country_code[1];
    /* ZZ支持14ch 单独处理 */
    if (country_code[0] == 'Z' && country_code[1] == 'Z') {
#ifdef _PRE_WLAN_FEATURE_015CHANNEL_EXP
        g_default_regdom_etc.n_reg_rules = NUM_4_REG_RULE;
#else
        g_default_regdom_etc.n_reg_rules = NUM_2_REG_RULE;
#endif
        g_default_regdom_etc.reg_rules[1].freq_range.start_freq_khz = MHZ_TO_KHZ(2474);  /* 14信道起始频率2474 */
        g_default_regdom_etc.reg_rules[1].freq_range.end_freq_khz = MHZ_TO_KHZ(2494);    /* 14信道结束频率2494 */
        g_default_regdom_etc.reg_rules[1].freq_range.max_bandwidth_khz = MHZ_TO_KHZ(20); /* 14信道最大带宽20 */
        g_default_regdom_etc.reg_rules[1].power_rule.max_antenna_gain = DBI_TO_MBI(0);
        g_default_regdom_etc.reg_rules[1].power_rule.max_eirp = DBM_TO_MBM(20);          /* 14信道最大dbm 20 */
        g_default_regdom_etc.reg_rules[1].flags = NL80211_RRF_NO_OFDM | 0;
#ifdef _PRE_WLAN_FEATURE_015CHANNEL_EXP
        g_default_regdom_etc.reg_rules[2].freq_range.start_freq_khz = MHZ_TO_KHZ(2502);  /* 下标2为15信道起始频率2502 */
        g_default_regdom_etc.reg_rules[2].freq_range.end_freq_khz = MHZ_TO_KHZ(2522);    /* 下标2为15信道结束频率2522 */
        g_default_regdom_etc.reg_rules[2].freq_range.max_bandwidth_khz = MHZ_TO_KHZ(20); /* 下标2为15信道最大带宽20 */
        g_default_regdom_etc.reg_rules[2].power_rule.max_antenna_gain = DBI_TO_MBI(0);
        g_default_regdom_etc.reg_rules[2].power_rule.max_eirp = DBM_TO_MBM(20);          /* 下标2为15信道最大dbm 20 */
        g_default_regdom_etc.reg_rules[2].flags = NL80211_RRF_NO_OFDM | 0;
        g_default_regdom_etc.reg_rules[3].freq_range.start_freq_khz = MHZ_TO_KHZ(2374);  /* 下标3为16信道起始频率2374 */
        g_default_regdom_etc.reg_rules[3].freq_range.end_freq_khz = MHZ_TO_KHZ(2394);    /* 下标3为16信道结束频率2394 */
        g_default_regdom_etc.reg_rules[3].freq_range.max_bandwidth_khz = MHZ_TO_KHZ(20); /* 下标3为16信道最大带宽20 */
        g_default_regdom_etc.reg_rules[3].power_rule.max_antenna_gain = DBI_TO_MBI(0);
        g_default_regdom_etc.reg_rules[3].power_rule.max_eirp = DBM_TO_MBM(20);          /* 下标3为16信道最大dbm 20 */
        g_default_regdom_etc.reg_rules[3].flags = 0;
#endif
    } else {
        g_default_regdom_etc.n_reg_rules = 1;
    }
}
/*****************************************************************************
 功能描述  : 根据国家字符串查找管制域数据库
 输入参数  : 无
 输出参数  : 无

 修改历史      :
  1.日    期   : 2013年10月16日
    修改内容   : 新生成函数

*****************************************************************************/
OAL_CONST oal_ieee80211_regdomain_stru *wal_regdb_find_db_etc(osal_s8 *country_code)
{
    osal_u8 idx;

    if (osal_unlikely(country_code == OAL_PTR_NULL)) {
        return OAL_PTR_NULL;
    }

    for (idx = 0; idx < oal_array_size(g_country_reg_list); idx++) {
        if (strstr(g_country_reg_list[idx].country_list, (const osal_char *)country_code) != OSAL_NULL) {
            wal_regdb_set_cur_regdom(country_code, g_country_reg_list[idx].reg_rule);
            return &g_default_regdom_etc;
        }
    }

    return OAL_PTR_NULL;
}
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
