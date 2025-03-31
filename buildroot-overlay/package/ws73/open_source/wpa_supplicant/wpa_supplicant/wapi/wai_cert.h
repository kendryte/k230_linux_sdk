/*
 * Copyright (c) CompanyNameMagicTag. 2023-2023. All rights reserved.
 * 文 件 名   : wai_cert.h
 * 作    者   : CompanyName
 * 生成日期   : 2023年1月11日
 * 功能描述   : wai_cert对应头文件，证书相关接口
 */

#ifndef __WAI_CERT_H__
#define __WAI_CERT_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


/*
 * 1 Other Header File Including
 */
#include "types.h"
#include "wai_asn1.h"


/*
 * 2 Macro Definition
 */

#define LENGTH_OF_VALIDATE              8
#define LENGTH_OF_PBKEY_VALUE           49
#define MIN_LENGTH_OF_ID                6
#define MAX_LENGTH_OF_ID                256
#define MAX_OID_LEN                     20

#define MAX_BYTE_DATA_LEN               256
#define WAPI_CERT_X509_V3               2
#define WAPI_CERT_MAX_NAME_ATTRIBUTES   20

#define WAPI_CERT_ISSUE_ID              0
#define WAPI_CERT_SUBJECT_ID            1

#define WAPI_CERT_SIGN_LEN              48
#define WAPI_CERT_SIGN_HALF_LEN         24

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


struct wapi_cert_algorithm_identifier {
    struct asn1_oid st_oid;
};

struct wapi_cert_name_attr {
    enum wapi_cert_name_attr_type {
        WAPI_CERT_NAME_ATTR_NOT_USED,
        WAPI_CERT_NAME_ATTR_DC,
        WAPI_CERT_NAME_ATTR_CN,
        WAPI_CERT_NAME_ATTR_C,
        WAPI_CERT_NAME_ATTR_L,
        WAPI_CERT_NAME_ATTR_ST,
        WAPI_CERT_NAME_ATTR_O,
        WAPI_CERT_NAME_ATTR_OU
    } em_type;
    int8 *pc_value;
};

struct wapi_cert_name {
    struct wapi_cert_name_attr      st_attr[WAPI_CERT_MAX_NAME_ATTRIBUTES];
    uint32              ul_num_attr;
    int8                *pc_email; /* emailAddress */

    /* from alternative name extension */
    int8                *pc_alt_email; /* rfc822Name */
    int8                *pc_dns; /* dNSName */
    int8                *pc_uri; /* uniformResourceIdentifier */
    uint8               *puc_ip; /* iPAddress */
    uint32              ul_ip_len; /* IPv4: 4, IPv6: 16 */
    struct asn1_oid     st_rid; /* registeredID */
};

typedef struct _id_stru
{
    uint8               uc_length;
    uint8              *pauc_cont;
}id_stru;

typedef struct _tlv_stru
{
    uint8               uc_type;
    uint16              us_length;
    uint8               auc_value[MAX_OID_LEN];
}tlv_stru;

typedef struct _public_key_stru
{
    uint16              us_length;
    uint8               uc_flag;
    tlv_stru            st_param;
    uint8               auc_value[LENGTH_OF_PBKEY_VALUE];
}pbkey_stru;

typedef struct _sign_alg_stru
{
    uint8              uc_flag;
    uint16             us_param_length;
    uint32             auc_param[MAX_OID_LEN];
}sign_alg_stru;

typedef struct _cert_stru
{
    uint16              us_version;
    id_stru             st_serial_num;
    sign_alg_stru       st_sign_alg;
    id_stru             st_issuer;
    uint32              ul_not_before_time;
    uint32              ul_not_after_time;
    id_stru             st_subject;
    pbkey_stru          st_subject_key;
    sign_alg_stru       st_sign_val_alg;
    uint32              us_sign_length;
    uint8              *puc_sign_val;
}cert_stru;

typedef struct _wapi_cert_private_key
{
    uint8           uc_tVersion;
    uint8           uc_lVersion;
    uint8           uc_vVersion;
    uint8           uc_verpad;

    uint8           uc_tPrivateKey;
    uint8           uc_lPrivateKey;
    uint8           auc_prikeypad[2];
    uint8           auc_vPrivateKey[MAX_BYTE_DATA_LEN];

    uint8           uc_tSPrivateKeyAlgorithm;
    uint8           uc_lSPrivateKeyAlgorithm;
    uint8           uc_tOID;
    uint8           uc_lOID;
    uint8           auc_vOID[MAX_BYTE_DATA_LEN];

    uint8           uc_tSPubkey;
    uint8           uc_lSPubkey;
    uint8           uc_tPubkey;
    uint8           uc_lPubkey;
    uint8           uc_vPubkey[MAX_BYTE_DATA_LEN];
}wapi_cert_private_key;

typedef struct _cert_stru_ext {
	uint16                  us_version;
	unsigned long           ulong_serial_number;
	struct wapi_cert_algorithm_identifier st_signature;

	struct wapi_cert_name   st_issuer;
	struct wapi_cert_name   st_subject;

	uint8                   *puc_serial_number;
	uint32                  ul_serial_number_len;
	uint8                   *puc_issuer_str;
	uint32                  ul_issuer_str_len;
	uint8                   *puc_subject_str;
	uint32                  ul_subject_str_len;

	os_time_t               long_not_before;
	os_time_t               long_not_after;

	struct wapi_cert_algorithm_identifier st_public_key_alg;
	uint8                   *puc_public_key;
	uint32                  ul_public_key_len;
	struct wapi_cert_algorithm_identifier st_signature_alg;
	uint8                   *puc_sign_value;
	uint32                  ul_sign_value_len;

	const uint8             *puc_cert_start;
	uint32                  ul_cert_len;
	const uint8             *puc_tbs_cert_start;
	uint32                  ul_tbs_cert_len;
}cert_stru_ext;

/*
 * 8 UNION Type Definition
 */


/*
 * 9 OTHERS Definition
 */


/*
 * 10 Function Declare
 */

extern uint8 *wapi_cert_get_certificate_buf(uint8 *puc_cert_buf,
					    uint32 ul_cert_buf_len,
					    size_t *pul_der_len);
extern __attribute__((visibility ("default"))) cert_stru *wapi_cert_parse_certificate(
	uint8 *puc_cert_buf, uint32 ul_cert_buf_len);
extern __attribute__((visibility ("default"))) int32  wapi_cert_parse_ecprivkey(
	uint8 *puc_cert_buf, uint32 ul_cert_buf_len,
	uint8 *puc_priv_key_buf, uint32 *pul_priv_key_len);
extern __attribute__((visibility ("default"))) int32 wapi_cert_verify_certificate(
	uint8 *puc_cert_buf, uint32 ul_cert_buf_len,
	uint8 *puc_asu_pubkey, uint32 ul_asu_pubkey_len);
extern __attribute__((visibility ("default"))) void wapi_cert_free_certificate(
	cert_stru *pst_cert);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of wai_cert.h */
