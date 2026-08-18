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

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

#define OV13850_I2C_SLAVE_ADDRESS (ctx->slave_addr)
#define CHECK_ERROR(x) if (x) { printf("error f=%s l=%d\n", __func__, __LINE__); return -1; }

#define OV13850_REG_MODE_SELECT          0x0100
#define OV13850_MODE_STANDBY            0x00
#define OV13850_MODE_STREAMING          0x01
#define OV13850_REG_CHIP_ID_H           0x300a
#define OV13850_REG_CHIP_ID_L           0x300b
#define OV13850_CHIP_ID                 0xd850
#define OV13850_REG_EXPOSURE            0x3500
#define OV13850_REG_AGAIN_H             0x350a
#define OV13850_REG_AGAIN_L             0x350b
#define OV13850_REG_TIMING_FMT1         0x3820
#define OV13850_REG_TIMING_FMT2         0x3821
#define OV13850_TIMING_VFLIP_BIT        0x04
#define OV13850_TIMING_MIRROR_BIT       0x04

#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef MAX
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#endif

struct reg_list {
    uint16_t addr;
    uint8_t value;
};

struct ov13850_mode {
    struct vvcam_sensor_mode mode;
    struct reg_list *regs;
};

struct ov13850_ctx {
    int i2c;
    int slave_addr;
    int i2c_bus;
    struct vvcam_sensor_hw hw;
    struct vvcam_sensor_mode mode;
    uint32_t sensor_again;
    uint32_t et_line;
    bool hflip;
    bool vflip;
    uint8_t orient_base_3820;
    uint8_t orient_base_3821;
};

static int read_reg(struct ov13850_ctx *ctx, uint16_t addr, uint8_t *value);
static int write_reg(struct ov13850_ctx *ctx, uint16_t addr, uint8_t value);
static int open_i2c(struct ov13850_ctx *sensor);

