#include <stdint.h>
#include <vvcam_sensor.h>
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
#include <math.h>

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))


#define I2C_SLAVE_ADDRESS_IMX335 0x1a
#define CHECK_ERROR(x) if(x){printf("error f=%s l=%d \n",__func__, __LINE__); return -1;}

#define imx335_REG_CHIP_ID_H                                0x300a
#define imx335_REG_CHIP_ID_L                                0x300b
#define imx335_REG_MIPI_CTRL00                              0x4800
#define imx335_REG_FRAME_OFF_NUMBER                         0x4202
#define imx335_REG_PAD_OUT                                  0x300d

#define imx335_REG_VTS_H                                    0x380e
#define imx335_REG_VTS_L                                    0x380f

#define imx335_REG_MIPI_CTRL14                              0x4814

#define imx335_SW_STANDBY                                   0x0100


#define imx335_REG_LONG_AGAIN_H                             0x0001
#define imx335_REG_LONG_AGAIN_L                             0x0002

#define imx335_REG_DGAIN_H	                                0x30e8	//0x00b8
#define imx335_REG_DGAIN_L	                                0x30e9	//0x00b9

#define imx335_REG_LONG_EXP_TIME_H                          0x0003
#define imx335_REG_LONG_EXP_TIME_L                          0x0004

#define imx335_MIN_GAIN_STEP                                (1.0f/16.0f)
#define imx335_SW_RESET                                     0x0103
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


/* Streaming Mode */
#define IMX335_REG_MODE_SELECT 0x3000
#define IMX335_MODE_STANDBY 0x01
#define IMX335_MODE_STREAMING 0x00

/* Lines per frame */
#define IMX335_REG_LPFR 0x3030

/* Chip ID */
#define IMX335_REG_ID 0x3912
#define IMX335_ID 0x00

/* Exposure control */
#define IMX335_REG_SHR0_L 0x3058
#define IMX335_REG_SHR0_M 0x3059
#define IMX335_REG_SHR1_L 0x305c
#define IMX335_REG_SHR1_M 0x305d
#define IMX335_REG_SHR2_L 0x3060
#define IMX335_REG_SHR2_M 0x3061
#define IMX335_REG_RHS1_L 0x3068
#define IMX335_REG_RHS1_M 0x3069
#define IMX335_REG_RHS2_L 0x306c
#define IMX335_REG_RHS2_M 0x306d
#define IMX335_VMAX_LINEAR 4500
#define IMX335_VMAX_DOL2 3980
#define IMX335_VMAX_DOL3    (uint16_t)(4500U)

/* Analog gain control */
#define IMX335_REG_AGAIN_L 0x30e8
#define IMX335_REG_AGAIN_H 0x30e9
#define IMX335_AGAIN_STEP (1.0f/256.0f)

/* Group hold register */
#define IMX335_REG_HOLD 0x3001

/* Input clock rate */
#define IMX335_INCLK_RATE 24000000

/* CSI2 HW configuration */
#define IMX335_LINK_FREQ 594000000
#define IMX335_NUM_DATA_LANES 4

#define IMX335_REG_MIN 0x00
#define IMX335_REG_MAX 0xfffff

#define DOL2_RHS1 482
#define DOL3_RHS1 986
#define DOL3_RHS2 2608//1072

#define DOL2_ratio 16.0
#define DOL3_LS_ratio 16.0
#define DOL3_VS_ratio 16.0


struct reg_list {
    uint16_t addr;
    uint8_t value;
};

struct imx335_mode {
    struct vvcam_sensor_mode mode;
    const struct reg_list* regs;
};


struct imx335_ctx {
    int i2c;
    struct vvcam_sensor_mode mode;      // fora 3a current val
    uint32_t sensor_again;
    uint32_t et_line;
};




