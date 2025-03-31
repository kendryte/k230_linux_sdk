/*
 * Copyright (c) CompanyNameMagicTag 2021-2021. All rights reserved.
 * Description: File where the function for processing multicast-to-unicast conversion is stored.
 * Create: 2021-12-15
 */
/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "hmac_m2u.h"

#include "oam_struct.h"
#include "hmac_tx_data.h"
#include "hmac_tx_encap.h"

#ifdef _PRE_WLAN_FEATURE_VIRTUAL_MULTI_STA
#include "hmac_wds.h"
#endif
#include "hmac_feature_dft.h"
#include "frw_hmac.h"
#include "hmac_feature_interface.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_HMAC_M2U_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

/*****************************************************************************
  3 函数实现
*****************************************************************************/

static INLINE__ osal_u8 is_igmp_report_leave_packet(const osal_u8 type)
{
    return (type == MAC_IGMPV1_REPORT_TYPE) || (type == MAC_IGMPV2_REPORT_TYPE) ||
        (type == MAC_IGMPV2_LEAVE_TYPE) || (type == MAC_IGMPV3_REPORT_TYPE);
}

/* Is packet type is either leave or report */
static INLINE__ osal_u8 is_igmpv3_mode(const osal_u8 type)
{
    return (type == IGMPV3_MODE_IS_INCLUDE) || (type == IGMPV3_MODE_IS_EXCLUDE) ||
        (type == IGMPV3_CHANGE_TO_INCLUDE) || (type == IGMPV3_CHANGE_TO_EXCLUDE) ||
        (type == IGMPV3_ALLOW_NEW_SOURCES) || (type == IGMPV3_BLOCK_OLD_SOURCES);
}

/* Is packet type is either leave or report */
static INLINE__ osal_u8 is_mld_report_leave_packet(const osal_u8 type)
{
    return (type == MLDV1_REPORT_TYPE) || (type == MLDV1_DONE_TYPE) || (type == MLDV2_REPORT_TYPE);
}

static INLINE__ osal_u8 is_mldv2_mode(const osal_u8 type)
{
    return (type == MLDV2_MODE_IS_INCLUDE) || (type == MLDV2_MODE_IS_EXCLUDE) ||
        (type == MLDV2_CHANGE_TO_INCLUDE) || (type == MLDV2_CHANGE_TO_EXCLUDE) ||
        (type == MLDV2_ALLOW_NEW_SOURCES) || (type == MLDV2_BLOCK_OLD_SOURCES);
}

static INLINE__ osal_bool oal_is_icmpv6_proto(osal_u16 ether_data_type, osal_u8 *ip_head)
{
    return (((ether_data_type) == oal_host2net_short(ETHER_TYPE_IPV6)) &&
        (((osal_u8)(ip_head[6]) == 0x3a) || ((osal_u8)(ip_head[40]) == 0x3a))); /* 6 40：数组下标 */
}

static INLINE__ osal_bool oal_is_igmp_proto(osal_u16 ether_data_type, osal_u8 *ip_head)
{
    /* 9：数组下标 */
    return ((ether_data_type == oal_host2net_short(ETHER_TYPE_IP)) && ((osal_u8)((ip_head)[9]) == 0x02));
}

static INLINE__ osal_u32 igmpv3_grp_rec_len(mac_igmp_v3_grec_stru *grec)
{
    return (osal_u32)(8 + (((osal_u16)(grec->grec_nsrcs + grec->grec_auxwords)) << 2)); /* 8：基本偏移量 2: 左移位数 */
}

static INLINE__ osal_bool oal_ipv4_is_multicast(osal_u8 *daddr)
{
    return ((osal_u8)(daddr[0]) >= 224 && ((osal_u8)(daddr[0]) <= 239)); /* IPv4多播范围: 224.0.0.0--239.255.255.255 */
}

