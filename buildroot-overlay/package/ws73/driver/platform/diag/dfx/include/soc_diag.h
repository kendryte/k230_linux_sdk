/*
 * Copyright (c) CompanyNameMagicTag 2020-2020. All rights reserved.
 * Description: diag interface
 */

#ifndef __SOC_DIAG__H__
#define __SOC_DIAG__H__

#include "soc_uart.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef td_u32 (*diag_cmd_f_prv)(td_u16 cmd_id, td_pvoid cmd_param, td_u16 cmd_param_size, td_u8 option);

/**
 * @ingroup  hct_diag
 * Structure for registration command
 */
typedef struct {
    td_u16 min_id;              /* Minimum DIAG ID */
    td_u16 max_id;              /* Maximum DIAG ID */
    diag_cmd_f_prv fn_input_cmd; /* This Handler is used to process the HSO command. */
} ext_diag_cmd_reg_obj;

/**
 * @ingroup  hct_diag
 * @brief  Registers the command handling function.
 *
 * @par Description:
 *         The DIAG subsystem supports a maximum of 20 different command tables. You can run
 *         diag_get_cmd_stat_reg_info to query the number of registered command tables.
 *
 * @attention The range of command IDs that can be used by the SDK user is [0xFD00, 0xFFFF).
 * @param  cmd_tbl  [IN] Type #ext_diag_cmd_reg_obj* Command table, which must be declared as a constant array
 *                       and transmitted to this parameter.
 * @param  cmd_num  [IN] Type #td_u16 Number of commands. The value cannot be 0.
 *
 * @retval #0               Success.
 * @retval #Other values    Failure. For details, see soc_errno.h.
 * @par Dependency:
 *           @li soc_diag.h: Describes DIAG APIs.
 * @see  None
 */
td_u32 diag_register_cmd_prv(TD_CONST ext_diag_cmd_reg_obj *cmd_tbl, td_u16 cmd_num);

/**
 * @ingroup  hct_diag
 * @brief Checks the connection status of the DIAG channel.
 *
 * @par Description:
 *           Checks the connection status of the DIAG channel.
 *
 * @attention None
 * @param  None
 *
 * @retval #TD_FALSE disconnected.
 * @retval #TD_TRUE  connected.
 * @par Dependency:
 *           @li soc_diag.h: Describes DIAG APIs.
 * @see  None
 */

td_bool diag_is_connect_prv(td_void);

 /**
 * @ingroup  hct_diag
 * @brief  Reports DIAG packets.
 *
 * @par Description:
 *           Reports DIAG channel packets to the DIAG client.
 *
 * @attention None
 * @param  cmd_id       [IN] Type #td_u16 DIAG data packet ID. For example, if the packet ID is the same as the command
 *                           request ID, use the cmd_id parameter in the command callback function diag_cmd_f_prv.
 * @param  instance_id  [IN] Type #td_u8 Command type. Valid values are as follows: EXT_DIAG_CMD_INSTANCE_LOCAL and
 *                           EXT_DIAG_CMD_INSTANCE_IREMOTE. Use the option parameter in the command callback function
 *                           diag_cmd_f_prv to notify the DIAG subsystem whether the packet is a local packet or a
 *                           remote packet.
 * @param  packet       [IN] Type #td_pbyte Buffer address of the data packet.The function does not release the pointer.
 * @param  pakcet_size  [IN] Type #td_u16 Data packet size (unit: byte)
 * @param  sync         [IN] Type #td_bool Synchronous or asynchronous DIAG packet pushing. TRUE indicates that the
 *                           packets are pushed synchronously and the operation is blocked. FALSE indicates the packets
 *                           are pushed asynchronously (with the memory allocated, the packet is cashed by the OS queue
 *                           before being pushed), and the operation is not blocked.
 *
 * @retval #0               Success.
 * @retval #Other values    Failure. For details, see soc_errno.h.
 * @par Dependency:
 *           @li soc_diag.h: Describes DIAG APIs.
 * @see  None
 */
