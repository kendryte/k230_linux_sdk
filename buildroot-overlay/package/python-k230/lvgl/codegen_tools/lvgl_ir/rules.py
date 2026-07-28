"""
API mapping rules for LVGL Python bindings.

This module contains manual overrides and special-case rules
that the automatic semantic classifier cannot handle.
"""

from __future__ import annotations
from typing import Dict, List, Optional, Set


# ---------------------------------------------------------------------------
# Functions that should be completely skipped (internal/unsafe)
# ---------------------------------------------------------------------------

SKIP_FUNCTIONS: Set[str] = {
    # Memory management - not safe to expose
    "lv_malloc", "lv_calloc", "lv_zalloc", "lv_malloc_zeroed",
    "lv_realloc", "lv_reallocf", "lv_malloc_core", "lv_realloc_core",
    "lv_free", "lv_mem_add_pool", "lv_mem_remove_pool",

    # Internal logging
    "lv_log_register_print_cb", "lv_log", "lv_log_add",

    # Internal draw pipeline
    "lv_draw_buf_init_with_default_handlers",
    "lv_draw_buf_handlers_init",
    "lv_draw_buf_get_handlers",
    "lv_draw_buf_get_font_handlers",
    "lv_draw_buf_get_image_handlers",

    # String/memory utilities (use Python equivalents)
    "lv_memcpy", "lv_memmove", "lv_memcmp",
    "lv_strlen", "lv_strnlen", "lv_strlcpy", "lv_strncpy",
    "lv_strcpy", "lv_strcmp", "lv_strncmp", "lv_strdup", "lv_strndup",
    "lv_strcat", "lv_strncat", "lv_strchr",
    "lv_snprintf", "lv_vsnprintf",

    # va_list functions
    "lv_obj_set_style_text",  # variadic, use set_text_fmt instead

    # Functions with incomplete struct parameters that cause compile errors
    "lv_chart_add_series",        # lv_chart_series_t* - incomplete type
    "lv_chart_remove_series",     # lv_chart_series_t* - incomplete type
    "lv_chart_set_series_color",  # lv_chart_series_t* - incomplete type
    "lv_chart_get_series_next",   # lv_chart_series_t* - incomplete type
    "lv_chart_set_series_value",  # lv_chart_series_t* - incomplete type
    "lv_chart_add_cursor",        # lv_chart_cursor_t* - incomplete type
    "lv_chart_set_cursor_pos",    # lv_chart_cursor_t* - incomplete type
    "lv_chart_set_cursor_point",  # lv_chart_cursor_t* - incomplete type
    "lv_chart_get_cursor_next",   # lv_chart_cursor_t* - incomplete type
    "lv_scale_add_section",       # lv_scale_section_t* - incomplete type
    "lv_scale_set_section_range", # lv_scale_section_t* - incomplete type
    "lv_image_decoder_create",    # lv_image_decoder_t* - incomplete type
    "lv_image_decoder_set_open_cb",  # lv_image_decoder_t* - incomplete type
    "lv_image_decoder_set_close_cb", # lv_image_decoder_t* - incomplete type
    "lv_image_decoder_set_get_info_cb",  # lv_image_decoder_t* - incomplete type
    "lv_image_decoder_set_open_cb",      # lv_image_decoder_t* - incomplete type
    "lv_image_decoder_open",       # lv_image_decoder_dsc_t* - incomplete type
    "lv_image_decoder_get_info",   # lv_image_decoder_dsc_t* - incomplete type
    "lv_image_decoder_close",      # lv_image_decoder_dsc_t* - incomplete type
    "lv_image_decoder_set_defs_open_cb",  # incomplete type
    "lv_image_decoder_set_defs_close_cb", # incomplete type

    # Functions with wrong argument count (libclang parses extra self param)
    "lv_obj_enable_style_refresh",       # takes bool, not obj
    "lv_obj_style_get_selector_state",   # takes selector, not obj
    "lv_obj_style_get_selector_part",    # takes selector, not obj
    "lv_canvas_buf_size",                # static calc, no obj param
    "lv_canvas_set_buffer",              # void* buf param, manual binding with uintptr_t
    "lv_line_set_points",                # const lv_point_precise_t[] param, manual binding from list
    "lv_image_src_get_type",             # takes const void*, no obj

    # Functions that return/write through non-obj double pointers
    "lv_obj_get_child_by_type",          # lv_obj_class_t* param
    "lv_obj_get_sibling_by_type",        # lv_obj_class_t* param
    "lv_obj_get_child_count_by_type",    # lv_obj_class_t* param
    "lv_obj_get_index_by_type",          # lv_obj_class_t* param

    # Delete descriptors - incomplete types
    "lv_obj_delete_anim_completed_cb",   # takes lv_anim_t*, not lv_obj_t*
    "lv_obj_delete_async_cb",            # takes lv_anim_t*, not lv_obj_t*

    # Functions with type mismatch (libclang parses wrong self)
    "lv_obj_report_style_change",        # takes lv_style_t*, not obj+style
    "lv_obj_null_on_delete",             # takes lv_obj_t**, double pointer
    "lv_image_buf_set_palette",          # takes lv_image_dsc_t*, not obj
    "lv_keyboard_get_map_array",         # returns const char* const*, not const char*
    "lv_obj_set_style_text_fmt",         # variadic
    "lv_label_set_text_fmt",             # variadic
    "lv_label_set_text_fmt_static",      # variadic
    "lv_btnmatrix_set_map",              # const char* const* param
    "lv_btnmatrix_set_ctrl_map",         # const lv_btnmatrix_ctrl_t[] param
    "lv_keyboard_set_map",               # const char* const* param

    # Functions with const-correctness issues
    "lv_obj_get_style_bg_image_src",     # returns void* but C API returns const void*
    "lv_image_buf_free",                 # takes lv_image_dsc_t*, not obj+dsc
    "lv_image_decoder_get_info",         # const void* -> void* mismatch
    "lv_image_src_get_type",             # const void* -> void* mismatch
    "lv_dropdown_get_selected_str",      # char* output buffer, const mismatch
    "lv_textarea_get_text",              # char* output buffer
    "lv_roller_get_selected_str",        # char* output buffer
    "lv_calendar_get_pressed_date",      # lv_calendar_date_t* output
    "lv_chart_set_all_dirty",            # lv_chart_series_t* incomplete
    "lv_chart_refresh",                  # lv_chart_series_t* incomplete
    "lv_obj_get_child_by_name",          # returns obj but takes parent, wrong self

    # Functions that create non-obj types via LvObjWrapper (wrong wrapper)
    "lv_display_create",                 # returns lv_display_t*, not lv_obj_t*
    "lv_display_get_default",            # returns lv_display_t*
    "lv_display_get_next",               # returns lv_display_t*
    "lv_indev_get_act",                  # returns lv_indev_t*
    "lv_indev_get_next",                 # returns lv_indev_t*
    "lv_theme_default_get",              # returns lv_theme_t*
    "lv_theme_basic_get",                # returns lv_theme_t*
    "lv_theme_mono_get",                 # returns lv_theme_t*

    # Subject/observer - internal subsystem
    "lv_subject_init_int",
    "lv_subject_init_string",
    "lv_subject_init_pointer",
    "lv_subject_init_group",
    "lv_subject_set_int",
    "lv_subject_set_string",
    "lv_subject_set_pointer",
    "lv_subject_get_int",
    "lv_subject_get_string",
    "lv_subject_get_pointer",
    "lv_subject_get_group_element",
    "lv_subject_add_observer",
    "lv_subject_add_observer_obj",
    "lv_subject_remove_observer",
    "lv_subject_notify",
    "lv_subject_notify_all",
    "lv_observer_get_target",
    "lv_obj_add_event_cb",             # duplicate of add_event_cb with extra params
    "lv_obj_remove_event_dsc",         # lv_event_dsc_t* incomplete
    "lv_obj_remove_event_cb",          # complex callback handling

    # Functions with const void* → void* return mismatch
    "lv_obj_get_style_arc_image_src",    # returns const void* but mapped to void*
    "lv_obj_get_style_bitmap_mask_src",  # returns const void* but mapped to void*
    "lv_canvas_get_buf",                 # returns const void* → void*
    "lv_image_get_src",                  # returns const void* → void*

    # Functions returning non-obj types wrapped as LvObjWrapper
    "lv_indev_create",                   # returns lv_indev_t*, not lv_obj_t*
    "lv_theme_create",                   # returns lv_theme_t*, not lv_obj_t*
    "lv_timer_create",                   # returns lv_timer_t*, not lv_obj_t* — manual binding uses LvTimerWrapper

    # Functions with char* output buffer (const mismatch)
    "lv_roller_get_option_str",          # char* output buffer

    # chart/delete_dsc still leaking through - skip all chart series/cursor ops
    "lv_chart_set_series_values",        # lv_chart_series_t* incomplete
    "lv_chart_set_series_color2",        # lv_chart_series_t* incomplete
    "lv_chart_set_y_start_point",        # lv_chart_series_t* incomplete
    "lv_chart_get_y_array",              # lv_chart_series_t* incomplete
    "lv_chart_set_x_start_point",        # lv_chart_series_t* incomplete
    "lv_chart_get_x_array",              # lv_chart_series_t* incomplete
    "lv_chart_set_point_count",          # lv_chart_series_t* incomplete
    "lv_obj_delete_delayed",             # lv_delete_dsc_t* incomplete
    "lv_obj_add_delete_cb",              # returns lv_delete_dsc_t* incomplete
}

