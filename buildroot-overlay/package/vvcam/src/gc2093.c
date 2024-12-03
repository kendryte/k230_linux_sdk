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

#define GC2093_REG_CHIP_ID_H                                0x300a
#define GC2093_REG_CHIP_ID_L                                0x300b
#define GC2093_REG_MIPI_CTRL00                              0x4800
#define GC2093_REG_FRAME_OFF_NUMBER                         0x4202
#define GC2093_REG_PAD_OUT                                  0x300d

#define GC2093_REG_VTS_H                                    0x380e
#define GC2093_REG_VTS_L                                    0x380f

#define GC2093_REG_MIPI_CTRL14                              0x4814

#define GC2093_SW_STANDBY                                   0x0100


#define GC2093_REG_LONG_AGAIN_H                             0x0001
#define GC2093_REG_LONG_AGAIN_L                             0x0002

#define GC2093_REG_DGAIN_H	                                0x00b1	//0x00b8
#define GC2093_REG_DGAIN_L	                                0x00b2	//0x00b9

#define GC2093_REG_LONG_EXP_TIME_H                          0x0003
#define GC2093_REG_LONG_EXP_TIME_L                          0x0004

#define GC2093_MIN_GAIN_STEP                                (1.0f/16.0f)
#define GC2093_SW_RESET                                     0x0103
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

struct gc2093_mode {
    struct vvcam_sensor_mode mode;
    struct reg_list* regs;
};

struct gc2093_ctx {
    int i2c;
    struct vvcam_sensor_mode mode;      // fora 3a current val
    uint32_t sensor_again;
    uint32_t et_line;
};

static int read_reg(struct gc2093_ctx* ctx, uint16_t addr, uint8_t* value) {
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
        fprintf(stderr, "gc2093: i2c read reg %04x error %d(%s)\n", be16toh(addr), errno, strerror(errno));
        return -1;
    }

    return 0;
}

static int write_reg(struct gc2093_ctx* ctx, uint16_t addr, uint8_t value) {
    uint8_t buffer[3];

    buffer[0] = (addr >> 8) & 0xff;
    buffer[1] = (addr >> 0) & 0xff;
    buffer[2] = value;
    if (write(ctx->i2c, buffer, 3) != 3) {
        fprintf(stderr, "gc2093: i2c write reg %04x error %d(%s)\n", addr, errno, strerror(errno));
        return -1;
    }
    // printf("gc2093 w %04x %02x\n", addr, value);
    return 0;
}


static int open_i2c(struct gc2093_ctx* sensor) {
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
    struct gc2093_ctx* sensor = calloc(1, sizeof(struct gc2093_ctx));
    sensor->i2c = -1;
    *ctx = sensor;
    return 0;
}

static void deinit(void* ctx) {
    struct gc2093_ctx* sensor = ctx;
    close(sensor->i2c);
    free(ctx);
}



