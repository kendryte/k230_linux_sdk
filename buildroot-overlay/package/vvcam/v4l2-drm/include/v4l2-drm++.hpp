#ifndef V4L2_DRM_PLUS_PLUS_HPP
#define V4L2_DRM_PLUS_PLUS_HPP

#include <cassert>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include <v4l2-drm.h>

struct display;
struct display_plane;
struct display_buffer;

//=============================================================================
// k230_osd class - OSD overlay support (pure C++, no pybind11)
//=============================================================================
class k230_osd {
public:
    k230_osd();
    ~k230_osd();

    int init(struct display* pdisplay);
    int update(void* data, size_t size);
    unsigned int fourcc_;

private:
    struct display_plane* plane_;
    struct display_buffer* buffer_[2];
    struct display_buffer* cur_buf_;
};

//=============================================================================
// V4l2Drm class - C++ wrapper for v4l2-drm C library
//=============================================================================
class V4l2Drm {
public:
    explicit V4l2Drm(size_t context_num = 1, bool osd = false);
    ~V4l2Drm();

    // Display initialization - returns (width, height) on success, (-1, -1) on
    // failure
    std::pair<int, int> drm_init(int drm_id);

    // Context configuration
    void set_context(size_t index = 0, unsigned device = 1,
                     unsigned width = 640, unsigned height = 480,
                     const std::string& format = "NV12", bool display = true);
    void set_offset(size_t index = 0, unsigned x = 0, unsigned y = 0);
    void set_crop(size_t index = 0, uint32_t w = 0, uint32_t h = 0,
                  uint32_t ox = 0, uint32_t oy = 0);
    void set_rotation(size_t index = 0, int rotation = 0);
    void set_flip(size_t index = 0, int8_t hflip = 0, int8_t vflip = 0);
    void set_buffer_num(size_t index = 0, unsigned num = 4);

    // Property getters
    size_t get_context_count() const;
    unsigned get_width(size_t index = 0) const;
    unsigned get_height(size_t index = 0) const;
    unsigned get_device(size_t index = 0) const;
    uint32_t get_video_format(size_t index = 0) const;
    std::string get_video_format_str(size_t index = 0) const;
    unsigned get_offset_x(size_t index = 0) const;
    unsigned get_offset_y(size_t index = 0) const;
    int get_rotation(size_t index = 0) const;
    int8_t get_hflip(size_t index = 0) const;
    int8_t get_vflip(size_t index = 0) const;
    unsigned get_frame_count(size_t index = 0) const;
    void set_frame_count(size_t index = 0, unsigned count = 0);
    int get_video_fd(size_t index = 0) const;

    // Buffer access (pure C++ - returns raw pointer + size)
    struct BufferInfo {
        void* ptr;
        size_t size;
    };
    BufferInfo get_buffer_data(size_t index = 0);

    // Buffer array access - returns shape/strides for numpy-like access
    struct ArrayInfo {
        void* ptr;
        std::vector<ssize_t> shape;
        std::vector<ssize_t> strides;
        size_t elem_size;  // bytes per element (always 1 for uint8)
    };
    ArrayInfo get_buffer_array(size_t index = 0);

    // Setup and control
    bool setup();
    bool dump_start(size_t index = 0);
    bool dump_stop(size_t index = 0);

    // Frame capture
    bool dump_frame(size_t index, int timeout_ms);
    bool dump_release(size_t index);

    // Display run loop
    void display_start();
    void display_stop();

    // Display frame count access
    unsigned int get_display_frame_count() { return display_frame_count_; }
    void set_display_frame_count(unsigned int count) {
        display_frame_count_ = count;
    }

    // OSD methods
    void set_osd_format(unsigned int fourcc);
    int osd_update(void* data, size_t size);

    // Access internal display pointer (for LVGL integration)
    struct display* get_display() const { return display_; }

private:
    struct display* display_;
    size_t context_num_;
    struct v4l2_drm_context* contexts_;
    k230_osd* posd_;
    std::thread display_thread_;
    static unsigned int display_frame_count_;  // Static for callback access

    void check_index(size_t index) const;
    void check_display_thread() const;
    static int callback_wrapper(struct v4l2_drm_context* ctx, bool displayed);
    void display_thread_func();

    // Helper functions
    std::string fourcc_to_string(uint32_t fourcc) const;
    uint32_t string_to_fourcc(const std::string& str) const;
    size_t calc_buffer_size(struct v4l2_drm_context* ctx) const;
    void check_buffer(struct v4l2_drm_context* ctx) const;
};

#endif  // V4L2_DRM_PLUS_PLUS_HPP
