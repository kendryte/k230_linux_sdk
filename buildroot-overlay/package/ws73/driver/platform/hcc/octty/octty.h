/**
 * Copyright (c) @CompanyNameMagicTag 2012-2023. All rights reserved.
 *
 * Description: octty.c 的头文件
 * Author: @CompanyNameTag
 */

#ifndef __OCTTY_H__
#define __OCTTY_H__


/* 宏定义 */
#define OCTTY_DEBUG

/* for huawei's BT, FM, GNSS, WLAN chip */
#define N_HW_BFG 29
#define ARM_NCCS 19

#define INSTALL_SYSFS_ENTRY "/sys/ws73_ps/install"
#define DEV_NAME_SYSFS      "/sys/ws73_ps/dev_name"
#define BAUD_RATE_SYSFS     "/sys/ws73_ps/baud_rate"
#define FLOW_CTRL_SYSFS     "/sys/ws73_ps/flow_cntrl"
#define OCTTY_ERR printf
#define OCTTY_DBG printf
#define OCTTY_WARNING printf
/* Paramaters to set the baud rate */
#define BOTHER  0x00001000
#define TCGETS2 _IOR('T', 0x2A, struct termios2)
#define TCSETS2 _IOW('T', 0x2B, struct termios2)

/* Termios2 structure for setting the secend baud rate */
struct termios2 {
    tcflag_t c_iflag;    /* input mode flags */
    tcflag_t c_oflag;    /* output mode flags */
    tcflag_t c_cflag;    /* control mode flags */
    tcflag_t c_lflag;    /* local mode flags */
    cc_t c_line;         /* line discipline */
    cc_t c_cc[ARM_NCCS]; /* control characters */
    speed_t c_ispeed;    /* input speed */
    speed_t c_ospeed;    /* output speed */
};

#endif /* end of octty.h */
