/*
 * Copyright (c) CompanyNameMagicTag 2020-2021. All rights reserved.
 * Description: ws73 memory macro definition.
 * Author: CompanyName
 * Create: 2021-09-15
 */
#ifndef __MEMORY_CONFIG_H__
#define __MEMORY_CONFIG_H__

#define CPU_ITCM_START                          (0x100000)
#define CPU_ITCM_LENGTH                         (432 * 1024)

#define WOW_RAM_START                           (0x650000)
#define WOW_RAM_LENGTH                          (32 * 1024)

/*****************************************************************************/

#define BOOTROM_ROM_CODE                        CPU_ITCM_START
#define BOOTROM_ROM_CODE_LEN                    (25 * 1024)

#define FW_ROM_CODE                             (BOOTROM_ROM_CODE + BOOTROM_ROM_CODE_LEN)
#define FW_ROM_CODE_LEN                         (412 * 1024)

#define FW_PLAT_ROM_CODE                        (BOOTROM_ROM_CODE + BOOTROM_ROM_CODE_LEN)
#define FW_PLAT_ROM_CODE_LEN                    (56 *1024)
#define FW_WIFI_ROM_CODE                        (FW_PLAT_ROM_CODE + FW_PLAT_ROM_CODE_LEN)
#define FW_WIFI_ROM_CODE_LEN                    (180 * 1024)
#define FW_BGLE_ROM_CODE                        (FW_WIFI_ROM_CODE + FW_WIFI_ROM_CODE_LEN)
#define FW_BGLE_ROM_CODE_LEN                    (184 * 1024)

#if ((BOOTROM_ROM_CODE + BOOTROM_ROM_CODE_LEN) > FW_ROM_CODE)
#error FW_ROM_CODE memory region is overlap !!!
#endif

/*****************************************************************************/

#define MEMORY_PATCH_TABLE                      (0x400000)
#define MEMORY_PATCH_TABLE_LEN                  (2 * 1024)

#define MEMORY_ROM_RAM_CB                       (MEMORY_PATCH_TABLE + MEMORY_PATCH_TABLE_LEN)
#define MEMORY_ROM_RAM_CB_LEN                   (2 * 1024)

#define FW_PLAT_ROM_DATA                        (MEMORY_ROM_RAM_CB + MEMORY_ROM_RAM_CB_LEN)
#define FW_PLAT_ROM_DATA_LEN                    (1 * 1024)
#define FW_WIFI_ROM_DATA                        (FW_PLAT_ROM_DATA + FW_PLAT_ROM_DATA_LEN)
#define FW_WIFI_ROM_DATA_LEN                    (8 * 1024)
#define FW_BGLE_ROM_DATA                        (FW_WIFI_ROM_DATA + FW_WIFI_ROM_DATA_LEN)
#define FW_BGLE_ROM_DATA_LEN                    (8 * 1024)

#if ((MEMORY_ROM_RAM_CB + MEMORY_ROM_RAM_CB_LEN) > FW_PLAT_ROM_DATA)
#error FW_PLAT_ROM_DATA memory region is overlap !!!
#endif

#define FW_PLAT_ROM_BSS                         (FW_BGLE_ROM_DATA + FW_BGLE_ROM_DATA_LEN)
#define FW_PLAT_ROM_BSS_LEN                     (12 * 1024)
#define FW_WIFI_ROM_BSS                         (FW_PLAT_ROM_BSS + FW_PLAT_ROM_BSS_LEN)
#define FW_WIFI_ROM_BSS_LEN                     (32 * 1024)
#define FW_BGLE_ROM_BSS                         (FW_WIFI_ROM_BSS + FW_WIFI_ROM_BSS_LEN)
#define FW_BGLE_ROM_BSS_LEN                     (6 * 1024)

#define MEMORY_DFR_STACK_FRAME                  (FW_BGLE_ROM_BSS + FW_BGLE_ROM_BSS_LEN)
#define MEMORY_DFR_STACK_FRAME_LEN              (1 * 1024)

/*****************************************************************************/

#define FW_PLAT_CODE                            (MEMORY_DFR_STACK_FRAME + MEMORY_DFR_STACK_FRAME_LEN)
#define FW_PLAT_CODE_LEN                        (4 * 1024)
#define FW_WIFI_CODE                            (FW_PLAT_CODE + FW_PLAT_CODE_LEN)
#define FW_WIFI_CODE_LEN                        (12 * 1024)
#define FW_BGLE_CODE                            (FW_WIFI_CODE + FW_WIFI_CODE_LEN)
#define FW_BGLE_CODE_LEN                        (4 * 1024)

#if ((MEMORY_DFR_STACK_FRAME + MEMORY_DFR_STACK_FRAME_LEN) > FW_PLAT_CODE)
#error FW_PLAT_CODE memory region is overlap !!!
#endif

#define FW_PLAT_DATA                            (FW_BGLE_CODE + FW_BGLE_CODE_LEN)
#define FW_PLAT_DATA_LEN                        (1 * 1024)
#define FW_WIFI_DATA                            (FW_PLAT_DATA + FW_PLAT_DATA_LEN)
#define FW_WIFI_DATA_LEN                        (1 * 1024)
#define FW_BGLE_DATA                            (FW_WIFI_DATA + FW_WIFI_DATA_LEN)
#define FW_BGLE_DATA_LEN                        (1 * 1024)

#define FW_PLAT_BSS                             (FW_BGLE_DATA + FW_BGLE_DATA_LEN)
#define FW_PLAT_BSS_LEN                         (4 * 1024)
#define FW_WIFI_BSS                             (FW_PLAT_BSS + FW_PLAT_BSS_LEN)
#define FW_WIFI_BSS_LEN                         (6 * 1024)
#define FW_BGLE_BSS                             (FW_WIFI_BSS + FW_WIFI_BSS_LEN)
#define FW_BGLE_BSS_LEN                         (1 * 1024)

#define MEMORY_PATCH_DATA                       (FW_BGLE_BSS + FW_BGLE_BSS_LEN)
#define MEMORY_PATCH_DATA_LEN                   (1 * 1024)

/*****************************************************************************/

#define MEMORY_WOW                              (0x650000)
#define MEMORY_WOW_LEN                          (32 * 1024)

#endif
