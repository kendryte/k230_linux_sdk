/*
 * Copyright (c) CompanyNameMagicTag 2020-2020. All rights reserved.
 * Description: hmac_hook.c 的头文件
 * Date: 2020-07-07
 */

#ifndef __HMAC_HOOK_H__
#define __HMAC_HOOK_H__

/*****************************************************************************
  1 其他头文件包吿
*****************************************************************************/
#include "hmac_main.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*****************************************************************************
  Hook机制对外API接口以及结构体定义
*****************************************************************************/
/* Hook点定义 */
enum HMAC_HOOK_NUM {
    /* HMAC侧数据帧和管理帧转发Hook点定义 */
    HMAC_FRAME_HOOK_START                 = 0,
    HMAC_FRAME_MGMT_RX_EVENT_D2H = HMAC_FRAME_HOOK_START,
    HMAC_FRAME_MGMT_RX_EVENT_FEATURE,
    HMAC_FRAME_DATA_RX_EVENT_D2H,        /* DMAC to HAMC DRX事件处理入口 */
    HMAC_FRAME_DATA_RX_EVENT_H2O,        /* HMAC to OS 交操作系统接收入口 */
    HMAC_FRAME_DATA_TX_BRIDGE_IN,        /* 报文处理入口 */
    HMAC_FRAME_DATA_TX_EVENT_H2D,        /* HMAC to DMAC DTX事件发送前 */
    HMAC_FRAME_HOOK_END
};

enum HMAC_HOOK_PRI {
    HMAC_HOOK_PRI_HIGHEST  = 0,
    HMAC_HOOK_PRI_HIGH     = 1,
    HMAC_HOOK_PRI_MIDDLE   = 2,
    HMAC_HOOK_PRI_LOW      = 3,
    HMAC_HOOK_PRI_NUM
};

typedef osal_u32 (*hmac_hookfn)(oal_netbuf_stru **netbuf, hmac_vap_stru *hmac_vap);

typedef struct {
    struct osal_list_head list_entry;
    osal_u16 hooknum;
    osal_u16 priority;
    hmac_hookfn hook_func;
} hmac_netbuf_hook_stru;

/*
  Hook函数参数定义如下
  Hook函数的合法返回值只有以下三个，返回其他值会造成异常
  OAL_CONTINUE: 当前Hook执行完成，判定报文正常处理，并继续后续Hook执行, Hook机制最终返回OAL_CONTINUE
  OAL_SUCC: Hook判定执行完当前Hook之后，跳过后续Hook执行返回主流程，Hook机制最终返回OAL_CONTINUE
  OAL_FAIL: Hook判定需要异常返回，Hook机制最终返回OAL_FAIL
*/
typedef osal_u32 hmac_netbuf_hookfn(oal_netbuf_stru **netbuf, hmac_vap_stru *hmac_vap);

/*
   功能: 注册netbuf类型的Hook函数
   参数: hooknum -- HMAC_HOOK_NUM, priority -- HMAC_HOOK_PRI, hook -- Hook函数
   返回值：成功返回OAL_SUCC, 失败返回OAL_FAIL
*/
osal_u32 hmac_register_netbuf_hook(hmac_netbuf_hook_stru *netbuf_hook_new);
/*
   功能: 注销netbuf类型的Hook函数
   参数: hooknum -- HMAC_HOOK_NUM, priority -- HMAC_HOOK_PRI
   返回值：无
*/
osal_void hmac_unregister_netbuf_hook(hmac_netbuf_hook_stru *netbuf_hook);
/*
   功能: 按注册优先级执行指定Hook点已注册的Hook函数
   参数: netbuf - 指向待处理报文缓存的2级指针 hmac_vap - dmac vap指针 hooknum - Hook点
   返回值：OAL_CONTINUE - Hook执行成功并需要继续主流程处理; OAL_SUCC - 执行成功并退出主流程; OAL_FAIL - 需要异常返回
*/
osal_u32 hmac_call_netbuf_hooks(oal_netbuf_stru **netbuf, hmac_vap_stru *hmac_vap, osal_u8 hooknum);

osal_void hmac_hook_init(osal_void);
osal_void hmac_hook_exit(osal_void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of hmac_hook.h */
