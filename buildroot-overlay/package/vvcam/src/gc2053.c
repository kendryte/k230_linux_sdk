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

#define I2C_SLAVE_ADDRESS 0x37
#define CHECK_ERROR(x) if(x)return -1

#define GC2053_REG_CHIP_ID_H                                0xf0
#define GC2053_REG_CHIP_ID_L                                0xf1
#define GC2053_CHIP_ID                                      0x2053

#define GC2053_REG_LONG_AGAIN_H                             0x0001
#define GC2053_REG_LONG_AGAIN_L                             0x0002

#define GC2053_REG_DGAIN_H	                                0xb1	//0x00b8
#define GC2053_REG_DGAIN_L	                                0xb2	//0x00b9

#define GC2053_REG_LONG_EXP_TIME_H                          0x03
#define GC2053_REG_LONG_EXP_TIME_L                          0x04

#define GC2053_MIN_GAIN_STEP                                (1.0f/16.0f)
#define GC2053_SW_RESET                                     0x0103
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

struct gc2053_mode {
    struct vvcam_sensor_mode mode;
    struct reg_list* regs;
};

struct gc2053_ctx {
    int i2c;
    uint8_t i2c_bus;
    struct vvcam_sensor_hw hw;
    struct vvcam_sensor_mode mode;      // fora 3a current val
    uint32_t sensor_again;
    uint32_t et_line;
};

static int read_reg(struct gc2053_ctx* ctx, uint16_t addr, uint8_t* value) {
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
        fprintf(stderr, "gc2053: i2c read reg %04x error %d(%s)\n", be16toh(addr), errno, strerror(errno));
        return -1;
    }

    return 0;
}

static int write_reg(struct gc2053_ctx* ctx, uint16_t addr, uint8_t value) {
    uint8_t buffer[3];

    // buffer[0] = (addr >> 8) & 0xff;
    // buffer[1] = (addr >> 0) & 0xff;
    // buffer[2] = value;

    buffer[0] = (addr >> 0) & 0xff;
    buffer[1] = value;

    if (write(ctx->i2c, buffer, 2) != 2) {
        fprintf(stderr, "gc2053: i2c write reg %04x error %d(%s)\n", addr, errno, strerror(errno));
        return -1;
    }
    // printf("gc2053 w %04x %02x\n", addr, value);
    return 0;
}


static int open_i2c(struct gc2053_ctx* sensor) {
    char i2c_dev[32];

    if (sensor->i2c < 0) {
        snprintf(i2c_dev, sizeof(i2c_dev), "/dev/i2c-%u", sensor->i2c_bus);
        sensor->i2c = open(i2c_dev, O_RDWR);
        if (sensor->i2c < 0) {
            perror(i2c_dev);
            return -1;
        }
        if (ioctl(sensor->i2c, I2C_SLAVE_FORCE, I2C_SLAVE_ADDRESS) < 0) {
            perror("i2c ctrl 0x36");
            return -1;
        }
    }
    return 0;
}

static void gc2053_close_i2c(struct gc2053_ctx *sensor)
{
    if (sensor && sensor->i2c >= 0) {
        close(sensor->i2c);
        sensor->i2c = -1;
    }
}

static int gc2053_read_chip_id(struct gc2053_ctx *sensor, uint32_t *chip_id)
{
    uint8_t id_h = 0;
    uint8_t id_l = 0;
    uint32_t id;

    if (read_reg(sensor, GC2053_REG_CHIP_ID_H, &id_h))
        return -1;
    if (read_reg(sensor, GC2053_REG_CHIP_ID_L, &id_l))
        return -1;

    id = ((uint32_t)id_h << 8) | id_l;
    if (chip_id)
        *chip_id = id;
    if (id == GC2053_CHIP_ID)
        return 0;

    fprintf(stderr, "gc2053: chip id mismatch 0x%04x (expect 0x%04x)\n",
        id, GC2053_CHIP_ID);
    return -1;
}