static struct reg_list gc2093_1920x1080_30fps[] = {
    {0x03fe,0xf0},
    {0x03fe,0xf0},
    {0x03fe,0xf0},
    {0x03fe,0x00},
    {0x03f2,0x00},
    {0x03f3,0x00},
    {0x03f4,0x36},
    {0x03f5,0xc0},
    {0x03f6,0x0B},
    {0x03f7,0x11},
    {0x03f8,0x30},
    {0x03f9,0x42},
    {0x03fc,0x8e},
    /****CISCTL & ANALOG****/
    {0x0087, 0x18},
    {0x00ee, 0x30},
    {0x00d0, 0xbf},
    {0x01a0, 0x00},
    {0x01a4, 0x40},
    {0x01a5, 0x40},
    {0x01a6, 0x40},
    {0x01af, 0x09},
    {0x0003, 0x00},	//ET
    {0x0004, 0x64},
    {0x0005, 0x05},	//line width = 0x522 = 1314 x 2 = 2628
    {0x0006, 0x22},
    {0x0007, 0x00},	//Vblank = 17
    {0x0008, 0x6e},
    {0x0009, 0x00},
    {0x000a, 0x02},
    {0x000b, 0x00},
    {0x000c, 0x04},
    {0x000d, 0x04},	//win_height = 1088
    {0x000e, 0x40},
    {0x000f, 0x07},	//win_width = 1932
    {0x0010, 0x8c},
    {0x0013, 0x15},
    {0x0019, 0x0c},
    {0x0041, 0x06},	// frame length = 0x04c2 = 1218
    {0x0042, 0x3a},
    {0x0053, 0x60},
    {0x008d, 0x92},
    {0x0090, 0x00},
    {0x00c7, 0xe1},
    {0x001b, 0x73},
    {0x0028, 0x0d},
    {0x0029, 0x40},
    {0x002b, 0x04},
    {0x002e, 0x23},
    {0x0037, 0x03},
    {0x0043, 0x04},
    {0x0044, 0x30},
    {0x004a, 0x01},
    {0x004b, 0x28},
    {0x0055, 0x30},
    {0x0066, 0x3f},
    {0x0068, 0x3f},
    {0x006b, 0x44},
    {0x0077, 0x00},
    {0x0078, 0x20},
    {0x007c, 0xa1},
    {0x00ce, 0x7c},
    {0x00d3, 0xd4},
    {0x00e6, 0x50},
    /*gain*/
    {0x00b6, 0xc0},
    {0x00b0, 0x68},
    {0x00b3, 0x00},
    {0x00b8, 0x01},
    {0x00b9, 0x00},
    {0x00b1, 0x01},
    {0x00b2, 0x00},
    /*isp*/
    {0x0101, 0x0c},
    {0x0102, 0x89},
    {0x0104, 0x01},
    {0x0107, 0xa6},
    {0x0108, 0xa9},
    {0x0109, 0xa8},
    {0x010a, 0xa7},
    {0x010b, 0xff},
    {0x010c, 0xff},
    {0x010f, 0x00},
    {0x0158, 0x00},
    {0x0428, 0x86},
    {0x0429, 0x86},
    {0x042a, 0x86},
    {0x042b, 0x68},
    {0x042c, 0x68},
    {0x042d, 0x68},
    {0x042e, 0x68},
    {0x042f, 0x68},
    {0x0430, 0x4f},
    {0x0431, 0x68},
    {0x0432, 0x67},
    {0x0433, 0x66},
    {0x0434, 0x66},
    {0x0435, 0x66},
    {0x0436, 0x66},
    {0x0437, 0x66},
    {0x0438, 0x62},
    {0x0439, 0x62},
    {0x043a, 0x62},
    {0x043b, 0x62},
    {0x043c, 0x62},
    {0x043d, 0x62},
    {0x043e, 0x62},
    {0x043f, 0x62},
    /*dark sun*/
    {0x0123, 0x08},
    {0x0123, 0x00},
    {0x0120, 0x01},
    {0x0121, 0x04},
    {0x0122, 0x65},
    {0x0124, 0x03},
    {0x0125, 0xff},
    {0x001a, 0x8c},
    {0x00c6, 0xe0},
    /*blk*/
    {0x0026, 0x30},
    {0x0142, 0x00},
    {0x0149, 0x1e},
    {0x014a, 0x0f},
    {0x014b, 0x00},
    {0x0155, 0x07},
    {0x0160, 0x10},	//WB_offset(dark offset)
    {0x0414, 0x78},
    {0x0415, 0x78},
    {0x0416, 0x78},
    {0x0417, 0x78},
    {0x04e0, 0x18},
    /*window*/
    {0x0192, 0x02},	//out_win_y_off = 2 
    {0x0194, 0x03},	//out_win_x_off = 3 
    {0x0195, 0x04},	//out_win_height = 1080
    {0x0196, 0x38}, 
    {0x0197, 0x07},	//out_win_width = 1920
    {0x0198, 0x80}, 
    /****DVP & MIPI****/
    {0x0199, 0x00},	//out window offset
    {0x019a, 0x06},
    {0x007b, 0x2a},
    {0x0023, 0x2d},
    {0x0201, 0x27},
    {0x0202, 0x56},
    {0x0203, 0xb6},
    {0x0212, 0x80},
    {0x0213, 0x07},
    {0x0215, 0x10},
    {0x003e, 0x91},
    { 0, 0x00 }
};

