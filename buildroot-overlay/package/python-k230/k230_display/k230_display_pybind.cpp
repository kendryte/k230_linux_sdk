#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <opencv2/opencv.hpp>
#include <display.h>

#include <cstdio>
#include <cstring>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>
#include "thead.h"
#include <riscv_vector.h>
#include <pthread.h>
#include <xf86drm.h>
#include <xf86drmMode.h>

namespace py = pybind11;

// 全局变量 - 双缓冲
static struct display* g_display = nullptr;
static struct display_plane* g_plane = nullptr;
static struct display_buffer* g_buffers[2] = {nullptr, nullptr};  // ping-pong buffers
static unsigned char  g_current_buffer_idx = 0;

extern int display_commit_buffer_new(const struct display_buffer* buffer, uint32_t x, uint32_t y);

static double get_time_ms() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000.0 + ts.tv_nsec / 1e6;
}


/**
 * @brief 使用跨步加载/存储适配极早期 RVV 0.10 工具链（BGR 3通道）
 * 完全避开 vuint8m1x3_t 元组类型，彻底解决编译未定义问题
 */
static void rotate90_clockwise_rvv_block(const cv::Mat& src, cv::Mat& dst, int block_size = 64) {
    int H = src.rows;
    int W = src.cols;

    dst.create(W, H, src.type());

    const uint8_t* src_ptr = src.data;
    uint8_t* dst_ptr       = dst.data;

    size_t src_stride = src.step;
    size_t dst_stride = dst.step;

    for (int by = 0; by < H; by += block_size) {
        for (int bx = 0; bx < W; bx += block_size) {
            int tile_h = std::min(block_size, H - by);
            int tile_w = std::min(block_size, W - bx);

            for (int y = 0; y < tile_h; y++) {
                const uint8_t* srow = src_ptr + (by + y) * src_stride + bx * 3;
                uint8_t* dcol       = dst_ptr + bx * dst_stride + (H - 1 - (by + y)) * 3;

                int width_left = tile_w;
                const uint8_t* sptr = srow;
                uint8_t* dptr = dcol;

                while (width_left > 0) {
                    size_t vl = __riscv_vsetvl_e8m1(width_left);

                    // 使用跨步加载（Strided Load）代替分段加载（Segment Load）
                    // 每一个单通道像素在源内存中横向相隔 3 个字节
                    vuint8m1_t v_b = __riscv_vlse8_v_u8m1(sptr + 0, 3, vl);
                    vuint8m1_t v_g = __riscv_vlse8_v_u8m1(sptr + 1, 3, vl);
                    vuint8m1_t v_r = __riscv_vlse8_v_u8m1(sptr + 2, 3, vl);

                    // 跨步存储写入目标列：目标内存中纵向相隔 dst_stride 个字节
                    __riscv_vsse8_v_u8m1(dptr + 0, dst_stride, v_b, vl);
                    __riscv_vsse8_v_u8m1(dptr + 1, dst_stride, v_g, vl);
                    __riscv_vsse8_v_u8m1(dptr + 2, dst_stride, v_r, vl);

                    sptr += vl * 3;          // 源指针横向右移
                    dptr += vl * dst_stride; // 目标指针纵向垂直下移
                    width_left -= vl;
                }
            }
        }
    }
}

//bgr
bool init_display(int dev_id = 0, uint32_t drm_format = DRM_FORMAT_BGR888) {
    if (g_display) {
        fprintf(stderr, "[k230_display] init: already initialized, skip\n");
        return true;
    }

    g_display = display_init(dev_id);
    if (!g_display) {
        PyErr_SetString(PyExc_RuntimeError, "Failed to initialize display");
        return false;
    }

    g_plane = display_get_plane(g_display, drm_format);
    if (!g_plane) {
        PyErr_SetString(PyExc_RuntimeError, "Failed to get display plane");
        display_exit(g_display);
        g_display = nullptr;
        return false;
    }

    g_buffers[0] = display_allocate_buffer(g_plane, g_display->width, g_display->height);
    if (!g_buffers[0]) {
        PyErr_SetString(PyExc_RuntimeError, "Failed to allocate buffer");
        display_free_plane(g_plane);
        g_plane = nullptr;
        display_exit(g_display);
        g_display = nullptr;
        return false;
    }

    g_buffers[1] = display_allocate_buffer(g_plane, g_display->width, g_display->height);
    if (!g_buffers[1]) {
        PyErr_SetString(PyExc_RuntimeError, "Failed to allocate second buffer");
        display_free_buffer(g_buffers[0]);
        g_buffers[0] = nullptr;
        display_free_plane(g_plane);
        g_plane = nullptr;
        display_exit(g_display);
        g_display = nullptr;
        return false;
    }

    if (display_commit_buffer_new(g_buffers[0], 0, 0) != 0) {
        fprintf(stderr, "[k230_display] warning: initial commit failed\n");
    }
    return true;
}

