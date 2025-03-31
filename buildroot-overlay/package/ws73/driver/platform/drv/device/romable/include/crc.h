/*
 * Copyright (c) CompanyNameMagicTag 2021-2021. All rights reserved.
 * Description: CRC16 table
 * Date: 2021-04-13
 */

#ifndef __CRC_H__
#define __CRC_H__

#include <td_base.h>

td_u16 uapi_crc16(td_u16 crc_start, TD_CONST td_u8 *buf, td_u32 len);

#endif /* _CRC_H_ */

