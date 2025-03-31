/**
 * Copyright (c) @CompanyNameMagicTag 2024. All rights reserved.
 *
 * Description: SLE Glp Manager module.
 */

/**
 * @defgroup sle_glp_manager glp manager API
 * @ingroup  SLE
 * @{
 */

#ifndef SLE_GLP_MANAGER_H
#define SLE_GLP_MANAGER_H

#include <stdint.h>
#include "errcode.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint8_t status;
    uint16_t con_hdl;
    int32_t  cfo;
} sle_cs_glp_report_t;

typedef void (*sle_cs_glp_report_callback)(uint16_t conn_id,
    sle_cs_glp_report_t *report);

typedef struct {
    sle_cs_glp_report_callback cs_glp_report_cb;
} sle_cs_glp_callbacks_t;

errcode_t sle_glp_register_callbacks(sle_cs_glp_callbacks_t *func);

#ifdef __cplusplus
}
#endif
#endif /* SLE_GLP_MANAGER_H */
/**
 * @}
 */
