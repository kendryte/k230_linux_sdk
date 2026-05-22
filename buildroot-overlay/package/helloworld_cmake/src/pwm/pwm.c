#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>

#define SYSFS_PWM_DIR "/sys/class/pwm"
#define MAX_LEN 256

static int current_chip = 0;
static int current_channel = 0;

// Function declarations
int write_sysfs_attr(const char *path, const char *value);
int pwm_export(int chip, int channel);
int pwm_unexport(int chip, int channel);
int pwm_set_polarity(int chip, int channel, const char *polarity);
int pwm_set_period(int chip, int channel, unsigned long period_ns);
int pwm_set_duty_cycle(int chip, int channel, unsigned long duty_ns);
int pwm_enable(int chip, int channel, int enable);
unsigned long pwm_read_value(int chip, int channel, const char *attr);

// Signal handler to cleanup PWM on Ctrl+C
void cleanup_handler(int sig)
{
    (void)sig;
    printf("\nStopping PWM... clean \n");
    // pwm_enable(current_chip, current_channel, 0);
    // pwm_unexport(current_chip, current_channel);
    exit(0);
}

// 写入 sysfs 属性
int write_sysfs_attr(const char *path, const char *value)
{
    int fd = open(path, O_WRONLY);
    if (fd < 0) {
        printf("Failed to open sysfs attribute path=%s \n",path);
        return -1;
    }

    ssize_t len = strlen(value);
    if (write(fd, value, len) != len) {
        printf("Failed to write sysfs attribute p=%s v=%s\n", path,value);
        close(fd);
        return -1;
    }

    close(fd);
    return 0;
}

// 导出 PWM 通道
int pwm_export(int chip, int channel)
{
    char path[MAX_LEN];
    snprintf(path, sizeof(path), "%s/pwmchip%d/export", SYSFS_PWM_DIR, chip);
    char value[16];
    snprintf(value, sizeof(value), "%d", channel);
    return write_sysfs_attr(path, value);
}

// 取消导出 PWM 通道
int pwm_unexport(int chip, int channel)
{
    char path[MAX_LEN];
    char check_path[MAX_LEN];

    // Check if PWM channel exists before unexporting
    snprintf(check_path, sizeof(check_path), "%s/pwmchip%d/pwm%d", SYSFS_PWM_DIR, chip, channel);
    if (access(check_path, F_OK) != 0) {
        // PWM channel does not exist, nothing to unexport
        return 0;
    }

    snprintf(path, sizeof(path), "%s/pwmchip%d/unexport", SYSFS_PWM_DIR, chip);
    char value[16];
    snprintf(value, sizeof(value), "%d", channel);
    return write_sysfs_attr(path, value);
}

// 设置 PWM 极性
int pwm_set_polarity(int chip, int channel, const char *polarity)
{
    char path[MAX_LEN];
    snprintf(path, sizeof(path), "%s/pwmchip%d/pwm%d/polarity",
             SYSFS_PWM_DIR, chip, channel);
    return write_sysfs_attr(path, polarity);
}

// 设置 PWM 周期
int pwm_set_period(int chip, int channel, unsigned long period_ns)
{
    char path[MAX_LEN];
    snprintf(path, sizeof(path), "%s/pwmchip%d/pwm%d/period",
             SYSFS_PWM_DIR, chip, channel);
    char value[32];
    snprintf(value, sizeof(value), "%lu", period_ns);
    return write_sysfs_attr(path, value);
}

// 设置 PWM 占空比
int pwm_set_duty_cycle(int chip, int channel, unsigned long duty_ns)
{
    char path[MAX_LEN];
    snprintf(path, sizeof(path), "%s/pwmchip%d/pwm%d/duty_cycle",
             SYSFS_PWM_DIR, chip, channel);
    char value[32];
    snprintf(value, sizeof(value), "%lu", duty_ns);
    return write_sysfs_attr(path, value);
}

