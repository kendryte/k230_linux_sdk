/*
 * Copyright (c) CompanyNameMagicTag 2020-2021. All rights reserved.
 * Description: crc interfaces.
 * Date: 2021-04-13
 */

#include <soc_crc.h>
#include "crc.h"

td_u16 uapi_crc16(td_u16 crc_start, TD_CONST td_u8 *buf, td_u32 len)
{
    return crc16_ccitt(crc_start, buf, len);
}

td_u32 uapi_crc32(td_u32 crc_start, TD_CONST td_u8 *buf, td_u32 len)
{
    return crc32(crc_start, buf, len);
}

td_u32 uapi_crc32_no_comp(td_u32 crc_start, TD_CONST td_u8 *buf, td_u32 len)
{
    return crc32_no_comp(crc_start, buf, len);
}