td_u32 diag_report_packet_prv(td_u16 cmd_id, td_u8 instance_id, TD_CONST td_pbyte packet,
    td_u16 pakcet_size, td_bool sync);

/**
 * @ingroup  hct_diag
 * @brief  Initializes the DIAG subsystem.
 *
 * @par Description:
 *         Initializes the DIAG subsystem.
 *
 * @attention The initialization of the DIAG subsystem needs to be performed only once. Repeated initialization
 *            is invalid.
 * @param  None
 *
 * @retval #0               Success.
 * @retval #Other values    Failure. For details, see soc_errno.h.
 * @par Dependency:
 *           @li soc_diag.h: Describes DIAG APIs.
 * @see  None
 */
td_u32 diag_init_prv(td_void);

/**
 * @ingroup  hct_diag
 * Structure for report zero parameters in log printing interface.
 */
typedef struct {
} diag_log_msg0;

/**
 * @ingroup  hct_diag
 * Structure for report one parameters in log printing interface.
 */
typedef struct {
    td_u32 data0;
} diag_log_msg1;

/**
 * @ingroup  hct_diag
 * Structure for report two parameters in log printing interface.
 */
typedef struct {
    td_u32 data0;
    td_u32 data1;
} diag_log_msg2;

/**
 * @ingroup  hct_diag
 * Structure for report three parameters in log printing interface.
 */
typedef struct {
    td_u32 data0;
    td_u32 data1;
    td_u32 data2;
} diag_log_msg3;

/**
 * @ingroup  hct_diag
 * Structure for report four parameters in log printing interface.
 */
typedef struct {
    td_u32 data0;
    td_u32 data1;
    td_u32 data2;
    td_u32 data3;
} diag_log_msg4;

typedef enum {
    APPLICATION_CORE = 0,
    PROTOCOL_CORE,
    SECURITY_CORE,
} core_type;

typedef struct {
    td_u32 data0[5];
    td_u32 data1;
} user_define_struct1;

typedef struct {
    td_u32 data0;
    td_u32 data1;
    user_define_struct1 data2;
} user_define_struct0;

 /**
 * @ingroup  hct_diag
 * @brief  Reports simple packet that without data.
 *
 * @par Description:
 *         Reports simple packet that without data.
 *
 * @attention None
 * @param  msg_id    [IN]  Type #td_u32 The message id of the print log.
 * @param  src_mod   [IN]  Type #td_u32 The source module id of the print log.
 * @param  dest_mod  [IN]  Type #td_u32 The destination module of the print log.
 *
 * @retval #0               Success.
 * @retval #Other values    Failure. For details, see soc_errno.h.
 * @par Dependency:
 *           @li soc_diag.h: Describes DIAG APIs.
 * @see  None
 */
td_u32 diag_log_msg0_prv(td_u32 msg_id, td_u32 src_mod, td_u32 dest_mod);

/**
 * @ingroup  hct_diag
 * @brief  Reports simple packet that with one parameters.
 *
 * @par Description:
 *         Reports simple packet that with one parameters.
 *
 * @attention None
 * @param  msg_id    [IN]  Type #td_u32 The message id of the print log.
 * @param  src_mod   [IN]  Type #td_u32 The source module id of the print log.
 * @param  dest_mod  [IN]  Type #td_u32 The destination module of the print log.
 * @param  d0        [IN]  Type #td_u32 The print data.
 *
 * @retval #0               Success.
 * @retval #Other values    Failure. For details, see soc_errno.h.
 * @par Dependency:
 *           @li soc_diag.h: Describes DIAG APIs.
 * @see  None
 */
td_u32 diag_log_msg1_prv(td_u32 msg_id, td_u32 src_mod, td_u32 dest_mod, td_u32 d0);