static int probe(const struct vvcam_sensor_hw *hw, uint32_t *chip_id)
{
    struct gc2053_ctx sensor;
    struct vvcam_sensor_hw local = vvcam_sensor_hw_or_default(hw);
    struct vvcam_mclk_setting probe_mclk = {
        .enable = true,
        .sel = VVCAM_PLL1_CLK_DIV4,
        .div = 25, /* 594/25 = 23.76 MHz */
    };
    uint32_t id = 0;

    memset(&sensor, 0, sizeof(sensor));
    sensor.i2c = -1;
    sensor.hw = local;
    sensor.i2c_bus = local.i2c_bus;

    vvcam_sensor_apply_mclk(&local, &probe_mclk);

    if (open_i2c(&sensor))
        return -1;
    if (gc2053_read_chip_id(&sensor, &id) != 0) {
        fprintf(stderr, "gc2053: read chip id failed on i2c-%u csi-%u mclk-%u\n",
            local.i2c_bus, local.csi_idx, local.mclk_id);
        gc2053_close_i2c(&sensor);
        return -1;
    }
    gc2053_close_i2c(&sensor);

    if (chip_id)
        *chip_id = id;

    fprintf(stderr, "gc2053: probe ok, chip id 0x%04x, i2c addr 0x%02x\n",
        id, I2C_SLAVE_ADDRESS);
    return 0;
}

static int init(void** ctx, const struct vvcam_sensor_hw *hw) {
    struct gc2053_ctx* sensor = calloc(1, sizeof(struct gc2053_ctx));
    struct vvcam_sensor_hw local = vvcam_sensor_hw_or_default(hw);

    sensor->i2c = -1;
    sensor->hw = local;
    sensor->i2c_bus = local.i2c_bus;
    *ctx = sensor;
    return 0;
}

static void deinit(void* ctx) {
    struct gc2053_ctx* sensor = ctx;
    vvcam_mclk_disable(sensor->hw.mclk_id);
    close(sensor->i2c);
    free(ctx);
}



static struct reg_list sensor_enable_regs[] = {
    {0xfe, 0x00},
	{0x3e, 0x81},
    {0x3e, 0x91},
    {0x0, 0x00},
};


static struct reg_list gc2053_1920x1080_30fps[] = {
    {0xfe, 0x80},
	{0xfe, 0x80},
	{0xfe, 0x80},
	{0xfe, 0x00},
	{0xf2, 0x00},
	{0xf3, 0x00},
	{0xf4, 0x36},
	{0xf5, 0xc0},
	{0xf6, 0x44},
	{0xf7, 0x01},
	{0xf8, 0x69},	//pllmp_div, MIPI clock divider, 105
	{0xf9, 0x40},
	{0xfc, 0x8e},
	/****CISCTL & ANALOG****/
	{0xfe, 0x00},	//Page 0
	{0x87, 0x18},
	{0xee, 0x30},
	{0xd0, 0xb7},
    {0x03, 0x00},	//ET = 0x60 = 96
    {0x04, 0x60},
	{0x05, 0x04},	//line lenth= 1100x2 = 2200
	{0x06, 0x4c},
	{0x07, 0x00},	//Vblank = 0x49 = 73, 16
	{0x08, 0x10},
	{0x09, 0x00},	//raw start = 0x02
	{0x0a, 0x02},
	{0x0b, 0x00},	//col start = 0x02
	{0x0c, 0x02},
	{0x0d, 0x04},	//win_height = 1084
	{0x0e, 0x3c},
	{0x0f, 0x07},	//win_width = 1924
	{0x10, 0x84},
	{0x12, 0xe2},
	{0x13, 0x16},
	{0x19, 0x0a},
	{0x21, 0x1c},
	{0x28, 0x0a},
	{0x29, 0x24},
	{0x2b, 0x04},
	{0x32, 0xf8},
	{0x37, 0x03},
	{0x39, 0x15},
	{0x43, 0x07},
	{0x44, 0x40},
	{0x46, 0x0b},
	{0x4b, 0x20},
	{0x4e, 0x08},
	{0x55, 0x20},
	{0x66, 0x05},
	{0x67, 0x05},
	{0x77, 0x01},
	{0x78, 0x00},
	{0x7c, 0x93},
	{0x8c, 0x12},
	{0x8d, 0x92},
	{0x90, 0x00},
	{0x41, 0x04},	//frame length, 0x49d = 1181
	{0x42, 0x9d},
	{0x9d, 0x10},
	{0xce, 0x7c},
	{0xd2, 0x41},
	{0xd3, 0xdc},
	{0xe6, 0x50},
	/*gain*/
	{0xb6, 0xc0},
	{0xb0, 0x60},	//0x70
	{0xb1, 0x01},	//gain: 1x
	{0xb2, 0x00},
	{0xb3, 0x00},
	{0xb4, 0x00},
	{0xb8, 0x01},
	{0xb9, 0x00},
	/*blk*/
	{0x26, 0x30},
	{0xfe, 0x01},
	{0x40, 0x23},	//black level & offset enable
	{0x55, 0x07},
	{0x60, 0x10},	//WB_offset(dark offset), default is 0x00
	{0xfe, 0x04},
	{0x14, 0x78},
	{0x15, 0x78},
	{0x16, 0x78},
	{0x17, 0x78},
	/*window*/
	{0xfe, 0x01},
	{0x92, 0x02},
	{0x94, 0x03},
	{0x95, 0x04},//[10:0]win_out_height=1080
	{0x96, 0x38},
	{0x97, 0x07},//[11:0]win_out_width=1920
	{0x98, 0x80},
	/*ISP*/
	{0xfe, 0x01},
	{0x01, 0x05},
	{0x02, 0x89},
	{0x04, 0x01},
	{0x07, 0xa6},
	{0x08, 0xa9},
	{0x09, 0xa8},
	{0x0a, 0xa7},
	{0x0b, 0xff},
	{0x0c, 0xff},
	{0x0f, 0x00},
	{0x50, 0x1c},
	{0x89, 0x03},
	{0xfe, 0x04},
	{0x28, 0x86},
	{0x29, 0x86},
	{0x2a, 0x86},
	{0x2b, 0x68},
	{0x2c, 0x68},
	{0x2d, 0x68},
	{0x2e, 0x68},
	{0x2f, 0x68},
	{0x30, 0x4f},
	{0x31, 0x68},
	{0x32, 0x67},
	{0x33, 0x66},
	{0x34, 0x66},
	{0x35, 0x66},
	{0x36, 0x66},
	{0x37, 0x66},
	{0x38, 0x62},
	{0x39, 0x62},
	{0x3a, 0x62},
	{0x3b, 0x62},
	{0x3c, 0x62},
	{0x3d, 0x62},
	{0x3e, 0x62},
	{0x3f, 0x62},
	/****DVP & MIPI****/
	{0xfe, 0x01},
	{0x9a, 0x06},
	{0x99, 0x00},	//out window offset
	{0xfe, 0x00},
	{0x7b, 0x2a},
	{0x23, 0x2d},
	{0xfe, 0x03},
	{0x01, 0x27},
	{0x02, 0x56},
	{0x03, 0x8e},
	{0x12, 0x80},
	{0x13, 0x07},
	{0xfe, 0x00},
	{0x3e, 0x81},
    {0x3e, 0x91},
    { 0, 0x00 }
};

