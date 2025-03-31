/*
 * Copyright (c) CompanyNameMagicTag 2013-2023. All rights reserved.
 * 文 件 名   : hmac_encap_frame.c
 * 生成日期   : 2013年6月28日
 * 功能描述   : AP模式和STA模式共有帧的组帧文件
 */


/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "wlan_spec.h"
#include "mac_resource_ext.h"
#include "hmac_encap_frame.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_HMAC_ENCAP_FRAME_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/


/*****************************************************************************
  3 函数实现
*****************************************************************************/

/*****************************************************************************
     函 数 名  : hmac_encap_sa_query_req_etc
     功能描述  : 组sa query 请求帧
     输入参数  : hmac_vap :mac vap结构体
                 data    :netbuf data指针
                 da      :目标用户的mac地址
                 trans_id    :sa query ie,用于辨别response和request是否一致
     输出参数  : 无
     返 回 值  : 帧头+帧体的长度
*****************************************************************************/
osal_u16 hmac_encap_sa_query_req_etc(hmac_vap_stru *hmac_vap, osal_u8 *data,
    osal_u8 *da, osal_u16 trans_id)
{
    osal_u16 len;

    /*************************************************************************/
    /*                        Management Frame Format                        */
    /* --------------------------------------------------------------------  */
    /* |Frame Control|Duration|DA|SA|BSSID|Sequence Control|Frame Body|FCS|  */
    /* --------------------------------------------------------------------  */
    /* | 2           |2       |6 |6 |6    |2               |0 - 2312  |4  |  */
    /* --------------------------------------------------------------------  */
    /*                                                                       */
    /*************************************************************************/

    /*************************************************************************/
    /*                Set the fields in the frame header                     */
    /*************************************************************************/
    /* All the fields of the Frame Control Field are set to zero. Only the   */
    /* Type/Subtype field is set.                                            */
    mac_hdr_set_frame_control(data, WLAN_FC0_SUBTYPE_ACTION);
    /*  Set DA  */
    oal_set_mac_addr(((mac_ieee80211_frame_stru *)data)->address1, da);
    /*  Set SA  */
    oal_set_mac_addr(((mac_ieee80211_frame_stru *)data)->address2,  mac_mib_get_station_id(hmac_vap));
    /*  Set SSID  */
    oal_set_mac_addr(((mac_ieee80211_frame_stru *)data)->address3, hmac_vap->bssid);
    /* 设置分片序号为0 */
    mac_hdr_set_fragment_number(data, 0);

    /*************************************************************************/
    /*                Set the contents of the frame body                     */
    /*************************************************************************/

    /*************************************************************************/
    /*                  SA Query Frame - Frame Body                          */
    /* --------------------------------------------------------------------- */
    /* |   Category   |SA Query Action |  Transaction Identifier           | */
    /* --------------------------------------------------------------------- */
    /* |1             |1               |2 Byte                             | */
    /* --------------------------------------------------------------------- */
    /*                                                                       */
    /*************************************************************************/
    data[MAC_80211_FRAME_LEN] = MAC_ACTION_CATEGORY_SA_QUERY;
    data[MAC_80211_FRAME_LEN + 1] = MAC_SA_QUERY_ACTION_REQUEST;
    data[MAC_80211_FRAME_LEN + 2] = (osal_u8)(trans_id & 0x00FF);     /* 2 偏移 */
    data[MAC_80211_FRAME_LEN + 3] = (osal_u8)((trans_id & 0xFF00) >> 8);    /* 3 偏移,右移8位 */

    len = MAC_80211_FRAME_LEN + MAC_SA_QUERY_LEN;
    return len;
}

