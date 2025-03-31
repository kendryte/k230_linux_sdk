/*
 * Copyright (c) @CompanyNameMagicTag 2020-2021. All rights reserved.
 * Description: Message queue processing of the IPC module.
 * Author: Huanghe
 * Create: 2020-01-01
 */

/*****************************************************************************
    头文件包含
*****************************************************************************/
#include "frw_ext_if.h"
#include "frw_timer.h"
#include "frw_hmac.h"
#include "frw_thread.h"
#include "wlan_msg.h"
#include "frw_hmac_hcc_adapt.h"
#if defined(_PRE_PRODUCT_ID_HOST)
#include "hcc_host_if.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_FRW_MAIN_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

/*****************************************************************************
    全局变量定义
*****************************************************************************/
frw_init_enum_uint16 g_en_wlan_driver_init_state_etc = FRW_INIT_STATE_BUTT;

/*****************************************************************************
    函数实现
*****************************************************************************/

/*****************************************************************************
 功能描述 : frw 事件注册
*****************************************************************************/

DEFINE_GET_BUILD_VERSION_FUNC(frw);

/*****************************************************************************
 功能描述 : FRW模块初始化总入口，包含FRW模块内部所有特性的初始化。
*****************************************************************************/
osal_s32 frw_main_init_etc(osal_void)
{
    frw_set_init_state_etc(FRW_INIT_STATE_START);
    if (frw_msg_host_init(WLAN_MSG_D2D_BEGIN, WLAN_MSG_W2D_C_END, EN_WLAN_MSG_END) != OAL_SUCC) {
        frw_debug("hmac_main_init_etc:frw_msg_host_init failed!\n");
    }

    frw_timer_init(FRW_TIMER_DEFAULT_TIME, frw_timer_timeout_proc_event_etc, 0);

    frw_start_hcc_service(HCC_ACTION_TYPE_WIFI, frw_rx_wifi_post_action_function);
    frw_start_hcc_service(HCC_ACTION_TYPE_CUSTOMIZE, frw_rx_wifi_cfg_function);

    /* 启动成功后，输出打印 设置状态始终放最后 */
    frw_set_init_state_etc(FRW_INIT_STATE_FRW_SUCC);

    frw_debug("frw_main_init_etc end\r\n");
    return OAL_SUCC;
}

/*****************************************************************************
 功能描述 : host FRW模块退出
*****************************************************************************/
static osal_void frw_main_host_exit(osal_bool thread_stop)
{
    /* HCC卸载接口 */
    frw_stop_hcc_service(HCC_ACTION_TYPE_WIFI);
    frw_stop_hcc_service(HCC_ACTION_TYPE_CUSTOMIZE);

    /* 卸载定时器 */
    frw_timer_exit();

    /* 销毁线程 */
    frw_thread_exit(thread_stop);

    /* 卸载成功后在置状态位 */
    frw_set_init_state_etc(FRW_INIT_STATE_START);

    /* 卸载成功后，输出打印 */
    frw_debug("frw_main_exit_etc end\r\n");
    return;
}

/*****************************************************************************
 功能描述 : FRW模块卸载,在KO卸载中使用
*****************************************************************************/
osal_void frw_main_exit_etc(osal_void)
{
    frw_main_host_exit(OSAL_TRUE);
}

#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
/*****************************************************************************
 功能描述 : FRW模块卸载,在reboot流程使用,不等待线程销毁,reboot流程概率死等线程销毁
*****************************************************************************/
osal_void frw_main_destroy(osal_void)
{
    frw_main_host_exit(OSAL_FALSE);
}
#endif

/*****************************************************************************
 功能描述 : 设置初始化状态
*****************************************************************************/
osal_void frw_set_init_state_etc(frw_init_enum_uint16 init_state)
{
    if (init_state >= FRW_INIT_STATE_BUTT) {
        oam_warning_log1(0, OAM_SF_FRW, "{frw_set_init_state_etc:: init_state >= FRW_INIT_STATE_BUTT: %d}",
            init_state);
        return;
    }

    g_en_wlan_driver_init_state_etc = init_state;

    return;
}

/*****************************************************************************
 功能描述 : 获取初始化状态
*****************************************************************************/
frw_init_enum_uint16 frw_get_init_state_etc(osal_void)
{
    return g_en_wlan_driver_init_state_etc;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
