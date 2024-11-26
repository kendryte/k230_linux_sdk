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

#define I2C_SLAVE_ADDRESS 0x36
#define CHECK_ERROR(x) if(x)return -1

#define OV5647_REG_CHIP_ID_H                                0x300a
#define OV5647_REG_CHIP_ID_L                                0x300b
#define OV5647_REG_MIPI_CTRL00                              0x4800
#define OV5647_REG_FRAME_OFF_NUMBER                         0x4202
#define OV5647_REG_PAD_OUT                                  0x300d

#define OV5647_REG_VTS_H                                  0x380e
#define OV5647_REG_VTS_L                                  0x380f

#define OV5647_REG_MIPI_CTRL14                              0x4814

#define OV5647_SW_STANDBY                                   0x0100


#define OV5647_REG_LONG_AGAIN_H                             0x350a
#define OV5647_REG_LONG_AGAIN_L                             0x350b

#define OV5647_REG_LONG_EXP_TIME_H                          0x3501
#define OV5647_REG_LONG_EXP_TIME_L                          0x3502

#define OV5647_MIN_GAIN_STEP                                (1.0f/16.0f)
#define OV5647_SW_RESET                                           0x0103
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

struct ov5647_mode {
    struct vvcam_sensor_mode mode;
    struct reg_list* regs;
};

struct ov5647_ctx {
    int i2c;
    struct vvcam_sensor_mode mode;      // fora 3a current val
    uint32_t sensor_again;
    uint32_t et_line;
};

static int read_reg(struct ov5647_ctx* ctx, uint16_t addr, uint8_t* value) {
    struct i2c_msg msg[2];
    struct i2c_rdwr_ioctl_data data;

    addr = htobe16(addr);
    msg[0].addr = I2C_SLAVE_ADDRESS;
    msg[0].buf = (uint8_t*)&addr;
    msg[0].len = 2;
    msg[0].flags = 0;

    msg[1].addr = I2C_SLAVE_ADDRESS;
    msg[1].buf = value;
    msg[1].len = 1;
    msg[1].flags = I2C_M_RD;

    data.msgs = msg;
    data.nmsgs = 2;

    int ret = ioctl(ctx->i2c, I2C_RDWR, &data);
    if (ret != 2) {
        fprintf(stderr, "ov5647: i2c read reg %04x error %d(%s)\n", be16toh(addr), errno, strerror(errno));
        return -1;
    }

    return 0;
}

static int write_reg(struct ov5647_ctx* ctx, uint16_t addr, uint8_t value) {
    uint8_t buffer[3];

    buffer[0] = (addr >> 8) & 0xff;
    buffer[1] = (addr >> 0) & 0xff;
    buffer[2] = value;
    if (write(ctx->i2c, buffer, 3) != 3) {
        fprintf(stderr, "ov5647: i2c write reg %04x error %d(%s)\n", addr, errno, strerror(errno));
        return -1;
    }
    // printf("ov5647 w %04x %02x\n", addr, value);
    return 0;
}

static int open_i2c(struct ov5647_ctx* sensor) {
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
    struct ov5647_ctx* sensor = calloc(1, sizeof(struct ov5647_ctx));
    sensor->i2c = -1;
    *ctx = sensor;
    return 0;
}

static void deinit(void* ctx) {
    struct ov5647_ctx* sensor = ctx;
    close(sensor->i2c);
    free(ctx);
}