static struct reg_list ov13850_mipi_2lane_raw10_3840x2160_7fps_regs[] = {
    {0x0102, 0x01},
    {0x0103, 0x01},
    {0x0300, 0x01},
    {0x0301, 0x00},
    {0x0302, 0x28},
    {0x0303, 0x00},
    {0x030a, 0x00},
    {0x300f, 0x11},
    {0x3010, 0x01},
    {0x3011, 0x76},
    {0x3012, 0x21},
    {0x3013, 0x12},
    {0x3014, 0x11},
    {0x301f, 0x03},
    {0x3106, 0x00},
    {0x3210, 0x47},
    {0x3500, 0x00},
    {0x3501, 0xfa},
    {0x3502, 0x00},
    {0x3506, 0x00},
    {0x3507, 0x00},
    {0x3508, 0x00},
    {0x350a, 0x00},
    {0x350b, 0x80},
    {0x350e, 0x00},
    {0x350f, 0x40},
    {0x351a, 0x00},
    {0x351b, 0x10},
    {0x351c, 0x00},
    {0x351d, 0x20},
    {0x351e, 0x00},
    {0x351f, 0x40},
    {0x3520, 0x00},
    {0x3521, 0x80},
    {0x3600, 0xc0},
    {0x3601, 0xfc},
    {0x3602, 0x02},
    {0x3603, 0x78},
    {0x3604, 0xb1},
    {0x3605, 0xb5},
    {0x3606, 0x73},
    {0x3607, 0x07},
    {0x3609, 0x40},
    {0x360a, 0x30},
    {0x360b, 0x91},
    {0x360c, 0x09},
    {0x360f, 0x02},
    {0x3611, 0x10},
    {0x3612, 0x27},
    {0x3613, 0x33},
    {0x3615, 0x0c},
    {0x3616, 0x0e},
    {0x3641, 0x02},
    {0x3660, 0x82},
    {0x3668, 0x54},
    {0x3669, 0x00},
    {0x366a, 0x3f},
    {0x3667, 0xa0},
    {0x3702, 0x40},
    {0x3703, 0x44},
    {0x3704, 0x2c},
    {0x3705, 0x01},
    {0x3706, 0x15},
    {0x3707, 0x44},
    {0x3708, 0x3c},
    {0x3709, 0x1f},
    {0x370a, 0x27},
    {0x370b, 0x3c},
    {0x3720, 0x55},
    {0x3722, 0x84},
    {0x3728, 0x40},
    {0x372a, 0x00},
    {0x372b, 0x02},
    {0x372e, 0x22},
    {0x372f, 0x90},
    {0x3730, 0x00},
    {0x3731, 0x00},
    {0x3732, 0x00},
    {0x3733, 0x00},
    {0x3710, 0x28},
    {0x3716, 0x03},
    {0x3718, 0x10},
    {0x3719, 0x0c},
    {0x371a, 0x08},
    {0x371c, 0xfc},
    {0x3748, 0x00},
    {0x3760, 0x13},
    {0x3761, 0x33},
    {0x3762, 0x86},
    {0x3763, 0x16},
    {0x3767, 0x24},
    {0x3768, 0x06},
    {0x3769, 0x45},
    {0x376c, 0x23},
    {0x376f, 0x80},
    {0x3773, 0x06},
    {0x3d84, 0x00},
    {0x3d85, 0x17},
    {0x3d8c, 0x73},
    {0x3d8d, 0xbf},
    {0x3800, 0x00},
    {0x3801, 0x08},
    {0x3802, 0x00},
    {0x3803, 0x04},
    {0x3804, 0x10},
    {0x3805, 0x97},
    {0x3806, 0x0c},
    {0x3807, 0x4b},
    {0x3808, 0x08},
    {0x3809, 0x40},
    {0x380a, 0x06},
    {0x380b, 0x20},
    {0x380c, 0x12},
    {0x380d, 0xc0},
    {0x380e, 0x06},
    {0x380f, 0x80},
    {0x3810, 0x00},
    {0x3811, 0x04},
    {0x3812, 0x00},
    {0x3813, 0x02},
    {0x3814, 0x11},
    {0x3815, 0x11},
    {0x3820, 0x00},
    {0x3821, 0x04},
    {0x3823, 0x00},
    {0x3826, 0x00},
    {0x3827, 0x02},
    {0x3834, 0x00},
    {0x3835, 0x1c},
    {0x3836, 0x08},
    {0x3837, 0x02},
    {0x4000, 0xf1},
    {0x4001, 0x00},
    {0x4006, 0x04},
    {0x4007, 0x04},
    {0x400b, 0x0c},
    {0x4011, 0x00},
    {0x401a, 0x00},
    {0x401b, 0x40},
    {0x401c, 0x00},
    {0x401d, 0x00},
    {0x4020, 0x00},
    {0x4021, 0xe4},
    {0x4022, 0x04},
    {0x4023, 0xd7},
    {0x4024, 0x05},
    {0x4025, 0xbc},
    {0x4026, 0x05},
    {0x4027, 0xbf},
    {0x4028, 0x00},
    {0x4029, 0x02},
    {0x402a, 0x04},
    {0x402b, 0x08},
    {0x402c, 0x02},
    {0x402d, 0x02},
    {0x402e, 0x0c},
    {0x402f, 0x08},
    {0x403d, 0x2c},
    {0x403f, 0x7f},
    {0x4041, 0x07},
    {0x4500, 0x82},
    {0x4501, 0x3c},
    {0x458b, 0x00},
    {0x459c, 0x00},
    {0x459d, 0x00},
    {0x459e, 0x00},
    {0x4601, 0x83},
    {0x4602, 0x22},
    {0x4603, 0x01},
    {0x4800, 0x24},
    {0x4837, 0x19},
    {0x4d00, 0x04},
    {0x4d01, 0x42},
    {0x4d02, 0xd1},
    {0x4d03, 0x90},
    {0x4d04, 0x66},
    {0x4d05, 0x65},
    {0x4d0b, 0x00},
    {0x5000, 0x0e},
    {0x5001, 0x01},
    {0x5002, 0x07},
    {0x5013, 0x40},
    {0x501c, 0x00},
    {0x501d, 0x10},
    {0x510f, 0xfc},
    {0x5110, 0xf0},
    {0x5111, 0x10},
    {0x536d, 0x02},
    {0x536e, 0x67},
    {0x536f, 0x01},
    {0x5370, 0x4c},
    {0x5400, 0x00},
    {0x5400, 0x00},
    {0x5401, 0x61},
    {0x5402, 0x00},
    {0x5403, 0x00},
    {0x5404, 0x00},
    {0x5405, 0x40},
    {0x540c, 0x05},
    {0x5501, 0x00},
    {0x5b00, 0x00},
    {0x5b01, 0x00},
    {0x5b02, 0x01},
    {0x5b03, 0xff},
    {0x5b04, 0x02},
    {0x5b05, 0x6c},
    {0x5b09, 0x02},
    {0x5e00, 0x00},
    {0x5e10, 0x1c},
    {0x0102, 0x01},
    {0x3612, 0x2f},
    {0x370a, 0x24},
    {0x372a, 0x00},
    {0x372f, 0xa0},
    {0x3800, 0x00},
    {0x3801, 0xc8},
    {0x3802, 0x01},
    {0x3803, 0xe8},
    {0x3804, 0x0f},
    {0x3805, 0xd7},
    {0x3806, 0x0a},
    {0x3807, 0x67},
    {0x3808, 0x0f},
    {0x3809, 0x00},
    {0x380a, 0x08},
    {0x380b, 0x70},
    {0x380c, 0x12},
    {0x380d, 0xc0},
    {0x380e, 0x09},
    {0x380f, 0x20},
    {0x3810, 0x00},
    {0x3811, 0x04},
    {0x3812, 0x00},
    {0x3813, 0x04},
    {0x3814, 0x11},
    {0x3815, 0x11},
    {0x3820, 0x00},
    {0x3821, 0x04},
    {0x3836, 0x04},
    {0x3837, 0x01},
    {0x4601, 0x87},
    {0x4603, 0x01},
    {0x4020, 0x00},
    {0x4021, 0xe6},
    {0x4022, 0x0e},
    {0x4023, 0x26},
    {0x4024, 0x0f},
    {0x4025, 0x08},
    {0x4026, 0x0f},
    {0x4027, 0x0e},
    {0x4603, 0x00},
    {0x5401, 0x71},
    {0x5405, 0x80},
    {0x0000, 0x00},
};