/*****************************************************************************
     函 数 名  : hmac_encap_sa_query_rsp_etc
     功能描述  : 组sa query 反馈帧
     输入参数  : hdr:sa query request frame
                 data:sa query response frame
     输出参数  : 无
     返 回 值  : 帧头+帧体的长度
*****************************************************************************/
osal_u16 hmac_encap_sa_query_rsp_etc(hmac_vap_stru *hmac_vap, osal_u8 *hdr, osal_u8 *data)
{
    osal_u16 len;

    /*************************************************************************/
    /*                        Management Frame Format                        */
    /* --------------------------------------------------------------------  */
    /* |Frame Control|Duration|DA|SA|BSSID|Sequence Control|Frame Body|FCS|  */
    /* --------------------------------------------------------------------  */
    /* | 2           |2       |6 |6 |6    |2               |0 - 2312  |4  |  */
    /* --------------------------------------------------------------------  */
    /*                                                                       */
    /*************************************************************************/

    /*************************************************************************/
    /*                Set the fields in the frame header                     */
    /*************************************************************************/
    /* All the fields of the Frame Control Field are set to zero. Only the   */
    /* Type/Subtype field is set.                                            */
    mac_hdr_set_frame_control(data, WLAN_FC0_SUBTYPE_ACTION);
    /* Set DA  */
    oal_set_mac_addr(((mac_ieee80211_frame_stru *)data)->address1,
        ((mac_ieee80211_frame_stru *)hdr)->address2);
    /*  Set SA  */
    oal_set_mac_addr(((mac_ieee80211_frame_stru *)data)->address2,  mac_mib_get_station_id(hmac_vap));
    /*  Set SSID  */
    oal_set_mac_addr(((mac_ieee80211_frame_stru *)data)->address3, hmac_vap->bssid);
    /* 设置分片序号为0 */
    mac_hdr_set_fragment_number(data, 0);
    /*************************************************************************/
    /*                Set the contents of the frame body                     */
    /*************************************************************************/

    /*************************************************************************/
    /*                  SA Query Frame - Frame Body                          */
    /* --------------------------------------------------------------------- */
    /* |   Category   |SA Query Action |  Transaction Identifier           | */
    /* --------------------------------------------------------------------- */
    /* |1             |1               |2 Byte                             | */
    /* --------------------------------------------------------------------- */
    /*                                                                       */
    /*************************************************************************/
    data[MAC_80211_FRAME_LEN] = hdr[MAC_80211_FRAME_LEN];
    data[MAC_80211_FRAME_LEN + 1] = MAC_SA_QUERY_ACTION_RESPONSE;
    data[MAC_80211_FRAME_LEN + 2] = hdr[MAC_80211_FRAME_LEN + 2];   /* 2 偏移 */
    data[MAC_80211_FRAME_LEN + 3] = hdr[MAC_80211_FRAME_LEN + 3];   /* 3 偏移 */

    len = MAC_80211_FRAME_LEN + MAC_SA_QUERY_LEN;
    return len;
}

/*****************************************************************************
 函 数 名  : hmac_mgmt_prepare_deauth
 功能描述  : 组去认证帧
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
osal_u16  hmac_mgmt_encap_deauth_etc(hmac_vap_stru *hmac_vap, osal_u8 *data, osal_u8 *da,
    osal_u16 err_code)
{
    osal_u16          deauth_len;
#ifdef _PRE_WLAN_FEATURE_P2P
    hmac_device_stru    *hmac_device;
    hmac_vap_stru       *up_vap1, *up_vap2;
    osal_u32          ret;
#endif

    /*************************************************************************/
    /*                        Management Frame Format                        */
    /* --------------------------------------------------------------------  */
    /* |Frame Control|Duration|DA|SA|BSSID|Sequence Control|Frame Body|FCS|  */
    /* --------------------------------------------------------------------  */
    /* | 2           |2       |6 |6 |6    |2               |0 - 2312  |4  |  */
    /* --------------------------------------------------------------------  */
    /*                                                                       */
    /*************************************************************************/

    /*************************************************************************/
    /*                Set the fields in the frame header                     */
    /*************************************************************************/

    /* All the fields of the Frame Control Field are set to zero. Only the   */
    /* Type/Subtype field is set.                                            */
    mac_hdr_set_frame_control(data, WLAN_FC0_SUBTYPE_DEAUTH);
    /* 设置分片序号为0 */
    mac_hdr_set_fragment_number(data, 0);

    /* Set DA to address of unauthenticated STA */
    oal_set_mac_addr(((mac_ieee80211_frame_stru *)data)->address1, da);

#ifdef _PRE_WLAN_FEATURE_P2P
    if (err_code & MAC_SEND_TWO_DEAUTH_FLAG) {
        err_code = err_code & ~MAC_SEND_TWO_DEAUTH_FLAG;

        hmac_device = hmac_res_get_mac_dev_etc(hmac_vap->device_id);
        if (OAL_UNLIKELY(hmac_device == OAL_PTR_NULL)) {
            deauth_len = 0;
            oam_error_log1(0, OAM_SF_ANY, "{hmac_mgmt_encap_deauth_etc::mac_device[%d] null!}", hmac_vap->device_id);
            return deauth_len;
        }

        ret = hmac_device_find_2up_vap_etc(hmac_device, &up_vap1, &up_vap2);
        if (ret == OAL_SUCC) {
            /* 获取另外一个VAP，组帧时修改地址2为另外1个VAP的MAC地址 */
            if (hmac_vap->vap_id != up_vap1->vap_id) {
                up_vap2 = up_vap1;
            }

            if (up_vap2->mib_info == OAL_PTR_NULL) {
                deauth_len = 0;
                oam_error_log1(0, OAM_SF_AUTH, "vap_id[%d]hmac_mgmt_encap_deauth_etc:vap2 mib null", up_vap2->vap_id);
                return deauth_len;
            }
            oal_set_mac_addr(((mac_ieee80211_frame_stru *)data)->address2, mac_mib_get_station_id(up_vap2));
            oal_set_mac_addr(((mac_ieee80211_frame_stru *)data)->address3, up_vap2->bssid);
        }

        oam_warning_log1(0, OAM_SF_AUTH, "hmac_mgmt_encap_deauth_etc:send the second deauth frame.error:%d", err_code);
    } else {
#endif

        if (hmac_vap->mib_info == OAL_PTR_NULL) {
            deauth_len = 0;
            oam_error_log1(0, OAM_SF_AUTH, "vap_id[%d] hmac_mgmt_encap_deauth_etc: mib ptr null.", hmac_vap->vap_id);
            return deauth_len;
        }

        /* SA is the dot11MACAddress */
        oal_set_mac_addr(((mac_ieee80211_frame_stru *)data)->address2, mac_mib_get_station_id(hmac_vap));
        oal_set_mac_addr(((mac_ieee80211_frame_stru *)data)->address3, hmac_vap->bssid);
#ifdef _PRE_WLAN_FEATURE_P2P
    }
