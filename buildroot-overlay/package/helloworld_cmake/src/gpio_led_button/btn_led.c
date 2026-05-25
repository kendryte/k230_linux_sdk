#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <gpiod.h>
#include <poll.h>

// Modify according to your K230 board pins
#define LED_GPIO_CHIP_PATH   "/dev/gpiochip1"
#define LED_LINE_NUM         20
#define BUTTON_GPIO_CHIP_PATH "/dev/gpiochip0"
#define BUTTON_LINE_NUM      21

// Helper function to print event type
static void print_edge_event_type(unsigned int event_type, int current_val) {
    switch (event_type) {
        case GPIOD_EDGE_EVENT_RISING_EDGE:
            printf("[EVENT] Button released! (Level: %d)\n", current_val);
            break;
        case GPIOD_EDGE_EVENT_FALLING_EDGE:
            printf("[EVENT] Button pressed! (Level: %d)\n", current_val);
            break;
        default:
            printf("[EVENT] Unknown event type: %u, Level: %d\n", event_type, current_val);
            break;
    }
}

int main(void) {
    struct gpiod_chip *led_chip = NULL;
    struct gpiod_chip *button_chip = NULL;
    struct gpiod_line_settings *settings = NULL;
    struct gpiod_line_config *line_cfg = NULL;
    struct gpiod_request_config *req_cfg = NULL;
    struct gpiod_line_request *led_request = NULL;
    struct gpiod_line_request *button_request = NULL;
    struct gpiod_edge_event_buffer *event_buf = NULL;
    unsigned int offsets[1];
    int ret = EXIT_FAILURE;

    // 1. Open LED GPIO chip controller
    led_chip = gpiod_chip_open(LED_GPIO_CHIP_PATH);
    if (!led_chip) {
        perror("Failed to open LED gpiochip");
        goto cleanup;
    }

    // 2. Open Button GPIO chip controller
    button_chip = gpiod_chip_open(BUTTON_GPIO_CHIP_PATH);
    if (!button_chip) {
        perror("Failed to open Button gpiochip");
        goto cleanup;
    }

    // 3. Create and configure pin settings
    settings = gpiod_line_settings_new();
    if (!settings) {
        perror("Failed to create line settings");
        goto cleanup;
    }

    // 4. Create LED pin configuration
    line_cfg = gpiod_line_config_new();
    if (!line_cfg) {
        perror("Failed to create line config");
        goto cleanup;
    }

    // Configure LED pin: set as output
    gpiod_line_settings_set_direction(settings, GPIOD_LINE_DIRECTION_OUTPUT);
    offsets[0] = LED_LINE_NUM;
    if (gpiod_line_config_add_line_settings(line_cfg, &offsets[0], 1, settings) < 0) {
        perror("Failed to configure LED pin");
        goto cleanup;
    }

    // 5. Create LED pin request configuration
    req_cfg = gpiod_request_config_new();
    if (!req_cfg) {
        perror("Failed to create request config");
        goto cleanup;
    }
    gpiod_request_config_set_consumer(req_cfg, "k230_led");

    // 6. Request LED GPIO pin
    led_request = gpiod_chip_request_lines(led_chip, req_cfg, line_cfg);
    if (!led_request) {
        perror("Failed to request LED GPIO pin");
        goto cleanup;
    }

    // 7. Reset configuration for Button (input + both-edge interrupt)
    gpiod_line_config_free(line_cfg);
    line_cfg = gpiod_line_config_new();
    if (!line_cfg) {
        perror("Failed to create line config");
        goto cleanup;
    }

    gpiod_line_settings_set_direction(settings, GPIOD_LINE_DIRECTION_INPUT);
    gpiod_line_settings_set_edge_detection(settings, GPIOD_LINE_EDGE_BOTH);

    offsets[0] = BUTTON_LINE_NUM;
    if (gpiod_line_config_add_line_settings(line_cfg, &offsets[0], 1, settings) < 0) {
        perror("Failed to configure Button pin");
        goto cleanup;
    }

    // 8. Create Button pin request configuration
    gpiod_request_config_free(req_cfg);
    req_cfg = gpiod_request_config_new();
    if (!req_cfg) {
        perror("Failed to create request config");
        goto cleanup;
    }
    gpiod_request_config_set_consumer(req_cfg, "k230_button_irq");

    // 9. Request Button GPIO pin
    button_request = gpiod_chip_request_lines(button_chip, req_cfg, line_cfg);
    if (!button_request) {
        perror("Failed to request Button GPIO pin");
        goto cleanup;
    }

    // 10. Initialize event buffer (to clear kernel interrupt queue)
    event_buf = gpiod_edge_event_buffer_new(2);
    if (!event_buf) {
        perror("Failed to create event buffer");
        goto cleanup;
    }

    // 11. Get Button poll file descriptor
    int button_fd = gpiod_line_request_get_fd(button_request);
    if (button_fd < 0) {
        perror("Failed to get poll file descriptor");
        goto cleanup;
    }

    // 12. Initialize LED to off state (LED=0 off, lights up when button pressed)
    ret = gpiod_line_request_set_value(led_request, LED_LINE_NUM, 0);
    if (ret < 0) {
        perror("Failed to initialize LED");
        goto cleanup;
    }

    printf("K230 GPIO interrupt detection initialized successfully!\n");
    printf("LED: %s:%u, Button: %s:%u\n", LED_GPIO_CHIP_PATH, LED_LINE_NUM,
           BUTTON_GPIO_CHIP_PATH, BUTTON_LINE_NUM);
    printf("Start using poll to wait for button events (CPU usage very low), press Ctrl+C to exit...\n\n");

    // 13. Main loop: wait for interrupts using poll
    while (1) {
        struct pollfd pfd = {
            .fd = button_fd,
            .events = POLLIN,
            .revents = 0
        };

        // Block indefinitely waiting for interrupt
        int poll_ret = poll(&pfd, 1, -1);
        if (poll_ret < 0) {
            perror("poll failed");
            break;
        }

        if (!(pfd.revents & POLLIN)) continue;

        size_t num_events = gpiod_line_request_read_edge_events(button_request, event_buf, 2);
        if (num_events == 0) continue;

        int current_val = gpiod_line_request_get_value(button_request, BUTTON_LINE_NUM);
        if (current_val < 0) {
            perror("Failed to read button value");
            break;
        }

        for (size_t i = 0; i < num_events; i++) {
            struct gpiod_edge_event *event = gpiod_edge_event_buffer_get_event(event_buf, i);
            if (!event) continue;

            unsigned int event_type = gpiod_edge_event_get_event_type(event);
            print_edge_event_type(event_type, current_val);

            gpiod_line_request_set_value(led_request, LED_LINE_NUM, !current_val);
        }
    }

    ret = EXIT_SUCCESS;

cleanup:
    if (event_buf) gpiod_edge_event_buffer_free(event_buf);
    if (button_request) gpiod_line_request_release(button_request);
    if (led_request) gpiod_line_request_release(led_request);
    if (req_cfg) gpiod_request_config_free(req_cfg);
    if (line_cfg) gpiod_line_config_free(line_cfg);
    if (settings) gpiod_line_settings_free(settings);
    if (button_chip) gpiod_chip_close(button_chip);
    if (led_chip) gpiod_chip_close(led_chip);

    return ret;
}
