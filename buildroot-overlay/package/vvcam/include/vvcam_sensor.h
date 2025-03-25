#pragma once
#ifndef __VVCAM_SENSOR_H__
#define __VVCAM_SENSOR_H__

#define VVCAM_API_VERSION 1UL

#include <stdbool.h>
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

struct vvcam_fps_range {
    uint32_t min;
    uint32_t max;
};

struct vvcam_auto_fps {
    struct vvcam_fps_range afps_range;
    uint32_t max_gain;
};

struct vvcam_gain_info {
    float min;
    float max;
    float step;
};

struct vvcam_ae_info {
    uint16_t frame_length;
    uint16_t cur_frame_length;

    float one_line_exp_time;

    uint32_t gain_accuracy;

    float min_gain;
    float max_gain;

    float integration_time_increment;
    float gain_increment;

    uint16_t max_long_integraion_line;
    uint16_t min_long_integraion_line;

    uint16_t max_integraion_line;
    uint16_t min_integraion_line;

    uint16_t max_vs_integraion_line;
    uint16_t min_vs_integraion_line;

    float max_long_integraion_time;
    float min_long_integraion_time;

    float max_integraion_time;
    float min_integraion_time;

    float max_vs_integraion_time;
    float min_vs_integraion_time;


    float cur_long_integration_time;
    float cur_integration_time;
    float cur_vs_integration_time;

    float cur_long_gain;
    float cur_long_again;
    float cur_long_dgain;

    float cur_gain;
    float cur_again;
    float cur_dgain;

    float cur_vs_gain;
    float cur_vs_again;
    float cur_vs_dgain;


    struct vvcam_gain_info long_gain;
    struct vvcam_gain_info gain;
    struct vvcam_gain_info vs_gain;

    struct vvcam_gain_info a_long_gain;
    struct vvcam_gain_info a_gain;
    struct vvcam_gain_info   a_vs_gain;

    struct vvcam_gain_info d_long_gain;
    struct vvcam_gain_info d_gain;
    struct vvcam_gain_info d_vs_gain;

    uint32_t max_fps;
    uint32_t min_fps;
    uint32_t cur_fps;
    struct vvcam_auto_fps afps_info;
    uint32_t hdr_ratio;

    uint32_t int_time_delay_frame;
    uint32_t gain_delay_frame;

    uint8_t color_type;		//0, color image; 1, mono sensor image; 2, color sensor gray image.
};

struct vvcam_sensor_mode {
    uint16_t width;
    uint16_t height;
    uint32_t clk;
    uint32_t bit_width;
    enum vvcam_sensor_bayer bayer;
    enum vvcam_sensor_freq freq;
    enum vvcam_sensor_lanes lanes;
    struct vvcam_ae_info ae_info;
};

struct vvcam_sensor_ctrl {
    int (*init)(void** ctx);
    void (*deinit)(void* ctx);
    int (*enum_mode)(void* ctx, uint32_t index, struct vvcam_sensor_mode* mode);
    int (*get_mode)(void* ctx, struct vvcam_sensor_mode* mode);
    int (*set_mode)(void* ctx, uint32_t index);
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
void vvcam_sensor_init(void);

#endif