static struct reg_list gc2053_1280x960_50fps[] = {
    {0x00fe, 0x80},
    {0x00fe, 0x80},
    {0x00fe, 0x80},
    {0x00fe, 0x00},
    {0x00f2, 0x00},
    {0x00f3, 0x00},
    {0x00f4, 0x36},
    {0x00f5, 0xc0},
    {0x00f6, 0x81},
    {0x00f7, 0x01},
    {0x00f8, 0x29},
    {0x00f9, 0x80},
    {0x00fc, 0x8e},
    {0x00fe, 0x00},
    {0x0087, 0x18},
    {0x00ee, 0x30},
    {0x00d0, 0xb7},
    {0x0003, 0x00},
    {0x0004, 0x60},
    {0x0005, 0x03},
    {0x0006, 0x79},
    {0x0007, 0x00},
    {0x0008, 0x10},
    {0x0009, 0x00},
    {0x000a, 0x3e},
    {0x000b, 0x01},
    {0x000c, 0x44},
    {0x000d, 0x03},
    {0x000e, 0xc4},
    {0x000f, 0x05},
    {0x0010, 0x04},
    {0x0012, 0xe2},
    {0x0013, 0x16},
    {0x0019, 0x0a},
    {0x0021, 0x1c},
    {0x0028, 0x0a},
    {0x0029, 0x24},
    {0x002b, 0x04},
    {0x0032, 0xf8},
    {0x0037, 0x03},
    {0x0039, 0x15},
    {0x0043, 0x07},
    {0x0044, 0x40},
    {0x0046, 0x0b},
    {0x004b, 0x20},
    {0x004e, 0x08},
    {0x0055, 0x20},
    {0x0066, 0x05},
    {0x0067, 0x05},
    {0x0077, 0x01},
    {0x0078, 0x00},
    {0x007c, 0x93},
    {0x008c, 0x12},
    {0x008d, 0x92},
    {0x0090, 0x00},
    {0x0041, 0x04},
    {0x0042, 0x1e},
    {0x009d, 0x10},
    {0x00ce, 0x7c},
    {0x00d2, 0x41},
    {0x00d3, 0xdc},
    {0x00e6, 0x50},
    {0x00b6, 0xc0},
    {0x00b0, 0x60},
    {0x00b1, 0x01},
    {0x00b2, 0x00},
    {0x00b3, 0x00},
    {0x00b4, 0x00},
    {0x00b8, 0x01},
    {0x00b9, 0x00},
    {0x0026, 0x30},
    {0x00fe, 0x01},
    {0x0040, 0x23},
    {0x0055, 0x07},
    {0x0060, 0x10},
    {0x00fe, 0x04},
    {0x0014, 0x78},
    {0x0015, 0x78},
    {0x0016, 0x78},
    {0x0017, 0x78},
    {0x00fe, 0x01},
    {0x0092, 0x02},
    {0x0094, 0x03},
    {0x0095, 0x03},
    {0x0096, 0xc0},
    {0x0097, 0x05},
    {0x0098, 0x00},
    {0x00fe, 0x01},
    {0x0001, 0x05},
    {0x0002, 0x89},
    {0x0004, 0x01},
    {0x0007, 0xa6},
    {0x0008, 0xa9},
    {0x0009, 0xa8},
    {0x000a, 0xa7},
    {0x000b, 0xff},
    {0x000c, 0xff},
    {0x000f, 0x00},
    {0x0050, 0x1c},
    {0x0089, 0x03},
    {0x00fe, 0x04},
    {0x0028, 0x86},
    {0x0029, 0x86},
    {0x002a, 0x86},
    {0x002b, 0x68},
    {0x002c, 0x68},
    {0x002d, 0x68},
    {0x002e, 0x68},
    {0x002f, 0x68},
    {0x0030, 0x4f},
    {0x0031, 0x68},
    {0x0032, 0x67},
    {0x0033, 0x66},
    {0x0034, 0x66},
    {0x0035, 0x66},
    {0x0036, 0x66},
    {0x0037, 0x66},
    {0x0038, 0x62},
    {0x0039, 0x62},
    {0x003a, 0x62},
    {0x003b, 0x62},
    {0x003c, 0x62},
    {0x003d, 0x62},
    {0x003e, 0x62},
    {0x003f, 0x62},
    {0x00fe, 0x01},
    {0x009a, 0x06},
    {0x0099, 0x00},
    {0x00fe, 0x00},
    {0x007b, 0x2a},
    {0x0023, 0x2d},
    {0x00fe, 0x03},
    {0x0001, 0x27},
    {0x0002, 0x56},
    {0x0003, 0x8e},
    {0x0012, 0x80},
    {0x0013, 0x07},
    {0x00fe, 0x00},
    {0x003e, 0x81},
    {0x003e, 0x91},
    { 0, 0x00 }
};

