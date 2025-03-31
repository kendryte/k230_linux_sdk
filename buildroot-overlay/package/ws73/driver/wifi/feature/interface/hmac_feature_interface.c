/*
 * Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
 * Description: hmac feature
 * Create: 2023-2-17
 */

#include "hmac_feature_interface.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

osal_void *g_feature_fhook_tb[HMAC_FHOOK_INDEX_BUTT];

osal_void hmac_feature_hook_register(hmac_fhook_index_enum fhook_idx, osal_void *fun)
{
    if (osal_unlikely(fhook_idx >= HMAC_FHOOK_INDEX_BUTT)) {
        return;
    }

    g_feature_fhook_tb[fhook_idx] = fun;
}

osal_void hmac_feature_hook_unregister(hmac_fhook_index_enum fhook_idx)
{
    if (osal_unlikely(fhook_idx >= HMAC_FHOOK_INDEX_BUTT)) {
        return;
    }

    g_feature_fhook_tb[fhook_idx] = OSAL_NULL;
}

WIFI_HMAC_TCM_TEXT WIFI_TCM_TEXT osal_void *hmac_get_feature_fhook(hmac_fhook_index_enum fhook_idx)
{
    if (osal_unlikely(fhook_idx >= HMAC_FHOOK_INDEX_BUTT)) {
        return OSAL_NULL;
    }
    return g_feature_fhook_tb[fhook_idx];
}

WIFI_TCM_TEXT osal_u8 hmac_vap_id_param_check(osal_u8 vap_id)
{
    if (osal_unlikely(vap_id == 0 || vap_id >= WLAN_VAP_MAX_NUM_PER_DEVICE_LIMIT)) {
        return OSAL_FALSE;
    }
    return OSAL_TRUE;
}

osal_u8 hmac_vap_mode_param_check(const hmac_vap_stru *hmac_vap)
{
    if (osal_unlikely(hmac_vap == OSAL_NULL || hmac_vap_id_param_check(hmac_vap->vap_id) == OSAL_FALSE || \
        hmac_vap->vap_mode == WLAN_VAP_MODE_CONFIG)) {
        return OSAL_FALSE;
    }
    return OSAL_TRUE;
}

osal_u8 hmac_user_assoc_id_param_check(osal_u16 assoc_id)
{
    if (osal_unlikely(assoc_id >= WLAN_USER_MAX_USER_LIMIT)) {
        return OSAL_FALSE;
    }
    return OSAL_TRUE;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