bool show_display(py::array img_array, int x = 0, int y = 0) {

    double show_display_start = get_time_ms();
    if (!g_display) {
        PyErr_SetString(PyExc_RuntimeError, "Display not initialized. Call k230_display.init() first.");
        return false;
    }

    py::buffer_info buf = img_array.request();
    if (buf.ndim != 3) {
        PyErr_SetString(PyExc_ValueError, "Image must be 3-dimensional (height, width, channels)");
        return false;
    }

    int img_h = static_cast<int>(buf.shape[0]);
    int img_w = static_cast<int>(buf.shape[1]);
    int img_c = static_cast<int>(buf.shape[2]);

    if (img_w <= 0 || img_h <= 0) {
        PyErr_SetString(PyExc_ValueError, "Invalid image dimensions");
        return false;
    }

    if (img_c != 3) {
        PyErr_SetString(PyExc_ValueError, "Image must have 3 channels (BGR)");
        return false;
    }

    size_t img_stride = buf.strides[0];
    cv::Mat img(img_h, img_w, CV_8UC3, buf.ptr, img_stride);

    // 竖屏时图片是横屏构建的，需要交换宽高比较
    bool is_portrait = g_display->width < g_display->height;
    int display_w = is_portrait ? g_display->height : g_display->width;
    int display_h = is_portrait ? g_display->width : g_display->height;

    if (img_w > display_w || img_h > display_h) {
        PyErr_SetString(PyExc_ValueError, "Image size exceeds display size");
        return false;
    }

    if (x < 0 || y < 0) {
        PyErr_SetString(PyExc_ValueError, "Coordinates must be non-negative");
        return false;
    }

    g_current_buffer_idx = 1 - g_current_buffer_idx;
    struct display_buffer* buffer = g_buffers[g_current_buffer_idx];
    uint8_t* dst_start = (uint8_t*)buffer->map;
    int dst_stride = buffer->stride;


    // 直接使用 BGR 图像
    cv::Mat display_img = img;
    cv::Mat rotated_img;

    if (g_display->width < g_display->height) {
        rotate90_clockwise_rvv_block(display_img, rotated_img, 64);
        memcpy(dst_start, rotated_img.data, buffer->size);
    } else {
        memcpy(dst_start, display_img.data, buffer->size);
    }

    thead_csi_dcache_clean_invalid_range(dst_start, buffer->size);

    // 异步渲染：commit 不等待
    if (display_commit_buffer_new(buffer, x, y) != 0) {
        PyErr_SetString(PyExc_RuntimeError, "Failed to commit buffer to display");
        return false;
    }

    //fprintf(stderr, "[%.2f]  use : %.2f ms\n", get_time_ms(), get_time_ms() - show_display_start);
    return true;
}

void deinit_display() {
    if (!g_display) return;

    display_free_buffer(g_buffers[0]);
    display_free_buffer(g_buffers[1]);
    display_free_plane(g_plane);
    display_exit(g_display);
    g_buffers[0] = nullptr;
    g_buffers[1] = nullptr;
    g_plane = nullptr;
    g_display = nullptr;
    g_current_buffer_idx = 0;
}

int get_width() { return g_display ? g_display->width : 0; }
int get_height() { return g_display ? g_display->height : 0; }

PYBIND11_MODULE(k230_display, m) {
    m.doc() = "K230 DRM display Python Module (pybind11 + OpenCV + RVV)";

    m.def("init", &init_display,
          py::arg("dev_id") = 0,
          py::arg("drm_format") = DRM_FORMAT_BGR888,
          "Initialize display device");

    m.def("show", &show_display,
          py::arg("img"),
          py::arg("x") = 0,
          py::arg("y") = 0,
          "Show image on display (RGB format)");

    m.def("deinit", &deinit_display,
          "Deinitialize and release display");

    m.def("get_width", &get_width, "Get display width");
    m.def("get_height", &get_height, "Get display height");
}
