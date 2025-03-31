/**
 * Copyright (c) @CompanyNameMagicTag 2012-2023. All rights reserved.
 *
 * Description: open or close tty uart
 * Author: @CompanyNameTag
 */

/* 头文件包含 */
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <poll.h>
#include <securec.h>
#include <unistd.h>
#include "octty.h"

#define TTY_UNINSTALL_CMD     '0'
#define TTY_INSTALL_CMD       '1'
#define TTY_BAUD_RECONFIG_CMD '2'

/* File descriptor for the UART device */
int g_uart_fd;


/* Note:expression with side effects passed to repeated parameter 2 in "OCTTY_ERR" */
#define LINE_DIS_RETRY_TIMES 5

/*
 * Setting the uart line discipline.
 * consider ioctl may interrupt by system call
 * retry if failed
 */
int set_line_discipline(int ldisc)
{
    int line_retry_cnt = LINE_DIS_RETRY_TIMES;

    while (ioctl(g_uart_fd, TIOCSETD, &ldisc) < 0) {
        OCTTY_WARNING("set line discipline errno:%s,g_uart_fd:%d,left retry cnt:%d,ldsic=%d,do retry",
                      strerror(errno), g_uart_fd, line_retry_cnt, ldisc);
        if (--line_retry_cnt) {
            usleep(100); // delay 100 us
        } else {
            OCTTY_ERR("set line discipline error");
            return -1;
        }
    }
    return 0;
}

static inline void close_uart(void)
{
    if (g_uart_fd == -1) {
        return;
    }

    if (!set_line_discipline(0)) {
        OCTTY_DBG("%s Have uninstalled N_HW_BFG Line displine", __func__);
    }

    close(g_uart_fd);
    g_uart_fd = -1;
}

/*
 * Function to set the default baud rate
 * The default baud rate of 115200 is set to the UART from the host side
 * by making a call to this function.This function is also called before
 * making a call to set the custom baud rate
 */
static int set_baud_rate(unsigned char flow_ctrl)
{
    struct termios baud;

    OCTTY_DBG(" %s", __func__);

    tcflush(g_uart_fd, TCIOFLUSH);

    /* Get the attributes of UART */
    if (tcgetattr(g_uart_fd, &baud) < 0) {
        OCTTY_ERR(" Can't get port settings;errno:%s", strerror(errno));
        return -1;
    }

    /* Change the UART attributes before setting the default baud rate */
    cfmakeraw(&baud);

    baud.c_cflag |= CLOCAL;
    if (flow_ctrl) {
        baud.c_cflag |= CRTSCTS;
    } else {
        baud.c_cflag &= ~CRTSCTS;
    }

    /* Set the attributes of UART after making the above changes */
    tcsetattr(g_uart_fd, TCSANOW, &baud);

    /* Set the actual default baud rate */
    cfsetospeed(&baud, B115200);
    cfsetispeed(&baud, B115200);
    tcsetattr(g_uart_fd, TCSANOW, &baud);

    tcflush(g_uart_fd, TCIOFLUSH);
    OCTTY_DBG(" set_baud_rate() done");

    return 0;
}

/*
 * Function to set the UART custom baud rate.
 * The UART baud rate has already been
 * set to default value 115200 before calling this function.
 * The baud rate is then changed to custom baud rate by this function
 */
static int change_baud_rate(int second_baud_rate, unsigned char flow_ctrl)
{
    struct termios baud;
    struct termios2 baud2;

    OCTTY_DBG(" %s", __func__);

    /* Get the attributes of UART */
    if (tcgetattr(g_uart_fd, &baud) < 0) {
        OCTTY_ERR(" Can't get port settings;errno:%s", strerror(errno));
        return -1;
    }

    /* Flush non-transmitted output data, non-read input data or both */
    tcflush(g_uart_fd, TCIOFLUSH);

    /* Set the UART flow control */
    if (flow_ctrl) {
        baud.c_cflag |= CRTSCTS;
    } else {
        baud.c_cflag &= ~CRTSCTS;
    }

    /*
     * Set the parameters associated with the UART
     * The change will occur immediately by using TCSANOW
     */
    if (tcsetattr(g_uart_fd, TCSANOW, &baud) < 0) {
        OCTTY_ERR(" Can't set port settings;errno:%s", strerror(errno));
        return -1;
    }

    tcflush(g_uart_fd, TCIOFLUSH);

    /* Set the actual baud rate */
    ioctl(g_uart_fd, TCGETS2, &baud2);
    baud2.c_cflag &= ~CBAUD;
    baud2.c_cflag |= BOTHER;
    baud2.c_ospeed = (speed_t)second_baud_rate;
    ioctl(g_uart_fd, TCSETS2, &baud2);
    OCTTY_DBG(" change_baud_rate() done");
    return 0;
}

