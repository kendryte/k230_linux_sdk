/**
 * Copyright (c) CompanyNameMagicTag 2021-2021. All rights reserved. \n
 * Description: SOC lOG \n
 * Author: CompanyName \n
 */

/**
 * @file soc_log.h
 */

/** @defgroup dfx dfx */
#ifndef __SOC_LOG_H__
#define __SOC_LOG_H__

#include "td_base.h"
#include "soc_module.h"

/**
 * @ingroup dfx
 * Default level of the output debugging information.
 * CNcomment: 默认的调试信息输出级别.CNend
 */
#define SOC_LOG_LEVEL_DEFAULT SOC_LOG_LEVEL_INFO

/**
 * @ingroup dfx
 * SDK internal Use only.
 * Allow modules to modify, If the related module does not define it, no information output.
 */
#ifndef LOG_FUNC_TRACE
#define LOG_FUNC_TRACE 0
#endif

/**
 * @ingroup dfx
 * SDK internal Use Only.
 * Allow modules to modify, If the related module does not define it, no information output.
 */
#ifndef LOG_UNF_TRACE
#define LOG_UNF_TRACE 0
#endif

/**
 * @ingroup dfx
 * SDK internal use only.
 * Allow modules to modify, default value is SOC_ID_SYS.
 */
#ifndef LOG_MODULE_ID
#define LOG_MODULE_ID SOC_ID_MAIN
#endif

#define LOG_BLOCK_PERLINE 16
#define LOG_BLOCK_BUFSIZE (LOG_BLOCK_PERLINE * 3 + 1)

#define ENABLE_FUNC_LINE
#define SOC_DEBUG

/**
 * @ingroup dfx
 * Level of the output debugging information.
 * CNcomment: 调试信息输出级别.CNend
 */
typedef enum {
    SOC_LOG_LEVEL_ALERT = 0,    /**< Alert information. */
                                /**< It indicates that a important exception
                                 *  occurs in the system.action must be taken
                                 *  immediately. \n */
                                /**< CNcomment: 警告信息, 此类错误默认不打印到串口，
                                 * 默认打印到BBOX后台日志收集系统，用户系统重要状态
                                 * 变化，非常重要的异常。CNend */
    SOC_LOG_LEVEL_FATAL = 1,    /**< Fatal error. */
                                /**< It indicates that a Fatal problem occurs in
                                 *  the system. Therefore, you must pay
                                 *  attention to it. \n */
                                /**< CNcomment: 致命错误, 此类错误需要特别关注，一
                                 * 般出现此类错误代表系统出现了重大问题CNend */
    SOC_LOG_LEVEL_ERROR = 2,    /**< Major error. */
                                /**< It indicates that a major problem occurs in
                                 *  the system and the system cannot run. \n */
                                /**< CNcomment: 一般错误, 一般出现此类错误代表系统
                                 * 出现了比较大的问题，不能再正常运行CNend */
    SOC_LOG_LEVEL_WARNING = 3,  /**< Warning. */
                                /**< It indicates that a minor problem occurs in
                                 *  the system, but the system still can run
                                 *  properly. \n */
                                /**< CNcomment: 告警信息, 一般出现此类信息代表系统
                                 * 可能出现问题，但是还能继续运行CNend */
    SOC_LOG_LEVEL_NOTICE = 4,   /**< Notice. */
                                /**< It is used to prompt users. Users can open
                                 *  the message when locating problems. \n */
                                /**< CNcomment: 正常但重要的信息, 一般出现在系统的
                                 * 关键路径调用上CNend */
    SOC_LOG_LEVEL_INFO = 5,     /**< INFO. */
                                /**< It is used to prompt users. Users can open
                                 *  the message when locating problems. \n */
                                /**< It is recommended to disable this message
                                 *  in general. \n */
                                /**< CNcomment: 提示信息, 一般是为提醒用户而输出，
                                 * 在定位问题的时候可以打开，一般情况下建议关闭CNend */
    SOC_LOG_LEVEL_DBG = 6,      /**< Debug. */
                                /**< It is used to prompt developers. Developers
                                 *  can open the message when locating
                                 *  problems. \n */
                                /**< It is recommended to disable this message
                                 *  in general. \n */
                                /**< CNcomment: 提示信息, 一般是为开发人员调试问题
                                 * 而设定的打印级别，一般情况下建议关闭CNend */
    SOC_LOG_LEVEL_TRACE = 7,    /**< Trace. */
                                /**< It is used to track the entry and exit of
                                 *  function when the interface is called. \n */
                                /**< CNcomment: 提示信息，一般用于跟踪接口调用时函
                                 * 数的进入与退出CNend */
    SOC_LOG_LEVEL_MAX
} soc_log_level;

