/*
 * Copyright (c) CompanyNameMagicTag 2021-2021. All rights reserved.
 * Description: CRC16 table
 * Date: 2021-04-13
 */

#ifndef _CRC_H_
#define _CRC_H_

#include <td_base.h>

td_u16 crc16_ccitt(td_u16 crc_start, TD_CONST td_u8 *s, td_u32 len);
td_u32 crc32(td_u32 crc, TD_CONST td_u8 *p, td_u32 len);
td_u32 crc32_no_comp(td_u32 crc, TD_CONST td_u8 *buf, td_u32 len);

#endif /* _CRC_H_ */

