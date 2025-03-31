/*
 * Copyright (c) CompanyNameMagicTag 2020-2020. All rights reserved.
 * Description: efuse driver.
 * Author: Huanghe
 * Create: 2020-01-01
 */

#include "efuse_opt.h"
#include "hal_soc_reg.h"
#include "oal_types.h"
#include "securec.h"
#include "hcc_cfg.h"
#include "hcc_bus.h"
#include "hcc_if.h"
#include "customize.h"
#include "oal_debug.h"
#include "plat_hcc_srv.h"
#include "plat_hcc_msg_type.h"
#include "plat_misc.h"

#define EFUSE_DEBUG oal_print_err

#define EFUSE_READ_MAX_BYTES   32

#ifdef __cplusplus
#if __cplusplus
    extern "C" {
#endif
#endif

/* ****************************************************************************************** */
#ifndef CONFIG_HCC_SUPPORT_UART

#define EFUSE_READ_MAX_BYTES   32

static osal_u32 efuse_wait_busy_status(osal_void)
{
    osal_s32 times = RETRY_TIMES;
    u_efuse_ctrl_st efuse_ctrl_st = {0};
    ext_errno err;

    while (times) {
        err = hcc_read_reg(HCC_CHANNEL_AP, EFUSE_CTRL_RB_EFUSE_CTRL_ST_REG, &(efuse_ctrl_st.u32));
        if (err != EXT_ERR_SUCCESS) {
            return OAL_FAIL;
        }

        if (efuse_ctrl_st.bits.efuse_ctrl_busy == 0) {
            break;
        }
        times--;
        osal_udelay(1);
    }
    if (!times) {
        return OAL_FAIL;
    }
    return OAL_SUCC;
}

static osal_u32 efuse_check_status(osal_void)
{
    u_efuse_ctrl_st efuse_ctrl_st = {0};
    osal_s32 times = RETRY_TIMES;
    ext_errno err;

    while (times) {
        err = hcc_read_reg(HCC_CHANNEL_AP, EFUSE_CTRL_RB_EFUSE_CTRL_ST_REG, &(efuse_ctrl_st.u32));
        if (err != EXT_ERR_SUCCESS) {
            return OAL_FAIL;
        }

        if (efuse_ctrl_st.bits.efuse_boot_st == 1) {
            break;
        }
        osal_udelay(1);
        times--;
    }
    if (!times) {
        return OAL_FAIL;
    }
    return efuse_wait_busy_status();
}

static osal_u32 efuse_wait_read_completion(osal_void)
{
    td_s32 times = RETRY_TIMES;
    u_efuse_ctrl_st efuse_ctrl_st = {0};
    ext_errno err;

    while (times) {
        err = hcc_read_reg(HCC_CHANNEL_AP, EFUSE_CTRL_RB_EFUSE_CTRL_ST_REG, &(efuse_ctrl_st.u32));
        if (err != EXT_ERR_SUCCESS) {
            return OAL_FAIL;
        }

        if (efuse_ctrl_st.bits.efuse_rd_st == 1 && efuse_ctrl_st.bits.efuse_ctrl_busy == 0) {
            break;
        }
        osal_udelay(1);
        times--;
    }
    if (!times) {
        return OAL_FAIL;
    }
    return OAL_SUCC;
}

static osal_u32 efuse_normal_read(osal_u32 addr, osal_u8 *data)
{
    td_u32 readdata;
    ext_errno err;

    if (efuse_check_status() != OAL_SUCC) {
        return OAL_FAIL;
    }

    err = hcc_write_reg(HCC_CHANNEL_AP, EFUSE_CTRL_RB_EFUSE_WR_ADDR_REG, (addr >> BIT_TO_BYTE));
    if (err != EXT_ERR_SUCCESS) {
        return OAL_FAIL;
    }

    err = hcc_write_reg(HCC_CHANNEL_AP, EFUSE_CTRL_RB_EFUSE_MODE_EN_REG, READ_EN);
    if (err != EXT_ERR_SUCCESS) {
        return OAL_FAIL;
    }

    if (efuse_wait_read_completion() != OAL_SUCC) {
        return OAL_FAIL;
    }

    err = hcc_read_reg(HCC_CHANNEL_AP, EFUSE_CTRL_RB_EFUSE_RDATA_REG, &readdata);
    if (err != EXT_ERR_SUCCESS) {
        return OAL_FAIL;
    }

    *data = (osal_u8)readdata;
    return OAL_SUCC;
}

static osal_u32 efuse_normal_write(osal_u32 addr)
{
    u_efuse_ctrl_st efuse_ctrl_st = {0};
    ext_errno err;

    if (efuse_check_status() != OAL_SUCC) {
        return OAL_FAIL;
    }

    err = hcc_write_reg(HCC_CHANNEL_AP, EFUSE_CTRL_RB_EFUSE_WR_ADDR_REG, addr);
    if (err != EXT_ERR_SUCCESS) {
        return OAL_FAIL;
    }

    err = hcc_write_reg(HCC_CHANNEL_AP, EFUSE_CTRL_RB_EFUSE_MODE_EN_REG, WRITE_EN);
    if (err != EXT_ERR_SUCCESS) {
        return OAL_FAIL;
    }

    if (efuse_wait_busy_status() != OAL_SUCC) {
        return OAL_FAIL;
    }
    err = hcc_read_reg(HCC_CHANNEL_AP, EFUSE_CTRL_RB_EFUSE_CTRL_ST_REG, &(efuse_ctrl_st.u32));
    if (err != EXT_ERR_SUCCESS) {
        return OAL_FAIL;
    }

    if (efuse_ctrl_st.bits.efuse_pgm_err == 1) {
        return OAL_FAIL;
    } else {
        if (efuse_ctrl_st.bits.efuse_pgm_st == 0) {
            return OAL_FAIL;
        }
    }
    return OAL_SUCC;
}

static osal_u32 efuse_drv_read(osal_u16 start_bit, osal_u16 bit_cnt, osal_u8 *data, osal_u8 data_len)
{
    osal_u32 ret;
    osal_u16 start_index;
    osal_u16 end_index;
    osal_u16 read_times;
    osal_u16 start_bit_aligned;
    osal_u16 start_byte_offset;
    osal_u8 tmpdata[EFUSE_READ_MAX_BYTES + 2] = {0};
    osal_u16 index = 0;

    if ((bit_cnt > (EFUSE_READ_MAX_BYTES * SIZE_8_BITS)) ||
        ((start_bit + bit_cnt - 1) > EFUSE_MAX_INDEX) ||
        (data == NULL || data_len == 0)) {
        return OAL_FAIL;
    }
    if (((bit_cnt + MAX_OFFSET_COMP) / SIZE_8_BITS) > data_len) {
        return OAL_FAIL;
    }
    (osal_void)memset_s(data, data_len, 0, data_len);
    start_index = start_bit / SIZE_8_BITS;
    start_byte_offset = start_bit % SIZE_8_BITS;
    end_index = (start_bit + bit_cnt - 1) / SIZE_8_BITS;
    read_times = end_index - start_index + 1;
    start_bit_aligned = start_bit - start_byte_offset;
    while (read_times > 0) {
        ret = efuse_normal_read((osal_u32)start_bit_aligned, &(tmpdata[index]));
        if (ret != OAL_SUCC) {
            return ret;
        }
        index++;
        read_times--;
        start_bit_aligned = start_bit_aligned + SIZE_8_BITS;
    }

    for (index = 0; index < bit_cnt; index++) {
        osal_u16 byte_pos = index / SIZE_8_BITS;
        osal_u16 byte_offset = index  % SIZE_8_BITS;
        if ((tmpdata[(index + start_byte_offset) / SIZE_8_BITS] >> ((index + start_bit) % SIZE_8_BITS)) & 0x1) {
            data[byte_pos] |= 1 << byte_offset;
        } else {
            data[byte_pos] &= ~(1 << byte_offset);
        }
    }

    return ret;
}

// start_bit:写bit起始位，size:写的bit数，data:写的数据，data_len：size的8位向上取整个byte
static osal_u32 efuse_drv_write(osal_u16 start_bit, osal_u16 bit_cnt, osal_u8 *data, osal_u8 data_len)
{
    osal_u32 index;
    osal_u32 ret = OAL_SUCC;
    if (data == NULL || bit_cnt == 0 || (start_bit + bit_cnt - 1) > EFUSE_MAX_INDEX) {
        return OAL_FAIL;
    }
    if (((bit_cnt + MAX_OFFSET_COMP) / SIZE_8_BITS) > data_len) {
        return OAL_FAIL;
    }
    for (index = 0; index < bit_cnt; index++) {
        if (data[index / SIZE_8_BITS] & (1 << (index % SIZE_8_BITS))) {
            ret = efuse_normal_write((osal_u32)(start_bit + index));
            if (ret != OAL_SUCC) {
                break;
            }
        }
    }
    return ret;
}

/* ****************************************************************************************** */
typedef struct {
    td_u16 id_start_bit;    /* 起始 bit位 */
    td_u16 id_size;         /* 以bit为单位 */
} efuse_stru;

const efuse_stru g_efuse_cfg[EXT_EFUSE_MAX] = {
    { 0,     8 },
    { 8,   144 },
    { 152,   8 },
    { 160,   8 },
    { 168,   4 },
    { 172,   2 },
    { 174,   6 },
    { 180,  12 },
    { 192,  12 },
    { 204,   4 },
    { 208,   4 },
    { 212,   4 },
    { 216,   4 },
    { 220,   4 },
    { 224,   4 },
    { 228,   4 },
    { 232,   4 },
    { 236,   4 },
    { 240,   4 },
    { 244,   4 },
    { 248,   4 },
    { 252,   4 },
    { 256,   4 },
    { 260,   8 },
    { 268,   4 },
    { 272,   2 },
    { 274,   2 },
    { 276,   2 },
    { 278,   2 },
    { 280,   4 },
    { 284,   4 },
    { 288,   8 },
    { 296,  16 },
    { 312,  16 },
    { 328,   8 },
    { 336,   8 },
    { 344,  16 },
    { 360,   8 },
    { 368,   5 },
    { 373,  53 },
    { 426,  16 },
    { 442,   5 },
    { 447,   1 },
    { 448,   8 },
    { 456,  24 },
    { 480,  64 },
    { 544,   8 },
    { 552,   4 },
    { 556,   4 },
    { 560,  16 },
    { 576,  16 },
    { 592,  16 },
    { 608,  16 },
    { 624,  16 },
    { 640,  16 },
    { 656,   5 },
    { 661,   5 },
    { 666,   5 },
    { 671,   1 },
    { 672,   8 },
    { 680,   4 },
    { 684,   4 },
    { 688,   16 },
    { 704,   16 },
    { 720,   16 },
    { 736,   16 },
    { 752,   16 },
    { 768,   16 },
    { 784,    5 },
    { 789,    5 },
    { 794,    5 },
    { 799,    1 },
    { 800,   48 },
    { 848,   48 },
    { 896,   48 },
    { 944,   16 },
    { 960,   16 },
    { 976,   1 },
    { 977,   1 },
    { 978,   1 },
    { 979,   1 },
    { 980,   1 },
    { 981,   1 },
    { 982,   1 },
    { 983,   1 },
    { 984,   1 },
    { 985,   1 },
    { 986,   1 },
    { 987,   1 },
    { 988,   1 },
    { 989,   1 },
    { 990,   1 },
    { 991,   1 },
    { 992,   1 },
    { 993,   1 },
    { 994,   1 },
    { 995,   1 },
    { 996,   1 },
    { 997,   1 },
    { 998,   1 },
    { 999,   1 },
    { 1000,  1 },
    { 1001,  1 },
    { 1002,  1 },
    { 1003,  1 },
    { 1004,  1 },
    { 1005,  1 },
    { 1006,  1 },
    { 1007,  1 },
    { 1008,  1 },
    { 1009,  1 },
    { 1010,  1 },
    { 1011,  1 },
    { 1012,  1 },
    { 1013,  1 },
    { 1014,  1 },
    { 1015,  1 },
    { 1016,  1 },
    { 1017,  1 },
    { 1018,  1 },
    { 1019,  1 },
    { 1020,  1 },
    { 1021,  1 },
    { 1022,  1 },
    { 1023,  1 },
    { 496,   48},
    { 976,   48},
    { 448,   32},
    { 994,   1 },
};

osal_u32 uapi_efuse_read(osal_efuse_id efuse_id, osal_u8 *data, osal_u8 data_len)
{
    td_u16 start_bit;
    td_u16 size;

    if (efuse_id >= EXT_EFUSE_MAX || data == NULL || data_len == 0) {
        return OAL_FAIL;
    }
    start_bit = g_efuse_cfg[efuse_id].id_start_bit;
    size = g_efuse_cfg[efuse_id].id_size;
    return efuse_drv_read(start_bit, size, data, data_len);
}
osal_module_export(uapi_efuse_read);

osal_u32 uapi_efuse_write(osal_efuse_id efuse_id, osal_u8 *data, osal_u8 len)
{
    td_u16 start_bit;
    td_u16 size;

    td_u16 bit_size = len * SIZE_8_BITS;
    if (efuse_id >= EXT_EFUSE_MAX || data == NULL || len == 0) {
        return OAL_FAIL;
    }
    start_bit = g_efuse_cfg[efuse_id].id_start_bit;
    size = g_efuse_cfg[efuse_id].id_size;
    size = osal_min(size, bit_size);
    return efuse_drv_write(start_bit, size, data, len);
}
osal_module_export(uapi_efuse_write);
#endif

osal_u32 efuse_write_dev_addr(osal_u8 *pc_addr, osal_u8 addr_len)
{
    osal_u8 zero_mac[WLAN_MAC_ADDR_LEN] = {0};
    osal_u8 tmp_mac[WLAN_MAC_ADDR_LEN] = {0};
    osal_u32 ret = OAL_FAIL;
    osal_u32 index;

    for (index = 0; index < EFUSE_MAC_NUM; index++) {
        if ((uapi_efuse_read(EXT_EFUSE_IPV4_MAC_ADDR_01_ID + index, tmp_mac, WLAN_MAC_ADDR_LEN) == OAL_SUCC) &&
                (memcmp(tmp_mac, zero_mac, WLAN_MAC_ADDR_LEN) == 0)) {
            ret = uapi_efuse_write(EXT_EFUSE_IPV4_MAC_ADDR_01_ID + index, pc_addr, addr_len);
            if (ret == OAL_FAIL) {
                return ret;
            }
            return uapi_efuse_lock(EXT_EFUSE_LOCK_IPV4_MAC_ADDR_01_ID + index);
        }
    }
    return ret;
}
osal_module_export(efuse_write_dev_addr);

osal_u32 efuse_write_spark_addr(osal_u8 *pc_addr, osal_u8 addr_len)
{
    osal_u8 zero_mac[WLAN_MAC_ADDR_LEN] = {0};
    osal_u8 tmp_mac[WLAN_MAC_ADDR_LEN] = {0};
    osal_u32 ret = OAL_FAIL;
    if ((uapi_efuse_read(EXT_EFUSE_SLE_EFUSE_ID, tmp_mac, WLAN_MAC_ADDR_LEN) == OAL_SUCC) &&
            (memcmp(tmp_mac, zero_mac, WLAN_MAC_ADDR_LEN) == 0)) {
        return uapi_efuse_write(EXT_EFUSE_SLE_EFUSE_ID, pc_addr, addr_len);
    }
    return ret;
}
osal_module_export(efuse_write_spark_addr);

osal_u32 efuse_write_pid_vid(osal_u16 pid, osal_u16 vid)
{
    osal_u16 read_data = 0;
    if ((uapi_efuse_read(EXT_EFUSE_LOCK_USB_SDIO_PID_ID,
        (osal_u8 *)(&read_data), sizeof(read_data)) == OAL_SUCC) && (read_data == 0)) {
        if (uapi_efuse_write(EXT_EFUSE_USB_SDIO_PID_ID, (osal_u8 *)(&pid), sizeof(pid)) == OAL_FAIL) {
            return OAL_FAIL;
        }
    } else {
        return OAL_FAIL;
    }

    if ((uapi_efuse_read(EXT_EFUSE_LOCK_USB_SDIO1_VID_ID,
        (osal_u8 *)(&read_data), sizeof(read_data)) == OAL_SUCC) && (read_data == 0)) {
        if (uapi_efuse_write(EXT_EFUSE_USB_SDIO1_VID_ID, (osal_u8 *)(&vid), sizeof(pid)) == OAL_FAIL) {
            return OAL_FAIL;
        }
    } else {
        return OAL_FAIL;
    }
    return uapi_efuse_lock(EXT_EFUSE_LOCK_USB_SDIO_PID_ID);
}
osal_module_export(efuse_write_pid_vid);

osal_u32 uapi_efuse_get_lockstat(osal_u64 *state)
{
    osal_u32 ret;
    if (state == NULL) {
        return OAL_FAIL;
    }
    ret = uapi_efuse_read(EXT_EFUSE_ALL_LOCK_ID, (osal_u8*)state, sizeof(osal_u64));
    if (ret != OAL_SUCC) {
        EFUSE_DEBUG(" read failed!\n");
        return OAL_FAIL;
    }
    return OAL_SUCC;
}
osal_module_export(uapi_efuse_get_lockstat);

/* 锁定某个区域lock_id 0~47个区域 */
osal_u32 uapi_efuse_lock(osal_efuse_id lock_id)
{
    osal_u32 ret = 0;
    osal_u8 lock = 1;

    if (lock_id >= EXT_EFUSE_MAX) {
        return OAL_FAIL;
    }
    ret = uapi_efuse_write(lock_id, &lock, sizeof(lock));
    if (ret != OAL_SUCC) {
        EFUSE_DEBUG("uapi_efuse_lock write failed!\n");
        return OAL_FAIL;
    }

    return ret;
}
osal_module_export(uapi_efuse_lock);

#ifdef CONFIG_HCC_SUPPORT_UART
#define MAX_EFUSE_DATA_LEN 16

typedef struct {
    osal_u8 id;
    osal_u8 len;
} efuse_opt_read_msg_st;

typedef struct {
    osal_u8 id;
    osal_u8 len;
    osal_u8 data[MAX_EFUSE_DATA_LEN];
} efuse_opt_write_msg_st;

static plat_msg_ctl_struct g_efuse_read_st = {
    .data_size = 0,
    .wait_cond = OSAL_FALSE,
};

osal_u32 uapi_efuse_read(osal_efuse_id id, osal_u8 *data, osal_u8 data_len)
{
    osal_u32 ret = EXT_ERR_FAILURE;
    efuse_opt_read_msg_st msg_data;
    plat_data_len_struct input_data_st;
    plat_data_len_struct output_data_st;

    if (data == TD_NULL || data_len == 0) {
        return EXT_ERR_TRNG_INVALID_PARAMETER;
    }

    msg_data.id = id;
    msg_data.len = data_len;

    input_data_st.data = (osal_u8 *)&msg_data;
    input_data_st.data_len = sizeof(efuse_opt_read_msg_st);

    output_data_st.data = data;
    output_data_st.data_len = data_len;

    ret = send_message_to_device(&input_data_st, H2D_PLAT_CFG_MSG_READ_EFUSE,
        &g_efuse_read_st, &output_data_st);
    if (ret != EXT_ERR_SUCCESS) {
        oal_print_err("{uapi_efuse_read failed!}\r\n");
        return ret;
    }
    return EXT_ERR_SUCCESS;
}

osal_u32 plat_efuse_get_read_data_saved(osal_u8 *data, osal_u16 len)
{
    return plat_d2h_msg_callback(&g_efuse_read_st, data, len);
}

static plat_msg_ctl_struct g_efuse_write_st = {
    .data_size = 0,
    .wait_cond = OSAL_FALSE,
};

osal_u32 uapi_efuse_write(osal_efuse_id id, osal_u8 *data, osal_u8 data_len)
{
    osal_u32 ret = EXT_ERR_FAILURE;
    osal_u32 write_result;
    efuse_opt_write_msg_st msg_data;
    plat_data_len_struct input_data_st;
    plat_data_len_struct output_data_st;

    if (data == TD_NULL || data_len == 0 || data_len > MAX_EFUSE_DATA_LEN) {
        return EXT_ERR_TRNG_INVALID_PARAMETER;
    }

    msg_data.id = id;
    msg_data.len = data_len;
    ret = (osal_u32)memcpy_s(msg_data.data, MAX_EFUSE_DATA_LEN, data, data_len);
    if (ret != EXT_ERR_SUCCESS) {
        oal_print_err("host_write_efuse copy param fail %d!\r\n", ret);
    }

    input_data_st.data = (osal_u8 *)&msg_data;
    input_data_st.data_len = sizeof(efuse_opt_write_msg_st);

    output_data_st.data = (osal_u8 *)&write_result;
    output_data_st.data_len = sizeof(write_result);

    ret = send_message_to_device(&input_data_st, H2D_PLAT_CFG_MSG_WRITE_EFUSE,
        &g_efuse_write_st, &output_data_st);
    if (ret != EXT_ERR_SUCCESS) {
        oal_print_err("{uapi_efuse_write failed!}\r\n");
        return ret;
    }

    if (write_result != EXT_ERR_SUCCESS) {
        return EXT_ERR_FAILURE;
    }
    return EXT_ERR_SUCCESS;
}

osal_u32 plat_efuse_get_write_data_saved(td_u8 *data, td_u16 len)
{
    return plat_d2h_msg_callback(&g_efuse_write_st, data, len);
}

osal_u32 efuse_wait_init(osal_void)
{
    if (osal_wait_init(&g_efuse_write_st.wait_r) != OSAL_SUCCESS) {
        oal_print_err("init osal g_efuse_write_st wait_read failed\n");
        return EXT_ERR_FAILURE;
    }
    if (osal_wait_init(&g_efuse_read_st.wait_r) != OSAL_SUCCESS) {
        oal_print_err("init osal g_efuse_read_st wait_read failed\n");
        return EXT_ERR_FAILURE;
    }
    return EXT_ERR_SUCCESS;
}

osal_void efuse_wait_destroy(osal_void)
{
    osal_wait_destroy(&g_efuse_write_st.wait_r);
    osal_wait_destroy(&g_efuse_read_st.wait_r);
}

osal_module_export(uapi_efuse_read);
osal_module_export(uapi_efuse_write);
#endif

#ifdef __cplusplus
#if __cplusplus
        }
#endif
#endif