/**
 * @ingroup dfx
 * Just only for alert level print.
 * CNcomment: 为了打印告警信息而制定的宏打印级别CNend
 */
#define SOC_TRACE_LEVEL_ALERT (0)
/**
 * @ingroup dfx
 * Just only for fatal level print.
 * CNcomment: 为了打印致命信息而制定的宏打印级别CNend
 */
#define SOC_TRACE_LEVEL_FATAL (1)
/**
 * @ingroup dfx
 * Just only for error level print.
 * CNcomment: 为了打印错误信息而制定的宏打印级别CNend
 */
#define SOC_TRACE_LEVEL_ERROR (2)
/**
 * @ingroup dfx
 * Just only for warning level print.
 * CNcomment: 为了打印警告信息而制定的宏打印级别CNend
 */
#define SOC_TRACE_LEVEL_WARN (3)
/**
 * @ingroup dfx
 * Just only for notice level print.
 * CNcomment: 为了打印注意信息而制定的宏打印级别CNend
 */
#define SOC_TRACE_LEVEL_NOTICE (4)
/**
 * @ingroup dfx
 * Just only for info level print.
 * CNcomment: 为了打印信息级别而制定的宏打印级别CNend
 */
#define SOC_TRACE_LEVEL_INFO (5)
/**
 * @ingroup dfx
 * Just only for debug level print.
 * CNcomment: 为了打印调试信息而制定的宏打印级别CNend
 */
#define SOC_TRACE_LEVEL_DBG (6)
/**
 * @ingroup dfx
 * Just only for trace level print.
 * CNcomment: 为了打印接口跟踪信息而制定的宏打印级别CNend
 */
#define SOC_TRACE_LEVEL_TRACE (7)

#ifndef CONFIG_SOCT_LOG_LEVEL
#define CONFIG_SOCT_LOG_LEVEL (SOC_TRACE_LEVEL_INFO)
#endif

#ifdef ENABLE_FUNC_LINE
#define SOC_LOG_LINE __LINE__
#else
#define SOC_LOG_LINE 0
#endif

/**
 * Just only debug output, MUST BE NOT calling it.
 * CNcomment: 调试输出信息接口，不推荐直接调用此接口CNend
 */
td_void soc_log_print(td_u32 level, td_u32 module_id, TD_CONST td_char *fn_name, td_u32 line_num,
    TD_CONST td_char *format, ...);

/**
 * CNcomment: 调试输出信息接口，是输出信息，不换行,不建议直接使用CNend
 */
td_void soc_log_simple_print(TD_CONST td_char *format, ...);

/**
 * CNcomment: 关键信息记录,不建议直接使用CNend
 */
td_void soc_key_trace(td_u32 module_id, TD_CONST td_char *format, ...);

/**
 * CNcomment: 存储关键日志信息,是否支持由产品决定,不建议直接使用CNend
 */
#define soc_key_log_trace(fmt...) soc_key_trace(LOG_MODULE_ID, fmt)

#ifdef SOC_DEBUG
/**
 * Just only debug output, MUST BE NOT calling it.
 * CNcomment: 调试输出信息接口，不推荐直接调用此接口CNend
 */
