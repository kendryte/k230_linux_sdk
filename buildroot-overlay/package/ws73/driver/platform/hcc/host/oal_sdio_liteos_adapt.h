/*
 * Copyright (c) CompanyNameMagicTag 2022-2023. All rights reserved.
 * Description: host sdio driver os adapt.
 * Author: Huanghe
 * Create: 2022-12-20
 */

#ifndef __OAL_SDIO_LITEOS_ADAPT_H__
#define __OAL_SDIO_LITEOS_ADAPT_H__

#ifdef CONFIG_HCC_SUPPORT_SDIO
#include "sdio.h"
#include "sdio_func.h"
#include "oal_sdio_liteos_scatter.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define SDIO_ANY_ID (~0)
#define SDIO_DEVICE(vend, dev) \
    .class = SDIO_ANY_ID, \
             .vendor = (vend), .device = (dev)

struct sdio_device_id {
    td_u8    class;                  /* Standard interface or SDIO_ANY_ID */
    td_u16   vendor;                 /* Vendor or SDIO_ANY_ID */
    td_u16   device;                 /* Device ID or SDIO_ANY_ID */
};

/*
 * SDIO function device driver
 */
struct sdio_driver {
    td_u8 *name;
    const struct sdio_device_id *id_table;

    td_s32 (*probe)(struct sdio_func *, const struct sdio_device_id *);
    td_void (*remove)(struct sdio_func *);
};

static inline td_s32 sdio_enable_func(struct sdio_func *func)
{
    return sdio_en_func(func);
}

static inline td_s32 sdio_disable_func(struct sdio_func *func)
{
    return sdio_dis_func(func);
}

static inline td_s32 sdio_set_block_size(struct sdio_func *func, td_u32 blksz)
{
    return sdio_set_cur_blk_size(func, blksz);
}

static inline struct oal_sdio *sdio_get_drvdata(struct sdio_func *func)
{
    return func->data;
}

static inline td_void sdio_set_drvdata(struct sdio_func *func, struct oal_sdio *priv)
{
    func->data = (void *)priv;
}

static inline td_void sdio_set_en_timeout(struct sdio_func *func, td_u32 timeout)
{
    func->en_timeout_ms = timeout;
}

static inline td_u32 sdio_func_num(struct sdio_func *func)
{
    return func->func_num;
}

static inline td_u32 sdio_func_host_max_blk_size(struct sdio_func *func)
{
    return func->card->host->max_blk_size;
}

static inline td_u32 sdio_func_host_max_blk_num(struct sdio_func *func)
{
    return func->card->host->max_blk_num;
}

static inline td_u32 sdio_func_host_max_req_size(struct sdio_func *func)
{
    return func->card->host->max_request_size;
}

static inline td_void sdio_claim_host(struct sdio_func *func)
{
    if (func != TD_NULL) {
        mmc_acquire_card(func->card);
    }
}

static inline td_void sdio_release_host(struct sdio_func *func)
{
    if (func != TD_NULL) {
        mmc_release_card(func->card);
    }
}

/**
 *  oal_sdio_adapt_writesb - write to a FIFO of a SDIO function
 *  @func: SDIO function to access
 *  @addr: address of (single byte) FIFO
 *  @src: buffer that contains the data to write
 *  @count: number of bytes to write
 *  Writes to the specified FIFO of a given SDIO function. Return
 *  value indicates if the transfer succeeded or not.
 */
static inline td_s32 oal_sdio_adapt_writesb(struct sdio_func *func, td_u32 addr, td_void *src, td_u32 count)
{
    return sdio_write_fifo_block(func, addr, src, count);
}

/**
 *  oal_sdio_adapt_readsb - read from a FIFO on a SDIO function
 *  @func: SDIO function to access
 *  @dst: buffer to store the data
 *  @addr: address of (single byte) FIFO
 *  @count: number of bytes to read
 *  Reads from the specified FIFO of a given SDIO function. Return
 *  value indicates if the transfer succeeded or not.
 */
static inline td_s32 oal_sdio_adapt_readsb(struct sdio_func *func, td_void *dst, td_u32 addr, td_s32 count)
{
    return sdio_read_fifo_block(func, dst, addr, count);
}

/**
 *  oal_sdio_readb - read a single byte from a SDIO function
 *  @func: SDIO function to access
 *  @addr: address to read
 *  @err_ret: optional status value from transfer
 *
 *  Reads a single byte from the address space of a given SDIO
 *  function. If there is a problem reading the address, 0xff
 *  is returned and @err_ret will contain the error code.
 */
static inline td_u8 oal_sdio_readb(struct sdio_func *func, td_u32 addr, td_s32 *err_ret)
{
    return sdio_read_byte(func, addr, err_ret);
}

/**
 *  oal_sdio_writeb - write a single byte to a SDIO function
 *  @func: SDIO function to access
 *  @b: byte to write
 *  @addr: address to write to
 *  @err_ret: optional status value from transfer
 *
 *  Writes a single byte to the address space of a given SDIO
 *  function. @err_ret will contain the status of the actual
 *  transfer.
 */
static inline void oal_sdio_writeb(struct sdio_func *func, td_u8 b, td_u32 addr, td_s32 *err_ret)
{
    sdio_write_byte(func, b, addr, err_ret);
}

/**
 *  oal_sdio_writel - write a 32 bit integer to a SDIO function
 *  @func: SDIO function to access
 *  @b: integer to write
 *  @addr: address to write to
 *  @err_ret: optional status value from transfer
 *
 *  Writes a 32 bit integer to the address space of a given SDIO
 *  function. @err_ret will contain the status of the actual
 *  transfer.
 */
static inline td_s32 oal_sdio_writel(struct sdio_func *func, td_u32 b, td_u32 addr, td_s32 *err_ret)
{
    td_s32 ret;

    ret = sdio_write_incr_block(func, addr, &b, sizeof(td_u32));
    return ret;
}

/**
 *  oal_sdio_adapt_memcpy_fromio - read a chunk of memory from a SDIO function
 *  @func: SDIO function to access
 *  @dst: buffer to store the data
 *  @addr: address to begin reading from
 *  @count: number of bytes to read
 *  Reads from the address space of a given SDIO function. Return
 *  value indicates if the transfer succeeded or not.
 */
static inline td_s32 oal_sdio_adapt_memcpy_fromio(struct sdio_func *func, td_void *dst,
    td_u32 addr, td_s32 count)
{
    return sdio_read_incr_block(func, dst, addr, count);
}

static inline td_u8 oal_sdio_func0_read_byte(struct sdio_func *func, td_u32 addr, td_s32 *err_ret)
{
    return sdio_func0_read_byte(func, addr, err_ret);
}

static inline td_void oal_sdio_func0_write_byte(struct sdio_func *func, td_u8 byte, td_u32 addr, td_s32 *err_ret)
{
    return sdio_func0_write_byte(func, byte, addr, err_ret);
}
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif
#endif
