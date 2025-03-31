/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: common_log_dbg_rom头文件
 * Date: 2022-03-08
 */

#ifndef __COMMON_LOG_DBG_ROM_H__
#define __COMMON_LOG_DBG_ROM_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define common_log_dbg0(vap_id, oam_sf, fmt)             \
    do {                                      \
        oam_warning_log0(vap_id, oam_sf, fmt); \
    } while (0)

#define common_log_dbg1(vap_id, oam_sf, fmt, p1)             \
    do {                                      \
        oam_warning_log1(vap_id, oam_sf, fmt, p1); \
    } while (0)

#define common_log_dbg2(vap_id, oam_sf, fmt, p1, p2)             \
    do {                                      \
        oam_warning_log2(vap_id, oam_sf, fmt, p1, p2); \
    } while (0)

#define common_log_dbg3(vap_id, oam_sf, fmt, p1, p2, p3)             \
    do {                                      \
        oam_warning_log3(vap_id, oam_sf, fmt, p1, p2, p3); \
    } while (0)

#define common_log_dbg4(vap_id, oam_sf, fmt, p1, p2, p3, p4)             \
    do {                                      \
        oam_warning_log4(vap_id, oam_sf, fmt, p1, p2, p3, p4); \
    } while (0)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of common_log_dbg_rom.h */
