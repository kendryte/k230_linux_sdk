#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <gpiod.h>

#define LED_GPIO_CHIP_PATH "/dev/gpiochip1"
#define LED_LINE_NUM       20

// Blink interval (milliseconds), can be specified via command line argument
#define DEFAULT_INTERVAL_MS 500

int main(int argc, char *argv[]) {
    struct gpiod_chip *led_chip = NULL;
    struct gpiod_line_settings *settings = NULL;
    struct gpiod_line_config *line_cfg = NULL;
    struct gpiod_request_config *req_cfg = NULL;
    struct gpiod_line_request *led_request = NULL;
    unsigned int offsets[1];
    int interval_ms = DEFAULT_INTERVAL_MS;
    int ret = EXIT_FAILURE;

    // Parse command line argument: led_blink [interval_ms]
    if (argc > 1) {
        interval_ms = atoi(argv[1]);
        if (interval_ms <= 0) {
            fprintf(stderr, "Invalid interval time: %s\n", argv[1]);
            fprintf(stderr, "Usage: %s [interval_ms]\n", argv[0]);
            return EXIT_FAILURE;
        }
    }

    printf("LED Blink Test - Interval: %d ms\n", interval_ms);

    // 1. Open LED GPIO chip controller
    led_chip = gpiod_chip_open(LED_GPIO_CHIP_PATH);
    if (!led_chip) {
        perror("Failed to open LED gpiochip");
        goto cleanup;
    }

    // 2. Create and configure pin settings
    settings = gpiod_line_settings_new();
    if (!settings) {
        perror("Failed to create line settings");
        goto cleanup;
    }

    // 3. Create pin configuration
    line_cfg = gpiod_line_config_new();
    if (!line_cfg) {
        perror("Failed to create line config");
        goto cleanup;
    }

    // Configure LED pin: set as output, initial value 0 (off)
    gpiod_line_settings_set_direction(settings, GPIOD_LINE_DIRECTION_OUTPUT);
    gpiod_line_settings_set_output_value(settings, 0);
    offsets[0] = LED_LINE_NUM;
    if (gpiod_line_config_add_line_settings(line_cfg, &offsets[0], 1, settings) < 0) {
        perror("Failed to configure LED pin");
        goto cleanup;
    }

    // 4. Create request configuration
    req_cfg = gpiod_request_config_new();
    if (!req_cfg) {
        perror("Failed to create request config");
        goto cleanup;
    }
    gpiod_request_config_set_consumer(req_cfg, "k230_led");

    // 5. Request LED GPIO pin
    led_request = gpiod_chip_request_lines(led_chip, req_cfg, line_cfg);
    if (!led_request) {
        perror("Failed to request LED GPIO pin");
        goto cleanup;
    }

    printf("LED initialized successfully! Starting blink...\n");
    printf("LED: %s:%u\n", LED_GPIO_CHIP_PATH, LED_LINE_NUM);

    // 6. Main loop: LED blink
    while (1) {
        // LED ON
        ret = gpiod_line_request_set_value(led_request, LED_LINE_NUM, 1);
        if (ret < 0) {
            perror("Failed to write LED");
            break;
        }
        printf("LED ON\n");
        usleep(interval_ms * 1000);

        // LED OFF
        ret = gpiod_line_request_set_value(led_request, LED_LINE_NUM, 0);
        if (ret < 0) {
            perror("Failed to write LED");
            break;
        }
        printf("LED OFF\n");
        usleep(interval_ms * 1000);
    }

    ret = EXIT_SUCCESS;

cleanup:
    // Release resources
    if (led_request) gpiod_line_request_release(led_request);
    if (req_cfg) gpiod_request_config_free(req_cfg);
    if (line_cfg) gpiod_line_config_free(line_cfg);
    if (settings) gpiod_line_settings_free(settings);
    if (led_chip) gpiod_chip_close(led_chip);

    return ret;
}
