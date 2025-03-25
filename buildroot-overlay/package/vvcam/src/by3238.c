#include "common.h"
#include <endian.h>
#include <string.h>
#include <vvcam_sensor.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>

#define I2C_SLAVE_ADDRESS 0x6e

#define CHECK_ERROR(x) if(x)return -1

#define BY3238_REG_CHIP_ID_H                                0xf0
#define BY3238_REG_CHIP_ID_L                                0xf1

#define BY3238_REG_MIPI_CTRL00                              0x4800
#define BY3238_REG_FRAME_OFF_NUMBER                         0x4202
#define BY3238_REG_PAD_OUT                                  0x300d

#define BY3238_REG_VTS_H                                    0x380e
#define BY3238_REG_VTS_L                                    0x380f

#define BY3238_REG_MIPI_CTRL14                              0x4814

#define BY3238_SW_STANDBY                                   0x0100


#define BY3238_REG_LONG_AGAIN_H                             0x0001
#define BY3238_REG_LONG_AGAIN_L                             0x0002

#define BY3238_REG_DGAIN_H	                                0xb1	//0x00b8
#define BY3238_REG_DGAIN_L	                                0xb2	//0x00b9

#define BY3238_REG_LONG_EXP_TIME_H                          0x03
#define BY3238_REG_LONG_EXP_TIME_L                          0x04

#define BY3238_MIN_GAIN_STEP                                (1.0f/16.0f)
#define BY3238_SW_RESET                                     0x0103
#define MIPI_CTRL00_CLOCK_LANE_GATE                         (1 << 5)
#define MIPI_CTRL00_LINE_SYNC_ENABLE                        (1 << 4)
#define MIPI_CTRL00_BUS_IDLE                                (1 << 1)
#define MIPI_CTRL00_CLOCK_LANE_DISABLE                      (1 << 0)


#ifndef MIN
#define MIN(a, b)   ( ((a)<(b)) ? (a) : (b) )
#endif /* MIN */

#ifndef MAX
#define MAX(a, b)   ( ((a)>(b)) ? (a) : (b) )
#endif /* MAX */


struct reg_list {
    uint16_t addr;
    uint8_t value;
};

struct bf3238_mode {
    struct vvcam_sensor_mode mode;
    struct reg_list* regs;
};

struct bf3238_ctx {
    int i2c;
    struct vvcam_sensor_mode mode;      // fora 3a current val
    uint32_t sensor_again;
    uint32_t et_line;
};

static int read_reg(struct bf3238_ctx* ctx, uint16_t addr, uint8_t* value) {
    struct i2c_msg msg[2];
    struct i2c_rdwr_ioctl_data data;

    addr = htobe16(addr);
    msg[0].addr = I2C_SLAVE_ADDRESS;
    msg[0].buf = (uint8_t*)&addr;
    msg[0].len = 1;
    msg[0].flags = 0;

    msg[1].addr = I2C_SLAVE_ADDRESS;
    msg[1].buf = value;
    msg[1].len = 1;
    msg[1].flags = I2C_M_RD;

    data.msgs = msg;
    data.nmsgs = 2;

    int ret = ioctl(ctx->i2c, I2C_RDWR, &data);
    if (ret != 2) {
        fprintf(stderr, "bf3238: i2c read reg %04x error %d(%s)\n", be16toh(addr), errno, strerror(errno));
        return -1;
    }

    return 0;
}

static int write_reg(struct bf3238_ctx* ctx, uint16_t addr, uint8_t value) {
    uint8_t buffer[3];

    // buffer[0] = (addr >> 8) & 0xff;
    // buffer[1] = (addr >> 0) & 0xff;
    // buffer[2] = value;

    buffer[0] = (addr >> 0) & 0xff;
    buffer[1] = value;

    if (write(ctx->i2c, buffer, 2) != 2) {
        fprintf(stderr, "bf3238: i2c write reg %04x error %d(%s) I2C_SLAVE_ADDRESS is %x \n", addr, errno, strerror(errno), I2C_SLAVE_ADDRESS);
        return -1;
    }
    // printf("bf3238 w %04x %02x\n", addr, value);
    return 0;
}


static int open_i2c(struct bf3238_ctx* sensor) {
    // i2c
    if (sensor->i2c < 0) {
        sensor->i2c = open("/dev/i2c-0", O_RDWR);
        if (sensor->i2c < 0) {
            perror("open /dev/i2c-0");
            return -1;
        }
        if (ioctl(sensor->i2c, I2C_SLAVE_FORCE, I2C_SLAVE_ADDRESS) < 0) {
            perror("i2c ctrl 0x36");
            return -1;
        }
    }
    return 0;
}

static int init(void** ctx) {
    struct bf3238_ctx* sensor = calloc(1, sizeof(struct bf3238_ctx));
    sensor->i2c = -1;
    *ctx = sensor;
    return 0;
}

static void deinit(void* ctx) {
    struct bf3238_ctx* sensor = ctx;
    close(sensor->i2c);
    free(ctx);
}