static struct gc2093_mode modes[] = {
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
                .frame_length = 1200,
                .cur_frame_length = 1200,
                .one_line_exp_time = 0.000027778,
                .gain_accuracy = 1024,
                .min_gain = 1.0,
                .max_gain = 18.0,
                .int_time_delay_frame = 2,
                .gain_delay_frame = 2,
                .color_type = 0,
                .integration_time_increment = 0.000027778,
                .gain_increment = (1.0f/16.0f),
                .max_long_integraion_line = 1200 - 12,
                .min_long_integraion_line = 2,
                .max_integraion_line = 1200 - 12,
                .min_integraion_line = 2,
                .max_long_integraion_time = 0.000027778 * (1200 - 12),
                .min_long_integraion_time = 0.000027778 * 2,
                .max_integraion_time = 0.000027778 * (1200 - 12),
                .min_integraion_time = 0.000027778 * 2,
                .cur_long_integration_time = 0.0,
                .cur_integration_time = 0.0,
                .cur_long_again = 0.0,
                .cur_long_dgain = 0.0,
                .cur_again = 0.0,
                .cur_dgain = 0.0,
                .a_gain.min = 1.0,
                .a_gain.max = 63.984375,
                .a_gain.step = (1.0f/64.0f),
                .d_gain.max = 1.0,
                .d_gain.min = 1.0,
                .d_gain.step = (1.0f/1024.0f),
                .cur_fps = 30,
            }
        },
        .regs = gc2093_1920x1080_30fps
    }
};
static unsigned modes_len = sizeof(modes) / sizeof(struct gc2093_mode);

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
    struct gc2093_ctx* sensor = ctx;
    // printf("gc2093 %s   \n", __func__);

    memcpy(mode, &sensor->mode, sizeof(struct vvcam_sensor_mode));

    return 0;
}

static int set_mode(void* ctx, uint32_t index) {
    struct gc2093_ctx* sensor = ctx;
    if (index > modes_len) {
        // out of range
        return -1;
    }
    struct vvcam_sensor_mode* mode = &modes[index].mode;

    printf("gc2093: %s %ux%u\n", __func__, mode->width, mode->height);
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

    CHECK_ERROR(read_reg(ctx, GC2093_REG_DGAIN_H, &again_h));
    CHECK_ERROR(read_reg(ctx, GC2093_REG_DGAIN_L, &again_l));
    again = (float)(again_l)/64.0f + again_h;

    printf("*****************************mode->ae_info.again is %f \n", again);

    sensor->sensor_again = (again * 64 + 0.5);

    again = 1.0;
    dgain = 1.0;
    mode->ae_info.cur_gain = again * dgain;
    mode->ae_info.cur_long_gain = mode->ae_info.cur_gain;
    mode->ae_info.cur_vs_gain = mode->ae_info.cur_gain;

    CHECK_ERROR(read_reg(ctx, GC2093_REG_LONG_EXP_TIME_H, &exp_time_h));
    CHECK_ERROR(read_reg(ctx, GC2093_REG_LONG_EXP_TIME_L, &exp_time_l));
    exp_time = ((exp_time_h & 0x3f) << 8) + exp_time_l;

    mode->ae_info.cur_integration_time = exp_time * mode->ae_info.one_line_exp_time;

    printf("mode->ae_info.cur_integration_time is %f \n", mode->ae_info.cur_integration_time);

    // save current mode
    memcpy(&sensor->mode , mode, sizeof(struct vvcam_sensor_mode));

    return 0;
}

static int set_stream(void* ctx, bool on) {
    struct gc2093_ctx* sensor = ctx;
    printf("gc2093 %s %d\n", __func__, on);
    if (open_i2c(sensor)) {
        return -1;
    }
    if (on) {
        // if (write_reg(sensor, 0x0100, 1)) {
        //     printf("gc2093 write reg error");
        //     return -1;
        // }
    } else {
        write_reg(sensor, 0x03fe, 0xf0);
        write_reg(sensor, 0x03fe, 0xf0);
        write_reg(sensor, 0x03fe, 0xf0);
    }

    return 0;
}