static const struct reg_list imx335_mipi_2lane_raw12_1920x1080_30fps_mclk_24m_regs[] = {
    { 0x3000, 0x01 },
    { 0x3002, 0x00 },
    { 0x3a01, 0x01 }, // 2 lane
    { 0x300c, 0x3b }, // lane 24MHz
    { 0x300d, 0x2a },
    { 0x314c, 0xc6 },
    { 0x314d, 0x00 },
    { 0x315a, 0x02 },
    { 0x3168, 0xa0 },
    { 0x316a, 0x7e },
    { 0x319e, 0x01 },    //1188Mbps
    { 0x3a18, 0x8f }, // mipi phy
    { 0x3a19, 0x00 },
    { 0x3a1a, 0x4f },
    { 0x3a1b, 0x00 },
    { 0x3a1c, 0x47 },
    { 0x3a1d, 0x00 },
    { 0x3a1e, 0x37 },
    { 0x3a1f, 0x01 },
    { 0x3a20, 0x4f },
    { 0x3a21, 0x00 },
    { 0x3a22, 0x87 },
    { 0x3a23, 0x00 },
    { 0x3a24, 0x4f },
    { 0x3a25, 0x00 },
    { 0x3a26, 0x7f },
    { 0x3a27, 0x00 },
    { 0x3a28, 0x3f },
    { 0x3a29, 0x00 },
    { 0x3018, 0x04 }, // window mode
    { 0x3300, 0x00 }, // scan mode or binning
    { 0x3302, 0x10 }, //black level
    { 0x3199, 0x00 }, // HADD VADD
    { 0x3030, 0x94 }, // V span max
    { 0x3031, 0x11 },
    { 0x3032, 0x00 },
    { 0x3034, 0x26 }, // H span max
    { 0x3035, 0x02 },
    { 0x304c, 0x14 }, // V OB width
    { 0x304e, 0x00 }, // H dir inv
    { 0x304f, 0x00 }, // V dir inv
    { 0x3058, 0xac }, // shutter sweep time, 1000 ET Line
    { 0x3059, 0x0d },
    { 0x305a, 0x00 },
    { 0x30e8, 0x00 },
    { 0x30e9, 0x00 },
    { 0x3056, 0x48 }, // effective pixel line
    { 0x3057, 0x04 },
    { 0x302c, 0x80 }, // crop mode H start
    { 0x302d, 0x01 },

    { 0x302e, 0x80 }, // crop mode H size, 1944(798) - > 1920(780)
    { 0x302f, 0x07 },

    { 0x3072, 0x28 }, // crop mode OB V size
    { 0x3073, 0x00 },
    { 0x3074, 0x60 }, // crop mode UL V start
    { 0x3075, 0x02 },
    { 0x3076, 0x90 }, // crop mode V size, 2192
    { 0x3077, 0x08 },
    { 0x30c6, 0x12 }, // crop mode black offset
    { 0x30c7, 0x00 },
    { 0x30ce, 0x64 }, // crop mode UNRD_LINE_MAX
    { 0x30cf, 0x00 },
    { 0x30d8, 0xc0 }, // crop mode UNREAD_ED_ADR
    { 0x30d9, 0x0b },
    { 0x00, 0x00 }
};
static const struct  reg_list imx335_mipi_2lane_raw12_2592x1944_30fps_mclk_24m_regs[] = {
    { 0x3000, 0x01 },
    { 0x3002, 0x00 },
    { 0x3a01, 0x01 }, // 2 lane
    { 0x300c, 0x3b }, // lane 24MHz
    { 0x300d, 0x2a },
    { 0x314c, 0xc6 },
    { 0x314d, 0x00 },
    { 0x315a, 0x02 },
    { 0x3168, 0xa0 },
    { 0x316a, 0x7e },
    { 0x319e, 0x01 },    //1188Mbps
    { 0x3a18, 0x8f }, // mipi phy
    { 0x3a19, 0x00 },
    { 0x3a1a, 0x4f },
    { 0x3a1b, 0x00 },
    { 0x3a1c, 0x47 },
    { 0x3a1d, 0x00 },
    { 0x3a1e, 0x37 },
    { 0x3a1f, 0x01 },
    { 0x3a20, 0x4f },
    { 0x3a21, 0x00 },
    { 0x3a22, 0x87 },
    { 0x3a23, 0x00 },
    { 0x3a24, 0x4f },
    { 0x3a25, 0x00 },
    { 0x3a26, 0x7f },
    { 0x3a27, 0x00 },
    { 0x3a28, 0x3f },
    { 0x3a29, 0x00 },

    { 0x302e, 0x20 }, // crop mode H size, 2616(a38) - > 2592(a20)
    { 0x302f, 0x0a },

    { 0x3302, 0x10 }, //black level
    { 0x3058, 0xac }, // shutter sweep time, 1000 ET Line
    { 0x3059, 0x0d },
    { 0x305a, 0x00 },
    { 0x30e8, 0x00 },
    { 0x30e9, 0x00 },
    { 0, 0x00 }
};
static const struct  reg_list imx335_mipi_2lane_raw12_1920x1080_30fps_mclk_74_25_regs[] = {
    { 0x3000, 0x01 },
    { 0x3002, 0x00 },
    { 0x3a01, 0x01 }, // 2 lane
    { 0x300c, 0xb6 }, // lane 24MHz
    { 0x300d, 0x7f },
    { 0x314c, 0x80 },
    { 0x314d, 0x00 },
    { 0x315a, 0x03 },
    { 0x3168, 0x68 },
    { 0x316a, 0x7f },
    { 0x319e, 0x01 },    //1188Mbps
    { 0x3a18, 0x8f }, // mipi phy
    { 0x3a19, 0x00 },
    { 0x3a1a, 0x4f },
    { 0x3a1b, 0x00 },
    { 0x3a1c, 0x47 },
    { 0x3a1d, 0x00 },
    { 0x3a1e, 0x37 },
    { 0x3a1f, 0x01 },
    { 0x3a20, 0x4f },
    { 0x3a21, 0x00 },
    { 0x3a22, 0x87 },
    { 0x3a23, 0x00 },
    { 0x3a24, 0x4f },
    { 0x3a25, 0x00 },
    { 0x3a26, 0x7f },
    { 0x3a27, 0x00 },
    { 0x3a28, 0x3f },
    { 0x3a29, 0x00 },
    { 0x3018, 0x04 }, // window mode
    { 0x3300, 0x00 }, // scan mode or binning
    { 0x3302, 0x10 }, //black level
    { 0x3199, 0x00 }, // HADD VADD
    { 0x3030, 0x94 }, // V span max
    { 0x3031, 0x11 },
    { 0x3032, 0x00 },
    { 0x3034, 0x26 }, // H span max
    { 0x3035, 0x02 },
    { 0x304c, 0x14 }, // V OB width
    { 0x304e, 0x00 }, // H dir inv
    { 0x304f, 0x00 }, // V dir inv
    { 0x3058, 0xac }, // shutter sweep time, 1000 ET Line
    { 0x3059, 0x0d },
    { 0x305a, 0x00 },
    { 0x30e8, 0x00 },
    { 0x30e9, 0x00 },
    { 0x3056, 0x48 }, // effective pixel line
    { 0x3057, 0x04 },
    { 0x302c, 0x80 }, // crop mode H start  384
    { 0x302d, 0x01 },

    { 0x302e, 0x80 }, // crop mode H size, 1944(798) - > 1920(780)
    { 0x302f, 0x07 },

    { 0x3072, 0x28 }, // crop mode OB V size
    { 0x3073, 0x00 },
    { 0x3074, 0x60 }, // crop mode UL V start
    { 0x3075, 0x02 },
    { 0x3076, 0x90 }, // crop mode V size, 2192
    { 0x3077, 0x08 },
    { 0x30c6, 0x12 }, // crop mode black offset
    { 0x30c7, 0x00 },
    { 0x30ce, 0x64 }, // crop mode UNRD_LINE_MAX
    { 0x30cf, 0x00 },
    { 0x30d8, 0xc0 }, // crop mode UNREAD_ED_ADR
    { 0x30d9, 0x0b },
    { 0, 0x00 }
};
static const struct  reg_list imx335_mipi_2lane_raw12_2592x1944_30fps_mclk_74_25_regs[] = {
    { 0x3000, 0x01 },
    { 0x3002, 0x00 },
    { 0x3a01, 0x01 }, // 2 lane
    { 0x300c, 0xb6 }, // lane 24MHz
    { 0x300d, 0x7f },
    { 0x314c, 0x80 },
    { 0x314d, 0x00 },
    { 0x315a, 0x03 },
    { 0x3168, 0x68 },
    { 0x316a, 0x7f },
    { 0x319e, 0x01 },    //1188Mbps
    { 0x3a18, 0x8f }, // mipi phy
    { 0x3a19, 0x00 },
    { 0x3a1a, 0x4f },
    { 0x3a1b, 0x00 },
    { 0x3a1c, 0x47 },
    { 0x3a1d, 0x00 },
    { 0x3a1e, 0x37 },
    { 0x3a1f, 0x01 },
    { 0x3a20, 0x4f },
    { 0x3a21, 0x00 },
    { 0x3a22, 0x87 },
    { 0x3a23, 0x00 },
    { 0x3a24, 0x4f },
    { 0x3a25, 0x00 },
    { 0x3a26, 0x7f },
    { 0x3a27, 0x00 },
    { 0x3a28, 0x3f },
    { 0x3a29, 0x00 },

    { 0x302e, 0x20 }, // crop mode H size, 2616(a38) - > 2592(a20)
    { 0x302f, 0x0a },

    { 0x3302, 0x10 }, //black level
    { 0x3058, 0xac }, // shutter sweep time, 1000 ET Line
    { 0x3059, 0x0d },
    { 0x305a, 0x00 },
    { 0x30e8, 0x00 },
    { 0x30e9, 0x00 },
    { 0, 0x00 }
};

