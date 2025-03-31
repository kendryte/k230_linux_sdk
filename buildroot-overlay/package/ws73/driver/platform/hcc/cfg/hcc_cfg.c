/*
 * Copyright (c) CompanyNameMagicTag 2021-2023. All rights reserved.
 * Description: hcc product configuration.
 * Author: CompanyName
 * Create: 2021-05-13
 */

#include "hcc_cfg.h"
#include "td_base.h"
#include "hcc_comm.h"

#ifdef CONFIG_HCC_SUPPORT_IPC
#include "hcc_adapt_ipc.h"
#endif

#ifdef CONFIG_HCC_SUPPORT_SDIO
#include "hcc_adapt_sdio.h"
#endif

#ifdef CONFIG_HCC_SUPPORT_USB
#include "hcc_adapt_usb.h"
#endif

#ifdef CONFIG_HCC_SUPPORT_UART
#include "hcc_adapt_uart.h"
#endif

static osal_char *g_hcc_task_name[HCC_CHANNEL_COUNT] = {
    [HCC_CHANNEL_HMAC] = "hcc_hmac",
    [HCC_CHANNEL_DMAC] = "hcc_dmac",
    [HCC_CHANNEL_AP] = "hcc_ap",
};

static bus_load_unload g_bus_load_func[HCC_BUS_BUTT] = {
#ifdef CONFIG_HCC_SUPPORT_IPC
    [HCC_BUS_IPC]  = { hcc_adapt_ipc_load, OSAL_NULL },
#endif

#ifdef CONFIG_HCC_SUPPORT_SDIO
    [HCC_BUS_SDIO] = { hcc_adapt_sdio_load, hcc_adapt_sdio_unload },
#endif

#ifdef CONFIG_HCC_SUPPORT_USB
    [HCC_BUS_USB]  = { hcc_adapt_usb_load, hcc_adapt_usb_unload },
#endif

#ifdef CONFIG_HCC_SUPPORT_UART
    [HCC_BUS_UART] = { hcc_adapt_uart_load, hcc_adapt_uart_unload },
#endif

    [HCC_BUS_PCIE] = { OSAL_NULL, OSAL_NULL },
};

