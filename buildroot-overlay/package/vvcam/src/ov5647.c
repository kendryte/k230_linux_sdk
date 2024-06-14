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

// FIXME

enum k230_mclk {
    K230_MCLK0 = 0,
    K230_MCLK1 = 1,
    K230_MCLK2 = 2,
};

// Enable MCLK
int k230_mclk_enable(enum k230_mclk mclk);

// Disable MCLK
int k230_mclk_disable(enum k230_mclk mclk);

// Set MCLK frequency
uint32_t k230_mclk_set_frequency(enum k230_mclk mclk, uint32_t frequency);

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
    // power & clock
    k230_mclk_set_frequency(K230_MCLK0, 25000000);
    k230_mclk_enable(K230_MCLK0);
    // i2c
    int i2c = open("/dev/i2c-0", O_RDWR);
    if (i2c < 0) {
        perror("open /dev/i2c-0");
        return -1;
    }
    if (ioctl(i2c, I2C_SLAVE_FORCE, 0x36) < 0) {
        perror("i2c ctrl 0x36");
        return -1;
    }
    struct ov5647_ctx* sensor = malloc(sizeof(struct ov5647_ctx));
    sensor->i2c = i2c;
    *ctx = sensor;
    return 0;
}

static void deinit(void* ctx) {
    struct ov5647_ctx* sensor = ctx;
    k230_mclk_disable(K230_MCLK0);
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
