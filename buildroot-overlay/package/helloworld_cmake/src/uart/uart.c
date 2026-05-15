
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <sys/select.h>

// 串口配置结构体
typedef struct {
    int fd;
    speed_t baud_rate;
    char parity;
    int data_bits;
    int stop_bits;
} uart_config_t;



// 打开串口
int uart_open(const char *device) {
    int fd = open(device, O_RDWR | O_NOCTTY);
    if (fd < 0) {
        fprintf(stderr, "Error opening %s: %s\n", device, strerror(errno));
        return -1;
    }

    // 设置为非阻塞模式
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0) {
        fprintf(stderr, "Error fcntl GETFL: %s\n", strerror(errno));
        close(fd);
        return -1;
    }
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0) {
        fprintf(stderr, "Error fcntl SETFL: %s\n", strerror(errno));
        close(fd);
        return -1;
    }

    return fd;
}

// 配置串口参数
int uart_configure(int fd, uart_config_t *config) {
    struct termios tty;

    // 清空 termios 结构
    memset(&tty, 0, sizeof(tty));

    // 获取当前串口设置
    if (tcgetattr(fd, &tty) != 0) {
        fprintf(stderr, "Error tcgetattr: %s\n", strerror(errno));
        return -1;
    }

    // 基本设置：8 位数据，无校验，1 位停止位
    tty.c_cflag &= ~PARENB;  // 无校验
    tty.c_cflag &= ~CSTOPB;  // 1 位停止位
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;       // 8 位数据

    // 设置波特率
    cfsetispeed(&tty, config->baud_rate);
    cfsetospeed(&tty, config->baud_rate);

    // 设置为原始模式
    tty.c_cflag |= (CLOCAL | CREAD);
    tty.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    tty.c_oflag &= ~OPOST;

    // 禁用软件流控制
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);

    // 禁用硬件流控制
    tty.c_cflag &= ~CRTSCTS;

    // 设置最小字符数和等待时间 - 用于非阻塞读取
    tty.c_cc[VMIN] = 0;   // 不等待字符
    tty.c_cc[VTIME] = 0;  // 不等待

    // 应用设置
    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        fprintf(stderr, "Error tcsetattr: %s\n", strerror(errno));
        return -1;
    }

    return 0;
}

// 发送数据
int uart_write_data(int fd, const char *data, size_t len) {
    ssize_t written = write(fd, data, len);
    if (written < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return 0; // 写缓冲区满，返回0
        }
        fprintf(stderr, "Error write: %s\n", strerror(errno));
        return -1;
    }
    return (int)written;
}



// 关闭串口
void uart_close(int fd) {
    close(fd);
}

// 主函数示例
int main(int argc, char *argv[]) {
    const char *device = "/dev/ttyS3";  // K230 通常使用 ttyS1
    int fd;
    uart_config_t config;
    char buffer[1024];
    int bytes_read;

    // 处理命令行参数
    if (argc > 1) {
        device = argv[1];
    }

    printf("K230 UART Example\n");
    printf("Device: %s\n", device);
    printf("Baud: 115200, 8N1\n\n");

    // 打开串口
    fd = uart_open(device);
    if (fd < 0) {
        return 1;
    }

    // 配置串口
    config.baud_rate = B115200;
    config.parity = 'N';
    config.data_bits = 8;
    config.stop_bits = 1;

    if (uart_configure(fd, &config) < 0) {
        uart_close(fd);
        return 1;
    }

    printf("UART configured successfully!\n");
    printf("Type something and press Enter...\n");
    printf("Press Ctrl+C to exit\n\n");

    // 主循环：使用同一个 select 同时监听 stdin 和串口
    while (1) {
        fd_set readfds;
        int maxfd = (fd > STDIN_FILENO) ? fd : STDIN_FILENO;
        struct timeval tv;

        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);
        FD_SET(fd, &readfds);

        tv.tv_sec = 0;
        tv.tv_usec = 10000; // 10ms

        int ret = select(maxfd + 1, &readfds, NULL, NULL, &tv);
        if (ret < 0) {
            fprintf(stderr, "Error select: %s\n", strerror(errno));
            break;
        }

        // 检查 stdin 是否有输入
        if (FD_ISSET(STDIN_FILENO, &readfds)) {
            bytes_read = read(STDIN_FILENO, buffer, sizeof(buffer) - 1);
            if (bytes_read > 0) {
                buffer[bytes_read] = '\0';
                // 发送到串口
                if (uart_write_data(fd, buffer, strlen(buffer)) < 0) {
                    break;
                }
                printf("Sent: %s", buffer);
            }
        }

        // 检查串口是否有数据
        if (FD_ISSET(fd, &readfds)) {
            bytes_read = read(fd, buffer, sizeof(buffer) - 1);
            if (bytes_read > 0) {
                buffer[bytes_read] = '\0';
                printf("\nReceived: %s", buffer);
                fflush(stdout);
            }
        }
    }

    uart_close(fd);
    return 0;
}
