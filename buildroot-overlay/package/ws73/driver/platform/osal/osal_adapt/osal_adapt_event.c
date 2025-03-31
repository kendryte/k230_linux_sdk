/*
 * Copyright (c) CompanyNameMagicTag 2022-2023. All rights reserved.
 * Description: osal adapt event source file.
 * Author: CompanyName
 * Create: 2022-11-01
 * This file should be changed only infrequently and with great care.
 */

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#ifdef _OSAL_LITEOS_SDK_
#include <los_memory.h>
#include "los_event.h"
#include "los_sys.h"
#include "los_task.h"
#endif
#include "osal_adapt.h"

/*****************************************************************************
  2 宏定义
*****************************************************************************/
#define BIT_31 (1 << 31)
/* Event 相关接口 用于适配liteos系统 (Linux 系统无该接口) */

#ifdef _OSAL_LITEOS_SDK_
/************************ osal_event ************************/
int osal_adapt_event_init(osal_event *event_obj)
{
    unsigned int ret;
    if (event_obj == NULL) {
        osal_printk("parameter invalid!\n");
        return OSAL_FAILURE;
    }
    event_obj->event = (EVENT_CB_S *)LOS_MemAlloc((void*)m_aucSysMem0, sizeof(EVENT_CB_S));
    if (event_obj->event == NULL) {
        osal_printk("LOS_MemAlloc failed!\n");
        return OSAL_FAILURE;
    }
    ret = LOS_EventInit(event_obj->event);
    if (ret != LOS_OK) {
        LOS_MemFree((void*)m_aucSysMem0, event_obj->event);
        event_obj->event = NULL;
        osal_printk("LOS_EventInit failed! ret = %#x.\n", ret);
        return OSAL_FAILURE;
    }
    return OSAL_SUCCESS;
}

int osal_adapt_event_write(osal_event *event_obj, unsigned int mask)
{
    unsigned int ret;
    if (event_obj == NULL || (mask & BIT_31) == 1) {
        osal_printk("parameter invalid! mask=%#x.\n", mask);
        return OSAL_FAILURE;
    }
    ret = LOS_EventWrite(event_obj->event, mask);
    if (ret != OSAL_SUCCESS) {
        osal_printk("LOS_EventWrite failed! ret = %#x.\n", ret);
        return OSAL_FAILURE;
    }
    return OSAL_SUCCESS;
}

int osal_adapt_event_read(osal_event *event_obj, unsigned int mask, unsigned int timeout_ms, unsigned int mode)
{
    unsigned int ret;
    if (event_obj == NULL || (mask & BIT_31) == 1) {
        osal_printk("parameter invalid!\n");
        return OSAL_FAILURE;
    }
    ret = LOS_EventRead(event_obj->event, mask, mode, LOS_MS2Tick(timeout_ms));
    if (!ret || (ret & LOS_ERRTYPE_ERROR)) {
        osal_printk("LOS_EventRead failed! ret = %#x.\n", ret);
        return OSAL_FAILURE;
    } else {
        return (int)ret;
    }
}

int osal_adapt_event_clear(osal_event *event_obj, unsigned int mask)
{
    unsigned int ret;
    if (event_obj == NULL) {
        osal_printk("parameter invalid!\n");
        return OSAL_FAILURE;
    }
    ret = LOS_EventClear(event_obj->event, ~mask);
    if (ret != OSAL_SUCCESS) {
        osal_printk("LOS_EventClear failed! ret = %#x.\n", ret);
        return OSAL_FAILURE;
    }
    return OSAL_SUCCESS;
}

int osal_adapt_event_destroy(osal_event *event_obj)
{
    unsigned int ret;
    if (event_obj == NULL) {
        osal_printk("parameter invalid!\n");
        return OSAL_FAILURE;
    }
    ret = LOS_EventDestroy(event_obj->event);
    LOS_MemFree((void*)m_aucSysMem0, event_obj->event);
    event_obj->event = NULL;
    if (ret != OSAL_SUCCESS) {
        osal_printk("LOS_EventDestroy failed! ret = %#x.\n", ret);
        return OSAL_FAILURE;
    }
    return OSAL_SUCCESS;
}
#endif
