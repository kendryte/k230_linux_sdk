#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <pthread.h>

static int uart_fd = -1;
static volatile int running = 1;

// 接收线程
void *uart_read_thread(void *arg) {
    char buffer[1024];
    int n;

    while (running) {
        n = read(uart_fd, buffer, sizeof(buffer) - 1);
        if (n > 0) {
            buffer[n] = '\0';
            printf("\n[RX] %s", buffer);
            fflush(stdout);
        } else if (n < 0) {
            perror("read");
            break;
        }
        usleep(1000);
    }

    return NULL;
}

// 主函数
int main(int argc, char *argv[]) {
    const char *device = "/dev/ttyS3";
    pthread_t read_thread;
    struct termios tty;
    char input[256];

    if (argc > 1) {
        device = argv[1];
    }

    printf("K230 UART - Multi-thread Example\n");
    printf("Device: %s\n", device);
    printf("Commands: 'exit' to quit\n\n");

    // 打开串口
    uart_fd = open(device, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (uart_fd < 0) {
        perror("Cannot open device");
        return 1;
    }

    // 配置串口
    memset(&tty, 0, sizeof(tty));
    tcgetattr(uart_fd, &tty);

    tty.c_cflag = B115200 | CS8 | CLOCAL | CREAD;
    tty.c_lflag = 0;
    tty.c_oflag = 0;
    tty.c_iflag = 0;
    tty.c_cc[VMIN] = 0;
    tty.c_cc[VTIME] = 0;

    tcsetattr(uart_fd, TCSANOW, &tty);

    // 设置为非阻塞模式
    int flags = fcntl(uart_fd, F_GETFL, 0);
    fcntl(uart_fd, F_SETFL, flags | O_NONBLOCK);

    // 创建读线程
    if (pthread_create(&read_thread, NULL, uart_read_thread, NULL) != 0) {
        fprintf(stderr, "Failed to create read thread\n");
        close(uart_fd);
        return 1;
    }

    printf("UART initialized. Start typing (Press Enter to send):\n");

    // 主循环：读取键盘输入并发送
    while (running) {
        if (fgets(input, sizeof(input), stdin) != NULL) {
            // 移除换行符
            size_t len = strlen(input);
            if (len > 0 && input[len-1] == '\n') {
                input[len-1] = '\0';
            }

            // 退出命令
            if (strcmp(input, "exit") == 0) {
                break;
            }

            // 发送数据
            if (strlen(input) > 0) {
                ssize_t written = write(uart_fd, input, strlen(input));
                if (written < 0) {
                    perror("write");
                }
                printf("[TX] %s\n", input);
            }
        }
        usleep(1000);
    }

    running = 0;
    pthread_join(read_thread, NULL);
    close(uart_fd);
    printf("UART closed.\n");

    return 0;
}
