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

class __attribute__((visibility("hidden"))) LvObjWrapper {
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

    // Keep a reference to the parent Python wrapper to prevent GC from
    // collecting it while this child object is still alive. Without this,
    // Python GC could collect the parent wrapper, but the LVGL object tree
    // still references it → use-after-free crash.
    void keep_parent(py::object parent) { parent_ref_ = parent; }
    py::object get_parent_ref() const { return parent_ref_; }

    /** Infer the LVGL widget type name from the object's class.
     *  Used by the Python dispatch layer when _widget_type is not set
     *  (e.g. objects received in C callbacks like anim exec_cb). */
    const char *infer_widget_type() const {
        if (!obj_) return "obj";
        const lv_obj_class_t *cls = lv_obj_get_class(obj_);
        // Only reference class symbols that are declared as extern in LVGL headers.
        // The base obj class is "lv_obj_class" (not "lv_obj").
        if (cls == &lv_obj_class) return "obj";
        if (cls == &lv_arc_class) return "arc";
        if (cls == &lv_bar_class) return "bar";
        if (cls == &lv_slider_class) return "slider";
        if (cls == &lv_spinbox_class) return "spinbox";
        if (cls == &lv_button_class) return "button";
        if (cls == &lv_label_class) return "label";
        if (cls == &lv_image_class) return "image";
        if (cls == &lv_checkbox_class) return "checkbox";
        if (cls == &lv_dropdown_class) return "dropdown";
        if (cls == &lv_roller_class) return "roller";
        if (cls == &lv_textarea_class) return "textarea";
        if (cls == &lv_keyboard_class) return "keyboard";
        if (cls == &lv_chart_class) return "chart";
        if (cls == &lv_table_class) return "table";
        if (cls == &lv_canvas_class) return "canvas";
        if (cls == &lv_line_class) return "line";
        if (cls == &lv_switch_class) return "switch";
        if (cls == &lv_scale_class) return "scale";
        if (cls == &lv_tabview_class) return "tabview";
        if (cls == &lv_msgbox_class) return "msgbox";
        if (cls == &lv_win_class) return "win";
        if (cls == &lv_menu_class) return "menu";
        if (cls == &lv_list_class) return "list";
        if (cls == &lv_animimg_class) return "animimg";
        if (cls == &lv_calendar_class) return "calendar";
        if (cls == &lv_led_class) return "led";
        if (cls == &lv_spinner_class) return "spinner";
        if (cls == &lv_buttonmatrix_class) return "buttonmatrix";
        if (cls == &lv_spangroup_class) return "spangroup";
#if LV_USE_BARCODE
        if (cls == &lv_barcode_class) return "barcode";
#endif
#if LV_USE_QRCODE
        if (cls == &lv_qrcode_class) return "qrcode";
#endif
        return "obj";
    }

private:
    lv_obj_t *obj_;
    bool owned_;
    py::object parent_ref_;  // Prevents Python GC of parent while child lives
};

// ============================================================================
// LvFontWrapper - wraps lv_font_t* for pybind11 (e.g. freetype fonts)
// ============================================================================

class __attribute__((visibility("hidden"))) LvFontWrapper {
public:
    LvFontWrapper() : font_(nullptr), owned_(false) {}
    explicit LvFontWrapper(lv_font_t *font, bool owned = false)
        : font_(font), owned_(owned) {}

    ~LvFontWrapper() {
        /* If this wrapper owns the font (e.g. created via freetype),
         * delete it on destruction to avoid memory leaks. */
#if LV_USE_FREETYPE
        if(owned_ && font_) {
            lv_freetype_font_delete(font_);
        }
#endif
    }

    // Non-copyable to avoid double-free
    LvFontWrapper(const LvFontWrapper &) = delete;
    LvFontWrapper & operator=(const LvFontWrapper &) = delete;