/**
 * @ingroup  hct_diag
 * @brief  Reports simple packet that with two parameters.
 *
 * @par Description:
 *         Reports simple packet that with two parameters.
 *
 * @attention None
 * @param  msg_id    [IN]  Type #td_u32 The message id of the print log.
 * @param  src_mod   [IN]  Type #td_u32 The source module id of the print log.
 * @param  dest_mod  [IN]  Type #td_u32 The destination module of the print log.
 * @param  d0        [IN]  Type #td_u32 The print data.
 * @param  d1        [IN]  Type #td_u32 The print data.
 *
 * @retval #0               Success.
 * @retval #Other values    Failure. For details, see soc_errno.h.
 * @par Dependency:
 *           @li soc_diag.h: Describes DIAG APIs.
 * @see  None
 */
td_u32 diag_log_msg2_prv(td_u32 msg_id, td_u32 src_mod, td_u32 dest_mod, td_u32 d0, td_u32 d1);

/**
 * @ingroup  hct_diag
 * @brief  Reports simple packet that with three parameters.
 *
 * @par Description:
 *         Reports simple packet that with three parameters.
 *
 * @attention None
 * @param  msg_id    [IN]  Type #td_u32 The message id of the print log.
 * @param  src_mod   [IN]  Type #td_u32 The source module id of the print log.
 * @param  dest_mod  [IN]  Type #td_u32 The destination module of the print log.
 * @param  log_msg   [IN]  Type #diag_log_msg3 The print data.
 *
 * @retval #0               Success.
 * @retval #Other values    Failure. For details, see soc_errno.h.
 * @par Dependency:
 *           @li soc_diag.h: Describes DIAG APIs.
 * @see  None
 */
td_u32 diag_log_msg3_prv(td_u32 msg_id, td_u32 src_mod, td_u32 dest_mod, diag_log_msg3 log_msg);

/**
 * @ingroup  hct_diag
 * @brief  Reports simple packet that with four parameters.
 *
 * @par Description:
 *         Reports simple packet that with four parameters.
 *
 * @attention None
 * @param  msg_id    [IN]  Type #td_u32 The message id of the print log.
 * @param  src_mod   [IN]  Type #td_u32 The source module id of the print log.
 * @param  dest_mod  [IN]  Type #td_u32 The destination module of the print log.
 * @param  log_msg   [IN]  Type #diag_log_msg4 The print data.
 *
 * @retval #0               Success.
 * @retval #Other values    Failure. For details, see soc_errno.h.
 * @par Dependency:
 *           @li soc_diag.h: Describes DIAG APIs.
 * @see  None
 */
td_u32 diag_log_msg4_prv(td_u32 msg_id, td_u32 src_mod, td_u32 dest_mod, diag_log_msg4 log_msg);

/**
 * @ingroup  hct_diag
 * @brief  Reports simple packet that with buffer.
 *
 * @par Description:
 *         Reports simple packet that with buffer.
 *
 * @attention None
 * @param  msg_id    [IN]  Type #td_u32 The message id of the print log.
 * @param  src_mod   [IN]  Type #td_u32 The source module id of the print log.
 * @param  dest_mod  [IN]  Type #td_u32 The destination module of the print log.
 * @param  buffer    [IN]  Type #td_pvoid The print buffer.
 * @param  size      [IN]  Type #td_u16 The buffer size.
 *
 * @retval #0               Success.
 * @retval #Other values    Failure. For details, see soc_errno.h.
 * @par Dependency:
 *           @li soc_diag.h: Describes DIAG APIs.
 * @see  None
 */
td_u32 diag_log_msg_buffer_prv(td_u32 msg_id, td_u32 src_mod, td_u32 dest_mod, td_pvoid buffer, td_u16 size);

/* 临时调试PLC APP用，待后续平台提供正式接口后删除 */
td_void dms_regist_app_uart_rx_prv(td_pvoid fun1, td_pvoid fun2);
/* 临时调试PLC APP用，待后续平台提供正式接口后删除 */
td_void dms_regist_app_uart_cb_prv(td_pvoid fun);

#ifdef __cplusplus
}
#endif
#endif /* __SOC_DIAG__H__ */