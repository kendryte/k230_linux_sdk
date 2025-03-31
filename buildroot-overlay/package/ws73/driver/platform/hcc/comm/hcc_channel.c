/*
 * Copyright (c) CompanyNameMagicTag 2021-2023. All rights reserved.
 * Description: hcc channel.
 * Author: CompanyName
 * Create: 2021-09-11
 */

#include "hcc_channel.h"
#include "hcc_bus.h"
#include "hcc_if.h"
#include "osal_def.h"

static hcc_handler_list g_hcc_handler_list = {
    {&g_hcc_handler_list.handler_list, &g_hcc_handler_list.handler_list}, TD_NULL
};

hcc_handler *hcc_get_handler(hcc_channel_name channel_name)
{
    return (g_hcc_handler_list.handler == TD_NULL ? TD_NULL :
            (g_hcc_handler_list.handler->channel_name != channel_name ? TD_NULL :
             g_hcc_handler_list.handler));
}

td_s32 hcc_add_handler(hcc_channel_name channel_name, hcc_handler *hcc)
{
    uapi_unused(channel_name);
    if (g_hcc_handler_list.handler != TD_NULL) {
        return EXT_ERR_HCC_HANDLER_REPEAT;
    }
    g_hcc_handler_list.handler = hcc;
    return EXT_ERR_SUCCESS;
}

td_void hcc_delete_handler(hcc_channel_name channel_name)
{
    uapi_unused(channel_name);
    osal_kfree(g_hcc_handler_list.handler);
    g_hcc_handler_list.handler = TD_NULL;
}

td_bool hcc_get_state(hcc_channel_name channel_name)
{
    hcc_handler *hcc = hcc_get_handler(channel_name);
    if (hcc == TD_NULL) {
        return TD_FALSE;
    }
    return osal_atomic_read(&hcc->hcc_state) == HCC_ON;
}

td_bool hcc_chan_is_busy(hcc_channel_name channel_name)
{
    hcc_handler *hcc = hcc_get_handler(channel_name);
    if (hcc == TD_NULL) {
        return TD_FALSE;
    }

    if (hcc_bus_is_busy(hcc->bus, HCC_DIR_TX) || hcc_bus_is_busy(hcc->bus, HCC_DIR_RX)) {
        return TD_TRUE;
    }

    return TD_FALSE;
}
osal_module_export(hcc_chan_is_busy);
osal_module_export(hcc_get_state);
osal_module_export(hcc_get_handler);
