#ifndef SLE_SSAP_H
#define SLE_SSAP_H

#include <stdint.h>
#include "sle_ssap_stru.h"
#include "com_android_nearlink.h"

#define RETURN(error_code)                                                      \
    do {                                                                        \
        if (error_code == ERRCODE_SLE_SUCCESS) {                                \
            ALOGD("%s execute finished, error_code=%d", __func__, error_code);  \
        } else {                                                                \
            ALOGE("%s execute failed, error code: 0x%x", __func__, error_code); \
        }                                                                       \
        return error_code;                                                      \
    } while (0)

namespace android {

void from_java_uuid(sle_uuid_t *uu, jlong uuid_msb, jlong uuid_lsb);

uint64_t uuid_lsb(const sle_uuid_t *uu);

uint64_t uuid_msb(const sle_uuid_t *uu);

}  // namespace android

#endif  // SLE_SSAP_H