static struct ov13850_mode modes[] = {
    {
        .mode = {
            .clk = 24000000,
            .mclk = {
                .enable = true,
                .sel = VVCAM_PLL1_CLK_DIV4,
                .div = 8, /* 594/8 = 74.25 MHz */
            },
            .width = 3840,
            .height = 2160,
            .lanes = VVCAM_SENSOR_2LANE,
            .freq = VVCAM_SENSOR_800M,
            .bayer = VVCAM_BAYER_PAT_BGGR,
            .bit_width = 10,
            .ae_info = {
                .frame_length = 2336,
                .cur_frame_length = 2336,
                .one_line_exp_time = 0.000061155f,
                .gain_accuracy = 1024,
                .min_gain = 1.0f,
                .max_gain = 63.9375f,
                .int_time_delay_frame = 2,
                .gain_delay_frame = 2,
                .color_type = 0,
                .integration_time_increment = 0.000061155f,
                .gain_increment = (1.0f / 16.0f),
                .max_long_integraion_line = 2336 - 12,
                .min_long_integraion_line = 1,
                .max_integraion_line = 2336 - 12,
                .min_integraion_line = 1,
                .max_vs_integraion_line = 2336 - 12,
                .min_vs_integraion_line = 1,
                .max_long_integraion_time = 0.000061155f * (2336 - 12),
                .min_long_integraion_time = 0.000061155f * 2,
                .max_integraion_time = 0.000061155f * (2336 - 12),
                .min_integraion_time = 0.000061155f * 2,
                .max_vs_integraion_time = 0.000061155f * (2336 - 12),
                .min_vs_integraion_time = 0.000061155f * 2,
                .cur_long_integration_time = 0.0f,
                .cur_integration_time = 0.0f,
                .cur_vs_integration_time = 0.0f,
                .cur_long_again = 0.0f,
                .cur_long_dgain = 0.0f,
                .cur_again = 0.0f,
                .cur_dgain = 0.0f,
                .cur_vs_again = 0.0f,
                .cur_vs_dgain = 0.0f,
                .a_long_gain.min = 1.0f,
                .a_long_gain.max = 63.9375f,
                .a_long_gain.step = (1.0f / 16.0f),
                .a_gain.min = 1.0f,
                .a_gain.max = 63.9375f,
                .a_gain.step = (1.0f / 16.0f),
                .a_vs_gain.min = 1.0f,
                .a_vs_gain.max = 63.9375f,
                .a_vs_gain.step = (1.0f / 16.0f),
                .d_long_gain.max = 1.0f,
                .d_long_gain.min = 1.0f,
                .d_long_gain.step = 1.0f,
                .d_gain.max = 1.0f,
                .d_gain.min = 1.0f,
                .d_gain.step = 1.0f,
                .d_vs_gain.max = 1.0f,
                .d_vs_gain.min = 1.0f,
                .d_vs_gain.step = 1.0f,
                .cur_fps = 7,
            },
        },
        .regs = ov13850_mipi_2lane_raw10_3840x2160_7fps_regs,
    },
};

