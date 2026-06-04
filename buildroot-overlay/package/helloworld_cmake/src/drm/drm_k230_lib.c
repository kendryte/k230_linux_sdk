#include "display.h"
#include "stdio.h"
#include <unistd.h>

int main() {
    struct display* display = NULL;
    struct display_plane* plane = NULL;
    struct display_buffer* buffer = NULL;

    // 1. Initialize display
    display = display_init(0);
    if (!display) {
        fprintf(stderr, "Failed to initialize display\n");
        return -1;
    }
    printf("Display: %dx%d\n", display->width, display->height);

    // 2. Get RGB888 plane
    plane = display_get_plane(display, DRM_FORMAT_RGB888);
    if (!plane) {
        fprintf(stderr, "Failed to get RGB888 plane\n");
        display_exit(display);
        return -1;
    }
    printf("Plane format: %c%c%c%c\n",
           (plane->fourcc >> 0) & 0xff,
           (plane->fourcc >> 8) & 0xff,
           (plane->fourcc >> 16) & 0xff,
           (plane->fourcc >> 24) & 0xff);

    // 3. Allocate buffer
    buffer = display_allocate_buffer(plane, display->width, display->height);
    if (!buffer) {
        fprintf(stderr, "Failed to allocate buffer\n");
        display_free_plane(plane);
        display_exit(display);
        return -1;
    }

    // 4. Fill with color (RGB888: BGR format)
    uint8_t* map = (uint8_t*)buffer->map;
    for (int i = 0; i < display->height; i++) {
        for (int j = 0; j < display->width; j++) {
            map[i * buffer->stride + j * 3 + 0] = 0x00;  // B
            map[i * buffer->stride + j * 3 + 1] = 0xFF;  // G
            map[i * buffer->stride + j * 3 + 2] = 0x00;  // R
        }
    }

    // 5. Display
    if (display_commit_buffer(buffer, 0, 0) != 0) {
        fprintf(stderr, "Display failed\n");
        display_free_buffer(buffer);
        display_free_plane(plane);
        display_exit(display);
        return -1;
    }

    printf("Screen is GREEN. Waiting 5 seconds...\n");
    sleep(5);

    // 6. Cleanup
    display_free_buffer(buffer);
    display_free_plane(plane);
    display_exit(display);
    return 0;
}