# ---------------------------------------------------------------------------
# Functions whose first parameter is NOT the "self" object
# even though it looks like a method of some type
# ---------------------------------------------------------------------------

FORCE_MODULE_LEVEL: Set[str] = {
    "lv_init", "lv_deinit", "lv_is_initialized",
    "lv_timer_handler",
    "lv_version_major", "lv_version_minor",
    "lv_version_patch", "lv_version_info",
    "lv_display_create", "lv_display_get_default",
    "lv_group_create", "lv_group_get_default",
    "lv_indev_create",
    "lv_color_make", "lv_color_hex", "lv_color_hex3",
    "lv_color_black", "lv_color_white",
    "lv_color_from_rgb", "lv_color_from_hsv",
    "lv_font_get_default",
}

# ---------------------------------------------------------------------------
# Widget name overrides
# ---------------------------------------------------------------------------

WIDGET_NAME_OVERRIDES: Dict[str, str] = {
    # Some functions don't follow the lv_WIDGET_xxx pattern
    "lv_screen_active": "screen",
    "lv_screen_load": "screen",
    "lv_screen_load_anim": "screen",
    "lv_obj_create": "obj",
}

# ---------------------------------------------------------------------------
# Type alias for parameter type resolution
# ---------------------------------------------------------------------------

TYPE_ALIASES: Dict[str, str] = {
    # LVGL typedefs that resolve to basic types
    # IMPORTANT: Only include types that are ACTUALLY typedef'd to basic types in C.
    # LVGL enum types like lv_align_t, lv_flex_align_t, etc. are C enums,
    # and C++ requires exact type matching (no implicit uint8_t → enum conversion).
    # These must be kept as their original type names for pybind11.
    "lv_coord_t": "int32_t",
    "lv_opa_t": "uint8_t",
    "lv_style_selector_t": "uint32_t",
    "lv_anim_enable_t": "bool",           # LV_RESULT_OK = true, LV_RESULT_INVALID = false
    "lv_result_t": "bool",
    "lv_style_prop_t": "uint32_t",        # Style property ID - this is a typedef to uint32_t
    # Standard C types
    "_Bool": "bool",
    # NOTE: The following are C enums, NOT basic types.
    # They must NOT be aliased to uint8_t because C++ requires exact enum type matching.
    # lv_align_t, lv_dir_t, lv_flex_align_t, lv_grid_align_t, etc.
    # are declared as: typedef enum { ... } lv_xxx_t;
    # In C++ code, they must be passed as their original enum type.
}