static const struct vvcam_ae_info aeinfo = {
    .frame_length = IMX335_VMAX_LINEAR,
    .cur_frame_length = IMX335_VMAX_LINEAR,
    .one_line_exp_time = 0.000007407,
    .gain_accuracy = 1024,

    .min_gain = 1.0,
    .max_gain = 50.0,

    .int_time_delay_frame = 1,
    .gain_delay_frame = 1,
    .color_type = 0,

    .integration_time_increment = 0.000007407,
    .gain_increment =IMX335_AGAIN_STEP,

    .max_long_integraion_line =  IMX335_VMAX_LINEAR - 9,
    .min_long_integraion_line = 1,

    .max_integraion_line =  IMX335_VMAX_LINEAR - 9,
    .min_integraion_line = 1,

    .max_vs_integraion_line = IMX335_VMAX_LINEAR - 9,
    .min_vs_integraion_line = 1,

    .max_long_integraion_time = 0.000007407 * (IMX335_VMAX_LINEAR - 9),
    .min_long_integraion_time = 0.000007407 * 1,

    .max_integraion_time = 0.000007407 * (IMX335_VMAX_LINEAR - 9),
    .min_integraion_time = 0.000007407 * 1,

    .max_vs_integraion_time = 0.000007407 * (IMX335_VMAX_LINEAR - 9),
    .min_vs_integraion_time = 0.000007407 * 1,

    .cur_long_integration_time = 0.0,
    .cur_integration_time = 0.0,
    .cur_vs_integration_time = 0.0,

    .cur_long_again = 0.0,
    .cur_long_dgain = 0.0,

    .cur_again = 0.0,
    .cur_dgain = 0.0,

    .cur_vs_again = 0.0,
    .cur_vs_dgain = 0.0,

    .a_long_gain.min = 1.0,
    .a_long_gain.max = 100.0,
    .a_long_gain.step = (1.0f / 256.0f),

    .a_gain.min = 1.0,
    .a_gain.max = 100.0,
    .a_gain.step = (1.0f / 256.0f),

    .a_vs_gain.min = 1.0,
    .a_vs_gain.max = 100.0,
    .a_vs_gain.step = (1.0f / 256.0f),

    .d_long_gain.max = 1.0,
    .d_long_gain.min = 1.0,
    .d_long_gain.step = (1.0f / 1024.0f),

    .d_gain.max = 1.0,
    .d_gain.min = 1.0,
    .d_gain.step = (1.0f/1024.0f),

    .d_vs_gain.max = 1.0,
    .d_vs_gain.min = 1.0,
    .d_vs_gain.step = (1.0f / 1024.0f),
    .cur_fps = 30,
};
static struct imx335_mode modes[] = {
     {
        .mode = {
            .clk = 24000000,
            .width = 1920,
            .height = 1080,
            .lanes = VVCAM_SENSOR_2LANE,
            .freq = VVCAM_SENSOR_1200M,
            .bayer = VVCAM_BAYER_PAT_RGGB,
            .bit_width = 12,
            .ae_info = aeinfo,
        },
        .regs = imx335_mipi_2lane_raw12_1920x1080_30fps_mclk_24m_regs,//
    },
    {
        .mode = {
            .clk = 24000000,
            .width = 2592,
            .height = 1944,
            .lanes = VVCAM_SENSOR_2LANE,
            .freq = VVCAM_SENSOR_1200M,
            .bayer = VVCAM_BAYER_PAT_RGGB,
            .bit_width = 12,
            .ae_info = aeinfo,
        },
        .regs = imx335_mipi_2lane_raw12_2592x1944_30fps_mclk_24m_regs,//
    },
    {
        .mode = {
            .clk = 74250000,
            .width = 1920,
            .height = 1080,
            .lanes = VVCAM_SENSOR_2LANE,
            .freq = VVCAM_SENSOR_1200M,
            .bayer = VVCAM_BAYER_PAT_RGGB,
            .bit_width = 12,
            .ae_info = aeinfo,
        },
        .regs = imx335_mipi_2lane_raw12_1920x1080_30fps_mclk_74_25_regs,//
    },
    {
    .mode = {
        .clk = 74250000,
        .width = 2592,
        .height = 1944,
        .lanes = VVCAM_SENSOR_2LANE,
        .freq = VVCAM_SENSOR_1200M,
        .bayer = VVCAM_BAYER_PAT_RGGB,
        .bit_width = 12,
        .ae_info = aeinfo,
    },
    .regs = imx335_mipi_2lane_raw12_2592x1944_30fps_mclk_74_25_regs,//
    }

};