static struct reg_list bf3238_1920x1080_30fps[] = {
    {0xf2, 0x01},
    {0xf2, 0x00},
    {0xf3, 0x00},
    {0xf3, 0x00},
    {0xf3, 0x00},
    {0x00, 0x41},
    {0x03, 0x20},//10bit

    {0x06, 0x50},//     add vblank(default 0x13 = 19 ) -> 80 = 0x50

    {0x0b, 0x4c},
    {0x0c, 0x04},
    {0x0F, 0x48},
    {0x15, 0x4b},
    {0x16, 0x63},
    {0x19, 0x4e},
    {0x1b, 0x7a},
    {0x1d, 0x7a},
    {0x25, 0x88},
    {0x26, 0x48},
    {0x27, 0x86},
    {0x28, 0x44},
    {0x2a, 0x7c},
    {0x2B, 0x7a},
    {0x2E, 0x06},
    {0x2F, 0x53},
    {0xe0, 0x00},
    {0xe1, 0xef},
    {0xe2, 0x47},
    {0xe3, 0x43},
    {0xe7, 0x2B},
    {0xe8, 0x69},
    {0xe9, 0x8b},//0x0b},
    {0xea, 0xb7},
    {0xeb, 0x04},
    {0xe4, 0x7a},
    {0x7d, 0x0e},  //MIPI
    {0xc9, 0x80}, //1928*1088
    // {0xcd, 0x88},
    // {0xcf, 0x40},

    {0xca ,0x70},       // hwin
    {0xcb ,0x40},       // v win 

    {0xcc ,0x00},       // h start = 0x8c = 12
    {0xce ,0x00},       // v start 

    {0xcd ,0x80},       //  hstop  1920*1080
    {0xcf ,0x38},       // v stop 

    {0x30, 0x01},
    {0x4d, 0x00},

    {0x59, 0x10},
    {0x5a, 0x10},
    {0x5b, 0x10},
    {0x5c, 0x10},
    {0x5e, 0x22},
    {0x6a, 0x1f},
    {0x6b, 0x04},
    {0x6c, 0x20},
    {0x6f, 0x10},
    { 0, 0x00 }
};

static struct bf3238_mode modes[] = {
    {
        .mode = {
            .clk = 24000000,
            .width = 1920,
            .height = 1080,
            .lanes = VVCAM_SENSOR_1LANE,
            .freq = VVCAM_SENSOR_1200M,
            .bayer = VVCAM_BAYER_PAT_BGGR,
            .bit_width = 10,
            .ae_info = {
                .frame_length = 1125,
                .cur_frame_length = 1125,
                .one_line_exp_time = 0.000028,
                .gain_accuracy = 1024,
                .min_gain = 1.0,
                .max_gain = 16.0,
                .int_time_delay_frame = 2,
                .gain_delay_frame = 2,
                .color_type = 0,
                .integration_time_increment = 0.000028,
                .gain_increment = (1.0f/64.0f),
                .max_long_integraion_line = 1125 - 8,
                .min_long_integraion_line = 2,
                .max_integraion_line = 1125 - 8,
                .min_integraion_line = 2,
                .max_long_integraion_time = 0.000028 * (1125 - 8),
                .min_long_integraion_time = 0.000028 * 2,
                .max_integraion_time = 0.000028 * (1125 - 8),
                .min_integraion_time = 0.000028 * 2,
                .cur_long_integration_time = 0.0,
                .cur_integration_time = 0.0,
                .cur_long_again = 0.0,
                .cur_long_dgain = 0.0,
                .cur_again = 0.0,
                .cur_dgain = 0.0,
                .a_gain.min = 1.0,
                .a_gain.max = 15,
                .a_gain.step = (1.0f/128.0f),
                .d_gain.max = 1.0,
                .d_gain.min = 1.0,
                .d_gain.step = (1.0f/1024.0f),
                .cur_fps = 30,
            }
        },
        .regs = bf3238_1920x1080_30fps
    }
};


static unsigned modes_len = sizeof(modes) / sizeof(struct bf3238_mode);

static int enum_mode(void* ctx, uint32_t index, struct vvcam_sensor_mode* mode) {
    if (index == 0) {
        memcpy(mode, &modes[0].mode, sizeof(struct vvcam_sensor_mode));
        return 0;
    } else {
        return -1;
    }
}

static int get_mode(void* ctx, struct vvcam_sensor_mode* mode) {
    // memcpy(mode, &modes[0].mode, sizeof(struct vvcam_sensor_mode));
    struct bf3238_ctx* sensor = ctx;
    // printf("bf3238 %s   \n", __func__);

    memcpy(mode, &sensor->mode, sizeof(struct vvcam_sensor_mode));

    return 0;
}