static unsigned char regValTable[25][7] = {
       //   0xb3 0xb8 0xb9 0x155 0xc2 0xcf 0xd9
    {0x00,0x01,0x00,0x08,0x10,0x08,0x0a},
    {0x10,0x01,0x0c,0x08,0x10,0x08,0x0a},
    {0x20,0x01,0x1b,0x08,0x10,0x08,0x0a},
    {0x30,0x01,0x2c,0x08,0x11,0x08,0x0c},
    {0x40,0x01,0x3f,0x08,0x12,0x08,0x0e},
    {0x50,0x02,0x16,0x08,0x14,0x08,0x12},
    {0x60,0x02,0x35,0x08,0x15,0x08,0x14},
    {0x70,0x03,0x16,0x08,0x17,0x08,0x18},
    {0x80,0x04,0x02,0x08,0x18,0x08,0x1a},
    {0x90,0x04,0x31,0x08,0x19,0x08,0x1c},
    {0xa0,0x05,0x32,0x08,0x1b,0x08,0x20},
    {0xb0,0x06,0x35,0x08,0x1c,0x08,0x22},
    {0xc0,0x08,0x04,0x08,0x1e,0x08,0x26},
    {0x5a,0x09,0x19,0x08,0x1c,0x08,0x26},
    {0x83,0x0b,0x0f,0x08,0x1c,0x08,0x26},
    {0x93,0x0d,0x12,0x08,0x1f,0x08,0x28},
    {0x84,0x10,0x00,0x0b,0x20,0x08,0x2a},
    {0x94,0x12,0x3a,0x0b,0x22,0x08,0x2e},
    {0x5d,0x1a,0x02,0x0b,0x27,0x08,0x38},
    {0x9b,0x1b,0x20,0x0b,0x28,0x08,0x3a},
    {0x8c,0x20,0x0f,0x0b,0x2a,0x08,0x3e},
    {0x9c,0x26,0x07,0x12,0x2d,0x08,0x44},
    {0xB6,0x36,0x21,0x12,0x2d,0x08,0x44},
    {0xad,0x37,0x3a,0x12,0x2d,0x08,0x44},
    {0xbd,0x3d,0x02,0x12,0x2d,0x08,0x44},
};

static unsigned int gainLevelTable[26] = {
	64,
	76,
	91,
	107,
	125,
	147,
	177,
	211,
	248,
	297,
	356,
	425,
	504,
	599,
	709,
	836,
	978,
	1153,
	1647,
	1651,
	1935,
	2292,
	3239,
	3959,
	4686,
	0xffffffff,
};

static int set_analog_gain(void* ctx, float gain) {
    struct gc2093_ctx* sensor = ctx;
    uint32_t again, dgain, total;;
    uint32_t i = 0;
    // printf("gc2093 %s %f\n", __func__, gain);

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

        CHECK_ERROR(write_reg(ctx, 0x00b3, regValTable[i][0]));
        CHECK_ERROR(write_reg(ctx, 0x00b8, regValTable[i][1]));
        CHECK_ERROR(write_reg(ctx, 0x00b9, regValTable[i][2]));
        CHECK_ERROR(write_reg(ctx, 0x0155, regValTable[i][3]));
        CHECK_ERROR(write_reg(ctx, 0x031d, 0x2d));
        CHECK_ERROR(write_reg(ctx, 0x00c2, regValTable[i][4]));
        CHECK_ERROR(write_reg(ctx, 0x00cf, regValTable[i][5]));
        CHECK_ERROR(write_reg(ctx, 0x00d9, regValTable[i][6]));
        CHECK_ERROR(write_reg(ctx, 0x031d, 0x28));

        CHECK_ERROR(write_reg(ctx, 0x00b1, (dgain>>6)));
        CHECK_ERROR(write_reg(ctx, 0x00b2, ((dgain&0x3f)<<2)));

        sensor->sensor_again = again;
    }

    sensor->mode.ae_info.cur_gain = (float)sensor->sensor_again/64.0f;

    return 0;
}

static int set_digital_gain(void* ctx, float gain) {
    // printf("gc2093 %s %f\n", __func__, gain);
    return 0;
}

static int set_int_time(void* ctx, float time) {
    struct gc2093_ctx* sensor = ctx;
    uint16_t exp_line = 0;
    float integraion_time = 0;

    // printf("gc2093 %s %f\n", __func__, time);

    integraion_time = time;

    exp_line = integraion_time / sensor->mode.ae_info.one_line_exp_time;
    exp_line = MIN(sensor->mode.ae_info.max_integraion_line, MAX(sensor->mode.ae_info.min_integraion_line, exp_line));

    if (sensor->et_line != exp_line)
    {
        CHECK_ERROR(write_reg(ctx, GC2093_REG_LONG_EXP_TIME_H, (exp_line >> 8) & 0x3f));
        CHECK_ERROR(write_reg(ctx, GC2093_REG_LONG_EXP_TIME_L, (exp_line) & 0xff));

        sensor->et_line = exp_line;
    }
    sensor->mode.ae_info.cur_integration_time = (float)sensor->et_line * sensor->mode.ae_info.one_line_exp_time;

    return 0;
}

struct vvcam_sensor vvcam_gc2093 = {
    .name = "gc2093",
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