static int read_reg(struct imx335_ctx* ctx, uint16_t addr, uint8_t* value) {
    struct i2c_msg msg[2];
    struct i2c_rdwr_ioctl_data data;

    addr = htobe16(addr);
    msg[0].addr = I2C_SLAVE_ADDRESS_IMX335;
    msg[0].buf = (uint8_t*)&addr;
    msg[0].len = 2;
    msg[0].flags = 0;

    msg[1].addr = I2C_SLAVE_ADDRESS_IMX335;
    msg[1].buf = value;
    msg[1].len = 1;
    msg[1].flags = I2C_M_RD;

    data.msgs = msg;
    data.nmsgs = 2;

    int ret = ioctl(ctx->i2c, I2C_RDWR, &data);
    if (ret != 2) {
        fprintf(stderr, "imx335: i2c read reg %04x error %d(%s)\n", be16toh(addr), errno, strerror(errno));
        return -1;
    }

    return 0;
}

static int write_reg(struct imx335_ctx* ctx, uint16_t addr, uint8_t value) {
    uint8_t buffer[3];

    buffer[0] = (addr >> 8) & 0xff;
    buffer[1] = (addr >> 0) & 0xff;
    buffer[2] = value;
    if (write(ctx->i2c, buffer, 3) != 3) {
        fprintf(stderr, "imx335: i2c write reg %04x error %d(%s)\n", addr,  errno, strerror(errno));
        return -1;
    }
    // printf("imx335 w %04x %02x\n", addr, value);
    return 0;
}

