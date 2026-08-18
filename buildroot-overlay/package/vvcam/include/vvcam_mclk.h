#pragma once
#ifndef __VVCAM_MCLK_H__
#define __VVCAM_MCLK_H__

#include <stdbool.h>
#include <stdint.h>

enum vvcam_mclk_id {
    VVCAM_MCLK0 = 0,
    VVCAM_MCLK1 = 1,
    VVCAM_MCLK2 = 2,
};

/* Same values as RTOS k_sensor_mclk_sel / k_vicap_mclk_sel. */
enum vvcam_mclk_sel {
    VVCAM_PLL0_CLK_DIV4 = 5,
    VVCAM_PLL1_CLK_DIV3 = 8,
    VVCAM_PLL1_CLK_DIV4 = 9,
};

struct vvcam_mclk_setting {
    bool enable;
    uint8_t sel;
    uint8_t div;
};

/*
 * HW parent after mapping k_sensor_mclk_sel → 2-bit sel:
 *   VVCAM_PLL1_CLK_DIV4 → 594 MHz  (div=25 → 23.76 MHz, div=8 → 74.25 MHz)
 *   VVCAM_PLL1_CLK_DIV3 → 792 MHz
 *   VVCAM_PLL0_CLK_DIV4 → 400 MHz  (div=16 → 25 MHz)
 * Sensor drivers set .sel / .div per mode (same as RTOS mclk_setting).
 */

int vvcam_mclk_apply(uint8_t mclk_id, const struct vvcam_mclk_setting *setting);
int vvcam_mclk_disable(uint8_t mclk_id);

#endif
