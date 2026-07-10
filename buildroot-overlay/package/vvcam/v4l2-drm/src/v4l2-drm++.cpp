#include "../include/v4l2-drm++.hpp"

#include <algorithm>
#include <cassert>
#include <cstring>
#include <stdexcept>

#include <display.h>
#include <drm_fourcc.h>
#include <linux/videodev2.h>
#include <v4l2-drm.h>

#include "thead.h"

//=============================================================================
// k230_osd implementation
//=============================================================================

k230_osd::k230_osd()
    : plane_(nullptr), buffer_{nullptr, nullptr}, cur_buf_(nullptr), fourcc_(DRM_FORMAT_ARGB8888) {}

k230_osd::~k230_osd() {
    display_free_buffer(buffer_[0]);
    display_free_buffer(buffer_[1]);

    if (plane_)
        display_free_plane(plane_);
}

int k230_osd::init(struct display* pdisplay) {
    if (pdisplay == nullptr)
        return 0;

    plane_ = display_get_plane(pdisplay, fourcc_);
    if (!plane_) {
        return 1;
    }
    plane_->drm_rotation = pdisplay->drm_rotation;

    // OSD buffer size (ensure width >= height)
    int osd_width = std::max(pdisplay->width, pdisplay->height);
    int osd_height = std::min(pdisplay->width, pdisplay->height);

    buffer_[0] = display_allocate_buffer(plane_, osd_width, osd_height);
    if (!buffer_[0]) {
        display_free_plane(plane_);
        plane_ = nullptr;
        return 2;
    }

    buffer_[1] = display_allocate_buffer(plane_, osd_width, osd_height);
    if (!buffer_[1]) {
        display_free_buffer(buffer_[0]);
        buffer_[0] = nullptr;
        display_free_plane(plane_);
        plane_ = nullptr;
        return 3;
    }

    return 0;
}

int k230_osd::update(void* data, size_t size) {
    if (!plane_ || !buffer_[0] || !buffer_[1]) {
        return -1;  // OSD not initialized
    }

    // Switch to the other buffer
    cur_buf_ = (cur_buf_ == buffer_[0]) ? buffer_[1] : buffer_[0];
    if (!cur_buf_ || !cur_buf_->map) {
        return -2;  // Buffer not mapped
    }

    if (size != cur_buf_->size) {
        printf("[k230_osd::update] size mismatch: data_size=%zu, buf_size=%d\n",
               size, cur_buf_->size);
        return -3;
    }

    memcpy(cur_buf_->map, data, cur_buf_->size);
    thead_csi_dcache_clean_invalid_range(static_cast<uint8_t*>(cur_buf_->map),
                                         cur_buf_->size);

    // Commit the buffer (non-blocking)
    plane_->display->osd_disp_buffer = cur_buf_;

    return 0;
}

//=============================================================================
// V4l2Drm implementation
//=============================================================================

// Static member definition
unsigned int V4l2Drm::display_frame_count_ = 0;

V4l2Drm::V4l2Drm(size_t context_num, bool osd)
    : display_(nullptr), context_num_(context_num), posd_(nullptr) {
    contexts_ = new struct v4l2_drm_context[context_num_];
    for (size_t i = 0; i < context_num_; i++) {
        v4l2_drm_default_context(&contexts_[i]);
    }
    if (osd) {
        posd_ = new k230_osd;
        assert(posd_);
    }
}

V4l2Drm::~V4l2Drm() {
    // Set global flag to stop callback loop first
    v4l2_drm_run_v4l2_2_drm_need_run = 0;

    // Stop all contexts to break v4l2_drm_run loop
    for (size_t i = 0; i < context_num_; i++) {
        v4l2_drm_stop(&contexts_[i]);
    }

    // Wait for background thread to finish
    if (display_thread_.joinable()) {
        display_thread_.join();
    }

    // Free OSD first (before display_exit, as it uses display resources)
    if (posd_) {
        delete posd_;
        posd_ = nullptr;
    }

    // Then free display
    if (display_) {
        display_exit(display_);
        display_ = nullptr;
    }

    if (contexts_) {
        delete[] contexts_;
        contexts_ = nullptr;
    }
}

std::pair<int, int> V4l2Drm::drm_init(int drm_id) {
    // Check if display is already initialized
    if (display_) {
        display_exit(display_);
        display_ = nullptr;
    }

    display_ = display_init(drm_id);
    if (display_ == nullptr) {
        printf("[V4l2Drm::drm_init] display_init failed, drm_id=%d\n", drm_id);
        return {-1, -1};
    }

    printf("[V4l2Drm::drm_init] success, drm_id=%d, width=%u, height=%u\n",
           drm_id, display_->width, display_->height);
    return {static_cast<int>(display_->width),
            static_cast<int>(display_->height)};
}

void V4l2Drm::set_osd_format(unsigned int fourcc) {
    if (posd_)
        delete posd_;
    posd_ = new k230_osd;
    assert(posd_);
    posd_->fourcc_ = fourcc;
}