static int read_reg(struct ov13850_ctx *ctx, uint16_t addr, uint8_t *value)
{
    struct i2c_msg msg[2];
    struct i2c_rdwr_ioctl_data data;
    uint16_t addr_be = htobe16(addr);

    msg[0].addr = OV13850_I2C_SLAVE_ADDRESS;
    msg[0].buf = (uint8_t *)&addr_be;
    msg[0].len = 2;
    msg[0].flags = 0;

    msg[1].addr = OV13850_I2C_SLAVE_ADDRESS;
    msg[1].buf = value;
    msg[1].len = 1;
    msg[1].flags = I2C_M_RD;

    data.msgs = msg;
    data.nmsgs = 2;

    if (ioctl(ctx->i2c, I2C_RDWR, &data) != 2) {
        fprintf(stderr, "ov13850: i2c read reg %04x error %d(%s)\n", addr, errno, strerror(errno));
        return -1;
    }

    return 0;
}

static int write_reg(struct ov13850_ctx *ctx, uint16_t addr, uint8_t value)
{
    uint8_t buffer[3];

    buffer[0] = (addr >> 8) & 0xff;
    buffer[1] = addr & 0xff;
    buffer[2] = value;
    if (write(ctx->i2c, buffer, 3) != 3) {
        fprintf(stderr, "ov13850: i2c write reg %04x error %d(%s)\n", addr, errno, strerror(errno));
        return -1;
    }

    return 0;
}

static int open_i2c(struct ov13850_ctx *sensor)
{
    char i2c_dev[32];

    if (sensor->i2c < 0) {
        snprintf(i2c_dev, sizeof(i2c_dev), "/dev/i2c-%u", sensor->i2c_bus);
        sensor->i2c = open(i2c_dev, O_RDWR);
        if (sensor->i2c < 0) {
            perror(i2c_dev);
            return -1;
        }
        if (ioctl(sensor->i2c, I2C_SLAVE_FORCE, sensor->slave_addr) < 0) {
            perror("I2C_SLAVE_FORCE");
            return -1;
        }
    }

    return 0;
}