# ---------------------------------------------------------------------------
# C types that pybind11 can handle directly (pass by value or reference)
# ---------------------------------------------------------------------------

BASIC_CPP_TYPES: Set[str] = {
    "bool", "_Bool",
    "char", "unsigned char", "signed char",
    "short", "unsigned short",
    "int", "unsigned int",
    "long", "unsigned long",
    "long long", "unsigned long long",
    "int8_t", "uint8_t",
    "int16_t", "uint16_t",
    "int32_t", "uint32_t",
    "int64_t", "uint64_t",
    "size_t", "ssize_t",
    "float", "double",
    "lv_coord_t",
    "lv_style_selector_t",
    "lv_color_t",
}

# C types that are pointer-to-basic-type (safe for pybind11)
BASIC_PTR_TYPES: Set[str] = {
    "char *", "const char *",
    "uint8_t *", "const uint8_t *",
    "int8_t *", "const int8_t *",
    "int32_t *", "const int32_t *",
    "uint32_t *", "const uint32_t *",
    "void *", "const void *",
}

# Opaque struct types - pointer to these cannot be directly used in pybind11
# unless we create a wrapper class for them
OPAQUE_STRUCT_TYPES: Set[str] = {
    "lv_display_t",
    "lv_indev_t",
    "lv_group_t",
    "lv_timer_t",
    "lv_draw_ctx_t",
    "lv_layer_t",
    "lv_event_dsc_t",
    "lv_img_decoder_t",
    "lv_img_cache_entry_t",
    "lv_fragment_t",
    "lv_vector_context_t",
    "lv_obj_class_t",       # Opaque class descriptor - can't be used in Python
    "lv_subject_t",         # Subject/observer internal type
    "lv_ll_t",              # Linked list internal
    "lv_event_t",           # Event struct (has callback pointers)
    "lv_delete_dsc_t",      # Incomplete - only forward-declared in public headers
    "lv_theme_t",           # Incomplete - only forward-declared in public headers
    "lv_chart_series_t",    # Incomplete - only forward-declared in public headers
    "lv_chart_cursor_t",    # Incomplete - only forward-declared in public headers
    "lv_scale_section_t",   # Incomplete - only forward-declared in public headers
    "lv_style_t",           # Incomplete - only forward-declared in public headers
    "lv_style_transition_dsc_t",  # Incomplete - only forward-declared in public headers
    "lv_style_value_t",     # Incomplete - only forward-declared in public headers
    "lv_grad_dsc_t",        # Incomplete - only forward-declared in public headers
    "lv_image_colorkey_t",  # Incomplete - only forward-declared in public headers
    "lv_draw_arc_dsc_t",    # Incomplete - only forward-declared in public headers
    "lv_draw_blur_dsc_t",   # Incomplete - only forward-declared in public headers
    "lv_draw_label_dsc_t",  # Incomplete - only forward-declared in public headers
    "lv_draw_line_dsc_t",   # Incomplete - only forward-declared in public headers
    "lv_draw_rect_dsc_t",   # Incomplete - only forward-declared in public headers
    "lv_color_filter_dsc_t",  # Has callback ptr - can't be used from Python without registration
    "lv_anim_timeline_t",   # Incomplete - only forward-declared in public headers
    "lv_image_decoder_t",   # Incomplete - only forward-declared in public headers
    "lv_image_decoder_dsc_t",  # Incomplete - only forward-declared in public headers
    "lv_subject_increment_dsc_t",  # Incomplete - only forward-declared in public headers
    "lv_draw_image_dsc_t",  # Has callback ptrs - can't be used from Python without registration
}

# Struct types that have wrapper classes in lvgl_pybind_helpers.h
WRAPPED_STRUCT_TYPES: Dict[str, str] = {
    "lv_obj_t": "LvObjWrapper",
    "lv_display_t": "LvDisplayWrapper",
    "lv_indev_t": "LvIndevWrapper",
    "lv_timer_t": "LvTimerWrapper",
    # These will need new wrappers:
    # "lv_group_t": "LvGroupWrapper",
    # "lv_style_t": "LvStyleWrapper",
}