void V4l2Drm::check_index(size_t index) const {
    if (index >= context_num_) {
        throw std::out_of_range("Context index out of range");
    }
}

void V4l2Drm::check_display_thread() const {
    if (display_thread_.joinable()) {
        throw std::runtime_error("Display thread is already running");
    }
}

std::string V4l2Drm::fourcc_to_string(uint32_t fourcc) const {
    char buf[5] = {static_cast<char>((fourcc >> 0) & 0xff),
                   static_cast<char>((fourcc >> 8) & 0xff),
                   static_cast<char>((fourcc >> 16) & 0xff),
                   static_cast<char>((fourcc >> 24) & 0xff), 0};
    return std::string(buf);
}

uint32_t V4l2Drm::string_to_fourcc(const std::string& str) const {
    if (str.size() != 4) {
        throw std::invalid_argument(
            "FourCC string must be exactly 4 characters");
    }
    return static_cast<uint32_t>((static_cast<uint8_t>(str[0]) << 0) |
                                 (static_cast<uint8_t>(str[1]) << 8) |
                                 (static_cast<uint8_t>(str[2]) << 16) |
                                 (static_cast<uint8_t>(str[3]) << 24));
}

size_t V4l2Drm::calc_buffer_size(struct v4l2_drm_context* ctx) const {
    switch (ctx->video_format) {
        case V4L2_PIX_FMT_NV12:
        case V4L2_PIX_FMT_NV21:
            return ctx->width * ctx->height * 3 / 2;
        case V4L2_PIX_FMT_NV16:
        case V4L2_PIX_FMT_NV61:
            return ctx->width * ctx->height * 2;
        case V4L2_PIX_FMT_BGR24:
        case V4L2_PIX_FMT_RGB24:
            return ctx->width * ctx->height * 3;
        case V4L2_PIX_FMT_YUYV:
            return ctx->width * ctx->height * 2;
        case v4l2_fourcc('B', 'G', '3', 'P'):  // BGR32/BGRA32
            return ctx->width * ctx->height * 4;
        default:
            return ctx->vbuffer.length > 0 ? ctx->vbuffer.length
                                           : ctx->width * ctx->height * 3 / 2;
    }
}

void V4l2Drm::check_buffer(struct v4l2_drm_context* ctx) const {
    if (!ctx->buffers || ctx->video_fd < 0) {
        throw std::runtime_error(
            "Context not initialized or no buffers allocated");
    }
    if (!ctx->buffers[ctx->vbuffer.index].mmap) {
        throw std::runtime_error("Buffer not mapped");
    }
}

void V4l2Drm::set_context(size_t index, unsigned device, unsigned width,
                          unsigned height, const std::string& format,
                          bool display) {
    check_index(index);
    struct v4l2_drm_context* ctx = &contexts_[index];
    ctx->device = device;
    ctx->width = width;
    ctx->height = height;
    ctx->video_format = string_to_fourcc(format);
    ctx->display = display;
}

void V4l2Drm::set_offset(size_t index, unsigned x, unsigned y) {
    check_index(index);
    contexts_[index].offset_x = x;
    contexts_[index].offset_y = y;
}

void V4l2Drm::set_crop(size_t index, uint32_t w, uint32_t h, uint32_t ox,
                       uint32_t oy) {
    check_index(index);
    struct v4l2_drm_context* ctx = &contexts_[index];
    ctx->crop_size.width = w;
    ctx->crop_size.height = h;
    ctx->crop_size.offset_x = ox;
    ctx->crop_size.offset_y = oy;
    ctx->crop_size.crop_en = 1;
}

void V4l2Drm::set_rotation(size_t index, int rotation) {
    check_index(index);
    contexts_[index].drm_rotation = static_cast<drm_rotation>(rotation);
}

void V4l2Drm::set_flip(size_t index, int8_t hflip, int8_t vflip) {
    check_index(index);
    contexts_[index].hflip = hflip;
    contexts_[index].vflip = vflip;
}

void V4l2Drm::set_buffer_num(size_t index, unsigned num) {
    check_index(index);
    contexts_[index].buffer_num = num;
}

size_t V4l2Drm::get_context_count() const { return context_num_; }

unsigned V4l2Drm::get_width(size_t index) const {
    check_index(index);
    return contexts_[index].width;
}

unsigned V4l2Drm::get_height(size_t index) const {
    check_index(index);
    return contexts_[index].height;
}

unsigned V4l2Drm::get_device(size_t index) const {
    check_index(index);
    return contexts_[index].device;
}

uint32_t V4l2Drm::get_video_format(size_t index) const {
    check_index(index);
    return contexts_[index].video_format;
}

std::string V4l2Drm::get_video_format_str(size_t index) const {
    check_index(index);
    return fourcc_to_string(contexts_[index].video_format);
}

unsigned V4l2Drm::get_offset_x(size_t index) const {
    check_index(index);
    return contexts_[index].offset_x;
}

