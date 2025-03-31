/*
 * Copyright (c) CompanyNameMagicTag 2021-2023. All rights reserved.
 * Description: hcc adapt bus completion.
 * Author: CompanyName
 * Create: 2021-05-13
 */

#include "hcc_bus.h"
#include "hcc_bus_types.h"
#include "hcc_comm.h"

#include "soc_module.h"
#include "soc_osal.h"
#include "osal_def.h"
#include "osal_list.h"

#include "securec.h"
#include "hcc_cfg.h"
#include "hcc_if.h"
#include "hcc.h"
#include "hcc_channel.h"
#include "hcc_service.h"
#include "td_type.h"
#include "td_base.h"

td_u32 hcc_get_bus_max_trans_size(int bus_type_)
{
    hcc_bus *bus = hcc_get_channel_bus(HCC_CHANNEL_AP);
    uapi_unused(bus_type_);
    return (bus == TD_NULL ? 0 : bus->max_trans_size);
}

td_void hcc_force_update_queue_id(hcc_bus *bus, td_u8 value)
{
    if (bus == TD_NULL_PTR) {
        return;
    }
    bus->force_update_queue_id = value;
}

td_s32 hcc_bus_power_action(hcc_bus *bus, hcc_bus_power_action_type action)
{
    bus_dev_ops *bus_ops = hcc_get_bus_ops(bus);
    if (bus_ops != TD_NULL) {
        if (bus_ops->power_action != TD_NULL) {
            return bus_ops->power_action(bus, action);
        }
    }
    return EXT_ERR_HCC_BUS_ERR;
}

td_s32 hcc_bus_sleep_request(hcc_bus *bus)
{
    bus_dev_ops *bus_ops = hcc_get_bus_ops(bus);
    if (bus_ops != TD_NULL) {
        if (bus_ops->sleep_request != TD_NULL) {
            return bus_ops->sleep_request(bus);
        }
    }
    return EXT_ERR_HCC_BUS_ERR;
}

td_s32 hcc_bus_wakeup_request(hcc_bus *bus)
{
    bus_dev_ops *bus_ops = hcc_get_bus_ops(bus);
    if (bus_ops != TD_NULL) {
        if (bus_ops->wakeup_request != TD_NULL) {
            return bus_ops->wakeup_request(bus);
        }
    }
    return EXT_ERR_HCC_BUS_ERR;
}

hcc_bus *hcc_get_channel_bus(hcc_channel_name chl)
{
    hcc_handler *hcc = hcc_get_handler(chl);
    if (hcc != TD_NULL) {
        return hcc->bus;
    }
    return TD_NULL;
}

bus_dev_ops *hcc_get_bus_ops(hcc_bus *bus)
{
    if (bus == TD_NULL) {
        return TD_NULL;
    }
    return bus->bus_ops;
}

td_u32 hcc_get_pkt_max_len(hcc_channel_name channel_name)
{
    hcc_bus *bus = hcc_get_channel_bus(channel_name);
    return (bus == TD_NULL ? 0 : bus->max_trans_size);
}

ext_errno hcc_send_message(hcc_channel_name channel_name, hcc_tx_msg_type msg_id, hcc_service_type service_type)
{
    hcc_bus *bus = hcc_get_channel_bus(channel_name);
    bus_dev_ops *bus_ops = hcc_get_bus_ops(bus);

    uapi_unused(service_type);
    if (bus_ops == TD_NULL || msg_id >= HCC_TX_MAX_MESSAGE) {
        hcc_printf_err_log("msg-%d\r\n", msg_id);
        return EXT_ERR_HCC_PARAM_ERR;
    }
    if (bus_ops->send_and_clear_msg != TD_NULL) {
        return bus_ops->send_and_clear_msg(bus, msg_id);
    }
    return EXT_ERR_HCC_BUS_ERR;
}

td_s32 hcc_bus_call_rx_message(hcc_bus *bus, hcc_rx_msg_type msg)
{
    td_s32 ret = EXT_ERR_FAILURE;
    if (msg >= HCC_RX_MAX_MESSAGE) {
        hcc_printf_err_log("msg cb-%d\r\n", msg);
        return ret;
    }
    osal_atomic_inc(&bus->msg[msg].count);
    bus->last_msg = msg;

    if (bus->msg[msg].msg_rx != TD_NULL) {
        ret = (td_s32)bus->msg[msg].msg_rx(bus->msg[msg].data);
    } else {
        hcc_printf_null("msg not registered, msg -%d\r\n", msg);
    }

    return ret;
}

