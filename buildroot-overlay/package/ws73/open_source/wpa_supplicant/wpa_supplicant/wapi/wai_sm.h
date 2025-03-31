/*
 * Copyright (c) CompanyNameMagicTag. 2023-2023. All rights reserved.
 * 文 件 名   : wai_sm.h
 * 作    者   : CompanyName
 * 生成日期   : 2023年1月11日
 * 功能描述   : wai_sm头文件，状态机相关
 */


#ifndef __WAI_SM_H__
#define __WAI_SM_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


/*
 * 1 Other Header File Including
 */
#include "types.h"
#include "wai_cert.h"
#include "wpa_supplicant_i.h"

/*
 * 2 Macro Definition
 */
#define WAI_FLAG_BK_UPDATE          BIT(0)
#define WAI_FLAG_PRE_AUTH           BIT(1)
#define WAI_FLAG_CERT_REQ           BIT(2)
#define WAI_FLAG_OPT_FIELD          BIT(3)
#define WAI_FLAG_USK_UPDATE         BIT(4)
#define WAI_FLAG_STAKEY_NEG         BIT(5)
#define WAI_FLAG_STAKEY_DEL         BIT(6)
#define WAI_FLAG_RESERVED           BIT(7)


#define MAX_CERT_BUFF_SIZE          2048
#define CERT_X509V3                 1
#define CERT_GBW                    2

#define ECDH_PARAM_FLAG_OID         1

#define WAI_USK_NUMBER              2

#define MAX_KEYDATA_SIZE            256
#define SERIAL_NUM_SIZE             4

/* �ֶγ��ȶ��� */
#define WAI_AUTH_ID_SIZE            32
#define WAI_FLAG_SIZE               1
#define WAI_ID_FLAG_SIZE            2
#define WAI_ID_LENGTH_SIZE          2
#define WAI_LOCAL_MAX_ID_SIZE       3048
#define WAI_CERT_FLAG_SIZE          2
#define WAI_CERT_LENGTH_SIZE        2
#define WAI_ECDHPARAM_FLAG_SIZE     1
#define WAI_ECDHPARAM_LENGTH_SIZE   2
#define WAI_CHALLENGE_SIZE          32
#define WAI_ACCESS_RESULT_SIZE      1
#define WAI_KEY_LENGTH_SIZE         1
#define WAI_ATTRIB_TYPE_SIZE        1
#define WAI_ATTRIB_LENGTH_SIZE      2
#define WAI_BKID_SIZE               16
#define WAI_BK_SIZE                 16
#define WAI_ADDID_SIZE              12
#define WAI_USKID_SIZE              1
#define WAI_MSKID_SIZE              1
#define WAI_MSK_ANNO_ID_SIZE        16
#define WAI_IV_LEN	                16
#define WAI_DATA_SERIAL_NUMBER      16
#define WAI_MIC_SIZE                20
#define WAI_MSK_SIZE                32
#define WAI_NMK_SIZE                16
#define WAI_MAX_TX_COUNT            3

#define WAI_SIGN_PARA_TYPE_SIZE     1
#define WAI_SIGN_PARA_LEN           2
#define WAI_SIGN_VALUE_LEN          2
#define WAI_SIGN_ALG_LEN            2
#define WAI_SIGN_ALG_ID             1
#define WAI_SIGN_ALG_PARA_ID_LEN    1
#define WAI_SIGN_ALG_PARA_VALUE_LEN 2

#define WAPI_IE_ID_SIZE             1
#define WAPI_IE_LENGTH_SIZE         1
#define WAPI_IE_MIN_SIZE            16
#define WAPI_IE_MAX_SIZE            255
#define WAI_SUBKEY_SIZE             16
#define ECDH_KEY_SIZE               24

#define WAI_BK_DERIVATION_SIZE      48
#define WAI_USK_DERIVATION_SIZE     96
#define MAX_SIGNATURE_SIZE          1024


#define WAIATTRIB_TYPE_SIGNATURE    1
#define WAIATTRIB_TYPE_VERIFY_RST   2
#define WAIATTRIB_TYPE_IDENT_LIST   3

#define WAPIRC_CERT_FAILED          32

#define WAI_BK_TXT      "base key expansion for key and additional nonce"
#define WAI_USK_TEXT    "pairwise key expansion for unicast and additional keys and nonce"
#define WAI_MSK_TEXT    "multicast or station key expansion for station unicast and multicast and broadcast"
#define WAI_PSK_TEXT    "preshared key expansion for authentication and key negotiation"


/*
 * 3 Enum Type Definition
 */

typedef enum {
	WAISM_INIT = 0,
	WAISM_ALREADY_ASSOC,
	WAISM_ACCESSAUTH_REQ,
	WAISM_ACCESSAUTH_RES,
	WAISM_USKNEG_RES,
	WAISM_USKNEG_CONFIRM,
	WAISM_FINSHED
} wai_state_enum;