static int ov13850_read_chip_id(struct ov13850_ctx *sensor, uint32_t *chip_id)
{
    uint8_t id_h = 0;
    uint8_t id_l = 0;

    if (read_reg(sensor, OV13850_REG_CHIP_ID_H, &id_h)) {
        return -1;
    }
    if (read_reg(sensor, OV13850_REG_CHIP_ID_L, &id_l)) {
        return -1;
    }
    *chip_id = ((uint32_t)id_h << 8) | id_l;

    return 0;
}

static int probe(const struct vvcam_sensor_hw *hw, uint32_t *chip_id)
{
    struct ov13850_ctx sensor;
    struct vvcam_sensor_hw local = vvcam_sensor_hw_or_default(hw);
    struct vvcam_mclk_setting probe_mclk = {
        .enable = true,
        .sel = VVCAM_PLL1_CLK_DIV4,
        .div = 8, /* 594/8 = 74.25 MHz */
    };
    uint32_t id = 0;

    memset(&sensor, 0, sizeof(sensor));
    sensor.i2c = -1;
    sensor.hw = local;
    sensor.i2c_bus = local.i2c_bus;
    sensor.slave_addr = 0x10;

    vvcam_sensor_apply_mclk(&local, &probe_mclk);

    if (open_i2c(&sensor)) {
        return -1;
    }
    if (ov13850_read_chip_id(&sensor, &id)) {
        close(sensor.i2c);
        return -1;
    }
    close(sensor.i2c);

    if (chip_id) {
        *chip_id = id;
    }
    if (id != OV13850_CHIP_ID) {
        fprintf(stderr, "ov13850: chip id mismatch 0x%04x (expect 0x%04x)\n",
            id, OV13850_CHIP_ID);
        return -1;
    }

    fprintf(stderr, "ov13850: probe ok, chip id 0x%04x, i2c addr 0x%02x\n",
        id, sensor.slave_addr);

    return 0;
}

static int init(void **ctx, const struct vvcam_sensor_hw *hw)
{
    struct ov13850_ctx *sensor = calloc(1, sizeof(*sensor));
    struct vvcam_sensor_hw local = vvcam_sensor_hw_or_default(hw);

    if (!sensor) {
        return -1;
    }

    sensor->i2c = -1;
    sensor->hw = local;
    sensor->i2c_bus = local.i2c_bus;
    sensor->slave_addr = 0x10;
    memcpy(&sensor->mode, &modes[0].mode, sizeof(sensor->mode));
    *ctx = sensor;

    return 0;
}

static void deinit(void *ctx)
{
    struct ov13850_ctx *sensor = ctx;

    if (!sensor) {
        return;
    }
    vvcam_mclk_disable(sensor->hw.mclk_id);
    if (sensor->i2c >= 0) {
        close(sensor->i2c);
    }
    free(sensor);
}

static int ov13850_apply_orient(struct ov13850_ctx *sensor)
{
    uint8_t r3820 = sensor->orient_base_3820;
    uint8_t r3821 = sensor->orient_base_3821;

    if (open_i2c(sensor)) {
        return -1;
    }

    r3820 &= (uint8_t)~OV13850_TIMING_VFLIP_BIT;
    r3821 &= (uint8_t)~OV13850_TIMING_MIRROR_BIT;
    if (sensor->vflip) {
        r3820 |= OV13850_TIMING_VFLIP_BIT;
    }
    if (sensor->hflip) {
        r3821 |= OV13850_TIMING_MIRROR_BIT;
    }

    CHECK_ERROR(write_reg(sensor, OV13850_REG_TIMING_FMT1, r3820));
    CHECK_ERROR(write_reg(sensor, OV13850_REG_TIMING_FMT2, r3821));

    sensor->mode.bayer = VVCAM_BAYER_PAT_BGGR;

    return 0;
}

static int enum_mode(void *ctx, uint32_t index, struct vvcam_sensor_mode *mode)
{
    if (index >= ARRAY_SIZE(modes)) {
        return -1;
    }
    memcpy(mode, &modes[index].mode, sizeof(*mode));
    return 0;
}