hcc_bus *hcc_alloc_bus(td_void)
{
    hcc_bus *bus = (hcc_bus *)osal_kmalloc(sizeof(hcc_bus), OSAL_GFP_KERNEL);
    if (bus == TD_NULL) {
        hcc_printf_err_log("bus malloc:%zu", sizeof(hcc_bus));
        return TD_NULL;
    }

    memset_s(bus, sizeof(hcc_bus), 0, sizeof(hcc_bus));
    // hcc bus中默认初始化的值需要在调用该接口以后赋值，如对齐长度
    return bus;
}

td_void hcc_free_bus(hcc_bus *bus)
{
    if (bus != TD_NULL) {
        osal_kfree(bus);
        bus = TD_NULL;
    }
}

td_u32 hcc_bus_load(hcc_bus_type bus_type, hcc_handler *hcc)
{
    hcc_bus *bus = TD_NULL;
    hcc_bus_load_func load = hcc_get_bus_load_func(bus_type);
    if (load == TD_NULL) {
        return EXT_ERR_FAILURE;
    }

    bus = load();
    if (bus == TD_NULL) {
        return EXT_ERR_FAILURE;
    }
    bus->hcc = hcc;
    hcc->bus = bus;
    hcc_printf("[success] - load bus: %d\n", bus_type);
    return EXT_ERR_SUCCESS;
}

td_void hcc_bus_unload(hcc_bus *bus)
{
    if (bus != TD_NULL) {
        hcc_bus_unload_func unload = hcc_get_bus_unload_func(bus->bus_type);
        if (unload == TD_NULL) {
            return;
        }
        unload();
    }
}

td_u32 hcc_bus_tx_proc(hcc_bus *bus, hcc_trans_queue *queue, td_u16 *remain_pkt_nums)
{
    bus_dev_ops *bus_ops = hcc_get_bus_ops(bus);
    if (bus_ops != TD_NULL) {
        if (bus_ops->tx_proc != TD_NULL) {
            return bus_ops->tx_proc(bus, queue, remain_pkt_nums);
        }
    }
    hcc_printf("%s: bus ops tx proc is null\r\n", __FUNCTION__);
    return EXT_ERR_HCC_BUS_ERR;
}

td_bool hcc_bus_is_busy(hcc_bus *bus, hcc_queue_dir dir)
{
    bus_dev_ops *bus_ops = hcc_get_bus_ops(bus);
    if (bus_ops != TD_NULL) {
        if (bus_ops->is_busy != TD_NULL) {
            return bus_ops->is_busy(dir);
        }
    }
    return TD_FALSE;
}

td_void hcc_bus_update_credit(hcc_bus *bus, td_u32 credit)
{
    bus_dev_ops *bus_ops = hcc_get_bus_ops(bus);
    if (bus_ops != TD_NULL) {
        if (bus_ops->update_credit != TD_NULL) {
            bus_ops->update_credit(bus, credit);
        }
    }
}

td_s32 hcc_bus_get_credit(hcc_bus *bus, td_u32 *credit)
{
    bus_dev_ops *bus_ops = hcc_get_bus_ops(bus);
    if (bus_ops != TD_NULL && credit != TD_NULL) {
        if (bus_ops->get_credit != TD_NULL) {
            return bus_ops->get_credit(bus, credit);
        }
    }
    return EXT_ERR_HCC_BUS_ERR;
}

td_void hcc_set_tx_sched_count(hcc_channel_name channel_name, td_u8 count)
{
    hcc_bus *bus = hcc_get_channel_bus(channel_name);
    if (bus != TD_NULL) {
        bus->tx_sched_count = count;
    }
}

td_u32 hcc_get_channel_align_len(hcc_channel_name channel_name)
{
    hcc_handler *hcc = hcc_get_handler(channel_name);
    if (hcc == TD_NULL) {
        return HCC_DEFAULT_ALIGN_LEN;
    }

    if (hcc->bus == TD_NULL) {
        return HCC_DEFAULT_ALIGN_LEN;
    }
    return hcc->bus->len_align;
}

td_s32 hcc_bus_reinit(hcc_bus *bus)
{
    bus_dev_ops *bus_ops = hcc_get_bus_ops(bus);
    if (bus_ops != TD_NULL) {
        if (bus_ops->reinit != TD_NULL) {
            return bus_ops->reinit(bus);
        }
    }
    return EXT_ERR_HCC_BUS_ERR;
}


