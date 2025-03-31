/*
 * Copyright (c) @CompanyNameMagicTag 2023. All rights reserved.
 */
#include "bsle_dft_host_register.h"
#include "bsle_dft_chan_kernel.h"
#include "bsle_dft_hcc_proc.h"

#include <asm/unaligned.h>

MODULE_LICENSE("GPL");

static int32_t sle_host_init(void)
{
    int32_t ret;
    ret = sle_chan_init();
    if (ret == SLE_CHAN_FAILED) {
        oal_print_err("sle_chan_init failed\r\n");
        return OAL_FAIL;
    }
    sle_hci_register_send_frame_cb((sle_send_frame_cb_t)sle_hci_send_frame);
    ble_hci_register_send_frame_cb((sle_send_frame_cb_t)ble_hci_send_frame);

    oal_print_err("sle host init finished\n");
    return OAL_SUCC;
}

static osal_void sle_host_deinit(void)
{
    sle_chan_exit();
    oal_print_err("finish: H2D_MSG_SLE_CLOSE\n");
}

module_init(sle_host_init);
module_exit(sle_host_deinit);