#define soc_trace(level, module_id, fmt...)                                                  \
    do {                                                                                     \
        soc_log_print(level, (td_u32)module_id, (td_char *)__FUNCTION__, SOC_LOG_LINE, fmt); \
    } while (0)

/**
 * CNcomment: 调试输出信息接口，是输出信息，不换行,不建议直接使用CNend
 */
#define soc_simple_trace(fmt...)   \
    do {                           \
        soc_log_simple_print(fmt); \
    } while (0)

#if (CONFIG_SOCT_LOG_LEVEL == SOC_TRACE_LEVEL_ALERT)
/**
 * Supported for debug output to serial/network/u-disk.
 * CNcomment: 各个模块需要调用以下宏进行输出调试信息、可输出到串口、网口、U盘存储等CNend
 * Just only reserve the alert level output.
 * CNcomment: 仅仅保留BBOX关键的调试信息CNend
 */
#define soc_log_alert(fmt...) soc_trace(SOC_TRACE_LEVEL_ALERT, LOG_MODULE_ID, fmt)
#define soc_log_fatal(fmt...)
#define soc_log_err(fmt...)
#define soc_log_warn(fmt...)
#define soc_log_notice(fmt...)
#define soc_log_info(fmt...)
#define soc_log_dbg(fmt...)
#define soc_log_trace(fmt...)

/**
 * Supported for debug output to serial/network/u-disk.
 * CNcomment: 各个模块需要调用以下宏进行输出调试信息、可输出到串口、网口、U盘存储等CNend
 * Just only reserve the fatal level output.
 * CNcomment: 仅仅保留致命的调试信息CNend
 */
#elif (CONFIG_SOCT_LOG_LEVEL == SOC_TRACE_LEVEL_FATAL)
#define soc_log_alert(fmt...) soc_trace(SOC_TRACE_LEVEL_ALERT, LOG_MODULE_ID, fmt)
#define soc_log_fatal(fmt...) soc_trace(SOC_TRACE_LEVEL_FATAL, LOG_MODULE_ID, fmt)
#define soc_log_err(fmt...)
#define soc_log_warn(fmt...)
#define soc_log_notice(fmt...)
#define soc_log_info(fmt...)
#define soc_log_dbg(fmt...)
#define soc_log_trace(fmt...)


/**
 * Just only reserve the alert/fatal/error level output.
 * CNcomment: 仅仅保留致命的和错误级别的调试信息CNend
 */
#elif (CONFIG_SOCT_LOG_LEVEL == SOC_TRACE_LEVEL_ERROR)
#define soc_log_alert(fmt...)                soc_trace(SOC_TRACE_LEVEL_ALERT, LOG_MODULE_ID, fmt)
#define soc_log_fatal(fmt...)                soc_trace(SOC_TRACE_LEVEL_FATAL, LOG_MODULE_ID, fmt)
#define soc_log_err(fmt...)                  soc_trace(SOC_TRACE_LEVEL_ERROR, LOG_MODULE_ID, fmt)
#define soc_log_warn(fmt...)
#define soc_log_notice(fmt...)
#define soc_log_info(fmt...)
#define soc_log_dbg(fmt...)
#define soc_log_trace(fmt...)

/**
 * Just only reserve the alert/fatal/error/warning level output.
 * CNcomment: 仅仅保留致命的、错误的、警告级别的调试信息CNend
 */
#elif (CONFIG_SOCT_LOG_LEVEL == SOC_TRACE_LEVEL_WARN)
#define soc_log_alert(fmt...)                soc_trace(SOC_TRACE_LEVEL_ALERT, LOG_MODULE_ID, fmt)
#define soc_log_fatal(fmt...)                soc_trace(SOC_TRACE_LEVEL_FATAL, LOG_MODULE_ID, fmt)
#define soc_log_err(fmt...)                  soc_trace(SOC_TRACE_LEVEL_ERROR, LOG_MODULE_ID, fmt)
#define soc_log_warn(fmt...)                 soc_trace(SOC_TRACE_LEVEL_WARN,  LOG_MODULE_ID, fmt)
#define soc_log_notice(fmt...)
#define soc_log_info(fmt...)
#define soc_log_dbg(fmt...)
#define soc_log_trace(fmt...)

