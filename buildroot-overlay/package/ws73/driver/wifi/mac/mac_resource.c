/*
 * Copyright (c) CompanyNameMagicTag 2020-2023. All rights reserved.
 * Description: Source file of the MAC resource pool.
 * Create: 2020-01-01
 */

/*****************************************************************************
    头文件包含
*****************************************************************************/
#include "mac_resource_ext.h"
#ifdef _PRE_PRODUCT_ID_HOST
#include "hmac_vap.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_MAC_RESOURCE_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

/*****************************************************************************
    全局变量定义
*****************************************************************************/
mac_res_stru g_st_mac_res_etc;
hmac_res_stru g_st_hmac_res_etc;
mac_res_stru *g_pst_mac_res = &g_st_mac_res_etc;

#if defined(_PRE_PRODUCT_ID_HIMPXX_DEV)
hmac_device_stru g_ast_dmac_vap[WLAN_VAP_SUPPORT_MAX_NUM_LIMIT];
hmac_user_stru g_ast_dmac_user[MAC_RES_MAX_USER_LIMIT];
mac_res_user_idx_size_stru g_st_dmac_user_idx_size[MAC_RES_MAX_USER_LIMIT];
mac_res_user_cnt_size_stru g_st_dmac_user_cnt_size[MAC_RES_MAX_USER_LIMIT];

hmac_device_stru *g_pst_dmac_vap = &g_ast_dmac_vap[0];
hmac_user_stru *g_pst_dmac_user = &g_ast_dmac_user[0];
mac_res_user_idx_size_stru *g_pst_dmac_user_idx_size = &g_st_dmac_user_idx_size[0];
mac_res_user_cnt_size_stru *g_pst_dmac_user_cnt_size = &g_st_dmac_user_cnt_size[0];
#endif

/*****************************************************************************
    函数实现
*****************************************************************************/
WIFI_HMAC_TCM_TEXT WIFI_TCM_TEXT mac_res_stru *mac_get_pst_mac_res(osal_void)
{
    return &g_st_mac_res_etc;
}

WIFI_HMAC_TCM_TEXT WIFI_TCM_TEXT hmac_res_stru *mac_pst_hmac_res_etc(osal_void)
{
    return &g_st_hmac_res_etc;
}

