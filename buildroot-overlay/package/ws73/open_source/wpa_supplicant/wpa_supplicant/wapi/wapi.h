/*
 * Copyright (c) CompanyNameMagicTag. 2023-2023. All rights reserved.
 * 文 件 名   : wapi.h
 * 作    者   : CompanyName
 * 生成日期   : 2023年1月11日
 * 功能描述   : wapi对应头文件
 */

#ifndef __WAPI_H__
#define __WAPI_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


/*
 * 1 Other Header File Including
 */
#include "common/wpa_common.h"
#include "drivers/driver.h"
#include "types.h"
#include "wai_sm.h"                 /* for wai_state_enum */


/*
 * 2 Macro Definition
 */
#define ETH_TYPE_WAI            0x88B4

#define wapi_get_func_from_dl(dl_handle, fun, fun_name) do{\
fun= dlsym(dl_handle, fun_name);\
    dlerr = dlerror();\
    if ((dlerr != NULL) || (NULL ==  fun))\
    {\
        wpa_printf(MSG_INFO,"dlsym %s failed,error is %s", fun_name, dlerr);\
        dlclose(dl_handle);\
        dl_handle = NULL;\
        dlerr = dlerror();\
        if (dlerr != NULL)\
        {\
            wpa_printf(MSG_INFO,"dlclose failed,error is %s", dlerr);\
        }\
      return -1;\
    }\
}while(0);

#if defined(__LP64__)
#define LIBWAPI_PATH "/vendor/lib64/libwifi_wapi.so"
#else
#define LIBWAPI_PATH "/lib/libwifi_wapi.so"
#endif



/*
 * 3 Enum Type Definition
 */


/*
 * 4 Global Variable Declaring
 */


/*
 * 5 Message Header Definition
 */


/*
 * 6 Message Definition
 */


/*
 * 7 STRUCT Type Definition
 */
struct wpa_supplicant;
struct l2_packet_data;
struct wpabuf;
struct wpa_ie_data;
struct wpa_bss;
struct wpa_driver_associate_params;

typedef enum _auth_type_enum{
    AUTH_TYPE_NONE_WAPI = 0,	/* no WAPI */
    AUTH_TYPE_WAPI_CERT,		/* Certificate */
    AUTH_TYPE_WAPI_PSK		    /* Pre-PSK */
}auth_type_enum;

typedef enum {
    KEY_TYPE_ASCII = 0,		    /* ascii */
    KEY_TYPE_HEX		        /* HEX */
}key_type_enum;

typedef enum {
    CONN_ASSOC = 0,
    CONN_DISASSOC
}conn_status_enum;


/* Pre-PSK */
typedef struct _auth_type_psk_stru{
    key_type_enum en_key_type;	       /* Pre-PSK: Key type */
    uint32        ul_key_length;	       /* Pre-PSK: key length */
    uint8         uc_key_val[128];      /* Pre-PSK: value */
}auth_type_psk_stru;



/* wapi ���ؽṹ */
struct wapi_supplicant_stru
{
    struct wpa_supplicant *pst_wpa;
    struct l2_packet_data *pst_wapi_l2;

    wai_state_enum         en_state;

    struct wpabuf         *pst_fragbuf;
    uint16                 us_nextframeseq;
    uint8                  uc_nextfragseq;
    uint16                 us_txframe_seq;

    uint8                  auc_own_mac[ETH_ALEN];
    uint8                  auc_bssid[ETH_ALEN];

    uint8                  auc_wapi_ie[WAPI_IE_MAX_SIZE];
    int32                  uc_wapi_ie_len;
    uint8                  auc_assoc_wapi_ie[WAPI_IE_MAX_SIZE];
    int32                  uc_assoc_wapi_ie_len;

    uint8                  uc_wai_flag;
    uint8                  auc_next_authid[WAI_AUTH_ID_SIZE];

    uint8                  auc_bk_nasue[WAI_CHALLENGE_SIZE];
    uint8                  auc_usk_nasue[WAI_CHALLENGE_SIZE];
    uint8                  auc_usk_next_nae[WAI_CHALLENGE_SIZE];

    uint8                  auc_asue_pubkey[MAX_KEYDATA_SIZE];
    uint32                 ul_asue_pubkey_len;
    uint8                  auc_asue_privkey[MAX_KEYDATA_SIZE];
    uint32                 ul_asue_privkey_len;
    uint8                  auc_asue_cert_privkey[MAX_KEYDATA_SIZE];
    uint32                 ul_asue_cert_privkey_len;

    cert_stru             *pst_asue_cert;
    cert_stru             *pst_ae_cert;
    cert_stru             *pst_as_cert;

    uint8                  auc_as_certfile[2048];
    uint32                 ul_as_certfile_len;
	uint8                  auc_user_certfile[2048];
	uint32                 ul_user_certfile_len;

    wapi_bksa_stru         st_bksa;
    wapi_usksa_stru        st_usksa;
    wapi_msksa_stru        st_msksa;

    uint8                  auc_addid[WAI_ADDID_SIZE];              /* ADDID( MAC || MAC ) */

    auth_type_enum         en_auth_type;
    auth_type_psk_stru     st_auth_psk_type;

    struct wpabuf         *pst_tx_framebuf;
    uint32                 ul_tx_count;

    uint8                  uc_bk[WAI_BK_SIZE];

};

/*
 * 8 UNION Type Definition
 */


/*
 * 9 OTHERS Definition
 */


/*
 * 10 Function Declare
 */

int32 wapi_deinit_iface(struct wpa_supplicant* pst_wpa);
int32 wapi_init_iface(struct wpa_supplicant* pst_wpa);
int32 wapi_parse_wapi_ie(const uint8 *pauc_wapi_ie, uint32 ul_ie_len, struct wpa_ie_data *pst_ie_data );
void wapi_supplicant_deinit(struct wpa_supplicant *pst_wpa);
void wapi_supplicant_init(struct wpa_supplicant *pst_wpa);
int32 wapi_supplicant_event_assoc(struct wpa_supplicant *pst_wpa,struct wpa_bss *pst_bss,
		                          struct wpa_ssid *pst_ssid, struct wpa_driver_associate_params *pst_params);
int32 wapi_supplicant_event_disassoc(struct wpa_supplicant *pst_wpa,uint16 us_reason);
int32 wapi_supplicant_event(struct wpa_supplicant *pst_wpa, enum wpa_event_type en_event,void *data);
void  wapi_notify_wrong_msg(struct wpa_supplicant  *pst_wpa);
int32 wapi_call_back_init();
int wapi_init_dlhandle(void );

#ifdef __cplusplus
	#if __cplusplus
		}
	#endif
#endif

#endif /* end of wapi.h */
