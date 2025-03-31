/*
 * Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
 * Description: platform sha256sum calculate header file
 * Author: Huanghe
 * Create: 2023-01-13
 */

#ifndef __PLAT_SHA256_CALC_H__
#define __PLAT_SHA256_CALC_H__


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#if defined(_PRE_PLAT_SHA256SUM_CHECK) && (_PRE_PLAT_SHA256SUM_CHECK == 1)

#define SHA256_HEADER_ARR_LEN   32  // 32 Bytes (256 bits).
#define SHA256_HEADER_BYTES_LEN 2   // Each byte is displayed as two characters.
#define SHA256_HEADER_LENGTH    64  // 32 Bytes (256 bits). Each byte is displayed as two characters.

#define FILE_READ_SIZE          (32 * 1024)

osal_s32 do_sha256(const osal_char *data, osal_u32 datalen, osal_char *out_digest);
osal_s32 firmware_sha256_checksum(const osal_char *path);

#endif // defined(_PRE_PLAT_SHA256SUM_CHECK) && (_PRE_PLAT_SHA256SUM_CHECK == 1)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
