/*
 * Copyright (c) @CompanyNameMagicTag 2020-2023. All rights reserved.
 * Description: Header file
 */

#ifndef MSG_REKEY_OFFLOAD_ROM_H
#define MSG_REKEY_OFFLOAD_ROM_H

#define MAC_REKEY_OFFLOAD_KCK_LEN              16
#define MAC_REKEY_OFFLOAD_KEK_LEN              16
#define MAC_REKEY_OFFLOAD_REPLAY_LEN           8

typedef struct rekey_offload {
    osal_u8   kck[MAC_REKEY_OFFLOAD_KCK_LEN];
    osal_u8   kek[MAC_REKEY_OFFLOAD_KEK_LEN];
    osal_u8   replay_ctr[MAC_REKEY_OFFLOAD_REPLAY_LEN];
} mac_rekey_offload_stru;
 
typedef struct {
    mac_rekey_offload_stru rekey_offload;
    osal_u8 rekey_info_exist;
} mac_rekey_offload_param_sync;
#endif