unsigned V4l2Drm::get_offset_y(size_t index) const {
    check_index(index);
    return contexts_[index].offset_y;
}

int V4l2Drm::get_rotation(size_t index) const {
    check_index(index);
    return contexts_[index].drm_rotation;
}

int8_t V4l2Drm::get_hflip(size_t index) const {
    check_index(index);
    return contexts_[index].hflip;
}

int8_t V4l2Drm::get_vflip(size_t index) const {
    check_index(index);
    return contexts_[index].vflip;
}

unsigned V4l2Drm::get_frame_count(size_t index) const {
    check_index(index);
    return contexts_[index].frame_count;
}

void V4l2Drm::set_frame_count(size_t index, unsigned count) {
    check_index(index);
    contexts_[index].frame_count = count;
}

int V4l2Drm::get_video_fd(size_t index) const {
    check_index(index);
    return contexts_[index].video_fd;
}

V4l2Drm::BufferInfo V4l2Drm::get_buffer_data(size_t index) {
    check_index(index);
    struct v4l2_drm_context* ctx = &contexts_[index];
    check_buffer(ctx);
    size_t sz = calc_buffer_size(ctx);
    void* data = ctx->buffers[ctx->vbuffer.index].mmap;
    return {data, sz};
}

V4l2Drm::ArrayInfo V4l2Drm::get_buffer_array(size_t index) {
    check_index(index);
    struct v4l2_drm_context* ctx = &contexts_[index];
    check_buffer(ctx);
    void* data = ctx->buffers[ctx->vbuffer.index].mmap;
    ssize_t w = static_cast<ssize_t>(ctx->width);
    ssize_t h = static_cast<ssize_t>(ctx->height);

    switch (ctx->video_format) {
        case V4L2_PIX_FMT_NV12:
        case V4L2_PIX_FMT_NV21:
            return {data, {h * 3 / 2, w}, {w, 1}, 1};
        case V4L2_PIX_FMT_BGR24:
        case V4L2_PIX_FMT_RGB24:
            return {data, {h, w, 3}, {w * 3, 3, 1}, 1};
        case V4L2_PIX_FMT_YUYV:
            return {data, {h, w, 2}, {w * 2, 2, 1}, 1};
        case v4l2_fourcc('B', 'G', '3', 'P'): {  // NCHW format for AI inference
            return {data, {1, 3, h, w}, {h * w * 3, h * w, w, 1}, 1};
        }
        default: {
            size_t sz = calc_buffer_size(ctx);
            return {data, {static_cast<ssize_t>(sz)}, {1}, 1};
        }
    }
}

bool V4l2Drm::setup() {
    int ret = 0;
    if (context_num_ == 0) {
        throw std::runtime_error("No contexts configured");
    }

    ret = v4l2_drm_setup(contexts_, context_num_, &display_);
    if (ret == 0 && display_ && posd_) {
        ret = posd_->init(display_);
    }

    if (ret) {
        throw std::runtime_error("setup error");
    }
    return ret == 0;
}

bool V4l2Drm::dump_start(size_t index) {
    check_index(index);
    if (contexts_[index].display)
        return false;
    return v4l2_drm_start(&contexts_[index]) == 0;
}

bool V4l2Drm::dump_stop(size_t index) {
    check_index(index);
    if (contexts_[index].display)
        return false;
    return v4l2_drm_stop(&contexts_[index]) == 0;
}

bool V4l2Drm::dump_frame(size_t index, int timeout_ms) {
    check_index(index);
    if (contexts_[index].display)
        return false;
    return v4l2_drm_dump(&contexts_[index], timeout_ms) == 0;
}

bool V4l2Drm::dump_release(size_t index) {
    check_index(index);
    if (contexts_[index].display)
        return false;
    return v4l2_drm_dump_release(&contexts_[index]) == 0;
}

void V4l2Drm::display_start() {
    int i = 0;
    check_display_thread();
    for (i = 0; i < context_num_; i++) {
        if (contexts_[i].display)
            break;
    }
    if (i == context_num_)
        throw std::runtime_error("No display context configured");

    // Start background thread
    display_thread_ = std::thread(&V4l2Drm::display_thread_func, this);
}

void V4l2Drm::display_stop() {
    // Set global flag to stop loop
    v4l2_drm_run_v4l2_2_drm_need_run = 0;
    // Wait for thread to finish
    if (display_thread_.joinable()) {
        display_thread_.join();
    }
}

void V4l2Drm::display_thread_func() {
    v4l2_drm_run_v4l2_2_drm(contexts_, context_num_, callback_wrapper);
}

int V4l2Drm::callback_wrapper(struct v4l2_drm_context* ctx, bool displayed) {
    // Check global flag first
    if (!v4l2_drm_run_v4l2_2_drm_need_run) {
        return -1;
    }
    if (!displayed)
        return 0;

    display_frame_count_++;
    return 0;
}

int V4l2Drm::osd_update(void* data, size_t size) {
    return posd_->update(data, size);
}
