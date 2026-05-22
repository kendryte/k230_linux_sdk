
// file: spi_nor_example.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include <stdint.h>

#define SPI_DEVICE "/dev/spidev0.0"
#define SPI_FREQ 1000000

// SPI NOR Flash 命令
#define CMD_JEDEC_ID      0x9F
#define CMD_READ_STATUS   0x05
#define CMD_WRITE_ENABLE  0x06
#define CMD_PAGE_PROGRAM  0x02
#define CMD_READ_DATA     0x03

// 等待 Flash 就绪
int spi_nor_wait_ready(int fd, int timeout)
{
    uint8_t tx[] = {CMD_READ_STATUS};
    uint8_t rx[2] = {0};
    int i;

    for (i = 0; i < timeout; i++) {
        struct spi_ioc_transfer tr = {
            .tx_buf = (unsigned long)tx,
            .rx_buf = (unsigned long)rx,
            .len = 2,
            .speed_hz = SPI_FREQ,
            .bits_per_word = 8,
        };

        if (ioctl(fd, SPI_IOC_MESSAGE(1), &tr) < 0) {
            return -1;
        }

        if (!(rx[1] & 0x01)) {  // WIP bit cleared
            return 0;
        }

        usleep(1000);  // 1ms delay
    }

    return -1;  // Timeout
}

int spi_nor_read_jedec_id(int fd, uint8_t *id)
{
    uint8_t tx[] = {CMD_JEDEC_ID, 0x00, 0x00, 0x00};
    uint8_t rx[4] = {0};

    struct spi_ioc_transfer tr = {
        .tx_buf = (unsigned long)tx,
        .rx_buf = (unsigned long)rx,
        .len = 4,
        .speed_hz = SPI_FREQ,
        .bits_per_word = 8,
    };

    if (ioctl(fd, SPI_IOC_MESSAGE(1), &tr) < 0) {
        return -1;
    }

    // ID 从 rx[1] 开始 (3 字节)
    id[0] = rx[1];
    id[1] = rx[2];
    id[2] = rx[3];

    return 0;
}

int spi_nor_read_status(int fd, uint8_t *status)
{
    uint8_t tx[] = {CMD_READ_STATUS};
    uint8_t rx[2] = {0};

    struct spi_ioc_transfer tr = {
        .tx_buf = (unsigned long)tx,
        .rx_buf = (unsigned long)rx,
        .len = 2,
        .speed_hz = SPI_FREQ,
        .bits_per_word = 8,
    };

    if (ioctl(fd, SPI_IOC_MESSAGE(1), &tr) < 0) {
        return -1;
    }

    *status = rx[1];
    return 0;
}

int spi_nor_write_enable(int fd)
{
    uint8_t tx[] = {CMD_WRITE_ENABLE};

    struct spi_ioc_transfer tr = {
        .tx_buf = (unsigned long)tx,
        .rx_buf = 0,
        .len = 1,
        .speed_hz = SPI_FREQ,
        .bits_per_word = 8,
    };

    return ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
}

int spi_nor_page_program(int fd, uint32_t addr, uint8_t *data, size_t len)
{
    uint8_t tx[4 + len];

    tx[0] = CMD_PAGE_PROGRAM;
    tx[1] = (addr >> 16) & 0xFF;
    tx[2] = (addr >> 8) & 0xFF;
    tx[3] = addr & 0xFF;
    memcpy(&tx[4], data, len);

    struct spi_ioc_transfer tr = {
        .tx_buf = (unsigned long)tx,
        .rx_buf = 0,
        .len = 4 + len,
        .speed_hz = SPI_FREQ,
        .bits_per_word = 8,
    };

    return ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
}

int spi_nor_read_data(int fd, uint32_t addr, uint8_t *data, size_t len)
{
    uint8_t tx[4 + len];
    uint8_t rx[4 + len];

    tx[0] = CMD_READ_DATA;
    tx[1] = (addr >> 16) & 0xFF;
    tx[2] = (addr >> 8) & 0xFF;
    tx[3] = addr & 0xFF;

    struct spi_ioc_transfer tr = {
        .tx_buf = (unsigned long)tx,
        .rx_buf = (unsigned long)rx,
        .len = 4 + len,
        .speed_hz = SPI_FREQ,
        .bits_per_word = 8,
    };

    if (ioctl(fd, SPI_IOC_MESSAGE(1), &tr) < 0) {
        return -1;
    }

    memcpy(data, &rx[4], len);
    return 0;
}

int main(int argc, char *argv[])
{
    int fd;
    uint8_t id[3];
    uint8_t status;
    uint8_t test_data[256];
    uint8_t read_data[256];
    int i;

    fd = open(SPI_DEVICE, O_RDWR);
    if (fd < 0) {
        perror("Failed to open SPI device");
        return -1;
    }

    // 配置 SPI
    uint8_t mode = SPI_MODE_0;
    uint32_t speed = SPI_FREQ;
    ioctl(fd, SPI_IOC_WR_MODE, &mode);
    ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);

    // 读取 JEDEC ID
    printf("Reading JEDEC ID...\n");
    if (spi_nor_read_jedec_id(fd, id) == 0) {
        printf("Manufacturer ID: 0x%02X\n", id[0]);
        printf("Memory Type: 0x%02X\n", id[1]);
        printf("Capacity: 0x%02X\n", id[2]);
    }

    // 读取状态寄存器
    printf("\nReading Status Register...\n");
    if (spi_nor_read_status(fd, &status) == 0) {
        printf("Status: 0x%02X\n", status);
    }

    // 擦除和写入测试
    printf("\nTesting write operation...\n");

    // 生成测试数据
    for (i = 0; i < 256; i++) {
        test_data[i] = i & 0xFF;
    }

    // 写入使能
    spi_nor_wait_ready(fd, 100);
    spi_nor_write_enable(fd);

    // 页编程 (256 字节)
    spi_nor_page_program(fd, 0x000000, test_data, 256);
    spi_nor_wait_ready(fd, 1000);

    // 读回验证
    spi_nor_read_data(fd, 0x000000, read_data, 256);

    printf("\nVerify: %s\n",
           memcmp(test_data, read_data, 256) == 0 ? "PASS" : "FAIL");

    close(fd);
    return 0;
}