static struct reg_list gc2053_1280x720_60fps[] = {
    {0x00fe, 0x80},
    {0x00fe, 0x80},
    {0x00fe, 0x80},
    {0x00fe, 0x00},
    {0x00f2, 0x00},
    {0x00f3, 0x00},
    {0x00f4, 0x36},
    {0x00f5, 0xc0},
    {0x00f6, 0x81},
    {0x00f7, 0x01},
    {0x00f8, 0x23},
    {0x00f9, 0x80},
    {0x00fc, 0x8e},
    {0x00fe, 0x00},
    {0x0087, 0x18},
    {0x00ee, 0x30},
    {0x00d0, 0xb7},
    {0x0003, 0x00},
    {0x0004, 0x60},
    {0x0005, 0x03},
    {0x0006, 0x79},
    {0x0007, 0x00},
    {0x0008, 0x10},
    {0x0009, 0x00},
    {0x000a, 0xb6},
    {0x000b, 0x01},
    {0x000c, 0x44},
    {0x000d, 0x02},
    {0x000e, 0xd4},
    {0x000f, 0x05},
    {0x0010, 0x04},
    {0x0012, 0xe2},
    {0x0013, 0x16},
    {0x0019, 0x0a},
    {0x0021, 0x1c},
    {0x0028, 0x0a},
    {0x0029, 0x24},
    {0x002b, 0x04},
    {0x0032, 0xf8},
    {0x0037, 0x03},
    {0x0039, 0x15},
    {0x0043, 0x07},
    {0x0044, 0x40},
    {0x0046, 0x0b},
    {0x004b, 0x20},
    {0x004e, 0x08},
    {0x0055, 0x20},
    {0x0066, 0x05},
    {0x0067, 0x05},
    {0x0077, 0x01},
    {0x0078, 0x00},
    {0x007c, 0x93},
    {0x008c, 0x12},
    {0x008d, 0x92},
    {0x0090, 0x00},
    {0x0041, 0x03},
    {0x0042, 0x2c},
    {0x009d, 0x10},
    {0x00ce, 0x7c},
    {0x00d2, 0x41},
    {0x00d3, 0xdc},
    {0x00e6, 0x50},
    {0x00b6, 0xc0},
    {0x00b0, 0x60},
    {0x00b1, 0x01},
    {0x00b2, 0x00},
    {0x00b3, 0x00},
    {0x00b4, 0x00},
    {0x00b8, 0x01},
    {0x00b9, 0x00},
    {0x0026, 0x30},
    {0x00fe, 0x01},
    {0x0040, 0x23},
    {0x0055, 0x07},
    {0x0060, 0x10},
    {0x00fe, 0x04},
    {0x0014, 0x78},
    {0x0015, 0x78},
    {0x0016, 0x78},
    {0x0017, 0x78},
    {0x00fe, 0x01},
    {0x0092, 0x02},
    {0x0094, 0x03},
    {0x0095, 0x02},
    {0x0096, 0xd0},
    {0x0097, 0x05},
    {0x0098, 0x00},
    {0x00fe, 0x01},
    {0x0001, 0x05},
    {0x0002, 0x89},
    {0x0004, 0x01},
    {0x0007, 0xa6},
    {0x0008, 0xa9},
    {0x0009, 0xa8},
    {0x000a, 0xa7},
    {0x000b, 0xff},
    {0x000c, 0xff},
    {0x000f, 0x00},
    {0x0050, 0x1c},
    {0x0089, 0x03},
    {0x00fe, 0x04},
    {0x0028, 0x86},
    {0x0029, 0x86},
    {0x002a, 0x86},
    {0x002b, 0x68},
    {0x002c, 0x68},
    {0x002d, 0x68},
    {0x002e, 0x68},
    {0x002f, 0x68},
    {0x0030, 0x4f},
    {0x0031, 0x68},
    {0x0032, 0x67},
    {0x0033, 0x66},
    {0x0034, 0x66},
    {0x0035, 0x66},
    {0x0036, 0x66},
    {0x0037, 0x66},
    {0x0038, 0x62},
    {0x0039, 0x62},
    {0x003a, 0x62},
    {0x003b, 0x62},
    {0x003c, 0x62},
    {0x003d, 0x62},
    {0x003e, 0x62},
    {0x003f, 0x62},
    {0x00fe, 0x01},
    {0x009a, 0x06},
    {0x0099, 0x00},
    {0x00fe, 0x00},
    {0x007b, 0x2a},
    {0x0023, 0x2d},
    {0x00fe, 0x03},
    {0x0001, 0x27},
    {0x0002, 0x56},
    {0x0003, 0x8e},
    {0x0012, 0x80},
    {0x0013, 0x07},
    {0x00fe, 0x00},
    {0x003e, 0x81},
    {0x003e, 0x91},
    { 0, 0x00 }
};