static int get_mode(void *ctx, struct vvcam_sensor_mode *mode)
{
    struct ov13850_ctx *sensor = ctx;

    memcpy(mode, &sensor->mode, sizeof(*mode));
    return 0;
}

static int set_mode(void *ctx, uint32_t index)
{
    struct ov13850_ctx *sensor = ctx;
    struct vvcam_sensor_mode *mode;
    uint8_t again_h = 0;
    uint8_t again_l = 0;
    uint8_t exp_hh = 0;
    uint8_t exp_h = 0;
    uint8_t exp_l = 0;
    uint32_t exp_line;

    if (index >= ARRAY_SIZE(modes)) {
        return -1;
    }
    mode = &modes[index].mode;

    vvcam_sensor_apply_mclk(&sensor->hw, &mode->mclk);

    if (open_i2c(sensor)) {
        return -1;
    }

    sensor->orient_base_3820 = 0;
    sensor->orient_base_3821 = 0;
    for (unsigned i = 0;; i++) {
        if ((modes[index].regs[i].addr == 0) && (modes[index].regs[i].value == 0)) {
            break;
        }
        if (modes[index].regs[i].addr == OV13850_REG_TIMING_FMT1) {
            sensor->orient_base_3820 = modes[index].regs[i].value;
        } else if (modes[index].regs[i].addr == OV13850_REG_TIMING_FMT2) {
            sensor->orient_base_3821 = modes[index].regs[i].value;
        }
        CHECK_ERROR(write_reg(sensor, modes[index].regs[i].addr, modes[index].regs[i].value));
    }

    CHECK_ERROR(read_reg(sensor, OV13850_REG_AGAIN_H, &again_h));
    CHECK_ERROR(read_reg(sensor, OV13850_REG_AGAIN_L, &again_l));
    sensor->sensor_again = ((uint32_t)again_h << 8) | again_l;
    mode->ae_info.cur_gain = (float)sensor->sensor_again / 16.0f;
    mode->ae_info.cur_long_gain = mode->ae_info.cur_gain;
    mode->ae_info.cur_vs_gain = mode->ae_info.cur_gain;
    mode->ae_info.cur_again = mode->ae_info.cur_gain;

    CHECK_ERROR(read_reg(sensor, OV13850_REG_EXPOSURE, &exp_hh));
    CHECK_ERROR(read_reg(sensor, OV13850_REG_EXPOSURE + 1, &exp_h));
    CHECK_ERROR(read_reg(sensor, OV13850_REG_EXPOSURE + 2, &exp_l));
    exp_line = (((uint32_t)exp_hh & 0x0f) << 12) | ((uint32_t)exp_h << 4) | (exp_l >> 4);
    sensor->et_line = exp_line;
    mode->ae_info.cur_integration_time = (float)exp_line * mode->ae_info.one_line_exp_time;

    memcpy(&sensor->mode, mode, sizeof(sensor->mode));
    CHECK_ERROR(ov13850_apply_orient(sensor));

    return 0;
}

static int set_hflip(void *ctx, bool on)
{
    struct ov13850_ctx *sensor = ctx;

    sensor->hflip = on;
    return ov13850_apply_orient(sensor);
}

static int get_hflip(void *ctx, bool *on)
{
    struct ov13850_ctx *sensor = ctx;
    uint8_t r3821 = 0;

    if (!on) {
        return -1;
    }
    if (open_i2c(sensor)) {
        return -1;
    }
    if (read_reg(sensor, OV13850_REG_TIMING_FMT2, &r3821)) {
        return -1;
    }
    *on = (r3821 & OV13850_TIMING_MIRROR_BIT) != 0;
    sensor->hflip = *on;
    sensor->mode.bayer = VVCAM_BAYER_PAT_BGGR;

    return 0;
}

static int set_vflip(void *ctx, bool on)
{
    struct ov13850_ctx *sensor = ctx;

    sensor->vflip = on;
    return ov13850_apply_orient(sensor);
}