static int open_i2c(struct imx335_ctx* sensor) {

    // i2c
    if (sensor->i2c < 0) {
        sensor->i2c = open("/dev/i2c-0", O_RDWR);
        if (sensor->i2c < 0) {
            perror("open /dev/i2c-0");
            return -1;
        }
        //printf("I2C_SLAVE_ADDRESS_IMX335=%x\n", I2C_SLAVE_ADDRESS_IMX335);
        if (ioctl(sensor->i2c, I2C_SLAVE_FORCE, I2C_SLAVE_ADDRESS_IMX335) < 0) {
            perror("i2c ctrl 0x36");
            return -1;
        }
    }
    return 0;
}

static int init(void** ctx) {
    struct imx335_ctx* sensor = calloc(1, sizeof(struct imx335_ctx));
    sensor->i2c = -1;
    *ctx = sensor;

    return 0;
}

static void deinit(void* ctx) {
    struct imx335_ctx* sensor = ctx;
    close(sensor->i2c);
    free(ctx);
}

static int enum_mode(void* ctx, uint32_t index, struct vvcam_sensor_mode* mode) {
    //printf("f=%s l=%d index =%d\n", __func__, __LINE__,index);
    if (index >= ARRAY_SIZE(modes)) {
        // out of range
        return -1;
    }
    memcpy(mode, &modes[index].mode, sizeof(struct vvcam_sensor_mode));

    return 0;
}