static struct gc2053_mode modes[] = {
    {
        .mode = {
            .clk = 24000000,
            .mclk = {
                .enable = true,
                .sel = VVCAM_PLL1_CLK_DIV4,
                .div = 25, /* 594/25 = 23.76 MHz */
            },
            .width = 1920,
            .height = 1080,
            .lanes = VVCAM_SENSOR_2LANE,
            .freq = VVCAM_SENSOR_1200M,
            .bayer = VVCAM_BAYER_PAT_RGGB,
            .bit_width = 10,
            .ae_info = {
                .frame_length = 1181,
                .cur_frame_length = 1181,
                .one_line_exp_time = 0.000028219,
                .gain_accuracy = 1024,
                .min_gain = 1.0,
                .max_gain = 18.0,
                .int_time_delay_frame = 2,
                .gain_delay_frame = 2,
                .color_type = 0,
                .integration_time_increment = 0.000028219,
                .gain_increment = (1.0f/64.0f),
                .max_long_integraion_line = 1181 - 1,
                .min_long_integraion_line = 2,
                .max_integraion_line = 1181 - 1,
                .min_integraion_line = 2,
                .max_long_integraion_time = 0.000028219 * (1181 - 1),
                .min_long_integraion_time = 0.000028219 * 2,
                .max_integraion_time = 0.000028219 * (1181 - 1),
                .min_integraion_time = 0.000028219 * 2,
                .cur_long_integration_time = 0.0,
                .cur_integration_time = 0.0,
                .cur_long_again = 0.0,
                .cur_long_dgain = 0.0,
                .cur_again = 0.0,
                .cur_dgain = 0.0,
                .a_gain.min = 1.0,
                .a_gain.max = 50,
                .a_gain.step = (1.0f/64.0f),
                .d_gain.max = 1.0,
                .d_gain.min = 1.0,
                .d_gain.step = (1.0f/1024.0f),
                .cur_fps = 30,
            }
        },
        .regs = gc2053_1920x1080_30fps
    },
    {
        .mode = {
            .clk = 24000000,
            .mclk = {
                .enable = true,
                .sel = VVCAM_PLL1_CLK_DIV4,
                .div = 25, /* 594/25 = 23.76 MHz */
            },
            .width = 1280,
            .height = 960,
            .lanes = VVCAM_SENSOR_2LANE,
            .freq = VVCAM_SENSOR_1200M,
            .bayer = VVCAM_BAYER_PAT_RGGB,
            .bit_width = 10,
            .ae_info = {
                .frame_length = 1054,
                .cur_frame_length = 1054,
                .one_line_exp_time = 0.000018982,
                .gain_accuracy = 1024,
                .min_gain = 1.0,
                .max_gain = 18.0,
                .int_time_delay_frame = 2,
                .gain_delay_frame = 2,
                .color_type = 0,
                .integration_time_increment = 0.000018982,
                .gain_increment = (1.0f/64.0f),
                .max_long_integraion_line = 1054 - 1,
                .min_long_integraion_line = 1,
                .max_integraion_line = 1054 - 1,
                .min_integraion_line = 1,
                .max_long_integraion_time = 0.000018982 * (1054 - 1),
                .min_long_integraion_time = 0.000018982 * 1,
                .max_integraion_time = 0.000018982 * (1054 - 1),
                .min_integraion_time = 0.000018982 * 1,
                .cur_long_integration_time = 0.0,
                .cur_integration_time = 0.0,
                .cur_long_again = 0.0,
                .cur_long_dgain = 0.0,
                .cur_again = 0.0,
                .cur_dgain = 0.0,
                .a_gain.min = 1.0,
                .a_gain.max = 50,
                .a_gain.step = (1.0f/64.0f),
                .d_gain.max = 1.0,
                .d_gain.min = 1.0,
                .d_gain.step = (1.0f/1024.0f),
                .cur_fps = 50,
            }
        },
        .regs = gc2053_1280x960_50fps
    },
    {
        .mode = {
            .clk = 24000000,
            .mclk = {
                .enable = true,
                .sel = VVCAM_PLL1_CLK_DIV4,
                .div = 25, /* 594/25 = 23.76 MHz */
            },
            .width = 1280,
            .height = 720,
            .lanes = VVCAM_SENSOR_2LANE,
            .freq = VVCAM_SENSOR_1200M,
            .bayer = VVCAM_BAYER_PAT_RGGB,
            .bit_width = 10,
            .ae_info = {
                .frame_length = 812,
                .cur_frame_length = 812,
                .one_line_exp_time = 0.000020525,
                .gain_accuracy = 1024,
                .min_gain = 1.0,
                .max_gain = 18.0,
                .int_time_delay_frame = 2,
                .gain_delay_frame = 2,
                .color_type = 0,
                .integration_time_increment = 0.000020525,
                .gain_increment = (1.0f/64.0f),
                .max_long_integraion_line = 812 - 1,
                .min_long_integraion_line = 1,
                .max_integraion_line = 812 - 1,
                .min_integraion_line = 1,
                .max_long_integraion_time = 0.000020525 * (812 - 1),
                .min_long_integraion_time = 0.000020525 * 1,
                .max_integraion_time = 0.000020525 * (812 - 1),
                .min_integraion_time = 0.000020525 * 1,
                .cur_long_integration_time = 0.0,
                .cur_integration_time = 0.0,
                .cur_long_again = 0.0,
                .cur_long_dgain = 0.0,
                .cur_again = 0.0,
                .cur_dgain = 0.0,
                .a_gain.min = 1.0,
                .a_gain.max = 50,
                .a_gain.step = (1.0f/64.0f),
                .d_gain.max = 1.0,
                .d_gain.min = 1.0,
                .d_gain.step = (1.0f/1024.0f),
                .cur_fps = 60,
            }
        },
        .regs = gc2053_1280x720_60fps
    }
};
static unsigned modes_len = sizeof(modes) / sizeof(struct gc2053_mode);