// 启用/禁用 PWM
int pwm_enable(int chip, int channel, int enable)
{
    char path[MAX_LEN];
    snprintf(path, sizeof(path), "%s/pwmchip%d/pwm%d/enable",
             SYSFS_PWM_DIR, chip, channel);
    return write_sysfs_attr(path, enable ? "1" : "0");
}

// 读取 PWM 值
unsigned long pwm_read_value(int chip, int channel, const char *attr)
{
    char path[MAX_LEN];
    snprintf(path, sizeof(path), "%s/pwmchip%d/pwm%d/%s",
             SYSFS_PWM_DIR, chip, channel, attr);

    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        perror("Failed to open sysfs attribute");
        return 0;
    }

    char buffer[32];
    ssize_t len = read(fd, buffer, sizeof(buffer) - 1);
    close(fd);

    if (len > 0) {
        buffer[len] = '\0';
        return strtoul(buffer, NULL, 10);
    }

    return 0;
}

void print_usage(const char *program)
{
    printf("K230 PWM Basic Test\n");
    printf("====================\n\n");
    printf("Usage: %s [chip] [channel] [frequency] [duty]\n\n", program);
    printf("Arguments:\n");
    printf("  chip     - PWM chip number (0 or 3), default: 0\n");
    printf("  channel  - PWM channel number (0, 1, 2), default: 0\n");
    printf("  frequency- Frequency in Hz, default: 1000\n");
    printf("  duty     - Duty cycle (0-100), default: 50\n\n");
    printf("Examples:\n");
    printf("  %s           # Default: pwmchip0 channel 0, 1kHz, 50%%\n", program);
    printf("  %s 0 2 1000 50   # pwmchip0 channel 2, 1kHz, 50%%\n", program);
    printf("  %s 3 0 10000 25  # pwmchip3 channel 0, 10kHz, 25%%\n", program);
}

int main(int argc, char *argv[])
{
    int chip = 0;           // pwmchip0
    int channel = 2;        // channel 0
    int frequency = 1000;   // 1kHz
    int duty = 50;          // 50%

    // Parse arguments
    if (argc > 1) {
        if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        }
        chip = atoi(argv[1]);
    }
    if (argc > 2) {
        channel = atoi(argv[2]);
    }
    if (argc > 3) {
        frequency = atoi(argv[3]);
    }
    if (argc > 4) {
        duty = atoi(argv[4]);
    }

    // Store for cleanup handler
    current_chip = chip;
    current_channel = channel;

    printf("K230 PWM Basic Test\n");
    printf("====================\n\n");

    // Calculate period and duty cycle (ns)
    unsigned long period_ns = 1000000000UL / frequency;
    unsigned long duty_ns = period_ns * duty / 100;

    printf("Configuring PWM %d:%d\n", chip, channel);
    printf("  Frequency: %d Hz\n", frequency);
    printf("  Period: %lu ns\n", period_ns);
    printf("  Duty: %d%% (%lu ns)\n\n", duty, duty_ns);


    //pwm_enable(chip, channel, 0);
    pwm_unexport(chip, channel);
    usleep(10);

    // Export PWM
    printf("Exporting PWM...\n");
    if (pwm_export(chip, channel) < 0) {
        printf("PWM already exported or error\n");
    }
    pwm_enable(chip, channel, 1);

    // Set parameters
    pwm_set_polarity(chip, channel, "inversed");
    pwm_set_period(chip, channel, period_ns);
    pwm_set_duty_cycle(chip, channel, duty_ns);



    // Read and verify
    printf("\nCurrent Configuration:\n");
    printf("  Period: %lu ns\n", pwm_read_value(chip, channel, "period"));
    printf("  Duty: %lu ns\n", pwm_read_value(chip, channel, "duty_cycle"));
    printf("  Enable: %lu\n", pwm_read_value(chip, channel, "enable"));

    printf("\nPWM running... Press Ctrl+C to stop.\n");

    // Install signal handler for Ctrl+C
    signal(SIGINT, cleanup_handler);

    // Run 30 seconds
    while(1) sleep(30);
    return 0;
}