/*****************************************************************************
 函 数 名  : hmac_m2u_add_member_list
 功能描述  : 将一个组成员加入到相对应的组中去
 输入参数  : grp_list 组播组结构体; list_entry 待更新节点结构体
 输出参数  : ul_ret
*****************************************************************************/
OAL_STATIC osal_u32 hmac_m2u_add_member_list(hmac_m2u_grp_list_entry_stru *grp_list,
    hmac_m2u_list_update_stru *list_entry)
{
    hmac_m2u_grp_member_stru *grp_member;
    hmac_m2u_stru *m2u_struct = (hmac_m2u_stru *)list_entry->hmac_vap->m2u;
    osal_u32 ul_ret             = OAL_SUCC;

    if ((grp_list->sta_num >= MAX_STA_NUM_OF_ONE_GROUP) ||
        (m2u_struct->m2u_snooplist.total_sta_num >= MAX_STA_NUM_OF_ALL_GROUP)) {
        oam_warning_log3(0, OAM_SF_M2U,
            "vap_id[%d] {hmac_m2u_add_member_list::sta num out of limit!current total_sta_num:[%d] ,sta_num:[%d].}",
            list_entry->hmac_vap->vap_id,
            m2u_struct->m2u_snooplist.total_sta_num, grp_list->sta_num);
        return OAL_ERR_CODE_ARRAY_OVERFLOW;
    }

    grp_member = (hmac_m2u_grp_member_stru *)oal_mem_alloc(OAL_MEM_POOL_ID_LOCAL,
        OAL_SIZEOF(hmac_m2u_grp_member_stru), OAL_TRUE);
    if (osal_unlikely(grp_member == OAL_PTR_NULL)) {
        oam_error_log1(0, OAM_SF_M2U, "vap_id[%d] {hmac_m2u_add_member_list:: oal_mem_alloc failed!!}",
            list_entry->hmac_vap->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    memset_s(grp_member, OAL_SIZEOF(hmac_m2u_grp_member_stru), 0x0,  OAL_SIZEOF(hmac_m2u_grp_member_stru));
    osal_list_add_tail(&(grp_member->member_entry), &(grp_list->src_list)); // 插入节点
    if (memcpy_s(grp_member->src_ip_addr, sizeof(grp_member->src_ip_addr), list_entry->src_ip_addr,
        sizeof(list_entry->src_ip_addr)) != EOK) {
        oam_error_log0(0, OAM_SF_M2U, "{hmac_m2u_add_member_list::memcpy_s ip error}");
    }
    oal_set_mac_addr(grp_member->grp_member_mac, list_entry->new_member_mac);

    grp_member->hmac_user       = list_entry->hmac_user;
    grp_member->mode             = list_entry->cmd;
    grp_member->timestamp        = list_entry->timestamp;
    grp_member->src_ip_addr_len  = list_entry->src_ip_addr_len;
    grp_list->sta_num++;
    m2u_struct->m2u_snooplist.total_sta_num++;

    oam_warning_log3(0, OAM_SF_M2U,
        "vap_id[%d] {hmac_m2u_add_member_list::add one sta !current total_sta_num:[%d] ,sta_num:[%d].}",
        list_entry->hmac_vap->vap_id,
        m2u_struct->m2u_snooplist.total_sta_num, grp_list->sta_num);
    return ul_ret;
}

/*****************************************************************************
 函 数 名  : hmac_m2u_remove_expired_member
 功能描述  : 删除某个组内超时的组成员
 输入参数  : grp_list 组播组结构体; hmac_vap vap 结构体; nowtimestamp nowtime变量
 输出参数  : 无
*****************************************************************************/
OAL_STATIC osal_u32 hmac_m2u_remove_expired_member(hmac_m2u_grp_list_entry_stru *grp_list,
    hmac_vap_stru *hmac_vap, osal_u32 nowtimestamp)
{
    hmac_m2u_grp_member_stru *grp_member;
    struct osal_list_head      *grp_member_entry;
    struct osal_list_head      *member_entry_temp;
    hmac_m2u_stru            *m2u = (hmac_m2u_stru *)(hmac_vap->m2u);
    osal_u32                ul_ret = OAL_SUCC;

    if (m2u == OAL_PTR_NULL) {
        return OAL_FAIL;
    }

    /* 遍历一个组，每次取出组中成员，超时则删除该成员 */
    osal_list_for_each_safe(grp_member_entry, member_entry_temp, &(grp_list->src_list)) {
        grp_member = (hmac_m2u_grp_member_stru *)osal_list_entry(grp_member_entry,
            hmac_m2u_grp_member_stru, member_entry);
        if (oal_time_get_runtime((grp_member->timestamp), nowtimestamp) > (m2u->timeout)) {
            osal_list_del(&(grp_member->member_entry));
            oal_mem_free(grp_member, OAL_TRUE);

            oam_warning_log3(0, OAM_SF_M2U,
                "vap_id[%d] {hmac_m2u_remove_expired_member:: current total sta num:[%d],sta num:[%d]}",
                hmac_vap->vap_id,
                m2u->m2u_snooplist.total_sta_num, grp_list->sta_num);

            grp_list->sta_num--;
            m2u->m2u_snooplist.total_sta_num--;

            oam_warning_log3(0, OAM_SF_M2U,
                "vap_id[%d] {hmac_m2u_remove_expired_member::remove one sta. current total sta num:[%d],sta num:[%d]}",
                hmac_vap->vap_id,
                m2u->m2u_snooplist.total_sta_num, grp_list->sta_num);
        }
    }
    return ul_ret;
}

/*****************************************************************************
 函 数 名  : hmac_m2u_remove_one_member
 功能描述  : 从一个给定的组播组下，删除某个组播组成员
 输入参数  : grp_list 组播组结构体; grp_member_addr 组播成员IP地址
 输出参数  : 无
*****************************************************************************/
OAL_STATIC osal_void  hmac_m2u_remove_one_member(hmac_m2u_grp_list_entry_stru *grp_list,
    hmac_vap_stru  *hmac_vap, osal_u8 *grp_member_addr)
{
    hmac_m2u_grp_member_stru *grp_member;
    struct osal_list_head      *grp_member_entry;
    struct osal_list_head      *member_entry_temp;
    hmac_m2u_stru            *m2u = (hmac_m2u_stru *)(hmac_vap->m2u);

    if (m2u == OAL_PTR_NULL) {
        oam_warning_log1(0, OAM_SF_M2U, "vap_id[%d] {hmac_m2u_remove_one_member::m2u is null}",
            hmac_vap->vap_id);
        return;
    }

    /* 遍历一个组播组删除指定成员 */
    osal_list_for_each_safe(grp_member_entry, member_entry_temp, &(grp_list->src_list)) {
        grp_member = (hmac_m2u_grp_member_stru *)osal_list_entry(grp_member_entry,
            hmac_m2u_grp_member_stru, member_entry);
        if (oal_compare_mac_addr(grp_member_addr, grp_member->grp_member_mac) == 0) {
            osal_list_del(&(grp_member->member_entry));
            oal_mem_free(grp_member, OAL_TRUE);

            oam_warning_log4(0, OAM_SF_M2U,
                "vap_id[%d] {hmac_m2u_remove_one_member::current sta_num:%d  total_sta_num:%d  group_num:%d}",
                hmac_vap->vap_id, grp_list->sta_num,
                m2u->m2u_snooplist.total_sta_num, m2u->m2u_snooplist.group_list_count);

            grp_list->sta_num--;
            m2u->m2u_snooplist.total_sta_num--;

            oam_warning_log4(0, OAM_SF_M2U,
                "{hmac_m2u_remove_one_member::remove one sta from group:0x%02x 0x%02x 0x%02x 0x%02x 0xxx 0xxx!}",
                /* 0，1，2表示组播组的MAC地址位 */
                grp_list->group_mac[0], grp_list->group_mac[1], grp_list->group_mac[2],
                grp_list->group_mac[3]); /* 3表示组播组的MAC地址位 */
            oam_warning_log4(0, OAM_SF_M2U,
                "vap_id[%d] {hmac_m2u_remove_one_member::current sta_num:%d total_sta_num:%d group_num:%d}",
                hmac_vap->vap_id, grp_list->sta_num,
                m2u->m2u_snooplist.total_sta_num, m2u->m2u_snooplist.group_list_count);
        }
    }
}

/*****************************************************************************
 函 数 名  : hmac_m2u_find_member_src
 功能描述  : 根据成员的mac地址以及组播源地址返回所需的组成员
 输入参数  : grp_list 组播组结构体; grp_member_addr 组播成员IP地址; src_ip_addr 组播源IP地址
 输出参数  : grp_member 组播组成员结构体 OR 空指针
*****************************************************************************/
OAL_STATIC hmac_m2u_grp_member_stru *hmac_m2u_find_member_src(hmac_m2u_grp_list_entry_stru *grp_list,
    osal_u8 *grp_member_addr, osal_u8 *src_ip_addr)
{
    hmac_m2u_grp_member_stru *grp_member;
    struct osal_list_head      *grp_member_entry;

    /* 遍历一个组播组，找到该组中src ip匹配的成员 */
    osal_list_for_each(grp_member_entry, &(grp_list->src_list)) {
        grp_member = (hmac_m2u_grp_member_stru *)osal_list_entry(grp_member_entry,
            hmac_m2u_grp_member_stru, member_entry);
        if ((oal_compare_mac_addr(grp_member_addr, grp_member->grp_member_mac) == 0) &&
            (osal_memcmp(src_ip_addr, grp_member->src_ip_addr, grp_member->src_ip_addr_len) == 0)) {
            return grp_member;
        }
    }
    return OAL_PTR_NULL;
}

/*****************************************************************************
 函 数 名  : hmac_m2u_find_member
 功能描述  : 根据成员的mac地址返回所需的组成员
 输入参数  : grp_list 组播组结构体; grp_member_addr 组播组成员IP地址
 输出参数  : grp_member 组播组成员结构体 OR 空指针
*****************************************************************************/
OAL_STATIC hmac_m2u_grp_member_stru *hmac_m2u_find_member(hmac_m2u_grp_list_entry_stru *grp_list,
    osal_u8 *grp_member_addr)
{
    hmac_m2u_grp_member_stru *grp_member;
    struct osal_list_head      *grp_member_entry;

    /* 遍历一个组播组，找到该组中地址匹配的成员 */
    osal_list_for_each(grp_member_entry, &(grp_list->src_list)) {
        grp_member = (hmac_m2u_grp_member_stru *)osal_list_entry(grp_member_entry,
            hmac_m2u_grp_member_stru, member_entry);
        if (oal_compare_mac_addr(grp_member_addr, grp_member->grp_member_mac) == 0) {
            return grp_member;
        }
    }

    return OAL_PTR_NULL;
}

/*****************************************************************************
 函 数 名  : hmac_m2u_find_group_list
 功能描述  : 根据组的mac地址返回所需的组
 输入参数  : hmac_vap vap结构体; hmac_m2u_list_update_stru *list_entry待查找组播组信息
 输出参数  : grp_list_member 组播组结构体 OR 空指针
*****************************************************************************/
OAL_STATIC hmac_m2u_grp_list_entry_stru *hmac_m2u_find_group_list(hmac_vap_stru *hmac_vap,
    hmac_m2u_list_update_stru *list_entry)
{
    hmac_m2u_snoop_list_stru         *snp_list;
    hmac_m2u_grp_list_entry_stru     *grp_list_member;
    struct osal_list_head              *grp_list_entry;
    hmac_m2u_stru                    *m2u = (hmac_m2u_stru *)(hmac_vap->m2u);

    if (m2u == OAL_PTR_NULL) {
        return OAL_PTR_NULL;
    }

    snp_list = &(m2u->m2u_snooplist);

    /* 遍历组播组链表，找到地址匹配的组播组 */
    osal_list_for_each(grp_list_entry, &(snp_list->grp_list)) {
        grp_list_member = (hmac_m2u_grp_list_entry_stru *)osal_list_entry(grp_list_entry,
            hmac_m2u_grp_list_entry_stru, grp_entry);
        if ((oal_compare_mac_addr(list_entry->grp_mac, grp_list_member->group_mac) == 0) &&
            (osal_memcmp(&(list_entry->outer_vlan_tag), &(grp_list_member->outer_vlan_tag),
            OAL_SIZEOF(mac_vlan_tag_stru)) == 0)) {
            return grp_list_member;
        }
    }

    return OAL_PTR_NULL;
}

/*****************************************************************************
 函 数 名  : hmac_m2u_create_grp_list
 功能描述  : 根据组播组的MAC地址创建一个组，如果此组存在则返回该组，否则重新创建
 输入参数  : hmac_vap vap结构体; hmac_m2u_list_update_stru *list_entry,待 操作节点信息
 输出参数  : grp_list_member 组播组结构体
*****************************************************************************/
OAL_STATIC hmac_m2u_grp_list_entry_stru *hmac_m2u_create_grp_list(hmac_vap_stru *hmac_vap,
    hmac_m2u_list_update_stru *list_entry)
{
    hmac_m2u_snoop_list_stru         *snp_list;
    hmac_m2u_grp_list_entry_stru     *grp_list_member = OAL_PTR_NULL;
    hmac_m2u_stru                    *m2u = (hmac_m2u_stru *)(hmac_vap->m2u);

    if (osal_unlikely(m2u == OAL_PTR_NULL)) {
        return grp_list_member;
    }

    snp_list        = &(m2u->m2u_snooplist);
    grp_list_member = hmac_m2u_find_group_list(hmac_vap, list_entry);
    if (grp_list_member == OAL_PTR_NULL) {
        if (snp_list->group_list_count >= MAX_NUM_OF_GROUP) {
            oam_warning_log2(0, OAM_SF_M2U,
                "vap_id[%d] {hmac_m2u_create_grp_list::snp_list->group_list_count is[%d].}",
                hmac_vap->vap_id, snp_list->group_list_count);
            return OAL_PTR_NULL;
        }
        grp_list_member = (hmac_m2u_grp_list_entry_stru *)oal_mem_alloc(OAL_MEM_POOL_ID_LOCAL,
            OAL_SIZEOF(hmac_m2u_grp_list_entry_stru), OAL_TRUE);
        if (osal_unlikely(grp_list_member == OAL_PTR_NULL)) {
            oam_warning_log1(0, OAM_SF_M2U, "vap_id[%d] {hmac_m2u_create_grp_list::grp_list_member null.}",
                hmac_vap->vap_id);
            return grp_list_member;
        }
        memset_s(grp_list_member, OAL_SIZEOF(hmac_m2u_grp_list_entry_stru), 0x0,
            OAL_SIZEOF(hmac_m2u_grp_list_entry_stru));
        if (memcpy_s(&(grp_list_member->outer_vlan_tag), OAL_SIZEOF(grp_list_member->outer_vlan_tag),
            &(list_entry->outer_vlan_tag), OAL_SIZEOF(mac_vlan_tag_stru)) != EOK) {
            oam_warning_log1(0, OAM_SF_M2U, "vap[%d]hmac_m2u_create_grp_list::memcpy vlan fail", hmac_vap->vap_id);
        }
        osal_list_add_tail(&(grp_list_member->grp_entry), &(snp_list->grp_list));
        oal_set_mac_addr(grp_list_member->group_mac, list_entry->grp_mac);
        OSAL_INIT_LIST_HEAD(&(grp_list_member->src_list));
        snp_list->group_list_count++;

        oam_warning_log2(0, OAM_SF_M2U, "vap_id[%d] {hmac_m2u_create_grp_list::create a new group.group num:[%d]}",
            hmac_vap->vap_id, snp_list->group_list_count);
    }
    return grp_list_member;
}

/*
 * 功能描述: 更新的报文中有组播源的场景
 */
OAL_STATIC osal_u32 hmac_m2u_update_snoop_list_with_multicast_source(hmac_m2u_grp_list_entry_stru *grp_list,
    hmac_m2u_list_update_stru *list_entry)
{
    hmac_m2u_grp_member_stru      *grp_member_list = OSAL_NULL;
    osal_u32                     ret = OAL_SUCC;

    oam_info_log3(0, OAM_SF_M2U,
        "{hmac_m2u_update_snoop_list::list_entry is assigned src_ip_addr the first 4 byte[%02x][%02x][%02x][xx]}",
        /* 0，1，2表示组播源节点的MAC地址位 */
        list_entry->src_ip_addr[0], list_entry->src_ip_addr[1], list_entry->src_ip_addr[2]);
        grp_member_list = hmac_m2u_find_member_src(grp_list, list_entry->new_member_mac,
        list_entry->src_ip_addr);
    if (grp_member_list != OAL_PTR_NULL) {
        grp_member_list->mode      = list_entry->cmd;
        grp_member_list->timestamp = list_entry->timestamp; // 有组播源时更新时间戳
    } else {
        ret = hmac_m2u_add_member_list(grp_list, list_entry);
    }

    return ret;
}

/*
 * 功能描述: 更新的报文中没有组播源的场景
 */
OAL_STATIC osal_u32 hmac_m2u_update_snoop_list_without_multicast_source(hmac_m2u_grp_list_entry_stru *grp_list,
    hmac_m2u_list_update_stru *list_entry)
{
    hmac_m2u_grp_member_stru      *grp_member_list = OSAL_NULL;
    hmac_m2u_stru                 *m2u = (hmac_m2u_stru *)(list_entry->hmac_vap->m2u);
    osal_u32                     ret = OAL_SUCC;

    grp_member_list = hmac_m2u_find_member(grp_list, list_entry->new_member_mac);
    if (grp_member_list != OAL_PTR_NULL) {
        /* 存在该节点，并且该组中的该节点的有指定的组播源，清空组中的所有该节点，如果更新节点的cmd为INC 再add */
        if (!OAL_IPV6_IS_UNSPECIFIED_ADDR(grp_member_list->src_ip_addr)) {
            hmac_m2u_remove_one_member(grp_list, list_entry->hmac_vap,
                list_entry->new_member_mac);
            if (grp_member_list->mode == HMAC_M2U_CMD_INCLUDE_LIST) {
                oam_info_log2(0, OAM_SF_M2U,
                    "vap_id[%d] {hmac_m2u_update_snoop_list::grp_member_list->mode is [%d].}",
                    list_entry->hmac_vap->vap_id, grp_member_list->mode);
                ret = hmac_m2u_add_member_list(grp_list, list_entry);
            }
        } else if (list_entry->cmd == HMAC_M2U_CMD_EXCLUDE_LIST) {
            /* 原组中的该节点未指定组播源，如果待更新节点的CMD为EXC，则删除原组播组中的该节点 */
            oam_info_log2(0, OAM_SF_M2U, "vap_id[%d] {hmac_m2u_update_snoop_list::list_entry->cmd is [%d].}",
                list_entry->hmac_vap->vap_id, list_entry->cmd);
            osal_list_del(&(grp_member_list->member_entry));
            oal_mem_free(grp_member_list, OAL_TRUE);

            oam_warning_log3(0, OAM_SF_M2U,
                "vap_id[%d]{hmac_m2u_update_snoop_list::current total_sta_num:[%d] ,sta_num:[%d]}",
                list_entry->hmac_vap->vap_id,
                m2u->m2u_snooplist.total_sta_num, grp_list->sta_num);

            grp_list->sta_num--;
            m2u->m2u_snooplist.total_sta_num--;

            oam_warning_log3(0, OAM_SF_M2U,
                "vap_id[%d] {hmac_m2u_update_snoop_list::remove one sta. current total_sta_num:[%d] ,sta_num:[%d]}",
                list_entry->hmac_vap->vap_id,
                m2u->m2u_snooplist.total_sta_num, grp_list->sta_num);
        }
    } else if (list_entry->cmd != HMAC_M2U_CMD_EXCLUDE_LIST) {
        /* 特定组中不存在该节点，并且更新节点的CMD不为EXC则ADD */
        oam_info_log2(0, OAM_SF_M2U,
            "vap_id[%d] {hmac_m2u_update_snoop_list:grp_member_list == NULL && list_entry->cmd is [%d]}",
            list_entry->hmac_vap->vap_id, list_entry->cmd);
        ret = hmac_m2u_add_member_list(grp_list, list_entry);
    }

    return ret;
}

/*****************************************************************************
 函 数 名  : hmac_m2u_Update_Snoop_List
 功能描述  : 根据接收到的用户信息更新链表
 输入参数  : list_entry  待更新节点结构体
*****************************************************************************/
osal_u32 hmac_m2u_update_snoop_list(hmac_m2u_list_update_stru *list_entry)
{
    hmac_m2u_grp_list_entry_stru  *grp_list;

    list_entry->timestamp = (osal_u32)oal_time_get_stamp_ms();
    grp_list = hmac_m2u_find_group_list(list_entry->hmac_vap, list_entry);
    if (grp_list == OAL_PTR_NULL) {
        if (list_entry->cmd == HMAC_M2U_CMD_EXCLUDE_LIST) {
            oam_warning_log1(0, OAM_SF_M2U, "vap_id[%d] {hmac_m2u_update_snoop_list::find_grp_list null.}",
                list_entry->hmac_vap->vap_id);
            return OAL_ERR_CODE_PTR_NULL;
        } else {
            grp_list = hmac_m2u_create_grp_list(list_entry->hmac_vap, list_entry);
            if (osal_unlikely(grp_list == OAL_PTR_NULL)) {
                oam_warning_log1(0, OAM_SF_M2U, "vap_id[%d] {hmac_m2u_update_snoop_list::create_grp_list null.}",
                    list_entry->hmac_vap->vap_id);
                return OAL_ERR_CODE_PTR_NULL;
            }
        }
    }

    if (!OAL_IPV6_IS_UNSPECIFIED_ADDR(list_entry->src_ip_addr)) {
        /* 更新信息中有组播源，如果链表中存在指定该组播源的该节点，则更新该节点的状态，否则重新创建加入链表 */
        return hmac_m2u_update_snoop_list_with_multicast_source(grp_list, list_entry);
    } else {
        /* 更新信息中没有组播源 */
        return hmac_m2u_update_snoop_list_without_multicast_source(grp_list, list_entry);
    }
}

/*****************************************************************************
 函 数 名  : hmac_m2u_show_snoop_deny_table
 功能描述  : 打印黑名单
 输入参数  : hmac_vap vap结构体
*****************************************************************************/
osal_void hmac_m2u_show_snoop_deny_table(hmac_vap_stru *hmac_vap)
{
    hmac_m2u_stru             *m2u;
    osal_u8                  idx;
    hmac_m2u_snoop_list_stru  *snp_list;

    if (osal_unlikely(hmac_vap == OAL_PTR_NULL)) {
        oam_error_log0(0, OAM_SF_M2U, "hmac_m2u_show_snoop_deny_table::hmac_vap is null}");
        return;
    }

    m2u = (hmac_m2u_stru *)(hmac_vap->m2u);

    if (osal_unlikely(m2u == OAL_PTR_NULL)) {
        oam_warning_log1(0, OAM_SF_M2U, "vap_id[%d] hmac_m2u_show_snoop_deny_table::m2u is null}",
            hmac_vap->vap_id);
        return;
    }

    snp_list = &(m2u->m2u_snooplist);

    if ((snp_list->deny_count_ipv4 == 0) && (snp_list->deny_count_ipv6 == 0)) {
        return;
    }
    for (idx = 0; idx < snp_list->deny_count_ipv4; idx++) {
        /* 打印黑名单的IPv4地址 */
        char  temp[MAX_STR_SIZE_OF_U8]; // 最多3个字符 + 1个'\0' = 4
        int len = snprintf_s(temp, MAX_STR_SIZE_OF_U8, MAX_STR_SIZE_OF_U8 - 1,
            "%u", ((snp_list->deny_group[idx]) & 0xff)); // itoa得到字符串
        if (len >= 0) {
            (osal_void)memset_s(temp, sizeof(temp), 'x', (size_t)len); // 隐私保护处理,用'x'掩去
            oam_warning_log3(0, OAM_SF_M2U, "{hmac_m2u_show_snoop_deny_table:: deny addr_ipv4 = %d.%d.%d.x}",
                (osal_u32)((snp_list->deny_group[idx] >> 24) & 0xff), /* 24表示黑名单按右偏移取值 */
                (osal_u32)((snp_list->deny_group[idx] >> 16) & 0xff), /* 16表示黑名单按右偏移取值 */
                (osal_u32)((snp_list->deny_group[idx] >> 8) & 0xff)); /* 8表示黑名单按右偏移取值 */
        }
    }
    for (idx = 0; idx < snp_list->deny_count_ipv6; idx++) {
        /* 打印黑名单的IPv6地址 */
        oam_warning_log1(0, OAM_SF_M2U,
            "{hmac_m2u_show_snoop_deny_table:: deny addr_ipv6 = [%08x][xxxxxxxx][xxxxxxxx][xxxxxxxx]}",
            oal_host2net_long(*(osal_u32*)(&snp_list->deny_group_ipv6[idx][0])));
    }
    oam_warning_log0(0, OAM_SF_M2U, "CCPRIV m2u_snoop_deny_table OK}");
}

/*****************************************************************************
 函 数 名  : hmac_m2u_snoop_is_denied
 功能描述  : 所加入组播组的IP地址是否在黑名单内
 输入参数  : hmac_vap vap结构体; grpaddr 组播组的IP地址
 输出参数  : OAL_FALSE OR OAL_TRUE
*****************************************************************************/
OAL_STATIC osal_u32 hmac_m2u_snoop_is_denied_ipv4(hmac_vap_stru *hmac_vap, osal_u32 grpaddr)
{
    osal_u8                 idx;
    hmac_m2u_snoop_list_stru *snp_list;
    hmac_m2u_stru            *m2u;

    if (osal_unlikely(hmac_vap == OAL_PTR_NULL)) {
        oam_error_log0(0, OAM_SF_M2U, "hmac_m2u_snoop_is_denied_ipv4::hmac_vap is null}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    m2u = (hmac_m2u_stru *)(hmac_vap->m2u);

    if (m2u == OAL_PTR_NULL) {
        oam_warning_log1(0, OAM_SF_M2U, "vap_id[%d] hmac_m2u_snoop_is_denied_ipv4::m2u is null}",
            hmac_vap->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    snp_list = &(m2u->m2u_snooplist);

    if (snp_list->deny_count_ipv4 == 0) {
        return OAL_FALSE;
    }
    for (idx = 0; idx < snp_list->deny_count_ipv4; idx++) {
        if (grpaddr != snp_list->deny_group[idx]) {
            continue;
        }
        return OAL_TRUE;
    }
    return OAL_FALSE;
}

/*****************************************************************************
 函 数 名  : hmac_m2u_snoop_is_denied_ipv6
 功能描述  : 所加入组播组的IPv6地址是否在黑名单内
 输入参数  : hmac_vap vap结构体; grpaddr 组播组的IPv6地址
 输出参数  : OAL_FALSE OR OAL_TRUE
*****************************************************************************/
OAL_STATIC osal_u32 hmac_m2u_snoop_is_denied_ipv6(hmac_vap_stru *hmac_vap, osal_u8 *grpaddr)
{
    osal_u8                 idx;
    hmac_m2u_snoop_list_stru *snp_list;
    hmac_m2u_stru            *m2u;

    if (osal_unlikely(hmac_vap == OAL_PTR_NULL)) {
        oam_error_log0(0, OAM_SF_M2U, "hmac_m2u_snoop_is_denied_ipv6::hmac_vap is null}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    m2u = (hmac_m2u_stru *)(hmac_vap->m2u);

    if (m2u == OAL_PTR_NULL) {
        oam_warning_log1(0, OAM_SF_M2U, "vap_id[%d] hmac_m2u_snoop_is_denied_ipv6::m2u is null}",
            hmac_vap->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    snp_list = &(m2u->m2u_snooplist);
    if (snp_list->deny_count_ipv6 == 0) {
        return OAL_FALSE;
    }
    for (idx = 0; idx < snp_list->deny_count_ipv6; idx++) {
        if (osal_memcmp(grpaddr, snp_list->deny_group_ipv6[idx], OAL_IPV6_ADDR_SIZE) == 0) {
            return OAL_TRUE;
        }
    }
    return OAL_FALSE;
}
/*****************************************************************************
 函 数 名  : hmac_m2u_clear_deny_table
 功能描述  : 清空黑名单
 输入参数  : hmac_vap vap结构体
 输出参数  : 无
*****************************************************************************/
osal_void hmac_m2u_clear_deny_table(hmac_vap_stru *hmac_vap)
{
    hmac_m2u_snoop_list_stru *snp_list;
    hmac_m2u_stru            *m2u;

    if (osal_unlikely(hmac_vap == OAL_PTR_NULL)) {
        oam_error_log0(0, OAM_SF_M2U, "hmac_m2u_clear_deny_table::hmac_vap is null}");
        return;
    }

    m2u = (hmac_m2u_stru *)(hmac_vap->m2u);

    if (osal_unlikely(m2u == OAL_PTR_NULL)) {
        oam_warning_log1(0, OAM_SF_M2U, "vap_id[%d] hmac_m2u_clear_deny_table::m2u is null}",
            hmac_vap->vap_id);
        return;
    }

    snp_list = &(m2u->m2u_snooplist);
    snp_list->deny_count_ipv4 = DEFAULT_IPV4_DENY_GROUP_COUNT;
    snp_list->deny_count_ipv6 = DEFAULT_IPV6_DENY_GROUP_COUNT;
    oam_warning_log0(0, OAM_SF_M2U, "CCPRIV m2u_snoop_deny_table OK}");
    return;
}

/*****************************************************************************
 功能描述  : 判断黑名单成员是否已经存在
*****************************************************************************/
OAL_STATIC osal_u8 ipv4_deny_exist(const hmac_m2u_snoop_list_stru *snp_list, const osal_u32 *pul_grpaddr)
{
    osal_u8                ret = OSAL_FALSE;
    osal_u8                idx;

    for (idx = 0; idx < snp_list->deny_count_ipv4; idx++) {
        if (*pul_grpaddr == snp_list->deny_group[idx]) { // 判断黑名单与列表是否相同
            ret = OSAL_TRUE;
            break;
        }
    }
    return ret;
}

OAL_STATIC osal_u8 ipv6_deny_exist(hmac_m2u_snoop_list_stru *snp_list, const osal_u8 *grpaddr)
{
    osal_u8                ret = OSAL_FALSE;
    osal_u8                idx;

    for (idx = 0; idx < snp_list->deny_count_ipv6; idx++) {
        if (osal_memcmp(grpaddr, snp_list->deny_group_ipv6[idx], OAL_IPV6_ADDR_SIZE) == 0) { // 判断黑名单与列表是否相同
            ret = OSAL_TRUE;
            break;
        }
    }
    return ret;
}

/*****************************************************************************
 函 数 名  : hmac_m2u_add__snoop_deny_entry
 功能描述  : 增加黑名单成员
 输入参数  : hmac_vap vap结构体; grpaddr 组播组的IP地址
*****************************************************************************/
osal_void hmac_m2u_add_snoop_ipv4_deny_entry(hmac_vap_stru *hmac_vap, osal_u32 *pul_grpaddr)
{
    osal_u8                 idx;
    hmac_m2u_snoop_list_stru *snp_list;
    hmac_m2u_stru            *m2u;

    if (osal_unlikely(hmac_vap == OAL_PTR_NULL)) {
        oam_error_log0(0, OAM_SF_M2U, "hmac_m2u_add_snoop_ipv4_deny_entry::hmac_vap is null}");
        return;
    }

    m2u = (hmac_m2u_stru *)(hmac_vap->m2u);

    if (osal_unlikely(m2u == OAL_PTR_NULL)) {
        oam_warning_log1(0, OAM_SF_M2U, "vap_id[%d] hmac_m2u_add_snoop_ipv4_deny_entry::m2u is null}",
            hmac_vap->vap_id);
        return;
    }

    snp_list = &(m2u->m2u_snooplist);
    idx = snp_list->deny_count_ipv4;
    if (ipv4_deny_exist(snp_list, pul_grpaddr) != 0) { // 列表中已有
        oam_warning_log0(0, OAM_SF_M2U, "+NOTICE:ADD ERROR, ALREADY EXISTS");
        return;
    }
    /* ipv4和ipv6目前的黑名单个数相加超过最大黑名单个数 */
    if (idx + snp_list->deny_count_ipv6 >= HMAC_M2U_GRPADDR_FILTEROUT_NUM) {
        oam_warning_log0(0, OAM_SF_M2U, "+NOTICE:ADD ERROR, EXCEEDED LIMIT");
        return;
    }
    snp_list->deny_count_ipv4++;
    snp_list->deny_group[idx] = *pul_grpaddr;
    oam_warning_log0(0, OAM_SF_M2U, "CCPRIV m2u_snoop_deny_table OK}");
    return;
}

/*****************************************************************************
 函 数 名  : hmac_m2u_add_snoop_ipv6_deny_entry
 功能描述  : 增加IPV6黑名单成员
 输入参数  : hmac_vap vap结构体; grpaddr 组播组的IP地址
*****************************************************************************/
osal_void hmac_m2u_add_snoop_ipv6_deny_entry(hmac_vap_stru *hmac_vap, osal_u8 *grpaddr)
{
    osal_u8                 idx;
    hmac_m2u_snoop_list_stru *snp_list;
    hmac_m2u_stru            *m2u;

    if (osal_unlikely(hmac_vap == OAL_PTR_NULL)) {
        oam_error_log0(0, OAM_SF_M2U, "hmac_m2u_add_snoop_ipv6_deny_entry::hmac_vap is null}");
        return;
    }

    m2u = (hmac_m2u_stru *)(hmac_vap->m2u);

    if (osal_unlikely(m2u == OAL_PTR_NULL)) {
        oam_warning_log1(0, OAM_SF_M2U, "vap_id[%d] hmac_m2u_add_snoop_ipv6_deny_entry::m2u is null}",
            hmac_vap->vap_id);
        return;
    }
    snp_list = &(m2u->m2u_snooplist);
    idx = snp_list->deny_count_ipv6;
    if (ipv6_deny_exist(snp_list, grpaddr) != 0) { // 列表中已有
        oam_warning_log0(0, OAM_SF_M2U, "+NOTICE:ADD ERROR, ALREADY EXISTS");
        return;
    }
    /* ipv4和ipv6目前的黑名单个数相加超过最大黑名单个数 */
    if (idx + snp_list->deny_count_ipv4 >= HMAC_M2U_GRPADDR_FILTEROUT_NUM) {
        oam_warning_log0(0, OAM_SF_M2U, "+NOTICE:ADD ERROR, EXCEEDED LIMIT");
        return;
    }
    snp_list->deny_count_ipv6++;
    if (memcpy_s(&snp_list->deny_group_ipv6[idx], OAL_SIZEOF(snp_list->deny_group_ipv6[idx]),
        grpaddr, OAL_IPV6_ADDR_SIZE) != EOK) {
        oam_warning_log1(0, OAM_SF_M2U, "vap[%d]hmac_m2u_add_snoop_ipv6_deny_entry:memcpy addr fail", hmac_vap->vap_id);
    }
    oam_warning_log0(0, OAM_SF_M2U, "CCPRIV m2u_snoop_deny_table OK}");
    return;
}

/*****************************************************************************
 函 数 名  : hmac_m2u_del_ipv4_deny_entry
 功能描述  : 删除黑名单成员
 输入参数  : hmac_vap vap结构体; grpaddr 组播组的IP地址
*****************************************************************************/
osal_void hmac_m2u_del_ipv4_deny_entry(hmac_vap_stru *hmac_vap, const osal_u32 *pul_grpaddr)
{
    osal_u8                 idx;
    osal_u8                 num;
    hmac_m2u_snoop_list_stru *snp_list;
    hmac_m2u_stru            *m2u;

    if (osal_unlikely(hmac_vap == OAL_PTR_NULL)) {
        oam_error_log0(0, OAM_SF_M2U, "hmac_m2u_del_ipv4_deny_entry::hmac_vap is null}");
        return;
    }

    m2u = (hmac_m2u_stru *)(hmac_vap->m2u);

    if (osal_unlikely(m2u == OAL_PTR_NULL)) {
        oam_warning_log1(0, OAM_SF_M2U, "vap_id[%d] hmac_m2u_del_ipv4_deny_entry::m2u is null}",
            hmac_vap->vap_id);
        return;
    }

    snp_list = &(m2u->m2u_snooplist);
    num       = snp_list->deny_count_ipv4;

    if (num <= DEFAULT_IPV4_DENY_GROUP_COUNT) {
        oam_warning_log0(0, OAM_SF_M2U, "+NOTICE:DEL ERROR");
        return;
    }
    if (*pul_grpaddr == snp_list->deny_group[0]) { // 删除默认地址
        oam_warning_log0(0, OAM_SF_M2U, "+NOTICE:DEL ERROR, DEFAULE ADD");
        return;
    }
    for (idx = DEFAULT_IPV4_DENY_GROUP_COUNT; idx < num; idx++) {
        if (*pul_grpaddr == snp_list->deny_group[idx]) { // 4字节，共32位
            break;
        }
    }

    if (idx < num) {
        snp_list->deny_group[idx] = snp_list->deny_group[num - 1];
        snp_list->deny_count_ipv4--;
    }
    oam_warning_log0(0, OAM_SF_M2U, "CCPRIV m2u_snoop_deny_table OK}");
    return;
}

/*****************************************************************************
 函 数 名  : hmac_m2u_del_ipv6_deny_entry
 功能描述  : 删除IPV6黑名单成员
 输入参数  : hmac_vap vap结构体; grpaddr 组播组的IP地址
*****************************************************************************/
osal_void hmac_m2u_del_ipv6_deny_entry(hmac_vap_stru *hmac_vap, osal_u8 *grpaddr)
{
    osal_u8                 idx;
    osal_u8                 num;
    hmac_m2u_snoop_list_stru *snp_list;
    hmac_m2u_stru            *m2u;

    if (osal_unlikely(hmac_vap == OAL_PTR_NULL)) {
        oam_error_log0(0, OAM_SF_M2U, "hmac_m2u_del_ipv6_deny_entry::hmac_vap is null}");
        return;
    }

    m2u = (hmac_m2u_stru *)(hmac_vap->m2u);

    if (osal_unlikely(m2u == OAL_PTR_NULL)) {
        oam_warning_log1(0, OAM_SF_M2U, "vap_id[%d] hmac_m2u_del_ipv6_deny_entry::m2u is null}",
            hmac_vap->vap_id);
        return;
    }

    snp_list = &(m2u->m2u_snooplist);
    num       = snp_list->deny_count_ipv6;

    if (num <= DEFAULT_IPV6_DENY_GROUP_COUNT) {
        oam_warning_log0(0, OAM_SF_M2U, "+NOTICE:DEL ERROR");
        return;
    }
    if (osal_memcmp(grpaddr, snp_list->deny_group_ipv6[0], OAL_IPV6_ADDR_SIZE) == 0) {
        oam_warning_log0(0, OAM_SF_M2U, "+NOTICE:DEL ERROR, DEFAULE ADD");
        return;
    }
    for (idx = DEFAULT_IPV6_DENY_GROUP_COUNT; idx < num; idx++) {
        if (osal_memcmp(grpaddr, snp_list->deny_group_ipv6[idx], OAL_IPV6_ADDR_SIZE) == 0) { // 16字节
            break;
        }
    }

    if (idx < num) {
        if (memcpy_s(&snp_list->deny_group_ipv6[idx], OAL_SIZEOF(snp_list->deny_group_ipv6[idx]),
            &snp_list->deny_group_ipv6[num - 1], OAL_IPV6_ADDR_SIZE) != EOK) {
            oam_warning_log1(0, OAM_SF_M2U, "vap[%d]hmac_m2u_del_ipv6_deny_entry:memcpy addr fail", hmac_vap->vap_id);
        }
        snp_list->deny_count_ipv6--;
    }
    oam_warning_log0(0, OAM_SF_M2U, "CCPRIV m2u_snoop_deny_table OK}");
    return;
}

OAL_STATIC osal_void hmac_m2u_print_grp_member_ip(const hmac_m2u_grp_member_stru *grp_member)
{
    if (grp_member->src_ip_addr_len == OAL_IPV6_ADDR_SIZE) {
        oam_info_log3(0, OAM_SF_M2U,
            "group_src_ip_addr = [%02x]:[%02x]:[%02x]:[xx]:[xx]:[xx]:[xx]:[xx]",
            oal_net2host_short(*(osal_u16 *)(&grp_member->src_ip_addr[0])),
            oal_net2host_short(*(osal_u16 *)(&grp_member->src_ip_addr[2])),    /* 2:组播组成员ip地址位 */
            oal_net2host_short(*(osal_u16 *)(&grp_member->src_ip_addr[4])));   /* 4:组播组成员ip地址位 */
    } else {
        oam_info_log3(0, OAM_SF_M2U,
            "group_src_ip_addr = [%d].[%d].[%d].[xxx]",
            grp_member->src_ip_addr[0], grp_member->src_ip_addr[1],
            grp_member->src_ip_addr[2]);  /* 2:组播组成员ip地址位 */
    }
}

OAL_STATIC osal_void hmac_m2u_print_grp_member_list(const hmac_m2u_grp_list_entry_stru *grp_list_member)
{
    hmac_m2u_grp_member_stru *grp_member = OAL_PTR_NULL;
    struct osal_list_head *member_entry_temp = OAL_PTR_NULL;
    oal_snoop_group_stru snoop_group;
    struct osal_list_head *grp_member_entry = OAL_PTR_NULL;

    (osal_void)memset_s(&snoop_group, OAL_SIZEOF(oal_snoop_group_stru), 0, OAL_SIZEOF(oal_snoop_group_stru));

    osal_list_for_each_safe(grp_member_entry, member_entry_temp, &(grp_list_member->src_list)) {
        grp_member = (hmac_m2u_grp_member_stru *)osal_list_entry(grp_member_entry, hmac_m2u_grp_member_stru,
            member_entry);
        /* 打印组播组内成员的src ip地址和 mac地址 */
        hmac_m2u_print_grp_member_ip(grp_member);

        /* 打印 0 1 2 3 位mac */
        oam_warning_log4(0, OAM_SF_M2U,
            "sta_mac and mode = [%02x]:[%02x]:[%02x]:[%02x]:[xx]:[xx]",
            grp_member->grp_member_mac[0], grp_member->grp_member_mac[1],
            grp_member->grp_member_mac[2],  /* 打印 0 1 2 3 位mac */
            grp_member->grp_member_mac[3]); /* 打印 0 1 2 3 位mac */
        oam_warning_log1(0, OAM_SF_M2U, "mode %d", grp_member->mode);
    }
}

/* 输出snoop表所有成员 */
osal_u32 hmac_m2u_print_all_snoop_list(hmac_vap_stru *hmac_vap, oal_snoop_all_group_stru *snoop_all_grp)
{
    hmac_m2u_snoop_list_stru *snp_list = OAL_PTR_NULL;
    hmac_m2u_grp_list_entry_stru *grp_list_member = OAL_PTR_NULL;
    struct osal_list_head *grp_list_entry = OAL_PTR_NULL;
    hmac_m2u_stru *m2u = OAL_PTR_NULL;
    osal_u16 group_cnt = 0;

    if (osal_unlikely(hmac_vap == OAL_PTR_NULL)) {
        oam_warning_log0(0, OAM_SF_M2U, "hmac_m2u_print_all_snoop_list::hmac_vap is null}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    m2u = (hmac_m2u_stru *)(hmac_vap->m2u);
    if (osal_unlikely(m2u == OAL_PTR_NULL)) {
        oam_warning_log1(0, OAM_SF_M2U, "vap_id[%d] hmac_m2u_print_all_snoop_list::m2u is null}",
            hmac_vap->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    snp_list = &(m2u->m2u_snooplist);
    if (snp_list == OAL_PTR_NULL) {
        oam_warning_log1(0, OAM_SF_M2U, "vap_id[%d] hmac_m2u_print_all_snoop_list::snp_list is null}",
            hmac_vap->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    oam_warning_log4(0, OAM_SF_M2U, "sta_num: [%d] group_num:[%d] snoop_enable:[%d] vap_mode:[%d]",
        snp_list->total_sta_num, snp_list->group_list_count, m2u->snoop_enable, hmac_vap->vap_mode);

    if ((hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_AP) && (m2u->snoop_enable == OAL_TRUE)) {
        if (snoop_all_grp != OAL_PTR_NULL) {
            snoop_all_grp->group_cnt = snp_list->group_list_count;
            group_cnt = 0;
        }
        osal_list_for_each(grp_list_entry, &(snp_list->grp_list)) {
            grp_list_member = (hmac_m2u_grp_list_entry_stru *)osal_list_entry(grp_list_entry,
                hmac_m2u_grp_list_entry_stru, grp_entry);
            /* 打印组播组的mac地址 */
            oam_warning_log4(0, OAM_SF_M2U,
                "group addr = [%02x]:[%02x]:[%02x]:[%02x]:[xx]:[xx]",
                grp_list_member->group_mac[0], grp_list_member->group_mac[1],
                grp_list_member->group_mac[2], grp_list_member->group_mac[3]); /* 打印 0 1 2 3 位mac */
            oam_warning_log1(0, OAM_SF_M2U, "vlan tag = 0x%x", *(osal_u32 *)(&(grp_list_member->outer_vlan_tag)));
            hmac_m2u_print_grp_member_list(grp_list_member);
            if (snoop_all_grp != OAL_PTR_NULL) {
                group_cnt++;
            }
        }
    } else {
        oam_warning_log2(0, OAM_SF_M2U, "{hmac_m2u_print_all_snoop_list::snoop_enable vap_mode = [%d].[%d]}",
            m2u->snoop_enable, hmac_vap->vap_mode);
    }
    oam_warning_log0(0, OAM_SF_M2U, "CCPRIV m2u_snoop_list OK}");
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_m2u_get_group_mac
 功能描述  : 根据组播组ip地址获取组播组mac地址
 输入参数  : group_ip指向ip地址的指针; ip_len ip地址长度
 输出参数  : group_mac 存储组播组mac地址的指针
*****************************************************************************/
OAL_STATIC osal_void hmac_m2u_get_group_mac(osal_u8 *group_mac, osal_u8 *group_ip, osal_u8 ip_len)
{
    osal_u8 *group_ip_tmp = group_ip;
    const osal_u8 group_mac_len_ipv6 = 4; /* 取最后4个字节 */
    const osal_u8 group_mac_len_ipv4 = 3; /* 3:组播组成员mac地址位 */

    if (ip_len == OAL_IPV6_ADDR_SIZE) {
        // ipv6下，组播mac最后4字节由IP地址映射而来
        group_mac[0] = 0x33;
        group_mac[1] = 0x33;
        group_mac += 2; /* 2:跳过前面已赋值字节 */
        group_ip_tmp += 12; // 12取最后4个字节
        if (memcpy_s(group_mac, group_mac_len_ipv6, group_ip_tmp, 4) != EOK) { /* 取最后4个字节 */
            oam_warning_log0(0, OAM_SF_M2U, "hmac_m2u_get_group_mac:memcpy addr fail");
        }
        oam_info_log4(0, OAM_SF_M2U,
                      "{hmac_m2u_get_group_mac::ipv6 multicast packets are restored to unicast , \
                      the group mac [%02x][%02x][%02x][%02x][xx][xx]}", \
                      group_mac[0], group_mac[1], group_mac[2], group_mac[3]); /* 打印第0、1、2、3位mac */
    } else {
        // ipv4下，组播mac最后23bit由IP地址映射而来
        group_mac[0] = 0x01;
        group_mac[1] = 0x0;
        group_mac[2] = 0x5e; /* 2:组播组成员mac地址位 */
        group_ip_tmp += 1;
        if (memcpy_s(group_mac + 3, group_mac_len_ipv4, group_ip_tmp, 3) != EOK) { /* 3:组播组成员mac地址位 */
            oam_warning_log0(0, OAM_SF_M2U, "hmac_m2u_get_group_mac:memcpy addr fail");
        }
        group_mac[3] &= 0x7f; /* 3:组播组成员mac地址位 */
        oam_info_log4(0, OAM_SF_M2U,
                      "{hmac_m2u_get_group_mac::ipv4 multicast packets are restored to unicast , \
                      the group mac [%02x][%02x][%02x][%02x][xx][xx]}", \
                      group_mac[0], group_mac[1], group_mac[2], group_mac[3]); /* 打印第0、1、2、3位mac */
    }
}

/*****************************************************************************
 函 数 名  : hmac_m2u_igmp_v1v2_update
 功能描述  : IGMP V1 V2 报文更新snoop链表
 输入参数  : hmac_vap vap结构体; list_entry 待更新的节点结构体; igmp IGMP报文结构体
*****************************************************************************/
osal_u32 hmac_m2u_igmp_v1v2_update(hmac_vap_stru *hmac_vap, hmac_m2u_list_update_stru *list_entry,
    mac_igmp_header_stru *igmp)
{
    osal_u32           group_addr;                    /* to hold group address from group record */
    osal_u32           ul_ret = OAL_SUCC;

    group_addr = igmp->group;
    if (hmac_m2u_snoop_is_denied_ipv4(hmac_vap, oal_net2host_long(group_addr)) != 0) {
        oam_warning_log3(0, OAM_SF_M2U, "{hmac_m2u_igmp_v1v2_update::group_addr[%02x].[%02x].[%02x].[xx] is denied}}",
            (osal_u32)((group_addr) & 0xff), (osal_u32)((group_addr>> 8)  & 0xff), /* 8:偏移取值 */
            (osal_u32)((group_addr>> 16) & 0xff)); /* 16，24:偏移取值 */
        return ul_ret;
    }

    if (igmp->type != MAC_IGMPV2_LEAVE_TYPE) {
        list_entry->cmd = HMAC_M2U_CMD_INCLUDE_LIST;
    } else {
        list_entry->cmd = HMAC_M2U_CMD_EXCLUDE_LIST;
    }

    list_entry->src_ip_addr_len = OAL_IPV4_ADDR_SIZE;
    hmac_m2u_get_group_mac(list_entry->grp_mac, (osal_u8 *)(&group_addr), OAL_IPV4_ADDR_SIZE);
    memset_s(list_entry->src_ip_addr, OAL_IPV6_ADDR_SIZE, 0, OAL_IPV6_ADDR_SIZE);
    ul_ret = hmac_m2u_update_snoop_list(list_entry);
    return ul_ret;
}

OAL_STATIC osal_void hmac_m2u_exc_update(hmac_vap_stru *hmac_vap, osal_u16 no_srec,
    hmac_m2u_list_update_stru *list_entry, osal_u32 *ret)
{
    hmac_m2u_grp_list_entry_stru *grp_list = OAL_PTR_NULL;

    list_entry->cmd = HMAC_M2U_CMD_EXCLUDE_LIST;
    grp_list = hmac_m2u_find_group_list(hmac_vap, list_entry);
    /* 更新组播组内成员的状态，如果组播组内存在该目标成员，则清空目标成员 */
    if (grp_list == OAL_PTR_NULL) {
        oam_info_log1(0, OAM_SF_M2U, "vap_id[%d] {hmac_m2u_exc_update::grp_list is null.}",
            hmac_vap->vap_id);
    } else {
        hmac_m2u_remove_one_member(grp_list, hmac_vap, list_entry->new_member_mac);
    }

    /* 更新组播组内成员的状态，如果组播组内不存在该目标成员，该成员inc所有src ip */
    if (no_srec == 0) {
        oam_info_log2(0, OAM_SF_M2U, "vap_id[%d] {hmac_m2u_exc_update::not exist the user no_srec is %d}",
            hmac_vap->vap_id, no_srec);
        (osal_void)memset_s(list_entry->src_ip_addr, OAL_IPV6_ADDR_SIZE, 0, OAL_IPV6_ADDR_SIZE);
        list_entry->cmd = HMAC_M2U_CMD_INCLUDE_LIST;
        *ret = hmac_m2u_update_snoop_list(list_entry);
    }
}

OAL_STATIC osal_void hmac_m2u_inc_update(hmac_vap_stru *hmac_vap, hmac_m2u_list_update_stru *list_entry)
{
    hmac_m2u_grp_list_entry_stru *grp_list = OAL_PTR_NULL;

    list_entry->cmd = HMAC_M2U_CMD_INCLUDE_LIST;
    grp_list = hmac_m2u_find_group_list(hmac_vap, list_entry);
    if (grp_list == OAL_PTR_NULL) {
        oam_info_log1(0, OAM_SF_M2U, "vap_id[%d] {hmac_m2u_inc_update::grp_list is null.}",
            hmac_vap->vap_id);
    } else {
        hmac_m2u_remove_one_member(grp_list, hmac_vap, list_entry->new_member_mac);
    }
}

OAL_STATIC osal_void hmac_m2u_delete_old_source_records(hmac_vap_stru *hmac_vap, hmac_m2u_list_update_stru *list_entry,
    hmac_m2u_stru *m2u)
{
    hmac_m2u_grp_list_entry_stru *grp_list = OAL_PTR_NULL;
    hmac_m2u_grp_member_stru *grp_member_list = OAL_PTR_NULL;

    grp_list = hmac_m2u_find_group_list(hmac_vap, list_entry);
    if (grp_list == OAL_PTR_NULL) {
        return;
    }
    grp_member_list = hmac_m2u_find_member_src(grp_list, list_entry->new_member_mac, list_entry->src_ip_addr);
    if (grp_member_list == OAL_PTR_NULL) {
        return;
    }
    osal_list_del(&grp_member_list->member_entry);
    oal_mem_free(grp_member_list, OAL_TRUE);

    oam_warning_log3(0, OAM_SF_M2U,
        "vap_id[%d] {hmac_m2u_delete_old_source_records:: current total_sta_num:[%d] ,sta_num:[%d]}",
        hmac_vap->vap_id, m2u->m2u_snooplist.total_sta_num, grp_list->sta_num);

    grp_list->sta_num--;
    m2u->m2u_snooplist.total_sta_num--;

    oam_warning_log3(0, OAM_SF_M2U,
        "vap_id[%d]{hmac_m2u_delete_old_source_records::remove one sta current total_sta_num[%d]sta_num[%d]}",
        hmac_vap->vap_id, m2u->m2u_snooplist.total_sta_num, grp_list->sta_num);
}

OAL_STATIC osal_void hmac_m2u_igmp_v3_update_list(hmac_vap_stru *hmac_vap, hmac_m2u_list_update_stru *list_entry,
    hmac_m2u_stru *m2u, mac_igmp_v3_grec_stru *grec, osal_u32 *ret)
{
    osal_u32 *src_addr = OAL_PTR_NULL; /* 组播源的IP地址 */
    osal_u16 no_srec;                  /* no of source records */

    no_srec = oal_host2net_short(grec->grec_nsrcs);

    /* IGMP V3 exc的处理 */
    if ((grec->grec_type == IGMPV3_CHANGE_TO_EXCLUDE) || (grec->grec_type == IGMPV3_MODE_IS_EXCLUDE)) {
        oam_info_log2(0, OAM_SF_M2U, "vap_id[%d] {hmac_m2u_igmp_v3_update::grec->grec_type is %d.}",
            hmac_vap->vap_id, grec->grec_type);
        hmac_m2u_exc_update(hmac_vap, no_srec, list_entry, ret);
    } else if ((grec->grec_type == IGMPV3_CHANGE_TO_INCLUDE) || (grec->grec_type == IGMPV3_MODE_IS_INCLUDE)) {
        /* IGMP V3 INC的处理 */
        hmac_m2u_inc_update(hmac_vap, list_entry);
    } else if (grec->grec_type == IGMPV3_ALLOW_NEW_SOURCES) {
        list_entry->cmd = HMAC_M2U_CMD_INCLUDE_LIST;
    }

    src_addr = (osal_u32 *)((osal_u8 *)grec + OAL_SIZEOF(mac_igmp_v3_grec_stru));

    /* 同一组播组内不同src ip的链表更新 */
    while (no_srec != 0) {
        *(osal_u32 *)(list_entry->src_ip_addr) = *src_addr;
        if (grec->grec_type != IGMPV3_BLOCK_OLD_SOURCES) {
            *ret = hmac_m2u_update_snoop_list(list_entry);
        } else { /* block old source时清空该成员 */
            hmac_m2u_delete_old_source_records(hmac_vap, list_entry, m2u);
        }
        src_addr++;
        no_srec--;
    }
}

/* IGMP V3报文更新snoop链表 */
OAL_STATIC osal_u32 hmac_m2u_igmp_v3_update(hmac_vap_stru *hmac_vap, hmac_m2u_list_update_stru *list_entry,
    mac_igmp_v3_report_stru *igmpr3)
{
    osal_u32 group_ip;                          /* to hold group ip address from group record */
    osal_u16 no_grec;                           /* no of group records  */
    mac_igmp_v3_grec_stru *grec = OAL_PTR_NULL; /* igmp group record */
    hmac_m2u_stru *m2u = OAL_PTR_NULL;
    osal_u32 ret = OAL_SUCC;

    no_grec = oal_host2net_short(igmpr3->ngrec);
    grec = (mac_igmp_v3_grec_stru *)(igmpr3 + 1);
    m2u = (hmac_m2u_stru *)(list_entry->hmac_vap->m2u);

    if (no_grec == 0) {
        oam_warning_log1(0, OAM_SF_M2U, "vap_id[%d] {hmac_m2u_igmp_v3_update::no_grec is 0.}",
            hmac_vap->vap_id);
        return OAL_FAIL;
    }

    while (no_grec != 0) {
        list_entry->cmd = grec->grec_type;
        group_ip = grec->grec_group_ip; // 获取组播组ip地址

        if (hmac_m2u_snoop_is_denied_ipv4(hmac_vap, oal_net2host_long(group_ip)) != 0) {
            oam_warning_log3(0, OAM_SF_M2U,
                "{hmac_m2u_igmp_v3_update::group_addr [%02x].[%02x].[%02x].[xx] is denied}}",
                (osal_u32)((group_ip >> 24) & 0xff), (osal_u32)((group_ip >> 16) & 0xff), /* 24，16:偏移取值 */
                (osal_u32)((group_ip >> 8) & 0xff));         /* 8:偏移取值 */
            /* move the grec to next group record */
            grec = (mac_igmp_v3_grec_stru *)((osal_u8 *)grec + igmpv3_grp_rec_len(grec));
            no_grec--;
            continue;
        }
        /* 非IGMPV3六种cmd的异常处理 */
        if (!is_igmpv3_mode(grec->grec_type)) {
            oam_info_log2(0, OAM_SF_M2U,
                "vap_id[%d] {hmac_m2u_igmp_v3_update::grec_type is [%x] not inside the six cmd.}",
                hmac_vap->vap_id, grec->grec_type);
            /* move the grec to next group record */
            grec = (mac_igmp_v3_grec_stru *)((osal_u8 *)grec + igmpv3_grp_rec_len(grec));
            no_grec--;
            continue;
        }

        hmac_m2u_get_group_mac(list_entry->grp_mac, (osal_u8 *)(&group_ip), OAL_IPV4_ADDR_SIZE);
        list_entry->src_ip_addr_len = OAL_IPV4_ADDR_SIZE;

        hmac_m2u_igmp_v3_update_list(hmac_vap, list_entry, m2u, grec, &ret);

        /* 取下一个组播组 */
        grec = (mac_igmp_v3_grec_stru *)((osal_u8 *)grec + igmpv3_grp_rec_len(grec));
        no_grec--;
    }
    return ret;
}

/*****************************************************************************
 函 数 名  : hmac_m2u_mld_v1_update
 功能描述  : MLDV1 报文更新snoop链表
 输入参数  : hmac_vap vap结构体; list_entry 待更新的节点结构体; mld_head MLD报文结构体
*****************************************************************************/
OAL_STATIC osal_u32 hmac_m2u_mld_v1_update(hmac_vap_stru *hmac_vap, hmac_m2u_list_update_stru *list_entry,
    mac_mld_v1_head_stru *mld_head)
{
    osal_u32           ul_ret = OAL_SUCC;
    if (hmac_m2u_snoop_is_denied_ipv6(hmac_vap, mld_head->group_ip) != 0) {
        oam_warning_log1(0, OAM_SF_M2U, "{hmac_m2u_mld_v1_update::group_ip \
            [%08x]:[xxxxxxxx]:[xxxxxxxx]:[xxxxxxxx] is denied}}",\
            oal_host2net_long(*(osal_u32 *)(&mld_head->group_ip[0])));

        return ul_ret;
    }

    /* 初始化组播组地址 */
    hmac_m2u_get_group_mac(list_entry->grp_mac, mld_head->group_ip, OAL_IPV6_ADDR_SIZE);

    if (mld_head->type != MLDV1_DONE_TYPE) {
        list_entry->cmd = HMAC_M2U_CMD_INCLUDE_LIST;
    } else {
        list_entry->cmd = HMAC_M2U_CMD_EXCLUDE_LIST;
    }
    list_entry->src_ip_addr_len = OAL_IPV6_ADDR_SIZE;
    memset_s(list_entry->src_ip_addr, OAL_IPV6_ADDR_SIZE, 0, OAL_IPV6_ADDR_SIZE);
    ul_ret = hmac_m2u_update_snoop_list(list_entry);
    return ul_ret;
}

OAL_STATIC osal_void hmac_m2u_mld_v2_update_list(hmac_vap_stru *hmac_vap, hmac_m2u_list_update_stru *list_entry,
    hmac_m2u_stru *m2u, mac_mld_v2_group_record_stru *grec, osal_u32 *ret)
{
    osal_u16 no_srec;                 /* no of source records */
    osal_u8 *src_addr = OAL_PTR_NULL; /* 组播源的IP地址 */

    no_srec = oal_host2net_short(grec->grec_srcaddr_num);

    /* MLD V2 exc的处理 */
    if ((grec->grec_type == MLDV2_CHANGE_TO_EXCLUDE) || (grec->grec_type == MLDV2_MODE_IS_EXCLUDE)) {
        oam_info_log2(0, OAM_SF_M2U, "vap_id[%d] {hmac_m2u_mld_v2_update_list::grec->grec_type is %d.}",
            hmac_vap->vap_id, grec->grec_type);
        hmac_m2u_exc_update(hmac_vap, no_srec, list_entry, ret);
    } else if ((grec->grec_type == MLDV2_CHANGE_TO_INCLUDE) || (grec->grec_type == MLDV2_MODE_IS_INCLUDE)) {
        /* MLD V2 INC的处理 */
        hmac_m2u_inc_update(hmac_vap, list_entry);
    } else if (grec->grec_type == MLDV2_ALLOW_NEW_SOURCES) {
        list_entry->cmd = HMAC_M2U_CMD_INCLUDE_LIST;
    }

    src_addr = (osal_u8 *)grec + OAL_SIZEOF(mac_mld_v2_group_record_stru);

    /* 同一组播组内不同src ip的链表更新 */
    while (no_srec != 0) {
        if (memcpy_s(list_entry->src_ip_addr, sizeof(list_entry->src_ip_addr), src_addr,
            OAL_IPV6_ADDR_SIZE) != EOK) {
            oam_warning_log0(0, OAM_SF_M2U, "{hmac_m2u_mld_v2_update_list::memcpy src_addr fail.}");
        }
        if (grec->grec_type != MLDV2_BLOCK_OLD_SOURCES) {
            *ret = hmac_m2u_update_snoop_list(list_entry);
        } else { /* block old source时清空该成员 */
            hmac_m2u_delete_old_source_records(hmac_vap, list_entry, m2u);
        }
        src_addr += OAL_IPV6_ADDR_SIZE;
        no_srec--;
    }
}

OAL_STATIC osal_u32 hmac_m2u_mldv2_grp_rec_len(mac_mld_v2_group_record_stru *grec)
{
    return OAL_SIZEOF(mac_mld_v2_group_record_stru) + (grec->grec_srcaddr_num << 4) + grec->grec_auxwords; /* 4 */
}

/* MLDV1 报文更新snoop链表 */
OAL_STATIC osal_u32 hmac_m2u_mld_v2_update(hmac_vap_stru *hmac_vap, hmac_m2u_list_update_stru *list_entry,
    mac_mld_v2_report_stru *mldv2)
{
    osal_u16 no_grec;                                  /* no of group records  */
    mac_mld_v2_group_record_stru *grec = OAL_PTR_NULL; /* igmp group record */
    hmac_m2u_stru *m2u = OAL_PTR_NULL;
    osal_u32 ret = OAL_SUCC;

    no_grec = oal_host2net_short(mldv2->group_address_num);
    grec = (mac_mld_v2_group_record_stru *)(mldv2 + 1);
    m2u = (hmac_m2u_stru *)(list_entry->hmac_vap->m2u);

    if (no_grec == 0) {
        oam_warning_log1(0, OAM_SF_M2U, "vap_id[%d] {hmac_m2u_mld_v2_update::no_grec is 0.}",
            hmac_vap->vap_id);
        return OAL_FAIL;
    }

    while (no_grec != 0) {
        list_entry->cmd = grec->grec_type;
        if (hmac_m2u_snoop_is_denied_ipv6(hmac_vap, grec->group_ip) != 0) {
            oam_warning_log1(0, OAM_SF_M2U,
                "{hmac_m2u_mld_v2_update::group_ip [%08x]:[xxxxxxxx]:[xxxxxxxx]:[xxxxxxxx] is denied}}",
                oal_host2net_long(*(osal_u32 *)(&grec->group_ip[0])));  /* 只打印32bit */
            grec = (mac_mld_v2_group_record_stru *)((osal_u8 *)grec + hmac_m2u_mldv2_grp_rec_len(grec));
            no_grec--;
            continue;
        }

        /* 非MLDV2六种cmd的异常处理 */
        if (!is_mldv2_mode(grec->grec_type)) {
            oam_warning_log2(0, OAM_SF_M2U,
                "vap_id[%d] {hmac_m2u_mld_v2_update::grec_type is [%x] not inside the six cmd.}",
                hmac_vap->vap_id, grec->grec_type);
            /* move the grec to next group record */
            grec = (mac_mld_v2_group_record_stru *)((osal_u8 *)grec + hmac_m2u_mldv2_grp_rec_len(grec));
            no_grec--;
            continue;
        }

        hmac_m2u_get_group_mac(list_entry->grp_mac, grec->group_ip, OAL_IPV6_ADDR_SIZE);
        list_entry->src_ip_addr_len = OAL_IPV6_ADDR_SIZE;
        hmac_m2u_mld_v2_update_list(hmac_vap, list_entry, m2u, grec, &ret);

        /* 取下一个组播组 */
        grec = (mac_mld_v2_group_record_stru *)((osal_u8 *)grec + hmac_m2u_mldv2_grp_rec_len(grec));
        no_grec--;
    }
    return ret;
}

/*****************************************************************************
 函 数 名  : hmac_m2u_snoop_inspecting_ipv4
 功能描述  :解析ipv4 下的IGMP报文
 输入参数  : hmac_vap vap结构体; hmac_user user结构体; buf netbuf结构体
*****************************************************************************/
OAL_STATIC osal_void hmac_m2u_snoop_inspecting_ipv4(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    hmac_m2u_list_update_stru *list_entry, osal_u8 *buf)
{
    mac_ip_header_stru              *ip_hdr;
    mac_igmp_header_stru            *igmp;                             /* igmp header for v1 and v2 */
    mac_igmp_v3_report_stru         *igmpr3;                           /* igmp header for v3 */
    osal_u8                        ip_hdr_len;

    /* 取ip头 */
    ip_hdr = (mac_ip_header_stru *)buf;

    if (ip_hdr->protocol != IPPROTO_IGMP) {
        return;
    }

    list_entry->hmac_vap  = hmac_vap;
    list_entry->hmac_user = hmac_user;

    /* bit序不同，取不同的ip头长度 */
    if (oal_netbuf_get_bitfield() == OAL_BITFIELD_BIG_ENDIAN) {
        ip_hdr_len = ip_hdr->version_ihl & 0x0F;
    } else {
        ip_hdr_len = (ip_hdr->version_ihl & 0xF0) >> 4; /* 4:偏移取值 */
    }
    if (ip_hdr_len < MIN_IP_HDR_LEN) {
        oam_warning_log2(0, OAM_SF_M2U, "vap_id[%d] {hmac_m2u_snoop_inspecting_ipv4::ip_hdr_len is [%x].}",
            hmac_vap->vap_id, ip_hdr_len);
        return;
    }
    /* v1 & v2 igmp */
    igmp = (mac_igmp_header_stru *)(buf + (ip_hdr_len << 2));

    /* v3 igmp */
    igmpr3 = (mac_igmp_v3_report_stru *)igmp;

    /* 如果报文不是IGMP report报文或leave报文,不进行链表更新 */
    if (is_igmp_report_leave_packet(igmp->type) == 0) {
        oam_warning_log2(0, OAM_SF_M2U, "vap_id[%d] {hmac_m2u_snoop_inspecting_ipv4::not igmp report[%x].}",
            hmac_vap->vap_id, igmp->type);
        return;
    }

    /* IGMP v1 v2 报文的链表更新 */
    if (igmp->type != MAC_IGMPV3_REPORT_TYPE) {
        oam_info_log2(0, OAM_SF_M2U, "vap_id[%d] {hmac_m2u_snoop_inspecting_ipv4::v1v2 update[%x].}",
            hmac_vap->vap_id, igmp->type);
        hmac_m2u_igmp_v1v2_update(hmac_vap, list_entry, igmp);
    } else { /* IGMP v3 report 报文的链表更新 */
        oam_info_log2(0, OAM_SF_M2U, "vap_id[%d] {hmac_m2u_snoop_inspecting_ipv4::v3 update[%x].}",
            hmac_vap->vap_id, igmp->type);
        hmac_m2u_igmp_v3_update(hmac_vap, list_entry, igmpr3);
    }
}

/*****************************************************************************
 函 数 名  : hmac_m2u_snoop_inspecting_ipv6
 功能描述  :解析ipv6 下的MLD报文
 输入参数  : hmac_vap vap结构体; hmac_user user结构体; buf netbuf结构体
*****************************************************************************/
OAL_STATIC osal_void hmac_m2u_snoop_inspecting_ipv6(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    hmac_m2u_list_update_stru *list_entry, osal_u8 *buf)
{
    oal_ipv6hdr_stru            *ip_hdr;
    mac_mld_v2_report_stru      *mldv2;                           /* mld report header for v2 */
    mac_mld_v1_head_stru        *mld_head;
    oal_icmp6hdr_stru           *icmp_head;

    /* 取ip头 */
    ip_hdr = (oal_ipv6hdr_stru *)(buf);
    icmp_head = (oal_icmp6hdr_stru *)(ip_hdr + 1); // 跳过IPV6 头获取ICMPV6头

    if ((icmp_head->icmp6_type != OAL_IPPROTO_ICMPV6) || (ip_hdr->nexthdr != 0x0)) {
        return;
    }

    mld_head = (mac_mld_v1_head_stru *)(icmp_head + 1); // 跳过IPV6 头获取ICMPV6头
    if (is_mld_report_leave_packet(mld_head->type) == 0) {
        return;
    }

    list_entry->hmac_vap  = hmac_vap;
    list_entry->hmac_user = hmac_user;
    mldv2 = (mac_mld_v2_report_stru *) mld_head;

    /* MLD v1  报文的链表更新 */
    if (mld_head->type != MLDV2_REPORT_TYPE) {
        oam_info_log2(0, OAM_SF_M2U, "vap_id[%d] {hmac_m2u_snoop_inspecting_ipv6::v1 update[%x].}",
            hmac_vap->vap_id, mld_head->type);
        hmac_m2u_mld_v1_update(hmac_vap, list_entry, mld_head);
    } else { /* MLD v2 report 报文的链表更新 */
        oam_info_log2(0, OAM_SF_M2U, "vap_id[%d] {hmac_m2u_snoop_inspecting_ipv6::v2 update[%x].}",
            hmac_vap->vap_id, mld_head->type);
        hmac_m2u_mld_v2_update(hmac_vap, list_entry, mldv2);
    }
}

/*****************************************************************************
 功能描述  : 获取外层vlan tag
*****************************************************************************/
OAL_STATIC osal_void hmac_m2u_snoop_inspecting_get_vlan_tag(hmac_vap_stru *hmac_vap,
    hmac_m2u_list_update_stru *list_entry, const mac_vlan_tag_stru *vlan_tag)
{
    if (memcpy_s(&(list_entry->outer_vlan_tag), OAL_SIZEOF(list_entry->outer_vlan_tag),
        vlan_tag, OAL_SIZEOF(mac_vlan_tag_stru)) != EOK) { // 存放vlan信息
        oam_warning_log1(0, OAM_SF_M2U, "vap[%d]hmac_m2u_snoop_inspecting_get_vlan_tag:memcpy fail", hmac_vap->vap_id);
    }
}

/*****************************************************************************
 函 数 名  : hmac_m2u_snoop_inspecting
 功能描述  : 监听IGMP报文来创建组播组链表
 输入参数  : hmac_vap vap结构体; hmac_user user结构体; buf netbuf结构体
*****************************************************************************/
osal_void hmac_m2u_snoop_inspecting(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user, oal_netbuf_stru *buf)
{
    mac_ether_header_stru *ether_header = (mac_ether_header_stru *)oal_netbuf_data(buf);
    /* source address which send the report and it is the member */
    /* list entry where all member details will be updated and passed on updating the snoop list */
    hmac_m2u_list_update_stru list_entry;
    mac_vlan_tag_stru *vlan_tag = OSAL_NULL;
    osal_u16 ether_data_type;
    osal_u8 *ip_head = OSAL_NULL;

    if (((hmac_m2u_stru *)(hmac_vap->m2u))->snoop_enable == OAL_FALSE) {
        oam_info_log1(0, OAM_SF_M2U, "vap_id[%d] {hmac_m2u_snoop_inspecting::snoop is not enable}", hmac_vap->vap_id);
        return;
    }

    memset_s(&list_entry, OAL_SIZEOF(hmac_m2u_list_update_stru), 0, OAL_SIZEOF(hmac_m2u_list_update_stru));

    /* 设置mac地址 */
    oal_set_mac_addr((osal_u8 *)list_entry.new_member_mac, (const osal_u8 *)ether_header->ether_shost);

    /* 带vlan tag 的情况 */
    if (ether_is_with_vlan_tag(ether_header->ether_type)) {
        // 偏移2个mac地址的长度,获取外层vlan tag
        vlan_tag = (mac_vlan_tag_stru *)(oal_netbuf_data(buf) + (ETHER_ADDR_LEN << 1));
        hmac_m2u_snoop_inspecting_get_vlan_tag(hmac_vap, &list_entry, vlan_tag);

        vlan_tag += 1; // 判断内层tag
        if (ether_is_with_vlan_tag(vlan_tag->tpid)) {
            vlan_tag += 1; // 不考虑内层tag，跳过
        }

        ether_data_type = *((osal_u16 *)(vlan_tag)); // 跳过tag后获取eth type
        ip_head        = (osal_u8 *)(vlan_tag) + 2; // 2：跳过 type 指向ip头起始地址
    } else { /* 非vlan 的情况 */
        ip_head = (osal_u8 *)(ether_header + 1);
        ether_data_type = ether_header->ether_type;
    }

    /* ap模式的接收处理 */
    if ((hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_AP) && (!ETHER_IS_BROADCAST(ether_header->ether_dhost))) {
        if ((ether_data_type == oal_host2net_short(ETHER_TYPE_IP)) &&
            (ether_is_ipv4_multicast(ether_header->ether_dhost) != 0)) {
            hmac_m2u_snoop_inspecting_ipv4(hmac_vap, hmac_user, &list_entry, ip_head);
        } else if ((ether_data_type == oal_host2net_short(ETHER_TYPE_IPV6)) &&
                   ETHER_IS_IPV6_MULTICAST(ether_header->ether_dhost)) {
            hmac_m2u_snoop_inspecting_ipv6(hmac_vap, hmac_user, &list_entry, ip_head);
        } else {
            oam_info_log2(0, OAM_SF_M2U, "vap_id[%d] {hmac_m2u_snoop_inspecting::ether type is not IP Protocol.[0x%x]}",
                hmac_vap->vap_id, oal_host2net_short(ether_header->ether_type));
            return;
        }
    }
    /* STA模式的接收处理 去掉自定义的tunnel协议头 */
    if ((ether_header->ether_type == oal_host2net_short(ETHER_TUNNEL_TYPE)) &&
        (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_STA)) {
        oam_info_log1(0, OAM_SF_M2U, "vap[%d]{hmac_m2u_snoop_inspecting:STA && ETHER_TUNNELTYPE}", hmac_vap->vap_id);
        oal_netbuf_pull(buf, OAL_SIZEOF(mac_ether_header_stru));
        oal_netbuf_pull(buf, OAL_SIZEOF(mcast_tunnel_hdr_stru));
    }
    return;
}

/*****************************************************************************
 函 数 名  : hmac_m2u_count_member_anysrclist
 功能描述  : 找出一个组播组内,组播源为所有源的所有STA用户
 输入参数  : grp_list 组播组结构体; table 组播组成员所在指针数组; timestamp nowtime变量
 输出参数  : count
*****************************************************************************/
OAL_STATIC osal_u8 hmac_m2u_count_member_anysrclist(hmac_m2u_grp_list_entry_stru *grp_list, osal_u8 *table,
    osal_u32 timestamp)
{
    hmac_m2u_grp_member_stru *grp_member;
    struct osal_list_head      *grp_member_entry;
    osal_u8                 count = 0;
    oal_bool_enum_uint8       ip_is_zero;

    osal_list_for_each(grp_member_entry, &(grp_list->src_list)) {
        grp_member = (hmac_m2u_grp_member_stru *)osal_list_entry(grp_member_entry,
            hmac_m2u_grp_member_stru, member_entry);
        if (grp_member->src_ip_addr_len == OAL_IPV6_ADDR_SIZE) {
            ip_is_zero = (osal_u8)OAL_IPV6_IS_UNSPECIFIED_ADDR(grp_member->src_ip_addr);
        } else {
            ip_is_zero = (osal_u8)(*((osal_s32 *)(grp_member->src_ip_addr)) == 0);
        }
        if (ip_is_zero) {
            if (count > MAX_STA_NUM_OF_ONE_GROUP) {
                break;
            }
            oal_set_mac_addr(&table[count * WLAN_MAC_ADDR_LEN], grp_member->grp_member_mac);
            grp_member->timestamp = timestamp;
            count++;
        }
    }
    return (count);
}

/*****************************************************************************
 函 数 名  : hmac_m2u_count_member_src_list
 功能描述  : 找出一个组播组内,组播源为指定源的所有STA用户
 输入参数  : grp_list 组播组结构体; src_ip_addr 组播源IP地址;
             table 组播组成员的MAC地址所在指针数组
             timestamp nowtime
*****************************************************************************/
OAL_STATIC osal_u8 hmac_m2u_count_member_src_list(hmac_m2u_grp_list_entry_stru *grp_list,
    osal_u8 *src_ip_addr, osal_u8 *table, osal_u32 timestamp, osal_u8 count)
{
    hmac_m2u_grp_member_stru *grp_member;
    struct osal_list_head      *grp_member_entry;
    osal_u8 count_tmp = count;

    if (count_tmp > MAX_STA_NUM_OF_ONE_GROUP) {
        return count_tmp;
    }

    osal_list_for_each(grp_member_entry, &(grp_list->src_list)) {
        grp_member = (hmac_m2u_grp_member_stru *)osal_list_entry(grp_member_entry,
            hmac_m2u_grp_member_stru, member_entry);
        if (count_tmp > MAX_STA_NUM_OF_ONE_GROUP) {
            break;
        }
        /* 组播源地址符合，模式是inc，加入到输出的table中 */
        if (osal_memcmp(src_ip_addr, grp_member->src_ip_addr, grp_member->src_ip_addr_len) == 0) {
            if (grp_member->mode == HMAC_M2U_CMD_INCLUDE_LIST) {
                oal_set_mac_addr((osal_u8 *)(&table[count_tmp * WLAN_MAC_ADDR_LEN]),
                    (const osal_u8 *)grp_member->grp_member_mac);
                grp_member->timestamp = timestamp;
                count_tmp++;
            }
        } else {
            /* 组播源未匹配，但模式为exc的情况也加入到输出table中 */
            if (grp_member->mode == HMAC_M2U_CMD_EXCLUDE_LIST) {
                oal_set_mac_addr((osal_u8 *)(&table[count_tmp * WLAN_MAC_ADDR_LEN]),
                    (const osal_u8 *)grp_member->grp_member_mac);
                grp_member->timestamp = timestamp;
                count_tmp++;
            }
        }
    }
    return count_tmp;
}

/*****************************************************************************
 函 数 名  : hmac_m2u_get_snooplist_member
 功能描述  : 找到一个group内需要该组播源的所有STA用户
 输入参数  : hmac_vap vap结构体;
             grp_addr  组播组mac地址;
             src_ip_addr 组播源IP地址;
             table 组播组成员的MAC地址所在指针数组
             mac_vlan_tag_stru      *vlan_tag;//vlan结构体指针
 输出参数  : count
*****************************************************************************/
OAL_STATIC osal_u8 hmac_m2u_get_snooplist_member(hmac_vap_stru *hmac_vap, osal_u8 *grp_addr,
    osal_u8 *src_ip_addr, osal_u8 *table, mac_vlan_tag_stru *vlan_tag)
{
    hmac_m2u_grp_list_entry_stru     *grp_list_member;
    osal_u8                         count = 0;
    osal_u32                        nowtime;
    hmac_m2u_list_update_stru         list_entry;

    memset_s(&list_entry, OAL_SIZEOF(hmac_m2u_list_update_stru), 0, OAL_SIZEOF(hmac_m2u_list_update_stru));
    if (memcpy_s(&(list_entry.grp_mac), OAL_SIZEOF(list_entry.grp_mac),
        grp_addr, WLAN_MAC_ADDR_LEN) != EOK) { // 存放vlan信息
        oam_warning_log1(0, OAM_SF_M2U, "vap[%d]hmac_m2u_get_snooplist_member:memcpy addr fail", hmac_vap->vap_id);
    }
    if (vlan_tag != OAL_PTR_NULL) {
        if (memcpy_s(&(list_entry.outer_vlan_tag), OAL_SIZEOF(list_entry.outer_vlan_tag),
            vlan_tag, OAL_SIZEOF(mac_vlan_tag_stru)) != EOK) { // 存放vlan信息
            oam_warning_log1(0, OAM_SF_M2U, "vap[%d]hmac_m2u_get_snooplist_member:memcpy vlan fail", hmac_vap->vap_id);
        }
    }

    nowtime = (osal_u32)oal_time_get_stamp_ms();
    grp_list_member = hmac_m2u_find_group_list(hmac_vap, &list_entry);
    if (grp_list_member != OAL_PTR_NULL) {
        count  = hmac_m2u_count_member_anysrclist(grp_list_member, &table[0], nowtime);
        count  = hmac_m2u_count_member_src_list(grp_list_member, src_ip_addr,
            &table[0], nowtime, count);
        oam_info_log3(0, OAM_SF_M2U, "vap_id[%d] {hmac_m2u_get_snooplist_member::count is [%d] vlanId is [%d]}",
            hmac_vap->vap_id, count, grp_list_member->outer_vlan_tag.vlan_id);
    }

    return (count);
}

/*****************************************************************************
 函 数 名  : hmac_m2u_remove_node_grp
 功能描述  : 删除一个组播组下属于特定STA的节点，如果该入参STA为空，则清空该组播组
 输入参数  : grp_list 组播组结构体; hmac_user user结构体
 输出参数  : sta_num_removed 删除的sta个数
*****************************************************************************/
OAL_STATIC osal_void hmac_m2u_remove_node_grp(hmac_m2u_grp_list_entry_stru *grp_list,
    const hmac_user_stru *hmac_user, osal_u8  *sta_num_removed)
{
    hmac_m2u_grp_member_stru *grp_member;
    struct osal_list_head      *grp_member_entry;
    struct osal_list_head      *member_entry_temp;
    osal_u8                 sta_count = 0;

    osal_list_for_each_safe(grp_member_entry, member_entry_temp, &grp_list->src_list) {
        grp_member = (hmac_m2u_grp_member_stru *)osal_list_entry(grp_member_entry,
            hmac_m2u_grp_member_stru, member_entry);
        if ((hmac_user == grp_member->hmac_user) || (hmac_user == OAL_PTR_NULL)) {
            osal_list_del(&(grp_member->member_entry));
            oal_mem_free(grp_member, OAL_TRUE);
            sta_count++;
        }
    }
    if (grp_list->sta_num >= sta_count) {
        oam_warning_log2(0, OAM_SF_M2U, "{hmac_m2u_remove_node_grp::remove [%d] sta from a group with [%d] sta}",
            sta_count, grp_list->sta_num);
        grp_list->sta_num -= sta_count;
    } else {
        oam_error_log2(0, OAM_SF_M2U, "{hmac_m2u_remove_node_grp::fail to remove [%d] sta from a group with [%d] sta}",
            sta_count, grp_list->sta_num);
        grp_list->sta_num = 0;
        sta_count = grp_list->sta_num;
    }
    *sta_num_removed = sta_count;
}

/*****************************************************************************
 函 数 名  : hmac_m2u_clean_snp_list
 功能描述  : 清空组播组链表以及组播组内的成员
 输入参数  : hmac_vap vap结构体
*****************************************************************************/
OAL_STATIC osal_void hmac_m2u_clean_snp_list(hmac_vap_stru *hmac_vap)
{
    osal_u8                      sta_num_removed = 0;
    hmac_m2u_stru                 *m2u;
    hmac_m2u_snoop_list_stru      *snp_list;
    hmac_m2u_grp_list_entry_stru  *grp_list;
    struct osal_list_head           *grp_list_entry;
    struct osal_list_head           *grp_list_entry_temp;

    /* 获取snoop链表头 */
    m2u      = (hmac_m2u_stru *)(hmac_vap->m2u);
    snp_list = &(m2u->m2u_snooplist);

    osal_list_for_each_safe(grp_list_entry, grp_list_entry_temp, &(snp_list->grp_list)) {
        grp_list = (hmac_m2u_grp_list_entry_stru *)osal_list_entry(grp_list_entry,
            hmac_m2u_grp_list_entry_stru, grp_entry);

        hmac_m2u_remove_node_grp(grp_list, OAL_PTR_NULL, &sta_num_removed);
        osal_list_del(&(grp_list->grp_entry));
        oal_mem_free(grp_list, OAL_TRUE);
        grp_list = OAL_PTR_NULL;
        snp_list->group_list_count--;
        if (snp_list->total_sta_num >= sta_num_removed) {
            snp_list->total_sta_num -= sta_num_removed;
            oam_warning_log3(0, OAM_SF_M2U,
                "vap_id[%d] {hmac_m2u_clean_snp_list::remove a group with [%d] sta.current total sta num:[%d]}",
                hmac_vap->vap_id,
                sta_num_removed, snp_list->total_sta_num);
        } else {
            oam_error_log3(0, OAM_SF_M2U,
                "vap_id[%d] {hmac_m2u_clean_snp_list::remove a group with [%d] sta. but current total sta num is[%d]}",
                hmac_vap->vap_id, sta_num_removed, snp_list->total_sta_num);
            snp_list->total_sta_num = 0;
        }
    }
}

/*****************************************************************************
 函 数 名  : hmac_m2u_cleanup_snoopwds_node
 功能描述  : 清空各个组播组内的某个STA用户
 输入参数  : hmac_user user结构体
*****************************************************************************/
osal_void hmac_m2u_cleanup_snoopwds_node(hmac_user_stru *hmac_user)
{
    osal_u8                      sta_num_removed = 0;
    hmac_vap_stru                 *hmac_vap;
    hmac_m2u_stru                 *m2u;
    hmac_m2u_snoop_list_stru      *snp_list;
    hmac_m2u_grp_list_entry_stru  *grp_list;
    struct osal_list_head           *grp_list_entry;
    struct osal_list_head           *grp_list_entry_temp;

    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(hmac_user->vap_id);
    if (osal_unlikely(hmac_vap == OAL_PTR_NULL)) {
        oam_info_log1(0, OAM_SF_ANY, "{hmac_m2u_cleanup_snoopwds_node::hmac_vap[id=%d] null!!}",
            hmac_user->vap_id);
        return;
    }

    m2u = (hmac_m2u_stru *)(hmac_vap->m2u);
    snp_list = &(m2u->m2u_snooplist);

    if ((hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_AP) &&
        (snp_list != OAL_PTR_NULL) && (m2u->snoop_enable == OAL_TRUE)) {
        osal_list_for_each_safe(grp_list_entry, grp_list_entry_temp, &(snp_list->grp_list)) {
            grp_list = (hmac_m2u_grp_list_entry_stru *)osal_list_entry(grp_list_entry,
                hmac_m2u_grp_list_entry_stru, grp_entry);
            hmac_m2u_remove_node_grp(grp_list, hmac_user, &sta_num_removed);
            snp_list->total_sta_num -= sta_num_removed;
            oam_info_log2(0, OAM_SF_M2U, "vap_id[%d] {hmac_m2u_cleanup_snoopwds_node::removed [%d] sta  }",
                hmac_vap->vap_id, sta_num_removed);
            if (osal_list_empty(&(grp_list->src_list)) == OSAL_TRUE) { // 删除没有用户的组播组
                osal_list_del(&(grp_list->grp_entry));
                oal_mem_free(grp_list, OAL_TRUE);
                grp_list = OAL_PTR_NULL;
                snp_list->group_list_count--;
                oam_info_log1(0, OAM_SF_M2U, "vap_id[%d] {hmac_m2u_cleanup_snoopwds_node::remove a null group  }",
                    hmac_vap->vap_id);
            }
        }
    }
}

/*****************************************************************************
 函 数 名  : hmac_m2u_tx_event
 功能描述  : 组播转单播，单播封装802.11头以及发送事件
 输入参数  : hmac_vap vap结构体; user user结构体; buf netbuf结构体
*****************************************************************************/
OAL_STATIC osal_u32 hmac_m2u_tx_event(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user, oal_netbuf_stru *buf)
{
    osal_u32 ret;

    /* 封装802.11头 */
    ret = hmac_tx_encap_etc(hmac_vap, hmac_user, buf);
    if (osal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log2(0, OAM_SF_TX,
            "vap_id[%d] {hmac_tx_lan_mpdu_process_ap::hmac_tx_encap_etc failed[%d].}",
            hmac_vap->vap_id, ret);
        OAM_STAT_VAP_INCR(hmac_vap->vap_id, tx_abnormal_msdu_dropped, 1);
        hmac_dft_print_drop_frame_info(THIS_FILE_ID, __LINE__, 1, OAL_PTR_NULL);
        return ret;
    }
    ret = hmac_tx_send_encap_data(hmac_vap, buf);
    if (osal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log2(0, OAM_SF_TX, "vap_id[%d]{hmac_tx_lan_to_wlan_etc::frw_send_data failed[%d].}",
            hmac_vap->vap_id, ret);
        OAM_STAT_VAP_INCR(hmac_vap->vap_id, tx_abnormal_msdu_dropped, 1);
    }

    return ret;
}

/*****************************************************************************
 函 数 名  : hmac_m2u_convert_loop_end
 输入参数  : hmac_vap vap结构体; buf netbuf 结构体
 输出参数  : ul_ret
*****************************************************************************/
OAL_STATIC osal_void hmac_m2u_convert_loop_end(oal_netbuf_stru *copy_buf, oal_netbuf_stru **buf,
    osal_u8 *ucast_sta_cnt, osal_u8 *ucast_sta_idx)
{
    if (copy_buf != OAL_PTR_NULL) {
        *buf = copy_buf;
        copy_buf = OAL_PTR_NULL;
    }
    (*ucast_sta_idx)++;
    if (*ucast_sta_cnt > 0) {
        (*ucast_sta_cnt)--;
    }
}

/*****************************************************************************
 函 数 名  : hmac_m2u_snoop_convert_count
 功能描述  : 组播转单播发送计数
 输入参数  : hmac_vap vap结构体; buf netbuf 结构体
 输出参数  : ul_ret
*****************************************************************************/
OAL_STATIC osal_void hmac_m2u_snoop_convert_count(hmac_vap_stru *hmac_vap, osal_u8 ucast_sta_cnt, osal_u32 ul_ret,
    oal_netbuf_stru *buf)
{
    unref_param(hmac_vap);
    unref_param(ucast_sta_cnt);
    /* ucast event fail 的发送计数 */
    if (ul_ret != OAL_SUCC) {
        hmac_free_netbuf_list_etc(buf);
    }
}

/*****************************************************************************
函 数 名  : hmac_m2u_snoop_change_mac_hdr
功能描述  : 组播转单播发送计数
输入参数  : hmac_vap vap结构体; buf netbuf 结构体
输出参数  : ul_ret
*****************************************************************************/
OAL_STATIC osal_void hmac_m2u_snoop_change_mac_hdr(const hmac_m2u_stru *m2u, mac_ether_header_stru  **ucast_ether_hdr,
    oal_netbuf_stru *buf)
{
    mcast_tunnel_hdr_stru  *mcast_tunhdr;
    mac_ether_header_stru  *ether_hdr;
    osal_u8               srcmac[WLAN_MAC_ADDR_LEN];

    ether_hdr  = (mac_ether_header_stru *)oal_netbuf_data(buf);
    oal_set_mac_addr((osal_u8 *)srcmac, (const osal_u8 *)ether_hdr->ether_shost);
    /* 加自定义tunnel协议 */
    if ((m2u->mcast_mode & BIT0) != 0) { /* mcast_mode = 1 */
        mcast_tunhdr = (mcast_tunnel_hdr_stru *) oal_netbuf_push(buf, OAL_SIZEOF(mcast_tunnel_hdr_stru));
        *ucast_ether_hdr = (mac_ether_header_stru *)oal_netbuf_push(buf, OAL_SIZEOF(mac_ether_header_stru));
        mcast_tunhdr->proto = MAC_ETH_PROTOCOL_SUBTYPE;

        /* 拷贝原始组播源地址 */
        oal_set_mac_addr((osal_u8 *)(&((*ucast_ether_hdr)->ether_shost[0])), (const osal_u8 *)srcmac);

        /* 拷贝新的协议类型 */
        (*ucast_ether_hdr)->ether_type = oal_host2net_short(ETHER_TUNNEL_TYPE);
    } else { /* 不加自定义tunnel协议 */                /* mcast_mode = 2 */
        *ucast_ether_hdr = (mac_ether_header_stru *)oal_netbuf_data(buf);
    }
}

/*****************************************************************************
 函 数 名  : hmac_m2u_group_is_special
 功能描述  : 所加入组播组的IP地址是否在特殊组播业务
 输入参数  : ether_data_type 以太网数据类型
 输出参数  : OAL_FALSE OR OAL_TRUE
*****************************************************************************/
OAL_STATIC osal_u32 hmac_m2u_group_is_special(hmac_vap_stru *hmac_vap, osal_u16 ether_data_type, osal_u8 *ip_head)
{
    mac_ip_header_stru       *ip_header;
    oal_ipv6hdr_stru         *ipv6_hdr;
    hmac_m2u_snoop_list_stru *snp_list;
    osal_u8                 idx;
    osal_u32                grp_addr;
    osal_u8                 grp_ip_addr[OAL_IPV6_ADDR_SIZE] = {0};
    hmac_m2u_stru            *m2u = (hmac_m2u_stru *)(hmac_vap->m2u);

    if (m2u == OAL_PTR_NULL) {
        return OAL_FALSE;
    }
    snp_list = &(m2u->m2u_snooplist);

    if (ether_data_type == oal_host2net_short(ETHER_TYPE_IP)) {
        ip_header = (mac_ip_header_stru *)ip_head;
        grp_addr = oal_host2net_long(ip_header->daddr);

        for (idx = 0; idx < SPECIAL_M2U_GROUP_COUNT_IPV4; idx++) {
            if (grp_addr == snp_list->special_group_ipv4[idx]) {
                return OAL_TRUE;
            }
        }
    }

    if (ether_data_type == oal_host2net_short(ETHER_TYPE_IPV6)) {
        ipv6_hdr = (oal_ipv6hdr_stru *)(ip_head);
        if (memcpy_s(grp_ip_addr, OAL_SIZEOF(grp_ip_addr),
            (osal_s8 *)(&(ipv6_hdr->daddr)), OAL_IPV6_ADDR_SIZE) != EOK) {
            oam_warning_log1(0, OAM_SF_M2U, "vap[%d]hmac_m2u_group_is_special:memcpy addr fail", hmac_vap->vap_id);
        }
        for (idx = 0; idx < SPECIAL_M2U_GROUP_COUNT_IPV6; idx++) {
            if (osal_memcmp(grp_ip_addr, snp_list->special_group_ipv6[idx], OAL_IPV6_ADDR_SIZE) == 0) {
                return OAL_TRUE;
            }
        }
    }

    return OAL_FALSE;
}

OAL_STATIC osal_u32 hmac_m2u_sta_get_ready_user(hmac_user_stru *hmac_user_tmp, osal_u8 *srcmac,
    osal_u8 vap_id, osal_u8 **dstmac, hmac_user_stru **hmac_user)
{
    if (hmac_user_tmp == OAL_PTR_NULL) {
        oam_error_log1(0, OAM_SF_M2U, "vap_id[%d] {hmac_m2u_sta_get_ready_user::user_tmp null.}", vap_id);
        return OSAL_FALSE;
    }

    /* 如果用户尚未关联成功 */
    if (hmac_user_tmp->user_asoc_state != MAC_USER_STATE_ASSOC) {
        oam_warning_log1(0, OAM_SF_M2U, "vap_id[%d] {hmac_m2u_sta_get_ready_user::sta not assoc.}", vap_id);
        return OSAL_FALSE;
    }

    *hmac_user = mac_res_get_hmac_user_etc(hmac_user_tmp->assoc_id);
    if (*hmac_user == OAL_PTR_NULL) {
        oam_error_log2(0, OAM_SF_M2U, "vap_id[%d] {hmac_m2u_sta_get_ready_user::hmac_user_tmp null.idx:%u}", vap_id,
            hmac_user_tmp->assoc_id);
        return OSAL_FALSE;
    }

    *dstmac  = hmac_user_tmp->user_mac_addr;

    /* 发送的目的地址和发来的源地址相同的异常处理 */
    if (oal_compare_mac_addr(*dstmac, srcmac) == 0) {
        oam_info_log1(0, OAM_SF_M2U, "vap_id[%d] {hmac_m2u_sta_get_ready_user::dstmac == srcmac.}", vap_id);
        return OSAL_FALSE;
    }

    return OSAL_TRUE;
}

OAL_STATIC osal_void hmac_m2u_sta_fill_cb_and_mac_hdr(oal_netbuf_stru *netbuf, const hmac_user_stru *hmac_user,
    hmac_m2u_stru *m2u, osal_u8 *dstmac, mac_tx_ctl_stru **tx_ctl_ret)
{
    mac_tx_ctl_stru        *tx_ctl;
    mac_ether_header_stru  *ucast_ether_hdr =  OAL_PTR_NULL;

    tx_ctl  = (mac_tx_ctl_stru *)OAL_NETBUF_CB(netbuf);
    /* 组播转单播 CB字段处理 */
    mac_get_cb_is_mcast(tx_ctl)     = OAL_FALSE;
    mac_get_cb_ack_polacy(tx_ctl)   = WLAN_TX_NORMAL_ACK;
    mac_get_cb_tx_user_idx(tx_ctl)  = (osal_u8)hmac_user->assoc_id;
    mac_get_cb_wme_tid_type(tx_ctl) = m2u->tid_num;
    mac_get_cb_wme_ac_type(tx_ctl)  = wlan_wme_tid_to_ac(m2u->tid_num);

    /* 组播转单播，mac头的封装 */
    hmac_m2u_snoop_change_mac_hdr(m2u, &ucast_ether_hdr, netbuf);

    /* 将DA替换成关联设备的MAC */
    oal_set_mac_addr(&ucast_ether_hdr->ether_dhost[0], dstmac);

    *tx_ctl_ret = tx_ctl;
}

/*****************************************************************************
 函 数 名  : hmac_m2u_sta_convert
 功能描述  : 发送方向对所有下挂sta进行组播转单播处理
 输入参数  : hmac_vap vap结构体; buf netbuf 结构体; srcmac 源mac地址
 输出参数  : ul_ret
*****************************************************************************/
OAL_STATIC osal_u32 hmac_m2u_sta_convert(hmac_vap_stru *hmac_vap, oal_netbuf_stru *buf, osal_u8 *srcmac,
    osal_u8 macaddr_len)
{
    hmac_user_stru         *hmac_user = OAL_PTR_NULL;
    osal_u8              *dstmac = OAL_PTR_NULL;
    osal_u8               ucast_sta_cnt;
    osal_u8               ucast_sta_idx = 0;

    oal_netbuf_stru        *copy_buf = OAL_PTR_NULL;
    osal_u32              ul_ret;
    mac_tx_ctl_stru        *tx_ctl = OAL_PTR_NULL;
    hmac_m2u_stru          *m2u = (hmac_m2u_stru *)(hmac_vap->m2u);
    struct osal_list_head    *entry = OAL_PTR_NULL;
    struct osal_list_head    *dlist_tmp = OAL_PTR_NULL;
    hmac_user_stru         *hmac_user_tmp = OAL_PTR_NULL;
    osal_u32                free_buffer_cnt = 0 ;

    unref_param(macaddr_len);
    ucast_sta_cnt = (osal_u8)hmac_vap->user_nums;

    if (ucast_sta_cnt == 0) {
        oam_info_log1(0, OAM_SF_M2U, "vap_id[%d]{hmac_m2u_sta_convert::no user associate this vap}", hmac_vap->vap_id);
        OAM_STAT_VAP_INCR(hmac_vap->vap_id, tx_m2u_mcast_cnt, 1);
        return HMAC_TX_PASS;
    }

    /* 遍历vap下所有用户 */
    osal_list_for_each_safe(entry, dlist_tmp, &(hmac_vap->mac_user_list_head)) {
        if (ucast_sta_cnt > 1) {
            copy_buf = oal_netbuf_copy(buf, GFP_ATOMIC);
        }

        hmac_user_tmp = osal_list_entry(entry, hmac_user_stru, user_dlist);
        if (hmac_m2u_sta_get_ready_user(hmac_user_tmp, srcmac, hmac_vap->vap_id,
            &dstmac, &hmac_user) != OSAL_TRUE) {
            free_buffer_cnt = hmac_free_netbuf_list_etc(buf);
            hmac_dft_print_drop_frame_info(THIS_FILE_ID, __LINE__, free_buffer_cnt, OAL_PTR_NULL);
             /* 组播转单播发送循环的末尾处理 */
            hmac_m2u_convert_loop_end(copy_buf, &buf, &ucast_sta_cnt, &ucast_sta_idx);
            continue;
        }

        hmac_m2u_sta_fill_cb_and_mac_hdr(buf, hmac_user_tmp, m2u, dstmac, &tx_ctl);

        ul_ret = hmac_tx_ucast_process_etc(hmac_vap, buf, hmac_user, tx_ctl);
        if (osal_unlikely(ul_ret != HMAC_TX_PASS)) {
            if (ul_ret != HMAC_TX_BUFF) {
                /* 不等于HMAC_TX_BUFF，不缓存的直接释放 */
                oam_warning_log2(0, OAM_SF_M2U,
                    "vap_id[%d] {hmac_m2u_sta_convert::hmac_tx_ucast_process_etc not pass or buff, ul_ret = [%d]}",
                    hmac_vap->vap_id, ul_ret);
                free_buffer_cnt = hmac_free_netbuf_list_etc(buf);
            }
            hmac_m2u_convert_loop_end(copy_buf, &buf, &ucast_sta_cnt, &ucast_sta_idx);
            continue;
        }

        ul_ret = hmac_m2u_tx_event(hmac_vap, hmac_user, buf);

        /* 组播转单播发送计数统计 */
        hmac_m2u_snoop_convert_count(hmac_vap, ucast_sta_cnt, ul_ret, buf);

        /* 组播转单播发送循环的末尾处理 */
        hmac_m2u_convert_loop_end(copy_buf, &buf, &ucast_sta_cnt, &ucast_sta_idx);
    }

    return HMAC_TX_DONE;
}

/*****************************************************************************
 函 数 名  : hmac_m2u_snoop_convert
 功能描述  : 发送方向的组播转单播处理
 输入参数  : hmac_vap vap结构体; buf netbuf 结构体
 输出参数  : ul_ret
*****************************************************************************/
osal_u32 hmac_m2u_snoop_convert(hmac_vap_stru *hmac_vap, oal_netbuf_stru *buf)
{
    hmac_user_stru         *hmac_user = OAL_PTR_NULL;
    mac_ether_header_stru  *ether_hdr;
    osal_u8              *dstmac;
    osal_u8               src_ip_addr[OAL_IPV6_ADDR_SIZE] = {0};
    osal_u8               dta_ip_addr[OAL_IPV6_ADDR_SIZE] = {0};
    osal_u32              grp_addr;
    osal_u8               srcmac[WLAN_MAC_ADDR_LEN];
    osal_u8               grpmac[WLAN_MAC_ADDR_LEN];
    osal_u8               empty_entry_mac[WLAN_MAC_ADDR_LEN] = {0};
    osal_u8               ucast_sta_mac[MAX_STA_NUM_OF_ONE_GROUP][WLAN_MAC_ADDR_LEN];
    osal_u8               ucast_sta_cnt;
    osal_u8               ucast_sta_idx = 0;
    mac_ether_header_stru  *ucast_ether_hdr =  OAL_PTR_NULL;
    oal_netbuf_stru        *copy_buf = OAL_PTR_NULL;
    osal_u16              user_idx;
    osal_u16              ether_data_type;
    osal_u32              ul_ret;
    osal_u8              *ip_head;
    mac_tx_ctl_stru        *tx_ctl;
    mac_ip_header_stru     *ip_header;
    oal_ipv6hdr_stru       *ipv6_hdr;
    mac_vlan_tag_stru      *vlan_tag;
    hmac_m2u_stru          *m2u = (hmac_m2u_stru *)(hmac_vap->m2u);
#ifdef _PRE_WLAN_FEATURE_HERA_MCAST
    hmac_m2u_grp_list_entry_stru     *grp_list_member;
    hmac_m2u_list_update_stru         list_entry;
#endif

    /* 未打开组转单播开关，发送原组播 */
    if (m2u->snoop_enable == OAL_FALSE) {
        oam_warning_log2(0, OAM_SF_M2U, "vap_id[%d] {hmac_m2u_snoop_convert::snoop is [%d] not enable}",
                         hmac_vap->vap_id, m2u->snoop_enable);
        OAM_STAT_VAP_INCR(hmac_vap->vap_id, tx_m2u_mcast_cnt, 1);
        return HMAC_TX_PASS;
    }
    ether_hdr  = (mac_ether_header_stru *)oal_netbuf_data(buf);
    /* 带vlan tag 的情况 */
    if (ether_is_with_vlan_tag(ether_hdr->ether_type)) {
        // 偏移2个mac地址的长度,获取外层vlan tag
        vlan_tag = (mac_vlan_tag_stru *)(oal_netbuf_data(buf) + (ETHER_ADDR_LEN << 1));
        vlan_tag += 1;
        if (!ether_is_with_vlan_tag(vlan_tag->tpid)) {                // 双层tag的情况，内层tag不考虑
            ether_data_type = *((osal_u16 *)vlan_tag);            // 跳过tag后获取eth  type
            ip_head = (osal_u8 *)vlan_tag + 2;                   // 2：跳过 type 和tag指向ip头起始地址
        } else {
            ether_data_type = *((osal_u16 *)(vlan_tag + 1));       // 跳过内层tag后获取eth  type
            ip_head = (osal_u8 *)(vlan_tag + 1) + 2;              // 2：跳过 type 和内层指向ip头起始地址
        }
        vlan_tag -= 1;                                                  // 指向外层tag，作为参数传递到下层
    } else { /* 非vlan  的情况 */
        ip_head = (osal_u8 *)(ether_hdr + 1);
        ether_data_type = ether_hdr->ether_type;
        vlan_tag = OAL_PTR_NULL;
    }
    oal_set_mac_addr((osal_u8 *)srcmac, (const osal_u8 *)ether_hdr->ether_shost);
    oal_set_mac_addr((osal_u8 *)grpmac, (const osal_u8 *)ether_hdr->ether_dhost);

    /* 部分组播控制管理帧，进行组播转单播，发送给每一个关联STA */
    if (oal_is_mdnsv4_mac((osal_u8 *)ether_hdr, ether_data_type) ||
        oal_is_mdnsv6_mac((osal_u8 *)ether_hdr, ether_data_type) ||
        oal_is_icmpv6_proto(ether_data_type, ip_head) ||
        oal_is_igmp_proto(ether_data_type, ip_head) ||
        (hmac_m2u_group_is_special(hmac_vap, ether_data_type, ip_head) == OAL_TRUE)) {
        return hmac_m2u_sta_convert(hmac_vap, buf, srcmac, WLAN_MAC_ADDR_LEN);
    }

    if (ether_data_type == oal_host2net_short(ETHER_TYPE_IP)) {
        ip_header = (mac_ip_header_stru *)ip_head;
        *((osal_u32 *)src_ip_addr) = ip_header->saddr;
        grp_addr = oal_host2net_long(ip_header->daddr);
        /* 该组在黑名单内,不进行组播转单播，组播处理 */
        if (hmac_m2u_snoop_is_denied_ipv4(hmac_vap, grp_addr) != 0) {
            oam_info_log3(0, OAM_SF_M2U, "{hmac_m2u_snoop_convert::group_addr[%02x].[%02x].[%02x].[xx] is denied}}",
                (osal_u32)((grp_addr >> 24) & 0xff),  /* 24:偏移取值 */
                (osal_u32)((grp_addr >> 16) & 0xff),  /* 16:偏移取值 */
                (osal_u32)((grp_addr >> 8)  & 0xff)); /* 8:偏移取值 */

            OAM_STAT_VAP_INCR(hmac_vap->vap_id, tx_m2u_mcast_cnt, 1);
            return HMAC_TX_PASS;
        }
    } else if (ether_data_type == oal_host2net_short(ETHER_TYPE_IPV6)) {
        ipv6_hdr = (oal_ipv6hdr_stru *)(ip_head);
        if (memcpy_s(src_ip_addr, OAL_SIZEOF(src_ip_addr),
            (osal_s8 *)(&(ipv6_hdr->saddr)), OAL_IPV6_ADDR_SIZE) != EOK) {
            oam_warning_log1(0, OAM_SF_M2U, "vap[%d]hmac_m2u_snoop_convert:memcpy addr fail", hmac_vap->vap_id);
        }

        if (memcpy_s(dta_ip_addr, OAL_SIZEOF(dta_ip_addr),
            (osal_s8 *)(&(ipv6_hdr->daddr)), OAL_IPV6_ADDR_SIZE) != EOK) {
            oam_warning_log1(0, OAM_SF_M2U, "vap[%d]hmac_m2u_snoop_convert:memcpy dta addr fail", hmac_vap->vap_id);
        }
        if (hmac_m2u_snoop_is_denied_ipv6(hmac_vap, (osal_u8 *)dta_ip_addr) != 0) {
            oam_info_log1(0, OAM_SF_M2U,
                "{hmac_m2u_snoop_convert::group_addr_IPV6 \
                [%08x]:[xxxxxxxx]:[xxxxxxxx]:[xxxxxxxx] is denied}}", \
                oal_host2net_long(*(osal_u32 *)(&dta_ip_addr[0])));
            OAM_STAT_VAP_INCR(hmac_vap->vap_id, tx_m2u_mcast_cnt, 1);
            return HMAC_TX_PASS;
        }
    } else {
        OAM_STAT_VAP_INCR(hmac_vap->vap_id, tx_m2u_mcast_cnt, 1);
        return HMAC_TX_PASS;
    }

#ifdef _PRE_WLAN_FEATURE_HERA_MCAST
    memset_s(&list_entry, OAL_SIZEOF(hmac_m2u_list_update_stru), 0, OAL_SIZEOF(hmac_m2u_list_update_stru));
    if (memcpy_s(&(list_entry.grp_mac), OAL_SIZEOF(list_entry.grp_mac), grpmac, WLAN_MAC_ADDR_LEN) != EOK) {
        oam_warning_log1(0, OAM_SF_M2U, "vap[%d]hmac_m2u_snoop_convert::memcpy addr fail", hmac_vap->vap_id);
    }
    if (vlan_tag != OAL_PTR_NULL) {
        if (memcpy_s(&(list_entry.outer_vlan_tag), OAL_SIZEOF(list_entry.outer_vlan_tag),
            vlan_tag, OAL_SIZEOF(mac_vlan_tag_stru)) != EOK) {
            oam_warning_log1(0, OAM_SF_M2U, "vap[%d]hmac_m2u_snoop_convert::memcpy vlan fail", hmac_vap->vap_id);
        }
    }
    grp_list_member = hmac_m2u_find_group_list(hmac_vap, &list_entry);
    /* 组播snoop表中不存在当前组播帧所在的组播组,组播转发处理 */
    if (grp_list_member == OAL_PTR_NULL_member) {
        oam_info_log1(0, OAM_SF_M2U, "vap_id[%d] {hmac_m2u_snoop_convert:: grp_list_member is null}",
            hmac_vap->vap_id);
        OAM_STAT_VAP_INCR(hmac_vap->vap_id, tx_m2u_mcast_cnt, 1);
        return HMAC_TX_PASS;
    }
#endif
    ucast_sta_cnt = hmac_m2u_get_snooplist_member(hmac_vap, (osal_u8 *)grpmac, (osal_u8 *)src_ip_addr,
        ucast_sta_mac[0], vlan_tag); // 找出接收该组播的 sta 个数及他们的mac
    /* 如果没有STA需要此netbuf,不进行组播转单播处理 */
    if (ucast_sta_cnt == 0) {
        oam_info_log1(0, OAM_SF_M2U, "vap[%d] {hmac_m2u_snoop_convert::no user need this packet}", hmac_vap->vap_id);
        OAM_STAT_VAP_INCR(hmac_vap->vap_id, tx_m2u_mcast_cnt, 1);
#ifdef _PRE_WLAN_FEATURE_HERA_MCAST
        // 直接丢弃ipv6包会导致ipv6 ping包失败，由HMAC_TX_DROP_MTOU_FAIL改为HMAC_TX_PASS
        if (oal_host2net_short(ETHER_TYPE_IPV6) != ether_data_type) {
            hmac_dft_print_drop_frame_info(THIS_FILE_ID, __LINE__, 1, buf);
            return HMAC_TX_DROP_MTOU_FAIL;
        }
#endif
        return HMAC_TX_PASS;
    }

    /* 如果有用户需要该netbuf，但不需要组播转单播，直接发送原组播 */
    if (m2u->mcast_mode == HMAC_M2U_MCAST_MAITAIN) { /* mcast_mode = 0 */
        oam_info_log3(0, OAM_SF_M2U, "vap_id[%d] {hmac_m2u_snoop_convert:: %d user found , mcast mode is %d}",
            hmac_vap->vap_id, ucast_sta_cnt, m2u->mcast_mode);
        OAM_STAT_VAP_INCR(hmac_vap->vap_id, tx_m2u_mcast_cnt, 1);
        return HMAC_TX_PASS;
    }

    tx_ctl = (mac_tx_ctl_stru *)OAL_NETBUF_CB(buf);

    /* 组播转单播 CB字段处理 */
    mac_get_cb_is_mcast(tx_ctl)     = OAL_FALSE;
    mac_get_cb_ack_polacy(tx_ctl)   = WLAN_TX_NORMAL_ACK;
    mac_get_cb_tx_user_idx(tx_ctl)  = MAC_INVALID_USER_ID;
    mac_get_cb_wme_tid_type(tx_ctl) = m2u->tid_num;
    mac_get_cb_wme_ac_type(tx_ctl)  = wlan_wme_tid_to_ac(m2u->tid_num);
    do {
        dstmac = &ucast_sta_mac[ucast_sta_idx][0];
        tx_ctl = (mac_tx_ctl_stru *)OAL_NETBUF_CB(buf);

        if (ucast_sta_cnt > 1) {
            copy_buf = oal_netbuf_copy(buf, GFP_ATOMIC);
        }

        /* 发送的目的地址和发来的源地址相同的异常处理 */
        if (oal_compare_mac_addr(dstmac, (const osal_u8 *)srcmac) == 0) {
            hmac_free_netbuf_list_etc(buf);
            oam_info_log1(0, OAM_SF_M2U, "vap_id[%d] {hmac_m2u_snoop_convert::dstmac == srcmac.}",
                hmac_vap->vap_id);

            /* 组播转单播发送循环的末尾处理 */
            hmac_m2u_convert_loop_end(copy_buf, &buf, &ucast_sta_cnt, &ucast_sta_idx);
            continue;
        }

        /* 如果接收端的STA的地址为空，则说明至少有一个STA加入到这个组后又离开了 */
        /* 这样的情况需要丢弃该帧 */
        if (oal_compare_mac_addr(dstmac, (const osal_u8 *)empty_entry_mac) == 0) {
            oam_info_log3(0, OAM_SF_M2U,
                "vap_id[%d] {hmac_m2u_snoop_convert::dstmac == NULL && ucast_sta_cnt is %d && discard mcast is %d}",
                hmac_vap->vap_id, ucast_sta_cnt, m2u->discard_mcast);
            if ((ucast_sta_cnt > 1) || (m2u->discard_mcast == OAL_TRUE)) {
                hmac_free_netbuf_list_etc(buf);
                /* 组播转单播发送循环的末尾处理 */
                hmac_m2u_convert_loop_end(copy_buf, &buf, &ucast_sta_cnt, &ucast_sta_idx);
                continue;
            } else { /* 如果只有一个地址为空的STA用户,将该MAC地址还原成组播组的MAC地址 */
                ucast_sta_cnt = 0;
                dstmac = ether_hdr->ether_dhost;

                /* 组播CB字段处理 */
                mac_get_cb_is_mcast(tx_ctl)      = OAL_TRUE;
                mac_get_cb_ack_polacy(tx_ctl)    = WLAN_TX_NO_ACK;
                mac_get_cb_wme_tid_type(tx_ctl)  = WLAN_TIDNO_BCAST;
                mac_get_cb_wme_ac_type(tx_ctl)   = wlan_wme_tid_to_ac(WLAN_TIDNO_BCAST);
            }
        }

        /* 组播用户的查找 */
        if ((ETHER_IS_MULTICAST(dstmac) != 0) && (!ETHER_IS_BROADCAST(dstmac))) {
            user_idx = hmac_vap->multi_user_idx;
            ul_ret = HMAC_TX_PASS;
        } else { /* 组播转单播用户的查找 */
            ul_ret = hmac_vap_find_user_by_macaddr_etc(hmac_vap, dstmac, &user_idx);
        }

        /* 用户未找到，丢弃该帧 */
        if (osal_unlikely(ul_ret != OAL_SUCC)) {
            oam_warning_log2(0, OAM_SF_M2U, "vap_id[%d] {hmac_m2u_snoop_convert::find user fail[%d].}",
                hmac_vap->vap_id, ul_ret);
            hmac_free_netbuf_list_etc(buf);
            hmac_m2u_convert_loop_end(copy_buf, &buf, &ucast_sta_cnt, &ucast_sta_idx);
            continue;
        }

        /* 转成HMAC的USER结构体 */
        hmac_user = (hmac_user_stru *)mac_res_get_hmac_user_etc(user_idx);
        if (osal_unlikely(hmac_user == OAL_PTR_NULL)) {
            oam_warning_log1(0, OAM_SF_CFG, "{hmac_m2u_snoop_convert::null param,user[%d].}", user_idx);
            hmac_free_netbuf_list_etc(buf);
            hmac_m2u_convert_loop_end(copy_buf, &buf, &ucast_sta_cnt, &ucast_sta_idx);
            continue;
        }
        /* 用户状态判断 */
        if (osal_unlikely(hmac_user->user_asoc_state != MAC_USER_STATE_ASSOC)) {
            if (ucast_sta_cnt > 0) {
                hmac_m2u_cleanup_snoopwds_node(hmac_user);
            }
            /* 发送失败计数 */
            hmac_m2u_snoop_convert_count(hmac_vap, ucast_sta_cnt, OAL_FAIL, buf);
            oam_warning_log2(0, OAM_SF_M2U, "vap_id[%d] {hmac_m2u_snoop_convert::asoc state is [%d].}",
                hmac_vap->vap_id, hmac_user->user_asoc_state);
            /* 组播转单播发送循环的末尾处理 */
            hmac_m2u_convert_loop_end(copy_buf, &buf, &ucast_sta_cnt, &ucast_sta_idx);
            continue;
        }

        /* 目标user指针 */
        mac_get_cb_tx_user_idx(tx_ctl) = (osal_u8)user_idx;

        if (ucast_sta_cnt <= 0) {
            /* 组播转单播,单播以太网头的封装 */
            ul_ret = hmac_m2u_tx_event(hmac_vap, hmac_user, buf);

            /* 组播转单播发送计数统计 */
            hmac_m2u_snoop_convert_count(hmac_vap, ucast_sta_cnt, ul_ret, buf);

            /* 组播转单播发送循环的末尾处理 */
            hmac_m2u_convert_loop_end(copy_buf, &buf, &ucast_sta_cnt, &ucast_sta_idx);
            continue;
        }

        /* 组播转单播，mac头的封装 */
        hmac_m2u_snoop_change_mac_hdr(m2u, &ucast_ether_hdr, buf);
#ifdef _PRE_WLAN_FEATURE_VIRTUAL_MULTI_STA
        if (OAL_TRUE == hmac_user->is_wds) {
            hmac_vap_stru           *hmac_vap_tmp;
            hmac_wds_stas_stru      *sta = OAL_PTR_NULL;
            hmac_vap_tmp = (hmac_vap_stru *)mac_res_get_hmac_vap(hmac_user->vap_id);
            /* 发送目的地址对应的sta挂在当前repeater下 */
            if (hmac_wds_find_sta(hmac_vap_tmp, (osal_u8 *)(&srcmac[0]), &sta) == OAL_SUCC) {
                hmac_free_netbuf_list_etc(buf);
                oam_info_log1(0, OAM_SF_M2U, "vap_id[%d] {hmac_m2u_snoop_convert::sta in wds tables.}",
                    hmac_vap->vap_id);
                hmac_m2u_convert_loop_end(copy_buf, &buf, &ucast_sta_cnt, &ucast_sta_idx);
                continue;
            }
        } else {
            oal_set_mac_addr((osal_u8 *)(&ucast_ether_hdr->ether_dhost[0]), &ucast_sta_mac[ucast_sta_idx][0]);
        }
#else
        {
            oal_set_mac_addr((osal_u8 *)(&ucast_ether_hdr->ether_dhost[0]), &ucast_sta_mac[ucast_sta_idx][0]);
        }
#endif
        tx_ctl->is_m2u_data = OSAL_TRUE;
        ul_ret = hmac_tx_ucast_process_etc(hmac_vap, buf, hmac_user, tx_ctl);
        if (osal_unlikely(ul_ret != HMAC_TX_PASS)) {
            if (ul_ret != HMAC_TX_BUFF) {
                /* 不等于HMAC_TX_BUFF，不缓存的直接释放 */
                oam_warning_log2(0, OAM_SF_M2U,
                    "vap_id[%d]{hmac_m2u_snoop_convert::hmac_tx_ucast_process_etc not pass or buff, ul_ret = [%d]}",
                    hmac_vap->vap_id, ul_ret);
                hmac_free_netbuf_list_etc(buf);
            }

            /* 组播转单播发送循环的末尾处理 */
            hmac_m2u_convert_loop_end(copy_buf, &buf, &ucast_sta_cnt, &ucast_sta_idx);
            continue;
        }

        ul_ret = hmac_m2u_tx_event(hmac_vap, hmac_user, buf);

        /* 组播转单播发送计数统计 */
        hmac_m2u_snoop_convert_count(hmac_vap, ucast_sta_cnt, ul_ret, buf);

        /* 组播转单播发送循环的末尾处理 */
        hmac_m2u_convert_loop_end(copy_buf, &buf, &ucast_sta_cnt, &ucast_sta_idx);
    } while ((ucast_sta_cnt > 0) && (m2u->snoop_enable == OAL_TRUE));
    return HMAC_TX_DONE;
}

/*****************************************************************************
 函 数 名  : hmac_m2u_snoop_list_init
 功能描述  : 发送方向的组播转单播处理
 输入参数  : hmac_vap vap结构体
*****************************************************************************/
OAL_STATIC osal_void hmac_m2u_snoop_list_init(hmac_vap_stru *hmac_vap)
{
    hmac_m2u_stru            *m2u  = (hmac_m2u_stru *)(hmac_vap->m2u);
    hmac_m2u_snoop_list_stru *snp_list = &(m2u->m2u_snooplist);

    snp_list->group_list_count = 0;
    snp_list->total_sta_num    = 0;
    snp_list->misc             = 0;
    OSAL_INIT_LIST_HEAD(&(snp_list->grp_list));
}

/*****************************************************************************
 函 数 名  : hmac_m2u_unicast_convert_multicast
 功能描述  : 接收方向的单播转组播处理
 输入参数  : hmac_vap vap结构体; buf netbuf 结构体
 输出参数  : ul_ret
*****************************************************************************/
WIFI_TCM_TEXT osal_void hmac_m2u_unicast_convert_multicast(hmac_vap_stru *hmac_vap, oal_netbuf_stru *pst_netbuf,
    hmac_msdu_stru *msdu)
{
    mac_llc_snap_stru                  *snap;
    mac_vlan_tag_stru                  *vlan_tag;
    osal_u16                          ether_type;
    osal_u16                          ether_data_type;
    osal_u8                          *ip_head;
    oal_ipv6hdr_stru                   *ipv6_hdr;
    mac_ip_header_stru                 *ip_header;

    /* pst_netbuf未转化为以太网格式的帧,此时只有8个字节的snap头空间 */
    snap = (mac_llc_snap_stru *)oal_netbuf_data(pst_netbuf);
    ether_type = snap->ether_type;

    /* 带vlan tag 的情况 */
    if (!ether_is_with_vlan_tag(ether_type)) {
        /* 非vlan  的情况 */
        ip_head = (osal_u8 *)(oal_netbuf_data(pst_netbuf) + ETHER_ADDR_LEN + 2); // 2：偏移8个字节长度的snap头空间
        ether_data_type = *((osal_u16 *)(oal_netbuf_data(pst_netbuf) + ETHER_ADDR_LEN));
        vlan_tag = OAL_PTR_NULL;
    } else {
        vlan_tag = (mac_vlan_tag_stru *)(oal_netbuf_data(pst_netbuf) +
                                             ETHER_ADDR_LEN); // 偏移1个mac地址的长度,获取外层vlan tag
        vlan_tag += 1;
        if (ether_is_with_vlan_tag(vlan_tag->tpid)) {                // 双层tag的情况，内层tag不考虑
            ether_data_type = *((osal_u16 *)(vlan_tag + 1));       // 跳过内层tag后获取eth  type
            ip_head = (osal_u8 *)(vlan_tag + 1) + 2;              // 2：跳过 type 和内层指向ip头起始地址
        } else {
            ether_data_type = *((osal_u16 *)vlan_tag);            // 跳过tag后获取eth  type
            ip_head = (osal_u8 *)vlan_tag + 2;                   // 2：跳过 type 和tag指向ip头起始地址
        }
        vlan_tag -= 1;                                           // 指向外层tag，作为参数传递到下层
    }

    if (ether_data_type == oal_host2net_short(ETHER_TYPE_IP)) {
        ip_header = (mac_ip_header_stru *)ip_head;

        if (oal_ipv4_is_multicast((osal_u8 *)(&(ip_header->daddr)))) {
            /* 根据组播ip还原mac */
            hmac_m2u_get_group_mac(msdu->da, (osal_u8 *)(&(ip_header->daddr)), OAL_IPV4_ADDR_SIZE);
            oam_info_log4(0, OAM_SF_M2U,
                "ipv4 group addr = [%02x]:[%02x]:[%02x]:[%02x]:[xx]:[xx]",
                msdu->da[0], msdu->da[1], msdu->da[2], msdu->da[3]); /* 打印 0 1 2 3 位mac */
        }
    } else if (ether_data_type == oal_host2net_short(ETHER_TYPE_IPV6)) {
        ipv6_hdr = (oal_ipv6hdr_stru *)(ip_head);

        if (oal_ipv6_is_multicast(ipv6_hdr->daddr.in6_u.u6_addr8) != 0) {
            hmac_m2u_get_group_mac(msdu->da, ipv6_hdr->daddr.in6_u.u6_addr8, OAL_IPV6_ADDR_SIZE);
            oam_info_log4(0, OAM_SF_M2U,
                "ipv6 group mac = [%02x]:[%02x]:[%02x]:[%02x]:[xx]:[xx]",
                msdu->da[0], msdu->da[1], msdu->da[2], msdu->da[3]); /* 打印 0 1 2 3 位mac */
        }
    } else {
        oam_info_log2(0, OAM_SF_M2U,
            "vap_id[%d] {hmac_m2u_unicast_convert_multicast::ether type is not IP Protocol.[0x%x]}",
            hmac_vap->vap_id, oal_host2net_short(ether_data_type));
        return;
    }
}

#ifdef _PRE_WLAN_FEATURE_HERA_MCAST
/*****************************************************************************
 函 数 名  : hmac_m2u_adaptive_list_init
 功能描述  : 配网模式初始化设置
 输入参数  : hmac_vap vap结构体
 输出参数  : 无
*****************************************************************************/
osal_void hmac_m2u_adaptive_list_init(hmac_vap_stru *hmac_vap)
{
    osal_u32          loop;
    hmac_m2u_stru      *m2u  = (hmac_m2u_stru *)(hmac_vap->m2u);

    /* 初始化链表 */
    for (loop = 0; loop < HMAC_M2U_ADAPTIVE_STA_HASHSIZE; loop++) {
        OSAL_INIT_LIST_HEAD(&(m2u->m2u_adaptive_hash[loop]));
    }
    m2u->adaptive_sta_count = 0;
}

/*****************************************************************************
 函 数 名  : hmac_m2u_clean_adaptive_list
 功能描述  : 清空配网STA链表
 输入参数  : hmac_vap vap结构体
 输出参数  : 无
*****************************************************************************/
OAL_STATIC osal_void hmac_m2u_clean_adaptive_list(hmac_vap_stru *hmac_vap)
{
    hmac_m2u_stru                       *m2u;
    hmac_m2u_adaptive_hash_stru         *hash_adaptive = OAL_PTR_NULL;
    struct osal_list_head                 *sta_list_entry;
    struct osal_list_head                 *sta_list_entry_temp;
    osal_u32                           loop;

    /* 获取配网STA链表头 */
    m2u = (hmac_m2u_stru *)(hmac_vap->m2u);

    /* 删除链表 */
    for (loop = 0; loop < HMAC_M2U_ADAPTIVE_STA_HASHSIZE; loop++) {
        osal_list_for_each_safe(sta_list_entry, sta_list_entry_temp,
            &(m2u->m2u_adaptive_hash[loop])) {
            hash_adaptive = osal_list_entry(sta_list_entry, hmac_m2u_adaptive_hash_stru, adaptive_entry);
            osal_list_del(sta_list_entry);
            oal_mem_free(hash_adaptive, OAL_TRUE);
        }
    }
    m2u->adaptive_sta_count = 0;
}

/*****************************************************************************
 函 数 名  : hmac_m2u_find_adaptive_list
 功能描述  : 根据配网设备的mac地址返回所需的组
 输入参数  : hmac_vap vap结构体; hmac_m2u_adaptive_list_update_stru *adaptive_list_entry待查找组播组信息
 输出参数  : adaptive_list_member 组播组结构体 OR 空指针
*****************************************************************************/
OAL_STATIC hmac_m2u_adaptive_hash_stru *hmac_m2u_find_adaptive_list(hmac_vap_stru  *hmac_vap,
    mac_m2u_adaptive_list_update_stru *adaptive_list_entry)
{
    hmac_m2u_adaptive_hash_stru         *hash_adaptive = OAL_PTR_NULL;
    struct osal_list_head                 *sta_list_entry;
    hmac_m2u_stru                       *m2u = (hmac_m2u_stru *)(hmac_vap->m2u);
    osal_u8                            hash;

    if (m2u == OAL_PTR_NULL) {
        return OAL_PTR_NULL;
    }

    /* 获取HASH桶值 以及HASH链表 */
    hash = (osal_u8)hmac_adaptive_cal_hash_value(adaptive_list_entry->new_member_mac);

    /* 遍历配网STA组链表，找到地址匹配的配网STA */
    osal_list_for_each(sta_list_entry, &(m2u->m2u_adaptive_hash[hash])) {
        hash_adaptive = osal_list_entry(sta_list_entry, hmac_m2u_adaptive_hash_stru, adaptive_entry);
        if ((!oal_compare_mac_addr(adaptive_list_entry->new_member_mac, hash_adaptive->adaptive_mac)) &&
            (!osal_memcmp(&(adaptive_list_entry->outer_vlan_tag), &(hash_adaptive->outer_vlan_tag),
                OAL_SIZEOF(mac_vlan_tag_stru)))) {
            return hash_adaptive;
        }
    }

    return OAL_PTR_NULL;
}

/*****************************************************************************
 函 数 名  : hmac_m2u_create_adaptive_list
 功能描述  : 根据配网STA的MAC地址创建一个组，如果此组存在则返回该组，否则重新创建
 输入参数  : hmac_vap vap结构体; hmac_m2u_adaptive_list_update_stru *list_entry,待 操作节点信息
 输出参数  : adaptive_list_member 配网STA结构体
*****************************************************************************/
OAL_STATIC hmac_m2u_adaptive_hash_stru *hmac_m2u_create_adaptive_list(hmac_vap_stru *hmac_vap,
    hmac_m2u_adaptive_list_update_stru *adaptive_list_entry)
{
    hmac_m2u_adaptive_hash_stru         *hash_adaptive     = OAL_PTR_NULL;
    hmac_m2u_stru                       *m2u               = (hmac_m2u_stru *)(hmac_vap->m2u);
    osal_u8                            hash;

    if (osal_unlikely(m2u == OAL_PTR_NULL)) {
        return hash_adaptive;
    }

    /* 获取HASH桶值 以及HASH链表 */
    hash = (osal_u8)hmac_adaptive_cal_hash_value(adaptive_list_entry->new_member_mac);

    hash_adaptive = hmac_m2u_find_adaptive_list(hmac_vap, adaptive_list_entry);
    if (hash_adaptive == OAL_PTR_NULL) {
        if (m2u->adaptive_sta_count >= MAX_STA_NUM_OF_ADAPTIVE) {
            oam_warning_log2(0, OAM_SF_M2U,
                "vap_id[%d] {hmac_m2u_create_adaptive_list::m2u->adaptive_sta_count is [%d].}",
                hmac_vap->vap_id, m2u->adaptive_sta_count);
            return OAL_PTR_NULL;
        }
        hash_adaptive = oal_mem_alloc(OAL_MEM_POOL_ID_LOCAL, OAL_SIZEOF(hmac_m2u_adaptive_hash_stru), OAL_TRUE);
        if (osal_unlikely(hash_adaptive == OAL_PTR_NULL)) {
            oam_warning_log1(0, OAM_SF_M2U, "vap_id[%d] {hmac_m2u_create_adaptive_list::hash_adaptive_new null.}",
                hmac_vap->vap_id);
            return hash_adaptive;
        }

        memset_s(hash_adaptive, OAL_SIZEOF(hmac_m2u_adaptive_hash_stru), 0x0, OAL_SIZEOF(hmac_m2u_adaptive_hash_stru));
        if (memcpy_s(&(hash_adaptive->outer_vlan_tag), OAL_SIZEOF(hash_adaptive->outer_vlan_tag),
            &(adaptive_list_entry->outer_vlan_tag), OAL_SIZEOF(mac_vlan_tag_stru)) != EOK) {
            oam_warning_log1(0, OAM_SF_M2U, "vap[%d]hmac_m2u_create_adaptive_list::memcpy vlan fail", hmac_vap->vap_id);
        }
        oal_set_mac_addr((osal_u8 *)hash_adaptive->adaptive_mac, (const osal_u8 *)adaptive_list_entry->new_member_mac);
        hash_adaptive->m2u_adaptive = OAL_FALSE;
        hash_adaptive->timestamp    = adaptive_list_entry->timestamp;
        hash_adaptive->adaptive_num = 1;
        osal_list_add_tail(&(hash_adaptive->adaptive_entry), &(m2u->m2u_adaptive_hash[hash]));
        m2u->adaptive_sta_count++;

        oam_warning_log2(0, OAM_SF_M2U, "vap_id[%d] {hmac_m2u_create_adaptive_list::create a new group.group num:[%d]}",
            hmac_vap->vap_id, m2u->adaptive_sta_count);
    }

    return hash_adaptive;
}

/*****************************************************************************
 功能描述  : 获取外层vlan tag
*****************************************************************************/
OAL_STATIC osal_void hmac_m2u_adaptive_inspecting_get_vlan(hmac_vap_stru *hmac_vap,
    hmac_m2u_adaptive_list_update_stru *adaptive_list_entry, const mac_vlan_tag_stru *vlan_tag)
{
    if (memcpy_s(&(adaptive_list_entry->outer_vlan_tag), OAL_SIZEOF(adaptive_list_entry->outer_vlan_tag),
        vlan_tag, OAL_SIZEOF(mac_vlan_tag_stru)) != EOK) {
        oam_warning_log1(0, OAM_SF_M2U, "vap[%d] hmac_m2u_adaptive_inspecting_get_vlan:memcpy fail", hmac_vap->vap_id);
    }
}

/*****************************************************************************
 函 数 名  : hmac_m2u_adaptive_inspecting
 功能描述  : 监听配网组播报文来创建配网STA链表
 输入参数  : hmac_vap vap结构体; buf netbuf结构体
 输出参数  : 无
*****************************************************************************/
osal_void hmac_m2u_adaptive_inspecting(hmac_vap_stru *hmac_vap, oal_netbuf_stru *buf)
{
    mac_ether_header_stru                *ether_header = (mac_ether_header_stru *)oal_netbuf_data(buf);
    /* source address which send the report and it is the member */
    osal_u8                            *src_addr = ether_header->ether_shost;
    /* list entry where all member details will be updated and passed on updating the adaptive list */
    hmac_m2u_adaptive_list_update_stru   adaptive_list_entry;
    hmac_m2u_adaptive_hash_stru          *hash_adaptive = OAL_PTR_NULL;
    hmac_m2u_stru                        *m2u = OAL_PTR_NULL;
    mac_vlan_tag_stru                    *vlan_tag = OAL_PTR_NULL;

    memset_s(&adaptive_list_entry, OAL_SIZEOF(adaptive_list_entry), 0, OAL_SIZEOF(adaptive_list_entry));
    m2u = (hmac_m2u_stru *)(hmac_vap->m2u);
    if (OAL_FALSE == m2u->adaptive_enable || OAL_FALSE == m2u->snoop_enable) {
        oam_info_log2(0, OAM_SF_M2U, "vap_id[%d] {hmac_m2u_adaptive_inspecting::adaptive is [%d] not enable}",
            hmac_vap->vap_id, m2u->frequency_enable);
        return;
    }

    /* 获取以太网头 */
    oal_set_mac_addr(adaptive_list_entry.new_member_mac, src_addr);
    adaptive_list_entry.hmac_vap = hmac_vap;
    adaptive_list_entry.timestamp = (osal_u32)oal_time_get_stamp_ms();

    /* 带vlan tag 的情况 */
    if (ether_is_with_vlan_tag(ether_header->ether_type)) {
        // 偏移2个mac地址的长度,获取外层vlan tag
        vlan_tag = (mac_vlan_tag_stru *)(oal_netbuf_data(buf) + (ETHER_ADDR_LEN << 1));
        hmac_m2u_adaptive_inspecting_get_vlan(hmac_vap, &adaptive_list_entry, vlan_tag);

        vlan_tag += 1; // 判断内层tag
        if (ether_is_with_vlan_tag(vlan_tag->tpid)) {
            vlan_tag += 1; // 不考虑内层tag，跳过
        }
    } else { /* 非vlan  的情况 */
        vlan_tag = OAL_PTR_NULL;
    }

    hash_adaptive = hmac_m2u_find_adaptive_list(hmac_vap, &adaptive_list_entry);
    if (hash_adaptive == OAL_PTR_NULL) {
        hash_adaptive = hmac_m2u_create_adaptive_list(hmac_vap, &adaptive_list_entry);
        if (osal_unlikely(hash_adaptive == OAL_PTR_NULL)) {
            oam_warning_log1(0, OAM_SF_M2U, "vap_id[%d] {hmac_m2u_adaptive_inspecting::list null.}", hmac_vap->vap_id);
            return;
        }
    } else {
        if (oal_time_get_runtime((hash_adaptive->timestamp), (adaptive_list_entry.timestamp)) > (m2u->threshold_time)) {
            hash_adaptive->adaptive_num = 1;
            hash_adaptive->timestamp    = adaptive_list_entry.timestamp;
            hash_adaptive->m2u_adaptive = OAL_FALSE;
        } else {
            hash_adaptive->timestamp    = adaptive_list_entry.timestamp;
            hash_adaptive->adaptive_num++;
            if (hash_adaptive->adaptive_num >= (m2u->adaptive_num)) {
                hash_adaptive->m2u_adaptive = OAL_TRUE;
                hash_adaptive->adaptive_num = m2u->adaptive_num;
            }
        }
    }
}

/*****************************************************************************
 函 数 名  : hmac_m2u_sa_is_hwsmart
 功能描述  : 组播报文是否属于hw智能家居单品
 输入参数  : ip_header ipv4头结构体
 输出参数  : OAL_FALSE OR OAL_TRUE
*****************************************************************************/
osal_u32 hmac_m2u_sa_is_hwsmart(osal_u16 ether_data_type, osal_u8 *ip_head)
{
    osal_u32                grp_addr;
    mac_ip_header_stru       *ip_header;

    if (ether_data_type != oal_host2net_short(ETHER_TYPE_IP)) {
        return OAL_FALSE;
    }

    ip_header = (mac_ip_header_stru *)ip_head;
    grp_addr   = oal_host2net_long(ip_header->daddr);
    if ((grp_addr == 0xeeeeeeee) ||
        (((grp_addr & 0xffff0000) >> 16) == 0xef7e) || /* 16:偏移取值 */
        (((grp_addr & 0xffff0000) >> 16) == 0xef76) || /* 16:偏移取值 */
        (((grp_addr & 0xffff0000) >> 16) == 0xef0a) || /* 16:偏移取值 */
        (((grp_addr & 0xffff0000) >> 16) == 0xef14) || /* 16:偏移取值 */
        (((grp_addr & 0xffff0000) >> 16) == 0xef1e) || /* 16:偏移取值 */
        (((grp_addr & 0xffff0000) >> 16) == 0xef28)) { /* 16:偏移取值 */
        return OAL_TRUE;
    } else {
        return OAL_FALSE;
    }
}

/*****************************************************************************
 函 数 名  : hmac_m2u_sta_is_adaptive
 功能描述  : 判断是否处于配网模式状态
 输入参数  : hmac_vap vap结构体; mac_addr组播报文源mac地址
 输出参数  : OAL_FALSE OR OAL_TRUE
*****************************************************************************/
osal_u32 hmac_m2u_sta_is_adaptive(hmac_vap_stru *hmac_vap, osal_u8 *mac_addr,
    mac_vlan_tag_stru *vlan_tag)
{
    hmac_m2u_adaptive_hash_stru           *hash_adaptive = OAL_PTR_NULL;
    hmac_m2u_adaptive_list_update_stru     adaptive_list_entry;
    osal_u32 ret = OAL_FALSE;

    memset_s(&adaptive_list_entry, OAL_SIZEOF(adaptive_list_entry), 0, OAL_SIZEOF(adaptive_list_entry));
    if (memcpy_s(&(adaptive_list_entry.new_member_mac), OAL_SIZEOF(adaptive_list_entry.new_member_mac),
        mac_addr, WLAN_MAC_ADDR_LEN) != EOK) {
        oam_warning_log1(0, OAM_SF_M2U, "vap[%d]hmac_m2u_sta_is_adaptive::memcpy addr fail", hmac_vap->vap_id);
    }
    if (vlan_tag != OAL_PTR_NULL) {
        if (memcpy_s(&(adaptive_list_entry.outer_vlan_tag), OAL_SIZEOF(adaptive_list_entry.outer_vlan_tag),
            vlan_tag, OAL_SIZEOF(mac_vlan_tag_stru)) != EOK) {
            oam_warning_log1(0, OAM_SF_M2U, "vap[%d]hmac_m2u_sta_is_adaptive::memcpy vlan fail", hmac_vap->vap_id);
        }
    }

    hash_adaptive = hmac_m2u_find_adaptive_list(hmac_vap, &adaptive_list_entry);
    if (hash_adaptive != OAL_PTR_NULL) {
        ret = hash_adaptive->m2u_adaptive;
    }

    return ret;
}

/*****************************************************************************
 函 数 名  : hmac_m2u_sa_is_hwsmart
 功能描述  : 组播报文是否为异频转发
 输入参数  : hmac_vap vap结构体; mac_addr组播报文源mac地址
 输出参数  : OAL_FALSE OR OAL_TRUE
*****************************************************************************/
osal_u32 hmac_m2u_multicast_is_frequency(hmac_vap_stru *hmac_vap, osal_u8 *mac_addr)
{
    hmac_vap_stru                *hmac_vap;
    hmac_device_stru             *hmac_device;
    osal_u16                    vap_idx;
    osal_u16                    user_idx = 0;
    wlan_channel_band_enum_uint8  freq_band_now;

    if (osal_unlikely(hmac_vap == OAL_PTR_NULL)) {
        oam_error_log0(0, OAM_SF_M2U, "hmac_m2u_multicast_is_frequency::hmac_vap is null}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_device = hmac_res_get_mac_dev_etc((osal_u32)hmac_vap->device_id);
    freq_band_now = hmac_vap->channel.band;

    if (osal_unlikely(hmac_device == OAL_PTR_NULL)) {
        oam_error_log2(0, OAM_SF_M2U, "vap_id[%d] {hmac_m2u_multicast_is_frequency::hmac_device[%d] is NULL!}",
            hmac_vap->vap_id, hmac_vap->device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    for (vap_idx = 0; vap_idx < hmac_device->vap_num; vap_idx++) {
        hmac_vap = mac_res_get_hmac_vap(hmac_device->vap_id[vap_idx]);
        if (osal_unlikely(hmac_vap == OAL_PTR_NULL)) {
            continue;
        }

        if (hmac_vap->vap_mode != WLAN_VAP_MODE_BSS_AP) {
            continue;
        }

        if ((hmac_vap->channel.band == freq_band_now) &&
            (hmac_vap_find_user_by_macaddr_etc(hmac_vap, mac_addr, &user_idx) == OAL_SUCC)) {
            return OAL_FALSE;
        }
    }
    return OAL_TRUE;
}

/*****************************************************************************
 函 数 名  : hmac_m2u_multicast_drop
 功能描述  : 发送方向不满足组播发送的进行DROP处理
 输入参数  : hmac_vap vap结构体; buf netbuf 结构体
 输出参数  : HMAC_TX_DROP_MCAST OR HMAC_TX_PASS
*****************************************************************************/
osal_u32 hmac_m2u_multicast_drop(hmac_vap_stru *hmac_vap, oal_netbuf_stru *buf)
{
    mac_ether_header_stru  *ether_hdr;
    osal_u16              ether_data_type;
    osal_u8              *ip_head;
    mac_vlan_tag_stru      *vlan_tag;
    hmac_m2u_stru          *m2u = (hmac_m2u_stru *)(hmac_vap->m2u);

    /* 未打开组转单播开关，直接返回不处理 */
    if (OAL_FALSE == m2u->snoop_enable) {
        oam_warning_log2(0, OAM_SF_M2U, "vap_id[%d] {hmac_m2u_multicast_drop::snoop is [%d] not enable}",
            hmac_vap->vap_id, m2u->snoop_enable);
        return HMAC_TX_PASS;
    }
    ether_hdr  = (mac_ether_header_stru *)oal_netbuf_data(buf);
    /* 带vlan tag 的情况 */
    if (ether_is_with_vlan_tag(ether_hdr->ether_type)) {
        // 偏移2个mac地址的长度,获取外层vlan tag
        vlan_tag = (mac_vlan_tag_stru *)(oal_netbuf_data(buf) + (ETHER_ADDR_LEN << 1));
        vlan_tag += 1;
        if (ether_is_with_vlan_tag(vlan_tag->tpid)) {                // 双层tag的情况，内层tag不考虑
            ether_data_type = *((osal_u16 *)(vlan_tag + 1));       // 跳过内层tag后获取eth  type
            ip_head = (osal_u8 *)(vlan_tag + 1) + 2;              // 2：跳过 type 和内层指向ip头起始地址
        } else {
            ether_data_type = *((osal_u16 *)vlan_tag);            // 跳过tag后获取eth  type
            ip_head = (osal_u8 *)vlan_tag + 2;                   // 2：跳过 type 和tag指向ip头起始地址
        }
        vlan_tag -= 1;                                                  // 指向外层tag，作为参数传递到下层
    } else { /* 非vlan  的情况 */
        vlan_tag = OAL_PTR_NULL;
        ip_head = (osal_u8 *)(ether_hdr + 1);
        ether_data_type = ether_hdr->ether_type;
    }

    /* 异频组播转发开关未打开 */
    if (m2u->frequency_enable == OAL_FALSE) {
        /* 异频组播转发关闭，不满足智能家居单品特征 */
        if ((hmac_m2u_sa_is_hwsmart(ether_data_type, ip_head) == OAL_FALSE) && (m2u->discard_mcast)) {
            hmac_dft_print_drop_frame_info(THIS_FILE_ID, __LINE__, 1, buf);
            return HMAC_TX_DROP_NOSMART;
        }
    } else { /* 异频组播转发开关打开 */
        /* 组播报文属于异频转发 */
        if (hmac_m2u_multicast_is_frequency(hmac_vap, ether_hdr->ether_shost)) {
            /* 组播报文来源STA不在配网模式 */
            if (((m2u->adaptive_enable == OAL_FALSE) ||
                (hmac_m2u_sta_is_adaptive(hmac_vap, ether_hdr->ether_shost, vlan_tag) == OAL_FALSE)) &&
                (m2u->discard_mcast)) {
                hmac_dft_print_drop_frame_info(THIS_FILE_ID, __LINE__, 1, buf);
                return HMAC_TX_DROP_NOADAP;
            }
        } else { /* 组播报文不属于异频转发 */
            /* 组播报文不属于异频转发,不满足智能家居单品特征 */
            if ((hmac_m2u_sa_is_hwsmart(ether_data_type, ip_head) == OAL_FALSE) && (m2u->discard_mcast)) {
                hmac_dft_print_drop_frame_info(THIS_FILE_ID, __LINE__, 1, buf);
                return HMAC_TX_DROP_NOSMART;
            }
        }
    }
    return HMAC_TX_PASS;
}

/*****************************************************************************
 函 数 名  : hmac_m2u_remove_old_sta
 功能描述  : 删除老化的配网STA
 输入参数  : p_arg 输入参数
 输出参数  : 无
*****************************************************************************/
osal_u32  hmac_m2u_remove_old_sta(osal_void *p_arg)
{
    hmac_vap_stru *hmac_vap                    = (hmac_vap_stru *)p_arg;
    hmac_m2u_stru *m2u                         = (hmac_m2u_stru *)(hmac_vap->m2u);
    hmac_m2u_adaptive_hash_stru *hash_adaptive = OAL_PTR_NULL;
    struct osal_list_head                 *sta_list_entry;
    struct osal_list_head                 *sta_list_entry_temp;
    osal_u32  nowtime = (osal_u32)oal_time_get_stamp_ms();
    osal_u32  loop;

    if (m2u == OAL_PTR_NULL) {
        return OAL_FAIL;
    }

    /* 遍历配网STA组链表，找到地址匹配的配网STA */
    for (loop = 0; loop < HMAC_M2U_ADAPTIVE_STA_HASHSIZE; loop++) {
        osal_list_for_each_safe(sta_list_entry, sta_list_entry_temp,
            &(m2u->m2u_adaptive_hash[loop])) {
            hash_adaptive = osal_list_entry(sta_list_entry,
                hmac_m2u_adaptive_hash_stru, adaptive_entry);
            if ((hash_adaptive->m2u_adaptive == OAL_FALSE) ||
                ((hash_adaptive->m2u_adaptive == OAL_TRUE) &&
                 (oal_time_get_runtime((hash_adaptive->timestamp), nowtime) >= (m2u->adaptive_timeout)))) {
                osal_list_del(&(hash_adaptive->adaptive_entry));
                oal_mem_free(hash_adaptive, OAL_TRUE);
                m2u->adaptive_sta_count--;
            }
        }
    }
    return OAL_SUCC;
}
#endif

/*****************************************************************************
 函 数 名  : hmac_m2u_time_fn
 功能描述  : 组播转单播的定时器，删除长时间沉默的成员
 输入参数  : p_arg 输入参数
*****************************************************************************/
OAL_STATIC osal_u32 hmac_m2u_time_fn(osal_void *p_arg)
{
    hmac_vap_stru *hmac_vap            = (hmac_vap_stru *)p_arg;
    hmac_m2u_stru *m2u                 = (hmac_m2u_stru *)(hmac_vap->m2u);
    hmac_m2u_snoop_list_stru *snp_list = &(m2u->m2u_snooplist);
    hmac_m2u_grp_list_entry_stru  *grp_list;
    struct osal_list_head           *grp_list_entry;
    struct osal_list_head           *grp_list_entry_temp;
    osal_u32                     ul_ret;

    osal_u32  nowtime = (osal_u32)oal_time_get_stamp_ms();

    osal_list_for_each_safe(grp_list_entry, grp_list_entry_temp, &(snp_list->grp_list)) {
        grp_list = (hmac_m2u_grp_list_entry_stru *)osal_list_entry(grp_list_entry,
            hmac_m2u_grp_list_entry_stru, grp_entry);
        ul_ret = hmac_m2u_remove_expired_member(grp_list, hmac_vap, nowtime); // 组成员
        if (ul_ret != OAL_SUCC) {
            return ul_ret;
        }

        if ((osal_list_empty(&(grp_list->src_list))) == OAL_TRUE) {
            osal_list_del(&(grp_list->grp_entry));
            oal_mem_free(grp_list, OAL_TRUE);
            grp_list = OAL_PTR_NULL;
            snp_list->group_list_count--;
        }
    }
    return OAL_SUCC;
}

OAL_STATIC osal_void hmac_m2u_sepcial_group_init(hmac_m2u_snoop_list_stru *snp_list)
{
    /* 特殊业务组播 IPV4 */
    snp_list->special_group_ipv4[0] = HMAC_M2U_SPECIAL_GROUP1;    /* 224.0.0.1 */
    snp_list->special_group_ipv4[1] = HMAC_M2U_SPECIAL_GROUP2;    /* 224.0.0.2 */
    snp_list->special_group_ipv4[2] = HMAC_M2U_RIPV2_GROUP;       /* 224.0.0.9 */
    snp_list->special_group_ipv4[3] = HMAC_M2U_SPECIAL_GROUP3;    /* 3：224.0.0.22 */
    snp_list->special_group_ipv4[4] = HMAC_M2U_UPNP_GROUP;        /* 239.255.255.250 */
    /* 特殊业务组播 IPV6 */
    /* IPV6 ff02::1 */
    memset_s(snp_list->special_group_ipv6[0], OAL_IPV6_ADDR_SIZE, 0, OAL_IPV6_ADDR_SIZE);
    snp_list->special_group_ipv6[0][0]  = 0xff;
    snp_list->special_group_ipv6[0][1]  = 0x2;
    snp_list->special_group_ipv6[0][15] = 0x1; /* 15：ipv6第15位取地址 */
    /* IPV6 ff02::2 */
    memset_s(snp_list->special_group_ipv6[1], OAL_IPV6_ADDR_SIZE, 0, OAL_IPV6_ADDR_SIZE);
    snp_list->special_group_ipv6[1][0]  = 0xff;
    snp_list->special_group_ipv6[1][1]  = 0x2;
    snp_list->special_group_ipv6[1][15] = 0x2; /* 15：ipv6第15位取地址 */
    /* IPV6 ff02::16 */
    memset_s(snp_list->special_group_ipv6[2], OAL_IPV6_ADDR_SIZE, 0, OAL_IPV6_ADDR_SIZE);
    snp_list->special_group_ipv6[2][0]  = 0xff; /* 2：特殊业务组播组 */
    snp_list->special_group_ipv6[2][1]  = 0x2; /* 2：特殊业务组播组 */
    snp_list->special_group_ipv6[2][15] = 0x16; /* 2：特殊业务组播组，15：ipv6第15位取地址 */
    /* IPV6 ff02::1:2 */
    memset_s(snp_list->special_group_ipv6[3], OAL_IPV6_ADDR_SIZE, 0, OAL_IPV6_ADDR_SIZE); /* 3：特殊业务组播组 */
    snp_list->special_group_ipv6[3][0]  = 0xff; /* 3：特殊业务组播组 */
    snp_list->special_group_ipv6[3][1]  = 0x2; /* 3：特殊业务组播组 */
    snp_list->special_group_ipv6[3][13] = 0x1; /* 3：特殊业务组播组 */ /* 13：ipv6第13位取地址 */
    snp_list->special_group_ipv6[3][15] = 0x2; /* 3：特殊业务组播组 */ /* 15：ipv6第15位取地址 */
    /* IPV6 ff02::1:ff02:b */
    memset_s(snp_list->special_group_ipv6[4], OAL_IPV6_ADDR_SIZE, 0, OAL_IPV6_ADDR_SIZE); /* 4：特殊业务组播组 */
    snp_list->special_group_ipv6[4][0]  = 0xff; /* 4：特殊业务组播组 */
    snp_list->special_group_ipv6[4][1]  = 0x2; /* 4：特殊业务组播组 */
    snp_list->special_group_ipv6[4][11] = 0x1; /* 4：特殊业务组播组 */ /* 11：ipv6第12位取地址 */
    snp_list->special_group_ipv6[4][12] = 0xff; /* 4：特殊业务组播组，12：ipv6第12位取地址 */
    snp_list->special_group_ipv6[4][13] = 0x2; /* 4：特殊业务组播组，13：ipv6第13位取地址 */
    snp_list->special_group_ipv6[4][15] = 0xb; /* 4：特殊业务组播组，15：ipv6第15位取地址 */
}

OAL_STATIC osal_void hmac_m2u_deny_table_init(hmac_m2u_snoop_list_stru *snp_list)
{
    /* 永久组播组(224.0.0.0~224.0.0.255)中的组播已加入到组播黑名单；不在此范围中的组播组，添加到deny_group */
    snp_list->deny_count_ipv4  = DEFAULT_IPV4_DENY_GROUP_COUNT; /* 默认额外添加1个组播为黑名单 */
    snp_list->deny_group[0]   = HMAC_M2U_DENY_GROUP;       /* 239.255.255.1 */

    snp_list->deny_count_ipv6  = DEFAULT_IPV6_DENY_GROUP_COUNT;
    memset_s(snp_list->deny_group_ipv6[0], OAL_IPV6_ADDR_SIZE, 0, OAL_IPV6_ADDR_SIZE);
    snp_list->deny_group_ipv6[0][0]  = 0xff; /* IPV6 LLMNR协议ff02::1:3 */
    snp_list->deny_group_ipv6[0][1]  = 0x2;
    snp_list->deny_group_ipv6[0][13] = 0x1; /* 13：ipv6第13位取地址 */
    snp_list->deny_group_ipv6[0][15] = 0x3; /* 15：ipv6第15位取地址 */
}

/*****************************************************************************
 函 数 名  : hmac_m2u_attach
 功能描述  : 组播转单播的初始化函数
 输入参数  : hmac_vap vap结构体
 输出参数  : 无
*****************************************************************************/
osal_void hmac_m2u_attach(hmac_vap_stru *hmac_vap)
{
    hmac_m2u_stru            *m2u = OSAL_NULL;
    hmac_m2u_snoop_list_stru *snp_list = OSAL_NULL;

    m2u = (hmac_m2u_stru *)oal_mem_alloc(OAL_MEM_POOL_ID_LOCAL, OAL_SIZEOF(hmac_m2u_stru), OAL_TRUE);
    if (osal_unlikely(m2u == OAL_PTR_NULL)) {
        oam_error_log1(0, OAM_SF_M2U, "vap_id[%d] {hmac_m2u_attach::m2u is null!}",
            hmac_vap->vap_id);
        return;
    }

    hmac_vap->m2u = m2u;
    snp_list = &(m2u->m2u_snooplist);

    memset_s(m2u, OAL_SIZEOF(hmac_m2u_stru), 0, OAL_SIZEOF(hmac_m2u_stru));
    /* 启动定时器 */
    frw_create_timer_entry(&(m2u->snooplist_timer), hmac_m2u_time_fn, HMAC_DEF_M2U_TIMER, hmac_vap,
        OAL_TRUE);

    m2u->snoop_enable = OAL_TRUE;  /* 默认使能，组播转发时需要根据snoop表确认是否有用户再转发 */
#ifdef _PRE_WLAN_FEATURE_HERA_MCAST
    m2u->adaptive_enable  = OAL_TRUE;  /* 配网模式识别默认使能 */
    m2u->frequency_enable = OAL_TRUE;  /* 异频组播转发默认使能 */
#endif

#ifdef _PRE_WLAN_FEATURE_HERA_MCAST
    /* 启动配网老化定时器 1000:定时器1秒 */
    frw_create_timer_entry(&(m2u->adaptivelist_timer), hmac_m2u_remove_old_sta, 1000, hmac_vap,
        OAL_TRUE);

    hmac_m2u_adaptive_list_init(hmac_vap);

    m2u->threshold_time        = HMAC_DEF_THRESHOLD_TIME;
    m2u->adaptive_timeout      = HMAC_DEF_ADAPTIVE_TIMEOUT;
    m2u->adaptive_num          = HMAC_DEF_NUM_OF_ADAPTIVE;
#endif

    hmac_m2u_sepcial_group_init(snp_list);

    m2u->discard_mcast         = OAL_TRUE;
    m2u->timeout               = HMAC_DEF_M2U_TIMEOUT;

    m2u->mcast_mode            = HMAC_M2U_MCAST_TRANSLATE;  /* 默认打开组播转单播开关 */
    hmac_m2u_deny_table_init(snp_list);

    snp_list->total_sta_num    = 0;
    snp_list->group_list_count = 0;
    hmac_m2u_snoop_list_init(hmac_vap);
}

/*****************************************************************************
 函 数 名  : hmac_m2u_detach
 功能描述  : 组播转单播的detach
 输入参数  : hmac_vap vap结构体
 输出参数  : 无
*****************************************************************************/
osal_void hmac_m2u_detach(hmac_vap_stru *hmac_vap)
{
    hmac_m2u_stru    *m2u  = (hmac_m2u_stru *)(hmac_vap->m2u);

    if (m2u != OAL_PTR_NULL) {
        hmac_m2u_clean_snp_list(hmac_vap);
        frw_destroy_timer_entry(&(m2u->snooplist_timer));
#ifdef _PRE_WLAN_FEATURE_HERA_MCAST
        frw_destroy_timer_entry(&(m2u->adaptivelist_timer));
        hmac_m2u_clean_adaptive_list(hmac_vap);
#endif
        oal_mem_free(m2u, OAL_TRUE);
        hmac_vap->m2u = OAL_PTR_NULL;
    }
}

OAL_STATIC osal_void hmac_m2u_feature_hook_register(osal_void)
{
    /* 对外接口注册 */
    hmac_feature_hook_register(HMAC_FHOOK_M2U_SNOOP_INSPECTING, hmac_m2u_snoop_inspecting);
    hmac_feature_hook_register(HMAC_FHOOK_M2U_CLEANUP_SNOOPWDS_NODE, hmac_m2u_cleanup_snoopwds_node);
    hmac_feature_hook_register(HMAC_FHOOK_M2U_SNOOP_CONVERT, hmac_m2u_snoop_convert);
    hmac_feature_hook_register(HMAC_FHOOK_M2U_UNICAST_CONVERT_MULTICAST, hmac_m2u_unicast_convert_multicast);
    hmac_feature_hook_register(HMAC_FHOOK_M2U_ATTACH, hmac_m2u_attach);
    hmac_feature_hook_register(HMAC_FHOOK_M2U_DETACH, hmac_m2u_detach);
    return;
}

OAL_STATIC osal_void hmac_m2u_feature_hook_unregister(osal_void)
{
    /* 对外接口去注册 */
    hmac_feature_hook_unregister(HMAC_FHOOK_M2U_SNOOP_INSPECTING);
    hmac_feature_hook_unregister(HMAC_FHOOK_M2U_CLEANUP_SNOOPWDS_NODE);
    hmac_feature_hook_unregister(HMAC_FHOOK_M2U_SNOOP_CONVERT);
    hmac_feature_hook_unregister(HMAC_FHOOK_M2U_UNICAST_CONVERT_MULTICAST);
    hmac_feature_hook_unregister(HMAC_FHOOK_M2U_ATTACH);
    hmac_feature_hook_unregister(HMAC_FHOOK_M2U_DETACH);
    return;
}

/*
 * 功能描述: 打开组播转单播snoop开关
 * 注意事项: 配置开关的同时，也需要配置模式
 */
OAL_STATIC osal_s32 hmac_m2u_snoop_on(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    mac_cfg_m2u_snoop_on_param_stru *m2u_snoop_on_param = OSAL_NULL;
    hmac_m2u_stru *m2u = OSAL_NULL;

    if ((hmac_vap == OSAL_NULL) || (msg->data == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_M2U, "{hmac_config_m2u_snoop_on:null ptr}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_m2u_attach(hmac_vap);
    m2u = (hmac_m2u_stru *)(hmac_vap->m2u);
    if (m2u == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_M2U, "{hmac_config_m2u_snoop_on:null m2u ptr}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    m2u_snoop_on_param = (mac_cfg_m2u_snoop_on_param_stru *)msg->data;

    if ((m2u_snoop_on_param->m2u_snoop_on != 0) && (m2u_snoop_on_param->m2u_snoop_on != 1)) {
        oam_error_log1(0, OAM_SF_M2U, "{hmac_config_m2u_snoop_on:error param:%d", m2u_snoop_on_param->m2u_snoop_on);
        return OAL_FAIL;
    }
    m2u->snoop_enable = (m2u_snoop_on_param->m2u_snoop_on == 0) ? OAL_FALSE : OAL_TRUE;

    if (m2u->snoop_enable == 1) {
        hmac_m2u_feature_hook_register();
    } else {
        hmac_m2u_feature_hook_unregister();
    }

    if ((m2u_snoop_on_param->m2u_mcast_mode != 0) && (m2u_snoop_on_param->m2u_mcast_mode != 1) &&
        (m2u_snoop_on_param->m2u_mcast_mode != 2)) { /* 0表示维持maitain 1表示tunnel 2表示组播转单播转换 */
        oam_error_log1(0, OAM_SF_M2U, "{hmac_config_m2u_snoop_on:error mode:%d", m2u_snoop_on_param->m2u_mcast_mode);
        return OAL_FAIL;
    }
    m2u->mcast_mode = m2u_snoop_on_param->m2u_mcast_mode;
    oam_warning_log0(0, OAM_SF_M2U, "CCPRIV m2u_snoop_enable OK");
    return OAL_SUCC;
}

OAL_STATIC osal_s32 hmac_show_m2u_snoop_table(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    unref_param(msg);
    if (hmac_vap == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_M2U, "{hmac_config_m2u_snoop_on:null ptr}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* show snoop 表 */
    hmac_m2u_print_all_snoop_list(hmac_vap, OSAL_NULL);
    return OAL_SUCC;
}

OAL_STATIC osal_s32 hmac_m2u_deny_table(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    mac_cfg_m2u_deny_table_stru *cfg_deny_table = OSAL_NULL;

    if ((hmac_vap == OSAL_NULL) || (msg->data == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_M2U, "{hmac_config_m2u_snoop_on:null ptr}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    cfg_deny_table = (mac_cfg_m2u_deny_table_stru *)msg->data;
    switch (cfg_deny_table->cfg_type) {
        case HMAC_H2U_DENY_TABLE_LIST:
            hmac_m2u_show_snoop_deny_table(hmac_vap);
            break;
        case HMAC_H2U_DENY_TABLE_CLEAR:
            hmac_m2u_clear_deny_table(hmac_vap);
            break;
        case HMAC_H2U_DENY_TABLE_ADD_IPV4:
            hmac_m2u_add_snoop_ipv4_deny_entry(hmac_vap, &cfg_deny_table->deny_group_ipv4_addr);
            break;
        case HMAC_H2U_DENY_TABLE_DEL_IPV4:
            hmac_m2u_del_ipv4_deny_entry(hmac_vap, &cfg_deny_table->deny_group_ipv4_addr);
            break;
        case HMAC_H2U_DENY_TABLE_ADD_IPV6:
            hmac_m2u_add_snoop_ipv6_deny_entry(hmac_vap, cfg_deny_table->deny_group_ipv6_addr);
            break;
        case HMAC_H2U_DENY_TABLE_DEL_IPV6:
            hmac_m2u_del_ipv6_deny_entry(hmac_vap, cfg_deny_table->deny_group_ipv6_addr);
            break;
        default:
            break;
    }
    return OAL_SUCC;
}

/*
 * 功能描述: 组一个igmpv2 reprt报文
 * 注意事项: size表示报文长度， 包含以太网头部， 不包括FCS， 取值范围应该为60~1514
 */
OAL_STATIC oal_netbuf_stru* hmac_m2u_create_igmp_packet(osal_u32 size, osal_u8 tid, osal_u8 *mac_ra, osal_u8 *mac_ta)
{
    oal_netbuf_stru *net_buf = OSAL_NULL;
    mac_ether_header_stru *ether_header = OSAL_NULL;
    mac_ip_header_stru *ip_header = OSAL_NULL;
    mac_igmp_header_stru *igmp_hdr = OSAL_NULL;
    osal_u32 reserve = 256;   /* 预留256个字节 */

    net_buf = oal_netbuf_alloc(size + reserve, reserve, 4); /* 4字节对齐 */
    if (osal_unlikely(net_buf == OAL_PTR_NULL)) {
        return OAL_PTR_NULL;
    }

    oal_netbuf_put(net_buf, size);
    oal_set_mac_addr(&net_buf->data[0], mac_ra);
    oal_set_mac_addr(&net_buf->data[6], mac_ta); /* 报文第6位填入ta mac地址 */

    ether_header = (mac_ether_header_stru *)oal_netbuf_data(net_buf);
    ether_header->ether_type = oal_host2net_short(ETHER_TYPE_IP);
    ip_header = (mac_ip_header_stru *)(ether_header + 1);      /* 偏移以太网头，取ip头 */
    ip_header->version_ihl = 0x45;  /* 0x45 ip协议号和长度 */
    ip_header->tos = (osal_u8)(tid << WLAN_IP_PRI_SHIFT);
    ip_header->protocol = IPPROTO_IGMP;
    ip_header->tot_len = oal_byteorder_net_to_host_uint16(0x0020);  /* 0x0020报文长度 */
    ip_header->frag_off = 0x0000;
    ip_header->ttl = 0x01;

    /* 指向igmp头指针 */
    igmp_hdr = (mac_igmp_header_stru *)(ip_header + 1);
    igmp_hdr->type = MAC_IGMPV2_REPORT_TYPE;
    igmp_hdr->code = 0x64;   /* 0x64代表max resp time,协议建议值 */
    igmp_hdr->csum = oal_byteorder_net_to_host_uint16(0xfafb);  /* igmp校验码是0xfafb */
    igmp_hdr->group = oal_byteorder_host_to_net_uint32(0xef000003);  /* 固定组为0xef000003，即239.0.0.3进行测试 */

    net_buf->next = OAL_PTR_NULL;
    net_buf->prev = OAL_PTR_NULL;

    memset_s(oal_netbuf_cb(net_buf), OAL_NETBUF_CB_SIZE(), 0, OAL_NETBUF_CB_SIZE());
    return net_buf;
}

/*
 * 功能描述: 发送一个IGMP报文
 * 注意事项: size表示报文长度， 包含以太网头部， 不包括FCS， 取值范围应该为60~1514
 */
OAL_STATIC osal_s32 hmac_m2u_igmp_packet_xmit(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    mac_cfg_mpdu_ampdu_tx_param_stru *ampdu_tx_on_param = OSAL_NULL;
    osal_u8 skb_num;
    osal_u8 skb_idx;
    osal_u8 tid;
    osal_u16 packet_len;
    oal_net_device_stru *dev = OSAL_NULL;
    oal_netbuf_stru *netbuf[32] = {OAL_PTR_NULL};   /* 最多32个报文 */

    ampdu_tx_on_param = (mac_cfg_mpdu_ampdu_tx_param_stru *)msg->data;
    skb_num = ampdu_tx_on_param->packet_num;
    tid = ampdu_tx_on_param->tid;
    packet_len = ampdu_tx_on_param->packet_len;

    dev = hmac_vap_get_net_device_etc(hmac_vap->vap_id);
    if (dev == OAL_PTR_NULL) {
        return OAL_SUCC;
    }

    if (skb_num <= 32) { /* 最多32个报文 */
        for (skb_idx = 0; skb_idx < skb_num; skb_idx++) {
            netbuf[skb_idx] = hmac_m2u_create_igmp_packet(packet_len, tid,
                ampdu_tx_on_param->ra_mac,
                hmac_vap->mib_info->wlan_mib_sta_config.dot11_station_id);
            frw_host_post_data(FRW_NETBUF_W2H_DATA_FRAME, hmac_vap->vap_id, netbuf[skb_idx]);
        }
    }
    return OAL_SUCC;
}

osal_u32 hmac_m2u_init(osal_void)
{
    /* 消息注册 */
    frw_msg_hook_register(WLAN_MSG_W2H_CFG_M2U_SNOOP_ON, hmac_m2u_snoop_on);
    frw_msg_hook_register(WLAN_MSG_W2H_CFG_M2U_DENY_TABLE, hmac_m2u_deny_table);
    frw_msg_hook_register(WLAN_MSG_W2H_CFG_SHOW_M2U_SNOOP_TABLE, hmac_show_m2u_snoop_table);
    frw_msg_hook_register(WLAN_MSG_W2H_CFG_IGMP_PACKET_XMIT, hmac_m2u_igmp_packet_xmit);

    return OAL_SUCC;
}

osal_void hmac_m2u_deinit(osal_void)
{
    /* 消息去注册 */
    frw_msg_hook_unregister(WLAN_MSG_W2H_CFG_M2U_SNOOP_ON);
    frw_msg_hook_unregister(WLAN_MSG_W2H_CFG_M2U_DENY_TABLE);
    frw_msg_hook_unregister(WLAN_MSG_W2H_CFG_SHOW_M2U_SNOOP_TABLE);
    frw_msg_hook_unregister(WLAN_MSG_W2H_CFG_IGMP_PACKET_XMIT);

    return;
}

oal_module_symbol(hmac_m2u_snoop_inspecting);
oal_module_symbol(hmac_m2u_attach);
oal_module_symbol(hmac_m2u_snoop_convert);
oal_module_symbol(hmac_m2u_detach);
oal_module_symbol(hmac_m2u_clear_deny_table);
oal_module_symbol(hmac_m2u_print_all_snoop_list);
oal_module_symbol(hmac_m2u_show_snoop_deny_table);
oal_module_symbol(hmac_m2u_add_snoop_ipv4_deny_entry);
oal_module_symbol(hmac_m2u_add_snoop_ipv6_deny_entry);
oal_module_symbol(hmac_m2u_del_ipv4_deny_entry);
oal_module_symbol(hmac_m2u_del_ipv6_deny_entry);
oal_module_symbol(hmac_m2u_cleanup_snoopwds_node);
oal_module_symbol(hmac_m2u_unicast_convert_multicast);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
