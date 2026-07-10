/**
 * @file lvgl_pybind_helpers.h
 * @brief Helper classes and functions for LVGL pybind11 bindings
 *
 * Provides:
 * - LvObjWrapper: RAII wrapper for lv_obj_t* pointers
 * - CallbackManager: Manages Python callable → C callback mapping
 * - GIL helpers for event loop integration
 */

#ifndef LVGL_PYBIND_HELPERS_H
#define LVGL_PYBIND_HELPERS_H

#include <pybind11/pybind11.h>
#include <pybind11/functional.h>
#include <pybind11/stl.h>

#include "lvgl/lvgl.h"

#include <unordered_map>
#include <functional>
#include <mutex>

namespace py = pybind11;

// ============================================================================
// LvObjWrapper - wraps lv_obj_t* for pybind11
// ============================================================================

class LvObjWrapper {
public:
    LvObjWrapper() : obj_(nullptr), owned_(false) {}
    explicit LvObjWrapper(lv_obj_t *obj, bool owned = false)
        : obj_(obj), owned_(owned) {}

    lv_obj_t *get() const { return obj_; }
    void set(lv_obj_t *obj, bool owned = false) { obj_ = obj; owned_ = owned; }
    bool is_valid() const { return obj_ != nullptr; }
    bool is_owned() const { return owned_; }

    // Enable implicit conversion for convenience
    operator lv_obj_t*() const { return obj_; }

private:
    lv_obj_t *obj_;
    bool owned_;
};

// ============================================================================
// LvDisplayWrapper - wraps lv_display_t* for pybind11
// ============================================================================

class LvDisplayWrapper {
public:
    LvDisplayWrapper() : disp_(nullptr) {}
    explicit LvDisplayWrapper(lv_display_t *disp) : disp_(disp) {}

    lv_display_t *get() const { return disp_; }
    bool is_valid() const { return disp_ != nullptr; }
    operator lv_display_t*() const { return disp_; }

private:
    lv_display_t *disp_;
};

// ============================================================================
// LvIndevWrapper - wraps lv_indev_t* for pybind11
// ============================================================================

class LvIndevWrapper {
public:
    LvIndevWrapper() : indev_(nullptr) {}
    explicit LvIndevWrapper(lv_indev_t *indev) : indev_(indev) {}

    lv_indev_t *get() const { return indev_; }
    bool is_valid() const { return indev_ != nullptr; }
    operator lv_indev_t*() const { return indev_; }

private:
    lv_indev_t *indev_;
};

// ============================================================================
// CallbackManager - manages Python callables for LVGL callbacks
// ============================================================================

class __attribute__((visibility("hidden"))) CallbackManager {
public:
    static CallbackManager& instance() {
        static CallbackManager inst;
        return inst;
    }

    // Store a Python callback and return a handle ID
    uint64_t store(py::function cb) {
        std::lock_guard<std::mutex> lock(mutex_);
        uint64_t id = next_id_++;
        callbacks_[id] = std::move(cb);
        return id;
    }

    // Retrieve a Python callback by handle ID
    py::function* get(uint64_t id) {
        auto it = callbacks_.find(id);
        if (it != callbacks_.end()) {
            return &(it->second);
        }
        return nullptr;
    }

    // Remove a callback
    void remove(uint64_t id) {
        std::lock_guard<std::mutex> lock(mutex_);
        callbacks_.erase(id);
    }

private:
    CallbackManager() : next_id_(1) {}
    std::unordered_map<uint64_t, py::function> callbacks_;
    std::mutex mutex_;
    uint64_t next_id_;
};

// ============================================================================
// Event callback helper - for lv_obj_add_event_cb
// ============================================================================

// Generic event callback trampoline that calls a Python function
inline void lvgl_event_callback_trampoline(lv_event_t *e) {
    void *user_data = lv_event_get_user_data(e);
    if (!user_data) return;

    uint64_t cb_id = reinterpret_cast<uintptr_t>(user_data);
    py::function *cb = CallbackManager::instance().get(cb_id);
    if (!cb || !*cb) return;

    py::gil_scoped_acquire gil;
    try {
        // Pass the event code as an argument so Python knows what happened
        lv_event_code_t code = lv_event_get_code(e);
        (*cb)(static_cast<int>(code));
    } catch (py::error_already_set &ex) {
        ex.discard_as_unraisable("LVGL event callback");
    }
}

// Helper to register a Python callback for an LVGL event
inline void register_event_callback(lv_obj_t *obj, lv_event_code_t filter, py::function callback) {
    uint64_t cb_id = CallbackManager::instance().store(std::move(callback));
    lv_obj_add_event_cb(obj, lvgl_event_callback_trampoline, filter,
                        reinterpret_cast<void*>(static_cast<uintptr_t>(cb_id)));
}

// ============================================================================
// Display flush callback helper
// ============================================================================

inline void lvgl_flush_callback_trampoline(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map) {
    void *user_data = lv_display_get_user_data(disp);
    if (!user_data) {
        lv_display_flush_ready(disp);
        return;
    }

    uint64_t cb_id = reinterpret_cast<uintptr_t>(user_data);
    py::function *cb = CallbackManager::instance().get(cb_id);
    if (!cb || !*cb) {
        lv_display_flush_ready(disp);
        return;
    }

    py::gil_scoped_acquire gil;
    try {
        (*cb)(area->x1, area->y1, area->x2, area->y2);
    } catch (py::error_already_set &ex) {
        ex.discard_as_unraisable("LVGL flush callback");
    }
    // Note: Python code must call lv_display_flush_ready() when done
}

// ============================================================================
// Utility: Convert lv_color_t to/from Python
// ============================================================================

inline lv_color_t py_color_to_lv(py::object color_obj) {
    // Accept int (hex) or tuple (r, g, b)
    if (py::isinstance<py::int_>(color_obj)) {
        return lv_color_hex(color_obj.cast<uint32_t>());
    } else if (py::isinstance<py::tuple>(color_obj)) {
        auto t = color_obj.cast<py::tuple>();
        uint8_t r = t[0].cast<uint8_t>();
        uint8_t g = t[1].cast<uint8_t>();
        uint8_t b = t[2].cast<uint8_t>();
        return lv_color_make(r, g, b);
    }
    return lv_color_black();
}

// ============================================================================
// GIL-aware timer handler
// ============================================================================

inline uint32_t py_timer_handler() {
    py::gil_scoped_release release;  // Release GIL so LVGL can work
    uint32_t time_till_next = lv_timer_handler();
    return time_till_next;  // Re-acquire GIL automatically on return
}

#endif // LVGL_PYBIND_HELPERS_H