    // Movable
    LvFontWrapper(LvFontWrapper &&o) noexcept : font_(o.font_), owned_(o.owned_) {
        o.font_ = nullptr;
        o.owned_ = false;
    }
    LvFontWrapper & operator=(LvFontWrapper &&o) noexcept {
        if(this != &o) {
#if LV_USE_FREETYPE
            if(owned_ && font_) lv_freetype_font_delete(font_);
#endif
            font_ = o.font_;
            owned_ = o.owned_;
            o.font_ = nullptr;
            o.owned_ = false;
        }
        return *this;
    }

    lv_font_t *get() const { return font_; }
    const lv_font_t *cget() const { return font_; }
    bool is_valid() const { return font_ != nullptr; }
    bool is_owned() const { return owned_; }

    /** Implicit conversion for use with LVGL API that takes const lv_font_t* */
    operator const lv_font_t*() const { return font_; }

private:
    lv_font_t *font_;
    bool owned_;
};

// ============================================================================
// LvDisplayWrapper - wraps lv_display_t* for pybind11
// ============================================================================

class __attribute__((visibility("hidden"))) LvDisplayWrapper {
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

class __attribute__((visibility("hidden"))) LvIndevWrapper {
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
// LvEventWrapper - wraps lv_event_t* for event callbacks (non-owning)
// ============================================================================
// Valid only during the event callback execution. Do NOT store beyond the callback.

class __attribute__((visibility("hidden"))) LvEventWrapper {
public:
    LvEventWrapper() : e_(nullptr) {}
    explicit LvEventWrapper(lv_event_t *e) : e_(e) {}

    lv_event_t *get() const { return e_; }
    bool is_valid() const { return e_ != nullptr; }

private:
    lv_event_t *e_;
};

// ============================================================================
// LvAnimWrapper - wraps lv_anim_t for pybind11 (stack-allocated anim struct)
// ============================================================================
// lv_anim_t is a value type (not a pointer), so we store it inline.
// The Python usage pattern is: a = lv.anim_t(); a.init(); a.set_...(...); a.start()

class __attribute__((visibility("hidden"))) LvAnimWrapper {
public:
    LvAnimWrapper() { lv_anim_init(&anim_); }
    // Do NOT delete the anim on destruction — lv_anim_start() copies the struct.

    lv_anim_t *get() { return &anim_; }
    const lv_anim_t *cget() const { return &anim_; }

private:
    lv_anim_t anim_;
};

// ============================================================================
// LvTimerWrapper - wraps lv_timer_t* for pybind11 (non-owning)
// ============================================================================

class __attribute__((visibility("hidden"))) LvTimerWrapper {
public:
    LvTimerWrapper() : timer_(nullptr) {}
    explicit LvTimerWrapper(lv_timer_t *timer) : timer_(timer) {}

    lv_timer_t *get() const { return timer_; }
    bool is_valid() const { return timer_ != nullptr; }

