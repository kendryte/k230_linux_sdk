#include "common.h"
#include <string.h>
#include <vvcam_sensor.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

struct ov5647_ctx {
    int i2c;
};

static int write_reg(struct ov5647_ctx* ctx, uint16_t addr, uint8_t value) {
    uint8_t buffer[3];
    buffer[0] = (addr >> 8) & 0xff;
    buffer[1] = (addr >> 0) & 0xff;
    buffer[2] = value;
    if (write(ctx->i2c, buffer, 3) != 3) {
        return -1;
    }
    printf("ov5647 w %04x %02x\n", addr, value);
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
        if (ioctl(sensor->i2c, I2C_SLAVE_FORCE, 0x36) < 0) {
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

struct reg_list {
    uint16_t addr;
    uint8_t value;
};

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

static struct ov5647_mode {
    struct vvcam_sensor_mode mode;
    struct reg_list* regs;
} modes[] = {
    {
        .mode = {
            .clk = 25000000,
            .width = 1920,
            .height = 1080,
            .lanes = VVCAM_SENSOR_2LANE,
            .freq = VVCAM_SENSOR_800M,
            .bayer = VVCAM_BAYER_PAT_GBRG,
            .bit_width = 10
        },
        .regs = ov5647_1920x1080_30fps
    }
};

static int enum_mode(void* ctx, uint32_t index, struct vvcam_sensor_mode* mode) {
    if (index == 0) {
        memcpy(mode, &modes[0].mode, sizeof(struct vvcam_sensor_mode));
        return 0;
    } else {
        return -1;
    }
}

static int get_mode(void* ctx, struct vvcam_sensor_mode* mode) {
    memcpy(mode, &modes[0], sizeof(struct vvcam_sensor_mode));
    return 0;
}

static int set_mode(void* ctx, struct vvcam_sensor_mode* mode) {
    struct ov5647_ctx* sensor = ctx;
    printf("ov5647: %s %ux%u\n", __func__, mode->width, mode->height);
    if (open_i2c(sensor)) {
        return -1;
    }
    // FIXME: find nearest mode
    for(unsigned i = 0;; i++) {
        if ((modes[0].regs[i].addr == 0) && (modes[0].regs[i].value == 0)) {
            break;
        }
        write_reg(sensor, modes[0].regs[i].addr, modes[0].regs[i].value);
    }
    return 0;
}

static int set_stream(void* ctx, bool on) {
    struct ov5647_ctx* sensor = ctx;
    printf("ov5647 %s %d\n", __func__, on);
    if (open_i2c(sensor)) {
        return -1;
    }
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
    printf("ov5647 %s %f\n", __func__, gain);
    return 0;
}

static int set_digital_gain(void* ctx, float gain) {
    printf("ov5647 %s %f\n", __func__, gain);
    return 0;
}

static int set_int_time(void* ctx, float gain) {
    printf("ov5647 %s %f\n", __func__, gain);
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