static int get_mode(void* ctx, struct vvcam_sensor_mode* mode) {
    struct imx335_ctx* sensor = ctx;
    // printf("imx335 %s   \n", __func__);
    memcpy(mode, &sensor->mode, sizeof(struct vvcam_sensor_mode));
    return 0;
}

static int set_mode(void* ctx, uint32_t index) {
    struct imx335_ctx* sensor = ctx;
    //printf("f=%s l=%d index =%d\n", __func__, __LINE__,index);

    if (index >= ARRAY_SIZE(modes)) {
        // out of range
        return -1;
    }
    struct vvcam_sensor_mode* mode = &modes[index].mode;

    printf("imx335: %s %ux%u\n", __func__, mode->width, mode->height);
    if (open_i2c(sensor)) {
        return -1;
    }


    for(unsigned i = 0;; i++) {
        if ((modes[index].regs[i].addr == 0) && (modes[index].regs[i].value == 0)) {
            break;
        }
        CHECK_ERROR(write_reg(sensor, modes[index].regs[i].addr, modes[index].regs[i].value));
    }
    //printf("wjxxx f=%s l=%d\n", __func__, __LINE__ );
    uint8_t again_h, again_l;
    uint8_t exp_time_h, exp_time_l;
    uint8_t exp_time;
    float again = 0, dgain = 0;

    CHECK_ERROR(read_reg(ctx, imx335_REG_DGAIN_H, &again_h));
    CHECK_ERROR(read_reg(ctx, imx335_REG_DGAIN_L, &again_l));
    again = (float)((again_h & 0x07)<<8 | again_l) * 0.015f;
    again = powf(10, again);    //times value
    sensor->sensor_again = (uint16_t)(log10f(again)*200.0f/3.0f + 0.5f);
    //sensor->et_line = 0;

    again = 1.0;
    dgain = 1.0;
    mode->ae_info.cur_gain = again * dgain;
    mode->ae_info.cur_long_gain = mode->ae_info.cur_gain;
    mode->ae_info.cur_vs_gain = mode->ae_info.cur_gain;


    //if(current_mode->hdr_mode == SENSOR_MODE_LINEAR)
	{
	    uint16_t SHR0_m, SHR0_l;
	    uint32_t exp_time;
	    read_reg(ctx, IMX335_REG_SHR0_L, (uint8_t*)&SHR0_l);
	    read_reg(ctx, IMX335_REG_SHR0_M, (uint8_t*)&SHR0_m);
	    exp_time =IMX335_VMAX_LINEAR - ((SHR0_m <<8) | SHR0_l);

	    mode->ae_info.cur_integration_time =  mode->ae_info.one_line_exp_time * exp_time;
        sensor->et_line = exp_time;
  	}

    // save current mode
    memcpy(&sensor->mode , mode, sizeof(struct vvcam_sensor_mode));

    return 0;
}

