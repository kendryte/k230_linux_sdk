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
    struct gc2053_ctx* sensor = calloc(1, sizeof(struct gc2053_ctx));
    sensor->i2c = -1;
    *ctx = sensor;
    return 0;
}

static void deinit(void* ctx) {
    struct gc2053_ctx* sensor = ctx;
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

static struct gc2053_mode modes[] = {
    {
        .mode = {
            .clk = 24000000,
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
    }
};
static unsigned modes_len = sizeof(modes) / sizeof(struct gc2053_mode);

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
    struct gc2053_ctx* sensor = ctx;
    // printf("gc2053 %s   \n", __func__);

    memcpy(mode, &sensor->mode, sizeof(struct vvcam_sensor_mode));

    return 0;
}

static int set_mode(void* ctx, uint32_t index) {
    struct gc2053_ctx* sensor = ctx;
    if (index > modes_len) {
        // out of range
        return -1;
    }
    struct vvcam_sensor_mode* mode = &modes[index].mode;

    printf("gc2053: %s %ux%u\n", __func__, mode->width, mode->height);
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
    uint8_t fe_val = 0;
    float again = 0, dgain = 0;

    CHECK_ERROR(read_reg(ctx, GC2053_REG_DGAIN_H, &again_h));
    CHECK_ERROR(read_reg(ctx, GC2053_REG_DGAIN_L, &again_l));
    again_l = 0x1;
    again = (float)(again_l>>2)/64.0f + again_h;

    printf("*****************************mode->ae_info.again is %f again_h is %d again_l is %d \n", again, again_h, again_l); 

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
        .set_int_time = set_int_time
    }
};