static hcc_queue_cfg g_hcc_queue_cfg[] = {
    { HCC_DIR_TX, HCC_QUEUE_INTERNAL,
      .queue_ctrl.service_type = HCC_ACTION_TYPE_OAM,
      .queue_ctrl.transfer_mode = HCC_SINGLE_MODE,
      .queue_ctrl.fc_enable = HCC_DISABLE,
      .queue_ctrl.flow_type = HCC_FLOWCTRL_CREDIT,
      .queue_ctrl.low_waterline = HCC_FLOWCTRL_DEFAULT_LO_WATER_LINE,
      .queue_ctrl.high_waterline = HCC_FLOWCTRL_DEFAULT_HI_WATER_LINE,
      .queue_ctrl.burst_limit = HCC_DEFAULT_QUEUE_TRANSFER_BURST_LIMIT,
      .queue_ctrl.credit_bottom_value = HCC_FLOWCTRL_DEFAULT_CREDIT_BOTTOM_VALUE,
      .queue_ctrl.group_id = bit(H2D_MSG_FLOWCTRL_ON) | bit(H2D_MSG_FLOWCTRL_OFF),
    },
    { HCC_DIR_RX, HCC_QUEUE_INTERNAL,
      .queue_ctrl.service_type = HCC_ACTION_TYPE_OAM,
      .queue_ctrl.transfer_mode = HCC_SINGLE_MODE,
      .queue_ctrl.fc_enable = HCC_DISABLE,
      .queue_ctrl.flow_type = HCC_FLOWCTRL_DATA,
      .queue_ctrl.low_waterline = HCC_FLOWCTRL_DEFAULT_LO_WATER_LINE,
      .queue_ctrl.high_waterline = HCC_FLOWCTRL_DEFAULT_HI_WATER_LINE,
      .queue_ctrl.burst_limit = HCC_DEFAULT_QUEUE_TRANSFER_BURST_LIMIT,
      .queue_ctrl.credit_bottom_value = HCC_FLOWCTRL_DEFAULT_CREDIT_BOTTOM_VALUE,
      .queue_ctrl.group_id = bit(H2D_MSG_FLOWCTRL_ON) | bit(H2D_MSG_FLOWCTRL_OFF),
    },
    { HCC_DIR_TX, BT_DATA_QUEUE,
      .queue_ctrl.service_type = HCC_ACTION_TYPE_BT,
      .queue_ctrl.transfer_mode = HCC_SINGLE_MODE,
      .queue_ctrl.fc_enable = HCC_DISABLE,
      .queue_ctrl.flow_type = HCC_FLOWCTRL_DATA,
      .queue_ctrl.low_waterline = HCC_FLOWCTRL_DEFAULT_LO_WATER_LINE,
      .queue_ctrl.high_waterline = HCC_FLOWCTRL_DEFAULT_HI_WATER_LINE,
      .queue_ctrl.burst_limit = HCC_DEFAULT_QUEUE_TRANSFER_BURST_LIMIT,
      .queue_ctrl.credit_bottom_value = HCC_FLOWCTRL_DEFAULT_CREDIT_BOTTOM_VALUE,
      .queue_ctrl.group_id = bit(H2D_MSG_FLOWCTRL_ON) | bit(H2D_MSG_FLOWCTRL_OFF),
    },
    { HCC_DIR_RX, BT_DATA_QUEUE,
      .queue_ctrl.service_type = HCC_ACTION_TYPE_BT,
      .queue_ctrl.transfer_mode = HCC_SINGLE_MODE,
      .queue_ctrl.fc_enable = HCC_DISABLE,
      .queue_ctrl.flow_type = HCC_FLOWCTRL_DATA,
      .queue_ctrl.low_waterline = HCC_FLOWCTRL_DEFAULT_LO_WATER_LINE,
      .queue_ctrl.high_waterline = HCC_FLOWCTRL_DEFAULT_HI_WATER_LINE,
      .queue_ctrl.burst_limit = HCC_DEFAULT_QUEUE_TRANSFER_BURST_LIMIT,
      .queue_ctrl.credit_bottom_value = HCC_FLOWCTRL_DEFAULT_CREDIT_BOTTOM_VALUE,
      .queue_ctrl.group_id = bit(H2D_MSG_FLOWCTRL_ON) | bit(H2D_MSG_FLOWCTRL_OFF),
    },
    { HCC_DIR_TX, SLE_DATA_QUEUE,
      .queue_ctrl.service_type = HCC_ACTION_TYPE_SLE,
      .queue_ctrl.transfer_mode = HCC_SINGLE_MODE,
      .queue_ctrl.fc_enable = HCC_DISABLE,
      .queue_ctrl.flow_type = HCC_FLOWCTRL_DATA,
      .queue_ctrl.low_waterline = HCC_FLOWCTRL_DEFAULT_LO_WATER_LINE,
      .queue_ctrl.high_waterline = HCC_FLOWCTRL_DEFAULT_HI_WATER_LINE,
      .queue_ctrl.burst_limit = HCC_DEFAULT_QUEUE_TRANSFER_BURST_LIMIT,
      .queue_ctrl.credit_bottom_value = HCC_FLOWCTRL_DEFAULT_CREDIT_BOTTOM_VALUE,
      .queue_ctrl.group_id = bit(H2D_MSG_FLOWCTRL_ON) | bit(H2D_MSG_FLOWCTRL_OFF),
    },
    { HCC_DIR_RX, SLE_DATA_QUEUE,
      .queue_ctrl.service_type = HCC_ACTION_TYPE_SLE,
      .queue_ctrl.transfer_mode = HCC_SINGLE_MODE,
      .queue_ctrl.fc_enable = HCC_DISABLE,
      .queue_ctrl.flow_type = HCC_FLOWCTRL_DATA,
      .queue_ctrl.low_waterline = HCC_FLOWCTRL_DEFAULT_LO_WATER_LINE,
      .queue_ctrl.high_waterline = HCC_FLOWCTRL_DEFAULT_HI_WATER_LINE,
      .queue_ctrl.burst_limit = HCC_DEFAULT_QUEUE_TRANSFER_BURST_LIMIT,
      .queue_ctrl.credit_bottom_value = HCC_FLOWCTRL_DEFAULT_CREDIT_BOTTOM_VALUE,
      .queue_ctrl.group_id = bit(H2D_MSG_FLOWCTRL_ON) | bit(H2D_MSG_FLOWCTRL_OFF),
    },
    { HCC_DIR_TX, BSLE_MSG_QUEUE,
      .queue_ctrl.service_type = HCC_ACTION_TYPE_BSLE_MSG,
      .queue_ctrl.transfer_mode = HCC_SINGLE_MODE,
      .queue_ctrl.fc_enable = HCC_DISABLE,
      .queue_ctrl.flow_type = HCC_FLOWCTRL_DATA,
      .queue_ctrl.low_waterline = HCC_FLOWCTRL_DEFAULT_LO_WATER_LINE,
      .queue_ctrl.high_waterline = HCC_FLOWCTRL_DEFAULT_HI_WATER_LINE,
      .queue_ctrl.burst_limit = HCC_DEFAULT_QUEUE_TRANSFER_BURST_LIMIT,
      .queue_ctrl.group_id = bit(H2D_MSG_FLOWCTRL_ON) | bit(H2D_MSG_FLOWCTRL_OFF),
    },
    { HCC_DIR_RX, BSLE_MSG_QUEUE,
      .queue_ctrl.service_type = HCC_ACTION_TYPE_BSLE_MSG,
      .queue_ctrl.transfer_mode = HCC_SINGLE_MODE,
      .queue_ctrl.fc_enable = HCC_DISABLE,
      .queue_ctrl.flow_type = HCC_FLOWCTRL_DATA,
      .queue_ctrl.low_waterline = HCC_FLOWCTRL_DEFAULT_LO_WATER_LINE,
      .queue_ctrl.high_waterline = HCC_FLOWCTRL_DEFAULT_HI_WATER_LINE,
      .queue_ctrl.burst_limit = HCC_DEFAULT_QUEUE_TRANSFER_BURST_LIMIT,
      .queue_ctrl.credit_bottom_value = HCC_FLOWCTRL_DEFAULT_CREDIT_BOTTOM_VALUE,
      .queue_ctrl.group_id = bit(H2D_MSG_FLOWCTRL_ON) | bit(H2D_MSG_FLOWCTRL_OFF),
    },
    { HCC_DIR_RX, CTRL_QUEUE,
      .queue_ctrl.service_type = HCC_ACTION_TYPE_CUSTOMIZE,
      .queue_ctrl.transfer_mode = HCC_SINGLE_MODE,
      .queue_ctrl.fc_enable = HCC_DISABLE,
      .queue_ctrl.flow_type = HCC_FLOWCTRL_DATA,
      .queue_ctrl.low_waterline = HCC_FLOWCTRL_DEFAULT_LO_WATER_LINE,
      .queue_ctrl.high_waterline = HCC_FLOWCTRL_DEFAULT_HI_WATER_LINE,
      .queue_ctrl.burst_limit = HCC_DEFAULT_QUEUE_TRANSFER_BURST_LIMIT,
      .queue_ctrl.credit_bottom_value = HCC_FLOWCTRL_DEFAULT_CREDIT_BOTTOM_VALUE,
      .queue_ctrl.group_id = bit(H2D_MSG_FLOWCTRL_ON) | bit(H2D_MSG_FLOWCTRL_OFF),
    },

    { HCC_DIR_TX, CTRL_QUEUE,
      .queue_ctrl.service_type = HCC_ACTION_TYPE_CUSTOMIZE,
      .queue_ctrl.transfer_mode = HCC_SINGLE_MODE,
      .queue_ctrl.fc_enable = HCC_DISABLE,
      .queue_ctrl.flow_type = HCC_FLOWCTRL_DATA,
      .queue_ctrl.low_waterline = HCC_FLOWCTRL_DEFAULT_LO_WATER_LINE,
      .queue_ctrl.high_waterline = HCC_FLOWCTRL_DEFAULT_HI_WATER_LINE,
      .queue_ctrl.burst_limit = HCC_DEFAULT_QUEUE_TRANSFER_BURST_LIMIT,
      .queue_ctrl.credit_bottom_value = HCC_FLOWCTRL_DEFAULT_CREDIT_BOTTOM_VALUE,
      .queue_ctrl.group_id = bit(H2D_MSG_FLOWCTRL_ON) | bit(H2D_MSG_FLOWCTRL_OFF),
    },

    { HCC_DIR_RX, DATA_HI_QUEUE,
      .queue_ctrl.service_type = HCC_ACTION_TYPE_WIFI,
      .queue_ctrl.transfer_mode = HCC_SINGLE_MODE,
      .queue_ctrl.fc_enable = HCC_DISABLE,
      .queue_ctrl.flow_type = HCC_FLOWCTRL_DATA,
      .queue_ctrl.low_waterline = HCC_FLOWCTRL_DEFAULT_LO_WATER_LINE,
      .queue_ctrl.high_waterline = HCC_FLOWCTRL_DEFAULT_HI_WATER_LINE,
      .queue_ctrl.burst_limit = HCC_DEFAULT_QUEUE_TRANSFER_BURST_LIMIT,
      .queue_ctrl.credit_bottom_value = HCC_FLOWCTRL_DEFAULT_CREDIT_BOTTOM_VALUE,
      .queue_ctrl.group_id = bit(H2D_MSG_FLOWCTRL_ON) | bit(H2D_MSG_FLOWCTRL_OFF),
    },

    { HCC_DIR_TX, DATA_HI_QUEUE,
      .queue_ctrl.service_type = HCC_ACTION_TYPE_WIFI,
      .queue_ctrl.transfer_mode = HCC_SINGLE_MODE,
      .queue_ctrl.fc_enable = HCC_DISABLE,
      .queue_ctrl.flow_type = HCC_FLOWCTRL_DATA,
      .queue_ctrl.low_waterline = HCC_FLOWCTRL_DEFAULT_LO_WATER_LINE,
      .queue_ctrl.high_waterline = HCC_FLOWCTRL_DEFAULT_HI_WATER_LINE,
      .queue_ctrl.burst_limit = HCC_DEFAULT_QUEUE_TRANSFER_BURST_LIMIT,
      .queue_ctrl.credit_bottom_value = HCC_FLOWCTRL_DEFAULT_CREDIT_BOTTOM_VALUE,
      .queue_ctrl.group_id = bit(H2D_MSG_FLOWCTRL_ON) | bit(H2D_MSG_FLOWCTRL_OFF),
    },
    { HCC_DIR_RX, DATA_LO_QUEUE,
      .queue_ctrl.service_type = HCC_ACTION_TYPE_WIFI,
      .queue_ctrl.transfer_mode = HCC_ASSEMBLE_MODE,
      .queue_ctrl.fc_enable = HCC_DISABLE,
      .queue_ctrl.flow_type = HCC_FLOWCTRL_DATA,
      .queue_ctrl.low_waterline = HCC_FLOWCTRL_DEFAULT_LO_WATER_LINE,
      .queue_ctrl.high_waterline = HCC_FLOWCTRL_DEFAULT_HI_WATER_LINE,
      .queue_ctrl.burst_limit = HCC_DEFAULT_QUEUE_TRANSFER_BURST_LIMIT,
      .queue_ctrl.credit_bottom_value = HCC_FLOWCTRL_DEFAULT_CREDIT_BOTTOM_VALUE,
      .queue_ctrl.group_id = bit(H2D_MSG_FLOWCTRL_ON) | bit(H2D_MSG_FLOWCTRL_OFF),
    },

    { HCC_DIR_TX, DATA_LO_QUEUE,
      .queue_ctrl.service_type = HCC_ACTION_TYPE_WIFI,
      .queue_ctrl.transfer_mode = HCC_ASSEMBLE_MODE,
      .queue_ctrl.fc_enable = HCC_DISABLE,
      .queue_ctrl.flow_type = HCC_FLOWCTRL_DATA,
      .queue_ctrl.low_waterline = HCC_FLOWCTRL_DEFAULT_LO_WATER_LINE,
      .queue_ctrl.high_waterline = HCC_FLOWCTRL_DEFAULT_HI_WATER_LINE,
      .queue_ctrl.burst_limit = HCC_DEFAULT_QUEUE_TRANSFER_BURST_LIMIT,
      .queue_ctrl.credit_bottom_value = HCC_FLOWCTRL_DEFAULT_CREDIT_BOTTOM_VALUE,
      .queue_ctrl.group_id = bit(H2D_MSG_FLOWCTRL_ON) | bit(H2D_MSG_FLOWCTRL_OFF),
    },

    { HCC_DIR_RX, DATA_UDP_DATA_QUEUE,
      .queue_ctrl.service_type = HCC_ACTION_TYPE_WIFI,
      .queue_ctrl.transfer_mode = HCC_ASSEMBLE_MODE,
      .queue_ctrl.fc_enable = HCC_DISABLE,
      .queue_ctrl.flow_type = HCC_FLOWCTRL_DATA,
      .queue_ctrl.low_waterline = HCC_FLOWCTRL_DEFAULT_LO_WATER_LINE,
      .queue_ctrl.high_waterline = HCC_FLOWCTRL_DEFAULT_HI_WATER_LINE,
      .queue_ctrl.burst_limit = HCC_DEFAULT_QUEUE_TRANSFER_BURST_LIMIT,
      .queue_ctrl.credit_bottom_value = HCC_FLOWCTRL_DEFAULT_CREDIT_BOTTOM_VALUE,
      .queue_ctrl.group_id = bit(H2D_MSG_FLOWCTRL_ON) | bit(H2D_MSG_FLOWCTRL_OFF),
    },

    { HCC_DIR_TX, DATA_UDP_DATA_QUEUE,
      .queue_ctrl.service_type = HCC_ACTION_TYPE_WIFI,
      .queue_ctrl.transfer_mode = HCC_ASSEMBLE_MODE,
      .queue_ctrl.fc_enable = HCC_ENABLE,
      .queue_ctrl.flow_type = HCC_FLOWCTRL_DATA,
      .queue_ctrl.low_waterline = HCC_FLOWCTRL_DEFAULT_LO_WATER_LINE,
      .queue_ctrl.high_waterline = HCC_FLOWCTRL_DEFAULT_HI_WATER_LINE,
      .queue_ctrl.burst_limit = HCC_DEFAULT_QUEUE_TRANSFER_BURST_LIMIT,
      .queue_ctrl.credit_bottom_value = HCC_FLOWCTRL_DEFAULT_CREDIT_BOTTOM_VALUE,
      .queue_ctrl.group_id = bit(H2D_MSG_FLOWCTRL_ON) | bit(H2D_MSG_FLOWCTRL_OFF),
    },

    { HCC_DIR_RX, DATA_TCP_ACK_QUEUE,
      .queue_ctrl.service_type = HCC_ACTION_TYPE_WIFI,
      .queue_ctrl.transfer_mode = HCC_ASSEMBLE_MODE,
      .queue_ctrl.fc_enable = HCC_DISABLE,
      .queue_ctrl.flow_type = HCC_FLOWCTRL_DATA,
      .queue_ctrl.low_waterline = HCC_FLOWCTRL_DEFAULT_LO_WATER_LINE,
      .queue_ctrl.high_waterline = HCC_FLOWCTRL_DEFAULT_HI_WATER_LINE,
      .queue_ctrl.burst_limit = HCC_DEFAULT_QUEUE_TRANSFER_BURST_LIMIT,
      .queue_ctrl.credit_bottom_value = HCC_FLOWCTRL_DEFAULT_CREDIT_BOTTOM_VALUE,
      .queue_ctrl.group_id = bit(H2D_MSG_FLOWCTRL_ON) | bit(H2D_MSG_FLOWCTRL_OFF),
    },

    { HCC_DIR_TX, DATA_TCP_ACK_QUEUE,
      .queue_ctrl.service_type = HCC_ACTION_TYPE_WIFI,
      .queue_ctrl.transfer_mode = HCC_ASSEMBLE_MODE,
      .queue_ctrl.fc_enable = HCC_ENABLE,
      .queue_ctrl.flow_type = HCC_FLOWCTRL_DATA,
      .queue_ctrl.low_waterline = HCC_FLOWCTRL_DEFAULT_LO_WATER_LINE,
      .queue_ctrl.high_waterline = HCC_FLOWCTRL_DEFAULT_HI_WATER_LINE,
      .queue_ctrl.burst_limit = HCC_DEFAULT_QUEUE_TRANSFER_BURST_LIMIT,
      .queue_ctrl.credit_bottom_value = HCC_FLOWCTRL_DEFAULT_CREDIT_BOTTOM_VALUE,
      .queue_ctrl.group_id = bit(H2D_MSG_FLOWCTRL_ON) | bit(H2D_MSG_FLOWCTRL_OFF),
    },

    { HCC_DIR_RX, DATA_TCP_DATA_QUEUE,
      .queue_ctrl.service_type = HCC_ACTION_TYPE_WIFI,
      .queue_ctrl.transfer_mode = HCC_ASSEMBLE_MODE,
      .queue_ctrl.fc_enable = HCC_DISABLE,
      .queue_ctrl.flow_type = HCC_FLOWCTRL_DATA,
      .queue_ctrl.low_waterline = HCC_FLOWCTRL_DEFAULT_LO_WATER_LINE,
      .queue_ctrl.high_waterline = HCC_FLOWCTRL_DEFAULT_HI_WATER_LINE,
      .queue_ctrl.burst_limit = HCC_DEFAULT_QUEUE_TRANSFER_BURST_LIMIT,
      .queue_ctrl.credit_bottom_value = HCC_FLOWCTRL_DEFAULT_CREDIT_BOTTOM_VALUE,
      .queue_ctrl.group_id = bit(H2D_MSG_FLOWCTRL_ON) | bit(H2D_MSG_FLOWCTRL_OFF),
    },

    { HCC_DIR_TX, DATA_TCP_DATA_QUEUE,
      .queue_ctrl.service_type = HCC_ACTION_TYPE_WIFI,
      .queue_ctrl.transfer_mode = HCC_ASSEMBLE_MODE,
      .queue_ctrl.fc_enable = HCC_ENABLE,
      .queue_ctrl.flow_type = HCC_FLOWCTRL_DATA,
      .queue_ctrl.low_waterline = HCC_FLOWCTRL_DEFAULT_LO_WATER_LINE,
      .queue_ctrl.high_waterline = HCC_FLOWCTRL_DEFAULT_HI_WATER_LINE,
      .queue_ctrl.burst_limit = HCC_DEFAULT_QUEUE_TRANSFER_BURST_LIMIT,
      .queue_ctrl.credit_bottom_value = HCC_FLOWCTRL_DEFAULT_CREDIT_BOTTOM_VALUE,
      .queue_ctrl.group_id = bit(H2D_MSG_FLOWCTRL_ON) | bit(H2D_MSG_FLOWCTRL_OFF),
    },

    { HCC_DIR_RX, DATA_UDP_BK_QUEUE,
      .queue_ctrl.service_type = HCC_ACTION_TYPE_SLP,
      .queue_ctrl.transfer_mode = HCC_SINGLE_MODE,
      .queue_ctrl.fc_enable = HCC_DISABLE,
      .queue_ctrl.flow_type = HCC_FLOWCTRL_DATA,
      .queue_ctrl.low_waterline = HCC_FLOWCTRL_DEFAULT_LO_WATER_LINE,
      .queue_ctrl.high_waterline = HCC_FLOWCTRL_DEFAULT_HI_WATER_LINE,
      .queue_ctrl.burst_limit = HCC_DEFAULT_QUEUE_TRANSFER_BURST_LIMIT,
      .queue_ctrl.credit_bottom_value = HCC_FLOWCTRL_DEFAULT_CREDIT_BOTTOM_VALUE,
      .queue_ctrl.group_id = bit(H2D_MSG_FLOWCTRL_ON) | bit(H2D_MSG_FLOWCTRL_OFF),
    },

    { HCC_DIR_TX, DATA_UDP_BK_QUEUE,
      .queue_ctrl.service_type = HCC_ACTION_TYPE_SLP,
      .queue_ctrl.transfer_mode = HCC_SINGLE_MODE,
      .queue_ctrl.fc_enable = HCC_DISABLE,
      .queue_ctrl.flow_type = HCC_FLOWCTRL_DATA,
      .queue_ctrl.low_waterline = HCC_FLOWCTRL_DEFAULT_LO_WATER_LINE,
      .queue_ctrl.high_waterline = HCC_FLOWCTRL_DEFAULT_HI_WATER_LINE,
      .queue_ctrl.burst_limit = HCC_DEFAULT_QUEUE_TRANSFER_BURST_LIMIT,
      .queue_ctrl.credit_bottom_value = HCC_FLOWCTRL_DEFAULT_CREDIT_BOTTOM_VALUE,
      .queue_ctrl.group_id = bit(H2D_MSG_FLOWCTRL_ON) | bit(H2D_MSG_FLOWCTRL_OFF),
    },
    { HCC_DIR_TX, TEST_SDIO_SINGLE_QUEUE,
      .queue_ctrl.service_type = HCC_ACTION_TYPE_TEST,
      .queue_ctrl.transfer_mode = HCC_SINGLE_MODE,
      .queue_ctrl.fc_enable = HCC_DISABLE,
      .queue_ctrl.flow_type = HCC_FLOWCTRL_CREDIT,
      .queue_ctrl.low_waterline = HCC_FLOWCTRL_DEFAULT_LO_WATER_LINE,
      .queue_ctrl.high_waterline = HCC_FLOWCTRL_DEFAULT_HI_WATER_LINE,
      .queue_ctrl.burst_limit = HCC_MAX_QUEUE_TRANSFER_BURST_LIMIT,
      .queue_ctrl.credit_bottom_value = HCC_FLOWCTRL_DEFAULT_CREDIT_BOTTOM_VALUE,
      .queue_ctrl.group_id = bit(H2D_MSG_FLOWCTRL_ON) | bit(H2D_MSG_FLOWCTRL_OFF),
    },
    { HCC_DIR_RX, TEST_SDIO_SINGLE_QUEUE,
      .queue_ctrl.service_type = HCC_ACTION_TYPE_TEST,
      .queue_ctrl.transfer_mode = HCC_SINGLE_MODE,
      .queue_ctrl.fc_enable = HCC_DISABLE,
      .queue_ctrl.flow_type = HCC_FLOWCTRL_DATA,
      .queue_ctrl.low_waterline = HCC_FLOWCTRL_DEFAULT_LO_WATER_LINE,
      .queue_ctrl.high_waterline = HCC_FLOWCTRL_DEFAULT_HI_WATER_LINE,
      .queue_ctrl.burst_limit = HCC_MAX_QUEUE_TRANSFER_BURST_LIMIT,
      .queue_ctrl.credit_bottom_value = HCC_FLOWCTRL_DEFAULT_CREDIT_BOTTOM_VALUE,
      .queue_ctrl.group_id = bit(H2D_MSG_FLOWCTRL_ON) | bit(H2D_MSG_FLOWCTRL_OFF),
    },
    { HCC_DIR_TX, TEST_SDIO_ASSEM_QUEUE,
      .queue_ctrl.service_type = HCC_ACTION_TYPE_TEST,
      .queue_ctrl.transfer_mode = HCC_ASSEMBLE_MODE,
      .queue_ctrl.fc_enable = HCC_DISABLE,
      .queue_ctrl.flow_type = HCC_FLOWCTRL_DATA,
      .queue_ctrl.low_waterline = HCC_FLOWCTRL_DEFAULT_LO_WATER_LINE,
      .queue_ctrl.high_waterline = HCC_FLOWCTRL_DEFAULT_HI_WATER_LINE,
      .queue_ctrl.burst_limit = HCC_MAX_QUEUE_TRANSFER_BURST_LIMIT,
      .queue_ctrl.credit_bottom_value = HCC_FLOWCTRL_DEFAULT_CREDIT_BOTTOM_VALUE,
      .queue_ctrl.group_id = bit(H2D_MSG_FLOWCTRL_ON) | bit(H2D_MSG_FLOWCTRL_OFF),
    },
    { HCC_DIR_RX, TEST_SDIO_ASSEM_QUEUE,
      .queue_ctrl.service_type = HCC_ACTION_TYPE_TEST,
      .queue_ctrl.transfer_mode = HCC_ASSEMBLE_MODE,
      .queue_ctrl.fc_enable = HCC_DISABLE,
      .queue_ctrl.flow_type = HCC_FLOWCTRL_DATA,
      .queue_ctrl.low_waterline = HCC_FLOWCTRL_DEFAULT_LO_WATER_LINE,
      .queue_ctrl.high_waterline = HCC_FLOWCTRL_DEFAULT_HI_WATER_LINE,
      .queue_ctrl.burst_limit = HCC_MAX_QUEUE_TRANSFER_BURST_LIMIT,
      .queue_ctrl.credit_bottom_value = HCC_FLOWCTRL_DEFAULT_CREDIT_BOTTOM_VALUE,
      .queue_ctrl.group_id = bit(H2D_MSG_FLOWCTRL_ON) | bit(H2D_MSG_FLOWCTRL_OFF),
    }
};

hcc_queue_cfg *hcc_get_queue_cfg(osal_u8 *arr_len)
{
    size_t len = sizeof(g_hcc_queue_cfg) / sizeof(g_hcc_queue_cfg[0]);

    if (arr_len != TD_NULL) {
        *arr_len = len;
    }
    return g_hcc_queue_cfg;
}

hcc_bus_load_func hcc_get_bus_load_func(osal_u8 bus_type)
{
    if (bus_type >= HCC_BUS_BUTT) {
        hcc_printf_err_log("get bus type err\r\n");
        return OSAL_NULL;
    }
    return g_bus_load_func[bus_type].load;
}

hcc_bus_unload_func hcc_get_bus_unload_func(osal_u8 bus_type)
{
    if (bus_type >= HCC_BUS_BUTT) {
        hcc_printf_err_log("get bus type err\r\n");
        return OSAL_NULL;
    }
    return g_bus_load_func[bus_type].unload;
}

osal_char *hcc_get_task_name(hcc_channel_name channel_name)
{
    if (channel_name >= HCC_CHANNEL_COUNT) {
        return HCC_TASK_NAME_DEFAULT;
    }
    return g_hcc_task_name[channel_name];
}