static int set_stream(void* ctx, bool on) {
    //printf("wjxxx f=%s l=%d\n", __func__, __LINE__ );

    struct imx335_ctx* sensor = ctx;
    int ret;
    printf("imx335  %s %d\n", __func__, on);
    if (open_i2c(sensor)) {
        return -1;
    }
    if (on) {
        ret = write_reg(ctx, IMX335_REG_MODE_SELECT, IMX335_MODE_STREAMING);
    } else {
        ret = write_reg(ctx, IMX335_REG_MODE_SELECT, IMX335_MODE_STREAMING);
    }
    return ret;
}

static int set_analog_gain(void* ctx, float gain) {
    //printf("wjxxx f=%s l=%d\n", __func__, __LINE__ );
    struct imx335_ctx* sensor = ctx;
    uint32_t again;//, dgain, total;;
    //uint32_t i = 0;
    float SensorGain;
    int ret;

    //printf("imx335 %s %f\n", __func__, gain);

    again = (uint16_t)(log10f(gain)*200.0f/3.0f + 0.5f);     //20*log(gain)*10/3
    if(sensor->sensor_again !=again)
    {
        ret = write_reg(ctx, IMX335_REG_AGAIN_L,(again & 0xff));
        ret |= write_reg(ctx, IMX335_REG_AGAIN_H,(again & 0x0700)>>8);
        sensor->sensor_again = again;
    }
    SensorGain = (float)(sensor->sensor_again) * 0.015f;    //db value/20,(RegVal * 3/10)/20
    sensor->mode.ae_info.cur_again = powf(10, SensorGain);

    return 0;
}

static int set_digital_gain(void* ctx, float gain) {
    //printf("wjxxx f=%s l=%d gain=%0.9f \n", __func__, __LINE__, gain);

    struct imx335_ctx* sensor = ctx;
    struct vvcam_sensor_mode *current_mode = &sensor->mode;
    //k_s32 ret = 0;
    uint32_t dgain;

    dgain = (uint32_t)(gain * 1024);
    current_mode->ae_info.cur_dgain = dgain / 1024.0f;

    current_mode->ae_info.cur_gain = current_mode->ae_info.cur_again * current_mode->ae_info.cur_dgain;
    current_mode->ae_info.cur_long_gain = current_mode->ae_info.cur_gain;
    current_mode->ae_info.cur_vs_gain = current_mode->ae_info.cur_gain;
    return 0;
}

static int set_int_time(void* ctx, float time) {
    //printf("wjxxx f=%s l=%d\n", __func__, __LINE__ );

    struct imx335_ctx* sensor = ctx;
    uint16_t exp_line = 0;
    float integraion_time = 0;
    int ret;

    // printf("imx335 %s %f\n", __func__, time);

    integraion_time = time;

    exp_line = integraion_time / sensor->mode.ae_info.one_line_exp_time;
    exp_line = MIN(sensor->mode.ae_info.max_integraion_line, MAX(sensor->mode.ae_info.min_integraion_line, exp_line));
    if (sensor->et_line != exp_line)
    {
        uint16_t SHR0 = IMX335_VMAX_LINEAR - exp_line;
        ret = write_reg(ctx, IMX335_REG_SHR0_L, SHR0 & 0xff);
        ret |= write_reg(ctx, IMX335_REG_SHR0_M, (SHR0 >> 8) & 0xff);
        sensor->et_line = exp_line;
    }
    sensor->mode.ae_info.cur_integration_time = (float)sensor->et_line * sensor->mode.ae_info.one_line_exp_time;
    return 0;
}

struct vvcam_sensor vvcam_imx335 = {
    .name = "imx335",
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
