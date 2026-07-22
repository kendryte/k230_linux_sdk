/*
 * Auto-Generated file by gen_pybind11.py (IR-based), DO NOT EDIT!
 *
 * LVGL Python bindings generated via libclang → IR → pybind11 pipeline.
 *
 * Module: lvgl
 * Total functions: 2054
 * Bound functions: 1302
 * Skipped functions: 752
 * Enums: 101
 * Widgets: 38
 */

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>
#include <pybind11/numpy.h>
#include "lvgl/lvgl.h"
#include "lvgl/driver_backends.h"
#include "lvgl/demos/benchmark/lv_demo_benchmark.h"
#include "lvgl/font/lv_freetype.h"
#include "lvgl_pybind_helpers.h"

namespace py = pybind11;

PYBIND11_MODULE(_lvgl, m) {
    m.doc() = "LVGL Python bindings (auto-generated via IR)";

    /*
     * Enum definitions
     */

    py::enum_<lv_result_t>(m, "RESULT")
        .value("INVALID", LV_RESULT_INVALID)
        .value("OK", LV_RESULT_OK)
;

    py::enum_<lv_align_t>(m, "ALIGN")
        .value("DEFAULT", LV_ALIGN_DEFAULT)
        .value("TOP_LEFT", LV_ALIGN_TOP_LEFT)
        .value("TOP_MID", LV_ALIGN_TOP_MID)
        .value("TOP_RIGHT", LV_ALIGN_TOP_RIGHT)
        .value("BOTTOM_LEFT", LV_ALIGN_BOTTOM_LEFT)
        .value("BOTTOM_MID", LV_ALIGN_BOTTOM_MID)
        .value("BOTTOM_RIGHT", LV_ALIGN_BOTTOM_RIGHT)
        .value("LEFT_MID", LV_ALIGN_LEFT_MID)
        .value("RIGHT_MID", LV_ALIGN_RIGHT_MID)
        .value("CENTER", LV_ALIGN_CENTER)
        .value("OUT_TOP_LEFT", LV_ALIGN_OUT_TOP_LEFT)
        .value("OUT_TOP_MID", LV_ALIGN_OUT_TOP_MID)
        .value("OUT_TOP_RIGHT", LV_ALIGN_OUT_TOP_RIGHT)
        .value("OUT_BOTTOM_LEFT", LV_ALIGN_OUT_BOTTOM_LEFT)
        .value("OUT_BOTTOM_MID", LV_ALIGN_OUT_BOTTOM_MID)
        .value("OUT_BOTTOM_RIGHT", LV_ALIGN_OUT_BOTTOM_RIGHT)
        .value("OUT_LEFT_TOP", LV_ALIGN_OUT_LEFT_TOP)
        .value("OUT_LEFT_MID", LV_ALIGN_OUT_LEFT_MID)
        .value("OUT_LEFT_BOTTOM", LV_ALIGN_OUT_LEFT_BOTTOM)
        .value("OUT_RIGHT_TOP", LV_ALIGN_OUT_RIGHT_TOP)
        .value("OUT_RIGHT_MID", LV_ALIGN_OUT_RIGHT_MID)
        .value("OUT_RIGHT_BOTTOM", LV_ALIGN_OUT_RIGHT_BOTTOM)
;

    py::enum_<lv_dir_t>(m, "DIR")
        .value("NONE", LV_DIR_NONE)
        .value("LEFT", LV_DIR_LEFT)
        .value("RIGHT", LV_DIR_RIGHT)
        .value("TOP", LV_DIR_TOP)
        .value("BOTTOM", LV_DIR_BOTTOM)
        .value("HOR", LV_DIR_HOR)
        .value("VER", LV_DIR_VER)
        .value("ALL", LV_DIR_ALL)
;

    py::enum_<lv_event_code_t>(m, "EVENT")
        .value("ALL", LV_EVENT_ALL)
        .value("PRESSED", LV_EVENT_PRESSED)
        .value("PRESSING", LV_EVENT_PRESSING)
        .value("PRESS_LOST", LV_EVENT_PRESS_LOST)
        .value("SHORT_CLICKED", LV_EVENT_SHORT_CLICKED)
        .value("SINGLE_CLICKED", LV_EVENT_SINGLE_CLICKED)
        .value("DOUBLE_CLICKED", LV_EVENT_DOUBLE_CLICKED)
        .value("TRIPLE_CLICKED", LV_EVENT_TRIPLE_CLICKED)
        .value("LONG_PRESSED", LV_EVENT_LONG_PRESSED)
        .value("LONG_PRESSED_REPEAT", LV_EVENT_LONG_PRESSED_REPEAT)
        .value("CLICKED", LV_EVENT_CLICKED)
        .value("RELEASED", LV_EVENT_RELEASED)
        .value("SCROLL_BEGIN", LV_EVENT_SCROLL_BEGIN)
        .value("SCROLL_THROW_BEGIN", LV_EVENT_SCROLL_THROW_BEGIN)
        .value("SCROLL_END", LV_EVENT_SCROLL_END)
        .value("SCROLL", LV_EVENT_SCROLL)
        .value("GESTURE", LV_EVENT_GESTURE)
        .value("KEY", LV_EVENT_KEY)
        .value("ROTARY", LV_EVENT_ROTARY)
        .value("FOCUSED", LV_EVENT_FOCUSED)
        .value("DEFOCUSED", LV_EVENT_DEFOCUSED)
        .value("LEAVE", LV_EVENT_LEAVE)
        .value("HIT_TEST", LV_EVENT_HIT_TEST)
        .value("INDEV_RESET", LV_EVENT_INDEV_RESET)
        .value("HOVER_OVER", LV_EVENT_HOVER_OVER)
        .value("HOVER_LEAVE", LV_EVENT_HOVER_LEAVE)
        .value("COVER_CHECK", LV_EVENT_COVER_CHECK)
        .value("REFR_EXT_DRAW_SIZE", LV_EVENT_REFR_EXT_DRAW_SIZE)
        .value("DRAW_MAIN_BEGIN", LV_EVENT_DRAW_MAIN_BEGIN)
        .value("DRAW_MAIN", LV_EVENT_DRAW_MAIN)
        .value("DRAW_MAIN_END", LV_EVENT_DRAW_MAIN_END)
        .value("DRAW_POST_BEGIN", LV_EVENT_DRAW_POST_BEGIN)
        .value("DRAW_POST", LV_EVENT_DRAW_POST)
        .value("DRAW_POST_END", LV_EVENT_DRAW_POST_END)
        .value("DRAW_TASK_ADDED", LV_EVENT_DRAW_TASK_ADDED)
        .value("VALUE_CHANGED", LV_EVENT_VALUE_CHANGED)
        .value("INSERT", LV_EVENT_INSERT)
        .value("REFRESH", LV_EVENT_REFRESH)
        .value("READY", LV_EVENT_READY)
        .value("CANCEL", LV_EVENT_CANCEL)
        .value("STATE_CHANGED", LV_EVENT_STATE_CHANGED)
        .value("CREATE", LV_EVENT_CREATE)
        .value("DELETE", LV_EVENT_DELETE)
        .value("CHILD_CHANGED", LV_EVENT_CHILD_CHANGED)
        .value("CHILD_CREATED", LV_EVENT_CHILD_CREATED)
        .value("CHILD_DELETED", LV_EVENT_CHILD_DELETED)
        .value("SCREEN_UNLOAD_START", LV_EVENT_SCREEN_UNLOAD_START)
        .value("SCREEN_LOAD_START", LV_EVENT_SCREEN_LOAD_START)
        .value("SCREEN_LOADED", LV_EVENT_SCREEN_LOADED)
        .value("SCREEN_UNLOADED", LV_EVENT_SCREEN_UNLOADED)
        .value("SIZE_CHANGED", LV_EVENT_SIZE_CHANGED)
        .value("STYLE_CHANGED", LV_EVENT_STYLE_CHANGED)
        .value("LAYOUT_CHANGED", LV_EVENT_LAYOUT_CHANGED)
        .value("GET_SELF_SIZE", LV_EVENT_GET_SELF_SIZE)
        .value("INVALIDATE_AREA", LV_EVENT_INVALIDATE_AREA)
        .value("RESOLUTION_CHANGED", LV_EVENT_RESOLUTION_CHANGED)
        .value("COLOR_FORMAT_CHANGED", LV_EVENT_COLOR_FORMAT_CHANGED)
        .value("REFR_REQUEST", LV_EVENT_REFR_REQUEST)
        .value("REFR_START", LV_EVENT_REFR_START)
        .value("REFR_READY", LV_EVENT_REFR_READY)
        .value("RENDER_START", LV_EVENT_RENDER_START)
        .value("RENDER_READY", LV_EVENT_RENDER_READY)
        .value("FLUSH_START", LV_EVENT_FLUSH_START)
        .value("FLUSH_FINISH", LV_EVENT_FLUSH_FINISH)
        .value("FLUSH_WAIT_START", LV_EVENT_FLUSH_WAIT_START)
        .value("FLUSH_WAIT_FINISH", LV_EVENT_FLUSH_WAIT_FINISH)
        .value("SYNC_START", LV_EVENT_SYNC_START)
        .value("SYNC_FINISH", LV_EVENT_SYNC_FINISH)
        .value("SYNC_WAIT_START", LV_EVENT_SYNC_WAIT_START)
        .value("SYNC_WAIT_FINISH", LV_EVENT_SYNC_WAIT_FINISH)
        .value("UPDATE_LAYOUT_COMPLETED", LV_EVENT_UPDATE_LAYOUT_COMPLETED)
        .value("VSYNC", LV_EVENT_VSYNC)
        .value("VSYNC_REQUEST", LV_EVENT_VSYNC_REQUEST)
        .value("LAST", LV_EVENT_LAST)
        .value("PREPROCESS", LV_EVENT_PREPROCESS)
        .value("MARKED_DELETING", LV_EVENT_MARKED_DELETING)
;

    py::enum_<lv_key_t>(m, "KEY")
        .value("UP", LV_KEY_UP)
        .value("DOWN", LV_KEY_DOWN)
        .value("RIGHT", LV_KEY_RIGHT)
        .value("LEFT", LV_KEY_LEFT)
        .value("ESC", LV_KEY_ESC)
        .value("DEL", LV_KEY_DEL)
        .value("BACKSPACE", LV_KEY_BACKSPACE)
        .value("ENTER", LV_KEY_ENTER)
        .value("NEXT", LV_KEY_NEXT)
        .value("PREV", LV_KEY_PREV)
        .value("HOME", LV_KEY_HOME)
        .value("END", LV_KEY_END)
;

    py::enum_<lv_group_refocus_policy_t>(m, "GROUP")
        .value("NEXT", LV_GROUP_REFOCUS_POLICY_NEXT)
        .value("PREV", LV_GROUP_REFOCUS_POLICY_PREV)
;

    /* Enum _lv_str_symbol_id_t - exported as UTF-8 symbol strings for use with icon APIs */
    m.attr("SYMBOL_BULLET") = LV_SYMBOL_BULLET;
    m.attr("SYMBOL_AUDIO") = LV_SYMBOL_AUDIO;
    m.attr("SYMBOL_VIDEO") = LV_SYMBOL_VIDEO;
    m.attr("SYMBOL_LIST") = LV_SYMBOL_LIST;
    m.attr("SYMBOL_OK") = LV_SYMBOL_OK;
    m.attr("SYMBOL_CLOSE") = LV_SYMBOL_CLOSE;
    m.attr("SYMBOL_POWER") = LV_SYMBOL_POWER;
    m.attr("SYMBOL_SETTINGS") = LV_SYMBOL_SETTINGS;
    m.attr("SYMBOL_HOME") = LV_SYMBOL_HOME;
    m.attr("SYMBOL_DOWNLOAD") = LV_SYMBOL_DOWNLOAD;
    m.attr("SYMBOL_DRIVE") = LV_SYMBOL_DRIVE;
    m.attr("SYMBOL_REFRESH") = LV_SYMBOL_REFRESH;
    m.attr("SYMBOL_MUTE") = LV_SYMBOL_MUTE;
    m.attr("SYMBOL_VOLUME_MID") = LV_SYMBOL_VOLUME_MID;
    m.attr("SYMBOL_VOLUME_MAX") = LV_SYMBOL_VOLUME_MAX;
    m.attr("SYMBOL_IMAGE") = LV_SYMBOL_IMAGE;
    m.attr("SYMBOL_TINT") = LV_SYMBOL_TINT;
    m.attr("SYMBOL_PREV") = LV_SYMBOL_PREV;
    m.attr("SYMBOL_PLAY") = LV_SYMBOL_PLAY;
    m.attr("SYMBOL_PAUSE") = LV_SYMBOL_PAUSE;
    m.attr("SYMBOL_STOP") = LV_SYMBOL_STOP;
    m.attr("SYMBOL_NEXT") = LV_SYMBOL_NEXT;
    m.attr("SYMBOL_EJECT") = LV_SYMBOL_EJECT;
    m.attr("SYMBOL_LEFT") = LV_SYMBOL_LEFT;
    m.attr("SYMBOL_RIGHT") = LV_SYMBOL_RIGHT;
    m.attr("SYMBOL_PLUS") = LV_SYMBOL_PLUS;
    m.attr("SYMBOL_MINUS") = LV_SYMBOL_MINUS;
    m.attr("SYMBOL_EYE_OPEN") = LV_SYMBOL_EYE_OPEN;
    m.attr("SYMBOL_EYE_CLOSE") = LV_SYMBOL_EYE_CLOSE;
    m.attr("SYMBOL_WARNING") = LV_SYMBOL_WARNING;
    m.attr("SYMBOL_SHUFFLE") = LV_SYMBOL_SHUFFLE;
    m.attr("SYMBOL_UP") = LV_SYMBOL_UP;
    m.attr("SYMBOL_DOWN") = LV_SYMBOL_DOWN;
    m.attr("SYMBOL_LOOP") = LV_SYMBOL_LOOP;
    m.attr("SYMBOL_DIRECTORY") = LV_SYMBOL_DIRECTORY;
    m.attr("SYMBOL_UPLOAD") = LV_SYMBOL_UPLOAD;
    m.attr("SYMBOL_CALL") = LV_SYMBOL_CALL;
    m.attr("SYMBOL_CUT") = LV_SYMBOL_CUT;
    m.attr("SYMBOL_COPY") = LV_SYMBOL_COPY;
    m.attr("SYMBOL_SAVE") = LV_SYMBOL_SAVE;
    m.attr("SYMBOL_BARS") = LV_SYMBOL_BARS;
    m.attr("SYMBOL_ENVELOPE") = LV_SYMBOL_ENVELOPE;
    m.attr("SYMBOL_CHARGE") = LV_SYMBOL_CHARGE;
    m.attr("SYMBOL_PASTE") = LV_SYMBOL_PASTE;
    m.attr("SYMBOL_BELL") = LV_SYMBOL_BELL;
    m.attr("SYMBOL_KEYBOARD") = LV_SYMBOL_KEYBOARD;
    m.attr("SYMBOL_GPS") = LV_SYMBOL_GPS;
    m.attr("SYMBOL_FILE") = LV_SYMBOL_FILE;
    m.attr("SYMBOL_WIFI") = LV_SYMBOL_WIFI;
    m.attr("SYMBOL_BATTERY_FULL") = LV_SYMBOL_BATTERY_FULL;
    m.attr("SYMBOL_BATTERY_3") = LV_SYMBOL_BATTERY_3;
    m.attr("SYMBOL_BATTERY_2") = LV_SYMBOL_BATTERY_2;
    m.attr("SYMBOL_BATTERY_1") = LV_SYMBOL_BATTERY_1;
    m.attr("SYMBOL_BATTERY_EMPTY") = LV_SYMBOL_BATTERY_EMPTY;
    m.attr("SYMBOL_USB") = LV_SYMBOL_USB;
    m.attr("SYMBOL_BLUETOOTH") = LV_SYMBOL_BLUETOOTH;
    m.attr("SYMBOL_TRASH") = LV_SYMBOL_TRASH;
    m.attr("SYMBOL_EDIT") = LV_SYMBOL_EDIT;
    m.attr("SYMBOL_BACKSPACE") = LV_SYMBOL_BACKSPACE;
    m.attr("SYMBOL_SD_CARD") = LV_SYMBOL_SD_CARD;
    m.attr("SYMBOL_NEW_LINE") = LV_SYMBOL_NEW_LINE;
    m.attr("SYMBOL_DUMMY") = LV_SYMBOL_DUMMY;

    /* Enum _lv_opacity_level_t (no C type found, emitting as constants) */
    m.attr("OPA_TRANSP") = (int)LV_OPA_TRANSP;
    m.attr("OPA_0") = (int)LV_OPA_0;
    m.attr("OPA_10") = (int)LV_OPA_10;
    m.attr("OPA_20") = (int)LV_OPA_20;
    m.attr("OPA_30") = (int)LV_OPA_30;
    m.attr("OPA_40") = (int)LV_OPA_40;
    m.attr("OPA_50") = (int)LV_OPA_50;
    m.attr("OPA_60") = (int)LV_OPA_60;
    m.attr("OPA_70") = (int)LV_OPA_70;
    m.attr("OPA_80") = (int)LV_OPA_80;
    m.attr("OPA_90") = (int)LV_OPA_90;
    m.attr("OPA_100") = (int)LV_OPA_100;
    m.attr("OPA_COVER") = (int)LV_OPA_COVER;

    py::enum_<lv_color_format_t>(m, "COLOR_FORMAT")
        .value("UNKNOWN", LV_COLOR_FORMAT_UNKNOWN)
        .value("RAW", LV_COLOR_FORMAT_RAW)
        .value("RAW_ALPHA", LV_COLOR_FORMAT_RAW_ALPHA)
        .value("L8", LV_COLOR_FORMAT_L8)
        .value("I1", LV_COLOR_FORMAT_I1)
        .value("I2", LV_COLOR_FORMAT_I2)
        .value("I4", LV_COLOR_FORMAT_I4)
        .value("I8", LV_COLOR_FORMAT_I8)
        .value("A8", LV_COLOR_FORMAT_A8)
        .value("RGB565", LV_COLOR_FORMAT_RGB565)
        .value("ARGB8565", LV_COLOR_FORMAT_ARGB8565)
        .value("RGB565A8", LV_COLOR_FORMAT_RGB565A8)
        .value("AL88", LV_COLOR_FORMAT_AL88)
        .value("RGB565_SWAPPED", LV_COLOR_FORMAT_RGB565_SWAPPED)
        .value("RGB888", LV_COLOR_FORMAT_RGB888)
        .value("ARGB8888", LV_COLOR_FORMAT_ARGB8888)
        .value("XRGB8888", LV_COLOR_FORMAT_XRGB8888)
        .value("ARGB8888_PREMULTIPLIED", LV_COLOR_FORMAT_ARGB8888_PREMULTIPLIED)
        .value("A1", LV_COLOR_FORMAT_A1)
        .value("A2", LV_COLOR_FORMAT_A2)
        .value("A4", LV_COLOR_FORMAT_A4)
        .value("ARGB1555", LV_COLOR_FORMAT_ARGB1555)
        .value("ARGB4444", LV_COLOR_FORMAT_ARGB4444)
        .value("ARGB2222", LV_COLOR_FORMAT_ARGB2222)
        .value("YUV_START", LV_COLOR_FORMAT_YUV_START)
        .value("I420", LV_COLOR_FORMAT_I420)
        .value("I422", LV_COLOR_FORMAT_I422)
        .value("I444", LV_COLOR_FORMAT_I444)
        .value("I400", LV_COLOR_FORMAT_I400)
        .value("NV21", LV_COLOR_FORMAT_NV21)
        .value("NV12", LV_COLOR_FORMAT_NV12)
        .value("YUY2", LV_COLOR_FORMAT_YUY2)
        .value("UYVY", LV_COLOR_FORMAT_UYVY)
        .value("YUV_END", LV_COLOR_FORMAT_YUV_END)
        .value("PROPRIETARY_START", LV_COLOR_FORMAT_PROPRIETARY_START)
        .value("NEMA_TSC_START", LV_COLOR_FORMAT_NEMA_TSC_START)
        .value("NEMA_TSC4", LV_COLOR_FORMAT_NEMA_TSC4)
        .value("NEMA_TSC6", LV_COLOR_FORMAT_NEMA_TSC6)
        .value("NEMA_TSC6A", LV_COLOR_FORMAT_NEMA_TSC6A)
        .value("NEMA_TSC6AP", LV_COLOR_FORMAT_NEMA_TSC6AP)
        .value("NEMA_TSC12", LV_COLOR_FORMAT_NEMA_TSC12)
        .value("NEMA_TSC12A", LV_COLOR_FORMAT_NEMA_TSC12A)
        .value("NEMA_TSC_END", LV_COLOR_FORMAT_NEMA_TSC_END)
        .value("NATIVE", LV_COLOR_FORMAT_NATIVE)
        .value("NATIVE_WITH_ALPHA", LV_COLOR_FORMAT_NATIVE_WITH_ALPHA)
;

    py::enum_<lv_palette_t>(m, "PALETTE")
        .value("RED", LV_PALETTE_RED)
        .value("PINK", LV_PALETTE_PINK)
        .value("PURPLE", LV_PALETTE_PURPLE)
        .value("DEEP_PURPLE", LV_PALETTE_DEEP_PURPLE)
        .value("INDIGO", LV_PALETTE_INDIGO)
        .value("BLUE", LV_PALETTE_BLUE)
        .value("LIGHT_BLUE", LV_PALETTE_LIGHT_BLUE)
        .value("CYAN", LV_PALETTE_CYAN)
        .value("TEAL", LV_PALETTE_TEAL)
        .value("GREEN", LV_PALETTE_GREEN)
        .value("LIGHT_GREEN", LV_PALETTE_LIGHT_GREEN)
        .value("LIME", LV_PALETTE_LIME)
        .value("YELLOW", LV_PALETTE_YELLOW)
        .value("AMBER", LV_PALETTE_AMBER)
        .value("ORANGE", LV_PALETTE_ORANGE)
        .value("DEEP_ORANGE", LV_PALETTE_DEEP_ORANGE)
        .value("BROWN", LV_PALETTE_BROWN)
        .value("BLUE_GREY", LV_PALETTE_BLUE_GREY)
        .value("GREY", LV_PALETTE_GREY)
        .value("LAST", LV_PALETTE_LAST)
        .value("NONE", LV_PALETTE_NONE)
;

    /* Enum _lvimage_flags_t (no C type found, emitting as constants) */
    m.attr("IMAGE_FLAGS_PREMULTIPLIED") = (int)LV_IMAGE_FLAGS_PREMULTIPLIED;
    m.attr("IMAGE_FLAGS_COMPRESSED") = (int)LV_IMAGE_FLAGS_COMPRESSED;
    m.attr("IMAGE_FLAGS_ALLOCATED") = (int)LV_IMAGE_FLAGS_ALLOCATED;
    m.attr("IMAGE_FLAGS_MODIFIABLE") = (int)LV_IMAGE_FLAGS_MODIFIABLE;
    m.attr("IMAGE_FLAGS_CUSTOM_DRAW") = (int)LV_IMAGE_FLAGS_CUSTOM_DRAW;
    m.attr("IMAGE_FLAGS_USER1") = (int)LV_IMAGE_FLAGS_USER1;
    m.attr("IMAGE_FLAGS_USER2") = (int)LV_IMAGE_FLAGS_USER2;
    m.attr("IMAGE_FLAGS_USER3") = (int)LV_IMAGE_FLAGS_USER3;
    m.attr("IMAGE_FLAGS_USER4") = (int)LV_IMAGE_FLAGS_USER4;
    m.attr("IMAGE_FLAGS_USER5") = (int)LV_IMAGE_FLAGS_USER5;
    m.attr("IMAGE_FLAGS_USER6") = (int)LV_IMAGE_FLAGS_USER6;
    m.attr("IMAGE_FLAGS_USER7") = (int)LV_IMAGE_FLAGS_USER7;
    m.attr("IMAGE_FLAGS_USER8") = (int)LV_IMAGE_FLAGS_USER8;

    py::enum_<lv_image_compress_t>(m, "IMAGE_COMPRESS")
        .value("NONE", LV_IMAGE_COMPRESS_NONE)
        .value("RLE", LV_IMAGE_COMPRESS_RLE)
        .value("LZ4", LV_IMAGE_COMPRESS_LZ4)
;

    py::enum_<lv_font_glyph_format_t>(m, "FONT_GLYPH_FORMAT")
        .value("NONE", LV_FONT_GLYPH_FORMAT_NONE)
        .value("A1", LV_FONT_GLYPH_FORMAT_A1)
        .value("A2", LV_FONT_GLYPH_FORMAT_A2)
        .value("A3", LV_FONT_GLYPH_FORMAT_A3)
        .value("A4", LV_FONT_GLYPH_FORMAT_A4)
        .value("A8", LV_FONT_GLYPH_FORMAT_A8)
        .value("IMAGE", LV_FONT_GLYPH_FORMAT_IMAGE)
        .value("VECTOR", LV_FONT_GLYPH_FORMAT_VECTOR)
        .value("SVG", LV_FONT_GLYPH_FORMAT_SVG)
        .value("CUSTOM", LV_FONT_GLYPH_FORMAT_CUSTOM)
;

    py::enum_<lv_font_subpx_t>(m, "FONT_SUBPX")
        .value("NONE", LV_FONT_SUBPX_NONE)
        .value("HOR", LV_FONT_SUBPX_HOR)
        .value("VER", LV_FONT_SUBPX_VER)
        .value("BOTH", LV_FONT_SUBPX_BOTH)
;

    py::enum_<lv_font_kerning_t>(m, "FONT_KERNING")
        .value("NORMAL", LV_FONT_KERNING_NORMAL)
        .value("NONE", LV_FONT_KERNING_NONE)
;

    py::enum_<lv_text_flag_t>(m, "TEXT_FLAG")
        .value("NONE", LV_TEXT_FLAG_NONE)
        .value("EXPAND", LV_TEXT_FLAG_EXPAND)
        .value("FIT", LV_TEXT_FLAG_FIT)
        .value("BREAK_ALL", LV_TEXT_FLAG_BREAK_ALL)
        .value("RECOLOR", LV_TEXT_FLAG_RECOLOR)
;

    py::enum_<lv_text_align_t>(m, "TEXT_ALIGN")
        .value("AUTO", LV_TEXT_ALIGN_AUTO)
        .value("LEFT", LV_TEXT_ALIGN_LEFT)
        .value("CENTER", LV_TEXT_ALIGN_CENTER)
        .value("RIGHT", LV_TEXT_ALIGN_RIGHT)
;

    py::enum_<lv_text_leading_trim_t>(m, "TEXT_LEADING_TRIM")
        .value("NONE", LV_TEXT_LEADING_TRIM_NONE)
        .value("CAPITAL_BASELINE", LV_TEXT_LEADING_TRIM_CAPITAL_BASELINE)
        .value("LOWER_BASELINE", LV_TEXT_LEADING_TRIM_LOWER_BASELINE)
        .value("CAPITAL", LV_TEXT_LEADING_TRIM_CAPITAL)
        .value("LOWER", LV_TEXT_LEADING_TRIM_LOWER)
;

    py::enum_<lv_base_dir_t>(m, "BASE_DIR")
        .value("LTR", LV_BASE_DIR_LTR)
        .value("RTL", LV_BASE_DIR_RTL)
        .value("AUTO", LV_BASE_DIR_AUTO)
        .value("NEUTRAL", LV_BASE_DIR_NEUTRAL)
        .value("WEAK", LV_BASE_DIR_WEAK)
;

    py::enum_<lv_grad_dir_t>(m, "GRAD_DIR")
        .value("NONE", LV_GRAD_DIR_NONE)
        .value("VER", LV_GRAD_DIR_VER)
        .value("HOR", LV_GRAD_DIR_HOR)
        .value("LINEAR", LV_GRAD_DIR_LINEAR)
        .value("RADIAL", LV_GRAD_DIR_RADIAL)
        .value("CONICAL", LV_GRAD_DIR_CONICAL)
;

    py::enum_<lv_grad_extend_t>(m, "GRAD_EXTEND")
        .value("PAD", LV_GRAD_EXTEND_PAD)
        .value("REPEAT", LV_GRAD_EXTEND_REPEAT)
        .value("REFLECT", LV_GRAD_EXTEND_REFLECT)
;

    py::enum_<lv_layout_t>(m, "LAYOUT")
        .value("NONE", LV_LAYOUT_NONE)
        .value("FLEX", LV_LAYOUT_FLEX)
        .value("GRID", LV_LAYOUT_GRID)
        .value("LAST", LV_LAYOUT_LAST)
;

    py::enum_<lv_flex_align_t>(m, "FLEX_ALIGN")
        .value("START", LV_FLEX_ALIGN_START)
        .value("END", LV_FLEX_ALIGN_END)
        .value("CENTER", LV_FLEX_ALIGN_CENTER)
        .value("SPACE_EVENLY", LV_FLEX_ALIGN_SPACE_EVENLY)
        .value("SPACE_AROUND", LV_FLEX_ALIGN_SPACE_AROUND)
        .value("SPACE_BETWEEN", LV_FLEX_ALIGN_SPACE_BETWEEN)
;

    py::enum_<lv_flex_flow_t>(m, "FLEX_FLOW")
        .value("ROW", LV_FLEX_FLOW_ROW)
        .value("COLUMN", LV_FLEX_FLOW_COLUMN)
        .value("ROW_WRAP", LV_FLEX_FLOW_ROW_WRAP)
        .value("ROW_REVERSE", LV_FLEX_FLOW_ROW_REVERSE)
        .value("ROW_WRAP_REVERSE", LV_FLEX_FLOW_ROW_WRAP_REVERSE)
        .value("COLUMN_WRAP", LV_FLEX_FLOW_COLUMN_WRAP)
        .value("COLUMN_REVERSE", LV_FLEX_FLOW_COLUMN_REVERSE)
        .value("COLUMN_WRAP_REVERSE", LV_FLEX_FLOW_COLUMN_WRAP_REVERSE)
;

    py::enum_<lv_grid_align_t>(m, "GRID_ALIGN")
        .value("START", LV_GRID_ALIGN_START)
        .value("CENTER", LV_GRID_ALIGN_CENTER)
        .value("END", LV_GRID_ALIGN_END)
        .value("STRETCH", LV_GRID_ALIGN_STRETCH)
        .value("SPACE_EVENLY", LV_GRID_ALIGN_SPACE_EVENLY)
        .value("SPACE_AROUND", LV_GRID_ALIGN_SPACE_AROUND)
        .value("SPACE_BETWEEN", LV_GRID_ALIGN_SPACE_BETWEEN)
;

    py::enum_<lv_blend_mode_t>(m, "BLEND")
        .value("NORMAL", LV_BLEND_MODE_NORMAL)
        .value("ADDITIVE", LV_BLEND_MODE_ADDITIVE)
        .value("SUBTRACTIVE", LV_BLEND_MODE_SUBTRACTIVE)
        .value("MULTIPLY", LV_BLEND_MODE_MULTIPLY)
        .value("DIFFERENCE", LV_BLEND_MODE_DIFFERENCE)
;

    py::enum_<lv_text_decor_t>(m, "TEXT_DECOR")
        .value("NONE", LV_TEXT_DECOR_NONE)
        .value("UNDERLINE", LV_TEXT_DECOR_UNDERLINE)
        .value("STRIKETHROUGH", LV_TEXT_DECOR_STRIKETHROUGH)
;

    py::enum_<lv_border_side_t>(m, "BORDER_SIDE")
        .value("NONE", LV_BORDER_SIDE_NONE)
        .value("BOTTOM", LV_BORDER_SIDE_BOTTOM)
        .value("TOP", LV_BORDER_SIDE_TOP)
        .value("LEFT", LV_BORDER_SIDE_LEFT)
        .value("RIGHT", LV_BORDER_SIDE_RIGHT)
        .value("FULL", LV_BORDER_SIDE_FULL)
        .value("INTERNAL", LV_BORDER_SIDE_INTERNAL)
;

    py::enum_<lv_blur_quality_t>(m, "BLUR_QUALITY")
        .value("AUTO", LV_BLUR_QUALITY_AUTO)
        .value("SPEED", LV_BLUR_QUALITY_SPEED)
        .value("PRECISION", LV_BLUR_QUALITY_PRECISION)
;

    /* Enum _lv_style_id_t (no C type found, emitting as constants) */
    m.attr("STYLE_PROP_INV") = (int)LV_STYLE_PROP_INV;
    m.attr("STYLE_WIDTH") = (int)LV_STYLE_WIDTH;
    m.attr("STYLE_HEIGHT") = (int)LV_STYLE_HEIGHT;
    m.attr("STYLE_LENGTH") = (int)LV_STYLE_LENGTH;
    m.attr("STYLE_TRANSFORM_WIDTH") = (int)LV_STYLE_TRANSFORM_WIDTH;
    m.attr("STYLE_TRANSFORM_HEIGHT") = (int)LV_STYLE_TRANSFORM_HEIGHT;
    m.attr("STYLE_MIN_WIDTH") = (int)LV_STYLE_MIN_WIDTH;
    m.attr("STYLE_MAX_WIDTH") = (int)LV_STYLE_MAX_WIDTH;
    m.attr("STYLE_MIN_HEIGHT") = (int)LV_STYLE_MIN_HEIGHT;
    m.attr("STYLE_MAX_HEIGHT") = (int)LV_STYLE_MAX_HEIGHT;
    m.attr("STYLE_TRANSLATE_X") = (int)LV_STYLE_TRANSLATE_X;
    m.attr("STYLE_TRANSLATE_Y") = (int)LV_STYLE_TRANSLATE_Y;
    m.attr("STYLE_RADIAL_OFFSET") = (int)LV_STYLE_RADIAL_OFFSET;
    m.attr("STYLE_X") = (int)LV_STYLE_X;
    m.attr("STYLE_Y") = (int)LV_STYLE_Y;
    m.attr("STYLE_ALIGN") = (int)LV_STYLE_ALIGN;
    m.attr("STYLE_PAD_TOP") = (int)LV_STYLE_PAD_TOP;
    m.attr("STYLE_PAD_BOTTOM") = (int)LV_STYLE_PAD_BOTTOM;
    m.attr("STYLE_PAD_LEFT") = (int)LV_STYLE_PAD_LEFT;
    m.attr("STYLE_PAD_RIGHT") = (int)LV_STYLE_PAD_RIGHT;
    m.attr("STYLE_PAD_RADIAL") = (int)LV_STYLE_PAD_RADIAL;
    m.attr("STYLE_PAD_ROW") = (int)LV_STYLE_PAD_ROW;
    m.attr("STYLE_PAD_COLUMN") = (int)LV_STYLE_PAD_COLUMN;
    m.attr("STYLE_MARGIN_TOP") = (int)LV_STYLE_MARGIN_TOP;
    m.attr("STYLE_MARGIN_BOTTOM") = (int)LV_STYLE_MARGIN_BOTTOM;
    m.attr("STYLE_MARGIN_LEFT") = (int)LV_STYLE_MARGIN_LEFT;
    m.attr("STYLE_MARGIN_RIGHT") = (int)LV_STYLE_MARGIN_RIGHT;
    m.attr("STYLE_BG_GRAD") = (int)LV_STYLE_BG_GRAD;
    m.attr("STYLE_BG_GRAD_DIR") = (int)LV_STYLE_BG_GRAD_DIR;
    m.attr("STYLE_BG_MAIN_OPA") = (int)LV_STYLE_BG_MAIN_OPA;
    m.attr("STYLE_BG_GRAD_OPA") = (int)LV_STYLE_BG_GRAD_OPA;
    m.attr("STYLE_BG_GRAD_COLOR") = (int)LV_STYLE_BG_GRAD_COLOR;
    m.attr("STYLE_BG_MAIN_STOP") = (int)LV_STYLE_BG_MAIN_STOP;
    m.attr("STYLE_BG_GRAD_STOP") = (int)LV_STYLE_BG_GRAD_STOP;
    m.attr("STYLE_BG_IMAGE_SRC") = (int)LV_STYLE_BG_IMAGE_SRC;
    m.attr("STYLE_BG_IMAGE_OPA") = (int)LV_STYLE_BG_IMAGE_OPA;
    m.attr("STYLE_BG_IMAGE_RECOLOR_OPA") = (int)LV_STYLE_BG_IMAGE_RECOLOR_OPA;
    m.attr("STYLE_BG_IMAGE_TILED") = (int)LV_STYLE_BG_IMAGE_TILED;
    m.attr("STYLE_BG_IMAGE_RECOLOR") = (int)LV_STYLE_BG_IMAGE_RECOLOR;
    m.attr("STYLE_BORDER_WIDTH") = (int)LV_STYLE_BORDER_WIDTH;
    m.attr("STYLE_BORDER_COLOR") = (int)LV_STYLE_BORDER_COLOR;
    m.attr("STYLE_BORDER_OPA") = (int)LV_STYLE_BORDER_OPA;
    m.attr("STYLE_BORDER_POST") = (int)LV_STYLE_BORDER_POST;
    m.attr("STYLE_BORDER_SIDE") = (int)LV_STYLE_BORDER_SIDE;
    m.attr("STYLE_OUTLINE_WIDTH") = (int)LV_STYLE_OUTLINE_WIDTH;
    m.attr("STYLE_OUTLINE_COLOR") = (int)LV_STYLE_OUTLINE_COLOR;
    m.attr("STYLE_OUTLINE_OPA") = (int)LV_STYLE_OUTLINE_OPA;
    m.attr("STYLE_OUTLINE_PAD") = (int)LV_STYLE_OUTLINE_PAD;
    m.attr("STYLE_BG_OPA") = (int)LV_STYLE_BG_OPA;
    m.attr("STYLE_BG_COLOR") = (int)LV_STYLE_BG_COLOR;
    m.attr("STYLE_SHADOW_WIDTH") = (int)LV_STYLE_SHADOW_WIDTH;
    m.attr("STYLE_LINE_WIDTH") = (int)LV_STYLE_LINE_WIDTH;
    m.attr("STYLE_ARC_WIDTH") = (int)LV_STYLE_ARC_WIDTH;
    m.attr("STYLE_TEXT_FONT") = (int)LV_STYLE_TEXT_FONT;
    m.attr("STYLE_IMAGE_RECOLOR_OPA") = (int)LV_STYLE_IMAGE_RECOLOR_OPA;
    m.attr("STYLE_IMAGE_OPA") = (int)LV_STYLE_IMAGE_OPA;
    m.attr("STYLE_SHADOW_OPA") = (int)LV_STYLE_SHADOW_OPA;
    m.attr("STYLE_LINE_OPA") = (int)LV_STYLE_LINE_OPA;
    m.attr("STYLE_ARC_OPA") = (int)LV_STYLE_ARC_OPA;
    m.attr("STYLE_TEXT_OPA") = (int)LV_STYLE_TEXT_OPA;
    m.attr("STYLE_SHADOW_COLOR") = (int)LV_STYLE_SHADOW_COLOR;
    m.attr("STYLE_IMAGE_RECOLOR") = (int)LV_STYLE_IMAGE_RECOLOR;
    m.attr("STYLE_LINE_COLOR") = (int)LV_STYLE_LINE_COLOR;
    m.attr("STYLE_ARC_COLOR") = (int)LV_STYLE_ARC_COLOR;
    m.attr("STYLE_TEXT_COLOR") = (int)LV_STYLE_TEXT_COLOR;
    m.attr("STYLE_ARC_IMAGE_SRC") = (int)LV_STYLE_ARC_IMAGE_SRC;
    m.attr("STYLE_SHADOW_OFFSET_X") = (int)LV_STYLE_SHADOW_OFFSET_X;
    m.attr("STYLE_SHADOW_OFFSET_Y") = (int)LV_STYLE_SHADOW_OFFSET_Y;
    m.attr("STYLE_SHADOW_SPREAD") = (int)LV_STYLE_SHADOW_SPREAD;
    m.attr("STYLE_LINE_DASH_WIDTH") = (int)LV_STYLE_LINE_DASH_WIDTH;
    m.attr("STYLE_TEXT_ALIGN") = (int)LV_STYLE_TEXT_ALIGN;
    m.attr("STYLE_TEXT_LETTER_SPACE") = (int)LV_STYLE_TEXT_LETTER_SPACE;
    m.attr("STYLE_TEXT_LINE_SPACE") = (int)LV_STYLE_TEXT_LINE_SPACE;
    m.attr("STYLE_LINE_DASH_GAP") = (int)LV_STYLE_LINE_DASH_GAP;
    m.attr("STYLE_LINE_ROUNDED") = (int)LV_STYLE_LINE_ROUNDED;
    m.attr("STYLE_IMAGE_COLORKEY") = (int)LV_STYLE_IMAGE_COLORKEY;
    m.attr("STYLE_TEXT_OUTLINE_STROKE_WIDTH") = (int)LV_STYLE_TEXT_OUTLINE_STROKE_WIDTH;
    m.attr("STYLE_TEXT_OUTLINE_STROKE_OPA") = (int)LV_STYLE_TEXT_OUTLINE_STROKE_OPA;
    m.attr("STYLE_TEXT_OUTLINE_STROKE_COLOR") = (int)LV_STYLE_TEXT_OUTLINE_STROKE_COLOR;
    m.attr("STYLE_TEXT_DECOR") = (int)LV_STYLE_TEXT_DECOR;
    m.attr("STYLE_ARC_ROUNDED") = (int)LV_STYLE_ARC_ROUNDED;
    m.attr("STYLE_OPA") = (int)LV_STYLE_OPA;
    m.attr("STYLE_OPA_LAYERED") = (int)LV_STYLE_OPA_LAYERED;
    m.attr("STYLE_COLOR_FILTER_DSC") = (int)LV_STYLE_COLOR_FILTER_DSC;
    m.attr("STYLE_COLOR_FILTER_OPA") = (int)LV_STYLE_COLOR_FILTER_OPA;
    m.attr("STYLE_ANIM") = (int)LV_STYLE_ANIM;
    m.attr("STYLE_ANIM_DURATION") = (int)LV_STYLE_ANIM_DURATION;
    m.attr("STYLE_TRANSITION") = (int)LV_STYLE_TRANSITION;
    m.attr("STYLE_RADIUS") = (int)LV_STYLE_RADIUS;
    m.attr("STYLE_BITMAP_MASK_SRC") = (int)LV_STYLE_BITMAP_MASK_SRC;
    m.attr("STYLE_BLEND_MODE") = (int)LV_STYLE_BLEND_MODE;
    m.attr("STYLE_ROTARY_SENSITIVITY") = (int)LV_STYLE_ROTARY_SENSITIVITY;
    m.attr("STYLE_TRANSLATE_RADIAL") = (int)LV_STYLE_TRANSLATE_RADIAL;
    m.attr("STYLE_CLIP_CORNER") = (int)LV_STYLE_CLIP_CORNER;
    m.attr("STYLE_BASE_DIR") = (int)LV_STYLE_BASE_DIR;
    m.attr("STYLE_RECOLOR") = (int)LV_STYLE_RECOLOR;
    m.attr("STYLE_RECOLOR_OPA") = (int)LV_STYLE_RECOLOR_OPA;
    m.attr("STYLE_LAYOUT") = (int)LV_STYLE_LAYOUT;
    m.attr("STYLE_BLUR_RADIUS") = (int)LV_STYLE_BLUR_RADIUS;
    m.attr("STYLE_BLUR_BACKDROP") = (int)LV_STYLE_BLUR_BACKDROP;
    m.attr("STYLE_BLUR_QUALITY") = (int)LV_STYLE_BLUR_QUALITY;
    m.attr("STYLE_DROP_SHADOW_RADIUS") = (int)LV_STYLE_DROP_SHADOW_RADIUS;
    m.attr("STYLE_DROP_SHADOW_OFFSET_X") = (int)LV_STYLE_DROP_SHADOW_OFFSET_X;
    m.attr("STYLE_DROP_SHADOW_OFFSET_Y") = (int)LV_STYLE_DROP_SHADOW_OFFSET_Y;
    m.attr("STYLE_DROP_SHADOW_COLOR") = (int)LV_STYLE_DROP_SHADOW_COLOR;
    m.attr("STYLE_DROP_SHADOW_OPA") = (int)LV_STYLE_DROP_SHADOW_OPA;
    m.attr("STYLE_DROP_SHADOW_QUALITY") = (int)LV_STYLE_DROP_SHADOW_QUALITY;
    m.attr("STYLE_TRANSFORM_SCALE_X") = (int)LV_STYLE_TRANSFORM_SCALE_X;
    m.attr("STYLE_TRANSFORM_SCALE_Y") = (int)LV_STYLE_TRANSFORM_SCALE_Y;
    m.attr("STYLE_TRANSFORM_PIVOT_X") = (int)LV_STYLE_TRANSFORM_PIVOT_X;
    m.attr("STYLE_TRANSFORM_PIVOT_Y") = (int)LV_STYLE_TRANSFORM_PIVOT_Y;
    m.attr("STYLE_TRANSFORM_ROTATION") = (int)LV_STYLE_TRANSFORM_ROTATION;
    m.attr("STYLE_TRANSFORM_SKEW_X") = (int)LV_STYLE_TRANSFORM_SKEW_X;
    m.attr("STYLE_TRANSFORM_SKEW_Y") = (int)LV_STYLE_TRANSFORM_SKEW_Y;
    m.attr("STYLE_FLEX_FLOW") = (int)LV_STYLE_FLEX_FLOW;
    m.attr("STYLE_FLEX_MAIN_PLACE") = (int)LV_STYLE_FLEX_MAIN_PLACE;
    m.attr("STYLE_FLEX_CROSS_PLACE") = (int)LV_STYLE_FLEX_CROSS_PLACE;
    m.attr("STYLE_FLEX_TRACK_PLACE") = (int)LV_STYLE_FLEX_TRACK_PLACE;
    m.attr("STYLE_FLEX_GROW") = (int)LV_STYLE_FLEX_GROW;
    m.attr("STYLE_GRID_COLUMN_DSC_ARRAY") = (int)LV_STYLE_GRID_COLUMN_DSC_ARRAY;
    m.attr("STYLE_GRID_ROW_DSC_ARRAY") = (int)LV_STYLE_GRID_ROW_DSC_ARRAY;
    m.attr("STYLE_GRID_COLUMN_ALIGN") = (int)LV_STYLE_GRID_COLUMN_ALIGN;
    m.attr("STYLE_GRID_ROW_ALIGN") = (int)LV_STYLE_GRID_ROW_ALIGN;
    m.attr("STYLE_GRID_CELL_COLUMN_POS") = (int)LV_STYLE_GRID_CELL_COLUMN_POS;
    m.attr("STYLE_GRID_CELL_COLUMN_SPAN") = (int)LV_STYLE_GRID_CELL_COLUMN_SPAN;
    m.attr("STYLE_GRID_CELL_X_ALIGN") = (int)LV_STYLE_GRID_CELL_X_ALIGN;
    m.attr("STYLE_GRID_CELL_ROW_POS") = (int)LV_STYLE_GRID_CELL_ROW_POS;
    m.attr("STYLE_GRID_CELL_ROW_SPAN") = (int)LV_STYLE_GRID_CELL_ROW_SPAN;
    m.attr("STYLE_GRID_CELL_Y_ALIGN") = (int)LV_STYLE_GRID_CELL_Y_ALIGN;
    m.attr("STYLE_TEXT_LEADING_TRIM") = (int)LV_STYLE_TEXT_LEADING_TRIM;
    m.attr("STYLE_LAST_BUILT_IN_PROP") = (int)LV_STYLE_LAST_BUILT_IN_PROP;
    m.attr("STYLE_NUM_BUILT_IN_PROPS") = (int)LV_STYLE_NUM_BUILT_IN_PROPS;
    m.attr("STYLE_PROP_ANY") = (int)LV_STYLE_PROP_ANY;
    m.attr("STYLE_PROP_CONST") = (int)LV_STYLE_PROP_CONST;

    py::enum_<lv_style_res_t>(m, "STYLE")
        .value("NOT_FOUND", LV_STYLE_RES_NOT_FOUND)
        .value("FOUND", LV_STYLE_RES_FOUND)
;

    py::enum_<lv_display_rotation_t>(m, "DISPLAY_ROTATION")
        .value("_0", LV_DISPLAY_ROTATION_0)
        .value("_90", LV_DISPLAY_ROTATION_90)
        .value("_180", LV_DISPLAY_ROTATION_180)
        .value("_270", LV_DISPLAY_ROTATION_270)
;

    py::enum_<lv_display_render_mode_t>(m, "DISPLAY_RENDER")
        .value("PARTIAL", LV_DISPLAY_RENDER_MODE_PARTIAL)
        .value("DIRECT", LV_DISPLAY_RENDER_MODE_DIRECT)
        .value("FULL", LV_DISPLAY_RENDER_MODE_FULL)
;

    py::enum_<lv_screen_load_anim_t>(m, "SCREEN")
        .value("NONE", LV_SCREEN_LOAD_ANIM_NONE)
        .value("OVER_LEFT", LV_SCREEN_LOAD_ANIM_OVER_LEFT)
        .value("OVER_RIGHT", LV_SCREEN_LOAD_ANIM_OVER_RIGHT)
        .value("OVER_TOP", LV_SCREEN_LOAD_ANIM_OVER_TOP)
        .value("OVER_BOTTOM", LV_SCREEN_LOAD_ANIM_OVER_BOTTOM)
        .value("MOVE_LEFT", LV_SCREEN_LOAD_ANIM_MOVE_LEFT)
        .value("MOVE_RIGHT", LV_SCREEN_LOAD_ANIM_MOVE_RIGHT)
        .value("MOVE_TOP", LV_SCREEN_LOAD_ANIM_MOVE_TOP)
        .value("MOVE_BOTTOM", LV_SCREEN_LOAD_ANIM_MOVE_BOTTOM)
        .value("FADE_IN", LV_SCREEN_LOAD_ANIM_FADE_IN)
        .value("FADE_ON", LV_SCREEN_LOAD_ANIM_FADE_ON)
        .value("FADE_OUT", LV_SCREEN_LOAD_ANIM_FADE_OUT)
        .value("OUT_LEFT", LV_SCREEN_LOAD_ANIM_OUT_LEFT)
        .value("OUT_RIGHT", LV_SCREEN_LOAD_ANIM_OUT_RIGHT)
        .value("OUT_TOP", LV_SCREEN_LOAD_ANIM_OUT_TOP)
        .value("OUT_BOTTOM", LV_SCREEN_LOAD_ANIM_OUT_BOTTOM)
;

    py::enum_<lv_obj_tree_walk_res_t>(m, "OBJ_TREE_WALK")
        .value("NEXT", LV_OBJ_TREE_WALK_NEXT)
        .value("SKIP_CHILDREN", LV_OBJ_TREE_WALK_SKIP_CHILDREN)
        .value("END", LV_OBJ_TREE_WALK_END)
;

    py::enum_<lv_obj_point_transform_flag_t>(m, "OBJ_POINT")
        .value("NONE", LV_OBJ_POINT_TRANSFORM_FLAG_NONE)
        .value("RECURSIVE", LV_OBJ_POINT_TRANSFORM_FLAG_RECURSIVE)
        .value("INVERSE", LV_OBJ_POINT_TRANSFORM_FLAG_INVERSE)
        .value("INVERSE_RECURSIVE", LV_OBJ_POINT_TRANSFORM_FLAG_INVERSE_RECURSIVE)
;

    py::enum_<lv_scrollbar_mode_t>(m, "SCROLLBAR")
        .value("OFF", LV_SCROLLBAR_MODE_OFF)
        .value("ON", LV_SCROLLBAR_MODE_ON)
        .value("ACTIVE", LV_SCROLLBAR_MODE_ACTIVE)
        .value("AUTO", LV_SCROLLBAR_MODE_AUTO)
;

    py::enum_<lv_scroll_snap_t>(m, "SCROLL_SNAP")
        .value("NONE", LV_SCROLL_SNAP_NONE)
        .value("START", LV_SCROLL_SNAP_START)
        .value("END", LV_SCROLL_SNAP_END)
        .value("CENTER", LV_SCROLL_SNAP_CENTER)
;

    py::enum_<lv_state_t>(m, "STATE")
        .value("DEFAULT", LV_STATE_DEFAULT)
        .value("ALT", LV_STATE_ALT)
        .value("CHECKED", LV_STATE_CHECKED)
        .value("FOCUSED", LV_STATE_FOCUSED)
        .value("FOCUS_KEY", LV_STATE_FOCUS_KEY)
        .value("EDITED", LV_STATE_EDITED)
        .value("HOVERED", LV_STATE_HOVERED)
        .value("PRESSED", LV_STATE_PRESSED)
        .value("SCROLLED", LV_STATE_SCROLLED)
        .value("DISABLED", LV_STATE_DISABLED)
        .value("USER_1", LV_STATE_USER_1)
        .value("USER_2", LV_STATE_USER_2)
        .value("USER_3", LV_STATE_USER_3)
        .value("USER_4", LV_STATE_USER_4)
        .value("ANY", LV_STATE_ANY)
;

    py::enum_<lv_part_t>(m, "PART")
        .value("MAIN", LV_PART_MAIN)
        .value("SCROLLBAR", LV_PART_SCROLLBAR)
        .value("INDICATOR", LV_PART_INDICATOR)
        .value("KNOB", LV_PART_KNOB)
        .value("SELECTED", LV_PART_SELECTED)
        .value("ITEMS", LV_PART_ITEMS)
        .value("CURSOR", LV_PART_CURSOR)
        .value("CUSTOM_FIRST", LV_PART_CUSTOM_FIRST)
        .value("ANY", LV_PART_ANY)
;

    py::enum_<lv_style_state_cmp_t>(m, "STYLE_STATE")
        .value("SAME", LV_STYLE_STATE_CMP_SAME)
        .value("DIFF_REDRAW", LV_STYLE_STATE_CMP_DIFF_REDRAW)
        .value("DIFF_DRAW_PAD", LV_STYLE_STATE_CMP_DIFF_DRAW_PAD)
        .value("DIFF_LAYOUT", LV_STYLE_STATE_CMP_DIFF_LAYOUT)
;

    py::enum_<lv_draw_task_type_t>(m, "DRAW_TASK")
        .value("NONE", LV_DRAW_TASK_TYPE_NONE)
        .value("FILL", LV_DRAW_TASK_TYPE_FILL)
        .value("BORDER", LV_DRAW_TASK_TYPE_BORDER)
        .value("BOX_SHADOW", LV_DRAW_TASK_TYPE_BOX_SHADOW)
        .value("LETTER", LV_DRAW_TASK_TYPE_LETTER)
        .value("LABEL", LV_DRAW_TASK_TYPE_LABEL)
        .value("IMAGE", LV_DRAW_TASK_TYPE_IMAGE)
        .value("LAYER", LV_DRAW_TASK_TYPE_LAYER)
        .value("LINE", LV_DRAW_TASK_TYPE_LINE)
        .value("ARC", LV_DRAW_TASK_TYPE_ARC)
        .value("TRIANGLE", LV_DRAW_TASK_TYPE_TRIANGLE)
        .value("MASK_RECTANGLE", LV_DRAW_TASK_TYPE_MASK_RECTANGLE)
        .value("MASK_BITMAP", LV_DRAW_TASK_TYPE_MASK_BITMAP)
        .value("BLUR", LV_DRAW_TASK_TYPE_BLUR)
        .value("VECTOR", LV_DRAW_TASK_TYPE_VECTOR)
;

    py::enum_<lv_draw_task_state_t>(m, "DRAW_TASK_STATE")
        .value("BLOCKED", LV_DRAW_TASK_STATE_BLOCKED)
        .value("WAITING", LV_DRAW_TASK_STATE_WAITING)
        .value("QUEUED", LV_DRAW_TASK_STATE_QUEUED)
        .value("IN_PROGRESS", LV_DRAW_TASK_STATE_IN_PROGRESS)
        .value("FINISHED", LV_DRAW_TASK_STATE_FINISHED)
;

    py::enum_<lv_fs_res_t>(m, "FS_RES")
        .value("OK", LV_FS_RES_OK)
        .value("HW_ERR", LV_FS_RES_HW_ERR)
        .value("FS_ERR", LV_FS_RES_FS_ERR)
        .value("NOT_EX", LV_FS_RES_NOT_EX)
        .value("FULL", LV_FS_RES_FULL)
        .value("LOCKED", LV_FS_RES_LOCKED)
        .value("DENIED", LV_FS_RES_DENIED)
        .value("BUSY", LV_FS_RES_BUSY)
        .value("TOUT", LV_FS_RES_TOUT)
        .value("NOT_IMP", LV_FS_RES_NOT_IMP)
        .value("OUT_OF_MEM", LV_FS_RES_OUT_OF_MEM)
        .value("INV_PARAM", LV_FS_RES_INV_PARAM)
        .value("DRIVE_LETTER_ALREADY_USED", LV_FS_RES_DRIVE_LETTER_ALREADY_USED)
        .value("UNKNOWN", LV_FS_RES_UNKNOWN)
;

    py::enum_<lv_fs_mode_t>(m, "FS_MODE")
        .value("WR", LV_FS_MODE_WR)
        .value("RD", LV_FS_MODE_RD)
;

    py::enum_<lv_fs_whence_t>(m, "FS_WHENCE")
        .value("FS_SEEK_SET", LV_FS_SEEK_SET)
        .value("FS_SEEK_CUR", LV_FS_SEEK_CUR)
        .value("FS_SEEK_END", LV_FS_SEEK_END)
;

    py::enum_<lv_image_src_t>(m, "IMAGE_SRC")
        .value("VARIABLE", LV_IMAGE_SRC_VARIABLE)
        .value("FILE", LV_IMAGE_SRC_FILE)
        .value("SYMBOL", LV_IMAGE_SRC_SYMBOL)
        .value("UNKNOWN", LV_IMAGE_SRC_UNKNOWN)
;

    py::enum_<lv_layer_type_t>(m, "LAYER")
        .value("NONE", LV_LAYER_TYPE_NONE)
        .value("SIMPLE", LV_LAYER_TYPE_SIMPLE)
        .value("TRANSFORM", LV_LAYER_TYPE_TRANSFORM)
;

    py::enum_<lv_obj_class_editable_t>(m, "OBJ_CLASS_EDITABLE")
        .value("INHERIT", LV_OBJ_CLASS_EDITABLE_INHERIT)
        .value("TRUE", LV_OBJ_CLASS_EDITABLE_TRUE)
        .value("FALSE", LV_OBJ_CLASS_EDITABLE_FALSE)
;

    py::enum_<lv_obj_class_group_def_t>(m, "OBJ_CLASS_GROUP_DEF")
        .value("INHERIT", LV_OBJ_CLASS_GROUP_DEF_INHERIT)
        .value("TRUE", LV_OBJ_CLASS_GROUP_DEF_TRUE)
        .value("FALSE", LV_OBJ_CLASS_GROUP_DEF_FALSE)
;

    py::enum_<lv_obj_class_theme_inheritable_t>(m, "OBJ_CLASS_THEME")
        .value("FALSE", LV_OBJ_CLASS_THEME_INHERITABLE_FALSE)
        .value("TRUE", LV_OBJ_CLASS_THEME_INHERITABLE_TRUE)
;

    py::enum_<lv_indev_type_t>(m, "INDEV_TYPE")
        .value("NONE", LV_INDEV_TYPE_NONE)
        .value("POINTER", LV_INDEV_TYPE_POINTER)
        .value("KEYPAD", LV_INDEV_TYPE_KEYPAD)
        .value("BUTTON", LV_INDEV_TYPE_BUTTON)
        .value("ENCODER", LV_INDEV_TYPE_ENCODER)
;

    py::enum_<lv_indev_state_t>(m, "INDEV_STATE")
        .value("RELEASED", LV_INDEV_STATE_RELEASED)
        .value("PRESSED", LV_INDEV_STATE_PRESSED)
;

    py::enum_<lv_indev_mode_t>(m, "INDEV_MODE")
        .value("NONE", LV_INDEV_MODE_NONE)
        .value("TIMER", LV_INDEV_MODE_TIMER)
        .value("EVENT", LV_INDEV_MODE_EVENT)
;

    py::enum_<lv_indev_gesture_type_t>(m, "INDEV_GESTURE")
        .value("NONE", LV_INDEV_GESTURE_NONE)
        .value("PINCH", LV_INDEV_GESTURE_PINCH)
        .value("SWIPE", LV_INDEV_GESTURE_SWIPE)
        .value("ROTATE", LV_INDEV_GESTURE_ROTATE)
        .value("TWO_FINGERS_SWIPE", LV_INDEV_GESTURE_TWO_FINGERS_SWIPE)
        .value("SCROLL", LV_INDEV_GESTURE_SCROLL)
        .value("CNT", LV_INDEV_GESTURE_CNT)
;

    py::enum_<lv_cover_res_t>(m, "COVER")
        .value("COVER", LV_COVER_RES_COVER)
        .value("NOT_COVER", LV_COVER_RES_NOT_COVER)
        .value("MASKED", LV_COVER_RES_MASKED)
;

    py::enum_<lv_obj_flag_t>(m, "OBJ_FLAG")
        .value("HIDDEN", LV_OBJ_FLAG_HIDDEN)
        .value("CLICKABLE", LV_OBJ_FLAG_CLICKABLE)
        .value("CLICK_FOCUSABLE", LV_OBJ_FLAG_CLICK_FOCUSABLE)
        .value("CHECKABLE", LV_OBJ_FLAG_CHECKABLE)
        .value("SCROLLABLE", LV_OBJ_FLAG_SCROLLABLE)
        .value("SCROLL_ELASTIC", LV_OBJ_FLAG_SCROLL_ELASTIC)
        .value("SCROLL_MOMENTUM", LV_OBJ_FLAG_SCROLL_MOMENTUM)
        .value("SCROLL_ONE", LV_OBJ_FLAG_SCROLL_ONE)
        .value("SCROLL_CHAIN_HOR", LV_OBJ_FLAG_SCROLL_CHAIN_HOR)
        .value("SCROLL_CHAIN_VER", LV_OBJ_FLAG_SCROLL_CHAIN_VER)
        .value("SCROLL_CHAIN", LV_OBJ_FLAG_SCROLL_CHAIN)
        .value("SCROLL_ON_FOCUS", LV_OBJ_FLAG_SCROLL_ON_FOCUS)
        .value("SCROLL_WITH_ARROW", LV_OBJ_FLAG_SCROLL_WITH_ARROW)
        .value("SNAPPABLE", LV_OBJ_FLAG_SNAPPABLE)
        .value("PRESS_LOCK", LV_OBJ_FLAG_PRESS_LOCK)
        .value("EVENT_BUBBLE", LV_OBJ_FLAG_EVENT_BUBBLE)
        .value("GESTURE_BUBBLE", LV_OBJ_FLAG_GESTURE_BUBBLE)
        .value("ADV_HITTEST", LV_OBJ_FLAG_ADV_HITTEST)
        .value("IGNORE_LAYOUT", LV_OBJ_FLAG_IGNORE_LAYOUT)
        .value("FLOATING", LV_OBJ_FLAG_FLOATING)
        .value("SEND_DRAW_TASK_EVENTS", LV_OBJ_FLAG_SEND_DRAW_TASK_EVENTS)
        .value("OVERFLOW_VISIBLE", LV_OBJ_FLAG_OVERFLOW_VISIBLE)
        .value("EVENT_TRICKLE", LV_OBJ_FLAG_EVENT_TRICKLE)
        .value("STATE_TRICKLE", LV_OBJ_FLAG_STATE_TRICKLE)
        .value("LAYOUT_1", LV_OBJ_FLAG_LAYOUT_1)
        .value("LAYOUT_2", LV_OBJ_FLAG_LAYOUT_2)
        .value("FLEX_IN_NEW_TRACK", LV_OBJ_FLAG_FLEX_IN_NEW_TRACK)
        .value("WIDGET_1", LV_OBJ_FLAG_WIDGET_1)
        .value("WIDGET_2", LV_OBJ_FLAG_WIDGET_2)
        .value("USER_1", LV_OBJ_FLAG_USER_1)
        .value("USER_2", LV_OBJ_FLAG_USER_2)
        .value("USER_3", LV_OBJ_FLAG_USER_3)
        .value("USER_4", LV_OBJ_FLAG_USER_4)
;

    py::enum_<lv_subject_type_t>(m, "SUBJECT")
        .value("INVALID", LV_SUBJECT_TYPE_INVALID)
        .value("NONE", LV_SUBJECT_TYPE_NONE)
        .value("INT", LV_SUBJECT_TYPE_INT)
        .value("FLOAT", LV_SUBJECT_TYPE_FLOAT)
        .value("POINTER", LV_SUBJECT_TYPE_POINTER)
        .value("COLOR", LV_SUBJECT_TYPE_COLOR)
        .value("GROUP", LV_SUBJECT_TYPE_GROUP)
        .value("STRING", LV_SUBJECT_TYPE_STRING)
;

    py::enum_<lv_vector_fill_t>(m, "VECTOR_FILL")
        .value("NONZERO", LV_VECTOR_FILL_NONZERO)
        .value("EVENODD", LV_VECTOR_FILL_EVENODD)
;

    py::enum_<lv_vector_stroke_cap_t>(m, "VECTOR_STROKE_CAP")
        .value("BUTT", LV_VECTOR_STROKE_CAP_BUTT)
        .value("SQUARE", LV_VECTOR_STROKE_CAP_SQUARE)
        .value("ROUND", LV_VECTOR_STROKE_CAP_ROUND)
;

    py::enum_<lv_vector_stroke_join_t>(m, "VECTOR_STROKE_JOIN")
        .value("MITER", LV_VECTOR_STROKE_JOIN_MITER)
        .value("BEVEL", LV_VECTOR_STROKE_JOIN_BEVEL)
        .value("ROUND", LV_VECTOR_STROKE_JOIN_ROUND)
;

    py::enum_<lv_vector_path_quality_t>(m, "VECTOR_PATH_QUALITY")
        .value("MEDIUM", LV_VECTOR_PATH_QUALITY_MEDIUM)
        .value("HIGH", LV_VECTOR_PATH_QUALITY_HIGH)
        .value("LOW", LV_VECTOR_PATH_QUALITY_LOW)
;

    py::enum_<lv_vector_blend_t>(m, "VECTOR_BLEND")
        .value("OVER", LV_VECTOR_BLEND_SRC_OVER)
        .value("IN", LV_VECTOR_BLEND_SRC_IN)
        .value("DST_OVER", LV_VECTOR_BLEND_DST_OVER)
        .value("DST_IN", LV_VECTOR_BLEND_DST_IN)
        .value("SCREEN", LV_VECTOR_BLEND_SCREEN)
        .value("MULTIPLY", LV_VECTOR_BLEND_MULTIPLY)
        .value("NONE", LV_VECTOR_BLEND_NONE)
        .value("ADDITIVE", LV_VECTOR_BLEND_ADDITIVE)
        .value("SUBTRACTIVE", LV_VECTOR_BLEND_SUBTRACTIVE)
;

    py::enum_<lv_vector_path_op_t>(m, "VECTOR_PATH_OP")
        .value("MOVE_TO", LV_VECTOR_PATH_OP_MOVE_TO)
        .value("LINE_TO", LV_VECTOR_PATH_OP_LINE_TO)
        .value("QUAD_TO", LV_VECTOR_PATH_OP_QUAD_TO)
        .value("CUBIC_TO", LV_VECTOR_PATH_OP_CUBIC_TO)
        .value("CLOSE", LV_VECTOR_PATH_OP_CLOSE)
;

    py::enum_<lv_vector_draw_style_t>(m, "VECTOR_DRAW_STYLE")
        .value("SOLID", LV_VECTOR_DRAW_STYLE_SOLID)
        .value("PATTERN", LV_VECTOR_DRAW_STYLE_PATTERN)
        .value("GRADIENT", LV_VECTOR_DRAW_STYLE_GRADIENT)
;

    py::enum_<lv_vector_gradient_spread_t>(m, "VECTOR_GRADIENT_SPREAD")
        .value("PAD", LV_VECTOR_GRADIENT_SPREAD_PAD)
        .value("REPEAT", LV_VECTOR_GRADIENT_SPREAD_REPEAT)
        .value("REFLECT", LV_VECTOR_GRADIENT_SPREAD_REFLECT)
;

    py::enum_<lv_vector_gradient_style_t>(m, "VECTOR_GRADIENT_STYLE")
        .value("LINEAR", LV_VECTOR_GRADIENT_STYLE_LINEAR)
        .value("RADIAL", LV_VECTOR_GRADIENT_STYLE_RADIAL)
;

    py::enum_<lv_vector_fill_units_t>(m, "VECTOR_FILL_UNITS")
        .value("OBJECT_BOUNDING_BOX", LV_VECTOR_FILL_UNITS_OBJECT_BOUNDING_BOX)
        .value("USER_SPACE_ON_USE", LV_VECTOR_FILL_UNITS_USER_SPACE_ON_USE)
;

    py::enum_<lv_evdev_type_t>(m, "EVDEV")
        .value("REL", LV_EVDEV_TYPE_REL)
        .value("ABS", LV_EVDEV_TYPE_ABS)
        .value("KEY", LV_EVDEV_TYPE_KEY)
;

    py::enum_<lv_font_fmt_txt_cmap_type_t>(m, "FONT_FMT_TXT_CMAP")
        .value("FORMAT0_FULL", LV_FONT_FMT_TXT_CMAP_FORMAT0_FULL)
        .value("SPARSE_FULL", LV_FONT_FMT_TXT_CMAP_SPARSE_FULL)
        .value("FORMAT0_TINY", LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY)
        .value("SPARSE_TINY", LV_FONT_FMT_TXT_CMAP_SPARSE_TINY)
;

    py::enum_<lv_font_fmt_txt_bitmap_format_t>(m, "FONT_FMT_TXT_BITMAP_FORMAT")
        .value("FONT_FMT_TXT_PLAIN", LV_FONT_FMT_TXT_PLAIN)
        .value("FONT_FMT_TXT_COMPRESSED", LV_FONT_FMT_TXT_COMPRESSED)
        .value("FONT_FMT_TXT_COMPRESSED_NO_PREFILTER", LV_FONT_FMT_TXT_COMPRESSED_NO_PREFILTER)
;

    py::enum_<lv_freetype_font_style_t>(m, "FREETYPE_FONT_STYLE")
        .value("NORMAL", LV_FREETYPE_FONT_STYLE_NORMAL)
        .value("ITALIC", LV_FREETYPE_FONT_STYLE_ITALIC)
        .value("BOLD", LV_FREETYPE_FONT_STYLE_BOLD)
;

    py::enum_<lv_freetype_font_render_mode_t>(m, "FREETYPE_FONT_RENDER")
        .value("BITMAP", LV_FREETYPE_FONT_RENDER_MODE_BITMAP)
        .value("OUTLINE", LV_FREETYPE_FONT_RENDER_MODE_OUTLINE)
;

    py::enum_<lv_freetype_outline_type_t>(m, "FREETYPE_OUTLINE")
        .value("END", LV_FREETYPE_OUTLINE_END)
        .value("MOVE_TO", LV_FREETYPE_OUTLINE_MOVE_TO)
        .value("LINE_TO", LV_FREETYPE_OUTLINE_LINE_TO)
        .value("CUBIC_TO", LV_FREETYPE_OUTLINE_CUBIC_TO)
        .value("CONIC_TO", LV_FREETYPE_OUTLINE_CONIC_TO)
;

    /* Enum _lv_tree_walk_mode_t (no C type found, emitting as constants) */
    m.attr("TREE_WALK_PRE_ORDER") = (int)LV_TREE_WALK_PRE_ORDER;
    m.attr("TREE_WALK_POST_ORDER") = (int)LV_TREE_WALK_POST_ORDER;

    py::enum_<lv_image_align_t>(m, "IMAGE_ALIGN")
        .value("DEFAULT", LV_IMAGE_ALIGN_DEFAULT)
        .value("TOP_LEFT", LV_IMAGE_ALIGN_TOP_LEFT)
        .value("TOP_MID", LV_IMAGE_ALIGN_TOP_MID)
        .value("TOP_RIGHT", LV_IMAGE_ALIGN_TOP_RIGHT)
        .value("BOTTOM_LEFT", LV_IMAGE_ALIGN_BOTTOM_LEFT)
        .value("BOTTOM_MID", LV_IMAGE_ALIGN_BOTTOM_MID)
        .value("BOTTOM_RIGHT", LV_IMAGE_ALIGN_BOTTOM_RIGHT)
        .value("LEFT_MID", LV_IMAGE_ALIGN_LEFT_MID)
        .value("RIGHT_MID", LV_IMAGE_ALIGN_RIGHT_MID)
        .value("CENTER", LV_IMAGE_ALIGN_CENTER)
        .value("_LV_IMAGE_ALIGN_AUTO_TRANSFORM", _LV_IMAGE_ALIGN_AUTO_TRANSFORM)
        .value("STRETCH", LV_IMAGE_ALIGN_STRETCH)
        .value("TILE", LV_IMAGE_ALIGN_TILE)
        .value("CONTAIN", LV_IMAGE_ALIGN_CONTAIN)
        .value("CONTAIN_DOWNSCALE", LV_IMAGE_ALIGN_CONTAIN_DOWNSCALE)
        .value("COVER", LV_IMAGE_ALIGN_COVER)
;

    py::enum_<lv_animimg_part_t>(m, "ANIMIMG_PART")
        .value("ANIM_IMAGE_PART_MAIN", LV_ANIM_IMAGE_PART_MAIN)
;

    py::enum_<lv_arc_mode_t>(m, "ARC")
        .value("NORMAL", LV_ARC_MODE_NORMAL)
        .value("SYMMETRICAL", LV_ARC_MODE_SYMMETRICAL)
        .value("REVERSE", LV_ARC_MODE_REVERSE)
;

    py::enum_<lv_arclabel_dir_t>(m, "ARCLABEL_DIR")
        .value("CLOCKWISE", LV_ARCLABEL_DIR_CLOCKWISE)
        .value("COUNTER_CLOCKWISE", LV_ARCLABEL_DIR_COUNTER_CLOCKWISE)
;

    py::enum_<lv_arclabel_text_align_t>(m, "ARCLABEL_TEXT_ALIGN")
        .value("DEFAULT", LV_ARCLABEL_TEXT_ALIGN_DEFAULT)
        .value("LEADING", LV_ARCLABEL_TEXT_ALIGN_LEADING)
        .value("CENTER", LV_ARCLABEL_TEXT_ALIGN_CENTER)
        .value("TRAILING", LV_ARCLABEL_TEXT_ALIGN_TRAILING)
;

    py::enum_<lv_arclabel_overflow_t>(m, "ARCLABEL_OVERFLOW")
        .value("VISIBLE", LV_ARCLABEL_OVERFLOW_VISIBLE)
        .value("ELLIPSIS", LV_ARCLABEL_OVERFLOW_ELLIPSIS)
        .value("CLIP", LV_ARCLABEL_OVERFLOW_CLIP)
;

    py::enum_<lv_label_long_mode_t>(m, "LABEL_LONG")
        .value("WRAP", LV_LABEL_LONG_MODE_WRAP)
        .value("DOTS", LV_LABEL_LONG_MODE_DOTS)
        .value("SCROLL", LV_LABEL_LONG_MODE_SCROLL)
        .value("SCROLL_CIRCULAR", LV_LABEL_LONG_MODE_SCROLL_CIRCULAR)
        .value("CLIP", LV_LABEL_LONG_MODE_CLIP)
;

    py::enum_<lv_bar_mode_t>(m, "BAR")
        .value("NORMAL", LV_BAR_MODE_NORMAL)
        .value("SYMMETRICAL", LV_BAR_MODE_SYMMETRICAL)
        .value("RANGE", LV_BAR_MODE_RANGE)
;

    py::enum_<lv_bar_orientation_t>(m, "BAR_ORIENTATION")
        .value("AUTO", LV_BAR_ORIENTATION_AUTO)
        .value("HORIZONTAL", LV_BAR_ORIENTATION_HORIZONTAL)
        .value("VERTICAL", LV_BAR_ORIENTATION_VERTICAL)
;

    py::enum_<lv_barcode_encoding_t>(m, "BARCODE")
        .value("CODE128_GS1", LV_BARCODE_ENCODING_CODE128_GS1)
        .value("CODE128_RAW", LV_BARCODE_ENCODING_CODE128_RAW)
;

    py::enum_<lv_buttonmatrix_ctrl_t>(m, "BUTTONMATRIX_CTRL")
        .value("NONE", LV_BUTTONMATRIX_CTRL_NONE)
        .value("WIDTH_1", LV_BUTTONMATRIX_CTRL_WIDTH_1)
        .value("WIDTH_2", LV_BUTTONMATRIX_CTRL_WIDTH_2)
        .value("WIDTH_3", LV_BUTTONMATRIX_CTRL_WIDTH_3)
        .value("WIDTH_4", LV_BUTTONMATRIX_CTRL_WIDTH_4)
        .value("WIDTH_5", LV_BUTTONMATRIX_CTRL_WIDTH_5)
        .value("WIDTH_6", LV_BUTTONMATRIX_CTRL_WIDTH_6)
        .value("WIDTH_7", LV_BUTTONMATRIX_CTRL_WIDTH_7)
        .value("WIDTH_8", LV_BUTTONMATRIX_CTRL_WIDTH_8)
        .value("WIDTH_9", LV_BUTTONMATRIX_CTRL_WIDTH_9)
        .value("WIDTH_10", LV_BUTTONMATRIX_CTRL_WIDTH_10)
        .value("WIDTH_11", LV_BUTTONMATRIX_CTRL_WIDTH_11)
        .value("WIDTH_12", LV_BUTTONMATRIX_CTRL_WIDTH_12)
        .value("WIDTH_13", LV_BUTTONMATRIX_CTRL_WIDTH_13)
        .value("WIDTH_14", LV_BUTTONMATRIX_CTRL_WIDTH_14)
        .value("WIDTH_15", LV_BUTTONMATRIX_CTRL_WIDTH_15)
        .value("HIDDEN", LV_BUTTONMATRIX_CTRL_HIDDEN)
        .value("NO_REPEAT", LV_BUTTONMATRIX_CTRL_NO_REPEAT)
        .value("DISABLED", LV_BUTTONMATRIX_CTRL_DISABLED)
        .value("CHECKABLE", LV_BUTTONMATRIX_CTRL_CHECKABLE)
        .value("CHECKED", LV_BUTTONMATRIX_CTRL_CHECKED)
        .value("CLICK_TRIG", LV_BUTTONMATRIX_CTRL_CLICK_TRIG)
        .value("POPOVER", LV_BUTTONMATRIX_CTRL_POPOVER)
        .value("RECOLOR", LV_BUTTONMATRIX_CTRL_RECOLOR)
        .value("RESERVED_1", LV_BUTTONMATRIX_CTRL_RESERVED_1)
        .value("RESERVED_2", LV_BUTTONMATRIX_CTRL_RESERVED_2)
        .value("CUSTOM_1", LV_BUTTONMATRIX_CTRL_CUSTOM_1)
        .value("CUSTOM_2", LV_BUTTONMATRIX_CTRL_CUSTOM_2)
;

    py::enum_<lv_chart_type_t>(m, "CHART")
        .value("NONE", LV_CHART_TYPE_NONE)
        .value("LINE", LV_CHART_TYPE_LINE)
        .value("CURVE", LV_CHART_TYPE_CURVE)
        .value("BAR", LV_CHART_TYPE_BAR)
        .value("STACKED", LV_CHART_TYPE_STACKED)
        .value("SCATTER", LV_CHART_TYPE_SCATTER)
;

    py::enum_<lv_chart_update_mode_t>(m, "CHART_UPDATE")
        .value("SHIFT", LV_CHART_UPDATE_MODE_SHIFT)
        .value("CIRCULAR", LV_CHART_UPDATE_MODE_CIRCULAR)
;

    py::enum_<lv_chart_axis_t>(m, "CHART_AXIS")
        .value("PRIMARY_Y", LV_CHART_AXIS_PRIMARY_Y)
        .value("SECONDARY_Y", LV_CHART_AXIS_SECONDARY_Y)
        .value("PRIMARY_X", LV_CHART_AXIS_PRIMARY_X)
        .value("SECONDARY_X", LV_CHART_AXIS_SECONDARY_X)
        .value("LAST", LV_CHART_AXIS_LAST)
;

    py::enum_<lv_imagebutton_state_t>(m, "IMAGEBUTTON_STATE")
        .value("RELEASED", LV_IMAGEBUTTON_STATE_RELEASED)
        .value("PRESSED", LV_IMAGEBUTTON_STATE_PRESSED)
        .value("DISABLED", LV_IMAGEBUTTON_STATE_DISABLED)
        .value("CHECKED_RELEASED", LV_IMAGEBUTTON_STATE_CHECKED_RELEASED)
        .value("CHECKED_PRESSED", LV_IMAGEBUTTON_STATE_CHECKED_PRESSED)
        .value("CHECKED_DISABLED", LV_IMAGEBUTTON_STATE_CHECKED_DISABLED)
        .value("NUM", LV_IMAGEBUTTON_STATE_NUM)
;

    py::enum_<lv_keyboard_mode_t>(m, "KEYBOARD")
        .value("TEXT_LOWER", LV_KEYBOARD_MODE_TEXT_LOWER)
        .value("TEXT_UPPER", LV_KEYBOARD_MODE_TEXT_UPPER)
        .value("SPECIAL", LV_KEYBOARD_MODE_SPECIAL)
        .value("NUMBER", LV_KEYBOARD_MODE_NUMBER)
        .value("USER_1", LV_KEYBOARD_MODE_USER_1)
        .value("USER_2", LV_KEYBOARD_MODE_USER_2)
        .value("USER_3", LV_KEYBOARD_MODE_USER_3)
        .value("USER_4", LV_KEYBOARD_MODE_USER_4)
;

    py::enum_<lv_menu_mode_header_t>(m, "MENU_MODE_HEADER")
        .value("MENU_HEADER_TOP_FIXED", LV_MENU_HEADER_TOP_FIXED)
        .value("MENU_HEADER_TOP_UNFIXED", LV_MENU_HEADER_TOP_UNFIXED)
        .value("MENU_HEADER_BOTTOM_FIXED", LV_MENU_HEADER_BOTTOM_FIXED)
;

    py::enum_<lv_menu_mode_root_back_button_t>(m, "MENU_MODE_ROOT_BACK_BUTTON")
        .value("MENU_ROOT_BACK_BUTTON_DISABLED", LV_MENU_ROOT_BACK_BUTTON_DISABLED)
        .value("MENU_ROOT_BACK_BUTTON_ENABLED", LV_MENU_ROOT_BACK_BUTTON_ENABLED)
;

    py::enum_<lv_roller_mode_t>(m, "ROLLER")
        .value("NORMAL", LV_ROLLER_MODE_NORMAL)
        .value("INFINITE", LV_ROLLER_MODE_INFINITE)
;

    py::enum_<lv_scale_mode_t>(m, "SCALE")
        .value("HORIZONTAL_TOP", LV_SCALE_MODE_HORIZONTAL_TOP)
        .value("HORIZONTAL_BOTTOM", LV_SCALE_MODE_HORIZONTAL_BOTTOM)
        .value("VERTICAL_LEFT", LV_SCALE_MODE_VERTICAL_LEFT)
        .value("VERTICAL_RIGHT", LV_SCALE_MODE_VERTICAL_RIGHT)
        .value("ROUND_INNER", LV_SCALE_MODE_ROUND_INNER)
        .value("ROUND_OUTER", LV_SCALE_MODE_ROUND_OUTER)
        .value("LAST", LV_SCALE_MODE_LAST)
;

    py::enum_<lv_slider_mode_t>(m, "SLIDER")
        .value("NORMAL", LV_SLIDER_MODE_NORMAL)
        .value("SYMMETRICAL", LV_SLIDER_MODE_SYMMETRICAL)
        .value("RANGE", LV_SLIDER_MODE_RANGE)
;

    py::enum_<lv_slider_orientation_t>(m, "SLIDER_ORIENTATION")
        .value("AUTO", LV_SLIDER_ORIENTATION_AUTO)
        .value("HORIZONTAL", LV_SLIDER_ORIENTATION_HORIZONTAL)
        .value("VERTICAL", LV_SLIDER_ORIENTATION_VERTICAL)
;

    py::enum_<lv_span_overflow_t>(m, "SPAN_OVERFLOW")
        .value("CLIP", LV_SPAN_OVERFLOW_CLIP)
        .value("ELLIPSIS", LV_SPAN_OVERFLOW_ELLIPSIS)
        .value("LAST", LV_SPAN_OVERFLOW_LAST)
;

    py::enum_<lv_span_mode_t>(m, "SPAN")
        .value("FIXED", LV_SPAN_MODE_FIXED)
        .value("EXPAND", LV_SPAN_MODE_EXPAND)
        .value("BREAK", LV_SPAN_MODE_BREAK)
        .value("LAST", LV_SPAN_MODE_LAST)
;

    py::enum_<lv_switch_orientation_t>(m, "SWITCH_ORIENTATION")
        .value("AUTO", LV_SWITCH_ORIENTATION_AUTO)
        .value("HORIZONTAL", LV_SWITCH_ORIENTATION_HORIZONTAL)
        .value("VERTICAL", LV_SWITCH_ORIENTATION_VERTICAL)
;

    py::enum_<lv_table_cell_ctrl_t>(m, "TABLE_CELL_CTRL")
        .value("NONE", LV_TABLE_CELL_CTRL_NONE)
        .value("MERGE_RIGHT", LV_TABLE_CELL_CTRL_MERGE_RIGHT)
        .value("TEXT_CROP", LV_TABLE_CELL_CTRL_TEXT_CROP)
        .value("CUSTOM_1", LV_TABLE_CELL_CTRL_CUSTOM_1)
        .value("CUSTOM_2", LV_TABLE_CELL_CTRL_CUSTOM_2)
        .value("CUSTOM_3", LV_TABLE_CELL_CTRL_CUSTOM_3)
        .value("CUSTOM_4", LV_TABLE_CELL_CTRL_CUSTOM_4)
;

    /*
     * Export non-conflicting enum values into module namespace
     * (conflicting values are only accessible via their enum type)
     */
    m.attr("OUT_TOP_LEFT") = (int)LV_ALIGN_OUT_TOP_LEFT;
    m.attr("OUT_TOP_MID") = (int)LV_ALIGN_OUT_TOP_MID;
    m.attr("OUT_TOP_RIGHT") = (int)LV_ALIGN_OUT_TOP_RIGHT;
    m.attr("OUT_BOTTOM_LEFT") = (int)LV_ALIGN_OUT_BOTTOM_LEFT;
    m.attr("OUT_BOTTOM_MID") = (int)LV_ALIGN_OUT_BOTTOM_MID;
    m.attr("OUT_BOTTOM_RIGHT") = (int)LV_ALIGN_OUT_BOTTOM_RIGHT;
    m.attr("OUT_LEFT_TOP") = (int)LV_ALIGN_OUT_LEFT_TOP;
    m.attr("OUT_LEFT_MID") = (int)LV_ALIGN_OUT_LEFT_MID;
    m.attr("OUT_LEFT_BOTTOM") = (int)LV_ALIGN_OUT_LEFT_BOTTOM;
    m.attr("OUT_RIGHT_TOP") = (int)LV_ALIGN_OUT_RIGHT_TOP;
    m.attr("OUT_RIGHT_MID") = (int)LV_ALIGN_OUT_RIGHT_MID;
    m.attr("OUT_RIGHT_BOTTOM") = (int)LV_ALIGN_OUT_RIGHT_BOTTOM;
    m.attr("PRESSING") = (int)LV_EVENT_PRESSING;
    m.attr("PRESS_LOST") = (int)LV_EVENT_PRESS_LOST;
    m.attr("SHORT_CLICKED") = (int)LV_EVENT_SHORT_CLICKED;
    m.attr("SINGLE_CLICKED") = (int)LV_EVENT_SINGLE_CLICKED;
    m.attr("DOUBLE_CLICKED") = (int)LV_EVENT_DOUBLE_CLICKED;
    m.attr("TRIPLE_CLICKED") = (int)LV_EVENT_TRIPLE_CLICKED;
    m.attr("LONG_PRESSED") = (int)LV_EVENT_LONG_PRESSED;
    m.attr("LONG_PRESSED_REPEAT") = (int)LV_EVENT_LONG_PRESSED_REPEAT;
    m.attr("CLICKED") = (int)LV_EVENT_CLICKED;
    m.attr("SCROLL_BEGIN") = (int)LV_EVENT_SCROLL_BEGIN;
    m.attr("SCROLL_THROW_BEGIN") = (int)LV_EVENT_SCROLL_THROW_BEGIN;
    m.attr("SCROLL_END") = (int)LV_EVENT_SCROLL_END;
    m.attr("GESTURE") = (int)LV_EVENT_GESTURE;
    m.attr("ROTARY") = (int)LV_EVENT_ROTARY;
    m.attr("DEFOCUSED") = (int)LV_EVENT_DEFOCUSED;
    m.attr("LEAVE") = (int)LV_EVENT_LEAVE;
    m.attr("HIT_TEST") = (int)LV_EVENT_HIT_TEST;
    m.attr("INDEV_RESET") = (int)LV_EVENT_INDEV_RESET;
    m.attr("HOVER_OVER") = (int)LV_EVENT_HOVER_OVER;
    m.attr("HOVER_LEAVE") = (int)LV_EVENT_HOVER_LEAVE;
    m.attr("COVER_CHECK") = (int)LV_EVENT_COVER_CHECK;
    m.attr("REFR_EXT_DRAW_SIZE") = (int)LV_EVENT_REFR_EXT_DRAW_SIZE;
    m.attr("DRAW_MAIN_BEGIN") = (int)LV_EVENT_DRAW_MAIN_BEGIN;
    m.attr("DRAW_MAIN") = (int)LV_EVENT_DRAW_MAIN;
    m.attr("DRAW_MAIN_END") = (int)LV_EVENT_DRAW_MAIN_END;
    m.attr("DRAW_POST_BEGIN") = (int)LV_EVENT_DRAW_POST_BEGIN;
    m.attr("DRAW_POST") = (int)LV_EVENT_DRAW_POST;
    m.attr("DRAW_POST_END") = (int)LV_EVENT_DRAW_POST_END;
    m.attr("DRAW_TASK_ADDED") = (int)LV_EVENT_DRAW_TASK_ADDED;
    m.attr("VALUE_CHANGED") = (int)LV_EVENT_VALUE_CHANGED;
    m.attr("INSERT") = (int)LV_EVENT_INSERT;
    m.attr("REFRESH") = (int)LV_EVENT_REFRESH;
    m.attr("READY") = (int)LV_EVENT_READY;
    m.attr("CANCEL") = (int)LV_EVENT_CANCEL;
    m.attr("STATE_CHANGED") = (int)LV_EVENT_STATE_CHANGED;
    m.attr("CREATE") = (int)LV_EVENT_CREATE;
    m.attr("DELETE") = (int)LV_EVENT_DELETE;
    m.attr("CHILD_CHANGED") = (int)LV_EVENT_CHILD_CHANGED;
    m.attr("CHILD_CREATED") = (int)LV_EVENT_CHILD_CREATED;
    m.attr("CHILD_DELETED") = (int)LV_EVENT_CHILD_DELETED;
    m.attr("SCREEN_UNLOAD_START") = (int)LV_EVENT_SCREEN_UNLOAD_START;
    m.attr("SCREEN_LOAD_START") = (int)LV_EVENT_SCREEN_LOAD_START;
    m.attr("SCREEN_LOADED") = (int)LV_EVENT_SCREEN_LOADED;
    m.attr("SCREEN_UNLOADED") = (int)LV_EVENT_SCREEN_UNLOADED;
    m.attr("SIZE_CHANGED") = (int)LV_EVENT_SIZE_CHANGED;
    m.attr("STYLE_CHANGED") = (int)LV_EVENT_STYLE_CHANGED;
    m.attr("LAYOUT_CHANGED") = (int)LV_EVENT_LAYOUT_CHANGED;
    m.attr("GET_SELF_SIZE") = (int)LV_EVENT_GET_SELF_SIZE;
    m.attr("INVALIDATE_AREA") = (int)LV_EVENT_INVALIDATE_AREA;
    m.attr("RESOLUTION_CHANGED") = (int)LV_EVENT_RESOLUTION_CHANGED;
    m.attr("COLOR_FORMAT_CHANGED") = (int)LV_EVENT_COLOR_FORMAT_CHANGED;
    m.attr("REFR_REQUEST") = (int)LV_EVENT_REFR_REQUEST;
    m.attr("REFR_START") = (int)LV_EVENT_REFR_START;
    m.attr("REFR_READY") = (int)LV_EVENT_REFR_READY;
    m.attr("RENDER_START") = (int)LV_EVENT_RENDER_START;
    m.attr("RENDER_READY") = (int)LV_EVENT_RENDER_READY;
    m.attr("FLUSH_START") = (int)LV_EVENT_FLUSH_START;
    m.attr("FLUSH_FINISH") = (int)LV_EVENT_FLUSH_FINISH;
    m.attr("FLUSH_WAIT_START") = (int)LV_EVENT_FLUSH_WAIT_START;
    m.attr("FLUSH_WAIT_FINISH") = (int)LV_EVENT_FLUSH_WAIT_FINISH;
    m.attr("SYNC_START") = (int)LV_EVENT_SYNC_START;
    m.attr("SYNC_FINISH") = (int)LV_EVENT_SYNC_FINISH;
    m.attr("SYNC_WAIT_START") = (int)LV_EVENT_SYNC_WAIT_START;
    m.attr("SYNC_WAIT_FINISH") = (int)LV_EVENT_SYNC_WAIT_FINISH;
    m.attr("UPDATE_LAYOUT_COMPLETED") = (int)LV_EVENT_UPDATE_LAYOUT_COMPLETED;
    m.attr("VSYNC") = (int)LV_EVENT_VSYNC;
    m.attr("VSYNC_REQUEST") = (int)LV_EVENT_VSYNC_REQUEST;
    m.attr("PREPROCESS") = (int)LV_EVENT_PREPROCESS;
    m.attr("MARKED_DELETING") = (int)LV_EVENT_MARKED_DELETING;
    m.attr("UP") = (int)LV_KEY_UP;
    m.attr("DOWN") = (int)LV_KEY_DOWN;
    m.attr("ESC") = (int)LV_KEY_ESC;
    m.attr("DEL") = (int)LV_KEY_DEL;
    m.attr("BACKSPACE") = (int)LV_KEY_BACKSPACE;
    m.attr("ENTER") = (int)LV_KEY_ENTER;
    m.attr("HOME") = (int)LV_KEY_HOME;
    m.attr("RAW") = (int)LV_COLOR_FORMAT_RAW;
    m.attr("RAW_ALPHA") = (int)LV_COLOR_FORMAT_RAW_ALPHA;
    m.attr("L8") = (int)LV_COLOR_FORMAT_L8;
    m.attr("I1") = (int)LV_COLOR_FORMAT_I1;
    m.attr("I2") = (int)LV_COLOR_FORMAT_I2;
    m.attr("I4") = (int)LV_COLOR_FORMAT_I4;
    m.attr("I8") = (int)LV_COLOR_FORMAT_I8;
    m.attr("RGB565") = (int)LV_COLOR_FORMAT_RGB565;
    m.attr("ARGB8565") = (int)LV_COLOR_FORMAT_ARGB8565;
    m.attr("RGB565A8") = (int)LV_COLOR_FORMAT_RGB565A8;
    m.attr("AL88") = (int)LV_COLOR_FORMAT_AL88;
    m.attr("RGB565_SWAPPED") = (int)LV_COLOR_FORMAT_RGB565_SWAPPED;
    m.attr("RGB888") = (int)LV_COLOR_FORMAT_RGB888;
    m.attr("ARGB8888") = (int)LV_COLOR_FORMAT_ARGB8888;
    m.attr("XRGB8888") = (int)LV_COLOR_FORMAT_XRGB8888;
    m.attr("ARGB8888_PREMULTIPLIED") = (int)LV_COLOR_FORMAT_ARGB8888_PREMULTIPLIED;
    m.attr("ARGB1555") = (int)LV_COLOR_FORMAT_ARGB1555;
    m.attr("ARGB4444") = (int)LV_COLOR_FORMAT_ARGB4444;
    m.attr("ARGB2222") = (int)LV_COLOR_FORMAT_ARGB2222;
    m.attr("YUV_START") = (int)LV_COLOR_FORMAT_YUV_START;
    m.attr("I420") = (int)LV_COLOR_FORMAT_I420;
    m.attr("I422") = (int)LV_COLOR_FORMAT_I422;
    m.attr("I444") = (int)LV_COLOR_FORMAT_I444;
    m.attr("I400") = (int)LV_COLOR_FORMAT_I400;
    m.attr("NV21") = (int)LV_COLOR_FORMAT_NV21;
    m.attr("NV12") = (int)LV_COLOR_FORMAT_NV12;
    m.attr("YUY2") = (int)LV_COLOR_FORMAT_YUY2;
    m.attr("UYVY") = (int)LV_COLOR_FORMAT_UYVY;
    m.attr("YUV_END") = (int)LV_COLOR_FORMAT_YUV_END;
    m.attr("PROPRIETARY_START") = (int)LV_COLOR_FORMAT_PROPRIETARY_START;
    m.attr("NEMA_TSC_START") = (int)LV_COLOR_FORMAT_NEMA_TSC_START;
    m.attr("NEMA_TSC4") = (int)LV_COLOR_FORMAT_NEMA_TSC4;
    m.attr("NEMA_TSC6") = (int)LV_COLOR_FORMAT_NEMA_TSC6;
    m.attr("NEMA_TSC6A") = (int)LV_COLOR_FORMAT_NEMA_TSC6A;
    m.attr("NEMA_TSC6AP") = (int)LV_COLOR_FORMAT_NEMA_TSC6AP;
    m.attr("NEMA_TSC12") = (int)LV_COLOR_FORMAT_NEMA_TSC12;
    m.attr("NEMA_TSC12A") = (int)LV_COLOR_FORMAT_NEMA_TSC12A;
    m.attr("NEMA_TSC_END") = (int)LV_COLOR_FORMAT_NEMA_TSC_END;
    m.attr("NATIVE") = (int)LV_COLOR_FORMAT_NATIVE;
    m.attr("NATIVE_WITH_ALPHA") = (int)LV_COLOR_FORMAT_NATIVE_WITH_ALPHA;
    m.attr("RED") = (int)LV_PALETTE_RED;
    m.attr("PINK") = (int)LV_PALETTE_PINK;
    m.attr("PURPLE") = (int)LV_PALETTE_PURPLE;
    m.attr("DEEP_PURPLE") = (int)LV_PALETTE_DEEP_PURPLE;
    m.attr("INDIGO") = (int)LV_PALETTE_INDIGO;
    m.attr("BLUE") = (int)LV_PALETTE_BLUE;
    m.attr("LIGHT_BLUE") = (int)LV_PALETTE_LIGHT_BLUE;
    m.attr("CYAN") = (int)LV_PALETTE_CYAN;
    m.attr("TEAL") = (int)LV_PALETTE_TEAL;
    m.attr("GREEN") = (int)LV_PALETTE_GREEN;
    m.attr("LIGHT_GREEN") = (int)LV_PALETTE_LIGHT_GREEN;
    m.attr("LIME") = (int)LV_PALETTE_LIME;
    m.attr("YELLOW") = (int)LV_PALETTE_YELLOW;
    m.attr("AMBER") = (int)LV_PALETTE_AMBER;
    m.attr("ORANGE") = (int)LV_PALETTE_ORANGE;
    m.attr("DEEP_ORANGE") = (int)LV_PALETTE_DEEP_ORANGE;
    m.attr("BROWN") = (int)LV_PALETTE_BROWN;
    m.attr("BLUE_GREY") = (int)LV_PALETTE_BLUE_GREY;
    m.attr("GREY") = (int)LV_PALETTE_GREY;
    m.attr("RLE") = (int)LV_IMAGE_COMPRESS_RLE;
    m.attr("LZ4") = (int)LV_IMAGE_COMPRESS_LZ4;
    m.attr("A3") = (int)LV_FONT_GLYPH_FORMAT_A3;
    m.attr("SVG") = (int)LV_FONT_GLYPH_FORMAT_SVG;
    m.attr("CUSTOM") = (int)LV_FONT_GLYPH_FORMAT_CUSTOM;
    m.attr("BOTH") = (int)LV_FONT_SUBPX_BOTH;
    m.attr("FIT") = (int)LV_TEXT_FLAG_FIT;
    m.attr("BREAK_ALL") = (int)LV_TEXT_FLAG_BREAK_ALL;
    m.attr("CAPITAL_BASELINE") = (int)LV_TEXT_LEADING_TRIM_CAPITAL_BASELINE;
    m.attr("LOWER_BASELINE") = (int)LV_TEXT_LEADING_TRIM_LOWER_BASELINE;
    m.attr("CAPITAL") = (int)LV_TEXT_LEADING_TRIM_CAPITAL;
    m.attr("LOWER") = (int)LV_TEXT_LEADING_TRIM_LOWER;
    m.attr("LTR") = (int)LV_BASE_DIR_LTR;
    m.attr("RTL") = (int)LV_BASE_DIR_RTL;
    m.attr("NEUTRAL") = (int)LV_BASE_DIR_NEUTRAL;
    m.attr("WEAK") = (int)LV_BASE_DIR_WEAK;
    m.attr("CONICAL") = (int)LV_GRAD_DIR_CONICAL;
    m.attr("FLEX") = (int)LV_LAYOUT_FLEX;
    m.attr("GRID") = (int)LV_LAYOUT_GRID;
    m.attr("ROW") = (int)LV_FLEX_FLOW_ROW;
    m.attr("COLUMN") = (int)LV_FLEX_FLOW_COLUMN;
    m.attr("ROW_WRAP") = (int)LV_FLEX_FLOW_ROW_WRAP;
    m.attr("ROW_REVERSE") = (int)LV_FLEX_FLOW_ROW_REVERSE;
    m.attr("ROW_WRAP_REVERSE") = (int)LV_FLEX_FLOW_ROW_WRAP_REVERSE;
    m.attr("COLUMN_WRAP") = (int)LV_FLEX_FLOW_COLUMN_WRAP;
    m.attr("COLUMN_REVERSE") = (int)LV_FLEX_FLOW_COLUMN_REVERSE;
    m.attr("COLUMN_WRAP_REVERSE") = (int)LV_FLEX_FLOW_COLUMN_WRAP_REVERSE;
    m.attr("DIFFERENCE") = (int)LV_BLEND_MODE_DIFFERENCE;
    m.attr("UNDERLINE") = (int)LV_TEXT_DECOR_UNDERLINE;
    m.attr("STRIKETHROUGH") = (int)LV_TEXT_DECOR_STRIKETHROUGH;
    m.attr("INTERNAL") = (int)LV_BORDER_SIDE_INTERNAL;
    m.attr("SPEED") = (int)LV_BLUR_QUALITY_SPEED;
    m.attr("PRECISION") = (int)LV_BLUR_QUALITY_PRECISION;
    m.attr("NOT_FOUND") = (int)LV_STYLE_RES_NOT_FOUND;
    m.attr("FOUND") = (int)LV_STYLE_RES_FOUND;
    m.attr("_0") = (int)LV_DISPLAY_ROTATION_0;
    m.attr("_90") = (int)LV_DISPLAY_ROTATION_90;
    m.attr("_180") = (int)LV_DISPLAY_ROTATION_180;
    m.attr("_270") = (int)LV_DISPLAY_ROTATION_270;
    m.attr("PARTIAL") = (int)LV_DISPLAY_RENDER_MODE_PARTIAL;
    m.attr("DIRECT") = (int)LV_DISPLAY_RENDER_MODE_DIRECT;
    m.attr("OVER_LEFT") = (int)LV_SCREEN_LOAD_ANIM_OVER_LEFT;
    m.attr("OVER_RIGHT") = (int)LV_SCREEN_LOAD_ANIM_OVER_RIGHT;
    m.attr("OVER_TOP") = (int)LV_SCREEN_LOAD_ANIM_OVER_TOP;
    m.attr("OVER_BOTTOM") = (int)LV_SCREEN_LOAD_ANIM_OVER_BOTTOM;
    m.attr("MOVE_LEFT") = (int)LV_SCREEN_LOAD_ANIM_MOVE_LEFT;
    m.attr("MOVE_RIGHT") = (int)LV_SCREEN_LOAD_ANIM_MOVE_RIGHT;
    m.attr("MOVE_TOP") = (int)LV_SCREEN_LOAD_ANIM_MOVE_TOP;
    m.attr("MOVE_BOTTOM") = (int)LV_SCREEN_LOAD_ANIM_MOVE_BOTTOM;
    m.attr("FADE_IN") = (int)LV_SCREEN_LOAD_ANIM_FADE_IN;
    m.attr("FADE_ON") = (int)LV_SCREEN_LOAD_ANIM_FADE_ON;
    m.attr("FADE_OUT") = (int)LV_SCREEN_LOAD_ANIM_FADE_OUT;
    m.attr("OUT_LEFT") = (int)LV_SCREEN_LOAD_ANIM_OUT_LEFT;
    m.attr("OUT_RIGHT") = (int)LV_SCREEN_LOAD_ANIM_OUT_RIGHT;
    m.attr("OUT_TOP") = (int)LV_SCREEN_LOAD_ANIM_OUT_TOP;
    m.attr("OUT_BOTTOM") = (int)LV_SCREEN_LOAD_ANIM_OUT_BOTTOM;
    m.attr("SKIP_CHILDREN") = (int)LV_OBJ_TREE_WALK_SKIP_CHILDREN;
    m.attr("RECURSIVE") = (int)LV_OBJ_POINT_TRANSFORM_FLAG_RECURSIVE;
    m.attr("INVERSE") = (int)LV_OBJ_POINT_TRANSFORM_FLAG_INVERSE;
    m.attr("INVERSE_RECURSIVE") = (int)LV_OBJ_POINT_TRANSFORM_FLAG_INVERSE_RECURSIVE;
    m.attr("OFF") = (int)LV_SCROLLBAR_MODE_OFF;
    m.attr("ON") = (int)LV_SCROLLBAR_MODE_ON;
    m.attr("ACTIVE") = (int)LV_SCROLLBAR_MODE_ACTIVE;
    m.attr("ALT") = (int)LV_STATE_ALT;
    m.attr("FOCUS_KEY") = (int)LV_STATE_FOCUS_KEY;
    m.attr("EDITED") = (int)LV_STATE_EDITED;
    m.attr("HOVERED") = (int)LV_STATE_HOVERED;
    m.attr("SCROLLED") = (int)LV_STATE_SCROLLED;
    m.attr("MAIN") = (int)LV_PART_MAIN;
    m.attr("INDICATOR") = (int)LV_PART_INDICATOR;
    m.attr("KNOB") = (int)LV_PART_KNOB;
    m.attr("SELECTED") = (int)LV_PART_SELECTED;
    m.attr("ITEMS") = (int)LV_PART_ITEMS;
    m.attr("CURSOR") = (int)LV_PART_CURSOR;
    m.attr("CUSTOM_FIRST") = (int)LV_PART_CUSTOM_FIRST;
    m.attr("SAME") = (int)LV_STYLE_STATE_CMP_SAME;
    m.attr("DIFF_REDRAW") = (int)LV_STYLE_STATE_CMP_DIFF_REDRAW;
    m.attr("DIFF_DRAW_PAD") = (int)LV_STYLE_STATE_CMP_DIFF_DRAW_PAD;
    m.attr("DIFF_LAYOUT") = (int)LV_STYLE_STATE_CMP_DIFF_LAYOUT;
    m.attr("FILL") = (int)LV_DRAW_TASK_TYPE_FILL;
    m.attr("BORDER") = (int)LV_DRAW_TASK_TYPE_BORDER;
    m.attr("BOX_SHADOW") = (int)LV_DRAW_TASK_TYPE_BOX_SHADOW;
    m.attr("LETTER") = (int)LV_DRAW_TASK_TYPE_LETTER;
    m.attr("LABEL") = (int)LV_DRAW_TASK_TYPE_LABEL;
    m.attr("TRIANGLE") = (int)LV_DRAW_TASK_TYPE_TRIANGLE;
    m.attr("MASK_RECTANGLE") = (int)LV_DRAW_TASK_TYPE_MASK_RECTANGLE;
    m.attr("MASK_BITMAP") = (int)LV_DRAW_TASK_TYPE_MASK_BITMAP;
    m.attr("BLUR") = (int)LV_DRAW_TASK_TYPE_BLUR;
    m.attr("BLOCKED") = (int)LV_DRAW_TASK_STATE_BLOCKED;
    m.attr("WAITING") = (int)LV_DRAW_TASK_STATE_WAITING;
    m.attr("QUEUED") = (int)LV_DRAW_TASK_STATE_QUEUED;
    m.attr("IN_PROGRESS") = (int)LV_DRAW_TASK_STATE_IN_PROGRESS;
    m.attr("FINISHED") = (int)LV_DRAW_TASK_STATE_FINISHED;
    m.attr("HW_ERR") = (int)LV_FS_RES_HW_ERR;
    m.attr("FS_ERR") = (int)LV_FS_RES_FS_ERR;
    m.attr("NOT_EX") = (int)LV_FS_RES_NOT_EX;
    m.attr("LOCKED") = (int)LV_FS_RES_LOCKED;
    m.attr("DENIED") = (int)LV_FS_RES_DENIED;
    m.attr("BUSY") = (int)LV_FS_RES_BUSY;
    m.attr("TOUT") = (int)LV_FS_RES_TOUT;
    m.attr("NOT_IMP") = (int)LV_FS_RES_NOT_IMP;
    m.attr("OUT_OF_MEM") = (int)LV_FS_RES_OUT_OF_MEM;
    m.attr("INV_PARAM") = (int)LV_FS_RES_INV_PARAM;
    m.attr("DRIVE_LETTER_ALREADY_USED") = (int)LV_FS_RES_DRIVE_LETTER_ALREADY_USED;
    m.attr("WR") = (int)LV_FS_MODE_WR;
    m.attr("RD") = (int)LV_FS_MODE_RD;
    m.attr("FS_SEEK_SET") = (int)LV_FS_SEEK_SET;
    m.attr("FS_SEEK_CUR") = (int)LV_FS_SEEK_CUR;
    m.attr("FS_SEEK_END") = (int)LV_FS_SEEK_END;
    m.attr("VARIABLE") = (int)LV_IMAGE_SRC_VARIABLE;
    m.attr("FILE") = (int)LV_IMAGE_SRC_FILE;
    m.attr("SYMBOL") = (int)LV_IMAGE_SRC_SYMBOL;
    m.attr("SIMPLE") = (int)LV_LAYER_TYPE_SIMPLE;
    m.attr("TRANSFORM") = (int)LV_LAYER_TYPE_TRANSFORM;
    m.attr("KEYPAD") = (int)LV_INDEV_TYPE_KEYPAD;
    m.attr("BUTTON") = (int)LV_INDEV_TYPE_BUTTON;
    m.attr("ENCODER") = (int)LV_INDEV_TYPE_ENCODER;
    m.attr("TIMER") = (int)LV_INDEV_MODE_TIMER;
    m.attr("PINCH") = (int)LV_INDEV_GESTURE_PINCH;
    m.attr("SWIPE") = (int)LV_INDEV_GESTURE_SWIPE;
    m.attr("ROTATE") = (int)LV_INDEV_GESTURE_ROTATE;
    m.attr("TWO_FINGERS_SWIPE") = (int)LV_INDEV_GESTURE_TWO_FINGERS_SWIPE;
    m.attr("CNT") = (int)LV_INDEV_GESTURE_CNT;
    m.attr("NOT_COVER") = (int)LV_COVER_RES_NOT_COVER;
    m.attr("MASKED") = (int)LV_COVER_RES_MASKED;
    m.attr("CLICKABLE") = (int)LV_OBJ_FLAG_CLICKABLE;
    m.attr("CLICK_FOCUSABLE") = (int)LV_OBJ_FLAG_CLICK_FOCUSABLE;
    m.attr("SCROLLABLE") = (int)LV_OBJ_FLAG_SCROLLABLE;
    m.attr("SCROLL_ELASTIC") = (int)LV_OBJ_FLAG_SCROLL_ELASTIC;
    m.attr("SCROLL_MOMENTUM") = (int)LV_OBJ_FLAG_SCROLL_MOMENTUM;
    m.attr("SCROLL_ONE") = (int)LV_OBJ_FLAG_SCROLL_ONE;
    m.attr("SCROLL_CHAIN_HOR") = (int)LV_OBJ_FLAG_SCROLL_CHAIN_HOR;
    m.attr("SCROLL_CHAIN_VER") = (int)LV_OBJ_FLAG_SCROLL_CHAIN_VER;
    m.attr("SCROLL_CHAIN") = (int)LV_OBJ_FLAG_SCROLL_CHAIN;
    m.attr("SCROLL_ON_FOCUS") = (int)LV_OBJ_FLAG_SCROLL_ON_FOCUS;
    m.attr("SCROLL_WITH_ARROW") = (int)LV_OBJ_FLAG_SCROLL_WITH_ARROW;
    m.attr("SNAPPABLE") = (int)LV_OBJ_FLAG_SNAPPABLE;
    m.attr("PRESS_LOCK") = (int)LV_OBJ_FLAG_PRESS_LOCK;
    m.attr("EVENT_BUBBLE") = (int)LV_OBJ_FLAG_EVENT_BUBBLE;
    m.attr("GESTURE_BUBBLE") = (int)LV_OBJ_FLAG_GESTURE_BUBBLE;
    m.attr("ADV_HITTEST") = (int)LV_OBJ_FLAG_ADV_HITTEST;
    m.attr("IGNORE_LAYOUT") = (int)LV_OBJ_FLAG_IGNORE_LAYOUT;
    m.attr("FLOATING") = (int)LV_OBJ_FLAG_FLOATING;
    m.attr("SEND_DRAW_TASK_EVENTS") = (int)LV_OBJ_FLAG_SEND_DRAW_TASK_EVENTS;
    m.attr("OVERFLOW_VISIBLE") = (int)LV_OBJ_FLAG_OVERFLOW_VISIBLE;
    m.attr("EVENT_TRICKLE") = (int)LV_OBJ_FLAG_EVENT_TRICKLE;
    m.attr("STATE_TRICKLE") = (int)LV_OBJ_FLAG_STATE_TRICKLE;
    m.attr("LAYOUT_1") = (int)LV_OBJ_FLAG_LAYOUT_1;
    m.attr("LAYOUT_2") = (int)LV_OBJ_FLAG_LAYOUT_2;
    m.attr("FLEX_IN_NEW_TRACK") = (int)LV_OBJ_FLAG_FLEX_IN_NEW_TRACK;
    m.attr("WIDGET_1") = (int)LV_OBJ_FLAG_WIDGET_1;
    m.attr("WIDGET_2") = (int)LV_OBJ_FLAG_WIDGET_2;
    m.attr("INT") = (int)LV_SUBJECT_TYPE_INT;
    m.attr("FLOAT") = (int)LV_SUBJECT_TYPE_FLOAT;
    m.attr("COLOR") = (int)LV_SUBJECT_TYPE_COLOR;
    m.attr("STRING") = (int)LV_SUBJECT_TYPE_STRING;
    m.attr("NONZERO") = (int)LV_VECTOR_FILL_NONZERO;
    m.attr("EVENODD") = (int)LV_VECTOR_FILL_EVENODD;
    m.attr("BUTT") = (int)LV_VECTOR_STROKE_CAP_BUTT;
    m.attr("SQUARE") = (int)LV_VECTOR_STROKE_CAP_SQUARE;
    m.attr("MITER") = (int)LV_VECTOR_STROKE_JOIN_MITER;
    m.attr("BEVEL") = (int)LV_VECTOR_STROKE_JOIN_BEVEL;
    m.attr("MEDIUM") = (int)LV_VECTOR_PATH_QUALITY_MEDIUM;
    m.attr("HIGH") = (int)LV_VECTOR_PATH_QUALITY_HIGH;
    m.attr("LOW") = (int)LV_VECTOR_PATH_QUALITY_LOW;
    m.attr("OVER") = (int)LV_VECTOR_BLEND_SRC_OVER;
    m.attr("IN") = (int)LV_VECTOR_BLEND_SRC_IN;
    m.attr("DST_OVER") = (int)LV_VECTOR_BLEND_DST_OVER;
    m.attr("DST_IN") = (int)LV_VECTOR_BLEND_DST_IN;
    m.attr("QUAD_TO") = (int)LV_VECTOR_PATH_OP_QUAD_TO;
    m.attr("CLOSE") = (int)LV_VECTOR_PATH_OP_CLOSE;
    m.attr("SOLID") = (int)LV_VECTOR_DRAW_STYLE_SOLID;
    m.attr("PATTERN") = (int)LV_VECTOR_DRAW_STYLE_PATTERN;
    m.attr("GRADIENT") = (int)LV_VECTOR_DRAW_STYLE_GRADIENT;
    m.attr("OBJECT_BOUNDING_BOX") = (int)LV_VECTOR_FILL_UNITS_OBJECT_BOUNDING_BOX;
    m.attr("USER_SPACE_ON_USE") = (int)LV_VECTOR_FILL_UNITS_USER_SPACE_ON_USE;
    m.attr("REL") = (int)LV_EVDEV_TYPE_REL;
    m.attr("ABS") = (int)LV_EVDEV_TYPE_ABS;
    m.attr("FORMAT0_FULL") = (int)LV_FONT_FMT_TXT_CMAP_FORMAT0_FULL;
    m.attr("SPARSE_FULL") = (int)LV_FONT_FMT_TXT_CMAP_SPARSE_FULL;
    m.attr("FORMAT0_TINY") = (int)LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY;
    m.attr("SPARSE_TINY") = (int)LV_FONT_FMT_TXT_CMAP_SPARSE_TINY;
    m.attr("FONT_FMT_TXT_PLAIN") = (int)LV_FONT_FMT_TXT_PLAIN;
    m.attr("FONT_FMT_TXT_COMPRESSED") = (int)LV_FONT_FMT_TXT_COMPRESSED;
    m.attr("FONT_FMT_TXT_COMPRESSED_NO_PREFILTER") = (int)LV_FONT_FMT_TXT_COMPRESSED_NO_PREFILTER;
    m.attr("ITALIC") = (int)LV_FREETYPE_FONT_STYLE_ITALIC;
    m.attr("BOLD") = (int)LV_FREETYPE_FONT_STYLE_BOLD;
    m.attr("BITMAP") = (int)LV_FREETYPE_FONT_RENDER_MODE_BITMAP;
    m.attr("OUTLINE") = (int)LV_FREETYPE_FONT_RENDER_MODE_OUTLINE;
    m.attr("CONIC_TO") = (int)LV_FREETYPE_OUTLINE_CONIC_TO;
    m.attr("_LV_IMAGE_ALIGN_AUTO_TRANSFORM") = (int)_LV_IMAGE_ALIGN_AUTO_TRANSFORM;
    m.attr("TILE") = (int)LV_IMAGE_ALIGN_TILE;
    m.attr("CONTAIN") = (int)LV_IMAGE_ALIGN_CONTAIN;
    m.attr("CONTAIN_DOWNSCALE") = (int)LV_IMAGE_ALIGN_CONTAIN_DOWNSCALE;
    m.attr("ANIM_IMAGE_PART_MAIN") = (int)LV_ANIM_IMAGE_PART_MAIN;
    m.attr("REVERSE") = (int)LV_ARC_MODE_REVERSE;
    m.attr("CLOCKWISE") = (int)LV_ARCLABEL_DIR_CLOCKWISE;
    m.attr("COUNTER_CLOCKWISE") = (int)LV_ARCLABEL_DIR_COUNTER_CLOCKWISE;
    m.attr("LEADING") = (int)LV_ARCLABEL_TEXT_ALIGN_LEADING;
    m.attr("TRAILING") = (int)LV_ARCLABEL_TEXT_ALIGN_TRAILING;
    m.attr("VISIBLE") = (int)LV_ARCLABEL_OVERFLOW_VISIBLE;
    m.attr("WRAP") = (int)LV_LABEL_LONG_MODE_WRAP;
    m.attr("DOTS") = (int)LV_LABEL_LONG_MODE_DOTS;
    m.attr("SCROLL_CIRCULAR") = (int)LV_LABEL_LONG_MODE_SCROLL_CIRCULAR;
    m.attr("CODE128_GS1") = (int)LV_BARCODE_ENCODING_CODE128_GS1;
    m.attr("CODE128_RAW") = (int)LV_BARCODE_ENCODING_CODE128_RAW;
    m.attr("WIDTH_1") = (int)LV_BUTTONMATRIX_CTRL_WIDTH_1;
    m.attr("WIDTH_2") = (int)LV_BUTTONMATRIX_CTRL_WIDTH_2;
    m.attr("WIDTH_3") = (int)LV_BUTTONMATRIX_CTRL_WIDTH_3;
    m.attr("WIDTH_4") = (int)LV_BUTTONMATRIX_CTRL_WIDTH_4;
    m.attr("WIDTH_5") = (int)LV_BUTTONMATRIX_CTRL_WIDTH_5;
    m.attr("WIDTH_6") = (int)LV_BUTTONMATRIX_CTRL_WIDTH_6;
    m.attr("WIDTH_7") = (int)LV_BUTTONMATRIX_CTRL_WIDTH_7;
    m.attr("WIDTH_8") = (int)LV_BUTTONMATRIX_CTRL_WIDTH_8;
    m.attr("WIDTH_9") = (int)LV_BUTTONMATRIX_CTRL_WIDTH_9;
    m.attr("WIDTH_10") = (int)LV_BUTTONMATRIX_CTRL_WIDTH_10;
    m.attr("WIDTH_11") = (int)LV_BUTTONMATRIX_CTRL_WIDTH_11;
    m.attr("WIDTH_12") = (int)LV_BUTTONMATRIX_CTRL_WIDTH_12;
    m.attr("WIDTH_13") = (int)LV_BUTTONMATRIX_CTRL_WIDTH_13;
    m.attr("WIDTH_14") = (int)LV_BUTTONMATRIX_CTRL_WIDTH_14;
    m.attr("WIDTH_15") = (int)LV_BUTTONMATRIX_CTRL_WIDTH_15;
    m.attr("NO_REPEAT") = (int)LV_BUTTONMATRIX_CTRL_NO_REPEAT;
    m.attr("CLICK_TRIG") = (int)LV_BUTTONMATRIX_CTRL_CLICK_TRIG;
    m.attr("POPOVER") = (int)LV_BUTTONMATRIX_CTRL_POPOVER;
    m.attr("RESERVED_1") = (int)LV_BUTTONMATRIX_CTRL_RESERVED_1;
    m.attr("RESERVED_2") = (int)LV_BUTTONMATRIX_CTRL_RESERVED_2;
    m.attr("CURVE") = (int)LV_CHART_TYPE_CURVE;
    m.attr("STACKED") = (int)LV_CHART_TYPE_STACKED;
    m.attr("SCATTER") = (int)LV_CHART_TYPE_SCATTER;
    m.attr("SHIFT") = (int)LV_CHART_UPDATE_MODE_SHIFT;
    m.attr("CIRCULAR") = (int)LV_CHART_UPDATE_MODE_CIRCULAR;
    m.attr("PRIMARY_Y") = (int)LV_CHART_AXIS_PRIMARY_Y;
    m.attr("SECONDARY_Y") = (int)LV_CHART_AXIS_SECONDARY_Y;
    m.attr("PRIMARY_X") = (int)LV_CHART_AXIS_PRIMARY_X;
    m.attr("SECONDARY_X") = (int)LV_CHART_AXIS_SECONDARY_X;
    m.attr("CHECKED_RELEASED") = (int)LV_IMAGEBUTTON_STATE_CHECKED_RELEASED;
    m.attr("CHECKED_PRESSED") = (int)LV_IMAGEBUTTON_STATE_CHECKED_PRESSED;
    m.attr("CHECKED_DISABLED") = (int)LV_IMAGEBUTTON_STATE_CHECKED_DISABLED;
    m.attr("NUM") = (int)LV_IMAGEBUTTON_STATE_NUM;
    m.attr("TEXT_LOWER") = (int)LV_KEYBOARD_MODE_TEXT_LOWER;
    m.attr("TEXT_UPPER") = (int)LV_KEYBOARD_MODE_TEXT_UPPER;
    m.attr("SPECIAL") = (int)LV_KEYBOARD_MODE_SPECIAL;
    m.attr("NUMBER") = (int)LV_KEYBOARD_MODE_NUMBER;
    m.attr("MENU_HEADER_TOP_FIXED") = (int)LV_MENU_HEADER_TOP_FIXED;
    m.attr("MENU_HEADER_TOP_UNFIXED") = (int)LV_MENU_HEADER_TOP_UNFIXED;
    m.attr("MENU_HEADER_BOTTOM_FIXED") = (int)LV_MENU_HEADER_BOTTOM_FIXED;
    m.attr("MENU_ROOT_BACK_BUTTON_DISABLED") = (int)LV_MENU_ROOT_BACK_BUTTON_DISABLED;
    m.attr("MENU_ROOT_BACK_BUTTON_ENABLED") = (int)LV_MENU_ROOT_BACK_BUTTON_ENABLED;
    m.attr("INFINITE") = (int)LV_ROLLER_MODE_INFINITE;
    m.attr("HORIZONTAL_TOP") = (int)LV_SCALE_MODE_HORIZONTAL_TOP;
    m.attr("HORIZONTAL_BOTTOM") = (int)LV_SCALE_MODE_HORIZONTAL_BOTTOM;
    m.attr("VERTICAL_LEFT") = (int)LV_SCALE_MODE_VERTICAL_LEFT;
    m.attr("VERTICAL_RIGHT") = (int)LV_SCALE_MODE_VERTICAL_RIGHT;
    m.attr("ROUND_INNER") = (int)LV_SCALE_MODE_ROUND_INNER;
    m.attr("ROUND_OUTER") = (int)LV_SCALE_MODE_ROUND_OUTER;
    m.attr("FIXED") = (int)LV_SPAN_MODE_FIXED;
    m.attr("BREAK") = (int)LV_SPAN_MODE_BREAK;
    m.attr("MERGE_RIGHT") = (int)LV_TABLE_CELL_CTRL_MERGE_RIGHT;
    m.attr("TEXT_CROP") = (int)LV_TABLE_CELL_CTRL_TEXT_CROP;
    m.attr("CUSTOM_3") = (int)LV_TABLE_CELL_CTRL_CUSTOM_3;
    m.attr("CUSTOM_4") = (int)LV_TABLE_CELL_CTRL_CUSTOM_4;

    /* Module-level functions */
    m.def("init", &lv_init, "Initialize LVGL library");
    m.def("deinit", &lv_deinit, "Deinitialize LVGL library");
    m.def("is_initialized", &lv_is_initialized, "Check if LVGL is initialized");
    m.def("timer_handler", &py_timer_handler, "Call LVGL timer handler");
    m.def("task_handler", &py_timer_handler, "Alias for timer_handler");
    m.def("tick_get", &lv_tick_get, "Get elapsed milliseconds");
    m.def("tick_inc", &lv_tick_inc, "Update tick value", py::arg("ms"));

    /* Version info */
    m.def("version_major", []() { return lv_version_major(); });
    m.def("version_minor", []() { return lv_version_minor(); });
    m.def("version_patch", []() { return lv_version_patch(); });
    m.def("version_info", []() { return std::string(lv_version_info()); });

    /* LVGL Demos (from lvgl_demos library) */
    m.def("demo_benchmark", []() { lv_demo_benchmark(); }, "Run LVGL benchmark demo");

    /* Base Object class */
    auto obj_cls = py::class_<LvObjWrapper>(m, "Obj", py::dynamic_attr());
    obj_cls.def(py::init<>());
    obj_cls.def("_keep_parent", [](LvObjWrapper &self, py::object parent) {
        self.keep_parent(parent);
    }, py::arg("parent"));
    obj_cls.def("set_flex_flow", [](LvObjWrapper &self, lv_flex_flow_t flow) { lv_obj_set_flex_flow(self.get(), flow); return; }
        , py::arg("flow"));
    obj_cls.def("set_flex_align", [](LvObjWrapper &self, lv_flex_align_t main_place, lv_flex_align_t cross_place, lv_flex_align_t track_cross_place) { lv_obj_set_flex_align(self.get(), main_place, cross_place, track_cross_place); return; }
        , py::arg("main_place"), py::arg("cross_place"), py::arg("track_cross_place"));
    obj_cls.def("set_flex_grow", [](LvObjWrapper &self, uint8_t grow) { lv_obj_set_flex_grow(self.get(), grow); return; }
        , py::arg("grow"));
    obj_cls.def("set_grid_align", [](LvObjWrapper &self, lv_grid_align_t column_align, lv_grid_align_t row_align) { lv_obj_set_grid_align(self.get(), column_align, row_align); return; }
        , py::arg("column_align"), py::arg("row_align"));
    obj_cls.def("set_grid_cell", [](LvObjWrapper &self, lv_grid_align_t column_align, int32_t col_pos, int32_t col_span, lv_grid_align_t row_align, int32_t row_pos, int32_t row_span) { lv_obj_set_grid_cell(self.get(), column_align, col_pos, col_span, row_align, row_pos, row_span); return; }
        , py::arg("column_align"), py::arg("col_pos"), py::arg("col_span"), py::arg("row_align"), py::arg("row_pos"), py::arg("row_span"));
    obj_cls.def("delete_obj", [](LvObjWrapper &self) { lv_obj_delete(self.get()); return; }
        );
    obj_cls.def("clean", [](LvObjWrapper &self) { lv_obj_clean(self.get()); return; }
        );
    obj_cls.def("delete_async", [](LvObjWrapper &self) { lv_obj_delete_async(self.get()); return; }
        );
    obj_cls.def("set_parent", [](LvObjWrapper &self, LvObjWrapper & parent) { lv_obj_set_parent(self.get(), parent.get()); return; }
        , py::arg("parent"));
    obj_cls.def("swap", [](LvObjWrapper &self, LvObjWrapper & obj2) { lv_obj_swap(self.get(), obj2.get()); return; }
        , py::arg("obj2"));
    obj_cls.def("move_to_index", [](LvObjWrapper &self, int32_t index) { lv_obj_move_to_index(self.get(), index); return; }
        , py::arg("index"));
    obj_cls.def("get_screen", [](LvObjWrapper &self) -> LvObjWrapper* {
        lv_obj_t *result = lv_obj_get_screen(self.get());
        return result ? new LvObjWrapper(result) : nullptr;
    }
        );
    obj_cls.def("get_parent", [](LvObjWrapper &self) -> LvObjWrapper* {
        lv_obj_t *result = lv_obj_get_parent(self.get());
        return result ? new LvObjWrapper(result) : nullptr;
    }
        );
    obj_cls.def("get_child", [](LvObjWrapper &self, int32_t idx) -> LvObjWrapper* {
        lv_obj_t *result = lv_obj_get_child(self.get(), idx);
        return result ? new LvObjWrapper(result) : nullptr;
    }
        , py::arg("idx"));
    obj_cls.def("get_sibling", [](LvObjWrapper &self, int32_t idx) -> LvObjWrapper* {
        lv_obj_t *result = lv_obj_get_sibling(self.get(), idx);
        return result ? new LvObjWrapper(result) : nullptr;
    }
        , py::arg("idx"));
    obj_cls.def("get_child_count", [](LvObjWrapper &self) -> uint32_t { return lv_obj_get_child_count(self.get()); }
        );
    obj_cls.def("set_name", [](LvObjWrapper &self, const char * name) { lv_obj_set_name(self.get(), name); return; }
        , py::arg("name"));
    obj_cls.def("set_name_static", [](LvObjWrapper &self, const char * name) { lv_obj_set_name_static(self.get(), name); return; }
        , py::arg("name"));
    obj_cls.def("get_name", [](LvObjWrapper &self) -> const char * { return lv_obj_get_name(self.get()); }
        );
    obj_cls.def("find_by_name", [](LvObjWrapper &self, const char * name) -> LvObjWrapper* {
        lv_obj_t *result = lv_obj_find_by_name(self.get(), name);
        return result ? new LvObjWrapper(result) : nullptr;
    }
        , py::arg("name"));
    obj_cls.def("get_index", [](LvObjWrapper &self) -> int32_t { return lv_obj_get_index(self.get()); }
        );
    obj_cls.def("tree_walk", [](LvObjWrapper &self, lv_obj_tree_walk_cb_t cb, void * user_data) { lv_obj_tree_walk(self.get(), cb, user_data); return; }
        , py::arg("cb"), py::arg("user_data"));
    obj_cls.def("dump_tree", [](LvObjWrapper &self) { lv_obj_dump_tree(self.get()); return; }
        );
    obj_cls.def("set_pos", [](LvObjWrapper &self, int32_t x, int32_t y) { lv_obj_set_pos(self.get(), x, y); return; }
        , py::arg("x"), py::arg("y"));
    obj_cls.def("set_x", [](LvObjWrapper &self, int32_t x) { lv_obj_set_x(self.get(), x); return; }
        , py::arg("x"));
    obj_cls.def("set_y", [](LvObjWrapper &self, int32_t y) { lv_obj_set_y(self.get(), y); return; }
        , py::arg("y"));
    obj_cls.def("set_size", [](LvObjWrapper &self, int32_t w, int32_t h) { lv_obj_set_size(self.get(), w, h); return; }
        , py::arg("w"), py::arg("h"));
    obj_cls.def("refr_size", [](LvObjWrapper &self) -> bool { return lv_obj_refr_size(self.get()); }
        );
    obj_cls.def("set_width", [](LvObjWrapper &self, int32_t w) { lv_obj_set_width(self.get(), w); return; }
        , py::arg("w"));
    obj_cls.def("set_height", [](LvObjWrapper &self, int32_t h) { lv_obj_set_height(self.get(), h); return; }
        , py::arg("h"));
    obj_cls.def("set_content_width", [](LvObjWrapper &self, int32_t w) { lv_obj_set_content_width(self.get(), w); return; }
        , py::arg("w"));
    obj_cls.def("set_content_height", [](LvObjWrapper &self, int32_t h) { lv_obj_set_content_height(self.get(), h); return; }
        , py::arg("h"));
    obj_cls.def("set_layout", [](LvObjWrapper &self, uint32_t layout) { lv_obj_set_layout(self.get(), layout); return; }
        , py::arg("layout"));
    obj_cls.def("is_layout_positioned", [](LvObjWrapper &self) -> bool { return lv_obj_is_layout_positioned(self.get()); }
        );
    obj_cls.def("mark_layout_as_dirty", [](LvObjWrapper &self) { lv_obj_mark_layout_as_dirty(self.get()); return; }
        );
    obj_cls.def("update_layout", [](LvObjWrapper &self) { lv_obj_update_layout(self.get()); return; }
        );
    obj_cls.def("set_align", [](LvObjWrapper &self, lv_align_t align) { lv_obj_set_align(self.get(), align); return; }
        , py::arg("align"));
    obj_cls.def("align", [](LvObjWrapper &self, lv_align_t align, int32_t x_ofs, int32_t y_ofs) { lv_obj_align(self.get(), align, x_ofs, y_ofs); return; }
        , py::arg("align"), py::arg("x_ofs"), py::arg("y_ofs"));
    obj_cls.def("align_to", [](LvObjWrapper &self, LvObjWrapper & base, lv_align_t align, int32_t x_ofs, int32_t y_ofs) { lv_obj_align_to(self.get(), base.get(), align, x_ofs, y_ofs); return; }
        , py::arg("base"), py::arg("align"), py::arg("x_ofs"), py::arg("y_ofs"));
    obj_cls.def("center", [](LvObjWrapper &self) { lv_obj_center(self.get()); return; }
        );
    obj_cls.def("set_transform", [](LvObjWrapper &self, const lv_matrix_t * matrix) { lv_obj_set_transform(self.get(), matrix); return; }
        , py::arg("matrix"));
    obj_cls.def("reset_transform", [](LvObjWrapper &self) { lv_obj_reset_transform(self.get()); return; }
        );
    obj_cls.def("get_coords", [](LvObjWrapper &self, lv_area_t * coords) { lv_obj_get_coords(self.get(), coords); return; }
        , py::arg("coords"));
    obj_cls.def("get_x", [](LvObjWrapper &self) -> int32_t { return lv_obj_get_x(self.get()); }
        );
    obj_cls.def("get_x2", [](LvObjWrapper &self) -> int32_t { return lv_obj_get_x2(self.get()); }
        );
    obj_cls.def("get_y", [](LvObjWrapper &self) -> int32_t { return lv_obj_get_y(self.get()); }
        );
    obj_cls.def("get_y2", [](LvObjWrapper &self) -> int32_t { return lv_obj_get_y2(self.get()); }
        );
    obj_cls.def("get_x_aligned", [](LvObjWrapper &self) -> int32_t { return lv_obj_get_x_aligned(self.get()); }
        );
    obj_cls.def("get_y_aligned", [](LvObjWrapper &self) -> int32_t { return lv_obj_get_y_aligned(self.get()); }
        );
    obj_cls.def("get_width", [](LvObjWrapper &self) -> int32_t { return lv_obj_get_width(self.get()); }
        );
    obj_cls.def("get_height", [](LvObjWrapper &self) -> int32_t { return lv_obj_get_height(self.get()); }
        );
    obj_cls.def("get_content_width", [](LvObjWrapper &self) -> int32_t { return lv_obj_get_content_width(self.get()); }
        );
    obj_cls.def("get_content_height", [](LvObjWrapper &self) -> int32_t { return lv_obj_get_content_height(self.get()); }
        );
    obj_cls.def("get_content_coords", [](LvObjWrapper &self, lv_area_t * area) { lv_obj_get_content_coords(self.get(), area); return; }
        , py::arg("area"));
    obj_cls.def("get_self_width", [](LvObjWrapper &self) -> int32_t { return lv_obj_get_self_width(self.get()); }
        );
    obj_cls.def("get_self_height", [](LvObjWrapper &self) -> int32_t { return lv_obj_get_self_height(self.get()); }
        );
    obj_cls.def("get_style_clamped_width", [](LvObjWrapper &self) -> int32_t { return lv_obj_get_style_clamped_width(self.get()); }
        );
    obj_cls.def("get_style_clamped_height", [](LvObjWrapper &self) -> int32_t { return lv_obj_get_style_clamped_height(self.get()); }
        );
    obj_cls.def("is_style_any_width_content", [](LvObjWrapper &self) -> bool { return lv_obj_is_style_any_width_content(self.get()); }
        );
    obj_cls.def("is_style_any_height_content", [](LvObjWrapper &self) -> bool { return lv_obj_is_style_any_height_content(self.get()); }
        );
    obj_cls.def("is_width_min", [](LvObjWrapper &self) -> bool { return lv_obj_is_width_min(self.get()); }
        );
    obj_cls.def("is_height_min", [](LvObjWrapper &self) -> bool { return lv_obj_is_height_min(self.get()); }
        );
    obj_cls.def("is_width_max", [](LvObjWrapper &self) -> bool { return lv_obj_is_width_max(self.get()); }
        );
    obj_cls.def("is_height_max", [](LvObjWrapper &self) -> bool { return lv_obj_is_height_max(self.get()); }
        );
    obj_cls.def("refresh_self_size", [](LvObjWrapper &self) -> bool { return lv_obj_refresh_self_size(self.get()); }
        );
    obj_cls.def("refr_pos", [](LvObjWrapper &self) { lv_obj_refr_pos(self.get()); return; }
        );
    obj_cls.def("move_to", [](LvObjWrapper &self, int32_t x, int32_t y) { lv_obj_move_to(self.get(), x, y); return; }
        , py::arg("x"), py::arg("y"));
    obj_cls.def("move_children_by", [](LvObjWrapper &self, int32_t x_diff, int32_t y_diff, bool ignore_floating) { lv_obj_move_children_by(self.get(), x_diff, y_diff, ignore_floating); return; }
        , py::arg("x_diff"), py::arg("y_diff"), py::arg("ignore_floating"));
    obj_cls.def("get_transform", [](LvObjWrapper &self) -> const lv_matrix_t * { return lv_obj_get_transform(self.get()); }
        );
    obj_cls.def("transform_point", [](LvObjWrapper &self, lv_point_t * p, lv_obj_point_transform_flag_t flags) { lv_obj_transform_point(self.get(), p, flags); return; }
        , py::arg("p"), py::arg("flags"));
    obj_cls.def("get_transformed_area", [](LvObjWrapper &self, lv_area_t * area, lv_obj_point_transform_flag_t flags) { lv_obj_get_transformed_area(self.get(), area, flags); return; }
        , py::arg("area"), py::arg("flags"));
    obj_cls.def("invalidate_area", [](LvObjWrapper &self, const lv_area_t * area) -> bool { return lv_obj_invalidate_area(self.get(), area); }
        , py::arg("area"));
    obj_cls.def("invalidate", [](LvObjWrapper &self) -> bool { return lv_obj_invalidate(self.get()); }
        );
    obj_cls.def("area_is_visible", [](LvObjWrapper &self, lv_area_t * area) -> bool { return lv_obj_area_is_visible(self.get(), area); }
        , py::arg("area"));
    obj_cls.def("is_visible", [](LvObjWrapper &self) -> bool { return lv_obj_is_visible(self.get()); }
        );
    obj_cls.def("set_ext_click_area", [](LvObjWrapper &self, int32_t size) { lv_obj_set_ext_click_area(self.get(), size); return; }
        , py::arg("size"));
    obj_cls.def("get_click_area", [](LvObjWrapper &self, lv_area_t * area) { lv_obj_get_click_area(self.get(), area); return; }
        , py::arg("area"));
    obj_cls.def("hit_test", [](LvObjWrapper &self, const lv_point_t * point) -> bool { return lv_obj_hit_test(self.get(), point); }
        , py::arg("point"));
    obj_cls.def("calc_dynamic_width", [](LvObjWrapper &self, uint32_t prop) -> int32_t { return lv_obj_calc_dynamic_width(self.get(), prop); }
        , py::arg("prop"));
    obj_cls.def("calc_dynamic_height", [](LvObjWrapper &self, uint32_t prop) -> int32_t { return lv_obj_calc_dynamic_height(self.get(), prop); }
        , py::arg("prop"));
    obj_cls.def("set_scrollbar_mode", [](LvObjWrapper &self, lv_scrollbar_mode_t mode) { lv_obj_set_scrollbar_mode(self.get(), mode); return; }
        , py::arg("mode"));
    obj_cls.def("set_scroll_dir", [](LvObjWrapper &self, lv_dir_t dir) { lv_obj_set_scroll_dir(self.get(), dir); return; }
        , py::arg("dir"));
    obj_cls.def("set_scroll_snap_x", [](LvObjWrapper &self, lv_scroll_snap_t align) { lv_obj_set_scroll_snap_x(self.get(), align); return; }
        , py::arg("align"));
    obj_cls.def("set_scroll_snap_y", [](LvObjWrapper &self, lv_scroll_snap_t align) { lv_obj_set_scroll_snap_y(self.get(), align); return; }
        , py::arg("align"));
    obj_cls.def("get_scrollbar_mode", [](LvObjWrapper &self) -> lv_scrollbar_mode_t { return lv_obj_get_scrollbar_mode(self.get()); }
        );
    obj_cls.def("get_scroll_dir", [](LvObjWrapper &self) -> lv_dir_t { return lv_obj_get_scroll_dir(self.get()); }
        );
    obj_cls.def("get_scroll_snap_x", [](LvObjWrapper &self) -> lv_scroll_snap_t { return lv_obj_get_scroll_snap_x(self.get()); }
        );
    obj_cls.def("get_scroll_snap_y", [](LvObjWrapper &self) -> lv_scroll_snap_t { return lv_obj_get_scroll_snap_y(self.get()); }
        );
    obj_cls.def("get_scroll_x", [](LvObjWrapper &self) -> int32_t { return lv_obj_get_scroll_x(self.get()); }
        );
    obj_cls.def("get_scroll_y", [](LvObjWrapper &self) -> int32_t { return lv_obj_get_scroll_y(self.get()); }
        );
    obj_cls.def("get_scroll_top", [](LvObjWrapper &self) -> int32_t { return lv_obj_get_scroll_top(self.get()); }
        );
    obj_cls.def("get_scroll_bottom", [](LvObjWrapper &self) -> int32_t { return lv_obj_get_scroll_bottom(self.get()); }
        );
    obj_cls.def("get_scroll_left", [](LvObjWrapper &self) -> int32_t { return lv_obj_get_scroll_left(self.get()); }
        );
    obj_cls.def("get_scroll_right", [](LvObjWrapper &self) -> int32_t { return lv_obj_get_scroll_right(self.get()); }
        );
    obj_cls.def("get_scroll_end", [](LvObjWrapper &self, lv_point_t * end) { lv_obj_get_scroll_end(self.get(), end); return; }
        , py::arg("end"));
    obj_cls.def("scroll_by", [](LvObjWrapper &self, int32_t dx, int32_t dy, bool anim_en) { lv_obj_scroll_by(self.get(), dx, dy, anim_en); return; }
        , py::arg("dx"), py::arg("dy"), py::arg("anim_en"));
    obj_cls.def("scroll_by_bounded", [](LvObjWrapper &self, int32_t dx, int32_t dy, bool anim_en) { lv_obj_scroll_by_bounded(self.get(), dx, dy, anim_en); return; }
        , py::arg("dx"), py::arg("dy"), py::arg("anim_en"));
    obj_cls.def("scroll_to", [](LvObjWrapper &self, int32_t x, int32_t y, bool anim_en) { lv_obj_scroll_to(self.get(), x, y, anim_en); return; }
        , py::arg("x"), py::arg("y"), py::arg("anim_en"));
    obj_cls.def("scroll_to_x", [](LvObjWrapper &self, int32_t x, bool anim_en) { lv_obj_scroll_to_x(self.get(), x, anim_en); return; }
        , py::arg("x"), py::arg("anim_en"));
    obj_cls.def("scroll_to_y", [](LvObjWrapper &self, int32_t y, bool anim_en) { lv_obj_scroll_to_y(self.get(), y, anim_en); return; }
        , py::arg("y"), py::arg("anim_en"));
    obj_cls.def("scroll_to_view", [](LvObjWrapper &self, bool anim_en) { lv_obj_scroll_to_view(self.get(), anim_en); return; }
        , py::arg("anim_en"));
    obj_cls.def("scroll_to_view_recursive", [](LvObjWrapper &self, bool anim_en) { lv_obj_scroll_to_view_recursive(self.get(), anim_en); return; }
        , py::arg("anim_en"));
    obj_cls.def("is_scrolling", [](LvObjWrapper &self) -> bool { return lv_obj_is_scrolling(self.get()); }
        );
    obj_cls.def("stop_scroll_anim", [](LvObjWrapper &self) { lv_obj_stop_scroll_anim(self.get()); return; }
        );
    obj_cls.def("update_snap", [](LvObjWrapper &self, bool anim_en) { lv_obj_update_snap(self.get(), anim_en); return; }
        , py::arg("anim_en"));
    obj_cls.def("get_scrollbar_area", [](LvObjWrapper &self, lv_area_t * hor, lv_area_t * ver) { lv_obj_get_scrollbar_area(self.get(), hor, ver); return; }
        , py::arg("hor"), py::arg("ver"));
    obj_cls.def("scrollbar_invalidate", [](LvObjWrapper &self) { lv_obj_scrollbar_invalidate(self.get()); return; }
        );
    obj_cls.def("readjust_scroll", [](LvObjWrapper &self, bool anim_en) { lv_obj_readjust_scroll(self.get(), anim_en); return; }
        , py::arg("anim_en"));
    obj_cls.def("remove_theme", [](LvObjWrapper &self, uint32_t selector) { lv_obj_remove_theme(self.get(), selector); return; }
        , py::arg("selector") = 0);
    obj_cls.def("remove_style_all", [](LvObjWrapper &self) { lv_obj_remove_style_all(self.get()); return; }
        );
    obj_cls.def("refresh_style", [](LvObjWrapper &self, lv_part_t part, uint32_t prop) { lv_obj_refresh_style(self.get(), part, prop); return; }
        , py::arg("part"), py::arg("prop"));
    obj_cls.def("get_style_prop", [](LvObjWrapper &self, lv_part_t part, uint32_t prop) -> lv_style_value_t { return lv_obj_get_style_prop(self.get(), part, prop); }
        , py::arg("part"), py::arg("prop"));
    obj_cls.def("has_style_prop", [](LvObjWrapper &self, uint32_t selector, uint32_t prop) -> bool { return lv_obj_has_style_prop(self.get(), selector, prop); }
        , py::arg("selector") = 0, py::arg("prop"));
    obj_cls.def("remove_local_style_prop", [](LvObjWrapper &self, uint32_t prop, uint32_t selector) -> bool { return lv_obj_remove_local_style_prop(self.get(), prop, selector); }
        , py::arg("prop"), py::arg("selector") = 0);
    obj_cls.def("fade_in", [](LvObjWrapper &self, uint32_t time, uint32_t delay) { lv_obj_fade_in(self.get(), time, delay); return; }
        , py::arg("time"), py::arg("delay"));
    obj_cls.def("fade_out", [](LvObjWrapper &self, uint32_t time, uint32_t delay) { lv_obj_fade_out(self.get(), time, delay); return; }
        , py::arg("time"), py::arg("delay"));
    obj_cls.def("get_style_width", [](LvObjWrapper &self, lv_part_t part) -> int32_t { return lv_obj_get_style_width(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_min_width", [](LvObjWrapper &self, lv_part_t part) -> int32_t { return lv_obj_get_style_min_width(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_max_width", [](LvObjWrapper &self, lv_part_t part) -> int32_t { return lv_obj_get_style_max_width(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_height", [](LvObjWrapper &self, lv_part_t part) -> int32_t { return lv_obj_get_style_height(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_min_height", [](LvObjWrapper &self, lv_part_t part) -> int32_t { return lv_obj_get_style_min_height(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_max_height", [](LvObjWrapper &self, lv_part_t part) -> int32_t { return lv_obj_get_style_max_height(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_length", [](LvObjWrapper &self, lv_part_t part) -> int32_t { return lv_obj_get_style_length(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_x", [](LvObjWrapper &self, lv_part_t part) -> int32_t { return lv_obj_get_style_x(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_y", [](LvObjWrapper &self, lv_part_t part) -> int32_t { return lv_obj_get_style_y(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_align", [](LvObjWrapper &self, lv_part_t part) -> lv_align_t { return lv_obj_get_style_align(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_transform_width", [](LvObjWrapper &self, lv_part_t part) -> int32_t { return lv_obj_get_style_transform_width(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_transform_height", [](LvObjWrapper &self, lv_part_t part) -> int32_t { return lv_obj_get_style_transform_height(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_translate_x", [](LvObjWrapper &self, lv_part_t part) -> int32_t { return lv_obj_get_style_translate_x(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_translate_y", [](LvObjWrapper &self, lv_part_t part) -> int32_t { return lv_obj_get_style_translate_y(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_translate_radial", [](LvObjWrapper &self, lv_part_t part) -> int32_t { return lv_obj_get_style_translate_radial(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_transform_scale_x", [](LvObjWrapper &self, lv_part_t part) -> int32_t { return lv_obj_get_style_transform_scale_x(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_transform_scale_y", [](LvObjWrapper &self, lv_part_t part) -> int32_t { return lv_obj_get_style_transform_scale_y(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_transform_rotation", [](LvObjWrapper &self, lv_part_t part) -> int32_t { return lv_obj_get_style_transform_rotation(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_transform_pivot_x", [](LvObjWrapper &self, lv_part_t part) -> int32_t { return lv_obj_get_style_transform_pivot_x(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_transform_pivot_y", [](LvObjWrapper &self, lv_part_t part) -> int32_t { return lv_obj_get_style_transform_pivot_y(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_transform_skew_x", [](LvObjWrapper &self, lv_part_t part) -> int32_t { return lv_obj_get_style_transform_skew_x(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_transform_skew_y", [](LvObjWrapper &self, lv_part_t part) -> int32_t { return lv_obj_get_style_transform_skew_y(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_pad_top", [](LvObjWrapper &self, lv_part_t part) -> int32_t { return lv_obj_get_style_pad_top(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_pad_bottom", [](LvObjWrapper &self, lv_part_t part) -> int32_t { return lv_obj_get_style_pad_bottom(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_pad_left", [](LvObjWrapper &self, lv_part_t part) -> int32_t { return lv_obj_get_style_pad_left(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_pad_right", [](LvObjWrapper &self, lv_part_t part) -> int32_t { return lv_obj_get_style_pad_right(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_pad_row", [](LvObjWrapper &self, lv_part_t part) -> int32_t { return lv_obj_get_style_pad_row(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_pad_column", [](LvObjWrapper &self, lv_part_t part) -> int32_t { return lv_obj_get_style_pad_column(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_pad_radial", [](LvObjWrapper &self, lv_part_t part) -> int32_t { return lv_obj_get_style_pad_radial(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_margin_top", [](LvObjWrapper &self, lv_part_t part) -> int32_t { return lv_obj_get_style_margin_top(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_margin_bottom", [](LvObjWrapper &self, lv_part_t part) -> int32_t { return lv_obj_get_style_margin_bottom(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_margin_left", [](LvObjWrapper &self, lv_part_t part) -> int32_t { return lv_obj_get_style_margin_left(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_margin_right", [](LvObjWrapper &self, lv_part_t part) -> int32_t { return lv_obj_get_style_margin_right(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_bg_color", [](LvObjWrapper &self, lv_part_t part) -> lv_color_t { return lv_obj_get_style_bg_color(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_bg_color_filtered", [](LvObjWrapper &self, lv_part_t part) -> lv_color_t { return lv_obj_get_style_bg_color_filtered(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_bg_opa", [](LvObjWrapper &self, lv_part_t part) -> uint8_t { return lv_obj_get_style_bg_opa(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_bg_grad_color", [](LvObjWrapper &self, lv_part_t part) -> lv_color_t { return lv_obj_get_style_bg_grad_color(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_bg_grad_color_filtered", [](LvObjWrapper &self, lv_part_t part) -> lv_color_t { return lv_obj_get_style_bg_grad_color_filtered(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_bg_grad_dir", [](LvObjWrapper &self, lv_part_t part) -> lv_grad_dir_t { return lv_obj_get_style_bg_grad_dir(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_bg_main_stop", [](LvObjWrapper &self, lv_part_t part) -> int32_t { return lv_obj_get_style_bg_main_stop(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_bg_grad_stop", [](LvObjWrapper &self, lv_part_t part) -> int32_t { return lv_obj_get_style_bg_grad_stop(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_bg_main_opa", [](LvObjWrapper &self, lv_part_t part) -> uint8_t { return lv_obj_get_style_bg_main_opa(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_bg_grad_opa", [](LvObjWrapper &self, lv_part_t part) -> uint8_t { return lv_obj_get_style_bg_grad_opa(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_bg_image_opa", [](LvObjWrapper &self, lv_part_t part) -> uint8_t { return lv_obj_get_style_bg_image_opa(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_bg_image_recolor", [](LvObjWrapper &self, lv_part_t part) -> lv_color_t { return lv_obj_get_style_bg_image_recolor(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_bg_image_recolor_filtered", [](LvObjWrapper &self, lv_part_t part) -> lv_color_t { return lv_obj_get_style_bg_image_recolor_filtered(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_bg_image_recolor_opa", [](LvObjWrapper &self, lv_part_t part) -> uint8_t { return lv_obj_get_style_bg_image_recolor_opa(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_bg_image_tiled", [](LvObjWrapper &self, lv_part_t part) -> bool { return lv_obj_get_style_bg_image_tiled(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_border_color", [](LvObjWrapper &self, lv_part_t part) -> lv_color_t { return lv_obj_get_style_border_color(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_border_color_filtered", [](LvObjWrapper &self, lv_part_t part) -> lv_color_t { return lv_obj_get_style_border_color_filtered(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_border_opa", [](LvObjWrapper &self, lv_part_t part) -> uint8_t { return lv_obj_get_style_border_opa(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_border_width", [](LvObjWrapper &self, lv_part_t part) -> int32_t { return lv_obj_get_style_border_width(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_border_side", [](LvObjWrapper &self, lv_part_t part) -> lv_border_side_t { return lv_obj_get_style_border_side(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_border_post", [](LvObjWrapper &self, lv_part_t part) -> bool { return lv_obj_get_style_border_post(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_outline_width", [](LvObjWrapper &self, lv_part_t part) -> int32_t { return lv_obj_get_style_outline_width(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_outline_color", [](LvObjWrapper &self, lv_part_t part) -> lv_color_t { return lv_obj_get_style_outline_color(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_outline_color_filtered", [](LvObjWrapper &self, lv_part_t part) -> lv_color_t { return lv_obj_get_style_outline_color_filtered(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_outline_opa", [](LvObjWrapper &self, lv_part_t part) -> uint8_t { return lv_obj_get_style_outline_opa(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_outline_pad", [](LvObjWrapper &self, lv_part_t part) -> int32_t { return lv_obj_get_style_outline_pad(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_shadow_width", [](LvObjWrapper &self, lv_part_t part) -> int32_t { return lv_obj_get_style_shadow_width(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_shadow_offset_x", [](LvObjWrapper &self, lv_part_t part) -> int32_t { return lv_obj_get_style_shadow_offset_x(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_shadow_offset_y", [](LvObjWrapper &self, lv_part_t part) -> int32_t { return lv_obj_get_style_shadow_offset_y(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_shadow_spread", [](LvObjWrapper &self, lv_part_t part) -> int32_t { return lv_obj_get_style_shadow_spread(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_shadow_color", [](LvObjWrapper &self, lv_part_t part) -> lv_color_t { return lv_obj_get_style_shadow_color(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_shadow_color_filtered", [](LvObjWrapper &self, lv_part_t part) -> lv_color_t { return lv_obj_get_style_shadow_color_filtered(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_shadow_opa", [](LvObjWrapper &self, lv_part_t part) -> uint8_t { return lv_obj_get_style_shadow_opa(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_image_opa", [](LvObjWrapper &self, lv_part_t part) -> uint8_t { return lv_obj_get_style_image_opa(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_image_recolor", [](LvObjWrapper &self, lv_part_t part) -> lv_color_t { return lv_obj_get_style_image_recolor(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_image_recolor_filtered", [](LvObjWrapper &self, lv_part_t part) -> lv_color_t { return lv_obj_get_style_image_recolor_filtered(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_image_recolor_opa", [](LvObjWrapper &self, lv_part_t part) -> uint8_t { return lv_obj_get_style_image_recolor_opa(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_line_width", [](LvObjWrapper &self, lv_part_t part) -> int32_t { return lv_obj_get_style_line_width(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_line_dash_width", [](LvObjWrapper &self, lv_part_t part) -> int32_t { return lv_obj_get_style_line_dash_width(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_line_dash_gap", [](LvObjWrapper &self, lv_part_t part) -> int32_t { return lv_obj_get_style_line_dash_gap(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_line_rounded", [](LvObjWrapper &self, lv_part_t part) -> bool { return lv_obj_get_style_line_rounded(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_line_color", [](LvObjWrapper &self, lv_part_t part) -> lv_color_t { return lv_obj_get_style_line_color(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_line_color_filtered", [](LvObjWrapper &self, lv_part_t part) -> lv_color_t { return lv_obj_get_style_line_color_filtered(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_line_opa", [](LvObjWrapper &self, lv_part_t part) -> uint8_t { return lv_obj_get_style_line_opa(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_arc_width", [](LvObjWrapper &self, lv_part_t part) -> int32_t { return lv_obj_get_style_arc_width(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_arc_rounded", [](LvObjWrapper &self, lv_part_t part) -> bool { return lv_obj_get_style_arc_rounded(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_arc_color", [](LvObjWrapper &self, lv_part_t part) -> lv_color_t { return lv_obj_get_style_arc_color(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_arc_color_filtered", [](LvObjWrapper &self, lv_part_t part) -> lv_color_t { return lv_obj_get_style_arc_color_filtered(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_arc_opa", [](LvObjWrapper &self, lv_part_t part) -> uint8_t { return lv_obj_get_style_arc_opa(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_text_color", [](LvObjWrapper &self, lv_part_t part) -> lv_color_t { return lv_obj_get_style_text_color(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_text_color_filtered", [](LvObjWrapper &self, lv_part_t part) -> lv_color_t { return lv_obj_get_style_text_color_filtered(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_text_opa", [](LvObjWrapper &self, lv_part_t part) -> uint8_t { return lv_obj_get_style_text_opa(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_text_font", [](LvObjWrapper &self, lv_part_t part) -> const lv_font_t * { return lv_obj_get_style_text_font(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_text_letter_space", [](LvObjWrapper &self, lv_part_t part) -> int32_t { return lv_obj_get_style_text_letter_space(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_text_line_space", [](LvObjWrapper &self, lv_part_t part) -> int32_t { return lv_obj_get_style_text_line_space(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_text_decor", [](LvObjWrapper &self, lv_part_t part) -> lv_text_decor_t { return lv_obj_get_style_text_decor(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_text_align", [](LvObjWrapper &self, lv_part_t part) -> lv_text_align_t { return lv_obj_get_style_text_align(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_text_outline_stroke_color", [](LvObjWrapper &self, lv_part_t part) -> lv_color_t { return lv_obj_get_style_text_outline_stroke_color(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_text_outline_stroke_color_filtered", [](LvObjWrapper &self, lv_part_t part) -> lv_color_t { return lv_obj_get_style_text_outline_stroke_color_filtered(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_text_outline_stroke_width", [](LvObjWrapper &self, lv_part_t part) -> int32_t { return lv_obj_get_style_text_outline_stroke_width(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_text_outline_stroke_opa", [](LvObjWrapper &self, lv_part_t part) -> uint8_t { return lv_obj_get_style_text_outline_stroke_opa(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_text_leading_trim", [](LvObjWrapper &self, lv_part_t part) -> lv_text_leading_trim_t { return lv_obj_get_style_text_leading_trim(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_blur_radius", [](LvObjWrapper &self, lv_part_t part) -> int32_t { return lv_obj_get_style_blur_radius(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_blur_backdrop", [](LvObjWrapper &self, lv_part_t part) -> bool { return lv_obj_get_style_blur_backdrop(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_blur_quality", [](LvObjWrapper &self, lv_part_t part) -> lv_blur_quality_t { return lv_obj_get_style_blur_quality(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_drop_shadow_radius", [](LvObjWrapper &self, lv_part_t part) -> int32_t { return lv_obj_get_style_drop_shadow_radius(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_drop_shadow_offset_x", [](LvObjWrapper &self, lv_part_t part) -> int32_t { return lv_obj_get_style_drop_shadow_offset_x(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_drop_shadow_offset_y", [](LvObjWrapper &self, lv_part_t part) -> int32_t { return lv_obj_get_style_drop_shadow_offset_y(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_drop_shadow_color", [](LvObjWrapper &self, lv_part_t part) -> lv_color_t { return lv_obj_get_style_drop_shadow_color(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_drop_shadow_color_filtered", [](LvObjWrapper &self, lv_part_t part) -> lv_color_t { return lv_obj_get_style_drop_shadow_color_filtered(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_drop_shadow_opa", [](LvObjWrapper &self, lv_part_t part) -> uint8_t { return lv_obj_get_style_drop_shadow_opa(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_drop_shadow_quality", [](LvObjWrapper &self, lv_part_t part) -> lv_blur_quality_t { return lv_obj_get_style_drop_shadow_quality(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_radius", [](LvObjWrapper &self, lv_part_t part) -> int32_t { return lv_obj_get_style_radius(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_radial_offset", [](LvObjWrapper &self, lv_part_t part) -> int32_t { return lv_obj_get_style_radial_offset(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_clip_corner", [](LvObjWrapper &self, lv_part_t part) -> bool { return lv_obj_get_style_clip_corner(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_opa", [](LvObjWrapper &self, lv_part_t part) -> uint8_t { return lv_obj_get_style_opa(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_opa_layered", [](LvObjWrapper &self, lv_part_t part) -> uint8_t { return lv_obj_get_style_opa_layered(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_color_filter_opa", [](LvObjWrapper &self, lv_part_t part) -> uint8_t { return lv_obj_get_style_color_filter_opa(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_recolor", [](LvObjWrapper &self, lv_part_t part) -> lv_color_t { return lv_obj_get_style_recolor(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_recolor_opa", [](LvObjWrapper &self, lv_part_t part) -> uint8_t { return lv_obj_get_style_recolor_opa(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_anim_duration", [](LvObjWrapper &self, lv_part_t part) -> uint32_t { return lv_obj_get_style_anim_duration(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_blend_mode", [](LvObjWrapper &self, lv_part_t part) -> lv_blend_mode_t { return lv_obj_get_style_blend_mode(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_layout", [](LvObjWrapper &self, lv_part_t part) -> uint16_t { return lv_obj_get_style_layout(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_base_dir", [](LvObjWrapper &self, lv_part_t part) -> lv_base_dir_t { return lv_obj_get_style_base_dir(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_rotary_sensitivity", [](LvObjWrapper &self, lv_part_t part) -> uint32_t { return lv_obj_get_style_rotary_sensitivity(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_flex_flow", [](LvObjWrapper &self, lv_part_t part) -> lv_flex_flow_t { return lv_obj_get_style_flex_flow(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_flex_main_place", [](LvObjWrapper &self, lv_part_t part) -> lv_flex_align_t { return lv_obj_get_style_flex_main_place(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_flex_cross_place", [](LvObjWrapper &self, lv_part_t part) -> lv_flex_align_t { return lv_obj_get_style_flex_cross_place(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_flex_track_place", [](LvObjWrapper &self, lv_part_t part) -> lv_flex_align_t { return lv_obj_get_style_flex_track_place(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_flex_grow", [](LvObjWrapper &self, lv_part_t part) -> uint8_t { return lv_obj_get_style_flex_grow(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_grid_column_dsc_array", [](LvObjWrapper &self, lv_part_t part) -> const int32_t * { return lv_obj_get_style_grid_column_dsc_array(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_grid_column_align", [](LvObjWrapper &self, lv_part_t part) -> lv_grid_align_t { return lv_obj_get_style_grid_column_align(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_grid_row_dsc_array", [](LvObjWrapper &self, lv_part_t part) -> const int32_t * { return lv_obj_get_style_grid_row_dsc_array(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_grid_row_align", [](LvObjWrapper &self, lv_part_t part) -> lv_grid_align_t { return lv_obj_get_style_grid_row_align(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_grid_cell_column_pos", [](LvObjWrapper &self, lv_part_t part) -> int32_t { return lv_obj_get_style_grid_cell_column_pos(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_grid_cell_x_align", [](LvObjWrapper &self, lv_part_t part) -> lv_grid_align_t { return lv_obj_get_style_grid_cell_x_align(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_grid_cell_column_span", [](LvObjWrapper &self, lv_part_t part) -> int32_t { return lv_obj_get_style_grid_cell_column_span(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_grid_cell_row_pos", [](LvObjWrapper &self, lv_part_t part) -> int32_t { return lv_obj_get_style_grid_cell_row_pos(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_grid_cell_y_align", [](LvObjWrapper &self, lv_part_t part) -> lv_grid_align_t { return lv_obj_get_style_grid_cell_y_align(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_grid_cell_row_span", [](LvObjWrapper &self, lv_part_t part) -> int32_t { return lv_obj_get_style_grid_cell_row_span(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("set_style_width", [](LvObjWrapper &self, int32_t value, uint32_t selector) { lv_obj_set_style_width(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_min_width", [](LvObjWrapper &self, int32_t value, uint32_t selector) { lv_obj_set_style_min_width(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_max_width", [](LvObjWrapper &self, int32_t value, uint32_t selector) { lv_obj_set_style_max_width(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_height", [](LvObjWrapper &self, int32_t value, uint32_t selector) { lv_obj_set_style_height(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_min_height", [](LvObjWrapper &self, int32_t value, uint32_t selector) { lv_obj_set_style_min_height(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_max_height", [](LvObjWrapper &self, int32_t value, uint32_t selector) { lv_obj_set_style_max_height(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_length", [](LvObjWrapper &self, int32_t value, uint32_t selector) { lv_obj_set_style_length(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_x", [](LvObjWrapper &self, int32_t value, uint32_t selector) { lv_obj_set_style_x(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_y", [](LvObjWrapper &self, int32_t value, uint32_t selector) { lv_obj_set_style_y(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_align", [](LvObjWrapper &self, lv_align_t value, uint32_t selector) { lv_obj_set_style_align(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_transform_width", [](LvObjWrapper &self, int32_t value, uint32_t selector) { lv_obj_set_style_transform_width(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_transform_height", [](LvObjWrapper &self, int32_t value, uint32_t selector) { lv_obj_set_style_transform_height(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_translate_x", [](LvObjWrapper &self, int32_t value, uint32_t selector) { lv_obj_set_style_translate_x(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_translate_y", [](LvObjWrapper &self, int32_t value, uint32_t selector) { lv_obj_set_style_translate_y(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_translate_radial", [](LvObjWrapper &self, int32_t value, uint32_t selector) { lv_obj_set_style_translate_radial(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_transform_scale_x", [](LvObjWrapper &self, int32_t value, uint32_t selector) { lv_obj_set_style_transform_scale_x(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_transform_scale_y", [](LvObjWrapper &self, int32_t value, uint32_t selector) { lv_obj_set_style_transform_scale_y(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_transform_rotation", [](LvObjWrapper &self, int32_t value, uint32_t selector) { lv_obj_set_style_transform_rotation(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_transform_pivot_x", [](LvObjWrapper &self, int32_t value, uint32_t selector) { lv_obj_set_style_transform_pivot_x(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_transform_pivot_y", [](LvObjWrapper &self, int32_t value, uint32_t selector) { lv_obj_set_style_transform_pivot_y(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_transform_skew_x", [](LvObjWrapper &self, int32_t value, uint32_t selector) { lv_obj_set_style_transform_skew_x(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_transform_skew_y", [](LvObjWrapper &self, int32_t value, uint32_t selector) { lv_obj_set_style_transform_skew_y(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_pad_top", [](LvObjWrapper &self, int32_t value, uint32_t selector) { lv_obj_set_style_pad_top(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_pad_bottom", [](LvObjWrapper &self, int32_t value, uint32_t selector) { lv_obj_set_style_pad_bottom(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_pad_left", [](LvObjWrapper &self, int32_t value, uint32_t selector) { lv_obj_set_style_pad_left(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_pad_right", [](LvObjWrapper &self, int32_t value, uint32_t selector) { lv_obj_set_style_pad_right(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_pad_row", [](LvObjWrapper &self, int32_t value, uint32_t selector) { lv_obj_set_style_pad_row(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_pad_column", [](LvObjWrapper &self, int32_t value, uint32_t selector) { lv_obj_set_style_pad_column(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_pad_radial", [](LvObjWrapper &self, int32_t value, uint32_t selector) { lv_obj_set_style_pad_radial(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_margin_top", [](LvObjWrapper &self, int32_t value, uint32_t selector) { lv_obj_set_style_margin_top(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_margin_bottom", [](LvObjWrapper &self, int32_t value, uint32_t selector) { lv_obj_set_style_margin_bottom(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_margin_left", [](LvObjWrapper &self, int32_t value, uint32_t selector) { lv_obj_set_style_margin_left(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_margin_right", [](LvObjWrapper &self, int32_t value, uint32_t selector) { lv_obj_set_style_margin_right(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_bg_color", [](LvObjWrapper &self, lv_color_t value, uint32_t selector) { lv_obj_set_style_bg_color(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_bg_opa", [](LvObjWrapper &self, uint8_t value, uint32_t selector) { lv_obj_set_style_bg_opa(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_bg_grad_color", [](LvObjWrapper &self, lv_color_t value, uint32_t selector) { lv_obj_set_style_bg_grad_color(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_bg_grad_dir", [](LvObjWrapper &self, lv_grad_dir_t value, uint32_t selector) { lv_obj_set_style_bg_grad_dir(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_bg_main_stop", [](LvObjWrapper &self, int32_t value, uint32_t selector) { lv_obj_set_style_bg_main_stop(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_bg_grad_stop", [](LvObjWrapper &self, int32_t value, uint32_t selector) { lv_obj_set_style_bg_grad_stop(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_bg_main_opa", [](LvObjWrapper &self, uint8_t value, uint32_t selector) { lv_obj_set_style_bg_main_opa(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_bg_grad_opa", [](LvObjWrapper &self, uint8_t value, uint32_t selector) { lv_obj_set_style_bg_grad_opa(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_bg_image_src", [](LvObjWrapper &self, const char * value, uint32_t selector) { lv_obj_set_style_bg_image_src(self.get(), (const void *)value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_bg_image_opa", [](LvObjWrapper &self, uint8_t value, uint32_t selector) { lv_obj_set_style_bg_image_opa(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_bg_image_recolor", [](LvObjWrapper &self, lv_color_t value, uint32_t selector) { lv_obj_set_style_bg_image_recolor(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_bg_image_recolor_opa", [](LvObjWrapper &self, uint8_t value, uint32_t selector) { lv_obj_set_style_bg_image_recolor_opa(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_bg_image_tiled", [](LvObjWrapper &self, bool value, uint32_t selector) { lv_obj_set_style_bg_image_tiled(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_border_color", [](LvObjWrapper &self, lv_color_t value, uint32_t selector) { lv_obj_set_style_border_color(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_border_opa", [](LvObjWrapper &self, uint8_t value, uint32_t selector) { lv_obj_set_style_border_opa(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_border_width", [](LvObjWrapper &self, int32_t value, uint32_t selector) { lv_obj_set_style_border_width(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_border_side", [](LvObjWrapper &self, lv_border_side_t value, uint32_t selector) { lv_obj_set_style_border_side(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_border_post", [](LvObjWrapper &self, bool value, uint32_t selector) { lv_obj_set_style_border_post(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_outline_width", [](LvObjWrapper &self, int32_t value, uint32_t selector) { lv_obj_set_style_outline_width(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_outline_color", [](LvObjWrapper &self, lv_color_t value, uint32_t selector) { lv_obj_set_style_outline_color(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_outline_opa", [](LvObjWrapper &self, uint8_t value, uint32_t selector) { lv_obj_set_style_outline_opa(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_outline_pad", [](LvObjWrapper &self, int32_t value, uint32_t selector) { lv_obj_set_style_outline_pad(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_shadow_width", [](LvObjWrapper &self, int32_t value, uint32_t selector) { lv_obj_set_style_shadow_width(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_shadow_offset_x", [](LvObjWrapper &self, int32_t value, uint32_t selector) { lv_obj_set_style_shadow_offset_x(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_shadow_offset_y", [](LvObjWrapper &self, int32_t value, uint32_t selector) { lv_obj_set_style_shadow_offset_y(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_shadow_spread", [](LvObjWrapper &self, int32_t value, uint32_t selector) { lv_obj_set_style_shadow_spread(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_shadow_color", [](LvObjWrapper &self, lv_color_t value, uint32_t selector) { lv_obj_set_style_shadow_color(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_shadow_opa", [](LvObjWrapper &self, uint8_t value, uint32_t selector) { lv_obj_set_style_shadow_opa(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_image_opa", [](LvObjWrapper &self, uint8_t value, uint32_t selector) { lv_obj_set_style_image_opa(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_image_recolor", [](LvObjWrapper &self, lv_color_t value, uint32_t selector) { lv_obj_set_style_image_recolor(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_image_recolor_opa", [](LvObjWrapper &self, uint8_t value, uint32_t selector) { lv_obj_set_style_image_recolor_opa(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_line_width", [](LvObjWrapper &self, int32_t value, uint32_t selector) { lv_obj_set_style_line_width(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_line_dash_width", [](LvObjWrapper &self, int32_t value, uint32_t selector) { lv_obj_set_style_line_dash_width(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_line_dash_gap", [](LvObjWrapper &self, int32_t value, uint32_t selector) { lv_obj_set_style_line_dash_gap(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_line_rounded", [](LvObjWrapper &self, bool value, uint32_t selector) { lv_obj_set_style_line_rounded(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_line_color", [](LvObjWrapper &self, lv_color_t value, uint32_t selector) { lv_obj_set_style_line_color(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_line_opa", [](LvObjWrapper &self, uint8_t value, uint32_t selector) { lv_obj_set_style_line_opa(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_arc_width", [](LvObjWrapper &self, int32_t value, uint32_t selector) { lv_obj_set_style_arc_width(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_arc_rounded", [](LvObjWrapper &self, bool value, uint32_t selector) { lv_obj_set_style_arc_rounded(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_arc_color", [](LvObjWrapper &self, lv_color_t value, uint32_t selector) { lv_obj_set_style_arc_color(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_arc_opa", [](LvObjWrapper &self, uint8_t value, uint32_t selector) { lv_obj_set_style_arc_opa(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_arc_image_src", [](LvObjWrapper &self, const char * value, uint32_t selector) { lv_obj_set_style_arc_image_src(self.get(), (const void *)value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_text_color", [](LvObjWrapper &self, lv_color_t value, uint32_t selector) { lv_obj_set_style_text_color(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_text_opa", [](LvObjWrapper &self, uint8_t value, uint32_t selector) { lv_obj_set_style_text_opa(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_text_font", [](LvObjWrapper &self, const lv_font_t * value, uint32_t selector) { lv_obj_set_style_text_font(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_text_letter_space", [](LvObjWrapper &self, int32_t value, uint32_t selector) { lv_obj_set_style_text_letter_space(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_text_line_space", [](LvObjWrapper &self, int32_t value, uint32_t selector) { lv_obj_set_style_text_line_space(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_text_decor", [](LvObjWrapper &self, lv_text_decor_t value, uint32_t selector) { lv_obj_set_style_text_decor(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_text_align", [](LvObjWrapper &self, lv_text_align_t value, uint32_t selector) { lv_obj_set_style_text_align(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_text_outline_stroke_color", [](LvObjWrapper &self, lv_color_t value, uint32_t selector) { lv_obj_set_style_text_outline_stroke_color(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_text_outline_stroke_width", [](LvObjWrapper &self, int32_t value, uint32_t selector) { lv_obj_set_style_text_outline_stroke_width(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_text_outline_stroke_opa", [](LvObjWrapper &self, uint8_t value, uint32_t selector) { lv_obj_set_style_text_outline_stroke_opa(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_text_leading_trim", [](LvObjWrapper &self, lv_text_leading_trim_t value, uint32_t selector) { lv_obj_set_style_text_leading_trim(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_blur_radius", [](LvObjWrapper &self, int32_t value, uint32_t selector) { lv_obj_set_style_blur_radius(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_blur_backdrop", [](LvObjWrapper &self, bool value, uint32_t selector) { lv_obj_set_style_blur_backdrop(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_blur_quality", [](LvObjWrapper &self, lv_blur_quality_t value, uint32_t selector) { lv_obj_set_style_blur_quality(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_drop_shadow_radius", [](LvObjWrapper &self, int32_t value, uint32_t selector) { lv_obj_set_style_drop_shadow_radius(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_drop_shadow_offset_x", [](LvObjWrapper &self, int32_t value, uint32_t selector) { lv_obj_set_style_drop_shadow_offset_x(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_drop_shadow_offset_y", [](LvObjWrapper &self, int32_t value, uint32_t selector) { lv_obj_set_style_drop_shadow_offset_y(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_drop_shadow_color", [](LvObjWrapper &self, lv_color_t value, uint32_t selector) { lv_obj_set_style_drop_shadow_color(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_drop_shadow_opa", [](LvObjWrapper &self, uint8_t value, uint32_t selector) { lv_obj_set_style_drop_shadow_opa(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_drop_shadow_quality", [](LvObjWrapper &self, lv_blur_quality_t value, uint32_t selector) { lv_obj_set_style_drop_shadow_quality(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_radius", [](LvObjWrapper &self, int32_t value, uint32_t selector) { lv_obj_set_style_radius(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_radial_offset", [](LvObjWrapper &self, int32_t value, uint32_t selector) { lv_obj_set_style_radial_offset(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_clip_corner", [](LvObjWrapper &self, bool value, uint32_t selector) { lv_obj_set_style_clip_corner(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_opa", [](LvObjWrapper &self, uint8_t value, uint32_t selector) { lv_obj_set_style_opa(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_opa_layered", [](LvObjWrapper &self, uint8_t value, uint32_t selector) { lv_obj_set_style_opa_layered(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_color_filter_opa", [](LvObjWrapper &self, uint8_t value, uint32_t selector) { lv_obj_set_style_color_filter_opa(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_recolor", [](LvObjWrapper &self, lv_color_t value, uint32_t selector) { lv_obj_set_style_recolor(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_recolor_opa", [](LvObjWrapper &self, uint8_t value, uint32_t selector) { lv_obj_set_style_recolor_opa(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_anim_duration", [](LvObjWrapper &self, uint32_t value, uint32_t selector) { lv_obj_set_style_anim_duration(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_blend_mode", [](LvObjWrapper &self, lv_blend_mode_t value, uint32_t selector) { lv_obj_set_style_blend_mode(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_layout", [](LvObjWrapper &self, uint16_t value, uint32_t selector) { lv_obj_set_style_layout(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_base_dir", [](LvObjWrapper &self, lv_base_dir_t value, uint32_t selector) { lv_obj_set_style_base_dir(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_bitmap_mask_src", [](LvObjWrapper &self, const char * value, uint32_t selector) { lv_obj_set_style_bitmap_mask_src(self.get(), (const void *)value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_rotary_sensitivity", [](LvObjWrapper &self, uint32_t value, uint32_t selector) { lv_obj_set_style_rotary_sensitivity(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_flex_flow", [](LvObjWrapper &self, lv_flex_flow_t value, uint32_t selector) { lv_obj_set_style_flex_flow(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_flex_main_place", [](LvObjWrapper &self, lv_flex_align_t value, uint32_t selector) { lv_obj_set_style_flex_main_place(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_flex_cross_place", [](LvObjWrapper &self, lv_flex_align_t value, uint32_t selector) { lv_obj_set_style_flex_cross_place(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_flex_track_place", [](LvObjWrapper &self, lv_flex_align_t value, uint32_t selector) { lv_obj_set_style_flex_track_place(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_flex_grow", [](LvObjWrapper &self, uint8_t value, uint32_t selector) { lv_obj_set_style_flex_grow(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_grid_column_dsc_array", [](LvObjWrapper &self, const int32_t * value, uint32_t selector) { lv_obj_set_style_grid_column_dsc_array(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_grid_column_align", [](LvObjWrapper &self, lv_grid_align_t value, uint32_t selector) { lv_obj_set_style_grid_column_align(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_grid_row_dsc_array", [](LvObjWrapper &self, const int32_t * value, uint32_t selector) { lv_obj_set_style_grid_row_dsc_array(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_grid_row_align", [](LvObjWrapper &self, lv_grid_align_t value, uint32_t selector) { lv_obj_set_style_grid_row_align(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_grid_cell_column_pos", [](LvObjWrapper &self, int32_t value, uint32_t selector) { lv_obj_set_style_grid_cell_column_pos(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_grid_cell_x_align", [](LvObjWrapper &self, lv_grid_align_t value, uint32_t selector) { lv_obj_set_style_grid_cell_x_align(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_grid_cell_column_span", [](LvObjWrapper &self, int32_t value, uint32_t selector) { lv_obj_set_style_grid_cell_column_span(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_grid_cell_row_pos", [](LvObjWrapper &self, int32_t value, uint32_t selector) { lv_obj_set_style_grid_cell_row_pos(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_grid_cell_y_align", [](LvObjWrapper &self, lv_grid_align_t value, uint32_t selector) { lv_obj_set_style_grid_cell_y_align(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_grid_cell_row_span", [](LvObjWrapper &self, int32_t value, uint32_t selector) { lv_obj_set_style_grid_cell_row_span(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_pad_all", [](LvObjWrapper &self, int32_t value, uint32_t selector) { lv_obj_set_style_pad_all(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_pad_hor", [](LvObjWrapper &self, int32_t value, uint32_t selector) { lv_obj_set_style_pad_hor(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_pad_ver", [](LvObjWrapper &self, int32_t value, uint32_t selector) { lv_obj_set_style_pad_ver(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_margin_all", [](LvObjWrapper &self, int32_t value, uint32_t selector) { lv_obj_set_style_margin_all(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_margin_hor", [](LvObjWrapper &self, int32_t value, uint32_t selector) { lv_obj_set_style_margin_hor(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_margin_ver", [](LvObjWrapper &self, int32_t value, uint32_t selector) { lv_obj_set_style_margin_ver(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_pad_gap", [](LvObjWrapper &self, int32_t value, uint32_t selector) { lv_obj_set_style_pad_gap(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("set_style_size", [](LvObjWrapper &self, int32_t width, int32_t height, uint32_t selector) { lv_obj_set_style_size(self.get(), width, height, selector); return; }
        , py::arg("width"), py::arg("height"), py::arg("selector") = 0);
    obj_cls.def("set_style_transform_scale", [](LvObjWrapper &self, int32_t value, uint32_t selector) { lv_obj_set_style_transform_scale(self.get(), value, selector); return; }
        , py::arg("value"), py::arg("selector") = 0);
    obj_cls.def("get_style_space_left", [](LvObjWrapper &self, lv_part_t part) -> int32_t { return lv_obj_get_style_space_left(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_space_right", [](LvObjWrapper &self, lv_part_t part) -> int32_t { return lv_obj_get_style_space_right(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_space_top", [](LvObjWrapper &self, lv_part_t part) -> int32_t { return lv_obj_get_style_space_top(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_space_bottom", [](LvObjWrapper &self, lv_part_t part) -> int32_t { return lv_obj_get_style_space_bottom(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("calculate_style_text_align", [](LvObjWrapper &self, lv_part_t part, const char * txt) -> lv_text_align_t { return lv_obj_calculate_style_text_align(self.get(), part, txt); }
        , py::arg("part"), py::arg("txt"));
    obj_cls.def("get_style_transform_scale_x_safe", [](LvObjWrapper &self, lv_part_t part) -> int32_t { return lv_obj_get_style_transform_scale_x_safe(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_transform_scale_y_safe", [](LvObjWrapper &self, lv_part_t part) -> int32_t { return lv_obj_get_style_transform_scale_y_safe(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("get_style_opa_recursive", [](LvObjWrapper &self, lv_part_t part) -> uint8_t { return lv_obj_get_style_opa_recursive(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("style_apply_recolor", [](LvObjWrapper &self, lv_part_t part, lv_color32_t color) -> lv_color32_t { return lv_obj_style_apply_recolor(self.get(), part, color); }
        , py::arg("part"), py::arg("color"));
    obj_cls.def("get_style_recolor_recursive", [](LvObjWrapper &self, lv_part_t part) -> lv_color32_t { return lv_obj_get_style_recolor_recursive(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("set_ext_draw_size", [](LvObjWrapper &self, int32_t size) { lv_obj_set_ext_draw_size(self.get(), size); return; }
        , py::arg("size"));
    obj_cls.def("calculate_ext_draw_size", [](LvObjWrapper &self, lv_part_t part) -> int32_t { return lv_obj_calculate_ext_draw_size(self.get(), part); }
        , py::arg("part"));
    obj_cls.def("refresh_ext_draw_size", [](LvObjWrapper &self) { lv_obj_refresh_ext_draw_size(self.get()); return; }
        );
    obj_cls.def("class_init_obj", [](LvObjWrapper &self) { lv_obj_class_init_obj(self.get()); return; }
        );
    obj_cls.def("is_editable", [](LvObjWrapper &self) -> bool { return lv_obj_is_editable(self.get()); }
        );
    obj_cls.def("is_group_def", [](LvObjWrapper &self) -> bool { return lv_obj_is_group_def(self.get()); }
        );
    obj_cls.def("send_event", [](LvObjWrapper &self, lv_event_code_t event_code, void * param) -> bool { return lv_obj_send_event(self.get(), event_code, param); }
        , py::arg("event_code"), py::arg("param"));
    obj_cls.def("get_event_count", [](LvObjWrapper &self) -> uint32_t { return lv_obj_get_event_count(self.get()); }
        );
    obj_cls.def("remove_event", [](LvObjWrapper &self, uint32_t index) -> bool { return lv_obj_remove_event(self.get(), index); }
        , py::arg("index"));
    obj_cls.def("remove_event_cb_with_user_data", [](LvObjWrapper &self, lv_event_cb_t event_cb, void * user_data) -> uint32_t { return lv_obj_remove_event_cb_with_user_data(self.get(), event_cb, user_data); }
        , py::arg("event_cb"), py::arg("user_data"));
    obj_cls.def("obj", [](LvObjWrapper &self) -> LvObjWrapper* {
        lv_obj_t *result = lv_obj_create(self.get());
        return result ? new LvObjWrapper(result) : nullptr;
    }
        );
    obj_cls.def("add_flag", [](LvObjWrapper &self, lv_obj_flag_t f) { lv_obj_add_flag(self.get(), f); return; }
        , py::arg("f"));
    obj_cls.def("remove_flag", [](LvObjWrapper &self, lv_obj_flag_t f) { lv_obj_remove_flag(self.get(), f); return; }
        , py::arg("f"));
    obj_cls.def("set_flag", [](LvObjWrapper &self, lv_obj_flag_t f, bool v) { lv_obj_set_flag(self.get(), f, v); return; }
        , py::arg("f"), py::arg("v"));
    obj_cls.def("add_state", [](LvObjWrapper &self, lv_state_t state) { lv_obj_add_state(self.get(), state); return; }
        , py::arg("state"));
    obj_cls.def("remove_state", [](LvObjWrapper &self, lv_state_t state) { lv_obj_remove_state(self.get(), state); return; }
        , py::arg("state"));
    obj_cls.def("set_state", [](LvObjWrapper &self, lv_state_t state, bool v) { lv_obj_set_state(self.get(), state, v); return; }
        , py::arg("state"), py::arg("v"));
    obj_cls.def("set_user_data", [](LvObjWrapper &self, void * user_data) { lv_obj_set_user_data(self.get(), user_data); return; }
        , py::arg("user_data"));
    obj_cls.def("set_radio_button", [](LvObjWrapper &self, bool en) { lv_obj_set_radio_button(self.get(), en); return; }
        , py::arg("en"));
    obj_cls.def("has_flag", [](LvObjWrapper &self, lv_obj_flag_t f) -> bool { return lv_obj_has_flag(self.get(), f); }
        , py::arg("f"));
    obj_cls.def("has_flag_any", [](LvObjWrapper &self, lv_obj_flag_t f) -> bool { return lv_obj_has_flag_any(self.get(), f); }
        , py::arg("f"));
    obj_cls.def("get_state", [](LvObjWrapper &self) -> lv_state_t { return lv_obj_get_state(self.get()); }
        );
    obj_cls.def("has_state", [](LvObjWrapper &self, lv_state_t state) -> bool { return lv_obj_has_state(self.get(), state); }
        , py::arg("state"));
    obj_cls.def("is_radio_button", [](LvObjWrapper &self) -> bool { return lv_obj_is_radio_button(self.get()); }
        );
    obj_cls.def("get_user_data", [](LvObjWrapper &self) -> void * { return lv_obj_get_user_data(self.get()); }
        );
    obj_cls.def("is_valid", [](LvObjWrapper &self) -> bool { return lv_obj_is_valid(self.get()); }
        );
    obj_cls.def("add_screen_load_event", [](LvObjWrapper &self, lv_event_code_t trigger, LvObjWrapper & screen, lv_screen_load_anim_t anim_type, uint32_t duration, uint32_t delay) { lv_obj_add_screen_load_event(self.get(), trigger, screen.get(), anim_type, duration, delay); return; }
        , py::arg("trigger"), py::arg("screen"), py::arg("anim_type"), py::arg("duration"), py::arg("delay"));
    obj_cls.def("add_screen_create_event", [](LvObjWrapper &self, lv_event_code_t trigger, lv_screen_create_cb_t screen_create_cb, lv_screen_load_anim_t anim_type, uint32_t duration, uint32_t delay) { lv_obj_add_screen_create_event(self.get(), trigger, screen_create_cb, anim_type, duration, delay); return; }
        , py::arg("trigger"), py::arg("screen_create_cb"), py::arg("anim_type"), py::arg("duration"), py::arg("delay"));
    obj_cls.def("move_foreground", [](LvObjWrapper &self) { lv_obj_move_foreground(self.get()); return; }
        );
    obj_cls.def("move_background", [](LvObjWrapper &self) { lv_obj_move_background(self.get()); return; }
        );

    /* Screen management */
    m.def("screen_active", []() -> LvObjWrapper* {
        return new LvObjWrapper(lv_screen_active(), false);
    });
    m.def("screen_load", [](LvObjWrapper &screen) {
        lv_screen_load(screen.get());
    }, py::arg("screen"));

    /* Widget factory functions */
    m.def("Animimg", [](py::object parent_obj) -> LvObjWrapper* {
        LvObjWrapper *_parent = parent_obj.is_none() ? nullptr : parent_obj.cast<LvObjWrapper*>();
        LvObjWrapper *wrapper = new LvObjWrapper(lv_animimg_create(_parent ? _parent->get() : lv_screen_active()));
        if (_parent) wrapper->keep_parent(parent_obj);
        return wrapper;
    }, py::arg("parent") = py::none());
    m.def("Arc", [](py::object parent_obj) -> LvObjWrapper* {
        LvObjWrapper *_parent = parent_obj.is_none() ? nullptr : parent_obj.cast<LvObjWrapper*>();
        LvObjWrapper *wrapper = new LvObjWrapper(lv_arc_create(_parent ? _parent->get() : lv_screen_active()));
        if (_parent) wrapper->keep_parent(parent_obj);
        return wrapper;
    }, py::arg("parent") = py::none());
    m.def("Bar", [](py::object parent_obj) -> LvObjWrapper* {
        LvObjWrapper *_parent = parent_obj.is_none() ? nullptr : parent_obj.cast<LvObjWrapper*>();
        LvObjWrapper *wrapper = new LvObjWrapper(lv_bar_create(_parent ? _parent->get() : lv_screen_active()));
        if (_parent) wrapper->keep_parent(parent_obj);
        return wrapper;
    }, py::arg("parent") = py::none());
    m.def("Button", [](py::object parent_obj) -> LvObjWrapper* {
        LvObjWrapper *_parent = parent_obj.is_none() ? nullptr : parent_obj.cast<LvObjWrapper*>();
        LvObjWrapper *wrapper = new LvObjWrapper(lv_button_create(_parent ? _parent->get() : lv_screen_active()));
        if (_parent) wrapper->keep_parent(parent_obj);
        return wrapper;
    }, py::arg("parent") = py::none());
    m.def("Canvas", [](py::object parent_obj) -> LvObjWrapper* {
        LvObjWrapper *_parent = parent_obj.is_none() ? nullptr : parent_obj.cast<LvObjWrapper*>();
        LvObjWrapper *wrapper = new LvObjWrapper(lv_canvas_create(_parent ? _parent->get() : lv_screen_active()));
        if (_parent) wrapper->keep_parent(parent_obj);
        return wrapper;
    }, py::arg("parent") = py::none());
    m.def("Chart", [](py::object parent_obj) -> LvObjWrapper* {
        LvObjWrapper *_parent = parent_obj.is_none() ? nullptr : parent_obj.cast<LvObjWrapper*>();
        LvObjWrapper *wrapper = new LvObjWrapper(lv_chart_create(_parent ? _parent->get() : lv_screen_active()));
        if (_parent) wrapper->keep_parent(parent_obj);
        return wrapper;
    }, py::arg("parent") = py::none());
    m.def("Checkbox", [](py::object parent_obj) -> LvObjWrapper* {
        LvObjWrapper *_parent = parent_obj.is_none() ? nullptr : parent_obj.cast<LvObjWrapper*>();
        LvObjWrapper *wrapper = new LvObjWrapper(lv_checkbox_create(_parent ? _parent->get() : lv_screen_active()));
        if (_parent) wrapper->keep_parent(parent_obj);
        return wrapper;
    }, py::arg("parent") = py::none());
    m.def("Dropdown", [](py::object parent_obj) -> LvObjWrapper* {
        LvObjWrapper *_parent = parent_obj.is_none() ? nullptr : parent_obj.cast<LvObjWrapper*>();
        LvObjWrapper *wrapper = new LvObjWrapper(lv_dropdown_create(_parent ? _parent->get() : lv_screen_active()));
        if (_parent) wrapper->keep_parent(parent_obj);
        return wrapper;
    }, py::arg("parent") = py::none());
    m.def("Image", [](py::object parent_obj) -> LvObjWrapper* {
        LvObjWrapper *_parent = parent_obj.is_none() ? nullptr : parent_obj.cast<LvObjWrapper*>();
        LvObjWrapper *wrapper = new LvObjWrapper(lv_image_create(_parent ? _parent->get() : lv_screen_active()));
        if (_parent) wrapper->keep_parent(parent_obj);
        return wrapper;
    }, py::arg("parent") = py::none());
    m.def("Keyboard", [](py::object parent_obj) -> LvObjWrapper* {
        LvObjWrapper *_parent = parent_obj.is_none() ? nullptr : parent_obj.cast<LvObjWrapper*>();
        LvObjWrapper *wrapper = new LvObjWrapper(lv_keyboard_create(_parent ? _parent->get() : lv_screen_active()));
        if (_parent) wrapper->keep_parent(parent_obj);
        return wrapper;
    }, py::arg("parent") = py::none());
    m.def("Label", [](py::object parent_obj) -> LvObjWrapper* {
        LvObjWrapper *_parent = parent_obj.is_none() ? nullptr : parent_obj.cast<LvObjWrapper*>();
        LvObjWrapper *wrapper = new LvObjWrapper(lv_label_create(_parent ? _parent->get() : lv_screen_active()));
        if (_parent) wrapper->keep_parent(parent_obj);
        return wrapper;
    }, py::arg("parent") = py::none());
    m.def("Led", [](py::object parent_obj) -> LvObjWrapper* {
        LvObjWrapper *_parent = parent_obj.is_none() ? nullptr : parent_obj.cast<LvObjWrapper*>();
        LvObjWrapper *wrapper = new LvObjWrapper(lv_led_create(_parent ? _parent->get() : lv_screen_active()));
        if (_parent) wrapper->keep_parent(parent_obj);
        return wrapper;
    }, py::arg("parent") = py::none());
    m.def("Line", [](py::object parent_obj) -> LvObjWrapper* {
        LvObjWrapper *_parent = parent_obj.is_none() ? nullptr : parent_obj.cast<LvObjWrapper*>();
        LvObjWrapper *wrapper = new LvObjWrapper(lv_line_create(_parent ? _parent->get() : lv_screen_active()));
        if (_parent) wrapper->keep_parent(parent_obj);
        return wrapper;
    }, py::arg("parent") = py::none());
    m.def("List", [](py::object parent_obj) -> LvObjWrapper* {
        LvObjWrapper *_parent = parent_obj.is_none() ? nullptr : parent_obj.cast<LvObjWrapper*>();
        LvObjWrapper *wrapper = new LvObjWrapper(lv_list_create(_parent ? _parent->get() : lv_screen_active()));
        if (_parent) wrapper->keep_parent(parent_obj);
        return wrapper;
    }, py::arg("parent") = py::none());
    m.def("Menu", [](py::object parent_obj) -> LvObjWrapper* {
        LvObjWrapper *_parent = parent_obj.is_none() ? nullptr : parent_obj.cast<LvObjWrapper*>();
        LvObjWrapper *wrapper = new LvObjWrapper(lv_menu_create(_parent ? _parent->get() : lv_screen_active()));
        if (_parent) wrapper->keep_parent(parent_obj);
        return wrapper;
    }, py::arg("parent") = py::none());
    m.def("Msgbox", [](py::object parent_obj) -> LvObjWrapper* {
        LvObjWrapper *_parent = parent_obj.is_none() ? nullptr : parent_obj.cast<LvObjWrapper*>();
        LvObjWrapper *wrapper = new LvObjWrapper(lv_msgbox_create(_parent ? _parent->get() : NULL));
        if (_parent) wrapper->keep_parent(parent_obj);
        return wrapper;
    }, py::arg("parent") = py::none());
    m.def("Roller", [](py::object parent_obj) -> LvObjWrapper* {
        LvObjWrapper *_parent = parent_obj.is_none() ? nullptr : parent_obj.cast<LvObjWrapper*>();
        LvObjWrapper *wrapper = new LvObjWrapper(lv_roller_create(_parent ? _parent->get() : lv_screen_active()));
        if (_parent) wrapper->keep_parent(parent_obj);
        return wrapper;
    }, py::arg("parent") = py::none());
    m.def("Scale", [](py::object parent_obj) -> LvObjWrapper* {
        LvObjWrapper *_parent = parent_obj.is_none() ? nullptr : parent_obj.cast<LvObjWrapper*>();
        LvObjWrapper *wrapper = new LvObjWrapper(lv_scale_create(_parent ? _parent->get() : lv_screen_active()));
        if (_parent) wrapper->keep_parent(parent_obj);
        return wrapper;
    }, py::arg("parent") = py::none());
    m.def("Slider", [](py::object parent_obj) -> LvObjWrapper* {
        LvObjWrapper *_parent = parent_obj.is_none() ? nullptr : parent_obj.cast<LvObjWrapper*>();
        LvObjWrapper *wrapper = new LvObjWrapper(lv_slider_create(_parent ? _parent->get() : lv_screen_active()));
        if (_parent) wrapper->keep_parent(parent_obj);
        return wrapper;
    }, py::arg("parent") = py::none());
    m.def("Spinbox", [](py::object parent_obj) -> LvObjWrapper* {
        LvObjWrapper *_parent = parent_obj.is_none() ? nullptr : parent_obj.cast<LvObjWrapper*>();
        LvObjWrapper *wrapper = new LvObjWrapper(lv_spinbox_create(_parent ? _parent->get() : lv_screen_active()));
        if (_parent) wrapper->keep_parent(parent_obj);
        return wrapper;
    }, py::arg("parent") = py::none());
    m.def("Spinner", [](py::object parent_obj) -> LvObjWrapper* {
        LvObjWrapper *_parent = parent_obj.is_none() ? nullptr : parent_obj.cast<LvObjWrapper*>();
        LvObjWrapper *wrapper = new LvObjWrapper(lv_spinner_create(_parent ? _parent->get() : lv_screen_active()));
        if (_parent) wrapper->keep_parent(parent_obj);
        return wrapper;
    }, py::arg("parent") = py::none());
    m.def("Switch", [](py::object parent_obj) -> LvObjWrapper* {
        LvObjWrapper *_parent = parent_obj.is_none() ? nullptr : parent_obj.cast<LvObjWrapper*>();
        LvObjWrapper *wrapper = new LvObjWrapper(lv_switch_create(_parent ? _parent->get() : lv_screen_active()));
        if (_parent) wrapper->keep_parent(parent_obj);
        return wrapper;
    }, py::arg("parent") = py::none());
    m.def("Table", [](py::object parent_obj) -> LvObjWrapper* {
        LvObjWrapper *_parent = parent_obj.is_none() ? nullptr : parent_obj.cast<LvObjWrapper*>();
        LvObjWrapper *wrapper = new LvObjWrapper(lv_table_create(_parent ? _parent->get() : lv_screen_active()));
        if (_parent) wrapper->keep_parent(parent_obj);
        return wrapper;
    }, py::arg("parent") = py::none());
    m.def("Tabview", [](py::object parent_obj) -> LvObjWrapper* {
        LvObjWrapper *_parent = parent_obj.is_none() ? nullptr : parent_obj.cast<LvObjWrapper*>();
        LvObjWrapper *wrapper = new LvObjWrapper(lv_tabview_create(_parent ? _parent->get() : lv_screen_active()));
        if (_parent) wrapper->keep_parent(parent_obj);
        return wrapper;
    }, py::arg("parent") = py::none());
    m.def("Textarea", [](py::object parent_obj) -> LvObjWrapper* {
        LvObjWrapper *_parent = parent_obj.is_none() ? nullptr : parent_obj.cast<LvObjWrapper*>();
        LvObjWrapper *wrapper = new LvObjWrapper(lv_textarea_create(_parent ? _parent->get() : lv_screen_active()));
        if (_parent) wrapper->keep_parent(parent_obj);
        return wrapper;
    }, py::arg("parent") = py::none());
    m.def("Tileview", [](py::object parent_obj) -> LvObjWrapper* {
        LvObjWrapper *_parent = parent_obj.is_none() ? nullptr : parent_obj.cast<LvObjWrapper*>();
        LvObjWrapper *wrapper = new LvObjWrapper(lv_tileview_create(_parent ? _parent->get() : lv_screen_active()));
        if (_parent) wrapper->keep_parent(parent_obj);
        return wrapper;
    }, py::arg("parent") = py::none());
    m.def("Win", [](py::object parent_obj) -> LvObjWrapper* {
        LvObjWrapper *_parent = parent_obj.is_none() ? nullptr : parent_obj.cast<LvObjWrapper*>();
        LvObjWrapper *wrapper = new LvObjWrapper(lv_win_create(_parent ? _parent->get() : lv_screen_active()));
        if (_parent) wrapper->keep_parent(parent_obj);
        return wrapper;
    }, py::arg("parent") = py::none());

    /* Widget-specific methods on Obj */
    obj_cls.def("start", [](LvObjWrapper &self) { lv_animimg_start(self.get()); return; }
        );
    obj_cls.def("delete_obj", [](LvObjWrapper &self) -> bool { return lv_animimg_delete(self.get()); }
        );
    obj_cls.def("set_duration", [](LvObjWrapper &self, uint32_t duration) { lv_animimg_set_duration(self.get(), duration); return; }
        , py::arg("duration"));
    obj_cls.def("set_repeat_count", [](LvObjWrapper &self, uint32_t count) { lv_animimg_set_repeat_count(self.get(), count); return; }
        , py::arg("count"));
    obj_cls.def("set_reverse_duration", [](LvObjWrapper &self, uint32_t duration) { lv_animimg_set_reverse_duration(self.get(), duration); return; }
        , py::arg("duration"));
    obj_cls.def("set_reverse_delay", [](LvObjWrapper &self, uint32_t duration) { lv_animimg_set_reverse_delay(self.get(), duration); return; }
        , py::arg("duration"));
    obj_cls.def("set_start_cb", [](LvObjWrapper &self, lv_anim_start_cb_t start_cb) { lv_animimg_set_start_cb(self.get(), start_cb); return; }
        , py::arg("start_cb"));
    obj_cls.def("set_completed_cb", [](LvObjWrapper &self, lv_anim_completed_cb_t completed_cb) { lv_animimg_set_completed_cb(self.get(), completed_cb); return; }
        , py::arg("completed_cb"));
    obj_cls.def("get_src_count", [](LvObjWrapper &self) -> uint8_t { return lv_animimg_get_src_count(self.get()); }
        );
    obj_cls.def("get_duration", [](LvObjWrapper &self) -> uint32_t { return lv_animimg_get_duration(self.get()); }
        );
    obj_cls.def("get_repeat_count", [](LvObjWrapper &self) -> uint32_t { return lv_animimg_get_repeat_count(self.get()); }
        );
    obj_cls.def("set_start_angle", [](LvObjWrapper &self, lv_value_precise_t start) { lv_arc_set_start_angle(self.get(), start); return; }
        , py::arg("start"));
    obj_cls.def("set_end_angle", [](LvObjWrapper &self, lv_value_precise_t end) { lv_arc_set_end_angle(self.get(), end); return; }
        , py::arg("end"));
    obj_cls.def("set_angles", [](LvObjWrapper &self, lv_value_precise_t start, lv_value_precise_t end) { lv_arc_set_angles(self.get(), start, end); return; }
        , py::arg("start"), py::arg("end"));
    obj_cls.def("set_bg_start_angle", [](LvObjWrapper &self, lv_value_precise_t start) { lv_arc_set_bg_start_angle(self.get(), start); return; }
        , py::arg("start"));
    obj_cls.def("set_bg_end_angle", [](LvObjWrapper &self, lv_value_precise_t end) { lv_arc_set_bg_end_angle(self.get(), end); return; }
        , py::arg("end"));
    obj_cls.def("set_bg_angles", [](LvObjWrapper &self, lv_value_precise_t start, lv_value_precise_t end) { lv_arc_set_bg_angles(self.get(), start, end); return; }
        , py::arg("start"), py::arg("end"));
    obj_cls.def("arc_set_rotation", [](LvObjWrapper &self, int32_t rotation) { lv_arc_set_rotation(self.get(), rotation); return; }
        , py::arg("rotation"));
    obj_cls.def("arc_set_mode", [](LvObjWrapper &self, lv_arc_mode_t type) { lv_arc_set_mode(self.get(), type); return; }
        , py::arg("type"));
    obj_cls.def("arc_set_value", [](LvObjWrapper &self, int32_t value) { lv_arc_set_value(self.get(), value); return; }
        , py::arg("value"));
    obj_cls.def("arc_set_range", [](LvObjWrapper &self, int32_t min, int32_t max) { lv_arc_set_range(self.get(), min, max); return; }
        , py::arg("min"), py::arg("max"));
    obj_cls.def("arc_set_min_value", [](LvObjWrapper &self, int32_t min) { lv_arc_set_min_value(self.get(), min); return; }
        , py::arg("min"));
    obj_cls.def("arc_set_max_value", [](LvObjWrapper &self, int32_t max) { lv_arc_set_max_value(self.get(), max); return; }
        , py::arg("max"));
    obj_cls.def("set_change_rate", [](LvObjWrapper &self, uint32_t rate) { lv_arc_set_change_rate(self.get(), rate); return; }
        , py::arg("rate"));
    obj_cls.def("set_knob_offset", [](LvObjWrapper &self, int32_t offset) { lv_arc_set_knob_offset(self.get(), offset); return; }
        , py::arg("offset"));
    obj_cls.def("get_angle_start", [](LvObjWrapper &self) -> lv_value_precise_t { return lv_arc_get_angle_start(self.get()); }
        );
    obj_cls.def("get_angle_end", [](LvObjWrapper &self) -> lv_value_precise_t { return lv_arc_get_angle_end(self.get()); }
        );
    obj_cls.def("get_bg_angle_start", [](LvObjWrapper &self) -> lv_value_precise_t { return lv_arc_get_bg_angle_start(self.get()); }
        );
    obj_cls.def("get_bg_angle_end", [](LvObjWrapper &self) -> lv_value_precise_t { return lv_arc_get_bg_angle_end(self.get()); }
        );
    obj_cls.def("arc_get_value", [](LvObjWrapper &self) -> int32_t { return lv_arc_get_value(self.get()); }
        );
    obj_cls.def("arc_get_min_value", [](LvObjWrapper &self) -> int32_t { return lv_arc_get_min_value(self.get()); }
        );
    obj_cls.def("arc_get_max_value", [](LvObjWrapper &self) -> int32_t { return lv_arc_get_max_value(self.get()); }
        );
    obj_cls.def("arc_get_mode", [](LvObjWrapper &self) -> lv_arc_mode_t { return lv_arc_get_mode(self.get()); }
        );
    obj_cls.def("arc_get_rotation", [](LvObjWrapper &self) -> int32_t { return lv_arc_get_rotation(self.get()); }
        );
    obj_cls.def("get_knob_offset", [](LvObjWrapper &self) -> int32_t { return lv_arc_get_knob_offset(self.get()); }
        );
    obj_cls.def("get_change_rate", [](LvObjWrapper &self) -> uint32_t { return lv_arc_get_change_rate(self.get()); }
        );
    obj_cls.def("align_obj_to_angle", [](LvObjWrapper &self, LvObjWrapper & obj_to_align, int32_t r_offset) { lv_arc_align_obj_to_angle(self.get(), obj_to_align.get(), r_offset); return; }
        , py::arg("obj_to_align"), py::arg("r_offset"));
    obj_cls.def("rotate_obj_to_angle", [](LvObjWrapper &self, LvObjWrapper & obj_to_rotate, int32_t r_offset) { lv_arc_rotate_obj_to_angle(self.get(), obj_to_rotate.get(), r_offset); return; }
        , py::arg("obj_to_rotate"), py::arg("r_offset"));
    obj_cls.def("bar_set_value", [](LvObjWrapper &self, int32_t value, bool anim) { lv_bar_set_value(self.get(), value, anim); return; }
        , py::arg("value"), py::arg("anim"));
    obj_cls.def("bar_set_start_value", [](LvObjWrapper &self, int32_t start_value, bool anim) { lv_bar_set_start_value(self.get(), start_value, anim); return; }
        , py::arg("start_value"), py::arg("anim"));
    obj_cls.def("bar_set_range", [](LvObjWrapper &self, int32_t min, int32_t max) { lv_bar_set_range(self.get(), min, max); return; }
        , py::arg("min"), py::arg("max"));
    obj_cls.def("bar_set_min_value", [](LvObjWrapper &self, int32_t min) { lv_bar_set_min_value(self.get(), min); return; }
        , py::arg("min"));
    obj_cls.def("bar_set_max_value", [](LvObjWrapper &self, int32_t max) { lv_bar_set_max_value(self.get(), max); return; }
        , py::arg("max"));
    obj_cls.def("bar_set_mode", [](LvObjWrapper &self, lv_bar_mode_t mode) { lv_bar_set_mode(self.get(), mode); return; }
        , py::arg("mode"));
    obj_cls.def("bar_set_orientation", [](LvObjWrapper &self, lv_bar_orientation_t orientation) { lv_bar_set_orientation(self.get(), orientation); return; }
        , py::arg("orientation"));
    obj_cls.def("bar_get_value", [](LvObjWrapper &self) -> int32_t { return lv_bar_get_value(self.get()); }
        );
    obj_cls.def("get_start_value", [](LvObjWrapper &self) -> int32_t { return lv_bar_get_start_value(self.get()); }
        );
    obj_cls.def("bar_get_min_value", [](LvObjWrapper &self) -> int32_t { return lv_bar_get_min_value(self.get()); }
        );
    obj_cls.def("bar_get_max_value", [](LvObjWrapper &self) -> int32_t { return lv_bar_get_max_value(self.get()); }
        );
    obj_cls.def("bar_get_mode", [](LvObjWrapper &self) -> lv_bar_mode_t { return lv_bar_get_mode(self.get()); }
        );
    obj_cls.def("bar_get_orientation", [](LvObjWrapper &self) -> lv_bar_orientation_t { return lv_bar_get_orientation(self.get()); }
        );
    obj_cls.def("bar_is_symmetrical", [](LvObjWrapper &self) -> bool { return lv_bar_is_symmetrical(self.get()); }
        );
    obj_cls.def("set_buffer", [](LvObjWrapper &self, void * buf, int32_t w, int32_t h, lv_color_format_t cf) { lv_canvas_set_buffer(self.get(), buf, w, h, cf); return; }
        , py::arg("buf"), py::arg("w"), py::arg("h"), py::arg("cf"));
    obj_cls.def("set_draw_buf", [](LvObjWrapper &self, lv_draw_buf_t * draw_buf) { lv_canvas_set_draw_buf(self.get(), draw_buf); return; }
        , py::arg("draw_buf"));
    obj_cls.def("set_px", [](LvObjWrapper &self, int32_t x, int32_t y, lv_color_t color, uint8_t opa) { lv_canvas_set_px(self.get(), x, y, color, opa); return; }
        , py::arg("x"), py::arg("y"), py::arg("color"), py::arg("opa"));
    obj_cls.def("set_palette", [](LvObjWrapper &self, uint8_t index, lv_color32_t color) { lv_canvas_set_palette(self.get(), index, color); return; }
        , py::arg("index"), py::arg("color"));
    obj_cls.def("get_draw_buf", [](LvObjWrapper &self) -> lv_draw_buf_t * { return lv_canvas_get_draw_buf(self.get()); }
        );
    obj_cls.def("get_px", [](LvObjWrapper &self, int32_t x, int32_t y) -> lv_color32_t { return lv_canvas_get_px(self.get(), x, y); }
        , py::arg("x"), py::arg("y"));
    obj_cls.def("get_image", [](LvObjWrapper &self) -> lv_image_dsc_t * { return lv_canvas_get_image(self.get()); }
        );
    obj_cls.def("copy_buf", [](LvObjWrapper &self, const lv_area_t * canvas_area, lv_draw_buf_t * src_buf, const lv_area_t * src_area) { lv_canvas_copy_buf(self.get(), canvas_area, src_buf, src_area); return; }
        , py::arg("canvas_area"), py::arg("src_buf"), py::arg("src_area"));
    obj_cls.def("fill_bg", [](LvObjWrapper &self, lv_color_t color, uint8_t opa) { lv_canvas_fill_bg(self.get(), color, opa); return; }
        , py::arg("color"), py::arg("opa"));
    obj_cls.def("set_type", [](LvObjWrapper &self, lv_chart_type_t type) { lv_chart_set_type(self.get(), type); return; }
        , py::arg("type"));
    obj_cls.def("set_axis_range", [](LvObjWrapper &self, lv_chart_axis_t axis, int32_t min, int32_t max) { lv_chart_set_axis_range(self.get(), axis, min, max); return; }
        , py::arg("axis"), py::arg("min"), py::arg("max"));
    obj_cls.def("set_axis_min_value", [](LvObjWrapper &self, lv_chart_axis_t axis, int32_t min) { lv_chart_set_axis_min_value(self.get(), axis, min); return; }
        , py::arg("axis"), py::arg("min"));
    obj_cls.def("set_axis_max_value", [](LvObjWrapper &self, lv_chart_axis_t axis, int32_t max) { lv_chart_set_axis_max_value(self.get(), axis, max); return; }
        , py::arg("axis"), py::arg("max"));
    obj_cls.def("set_update_mode", [](LvObjWrapper &self, lv_chart_update_mode_t update_mode) { lv_chart_set_update_mode(self.get(), update_mode); return; }
        , py::arg("update_mode"));
    obj_cls.def("set_div_line_count", [](LvObjWrapper &self, uint32_t hdiv, uint32_t vdiv) { lv_chart_set_div_line_count(self.get(), hdiv, vdiv); return; }
        , py::arg("hdiv"), py::arg("vdiv"));
    obj_cls.def("set_hor_div_line_count", [](LvObjWrapper &self, uint32_t cnt) { lv_chart_set_hor_div_line_count(self.get(), cnt); return; }
        , py::arg("cnt"));
    obj_cls.def("set_ver_div_line_count", [](LvObjWrapper &self, uint32_t cnt) { lv_chart_set_ver_div_line_count(self.get(), cnt); return; }
        , py::arg("cnt"));
    obj_cls.def("get_type", [](LvObjWrapper &self) -> lv_chart_type_t { return lv_chart_get_type(self.get()); }
        );
    obj_cls.def("chart_get_point_count", [](LvObjWrapper &self) -> uint32_t { return lv_chart_get_point_count(self.get()); }
        );
    obj_cls.def("get_update_mode", [](LvObjWrapper &self) -> lv_chart_update_mode_t { return lv_chart_get_update_mode(self.get()); }
        );
    obj_cls.def("get_hor_div_line_count", [](LvObjWrapper &self) -> uint32_t { return lv_chart_get_hor_div_line_count(self.get()); }
        );
    obj_cls.def("get_ver_div_line_count", [](LvObjWrapper &self) -> uint32_t { return lv_chart_get_ver_div_line_count(self.get()); }
        );
    obj_cls.def("get_pressed_point", [](LvObjWrapper &self) -> uint32_t { return lv_chart_get_pressed_point(self.get()); }
        );
    obj_cls.def("get_first_point_center_offset", [](LvObjWrapper &self) -> int32_t { return lv_chart_get_first_point_center_offset(self.get()); }
        );
    obj_cls.def("checkbox_set_text", [](LvObjWrapper &self, const char * txt) { lv_checkbox_set_text(self.get(), txt); return; }
        , py::arg("txt"));
    obj_cls.def("checkbox_set_text_static", [](LvObjWrapper &self, const char * txt) { lv_checkbox_set_text_static(self.get(), txt); return; }
        , py::arg("txt"));
    obj_cls.def("checkbox_get_text", [](LvObjWrapper &self) -> const char * { return lv_checkbox_get_text(self.get()); }
        );
    obj_cls.def("dropdown_set_text", [](LvObjWrapper &self, const char * text) { lv_dropdown_set_text(self.get(), text); return; }
        , py::arg("text"));
    obj_cls.def("dropdown_set_text_static", [](LvObjWrapper &self, const char * text) { lv_dropdown_set_text_static(self.get(), text); return; }
        , py::arg("text"));
    obj_cls.def("dropdown_set_options", [](LvObjWrapper &self, const char * options) { lv_dropdown_set_options(self.get(), options); return; }
        , py::arg("options"));
    obj_cls.def("set_options_static", [](LvObjWrapper &self, const char * options) { lv_dropdown_set_options_static(self.get(), options); return; }
        , py::arg("options"));
    obj_cls.def("add_option", [](LvObjWrapper &self, const char * option, uint32_t pos) { lv_dropdown_add_option(self.get(), option, pos); return; }
        , py::arg("option"), py::arg("pos"));
    obj_cls.def("clear_options", [](LvObjWrapper &self) { lv_dropdown_clear_options(self.get()); return; }
        );
    obj_cls.def("dropdown_set_selected", [](LvObjWrapper &self, uint32_t sel_opt) { lv_dropdown_set_selected(self.get(), sel_opt); return; }
        , py::arg("sel_opt"));
    obj_cls.def("set_dir", [](LvObjWrapper &self, lv_dir_t dir) { lv_dropdown_set_dir(self.get(), dir); return; }
        , py::arg("dir"));
    obj_cls.def("set_symbol", [](LvObjWrapper &self, const char * symbol) { lv_dropdown_set_symbol(self.get(), (const void *)symbol); return; }
        , py::arg("symbol"));
    obj_cls.def("set_selected_highlight", [](LvObjWrapper &self, bool en) { lv_dropdown_set_selected_highlight(self.get(), en); return; }
        , py::arg("en"));
    obj_cls.def("get_list", [](LvObjWrapper &self) -> LvObjWrapper* {
        lv_obj_t *result = lv_dropdown_get_list(self.get());
        return result ? new LvObjWrapper(result) : nullptr;
    }
        );
    obj_cls.def("dropdown_get_text", [](LvObjWrapper &self) -> const char * { return lv_dropdown_get_text(self.get()); }
        );
    obj_cls.def("dropdown_get_options", [](LvObjWrapper &self) -> const char * { return lv_dropdown_get_options(self.get()); }
        );
    obj_cls.def("dropdown_get_selected", [](LvObjWrapper &self) -> uint32_t { return lv_dropdown_get_selected(self.get()); }
        );
    obj_cls.def("dropdown_get_option_count", [](LvObjWrapper &self) -> uint32_t { return lv_dropdown_get_option_count(self.get()); }
        );
    obj_cls.def("get_option_index", [](LvObjWrapper &self, const char * option) -> int32_t { return lv_dropdown_get_option_index(self.get(), option); }
        , py::arg("option"));
    obj_cls.def("get_symbol", [](LvObjWrapper &self) -> const char * { return lv_dropdown_get_symbol(self.get()); }
        );
    obj_cls.def("get_selected_highlight", [](LvObjWrapper &self) -> bool { return lv_dropdown_get_selected_highlight(self.get()); }
        );
    obj_cls.def("get_dir", [](LvObjWrapper &self) -> lv_dir_t { return lv_dropdown_get_dir(self.get()); }
        );
    obj_cls.def("open", [](LvObjWrapper &self) { lv_dropdown_open(self.get()); return; }
        );
    obj_cls.def("dropdown_close", [](LvObjWrapper &self) { lv_dropdown_close(self.get()); return; }
        );
    obj_cls.def("is_open", [](LvObjWrapper &self) -> bool { return lv_dropdown_is_open(self.get()); }
        );
    obj_cls.def("set_src", [](LvObjWrapper &self, const char * src) { lv_image_set_src(self.get(), (const void *)src); return; }
        , py::arg("src"));
    obj_cls.def("set_offset_x", [](LvObjWrapper &self, int32_t x) { lv_image_set_offset_x(self.get(), x); return; }
        , py::arg("x"));
    obj_cls.def("set_offset_y", [](LvObjWrapper &self, int32_t y) { lv_image_set_offset_y(self.get(), y); return; }
        , py::arg("y"));
    obj_cls.def("image_set_rotation", [](LvObjWrapper &self, int32_t angle) { lv_image_set_rotation(self.get(), angle); return; }
        , py::arg("angle"));
    obj_cls.def("set_pivot", [](LvObjWrapper &self, int32_t x, int32_t y) { lv_image_set_pivot(self.get(), x, y); return; }
        , py::arg("x"), py::arg("y"));
    obj_cls.def("set_pivot_x", [](LvObjWrapper &self, int32_t x) { lv_image_set_pivot_x(self.get(), x); return; }
        , py::arg("x"));
    obj_cls.def("set_pivot_y", [](LvObjWrapper &self, int32_t y) { lv_image_set_pivot_y(self.get(), y); return; }
        , py::arg("y"));
    obj_cls.def("set_scale", [](LvObjWrapper &self, uint32_t zoom) { lv_image_set_scale(self.get(), zoom); return; }
        , py::arg("zoom"));
    obj_cls.def("set_scale_x", [](LvObjWrapper &self, uint32_t zoom) { lv_image_set_scale_x(self.get(), zoom); return; }
        , py::arg("zoom"));
    obj_cls.def("set_scale_y", [](LvObjWrapper &self, uint32_t zoom) { lv_image_set_scale_y(self.get(), zoom); return; }
        , py::arg("zoom"));
    obj_cls.def("set_blend_mode", [](LvObjWrapper &self, lv_blend_mode_t blend_mode) { lv_image_set_blend_mode(self.get(), blend_mode); return; }
        , py::arg("blend_mode"));
    obj_cls.def("set_antialias", [](LvObjWrapper &self, bool antialias) { lv_image_set_antialias(self.get(), antialias); return; }
        , py::arg("antialias"));
    obj_cls.def("set_inner_align", [](LvObjWrapper &self, lv_image_align_t align) { lv_image_set_inner_align(self.get(), align); return; }
        , py::arg("align"));
    obj_cls.def("set_bitmap_map_src", [](LvObjWrapper &self, const lv_image_dsc_t * src) { lv_image_set_bitmap_map_src(self.get(), src); return; }
        , py::arg("src"));
    obj_cls.def("get_offset_x", [](LvObjWrapper &self) -> int32_t { return lv_image_get_offset_x(self.get()); }
        );
    obj_cls.def("get_offset_y", [](LvObjWrapper &self) -> int32_t { return lv_image_get_offset_y(self.get()); }
        );
    obj_cls.def("image_get_rotation", [](LvObjWrapper &self) -> int32_t { return lv_image_get_rotation(self.get()); }
        );
    obj_cls.def("get_pivot", [](LvObjWrapper &self, lv_point_t * pivot) { lv_image_get_pivot(self.get(), pivot); return; }
        , py::arg("pivot"));
    obj_cls.def("get_scale", [](LvObjWrapper &self) -> int32_t { return lv_image_get_scale(self.get()); }
        );
    obj_cls.def("get_scale_x", [](LvObjWrapper &self) -> int32_t { return lv_image_get_scale_x(self.get()); }
        );
    obj_cls.def("get_scale_y", [](LvObjWrapper &self) -> int32_t { return lv_image_get_scale_y(self.get()); }
        );
    obj_cls.def("get_src_width", [](LvObjWrapper &self) -> int32_t { return lv_image_get_src_width(self.get()); }
        );
    obj_cls.def("get_src_height", [](LvObjWrapper &self) -> int32_t { return lv_image_get_src_height(self.get()); }
        );
    obj_cls.def("get_transformed_width", [](LvObjWrapper &self) -> int32_t { return lv_image_get_transformed_width(self.get()); }
        );
    obj_cls.def("get_transformed_height", [](LvObjWrapper &self) -> int32_t { return lv_image_get_transformed_height(self.get()); }
        );
    obj_cls.def("get_blend_mode", [](LvObjWrapper &self) -> lv_blend_mode_t { return lv_image_get_blend_mode(self.get()); }
        );
    obj_cls.def("get_antialias", [](LvObjWrapper &self) -> bool { return lv_image_get_antialias(self.get()); }
        );
    obj_cls.def("get_inner_align", [](LvObjWrapper &self) -> lv_image_align_t { return lv_image_get_inner_align(self.get()); }
        );
    obj_cls.def("get_bitmap_map_src", [](LvObjWrapper &self) -> const lv_image_dsc_t * { return lv_image_get_bitmap_map_src(self.get()); }
        );
    obj_cls.def("set_textarea", [](LvObjWrapper &self, LvObjWrapper & ta) { lv_keyboard_set_textarea(self.get(), ta.get()); return; }
        , py::arg("ta"));
    obj_cls.def("keyboard_set_mode", [](LvObjWrapper &self, lv_keyboard_mode_t mode) { lv_keyboard_set_mode(self.get(), mode); return; }
        , py::arg("mode"));
    obj_cls.def("set_popovers", [](LvObjWrapper &self, bool en) { lv_keyboard_set_popovers(self.get(), en); return; }
        , py::arg("en"));
    obj_cls.def("get_textarea", [](LvObjWrapper &self) -> LvObjWrapper* {
        lv_obj_t *result = lv_keyboard_get_textarea(self.get());
        return result ? new LvObjWrapper(result) : nullptr;
    }
        );
    obj_cls.def("keyboard_get_mode", [](LvObjWrapper &self) -> lv_keyboard_mode_t { return lv_keyboard_get_mode(self.get()); }
        );
    obj_cls.def("get_popovers", [](LvObjWrapper &self) -> bool { return lv_keyboard_get_popovers(self.get()); }
        );
    obj_cls.def("get_selected_button", [](LvObjWrapper &self) -> uint32_t { return lv_keyboard_get_selected_button(self.get()); }
        );
    obj_cls.def("keyboard_get_button_text", [](LvObjWrapper &self, uint32_t btn_id) -> const char * { return lv_keyboard_get_button_text(self.get(), btn_id); }
        , py::arg("btn_id"));
    obj_cls.def("label_set_text", [](LvObjWrapper &self, const char * text) { lv_label_set_text(self.get(), text); return; }
        , py::arg("text"));
    obj_cls.def("label_set_text_static", [](LvObjWrapper &self, const char * text) { lv_label_set_text_static(self.get(), text); return; }
        , py::arg("text"));
    obj_cls.def("set_long_mode", [](LvObjWrapper &self, lv_label_long_mode_t long_mode) { lv_label_set_long_mode(self.get(), long_mode); return; }
        , py::arg("long_mode"));
    obj_cls.def("set_max_lines", [](LvObjWrapper &self, int32_t lines) { lv_label_set_max_lines(self.get(), lines); return; }
        , py::arg("lines"));
    obj_cls.def("set_text_selection_start", [](LvObjWrapper &self, uint32_t index) { lv_label_set_text_selection_start(self.get(), index); return; }
        , py::arg("index"));
    obj_cls.def("set_text_selection_end", [](LvObjWrapper &self, uint32_t index) { lv_label_set_text_selection_end(self.get(), index); return; }
        , py::arg("index"));
    obj_cls.def("set_recolor", [](LvObjWrapper &self, bool en) { lv_label_set_recolor(self.get(), en); return; }
        , py::arg("en"));
    obj_cls.def("label_get_text", [](LvObjWrapper &self) -> const char * { return lv_label_get_text(self.get()); }
        );
    obj_cls.def("get_long_mode", [](LvObjWrapper &self) -> lv_label_long_mode_t { return lv_label_get_long_mode(self.get()); }
        );
    obj_cls.def("get_max_lines", [](LvObjWrapper &self) -> int32_t { return lv_label_get_max_lines(self.get()); }
        );
    obj_cls.def("get_letter_pos", [](LvObjWrapper &self, uint32_t char_id, lv_point_t * pos) { lv_label_get_letter_pos(self.get(), char_id, pos); return; }
        , py::arg("char_id"), py::arg("pos"));
    obj_cls.def("get_letter_on", [](LvObjWrapper &self, lv_point_t * pos_in, bool bidi) -> uint32_t { return lv_label_get_letter_on(self.get(), pos_in, bidi); }
        , py::arg("pos_in"), py::arg("bidi"));
    obj_cls.def("is_char_under_pos", [](LvObjWrapper &self, lv_point_t * pos) -> bool { return lv_label_is_char_under_pos(self.get(), pos); }
        , py::arg("pos"));
    obj_cls.def("get_text_selection_start", [](LvObjWrapper &self) -> uint32_t { return lv_label_get_text_selection_start(self.get()); }
        );
    obj_cls.def("get_text_selection_end", [](LvObjWrapper &self) -> uint32_t { return lv_label_get_text_selection_end(self.get()); }
        );
    obj_cls.def("get_recolor", [](LvObjWrapper &self) -> bool { return lv_label_get_recolor(self.get()); }
        );
    obj_cls.def("ins_text", [](LvObjWrapper &self, uint32_t pos, const char * txt) { lv_label_ins_text(self.get(), pos, txt); return; }
        , py::arg("pos"), py::arg("txt"));
    obj_cls.def("cut_text", [](LvObjWrapper &self, uint32_t pos, uint32_t cnt) { lv_label_cut_text(self.get(), pos, cnt); return; }
        , py::arg("pos"), py::arg("cnt"));
    obj_cls.def("set_color", [](LvObjWrapper &self, lv_color_t color) { lv_led_set_color(self.get(), color); return; }
        , py::arg("color"));
    obj_cls.def("set_brightness", [](LvObjWrapper &self, uint8_t bright) { lv_led_set_brightness(self.get(), bright); return; }
        , py::arg("bright"));
    obj_cls.def("on", [](LvObjWrapper &self) { lv_led_on(self.get()); return; }
        );
    obj_cls.def("off", [](LvObjWrapper &self) { lv_led_off(self.get()); return; }
        );
    obj_cls.def("toggle", [](LvObjWrapper &self) { lv_led_toggle(self.get()); return; }
        );
    obj_cls.def("get_brightness", [](LvObjWrapper &self) -> uint8_t { return lv_led_get_brightness(self.get()); }
        );
    obj_cls.def("get_color", [](LvObjWrapper &self) -> lv_color_t { return lv_led_get_color(self.get()); }
        );
    obj_cls.def("set_y_invert", [](LvObjWrapper &self, bool en) { lv_line_set_y_invert(self.get(), en); return; }
        , py::arg("en"));
    obj_cls.def("get_points", [](LvObjWrapper &self) -> const lv_point_precise_t * { return lv_line_get_points(self.get()); }
        );
    obj_cls.def("line_get_point_count", [](LvObjWrapper &self) -> uint32_t { return lv_line_get_point_count(self.get()); }
        );
    obj_cls.def("is_point_array_mutable", [](LvObjWrapper &self) -> bool { return lv_line_is_point_array_mutable(self.get()); }
        );
    obj_cls.def("get_points_mutable", [](LvObjWrapper &self) -> lv_point_precise_t * { return lv_line_get_points_mutable(self.get()); }
        );
    obj_cls.def("get_y_invert", [](LvObjWrapper &self) -> bool { return lv_line_get_y_invert(self.get()); }
        );
    obj_cls.def("list_add_text", [](LvObjWrapper &self, const char * txt) -> LvObjWrapper* {
        lv_obj_t *result = lv_list_add_text(self.get(), txt);
        return result ? new LvObjWrapper(result) : nullptr;
    }
        , py::arg("txt"));
    obj_cls.def("list_add_button", [](LvObjWrapper &self, const char * icon, const char * txt) -> LvObjWrapper* {
        lv_obj_t *result = lv_list_add_button(self.get(), (const void *)icon, txt);
        return result ? new LvObjWrapper(result) : nullptr;
    }
        , py::arg("icon"), py::arg("txt"));
    obj_cls.def("list_get_button_text", [](LvObjWrapper &self, LvObjWrapper & btn) -> const char * { return lv_list_get_button_text(self.get(), btn.get()); }
        , py::arg("btn"));
    obj_cls.def("set_button_text", [](LvObjWrapper &self, LvObjWrapper & btn, const char * txt) { lv_list_set_button_text(self.get(), btn.get(), txt); return; }
        , py::arg("btn"), py::arg("txt"));
    obj_cls.def("set_page", [](LvObjWrapper &self, LvObjWrapper & page) { lv_menu_set_page(self.get(), page.get()); return; }
        , py::arg("page"));
    obj_cls.def("set_page_title", [](LvObjWrapper &self, const char * title) { lv_menu_set_page_title(self.get(), title); return; }
        , py::arg("title"));
    obj_cls.def("set_page_title_static", [](LvObjWrapper &self, const char * title) { lv_menu_set_page_title_static(self.get(), title); return; }
        , py::arg("title"));
    obj_cls.def("set_sidebar_page", [](LvObjWrapper &self, LvObjWrapper & page) { lv_menu_set_sidebar_page(self.get(), page.get()); return; }
        , py::arg("page"));
    obj_cls.def("set_mode_header", [](LvObjWrapper &self, lv_menu_mode_header_t mode) { lv_menu_set_mode_header(self.get(), mode); return; }
        , py::arg("mode"));
    obj_cls.def("set_mode_root_back_button", [](LvObjWrapper &self, lv_menu_mode_root_back_button_t mode) { lv_menu_set_mode_root_back_button(self.get(), mode); return; }
        , py::arg("mode"));
    obj_cls.def("set_load_page_event", [](LvObjWrapper &self, LvObjWrapper & obj, LvObjWrapper & page) { lv_menu_set_load_page_event(self.get(), obj.get(), page.get()); return; }
        , py::arg("obj"), py::arg("page"));
    obj_cls.def("get_cur_main_page", [](LvObjWrapper &self) -> LvObjWrapper* {
        lv_obj_t *result = lv_menu_get_cur_main_page(self.get());
        return result ? new LvObjWrapper(result) : nullptr;
    }
        );
    obj_cls.def("get_cur_sidebar_page", [](LvObjWrapper &self) -> LvObjWrapper* {
        lv_obj_t *result = lv_menu_get_cur_sidebar_page(self.get());
        return result ? new LvObjWrapper(result) : nullptr;
    }
        );
    obj_cls.def("get_main_header", [](LvObjWrapper &self) -> LvObjWrapper* {
        lv_obj_t *result = lv_menu_get_main_header(self.get());
        return result ? new LvObjWrapper(result) : nullptr;
    }
        );
    obj_cls.def("get_main_header_back_button", [](LvObjWrapper &self) -> LvObjWrapper* {
        lv_obj_t *result = lv_menu_get_main_header_back_button(self.get());
        return result ? new LvObjWrapper(result) : nullptr;
    }
        );
    obj_cls.def("get_sidebar_header", [](LvObjWrapper &self) -> LvObjWrapper* {
        lv_obj_t *result = lv_menu_get_sidebar_header(self.get());
        return result ? new LvObjWrapper(result) : nullptr;
    }
        );
    obj_cls.def("get_sidebar_header_back_button", [](LvObjWrapper &self) -> LvObjWrapper* {
        lv_obj_t *result = lv_menu_get_sidebar_header_back_button(self.get());
        return result ? new LvObjWrapper(result) : nullptr;
    }
        );
    obj_cls.def("back_button_is_root", [](LvObjWrapper &self, LvObjWrapper & obj) -> bool { return lv_menu_back_button_is_root(self.get(), obj.get()); }
        , py::arg("obj"));
    obj_cls.def("get_mode_header", [](LvObjWrapper &self) -> lv_menu_mode_header_t { return lv_menu_get_mode_header(self.get()); }
        );
    obj_cls.def("get_mode_root_back_button", [](LvObjWrapper &self) -> lv_menu_mode_root_back_button_t { return lv_menu_get_mode_root_back_button(self.get()); }
        );
    obj_cls.def("clear_history", [](LvObjWrapper &self) { lv_menu_clear_history(self.get()); return; }
        );
    obj_cls.def("msgbox_add_title", [](LvObjWrapper &self, const char * title) -> LvObjWrapper* {
        lv_obj_t *result = lv_msgbox_add_title(self.get(), title);
        return result ? new LvObjWrapper(result) : nullptr;
    }
        , py::arg("title"));
    obj_cls.def("add_header_button", [](LvObjWrapper &self, const char * icon) -> LvObjWrapper* {
        lv_obj_t *result = lv_msgbox_add_header_button(self.get(), (const void *)icon);
        return result ? new LvObjWrapper(result) : nullptr;
    }
        , py::arg("icon"));
    obj_cls.def("msgbox_add_text", [](LvObjWrapper &self, const char * text) -> LvObjWrapper* {
        lv_obj_t *result = lv_msgbox_add_text(self.get(), text);
        return result ? new LvObjWrapper(result) : nullptr;
    }
        , py::arg("text"));
    obj_cls.def("add_text_fmt", [](LvObjWrapper &self, const char * fmt) -> LvObjWrapper* {
        lv_obj_t *result = lv_msgbox_add_text_fmt(self.get(), fmt);
        return result ? new LvObjWrapper(result) : nullptr;
    }
        , py::arg("fmt"));
    obj_cls.def("add_footer_button", [](LvObjWrapper &self, const char * text) -> LvObjWrapper* {
        lv_obj_t *result = lv_msgbox_add_footer_button(self.get(), text);
        return result ? new LvObjWrapper(result) : nullptr;
    }
        , py::arg("text"));
    obj_cls.def("add_close_button", [](LvObjWrapper &self) -> LvObjWrapper* {
        lv_obj_t *result = lv_msgbox_add_close_button(self.get());
        return result ? new LvObjWrapper(result) : nullptr;
    }
        );
    obj_cls.def("msgbox_get_header", [](LvObjWrapper &self) -> LvObjWrapper* {
        lv_obj_t *result = lv_msgbox_get_header(self.get());
        return result ? new LvObjWrapper(result) : nullptr;
    }
        );
    obj_cls.def("get_footer", [](LvObjWrapper &self) -> LvObjWrapper* {
        lv_obj_t *result = lv_msgbox_get_footer(self.get());
        return result ? new LvObjWrapper(result) : nullptr;
    }
        );
    obj_cls.def("msgbox_get_content", [](LvObjWrapper &self) -> LvObjWrapper* {
        lv_obj_t *result = lv_msgbox_get_content(self.get());
        return result ? new LvObjWrapper(result) : nullptr;
    }
        );
    obj_cls.def("get_title", [](LvObjWrapper &self) -> LvObjWrapper* {
        lv_obj_t *result = lv_msgbox_get_title(self.get());
        return result ? new LvObjWrapper(result) : nullptr;
    }
        );
    obj_cls.def("msgbox_close", [](LvObjWrapper &self) { lv_msgbox_close(self.get()); return; }
        );
    obj_cls.def("close_async", [](LvObjWrapper &self) { lv_msgbox_close_async(self.get()); return; }
        );
    obj_cls.def("roller_set_options", [](LvObjWrapper &self, const char * options, lv_roller_mode_t mode) { lv_roller_set_options(self.get(), options, mode); return; }
        , py::arg("options"), py::arg("mode"));
    obj_cls.def("roller_set_selected", [](LvObjWrapper &self, uint32_t sel_opt, bool anim) { lv_roller_set_selected(self.get(), sel_opt, anim); return; }
        , py::arg("sel_opt"), py::arg("anim"));
    obj_cls.def("set_selected_str", [](LvObjWrapper &self, const char * sel_opt, bool anim) -> bool { return lv_roller_set_selected_str(self.get(), sel_opt, anim); }
        , py::arg("sel_opt"), py::arg("anim"));
    obj_cls.def("set_visible_row_count", [](LvObjWrapper &self, uint32_t row_cnt) { lv_roller_set_visible_row_count(self.get(), row_cnt); return; }
        , py::arg("row_cnt"));
    obj_cls.def("roller_get_selected", [](LvObjWrapper &self) -> uint32_t { return lv_roller_get_selected(self.get()); }
        );
    obj_cls.def("roller_get_options", [](LvObjWrapper &self) -> const char * { return lv_roller_get_options(self.get()); }
        );
    obj_cls.def("roller_get_option_count", [](LvObjWrapper &self) -> uint32_t { return lv_roller_get_option_count(self.get()); }
        );
    obj_cls.def("scale_set_mode", [](LvObjWrapper &self, lv_scale_mode_t mode) { lv_scale_set_mode(self.get(), mode); return; }
        , py::arg("mode"));
    obj_cls.def("set_total_tick_count", [](LvObjWrapper &self, uint32_t total_tick_count) { lv_scale_set_total_tick_count(self.get(), total_tick_count); return; }
        , py::arg("total_tick_count"));
    obj_cls.def("set_major_tick_every", [](LvObjWrapper &self, uint32_t major_tick_every) { lv_scale_set_major_tick_every(self.get(), major_tick_every); return; }
        , py::arg("major_tick_every"));
    obj_cls.def("set_label_show", [](LvObjWrapper &self, bool show_label) { lv_scale_set_label_show(self.get(), show_label); return; }
        , py::arg("show_label"));
    obj_cls.def("scale_set_range", [](LvObjWrapper &self, int32_t min, int32_t max) { lv_scale_set_range(self.get(), min, max); return; }
        , py::arg("min"), py::arg("max"));
    obj_cls.def("scale_set_min_value", [](LvObjWrapper &self, int32_t min) { lv_scale_set_min_value(self.get(), min); return; }
        , py::arg("min"));
    obj_cls.def("scale_set_max_value", [](LvObjWrapper &self, int32_t max) { lv_scale_set_max_value(self.get(), max); return; }
        , py::arg("max"));
    obj_cls.def("set_angle_range", [](LvObjWrapper &self, uint32_t angle_range) { lv_scale_set_angle_range(self.get(), angle_range); return; }
        , py::arg("angle_range"));
    obj_cls.def("scale_set_rotation", [](LvObjWrapper &self, int32_t rotation) { lv_scale_set_rotation(self.get(), rotation); return; }
        , py::arg("rotation"));
    obj_cls.def("set_line_needle_value", [](LvObjWrapper &self, LvObjWrapper & needle_line, int32_t needle_length, int32_t value) { lv_scale_set_line_needle_value(self.get(), needle_line.get(), needle_length, value); return; }
        , py::arg("needle_line"), py::arg("needle_length"), py::arg("value"));
    obj_cls.def("set_image_needle_value", [](LvObjWrapper &self, LvObjWrapper & needle_img, int32_t value) { lv_scale_set_image_needle_value(self.get(), needle_img.get(), value); return; }
        , py::arg("needle_img"), py::arg("value"));
    obj_cls.def("set_post_draw", [](LvObjWrapper &self, bool en) { lv_scale_set_post_draw(self.get(), en); return; }
        , py::arg("en"));
    obj_cls.def("set_draw_ticks_on_top", [](LvObjWrapper &self, bool en) { lv_scale_set_draw_ticks_on_top(self.get(), en); return; }
        , py::arg("en"));
    obj_cls.def("scale_get_mode", [](LvObjWrapper &self) -> lv_scale_mode_t { return lv_scale_get_mode(self.get()); }
        );
    obj_cls.def("get_total_tick_count", [](LvObjWrapper &self) -> int32_t { return lv_scale_get_total_tick_count(self.get()); }
        );
    obj_cls.def("get_major_tick_every", [](LvObjWrapper &self) -> int32_t { return lv_scale_get_major_tick_every(self.get()); }
        );
    obj_cls.def("scale_get_rotation", [](LvObjWrapper &self) -> int32_t { return lv_scale_get_rotation(self.get()); }
        );
    obj_cls.def("get_label_show", [](LvObjWrapper &self) -> bool { return lv_scale_get_label_show(self.get()); }
        );
    obj_cls.def("get_angle_range", [](LvObjWrapper &self) -> uint32_t { return lv_scale_get_angle_range(self.get()); }
        );
    obj_cls.def("get_range_min_value", [](LvObjWrapper &self) -> int32_t { return lv_scale_get_range_min_value(self.get()); }
        );
    obj_cls.def("get_range_max_value", [](LvObjWrapper &self) -> int32_t { return lv_scale_get_range_max_value(self.get()); }
        );
    obj_cls.def("slider_set_value", [](LvObjWrapper &self, int32_t value, bool anim) { lv_slider_set_value(self.get(), value, anim); return; }
        , py::arg("value"), py::arg("anim"));
    obj_cls.def("slider_set_start_value", [](LvObjWrapper &self, int32_t value, bool anim) { lv_slider_set_start_value(self.get(), value, anim); return; }
        , py::arg("value"), py::arg("anim"));
    obj_cls.def("slider_set_range", [](LvObjWrapper &self, int32_t min, int32_t max) { lv_slider_set_range(self.get(), min, max); return; }
        , py::arg("min"), py::arg("max"));
    obj_cls.def("slider_set_min_value", [](LvObjWrapper &self, int32_t min) { lv_slider_set_min_value(self.get(), min); return; }
        , py::arg("min"));
    obj_cls.def("slider_set_max_value", [](LvObjWrapper &self, int32_t max) { lv_slider_set_max_value(self.get(), max); return; }
        , py::arg("max"));
    obj_cls.def("slider_set_mode", [](LvObjWrapper &self, lv_slider_mode_t mode) { lv_slider_set_mode(self.get(), mode); return; }
        , py::arg("mode"));
    obj_cls.def("slider_set_orientation", [](LvObjWrapper &self, lv_slider_orientation_t orientation) { lv_slider_set_orientation(self.get(), orientation); return; }
        , py::arg("orientation"));
    obj_cls.def("slider_get_value", [](LvObjWrapper &self) -> int32_t { return lv_slider_get_value(self.get()); }
        );
    obj_cls.def("get_left_value", [](LvObjWrapper &self) -> int32_t { return lv_slider_get_left_value(self.get()); }
        );
    obj_cls.def("slider_get_min_value", [](LvObjWrapper &self) -> int32_t { return lv_slider_get_min_value(self.get()); }
        );
    obj_cls.def("slider_get_max_value", [](LvObjWrapper &self) -> int32_t { return lv_slider_get_max_value(self.get()); }
        );
    obj_cls.def("is_dragged", [](LvObjWrapper &self) -> bool { return lv_slider_is_dragged(self.get()); }
        );
    obj_cls.def("slider_get_mode", [](LvObjWrapper &self) -> lv_slider_mode_t { return lv_slider_get_mode(self.get()); }
        );
    obj_cls.def("slider_get_orientation", [](LvObjWrapper &self) -> lv_slider_orientation_t { return lv_slider_get_orientation(self.get()); }
        );
    obj_cls.def("slider_is_symmetrical", [](LvObjWrapper &self) -> bool { return lv_slider_is_symmetrical(self.get()); }
        );
    obj_cls.def("spinbox_set_value", [](LvObjWrapper &self, int32_t v) { lv_spinbox_set_value(self.get(), v); return; }
        , py::arg("v"));
    obj_cls.def("set_rollover", [](LvObjWrapper &self, bool rollover) { lv_spinbox_set_rollover(self.get(), rollover); return; }
        , py::arg("rollover"));
    obj_cls.def("set_digit_format", [](LvObjWrapper &self, uint32_t digit_count, uint32_t sep_pos) { lv_spinbox_set_digit_format(self.get(), digit_count, sep_pos); return; }
        , py::arg("digit_count"), py::arg("sep_pos"));
    obj_cls.def("set_digit_count", [](LvObjWrapper &self, uint32_t digit_count) { lv_spinbox_set_digit_count(self.get(), digit_count); return; }
        , py::arg("digit_count"));
    obj_cls.def("set_dec_point_pos", [](LvObjWrapper &self, uint32_t dec_point_pos) { lv_spinbox_set_dec_point_pos(self.get(), dec_point_pos); return; }
        , py::arg("dec_point_pos"));
    obj_cls.def("set_step", [](LvObjWrapper &self, uint32_t step) { lv_spinbox_set_step(self.get(), step); return; }
        , py::arg("step"));
    obj_cls.def("spinbox_set_range", [](LvObjWrapper &self, int32_t min_value, int32_t max_value) { lv_spinbox_set_range(self.get(), min_value, max_value); return; }
        , py::arg("min_value"), py::arg("max_value"));
    obj_cls.def("spinbox_set_min_value", [](LvObjWrapper &self, int32_t min_value) { lv_spinbox_set_min_value(self.get(), min_value); return; }
        , py::arg("min_value"));
    obj_cls.def("spinbox_set_max_value", [](LvObjWrapper &self, int32_t max_value) { lv_spinbox_set_max_value(self.get(), max_value); return; }
        , py::arg("max_value"));
    obj_cls.def("spinbox_set_cursor_pos", [](LvObjWrapper &self, uint32_t pos) { lv_spinbox_set_cursor_pos(self.get(), pos); return; }
        , py::arg("pos"));
    obj_cls.def("set_digit_step_direction", [](LvObjWrapper &self, lv_dir_t direction) { lv_spinbox_set_digit_step_direction(self.get(), direction); return; }
        , py::arg("direction"));
    obj_cls.def("get_rollover", [](LvObjWrapper &self) -> bool { return lv_spinbox_get_rollover(self.get()); }
        );
    obj_cls.def("spinbox_get_value", [](LvObjWrapper &self) -> int32_t { return lv_spinbox_get_value(self.get()); }
        );
    obj_cls.def("get_step", [](LvObjWrapper &self) -> int32_t { return lv_spinbox_get_step(self.get()); }
        );
    obj_cls.def("get_digit_count", [](LvObjWrapper &self) -> uint32_t { return lv_spinbox_get_digit_count(self.get()); }
        );
    obj_cls.def("get_dec_point_pos", [](LvObjWrapper &self) -> uint32_t { return lv_spinbox_get_dec_point_pos(self.get()); }
        );
    obj_cls.def("spinbox_get_min_value", [](LvObjWrapper &self) -> int32_t { return lv_spinbox_get_min_value(self.get()); }
        );
    obj_cls.def("spinbox_get_max_value", [](LvObjWrapper &self) -> int32_t { return lv_spinbox_get_max_value(self.get()); }
        );
    obj_cls.def("get_digit_step_direction", [](LvObjWrapper &self) -> lv_dir_t { return lv_spinbox_get_digit_step_direction(self.get()); }
        );
    obj_cls.def("step_next", [](LvObjWrapper &self) { lv_spinbox_step_next(self.get()); return; }
        );
    obj_cls.def("step_prev", [](LvObjWrapper &self) { lv_spinbox_step_prev(self.get()); return; }
        );
    obj_cls.def("increment", [](LvObjWrapper &self) { lv_spinbox_increment(self.get()); return; }
        );
    obj_cls.def("decrement", [](LvObjWrapper &self) { lv_spinbox_decrement(self.get()); return; }
        );
    obj_cls.def("set_anim_params", [](LvObjWrapper &self, uint32_t t, uint32_t angle) { lv_spinner_set_anim_params(self.get(), t, angle); return; }
        , py::arg("t"), py::arg("angle"));
    obj_cls.def("set_anim_duration", [](LvObjWrapper &self, uint32_t t) { lv_spinner_set_anim_duration(self.get(), t); return; }
        , py::arg("t"));
    obj_cls.def("set_arc_sweep", [](LvObjWrapper &self, uint32_t angle) { lv_spinner_set_arc_sweep(self.get(), angle); return; }
        , py::arg("angle"));
    obj_cls.def("get_anim_duration", [](LvObjWrapper &self) -> uint32_t { return lv_spinner_get_anim_duration(self.get()); }
        );
    obj_cls.def("get_arc_sweep", [](LvObjWrapper &self) -> uint32_t { return lv_spinner_get_arc_sweep(self.get()); }
        );
    obj_cls.def("switch_set_orientation", [](LvObjWrapper &self, lv_switch_orientation_t orientation) { lv_switch_set_orientation(self.get(), orientation); return; }
        , py::arg("orientation"));
    obj_cls.def("switch_get_orientation", [](LvObjWrapper &self) -> lv_switch_orientation_t { return lv_switch_get_orientation(self.get()); }
        );
    obj_cls.def("set_cell_value", [](LvObjWrapper &self, uint32_t row, uint32_t col, const char * txt) { lv_table_set_cell_value(self.get(), row, col, txt); return; }
        , py::arg("row"), py::arg("col"), py::arg("txt"));
    obj_cls.def("set_cell_value_fmt", [](LvObjWrapper &self, uint32_t row, uint32_t col, const char * fmt) { lv_table_set_cell_value_fmt(self.get(), row, col, fmt); return; }
        , py::arg("row"), py::arg("col"), py::arg("fmt"));
    obj_cls.def("set_row_count", [](LvObjWrapper &self, uint32_t row_cnt) { lv_table_set_row_count(self.get(), row_cnt); return; }
        , py::arg("row_cnt"));
    obj_cls.def("set_column_count", [](LvObjWrapper &self, uint32_t col_cnt) { lv_table_set_column_count(self.get(), col_cnt); return; }
        , py::arg("col_cnt"));
    obj_cls.def("set_column_width", [](LvObjWrapper &self, uint32_t col_id, int32_t w) { lv_table_set_column_width(self.get(), col_id, w); return; }
        , py::arg("col_id"), py::arg("w"));
    obj_cls.def("set_cell_ctrl", [](LvObjWrapper &self, uint32_t row, uint32_t col, lv_table_cell_ctrl_t ctrl) { lv_table_set_cell_ctrl(self.get(), row, col, ctrl); return; }
        , py::arg("row"), py::arg("col"), py::arg("ctrl"));
    obj_cls.def("clear_cell_ctrl", [](LvObjWrapper &self, uint32_t row, uint32_t col, lv_table_cell_ctrl_t ctrl) { lv_table_clear_cell_ctrl(self.get(), row, col, ctrl); return; }
        , py::arg("row"), py::arg("col"), py::arg("ctrl"));
    obj_cls.def("set_cell_user_data", [](LvObjWrapper &self, uint16_t row, uint16_t col, void * user_data) { lv_table_set_cell_user_data(self.get(), row, col, user_data); return; }
        , py::arg("row"), py::arg("col"), py::arg("user_data"));
    obj_cls.def("set_selected_cell", [](LvObjWrapper &self, uint16_t row, uint16_t col) { lv_table_set_selected_cell(self.get(), row, col); return; }
        , py::arg("row"), py::arg("col"));
    obj_cls.def("get_cell_value", [](LvObjWrapper &self, uint32_t row, uint32_t col) -> const char * { return lv_table_get_cell_value(self.get(), row, col); }
        , py::arg("row"), py::arg("col"));
    obj_cls.def("get_row_count", [](LvObjWrapper &self) -> uint32_t { return lv_table_get_row_count(self.get()); }
        );
    obj_cls.def("get_column_count", [](LvObjWrapper &self) -> uint32_t { return lv_table_get_column_count(self.get()); }
        );
    obj_cls.def("get_column_width", [](LvObjWrapper &self, uint32_t col) -> int32_t { return lv_table_get_column_width(self.get(), col); }
        , py::arg("col"));
    obj_cls.def("has_cell_ctrl", [](LvObjWrapper &self, uint32_t row, uint32_t col, lv_table_cell_ctrl_t ctrl) -> bool { return lv_table_has_cell_ctrl(self.get(), row, col, ctrl); }
        , py::arg("row"), py::arg("col"), py::arg("ctrl"));
    obj_cls.def("get_selected_cell", [](LvObjWrapper &self, uint32_t * row, uint32_t * col) { lv_table_get_selected_cell(self.get(), row, col); return; }
        , py::arg("row"), py::arg("col"));
    obj_cls.def("get_cell_user_data", [](LvObjWrapper &self, uint16_t row, uint16_t col) -> void * { return lv_table_get_cell_user_data(self.get(), row, col); }
        , py::arg("row"), py::arg("col"));
    obj_cls.def("add_tab", [](LvObjWrapper &self, const char * name) -> LvObjWrapper* {
        lv_obj_t *result = lv_tabview_add_tab(self.get(), name);
        return result ? new LvObjWrapper(result) : nullptr;
    }
        , py::arg("name"));
    obj_cls.def("set_tab_text", [](LvObjWrapper &self, uint32_t idx, const char * new_name) { lv_tabview_set_tab_text(self.get(), idx, new_name); return; }
        , py::arg("idx"), py::arg("new_name"));
    obj_cls.def("set_active", [](LvObjWrapper &self, uint32_t idx, bool anim_en) { lv_tabview_set_active(self.get(), idx, anim_en); return; }
        , py::arg("idx"), py::arg("anim_en"));
    obj_cls.def("set_tab_bar_position", [](LvObjWrapper &self, lv_dir_t dir) { lv_tabview_set_tab_bar_position(self.get(), dir); return; }
        , py::arg("dir"));
    obj_cls.def("set_tab_bar_size", [](LvObjWrapper &self, int32_t size) { lv_tabview_set_tab_bar_size(self.get(), size); return; }
        , py::arg("size"));
    obj_cls.def("get_tab_count", [](LvObjWrapper &self) -> uint32_t { return lv_tabview_get_tab_count(self.get()); }
        );
    obj_cls.def("get_tab_active", [](LvObjWrapper &self) -> uint32_t { return lv_tabview_get_tab_active(self.get()); }
        );
    obj_cls.def("get_tab_button", [](LvObjWrapper &self, int32_t idx) -> LvObjWrapper* {
        lv_obj_t *result = lv_tabview_get_tab_button(self.get(), idx);
        return result ? new LvObjWrapper(result) : nullptr;
    }
        , py::arg("idx"));
    obj_cls.def("tabview_get_content", [](LvObjWrapper &self) -> LvObjWrapper* {
        lv_obj_t *result = lv_tabview_get_content(self.get());
        return result ? new LvObjWrapper(result) : nullptr;
    }
        );
    obj_cls.def("get_tab_bar", [](LvObjWrapper &self) -> LvObjWrapper* {
        lv_obj_t *result = lv_tabview_get_tab_bar(self.get());
        return result ? new LvObjWrapper(result) : nullptr;
    }
        );
    obj_cls.def("get_tab_bar_position", [](LvObjWrapper &self) -> lv_dir_t { return lv_tabview_get_tab_bar_position(self.get()); }
        );
    obj_cls.def("add_char", [](LvObjWrapper &self, uint32_t c) { lv_textarea_add_char(self.get(), c); return; }
        , py::arg("c"));
    obj_cls.def("textarea_add_text", [](LvObjWrapper &self, const char * txt) { lv_textarea_add_text(self.get(), txt); return; }
        , py::arg("txt"));
    obj_cls.def("delete_char", [](LvObjWrapper &self) { lv_textarea_delete_char(self.get()); return; }
        );
    obj_cls.def("delete_char_forward", [](LvObjWrapper &self) { lv_textarea_delete_char_forward(self.get()); return; }
        );
    obj_cls.def("textarea_set_text", [](LvObjWrapper &self, const char * txt) { lv_textarea_set_text(self.get(), txt); return; }
        , py::arg("txt"));
    obj_cls.def("set_placeholder_text", [](LvObjWrapper &self, const char * txt) { lv_textarea_set_placeholder_text(self.get(), txt); return; }
        , py::arg("txt"));
    obj_cls.def("textarea_set_cursor_pos", [](LvObjWrapper &self, int32_t pos) { lv_textarea_set_cursor_pos(self.get(), pos); return; }
        , py::arg("pos"));
    obj_cls.def("set_cursor_click_pos", [](LvObjWrapper &self, bool en) { lv_textarea_set_cursor_click_pos(self.get(), en); return; }
        , py::arg("en"));
    obj_cls.def("set_password_mode", [](LvObjWrapper &self, bool en) { lv_textarea_set_password_mode(self.get(), en); return; }
        , py::arg("en"));
    obj_cls.def("set_password_bullet", [](LvObjWrapper &self, const char * bullet) { lv_textarea_set_password_bullet(self.get(), bullet); return; }
        , py::arg("bullet"));
    obj_cls.def("set_one_line", [](LvObjWrapper &self, bool en) { lv_textarea_set_one_line(self.get(), en); return; }
        , py::arg("en"));
    obj_cls.def("set_accepted_chars", [](LvObjWrapper &self, const char * list) { lv_textarea_set_accepted_chars(self.get(), list); return; }
        , py::arg("list"));
    obj_cls.def("set_accepted_chars_static", [](LvObjWrapper &self, const char * list) { lv_textarea_set_accepted_chars_static(self.get(), list); return; }
        , py::arg("list"));
    obj_cls.def("set_max_length", [](LvObjWrapper &self, uint32_t num) { lv_textarea_set_max_length(self.get(), num); return; }
        , py::arg("num"));
    obj_cls.def("set_insert_replace", [](LvObjWrapper &self, const char * txt) { lv_textarea_set_insert_replace(self.get(), txt); return; }
        , py::arg("txt"));
    obj_cls.def("set_text_selection", [](LvObjWrapper &self, bool en) { lv_textarea_set_text_selection(self.get(), en); return; }
        , py::arg("en"));
    obj_cls.def("set_password_show_time", [](LvObjWrapper &self, uint32_t time) { lv_textarea_set_password_show_time(self.get(), time); return; }
        , py::arg("time"));
    obj_cls.def("set_align", [](LvObjWrapper &self, lv_text_align_t align) { lv_textarea_set_align(self.get(), align); return; }
        , py::arg("align"));
    obj_cls.def("get_placeholder_text", [](LvObjWrapper &self) -> const char * { return lv_textarea_get_placeholder_text(self.get()); }
        );
    obj_cls.def("get_label", [](LvObjWrapper &self) -> LvObjWrapper* {
        lv_obj_t *result = lv_textarea_get_label(self.get());
        return result ? new LvObjWrapper(result) : nullptr;
    }
        );
    obj_cls.def("get_cursor_pos", [](LvObjWrapper &self) -> uint32_t { return lv_textarea_get_cursor_pos(self.get()); }
        );
    obj_cls.def("get_cursor_click_pos", [](LvObjWrapper &self) -> bool { return lv_textarea_get_cursor_click_pos(self.get()); }
        );
    obj_cls.def("get_password_mode", [](LvObjWrapper &self) -> bool { return lv_textarea_get_password_mode(self.get()); }
        );
    obj_cls.def("get_password_bullet", [](LvObjWrapper &self) -> const char * { return lv_textarea_get_password_bullet(self.get()); }
        );
    obj_cls.def("get_one_line", [](LvObjWrapper &self) -> bool { return lv_textarea_get_one_line(self.get()); }
        );
    obj_cls.def("get_accepted_chars", [](LvObjWrapper &self) -> const char * { return lv_textarea_get_accepted_chars(self.get()); }
        );
    obj_cls.def("get_max_length", [](LvObjWrapper &self) -> uint32_t { return lv_textarea_get_max_length(self.get()); }
        );
    obj_cls.def("text_is_selected", [](LvObjWrapper &self) -> bool { return lv_textarea_text_is_selected(self.get()); }
        );
    obj_cls.def("get_text_selection", [](LvObjWrapper &self) -> bool { return lv_textarea_get_text_selection(self.get()); }
        );
    obj_cls.def("get_password_show_time", [](LvObjWrapper &self) -> uint32_t { return lv_textarea_get_password_show_time(self.get()); }
        );
    obj_cls.def("get_current_char", [](LvObjWrapper &self) -> uint32_t { return lv_textarea_get_current_char(self.get()); }
        );
    obj_cls.def("clear_selection", [](LvObjWrapper &self) { lv_textarea_clear_selection(self.get()); return; }
        );
    obj_cls.def("cursor_right", [](LvObjWrapper &self) { lv_textarea_cursor_right(self.get()); return; }
        );
    obj_cls.def("cursor_left", [](LvObjWrapper &self) { lv_textarea_cursor_left(self.get()); return; }
        );
    obj_cls.def("cursor_down", [](LvObjWrapper &self) { lv_textarea_cursor_down(self.get()); return; }
        );
    obj_cls.def("cursor_up", [](LvObjWrapper &self) { lv_textarea_cursor_up(self.get()); return; }
        );
    obj_cls.def("add_tile", [](LvObjWrapper &self, uint8_t col_id, uint8_t row_id, lv_dir_t dir) -> LvObjWrapper* {
        lv_obj_t *result = lv_tileview_add_tile(self.get(), col_id, row_id, dir);
        return result ? new LvObjWrapper(result) : nullptr;
    }
        , py::arg("col_id"), py::arg("row_id"), py::arg("dir"));
    obj_cls.def("set_tile", [](LvObjWrapper &self, LvObjWrapper & tile_obj, bool anim_en) { lv_tileview_set_tile(self.get(), tile_obj.get(), anim_en); return; }
        , py::arg("tile_obj"), py::arg("anim_en"));
    obj_cls.def("set_tile_by_index", [](LvObjWrapper &self, uint32_t col_id, uint32_t row_id, bool anim_en) { lv_tileview_set_tile_by_index(self.get(), col_id, row_id, anim_en); return; }
        , py::arg("col_id"), py::arg("row_id"), py::arg("anim_en"));
    obj_cls.def("get_tile_active", [](LvObjWrapper &self) -> LvObjWrapper* {
        lv_obj_t *result = lv_tileview_get_tile_active(self.get());
        return result ? new LvObjWrapper(result) : nullptr;
    }
        );
    obj_cls.def("win_add_title", [](LvObjWrapper &self, const char * txt) -> LvObjWrapper* {
        lv_obj_t *result = lv_win_add_title(self.get(), txt);
        return result ? new LvObjWrapper(result) : nullptr;
    }
        , py::arg("txt"));
    obj_cls.def("win_add_button", [](LvObjWrapper &self, const char * icon, int32_t btn_w) -> LvObjWrapper* {
        lv_obj_t *result = lv_win_add_button(self.get(), (const void *)icon, btn_w);
        return result ? new LvObjWrapper(result) : nullptr;
    }
        , py::arg("icon"), py::arg("btn_w"));
    obj_cls.def("win_get_header", [](LvObjWrapper &self) -> LvObjWrapper* {
        lv_obj_t *result = lv_win_get_header(self.get());
        return result ? new LvObjWrapper(result) : nullptr;
    }
        );
    obj_cls.def("win_get_content", [](LvObjWrapper &self) -> LvObjWrapper* {
        lv_obj_t *result = lv_win_get_content(self.get());
        return result ? new LvObjWrapper(result) : nullptr;
    }
        );

    /* String overloads for icon/image source methods */

    /* Chart: manual bindings for series and data (series pointer stored as user_data) */
    obj_cls.def("set_point_count", [](LvObjWrapper &self, uint32_t cnt) {
        lv_chart_set_point_count(self.get(), cnt);
    }, py::arg("cnt"), "Set the number of points on a chart series");
    obj_cls.def("add_series", [](LvObjWrapper &self, lv_color_t color, lv_chart_axis_t axis) -> uintptr_t {
        lv_chart_series_t *ser = lv_chart_add_series(self.get(), color, axis);
        return reinterpret_cast<uintptr_t>(ser);
    }, py::arg("color"), py::arg("axis"), "Add a data series to chart, returns series handle as int");
    obj_cls.def("set_next_value", [](LvObjWrapper &self, uintptr_t series, int32_t value) {
        lv_chart_series_t *ser = reinterpret_cast<lv_chart_series_t*>(series);
        lv_chart_set_next_value(self.get(), ser, value);
    }, py::arg("series"), py::arg("value"), "Set next value on a chart series");
    obj_cls.def("set_next_value2", [](LvObjWrapper &self, uintptr_t series, int32_t x_value, int32_t y_value) {
        lv_chart_series_t *ser = reinterpret_cast<lv_chart_series_t*>(series);
        lv_chart_set_next_value2(self.get(), ser, x_value, y_value);
    }, py::arg("series"), py::arg("x_value"), py::arg("y_value"), "Set next X/Y value on a chart series (scatter)");
    obj_cls.def("refresh_chart", [](LvObjWrapper &self) {
        lv_chart_refresh(self.get());
    }, "Refresh chart after data change");
    obj_cls.def("set_series_color", [](LvObjWrapper &self, uintptr_t series, lv_color_t color) {
        lv_chart_series_t *ser = reinterpret_cast<lv_chart_series_t*>(series);
        lv_chart_set_series_color(self.get(), ser, color);
    }, py::arg("series"), py::arg("color"), "Set series color");
    obj_cls.def("set_all_values", [](LvObjWrapper &self, uintptr_t series, int32_t value) {
        lv_chart_series_t *ser = reinterpret_cast<lv_chart_series_t*>(series);
        lv_chart_set_all_values(self.get(), ser, value);
    }, py::arg("series"), py::arg("value"), "Set all values of a series to the same value");

    /* Event callback support */
    obj_cls.def("add_event_cb", [](LvObjWrapper &self, int filter, py::function callback) {
        register_event_callback(self.get(), (lv_event_code_t)filter, callback);
    }, py::arg("filter"), py::arg("callback"));

    /* Color type */
    py::class_<lv_color_t>(m, "Color")
        .def(py::init<>())
        .def_static("from_rgb", [](uint8_t r, uint8_t g, uint8_t b) -> lv_color_t {
            return lv_color_make(r, g, b);
        }, py::arg("r"), py::arg("g"), py::arg("b"))
        .def_static("from_hex", [](uint32_t c) -> lv_color_t {
            return lv_color_hex(c);
        }, py::arg("c"))
        .def_readonly("red", &lv_color_t::red)
        .def_readonly("green", &lv_color_t::green)
        .def_readonly("blue", &lv_color_t::blue);

    /* Color utilities */
    m.def("color_make", [](uint8_t r, uint8_t g, uint8_t b) { return lv_color_make(r, g, b); },
        py::arg("r"), py::arg("g"), py::arg("b"));
    m.def("color_hex", [](uint32_t c) { return lv_color_hex(c); }, py::arg("c"));
    m.def("color_black", []() { return lv_color_black(); });
    m.def("color_white", []() { return lv_color_white(); });

    /* Display */
    py::class_<LvDisplayWrapper>(m, "Display")
        .def(py::init<>())
        .def("set_resolution", [](LvDisplayWrapper &self, int32_t h, int32_t v) {
            lv_display_set_resolution(self.get(), h, v);
        }, py::arg("hor_res"), py::arg("ver_res"))
        .def("set_rotation", [](LvDisplayWrapper &self, lv_display_rotation_t r) {
            lv_display_set_rotation(self.get(), r);
        }, py::arg("rotation"))
        .def("get_horizontal_resolution", [](LvDisplayWrapper &self) -> int32_t {
            return lv_display_get_horizontal_resolution(self.get());
        })
        .def("get_vertical_resolution", [](LvDisplayWrapper &self) -> int32_t {
            return lv_display_get_vertical_resolution(self.get());
        })
        .def("flush_ready", [](LvDisplayWrapper &self) {
            lv_display_flush_ready(self.get());
        });

    m.def("display_create", [](int32_t h, int32_t v) -> LvDisplayWrapper* {
        return new LvDisplayWrapper(lv_display_create(h, v));
    }, py::arg("hor_res"), py::arg("ver_res"));
    m.def("display_get_default", []() -> LvDisplayWrapper* {
        return new LvDisplayWrapper(lv_display_get_default());
    });

    /* Input device */
    py::class_<LvIndevWrapper>(m, "Indev")
        .def(py::init<>());

    m.def("indev_create", []() -> LvIndevWrapper* {
        return new LvIndevWrapper(lv_indev_create());
    });

    /* FreeType font support */
#if LV_USE_FREETYPE
    auto font_cls = py::class_<LvFontWrapper>(m, "Font", py::dynamic_attr());
    font_cls.def(py::init<>());
    font_cls.def("is_valid", &LvFontWrapper::is_valid);

    m.def("freetype_font_create", [](const char *pathname,
                                     lv_freetype_font_render_mode_t render_mode,
                                     uint32_t size,
                                     lv_freetype_font_style_t style) -> LvFontWrapper* {
        lv_font_t *font = lv_freetype_font_create(pathname, render_mode, size, style);
        if(!font) {
            PyErr_SetString(PyExc_RuntimeError,
                            "freetype_font_create failed - check font path");
            throw py::error_already_set();
        }
        return new LvFontWrapper(font, true);
    }, py::arg("pathname"), py::arg("render_mode"), py::arg("size"), py::arg("style"));

    m.def("freetype_font_delete", [](LvFontWrapper &font) {
        lv_freetype_font_delete(font.get());
    });

    /* LvFontWrapper overload for set_style_text_font */
    obj_cls.def("set_style_text_font", [](LvObjWrapper &self, LvFontWrapper &font, uint32_t selector) {
        lv_obj_set_style_text_font(self.get(), font.get(), selector); return;
    }, py::arg("value"), py::arg("selector") = 0);
#endif /* LV_USE_FREETYPE */

    /* Driver backend functions */
    m.def("driver_backends_register", []() { driver_backends_register(); },
        "Register all available driver backends");
    m.def("driver_backends_init_backend", [](const std::string &name) -> int {
        return driver_backends_init_backend(const_cast<char*>(name.c_str()));
    }, py::arg("backend_name"),
        "Initialize a driver backend by name");
    m.def("driver_backends_is_supported", [](const std::string &name) -> int {
        return driver_backends_is_supported(const_cast<char*>(name.c_str()));
    }, py::arg("backend_name"),
        "Check if a backend is supported");
    m.def("driver_backends_print_supported", []() -> int {
        return driver_backends_print_supported();
    }, "Print supported backends");
    m.def("driver_backends_run_loop", []() { driver_backends_run_loop(); },
        "Enter the run loop");
    m.def("k230_driver_init", [](uintptr_t display_ptr, int v4l2_drm_run_flag) -> int {
        return k230_driver_init(reinterpret_cast<void*>(display_ptr), static_cast<char>(v4l2_drm_run_flag));
    }, py::arg("display_ptr"), py::arg("v4l2_drm_run_flag"),
        "Initialize K230 display driver with v4l2-drm backend");


    /* Functions not generated:
     * lv_anim_init (unsupported param type: lv_anim_t *)
     * lv_anim_set_var (unsupported param type: lv_anim_t *)
     * lv_anim_set_exec_cb (unsupported param type: lv_anim_t *)
     * lv_anim_set_duration (unsupported param type: lv_anim_t *)
     * lv_anim_set_delay (unsupported param type: lv_anim_t *)
     * lv_anim_resume (unsupported param type: lv_anim_t *)
     * lv_anim_pause (unsupported param type: lv_anim_t *)
     * lv_anim_pause_for (unsupported param type: lv_anim_t *)
     * lv_anim_is_paused (unsupported param type: lv_anim_t *)
     * lv_anim_set_values (unsupported param type: lv_anim_t *)
     * lv_anim_set_custom_exec_cb (unsupported param type: lv_anim_t *)
     * lv_anim_set_path_cb (unsupported param type: lv_anim_t *)
     * lv_anim_set_start_cb (unsupported param type: lv_anim_t *)
     * lv_anim_set_get_value_cb (unsupported param type: lv_anim_t *)
     * lv_anim_set_completed_cb (unsupported param type: lv_anim_t *)
     * lv_anim_set_deleted_cb (unsupported param type: lv_anim_t *)
     * lv_anim_set_reverse_duration (unsupported param type: lv_anim_t *)
     * lv_anim_set_reverse_time (unsupported param type: lv_anim_t *)
     * lv_anim_set_reverse_delay (unsupported param type: lv_anim_t *)
     * lv_anim_set_repeat_count (unsupported param type: lv_anim_t *)
     * lv_anim_set_repeat_delay (unsupported param type: lv_anim_t *)
     * lv_anim_set_early_apply (unsupported param type: lv_anim_t *)
     * lv_anim_set_user_data (unsupported param type: lv_anim_t *)
     * lv_anim_set_bezier3_param (unsupported param type: lv_anim_t *)
     * lv_anim_start (unsupported param type: const lv_anim_t *)
     * lv_anim_get_delay (unsupported param type: const lv_anim_t *)
     * lv_anim_get_playtime (unsupported param type: const lv_anim_t *)
     * lv_anim_get_time (unsupported param type: const lv_anim_t *)
     * lv_anim_get_repeat_count (unsupported param type: const lv_anim_t *)
     * lv_anim_get_user_data (unsupported param type: const lv_anim_t *)
     * lv_anim_get (unsupported return type: lv_anim_t *)
     * lv_anim_get_timer (unsupported return type: lv_timer_t *)
     * lv_anim_custom_delete (unsupported param type: lv_anim_t *)
     * lv_anim_custom_get (unsupported param type: lv_anim_t *)
     * lv_anim_path_linear (unsupported param type: const lv_anim_t *)
     * lv_anim_path_ease_in (unsupported param type: const lv_anim_t *)
     * lv_anim_path_ease_out (unsupported param type: const lv_anim_t *)
     * lv_anim_path_ease_in_out (unsupported param type: const lv_anim_t *)
     * lv_anim_path_overshoot (unsupported param type: const lv_anim_t *)
     * lv_anim_path_bounce (unsupported param type: const lv_anim_t *)
     * lv_anim_path_step (unsupported param type: const lv_anim_t *)
     * lv_anim_path_custom_bezier3 (unsupported param type: const lv_anim_t *)
     * lv_anim_timeline_create (unsupported return type: lv_anim_timeline_t *)
     * lv_anim_timeline_delete (unsupported param type: lv_anim_timeline_t *)
     * lv_anim_timeline_add (unsupported param type: lv_anim_timeline_t *)
     * lv_anim_timeline_start (unsupported param type: lv_anim_timeline_t *)
     * lv_anim_timeline_pause (unsupported param type: lv_anim_timeline_t *)
     * lv_anim_timeline_set_reverse (unsupported param type: lv_anim_timeline_t *)
     * lv_anim_timeline_set_delay (unsupported param type: lv_anim_timeline_t *)
     * lv_anim_timeline_set_repeat_count (unsupported param type: lv_anim_timeline_t *)
     * lv_anim_timeline_set_repeat_delay (unsupported param type: lv_anim_timeline_t *)
     * lv_anim_timeline_set_progress (unsupported param type: lv_anim_timeline_t *)
     * lv_anim_timeline_set_user_data (unsupported param type: lv_anim_timeline_t *)
     * lv_anim_timeline_get_playtime (unsupported param type: lv_anim_timeline_t *)
     * lv_anim_timeline_get_reverse (unsupported param type: lv_anim_timeline_t *)
     * lv_anim_timeline_get_delay (unsupported param type: lv_anim_timeline_t *)
     * lv_anim_timeline_get_progress (unsupported param type: lv_anim_timeline_t *)
     * lv_anim_timeline_get_repeat_count (unsupported param type: lv_anim_timeline_t *)
     * lv_anim_timeline_get_repeat_delay (unsupported param type: lv_anim_timeline_t *)
     * lv_anim_timeline_get_user_data (unsupported param type: lv_anim_timeline_t *)
     * lv_anim_timeline_merge (unsupported param type: lv_anim_timeline_t *)
     * lv_event_send (unsupported param type: lv_event_t *)
     * lv_event_add (unsupported return type: lv_event_dsc_t *)
     * lv_event_remove_dsc (unsupported param type: lv_event_dsc_t *)
     * lv_event_get_dsc (unsupported return type: lv_event_dsc_t *)
     * lv_event_dsc_get_cb (unsupported param type: lv_event_dsc_t *)
     * lv_event_dsc_get_user_data (unsupported param type: lv_event_dsc_t *)
     * lv_event_get_target (unsupported param type: lv_event_t *)
     * lv_event_get_current_target (unsupported param type: lv_event_t *)
     * lv_event_get_code (unsupported param type: lv_event_t *)
     * lv_event_get_param (unsupported param type: lv_event_t *)
     * lv_event_get_user_data (unsupported param type: lv_event_t *)
     * lv_event_stop_bubbling (unsupported param type: lv_event_t *)
     * lv_event_stop_trickling (unsupported param type: lv_event_t *)
     * lv_event_stop_processing (unsupported param type: lv_event_t *)
     * lv_event_free_user_data_cb (unsupported param type: lv_event_t *)
     * lv_group_create (unsupported return type: lv_group_t *)
     * lv_group_delete (unsupported param type: lv_group_t *)
     * lv_group_set_default (unsupported param type: lv_group_t *)
     * lv_group_get_default (unsupported return type: lv_group_t *)
     * lv_group_add_obj (unsupported param type: lv_group_t *)
     * lv_group_remove_all_objs (unsupported param type: lv_group_t *)
     * lv_group_focus_next (unsupported param type: lv_group_t *)
     * lv_group_focus_prev (unsupported param type: lv_group_t *)
     * lv_group_focus_freeze (unsupported param type: lv_group_t *)
     * lv_group_send_data (unsupported param type: lv_group_t *)
     * lv_group_set_focus_cb (unsupported param type: lv_group_t *)
     * lv_group_set_edge_cb (unsupported param type: lv_group_t *)
     * lv_group_set_refocus_policy (unsupported param type: lv_group_t *)
     * lv_group_set_editing (unsupported param type: lv_group_t *)
     * lv_group_set_wrap (unsupported param type: lv_group_t *)
     * lv_group_get_focused (unsupported param type: const lv_group_t *)
     * lv_group_get_focus_cb (unsupported param type: const lv_group_t *)
     * lv_group_get_edge_cb (unsupported param type: const lv_group_t *)
     * lv_group_get_editing (unsupported param type: const lv_group_t *)
     * lv_group_get_wrap (unsupported param type: lv_group_t *)
     * lv_group_get_obj_count (unsupported param type: lv_group_t *)
     * lv_group_get_obj_by_index (unsupported param type: lv_group_t *)
     * lv_group_by_index (unsupported return type: lv_group_t *)
     * lv_group_set_user_data (unsupported param type: lv_group_t *)
     * lv_group_get_user_data (unsupported param type: const lv_group_t *)
     * lv_log_register_print_cb (manual skip rule)
     * lv_log (manual skip rule)
     * lv_log_add (manual skip rule)
     * lv_mem_add_pool (manual skip rule)
     * lv_mem_remove_pool (manual skip rule)
     * lv_malloc (manual skip rule)
     * lv_calloc (manual skip rule)
     * lv_zalloc (manual skip rule)
     * lv_malloc_zeroed (manual skip rule)
     * lv_free (manual skip rule)
     * lv_realloc (manual skip rule)
     * lv_reallocf (manual skip rule)
     * lv_malloc_core (manual skip rule)
     * lv_realloc_core (manual skip rule)
     * lv_color16_luminance (unsupported param type: const lv_color16_t)
     * lv_color_filter_dsc_init (unsupported param type: lv_color_filter_dsc_t *)
     * lv_memcpy (manual skip rule)
     * lv_memmove (manual skip rule)
     * lv_memcmp (manual skip rule)
     * lv_strlen (manual skip rule)
     * lv_strnlen (manual skip rule)
     * lv_strlcpy (manual skip rule)
     * lv_strncpy (manual skip rule)
     * lv_strcpy (manual skip rule)
     * lv_strcmp (manual skip rule)
     * lv_strncmp (manual skip rule)
     * lv_strdup (manual skip rule)
     * lv_strndup (manual skip rule)
     * lv_strcat (manual skip rule)
     * lv_strncat (manual skip rule)
     * lv_strchr (manual skip rule)
     * lv_draw_buf_init_with_default_handlers (manual skip rule)
     * lv_draw_buf_handlers_init (manual skip rule)
     * lv_draw_buf_get_handlers (manual skip rule)
     * lv_draw_buf_get_font_handlers (manual skip rule)
     * lv_draw_buf_get_image_handlers (manual skip rule)
     * lv_image_buf_set_palette (manual skip rule)
     * lv_image_buf_free (manual skip rule)
     * lv_snprintf (manual skip rule)
     * lv_vsnprintf (manual skip rule)
     * lv_grad_init_stops (unsupported param type: lv_grad_dsc_t *)
     * lv_grad_horizontal_init (unsupported param type: lv_grad_dsc_t *)
     * lv_grad_vertical_init (unsupported param type: lv_grad_dsc_t *)
     * lv_grad_linear_init (unsupported param type: lv_grad_dsc_t *)
     * lv_grad_radial_init (unsupported param type: lv_grad_dsc_t *)
     * lv_grad_radial_set_focal (unsupported param type: lv_grad_dsc_t *)
     * lv_grad_conical_init (unsupported param type: lv_grad_dsc_t *)
     * lv_obj_set_grid_dsc_array (unsupported param type: const int32_t[])
     * lv_style_init (unsupported param type: lv_style_t *)
     * lv_style_reset (unsupported param type: lv_style_t *)
     * lv_style_copy (unsupported param type: lv_style_t *)
     * lv_style_merge (unsupported param type: lv_style_t *)
     * lv_style_is_const (unsupported param type: const lv_style_t *)
     * lv_style_remove_prop (unsupported param type: lv_style_t *)
     * lv_style_set_prop (unsupported param type: lv_style_t *)
     * lv_style_get_prop (unsupported param type: const lv_style_t *)
     * lv_style_transition_dsc_init (unsupported param type: lv_style_transition_dsc_t *)
     * lv_style_get_prop_inlined (unsupported param type: const lv_style_t *)
     * lv_style_is_empty (unsupported param type: const lv_style_t *)
     * lv_style_set_width (unsupported param type: lv_style_t *)
     * lv_style_set_min_width (unsupported param type: lv_style_t *)
     * lv_style_set_max_width (unsupported param type: lv_style_t *)
     * lv_style_set_height (unsupported param type: lv_style_t *)
     * lv_style_set_min_height (unsupported param type: lv_style_t *)
     * lv_style_set_max_height (unsupported param type: lv_style_t *)
     * lv_style_set_length (unsupported param type: lv_style_t *)
     * lv_style_set_x (unsupported param type: lv_style_t *)
     * lv_style_set_y (unsupported param type: lv_style_t *)
     * lv_style_set_align (unsupported param type: lv_style_t *)
     * lv_style_set_transform_width (unsupported param type: lv_style_t *)
     * lv_style_set_transform_height (unsupported param type: lv_style_t *)
     * lv_style_set_translate_x (unsupported param type: lv_style_t *)
     * lv_style_set_translate_y (unsupported param type: lv_style_t *)
     * lv_style_set_translate_radial (unsupported param type: lv_style_t *)
     * lv_style_set_transform_scale_x (unsupported param type: lv_style_t *)
     * lv_style_set_transform_scale_y (unsupported param type: lv_style_t *)
     * lv_style_set_transform_rotation (unsupported param type: lv_style_t *)
     * lv_style_set_transform_pivot_x (unsupported param type: lv_style_t *)
     * lv_style_set_transform_pivot_y (unsupported param type: lv_style_t *)
     * lv_style_set_transform_skew_x (unsupported param type: lv_style_t *)
     * lv_style_set_transform_skew_y (unsupported param type: lv_style_t *)
     * lv_style_set_pad_top (unsupported param type: lv_style_t *)
     * lv_style_set_pad_bottom (unsupported param type: lv_style_t *)
     * lv_style_set_pad_left (unsupported param type: lv_style_t *)
     * lv_style_set_pad_right (unsupported param type: lv_style_t *)
     * lv_style_set_pad_row (unsupported param type: lv_style_t *)
     * lv_style_set_pad_column (unsupported param type: lv_style_t *)
     * lv_style_set_pad_radial (unsupported param type: lv_style_t *)
     * lv_style_set_margin_top (unsupported param type: lv_style_t *)
     * lv_style_set_margin_bottom (unsupported param type: lv_style_t *)
     * lv_style_set_margin_left (unsupported param type: lv_style_t *)
     * lv_style_set_margin_right (unsupported param type: lv_style_t *)
     * lv_style_set_bg_color (unsupported param type: lv_style_t *)
     * lv_style_set_bg_opa (unsupported param type: lv_style_t *)
     * lv_style_set_bg_grad_color (unsupported param type: lv_style_t *)
     * lv_style_set_bg_grad_dir (unsupported param type: lv_style_t *)
     * lv_style_set_bg_main_stop (unsupported param type: lv_style_t *)
     * lv_style_set_bg_grad_stop (unsupported param type: lv_style_t *)
     * lv_style_set_bg_main_opa (unsupported param type: lv_style_t *)
     * lv_style_set_bg_grad_opa (unsupported param type: lv_style_t *)
     * lv_style_set_bg_grad (unsupported param type: lv_style_t *)
     * lv_style_set_bg_image_src (unsupported param type: lv_style_t *)
     * lv_style_set_bg_image_opa (unsupported param type: lv_style_t *)
     * lv_style_set_bg_image_recolor (unsupported param type: lv_style_t *)
     * lv_style_set_bg_image_recolor_opa (unsupported param type: lv_style_t *)
     * lv_style_set_bg_image_tiled (unsupported param type: lv_style_t *)
     * lv_style_set_border_color (unsupported param type: lv_style_t *)
     * lv_style_set_border_opa (unsupported param type: lv_style_t *)
     * lv_style_set_border_width (unsupported param type: lv_style_t *)
     * lv_style_set_border_side (unsupported param type: lv_style_t *)
     * lv_style_set_border_post (unsupported param type: lv_style_t *)
     * lv_style_set_outline_width (unsupported param type: lv_style_t *)
     * lv_style_set_outline_color (unsupported param type: lv_style_t *)
     * lv_style_set_outline_opa (unsupported param type: lv_style_t *)
     * lv_style_set_outline_pad (unsupported param type: lv_style_t *)
     * lv_style_set_shadow_width (unsupported param type: lv_style_t *)
     * lv_style_set_shadow_offset_x (unsupported param type: lv_style_t *)
     * lv_style_set_shadow_offset_y (unsupported param type: lv_style_t *)
     * lv_style_set_shadow_spread (unsupported param type: lv_style_t *)
     * lv_style_set_shadow_color (unsupported param type: lv_style_t *)
     * lv_style_set_shadow_opa (unsupported param type: lv_style_t *)
     * lv_style_set_image_opa (unsupported param type: lv_style_t *)
     * lv_style_set_image_recolor (unsupported param type: lv_style_t *)
     * lv_style_set_image_recolor_opa (unsupported param type: lv_style_t *)
     * lv_style_set_image_colorkey (unsupported param type: lv_style_t *)
     * lv_style_set_line_width (unsupported param type: lv_style_t *)
     * lv_style_set_line_dash_width (unsupported param type: lv_style_t *)
     * lv_style_set_line_dash_gap (unsupported param type: lv_style_t *)
     * lv_style_set_line_rounded (unsupported param type: lv_style_t *)
     * lv_style_set_line_color (unsupported param type: lv_style_t *)
     * lv_style_set_line_opa (unsupported param type: lv_style_t *)
     * lv_style_set_arc_width (unsupported param type: lv_style_t *)
     * lv_style_set_arc_rounded (unsupported param type: lv_style_t *)
     * lv_style_set_arc_color (unsupported param type: lv_style_t *)
     * lv_style_set_arc_opa (unsupported param type: lv_style_t *)
     * lv_style_set_arc_image_src (unsupported param type: lv_style_t *)
     * lv_style_set_text_color (unsupported param type: lv_style_t *)
     * lv_style_set_text_opa (unsupported param type: lv_style_t *)
     * lv_style_set_text_font (unsupported param type: lv_style_t *)
     * lv_style_set_text_letter_space (unsupported param type: lv_style_t *)
     * lv_style_set_text_line_space (unsupported param type: lv_style_t *)
     * lv_style_set_text_decor (unsupported param type: lv_style_t *)
     * lv_style_set_text_align (unsupported param type: lv_style_t *)
     * lv_style_set_text_outline_stroke_color (unsupported param type: lv_style_t *)
     * lv_style_set_text_outline_stroke_width (unsupported param type: lv_style_t *)
     * lv_style_set_text_outline_stroke_opa (unsupported param type: lv_style_t *)
     * lv_style_set_text_leading_trim (unsupported param type: lv_style_t *)
     * lv_style_set_blur_radius (unsupported param type: lv_style_t *)
     * lv_style_set_blur_backdrop (unsupported param type: lv_style_t *)
     * lv_style_set_blur_quality (unsupported param type: lv_style_t *)
     * lv_style_set_drop_shadow_radius (unsupported param type: lv_style_t *)
     * lv_style_set_drop_shadow_offset_x (unsupported param type: lv_style_t *)
     * lv_style_set_drop_shadow_offset_y (unsupported param type: lv_style_t *)
     * lv_style_set_drop_shadow_color (unsupported param type: lv_style_t *)
     * lv_style_set_drop_shadow_opa (unsupported param type: lv_style_t *)
     * lv_style_set_drop_shadow_quality (unsupported param type: lv_style_t *)
     * lv_style_set_radius (unsupported param type: lv_style_t *)
     * lv_style_set_radial_offset (unsupported param type: lv_style_t *)
     * lv_style_set_clip_corner (unsupported param type: lv_style_t *)
     * lv_style_set_opa (unsupported param type: lv_style_t *)
     * lv_style_set_opa_layered (unsupported param type: lv_style_t *)
     * lv_style_set_color_filter_dsc (unsupported param type: lv_style_t *)
     * lv_style_set_color_filter_opa (unsupported param type: lv_style_t *)
     * lv_style_set_recolor (unsupported param type: lv_style_t *)
     * lv_style_set_recolor_opa (unsupported param type: lv_style_t *)
     * lv_style_set_anim (unsupported param type: lv_style_t *)
     * lv_style_set_anim_duration (unsupported param type: lv_style_t *)
     * lv_style_set_transition (unsupported param type: lv_style_t *)
     * lv_style_set_blend_mode (unsupported param type: lv_style_t *)
     * lv_style_set_layout (unsupported param type: lv_style_t *)
     * lv_style_set_base_dir (unsupported param type: lv_style_t *)
     * lv_style_set_bitmap_mask_src (unsupported param type: lv_style_t *)
     * lv_style_set_rotary_sensitivity (unsupported param type: lv_style_t *)
     * lv_style_set_flex_flow (unsupported param type: lv_style_t *)
     * lv_style_set_flex_main_place (unsupported param type: lv_style_t *)
     * lv_style_set_flex_cross_place (unsupported param type: lv_style_t *)
     * lv_style_set_flex_track_place (unsupported param type: lv_style_t *)
     * lv_style_set_flex_grow (unsupported param type: lv_style_t *)
     * lv_style_set_grid_column_dsc_array (unsupported param type: lv_style_t *)
     * lv_style_set_grid_column_align (unsupported param type: lv_style_t *)
     * lv_style_set_grid_row_dsc_array (unsupported param type: lv_style_t *)
     * lv_style_set_grid_row_align (unsupported param type: lv_style_t *)
     * lv_style_set_grid_cell_column_pos (unsupported param type: lv_style_t *)
     * lv_style_set_grid_cell_x_align (unsupported param type: lv_style_t *)
     * lv_style_set_grid_cell_column_span (unsupported param type: lv_style_t *)
     * lv_style_set_grid_cell_row_pos (unsupported param type: lv_style_t *)
     * lv_style_set_grid_cell_y_align (unsupported param type: lv_style_t *)
     * lv_style_set_grid_cell_row_span (unsupported param type: lv_style_t *)
     * lv_style_set_size (unsupported param type: lv_style_t *)
     * lv_style_set_pad_all (unsupported param type: lv_style_t *)
     * lv_style_set_pad_hor (unsupported param type: lv_style_t *)
     * lv_style_set_pad_ver (unsupported param type: lv_style_t *)
     * lv_style_set_pad_gap (unsupported param type: lv_style_t *)
     * lv_style_set_margin_hor (unsupported param type: lv_style_t *)
     * lv_style_set_margin_ver (unsupported param type: lv_style_t *)
     * lv_style_set_margin_all (unsupported param type: lv_style_t *)
     * lv_style_set_transform_scale (unsupported param type: lv_style_t *)
     * lv_timer_create_basic (unsupported return type: lv_timer_t *)
     * lv_timer_create (unsupported return type: lv_timer_t *)
     * lv_timer_delete (unsupported param type: lv_timer_t *)
     * lv_timer_pause (unsupported param type: lv_timer_t *)
     * lv_timer_resume (unsupported param type: lv_timer_t *)
     * lv_timer_set_cb (unsupported param type: lv_timer_t *)
     * lv_timer_set_period (unsupported param type: lv_timer_t *)
     * lv_timer_ready (unsupported param type: lv_timer_t *)
     * lv_timer_set_repeat_count (unsupported param type: lv_timer_t *)
     * lv_timer_set_auto_delete (unsupported param type: lv_timer_t *)
     * lv_timer_set_user_data (unsupported param type: lv_timer_t *)
     * lv_timer_reset (unsupported param type: lv_timer_t *)
     * lv_timer_get_next (unsupported param type: lv_timer_t *)
     * lv_timer_get_user_data (unsupported param type: lv_timer_t *)
     * lv_timer_get_paused (unsupported param type: lv_timer_t *)
     * lv_display_create (manual skip rule)
     * lv_display_delete (non-obj wrapper param: lv_display_t *)
     * lv_display_set_default (non-obj wrapper param: lv_display_t *)
     * lv_display_get_default (manual skip rule)
     * lv_display_get_next (manual skip rule)
     * lv_display_set_resolution (non-obj wrapper param: lv_display_t *)
     * lv_display_set_physical_resolution (non-obj wrapper param: lv_display_t *)
     * lv_display_set_offset (non-obj wrapper param: lv_display_t *)
     * lv_display_set_rotation (non-obj wrapper param: lv_display_t *)
     * lv_display_set_matrix_rotation (non-obj wrapper param: lv_display_t *)
     * lv_display_set_dpi (non-obj wrapper param: lv_display_t *)
     * lv_display_get_horizontal_resolution (non-obj wrapper param: const lv_display_t *)
     * lv_display_get_vertical_resolution (non-obj wrapper param: const lv_display_t *)
     * lv_display_get_original_horizontal_resolution (non-obj wrapper param: const lv_display_t *)
     * lv_display_get_original_vertical_resolution (non-obj wrapper param: const lv_display_t *)
     * lv_display_get_physical_horizontal_resolution (non-obj wrapper param: const lv_display_t *)
     * lv_display_get_physical_vertical_resolution (non-obj wrapper param: const lv_display_t *)
     * lv_display_get_offset_x (non-obj wrapper param: const lv_display_t *)
     * lv_display_get_offset_y (non-obj wrapper param: const lv_display_t *)
     * lv_display_get_rotation (non-obj wrapper param: lv_display_t *)
     * lv_display_get_matrix_rotation (non-obj wrapper param: lv_display_t *)
     * lv_display_get_dpi (non-obj wrapper param: const lv_display_t *)
     * lv_display_set_buffers (non-obj wrapper param: lv_display_t *)
     * lv_display_set_buffers_with_stride (non-obj wrapper param: lv_display_t *)
     * lv_display_set_draw_buffers (non-obj wrapper param: lv_display_t *)
     * lv_display_set_3rd_draw_buffer (non-obj wrapper param: lv_display_t *)
     * lv_display_set_render_mode (non-obj wrapper param: lv_display_t *)
     * lv_display_set_flush_cb (non-obj wrapper param: lv_display_t *)
     * lv_display_get_flush_cb (non-obj wrapper param: lv_display_t *)
     * lv_display_set_flush_wait_cb (non-obj wrapper param: lv_display_t *)
     * lv_display_set_sync_cb (non-obj wrapper param: lv_display_t *)
     * lv_display_set_sync_wait_cb (non-obj wrapper param: lv_display_t *)
     * lv_display_set_color_format (non-obj wrapper param: lv_display_t *)
     * lv_display_get_color_format (non-obj wrapper param: lv_display_t *)
     * lv_display_set_tile_cnt (non-obj wrapper param: lv_display_t *)
     * lv_display_get_tile_cnt (non-obj wrapper param: lv_display_t *)
     * lv_display_set_antialiasing (non-obj wrapper param: lv_display_t *)
     * lv_display_get_antialiasing (non-obj wrapper param: lv_display_t *)
     * lv_display_flush_ready (non-obj wrapper param: lv_display_t *)
     * lv_display_flush_is_last (non-obj wrapper param: lv_display_t *)
     * lv_display_sync_ready (non-obj wrapper param: lv_display_t *)
     * lv_display_sync_is_last (non-obj wrapper param: lv_display_t *)
     * lv_display_is_double_buffered (non-obj wrapper param: lv_display_t *)
     * lv_display_get_render_mode (non-obj wrapper param: lv_display_t *)
     * lv_display_get_screen_active (non-obj wrapper param: lv_display_t *)
     * lv_display_get_screen_prev (non-obj wrapper param: lv_display_t *)
     * lv_display_get_screen_loading (non-obj wrapper param: lv_display_t *)
     * lv_display_get_layer_top (non-obj wrapper param: lv_display_t *)
     * lv_display_get_layer_sys (non-obj wrapper param: lv_display_t *)
     * lv_display_get_layer_bottom (non-obj wrapper param: lv_display_t *)
     * lv_display_get_screen_by_name (non-obj wrapper param: const lv_display_t *)
     * lv_screen_load (unsupported param type: struct _lv_obj_t *)
     * lv_display_add_event_cb (unsupported return type: lv_event_dsc_t *)
     * lv_display_get_event_count (non-obj wrapper param: lv_display_t *)
     * lv_display_get_event_dsc (unsupported return type: lv_event_dsc_t *)
     * lv_display_remove_event (non-obj wrapper param: lv_display_t *)
     * lv_display_remove_event_cb_with_user_data (non-obj wrapper param: lv_display_t *)
     * lv_display_send_event (non-obj wrapper param: lv_display_t *)
     * lv_event_get_invalidated_area (unsupported param type: lv_event_t *)
     * lv_display_set_theme (unsupported param type: lv_theme_t *)
     * lv_display_get_theme (unsupported return type: lv_theme_t *)
     * lv_display_get_inactive_time (non-obj wrapper param: const lv_display_t *)
     * lv_display_trigger_activity (non-obj wrapper param: lv_display_t *)
     * lv_display_enable_invalidation (non-obj wrapper param: lv_display_t *)
     * lv_display_is_invalidation_enabled (non-obj wrapper param: lv_display_t *)
     * lv_display_get_refr_timer (unsupported return type: lv_timer_t *)
     * lv_display_delete_refr_timer (non-obj wrapper param: lv_display_t *)
     * lv_display_register_vsync_event (non-obj wrapper param: lv_display_t *)
     * lv_display_unregister_vsync_event (non-obj wrapper param: lv_display_t *)
     * lv_display_send_vsync_event (non-obj wrapper param: lv_display_t *)
     * lv_display_set_user_data (non-obj wrapper param: lv_display_t *)
     * lv_display_set_driver_data (non-obj wrapper param: lv_display_t *)
     * lv_display_get_user_data (non-obj wrapper param: lv_display_t *)
     * lv_display_get_driver_data (non-obj wrapper param: lv_display_t *)
     * lv_display_get_buf_active (non-obj wrapper param: lv_display_t *)
     * lv_display_rotate_area (non-obj wrapper param: lv_display_t *)
     * lv_display_rotate_point (non-obj wrapper param: lv_display_t *)
     * lv_display_get_draw_buf_size (non-obj wrapper param: lv_display_t *)
     * lv_display_get_invalidated_draw_buf_size (non-obj wrapper param: lv_display_t *)
     * lv_display_dpx (non-obj wrapper param: const lv_display_t *)
     * lv_obj_delete_delayed (manual skip rule)
     * lv_obj_delete_anim_completed_cb (manual skip rule)
     * lv_obj_get_child_by_type (manual skip rule)
     * lv_obj_get_sibling_by_type (manual skip rule)
     * lv_obj_get_child_count_by_type (manual skip rule)
     * lv_obj_get_name_resolved (unsupported param type: char[])
     * lv_obj_get_child_by_name (manual skip rule)
     * lv_obj_get_index_by_type (manual skip rule)
     * lv_obj_transform_point_array (unsupported param type: lv_point_t[])
     * lv_obj_add_style (unsupported param type: const lv_style_t *)
     * lv_obj_replace_style (unsupported param type: const lv_style_t *)
     * lv_obj_remove_style (unsupported param type: const lv_style_t *)
     * lv_obj_report_style_change (manual skip rule)
     * lv_obj_style_set_disabled (unsupported param type: const lv_style_t *)
     * lv_obj_style_get_disabled (unsupported param type: const lv_style_t *)
     * lv_obj_enable_style_refresh (manual skip rule)
     * lv_obj_set_local_style_prop (incomplete struct param: lv_style_value_t)
     * lv_obj_get_local_style_prop (unsupported param type: lv_style_value_t *)
     * lv_obj_style_apply_color_filter (incomplete struct param: lv_style_value_t)
     * lv_obj_style_get_selector_state (manual skip rule)
     * lv_obj_style_get_selector_part (manual skip rule)
     * lv_obj_get_style_bg_grad (unsupported return type: const lv_grad_dsc_t *)
     * lv_obj_get_style_bg_image_src (manual skip rule)
     * lv_obj_get_style_image_colorkey (unsupported return type: const lv_image_colorkey_t *)
     * lv_obj_get_style_arc_image_src (manual skip rule)
     * lv_obj_get_style_color_filter_dsc (unsupported return type: const lv_color_filter_dsc_t *)
     * lv_obj_get_style_anim (unsupported return type: const lv_anim_t *)
     * lv_obj_get_style_transition (unsupported return type: const lv_style_transition_dsc_t *)
     * lv_obj_get_style_bitmap_mask_src (manual skip rule)
     * lv_obj_set_style_bg_grad (unsupported param type: const lv_grad_dsc_t *)
     * lv_obj_set_style_image_colorkey (unsupported param type: const lv_image_colorkey_t *)
     * lv_obj_set_style_color_filter_dsc (unsupported param type: const lv_color_filter_dsc_t *)
     * lv_obj_set_style_anim (unsupported param type: const lv_anim_t *)
     * lv_obj_set_style_transition (unsupported param type: const lv_style_transition_dsc_t *)
     * lv_obj_bind_style (unsupported param type: const lv_style_t *)
     * lv_obj_bind_style_prop (unsupported param type: lv_subject_t *)
     * lv_draw_add_task (unsupported param type: lv_layer_t *)
     * lv_draw_finalize_task_creation (unsupported param type: lv_layer_t *)
     * lv_draw_dispatch_layer (unsupported param type: lv_layer_t *)
     * lv_draw_get_available_task (unsupported param type: lv_layer_t *)
     * lv_draw_get_next_available_task (unsupported param type: lv_layer_t *)
     * lv_layer_init (unsupported param type: lv_layer_t *)
     * lv_layer_reset (unsupported param type: lv_layer_t *)
     * lv_draw_layer_create (unsupported param type: lv_layer_t *)
     * lv_draw_layer_init (unsupported param type: lv_layer_t *)
     * lv_draw_layer_alloc_buf (unsupported param type: lv_layer_t *)
     * lv_draw_layer_go_to_xy (unsupported param type: lv_layer_t *)
     * lv_draw_layer_create_drop_shadow (unsupported param type: lv_layer_t *)
     * lv_draw_layer_finish_drop_shadow (unsupported param type: lv_layer_t *)
     * lv_draw_rect_dsc_init (unsupported param type: lv_draw_rect_dsc_t *)
     * lv_draw_fill (unsupported param type: lv_layer_t *)
     * lv_draw_border (unsupported param type: lv_layer_t *)
     * lv_draw_box_shadow (unsupported param type: lv_layer_t *)
     * lv_draw_rect (unsupported param type: lv_layer_t *)
     * lv_draw_label_dsc_init (unsupported param type: lv_draw_label_dsc_t *)
     * lv_draw_task_get_label_dsc (unsupported return type: lv_draw_label_dsc_t *)
     * lv_draw_label (unsupported param type: lv_layer_t *)
     * lv_draw_character (unsupported param type: lv_layer_t *)
     * lv_draw_letter (unsupported param type: lv_layer_t *)
     * lv_draw_label_iterate_characters (unsupported param type: const lv_draw_label_dsc_t *)
     * lv_fs_get_buffer_from_path (unsupported param type: void * *)
     * lv_image_decoder_get_info (manual skip rule)
     * lv_image_decoder_open (manual skip rule)
     * lv_image_decoder_get_area (unsupported param type: lv_image_decoder_dsc_t *)
     * lv_image_decoder_close (manual skip rule)
     * lv_image_decoder_create (manual skip rule)
     * lv_image_decoder_delete (unsupported param type: lv_image_decoder_t *)
     * lv_image_decoder_get_next (unsupported param type: lv_image_decoder_t *)
     * lv_image_decoder_set_info_cb (unsupported param type: lv_image_decoder_t *)
     * lv_image_decoder_set_open_cb (manual skip rule)
     * lv_image_decoder_set_get_area_cb (unsupported param type: lv_image_decoder_t *)
     * lv_image_decoder_set_close_cb (manual skip rule)
     * lv_image_decoder_post_process (unsupported param type: lv_image_decoder_dsc_t *)
     * lv_draw_image_dsc_init (unsupported param type: lv_draw_image_dsc_t *)
     * lv_draw_task_get_image_dsc (unsupported return type: lv_draw_image_dsc_t *)
     * lv_draw_image (unsupported param type: lv_layer_t *)
     * lv_draw_layer (unsupported param type: lv_layer_t *)
     * lv_image_src_get_type (manual skip rule)
     * lv_draw_line_dsc_init (unsupported param type: lv_draw_line_dsc_t *)
     * lv_draw_task_get_line_dsc (unsupported return type: lv_draw_line_dsc_t *)
     * lv_draw_line (unsupported param type: lv_layer_t *)
     * lv_draw_line_iterate (unsupported param type: lv_draw_line_dsc_t *)
     * lv_draw_arc_dsc_init (unsupported param type: lv_draw_arc_dsc_t *)
     * lv_draw_task_get_arc_dsc (unsupported return type: lv_draw_arc_dsc_t *)
     * lv_draw_arc (unsupported param type: lv_layer_t *)
     * lv_draw_triangle (unsupported param type: lv_layer_t *)
     * lv_draw_blur_dsc_init (unsupported param type: lv_draw_blur_dsc_t *)
     * lv_draw_task_get_blur_dsc (unsupported return type: lv_draw_blur_dsc_t *)
     * lv_draw_blur (unsupported param type: lv_layer_t *)
     * lv_obj_init_draw_rect_dsc (unsupported param type: lv_draw_rect_dsc_t *)
     * lv_obj_init_draw_label_dsc (unsupported param type: lv_draw_label_dsc_t *)
     * lv_obj_init_draw_image_dsc (unsupported param type: lv_draw_image_dsc_t *)
     * lv_obj_init_draw_line_dsc (unsupported param type: lv_draw_line_dsc_t *)
     * lv_obj_init_draw_arc_dsc (unsupported param type: lv_draw_arc_dsc_t *)
     * lv_obj_init_draw_blur_dsc (unsupported param type: lv_draw_blur_dsc_t *)
     * lv_obj_class_create_obj (unsupported param type: const lv_obj_class_t *)
     * lv_indev_create (manual skip rule)
     * lv_indev_delete (non-obj wrapper param: lv_indev_t *)
     * lv_indev_get_next (manual skip rule)
     * lv_indev_read (non-obj wrapper param: lv_indev_t *)
     * lv_indev_read_timer_cb (unsupported param type: lv_timer_t *)
     * lv_indev_enable (non-obj wrapper param: lv_indev_t *)
     * lv_indev_set_type (non-obj wrapper param: lv_indev_t *)
     * lv_indev_set_read_cb (non-obj wrapper param: lv_indev_t *)
     * lv_indev_set_user_data (non-obj wrapper param: lv_indev_t *)
     * lv_indev_set_driver_data (non-obj wrapper param: lv_indev_t *)
     * lv_indev_set_display (unsupported param type: struct _lv_display_t *)
     * lv_indev_set_long_press_time (non-obj wrapper param: lv_indev_t *)
     * lv_indev_set_long_press_repeat_time (non-obj wrapper param: lv_indev_t *)
     * lv_indev_set_scroll_limit (non-obj wrapper param: lv_indev_t *)
     * lv_indev_set_scroll_throw (non-obj wrapper param: lv_indev_t *)
     * lv_indev_set_gesture_min_velocity (non-obj wrapper param: lv_indev_t *)
     * lv_indev_set_gesture_min_distance (non-obj wrapper param: lv_indev_t *)
     * lv_indev_get_type (non-obj wrapper param: const lv_indev_t *)
     * lv_indev_get_read_cb (non-obj wrapper param: lv_indev_t *)
     * lv_indev_get_state (non-obj wrapper param: const lv_indev_t *)
     * lv_indev_get_group (unsupported return type: lv_group_t *)
     * lv_indev_get_display (non-obj wrapper param: const lv_indev_t *)
     * lv_indev_get_user_data (non-obj wrapper param: const lv_indev_t *)
     * lv_indev_get_driver_data (non-obj wrapper param: const lv_indev_t *)
     * lv_indev_get_press_moved (non-obj wrapper param: const lv_indev_t *)
     * lv_indev_reset (non-obj wrapper param: lv_indev_t *)
     * lv_indev_stop_processing (non-obj wrapper param: lv_indev_t *)
     * lv_indev_reset_long_press (non-obj wrapper param: lv_indev_t *)
     * lv_indev_set_cursor (non-obj wrapper param: lv_indev_t *)
     * lv_indev_set_group (unsupported param type: lv_group_t *)
     * lv_indev_set_button_points (unsupported param type: const lv_point_t[])
     * lv_indev_get_point (non-obj wrapper param: const lv_indev_t *)
     * lv_indev_get_gesture_dir (non-obj wrapper param: const lv_indev_t *)
     * lv_indev_get_key (non-obj wrapper param: const lv_indev_t *)
     * lv_indev_get_short_click_streak (non-obj wrapper param: const lv_indev_t *)
     * lv_indev_get_scroll_dir (non-obj wrapper param: const lv_indev_t *)
     * lv_indev_get_scroll_obj (non-obj wrapper param: const lv_indev_t *)
     * lv_indev_get_vect (non-obj wrapper param: const lv_indev_t *)
     * lv_indev_get_cursor (non-obj wrapper param: lv_indev_t *)
     * lv_indev_wait_release (non-obj wrapper param: lv_indev_t *)
     * lv_indev_get_read_timer (unsupported return type: lv_timer_t *)
     * lv_indev_set_mode (non-obj wrapper param: lv_indev_t *)
     * lv_indev_get_mode (non-obj wrapper param: lv_indev_t *)
     * lv_indev_add_event_cb (unsupported return type: lv_event_dsc_t *)
     * lv_indev_get_event_count (non-obj wrapper param: lv_indev_t *)
     * lv_indev_get_event_dsc (unsupported return type: lv_event_dsc_t *)
     * lv_indev_remove_event (non-obj wrapper param: lv_indev_t *)
     * lv_indev_remove_event_cb_with_user_data (non-obj wrapper param: lv_indev_t *)
     * lv_indev_send_event (non-obj wrapper param: lv_indev_t *)
     * lv_indev_set_key_remap_cb (non-obj wrapper param: lv_indev_t *)
     * lv_obj_event_base (unsupported param type: const lv_obj_class_t *)
     * lv_event_get_current_target_obj (unsupported param type: lv_event_t *)
     * lv_event_get_target_obj (unsupported param type: lv_event_t *)
     * lv_obj_add_event_cb (manual skip rule)
     * lv_obj_get_event_dsc (unsupported return type: lv_event_dsc_t *)
     * lv_obj_remove_event_dsc (manual skip rule)
     * lv_obj_remove_event_cb (manual skip rule)
     * lv_event_get_indev (unsupported param type: lv_event_t *)
     * lv_event_get_layer (unsupported param type: lv_event_t *)
     * lv_event_get_old_size (unsupported param type: lv_event_t *)
     * lv_event_get_key (unsupported param type: lv_event_t *)
     * lv_event_get_rotary_diff (unsupported param type: lv_event_t *)
     * lv_event_get_scroll_anim (unsupported param type: lv_event_t *)
     * lv_event_set_ext_draw_size (unsupported param type: lv_event_t *)
     * lv_event_get_self_size_info (unsupported param type: lv_event_t *)
     * lv_event_get_hit_test_info (unsupported param type: lv_event_t *)
     * lv_event_get_cover_area (unsupported param type: lv_event_t *)
     * lv_event_set_cover_res (unsupported param type: lv_event_t *)
     * lv_event_get_draw_task (unsupported param type: lv_event_t *)
     * lv_event_get_prev_state (unsupported param type: lv_event_t *)
     * lv_obj_get_group (unsupported return type: lv_group_t *)
     * lv_obj_check_type (unsupported param type: const lv_obj_class_t *)
     * lv_obj_has_class (unsupported param type: const lv_obj_class_t *)
     * lv_obj_get_class (unsupported return type: const lv_obj_class_t *)
     * lv_obj_null_on_delete (manual skip rule)
     * lv_obj_add_delete_cb (manual skip rule)
     * lv_obj_remove_delete_cb (unsupported param type: lv_delete_dsc_t *)
     * lv_obj_add_play_timeline_event (unsupported param type: lv_anim_timeline_t *)
     * lv_ll_init (unsupported param type: lv_ll_t *)
     * lv_ll_ins_head (unsupported param type: lv_ll_t *)
     * lv_ll_ins_prev (unsupported param type: lv_ll_t *)
     * lv_ll_ins_tail (unsupported param type: lv_ll_t *)
     * lv_ll_remove (unsupported param type: lv_ll_t *)
     * lv_ll_clear_custom (unsupported param type: lv_ll_t *)
     * lv_ll_clear (unsupported param type: lv_ll_t *)
     * lv_ll_chg_list (unsupported param type: lv_ll_t *)
     * lv_ll_get_head (unsupported param type: const lv_ll_t *)
     * lv_ll_get_tail (unsupported param type: const lv_ll_t *)
     * lv_ll_get_next (unsupported param type: const lv_ll_t *)
     * lv_ll_get_prev (unsupported param type: const lv_ll_t *)
     * lv_ll_get_len (unsupported param type: const lv_ll_t *)
     * lv_ll_move_before (unsupported param type: lv_ll_t *)
     * lv_ll_is_empty (unsupported param type: lv_ll_t *)
     * lv_subject_init_int (manual skip rule)
     * lv_subject_set_int (manual skip rule)
     * lv_subject_get_int (manual skip rule)
     * lv_subject_get_previous_int (unsupported param type: lv_subject_t *)
     * lv_subject_set_min_value_int (unsupported param type: lv_subject_t *)
     * lv_subject_set_max_value_int (unsupported param type: lv_subject_t *)
     * lv_subject_init_float (unsupported param type: lv_subject_t *)
     * lv_subject_set_float (unsupported param type: lv_subject_t *)
     * lv_subject_get_float (unsupported param type: lv_subject_t *)
     * lv_subject_get_previous_float (unsupported param type: lv_subject_t *)
     * lv_subject_set_min_value_float (unsupported param type: lv_subject_t *)
     * lv_subject_set_max_value_float (unsupported param type: lv_subject_t *)
     * lv_subject_init_string (manual skip rule)
     * lv_subject_copy_string (unsupported param type: lv_subject_t *)
     * lv_subject_snprintf (unsupported param type: lv_subject_t *)
     * lv_subject_get_string (manual skip rule)
     * lv_subject_get_previous_string (unsupported param type: lv_subject_t *)
     * lv_subject_init_pointer (manual skip rule)
     * lv_subject_set_pointer (manual skip rule)
     * lv_subject_get_pointer (manual skip rule)
     * lv_subject_get_previous_pointer (unsupported param type: lv_subject_t *)
     * lv_subject_init_color (unsupported param type: lv_subject_t *)
     * lv_subject_set_color (unsupported param type: lv_subject_t *)
     * lv_subject_get_color (unsupported param type: lv_subject_t *)
     * lv_subject_get_previous_color (unsupported param type: lv_subject_t *)
     * lv_subject_init_group (manual skip rule)
     * lv_subject_deinit (unsupported param type: lv_subject_t *)
     * lv_subject_get_group_element (manual skip rule)
     * lv_subject_add_observer (manual skip rule)
     * lv_subject_add_observer_obj (manual skip rule)
     * lv_subject_add_observer_with_target (unsupported param type: lv_subject_t *)
     * lv_obj_remove_from_subject (unsupported param type: lv_subject_t *)
     * lv_observer_get_target (manual skip rule)
     * lv_subject_notify (manual skip rule)
     * lv_obj_add_subject_increment_event (unsupported param type: lv_subject_t *)
     * lv_obj_set_subject_increment_event_min_value (unsupported param type: lv_subject_increment_dsc_t *)
     * lv_obj_set_subject_increment_event_max_value (unsupported param type: lv_subject_increment_dsc_t *)
     * lv_obj_set_subject_increment_event_rollover (unsupported param type: lv_subject_increment_dsc_t *)
     * lv_obj_add_subject_toggle_event (unsupported param type: lv_subject_t *)
     * lv_obj_add_subject_set_int_event (unsupported param type: lv_subject_t *)
     * lv_obj_add_subject_set_float_event (unsupported param type: lv_subject_t *)
     * lv_obj_add_subject_set_string_event (unsupported param type: lv_subject_t *)
     * lv_obj_bind_flag_if_eq (unsupported param type: lv_subject_t *)
     * lv_obj_bind_flag_if_not_eq (unsupported param type: lv_subject_t *)
     * lv_obj_bind_flag_if_gt (unsupported param type: lv_subject_t *)
     * lv_obj_bind_flag_if_ge (unsupported param type: lv_subject_t *)
     * lv_obj_bind_flag_if_lt (unsupported param type: lv_subject_t *)
     * lv_obj_bind_flag_if_le (unsupported param type: lv_subject_t *)
     * lv_obj_bind_state_if_eq (unsupported param type: lv_subject_t *)
     * lv_obj_bind_state_if_not_eq (unsupported param type: lv_subject_t *)
     * lv_obj_bind_state_if_gt (unsupported param type: lv_subject_t *)
     * lv_obj_bind_state_if_ge (unsupported param type: lv_subject_t *)
     * lv_obj_bind_state_if_lt (unsupported param type: lv_subject_t *)
     * lv_obj_bind_state_if_le (unsupported param type: lv_subject_t *)
     * lv_obj_bind_checked (unsupported param type: lv_subject_t *)
     * lv_refr_now (non-obj wrapper param: lv_display_t *)
     * lv_obj_redraw (unsupported param type: lv_layer_t *)
     * lv_display_refr_timer (unsupported param type: lv_timer_t *)
     * lv_sysmon_create (non-obj wrapper param: lv_display_t *)
     * lv_sysmon_show_performance (non-obj wrapper param: lv_display_t *)
     * lv_sysmon_hide_performance (non-obj wrapper param: lv_display_t *)
     * lv_sysmon_performance_dump (non-obj wrapper param: lv_display_t *)
     * lv_sysmon_performance_resume (non-obj wrapper param: lv_display_t *)
     * lv_sysmon_performance_pause (non-obj wrapper param: lv_display_t *)
     * lv_draw_mask_rect (unsupported param type: lv_layer_t *)
     * lv_draw_vector_dsc_create (unsupported param type: lv_layer_t *)
     * lv_draw_vector_dsc_set_fill_units (unsupported param type: const lv_vector_fill_units_t)
     * lv_draw_vector_dsc_set_fill_image (unsupported param type: const lv_draw_image_dsc_t *)
     * lv_linux_drm_set_file (non-obj wrapper param: lv_display_t *)
     * lv_linux_drm_set_mode_cb (non-obj wrapper param: lv_display_t *)
     * lv_evdev_set_swap_axes (non-obj wrapper param: lv_indev_t *)
     * lv_evdev_set_calibration (non-obj wrapper param: lv_indev_t *)
     * lv_evdev_is_raw_key (unsupported param type: lv_event_t *)
     * lv_evdev_get_raw_key (unsupported param type: lv_event_t *)
     * lv_evdev_delete (non-obj wrapper param: lv_indev_t *)
     * lv_bin_decoder_info (unsupported param type: lv_image_decoder_t *)
     * lv_bin_decoder_get_area (unsupported param type: lv_image_decoder_t *)
     * lv_bin_decoder_open (unsupported param type: lv_image_decoder_t *)
     * lv_bin_decoder_close (unsupported param type: lv_image_decoder_t *)
     * lv_theme_create (manual skip rule)
     * lv_theme_copy (unsupported param type: lv_theme_t *)
     * lv_theme_get_from_obj (unsupported return type: lv_theme_t *)
     * lv_theme_set_parent (unsupported param type: lv_theme_t *)
     * lv_theme_set_apply_cb (unsupported param type: lv_theme_t *)
     * lv_theme_delete (unsupported param type: lv_theme_t *)
     * lv_theme_default_init (unsupported return type: lv_theme_t *)
     * lv_theme_default_get (manual skip rule)
     * lv_theme_mono_init (unsupported return type: lv_theme_t *)
     * lv_theme_mono_get (manual skip rule)
     * lv_theme_simple_init (unsupported return type: lv_theme_t *)
     * lv_theme_simple_get (unsupported return type: lv_theme_t *)
     * lv_image_get_src (manual skip rule)
     * lv_image_bind_src (unsupported param type: lv_subject_t *)
     * lv_animimg_set_src (unsupported param type: const void *[])
     * lv_animimg_set_src_reverse (unsupported param type: const void *[])
     * lv_animimg_get_src (unsupported return type: const void * *)
     * lv_animimg_get_anim (unsupported return type: lv_anim_t *)
     * lv_arc_bind_value (unsupported param type: lv_subject_t *)
     * lv_label_set_text_fmt (manual skip rule)
     * lv_label_set_text_vfmt (unsupported param type: va_list)
     * lv_label_bind_text (unsupported param type: lv_subject_t *)
     * lv_bar_bind_value (unsupported param type: lv_subject_t *)
     * lv_canvas_get_buf (manual skip rule)
     * lv_canvas_init_layer (unsupported param type: lv_layer_t *)
     * lv_canvas_finish_layer (unsupported param type: lv_layer_t *)
     * lv_canvas_buf_size (manual skip rule)
     * lv_buttonmatrix_set_map (unsupported param type: const char *const[])
     * lv_buttonmatrix_set_ctrl_map (unsupported param type: const lv_buttonmatrix_ctrl_t[])
     * lv_calendar_set_highlighted_dates (unsupported param type: lv_calendar_date_t[])
     * lv_calendar_get_pressed_date (manual skip rule)
     * lv_chart_set_point_count (manual skip rule)
     * lv_chart_get_x_start_point (unsupported param type: lv_chart_series_t *)
     * lv_chart_get_point_pos_by_id (unsupported param type: lv_chart_series_t *)
     * lv_chart_refresh (manual skip rule)
     * lv_chart_add_series (manual skip rule)
     * lv_chart_remove_series (manual skip rule)
     * lv_chart_hide_series (unsupported param type: lv_chart_series_t *)
     * lv_chart_set_series_color (manual skip rule)
     * lv_chart_get_series_color (unsupported param type: const lv_chart_series_t *)
     * lv_chart_set_x_start_point (manual skip rule)
     * lv_chart_get_series_next (manual skip rule)
     * lv_chart_add_cursor (manual skip rule)
     * lv_chart_remove_cursor (unsupported param type: lv_chart_cursor_t *)
     * lv_chart_set_cursor_pos (manual skip rule)
     * lv_chart_set_cursor_pos_x (unsupported param type: lv_chart_cursor_t *)
     * lv_chart_set_cursor_pos_y (unsupported param type: lv_chart_cursor_t *)
     * lv_chart_set_cursor_point (manual skip rule)
     * lv_chart_get_cursor_point (unsupported param type: lv_chart_cursor_t *)
     * lv_chart_set_all_values (unsupported param type: lv_chart_series_t *)
     * lv_chart_set_next_value (unsupported param type: lv_chart_series_t *)
     * lv_chart_set_next_value2 (unsupported param type: lv_chart_series_t *)
     * lv_chart_set_series_values (manual skip rule)
     * lv_chart_set_series_values2 (unsupported param type: lv_chart_series_t *)
     * lv_chart_set_series_value_by_id (unsupported param type: lv_chart_series_t *)
     * lv_chart_set_series_value_by_id2 (unsupported param type: lv_chart_series_t *)
     * lv_chart_set_series_ext_y_array (unsupported param type: lv_chart_series_t *)
     * lv_chart_set_series_ext_x_array (unsupported param type: lv_chart_series_t *)
     * lv_chart_get_series_y_array (unsupported param type: lv_chart_series_t *)
     * lv_chart_get_series_x_array (unsupported param type: lv_chart_series_t *)
     * lv_dropdown_get_selected_str (manual skip rule)
     * lv_dropdown_bind_value (unsupported param type: lv_subject_t *)
     * lv_keyboard_set_map (manual skip rule)
     * lv_keyboard_get_map_array (manual skip rule)
     * lv_keyboard_def_event_cb (unsupported param type: lv_event_t *)
     * lv_line_set_points (unsupported param type: const lv_point_precise_t[])
     * lv_line_set_points_mutable (unsupported param type: lv_point_precise_t[])
     * lv_lottie_get_anim (unsupported return type: lv_anim_t *)
     * lv_roller_get_selected_str (manual skip rule)
     * lv_roller_get_option_str (manual skip rule)
     * lv_roller_bind_value (unsupported param type: lv_subject_t *)
     * lv_scale_set_text_src (unsupported param type: const char *[])
     * lv_scale_add_section (manual skip rule)
     * lv_scale_section_set_range (unsupported param type: lv_scale_section_t *)
     * lv_scale_set_section_range (manual skip rule)
     * lv_scale_set_section_min_value (unsupported param type: lv_scale_section_t *)
     * lv_scale_set_section_max_value (unsupported param type: lv_scale_section_t *)
     * lv_scale_section_set_style (unsupported param type: lv_scale_section_t *)
     * lv_scale_set_section_style_main (unsupported param type: lv_scale_section_t *)
     * lv_scale_set_section_style_indicator (unsupported param type: lv_scale_section_t *)
     * lv_scale_set_section_style_items (unsupported param type: lv_scale_section_t *)
     * lv_scale_bind_section_min_value (unsupported param type: lv_scale_section_t *)
     * lv_scale_bind_section_max_value (unsupported param type: lv_scale_section_t *)
     * lv_scale_bind_line_needle_value (unsupported param type: lv_subject_t *)
     * lv_scale_bind_image_needle_value (unsupported param type: lv_subject_t *)
     * lv_slider_bind_value (unsupported param type: lv_subject_t *)
     * lv_spangroup_set_span_style (unsupported param type: const lv_style_t *)
     * lv_span_get_style (unsupported return type: lv_style_t *)
     * lv_spangroup_bind_span_text (unsupported param type: lv_subject_t *)
     * lv_textarea_get_text (manual skip rule)
     * lv_spinbox_bind_value (unsupported param type: lv_subject_t *)
     */

}  // PYBIND11_MODULE