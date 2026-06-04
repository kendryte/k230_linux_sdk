#include "display.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <png.h>


// Read PNG file and convert to BGR888 (3 bytes per pixel)
unsigned char* read_png_rgb888(const char* filename, int* width, int* height) {
    FILE* fp = fopen(filename, "rb");
    if (!fp) {
        fprintf(stderr, "Failed to open PNG file: %s\n", filename);
        return NULL;
    }

    unsigned char sig[8];
    if (fread(sig, 1, 8, fp) != 8) {
        fprintf(stderr, "Failed to read PNG signature\n");
        fclose(fp);
        return NULL;
    }

    if (png_sig_cmp(sig, 0, 8) != 0) {
        fprintf(stderr, "Not a valid PNG file\n");
        fclose(fp);
        return NULL;
    }

    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr) {
        fprintf(stderr, "Failed to create PNG read struct\n");
        fclose(fp);
        return NULL;
    }

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        fprintf(stderr, "Failed to create PNG info struct\n");
        png_destroy_read_struct(&png_ptr, NULL, NULL);
        fclose(fp);
        return NULL;
    }

    if (setjmp(png_jmpbuf(png_ptr))) {
        fprintf(stderr, "PNG read error\n");
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        fclose(fp);
        return NULL;
    }

    png_init_io(png_ptr, fp);
    png_set_sig_bytes(png_ptr, 8);
    png_read_info(png_ptr, info_ptr);

    // 解决类型不匹配隐患：用标准的 png_uint_32 接收
    png_uint_32 img_width, img_height;
    int bit_depth, color_type;
    png_get_IHDR(png_ptr, info_ptr, &img_width, &img_height, &bit_depth, &color_type, NULL, NULL, NULL);

    *width = (int)img_width;
    *height = (int)img_height;

    fprintf(stderr, "[read_png] PNG size: %dx%d\n", *width, *height);

    // --- 开始设置转换参数 ---

    // 16位转8位
    if (bit_depth == 16) {
        png_set_strip_16(png_ptr);
    }

    // 调色板转RGB
    if (color_type == PNG_COLOR_TYPE_PALETTE) {
        png_set_palette_to_rgb(png_ptr);
    }

    // 灰度图转RGB
    if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8) {
        png_set_expand_gray_1_2_4_to_8(png_ptr);
    }
    if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA) {
        png_set_gray_to_rgb(png_ptr);
    }

    // 核心修改 1：如果包含 Alpha 通道，直接裁剪掉，保留 3 字节的 RGB
    if (color_type & PNG_COLOR_MASK_ALPHA) {
        png_set_strip_alpha(png_ptr);
    }
    if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) {
        png_set_strip_alpha(png_ptr);
    }

    // 转换 RGB 为 BGR (方便大多数 Windows/Linux 屏幕显示库)
    png_set_bgr(png_ptr);

    // 更新转换后的图像信息
    png_read_update_info(png_ptr, info_ptr);

    // 核心修改 2：使用真实更新后的 rowbytes 分配内存
    size_t row_bytes = png_get_rowbytes(png_ptr, info_ptr);
    size_t buffer_size = row_bytes * (*height);

    unsigned char* buffer = malloc(buffer_size);
    if (!buffer) {
        fprintf(stderr, "Failed to allocate buffer\n");
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        fclose(fp);
        return NULL;
    }

    png_bytep* rows = malloc(*height * sizeof(png_bytep));
    if (!rows) {
        fprintf(stderr, "Failed to allocate rows\n");
        free(buffer);
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        fclose(fp);
        return NULL;
    }

    // 核心修改 3：依靠 row_bytes 步进，确保绝对安全
    for (int y = 0; y < *height; y++) {
        rows[y] = buffer + (size_t)y * row_bytes;
    }

    png_read_image(png_ptr, rows);
    free(rows);

    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    fclose(fp);

    return buffer;
}
int main() {
    fprintf(stderr, "[main] Starting...\n");
    struct display* display = NULL;
    struct display_plane* plane = NULL;
    struct display_buffer* buffer = NULL;

    // 1. Initialize display
    fprintf(stderr, "[main] Display init\n");
    display = display_init(0);
    if (!display) {
        fprintf(stderr, "Failed to initialize display\n");
        return -1;
    }
    fprintf(stderr, "[main] Display: %dx%d\n", display->width, display->height);

    // 2. Get RGB888 plane
    fprintf(stderr, "[main] Getting plane\n");
    plane = display_get_plane(display, DRM_FORMAT_RGB888);
    if (!plane) {
        fprintf(stderr, "Failed to get RGB888 plane\n");
        display_exit(display);
        return -1;
    }
    fprintf(stderr, "[main] Plane format: %c%c%c%c\n",
           (plane->fourcc >> 0) & 0xff,
           (plane->fourcc >> 8) & 0xff,
           (plane->fourcc >> 16) & 0xff,
           (plane->fourcc >> 24) & 0xff);

    // 3. Read PNG image
    const char* png_path = "/usr/share/images/test.png";
    int img_width, img_height;
    fprintf(stderr, "[main] Reading PNG: %s\n", png_path);

    // Try alternative paths
    if (access(png_path, F_OK) != 0) {
        png_path = "/tmp/test.png";
        if (access(png_path, F_OK) != 0) {
            png_path = "input.png";
        }
    }
    fprintf(stderr, "[main] Using path: %s\n", png_path);

    unsigned char* png_data = read_png_rgb888(png_path, &img_width, &img_height);
    if (!png_data) {
        fprintf(stderr, "Failed to read PNG image from %s\n", png_path);
        display_free_plane(plane);
        display_exit(display);
        return -1;
    }
    fprintf(stderr, "[main] PNG image: %dx%d\n", img_width, img_height);

    // 4. Allocate buffer matching display resolution
    fprintf(stderr, "[main] Allocating buffer\n");
    buffer = display_allocate_buffer(plane, display->width, display->height);
    if (!buffer) {
        fprintf(stderr, "Failed to allocate buffer\n");
        free(png_data);
        display_free_plane(plane);
        display_exit(display);
        return -1;
    }

    // 5. Copy PNG data to buffer (centered)
    fprintf(stderr, "[main] Copying image data\n");
    uint8_t* map = (uint8_t*)buffer->map;
    int x_off = (display->width - img_width) / 2;
    int y_off = (display->height - img_height) / 2;

    for (int y = 0; y < img_height; y++) {
        for (int x = 0; x < img_width; x++) {
            int src_idx = (y * img_width + x) * 3;  // RGB888 (already BGR from libpng)
            int dst_x = x + x_off;
            int dst_y = y + y_off;

            if (dst_x >= 0 && dst_x < display->width &&
                dst_y >= 0 && dst_y < display->height) {
                int dst_idx = (dst_y * display->width + dst_x) * 3;

                // RGB888: B at 0, G at 1, R at 2 (BGR order)
                map[dst_idx + 0] = png_data[src_idx + 0];  // B
                map[dst_idx + 1] = png_data[src_idx + 1];  // G
                map[dst_idx + 2] = png_data[src_idx + 2];  // R
            }
        }
    }

    // 6. Display
    fprintf(stderr, "[main] Displaying\n");
    if (display_commit_buffer(buffer, 0, 0) != 0) {
        fprintf(stderr, "Display failed\n");
        free(png_data);
        display_free_buffer(buffer);
        display_free_plane(plane);
        display_exit(display);
        return -1;
    }

    fprintf(stderr, "[main] PNG displayed! Waiting 10 seconds...\n");
    sleep(10);

    // 7. Cleanup
    fprintf(stderr, "[main] Cleaning up\n");
    free(png_data);
    display_free_buffer(buffer);
    display_free_plane(plane);
    display_exit(display);
    return 0;
}