static int enum_mode(void* ctx, uint32_t index, struct vvcam_sensor_mode* mode) {
    if (index >= modes_len) {
        return -1;
    }
    memcpy(mode, &modes[index].mode, sizeof(struct vvcam_sensor_mode));
    return 0;
}

static int get_mode(void* ctx, struct vvcam_sensor_mode* mode) {
    // memcpy(mode, &modes[0].mode, sizeof(struct vvcam_sensor_mode));
    struct gc2053_ctx* sensor = ctx;
    // printf("gc2053 %s   \n", __func__);

    memcpy(mode, &sensor->mode, sizeof(struct vvcam_sensor_mode));

    return 0;
}

static int set_mode(void* ctx, uint32_t index) {
    struct gc2053_ctx* sensor = ctx;
    if (index >= modes_len) {
        // out of range
        return -1;
    }
    struct vvcam_sensor_mode* mode = &modes[index].mode;

    vvcam_sensor_apply_mclk(&sensor->hw, &mode->mclk);

    if (open_i2c(sensor)) {
        return -1;
    }

    for(unsigned i = 0;; i++) {
        if ((modes[index].regs[i].addr == 0) && (modes[index].regs[i].value == 0)) {
            break;
        }
        CHECK_ERROR(write_reg(sensor, modes[index].regs[i].addr, modes[index].regs[i].value));
    }


    uint8_t again_h, again_l;
    uint8_t exp_time_h, exp_time_l;
    uint8_t exp_time;
    uint8_t fe_val = 0;
    float again = 0, dgain = 0;

    CHECK_ERROR(read_reg(ctx, GC2053_REG_DGAIN_H, &again_h));
    CHECK_ERROR(read_reg(ctx, GC2053_REG_DGAIN_L, &again_l));
    again_l = 0x1;
    again = (float)(again_l>>2)/64.0f + again_h;

    sensor->sensor_again = (again * 64 + 0.5);

    again = 1.0;
    dgain = 1.0;
    mode->ae_info.cur_gain = again * dgain;
    mode->ae_info.cur_long_gain = mode->ae_info.cur_gain;
    mode->ae_info.cur_vs_gain = mode->ae_info.cur_gain;

    CHECK_ERROR(read_reg(ctx, GC2053_REG_LONG_EXP_TIME_H, &exp_time_h));
    CHECK_ERROR(read_reg(ctx, GC2053_REG_LONG_EXP_TIME_L, &exp_time_l));
    exp_time = 1.0;//((exp_time_h & 0x3f) << 8) + exp_time_l;

    mode->ae_info.cur_integration_time = exp_time * mode->ae_info.one_line_exp_time;

    CHECK_ERROR(read_reg(ctx, 0x87, &fe_val));

    printf("mode->ae_info.cur_integration_time is %f fe_val is %x \n", mode->ae_info.cur_integration_time, fe_val);

    // save current mode
    memcpy(&sensor->mode , mode, sizeof(struct vvcam_sensor_mode));

    return 0;
}


