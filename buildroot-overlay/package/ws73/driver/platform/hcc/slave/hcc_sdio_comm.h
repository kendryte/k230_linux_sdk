/*
 * Copyright (c) CompanyNameMagicTag 2020-2020. All rights reserved.
 * Description: sdio common interface
 * Author: CompanyName
 * Create: 2020-05-02
 */

#ifndef HCC_SDIO_COMM_HEADER
#define HCC_SDIO_COMM_HEADER

#ifdef CONFIG_HCC_SUPPORT_SDIO

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef CONFIG_SDIO_DEBUG
#define CONFIG_SDIO_FUNC_EXTEND

#undef CONFIG_CREDIT_MSG_FLOW_CTRL_DEBUG
#undef CONFIG_SDIO_REINIT_SUPPORT


/* 定义此宏表示打开host to arm 的消息回读ACK，
  消息是从Host发送至ARM,
  必须读高字节才能产生ACK */
#undef CONFIG_SDIO_H2D_MSG_ACK

#define HH503_SDIO_BLOCK_SIZE               512 /* one size of data transfer block size, * 64, 128, 256, 512, 1024 */
/* The max scatter buffers when host to device */
#define HISDIO_HOST2DEV_SCATT_MAX           40
#define HISDIO_HOST2DEV_SCATT_SIZE          64

/* The max scatter buffers when device to host */
#define HISDIO_DEV2HOST_SCATT_MAX           63

/* The max scatt num of rx and tx */
#define HH503_SDIO_SCATT_MAX_NUM                (HISDIO_DEV2HOST_SCATT_MAX)

/* 64B used to store the scatt info,1B means 1 pkt. */
#define HISDIO_H2D_SCATT_BUFFLEN_ALIGN_BITS 3
/* 1 << 5 */
/* Host to device's descr align length depends on the
   CONFIG_HISDIO_H2D_SCATT_LIST_ASSEMBLE */
#define HISDIO_H2D_SCATT_BUFFLEN_ALIGN      8

/* Device To Host,descr just request 4 bytes aligned,
  but 10 bits round [0~1023], so we also aligned to 32 bytes */
#define HISDIO_D2H_SCATT_BUFFLEN_ALIGN_BITS 5

/* 1 << 5 */
#define HISDIO_D2H_SCATT_BUFFLEN_ALIGN      32
#define HISDIO_D2H_SCATT_BUFFADDR_ALIGN     4

#define HSDIO_HOST2DEV_PKTS_MAX_LEN         1560


#define D2H_MAX_MSG_COUNT   32
#define H2D_MAX_MSG_COUNT   32

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* CONFIG_HCC_SUPPORT_SDIO */
#endif /* HCC_SDIO_COMM_HEADER */
