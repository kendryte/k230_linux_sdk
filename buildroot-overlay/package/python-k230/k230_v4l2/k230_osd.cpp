#include "k230_osd.hpp"

#include <cstdio>
#include <cstring>

#include "thead.h"
#include <display.h>
#include <drm_fourcc.h>

//=============================================================================
// k230_osd implementation
//=============================================================================

k230_osd::k230_osd()
    : plane_(nullptr), buffer_{nullptr, nullptr}, cur_buf_(nullptr) {}

k230_osd::~k230_osd() {
    display_free_buffer(buffer_[0]);
    display_free_buffer(buffer_[1]);

    if (plane_)
        display_free_plane(plane_);
}

int k230_osd::init(struct display* pdisplay) {
    if (pdisplay == nullptr)
        return 0;

    plane_ = display_get_plane(pdisplay, DRM_FORMAT_ARGB8888);
    if (!plane_) {
        return 1;
    }

    buffer_[0] =
        display_allocate_buffer(plane_, pdisplay->width, pdisplay->height);
    if (!buffer_[0]) {
        display_free_plane(plane_);
        plane_ = nullptr;
        return 2;
    }

    buffer_[1] =
        display_allocate_buffer(plane_, pdisplay->width, pdisplay->height);
    if (!buffer_[1]) {
        display_free_buffer(buffer_[0]);
        buffer_[0] = nullptr;
        display_free_plane(plane_);
        plane_ = nullptr;
        return 3;
    }

    return 0;
}

int k230_osd::update(py::array img_array) {
    if (!plane_ || !buffer_[0] || !buffer_[1]) {
        return -1; // OSD not initialized
    }

    // 切换到另一个缓冲区
    cur_buf_ = (cur_buf_ == buffer_[0]) ? buffer_[1] : buffer_[0];
    if (!cur_buf_ || !cur_buf_->map) {
        return -2; // Buffer not mapped
    }

    py::buffer_info buf_info = img_array.request();
    int img_h = static_cast<int>(buf_info.shape[0]);
    int img_w = static_cast<int>(buf_info.shape[1]);
    int img_c = static_cast<int>(buf_info.shape[2]);

    if (img_h * img_w * img_c != cur_buf_->size) {
        printf(
            "[k230_osd::update] size mismatch: img_h=%d, img_w=%d, img_c=%d, "
            "img_size=%d, buf_size=%d\n",
            img_h, img_w, img_c, img_h * img_w * img_c, cur_buf_->size);
        return -3;
    }

    memcpy(cur_buf_->map, buf_info.ptr, cur_buf_->size);
    thead_csi_dcache_clean_invalid_range(static_cast<uint8_t*>(cur_buf_->map),
                                         cur_buf_->size);

    // Commit the buffer (non-blocking)
    int ret = display_commit_buffer_noblock(cur_buf_, 0, 0);

    return ret;
}
