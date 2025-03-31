/*
 * Copyright (c) CompanyNameMagicTag 2022-2023. All rights reserved.
 * Description: host sdio driver os adapt.
 * Author: Huanghe
 * Create: 2022-12-20
 */

#ifndef __OAL_SDIO_LINUX_ADAPT_H__
#define __OAL_SDIO_LINUX_ADAPT_H__

#ifdef CONFIG_HCC_SUPPORT_SDIO
#include <linux/version.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/err.h>
#include <linux/workqueue.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/mmc/card.h>
#include <linux/mmc/sdio_func.h>
#include <linux/mmc/sdio_ids.h>
#include <linux/mmc/sdio_func.h>
#include <linux/mmc/host.h>
#include <linux/pm_runtime.h>
#include <linux/mmc/sdio.h>
#include <linux/etherdevice.h>
#include <linux/scatterlist.h>
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE < KERNEL_VERSION(4,2,0))
#include <asm-generic/scatterlist.h>
#endif
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

static inline td_void sdio_set_en_timeout(struct sdio_func *func, td_u32 timeout)
{
    func->enable_timeout = timeout;
}

static inline td_u32 sdio_func_num(struct sdio_func *func)
{
    return func->num;
}

static inline td_u32 sdio_func_host_max_blk_size(struct sdio_func *func)
{
    return func->card->host->max_blk_size;
}

static inline td_u32 sdio_func_host_max_blk_num(struct sdio_func *func)
{
    return func->card->host->max_blk_count;
}

static inline td_u32 sdio_func_host_max_req_size(struct sdio_func *func)
{
    return func->card->host->max_req_size;
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
    return sdio_writesb(func, addr, src, count);
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
    return sdio_readsb(func, dst, addr, count);
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
    return sdio_readb(func, addr, err_ret);
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
    sdio_writeb(func, b, addr, err_ret);
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
static inline td_void oal_sdio_writel(struct sdio_func *func, td_u32 b, td_u32 addr, td_s32 *err_ret)
{
    sdio_writel(func, b, addr, err_ret);
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
    return sdio_memcpy_fromio(func, dst, addr, count);
}

static inline td_u8 oal_sdio_func0_read_byte(struct sdio_func *func, td_u32 addr, td_s32 *err_ret)
{
    return sdio_f0_readb(func, addr, err_ret);
}

static inline td_void oal_sdio_func0_write_byte(struct sdio_func *func, td_u8 byte, td_u32 addr, td_s32 *err_ret)
{
    return sdio_f0_writeb(func, byte, addr, err_ret);
}
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif
#endif
