#include <vvcam_mclk.h>

#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/file.h>
#include <sys/mman.h>
#include <unistd.h>

#define SYSCTL_BASE   0x91100000UL
#define MCLK_OFFSET   0x6c
#define MCLK_LOCK     "/tmp/k230-mclk.lock"
#define MCLK_STROBE   (1u << 31)

union mclk_reg {
    struct {
        uint32_t mclk0_enable : 1;
        uint32_t mclk1_enable : 1;
        uint32_t mclk2_enable : 1;
        uint32_t mclk0_sel : 2;
        uint32_t mclk0_div : 5;
        uint32_t mclk1_sel : 2;
        uint32_t mclk1_div : 5;
        uint32_t mclk2_sel : 2;
        uint32_t mclk2_div : 5;
        uint32_t reserved : 7;
        uint32_t mclk0_cdiv_upd : 1;
    } bits;
    uint32_t value;
};

static pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;
static int g_mem_fd = -1;
static volatile uint32_t *g_sysctl;
static int g_lock_fd = -1;

static int map_sysctl(void)
{
    if (g_sysctl)
        return 0;

    if (g_mem_fd < 0) {
        g_mem_fd = open("/dev/mem", O_RDWR | O_SYNC);
        if (g_mem_fd < 0) {
            perror("vvcam_mclk: open /dev/mem");
            return -1;
        }
    }

    g_sysctl = mmap(NULL, 0x1000, PROT_READ | PROT_WRITE, MAP_SHARED,
                    g_mem_fd, SYSCTL_BASE);
    if (g_sysctl == MAP_FAILED) {
        g_sysctl = NULL;
        perror("vvcam_mclk: mmap sysctl");
        return -1;
    }

    return 0;
}

static int lock_mclk(void)
{
    pthread_mutex_lock(&g_mutex);

    if (g_lock_fd < 0) {
        g_lock_fd = open(MCLK_LOCK, O_CREAT | O_RDWR, 0666);
        if (g_lock_fd < 0) {
            pthread_mutex_unlock(&g_mutex);
            perror("vvcam_mclk: open lock");
            return -1;
        }
    }

    if (flock(g_lock_fd, LOCK_EX) < 0) {
        pthread_mutex_unlock(&g_mutex);
        perror("vvcam_mclk: flock");
        return -1;
    }

    return 0;
}

static void unlock_mclk(void)
{
    if (g_lock_fd >= 0)
        flock(g_lock_fd, LOCK_UN);
    pthread_mutex_unlock(&g_mutex);
}

static union mclk_reg read_mclk(void)
{
    union mclk_reg reg;

    reg.value = g_sysctl[MCLK_OFFSET / 4];
    return reg;
}

static void write_mclk(union mclk_reg reg)
{
    g_sysctl[MCLK_OFFSET / 4] = reg.value | MCLK_STROBE;
    g_sysctl[MCLK_OFFSET / 4] = reg.value & ~MCLK_STROBE;
}

static int parent_to_hw_sel(uint8_t sel)
{
    switch (sel) {
    case VVCAM_PLL0_CLK_DIV4:
        return 2; /* 400 MHz */
    case VVCAM_PLL1_CLK_DIV3:
        return 1; /* 792 MHz */
    case VVCAM_PLL1_CLK_DIV4:
        return 0; /* 594 MHz */
    case 0:
    case 1:
    case 2:
        return sel;
    default:
        return -1;
    }
}

static void set_channel(union mclk_reg *reg, uint8_t mclk_id, int enable,
                        int hw_sel, uint8_t div)
{
    switch (mclk_id) {
    case VVCAM_MCLK0:
        reg->bits.mclk0_enable = enable ? 1 : 0;
        if (enable) {
            reg->bits.mclk0_sel = hw_sel;
            reg->bits.mclk0_div = div;
        }
        break;
    case VVCAM_MCLK1:
        reg->bits.mclk1_enable = enable ? 1 : 0;
        if (enable) {
            reg->bits.mclk1_sel = hw_sel;
            reg->bits.mclk1_div = div;
        }
        break;
    case VVCAM_MCLK2:
        reg->bits.mclk2_enable = enable ? 1 : 0;
        if (enable) {
            reg->bits.mclk2_sel = hw_sel;
            reg->bits.mclk2_div = div;
        }
        break;
    default:
        break;
    }
}

int vvcam_mclk_apply(uint8_t mclk_id, const struct vvcam_mclk_setting *setting)
{
    union mclk_reg reg;
    int hw_sel;
    uint8_t div;

    if (!setting || mclk_id > VVCAM_MCLK2)
        return -EINVAL;

    if (!setting->enable)
        return vvcam_mclk_disable(mclk_id);

    hw_sel = parent_to_hw_sel(setting->sel);
    if (hw_sel < 0) {
        fprintf(stderr, "vvcam_mclk: unsupported sel %u\n", setting->sel);
        return -EINVAL;
    }

    div = setting->div;
    if (div < 1)
        div = 1;
    if (div > 31)
        div = 31;

    if (lock_mclk())
        return -1;
    if (map_sysctl()) {
        unlock_mclk();
        return -1;
    }

    reg = read_mclk();
    set_channel(&reg, mclk_id, 1, hw_sel, div);
    write_mclk(reg);

    fprintf(stderr,
            "vvcam_mclk: apply id=%u en=1 sel=%u(hw=%d) div=%u reg=0x%08x\n",
            mclk_id, setting->sel, hw_sel, div, reg.value);

    unlock_mclk();
    /* OV5647/GC2093 need XCLK stable before SCCB; 8192 clocks @25MHz < 1ms */
    usleep(10000);
    return 0;
}

int vvcam_mclk_disable(uint8_t mclk_id)
{
    union mclk_reg reg;

    if (mclk_id > VVCAM_MCLK2)
        return -EINVAL;

    if (lock_mclk())
        return -1;
    if (map_sysctl()) {
        unlock_mclk();
        return -1;
    }

    reg = read_mclk();
    set_channel(&reg, mclk_id, 0, 0, 0);
    write_mclk(reg);

    fprintf(stderr, "vvcam_mclk: disable id=%u reg=0x%08x\n", mclk_id, reg.value);

    unlock_mclk();
    return 0;
}