typedef enum _wai_frame_enum {
	WAI_PREAUTH_START             = 1,
	WAI_STAKEY_REQUEST            = 2,
	WAI_AUTH_ACTIVE               = 3,
	WAI_ACCESS_AUTH_REQUEST       = 4,
	WAI_ACCESS_AUTH_RESPONSE      = 5,
	WAI_CERT_AUTH_REQUEST         = 6,
	WAI_CERT_AUTH_RESPONSE        = 7,
	WAI_USK_NEGOTIATION_REQUEST   = 8,
	WAI_USK_NEGOTIATION_RESPONSE  = 9,
	WAI_USK_NEGOTIATION_CONFIRM   = 10,
	WAI_MSK_ANNOUNCEMENT          = 11,
	WAI_MSK_ANNOUNCEMENT_RESPONSE = 12,
	WAI_SUBTYPE_MAX               = 13
} wai_frame_enum;





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
typedef int32 (*wai_dispose_func)(struct wpa_supplicant *pst_wpa,
				  const uint8 *pauc_payload,
				  uint32 ul_payload_len);
typedef struct _wai_sm_stru {
	wai_dispose_func wai_dispose_func[WAI_SUBTYPE_MAX+1];
} wai_sm_stru;

typedef struct _value_string_stru {
	uint32          ul_value;
	const char     *str;
} value_string_stru;

typedef struct _signature_stru {
	uint8   *pauc_ident;
	uint16   us_ident_len;
	uint8   *pauc_alg;
	uint16   us_alg_len;
	uint8   *pauc_sign_value;
	uint16   us_sign_value_len;
} signature_stru;

typedef struct _cert_verify_result_stru {
	uint8   *pauc_N1;
	uint8   *pauc_N2;
	uint8    uc_result1;
	uint8   *pauc_cert1;
	uint32   ul_cert1_len;
	uint8    uc_result2;
	uint8   *pauc_cert2;
	uint32   ul_cert2_len;
} cert_verify_result_stru;

typedef struct _wai_attrib_descriptor_stru {
	uint8    uc_attrib_type;
	uint16   us_attrib_length;
	union {
		signature_stru          st_signature;
		cert_verify_result_stru st_cert_verify_result;
	} un_attrib;
} wai_attrib_descriptor_stru;

/* BKSA struct */
typedef struct _wapi_bksa_stru {
	uint8   auc_bkid[WAI_BKID_SIZE];
	uint8   auc_bk[WAI_BK_SIZE];
	uint8   auc_ae_mac[ETH_ALEN];
	uint8   auc_asue_mac[ETH_ALEN];
	uint32  ul_akmp;
} wapi_bksa_stru;

typedef struct _wapi_usk_stru {
	uint8  auc_uek[WAI_SUBKEY_SIZE];
	uint8  auc_uck[WAI_SUBKEY_SIZE];
	uint8  auc_mak[WAI_SUBKEY_SIZE];
	uint8  auc_kek[WAI_SUBKEY_SIZE];
} wapi_usk_stru;


/* USKSA struct */
typedef struct _wapi_usksa_stru {
	uint8 uc_uskid;
	wapi_usk_stru st_usk[2];
	int32 ul_ucast_suite;
	uint8 auc_ae_mac[ETH_ALEN];
	uint8 auc_asue_mac[ETH_ALEN];
} wapi_usksa_stru;

/* MSKSA struct */
typedef struct _wapi_msksa_stru {
	uint8 uc_direction;
	uint8 uc_mskid;
	uint8 auc_msk[WAI_MSK_SIZE];
	uint8 auc_msk_ann_id[WAI_IV_LEN];
	int32 uc_ucast_suite;
	uint8 auc_ae_mac[ETH_ALEN];
} wapi_msksa_stru;


typedef struct _wai_auth_req_param_stru {
	const uint8          *pauc_authid;
	cert_stru            *pst_ae_cert;
	cert_stru            *pst_asue_cert;
	uint8                *pauc_ecdh;
	uint16                us_ecdh_len;
} wai_authreq_param_stru;


/*
 * 8 UNION Type Definition
 */


/*
 * 9 OTHERS Definition
 */


/*
 * 10 Function Declare
 */
struct wapi_supplicant_stru;
int32  wai_certauth_bk_derivation(struct wapi_supplicant_stru *pst_wapi,
				   uint8 *pauc_ae_pubkey, uint8 uc_ae_pubkey_len,
				   uint8 *pauc_nae);
int32  wai_usk_derivation(struct wapi_supplicant_stru *pst_wapi,
			  uint8 *pauc_usk_nae);
int32  wai_msk_derivation(uint8 *pauc_msk_nmk, uint32 ul_nmk_len,
			  uint8 *pauc_out_bk, uint32 ul_outkey_len);
__attribute__((visibility ("default"))) int32  wai_psk_derivation(
	uint8 *pauc_password, uint32 ul_ps_len, uint8 *pauc_out_bk);
void  wai_tx_timeout(void *eloop_ctx, void *timeout_ctx);
int32  wai_sm_process(struct wpa_supplicant *pst_wpa,
		      struct wpabuf *pst_waibuf);
wai_sm_stru * get_wai_sm();


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of wai_sm.h */
