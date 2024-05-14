#ifndef _THEAD_H_
#define _THEAD_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void thead_csi_dcache_clean_invalid_range(void* addr, uint64_t size);

#ifdef __cplusplus
}
#endif

#endif