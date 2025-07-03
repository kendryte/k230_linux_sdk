#include "display.h"
#include <stdio.h>
#include <drm/drm_fourcc.h>
#include <unistd.h>


#define RECT_X 0
#define RECT_Y 0

static int g_width = 480;
static int g_height = 800;

// 在NV12格式的缓冲区中绘制黑色矩形
void draw_rectangle(struct display_buffer* buffer) {
    // Y平面
    uint8_t* y_ptr = (uint8_t*)buffer->map;
    // UV平面
    uint8_t* uv_ptr = y_ptr + buffer->stride * g_height;

    // 更新Y平面
    for (int y = RECT_Y; y < RECT_Y + g_height; y++) {
        for (int x = RECT_X; x < RECT_X + g_width; x++) {
            int y_index = y * buffer->stride + x;
            // 设置亮度值为0以表示黑色
            y_ptr[y_index] = 0;
        }
    }

    // 对于黑色矩形，UV平面不需要特别处理，因为黑色只与亮度有关
    // 这里可以选择保持UV平面不变，或者简单地将其设置为默认值（通常是128）
    // 以下是将UV平面设置为默认值的示例代码
    int uv_width = g_width / 2;
    int uv_height = g_height / 2;
    int rect_uv_x = RECT_X / 2;
    int rect_uv_y = RECT_Y / 2;
    int rect_uv_width = g_width / 2;
    int rect_uv_height = g_height / 2;

    for (int y = rect_uv_y; y < rect_uv_y + rect_uv_height; y++) {
        for (int x = rect_uv_x; x < rect_uv_x + rect_uv_width; x++) {
            int uv_index = (y * uv_width + x) * 2;
            // 设置U分量为默认值
            uv_ptr[uv_index] = 128;
            // 设置V分量为默认值
            uv_ptr[uv_index + 1] = 128;
        }
    }
}

int main(void) {
    // 初始化显示设备
    struct display* display = display_init(0);
    if (display == NULL) {
        fprintf(stderr, "Failed to initialize display\n");
        return -1;
    }

    if (display->conn->connector_type != DRM_MODE_CONNECTOR_DSI )
    {
        display_exit(display);
        return 0;
    }
    //printf("display connector type:%d\n",display->conn->connector_type);

    g_width = display->width;
    g_height = display->height;

    printf("Display width: %d, height: %d\n", g_width, g_height);

    display->drm_rotation = rotation_90;
    // 获取显示平面
    struct display_plane* plane = display_get_plane(display, DRM_FORMAT_NV12);
    if (plane == NULL) {
        fprintf(stderr, "Failed to get display plane\n");
        display_exit(display);
        return -1;
    }

    plane->drm_rotation = rotation_90; // 设置平面旋转为90度
    // 分配缓冲区
    struct display_buffer* buffer = display_allocate_buffer(plane, g_width, g_height);
    if (buffer == NULL) {
        fprintf(stderr, "Failed to allocate display buffer\n");
        display_free_plane(plane);
        display_exit(display);
        return -1;
    }

    // 绘制黑色矩形
    draw_rectangle(buffer);

    display_wait_vsync(display);

    // 更新缓冲区并提交显示
    if (display_update_buffer(buffer, 0, 0) != 0) {
        fprintf(stderr, "Failed to update display buffer\n");
    }
    if (display_commit(display) != 0) {
        fprintf(stderr, "Failed to commit display\n");
    }

    // 等待一段时间
    //sleep(1);

    // 释放资源
    display_free_buffer(buffer);
    display_free_plane(plane);
    display_exit(display);

    return 0;
}