static int get_vflip(void *ctx, bool *on)
{
    struct ov13850_ctx *sensor = ctx;
    uint8_t r3820 = 0;

    if (!on) {
        return -1;
    }
    if (open_i2c(sensor)) {
        return -1;
    }
    if (read_reg(sensor, OV13850_REG_TIMING_FMT1, &r3820)) {
        return -1;
    }
    *on = (r3820 & OV13850_TIMING_VFLIP_BIT) != 0;
    sensor->vflip = *on;
    sensor->mode.bayer = VVCAM_BAYER_PAT_BGGR;

    return 0;
}

static int set_stream(void *ctx, bool on)
{
    struct ov13850_ctx *sensor = ctx;

    if (open_i2c(sensor)) {
        return -1;
    }
    if (on) {
        CHECK_ERROR(write_reg(sensor, OV13850_REG_MODE_SELECT, OV13850_MODE_STREAMING));
        CHECK_ERROR(write_reg(sensor, 0x5100, 0x80));
    } else {
        CHECK_ERROR(write_reg(sensor, OV13850_REG_MODE_SELECT, OV13850_MODE_STANDBY));
    }

    return 0;
}

static int set_analog_gain(void *ctx, float gain)
{
    struct ov13850_ctx *sensor = ctx;
    uint32_t again = (uint32_t)(gain * 16.0f + 0.5f);

    if (again > 0x3ff) {
        again = 0x3ff;
    }
    if (sensor->sensor_again != again) {
        CHECK_ERROR(write_reg(sensor, OV13850_REG_AGAIN_L, again & 0xff));
        CHECK_ERROR(write_reg(sensor, OV13850_REG_AGAIN_H, (again >> 8) & 0x3f));
        sensor->sensor_again = again;
    }

    sensor->mode.ae_info.cur_gain = (float)sensor->sensor_again / 16.0f;
    sensor->mode.ae_info.cur_long_gain = sensor->mode.ae_info.cur_gain;
    sensor->mode.ae_info.cur_vs_gain = sensor->mode.ae_info.cur_gain;
    sensor->mode.ae_info.cur_again = sensor->mode.ae_info.cur_gain;

    return 0;
}

static int set_digital_gain(void *ctx, float gain)
{
    (void)ctx;
    (void)gain;
    return 0;
}

static int set_int_time(void *ctx, float time)
{
    struct ov13850_ctx *sensor = ctx;
    uint32_t exp_line;

    exp_line = time / sensor->mode.ae_info.one_line_exp_time;
    exp_line = MIN(sensor->mode.ae_info.max_integraion_line,
        MAX(sensor->mode.ae_info.min_integraion_line, exp_line));

    if (sensor->et_line != exp_line) {
        uint32_t reg_value = exp_line << 4;

        CHECK_ERROR(write_reg(sensor, OV13850_REG_EXPOSURE, (reg_value >> 16) & 0x0f));
        CHECK_ERROR(write_reg(sensor, OV13850_REG_EXPOSURE + 1, (reg_value >> 8) & 0xff));
        CHECK_ERROR(write_reg(sensor, OV13850_REG_EXPOSURE + 2, reg_value & 0xf0));
        sensor->et_line = exp_line;
    }
    sensor->mode.ae_info.cur_integration_time =
        (float)sensor->et_line * sensor->mode.ae_info.one_line_exp_time;

    return 0;
}

struct vvcam_sensor vvcam_ov13850 = {
    .name = "ov13850",
    .ctrl = {
        .init = init,
        .deinit = deinit,
        .enum_mode = enum_mode,
        .get_mode = get_mode,
        .set_mode = set_mode,
        .set_stream = set_stream,
        .set_hflip = set_hflip,
        .get_hflip = get_hflip,
        .set_vflip = set_vflip,
        .get_vflip = get_vflip,
        .set_analog_gain = set_analog_gain,
        .set_digital_gain = set_digital_gain,
        .set_int_time = set_int_time,
        .probe = probe,
    }
};