#ifdef CONFIG_HCC_SUPPORT_REG_OPT
ext_errno hcc_read_reg(hcc_channel_name channel_name, td_u32 addr, td_u32 *value)
{
    td_s32 ret;
    bus_dev_ops *bus_ops = TD_NULL;
    hcc_handler *hcc = hcc_get_handler(HCC_CHANNEL_AP);
    if (hcc == TD_NULL || osal_atomic_read(&hcc->hcc_state) == HCC_BUS_FORBID) {
        return EXT_ERR_HCC_BUS_ERR;
    }
    bus_ops = hcc_get_bus_ops(hcc->bus);
    if (bus_ops != TD_NULL) {
        if (bus_ops->read_reg != TD_NULL) {
            ret = bus_ops->read_reg(hcc->bus, addr, value);
            if (ret != EXT_ERR_SUCCESS) {
                hcc_proc_fail_count_add();
            } else {
                hcc_proc_fail_count_clear();
            }
            return ret;
        }
    }
    return EXT_ERR_HCC_BUS_ERR;
}

ext_errno hcc_write_reg(hcc_channel_name channel_name, td_u32 addr, td_u32 value)
{
    td_s32 ret;
    bus_dev_ops *bus_ops = TD_NULL;
    hcc_handler *hcc = hcc_get_handler(HCC_CHANNEL_AP);
    if (hcc == TD_NULL || osal_atomic_read(&hcc->hcc_state) == HCC_BUS_FORBID) {
        return EXT_ERR_HCC_BUS_ERR;
    }
    bus_ops = hcc_get_bus_ops(hcc->bus);
    if (bus_ops != TD_NULL) {
        if (bus_ops->write_reg != TD_NULL) {
            ret = bus_ops->write_reg(hcc->bus, addr, value);
            if (ret != EXT_ERR_SUCCESS) {
                hcc_proc_fail_count_add();
            } else {
                hcc_proc_fail_count_clear();
            }
            return ret;
        }
    }
    return EXT_ERR_HCC_BUS_ERR;
}

ext_errno hcc_read_reg_force(hcc_channel_name channel_name, td_u32 addr, td_u32 *value)
{
    bus_dev_ops *bus_ops = TD_NULL;
    hcc_handler *hcc = hcc_get_handler(HCC_CHANNEL_AP);
    if (hcc == TD_NULL) {
        return EXT_ERR_HCC_BUS_ERR;
    }
    bus_ops = hcc_get_bus_ops(hcc->bus);
    if (bus_ops != TD_NULL) {
        if (bus_ops->read_reg != TD_NULL) {
            return bus_ops->read_reg(hcc->bus, addr, value);
        }
    }
    return EXT_ERR_HCC_BUS_ERR;
}

ext_errno hcc_write_reg_force(hcc_channel_name channel_name, td_u32 addr, td_u32 value)
{
    bus_dev_ops *bus_ops = TD_NULL;
    hcc_handler *hcc = hcc_get_handler(HCC_CHANNEL_AP);
    if (hcc == TD_NULL) {
        return EXT_ERR_HCC_BUS_ERR;
    }
    bus_ops = hcc_get_bus_ops(hcc->bus);
    if (bus_ops != TD_NULL) {
        if (bus_ops->write_reg != TD_NULL) {
            return bus_ops->write_reg(hcc->bus, addr, value);
        }
    }
    return EXT_ERR_HCC_BUS_ERR;
}
#endif

#ifdef CONFIG_HCC_SUPPORT_PATCH_OPT
td_s32 hcc_bus_patch_read(hcc_bus *bus, td_u8 *buff, td_s32 len, td_u32 timeout)
{
    bus_dev_ops *bus_ops = hcc_get_bus_ops(bus);
    if (bus_ops != TD_NULL) {
        if (bus_ops->patch_read != TD_NULL) {
            return bus_ops->patch_read(bus, buff, len, timeout);
        }
    }
    return EXT_ERR_HCC_BUS_ERR;
}

td_s32 hcc_bus_patch_write(hcc_bus *bus, td_u8 *buff, td_s32 len)
{
    bus_dev_ops *bus_ops = hcc_get_bus_ops(bus);
    if (bus_ops != TD_NULL) {
        if (bus_ops->patch_write != TD_NULL) {
            return bus_ops->patch_write(bus, buff, len);
        }
    }
    return EXT_ERR_HCC_BUS_ERR;
}
#endif
osal_module_export(hcc_send_message);
osal_module_export(hcc_read_reg);
osal_module_export(hcc_write_reg);
osal_module_export(hcc_get_pkt_max_len);
osal_module_export(hcc_get_channel_align_len);
osal_module_export(hcc_set_tx_sched_count);
