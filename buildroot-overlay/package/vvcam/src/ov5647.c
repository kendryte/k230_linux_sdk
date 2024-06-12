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

static ssize_t write_reg(struct ov5647_ctx* ctx, uint16_t addr, uint8_t value) {
    unsigned char buffer[3];
    buffer[0] = (addr >> 8) & 0xff;
    buffer[1] = (addr >> 0) & 0xff;
    buffer[2] = value;
    return write(ctx->i2c, buffer, 3);
}

static int init(void** ctx) {
    struct ov5647_ctx* sensor = malloc(sizeof(struct ov5647_ctx));
    sensor->i2c = open("/dev/i2c-0", O_RDWR);
    if (sensor->i2c < 0) {
        free(sensor);
        perror("open /dev/i2c-0");
        return -1;
    }
    if (ioctl(sensor->i2c, I2C_SLAVE_FORCE, 0x36) < 0) {
        free(sensor);
        perror("i2c ctrl 0x36");
        return -1;
    }
    *ctx = sensor;
    return 0;
}

static void deinit(void* ctx) {
    struct ov5647_ctx* sensor = ctx;
    close(sensor->i2c);
    free(ctx);
}

static struct vvcam_sensor_mode modes[] = {
    {
        .clk = 25000000,
        .width = 1920,
        .height = 1080,
        .lanes = VVCAM_SENSOR_2LANE,
        .freq = VVCAM_SENSOR_800M,
        .bayer = VVCAM_BAYER_PAT_GBRG,
    }
};

static int enum_mode(void* ctx, uint32_t index, struct vvcam_sensor_mode* mode) {
    if (index == 0) {
        memcpy(mode, &modes[0], sizeof(struct vvcam_sensor_mode));
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
    return 0;
}

static int set_stream(void* ctx, bool on) {
    struct ov5647_ctx* sensor = ctx;
    if (on) {
        write_reg(sensor, 0x0100, 1);
    } else {
        write_reg(sensor, 0x3018, 0xff);
        write_reg(sensor, 0x0100, 0);
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