static int set_stream(void* ctx, bool on) {
    struct gc2053_ctx* sensor = ctx;
    printf("gc2053 %s %d\n", __func__, on);
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
        write_reg(sensor, 0xfe, 0x00);
        write_reg(sensor, 0x3e, 0x81);
        write_reg(sensor, 0x3e, 0x91);
    } else {
        // write_reg(sensor, 0x03fe, 0xf0);
        // write_reg(sensor, 0x03fe, 0xf0);
        // write_reg(sensor, 0x03fe, 0xf0);

        write_reg(sensor, 0xfe, 0x00);
        write_reg(sensor, 0x3e, 0x11);
    }

    return 0;
}


static uint16_t regValTable[29][4] = {
	/*0xb4 0xb3  0xb8  0xb9*/
	{0x00, 0x00, 0x01, 0x00},
	{0x00, 0x10, 0x01, 0x0c},
	{0x00, 0x20, 0x01, 0x1b},
	{0x00, 0x30, 0x01, 0x2c},
	{0x00, 0x40, 0x01, 0x3f},
	{0x00, 0x50, 0x02, 0x16},
	{0x00, 0x60, 0x02, 0x35},
	{0x00, 0x70, 0x03, 0x16},
	{0x00, 0x80, 0x04, 0x02},
	{0x00, 0x90, 0x04, 0x31},
	{0x00, 0xa0, 0x05, 0x32},
	{0x00, 0xb0, 0x06, 0x35},
	{0x00, 0xc0, 0x08, 0x04},
	{0x00, 0x5a, 0x09, 0x19},
	{0x00, 0x83, 0x0b, 0x0f},
	{0x00, 0x93, 0x0d, 0x12},
	{0x00, 0x84, 0x10, 0x00},
	{0x00, 0x94, 0x12, 0x3a},
	{0x01, 0x2c, 0x1a, 0x02},
	{0x01, 0x3c, 0x1b, 0x20},
	{0x00, 0x8c, 0x20, 0x0f},
	{0x00, 0x9c, 0x26, 0x07},
	{0x02, 0x64, 0x36, 0x21},
	{0x02, 0x74, 0x37, 0x3a},
	{0x00, 0xc6, 0x3d, 0x02},
	{0x00, 0xdc, 0x3f, 0x3f},
	{0x02, 0x85, 0x3f, 0x3f},
	{0x02, 0x95, 0x3f, 0x3f},
	{0x00, 0xce, 0x3f, 0x3f},
};