/**
 * Just only reserve the alert/fatal/error/warning/notice level output.
 * CNcomment: 仅仅保留致命的、错误的、警告、重要级别的调试信息CNend
 */
#elif (CONFIG_SOCT_LOG_LEVEL == SOC_TRACE_LEVEL_NOTICE)
#define soc_log_alert(fmt...)                soc_trace(SOC_TRACE_LEVEL_ALERT,  LOG_MODULE_ID, fmt)
#define soc_log_fatal(fmt...)                soc_trace(SOC_TRACE_LEVEL_FATAL,  LOG_MODULE_ID, fmt)
#define soc_log_err(fmt...)                  soc_trace(SOC_TRACE_LEVEL_ERROR,  LOG_MODULE_ID, fmt)
#define soc_log_warn(fmt...)                 soc_trace(SOC_TRACE_LEVEL_WARN,   LOG_MODULE_ID, fmt)
#define soc_log_notice(fmt...)               soc_trace(SOC_TRACE_LEVEL_NOTICE, LOG_MODULE_ID, fmt)
#define soc_log_info(fmt...)
#define soc_log_dbg(fmt...)
#define soc_log_trace(fmt...)

/**
 * Just only reserve the alert/fatal/error/warning/info level output.
 * CNcomment: 仅仅保留致命的、错误的、警告和信息级别的调试信息CNend
 */
#elif (CONFIG_SOCT_LOG_LEVEL == SOC_TRACE_LEVEL_INFO)
/**
 * @ingroup dfx
 * @par 描述：
 * Include SOC_DEBUG.Supported for debug output to serial/network/u-disk. \n
 * CNcomment: 定义SOC_DEBUG宏时。各个模块需要调用该宏进行输出BBOX关键的调试信息，
 * 可输出到串口、网口、U盘存储等CNend
 * @attention 当打印等级CONFIG_SOCT_LOG_LEVEL在SOC_TRACE_LEVEL_ALERT及以上时，
 * 可使用
 * @param fmt    [IN] 打印输出信息
 * @retval None None
 * @par 依赖:
 * @li dfx
 * @li soc_log.h
 * @see None
 */
#define soc_log_alert(fmt...)                soc_trace(SOC_TRACE_LEVEL_ALERT,  LOG_MODULE_ID, fmt)
/**
 * @ingroup dfx
 * @par 描述：
 * Include SOC_DEBUG.Supported for debug output to serial/network/u-disk. \n
 * CNcomment: 定义SOC_DEBUG宏时。各个模块需要调用该宏进行输出致命级别的调试信息，
 * 可输出到串口、网口、U盘存储等CNend
 * @attention 当打印等级 CONFIG_SOCT_LOG_LEVEL 在 SOC_TRACE_LEVEL_FATAL 及以上时，
 * 可使用
 * @param fmt    [IN] 打印输出信息
 * @retval None None
 * @par 依赖:
 * @li dfx
 * @li soc_log.h
 * @see None
 */
#define soc_log_fatal(fmt...)                soc_trace(SOC_TRACE_LEVEL_FATAL,  LOG_MODULE_ID, fmt)
/**
 * @ingroup dfx
 * @par 描述：
 * Include SOC_DEBUG.Supported for debug output to serial/network/u-disk. \n
 * CNcomment: 定义SOC_DEBUG宏时。各个模块需要调用该宏进行输出错误级别的调试信息，
 * 可输出到串口、网口、U盘存储等CNend
 * @attention 当打印等级 CONFIG_SOCT_LOG_LEVEL 在 SOC_TRACE_LEVEL_ERROR 及以上时，
 * 可使用
 * @param fmt    [IN] 打印输出信息
 * @retval None None
 * @par 依赖:
 * @li dfx
 * @li soc_log.h
 * @see None
 */