static struct reg_list ov5647_1920x1080_30fps[] = {
    //pixel_rate = 81666700
    {0x0103, 0x01},
    {0x0100, 0x00},
    {0x3034, 0x1a},
    {0x3035, 0x21},
    {0x3036, 0x62},
    {0x303c, 0x11},
    {0x3106, 0xf5},
    {0x3821, 0x02},
    {0x3820, 0x00},
    {0x3827, 0xec},
    {0x370c, 0x03},
    {0x3612, 0x5b},
    {0x3618, 0x04},
    {0x5000, 0x06},
    {0x5001, 0x00},         // set awb disble
    {0x5002, 0x00},//41
    {0x5003, 0x08},
    {0x5a00, 0x08},
    {0x3000, 0x00},
    {0x3001, 0x00},
    {0x3002, 0x00},
    {0x3016, 0x08},
    {0x3017, 0xe0},
    {0x3018, 0x44},
    {0x301c, 0xf8},
    {0x301d, 0xf0},
    {0x3a18, 0x03},//00
    {0x3a19, 0xff},//f8
    {0x3c01, 0x80},
    {0x3b07, 0x0c},
    {0x380c, 0x09},         // h totle  high
    {0x380d, 0x70},         // h totle low
    {0x380e, 0x04},         // v totle high
    {0x380f, 0x66},         // v totle lo 50   0x66
    {0x3814, 0x11},
    {0x3815, 0x11},
    {0x3708, 0x64},
    {0x3709, 0x12},
    {0x3808, 0x07},
    {0x3809, 0x80},
    {0x380a, 0x04},
    {0x380b, 0x38},
    {0x3800, 0x01},
    {0x3801, 0x5c},
    {0x3802, 0x01},
    {0x3803, 0xb2},
    {0x3804, 0x08},
    {0x3805, 0xe3},
    {0x3806, 0x05},
    {0x3807, 0xf1},
    {0x3811, 0x04},
    {0x3813, 0x02},
    {0x3630, 0x2e},
    {0x3632, 0xe2},
    {0x3633, 0x23},
    {0x3634, 0x44},
    {0x3636, 0x06},
    {0x3620, 0x64},
    {0x3621, 0xe0},
    {0x3600, 0x37},
    {0x3704, 0xa0},
    {0x3703, 0x5a},
    {0x3715, 0x78},
    {0x3717, 0x01},
    {0x3731, 0x02},
    {0x370b, 0x60},
    {0x3705, 0x1a},
    {0x3f05, 0x02},
    {0x3f06, 0x10},
    {0x3f01, 0x0a},
    {0x3a00, 0x00},
    {0x3a08, 0x01},
    {0x3a09, 0x4b},
    {0x3a0a, 0x01},
    {0x3a0b, 0x13},
    {0x3a0d, 0x04},
    {0x3a0e, 0x03},
    {0x3a0f, 0x58},
    {0x3a10, 0x50},
    {0x3a1b, 0x58},
    {0x3a1e, 0x50},
    {0x3a11, 0x60},
    {0x3a1f, 0x28},
    {0x4001, 0x02},
    {0x4004, 0x04},
    {0x4000, 0x09},
    {0x4837, 0x19},
    {0x4800, 0x34},
    {0x3503, 0x07},         //0x0f
    {0x350b, 0x10},
    {0x3500, 0x3},
    {0x350a, 0x00},
    {0x350b, 0x30},
    {0, 0}
};


static struct ov5647_mode modes[] = {
    {
        .mode = {
            .clk = 25000000,
            .width = 1920,
            .height = 1080,
            .lanes = VVCAM_SENSOR_2LANE,
            .freq = VVCAM_SENSOR_800M,
            .bayer = VVCAM_BAYER_PAT_GBRG,
            .bit_width = 10,
            .ae_info = {
                .frame_length = 1199,
                .cur_frame_length = 1199,
                .one_line_exp_time = 0.000027808,
                .gain_accuracy = 1024,
                .min_gain = 1.0,
                .max_gain = 8.0,
                .int_time_delay_frame = 2,
                .gain_delay_frame = 2,
                .color_type = 0,
                .integration_time_increment = 0.000027808,
                .gain_increment = (1.0f/16.0f),
                .max_long_integraion_line = 1199 - 12,
                .min_long_integraion_line = 2,
                .max_integraion_line = 1199 - 12,
                .min_integraion_line = 2,
                .max_long_integraion_time = 0.000027808 * (1199 - 12),
                .min_long_integraion_time = 0.000027808 * 2,
                .max_integraion_time = 0.000027808 * (1199 - 12),
                .min_integraion_time = 0.000027808 * 2,
                .cur_long_integration_time = 0.0,
                .cur_integration_time = 0.0,
                .cur_long_again = 0.0,
                .cur_long_dgain = 0.0,
                .cur_again = 0.0,
                .cur_dgain = 0.0,
                .a_long_gain.min = 1.0,
                .a_long_gain.max = 8.0,
                .a_long_gain.step = (1.0f/16.0f),
                .a_gain.min = 1.0,
                .a_gain.max = 8.0,
                .a_gain.step = (1.0f/16.0f),
                .d_long_gain.max = 1.0,
                .d_long_gain.min = 1.0,
                .d_long_gain.step = (1.0f/1024.0f),
                .d_gain.max = 1.0,
                .d_gain.min = 1.0,
                .d_gain.step = (1.0f/1024.0f),
                .cur_fps = 30,
            }
        },
        .regs = ov5647_1920x1080_30fps
    }
};
static unsigned modes_len = sizeof(modes) / sizeof(struct ov5647_mode);

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
    struct ov5647_ctx* sensor = ctx;
    // printf("ov5647 %s   \n", __func__);

    memcpy(mode, &sensor->mode, sizeof(struct vvcam_sensor_mode));

    return 0;
}