static int set_mode(void* ctx, uint32_t index) {
    struct bf3238_ctx* sensor = ctx;
    if (index > modes_len) {
        // out of range
        return -1;
    }
    struct vvcam_sensor_mode* mode = &modes[index].mode;

    printf("bf3238: %s %ux%u\n", __func__, mode->width, mode->height);
    if (open_i2c(sensor)) {
        return -1;
    }

    for(unsigned i = 0;; i++) {
        if ((modes[0].regs[i].addr == 0) && (modes[0].regs[i].value == 0)) {
            break;
        }
        CHECK_ERROR(write_reg(sensor, modes[0].regs[i].addr, modes[0].regs[i].value));
    }

    uint8_t again_h, again_l;
    uint8_t exp_time_h, exp_time_l;
    uint8_t exp_time;
    float again = 0, dgain = 0;


    CHECK_ERROR(read_reg(ctx, 0x6a, &again_l));
    again = (float)(again_l>>2)/64.0f + again_h;
    if(again_l <= 0xf)
        again = 1.0;
    else
        again = (float)again_l / 15 ;  // 0xf = 1gain  0x32 - 0xf * /15


    printf("*****************************mode->ae_info.again is %f \n", again);

    sensor->sensor_again = (again * 15 + 0.5);

    again = 1.0;
    dgain = 1.0;
    mode->ae_info.cur_gain = again * dgain;
    mode->ae_info.cur_long_gain = mode->ae_info.cur_gain;
    mode->ae_info.cur_vs_gain = mode->ae_info.cur_gain;

    CHECK_ERROR(read_reg(ctx, 0x6b, &exp_time_h));
    CHECK_ERROR(read_reg(ctx, 0x6c, &exp_time_l));
    exp_time = ((exp_time_h & 0xff) << 8) + exp_time_l;

    mode->ae_info.cur_integration_time = exp_time * mode->ae_info.one_line_exp_time;

    printf("mode->ae_info.cur_integration_time is %f \n", mode->ae_info.cur_integration_time);

    // save current mode
    memcpy(&sensor->mode , mode, sizeof(struct vvcam_sensor_mode));

    return 0;
}


static int set_stream(void* ctx, bool on) {
    struct bf3238_ctx* sensor = ctx;
    printf("bf3238 %s %d\n", __func__, on);
    if (open_i2c(sensor)) {
        return -1;
    }
    if (on) {
        // ret = sensor_reg_list_write(&dev->i2c_info, sensor_enable_regs);
        // for(unsigned i = 0;; i++) {
        //     if ((sensor_enable_regs[i].addr == 0) && (sensor_enable_regs[i].value == 0)) {
        //         break;
        //     }
        //     CHECK_ERROR(write_reg(sensor, sensor_enable_regs[i].addr, sensor_enable_regs[i].value));
        // }
        // write_reg(sensor, 0xfe, 0x00);
        // write_reg(sensor, 0x3e, 0x81);
        // write_reg(sensor, 0x3e, 0x91);
    } else {
        // write_reg(sensor, 0x03fe, 0xf0);
        // write_reg(sensor, 0x03fe, 0xf0);
        write_reg(sensor, 0xf3, 0x01);
    }

    return 0;
}


static int set_analog_gain(void* ctx, float gain) {
    struct bf3238_ctx* sensor = ctx;
    uint32_t again, dgain, total;;
    uint32_t i = 0;
    // printf("bf3238 %s %f\n", __func__, gain);

    again = (uint32_t)(gain * 15 + 0.5);

    if(sensor->sensor_again !=again)
    {
        CHECK_ERROR(write_reg(ctx, 0x6a, again));
        sensor->sensor_again = again;
    }

    sensor->mode.ae_info.cur_gain = (float)sensor->sensor_again/15.0f;

    return 0;
}

static int set_digital_gain(void* ctx, float gain) {
    // printf("bf3238 %s %f\n", __func__, gain);
    return 0;
}

static int set_int_time(void* ctx, float time) {
    struct bf3238_ctx* sensor = ctx;
    uint16_t exp_line = 0;
    float integraion_time = 0;

    // printf("bf3238 %s %f\n", __func__, time);

    integraion_time = time;

    exp_line = integraion_time / sensor->mode.ae_info.one_line_exp_time;
    exp_line = MIN(sensor->mode.ae_info.max_integraion_line, MAX(sensor->mode.ae_info.min_integraion_line, exp_line));

    if (sensor->et_line != exp_line)
    {
        CHECK_ERROR(write_reg(ctx, 0x6b, (exp_line >> 8) & 0xff));
        CHECK_ERROR(write_reg(ctx, 0x6c, (exp_line) & 0xff));

        sensor->et_line = exp_line;
    }
    sensor->mode.ae_info.cur_integration_time = (float)sensor->et_line * sensor->mode.ae_info.one_line_exp_time;

    return 0;
}

struct vvcam_sensor vvcam_bf3238 = {
    .name = "bf3238",
    .ctrl = {
        .init = init,
        .deinit = deinit,
        .enum_mode = enum_mode,
        .get_mode = get_mode,
        .set_mode = set_mode,
        .set_stream = set_stream,
        .set_analog_gain = set_analog_gain,
        .set_digital_gain = set_digital_gain,
        .set_int_time = set_int_time
    }
};