#define soc_log_err(fmt...)                  soc_trace(SOC_TRACE_LEVEL_ERROR,  LOG_MODULE_ID, fmt)
/**
 * @ingroup dfx
 * @par 描述：
 * Include SOC_DEBUG.Supported for debug output to serial/network/u-disk. \n
 * CNcomment: 定义SOC_DEBUG宏时。各个模块需要调用该宏进行输出警告级别的调试信息，
 * 可输出到串口、网口、U盘存储等CNend
 * @attention 当打印等级 CONFIG_SOCT_LOG_LEVEL 在 SOC_TRACE_LEVEL_WARN 及以上时，
 * 可使用
 * @param fmt    [IN] 打印输出信息
 * @retval None None
 * @par 依赖:
 * @li dfx
 * @li soc_log.h
 * @see None
 */
#define soc_log_warn(fmt...)                 soc_trace(SOC_TRACE_LEVEL_WARN,   LOG_MODULE_ID, fmt)
/**
 * @ingroup dfx
 * @par 描述：
 * Include SOC_DEBUG.Supported for debug output to serial/network/u-disk. \n
 * CNcomment: 定义SOC_DEBUG宏时。各个模块需要调用该宏进行输出重要级别的调试信息，
 * 可输出到串口、网口、U盘存储等CNend
 * @attention 当打印等级 CONFIG_SOCT_LOG_LEVEL 在 SOC_TRACE_LEVEL_NOTICE 及以上时，
 * 可使用
 * @param fmt    [IN] 打印输出信息
 * @retval None None
 * @par 依赖:
 * @li dfx
 * @li soc_log.h
 * @see None
 */
#define soc_log_notice(fmt...)               soc_trace(SOC_TRACE_LEVEL_NOTICE, LOG_MODULE_ID, fmt)
/**
 * @ingroup dfx
 * @par 描述：
 * Include SOC_DEBUG.Supported for debug output to serial/network/u-disk. \n
 * CNcomment: 定义SOC_DEBUG宏时。各个模块需要调用该宏进行输出信息级别的调试信息，
 * 可输出到串口、网口、U盘存储等CNend
 * @attention 当打印等级 CONFIG_SOCT_LOG_LEVEL 在 SOC_TRACE_LEVEL_INFO 及以上时，
 * 可使用
 * @param fmt    [IN] 打印输出信息
 * @retval None None
 * @par 依赖:
 * @li dfx
 * @li soc_log.h
 * @see None
 */
#define soc_log_info(fmt...)                 soc_trace(SOC_TRACE_LEVEL_INFO,   LOG_MODULE_ID, fmt)
/**
 * @ingroup dfx
 * @par 描述：
 * Include SOC_DEBUG.Supported for debug output to serial/network/u-disk. \n
 * CNcomment: 定义SOC_DEBUG宏时。各个模块需要调用该宏进行输出debug级别的调试信息，
 * 可输出到串口、网口、U盘存储等CNend
 * @attention 当打印等级 CONFIG_SOCT_LOG_LEVEL 在 SOC_TRACE_LEVEL_DBG 及以上时，
 * 可使用
 * @param fmt    [IN] 打印输出信息
 * @retval None None
 * @par 依赖:
 * @li dfx
 * @li soc_log.h
 * @see None
 */
#define soc_log_dbg(fmt...)
/**
 * @ingroup dfx
 * @par 描述：
 * Include SOC_DEBUG.Supported for debug output to serial/network/u-disk. \n
 * CNcomment: 定义SOC_DEBUG宏时。各个模块需要调用该宏进行输出trace级别的调试信息，
 * 可输出到串口、网口、U盘存储等CNend
 * @attention 当打印等级 CONFIG_SOCT_LOG_LEVEL 在 SOC_TRACE_LEVEL_TRACE 时，可使用
 * @param fmt    [IN] 打印输出信息
 * @retval None None
 * @par 依赖:
 * @li dfx
 * @li soc_log.h
 * @see None
 */
