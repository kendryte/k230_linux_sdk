#include "utils.h"
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if CONFIG_USE_HWRNG
#include "drv_pufs.h"
#include <pthread.h>
#elif CONFIG_USE_LWIP
#include "mbedtls/entropy.h"
#else
#include <sys/random.h>
#endif
#include "mbedtls/md.h"

#if CONFIG_USE_HWRNG
static pthread_mutex_t hardware_rng_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif

int utils_random_bytes(uint8_t* output, size_t len) {
  if (output == NULL && len != 0) {
    return -1;
  }
  if (len == 0) {
    return 0;
  }

#if CONFIG_USE_HWRNG
  drv_pufs_inst dev;
  int ret;

  /* RT-Smart's getrandom() uses a deterministic PRNG, not the hardware RNG. */
  if (len > UINT32_MAX || pthread_mutex_lock(&hardware_rng_mutex) != 0) {
    return -1;
  }

  /* Concurrent handshakes must not overlap the /dev/pufs file lifecycle. */
  if (drv_pufs_open(&dev) != 0) {
    pthread_mutex_unlock(&hardware_rng_mutex);
    return -1;
  }

  ret = drv_pufs_rng_read(&dev, output, (uint32_t)len);
  drv_pufs_close(&dev);
  pthread_mutex_unlock(&hardware_rng_mutex);
  return ret == 0 ? 0 : -1;
#elif CONFIG_USE_LWIP
  mbedtls_entropy_context entropy;
  int ret;

  mbedtls_entropy_init(&entropy);
  ret = mbedtls_entropy_func(&entropy, output, len);
  mbedtls_entropy_free(&entropy);
  return ret == 0 ? 0 : -1;
#else
  size_t offset = 0;

  while (offset < len) {
    ssize_t ret = getrandom(output + offset, len - offset, 0);
    if (ret > 0) {
      offset += (size_t)ret;
      continue;
    }
    if (ret < 0 && errno == EINTR) {
      continue;
    }
    return -1;
  }
  return 0;
#endif
}

int utils_random_string(char* s, const int len) {
  int offset = 0;
  uint8_t random_bytes[32];

  static const char alphanum[] =
      "0123456789"
      "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
      "abcdefghijklmnopqrstuvwxyz";

  if (s == NULL || len < 0) {
    return -1;
  }

  while (offset < len) {
    if (utils_random_bytes(random_bytes, sizeof(random_bytes)) != 0) {
      s[0] = '\0';
      return -1;
    }
    for (size_t i = 0; i < sizeof(random_bytes) && offset < len; i++) {
      /* Ignore the top eight values to avoid modulo bias (248 is 4 * 62). */
      if (random_bytes[i] < 248) {
        s[offset++] = alphanum[random_bytes[i] % (sizeof(alphanum) - 1)];
      }
    }
  }

  s[len] = '\0';
  return 0;
}

int utils_get_hmac_sha1(const char* input, size_t input_len, const char* key, size_t key_len, unsigned char* output) {
  mbedtls_md_context_t ctx;
  const mbedtls_md_info_t* md_info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA1);
  int ret;

  if (!md_info) {
    return -1;
  }

  mbedtls_md_init(&ctx);
  ret = mbedtls_md_setup(&ctx, md_info, 1);
  if (ret == 0) {
    ret = mbedtls_md_hmac_starts(&ctx, (const unsigned char*)key, key_len);
  }
  if (ret == 0) {
    ret = mbedtls_md_hmac_update(&ctx, (const unsigned char*)input, input_len);
  }
  if (ret == 0) {
    ret = mbedtls_md_hmac_finish(&ctx, output);
  }
  mbedtls_md_free(&ctx);
  return ret;
}

void utils_get_md5(const char* input, size_t input_len, unsigned char* output) {
  mbedtls_md_context_t ctx;
  mbedtls_md_type_t md_type = MBEDTLS_MD_MD5;
  mbedtls_md_init(&ctx);
  mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(md_type), 1);
  mbedtls_md_starts(&ctx);
  mbedtls_md_update(&ctx, (const unsigned char*)input, input_len);
  mbedtls_md_finish(&ctx, output);
  mbedtls_md_free(&ctx);
}