static int set_mode(void* ctx, uint32_t index) {
    struct ov5647_ctx* sensor = ctx;
    if (index > modes_len) {
        // out of range
        return -1;
    }
    struct vvcam_sensor_mode* mode = &modes[index].mode;

    printf("ov5647: %s %ux%u\n", __func__, mode->width, mode->height);
    if (open_i2c(sensor)) {
        return -1;
    }
    uint8_t channel_id;
    CHECK_ERROR(read_reg(ctx, OV5647_REG_MIPI_CTRL14, &channel_id));
    channel_id &= ~(3 << 6);
    CHECK_ERROR(write_reg(ctx, OV5647_REG_MIPI_CTRL14, channel_id));
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

    CHECK_ERROR(read_reg(ctx, OV5647_REG_LONG_AGAIN_H, &again_h));
    CHECK_ERROR(read_reg(ctx, OV5647_REG_LONG_AGAIN_L, &again_l));
    again = (float)(((again_h & 0x03) << 8) + again_l) / 16.0f;

    sensor->sensor_again = (again * 16 + 0.5);

    dgain = 1.0;
    mode->ae_info.cur_gain = again * dgain;
    mode->ae_info.cur_long_gain = mode->ae_info.cur_gain;
    mode->ae_info.cur_vs_gain = mode->ae_info.cur_gain;

    CHECK_ERROR(read_reg(ctx, OV5647_REG_LONG_EXP_TIME_H, &exp_time_h));
    CHECK_ERROR(read_reg(ctx, OV5647_REG_LONG_EXP_TIME_L, &exp_time_l));
    exp_time = (exp_time_h << 4) + ((exp_time_l >> 4) & 0x0F);

    mode->ae_info.cur_integration_time = exp_time * mode->ae_info.one_line_exp_time;

    printf("mode->ae_info.cur_integration_time is %f \n", mode->ae_info.cur_integration_time);

    // save current mode
    memcpy(&sensor->mode , mode, sizeof(struct vvcam_sensor_mode));

    return 0;
}

static int set_stream(void* ctx, bool on) {
    struct ov5647_ctx* sensor = ctx;
    printf("ov5647 %s %d\n", __func__, on);
    if (open_i2c(sensor)) {
        return -1;
    }

    // while(1)
    // {
    //     sleep(1);
    // }
    if (on) {
        if (write_reg(sensor, 0x0100, 1)) {
            printf("ov5647 write reg error");
            return -1;
        }
    } else {
        if (write_reg(sensor, 0x3018, 0xff) || write_reg(sensor, 0x0100, 0)) {
            printf("ov5647 write reg error");
            return -1;
        }
    }
    return 0;
}

static int set_analog_gain(void* ctx, float gain) {
    struct ov5647_ctx* sensor = ctx;
    uint32_t again;

    // printf("ov5647 %s %f\n", __func__, gain);

    again = (uint32_t)(gain * 16 + 0.5);

    if(sensor->sensor_again !=again)
    {
        CHECK_ERROR(write_reg(ctx, OV5647_REG_LONG_AGAIN_H, (again & 0x0300)>>8));
        CHECK_ERROR(write_reg(ctx, OV5647_REG_LONG_AGAIN_L, (again & 0xff)));
        sensor->sensor_again = again;
    }

    sensor->mode.ae_info.cur_again = (float)sensor->sensor_again/16.0f;

    return 0;
}

static int set_digital_gain(void* ctx, float gain) {
    // printf("ov5647 %s %f\n", __func__, gain);
    return 0;
}

static int set_int_time(void* ctx, float time) {
    struct ov5647_ctx* sensor = ctx;
    uint16_t exp_line = 0;
    float integraion_time = 0;

    // printf("ov5647 %s %f\n", __func__, time);

    integraion_time = time;

    exp_line = integraion_time / sensor->mode.ae_info.one_line_exp_time;
    exp_line = MIN(sensor->mode.ae_info.max_integraion_line, MAX(sensor->mode.ae_info.min_integraion_line, exp_line));

    if (sensor->et_line != exp_line)
    {
        CHECK_ERROR(write_reg(ctx, OV5647_REG_LONG_EXP_TIME_H, ( exp_line >> 4) & 0xff));
        CHECK_ERROR(write_reg(ctx, OV5647_REG_LONG_EXP_TIME_L, ( exp_line << 4) & 0xff));

        sensor->et_line = exp_line;
    }
    sensor->mode.ae_info.cur_integration_time = (float)sensor->et_line * sensor->mode.ae_info.one_line_exp_time;

    return 0;
}

struct vvcam_sensor vvcam_ov5647 = {
    .name = "ov5647",
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
