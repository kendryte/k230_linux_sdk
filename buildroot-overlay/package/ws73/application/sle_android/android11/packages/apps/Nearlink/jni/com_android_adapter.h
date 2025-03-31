/**
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */
#ifndef COM_ANDROID_ADAPTER_H
#define COM_ANDROID_ADAPTER_H

#include "sle_common.h"

namespace android {
void nearlink_adapter_enable_callback(errcode_t status);
void nearlink_adapter_disable_callback(errcode_t status);
void nearlink_announce_enable_callback(uint32_t announce_id, errcode_t status);
void nearlink_announce_disable_callback(uint32_t announce_id, errcode_t status);
void nearlink_announce_terminal_callback(uint32_t announce_id);
void nearlink_seek_start_callback(errcode_t status);
void nearlink_seek_disable_callback(errcode_t status);
void nearlink_seek_result_callback(sle_seek_result_info_t *seek_result_data);

}  // namespace android

#endif
