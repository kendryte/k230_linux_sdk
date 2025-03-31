/*
 * Copyright (c) CompanyNameMagicTag. 2023-2023. All rights reserved.
 * 文 件 名   : wai_ec.h
 * 作    者   : CompanyName
 * 生成日期   : 2023年1月11日
 * 功能描述   : wai_ec对应头文件
 */


#ifndef __WAI_EC_H__
#define __WAI_EC_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


/*
 * 1 Other Header File Including
 */
#include "types.h"
#include <utils/common.h>
#include <stdlib.h>
#include <utils/os.h>

/*
 * 2 Macro Definition
 */



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

typedef struct {
	int32   l_field_type;
	uint32  ul_seed_len;
	uint32  ul_param_len;
	uint32  ul_cofactor;
} WAPI_EC_CURVE_DATA;


/*
 * 8 UNION Type Definition
 */


/*
 * 9 OTHERS Definition
 */

#define DGST_TO_SIGN_LENTH          32

#define RANDOM_HEX_LENTH            16

#define WAPI_CERT_PUBKEY_LEN        49

#define WAPI_CERT_SIGN_LEN          48
#define WAPI_CERT_SIGN_R_LEN        24
#define WAPI_CERT_SIGN_S_LEN        24

#define BN2OCT_STRING_LENTH         24
#define ECDH_SHAREKEY_LENTH         24


/*
 * 10 Function Declare
 */
int32 wapi_generate_temp_keygroup(uint8 *puc_asue_pubkey,
				   uint32 *pul_asue_pubkey_len,
				   uint8 *puc_asue_privkey,
				   uint32 *pul_asue_privkey_len);

int32 wapi_ecc192_sign(const uint8 *puc_digest, uint32 ul_digest_length,
			uint8 *puc_priv_key, uint32 ul_priv_key_len,
			uint8 *puc_sign_string, uint32 *pul_sign_length);

int32 wapi_ecc192_verify_signature(const uint8 *puc_digest,
				uint32 ul_digest_length, uint8 *puc_signature,
				uint32 ul_signature_len, uint8 *puc_pub_key,
				uint32 ul_pub_key_len);

int32 __attribute__((visibility ("default"))) wapi_ecc192_verify_keygroup(
	uint8 *puc_pub_key, uint32 ul_pub_key_len,
	uint8 *puc_priv_key, uint32 ul_priv_key_len);

int32 wapi_ecdh_compute_key(uint8 *puc_asue_priv_key,
			    uint32 ul_asue_priv_key_len, uint8 *puc_ae_pub_key,
			    uint32 ul_ae_pub_key_len, uint8 *puc_sharekey,
			    uint32 *pul_sharekey_len);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of wai_ec.h */