    // Delete the underlying LVGL timer
    void del() {
        if (timer_) {
            lv_timer_delete(timer_);
            timer_ = nullptr;
        }
    }

private:
    lv_timer_t *timer_;
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
// Passes an LvEventWrapper object (Event in Python) instead of just the event code int.
inline void lvgl_event_callback_trampoline(lv_event_t *e) {
    void *user_data = lv_event_get_user_data(e);
    if (!user_data) return;

    uint64_t cb_id = reinterpret_cast<uintptr_t>(user_data);
    py::function *cb = CallbackManager::instance().get(cb_id);
    if (!cb || !*cb) return;

    py::gil_scoped_acquire gil;
    try {
        LvEventWrapper evt(e);
        (*cb)(&evt);
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
// Animation exec callback trampoline
// ============================================================================
// Called by LVGL when an animation needs to update its value.
// The user_data holds the CallbackManager ID for the Python callable.
// Python signature: callback(var: LvObjWrapper, value: int) -> None

inline void lvgl_anim_exec_cb_trampoline(void *var, int32_t v) {
    // var is the animated object (set via lv_anim_set_var)
    // We don't have a separate user_data for exec_cb in lv_anim_t,
    // so we use a global mapping from anim struct address → callback ID.
    // Instead, we store the callback ID in a static map keyed by the anim's var pointer.
    // For simplicity, we use a separate static map.
}

// Static storage for animation exec callbacks: var_ptr → callback ID
#include <map>
static std::mutex g_anim_cb_mutex;
static std::map<void*, uint64_t> g_anim_exec_cbs;
static std::map<void*, uint64_t> g_anim_completed_cbs;

inline void lvgl_anim_exec_trampoline(void *var, int32_t v) {
    std::lock_guard<std::mutex> lock(g_anim_cb_mutex);
    auto it = g_anim_exec_cbs.find(var);
    if (it == g_anim_exec_cbs.end()) return;
    uint64_t cb_id = it->second;
    py::function *cb = CallbackManager::instance().get(cb_id);
    if (!cb || !*cb) return;

    py::gil_scoped_acquire gil;
    try {
        // Pass the var as LvObjWrapper and the value as int
        LvObjWrapper obj(static_cast<lv_obj_t*>(var));
        (*cb)(&obj, v);
    } catch (py::error_already_set &ex) {
        ex.discard_as_unraisable("LVGL anim exec callback");
    }
}

inline void lvgl_anim_completed_trampoline(lv_anim_t *a) {
    // The var is stored in a->var
    void *var = a->var;
    std::lock_guard<std::mutex> lock(g_anim_cb_mutex);
    auto it = g_anim_completed_cbs.find(var);
    if (it == g_anim_completed_cbs.end()) return;
    uint64_t cb_id = it->second;
    py::function *cb = CallbackManager::instance().get(cb_id);
    if (!cb || !*cb) return;

    py::gil_scoped_acquire gil;
    try {
        (*cb)();
    } catch (py::error_already_set &ex) {
        ex.discard_as_unraisable("LVGL anim completed callback");
    }
}

// Helper to register anim exec callback for a given var pointer
inline void register_anim_exec_cb(void *var, py::function callback) {
    uint64_t cb_id = CallbackManager::instance().store(std::move(callback));
    std::lock_guard<std::mutex> lock(g_anim_cb_mutex);
    g_anim_exec_cbs[var] = cb_id;
}

// Helper to register anim completed callback for a given var pointer
inline void register_anim_completed_cb(void *var, py::function callback) {
    uint64_t cb_id = CallbackManager::instance().store(std::move(callback));
    std::lock_guard<std::mutex> lock(g_anim_cb_mutex);
    g_anim_completed_cbs[var] = cb_id;
}

// ============================================================================
// Timer callback trampoline
// ============================================================================
// Called by LVGL when a timer fires.
// The timer's user_data holds the CallbackManager ID for the Python callable.
// Python signature: callback(timer: LvTimerWrapper) -> None

inline void lvgl_timer_cb_trampoline(lv_timer_t *timer) {
    void *user_data = lv_timer_get_user_data(timer);
    if (!user_data) return;

    uint64_t cb_id = reinterpret_cast<uintptr_t>(user_data);
    py::function *cb = CallbackManager::instance().get(cb_id);
    if (!cb || !*cb) return;

    py::gil_scoped_acquire gil;
    try {
        LvTimerWrapper tw(timer);
        (*cb)(&tw);
    } catch (py::error_already_set &ex) {
        ex.discard_as_unraisable("LVGL timer callback");
    }
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

// ============================================================================
// Animimg set_src helper - accepts Python list of file path strings
// ============================================================================

inline void py_animimg_set_src(lv_obj_t *obj, py::list sources) {
    size_t n = sources.size();
    // Allocate a C array of const void* pointers. The pointers and the
    // duplicated strings are intentionally leaked — LVGL stores the pointer
    // array internally and references it during animation. For typical usage
    // (set_src called once, script runs until process exit), the OS reclaims
    // the memory on exit.
    const void **c_arr = new const void*[n];
    for (size_t i = 0; i < n; i++) {
        std::string path = sources[i].cast<std::string>();
        char *c_str = new char[path.size() + 1];
        std::strcpy(c_str, path.c_str());
        c_arr[i] = c_str;
    }
    lv_animimg_set_src(obj, c_arr, n);
}

#endif // LVGL_PYBIND_HELPERS_H
