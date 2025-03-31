/*
 * Copyright (c) CompanyNameMagicTag 2021-2021. All rights reserved.
 * Description: diag channel
 * This file should be changed only infrequently and with great care.
 */
#include "zdiag_channel.h"
#include "securec.h"
#include "zdiag_mem.h"
#include "zdiag_config.h"
#include "zdiag_adapt_layer.h"

typedef struct {
    diag_channel_item item[DIAG_SUPPORT_CHANNEL_CNT];
} diag_channel_ctrl;

diag_channel_ctrl g_diag_channel_ctrl = {
    .item[0].init = 0,
};

TD_PRV diag_channel_ctrl *diag_get_channel_ctrl(td_void)
{
    return &g_diag_channel_ctrl;
}

diag_channel_item *diag_chan_idx_2_item(diag_channel_id id)
{
    diag_channel_ctrl *chan_ctrl = diag_get_channel_ctrl();
    return &chan_ctrl->item[id];
}

ext_errno soc_diag_chanel_set_zcopy_tx_hook(diag_channel_id id, diag_channel_zcopy_tx_hook hook)
{
    diag_channel_item *item = diag_chan_idx_2_item(id);
    if (item == TD_NULL) {
        return EXT_ERR_FAILURE;
    }

    item->zcopy_tx_hook = hook;
    return EXT_ERR_SUCCESS;
}

ext_errno soc_diag_channel_set_notify_hook(diag_channel_id id, diag_channel_notify_hook hook)
{
    diag_channel_item *item = diag_chan_idx_2_item(id);
    if (item == TD_NULL) {
        return EXT_ERR_FAILURE;
    }

    item->notify_hook = hook;
    return EXT_ERR_SUCCESS;
}

ext_errno soc_diag_channel_set_tx_hook(diag_channel_id id, diag_channel_tx_hook hook)
{
    diag_channel_item *item = diag_chan_idx_2_item(id);
    if (item == TD_NULL) {
        return EXT_ERR_FAILURE;
    }

    item->tx_hook = hook;
    return EXT_ERR_SUCCESS;
}

ext_errno soc_diag_channel_set_connect_hso_addr(diag_channel_id id, td_u8 hso_addr)
{
    diag_channel_item *item = diag_chan_idx_2_item(id);
    if (item == TD_NULL) {
        return EXT_ERR_FAILURE;
    }
    item->hso_addr = hso_addr;
    return EXT_ERR_SUCCESS;
}

diag_channel_item *zdiag_dst_2_chan(td_u8 addr)
{
    diag_channel_id channel_id = diag_adapt_dst_2_channel_id(addr);
    diag_channel_item *item = diag_chan_idx_2_item(channel_id);
    return item;
}

td_u32 zdiag_get_auid(td_void)
{
    diag_channel_id channel_id = diag_adapt_dst_2_channel_id(101);
    diag_channel_item *item = diag_chan_idx_2_item(channel_id);
    return item->au_id;
}

ext_errno soc_diag_channel_init(diag_channel_id id, td_u32 attribute)
{
    diag_channel_item *item = diag_chan_idx_2_item(id);
    if (item == TD_NULL || item->init == TD_TRUE) {
        return EXT_ERR_FAILURE;
    }

    if (attribute & SOC_DIAG_CHANNEL_ATTR_NEED_RX_BUF) {
        item->rx_buf_len = CONFIG_DIAG_RX_BUF_SIZE;
        item->rx_buf_pos = 0;
        item->rx_buf_is_using = TD_FALSE;
        item->init = TD_TRUE;
        item->rx_buf = diag_malloc(DIAG_MEM_TYPE_RX_DATA_BUF, item->rx_buf_len);
        if (item->rx_buf == TD_NULL) {
            return EXT_ERR_FAILURE;
        }
    }
    return EXT_ERR_SUCCESS;
}

ext_errno soc_diag_channel_exit(diag_channel_id id, td_u32 attribute)
{
    diag_channel_item *item = diag_chan_idx_2_item(id);
    if (item == TD_NULL || item->init == TD_FALSE) {
        return EXT_ERR_FAILURE;
    }
 
    if (attribute & SOC_DIAG_CHANNEL_ATTR_NEED_RX_BUF) {
        diag_free(item->rx_buf);
        item->rx_buf = TD_NULL;
        item->init = TD_FALSE;
    }
    return EXT_ERR_SUCCESS;
}