#define soc_log_trace(fmt...)

/**
 * Just only reserve the alert/fatal/error/warning/info/debug level output.
 * CNcomment: 仅仅保留致命的、错误的、警告和信息级别的调试信息CNend
 */
#elif (CONFIG_SOCT_LOG_LEVEL == SOC_TRACE_LEVEL_DBG)
#define soc_log_alert(fmt...)                soc_trace(SOC_TRACE_LEVEL_ALERT,  LOG_MODULE_ID, fmt)
#define soc_log_fatal(fmt...)                soc_trace(SOC_TRACE_LEVEL_FATAL,  LOG_MODULE_ID, fmt)
#define soc_log_err(fmt...)                  soc_trace(SOC_TRACE_LEVEL_ERROR,  LOG_MODULE_ID, fmt)
#define soc_log_warn(fmt...)                 soc_trace(SOC_TRACE_LEVEL_WARN,   LOG_MODULE_ID, fmt)
#define soc_log_notice(fmt...)               soc_trace(SOC_TRACE_LEVEL_NOTICE, LOG_MODULE_ID, fmt)
#define soc_log_info(fmt...)                 soc_trace(SOC_TRACE_LEVEL_INFO,   LOG_MODULE_ID, fmt)
#define soc_log_dbg(fmt...)                  soc_trace(SOC_TRACE_LEVEL_DBG,    LOG_MODULE_ID, fmt)
#define soc_log_trace(fmt...)

/**
 * Reserve all the levels output.
 * CNcomment: 保留所有级别调试信息CNend
 */
#else
#define soc_log_alert(fmt...)                soc_trace(SOC_TRACE_LEVEL_ALERT,  LOG_MODULE_ID, fmt)
#define soc_log_fatal(fmt...)                soc_trace(SOC_TRACE_LEVEL_FATAL,  LOG_MODULE_ID, fmt)
#define soc_log_err(fmt...)                  soc_trace(SOC_TRACE_LEVEL_ERROR,  LOG_MODULE_ID, fmt)
#define soc_log_warn(fmt...)                 soc_trace(SOC_TRACE_LEVEL_WARN,   LOG_MODULE_ID, fmt)
#define soc_log_notice(fmt...)               soc_trace(SOC_TRACE_LEVEL_NOTICE, LOG_MODULE_ID, fmt)
#define soc_log_info(fmt...)                 soc_trace(SOC_TRACE_LEVEL_INFO,   LOG_MODULE_ID, fmt)
#define soc_log_dbg(fmt...)                  soc_trace(SOC_TRACE_LEVEL_DBG,    LOG_MODULE_ID, fmt)
#define soc_log_trace(fmt...)                soc_trace(SOC_TRACE_LEVEL_TRACE,  LOG_MODULE_ID, fmt)
#endif

#else
#define soc_trace(level, module_id, fmt...) \
    do {                                    \
    } while (0)

#define soc_log_alert(fmt...) \
    do {                      \
    } while (0)
#define soc_log_fatal(fmt...) \
    do {                      \
    } while (0)
#define soc_log_err(fmt...)   \
    do {                      \
    } while (0)
#define soc_log_warn(fmt...)  \
    do {                      \
    } while (0)
#define soc_log_notice(fmt...) \
    do {                       \
    } while (0)
#define soc_log_info(fmt...) \
    do {                     \
    } while (0)
#define soc_log_dbg(fmt...) \
    do {                    \
    } while (0)
#define soc_log_trace(fmt...) \
    do {                      \
    } while (0)

#endif /* endif SOC_DEBUG */

#endif /* __SOC_LOG_H__ */
