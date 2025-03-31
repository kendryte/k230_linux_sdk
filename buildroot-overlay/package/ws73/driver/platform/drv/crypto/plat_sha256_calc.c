/*
 * Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
 * Description: platform sha256sum calculate source file
 * Author: Huanghe
 * Create: 2023-01-13
 */

#if defined(_PRE_OS_VERSION) && defined(_PRE_OS_VERSION_LINUX) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include <crypto/hash.h>
#include <linux/module.h>
#endif

#if defined(_PRE_OS_VERSION) && defined(_PRE_OS_VERSION_LITEOS) && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
#include "librhash/sha256.h"
#endif

#include "oal_types.h"
#include "osal_types.h"
#include "oal_debug.h"
#include "oal_kernel_file.h"
#include "plat_sha256_calc.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#if defined(_PRE_PLAT_SHA256SUM_CHECK) && (_PRE_PLAT_SHA256SUM_CHECK == 1)

#if defined(_PRE_OS_VERSION) && defined(_PRE_OS_VERSION_LINUX) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
struct sdesc {
    struct shash_desc shash;
    osal_char ctx[];
};

static struct sdesc *init_sdesc(struct crypto_shash *alg)
{
    struct sdesc *sdesc;
    size_t size;

    size = sizeof(struct shash_desc) + crypto_shash_descsize(alg);
    sdesc = kmalloc(size, GFP_KERNEL);
    if (!sdesc)
        return ERR_PTR(-ENOMEM);
    sdesc->shash.tfm = alg;
    return sdesc;
}

static osal_s32 calc_hash(struct crypto_shash *alg, const osal_u8 *data, osal_u32 datalen, osal_char *digest)
{
    struct sdesc *sdesc;
    osal_s32 ret;

    sdesc = init_sdesc(alg);
    if (IS_ERR(sdesc)) {
        oal_print_err("can't alloc sdesc\n");
        return PTR_ERR(sdesc);
    }

    ret = crypto_shash_digest(&sdesc->shash, data, datalen, digest);
    kfree(sdesc);
    return ret;
}

osal_s32 do_sha256(const osal_char *data, osal_u32 datalen, osal_char *out_digest)
{
    struct crypto_shash *alg;
    char *hash_alg_name = "sha256";
    osal_s32 ret;

    alg = crypto_alloc_shash(hash_alg_name, 0, 0);
    if (IS_ERR(alg)) {
        oal_print_err("can't alloc alg %s\n", hash_alg_name);
        return PTR_ERR(alg);
    }
    ret = calc_hash(alg, data, datalen, out_digest);

    crypto_free_shash(alg);
    return ret;
}

#elif defined(_PRE_OS_VERSION) && defined(_PRE_OS_VERSION_LITEOS) && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)

osal_s32 do_sha256(const osal_char *data, osal_u32 datalen, osal_char *out_digest)
{
    sha256_ctx ctx = {0};

    rhash_sha256_init(&ctx);
    rhash_sha256_update(&ctx, (osal_u8 *)data, datalen);
    rhash_sha256_final(&ctx, (osal_u8 *)out_digest);

    return OAL_SUCC;
}

#endif

#endif // end of defined(_PRE_PLAT_SHA256SUM_CHECK) && (_PRE_PLAT_SHA256SUM_CHECK == 1)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