static uint32_t gainLevelTable[] = {
	64,
	74,
	89,
	102,
	127,
	147,
	177,
	203,
	260,
	300,
	361,
	415,
	504,
	581,
	722,
	832,
	1027,
	1182,
	1408,
	1621,
	1990,
	2291,
	2850,
	3282,
	4048,
	5180,
	5500,
	6744,
	7073,
};


static int set_analog_gain(void* ctx, float gain) {
    struct gc2053_ctx* sensor = ctx;
    uint32_t again, dgain, total;;
    uint32_t i = 0;
    printf("gc2053 %s %f\n", __func__, gain);

    again = (uint32_t)(gain * 64 + 0.5);

    if(sensor->sensor_again !=again)
    {
        total = sizeof(gainLevelTable) / sizeof(uint32_t);
        for ( i = 0; i < total; i++)
        {
            if ((gainLevelTable[i] <= again) && (again < gainLevelTable[i + 1]))
            break;
        }
        dgain = (again <<6) / gainLevelTable[i];

        CHECK_ERROR(write_reg(ctx, 0xb4, regValTable[i][0]));
        CHECK_ERROR(write_reg(ctx, 0xb3, regValTable[i][1]));
        CHECK_ERROR(write_reg(ctx, 0xb8, regValTable[i][2]));
        CHECK_ERROR(write_reg(ctx, 0xb9, regValTable[i][3]));

        CHECK_ERROR(write_reg(ctx, 0xb1, (dgain>>6)));
        CHECK_ERROR(write_reg(ctx, 0xb2, ((dgain&0x3f)<<2)));

        sensor->sensor_again = again;
    }

    sensor->mode.ae_info.cur_gain = (float)sensor->sensor_again/64.0f;

    return 0;
}

static int set_digital_gain(void* ctx, float gain) {
    // printf("gc2053 %s %f\n", __func__, gain);
    return 0;
}

static int set_int_time(void* ctx, float time) {
    struct gc2053_ctx* sensor = ctx;
    uint16_t exp_line = 0;
    float integraion_time = 0;

    printf("gc2053 %s %f\n", __func__, time);

    integraion_time = time;

    exp_line = integraion_time / sensor->mode.ae_info.one_line_exp_time;
    exp_line = MIN(sensor->mode.ae_info.max_integraion_line, MAX(sensor->mode.ae_info.min_integraion_line, exp_line));

    if (sensor->et_line != exp_line)
    {
        CHECK_ERROR(write_reg(ctx, GC2053_REG_LONG_EXP_TIME_H, (exp_line >> 8) & 0x3f));
        CHECK_ERROR(write_reg(ctx, GC2053_REG_LONG_EXP_TIME_L, (exp_line) & 0xff));

        sensor->et_line = exp_line;
    }
    sensor->mode.ae_info.cur_integration_time = (float)sensor->et_line * sensor->mode.ae_info.one_line_exp_time;

    return 0;
}

struct vvcam_sensor vvcam_gc2053 = {
    .name = "gc2053",
    .ctrl = {
        .init = init,
        .deinit = deinit,
        .enum_mode = enum_mode,
        .get_mode = get_mode,
        .set_mode = set_mode,
        .set_stream = set_stream,
        .set_analog_gain = set_analog_gain,
        .set_digital_gain = set_digital_gain,
        .set_int_time = set_int_time,
        .probe = probe,
    }
};
