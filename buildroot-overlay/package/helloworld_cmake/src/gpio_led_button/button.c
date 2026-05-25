#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <gpiod.h>
#include <poll.h>

// Modify according to your K230 board pins
#define BUTTON_GPIO_CHIP_PATH "/dev/gpiochip0"
#define BUTTON_LINE_NUM       21

// Helper function to print event type
static void print_edge_event_type(unsigned int event_type) {
    switch (event_type) {
        case GPIOD_EDGE_EVENT_RISING_EDGE:
            printf("[EVENT] Button released! (Event: RISING)\n");
            break;
        case GPIOD_EDGE_EVENT_FALLING_EDGE:
            printf("[EVENT] Button pressed! (Event: FALLING)\n");
            break;
        default:
            printf("[EVENT] Unknown event type: %u\n", event_type);
            break;
    }
}

int main(void) {
    struct gpiod_chip *button_chip = NULL;
    struct gpiod_line_settings *settings = NULL;
    struct gpiod_line_config *line_cfg = NULL;
    struct gpiod_request_config *req_cfg = NULL;
    struct gpiod_line_request *button_request = NULL;
    unsigned int offsets[1];
    int ret = EXIT_FAILURE;

    // 1. Open Button GPIO chip controller
    button_chip = gpiod_chip_open(BUTTON_GPIO_CHIP_PATH);
    if (!button_chip) {
        perror("Failed to open Button gpiochip");
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

    // Configure Button pin: set as input
    gpiod_line_settings_set_direction(settings, GPIOD_LINE_DIRECTION_INPUT);

    // Enable both-edge interrupt detection
    gpiod_line_settings_set_edge_detection(settings, GPIOD_LINE_EDGE_BOTH);

    offsets[0] = BUTTON_LINE_NUM;
    if (gpiod_line_config_add_line_settings(line_cfg, &offsets[0], 1, settings) < 0) {
        perror("Failed to configure Button pin");
        goto cleanup;
    }

    // 4. Create request configuration
    req_cfg = gpiod_request_config_new();
    if (!req_cfg) {
        perror("Failed to create request config");
        goto cleanup;
    }
    gpiod_request_config_set_consumer(req_cfg, "k230_button_irq");

    // 5. Request Button GPIO pin
    button_request = gpiod_chip_request_lines(button_chip, req_cfg, line_cfg);
    if (!button_request) {
        perror("Failed to request Button GPIO pin");
        goto cleanup;
    }

    // 6. Initialize event buffer (to clear kernel interrupt queue)
    // Capacity of 2 is sufficient for button debouncing and accumulated edge events
    struct gpiod_edge_event_buffer *event_buf = gpiod_edge_event_buffer_new(2);
    if (!event_buf) {
        perror("Failed to create event buffer");
        goto cleanup;
    }

    // 7. Get file descriptor for poll
    int fd = gpiod_line_request_get_fd(button_request);
    if (fd < 0) {
        perror("Failed to get poll file descriptor");
        goto cleanup;
    }

    printf("K230 Button interrupt detection initialized successfully!\n");
    printf("Button: %s, Line: %u\n", BUTTON_GPIO_CHIP_PATH, BUTTON_LINE_NUM);
    printf("Start using poll to wait for button events (CPU usage very low), press Ctrl+C to exit...\n\n");

    // 8. Main loop: wait for interrupts using poll
    while (1) {
        struct pollfd pfd = {
            .fd = fd,
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

        for (size_t i = 0; i < num_events; i++) {
            struct gpiod_edge_event *event = gpiod_edge_event_buffer_get_event(event_buf, i);
            if (!event) continue;

            unsigned int event_type = gpiod_edge_event_get_event_type(event);
            print_edge_event_type(event_type);
        }
    }

    ret = EXIT_SUCCESS;

cleanup:
    if (event_buf) gpiod_edge_event_buffer_free(event_buf);
    if (button_request) gpiod_line_request_release(button_request);
    if (req_cfg) gpiod_request_config_free(req_cfg);
    if (line_cfg) gpiod_line_config_free(line_cfg);
    if (settings) gpiod_line_settings_free(settings);
    if (button_chip) gpiod_chip_close(button_chip);

    return ret;
}
