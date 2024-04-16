#pragma once
#include <stdbool.h>
#ifndef __VVCAM_SENSOR_H__
#define __VVCAM_SENSOR_H__

#include <stdint.h>

enum vvcam_sensor_freq {
    VVCAM_SENSOR_800M,
    VVCAM_SENSOR_1200M,
    VVCAM_SENSOR_1600M,
};

enum vvcam_sensor_lanes {
    VVCAM_SENSOR_1LANE,
    VVCAM_SENSOR_2LANE,
    VVCAM_SENSOR_4LANE,
};

enum vvcam_sensor_bayer {
    VVCAM_BAYER_PAT_RGGB      = 0x00,
    VVCAM_BAYER_PAT_GRBG      = 0x01,
    VVCAM_BAYER_PAT_GBRG      = 0x02,
    VVCAM_BAYER_PAT_BGGR      = 0x03,
    VVCAM_BAYER_PAT_BGGIR     = 0x10,
    VVCAM_BAYER_PAT_GRIRG     = 0x11,
    VVCAM_BAYER_PAT_RGGIR     = 0x12,
    VVCAM_BAYER_PAT_GBIRG     = 0x13,
    VVCAM_BAYER_PAT_GIRRG     = 0x14,
    VVCAM_BAYER_PAT_IRGGB     = 0x15,
    VVCAM_BAYER_PAT_GIRBG     = 0x16,
    VVCAM_BAYER_PAT_IRGGR     = 0x17,
    VVCAM_BAYER_PAT_RGIRB     = 0x18,
    VVCAM_BAYER_PAT_GRBIR     = 0x19,
    VVCAM_BAYER_PAT_IRBRG     = 0x20,
    VVCAM_BAYER_PAT_BIRGR     = 0x21,
    VVCAM_BAYER_PAT_BGIRR     = 0x22,
    VVCAM_BAYER_PAT_GBRIR     = 0x23,
    VVCAM_BAYER_PAT_IRRBG     = 0x24,
    VVCAM_BAYER_PAT_RIRGB     = 0x25,
    VVCAM_BAYER_PAT_RCCC      = 0x30,
    VVCAM_BAYER_PAT_RCCB      = 0x40,
    VVCAM_BAYER_PAT_RYYCY     = 0x50,
};

struct vvcam_sensor_mode {
    uint16_t width;
    uint16_t height;
    uint32_t clk;
    uint32_t bit_width;
    enum vvcam_sensor_bayer bayer;
    enum vvcam_sensor_freq freq;
    enum vvcam_sensor_lanes lanes;
};

struct vvcam_sensor_ctrl {
    int (*init)(void** ctx);
    void (*deinit)(void* ctx);
    int (*enum_mode)(void* ctx, uint32_t index, struct vvcam_sensor_mode* mode);
    int (*get_mode)(void* ctx, struct vvcam_sensor_mode* mode);
    int (*set_mode)(void* ctx, struct vvcam_sensor_mode* mode);
    int (*set_stream)(void* ctx, bool on);
    int (*set_analog_gain)(void* ctx, float gain);
    int (*set_digital_gain)(void* ctx, float gain);
    int (*set_int_time)(void* ctx, float time);
};

struct vvcam_sensor {
    const char* name;
    struct vvcam_sensor_ctrl ctrl;
};

void vvcam_sensor_add(struct vvcam_sensor* sensor);

#endif