static int read_uart_param(const char *param_file_name, unsigned char *buf, unsigned int buf_len)
{
    int len;
    int fd = open(param_file_name, O_RDONLY);
    if (fd < 0) {
        OCTTY_ERR("Can't open %s;errno:%s", param_file_name, strerror(errno));
        return -1;
    }

    len = read(fd, buf, buf_len - 1); // 预留1字节结束符
    if (len < 0) {
        OCTTY_ERR("read err (%s)", strerror(errno));
    } else {
        buf[len] = '\0';
    }

    close(fd);
    return len;
}

static int hcc_config_uart_pre(long *second_baud_rate, unsigned int *flow_ctrl,
    unsigned char *uart_dev_name, unsigned int name_len)
{
    unsigned char buf[32] = {0}; // 32 字节足够存放读取的参数

    if (read_uart_param(DEV_NAME_SYSFS, buf, sizeof(buf)) < 0) {
        return -1;
    }

    if (sscanf_s((const char*)buf, "%s", uart_dev_name, name_len) <= 0) {
        OCTTY_ERR("hcc_config_uart: sscanf_s failed.");
        return -1;
    }

    if (read_uart_param(BAUD_RATE_SYSFS, buf, sizeof(buf)) < 0) {
        return -1;
    }
    if (sscanf_s((const char*)buf, "%ld", second_baud_rate) <= 0) {
        OCTTY_ERR("hcc_config_uart: sscanf_s failed.");
        return -1;
    }

    if (read_uart_param(FLOW_CTRL_SYSFS, buf, sizeof(buf)) < 0) {
        return -1;
    }
    if (sscanf_s((const char*)buf, "%4u", flow_ctrl) <= 0) {
        OCTTY_ERR("hcc_config_uart: sscanf_s failed.");
        return -1;
    }

    return 0;
}

static int config_uart_after_open(unsigned char flow_ctrl, int second_baud_rate)
{
    /*
     * Set only the default baud rate.
     * This will set the baud rate to default 115200
     */
    if (set_baud_rate((unsigned char)flow_ctrl) < 0) {
        OCTTY_ERR(" set_baudrate() failed;errno:%s", strerror(errno));
        close_uart();
        return -1;
    }

    if (fcntl(g_uart_fd, F_SETFL, ((unsigned int)fcntl(g_uart_fd, F_GETFL) | O_NONBLOCK)) < 0) {
        OCTTY_DBG(" fcntl() < 0 ");
        close_uart();
        return -1;
    }

    /* Set only the custom baud rate */
    if (second_baud_rate != 115200) { // baud rate 115200
        /*
         * Writing the change speed command to the UART
         * This will change the UART speed at the controller side
         */
        if (change_baud_rate(second_baud_rate, flow_ctrl) < 0) {
            OCTTY_ERR(" change_baud_rate() failed;errno:%s", strerror(errno));
            close_uart();
            return -1;
        }
    }

    /*
     * After the UART speed has been changed, the IOCTL is
     * is called to set the line discipline to N_HW_BFG
     */
    if (set_line_discipline(N_HW_BFG) < 0) {
        return -1;
    }
    OCTTY_DBG("Have installed N_HW_BFG Line displine");

    return 0;
}

static void uninstall_uart(void)
{
    OCTTY_DBG(" Un-Installed N_HW_BFG Line displine");
    close_uart();
    g_uart_fd = -1;
}

static void install_uart(void)
{
    unsigned char uart_dev_name[32]; // buffer size is 32
    long second_baud_rate = 0;
    unsigned int flow_ctrl = 0;

    // 如果是打开状态，先关闭再重新打开
    if (g_uart_fd != -1) {
        OCTTY_DBG("tty may have been opened, close first and reopen tty!!");
        close_uart();
    }

    if (hcc_config_uart_pre(&second_baud_rate, &flow_ctrl, uart_dev_name, sizeof(uart_dev_name)) < 0) {
        OCTTY_ERR("hcc_config_uart_pre failed!");
        return;
    }

    // 打开tty
    OCTTY_DBG("install_uart: open %s ", uart_dev_name);
    g_uart_fd = open((const char *)uart_dev_name, O_RDWR | O_NOCTTY);
    if (g_uart_fd < 0) {
        OCTTY_ERR(" Can't open %s", (char *)uart_dev_name);
        return;
    }

    // 配置tty
    if (config_uart_after_open((unsigned char)flow_ctrl, (int)second_baud_rate) < 0) {
        OCTTY_ERR("config_uart_after_open failed");
    }
}