#endif
    /*************************************************************************/
    /*                Set the contents of the frame body                     */
    /*************************************************************************/

    /*************************************************************************/
    /*                  Deauthentication Frame - Frame Body                  */
    /* --------------------------------------------------------------------- */
    /* |                           Reason Code                             | */
    /* --------------------------------------------------------------------- */
    /* |2 Byte                                                             | */
    /* --------------------------------------------------------------------- */
    /*                                                                       */
    /*************************************************************************/

    /* Set Reason Code to 'Class2 error' */
    data[MAC_80211_FRAME_LEN]     = (osal_u8)(err_code & 0x00FF);
    data[MAC_80211_FRAME_LEN + 1] = (osal_u8)((err_code & 0xFF00) >> 8);    /* 右移8位 */

    deauth_len = MAC_80211_FRAME_LEN + WLAN_REASON_CODE_LEN;

    return deauth_len;
}

/*****************************************************************************
 函 数 名  : hmac_mgmt_encap_disassoc_etc
 功能描述  : 组去关联帧
 输入参数  : vap指针,DA,ErrCode
 输出参数  : 帧缓冲区
 返 回 值  : 帧长度
*****************************************************************************/
osal_u16  hmac_mgmt_encap_disassoc_etc(hmac_vap_stru *hmac_vap, osal_u8 *data, osal_u8 *da,
    osal_u16 err_code)
{
    osal_u16 disassoc_len;

    /*************************************************************************/
    /*                        Management Frame Format                        */
    /* --------------------------------------------------------------------  */
    /* |Frame Control|Duration|DA|SA|BSSID|Sequence Control|Frame Body|FCS|  */
    /* --------------------------------------------------------------------  */
    /* | 2           |2       |6 |6 |6    |2               |0 - 2312  |4  |  */
    /* --------------------------------------------------------------------  */
    /*                                                                       */
    /*************************************************************************/

    /*************************************************************************/
    /*                            设置帧头                                   */
    /*************************************************************************/

    /* 设置subtype   */
    mac_hdr_set_frame_control(data, WLAN_FC0_SUBTYPE_DISASSOC);

    if (hmac_vap->mib_info == OAL_PTR_NULL) {
        disassoc_len = 0;
        oam_error_log1(0, OAM_SF_ASSOC, "vap_id[%d] hmac_mgmt_encap_disassoc_etc: hmac_vap mib ptr null.",
            hmac_vap->vap_id);
        return disassoc_len;
    }
    /* 设置DA */
    oal_set_mac_addr(((mac_ieee80211_frame_stru *)data)->address1, da);

    /* 设置SA */
    oal_set_mac_addr(((mac_ieee80211_frame_stru *)data)->address2, mac_mib_get_station_id(hmac_vap));

    /* 设置bssid */
    oal_set_mac_addr(((mac_ieee80211_frame_stru *)data)->address3,
        hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_AP ? mac_mib_get_station_id(hmac_vap) : hmac_vap->bssid);

    /* 设置分片序号为0 */
    mac_hdr_set_fragment_number(data, 0);

    /*************************************************************************/
    /*                  Disassociation 帧 - 帧体                  */
    /* --------------------------------------------------------------------- */
    /* |                           Reason Code                             | */
    /* --------------------------------------------------------------------- */
    /* |2 Byte                                                             | */
    /* --------------------------------------------------------------------- */
    /*                                                                       */
    /*************************************************************************/

    /* 设置reason code */
    data[MAC_80211_FRAME_LEN]     = (osal_u8)(err_code & 0x00FF);
    data[MAC_80211_FRAME_LEN + 1] = (osal_u8)((err_code & 0xFF00) >> 8);    /* 右移8位 */

    disassoc_len = MAC_80211_FRAME_LEN + WLAN_REASON_CODE_LEN;

    return disassoc_len;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