/*****************************************************************************
 功能描述 : 获取一个DEV资源
*****************************************************************************/
osal_u32 mac_res_alloc_hmac_dev_etc(osal_u8 *dev_idx)
{
    osal_ulong dev_idx_temp;

    if (OAL_UNLIKELY(dev_idx == OAL_PTR_NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "mac_res_alloc_hmac_dev_etc::dev_idx null.");
        return OAL_FAIL;
    }

    dev_idx_temp = (osal_ulong)oal_queue_dequeue(&(g_pst_mac_res->dev_res.queue));
    if (dev_idx_temp == 0) { /* 0为无效值 */
        oam_error_log0(0, OAM_SF_ANY, "mac_res_alloc_hmac_dev_etc::dev_idx_temp=0.");
        return OAL_FAIL;
    }

    *dev_idx = (osal_u8)(dev_idx_temp - 1);

    (g_pst_mac_res->dev_res.auc_user_cnt[dev_idx_temp - 1])++;

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述 : 初始化MAC资源池内容
*****************************************************************************/
static osal_u32 mac_res_check_spec_etc(void)
{
    osal_u32 ret = OAL_SUCC;

    return ret;
}

#if (defined(_PRE_PRODUCT_ID_HIMPXX_DEV))
/*****************************************************************************
 功能描述 : 初始化vap资源池内容
*****************************************************************************/
static void hmac_res_vap_init(void)
{
    osal_u32 loop;

    osal_ulong one_vap_size;

    one_vap_size = 0;
    for (loop = 0; loop < WLAN_VAP_SUPPORT_MAX_NUM_LIMIT; loop++) {
        g_pst_mac_res->vap_res.past_vap_info[loop] = (osal_u8 *)g_pst_dmac_vap + one_vap_size;
        memset_s(g_pst_mac_res->vap_res.past_vap_info[loop], OAL_SIZEOF(hmac_device_stru),
            0, OAL_SIZEOF(hmac_device_stru));

        one_vap_size += OAL_SIZEOF(hmac_device_stru);
        /* 初始化对应的引用计数值为0 */
        g_pst_mac_res->vap_res.auc_user_cnt[loop] = 0;
    }
}

/*****************************************************************************
 功能描述 : 用户资源池初始化
*****************************************************************************/
static osal_u32 hmac_res_user_init_etc(void)
{
    osal_u32 loop;
    osal_ulong one_user_info_size;

    if ((g_pst_dmac_user == OAL_PTR_NULL) || (g_pst_dmac_user_idx_size == OAL_PTR_NULL) ||
        (g_pst_dmac_user_cnt_size == OAL_PTR_NULL)) {
        oam_error_log0(0, OAM_SF_UM, "{hmac_res_user_init_etc:input para null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 内存初始清0 */
    memset_s(g_pst_dmac_user, (OAL_SIZEOF(hmac_user_stru) * MAC_RES_MAX_USER_LIMIT),
        0, (OAL_SIZEOF(hmac_user_stru) * MAC_RES_MAX_USER_LIMIT));
    memset_s(g_pst_dmac_user_idx_size, (OAL_SIZEOF(osal_ulong) * MAC_RES_MAX_USER_LIMIT),
        0, (OAL_SIZEOF(osal_ulong) * MAC_RES_MAX_USER_LIMIT));
    memset_s(g_pst_dmac_user_cnt_size, (OAL_SIZEOF(osal_u8) * MAC_RES_MAX_USER_LIMIT),
        0, (OAL_SIZEOF(osal_u8) * MAC_RES_MAX_USER_LIMIT));

    g_pst_mac_res->user_res.pul_idx = (osal_ulong *)g_pst_dmac_user_idx_size;
    g_pst_mac_res->user_res.user_cnt = (osal_u8 *)g_pst_dmac_user_cnt_size;

    one_user_info_size = 0;
    for (loop = 0; loop < MAC_RES_MAX_USER_LIMIT; loop++) {
        /* 初始化对应的引用计数值为0 */
        g_pst_mac_res->user_res.past_user_info[loop] = (osal_u8 *)g_pst_dmac_user + one_user_info_size;
        one_user_info_size += OAL_SIZEOF(hmac_user_stru);
    }

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述 : 资源池退出，释放动态申请的内存
*****************************************************************************/
osal_u32 hmac_res_exit(void)
{
    osal_ulong loop;

    oal_mem_free((g_pst_mac_res->user_res.past_user_info[0]), OAL_TRUE);

    for (loop = 0; loop < MAC_RES_MAX_USER_LIMIT; loop++) {
        g_pst_mac_res->user_res.past_user_info[loop] = OAL_PTR_NULL;
    }

    g_pst_mac_res->user_res.pul_idx = OAL_PTR_NULL;
    g_pst_mac_res->user_res.user_cnt = OAL_PTR_NULL;

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述 : 初始化MAC资源池内容
*****************************************************************************/
osal_u32 mac_res_init_etc(void)
{
    osal_ulong loop;
    osal_u32 ret;

    memset_s(g_pst_mac_res, OAL_SIZEOF(mac_res_stru), 0, OAL_SIZEOF(mac_res_stru));
    /***************************************************************************
            初始化DEV的资源管理内容
    ***************************************************************************/
    oal_queue_set(&(g_pst_mac_res->dev_res.queue), g_pst_mac_res->dev_res.idx, MAC_RES_MAX_DEV_NUM);

    ret = mac_res_check_spec_etc();
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_ANY, "{mac_res_init_etc::hmac_res_init_etc failed[%d].}", ret);
        return ret;
    }

    for (loop = 0; loop < MAC_RES_MAX_DEV_NUM; loop++) {
        /* 初始值保存的是对应数组下标值加1 */
        oal_queue_enqueue(&(g_pst_mac_res->dev_res.queue), (void *)(loop + 1));

        /* 初始化对应的引用计数值为0 */
        g_pst_mac_res->dev_res.auc_user_cnt[loop] = 0;
    }

    /***************************************************************************
            初始化VAP的资源管理内容
    ***************************************************************************/
    hmac_res_vap_init();
    /***************************************************************************
            初始化USER的资源管理内容
            初始化HASH桶的资源管理内容
    ***************************************************************************/
    ret = hmac_res_user_init_etc();
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_ANY, "{mac_res_init_etc::hmac_res_user_init_etc failed[%d].}", ret);
        return ret;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述 : 释放对应HMAC USER的内存
*****************************************************************************/
osal_u32 hmac_res_free_mac_user_etc(osal_u16 idx)
{
    if (OAL_UNLIKELY(idx >= MAC_RES_MAX_USER_LIMIT)) {
        return OAL_FAIL;
    }

    if (g_pst_mac_res->user_res.user_cnt[idx] == 0) {
        oam_error_log1(0, OAM_SF_ANY, "hmac_res_free_mac_user_etc::cnt==0! idx:%d", idx);
        oal_dump_stack();
        return OAL_ERR_CODE_USER_RES_CNT_ZERO;
    }

    (g_pst_mac_res->user_res.user_cnt[idx])--;

    if (g_pst_mac_res->user_res.user_cnt[idx] != 0) {
        return OAL_SUCC;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述 : 释放对应HMAC VAP的内存
*****************************************************************************/
osal_u32 hmac_res_free_mac_vap_etc(osal_u32 idx)
{
    if (OAL_UNLIKELY(idx >= WLAN_VAP_SUPPORT_MAX_NUM_LIMIT)) {
        return OAL_FAIL;
    }

    if (g_pst_mac_res->vap_res.auc_user_cnt[idx] == 0) {
        oam_error_log1(0, OAM_SF_ANY, "hmac_res_free_mac_vap_etc::cnt==0! idx:%d", idx);
        oal_dump_stack();
        return OAL_SUCC;
    }

    (g_pst_mac_res->vap_res.auc_user_cnt[idx])--;

    if (g_pst_mac_res->vap_res.auc_user_cnt[idx] != 0) {
        return OAL_SUCC;
    }

    return OAL_SUCC;
}

#else
/*****************************************************************************
 功能描述 : 初始化vap资源池内容
*****************************************************************************/
static void hmac_res_vap_init(void)
{
    osal_ulong loop;

    oal_queue_set(&(g_pst_mac_res->vap_res.queue), g_pst_mac_res->vap_res.idx,
        (osal_u8)WLAN_VAP_SUPPORT_MAX_NUM_LIMIT);

    for (loop = 0; loop < WLAN_VAP_SUPPORT_MAX_NUM_LIMIT; loop++) {
#if (defined(_PRE_PRODUCT_ID_HOST))
        g_pst_mac_res->vap_res.past_vap_info[loop] = (void *)hmac_get_vap(loop);
        memset_s(g_pst_mac_res->vap_res.past_vap_info[loop], OAL_SIZEOF(hmac_vap_stru), 0, OAL_SIZEOF(hmac_vap_stru));
#else
        g_pst_mac_res->vap_res.past_vap_info[loop] = (void *)&g_ast_mac_res_vap[loop];
        memset_s(g_pst_mac_res->vap_res.past_vap_info[loop], OAL_SIZEOF(mac_res_mem_vap_stru),
            0, OAL_SIZEOF(mac_res_mem_vap_stru));
#endif

        /* 初始值保存的是对应数组下标值加1 */
        oal_queue_enqueue(&(g_pst_mac_res->vap_res.queue), (void *)(loop + 1));
        /* 初始化对应的引用计数值为0 */
        g_pst_mac_res->vap_res.auc_user_cnt[loop] = 0;
    }
}

/*****************************************************************************
 功能描述 : 用户资源池初始化
*****************************************************************************/
static osal_u32 hmac_res_user_init_etc(void)
{
    osal_ulong loop;
    void *idx = OAL_PTR_NULL;
    void *user_cnt = OAL_PTR_NULL;
    osal_u8 max_user_num = (osal_u8)hmac_board_get_max_user();

    /***************************************************************************
            初始化USER的资源管理内容
    ***************************************************************************/
    /* 动态申请用户资源池相关内存 */
    idx = oal_memalloc(OAL_SIZEOF(osal_ulong) * max_user_num);
    user_cnt = oal_memalloc(OAL_SIZEOF(osal_u8) * max_user_num);
    if ((idx == OAL_PTR_NULL) || (user_cnt == OAL_PTR_NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_res_user_init_etc::param null.}");

        if (idx != OAL_PTR_NULL) {
            oal_free(idx);
        }

        if (user_cnt != OAL_PTR_NULL) {
            oal_free(user_cnt);
        }

        return OAL_ERR_CODE_ALLOC_MEM_FAIL;
    }

    /* 内存初始清0 */
    memset_s(idx, (OAL_SIZEOF(osal_ulong) * max_user_num), 0, (OAL_SIZEOF(osal_ulong) * max_user_num));
    memset_s(user_cnt, (OAL_SIZEOF(osal_u8) * max_user_num), 0, (OAL_SIZEOF(osal_u8) * max_user_num));

    g_pst_mac_res->user_res.pul_idx = idx;
    g_pst_mac_res->user_res.user_cnt = user_cnt;

    oal_queue_set_16(&(g_pst_mac_res->user_res.queue), g_pst_mac_res->user_res.pul_idx,
        (osal_u16)max_user_num);

    for (loop = 0; loop < max_user_num; loop++) {
        /* 初始值保存的是对应数组下标值加1 */
        oal_queue_enqueue_16(&(g_pst_mac_res->user_res.queue), (void *)(loop + 1));
        /* 初始化对应的引用位置 */
        g_pst_mac_res->user_res.past_user_info[loop] = OSAL_NULL;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述 : 资源池退出，释放动态申请的内存
*****************************************************************************/
osal_u32 hmac_res_exit(void)
{
    osal_ulong loop;
    if (g_pst_mac_res->user_res.past_user_info[0] != OSAL_NULL) {
        oal_free(g_pst_mac_res->user_res.past_user_info[0]);
    }

    if (g_pst_mac_res->user_res.pul_idx != OSAL_NULL) {
        oal_free(g_pst_mac_res->user_res.pul_idx);
    }

    if (g_pst_mac_res->user_res.user_cnt != OSAL_NULL) {
        oal_free(g_pst_mac_res->user_res.user_cnt);
    }

    for (loop = 0; loop < MAC_RES_MAX_USER_LIMIT; loop++) {
        g_pst_mac_res->user_res.past_user_info[loop] = OAL_PTR_NULL;
    }
    g_pst_mac_res->user_res.pul_idx = OAL_PTR_NULL;
    g_pst_mac_res->user_res.user_cnt = OAL_PTR_NULL;

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述 : 初始化MAC资源池内容
*****************************************************************************/
osal_u32 mac_res_init_etc(void)
{
    osal_ulong loop;
    osal_u32 ret;

    memset_s(g_pst_mac_res, OAL_SIZEOF(mac_res_stru), 0, OAL_SIZEOF(mac_res_stru));
    /***************************************************************************
            初始化DEV的资源管理内容
    ***************************************************************************/
    oal_queue_set(&(g_pst_mac_res->dev_res.queue), g_pst_mac_res->dev_res.idx, MAC_RES_MAX_DEV_NUM);

    ret = mac_res_check_spec_etc();
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_ANY, "{mac_res_init_etc::hmac_res_user_init_etc failed[%d].}", ret);
        return ret;
    }

    for (loop = 0; loop < MAC_RES_MAX_DEV_NUM; loop++) {
        /* 初始值保存的是对应数组下标值加1 */
        oal_queue_enqueue(&(g_pst_mac_res->dev_res.queue), (void *)(loop + 1));

        /* 初始化对应的引用计数值为0 */
        g_pst_mac_res->dev_res.auc_user_cnt[loop] = 0;
    }

    /***************************************************************************
            初始化VAP的资源管理内容
    ***************************************************************************/
    hmac_res_vap_init();

    /***************************************************************************
            初始化USER的资源管理内容
            初始化HASH桶的资源管理内容
    ***************************************************************************/
    ret = hmac_res_user_init_etc();
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_ANY, "{mac_res_init_etc::hmac_res_user_init_etc failed[%d].}", ret);
        return ret;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述 : 释放对应HMAC USER的内存
*****************************************************************************/
osal_u32 hmac_res_free_mac_user_etc(osal_u16 idx)
{
    if (OAL_UNLIKELY(idx >= hmac_board_get_max_user())) {
        return OAL_FAIL;
    }

    if (g_pst_mac_res->user_res.user_cnt[idx] == 0) {
        oam_error_log1(0, OAM_SF_ANY, "hmac_res_free_mac_user_etc::cnt==0! idx:%d", idx);
        oal_dump_stack();
        return OAL_ERR_CODE_USER_RES_CNT_ZERO;
    }

    (g_pst_mac_res->user_res.user_cnt[idx])--;

    if (g_pst_mac_res->user_res.user_cnt[idx] != 0) {
        return OAL_SUCC;
    }

    /* 入队索引值需要加1操作 */
    oal_queue_enqueue_8user(&(g_pst_mac_res->user_res.queue), (void *)((osal_ulong)idx + 1));

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述 : 释放对应HMAC VAP的内存
*****************************************************************************/
osal_u32 hmac_res_free_mac_vap_etc(osal_u32 idx)
{
    if (OAL_UNLIKELY(idx >= WLAN_VAP_SUPPORT_MAX_NUM_LIMIT)) {
        return OAL_FAIL;
    }

    if (g_pst_mac_res->vap_res.auc_user_cnt[idx] == 0) {
        oam_error_log1(0, OAM_SF_ANY, "hmac_res_free_mac_vap_etc::cnt==0! idx:%d", idx);
        oal_dump_stack();
        return OAL_SUCC;
    }

    (g_pst_mac_res->vap_res.auc_user_cnt[idx])--;

    if (g_pst_mac_res->vap_res.auc_user_cnt[idx] != 0) {
        return OAL_SUCC;
    }

    /* 入队索引值需要加1操作 */
    oal_queue_enqueue(&(g_pst_mac_res->vap_res.queue), (void *)((osal_ulong)idx + 1));

    return OAL_SUCC;
}
#endif

/*****************************************************************************
 功能描述 : 释放对应DEV的内存
*****************************************************************************/
osal_u32 mac_res_free_dev_etc(osal_u32 dev_idx)
{
    if (OAL_UNLIKELY(dev_idx >= MAC_RES_MAX_DEV_NUM)) {
        oam_error_log1(0, OAM_SF_ANY, "{mac_res_free_dev_etc::invalid dev_idx[%d].}", dev_idx);

        return OAL_FAIL;
    }

    if (g_pst_mac_res->dev_res.auc_user_cnt[dev_idx] == 0) {
        oam_error_log1(0, OAM_SF_ANY, "mac_res_free_dev_etc::cnt==0! idx:%d", dev_idx);
        oal_dump_stack();
        return OAL_SUCC;
    }

    (g_pst_mac_res->dev_res.auc_user_cnt[dev_idx])--;

    if (g_pst_mac_res->dev_res.auc_user_cnt[dev_idx] != 0) {
        return OAL_SUCC;
    }

    /* 入队索引值需要加1操作 */
    oal_queue_enqueue(&(g_pst_mac_res->dev_res.queue), (void *)((osal_ulong)dev_idx + 1));

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述 : 读取最大关联用户数
*****************************************************************************/
osal_u16 mac_chip_get_max_asoc_user(osal_u8 chip_id)
{
    unref_param(chip_id);
    /* 关联单播最大用户数 */
    return mac_get_user_res_num();
}

WIFI_HMAC_TCM_TEXT WIFI_TCM_TEXT osal_u16 hmac_board_get_max_user(void)
{
    /* 整板单播和组播用户总数，整板单播和组播用户数直接取用宏值即可 */
    return mac_get_user_res_num() + WLAN_MULTI_USER_MAX_NUM_LIMIT;
}

/*****************************************************************************
  3 函数实现
*****************************************************************************/


/*****************************************************************************
 函 数 名  : hmac_res_alloc_mac_dev_etc
 功能描述  : 获取一个HMAC DEV资源
 输入参数  : 无
 输出参数  : HMAC DEV内存索引值
 返 回 值  : OAL_SUCC/OAL_FAIL
 修改历史      :
  1.日    期   : 2013年5月8日
    修改内容   : 新生成函数
*****************************************************************************/
osal_u32 hmac_res_alloc_mac_dev_etc(osal_u32    dev_idx)
{
    if (OAL_UNLIKELY(dev_idx >= MAC_RES_MAX_DEV_NUM)) {
        oam_error_log1(0, OAM_SF_ANY, "{hmac_res_alloc_hmac_dev::invalid dev_idx[%d].}", dev_idx);

        return OAL_FAIL;
    }

    (g_st_hmac_res_etc.hmac_dev_res.auc_user_cnt[dev_idx])++;

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_res_free_mac_dev_etc
 功能描述  : 释放对应HMAC DEV的内存
 输入参数  : 对应HMAC DEV内存索引
 输出参数  : 无
 返 回 值  : OAL_SUCC/OAL_FAIL
 修改历史      :
  1.日    期   : 2015年1月31日
    修改内容   : 新生成函数
*****************************************************************************/
osal_u32 hmac_res_free_mac_dev_etc(osal_u32 dev_idx)
{
    if (OAL_UNLIKELY(dev_idx >= MAC_RES_MAX_DEV_NUM)) {
        oam_error_log1(0, OAM_SF_ANY, "{mac_res_free_dev_etc::invalid dev_idx[%d].}", dev_idx);

        return OAL_FAIL;
    }

    (g_st_hmac_res_etc.hmac_dev_res.auc_user_cnt[dev_idx])--;

    if (g_st_hmac_res_etc.hmac_dev_res.auc_user_cnt[dev_idx] != 0) {
        return OAL_SUCC;
    }

    /* 入队索引值需要加1操作 */
    oal_queue_enqueue(&(g_st_hmac_res_etc.hmac_dev_res.queue), (osal_void *)((osal_ulong)dev_idx + 1));

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_res_init_etc
 功能描述  : 初始化HMAC资源池内容
 输入参数  : 无
 输出参数  : 无
 返 回 值  : OAL_SUCC
 修改历史      :
  1.日    期   : 2015年1月31日
    修改内容   : 新生成函数
*****************************************************************************/
osal_u32 hmac_res_init_etc(osal_void)
{
    osal_u32      loop;

    memset_s(&g_st_hmac_res_etc, OAL_SIZEOF(g_st_hmac_res_etc), 0, OAL_SIZEOF(g_st_hmac_res_etc));

    /***************************************************************************
            初始化HMAC DEV的资源管理内容
    ***************************************************************************/
    oal_queue_set(&(g_st_hmac_res_etc.hmac_dev_res.queue),
                  g_st_hmac_res_etc.hmac_dev_res.idx,
                  MAC_RES_MAX_DEV_NUM);

    for (loop = 0; loop < MAC_RES_MAX_DEV_NUM; loop++) {
        /* 初始值保存的是对应数组下标值加1 */
        oal_queue_enqueue(&(g_st_hmac_res_etc.hmac_dev_res.queue), (osal_void *)((osal_ulong)loop + 1));

        /* 初始化对应的引用计数值为0 */
        g_st_hmac_res_etc.hmac_dev_res.auc_user_cnt[loop] = 0;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_res_exit_etc
 功能描述  : 资源池退出，释放动态申请的内存
 修改历史      :
  1.日    期   : 2015年6月1日
*****************************************************************************/
osal_u32 hmac_res_exit_etc(mac_board_stru *hmac_board)
{
    osal_u8           chip;
    osal_u16          chip_max_num;
    mac_chip_stru       *pst_chip;
    osal_u32 ret;

    /* chip支持的最大数由PCIe总线处理提供; */
    chip_max_num = oal_bus_get_chip_num_etc();

    for (chip = 0; chip < chip_max_num; chip++) {
        pst_chip = &hmac_board->chip[chip];

        /* 释放hmac res资源 */
        ret = hmac_res_free_mac_dev_etc(pst_chip->device_id[0]);
        if (ret != OAL_SUCC) {
            oam_error_log1(0, OAM_SF_ANY, "{hmac_res_exit_etc::hmac_res_free_mac_dev_etc ret[%d].}", ret);
        }
    }
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_res_get_mac_dev_etc
 功能描述  : 获取对应HMAC DEV索引的内存
 输入参数  : 对应HMAC DEV内存索引
 输出参数  : 无
 返 回 值  : 对应内存地址
 修改历史      :
  1.日    期   : 2015年1月31日
    修改内容   : 新生成函数
*****************************************************************************/
WIFI_HMAC_TCM_TEXT WIFI_TCM_TEXT hmac_device_stru *hmac_res_get_mac_dev_etc(osal_u32 dev_idx)
{
    hmac_res_stru *hmac_res_etc = OSAL_NULL;

    if (OAL_UNLIKELY(dev_idx >= MAC_RES_MAX_DEV_NUM)) {
        oam_error_log1(0, OAM_SF_ANY, "{hmac_res_get_mac_dev_etc::invalid dev_idx[%d].}", dev_idx);

        return OAL_PTR_NULL;
    }
    hmac_res_etc = mac_pst_hmac_res_etc();
    return &(hmac_res_etc->hmac_dev_res.hmac_dev_info[dev_idx]);
}

oal_module_symbol(hmac_res_get_mac_dev_etc);

/* ****************************************************************************
 功能描述  : 获取对应HMAC VAP索引的内存,由HMAC层强转为自己的内存解析
**************************************************************************** */
WIFI_TCM_TEXT void *mac_res_get_hmac_vap(osal_u8 idx)
{
    mac_res_stru *mac_res = OSAL_NULL;

    if (OAL_UNLIKELY(idx >= WLAN_VAP_SUPPORT_MAX_NUM_LIMIT)) {
        return OAL_PTR_NULL;
    }
    mac_res = mac_get_pst_mac_res();
    return (void *)(mac_res->vap_res.past_vap_info[idx]);
}

void *_mac_res_alloc_hmac_user(osal_u16 idx)
{
    mac_res_stru *mac_res = OSAL_NULL;

    if (OAL_UNLIKELY(idx >= hmac_board_get_max_user())) {
        return OAL_PTR_NULL;
    }

    mac_res = mac_get_pst_mac_res();
    if (mac_res->user_res.past_user_info[idx] == OSAL_NULL) {
        mac_res->user_res.past_user_info[idx] = oal_memalloc(sizeof(hmac_user_stru));
        if (mac_res->user_res.past_user_info[idx] == OSAL_NULL) {
            oam_error_log1(0, OAM_SF_UM, "{_mac_res_alloc_hmac_user::[E]user alloc fail,user_idx=%d.}", idx);
        }
    } else {
        oam_error_log1(0, OAM_SF_UM, "{_mac_res_alloc_hmac_user::[E]user has been alloced,user_idx=%d.}", idx);
    }

    return (void *)mac_res->user_res.past_user_info[idx];
}

void _mac_res_free_hmac_user(osal_u16 idx)
{
    mac_res_stru *mac_res = OSAL_NULL;

    if (OAL_UNLIKELY(idx >= hmac_board_get_max_user())) {
        return;
    }

    mac_res = mac_get_pst_mac_res();
    if (mac_res->user_res.past_user_info[idx] != OSAL_NULL) {
        oal_free(mac_res->user_res.past_user_info[idx]);
        mac_res->user_res.past_user_info[idx] = OSAL_NULL;
    } else {
        oam_error_log1(0, OAM_SF_UM, "{_mac_res_free_hmac_user::[E]user has been freed,user_idx=%d.}", idx);
    }
}

/* ****************************************************************************
 功能描述  : 获取对应HMAC USER索引的内存,由HMAC层强转为自己的内存解析
**************************************************************************** */
WIFI_HMAC_TCM_TEXT WIFI_TCM_TEXT void *_mac_res_get_hmac_user(osal_u16 idx)
{
    mac_res_stru *mac_res = OSAL_NULL;

    if (OAL_UNLIKELY(idx >= hmac_board_get_max_user())) {
        return OAL_PTR_NULL;
    }
    mac_res = mac_get_pst_mac_res();
    return (void *)(mac_res->user_res.past_user_info[idx]);
}

osal_u32 mac_get_hal_vap_id(osal_u8 p2p_mode, osal_u8 vap_mode, osal_u8 *hal_vap_id)
{
    if (is_p2p_mode(p2p_mode)) {
        *hal_vap_id = HAL_VAP_VAP1;       /* P2P 固定使用ID1 */
    } else if (vap_mode == WLAN_VAP_MODE_BSS_STA) {
        *hal_vap_id = HAL_VAP_VAP0;      /* WLAN0 STA:固定使用ID0 */
    } else {
        *hal_vap_id = HAL_VAP_VAP2;         /* AP固定使用ID2 */
    }
    return OAL_SUCC;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