static void baud_rate_reconfig(void)
{
    unsigned char uart_dev_name[32]; // buffer size is 32
    long second_baud_rate = 0;
    unsigned int flow_ctrl = 0;

    if (hcc_config_uart_pre(&second_baud_rate, &flow_ctrl, uart_dev_name, sizeof(uart_dev_name)) < 0) {
        OCTTY_ERR("hcc_config_uart_pre failed!");
        return;
    }

    // 修改波特率时，如果串口已经关闭了，需要从新打开
    if (g_uart_fd == -1) {
        OCTTY_DBG(" tty has closed when set install to 2! reopen tty");
        OCTTY_DBG("baud_rate_reconfig: begin open %s", uart_dev_name);
        g_uart_fd = open((const char *)uart_dev_name, O_RDWR | O_NOCTTY);
        if (g_uart_fd < 0) {
            OCTTY_ERR(" Can't open %s", (char *)uart_dev_name);
            return;
        }
    } else { // 如果串口是打开状态， 卸载线路规程，后续再重新挂载
        OCTTY_DBG(" set install to 2, disable linedisc");
        if (set_line_discipline(0) < 0) {
            return;
        }
        OCTTY_DBG(" Have uninstalled N_HW_BFG Line displine");
    }

    // 配置tty
    if (config_uart_after_open((unsigned char)flow_ctrl, (int)second_baud_rate) < 0) {
        OCTTY_ERR("config_uart_after_open failed");
    }
}

/*
 * Handling the Signals sent from the Kernel Init Manager.
 * After receiving the indication from rfkill subsystem, configure the
 * baud rate, flow control and Install the N_TI_WL line discipline
 */
void hcc_config_uart(unsigned char install)
{
    OCTTY_DBG(" %s", __func__);

    switch (install) {
        case TTY_UNINSTALL_CMD:
            uninstall_uart();
            break;
        case TTY_INSTALL_CMD:
            install_uart();
            break;
        case TTY_BAUD_RECONFIG_CMD:
            baud_rate_reconfig();
            break;
        default:
            OCTTY_ERR("error install cmd");
            break;
    }
}

int main(void)
{
    int hw_fd, err;
    struct pollfd pfd;
    unsigned char install;

    err = 0, g_uart_fd = -1;
    OCTTY_ERR("octty open\r\n");
    while (access(INSTALL_SYSFS_ENTRY, F_OK) != 0) {
        sleep(1);
    }
    /* rfkill device's open/poll/read */
    hw_fd = open(INSTALL_SYSFS_ENTRY, O_RDONLY);
    if (hw_fd < 0) {
        OCTTY_ERR("unable to open %s ;errno:%s", INSTALL_SYSFS_ENTRY, strerror(errno));
        return -1;
    }

    for (;;) {
        err = read(hw_fd, &install, 1);
        if ((err > 0) && (install != '0')) {
            OCTTY_ERR(" install already set to %c", install);
        }
        memset_s(&pfd, sizeof(pfd), 0, sizeof(pfd));
        pfd.fd = hw_fd;
        /* sysfs entries can only break poll for following events */
        pfd.events = POLLERR | POLLHUP;

        for (;;) {
            pfd.revents = 0;
            err = poll(&pfd, 1, -1);
            if (err < 0 && errno == EINTR) {
                continue;
            }
            if (err) {
                break;
            }
        }
        close(hw_fd);
        hw_fd = open(INSTALL_SYSFS_ENTRY, O_RDONLY);
        if (hw_fd < 0) {
            OCTTY_ERR("re-opening %s failed;errno:%s", INSTALL_SYSFS_ENTRY, strerror(errno));
            return -1;
        }
        err = read(hw_fd, &install, 1);
        if (err <= 0) {
            OCTTY_ERR("reading %s failed;errno:%s", INSTALL_SYSFS_ENTRY, strerror(errno));
            close(hw_fd);
            return -1;
        }
        hcc_config_uart(install);
    }
    /* Note:unreachable code at token "close" */
    close(hw_fd);
    return 0;
}
