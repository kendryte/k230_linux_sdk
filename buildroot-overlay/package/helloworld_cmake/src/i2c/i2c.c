#include <linux/i2c-dev.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>

void print_usage(const char *prog)
{
    printf("Usage: %s [I2C_BUS]\n", prog);
    printf("  -h, --help    Show this help message\n");
    printf("  I2C_BUS       I2C bus device (default: /dev/i2c-0)\n");
}

int main(int argc, char *argv[])
{
    // 解析 -h 参数
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        }
    }

    int file;
    char *filename = "/dev/i2c-0";

    if (argc > 1) {
        filename = argv[1];
    }

    // 打开 I2C 设备
    file = open(filename, O_RDWR);
    if (file < 0) {
        perror("Failed to open i2c device");
        return -1;
    }

    printf("Scanning I2C bus: %s\n", filename);
    printf("Address  Device\n");
    printf("-------  ------\n");
    printf("\n7-bit addresses (0x00-0xff):\n");

    // 扫描所有 0x00 到 0xFF 的地址
    for (int addr = 0x00; addr <= 0xFF; addr++) {
        // 设置从设备地址
        if (ioctl(file, I2C_SLAVE, addr) < 0) {
            continue;
        }

        // 尝试读取 1 个字节
        // 这是安全的操作，不会影响设备状态
        unsigned char buf[1];
        int result = read(file, buf, 1);

        // 如果读取成功，说明设备存在
        if (result == 1) {
            printf("  0x%02X   Present\n", addr);
        }
    }

    printf("\nNote: 0x78-0x7F are reserved addresses.\n");

    close(file);
    return 0;
}
