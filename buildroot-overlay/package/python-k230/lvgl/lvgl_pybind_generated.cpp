/*
 * Auto-Generated file by gen_pybind11.py, DO NOT EDIT!
 *
 * Command line:
 * gen_pybind11.py -M lvgl -MP lv -I /home/wangjianxin/k230/k230_linux_sdk/output/k230_canmv_01studio_defconfig/build/lvgl-97a99a8affe966617f048830a007bbf8bea63da6/lvgl/include -I /home/wangjianxin/k230/k230_linux_sdk/output/k230_canmv_01studio_defconfig/build/lvgl-97a99a8affe966617f048830a007bbf8bea63da6/buildroot-build -o lvgl_pybind_generated.cpp /home/wangjianxin/k230/k230_linux_sdk/output/k230_canmv_01studio_defconfig/build/lvgl-97a99a8affe966617f048830a007bbf8bea63da6/lvgl/include/lvgl/lvgl.h
 *
 * Generating Objects: obj(None), image(obj), animimg(obj), arc(obj), arclabel(obj), label(obj), bar(obj), canvas(obj), barcode(obj), button(obj), buttonmatrix(obj), calendar(obj), chart(obj), checkbox(obj), dropdown(obj), imagebutton(obj), keyboard(obj), led(obj), line(obj), list(obj), lottie(obj), menu(obj), menu_page(obj), menu_cont(obj), menu_section(obj), menu_separator(obj), msgbox(obj), qrcode(obj), roller(obj), scale(obj), slider(obj), spangroup(obj), textarea(obj), spinbox(obj), spinner(obj), switch(obj), table(obj), tabview(obj), tileview(obj), win(obj)
 */

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>
#include <pybind11/numpy.h>
#include "lvgl/lvgl.h"
#include "lvgl/driver_backends.h"
#include "lvgl_pybind_helpers.h"

namespace py = pybind11;

PYBIND11_MODULE(_lvgl, m) {
    m.doc() = "LVGL Python bindings (auto-generated)";

    /*
     * Enum definitions
     */

    /* Enum LV_STR_SYMBOL (no C type found, emitting as constants) */
    m.attr("STR_SYMBOL_BULLET") = (int)LV_STR_SYMBOL_BULLET;
    m.attr("STR_SYMBOL_AUDIO") = (int)LV_STR_SYMBOL_AUDIO;
    m.attr("STR_SYMBOL_VIDEO") = (int)LV_STR_SYMBOL_VIDEO;
    m.attr("STR_SYMBOL_LIST") = (int)LV_STR_SYMBOL_LIST;
    m.attr("STR_SYMBOL_OK") = (int)LV_STR_SYMBOL_OK;
    m.attr("STR_SYMBOL_CLOSE") = (int)LV_STR_SYMBOL_CLOSE;
    m.attr("STR_SYMBOL_POWER") = (int)LV_STR_SYMBOL_POWER;
    m.attr("STR_SYMBOL_SETTINGS") = (int)LV_STR_SYMBOL_SETTINGS;
    m.attr("STR_SYMBOL_HOME") = (int)LV_STR_SYMBOL_HOME;
    m.attr("STR_SYMBOL_DOWNLOAD") = (int)LV_STR_SYMBOL_DOWNLOAD;
    m.attr("STR_SYMBOL_DRIVE") = (int)LV_STR_SYMBOL_DRIVE;
    m.attr("STR_SYMBOL_REFRESH") = (int)LV_STR_SYMBOL_REFRESH;
    m.attr("STR_SYMBOL_MUTE") = (int)LV_STR_SYMBOL_MUTE;
    m.attr("STR_SYMBOL_VOLUME_MID") = (int)LV_STR_SYMBOL_VOLUME_MID;
    m.attr("STR_SYMBOL_VOLUME_MAX") = (int)LV_STR_SYMBOL_VOLUME_MAX;
    m.attr("STR_SYMBOL_IMAGE") = (int)LV_STR_SYMBOL_IMAGE;
    m.attr("STR_SYMBOL_TINT") = (int)LV_STR_SYMBOL_TINT;
    m.attr("STR_SYMBOL_PREV") = (int)LV_STR_SYMBOL_PREV;
    m.attr("STR_SYMBOL_PLAY") = (int)LV_STR_SYMBOL_PLAY;
    m.attr("STR_SYMBOL_PAUSE") = (int)LV_STR_SYMBOL_PAUSE;
    m.attr("STR_SYMBOL_STOP") = (int)LV_STR_SYMBOL_STOP;
    m.attr("STR_SYMBOL_NEXT") = (int)LV_STR_SYMBOL_NEXT;
    m.attr("STR_SYMBOL_EJECT") = (int)LV_STR_SYMBOL_EJECT;
    m.attr("STR_SYMBOL_LEFT") = (int)LV_STR_SYMBOL_LEFT;
    m.attr("STR_SYMBOL_RIGHT") = (int)LV_STR_SYMBOL_RIGHT;
    m.attr("STR_SYMBOL_PLUS") = (int)LV_STR_SYMBOL_PLUS;
    m.attr("STR_SYMBOL_MINUS") = (int)LV_STR_SYMBOL_MINUS;
    m.attr("STR_SYMBOL_EYE_OPEN") = (int)LV_STR_SYMBOL_EYE_OPEN;
    m.attr("STR_SYMBOL_EYE_CLOSE") = (int)LV_STR_SYMBOL_EYE_CLOSE;
    m.attr("STR_SYMBOL_WARNING") = (int)LV_STR_SYMBOL_WARNING;
    m.attr("STR_SYMBOL_SHUFFLE") = (int)LV_STR_SYMBOL_SHUFFLE;
    m.attr("STR_SYMBOL_UP") = (int)LV_STR_SYMBOL_UP;
    m.attr("STR_SYMBOL_DOWN") = (int)LV_STR_SYMBOL_DOWN;
    m.attr("STR_SYMBOL_LOOP") = (int)LV_STR_SYMBOL_LOOP;
    m.attr("STR_SYMBOL_DIRECTORY") = (int)LV_STR_SYMBOL_DIRECTORY;
    m.attr("STR_SYMBOL_UPLOAD") = (int)LV_STR_SYMBOL_UPLOAD;
    m.attr("STR_SYMBOL_CALL") = (int)LV_STR_SYMBOL_CALL;
    m.attr("STR_SYMBOL_CUT") = (int)LV_STR_SYMBOL_CUT;
    m.attr("STR_SYMBOL_COPY") = (int)LV_STR_SYMBOL_COPY;
    m.attr("STR_SYMBOL_SAVE") = (int)LV_STR_SYMBOL_SAVE;
    m.attr("STR_SYMBOL_BARS") = (int)LV_STR_SYMBOL_BARS;
    m.attr("STR_SYMBOL_ENVELOPE") = (int)LV_STR_SYMBOL_ENVELOPE;
    m.attr("STR_SYMBOL_CHARGE") = (int)LV_STR_SYMBOL_CHARGE;
    m.attr("STR_SYMBOL_PASTE") = (int)LV_STR_SYMBOL_PASTE;
    m.attr("STR_SYMBOL_BELL") = (int)LV_STR_SYMBOL_BELL;
    m.attr("STR_SYMBOL_KEYBOARD") = (int)LV_STR_SYMBOL_KEYBOARD;
    m.attr("STR_SYMBOL_GPS") = (int)LV_STR_SYMBOL_GPS;
    m.attr("STR_SYMBOL_FILE") = (int)LV_STR_SYMBOL_FILE;
    m.attr("STR_SYMBOL_WIFI") = (int)LV_STR_SYMBOL_WIFI;
    m.attr("STR_SYMBOL_BATTERY_FULL") = (int)LV_STR_SYMBOL_BATTERY_FULL;
    m.attr("STR_SYMBOL_BATTERY_3") = (int)LV_STR_SYMBOL_BATTERY_3;
    m.attr("STR_SYMBOL_BATTERY_2") = (int)LV_STR_SYMBOL_BATTERY_2;
    m.attr("STR_SYMBOL_BATTERY_1") = (int)LV_STR_SYMBOL_BATTERY_1;
    m.attr("STR_SYMBOL_BATTERY_EMPTY") = (int)LV_STR_SYMBOL_BATTERY_EMPTY;
    m.attr("STR_SYMBOL_USB") = (int)LV_STR_SYMBOL_USB;
    m.attr("STR_SYMBOL_BLUETOOTH") = (int)LV_STR_SYMBOL_BLUETOOTH;
    m.attr("STR_SYMBOL_TRASH") = (int)LV_STR_SYMBOL_TRASH;
    m.attr("STR_SYMBOL_EDIT") = (int)LV_STR_SYMBOL_EDIT;
    m.attr("STR_SYMBOL_BACKSPACE") = (int)LV_STR_SYMBOL_BACKSPACE;
    m.attr("STR_SYMBOL_SD_CARD") = (int)LV_STR_SYMBOL_SD_CARD;
    m.attr("STR_SYMBOL_NEW_LINE") = (int)LV_STR_SYMBOL_NEW_LINE;
    m.attr("STR_SYMBOL_DUMMY") = (int)LV_STR_SYMBOL_DUMMY;

    /* Enum LV_OPA (no C type found, emitting as constants) */
    m.attr("OPA_TRANSP") = (int)LV_OPA_TRANSP;
    m.attr("OPA__0") = (int)LV_OPA_0;
    m.attr("OPA__10") = (int)LV_OPA_10;
    m.attr("OPA__20") = (int)LV_OPA_20;
    m.attr("OPA__30") = (int)LV_OPA_30;
    m.attr("OPA__40") = (int)LV_OPA_40;
    m.attr("OPA__50") = (int)LV_OPA_50;
    m.attr("OPA__60") = (int)LV_OPA_60;
    m.attr("OPA__70") = (int)LV_OPA_70;
    m.attr("OPA__80") = (int)LV_OPA_80;
    m.attr("OPA__90") = (int)LV_OPA_90;
    m.attr("OPA__100") = (int)LV_OPA_100;
    m.attr("OPA_COVER") = (int)LV_OPA_COVER;

    /* Enum LV_STYLE (no C type found, emitting as constants) */
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

    /* Enum LV_TREE_WALK (no C type found, emitting as constants) */
    m.attr("TREE_WALK_PRE_ORDER") = (int)LV_TREE_WALK_PRE_ORDER;
    m.attr("TREE_WALK_POST_ORDER") = (int)LV_TREE_WALK_POST_ORDER;

    py::enum_<lv_result_t>(m, "RESULT")
        .value("INVALID", LV_RESULT_INVALID)
        .value("OK", LV_RESULT_OK)
        .export_values();

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
        .export_values();

    py::enum_<lv_dir_t>(m, "DIR")
        .value("NONE", LV_DIR_NONE)
        .value("LEFT", LV_DIR_LEFT)
        .value("RIGHT", LV_DIR_RIGHT)
        .value("TOP", LV_DIR_TOP)
        .value("BOTTOM", LV_DIR_BOTTOM)
        .value("HOR", LV_DIR_HOR)
        .value("VER", LV_DIR_VER)
        .value("ALL", LV_DIR_ALL)
        .export_values();

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
        .value("EVENT_KEY", LV_EVENT_KEY)
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
        .export_values();

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
        .export_values();

    py::enum_<lv_group_refocus_policy_t>(m, "GROUP_REFOCUS_POLICY")
        .value("NEXT", LV_GROUP_REFOCUS_POLICY_NEXT)
        .value("PREV", LV_GROUP_REFOCUS_POLICY_PREV)
        .export_values();

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
        .export_values();

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
        .export_values();

    py::enum_<lv_image_flags_t>(m, "IMAGE_FLAGS")
        .value("PREMULTIPLIED", LV_IMAGE_FLAGS_PREMULTIPLIED)
        .value("COMPRESSED", LV_IMAGE_FLAGS_COMPRESSED)
        .value("ALLOCATED", LV_IMAGE_FLAGS_ALLOCATED)
        .value("MODIFIABLE", LV_IMAGE_FLAGS_MODIFIABLE)
        .value("CUSTOM_DRAW", LV_IMAGE_FLAGS_CUSTOM_DRAW)
        .value("USER1", LV_IMAGE_FLAGS_USER1)
        .value("USER2", LV_IMAGE_FLAGS_USER2)
        .value("USER3", LV_IMAGE_FLAGS_USER3)
        .value("USER4", LV_IMAGE_FLAGS_USER4)
        .value("USER5", LV_IMAGE_FLAGS_USER5)
        .value("USER6", LV_IMAGE_FLAGS_USER6)
        .value("USER7", LV_IMAGE_FLAGS_USER7)
        .value("USER8", LV_IMAGE_FLAGS_USER8)
        .export_values();

    py::enum_<lv_image_compress_t>(m, "IMAGE_COMPRESS")
        .value("NONE", LV_IMAGE_COMPRESS_NONE)
        .value("RLE", LV_IMAGE_COMPRESS_RLE)
        .value("LZ4", LV_IMAGE_COMPRESS_LZ4)
        .export_values();

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
        .export_values();

    py::enum_<lv_font_subpx_t>(m, "FONT_SUBPX")
        .value("NONE", LV_FONT_SUBPX_NONE)
        .value("HOR", LV_FONT_SUBPX_HOR)
        .value("VER", LV_FONT_SUBPX_VER)
        .value("BOTH", LV_FONT_SUBPX_BOTH)
        .export_values();

    py::enum_<lv_font_kerning_t>(m, "FONT_KERNING")
        .value("NORMAL", LV_FONT_KERNING_NORMAL)
        .value("NONE", LV_FONT_KERNING_NONE)
        .export_values();

    py::enum_<lv_text_flag_t>(m, "TEXT_FLAG")
        .value("NONE", LV_TEXT_FLAG_NONE)
        .value("EXPAND", LV_TEXT_FLAG_EXPAND)
        .value("FIT", LV_TEXT_FLAG_FIT)
        .value("BREAK_ALL", LV_TEXT_FLAG_BREAK_ALL)
        .value("RECOLOR", LV_TEXT_FLAG_RECOLOR)
        .export_values();

    py::enum_<lv_text_align_t>(m, "TEXT_ALIGN")
        .value("AUTO", LV_TEXT_ALIGN_AUTO)
        .value("LEFT", LV_TEXT_ALIGN_LEFT)
        .value("CENTER", LV_TEXT_ALIGN_CENTER)
        .value("RIGHT", LV_TEXT_ALIGN_RIGHT)
        .export_values();

    py::enum_<lv_text_leading_trim_t>(m, "TEXT_LEADING_TRIM")
        .value("NONE", LV_TEXT_LEADING_TRIM_NONE)
        .value("CAPITAL_BASELINE", LV_TEXT_LEADING_TRIM_CAPITAL_BASELINE)
        .value("LOWER_BASELINE", LV_TEXT_LEADING_TRIM_LOWER_BASELINE)
        .value("CAPITAL", LV_TEXT_LEADING_TRIM_CAPITAL)
        .value("LOWER", LV_TEXT_LEADING_TRIM_LOWER)
        .export_values();

    py::enum_<lv_base_dir_t>(m, "BASE_DIR")
        .value("LTR", LV_BASE_DIR_LTR)
        .value("RTL", LV_BASE_DIR_RTL)
        .value("AUTO", LV_BASE_DIR_AUTO)
        .value("NEUTRAL", LV_BASE_DIR_NEUTRAL)
        .value("WEAK", LV_BASE_DIR_WEAK)
        .export_values();

    py::enum_<lv_grad_dir_t>(m, "GRAD_DIR")
        .value("NONE", LV_GRAD_DIR_NONE)
        .value("VER", LV_GRAD_DIR_VER)
        .value("HOR", LV_GRAD_DIR_HOR)
        .value("LINEAR", LV_GRAD_DIR_LINEAR)
        .value("RADIAL", LV_GRAD_DIR_RADIAL)
        .value("CONICAL", LV_GRAD_DIR_CONICAL)
        .export_values();

    py::enum_<lv_grad_extend_t>(m, "GRAD_EXTEND")
        .value("PAD", LV_GRAD_EXTEND_PAD)
        .value("REPEAT", LV_GRAD_EXTEND_REPEAT)
        .value("REFLECT", LV_GRAD_EXTEND_REFLECT)
        .export_values();

    py::enum_<lv_layout_t>(m, "LAYOUT")
        .value("NONE", LV_LAYOUT_NONE)
        .value("FLEX", LV_LAYOUT_FLEX)
        .value("GRID", LV_LAYOUT_GRID)
        .value("LAST", LV_LAYOUT_LAST)
        .export_values();

    py::enum_<lv_flex_align_t>(m, "FLEX_ALIGN")
        .value("START", LV_FLEX_ALIGN_START)
        .value("END", LV_FLEX_ALIGN_END)
        .value("CENTER", LV_FLEX_ALIGN_CENTER)
        .value("SPACE_EVENLY", LV_FLEX_ALIGN_SPACE_EVENLY)
        .value("SPACE_AROUND", LV_FLEX_ALIGN_SPACE_AROUND)
        .value("SPACE_BETWEEN", LV_FLEX_ALIGN_SPACE_BETWEEN)
        .export_values();

    py::enum_<lv_flex_flow_t>(m, "FLEX_FLOW")
        .value("ROW", LV_FLEX_FLOW_ROW)
        .value("COLUMN", LV_FLEX_FLOW_COLUMN)
        .value("ROW_WRAP", LV_FLEX_FLOW_ROW_WRAP)
        .value("ROW_REVERSE", LV_FLEX_FLOW_ROW_REVERSE)
        .value("ROW_WRAP_REVERSE", LV_FLEX_FLOW_ROW_WRAP_REVERSE)
        .value("COLUMN_WRAP", LV_FLEX_FLOW_COLUMN_WRAP)
        .value("COLUMN_REVERSE", LV_FLEX_FLOW_COLUMN_REVERSE)
        .value("COLUMN_WRAP_REVERSE", LV_FLEX_FLOW_COLUMN_WRAP_REVERSE)
        .export_values();

    py::enum_<lv_grid_align_t>(m, "GRID_ALIGN")
        .value("START", LV_GRID_ALIGN_START)
        .value("CENTER", LV_GRID_ALIGN_CENTER)
        .value("END", LV_GRID_ALIGN_END)
        .value("STRETCH", LV_GRID_ALIGN_STRETCH)
        .value("SPACE_EVENLY", LV_GRID_ALIGN_SPACE_EVENLY)
        .value("SPACE_AROUND", LV_GRID_ALIGN_SPACE_AROUND)
        .value("SPACE_BETWEEN", LV_GRID_ALIGN_SPACE_BETWEEN)
        .export_values();

    py::enum_<lv_blend_mode_t>(m, "BLEND_MODE")
        .value("NORMAL", LV_BLEND_MODE_NORMAL)
        .value("ADDITIVE", LV_BLEND_MODE_ADDITIVE)
        .value("SUBTRACTIVE", LV_BLEND_MODE_SUBTRACTIVE)
        .value("MULTIPLY", LV_BLEND_MODE_MULTIPLY)
        .value("DIFFERENCE", LV_BLEND_MODE_DIFFERENCE)
        .export_values();

    py::enum_<lv_text_decor_t>(m, "TEXT_DECOR")
        .value("NONE", LV_TEXT_DECOR_NONE)
        .value("UNDERLINE", LV_TEXT_DECOR_UNDERLINE)
        .value("STRIKETHROUGH", LV_TEXT_DECOR_STRIKETHROUGH)
        .export_values();

    py::enum_<lv_border_side_t>(m, "BORDER_SIDE")
        .value("NONE", LV_BORDER_SIDE_NONE)
        .value("BOTTOM", LV_BORDER_SIDE_BOTTOM)
        .value("TOP", LV_BORDER_SIDE_TOP)
        .value("LEFT", LV_BORDER_SIDE_LEFT)
        .value("RIGHT", LV_BORDER_SIDE_RIGHT)
        .value("FULL", LV_BORDER_SIDE_FULL)
        .value("INTERNAL", LV_BORDER_SIDE_INTERNAL)
        .export_values();

    py::enum_<lv_blur_quality_t>(m, "BLUR_QUALITY")
        .value("AUTO", LV_BLUR_QUALITY_AUTO)
        .value("SPEED", LV_BLUR_QUALITY_SPEED)
        .value("PRECISION", LV_BLUR_QUALITY_PRECISION)
        .export_values();

    py::enum_<lv_style_res_t>(m, "STYLE_RES")
        .value("NOT_FOUND", LV_STYLE_RES_NOT_FOUND)
        .value("FOUND", LV_STYLE_RES_FOUND)
        .export_values();

    py::enum_<lv_display_rotation_t>(m, "DISPLAY_ROTATION")
        .value("_0", LV_DISPLAY_ROTATION_0)
        .value("_90", LV_DISPLAY_ROTATION_90)
        .value("_180", LV_DISPLAY_ROTATION_180)
        .value("_270", LV_DISPLAY_ROTATION_270)
        .export_values();

    py::enum_<lv_display_render_mode_t>(m, "DISPLAY_RENDER_MODE")
        .value("PARTIAL", LV_DISPLAY_RENDER_MODE_PARTIAL)
        .value("DIRECT", LV_DISPLAY_RENDER_MODE_DIRECT)
        .value("FULL", LV_DISPLAY_RENDER_MODE_FULL)
        .export_values();

    py::enum_<lv_screen_load_anim_t>(m, "SCREEN_LOAD_ANIM")
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
        .export_values();

    py::enum_<lv_obj_tree_walk_res_t>(m, "OBJ_TREE_WALK")
        .value("NEXT", LV_OBJ_TREE_WALK_NEXT)
        .value("SKIP_CHILDREN", LV_OBJ_TREE_WALK_SKIP_CHILDREN)
        .value("END", LV_OBJ_TREE_WALK_END)
        .export_values();

    py::enum_<lv_obj_point_transform_flag_t>(m, "OBJ_POINT_TRANSFORM_FLAG")
        .value("NONE", LV_OBJ_POINT_TRANSFORM_FLAG_NONE)
        .value("RECURSIVE", LV_OBJ_POINT_TRANSFORM_FLAG_RECURSIVE)
        .value("INVERSE", LV_OBJ_POINT_TRANSFORM_FLAG_INVERSE)
        .value("INVERSE_RECURSIVE", LV_OBJ_POINT_TRANSFORM_FLAG_INVERSE_RECURSIVE)
        .export_values();

    py::enum_<lv_scrollbar_mode_t>(m, "SCROLLBAR_MODE")
        .value("OFF", LV_SCROLLBAR_MODE_OFF)
        .value("ON", LV_SCROLLBAR_MODE_ON)
        .value("ACTIVE", LV_SCROLLBAR_MODE_ACTIVE)
        .value("AUTO", LV_SCROLLBAR_MODE_AUTO)
        .export_values();

    py::enum_<lv_scroll_snap_t>(m, "SCROLL_SNAP")
        .value("NONE", LV_SCROLL_SNAP_NONE)
        .value("START", LV_SCROLL_SNAP_START)
        .value("END", LV_SCROLL_SNAP_END)
        .value("CENTER", LV_SCROLL_SNAP_CENTER)
        .export_values();

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
        .export_values();

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
        .export_values();

    py::enum_<lv_style_state_cmp_t>(m, "STYLE_STATE_CMP")
        .value("SAME", LV_STYLE_STATE_CMP_SAME)
        .value("DIFF_REDRAW", LV_STYLE_STATE_CMP_DIFF_REDRAW)
        .value("DIFF_DRAW_PAD", LV_STYLE_STATE_CMP_DIFF_DRAW_PAD)
        .value("DIFF_LAYOUT", LV_STYLE_STATE_CMP_DIFF_LAYOUT)
        .export_values();

    py::enum_<lv_draw_task_type_t>(m, "DRAW_TASK_TYPE")
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
        .export_values();

    py::enum_<lv_draw_task_state_t>(m, "DRAW_TASK_STATE")
        .value("BLOCKED", LV_DRAW_TASK_STATE_BLOCKED)
        .value("WAITING", LV_DRAW_TASK_STATE_WAITING)
        .value("QUEUED", LV_DRAW_TASK_STATE_QUEUED)
        .value("IN_PROGRESS", LV_DRAW_TASK_STATE_IN_PROGRESS)
        .value("FINISHED", LV_DRAW_TASK_STATE_FINISHED)
        .export_values();

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
        .export_values();

    py::enum_<lv_fs_mode_t>(m, "FS_MODE")
        .value("WR", LV_FS_MODE_WR)
        .value("RD", LV_FS_MODE_RD)
        .export_values();

    py::enum_<lv_fs_whence_t>(m, "FS_SEEK")
        .value("SET", LV_FS_SEEK_SET)
        .value("CUR", LV_FS_SEEK_CUR)
        .value("END", LV_FS_SEEK_END)
        .export_values();

    py::enum_<lv_image_src_t>(m, "IMAGE_SRC")
        .value("VARIABLE", LV_IMAGE_SRC_VARIABLE)
        .value("FILE", LV_IMAGE_SRC_FILE)
        .value("SYMBOL", LV_IMAGE_SRC_SYMBOL)
        .value("UNKNOWN", LV_IMAGE_SRC_UNKNOWN)
        .export_values();

    py::enum_<lv_layer_type_t>(m, "LAYER_TYPE")
        .value("NONE", LV_LAYER_TYPE_NONE)
        .value("SIMPLE", LV_LAYER_TYPE_SIMPLE)
        .value("TRANSFORM", LV_LAYER_TYPE_TRANSFORM)
        .export_values();

    py::enum_<lv_obj_class_editable_t>(m, "OBJ_CLASS_EDITABLE")
        .value("INHERIT", LV_OBJ_CLASS_EDITABLE_INHERIT)
        .value("TRUE", LV_OBJ_CLASS_EDITABLE_TRUE)
        .value("FALSE", LV_OBJ_CLASS_EDITABLE_FALSE)
        .export_values();

    py::enum_<lv_obj_class_group_def_t>(m, "OBJ_CLASS_GROUP_DEF")
        .value("INHERIT", LV_OBJ_CLASS_GROUP_DEF_INHERIT)
        .value("TRUE", LV_OBJ_CLASS_GROUP_DEF_TRUE)
        .value("FALSE", LV_OBJ_CLASS_GROUP_DEF_FALSE)
        .export_values();

    py::enum_<lv_obj_class_theme_inheritable_t>(m, "OBJ_CLASS_THEME_INHERITABLE")
        .value("FALSE", LV_OBJ_CLASS_THEME_INHERITABLE_FALSE)
        .value("TRUE", LV_OBJ_CLASS_THEME_INHERITABLE_TRUE)
        .export_values();

    py::enum_<lv_indev_type_t>(m, "INDEV_TYPE")
        .value("NONE", LV_INDEV_TYPE_NONE)
        .value("POINTER", LV_INDEV_TYPE_POINTER)
        .value("KEYPAD", LV_INDEV_TYPE_KEYPAD)
        .value("BUTTON", LV_INDEV_TYPE_BUTTON)
        .value("ENCODER", LV_INDEV_TYPE_ENCODER)
        .export_values();

    py::enum_<lv_indev_state_t>(m, "INDEV_STATE")
        .value("RELEASED", LV_INDEV_STATE_RELEASED)
        .value("PRESSED", LV_INDEV_STATE_PRESSED)
        .export_values();

    py::enum_<lv_indev_mode_t>(m, "INDEV_MODE")
        .value("NONE", LV_INDEV_MODE_NONE)
        .value("TIMER", LV_INDEV_MODE_TIMER)
        .value("INDEV_MODE_EVENT", LV_INDEV_MODE_EVENT)
        .export_values();

    py::enum_<lv_indev_gesture_type_t>(m, "INDEV_GESTURE")
        .value("NONE", LV_INDEV_GESTURE_NONE)
        .value("PINCH", LV_INDEV_GESTURE_PINCH)
        .value("SWIPE", LV_INDEV_GESTURE_SWIPE)
        .value("ROTATE", LV_INDEV_GESTURE_ROTATE)
        .value("TWO_FINGERS_SWIPE", LV_INDEV_GESTURE_TWO_FINGERS_SWIPE)
        .value("SCROLL", LV_INDEV_GESTURE_SCROLL)
        .value("CNT", LV_INDEV_GESTURE_CNT)
        .export_values();

    py::enum_<lv_cover_res_t>(m, "COVER_RES")
        .value("COVER", LV_COVER_RES_COVER)
        .value("NOT_COVER", LV_COVER_RES_NOT_COVER)
        .value("MASKED", LV_COVER_RES_MASKED)
        .export_values();

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
        .export_values();

    py::enum_<lv_subject_type_t>(m, "SUBJECT_TYPE")
        .value("INVALID", LV_SUBJECT_TYPE_INVALID)
        .value("NONE", LV_SUBJECT_TYPE_NONE)
        .value("INT", LV_SUBJECT_TYPE_INT)
        .value("FLOAT", LV_SUBJECT_TYPE_FLOAT)
        .value("POINTER", LV_SUBJECT_TYPE_POINTER)
        .value("COLOR", LV_SUBJECT_TYPE_COLOR)
        .value("GROUP", LV_SUBJECT_TYPE_GROUP)
        .value("STRING", LV_SUBJECT_TYPE_STRING)
        .export_values();

    py::enum_<lv_vector_fill_t>(m, "VECTOR_FILL")
        .value("NONZERO", LV_VECTOR_FILL_NONZERO)
        .value("EVENODD", LV_VECTOR_FILL_EVENODD)
        .export_values();

    py::enum_<lv_vector_stroke_cap_t>(m, "VECTOR_STROKE_CAP")
        .value("BUTT", LV_VECTOR_STROKE_CAP_BUTT)
        .value("SQUARE", LV_VECTOR_STROKE_CAP_SQUARE)
        .value("ROUND", LV_VECTOR_STROKE_CAP_ROUND)
        .export_values();

    py::enum_<lv_vector_stroke_join_t>(m, "VECTOR_STROKE_JOIN")
        .value("MITER", LV_VECTOR_STROKE_JOIN_MITER)
        .value("BEVEL", LV_VECTOR_STROKE_JOIN_BEVEL)
        .value("ROUND", LV_VECTOR_STROKE_JOIN_ROUND)
        .export_values();

    py::enum_<lv_vector_path_quality_t>(m, "VECTOR_PATH_QUALITY")
        .value("MEDIUM", LV_VECTOR_PATH_QUALITY_MEDIUM)
        .value("HIGH", LV_VECTOR_PATH_QUALITY_HIGH)
        .value("LOW", LV_VECTOR_PATH_QUALITY_LOW)
        .export_values();

    py::enum_<lv_vector_blend_t>(m, "VECTOR_BLEND")
        .value("SRC_OVER", LV_VECTOR_BLEND_SRC_OVER)
        .value("SRC_IN", LV_VECTOR_BLEND_SRC_IN)
        .value("DST_OVER", LV_VECTOR_BLEND_DST_OVER)
        .value("DST_IN", LV_VECTOR_BLEND_DST_IN)
        .value("SCREEN", LV_VECTOR_BLEND_SCREEN)
        .value("MULTIPLY", LV_VECTOR_BLEND_MULTIPLY)
        .value("NONE", LV_VECTOR_BLEND_NONE)
        .value("ADDITIVE", LV_VECTOR_BLEND_ADDITIVE)
        .value("SUBTRACTIVE", LV_VECTOR_BLEND_SUBTRACTIVE)
        .export_values();

    py::enum_<lv_vector_path_op_t>(m, "VECTOR_PATH_OP")
        .value("MOVE_TO", LV_VECTOR_PATH_OP_MOVE_TO)
        .value("LINE_TO", LV_VECTOR_PATH_OP_LINE_TO)
        .value("QUAD_TO", LV_VECTOR_PATH_OP_QUAD_TO)
        .value("CUBIC_TO", LV_VECTOR_PATH_OP_CUBIC_TO)
        .value("CLOSE", LV_VECTOR_PATH_OP_CLOSE)
        .export_values();

    py::enum_<lv_vector_draw_style_t>(m, "VECTOR_DRAW_STYLE")
        .value("SOLID", LV_VECTOR_DRAW_STYLE_SOLID)
        .value("PATTERN", LV_VECTOR_DRAW_STYLE_PATTERN)
        .value("GRADIENT", LV_VECTOR_DRAW_STYLE_GRADIENT)
        .export_values();

    py::enum_<lv_vector_gradient_spread_t>(m, "VECTOR_GRADIENT_SPREAD")
        .value("PAD", LV_VECTOR_GRADIENT_SPREAD_PAD)
        .value("REPEAT", LV_VECTOR_GRADIENT_SPREAD_REPEAT)
        .value("REFLECT", LV_VECTOR_GRADIENT_SPREAD_REFLECT)
        .export_values();

    py::enum_<lv_vector_gradient_style_t>(m, "VECTOR_GRADIENT_STYLE")
        .value("LINEAR", LV_VECTOR_GRADIENT_STYLE_LINEAR)
        .value("RADIAL", LV_VECTOR_GRADIENT_STYLE_RADIAL)
        .export_values();

    py::enum_<lv_vector_fill_units_t>(m, "VECTOR_FILL_UNITS")
        .value("OBJECT_BOUNDING_BOX", LV_VECTOR_FILL_UNITS_OBJECT_BOUNDING_BOX)
        .value("USER_SPACE_ON_USE", LV_VECTOR_FILL_UNITS_USER_SPACE_ON_USE)
        .export_values();

    py::enum_<lv_evdev_type_t>(m, "EVDEV_TYPE")
        .value("REL", LV_EVDEV_TYPE_REL)
        .value("ABS", LV_EVDEV_TYPE_ABS)
        .value("EVDEV_TYPE_KEY", LV_EVDEV_TYPE_KEY)
        .export_values();

    py::enum_<lv_font_fmt_txt_cmap_type_t>(m, "FONT_FMT_TXT_CMAP")
        .value("FORMAT0_FULL", LV_FONT_FMT_TXT_CMAP_FORMAT0_FULL)
        .value("SPARSE_FULL", LV_FONT_FMT_TXT_CMAP_SPARSE_FULL)
        .value("FORMAT0_TINY", LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY)
        .value("SPARSE_TINY", LV_FONT_FMT_TXT_CMAP_SPARSE_TINY)
        .export_values();

    py::enum_<lv_font_fmt_txt_bitmap_format_t>(m, "FONT_FMT_TXT")
        .value("PLAIN", LV_FONT_FMT_TXT_PLAIN)
        .value("COMPRESSED", LV_FONT_FMT_TXT_COMPRESSED)
        .value("COMPRESSED_NO_PREFILTER", LV_FONT_FMT_TXT_COMPRESSED_NO_PREFILTER)
        .export_values();

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
        .value("STRETCH", LV_IMAGE_ALIGN_STRETCH)
        .value("TILE", LV_IMAGE_ALIGN_TILE)
        .value("CONTAIN", LV_IMAGE_ALIGN_CONTAIN)
        .value("CONTAIN_DOWNSCALE", LV_IMAGE_ALIGN_CONTAIN_DOWNSCALE)
        .value("COVER", LV_IMAGE_ALIGN_COVER)
        .export_values();

    py::enum_<lv_animimg_part_t>(m, "ANIM_IMAGE_PART")
        .value("MAIN", LV_ANIM_IMAGE_PART_MAIN)
        .export_values();

    py::enum_<lv_arc_mode_t>(m, "ARC_MODE")
        .value("NORMAL", LV_ARC_MODE_NORMAL)
        .value("SYMMETRICAL", LV_ARC_MODE_SYMMETRICAL)
        .value("REVERSE", LV_ARC_MODE_REVERSE)
        .export_values();

    py::enum_<lv_arclabel_dir_t>(m, "ARCLABEL_DIR")
        .value("CLOCKWISE", LV_ARCLABEL_DIR_CLOCKWISE)
        .value("COUNTER_CLOCKWISE", LV_ARCLABEL_DIR_COUNTER_CLOCKWISE)
        .export_values();

    py::enum_<lv_arclabel_text_align_t>(m, "ARCLABEL_TEXT_ALIGN")
        .value("DEFAULT", LV_ARCLABEL_TEXT_ALIGN_DEFAULT)
        .value("LEADING", LV_ARCLABEL_TEXT_ALIGN_LEADING)
        .value("CENTER", LV_ARCLABEL_TEXT_ALIGN_CENTER)
        .value("TRAILING", LV_ARCLABEL_TEXT_ALIGN_TRAILING)
        .export_values();

    py::enum_<lv_arclabel_overflow_t>(m, "ARCLABEL_OVERFLOW")
        .value("VISIBLE", LV_ARCLABEL_OVERFLOW_VISIBLE)
        .value("ELLIPSIS", LV_ARCLABEL_OVERFLOW_ELLIPSIS)
        .value("CLIP", LV_ARCLABEL_OVERFLOW_CLIP)
        .export_values();

    py::enum_<lv_label_long_mode_t>(m, "LABEL_LONG_MODE")
        .value("WRAP", LV_LABEL_LONG_MODE_WRAP)
        .value("DOTS", LV_LABEL_LONG_MODE_DOTS)
        .value("SCROLL", LV_LABEL_LONG_MODE_SCROLL)
        .value("SCROLL_CIRCULAR", LV_LABEL_LONG_MODE_SCROLL_CIRCULAR)
        .value("CLIP", LV_LABEL_LONG_MODE_CLIP)
        .export_values();

    py::enum_<lv_bar_mode_t>(m, "BAR_MODE")
        .value("NORMAL", LV_BAR_MODE_NORMAL)
        .value("SYMMETRICAL", LV_BAR_MODE_SYMMETRICAL)
        .value("RANGE", LV_BAR_MODE_RANGE)
        .export_values();

    py::enum_<lv_bar_orientation_t>(m, "BAR_ORIENTATION")
        .value("AUTO", LV_BAR_ORIENTATION_AUTO)
        .value("HORIZONTAL", LV_BAR_ORIENTATION_HORIZONTAL)
        .value("VERTICAL", LV_BAR_ORIENTATION_VERTICAL)
        .export_values();

    py::enum_<lv_barcode_encoding_t>(m, "BARCODE_ENCODING_CODE128")
        .value("GS1", LV_BARCODE_ENCODING_CODE128_GS1)
        .value("RAW", LV_BARCODE_ENCODING_CODE128_RAW)
        .export_values();

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
        .export_values();

    py::enum_<lv_chart_type_t>(m, "CHART_TYPE")
        .value("NONE", LV_CHART_TYPE_NONE)
        .value("LINE", LV_CHART_TYPE_LINE)
        .value("CURVE", LV_CHART_TYPE_CURVE)
        .value("BAR", LV_CHART_TYPE_BAR)
        .value("STACKED", LV_CHART_TYPE_STACKED)
        .value("SCATTER", LV_CHART_TYPE_SCATTER)
        .export_values();

    py::enum_<lv_chart_update_mode_t>(m, "CHART_UPDATE_MODE")
        .value("SHIFT", LV_CHART_UPDATE_MODE_SHIFT)
        .value("CIRCULAR", LV_CHART_UPDATE_MODE_CIRCULAR)
        .export_values();

    py::enum_<lv_chart_axis_t>(m, "CHART_AXIS")
        .value("PRIMARY_Y", LV_CHART_AXIS_PRIMARY_Y)
        .value("SECONDARY_Y", LV_CHART_AXIS_SECONDARY_Y)
        .value("PRIMARY_X", LV_CHART_AXIS_PRIMARY_X)
        .value("SECONDARY_X", LV_CHART_AXIS_SECONDARY_X)
        .value("LAST", LV_CHART_AXIS_LAST)
        .export_values();

    py::enum_<lv_imagebutton_state_t>(m, "IMAGEBUTTON_STATE")
        .value("RELEASED", LV_IMAGEBUTTON_STATE_RELEASED)
        .value("PRESSED", LV_IMAGEBUTTON_STATE_PRESSED)
        .value("DISABLED", LV_IMAGEBUTTON_STATE_DISABLED)
        .value("CHECKED_RELEASED", LV_IMAGEBUTTON_STATE_CHECKED_RELEASED)
        .value("CHECKED_PRESSED", LV_IMAGEBUTTON_STATE_CHECKED_PRESSED)
        .value("CHECKED_DISABLED", LV_IMAGEBUTTON_STATE_CHECKED_DISABLED)
        .value("NUM", LV_IMAGEBUTTON_STATE_NUM)
        .export_values();

    py::enum_<lv_keyboard_mode_t>(m, "KEYBOARD_MODE")
        .value("TEXT_LOWER", LV_KEYBOARD_MODE_TEXT_LOWER)
        .value("TEXT_UPPER", LV_KEYBOARD_MODE_TEXT_UPPER)
        .value("SPECIAL", LV_KEYBOARD_MODE_SPECIAL)
        .value("NUMBER", LV_KEYBOARD_MODE_NUMBER)
        .value("USER_1", LV_KEYBOARD_MODE_USER_1)
        .value("USER_2", LV_KEYBOARD_MODE_USER_2)
        .value("USER_3", LV_KEYBOARD_MODE_USER_3)
        .value("USER_4", LV_KEYBOARD_MODE_USER_4)
        .export_values();

    py::enum_<lv_menu_mode_header_t>(m, "MENU_HEADER")
        .value("TOP_FIXED", LV_MENU_HEADER_TOP_FIXED)
        .value("TOP_UNFIXED", LV_MENU_HEADER_TOP_UNFIXED)
        .value("BOTTOM_FIXED", LV_MENU_HEADER_BOTTOM_FIXED)
        .export_values();

    py::enum_<lv_menu_mode_root_back_button_t>(m, "MENU_ROOT_BACK_BUTTON")
        .value("DISABLED", LV_MENU_ROOT_BACK_BUTTON_DISABLED)
        .value("ENABLED", LV_MENU_ROOT_BACK_BUTTON_ENABLED)
        .export_values();

    py::enum_<lv_roller_mode_t>(m, "ROLLER_MODE")
        .value("NORMAL", LV_ROLLER_MODE_NORMAL)
        .value("INFINITE", LV_ROLLER_MODE_INFINITE)
        .export_values();

    py::enum_<lv_scale_mode_t>(m, "SCALE_MODE")
        .value("HORIZONTAL_TOP", LV_SCALE_MODE_HORIZONTAL_TOP)
        .value("HORIZONTAL_BOTTOM", LV_SCALE_MODE_HORIZONTAL_BOTTOM)
        .value("VERTICAL_LEFT", LV_SCALE_MODE_VERTICAL_LEFT)
        .value("VERTICAL_RIGHT", LV_SCALE_MODE_VERTICAL_RIGHT)
        .value("ROUND_INNER", LV_SCALE_MODE_ROUND_INNER)
        .value("ROUND_OUTER", LV_SCALE_MODE_ROUND_OUTER)
        .value("LAST", LV_SCALE_MODE_LAST)
        .export_values();

    py::enum_<lv_slider_mode_t>(m, "SLIDER_MODE")
        .value("NORMAL", LV_SLIDER_MODE_NORMAL)
        .value("SYMMETRICAL", LV_SLIDER_MODE_SYMMETRICAL)
        .value("RANGE", LV_SLIDER_MODE_RANGE)
        .export_values();

    py::enum_<lv_slider_orientation_t>(m, "SLIDER_ORIENTATION")
        .value("AUTO", LV_SLIDER_ORIENTATION_AUTO)
        .value("HORIZONTAL", LV_SLIDER_ORIENTATION_HORIZONTAL)
        .value("VERTICAL", LV_SLIDER_ORIENTATION_VERTICAL)
        .export_values();

    py::enum_<lv_span_overflow_t>(m, "SPAN_OVERFLOW")
        .value("CLIP", LV_SPAN_OVERFLOW_CLIP)
        .value("ELLIPSIS", LV_SPAN_OVERFLOW_ELLIPSIS)
        .value("LAST", LV_SPAN_OVERFLOW_LAST)
        .export_values();

    py::enum_<lv_span_mode_t>(m, "SPAN_MODE")
        .value("FIXED", LV_SPAN_MODE_FIXED)
        .value("EXPAND", LV_SPAN_MODE_EXPAND)
        .value("BREAK", LV_SPAN_MODE_BREAK)
        .value("LAST", LV_SPAN_MODE_LAST)
        .export_values();

    py::enum_<lv_switch_orientation_t>(m, "SWITCH_ORIENTATION")
        .value("AUTO", LV_SWITCH_ORIENTATION_AUTO)
        .value("HORIZONTAL", LV_SWITCH_ORIENTATION_HORIZONTAL)
        .value("VERTICAL", LV_SWITCH_ORIENTATION_VERTICAL)
        .export_values();

    py::enum_<lv_table_cell_ctrl_t>(m, "TABLE_CELL_CTRL")
        .value("NONE", LV_TABLE_CELL_CTRL_NONE)
        .value("MERGE_RIGHT", LV_TABLE_CELL_CTRL_MERGE_RIGHT)
        .value("TEXT_CROP", LV_TABLE_CELL_CTRL_TEXT_CROP)
        .value("CUSTOM_1", LV_TABLE_CELL_CTRL_CUSTOM_1)
        .value("CUSTOM_2", LV_TABLE_CELL_CTRL_CUSTOM_2)
        .value("CUSTOM_3", LV_TABLE_CELL_CTRL_CUSTOM_3)
        .value("CUSTOM_4", LV_TABLE_CELL_CTRL_CUSTOM_4)
        .export_values();

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

    /* Base Object class */
    auto obj_cls = py::class_<LvObjWrapper>(m, "Obj")
        .def(py::init<>())
        .def_static("create", [](LvObjWrapper *parent) -> LvObjWrapper* {
            lv_obj_t *p = parent ? parent->get() : nullptr;
            return new LvObjWrapper(lv_obj_create(p));
        }, py::arg("parent") = py::none())
        .def("set_style_pad_all", [](LvObjWrapper &self, int value, int selector) { lv_obj_set_style_pad_all(self.get(), value, selector); }
            , py::arg("value"), py::arg("selector"))
        .def("set_style_pad_hor", [](LvObjWrapper &self, int value, int selector) { lv_obj_set_style_pad_hor(self.get(), value, selector); }
            , py::arg("value"), py::arg("selector"))
        .def("set_style_pad_ver", [](LvObjWrapper &self, int value, int selector) { lv_obj_set_style_pad_ver(self.get(), value, selector); }
            , py::arg("value"), py::arg("selector"))
        .def("set_style_margin_all", [](LvObjWrapper &self, int value, int selector) { lv_obj_set_style_margin_all(self.get(), value, selector); }
            , py::arg("value"), py::arg("selector"))
        .def("set_style_margin_hor", [](LvObjWrapper &self, int value, int selector) { lv_obj_set_style_margin_hor(self.get(), value, selector); }
            , py::arg("value"), py::arg("selector"))
        .def("set_style_margin_ver", [](LvObjWrapper &self, int value, int selector) { lv_obj_set_style_margin_ver(self.get(), value, selector); }
            , py::arg("value"), py::arg("selector"))
        .def("set_style_pad_gap", [](LvObjWrapper &self, int value, int selector) { lv_obj_set_style_pad_gap(self.get(), value, selector); }
            , py::arg("value"), py::arg("selector"))
        .def("set_style_size", [](LvObjWrapper &self, int width, int height, int selector) { lv_obj_set_style_size(self.get(), width, height, selector); }
            , py::arg("width"), py::arg("height"), py::arg("selector"))
        .def("set_style_transform_scale", [](LvObjWrapper &self, int value, int selector) { lv_obj_set_style_transform_scale(self.get(), value, selector); }
            , py::arg("value"), py::arg("selector"))
        .def("move_foreground", [](LvObjWrapper &self) { lv_obj_move_foreground(self.get()); }
            )
        .def("move_background", [](LvObjWrapper &self) { lv_obj_move_background(self.get()); }
            )
        .def("set_flex_flow", [](LvObjWrapper &self, lv_flex_flow_t flow) { lv_obj_set_flex_flow(self.get(), flow); }
            , py::arg("flow"))
        .def("set_flex_align", [](LvObjWrapper &self, lv_flex_align_t main_place, lv_flex_align_t cross_place, lv_flex_align_t track_cross_place) { lv_obj_set_flex_align(self.get(), main_place, cross_place, track_cross_place); }
            , py::arg("main_place"), py::arg("cross_place"), py::arg("track_cross_place"))
        .def("set_flex_grow", [](LvObjWrapper &self, int grow) { lv_obj_set_flex_grow(self.get(), grow); }
            , py::arg("grow"))
        .def("set_grid_align", [](LvObjWrapper &self, lv_grid_align_t column_align, lv_grid_align_t row_align) { lv_obj_set_grid_align(self.get(), column_align, row_align); }
            , py::arg("column_align"), py::arg("row_align"))
        .def("set_grid_cell", [](LvObjWrapper &self, lv_grid_align_t column_align, int col_pos, int col_span, lv_grid_align_t row_align, int row_pos, int row_span) { lv_obj_set_grid_cell(self.get(), column_align, col_pos, col_span, row_align, row_pos, row_span); }
            , py::arg("column_align"), py::arg("col_pos"), py::arg("col_span"), py::arg("row_align"), py::arg("row_pos"), py::arg("row_span"))
        .def("delete_obj", [](LvObjWrapper &self) { lv_obj_delete(self.get()); }
            )
        .def("clean", [](LvObjWrapper &self) { lv_obj_clean(self.get()); }
            )
        .def("delete_delayed", [](LvObjWrapper &self, int delay_ms) { lv_obj_delete_delayed(self.get(), delay_ms); }
            , py::arg("delay_ms"))
        .def("delete_async", [](LvObjWrapper &self) { lv_obj_delete_async(self.get()); }
            )
        .def("set_parent", [](LvObjWrapper &self, LvObjWrapper & parent) { lv_obj_set_parent(self.get(), parent.get()); }
            , py::arg("parent"))
        .def("swap", [](LvObjWrapper &self, LvObjWrapper & obj2) { lv_obj_swap(self.get(), obj2.get()); }
            , py::arg("obj2"))
        .def("move_to_index", [](LvObjWrapper &self, int index) { lv_obj_move_to_index(self.get(), index); }
            , py::arg("index"))
        .def("set_name", [](LvObjWrapper &self, char * name) { lv_obj_set_name(self.get(), name); }
            , py::arg("name"))
        .def("set_name_static", [](LvObjWrapper &self, char * name) { lv_obj_set_name_static(self.get(), name); }
            , py::arg("name"))
        .def("dump_tree", [](LvObjWrapper &self) { lv_obj_dump_tree(self.get()); }
            )
        .def("set_pos", [](LvObjWrapper &self, int x, int y) { lv_obj_set_pos(self.get(), x, y); }
            , py::arg("x"), py::arg("y"))
        .def("set_x", [](LvObjWrapper &self, int x) { lv_obj_set_x(self.get(), x); }
            , py::arg("x"))
        .def("set_y", [](LvObjWrapper &self, int y) { lv_obj_set_y(self.get(), y); }
            , py::arg("y"))
        .def("set_size", [](LvObjWrapper &self, int w, int h) { lv_obj_set_size(self.get(), w, h); }
            , py::arg("w"), py::arg("h"))
        .def("set_width", [](LvObjWrapper &self, int w) { lv_obj_set_width(self.get(), w); }
            , py::arg("w"))
        .def("set_height", [](LvObjWrapper &self, int h) { lv_obj_set_height(self.get(), h); }
            , py::arg("h"))
        .def("set_content_width", [](LvObjWrapper &self, int w) { lv_obj_set_content_width(self.get(), w); }
            , py::arg("w"))
        .def("set_content_height", [](LvObjWrapper &self, int h) { lv_obj_set_content_height(self.get(), h); }
            , py::arg("h"))
        .def("set_layout", [](LvObjWrapper &self, int layout) { lv_obj_set_layout(self.get(), layout); }
            , py::arg("layout"))
        .def("mark_layout_as_dirty", [](LvObjWrapper &self) { lv_obj_mark_layout_as_dirty(self.get()); }
            )
        .def("update_layout", [](LvObjWrapper &self) { lv_obj_update_layout(self.get()); }
            )
        .def("set_align", [](LvObjWrapper &self, lv_align_t align) { lv_obj_set_align(self.get(), align); }
            , py::arg("align"))
        .def("align", [](LvObjWrapper &self, lv_align_t align, int x_ofs, int y_ofs) { lv_obj_align(self.get(), align, x_ofs, y_ofs); }
            , py::arg("align"), py::arg("x_ofs"), py::arg("y_ofs"))
        .def("align_to", [](LvObjWrapper &self, LvObjWrapper & base, lv_align_t align, int x_ofs, int y_ofs) { lv_obj_align_to(self.get(), base.get(), align, x_ofs, y_ofs); }
            , py::arg("base"), py::arg("align"), py::arg("x_ofs"), py::arg("y_ofs"))
        .def("center", [](LvObjWrapper &self) { lv_obj_center(self.get()); }
            )
        .def("reset_transform", [](LvObjWrapper &self) { lv_obj_reset_transform(self.get()); }
            )
        .def("refr_pos", [](LvObjWrapper &self) { lv_obj_refr_pos(self.get()); }
            )
        .def("move_to", [](LvObjWrapper &self, int x, int y) { lv_obj_move_to(self.get(), x, y); }
            , py::arg("x"), py::arg("y"))
        .def("move_children_by", [](LvObjWrapper &self, int x_diff, int y_diff, bool ignore_floating) { lv_obj_move_children_by(self.get(), x_diff, y_diff, ignore_floating); }
            , py::arg("x_diff"), py::arg("y_diff"), py::arg("ignore_floating"))
        .def("set_ext_click_area", [](LvObjWrapper &self, int size) { lv_obj_set_ext_click_area(self.get(), size); }
            , py::arg("size"))
        .def("set_scrollbar_mode", [](LvObjWrapper &self, lv_scrollbar_mode_t mode) { lv_obj_set_scrollbar_mode(self.get(), mode); }
            , py::arg("mode"))
        .def("set_scroll_dir", [](LvObjWrapper &self, lv_dir_t dir) { lv_obj_set_scroll_dir(self.get(), dir); }
            , py::arg("dir"))
        .def("set_scroll_snap_x", [](LvObjWrapper &self, lv_scroll_snap_t align) { lv_obj_set_scroll_snap_x(self.get(), align); }
            , py::arg("align"))
        .def("set_scroll_snap_y", [](LvObjWrapper &self, lv_scroll_snap_t align) { lv_obj_set_scroll_snap_y(self.get(), align); }
            , py::arg("align"))
        .def("scroll_by", [](LvObjWrapper &self, int dx, int dy, bool anim_en) { lv_obj_scroll_by(self.get(), dx, dy, anim_en); }
            , py::arg("dx"), py::arg("dy"), py::arg("anim_en"))
        .def("scroll_by_bounded", [](LvObjWrapper &self, int dx, int dy, bool anim_en) { lv_obj_scroll_by_bounded(self.get(), dx, dy, anim_en); }
            , py::arg("dx"), py::arg("dy"), py::arg("anim_en"))
        .def("scroll_to", [](LvObjWrapper &self, int x, int y, bool anim_en) { lv_obj_scroll_to(self.get(), x, y, anim_en); }
            , py::arg("x"), py::arg("y"), py::arg("anim_en"))
        .def("scroll_to_x", [](LvObjWrapper &self, int x, bool anim_en) { lv_obj_scroll_to_x(self.get(), x, anim_en); }
            , py::arg("x"), py::arg("anim_en"))
        .def("scroll_to_y", [](LvObjWrapper &self, int y, bool anim_en) { lv_obj_scroll_to_y(self.get(), y, anim_en); }
            , py::arg("y"), py::arg("anim_en"))
        .def("scroll_to_view", [](LvObjWrapper &self, bool anim_en) { lv_obj_scroll_to_view(self.get(), anim_en); }
            , py::arg("anim_en"))
        .def("scroll_to_view_recursive", [](LvObjWrapper &self, bool anim_en) { lv_obj_scroll_to_view_recursive(self.get(), anim_en); }
            , py::arg("anim_en"))
        .def("stop_scroll_anim", [](LvObjWrapper &self) { lv_obj_stop_scroll_anim(self.get()); }
            )
        .def("update_snap", [](LvObjWrapper &self, bool anim_en) { lv_obj_update_snap(self.get(), anim_en); }
            , py::arg("anim_en"))
        .def("scrollbar_invalidate", [](LvObjWrapper &self) { lv_obj_scrollbar_invalidate(self.get()); }
            )
        .def("readjust_scroll", [](LvObjWrapper &self, bool anim_en) { lv_obj_readjust_scroll(self.get(), anim_en); }
            , py::arg("anim_en"))
        .def("remove_theme", [](LvObjWrapper &self, int selector) { lv_obj_remove_theme(self.get(), selector); }
            , py::arg("selector"))
        .def("remove_style_all", [](LvObjWrapper &self) { lv_obj_remove_style_all(self.get()); }
            )
        .def("refresh_style", [](LvObjWrapper &self, lv_part_t part, int prop) { lv_obj_refresh_style(self.get(), part, prop); }
            , py::arg("part"), py::arg("prop"))
        .def("fade_in", [](LvObjWrapper &self, int time, int delay) { lv_obj_fade_in(self.get(), time, delay); }
            , py::arg("time"), py::arg("delay"))
        .def("fade_out", [](LvObjWrapper &self, int time, int delay) { lv_obj_fade_out(self.get(), time, delay); }
            , py::arg("time"), py::arg("delay"))
        .def("set_style_width", [](LvObjWrapper &self, int value, int selector) { lv_obj_set_style_width(self.get(), value, selector); }
            , py::arg("value"), py::arg("selector"))
        .def("set_style_min_width", [](LvObjWrapper &self, int value, int selector) { lv_obj_set_style_min_width(self.get(), value, selector); }
            , py::arg("value"), py::arg("selector"))
        .def("set_style_max_width", [](LvObjWrapper &self, int value, int selector) { lv_obj_set_style_max_width(self.get(), value, selector); }
            , py::arg("value"), py::arg("selector"))
        .def("set_style_height", [](LvObjWrapper &self, int value, int selector) { lv_obj_set_style_height(self.get(), value, selector); }
            , py::arg("value"), py::arg("selector"))
        .def("set_style_min_height", [](LvObjWrapper &self, int value, int selector) { lv_obj_set_style_min_height(self.get(), value, selector); }
            , py::arg("value"), py::arg("selector"))
        .def("set_style_max_height", [](LvObjWrapper &self, int value, int selector) { lv_obj_set_style_max_height(self.get(), value, selector); }
            , py::arg("value"), py::arg("selector"))
        .def("set_style_length", [](LvObjWrapper &self, int value, int selector) { lv_obj_set_style_length(self.get(), value, selector); }
            , py::arg("value"), py::arg("selector"))
        .def("set_style_x", [](LvObjWrapper &self, int value, int selector) { lv_obj_set_style_x(self.get(), value, selector); }
            , py::arg("value"), py::arg("selector"))
        .def("set_style_y", [](LvObjWrapper &self, int value, int selector) { lv_obj_set_style_y(self.get(), value, selector); }
            , py::arg("value"), py::arg("selector"))
        .def("set_style_align", [](LvObjWrapper &self, lv_align_t value, int selector) { lv_obj_set_style_align(self.get(), value, selector); }
            , py::arg("value"), py::arg("selector"))
        .def("set_style_transform_width", [](LvObjWrapper &self, int value, int selector) { lv_obj_set_style_transform_width(self.get(), value, selector); }
            , py::arg("value"), py::arg("selector"))
        .def("set_style_transform_height", [](LvObjWrapper &self, int value, int selector) { lv_obj_set_style_transform_height(self.get(), value, selector); }
            , py::arg("value"), py::arg("selector"))
        .def("set_style_translate_x", [](LvObjWrapper &self, int value, int selector) { lv_obj_set_style_translate_x(self.get(), value, selector); }
            , py::arg("value"), py::arg("selector"))
        .def("set_style_translate_y", [](LvObjWrapper &self, int value, int selector) { lv_obj_set_style_translate_y(self.get(), value, selector); }
            , py::arg("value"), py::arg("selector"))
        .def("set_style_translate_radial", [](LvObjWrapper &self, int value, int selector) { lv_obj_set_style_translate_radial(self.get(), value, selector); }
            , py::arg("value"), py::arg("selector"))
        .def("set_style_transform_scale_x", [](LvObjWrapper &self, int value, int selector) { lv_obj_set_style_transform_scale_x(self.get(), value, selector); }
            , py::arg("value"), py::arg("selector"))
        .def("set_style_transform_scale_y", [](LvObjWrapper &self, int value, int selector) { lv_obj_set_style_transform_scale_y(self.get(), value, selector); }
            , py::arg("value"), py::arg("selector"))
        .def("set_style_transform_rotation", [](LvObjWrapper &self, int value, int selector) { lv_obj_set_style_transform_rotation(self.get(), value, selector); }
            , py::arg("value"), py::arg("selector"))
        .def("set_style_transform_pivot_x", [](LvObjWrapper &self, int value, int selector) { lv_obj_set_style_transform_pivot_x(self.get(), value, selector); }
            , py::arg("value"), py::arg("selector"))
        .def("set_style_transform_pivot_y", [](LvObjWrapper &self, int value, int selector) { lv_obj_set_style_transform_pivot_y(self.get(), value, selector); }
            , py::arg("value"), py::arg("selector"))
        .def("set_style_transform_skew_x", [](LvObjWrapper &self, int value, int selector) { lv_obj_set_style_transform_skew_x(self.get(), value, selector); }
            , py::arg("value"), py::arg("selector"))
        .def("set_style_transform_skew_y", [](LvObjWrapper &self, int value, int selector) { lv_obj_set_style_transform_skew_y(self.get(), value, selector); }
            , py::arg("value"), py::arg("selector"))
        .def("set_style_pad_top", [](LvObjWrapper &self, int value, int selector) { lv_obj_set_style_pad_top(self.get(), value, selector); }
            , py::arg("value"), py::arg("selector"))
        .def("set_style_pad_bottom", [](LvObjWrapper &self, int value, int selector) { lv_obj_set_style_pad_bottom(self.get(), value, selector); }
            , py::arg("value"), py::arg("selector"))
        .def("set_style_pad_left", [](LvObjWrapper &self, int value, int selector) { lv_obj_set_style_pad_left(self.get(), value, selector); }
            , py::arg("value"), py::arg("selector"))
        .def("set_style_pad_right", [](LvObjWrapper &self, int value, int selector) { lv_obj_set_style_pad_right(self.get(), value, selector); }
            , py::arg("value"), py::arg("selector"))
        .def("set_style_pad_row", [](LvObjWrapper &self, int value, int selector) { lv_obj_set_style_pad_row(self.get(), value, selector); }
            , py::arg("value"), py::arg("selector"))
        .def("set_style_pad_column", [](LvObjWrapper &self, int value, int selector) { lv_obj_set_style_pad_column(self.get(), value, selector); }
            , py::arg("value"), py::arg("selector"))
        .def("set_style_pad_radial", [](LvObjWrapper &self, int value, int selector) { lv_obj_set_style_pad_radial(self.get(), value, selector); }
            , py::arg("value"), py::arg("selector"))
        .def("set_style_margin_top", [](LvObjWrapper &self, int value, int selector) { lv_obj_set_style_margin_top(self.get(), value, selector); }
            , py::arg("value"), py::arg("selector"))
        .def("set_style_margin_bottom", [](LvObjWrapper &self, int value, int selector) { lv_obj_set_style_margin_bottom(self.get(), value, selector); }
            , py::arg("value"), py::arg("selector"))
        .def("set_style_margin_left", [](LvObjWrapper &self, int value, int selector) { lv_obj_set_style_margin_left(self.get(), value, selector); }
            , py::arg("value"), py::arg("selector"))
        .def("set_style_margin_right", [](LvObjWrapper &self, int value, int selector) { lv_obj_set_style_margin_right(self.get(), value, selector); }
            , py::arg("value"), py::arg("selector"))
        .def("set_style_bg_opa", [](LvObjWrapper &self, int value, int selector) { lv_obj_set_style_bg_opa(self.get(), value, selector); }
            , py::arg("value"), py::arg("selector"))
        .def("set_style_bg_grad_dir", [](LvObjWrapper &self, lv_grad_dir_t value, int selector) { lv_obj_set_style_bg_grad_dir(self.get(), value, selector); }
            , py::arg("value"), py::arg("selector"))
        .def("set_style_bg_main_stop", [](LvObjWrapper &self, int value, int selector) { lv_obj_set_style_bg_main_stop(self.get(), value, selector); }
            , py::arg("value"), py::arg("selector"))
        .def("set_style_bg_grad_stop", [](LvObjWrapper &self, int value, int selector) { lv_obj_set_style_bg_grad_stop(self.get(), value, selector); }
            , py::arg("value"), py::arg("selector"))
        .def("set_style_bg_main_opa", [](LvObjWrapper &self, int value, int selector) { lv_obj_set_style_bg_main_opa(self.get(), value, selector); }
            , py::arg("value"), py::arg("selector"))
        .def("set_style_bg_grad_opa", [](LvObjWrapper &self, int value, int selector) { lv_obj_set_style_bg_grad_opa(self.get(), value, selector); }
            , py::arg("value"), py::arg("selector"))
        .def("set_style_bg_image_src", [](LvObjWrapper &self, void * value, int selector) { lv_obj_set_style_bg_image_src(self.get(), value, selector); }
            , py::arg("value"), py::arg("selector"))
        .def("set_style_bg_image_opa", [](LvObjWrapper &self, int value, int selector) { lv_obj_set_style_bg_image_opa(self.get(), value, selector); }
            , py::arg("value"), py::arg("selector"))
        .def("set_style_bg_image_recolor_opa", [](LvObjWrapper &self, int value, int selector) { lv_obj_set_style_bg_image_recolor_opa(self.get(), value, selector); }
            , py::arg("value"), py::arg("selector"))
        .def("set_style_bg_image_tiled", [](LvObjWrapper &self, bool value, int selector) { lv_obj_set_style_bg_image_tiled(self.get(), value, selector); }
            , py::arg("value"), py::arg("selector"))
        .def("set_style_border_opa", [](LvObjWrapper &self, int value, int selector) { lv_obj_set_style_border_opa(self.get(), value, selector); }
            , py::arg("value"), py::arg("selector"))
        .def("set_style_border_width", [](LvObjWrapper &self, int value, int selector) { lv_obj_set_style_border_width(self.get(), value, selector); }
            , py::arg("value"), py::arg("selector"))
        .def("set_style_border_side", [](LvObjWrapper &self, lv_border_side_t value, int selector) { lv_obj_set_style_border_side(self.get(), value, selector); }
            , py::arg("value"), py::arg("selector"))
        .def("set_style_border_post", [](LvObjWrapper &self, bool value, int selector) { lv_obj_set_style_border_post(self.get(), value, selector); }
            , py::arg("value"), py::arg("selector"))
        .def("set_style_outline_width", [](LvObjWrapper &self, int value, int selector) { lv_obj_set_style_outline_width(self.get(), value, selector); }
            , py::arg("value"), py::arg("selector"))
        .def("set_style_outline_opa", [](LvObjWrapper &self, int value, int selector) { lv_obj_set_style_outline_opa(self.get(), value, selector); }
            , py::arg("value"), py::arg("selector"))
        .def("set_style_outline_pad", [](LvObjWrapper &self, int value, int selector) { lv_obj_set_style_outline_pad(self.get(), value, selector); }
            , py::arg("value"), py::arg("selector"))
        .def("set_style_shadow_width", [](LvObjWrapper &self, int value, int selector) { lv_obj_set_style_shadow_width(self.get(), value, selector); }
            , py::arg("value"), py::arg("selector"))
        .def("set_style_shadow_offset_x", [](LvObjWrapper &self, int value, int selector) { lv_obj_set_style_shadow_offset_x(self.get(), value, selector); }
            , py::arg("value"), py::arg("selector"))
        .def("set_style_shadow_offset_y", [](LvObjWrapper &self, int value, int selector) { lv_obj_set_style_shadow_offset_y(self.get(), value, selector); }
            , py::arg("value"), py::arg("selector"))
        .def("set_style_shadow_spread", [](LvObjWrapper &self, int value, int selector) { lv_obj_set_style_shadow_spread(self.get(), value, selector); }
            , py::arg("value"), py::arg("selector"))
        .def("set_style_shadow_opa", [](LvObjWrapper &self, int value, int selector) { lv_obj_set_style_shadow_opa(self.get(), value, selector); }
            , py::arg("value"), py::arg("selector"))
        .def("set_style_image_opa", [](LvObjWrapper &self, int value, int selector) { lv_obj_set_style_image_opa(self.get(), value, selector); }
            , py::arg("value"), py::arg("selector"))
        .def("set_style_image_recolor_opa", [](LvObjWrapper &self, int value, int selector) { lv_obj_set_style_image_recolor_opa(self.get(), value, selector); }
            , py::arg("value"), py::arg("selector"))
        .def("set_style_line_width", [](LvObjWrapper &self, int value, int selector) { lv_obj_set_style_line_width(self.get(), value, selector); }
            , py::arg("value"), py::arg("selector"))
        .def("set_style_line_dash_width", [](LvObjWrapper &self, int value, int selector) { lv_obj_set_style_line_dash_width(self.get(), value, selector); }
            , py::arg("value"), py::arg("selector"))
        .def("set_style_line_dash_gap", [](LvObjWrapper &self, int value, int selector) { lv_obj_set_style_line_dash_gap(self.get(), value, selector); }
            , py::arg("value"), py::arg("selector"))
        .def("set_style_line_rounded", [](LvObjWrapper &self, bool value, int selector) { lv_obj_set_style_line_rounded(self.get(), value, selector); }
            , py::arg("value"), py::arg("selector"))
        .def("set_style_line_opa", [](LvObjWrapper &self, int value, int selector) { lv_obj_set_style_line_opa(self.get(), value, selector); }
            , py::arg("value"), py::arg("selector"))
        .def("set_style_arc_width", [](LvObjWrapper &self, int value, int selector) { lv_obj_set_style_arc_width(self.get(), value, selector); }
            , py::arg("value"), py::arg("selector"))
        .def("set_style_arc_rounded", [](LvObjWrapper &self, bool value, int selector) { lv_obj_set_style_arc_rounded(self.get(), value, selector); }
            , py::arg("value"), py::arg("selector"))
        .def("set_style_arc_opa", [](LvObjWrapper &self, int value, int selector) { lv_obj_set_style_arc_opa(self.get(), value, selector); }
            , py::arg("value"), py::arg("selector"))
        .def("set_style_arc_image_src", [](LvObjWrapper &self, void * value, int selector) { lv_obj_set_style_arc_image_src(self.get(), value, selector); }
            , py::arg("value"), py::arg("selector"))
        .def("set_style_text_opa", [](LvObjWrapper &self, int value, int selector) { lv_obj_set_style_text_opa(self.get(), value, selector); }
            , py::arg("value"), py::arg("selector"))
        .def("set_style_text_letter_space", [](LvObjWrapper &self, int value, int selector) { lv_obj_set_style_text_letter_space(self.get(), value, selector); }
            , py::arg("value"), py::arg("selector"))
        .def("set_style_text_line_space", [](LvObjWrapper &self, int value, int selector) { lv_obj_set_style_text_line_space(self.get(), value, selector); }
            , py::arg("value"), py::arg("selector"))
        .def("set_style_text_decor", [](LvObjWrapper &self, lv_text_decor_t value, int selector) { lv_obj_set_style_text_decor(self.get(), value, selector); }
            , py::arg("value"), py::arg("selector"))
        .def("set_style_text_align", [](LvObjWrapper &self, lv_text_align_t value, int selector) { lv_obj_set_style_text_align(self.get(), value, selector); }
            , py::arg("value"), py::arg("selector"))
        .def("set_style_text_outline_stroke_width", [](LvObjWrapper &self, int value, int selector) { lv_obj_set_style_text_outline_stroke_width(self.get(), value, selector); }
            , py::arg("value"), py::arg("selector"))
        .def("set_style_text_outline_stroke_opa", [](LvObjWrapper &self, int value, int selector) { lv_obj_set_style_text_outline_stroke_opa(self.get(), value, selector); }
            , py::arg("value"), py::arg("selector"))
        .def("set_style_text_leading_trim", [](LvObjWrapper &self, lv_text_leading_trim_t value, int selector) { lv_obj_set_style_text_leading_trim(self.get(), value, selector); }
            , py::arg("value"), py::arg("selector"))
        .def("set_style_blur_radius", [](LvObjWrapper &self, int value, int selector) { lv_obj_set_style_blur_radius(self.get(), value, selector); }
            , py::arg("value"), py::arg("selector"))
        .def("set_style_blur_backdrop", [](LvObjWrapper &self, bool value, int selector) { lv_obj_set_style_blur_backdrop(self.get(), value, selector); }
            , py::arg("value"), py::arg("selector"))
        .def("set_style_blur_quality", [](LvObjWrapper &self, lv_blur_quality_t value, int selector) { lv_obj_set_style_blur_quality(self.get(), value, selector); }
            , py::arg("value"), py::arg("selector"))
        .def("set_style_drop_shadow_radius", [](LvObjWrapper &self, int value, int selector) { lv_obj_set_style_drop_shadow_radius(self.get(), value, selector); }
            , py::arg("value"), py::arg("selector"))
        .def("set_style_drop_shadow_offset_x", [](LvObjWrapper &self, int value, int selector) { lv_obj_set_style_drop_shadow_offset_x(self.get(), value, selector); }
            , py::arg("value"), py::arg("selector"))
        .def("set_style_drop_shadow_offset_y", [](LvObjWrapper &self, int value, int selector) { lv_obj_set_style_drop_shadow_offset_y(self.get(), value, selector); }
            , py::arg("value"), py::arg("selector"))
        .def("set_style_drop_shadow_opa", [](LvObjWrapper &self, int value, int selector) { lv_obj_set_style_drop_shadow_opa(self.get(), value, selector); }
            , py::arg("value"), py::arg("selector"))
        .def("set_style_drop_shadow_quality", [](LvObjWrapper &self, lv_blur_quality_t value, int selector) { lv_obj_set_style_drop_shadow_quality(self.get(), value, selector); }
            , py::arg("value"), py::arg("selector"))
        .def("set_style_radius", [](LvObjWrapper &self, int value, int selector) { lv_obj_set_style_radius(self.get(), value, selector); }
            , py::arg("value"), py::arg("selector"))
        .def("set_style_radial_offset", [](LvObjWrapper &self, int value, int selector) { lv_obj_set_style_radial_offset(self.get(), value, selector); }
            , py::arg("value"), py::arg("selector"))
        .def("set_style_clip_corner", [](LvObjWrapper &self, bool value, int selector) { lv_obj_set_style_clip_corner(self.get(), value, selector); }
            , py::arg("value"), py::arg("selector"))
        .def("set_style_opa", [](LvObjWrapper &self, int value, int selector) { lv_obj_set_style_opa(self.get(), value, selector); }
            , py::arg("value"), py::arg("selector"))
        .def("set_style_opa_layered", [](LvObjWrapper &self, int value, int selector) { lv_obj_set_style_opa_layered(self.get(), value, selector); }
            , py::arg("value"), py::arg("selector"))
        .def("set_style_color_filter_opa", [](LvObjWrapper &self, int value, int selector) { lv_obj_set_style_color_filter_opa(self.get(), value, selector); }
            , py::arg("value"), py::arg("selector"))
        .def("set_style_recolor_opa", [](LvObjWrapper &self, int value, int selector) { lv_obj_set_style_recolor_opa(self.get(), value, selector); }
            , py::arg("value"), py::arg("selector"))
        .def("set_style_anim_duration", [](LvObjWrapper &self, int value, int selector) { lv_obj_set_style_anim_duration(self.get(), value, selector); }
            , py::arg("value"), py::arg("selector"))
        .def("set_style_blend_mode", [](LvObjWrapper &self, lv_blend_mode_t value, int selector) { lv_obj_set_style_blend_mode(self.get(), value, selector); }
            , py::arg("value"), py::arg("selector"))
        .def("set_style_layout", [](LvObjWrapper &self, int value, int selector) { lv_obj_set_style_layout(self.get(), value, selector); }
            , py::arg("value"), py::arg("selector"))
        .def("set_style_base_dir", [](LvObjWrapper &self, lv_base_dir_t value, int selector) { lv_obj_set_style_base_dir(self.get(), value, selector); }
            , py::arg("value"), py::arg("selector"))
        .def("set_style_bitmap_mask_src", [](LvObjWrapper &self, void * value, int selector) { lv_obj_set_style_bitmap_mask_src(self.get(), value, selector); }
            , py::arg("value"), py::arg("selector"))
        .def("set_style_rotary_sensitivity", [](LvObjWrapper &self, int value, int selector) { lv_obj_set_style_rotary_sensitivity(self.get(), value, selector); }
            , py::arg("value"), py::arg("selector"))
        .def("set_style_flex_flow", [](LvObjWrapper &self, lv_flex_flow_t value, int selector) { lv_obj_set_style_flex_flow(self.get(), value, selector); }
            , py::arg("value"), py::arg("selector"))
        .def("set_style_flex_main_place", [](LvObjWrapper &self, lv_flex_align_t value, int selector) { lv_obj_set_style_flex_main_place(self.get(), value, selector); }
            , py::arg("value"), py::arg("selector"))
        .def("set_style_flex_cross_place", [](LvObjWrapper &self, lv_flex_align_t value, int selector) { lv_obj_set_style_flex_cross_place(self.get(), value, selector); }
            , py::arg("value"), py::arg("selector"))
        .def("set_style_flex_track_place", [](LvObjWrapper &self, lv_flex_align_t value, int selector) { lv_obj_set_style_flex_track_place(self.get(), value, selector); }
            , py::arg("value"), py::arg("selector"))
        .def("set_style_flex_grow", [](LvObjWrapper &self, int value, int selector) { lv_obj_set_style_flex_grow(self.get(), value, selector); }
            , py::arg("value"), py::arg("selector"))
        .def("set_style_grid_column_dsc_array", [](LvObjWrapper &self, int32_t * value, int selector) { lv_obj_set_style_grid_column_dsc_array(self.get(), value, selector); }
            , py::arg("value"), py::arg("selector"))
        .def("set_style_grid_column_align", [](LvObjWrapper &self, lv_grid_align_t value, int selector) { lv_obj_set_style_grid_column_align(self.get(), value, selector); }
            , py::arg("value"), py::arg("selector"))
        .def("set_style_grid_row_dsc_array", [](LvObjWrapper &self, int32_t * value, int selector) { lv_obj_set_style_grid_row_dsc_array(self.get(), value, selector); }
            , py::arg("value"), py::arg("selector"))
        .def("set_style_grid_row_align", [](LvObjWrapper &self, lv_grid_align_t value, int selector) { lv_obj_set_style_grid_row_align(self.get(), value, selector); }
            , py::arg("value"), py::arg("selector"))
        .def("set_style_grid_cell_column_pos", [](LvObjWrapper &self, int value, int selector) { lv_obj_set_style_grid_cell_column_pos(self.get(), value, selector); }
            , py::arg("value"), py::arg("selector"))
        .def("set_style_grid_cell_x_align", [](LvObjWrapper &self, lv_grid_align_t value, int selector) { lv_obj_set_style_grid_cell_x_align(self.get(), value, selector); }
            , py::arg("value"), py::arg("selector"))
        .def("set_style_grid_cell_column_span", [](LvObjWrapper &self, int value, int selector) { lv_obj_set_style_grid_cell_column_span(self.get(), value, selector); }
            , py::arg("value"), py::arg("selector"))
        .def("set_style_grid_cell_row_pos", [](LvObjWrapper &self, int value, int selector) { lv_obj_set_style_grid_cell_row_pos(self.get(), value, selector); }
            , py::arg("value"), py::arg("selector"))
        .def("set_style_grid_cell_y_align", [](LvObjWrapper &self, lv_grid_align_t value, int selector) { lv_obj_set_style_grid_cell_y_align(self.get(), value, selector); }
            , py::arg("value"), py::arg("selector"))
        .def("set_style_grid_cell_row_span", [](LvObjWrapper &self, int value, int selector) { lv_obj_set_style_grid_cell_row_span(self.get(), value, selector); }
            , py::arg("value"), py::arg("selector"))
        .def("set_ext_draw_size", [](LvObjWrapper &self, int size) { lv_obj_set_ext_draw_size(self.get(), size); }
            , py::arg("size"))
        .def("refresh_ext_draw_size", [](LvObjWrapper &self) { lv_obj_refresh_ext_draw_size(self.get()); }
            )
        .def("class_init_obj", [](LvObjWrapper &self) { lv_obj_class_init_obj(self.get()); }
            )
        .def("add_flag", [](LvObjWrapper &self, lv_obj_flag_t f) { lv_obj_add_flag(self.get(), f); }
            , py::arg("f"))
        .def("remove_flag", [](LvObjWrapper &self, lv_obj_flag_t f) { lv_obj_remove_flag(self.get(), f); }
            , py::arg("f"))
        .def("set_flag", [](LvObjWrapper &self, lv_obj_flag_t f, bool v) { lv_obj_set_flag(self.get(), f, v); }
            , py::arg("f"), py::arg("v"))
        .def("add_state", [](LvObjWrapper &self, lv_state_t state) { lv_obj_add_state(self.get(), state); }
            , py::arg("state"))
        .def("remove_state", [](LvObjWrapper &self, lv_state_t state) { lv_obj_remove_state(self.get(), state); }
            , py::arg("state"))
        .def("set_state", [](LvObjWrapper &self, lv_state_t state, bool v) { lv_obj_set_state(self.get(), state, v); }
            , py::arg("state"), py::arg("v"))
        .def("set_user_data", [](LvObjWrapper &self, void * user_data) { lv_obj_set_user_data(self.get(), user_data); }
            , py::arg("user_data"))
        .def("set_radio_button", [](LvObjWrapper &self, bool en) { lv_obj_set_radio_button(self.get(), en); }
            , py::arg("en"))
        .def("add_screen_load_event", [](LvObjWrapper &self, lv_event_code_t trigger, LvObjWrapper & screen, lv_screen_load_anim_t anim_type, int duration, int delay) { lv_obj_add_screen_load_event(self.get(), trigger, screen.get(), anim_type, duration, delay); }
            , py::arg("trigger"), py::arg("screen"), py::arg("anim_type"), py::arg("duration"), py::arg("delay"))
        ;

    /* Screen management */
    m.def("screen_active", []() -> LvObjWrapper* {
        return new LvObjWrapper(lv_screen_active(), false);
    });
    m.def("screen_load", [](LvObjWrapper &screen) {
        lv_screen_load(screen.get());
    }, py::arg("screen"));

    /* Widget factory functions */
    m.def("Image", [](LvObjWrapper *parent) -> LvObjWrapper* {
        return new LvObjWrapper(lv_image_create(parent ? parent->get() : lv_screen_active()));
    }, py::arg("parent") = py::none());
    m.def("Animimg", [](LvObjWrapper *parent) -> LvObjWrapper* {
        return new LvObjWrapper(lv_animimg_create(parent ? parent->get() : lv_screen_active()));
    }, py::arg("parent") = py::none());
    m.def("Arc", [](LvObjWrapper *parent) -> LvObjWrapper* {
        return new LvObjWrapper(lv_arc_create(parent ? parent->get() : lv_screen_active()));
    }, py::arg("parent") = py::none());
    m.def("Arclabel", [](LvObjWrapper *parent) -> LvObjWrapper* {
        return new LvObjWrapper(lv_arclabel_create(parent ? parent->get() : lv_screen_active()));
    }, py::arg("parent") = py::none());
    m.def("Label", [](LvObjWrapper *parent) -> LvObjWrapper* {
        return new LvObjWrapper(lv_label_create(parent ? parent->get() : lv_screen_active()));
    }, py::arg("parent") = py::none());
    m.def("Bar", [](LvObjWrapper *parent) -> LvObjWrapper* {
        return new LvObjWrapper(lv_bar_create(parent ? parent->get() : lv_screen_active()));
    }, py::arg("parent") = py::none());
    m.def("Canvas", [](LvObjWrapper *parent) -> LvObjWrapper* {
        return new LvObjWrapper(lv_canvas_create(parent ? parent->get() : lv_screen_active()));
    }, py::arg("parent") = py::none());
    m.def("Barcode", [](LvObjWrapper *parent) -> LvObjWrapper* {
        return new LvObjWrapper(lv_barcode_create(parent ? parent->get() : lv_screen_active()));
    }, py::arg("parent") = py::none());
    m.def("Button", [](LvObjWrapper *parent) -> LvObjWrapper* {
        return new LvObjWrapper(lv_button_create(parent ? parent->get() : lv_screen_active()));
    }, py::arg("parent") = py::none());
    m.def("Buttonmatrix", [](LvObjWrapper *parent) -> LvObjWrapper* {
        return new LvObjWrapper(lv_buttonmatrix_create(parent ? parent->get() : lv_screen_active()));
    }, py::arg("parent") = py::none());
    m.def("Calendar", [](LvObjWrapper *parent) -> LvObjWrapper* {
        return new LvObjWrapper(lv_calendar_create(parent ? parent->get() : lv_screen_active()));
    }, py::arg("parent") = py::none());
    m.def("Chart", [](LvObjWrapper *parent) -> LvObjWrapper* {
        return new LvObjWrapper(lv_chart_create(parent ? parent->get() : lv_screen_active()));
    }, py::arg("parent") = py::none());
    m.def("Checkbox", [](LvObjWrapper *parent) -> LvObjWrapper* {
        return new LvObjWrapper(lv_checkbox_create(parent ? parent->get() : lv_screen_active()));
    }, py::arg("parent") = py::none());
    m.def("Dropdown", [](LvObjWrapper *parent) -> LvObjWrapper* {
        return new LvObjWrapper(lv_dropdown_create(parent ? parent->get() : lv_screen_active()));
    }, py::arg("parent") = py::none());
    m.def("Imagebutton", [](LvObjWrapper *parent) -> LvObjWrapper* {
        return new LvObjWrapper(lv_imagebutton_create(parent ? parent->get() : lv_screen_active()));
    }, py::arg("parent") = py::none());
    m.def("Keyboard", [](LvObjWrapper *parent) -> LvObjWrapper* {
        return new LvObjWrapper(lv_keyboard_create(parent ? parent->get() : lv_screen_active()));
    }, py::arg("parent") = py::none());
    m.def("Led", [](LvObjWrapper *parent) -> LvObjWrapper* {
        return new LvObjWrapper(lv_led_create(parent ? parent->get() : lv_screen_active()));
    }, py::arg("parent") = py::none());
    m.def("Line", [](LvObjWrapper *parent) -> LvObjWrapper* {
        return new LvObjWrapper(lv_line_create(parent ? parent->get() : lv_screen_active()));
    }, py::arg("parent") = py::none());
    m.def("List", [](LvObjWrapper *parent) -> LvObjWrapper* {
        return new LvObjWrapper(lv_list_create(parent ? parent->get() : lv_screen_active()));
    }, py::arg("parent") = py::none());
    m.def("Lottie", [](LvObjWrapper *parent) -> LvObjWrapper* {
        return new LvObjWrapper(lv_lottie_create(parent ? parent->get() : lv_screen_active()));
    }, py::arg("parent") = py::none());
    m.def("Menu", [](LvObjWrapper *parent) -> LvObjWrapper* {
        return new LvObjWrapper(lv_menu_create(parent ? parent->get() : lv_screen_active()));
    }, py::arg("parent") = py::none());
    m.def("Menu_page", [](LvObjWrapper *parent, char * title) -> LvObjWrapper* {
        return new LvObjWrapper(lv_menu_page_create(parent ? parent->get() : lv_screen_active(), title));
    }, py::arg("parent") = py::none(), py::arg("title"));
    m.def("Menu_cont", [](LvObjWrapper *parent) -> LvObjWrapper* {
        return new LvObjWrapper(lv_menu_cont_create(parent ? parent->get() : lv_screen_active()));
    }, py::arg("parent") = py::none());
    m.def("Menu_section", [](LvObjWrapper *parent) -> LvObjWrapper* {
        return new LvObjWrapper(lv_menu_section_create(parent ? parent->get() : lv_screen_active()));
    }, py::arg("parent") = py::none());
    m.def("Menu_separator", [](LvObjWrapper *parent) -> LvObjWrapper* {
        return new LvObjWrapper(lv_menu_separator_create(parent ? parent->get() : lv_screen_active()));
    }, py::arg("parent") = py::none());
    m.def("Msgbox", [](LvObjWrapper *parent) -> LvObjWrapper* {
        return new LvObjWrapper(lv_msgbox_create(parent ? parent->get() : lv_screen_active()));
    }, py::arg("parent") = py::none());
    m.def("Qrcode", [](LvObjWrapper *parent) -> LvObjWrapper* {
        return new LvObjWrapper(lv_qrcode_create(parent ? parent->get() : lv_screen_active()));
    }, py::arg("parent") = py::none());
    m.def("Roller", [](LvObjWrapper *parent) -> LvObjWrapper* {
        return new LvObjWrapper(lv_roller_create(parent ? parent->get() : lv_screen_active()));
    }, py::arg("parent") = py::none());
    m.def("Scale", [](LvObjWrapper *parent) -> LvObjWrapper* {
        return new LvObjWrapper(lv_scale_create(parent ? parent->get() : lv_screen_active()));
    }, py::arg("parent") = py::none());
    m.def("Slider", [](LvObjWrapper *parent) -> LvObjWrapper* {
        return new LvObjWrapper(lv_slider_create(parent ? parent->get() : lv_screen_active()));
    }, py::arg("parent") = py::none());
    m.def("Spangroup", [](LvObjWrapper *parent) -> LvObjWrapper* {
        return new LvObjWrapper(lv_spangroup_create(parent ? parent->get() : lv_screen_active()));
    }, py::arg("parent") = py::none());
    m.def("Textarea", [](LvObjWrapper *parent) -> LvObjWrapper* {
        return new LvObjWrapper(lv_textarea_create(parent ? parent->get() : lv_screen_active()));
    }, py::arg("parent") = py::none());
    m.def("Spinbox", [](LvObjWrapper *parent) -> LvObjWrapper* {
        return new LvObjWrapper(lv_spinbox_create(parent ? parent->get() : lv_screen_active()));
    }, py::arg("parent") = py::none());
    m.def("Spinner", [](LvObjWrapper *parent) -> LvObjWrapper* {
        return new LvObjWrapper(lv_spinner_create(parent ? parent->get() : lv_screen_active()));
    }, py::arg("parent") = py::none());
    m.def("Switch", [](LvObjWrapper *parent) -> LvObjWrapper* {
        return new LvObjWrapper(lv_switch_create(parent ? parent->get() : lv_screen_active()));
    }, py::arg("parent") = py::none());
    m.def("Table", [](LvObjWrapper *parent) -> LvObjWrapper* {
        return new LvObjWrapper(lv_table_create(parent ? parent->get() : lv_screen_active()));
    }, py::arg("parent") = py::none());
    m.def("Tabview", [](LvObjWrapper *parent) -> LvObjWrapper* {
        return new LvObjWrapper(lv_tabview_create(parent ? parent->get() : lv_screen_active()));
    }, py::arg("parent") = py::none());
    m.def("Tileview", [](LvObjWrapper *parent) -> LvObjWrapper* {
        return new LvObjWrapper(lv_tileview_create(parent ? parent->get() : lv_screen_active()));
    }, py::arg("parent") = py::none());
    m.def("Win", [](LvObjWrapper *parent) -> LvObjWrapper* {
        return new LvObjWrapper(lv_win_create(parent ? parent->get() : lv_screen_active()));
    }, py::arg("parent") = py::none());

    /* Widget-specific methods on Obj */
    obj_cls.def("set_src", [](LvObjWrapper &self, void * src) { lv_image_set_src(self.get(), src); }
        , py::arg("src"));
    obj_cls.def("set_offset_x", [](LvObjWrapper &self, int x) { lv_image_set_offset_x(self.get(), x); }
        , py::arg("x"));
    obj_cls.def("set_offset_y", [](LvObjWrapper &self, int y) { lv_image_set_offset_y(self.get(), y); }
        , py::arg("y"));
    obj_cls.def("set_rotation", [](LvObjWrapper &self, int angle) { lv_image_set_rotation(self.get(), angle); }
        , py::arg("angle"));
    obj_cls.def("set_pivot", [](LvObjWrapper &self, int x, int y) { lv_image_set_pivot(self.get(), x, y); }
        , py::arg("x"), py::arg("y"));
    obj_cls.def("set_pivot_x", [](LvObjWrapper &self, int x) { lv_image_set_pivot_x(self.get(), x); }
        , py::arg("x"));
    obj_cls.def("set_pivot_y", [](LvObjWrapper &self, int y) { lv_image_set_pivot_y(self.get(), y); }
        , py::arg("y"));
    obj_cls.def("set_scale", [](LvObjWrapper &self, int zoom) { lv_image_set_scale(self.get(), zoom); }
        , py::arg("zoom"));
    obj_cls.def("set_scale_x", [](LvObjWrapper &self, int zoom) { lv_image_set_scale_x(self.get(), zoom); }
        , py::arg("zoom"));
    obj_cls.def("set_scale_y", [](LvObjWrapper &self, int zoom) { lv_image_set_scale_y(self.get(), zoom); }
        , py::arg("zoom"));
    obj_cls.def("set_blend_mode", [](LvObjWrapper &self, lv_blend_mode_t blend_mode) { lv_image_set_blend_mode(self.get(), blend_mode); }
        , py::arg("blend_mode"));
    obj_cls.def("set_antialias", [](LvObjWrapper &self, bool antialias) { lv_image_set_antialias(self.get(), antialias); }
        , py::arg("antialias"));
    obj_cls.def("set_inner_align", [](LvObjWrapper &self, lv_image_align_t align) { lv_image_set_inner_align(self.get(), align); }
        , py::arg("align"));
    obj_cls.def("start", [](LvObjWrapper &self) { lv_animimg_start(self.get()); }
        );
    obj_cls.def("set_duration", [](LvObjWrapper &self, int duration) { lv_animimg_set_duration(self.get(), duration); }
        , py::arg("duration"));
    obj_cls.def("set_repeat_count", [](LvObjWrapper &self, int count) { lv_animimg_set_repeat_count(self.get(), count); }
        , py::arg("count"));
    obj_cls.def("set_reverse_duration", [](LvObjWrapper &self, int duration) { lv_animimg_set_reverse_duration(self.get(), duration); }
        , py::arg("duration"));
    obj_cls.def("set_reverse_delay", [](LvObjWrapper &self, int duration) { lv_animimg_set_reverse_delay(self.get(), duration); }
        , py::arg("duration"));
    obj_cls.def("set_start_angle", [](LvObjWrapper &self, float start) { lv_arc_set_start_angle(self.get(), start); }
        , py::arg("start"));
    obj_cls.def("set_end_angle", [](LvObjWrapper &self, float end) { lv_arc_set_end_angle(self.get(), end); }
        , py::arg("end"));
    obj_cls.def("set_angles", [](LvObjWrapper &self, float start, float end) { lv_arc_set_angles(self.get(), start, end); }
        , py::arg("start"), py::arg("end"));
    obj_cls.def("set_bg_start_angle", [](LvObjWrapper &self, float start) { lv_arc_set_bg_start_angle(self.get(), start); }
        , py::arg("start"));
    obj_cls.def("set_bg_end_angle", [](LvObjWrapper &self, float end) { lv_arc_set_bg_end_angle(self.get(), end); }
        , py::arg("end"));
    obj_cls.def("set_bg_angles", [](LvObjWrapper &self, float start, float end) { lv_arc_set_bg_angles(self.get(), start, end); }
        , py::arg("start"), py::arg("end"));
    obj_cls.def("arc_set_rotation", [](LvObjWrapper &self, int rotation) { lv_arc_set_rotation(self.get(), rotation); }
        , py::arg("rotation"));
    obj_cls.def("set_mode", [](LvObjWrapper &self, lv_arc_mode_t type) { lv_arc_set_mode(self.get(), type); }
        , py::arg("type"));
    obj_cls.def("set_value", [](LvObjWrapper &self, int value) { lv_arc_set_value(self.get(), value); }
        , py::arg("value"));
    obj_cls.def("set_range", [](LvObjWrapper &self, int min, int max) { lv_arc_set_range(self.get(), min, max); }
        , py::arg("min"), py::arg("max"));
    obj_cls.def("set_min_value", [](LvObjWrapper &self, int min) { lv_arc_set_min_value(self.get(), min); }
        , py::arg("min"));
    obj_cls.def("set_max_value", [](LvObjWrapper &self, int max) { lv_arc_set_max_value(self.get(), max); }
        , py::arg("max"));
    obj_cls.def("set_change_rate", [](LvObjWrapper &self, int rate) { lv_arc_set_change_rate(self.get(), rate); }
        , py::arg("rate"));
    obj_cls.def("set_knob_offset", [](LvObjWrapper &self, int offset) { lv_arc_set_knob_offset(self.get(), offset); }
        , py::arg("offset"));
    obj_cls.def("align_obj_to_angle", [](LvObjWrapper &self, LvObjWrapper & obj_to_align, int r_offset) { lv_arc_align_obj_to_angle(self.get(), obj_to_align.get(), r_offset); }
        , py::arg("obj_to_align"), py::arg("r_offset"));
    obj_cls.def("rotate_obj_to_angle", [](LvObjWrapper &self, LvObjWrapper & obj_to_rotate, int r_offset) { lv_arc_rotate_obj_to_angle(self.get(), obj_to_rotate.get(), r_offset); }
        , py::arg("obj_to_rotate"), py::arg("r_offset"));
    obj_cls.def("set_text", [](LvObjWrapper &self, char * text) { lv_arclabel_set_text(self.get(), text); }
        , py::arg("text"));
    obj_cls.def("set_text_static", [](LvObjWrapper &self, char * text) { lv_arclabel_set_text_static(self.get(), text); }
        , py::arg("text"));
    obj_cls.def("set_angle_start", [](LvObjWrapper &self, float start) { lv_arclabel_set_angle_start(self.get(), start); }
        , py::arg("start"));
    obj_cls.def("set_angle_size", [](LvObjWrapper &self, float size) { lv_arclabel_set_angle_size(self.get(), size); }
        , py::arg("size"));
    obj_cls.def("set_offset", [](LvObjWrapper &self, int offset) { lv_arclabel_set_offset(self.get(), offset); }
        , py::arg("offset"));
    obj_cls.def("set_dir", [](LvObjWrapper &self, lv_arclabel_dir_t dir) { lv_arclabel_set_dir(self.get(), dir); }
        , py::arg("dir"));
    obj_cls.def("set_recolor", [](LvObjWrapper &self, bool en) { lv_arclabel_set_recolor(self.get(), en); }
        , py::arg("en"));
    obj_cls.def("set_radius", [](LvObjWrapper &self, int radius) { lv_arclabel_set_radius(self.get(), radius); }
        , py::arg("radius"));
    obj_cls.def("set_center_offset_x", [](LvObjWrapper &self, int x) { lv_arclabel_set_center_offset_x(self.get(), x); }
        , py::arg("x"));
    obj_cls.def("set_center_offset_y", [](LvObjWrapper &self, int y) { lv_arclabel_set_center_offset_y(self.get(), y); }
        , py::arg("y"));
    obj_cls.def("set_text_vertical_align", [](LvObjWrapper &self, lv_arclabel_text_align_t align) { lv_arclabel_set_text_vertical_align(self.get(), align); }
        , py::arg("align"));
    obj_cls.def("set_text_horizontal_align", [](LvObjWrapper &self, lv_arclabel_text_align_t align) { lv_arclabel_set_text_horizontal_align(self.get(), align); }
        , py::arg("align"));
    obj_cls.def("set_overflow", [](LvObjWrapper &self, lv_arclabel_overflow_t overflow) { lv_arclabel_set_overflow(self.get(), overflow); }
        , py::arg("overflow"));
    obj_cls.def("set_end_overlap", [](LvObjWrapper &self, bool overlap) { lv_arclabel_set_end_overlap(self.get(), overlap); }
        , py::arg("overlap"));
    obj_cls.def("label_set_text", [](LvObjWrapper &self, char * text) { lv_label_set_text(self.get(), text); }
        , py::arg("text"));
    obj_cls.def("label_set_text_static", [](LvObjWrapper &self, char * text) { lv_label_set_text_static(self.get(), text); }
        , py::arg("text"));
    obj_cls.def("set_long_mode", [](LvObjWrapper &self, lv_label_long_mode_t long_mode) { lv_label_set_long_mode(self.get(), long_mode); }
        , py::arg("long_mode"));
    obj_cls.def("set_max_lines", [](LvObjWrapper &self, int lines) { lv_label_set_max_lines(self.get(), lines); }
        , py::arg("lines"));
    obj_cls.def("set_text_selection_start", [](LvObjWrapper &self, int index) { lv_label_set_text_selection_start(self.get(), index); }
        , py::arg("index"));
    obj_cls.def("set_text_selection_end", [](LvObjWrapper &self, int index) { lv_label_set_text_selection_end(self.get(), index); }
        , py::arg("index"));
    obj_cls.def("label_set_recolor", [](LvObjWrapper &self, bool en) { lv_label_set_recolor(self.get(), en); }
        , py::arg("en"));
    obj_cls.def("ins_text", [](LvObjWrapper &self, int pos, char * txt) { lv_label_ins_text(self.get(), pos, txt); }
        , py::arg("pos"), py::arg("txt"));
    obj_cls.def("cut_text", [](LvObjWrapper &self, int pos, int cnt) { lv_label_cut_text(self.get(), pos, cnt); }
        , py::arg("pos"), py::arg("cnt"));
    obj_cls.def("bar_set_value", [](LvObjWrapper &self, int value, bool anim) { lv_bar_set_value(self.get(), value, anim); }
        , py::arg("value"), py::arg("anim"));
    obj_cls.def("set_start_value", [](LvObjWrapper &self, int start_value, bool anim) { lv_bar_set_start_value(self.get(), start_value, anim); }
        , py::arg("start_value"), py::arg("anim"));
    obj_cls.def("bar_set_range", [](LvObjWrapper &self, int min, int max) { lv_bar_set_range(self.get(), min, max); }
        , py::arg("min"), py::arg("max"));
    obj_cls.def("bar_set_min_value", [](LvObjWrapper &self, int min) { lv_bar_set_min_value(self.get(), min); }
        , py::arg("min"));
    obj_cls.def("bar_set_max_value", [](LvObjWrapper &self, int max) { lv_bar_set_max_value(self.get(), max); }
        , py::arg("max"));
    obj_cls.def("bar_set_mode", [](LvObjWrapper &self, lv_bar_mode_t mode) { lv_bar_set_mode(self.get(), mode); }
        , py::arg("mode"));
    obj_cls.def("set_orientation", [](LvObjWrapper &self, lv_bar_orientation_t orientation) { lv_bar_set_orientation(self.get(), orientation); }
        , py::arg("orientation"));
    obj_cls.def("set_buffer", [](LvObjWrapper &self, void * buf, int w, int h, lv_color_format_t cf) { lv_canvas_set_buffer(self.get(), buf, w, h, cf); }
        , py::arg("buf"), py::arg("w"), py::arg("h"), py::arg("cf"));
    obj_cls.def("barcode_set_scale", [](LvObjWrapper &self, int scale) { lv_barcode_set_scale(self.get(), scale); }
        , py::arg("scale"));
    obj_cls.def("set_direction", [](LvObjWrapper &self, lv_dir_t direction) { lv_barcode_set_direction(self.get(), direction); }
        , py::arg("direction"));
    obj_cls.def("set_tiled", [](LvObjWrapper &self, bool tiled) { lv_barcode_set_tiled(self.get(), tiled); }
        , py::arg("tiled"));
    obj_cls.def("set_encoding", [](LvObjWrapper &self, lv_barcode_encoding_t encoding) { lv_barcode_set_encoding(self.get(), encoding); }
        , py::arg("encoding"));
    obj_cls.def("set_selected_button", [](LvObjWrapper &self, int btn_id) { lv_buttonmatrix_set_selected_button(self.get(), btn_id); }
        , py::arg("btn_id"));
    obj_cls.def("set_button_ctrl", [](LvObjWrapper &self, int btn_id, lv_buttonmatrix_ctrl_t ctrl) { lv_buttonmatrix_set_button_ctrl(self.get(), btn_id, ctrl); }
        , py::arg("btn_id"), py::arg("ctrl"));
    obj_cls.def("clear_button_ctrl", [](LvObjWrapper &self, int btn_id, lv_buttonmatrix_ctrl_t ctrl) { lv_buttonmatrix_clear_button_ctrl(self.get(), btn_id, ctrl); }
        , py::arg("btn_id"), py::arg("ctrl"));
    obj_cls.def("set_button_ctrl_all", [](LvObjWrapper &self, lv_buttonmatrix_ctrl_t ctrl) { lv_buttonmatrix_set_button_ctrl_all(self.get(), ctrl); }
        , py::arg("ctrl"));
    obj_cls.def("clear_button_ctrl_all", [](LvObjWrapper &self, lv_buttonmatrix_ctrl_t ctrl) { lv_buttonmatrix_clear_button_ctrl_all(self.get(), ctrl); }
        , py::arg("ctrl"));
    obj_cls.def("set_button_width", [](LvObjWrapper &self, int btn_id, int width) { lv_buttonmatrix_set_button_width(self.get(), btn_id, width); }
        , py::arg("btn_id"), py::arg("width"));
    obj_cls.def("set_one_checked", [](LvObjWrapper &self, bool en) { lv_buttonmatrix_set_one_checked(self.get(), en); }
        , py::arg("en"));
    obj_cls.def("set_today_date", [](LvObjWrapper &self, int year, int month, int day) { lv_calendar_set_today_date(self.get(), year, month, day); }
        , py::arg("year"), py::arg("month"), py::arg("day"));
    obj_cls.def("set_today_year", [](LvObjWrapper &self, int year) { lv_calendar_set_today_year(self.get(), year); }
        , py::arg("year"));
    obj_cls.def("set_today_month", [](LvObjWrapper &self, int month) { lv_calendar_set_today_month(self.get(), month); }
        , py::arg("month"));
    obj_cls.def("set_today_day", [](LvObjWrapper &self, int day) { lv_calendar_set_today_day(self.get(), day); }
        , py::arg("day"));
    obj_cls.def("set_month_shown", [](LvObjWrapper &self, int year, int month) { lv_calendar_set_month_shown(self.get(), year, month); }
        , py::arg("year"), py::arg("month"));
    obj_cls.def("set_shown_year", [](LvObjWrapper &self, int year) { lv_calendar_set_shown_year(self.get(), year); }
        , py::arg("year"));
    obj_cls.def("set_shown_month", [](LvObjWrapper &self, int month) { lv_calendar_set_shown_month(self.get(), month); }
        , py::arg("month"));
    obj_cls.def("header_dropdown_set_year_list", [](LvObjWrapper &self, char * years_list) { lv_calendar_header_dropdown_set_year_list(self.get(), years_list); }
        , py::arg("years_list"));
    obj_cls.def("set_type", [](LvObjWrapper &self, lv_chart_type_t type) { lv_chart_set_type(self.get(), type); }
        , py::arg("type"));
    obj_cls.def("set_point_count", [](LvObjWrapper &self, int cnt) { lv_chart_set_point_count(self.get(), cnt); }
        , py::arg("cnt"));
    obj_cls.def("set_axis_range", [](LvObjWrapper &self, lv_chart_axis_t axis, int min, int max) { lv_chart_set_axis_range(self.get(), axis, min, max); }
        , py::arg("axis"), py::arg("min"), py::arg("max"));
    obj_cls.def("set_axis_min_value", [](LvObjWrapper &self, lv_chart_axis_t axis, int min) { lv_chart_set_axis_min_value(self.get(), axis, min); }
        , py::arg("axis"), py::arg("min"));
    obj_cls.def("set_axis_max_value", [](LvObjWrapper &self, lv_chart_axis_t axis, int max) { lv_chart_set_axis_max_value(self.get(), axis, max); }
        , py::arg("axis"), py::arg("max"));
    obj_cls.def("set_update_mode", [](LvObjWrapper &self, lv_chart_update_mode_t update_mode) { lv_chart_set_update_mode(self.get(), update_mode); }
        , py::arg("update_mode"));
    obj_cls.def("set_div_line_count", [](LvObjWrapper &self, int hdiv, int vdiv) { lv_chart_set_div_line_count(self.get(), hdiv, vdiv); }
        , py::arg("hdiv"), py::arg("vdiv"));
    obj_cls.def("set_hor_div_line_count", [](LvObjWrapper &self, int cnt) { lv_chart_set_hor_div_line_count(self.get(), cnt); }
        , py::arg("cnt"));
    obj_cls.def("set_ver_div_line_count", [](LvObjWrapper &self, int cnt) { lv_chart_set_ver_div_line_count(self.get(), cnt); }
        , py::arg("cnt"));
    obj_cls.def("refresh", [](LvObjWrapper &self) { lv_chart_refresh(self.get()); }
        );
    obj_cls.def("checkbox_set_text", [](LvObjWrapper &self, char * txt) { lv_checkbox_set_text(self.get(), txt); }
        , py::arg("txt"));
    obj_cls.def("checkbox_set_text_static", [](LvObjWrapper &self, char * txt) { lv_checkbox_set_text_static(self.get(), txt); }
        , py::arg("txt"));
    obj_cls.def("dropdown_set_text", [](LvObjWrapper &self, char * text) { lv_dropdown_set_text(self.get(), text); }
        , py::arg("text"));
    obj_cls.def("dropdown_set_text_static", [](LvObjWrapper &self, char * text) { lv_dropdown_set_text_static(self.get(), text); }
        , py::arg("text"));
    obj_cls.def("set_options", [](LvObjWrapper &self, char * options) { lv_dropdown_set_options(self.get(), options); }
        , py::arg("options"));
    obj_cls.def("set_options_static", [](LvObjWrapper &self, char * options) { lv_dropdown_set_options_static(self.get(), options); }
        , py::arg("options"));
    obj_cls.def("add_option", [](LvObjWrapper &self, char * option, int pos) { lv_dropdown_add_option(self.get(), option, pos); }
        , py::arg("option"), py::arg("pos"));
    obj_cls.def("clear_options", [](LvObjWrapper &self) { lv_dropdown_clear_options(self.get()); }
        );
    obj_cls.def("set_selected", [](LvObjWrapper &self, int sel_opt) { lv_dropdown_set_selected(self.get(), sel_opt); }
        , py::arg("sel_opt"));
    obj_cls.def("dropdown_set_dir", [](LvObjWrapper &self, lv_dir_t dir) { lv_dropdown_set_dir(self.get(), dir); }
        , py::arg("dir"));
    obj_cls.def("set_symbol", [](LvObjWrapper &self, void * symbol) { lv_dropdown_set_symbol(self.get(), symbol); }
        , py::arg("symbol"));
    obj_cls.def("set_selected_highlight", [](LvObjWrapper &self, bool en) { lv_dropdown_set_selected_highlight(self.get(), en); }
        , py::arg("en"));
    obj_cls.def("get_selected_str", [](LvObjWrapper &self, char * buf, int buf_size) { lv_dropdown_get_selected_str(self.get(), buf, buf_size); }
        , py::arg("buf"), py::arg("buf_size"));
    obj_cls.def("open", [](LvObjWrapper &self) { lv_dropdown_open(self.get()); }
        );
    obj_cls.def("close", [](LvObjWrapper &self) { lv_dropdown_close(self.get()); }
        );
    obj_cls.def("imagebutton_set_src", [](LvObjWrapper &self, lv_imagebutton_state_t state, void * src_left, void * src_mid, void * src_right) { lv_imagebutton_set_src(self.get(), state, src_left, src_mid, src_right); }
        , py::arg("state"), py::arg("src_left"), py::arg("src_mid"), py::arg("src_right"));
    obj_cls.def("set_src_left", [](LvObjWrapper &self, lv_imagebutton_state_t state, void * src_left) { lv_imagebutton_set_src_left(self.get(), state, src_left); }
        , py::arg("state"), py::arg("src_left"));
    obj_cls.def("set_src_right", [](LvObjWrapper &self, lv_imagebutton_state_t state, void * src_right) { lv_imagebutton_set_src_right(self.get(), state, src_right); }
        , py::arg("state"), py::arg("src_right"));
    obj_cls.def("set_src_mid", [](LvObjWrapper &self, lv_imagebutton_state_t state, void * src_mid) { lv_imagebutton_set_src_mid(self.get(), state, src_mid); }
        , py::arg("state"), py::arg("src_mid"));
    obj_cls.def("set_state", [](LvObjWrapper &self, lv_imagebutton_state_t state) { lv_imagebutton_set_state(self.get(), state); }
        , py::arg("state"));
    obj_cls.def("set_textarea", [](LvObjWrapper &self, LvObjWrapper & ta) { lv_keyboard_set_textarea(self.get(), ta.get()); }
        , py::arg("ta"));
    obj_cls.def("keyboard_set_mode", [](LvObjWrapper &self, lv_keyboard_mode_t mode) { lv_keyboard_set_mode(self.get(), mode); }
        , py::arg("mode"));
    obj_cls.def("set_popovers", [](LvObjWrapper &self, bool en) { lv_keyboard_set_popovers(self.get(), en); }
        , py::arg("en"));
    obj_cls.def("set_brightness", [](LvObjWrapper &self, int bright) { lv_led_set_brightness(self.get(), bright); }
        , py::arg("bright"));
    obj_cls.def("on", [](LvObjWrapper &self) { lv_led_on(self.get()); }
        );
    obj_cls.def("off", [](LvObjWrapper &self) { lv_led_off(self.get()); }
        );
    obj_cls.def("toggle", [](LvObjWrapper &self) { lv_led_toggle(self.get()); }
        );
    obj_cls.def("set_y_invert", [](LvObjWrapper &self, bool en) { lv_line_set_y_invert(self.get(), en); }
        , py::arg("en"));
    obj_cls.def("set_button_text", [](LvObjWrapper &self, LvObjWrapper & btn, char * txt) { lv_list_set_button_text(self.get(), btn.get(), txt); }
        , py::arg("btn"), py::arg("txt"));
    obj_cls.def("lottie_set_buffer", [](LvObjWrapper &self, int w, int h, void * buf) { lv_lottie_set_buffer(self.get(), w, h, buf); }
        , py::arg("w"), py::arg("h"), py::arg("buf"));
    obj_cls.def("set_src_data", [](LvObjWrapper &self, void * src, int src_size) { lv_lottie_set_src_data(self.get(), src, src_size); }
        , py::arg("src"), py::arg("src_size"));
    obj_cls.def("set_src_file", [](LvObjWrapper &self, char * src) { lv_lottie_set_src_file(self.get(), src); }
        , py::arg("src"));
    obj_cls.def("set_page", [](LvObjWrapper &self, LvObjWrapper & page) { lv_menu_set_page(self.get(), page.get()); }
        , py::arg("page"));
    obj_cls.def("set_page_title", [](LvObjWrapper &self, char * title) { lv_menu_set_page_title(self.get(), title); }
        , py::arg("title"));
    obj_cls.def("set_page_title_static", [](LvObjWrapper &self, char * title) { lv_menu_set_page_title_static(self.get(), title); }
        , py::arg("title"));
    obj_cls.def("set_sidebar_page", [](LvObjWrapper &self, LvObjWrapper & page) { lv_menu_set_sidebar_page(self.get(), page.get()); }
        , py::arg("page"));
    obj_cls.def("set_mode_header", [](LvObjWrapper &self, lv_menu_mode_header_t mode) { lv_menu_set_mode_header(self.get(), mode); }
        , py::arg("mode"));
    obj_cls.def("set_mode_root_back_button", [](LvObjWrapper &self, lv_menu_mode_root_back_button_t mode) { lv_menu_set_mode_root_back_button(self.get(), mode); }
        , py::arg("mode"));
    obj_cls.def("set_load_page_event", [](LvObjWrapper &self, LvObjWrapper & obj, LvObjWrapper & page) { lv_menu_set_load_page_event(self.get(), obj.get(), page.get()); }
        , py::arg("obj"), py::arg("page"));
    obj_cls.def("clear_history", [](LvObjWrapper &self) { lv_menu_clear_history(self.get()); }
        );
    obj_cls.def("msgbox_close", [](LvObjWrapper &self) { lv_msgbox_close(self.get()); }
        );
    obj_cls.def("close_async", [](LvObjWrapper &self) { lv_msgbox_close_async(self.get()); }
        );
    obj_cls.def("set_size", [](LvObjWrapper &self, int size) { lv_qrcode_set_size(self.get(), size); }
        , py::arg("size"));
    obj_cls.def("set_data", [](LvObjWrapper &self, char * data) { lv_qrcode_set_data(self.get(), data); }
        , py::arg("data"));
    obj_cls.def("set_quiet_zone", [](LvObjWrapper &self, bool enable) { lv_qrcode_set_quiet_zone(self.get(), enable); }
        , py::arg("enable"));
    obj_cls.def("roller_set_options", [](LvObjWrapper &self, char * options, lv_roller_mode_t mode) { lv_roller_set_options(self.get(), options, mode); }
        , py::arg("options"), py::arg("mode"));
    obj_cls.def("roller_set_selected", [](LvObjWrapper &self, int sel_opt, bool anim) { lv_roller_set_selected(self.get(), sel_opt, anim); }
        , py::arg("sel_opt"), py::arg("anim"));
    obj_cls.def("set_visible_row_count", [](LvObjWrapper &self, int row_cnt) { lv_roller_set_visible_row_count(self.get(), row_cnt); }
        , py::arg("row_cnt"));
    obj_cls.def("roller_get_selected_str", [](LvObjWrapper &self, char * buf, int buf_size) { lv_roller_get_selected_str(self.get(), buf, buf_size); }
        , py::arg("buf"), py::arg("buf_size"));
    obj_cls.def("scale_set_mode", [](LvObjWrapper &self, lv_scale_mode_t mode) { lv_scale_set_mode(self.get(), mode); }
        , py::arg("mode"));
    obj_cls.def("set_total_tick_count", [](LvObjWrapper &self, int total_tick_count) { lv_scale_set_total_tick_count(self.get(), total_tick_count); }
        , py::arg("total_tick_count"));
    obj_cls.def("set_major_tick_every", [](LvObjWrapper &self, int major_tick_every) { lv_scale_set_major_tick_every(self.get(), major_tick_every); }
        , py::arg("major_tick_every"));
    obj_cls.def("set_label_show", [](LvObjWrapper &self, bool show_label) { lv_scale_set_label_show(self.get(), show_label); }
        , py::arg("show_label"));
    obj_cls.def("scale_set_range", [](LvObjWrapper &self, int min, int max) { lv_scale_set_range(self.get(), min, max); }
        , py::arg("min"), py::arg("max"));
    obj_cls.def("scale_set_min_value", [](LvObjWrapper &self, int min) { lv_scale_set_min_value(self.get(), min); }
        , py::arg("min"));
    obj_cls.def("scale_set_max_value", [](LvObjWrapper &self, int max) { lv_scale_set_max_value(self.get(), max); }
        , py::arg("max"));
    obj_cls.def("set_angle_range", [](LvObjWrapper &self, int angle_range) { lv_scale_set_angle_range(self.get(), angle_range); }
        , py::arg("angle_range"));
    obj_cls.def("scale_set_rotation", [](LvObjWrapper &self, int rotation) { lv_scale_set_rotation(self.get(), rotation); }
        , py::arg("rotation"));
    obj_cls.def("set_line_needle_value", [](LvObjWrapper &self, LvObjWrapper & needle_line, int needle_length, int value) { lv_scale_set_line_needle_value(self.get(), needle_line.get(), needle_length, value); }
        , py::arg("needle_line"), py::arg("needle_length"), py::arg("value"));
    obj_cls.def("set_image_needle_value", [](LvObjWrapper &self, LvObjWrapper & needle_img, int value) { lv_scale_set_image_needle_value(self.get(), needle_img.get(), value); }
        , py::arg("needle_img"), py::arg("value"));
    obj_cls.def("set_post_draw", [](LvObjWrapper &self, bool en) { lv_scale_set_post_draw(self.get(), en); }
        , py::arg("en"));
    obj_cls.def("set_draw_ticks_on_top", [](LvObjWrapper &self, bool en) { lv_scale_set_draw_ticks_on_top(self.get(), en); }
        , py::arg("en"));
    obj_cls.def("slider_set_value", [](LvObjWrapper &self, int value, bool anim) { lv_slider_set_value(self.get(), value, anim); }
        , py::arg("value"), py::arg("anim"));
    obj_cls.def("slider_set_start_value", [](LvObjWrapper &self, int value, bool anim) { lv_slider_set_start_value(self.get(), value, anim); }
        , py::arg("value"), py::arg("anim"));
    obj_cls.def("slider_set_range", [](LvObjWrapper &self, int min, int max) { lv_slider_set_range(self.get(), min, max); }
        , py::arg("min"), py::arg("max"));
    obj_cls.def("slider_set_min_value", [](LvObjWrapper &self, int min) { lv_slider_set_min_value(self.get(), min); }
        , py::arg("min"));
    obj_cls.def("slider_set_max_value", [](LvObjWrapper &self, int max) { lv_slider_set_max_value(self.get(), max); }
        , py::arg("max"));
    obj_cls.def("slider_set_mode", [](LvObjWrapper &self, lv_slider_mode_t mode) { lv_slider_set_mode(self.get(), mode); }
        , py::arg("mode"));
    obj_cls.def("slider_set_orientation", [](LvObjWrapper &self, lv_slider_orientation_t orientation) { lv_slider_set_orientation(self.get(), orientation); }
        , py::arg("orientation"));
    obj_cls.def("set_align", [](LvObjWrapper &self, lv_text_align_t align) { lv_spangroup_set_align(self.get(), align); }
        , py::arg("align"));
    obj_cls.def("spangroup_set_overflow", [](LvObjWrapper &self, lv_span_overflow_t overflow) { lv_spangroup_set_overflow(self.get(), overflow); }
        , py::arg("overflow"));
    obj_cls.def("set_indent", [](LvObjWrapper &self, int indent) { lv_spangroup_set_indent(self.get(), indent); }
        , py::arg("indent"));
    obj_cls.def("spangroup_set_mode", [](LvObjWrapper &self, lv_span_mode_t mode) { lv_spangroup_set_mode(self.get(), mode); }
        , py::arg("mode"));
    obj_cls.def("spangroup_set_max_lines", [](LvObjWrapper &self, int lines) { lv_spangroup_set_max_lines(self.get(), lines); }
        , py::arg("lines"));
    obj_cls.def("spangroup_refresh", [](LvObjWrapper &self) { lv_spangroup_refresh(self.get()); }
        );
    obj_cls.def("add_char", [](LvObjWrapper &self, int c) { lv_textarea_add_char(self.get(), c); }
        , py::arg("c"));
    obj_cls.def("textarea_add_text", [](LvObjWrapper &self, char * txt) { lv_textarea_add_text(self.get(), txt); }
        , py::arg("txt"));
    obj_cls.def("delete_char", [](LvObjWrapper &self) { lv_textarea_delete_char(self.get()); }
        );
    obj_cls.def("delete_char_forward", [](LvObjWrapper &self) { lv_textarea_delete_char_forward(self.get()); }
        );
    obj_cls.def("textarea_set_text", [](LvObjWrapper &self, char * txt) { lv_textarea_set_text(self.get(), txt); }
        , py::arg("txt"));
    obj_cls.def("set_placeholder_text", [](LvObjWrapper &self, char * txt) { lv_textarea_set_placeholder_text(self.get(), txt); }
        , py::arg("txt"));
    obj_cls.def("textarea_set_cursor_pos", [](LvObjWrapper &self, int pos) { lv_textarea_set_cursor_pos(self.get(), pos); }
        , py::arg("pos"));
    obj_cls.def("set_cursor_click_pos", [](LvObjWrapper &self, bool en) { lv_textarea_set_cursor_click_pos(self.get(), en); }
        , py::arg("en"));
    obj_cls.def("set_password_mode", [](LvObjWrapper &self, bool en) { lv_textarea_set_password_mode(self.get(), en); }
        , py::arg("en"));
    obj_cls.def("set_password_bullet", [](LvObjWrapper &self, char * bullet) { lv_textarea_set_password_bullet(self.get(), bullet); }
        , py::arg("bullet"));
    obj_cls.def("set_one_line", [](LvObjWrapper &self, bool en) { lv_textarea_set_one_line(self.get(), en); }
        , py::arg("en"));
    obj_cls.def("set_accepted_chars", [](LvObjWrapper &self, char * list) { lv_textarea_set_accepted_chars(self.get(), list); }
        , py::arg("list"));
    obj_cls.def("set_accepted_chars_static", [](LvObjWrapper &self, char * list) { lv_textarea_set_accepted_chars_static(self.get(), list); }
        , py::arg("list"));
    obj_cls.def("set_max_length", [](LvObjWrapper &self, int num) { lv_textarea_set_max_length(self.get(), num); }
        , py::arg("num"));
    obj_cls.def("set_insert_replace", [](LvObjWrapper &self, char * txt) { lv_textarea_set_insert_replace(self.get(), txt); }
        , py::arg("txt"));
    obj_cls.def("set_text_selection", [](LvObjWrapper &self, bool en) { lv_textarea_set_text_selection(self.get(), en); }
        , py::arg("en"));
    obj_cls.def("set_password_show_time", [](LvObjWrapper &self, int time) { lv_textarea_set_password_show_time(self.get(), time); }
        , py::arg("time"));
    obj_cls.def("textarea_set_align", [](LvObjWrapper &self, lv_text_align_t align) { lv_textarea_set_align(self.get(), align); }
        , py::arg("align"));
    obj_cls.def("clear_selection", [](LvObjWrapper &self) { lv_textarea_clear_selection(self.get()); }
        );
    obj_cls.def("cursor_right", [](LvObjWrapper &self) { lv_textarea_cursor_right(self.get()); }
        );
    obj_cls.def("cursor_left", [](LvObjWrapper &self) { lv_textarea_cursor_left(self.get()); }
        );
    obj_cls.def("cursor_down", [](LvObjWrapper &self) { lv_textarea_cursor_down(self.get()); }
        );
    obj_cls.def("cursor_up", [](LvObjWrapper &self) { lv_textarea_cursor_up(self.get()); }
        );
    obj_cls.def("spinbox_set_value", [](LvObjWrapper &self, int v) { lv_spinbox_set_value(self.get(), v); }
        , py::arg("v"));
    obj_cls.def("set_rollover", [](LvObjWrapper &self, bool rollover) { lv_spinbox_set_rollover(self.get(), rollover); }
        , py::arg("rollover"));
    obj_cls.def("set_digit_format", [](LvObjWrapper &self, int digit_count, int sep_pos) { lv_spinbox_set_digit_format(self.get(), digit_count, sep_pos); }
        , py::arg("digit_count"), py::arg("sep_pos"));
    obj_cls.def("set_digit_count", [](LvObjWrapper &self, int digit_count) { lv_spinbox_set_digit_count(self.get(), digit_count); }
        , py::arg("digit_count"));
    obj_cls.def("set_dec_point_pos", [](LvObjWrapper &self, int dec_point_pos) { lv_spinbox_set_dec_point_pos(self.get(), dec_point_pos); }
        , py::arg("dec_point_pos"));
    obj_cls.def("set_step", [](LvObjWrapper &self, int step) { lv_spinbox_set_step(self.get(), step); }
        , py::arg("step"));
    obj_cls.def("spinbox_set_range", [](LvObjWrapper &self, int min_value, int max_value) { lv_spinbox_set_range(self.get(), min_value, max_value); }
        , py::arg("min_value"), py::arg("max_value"));
    obj_cls.def("spinbox_set_min_value", [](LvObjWrapper &self, int min_value) { lv_spinbox_set_min_value(self.get(), min_value); }
        , py::arg("min_value"));
    obj_cls.def("spinbox_set_max_value", [](LvObjWrapper &self, int max_value) { lv_spinbox_set_max_value(self.get(), max_value); }
        , py::arg("max_value"));
    obj_cls.def("spinbox_set_cursor_pos", [](LvObjWrapper &self, int pos) { lv_spinbox_set_cursor_pos(self.get(), pos); }
        , py::arg("pos"));
    obj_cls.def("set_digit_step_direction", [](LvObjWrapper &self, lv_dir_t direction) { lv_spinbox_set_digit_step_direction(self.get(), direction); }
        , py::arg("direction"));
    obj_cls.def("step_next", [](LvObjWrapper &self) { lv_spinbox_step_next(self.get()); }
        );
    obj_cls.def("step_prev", [](LvObjWrapper &self) { lv_spinbox_step_prev(self.get()); }
        );
    obj_cls.def("increment", [](LvObjWrapper &self) { lv_spinbox_increment(self.get()); }
        );
    obj_cls.def("decrement", [](LvObjWrapper &self) { lv_spinbox_decrement(self.get()); }
        );
    obj_cls.def("set_anim_params", [](LvObjWrapper &self, int t, int angle) { lv_spinner_set_anim_params(self.get(), t, angle); }
        , py::arg("t"), py::arg("angle"));
    obj_cls.def("set_anim_duration", [](LvObjWrapper &self, int t) { lv_spinner_set_anim_duration(self.get(), t); }
        , py::arg("t"));
    obj_cls.def("set_arc_sweep", [](LvObjWrapper &self, int angle) { lv_spinner_set_arc_sweep(self.get(), angle); }
        , py::arg("angle"));
    obj_cls.def("switch_set_orientation", [](LvObjWrapper &self, lv_switch_orientation_t orientation) { lv_switch_set_orientation(self.get(), orientation); }
        , py::arg("orientation"));
    obj_cls.def("set_cell_value", [](LvObjWrapper &self, int row, int col, char * txt) { lv_table_set_cell_value(self.get(), row, col, txt); }
        , py::arg("row"), py::arg("col"), py::arg("txt"));
    obj_cls.def("set_row_count", [](LvObjWrapper &self, int row_cnt) { lv_table_set_row_count(self.get(), row_cnt); }
        , py::arg("row_cnt"));
    obj_cls.def("set_column_count", [](LvObjWrapper &self, int col_cnt) { lv_table_set_column_count(self.get(), col_cnt); }
        , py::arg("col_cnt"));
    obj_cls.def("set_column_width", [](LvObjWrapper &self, int col_id, int w) { lv_table_set_column_width(self.get(), col_id, w); }
        , py::arg("col_id"), py::arg("w"));
    obj_cls.def("set_cell_ctrl", [](LvObjWrapper &self, int row, int col, lv_table_cell_ctrl_t ctrl) { lv_table_set_cell_ctrl(self.get(), row, col, ctrl); }
        , py::arg("row"), py::arg("col"), py::arg("ctrl"));
    obj_cls.def("clear_cell_ctrl", [](LvObjWrapper &self, int row, int col, lv_table_cell_ctrl_t ctrl) { lv_table_clear_cell_ctrl(self.get(), row, col, ctrl); }
        , py::arg("row"), py::arg("col"), py::arg("ctrl"));
    obj_cls.def("set_cell_user_data", [](LvObjWrapper &self, int row, int col, void * user_data) { lv_table_set_cell_user_data(self.get(), row, col, user_data); }
        , py::arg("row"), py::arg("col"), py::arg("user_data"));
    obj_cls.def("set_selected_cell", [](LvObjWrapper &self, int row, int col) { lv_table_set_selected_cell(self.get(), row, col); }
        , py::arg("row"), py::arg("col"));
    obj_cls.def("get_selected_cell", [](LvObjWrapper &self, uint32_t * row, uint32_t * col) { lv_table_get_selected_cell(self.get(), row, col); }
        , py::arg("row"), py::arg("col"));
    obj_cls.def("set_tab_text", [](LvObjWrapper &self, int idx, char * new_name) { lv_tabview_set_tab_text(self.get(), idx, new_name); }
        , py::arg("idx"), py::arg("new_name"));
    obj_cls.def("set_active", [](LvObjWrapper &self, int idx, bool anim_en) { lv_tabview_set_active(self.get(), idx, anim_en); }
        , py::arg("idx"), py::arg("anim_en"));
    obj_cls.def("set_tab_bar_position", [](LvObjWrapper &self, lv_dir_t dir) { lv_tabview_set_tab_bar_position(self.get(), dir); }
        , py::arg("dir"));
    obj_cls.def("set_tab_bar_size", [](LvObjWrapper &self, int size) { lv_tabview_set_tab_bar_size(self.get(), size); }
        , py::arg("size"));
    obj_cls.def("set_tile", [](LvObjWrapper &self, LvObjWrapper & tile_obj, bool anim_en) { lv_tileview_set_tile(self.get(), tile_obj.get(), anim_en); }
        , py::arg("tile_obj"), py::arg("anim_en"));
    obj_cls.def("set_tile_by_index", [](LvObjWrapper &self, int col_id, int row_id, bool anim_en) { lv_tileview_set_tile_by_index(self.get(), col_id, row_id, anim_en); }
        , py::arg("col_id"), py::arg("row_id"), py::arg("anim_en"));

    /* Event callback support */
    obj_cls.def("add_event_cb", [](LvObjWrapper &self, int filter, py::function callback) {
        register_event_callback(self.get(), (lv_event_code_t)filter, callback);
    }, py::arg("filter"), py::arg("callback"));

    /* Color utilities */
    m.def("color_make", [](uint8_t r, uint8_t g, uint8_t b) { return lv_color_make(r, g, b); },
        py::arg("r"), py::arg("g"), py::arg("b"));
    m.def("color_hex", [](uint32_t c) { return lv_color_hex(c); }, py::arg("c"));
    m.def("color_black", []() { return lv_color_black(); });
    m.def("color_white", []() { return lv_color_white(); });


    /* Font default */
    m.def("font_get_default", []() -> const lv_font_t * { return lv_font_get_default(); });

    /* Global module functions */
    m.def("memzero", [](void * dst, int len) { lv_memzero(dst, len); }
        , py::arg("dst"), py::arg("len"));
    m.def("bidi_calculate_align", [](lv_text_align_t * align, lv_base_dir_t * base_dir, char * txt) { lv_bidi_calculate_align(align, base_dir, txt); }
        , py::arg("align"), py::arg("base_dir"), py::arg("txt"));
    m.def("rand_set_seed", [](int seed) { lv_rand_set_seed(seed); }
        , py::arg("seed"));
    m.def("anim_delete_all", []() { lv_anim_delete_all(); }
        );
    m.def("anim_refr_now", []() { lv_anim_refr_now(); }
        );
    m.def("group_swap_obj", [](LvObjWrapper & obj1, LvObjWrapper & obj2) { lv_group_swap_obj(obj1.get(), obj2.get()); }
        , py::arg("obj1"), py::arg("obj2"));
    m.def("group_remove_obj", [](LvObjWrapper & obj) { lv_group_remove_obj(obj.get()); }
        , py::arg("obj"));
    m.def("group_focus_obj", [](LvObjWrapper & obj) { lv_group_focus_obj(obj.get()); }
        , py::arg("obj"));
    m.def("init", []() { lv_init(); }
        );
    m.def("deinit", []() { lv_deinit(); }
        );
    m.def("mem_init", []() { lv_mem_init(); }
        );
    m.def("mem_deinit", []() { lv_mem_deinit(); }
        );
    m.def("free", [](void * data) { lv_free(data); }
        , py::arg("data"));
    m.def("free_core", [](void * p) { lv_free_core(p); }
        , py::arg("p"));
    m.def("memset", [](void * dst, int v, int len) { lv_memset(dst, v, len); }
        , py::arg("dst"), py::arg("v"), py::arg("len"));
    m.def("flex_init", []() { lv_flex_init(); }
        );
    m.def("grid_init", []() { lv_grid_init(); }
        );
    m.def("timer_periodic_handler", []() { lv_timer_periodic_handler(); }
        );
    m.def("timer_enable", [](bool en) { lv_timer_enable(en); }
        , py::arg("en"));
    m.def("screen_load", [](LvObjWrapper & scr) { lv_screen_load(scr.get()); }
        , py::arg("scr"));
    m.def("screen_load_anim", [](LvObjWrapper & scr, lv_screen_load_anim_t anim_type, int time, int delay, bool auto_del) { lv_screen_load_anim(scr.get(), anim_type, time, delay, auto_del); }
        , py::arg("scr"), py::arg("anim_type"), py::arg("time"), py::arg("delay"), py::arg("auto_del"));
    m.def("obj_enable_style_refresh", [](bool en) { lv_obj_enable_style_refresh(en); }
        , py::arg("en"));
    m.def("draw_init", []() { lv_draw_init(); }
        );
    m.def("draw_deinit", []() { lv_draw_deinit(); }
        );
    m.def("draw_dispatch", []() { lv_draw_dispatch(); }
        );
    m.def("draw_dispatch_wait_for_request", []() { lv_draw_dispatch_wait_for_request(); }
        );
    m.def("draw_wait_for_finish", []() { lv_draw_wait_for_finish(); }
        );
    m.def("draw_dispatch_request", []() { lv_draw_dispatch_request(); }
        );
    m.def("draw_unit_send_event", [](char * name, lv_event_code_t code, void * param) { lv_draw_unit_send_event(name, code, param); }
        , py::arg("name"), py::arg("code"), py::arg("param"));
    m.def("fs_remove_drive", [](char letter) { lv_fs_remove_drive(letter); }
        , py::arg("letter"));
    m.def("draw_sw_i1_to_argb8888", [](void * buf_i1, void * buf_argb8888, int width, int height, int buf_i1_stride, int buf_argb8888_stride, int index0_color, int index1_color) { lv_draw_sw_i1_to_argb8888(buf_i1, buf_argb8888, width, height, buf_i1_stride, buf_argb8888_stride, index0_color, index1_color); }
        , py::arg("buf_i1"), py::arg("buf_argb8888"), py::arg("width"), py::arg("height"), py::arg("buf_i1_stride"), py::arg("buf_argb8888_stride"), py::arg("index0_color"), py::arg("index1_color"));
    m.def("draw_sw_rgb565_swap", [](void * buf, int buf_size_px) { lv_draw_sw_rgb565_swap(buf, buf_size_px); }
        , py::arg("buf"), py::arg("buf_size_px"));
    m.def("draw_sw_i1_invert", [](void * buf, int buf_size) { lv_draw_sw_i1_invert(buf, buf_size); }
        , py::arg("buf"), py::arg("buf_size"));
    m.def("draw_sw_i1_convert_to_vtiled", [](void * buf, int buf_size, int width, int height, void * out_buf, int out_buf_size, bool bit_order_lsb) { lv_draw_sw_i1_convert_to_vtiled(buf, buf_size, width, height, out_buf, out_buf_size, bit_order_lsb); }
        , py::arg("buf"), py::arg("buf_size"), py::arg("width"), py::arg("height"), py::arg("out_buf"), py::arg("out_buf_size"), py::arg("bit_order_lsb"));
    m.def("draw_sw_rotate", [](void * src, void * dest, int src_width, int src_height, int src_stride, int dest_stride, lv_display_rotation_t rotation, lv_color_format_t color_format) { lv_draw_sw_rotate(src, dest, src_width, src_height, src_stride, dest_stride, rotation, color_format); }
        , py::arg("src"), py::arg("dest"), py::arg("src_width"), py::arg("src_height"), py::arg("src_stride"), py::arg("dest_stride"), py::arg("rotation"), py::arg("color_format"));
    m.def("fs_stdio_init", []() { lv_fs_stdio_init(); }
        );
    m.def("bin_decoder_init", []() { lv_bin_decoder_init(); }
        );
    m.def("bmp_init", []() { lv_bmp_init(); }
        );
    m.def("bmp_deinit", []() { lv_bmp_deinit(); }
        );
    m.def("lodepng_init", []() { lv_lodepng_init(); }
        );
    m.def("lodepng_deinit", []() { lv_lodepng_deinit(); }
        );
    m.def("tjpgd_init", []() { lv_tjpgd_init(); }
        );
    m.def("tjpgd_deinit", []() { lv_tjpgd_deinit(); }
        );
    m.def("lock", []() { lv_lock(); }
        );
    m.def("unlock", []() { lv_unlock(); }
        );
    m.def("sleep_ms", [](int ms) { lv_sleep_ms(ms); }
        , py::arg("ms"));
    m.def("theme_apply", [](LvObjWrapper & obj) { lv_theme_apply(obj.get()); }
        , py::arg("obj"));
    m.def("theme_default_deinit", []() { lv_theme_default_deinit(); }
        );
    m.def("theme_mono_deinit", []() { lv_theme_mono_deinit(); }
        );
    m.def("theme_simple_deinit", []() { lv_theme_simple_deinit(); }
        );
    m.def("tick_inc", [](int tick_period) { lv_tick_inc(tick_period); }
        , py::arg("tick_period"));
    m.def("delay_ms", [](int ms) { lv_delay_ms(ms); }
        , py::arg("ms"));
    m.def("span_stack_init", []() { lv_span_stack_init(); }
        );
    m.def("span_stack_deinit", []() { lv_span_stack_deinit(); }
        );

    /* Functions not generated:
     * lv_sqr
     * lv_area_copy
     * lv_array_size
     * lv_array_capacity
     * lv_array_is_empty
     * lv_array_is_full
     * lv_array_clear
     * lv_array_front
     * lv_array_back
     * lv_color_is_in_range
     * lv_color16_to_color
     * lv_color_swap_16
     * lv_streq
     * lv_draw_buf_has_flag
     * lv_draw_buf_set_flag
     * lv_draw_buf_clear_flag
     * lv_font_get_top_trim
     * lv_font_get_bottom_trim
     * lv_style_is_const
     * lv_style_get_prop_inlined
     * lv_style_get_prop_group
     * lv_style_set_size
     * lv_style_set_pad_all
     * lv_style_set_pad_hor
     * lv_style_set_pad_ver
     * lv_style_set_pad_gap
     * lv_style_set_margin_hor
     * lv_style_set_margin_ver
     * lv_style_set_margin_all
     * lv_style_set_transform_scale
     * lv_style_prop_has_flag
     * lv_obj_style_get_selector_state
     * lv_obj_style_get_selector_part
     * lv_obj_get_style_width
     * lv_obj_get_style_min_width
     * lv_obj_get_style_max_width
     * lv_obj_get_style_height
     * lv_obj_get_style_min_height
     * lv_obj_get_style_max_height
     * lv_obj_get_style_length
     * lv_obj_get_style_x
     * lv_obj_get_style_y
     * lv_obj_get_style_align
     * lv_obj_get_style_transform_width
     * lv_obj_get_style_transform_height
     * lv_obj_get_style_translate_x
     * lv_obj_get_style_translate_y
     * lv_obj_get_style_translate_radial
     * lv_obj_get_style_transform_scale_x
     * lv_obj_get_style_transform_scale_y
     * lv_obj_get_style_transform_rotation
     * lv_obj_get_style_transform_pivot_x
     * lv_obj_get_style_transform_pivot_y
     * lv_obj_get_style_transform_skew_x
     * lv_obj_get_style_transform_skew_y
     * lv_obj_get_style_pad_top
     * lv_obj_get_style_pad_bottom
     * lv_obj_get_style_pad_left
     * lv_obj_get_style_pad_right
     * lv_obj_get_style_pad_row
     * lv_obj_get_style_pad_column
     * lv_obj_get_style_pad_radial
     * lv_obj_get_style_margin_top
     * lv_obj_get_style_margin_bottom
     * lv_obj_get_style_margin_left
     * lv_obj_get_style_margin_right
     * lv_obj_get_style_bg_color
     * lv_obj_get_style_bg_color_filtered
     * lv_obj_get_style_bg_opa
     * lv_obj_get_style_bg_grad_color
     * lv_obj_get_style_bg_grad_color_filtered
     * lv_obj_get_style_bg_grad_dir
     * lv_obj_get_style_bg_main_stop
     * lv_obj_get_style_bg_grad_stop
     * lv_obj_get_style_bg_main_opa
     * lv_obj_get_style_bg_grad_opa
     * lv_obj_get_style_bg_grad
     * lv_obj_get_style_bg_image_src
     * lv_obj_get_style_bg_image_opa
     * lv_obj_get_style_bg_image_recolor
     * lv_obj_get_style_bg_image_recolor_filtered
     * lv_obj_get_style_bg_image_recolor_opa
     * lv_obj_get_style_bg_image_tiled
     * lv_obj_get_style_border_color
     * lv_obj_get_style_border_color_filtered
     * lv_obj_get_style_border_opa
     * lv_obj_get_style_border_width
     * lv_obj_get_style_border_side
     * lv_obj_get_style_border_post
     * lv_obj_get_style_outline_width
     * lv_obj_get_style_outline_color
     * lv_obj_get_style_outline_color_filtered
     * lv_obj_get_style_outline_opa
     * lv_obj_get_style_outline_pad
     * lv_obj_get_style_shadow_width
     * lv_obj_get_style_shadow_offset_x
     * lv_obj_get_style_shadow_offset_y
     * lv_obj_get_style_shadow_spread
     * lv_obj_get_style_shadow_color
     * lv_obj_get_style_shadow_color_filtered
     * lv_obj_get_style_shadow_opa
     * lv_obj_get_style_image_opa
     * lv_obj_get_style_image_recolor
     * lv_obj_get_style_image_recolor_filtered
     * lv_obj_get_style_image_recolor_opa
     * lv_obj_get_style_image_colorkey
     * lv_obj_get_style_line_width
     * lv_obj_get_style_line_dash_width
     * lv_obj_get_style_line_dash_gap
     * lv_obj_get_style_line_rounded
     * lv_obj_get_style_line_color
     * lv_obj_get_style_line_color_filtered
     * lv_obj_get_style_line_opa
     * lv_obj_get_style_arc_width
     * lv_obj_get_style_arc_rounded
     * lv_obj_get_style_arc_color
     * lv_obj_get_style_arc_color_filtered
     * lv_obj_get_style_arc_opa
     * lv_obj_get_style_arc_image_src
     * lv_obj_get_style_text_color
     * lv_obj_get_style_text_color_filtered
     * lv_obj_get_style_text_opa
     * lv_obj_get_style_text_font
     * lv_obj_get_style_text_letter_space
     * lv_obj_get_style_text_line_space
     * lv_obj_get_style_text_decor
     * lv_obj_get_style_text_align
     * lv_obj_get_style_text_outline_stroke_color
     * lv_obj_get_style_text_outline_stroke_color_filtered
     * lv_obj_get_style_text_outline_stroke_width
     * lv_obj_get_style_text_outline_stroke_opa
     * lv_obj_get_style_text_leading_trim
     * lv_obj_get_style_blur_radius
     * lv_obj_get_style_blur_backdrop
     * lv_obj_get_style_blur_quality
     * lv_obj_get_style_drop_shadow_radius
     * lv_obj_get_style_drop_shadow_offset_x
     * lv_obj_get_style_drop_shadow_offset_y
     * lv_obj_get_style_drop_shadow_color
     * lv_obj_get_style_drop_shadow_color_filtered
     * lv_obj_get_style_drop_shadow_opa
     * lv_obj_get_style_drop_shadow_quality
     * lv_obj_get_style_radius
     * lv_obj_get_style_radial_offset
     * lv_obj_get_style_clip_corner
     * lv_obj_get_style_opa
     * lv_obj_get_style_opa_layered
     * lv_obj_get_style_color_filter_dsc
     * lv_obj_get_style_color_filter_opa
     * lv_obj_get_style_recolor
     * lv_obj_get_style_recolor_opa
     * lv_obj_get_style_anim
     * lv_obj_get_style_anim_duration
     * lv_obj_get_style_transition
     * lv_obj_get_style_blend_mode
     * lv_obj_get_style_layout
     * lv_obj_get_style_base_dir
     * lv_obj_get_style_bitmap_mask_src
     * lv_obj_get_style_rotary_sensitivity
     * lv_obj_get_style_flex_flow
     * lv_obj_get_style_flex_main_place
     * lv_obj_get_style_flex_cross_place
     * lv_obj_get_style_flex_track_place
     * lv_obj_get_style_flex_grow
     * lv_obj_get_style_grid_column_dsc_array
     * lv_obj_get_style_grid_column_align
     * lv_obj_get_style_grid_row_dsc_array
     * lv_obj_get_style_grid_row_align
     * lv_obj_get_style_grid_cell_column_pos
     * lv_obj_get_style_grid_cell_x_align
     * lv_obj_get_style_grid_cell_column_span
     * lv_obj_get_style_grid_cell_row_pos
     * lv_obj_get_style_grid_cell_y_align
     * lv_obj_get_style_grid_cell_row_span
     * lv_obj_get_style_space_left
     * lv_obj_get_style_space_right
     * lv_obj_get_style_space_top
     * lv_obj_get_style_space_bottom
     * lv_obj_get_style_transform_scale_x_safe
     * lv_obj_get_style_transform_scale_y_safe
     * lv_vector_path_append_rect
     * lv_task_handler
     * lv_version_major
     * lv_version_minor
     * lv_version_patch
     * lv_version_info
     * lv_trigo_sin
     * lv_trigo_cos
     * lv_cubic_bezier
     * lv_bezier3
     * lv_atan2
     * lv_sqrt
     * lv_sqrt32
     * lv_pow
     * lv_map
     * lv_rand
     * lv_anim_init
     * lv_anim_set_var
     * lv_anim_set_exec_cb
     * lv_anim_set_duration
     * lv_anim_set_delay
     * lv_anim_resume
     * lv_anim_pause
     * lv_anim_pause_for
     * lv_anim_is_paused
     * lv_anim_set_values
     * lv_anim_set_custom_exec_cb
     * lv_anim_set_path_cb
     * lv_anim_set_start_cb
     * lv_anim_set_get_value_cb
     * lv_anim_set_completed_cb
     * lv_anim_set_deleted_cb
     * lv_anim_set_reverse_duration
     * lv_anim_set_reverse_time
     * lv_anim_set_reverse_delay
     * lv_anim_set_repeat_count
     * lv_anim_set_repeat_delay
     * lv_anim_set_early_apply
     * lv_anim_set_user_data
     * lv_anim_set_bezier3_param
     * lv_anim_start
     * lv_anim_get_delay
     * lv_anim_get_playtime
     * lv_anim_get_time
     * lv_anim_get_repeat_count
     * lv_anim_get_user_data
     * lv_anim_delete
     * lv_anim_get
     * lv_anim_get_timer
     * lv_anim_custom_delete
     * lv_anim_custom_get
     * lv_anim_count_running
     * lv_anim_speed
     * lv_anim_speed_clamped
     * lv_anim_resolve_speed
     * lv_anim_speed_to_time
     * lv_anim_path_linear
     * lv_anim_path_ease_in
     * lv_anim_path_ease_out
     * lv_anim_path_ease_in_out
     * lv_anim_path_overshoot
     * lv_anim_path_bounce
     * lv_anim_path_step
     * lv_anim_path_custom_bezier3
     * lv_anim_timeline_create
     * lv_anim_timeline_delete
     * lv_anim_timeline_add
     * lv_anim_timeline_start
     * lv_anim_timeline_pause
     * lv_anim_timeline_set_reverse
     * lv_anim_timeline_set_delay
     * lv_anim_timeline_set_repeat_count
     * lv_anim_timeline_set_repeat_delay
     * lv_anim_timeline_set_progress
     * lv_anim_timeline_set_user_data
     * lv_anim_timeline_get_playtime
     * lv_anim_timeline_get_reverse
     * lv_anim_timeline_get_delay
     * lv_anim_timeline_get_progress
     * lv_anim_timeline_get_repeat_count
     * lv_anim_timeline_get_repeat_delay
     * lv_anim_timeline_get_user_data
     * lv_anim_timeline_merge
     * lv_area_set
     * lv_area_get_width
     * lv_area_get_height
     * lv_area_set_width
     * lv_area_set_height
     * lv_area_get_size
     * lv_area_increase
     * lv_area_move
     * lv_area_align
     * lv_point_transform
     * lv_point_array_transform
     * lv_point_from_precise
     * lv_point_to_precise
     * lv_point_set
     * lv_point_precise_set
     * lv_point_swap
     * lv_point_precise_swap
     * lv_pct
     * lv_pct_to_px
     * lv_array_init
     * lv_array_init_from_buf
     * lv_array_resize
     * lv_array_deinit
     * lv_array_copy
     * lv_array_shrink
     * lv_array_remove
     * lv_array_remove_unordered
     * lv_array_erase
     * lv_array_concat
     * lv_array_push_back
     * lv_array_assign
     * lv_array_at
     * lv_event_send
     * lv_event_add
     * lv_event_remove_dsc
     * lv_event_get_count
     * lv_event_get_dsc
     * lv_event_dsc_get_cb
     * lv_event_dsc_get_user_data
     * lv_event_remove
     * lv_event_remove_all
     * lv_event_get_target
     * lv_event_get_current_target
     * lv_event_get_code
     * lv_event_get_param
     * lv_event_get_user_data
     * lv_event_stop_bubbling
     * lv_event_stop_trickling
     * lv_event_stop_processing
     * lv_event_free_user_data_cb
     * lv_event_register_id
     * lv_event_code_get_name
     * lv_group_create
     * lv_group_delete
     * lv_group_set_default
     * lv_group_get_default
     * lv_group_add_obj
     * lv_group_remove_all_objs
     * lv_group_focus_next
     * lv_group_focus_prev
     * lv_group_focus_freeze
     * lv_group_send_data
     * lv_group_set_focus_cb
     * lv_group_set_edge_cb
     * lv_group_set_refocus_policy
     * lv_group_set_editing
     * lv_group_set_wrap
     * lv_group_get_focused
     * lv_group_get_focus_cb
     * lv_group_get_edge_cb
     * lv_group_get_editing
     * lv_group_get_wrap
     * lv_group_get_obj_count
     * lv_group_get_obj_by_index
     * lv_group_get_count
     * lv_group_by_index
     * lv_group_set_user_data
     * lv_group_get_user_data
     * lv_is_initialized
     * lv_matrix_identity
     * lv_matrix_translate
     * lv_matrix_scale
     * lv_matrix_rotate
     * lv_matrix_skew
     * lv_matrix_multiply
     * lv_matrix_inverse
     * lv_matrix_transform_precise_point
     * lv_matrix_transform_area
     * lv_matrix_is_identity
     * lv_matrix_is_identity_or_translation
     * lv_matrix_transpose
     * lv_log_register_print_cb
     * lv_log
     * lv_log_add
     * lv_mem_add_pool
     * lv_mem_remove_pool
     * lv_malloc
     * lv_calloc
     * lv_zalloc
     * lv_malloc_zeroed
     * lv_realloc
     * lv_reallocf
     * lv_malloc_core
     * lv_realloc_core
     * lv_mem_monitor_core
     * lv_mem_test_core
     * lv_mem_test
     * lv_mem_monitor
     * lv_color_format_get_bpp
     * lv_color_format_get_size
     * lv_color_format_has_alpha
     * lv_color_to_32
     * lv_color_to_int
     * lv_color_eq
     * lv_color32_eq
     * lv_color_hex
     * lv_color_make
     * lv_color32_make
     * lv_color_hex3
     * lv_color_to_u16
     * lv_color_to_u32
     * lv_color_16_16_mix
     * lv_color_lighten
     * lv_color_darken
     * lv_color_hsv_to_rgb
     * lv_color_rgb_to_hsv
     * lv_color_to_hsv
     * lv_color_white
     * lv_color_black
     * lv_color_premultiply
     * lv_color16_premultiply
     * lv_color_luminance
     * lv_color16_luminance
     * lv_color24_luminance
     * lv_color32_luminance
     * lv_palette_main
     * lv_palette_lighten
     * lv_palette_darken
     * lv_color_mix
     * lv_color_mix32
     * lv_color_mix32_premultiplied
     * lv_color_brightness
     * lv_color_filter_dsc_init
     * lv_color_over32
     * lv_memcpy
     * lv_memmove
     * lv_memcmp
     * lv_strlen
     * lv_strnlen
     * lv_strlcpy
     * lv_strncpy
     * lv_strcpy
     * lv_strcmp
     * lv_strncmp
     * lv_strdup
     * lv_strndup
     * lv_strcat
     * lv_strncat
     * lv_strchr
     * lv_draw_buf_init_with_default_handlers
     * lv_draw_buf_handlers_init
     * lv_draw_buf_get_handlers
     * lv_draw_buf_get_font_handlers
     * lv_draw_buf_get_image_handlers
     * lv_draw_buf_align
     * lv_draw_buf_align_ex
     * lv_draw_buf_invalidate_cache
     * lv_draw_buf_flush_cache
     * lv_draw_buf_width_to_stride
     * lv_draw_buf_width_to_stride_ex
     * lv_draw_buf_clear
     * lv_draw_buf_create
     * lv_draw_buf_create_ex
     * lv_draw_buf_dup
     * lv_draw_buf_dup_ex
     * lv_draw_buf_init
     * lv_draw_buf_reshape
     * lv_draw_buf_destroy
     * lv_draw_buf_copy
     * lv_draw_buf_goto_xy
     * lv_draw_buf_adjust_stride
     * lv_draw_buf_premultiply
     * lv_draw_buf_from_image
     * lv_draw_buf_to_image
     * lv_draw_buf_set_palette
     * lv_image_buf_set_palette
     * lv_image_buf_free
     * lv_font_get_glyph_bitmap
     * lv_font_get_glyph_static_bitmap
     * lv_font_get_glyph_dsc
     * lv_font_glyph_release_draw_data
     * lv_font_get_glyph_width
     * lv_font_get_line_height
     * lv_font_set_kerning
     * lv_font_get_default
     * lv_font_info_is_equal
     * lv_font_has_static_bitmap
     * lv_snprintf
     * lv_vsnprintf
     * lv_text_get_size
     * lv_grad_init_stops
     * lv_grad_horizontal_init
     * lv_grad_vertical_init
     * lv_grad_linear_init
     * lv_grad_radial_init
     * lv_grad_radial_set_focal
     * lv_grad_conical_init
     * lv_layout_create
     * lv_layout_register
     * lv_obj_set_grid_dsc_array
     * lv_grid_fr
     * lv_style_init
     * lv_style_reset
     * lv_style_copy
     * lv_style_merge
     * lv_style_register_prop
     * lv_style_get_num_custom_props
     * lv_style_remove_prop
     * lv_style_set_prop
     * lv_style_get_prop
     * lv_style_transition_dsc_init
     * lv_style_prop_get_default
     * lv_style_is_empty
     * lv_style_prop_lookup_flags
     * lv_style_set_width
     * lv_style_set_min_width
     * lv_style_set_max_width
     * lv_style_set_height
     * lv_style_set_min_height
     * lv_style_set_max_height
     * lv_style_set_length
     * lv_style_set_x
     * lv_style_set_y
     * lv_style_set_align
     * lv_style_set_transform_width
     * lv_style_set_transform_height
     * lv_style_set_translate_x
     * lv_style_set_translate_y
     * lv_style_set_translate_radial
     * lv_style_set_transform_scale_x
     * lv_style_set_transform_scale_y
     * lv_style_set_transform_rotation
     * lv_style_set_transform_pivot_x
     * lv_style_set_transform_pivot_y
     * lv_style_set_transform_skew_x
     * lv_style_set_transform_skew_y
     * lv_style_set_pad_top
     * lv_style_set_pad_bottom
     * lv_style_set_pad_left
     * lv_style_set_pad_right
     * lv_style_set_pad_row
     * lv_style_set_pad_column
     * lv_style_set_pad_radial
     * lv_style_set_margin_top
     * lv_style_set_margin_bottom
     * lv_style_set_margin_left
     * lv_style_set_margin_right
     * lv_style_set_bg_color
     * lv_style_set_bg_opa
     * lv_style_set_bg_grad_color
     * lv_style_set_bg_grad_dir
     * lv_style_set_bg_main_stop
     * lv_style_set_bg_grad_stop
     * lv_style_set_bg_main_opa
     * lv_style_set_bg_grad_opa
     * lv_style_set_bg_grad
     * lv_style_set_bg_image_src
     * lv_style_set_bg_image_opa
     * lv_style_set_bg_image_recolor
     * lv_style_set_bg_image_recolor_opa
     * lv_style_set_bg_image_tiled
     * lv_style_set_border_color
     * lv_style_set_border_opa
     * lv_style_set_border_width
     * lv_style_set_border_side
     * lv_style_set_border_post
     * lv_style_set_outline_width
     * lv_style_set_outline_color
     * lv_style_set_outline_opa
     * lv_style_set_outline_pad
     * lv_style_set_shadow_width
     * lv_style_set_shadow_offset_x
     * lv_style_set_shadow_offset_y
     * lv_style_set_shadow_spread
     * lv_style_set_shadow_color
     * lv_style_set_shadow_opa
     * lv_style_set_image_opa
     * lv_style_set_image_recolor
     * lv_style_set_image_recolor_opa
     * lv_style_set_image_colorkey
     * lv_style_set_line_width
     * lv_style_set_line_dash_width
     * lv_style_set_line_dash_gap
     * lv_style_set_line_rounded
     * lv_style_set_line_color
     * lv_style_set_line_opa
     * lv_style_set_arc_width
     * lv_style_set_arc_rounded
     * lv_style_set_arc_color
     * lv_style_set_arc_opa
     * lv_style_set_arc_image_src
     * lv_style_set_text_color
     * lv_style_set_text_opa
     * lv_style_set_text_font
     * lv_style_set_text_letter_space
     * lv_style_set_text_line_space
     * lv_style_set_text_decor
     * lv_style_set_text_align
     * lv_style_set_text_outline_stroke_color
     * lv_style_set_text_outline_stroke_width
     * lv_style_set_text_outline_stroke_opa
     * lv_style_set_text_leading_trim
     * lv_style_set_blur_radius
     * lv_style_set_blur_backdrop
     * lv_style_set_blur_quality
     * lv_style_set_drop_shadow_radius
     * lv_style_set_drop_shadow_offset_x
     * lv_style_set_drop_shadow_offset_y
     * lv_style_set_drop_shadow_color
     * lv_style_set_drop_shadow_opa
     * lv_style_set_drop_shadow_quality
     * lv_style_set_radius
     * lv_style_set_radial_offset
     * lv_style_set_clip_corner
     * lv_style_set_opa
     * lv_style_set_opa_layered
     * lv_style_set_color_filter_dsc
     * lv_style_set_color_filter_opa
     * lv_style_set_recolor
     * lv_style_set_recolor_opa
     * lv_style_set_anim
     * lv_style_set_anim_duration
     * lv_style_set_transition
     * lv_style_set_blend_mode
     * lv_style_set_layout
     * lv_style_set_base_dir
     * lv_style_set_bitmap_mask_src
     * lv_style_set_rotary_sensitivity
     * lv_style_set_flex_flow
     * lv_style_set_flex_main_place
     * lv_style_set_flex_cross_place
     * lv_style_set_flex_track_place
     * lv_style_set_flex_grow
     * lv_style_set_grid_column_dsc_array
     * lv_style_set_grid_column_align
     * lv_style_set_grid_row_dsc_array
     * lv_style_set_grid_row_align
     * lv_style_set_grid_cell_column_pos
     * lv_style_set_grid_cell_x_align
     * lv_style_set_grid_cell_column_span
     * lv_style_set_grid_cell_row_pos
     * lv_style_set_grid_cell_y_align
     * lv_style_set_grid_cell_row_span
     * lv_timer_handler
     * lv_timer_handler_run_in_period
     * lv_timer_handler_set_resume_cb
     * lv_timer_create_basic
     * lv_timer_create
     * lv_timer_delete
     * lv_timer_pause
     * lv_timer_resume
     * lv_timer_set_cb
     * lv_timer_set_period
     * lv_timer_ready
     * lv_timer_set_repeat_count
     * lv_timer_set_auto_delete
     * lv_timer_set_user_data
     * lv_timer_reset
     * lv_timer_get_idle
     * lv_timer_get_time_until_next
     * lv_timer_get_next
     * lv_timer_get_user_data
     * lv_timer_get_paused
     * lv_display_create
     * lv_display_delete
     * lv_display_set_default
     * lv_display_get_default
     * lv_display_get_next
     * lv_display_set_resolution
     * lv_display_set_physical_resolution
     * lv_display_set_offset
     * lv_display_set_rotation
     * lv_display_set_matrix_rotation
     * lv_display_set_dpi
     * lv_display_get_horizontal_resolution
     * lv_display_get_vertical_resolution
     * lv_display_get_original_horizontal_resolution
     * lv_display_get_original_vertical_resolution
     * lv_display_get_physical_horizontal_resolution
     * lv_display_get_physical_vertical_resolution
     * lv_display_get_offset_x
     * lv_display_get_offset_y
     * lv_display_get_rotation
     * lv_display_get_matrix_rotation
     * lv_display_get_dpi
     * lv_display_set_buffers
     * lv_display_set_buffers_with_stride
     * lv_display_set_draw_buffers
     * lv_display_set_3rd_draw_buffer
     * lv_display_set_render_mode
     * lv_display_set_flush_cb
     * lv_display_get_flush_cb
     * lv_display_set_flush_wait_cb
     * lv_display_set_sync_cb
     * lv_display_set_sync_wait_cb
     * lv_display_set_color_format
     * lv_display_get_color_format
     * lv_display_set_tile_cnt
     * lv_display_get_tile_cnt
     * lv_display_set_antialiasing
     * lv_display_get_antialiasing
     * lv_display_flush_ready
     * lv_display_flush_is_last
     * lv_display_sync_ready
     * lv_display_sync_is_last
     * lv_display_is_double_buffered
     * lv_display_get_render_mode
     * lv_display_get_screen_active
     * lv_display_get_screen_prev
     * lv_display_get_screen_loading
     * lv_display_get_layer_top
     * lv_display_get_layer_sys
     * lv_display_get_layer_bottom
     * lv_display_get_screen_by_name
     * lv_screen_active
     * lv_layer_top
     * lv_layer_sys
     * lv_layer_bottom
     * lv_display_add_event_cb
     * lv_display_get_event_count
     * lv_display_get_event_dsc
     * lv_display_remove_event
     * lv_display_remove_event_cb_with_user_data
     * lv_display_send_event
     * lv_event_get_invalidated_area
     * lv_display_set_theme
     * lv_display_get_theme
     * lv_display_get_inactive_time
     * lv_display_trigger_activity
     * lv_display_enable_invalidation
     * lv_display_is_invalidation_enabled
     * lv_display_get_refr_timer
     * lv_display_delete_refr_timer
     * lv_display_register_vsync_event
     * lv_display_unregister_vsync_event
     * lv_display_send_vsync_event
     * lv_display_set_user_data
     * lv_display_set_driver_data
     * lv_display_get_user_data
     * lv_display_get_driver_data
     * lv_display_get_buf_active
     * lv_display_rotate_area
     * lv_display_rotate_point
     * lv_display_get_draw_buf_size
     * lv_display_get_invalidated_draw_buf_size
     * lv_dpx
     * lv_display_dpx
     * lv_obj_delete_anim_completed_cb
     * lv_obj_get_screen
     * lv_obj_get_display
     * lv_obj_get_parent
     * lv_obj_get_child
     * lv_obj_get_child_by_type
     * lv_obj_get_sibling
     * lv_obj_get_sibling_by_type
     * lv_obj_get_child_count
     * lv_obj_get_child_count_by_type
     * lv_obj_get_name
     * lv_obj_get_name_resolved
     * lv_obj_find_by_name
     * lv_obj_get_child_by_name
     * lv_obj_get_index
     * lv_obj_get_index_by_type
     * lv_obj_tree_walk
     * lv_obj_refr_size
     * lv_obj_is_layout_positioned
     * lv_obj_set_transform
     * lv_obj_get_coords
     * lv_obj_get_x
     * lv_obj_get_x2
     * lv_obj_get_y
     * lv_obj_get_y2
     * lv_obj_get_x_aligned
     * lv_obj_get_y_aligned
     * lv_obj_get_width
     * lv_obj_get_height
     * lv_obj_get_content_width
     * lv_obj_get_content_height
     * lv_obj_get_content_coords
     * lv_obj_get_self_width
     * lv_obj_get_self_height
     * lv_obj_get_style_clamped_width
     * lv_obj_get_style_clamped_height
     * lv_obj_is_style_any_width_content
     * lv_obj_is_style_any_height_content
     * lv_obj_is_width_min
     * lv_obj_is_height_min
     * lv_obj_is_width_max
     * lv_obj_is_height_max
     * lv_obj_refresh_self_size
     * lv_obj_get_transform
     * lv_obj_transform_point
     * lv_obj_transform_point_array
     * lv_obj_get_transformed_area
     * lv_obj_invalidate_area
     * lv_obj_invalidate
     * lv_obj_area_is_visible
     * lv_obj_is_visible
     * lv_obj_get_click_area
     * lv_obj_hit_test
     * lv_clamp_width
     * lv_clamp_height
     * lv_obj_calc_dynamic_width
     * lv_obj_calc_dynamic_height
     * lv_obj_get_scrollbar_mode
     * lv_obj_get_scroll_dir
     * lv_obj_get_scroll_snap_x
     * lv_obj_get_scroll_snap_y
     * lv_obj_get_scroll_x
     * lv_obj_get_scroll_y
     * lv_obj_get_scroll_top
     * lv_obj_get_scroll_bottom
     * lv_obj_get_scroll_left
     * lv_obj_get_scroll_right
     * lv_obj_get_scroll_end
     * lv_obj_is_scrolling
     * lv_obj_get_scrollbar_area
     * lv_obj_add_style
     * lv_obj_replace_style
     * lv_obj_remove_style
     * lv_obj_report_style_change
     * lv_obj_style_set_disabled
     * lv_obj_style_get_disabled
     * lv_obj_get_style_prop
     * lv_obj_has_style_prop
     * lv_obj_set_local_style_prop
     * lv_obj_get_local_style_prop
     * lv_obj_remove_local_style_prop
     * lv_obj_style_apply_color_filter
     * lv_obj_set_style_bg_color
     * lv_obj_set_style_bg_grad_color
     * lv_obj_set_style_bg_grad
     * lv_obj_set_style_bg_image_recolor
     * lv_obj_set_style_border_color
     * lv_obj_set_style_outline_color
     * lv_obj_set_style_shadow_color
     * lv_obj_set_style_image_recolor
     * lv_obj_set_style_image_colorkey
     * lv_obj_set_style_line_color
     * lv_obj_set_style_arc_color
     * lv_obj_set_style_text_color
     * lv_obj_set_style_text_font
     * lv_obj_set_style_text_outline_stroke_color
     * lv_obj_set_style_drop_shadow_color
     * lv_obj_set_style_color_filter_dsc
     * lv_obj_set_style_recolor
     * lv_obj_set_style_anim
     * lv_obj_set_style_transition
     * lv_obj_calculate_style_text_align
     * lv_obj_get_style_opa_recursive
     * lv_obj_style_apply_recolor
     * lv_obj_get_style_recolor_recursive
     * lv_obj_bind_style
     * lv_obj_bind_style_prop
     * lv_draw_create_unit
     * lv_draw_add_task
     * lv_draw_finalize_task_creation
     * lv_draw_dispatch_layer
     * lv_draw_get_unit_count
     * lv_draw_get_available_task
     * lv_draw_get_next_available_task
     * lv_draw_get_dependent_count
     * lv_layer_init
     * lv_layer_reset
     * lv_draw_layer_create
     * lv_draw_layer_init
     * lv_draw_layer_alloc_buf
     * lv_draw_layer_go_to_xy
     * lv_draw_task_get_type
     * lv_draw_task_get_draw_dsc
     * lv_draw_task_get_area
     * lv_draw_layer_create_drop_shadow
     * lv_draw_layer_finish_drop_shadow
     * lv_draw_rect_dsc_init
     * lv_draw_fill_dsc_init
     * lv_draw_task_get_fill_dsc
     * lv_draw_fill
     * lv_draw_border_dsc_init
     * lv_draw_task_get_border_dsc
     * lv_draw_border
     * lv_draw_box_shadow_dsc_init
     * lv_draw_task_get_box_shadow_dsc
     * lv_draw_box_shadow
     * lv_draw_rect
     * lv_draw_letter_dsc_init
     * lv_draw_label_dsc_init
     * lv_draw_task_get_label_dsc
     * lv_draw_glyph_dsc_init
     * lv_draw_label
     * lv_draw_character
     * lv_draw_letter
     * lv_draw_label_iterate_characters
     * lv_draw_unit_draw_letter
     * lv_fs_drv_init
     * lv_fs_drv_register
     * lv_fs_get_drv
     * lv_fs_is_ready
     * lv_fs_open
     * lv_fs_make_path_from_buffer
     * lv_fs_get_buffer_from_path
     * lv_fs_close
     * lv_fs_read
     * lv_fs_write
     * lv_fs_seek
     * lv_fs_tell
     * lv_fs_get_size
     * lv_fs_path_get_size
     * lv_fs_load_to_buf
     * lv_fs_load_with_alloc
     * lv_fs_dir_open
     * lv_fs_dir_read
     * lv_fs_dir_close
     * lv_fs_get_letters
     * lv_fs_get_ext
     * lv_fs_up
     * lv_fs_get_last
     * lv_fs_path_join
     * lv_image_decoder_get_info
     * lv_image_decoder_open
     * lv_image_decoder_get_area
     * lv_image_decoder_close
     * lv_image_decoder_create
     * lv_image_decoder_delete
     * lv_image_decoder_get_next
     * lv_image_decoder_set_info_cb
     * lv_image_decoder_set_open_cb
     * lv_image_decoder_set_get_area_cb
     * lv_image_decoder_set_close_cb
     * lv_image_decoder_post_process
     * lv_draw_image_dsc_init
     * lv_draw_task_get_image_dsc
     * lv_draw_image
     * lv_draw_layer
     * lv_image_src_get_type
     * lv_draw_line_dsc_init
     * lv_draw_task_get_line_dsc
     * lv_draw_line
     * lv_draw_line_iterate
     * lv_draw_arc_dsc_init
     * lv_draw_task_get_arc_dsc
     * lv_draw_arc
     * lv_draw_arc_get_area
     * lv_draw_triangle_dsc_init
     * lv_draw_task_get_triangle_dsc
     * lv_draw_triangle
     * lv_draw_blur_dsc_init
     * lv_draw_task_get_blur_dsc
     * lv_draw_blur
     * lv_obj_init_draw_rect_dsc
     * lv_obj_init_draw_label_dsc
     * lv_obj_init_draw_image_dsc
     * lv_obj_init_draw_line_dsc
     * lv_obj_init_draw_arc_dsc
     * lv_obj_init_draw_blur_dsc
     * lv_obj_calculate_ext_draw_size
     * lv_obj_class_create_obj
     * lv_obj_is_editable
     * lv_obj_is_group_def
     * lv_indev_create
     * lv_indev_delete
     * lv_indev_get_next
     * lv_indev_read
     * lv_indev_read_timer_cb
     * lv_indev_enable
     * lv_indev_active
     * lv_indev_set_type
     * lv_indev_set_read_cb
     * lv_indev_set_user_data
     * lv_indev_set_driver_data
     * lv_indev_set_display
     * lv_indev_set_long_press_time
     * lv_indev_set_long_press_repeat_time
     * lv_indev_set_scroll_limit
     * lv_indev_set_scroll_throw
     * lv_indev_set_gesture_min_velocity
     * lv_indev_set_gesture_min_distance
     * lv_indev_get_type
     * lv_indev_get_read_cb
     * lv_indev_get_state
     * lv_indev_get_group
     * lv_indev_get_display
     * lv_indev_get_user_data
     * lv_indev_get_driver_data
     * lv_indev_get_press_moved
     * lv_indev_reset
     * lv_indev_stop_processing
     * lv_indev_reset_long_press
     * lv_indev_set_cursor
     * lv_indev_set_group
     * lv_indev_set_button_points
     * lv_indev_get_point
     * lv_indev_get_gesture_dir
     * lv_indev_get_key
     * lv_indev_get_short_click_streak
     * lv_indev_get_scroll_dir
     * lv_indev_get_scroll_obj
     * lv_indev_get_vect
     * lv_indev_get_cursor
     * lv_indev_wait_release
     * lv_indev_get_active_obj
     * lv_indev_get_read_timer
     * lv_indev_set_mode
     * lv_indev_get_mode
     * lv_indev_search_obj
     * lv_indev_add_event_cb
     * lv_indev_get_event_count
     * lv_indev_get_event_dsc
     * lv_indev_remove_event
     * lv_indev_remove_event_cb_with_user_data
     * lv_indev_send_event
     * lv_indev_set_key_remap_cb
     * lv_obj_send_event
     * lv_obj_event_base
     * lv_event_get_current_target_obj
     * lv_event_get_target_obj
     * lv_obj_add_event_cb
     * lv_obj_get_event_count
     * lv_obj_get_event_dsc
     * lv_obj_remove_event
     * lv_obj_remove_event_dsc
     * lv_obj_remove_event_cb
     * lv_obj_remove_event_cb_with_user_data
     * lv_event_get_indev
     * lv_event_get_layer
     * lv_event_get_old_size
     * lv_event_get_key
     * lv_event_get_rotary_diff
     * lv_event_get_scroll_anim
     * lv_event_set_ext_draw_size
     * lv_event_get_self_size_info
     * lv_event_get_hit_test_info
     * lv_event_get_cover_area
     * lv_event_set_cover_res
     * lv_event_get_draw_task
     * lv_event_get_prev_state
     * lv_obj_has_flag
     * lv_obj_has_flag_any
     * lv_obj_get_state
     * lv_obj_has_state
     * lv_obj_is_radio_button
     * lv_obj_get_group
     * lv_obj_get_user_data
     * lv_obj_check_type
     * lv_obj_has_class
     * lv_obj_get_class
     * lv_obj_is_valid
     * lv_obj_null_on_delete
     * lv_obj_add_delete_cb
     * lv_obj_remove_delete_cb
     * lv_obj_add_screen_create_event
     * lv_obj_add_play_timeline_event
     * lv_ll_init
     * lv_ll_ins_head
     * lv_ll_ins_prev
     * lv_ll_ins_tail
     * lv_ll_remove
     * lv_ll_clear_custom
     * lv_ll_clear
     * lv_ll_chg_list
     * lv_ll_get_head
     * lv_ll_get_tail
     * lv_ll_get_next
     * lv_ll_get_prev
     * lv_ll_get_len
     * lv_ll_move_before
     * lv_ll_is_empty
     * lv_subject_init_int
     * lv_subject_set_int
     * lv_subject_get_int
     * lv_subject_get_previous_int
     * lv_subject_set_min_value_int
     * lv_subject_set_max_value_int
     * lv_subject_init_float
     * lv_subject_set_float
     * lv_subject_get_float
     * lv_subject_get_previous_float
     * lv_subject_set_min_value_float
     * lv_subject_set_max_value_float
     * lv_subject_init_string
     * lv_subject_copy_string
     * lv_subject_snprintf
     * lv_subject_get_string
     * lv_subject_get_previous_string
     * lv_subject_init_pointer
     * lv_subject_set_pointer
     * lv_subject_get_pointer
     * lv_subject_get_previous_pointer
     * lv_subject_init_color
     * lv_subject_set_color
     * lv_subject_get_color
     * lv_subject_get_previous_color
     * lv_subject_init_group
     * lv_subject_deinit
     * lv_subject_get_group_element
     * lv_subject_add_observer
     * lv_subject_add_observer_obj
     * lv_subject_add_observer_with_target
     * lv_observer_remove
     * lv_obj_remove_from_subject
     * lv_observer_get_target
     * lv_observer_get_target_obj
     * lv_observer_get_user_data
     * lv_subject_notify
     * lv_obj_add_subject_increment_event
     * lv_obj_set_subject_increment_event_min_value
     * lv_obj_set_subject_increment_event_max_value
     * lv_obj_set_subject_increment_event_rollover
     * lv_obj_add_subject_toggle_event
     * lv_obj_add_subject_set_int_event
     * lv_obj_add_subject_set_float_event
     * lv_obj_add_subject_set_string_event
     * lv_obj_bind_flag_if_eq
     * lv_obj_bind_flag_if_not_eq
     * lv_obj_bind_flag_if_gt
     * lv_obj_bind_flag_if_ge
     * lv_obj_bind_flag_if_lt
     * lv_obj_bind_flag_if_le
     * lv_obj_bind_state_if_eq
     * lv_obj_bind_state_if_not_eq
     * lv_obj_bind_state_if_gt
     * lv_obj_bind_state_if_ge
     * lv_obj_bind_state_if_lt
     * lv_obj_bind_state_if_le
     * lv_obj_bind_checked
     * lv_refr_now
     * lv_obj_redraw
     * lv_display_refr_timer
     * lv_sysmon_create
     * lv_sysmon_show_performance
     * lv_sysmon_hide_performance
     * lv_sysmon_performance_dump
     * lv_sysmon_performance_resume
     * lv_sysmon_performance_pause
     * lv_draw_mask_rect_dsc_init
     * lv_draw_task_get_mask_rect_dsc
     * lv_draw_mask_rect
     * lv_matrix_transform_point
     * lv_matrix_transform_path
     * lv_vector_path_create
     * lv_vector_path_copy
     * lv_vector_path_clear
     * lv_vector_path_delete
     * lv_vector_path_move_to
     * lv_vector_path_line_to
     * lv_vector_path_quad_to
     * lv_vector_path_cubic_to
     * lv_vector_path_arc_to
     * lv_vector_path_close
     * lv_vector_path_get_bounding
     * lv_vector_path_append_rectangle
     * lv_vector_path_append_circle
     * lv_vector_path_append_arc
     * lv_vector_path_append_path
     * lv_draw_vector_dsc_create
     * lv_draw_vector_dsc_delete
     * lv_draw_vector_dsc_set_transform
     * lv_draw_vector_dsc_set_blend_mode
     * lv_draw_vector_dsc_set_fill_color32
     * lv_draw_vector_dsc_set_fill_color
     * lv_draw_vector_dsc_set_fill_opa
     * lv_draw_vector_dsc_set_fill_rule
     * lv_draw_vector_dsc_set_fill_units
     * lv_draw_vector_dsc_set_fill_image
     * lv_draw_vector_dsc_set_fill_linear_gradient
     * lv_draw_vector_dsc_set_fill_radial_gradient
     * lv_draw_vector_dsc_set_fill_gradient_spread
     * lv_draw_vector_dsc_set_fill_gradient_color_stops
     * lv_draw_vector_dsc_set_fill_transform
     * lv_draw_vector_dsc_set_stroke_color32
     * lv_draw_vector_dsc_set_stroke_color
     * lv_draw_vector_dsc_set_stroke_opa
     * lv_draw_vector_dsc_set_stroke_width
     * lv_draw_vector_dsc_set_stroke_dash
     * lv_draw_vector_dsc_set_stroke_cap
     * lv_draw_vector_dsc_set_stroke_join
     * lv_draw_vector_dsc_set_stroke_miter_limit
     * lv_draw_vector_dsc_set_stroke_linear_gradient
     * lv_draw_vector_dsc_set_stroke_radial_gradient
     * lv_draw_vector_dsc_set_stroke_gradient_spread
     * lv_draw_vector_dsc_set_stroke_gradient_color_stops
     * lv_draw_vector_dsc_set_stroke_transform
     * lv_draw_vector_dsc_identity
     * lv_draw_vector_dsc_scale
     * lv_draw_vector_dsc_rotate
     * lv_draw_vector_dsc_translate
     * lv_draw_vector_dsc_skew
     * lv_draw_vector_dsc_add_path
     * lv_draw_vector_dsc_clear_area
     * lv_draw_vector
     * lv_draw_task_get_vector_dsc
     * lv_linux_drm_create
     * lv_linux_drm_set_file
     * lv_linux_drm_find_device_path
     * lv_linux_drm_set_mode_cb
     * lv_linux_drm_mode_get_horizontal_resolution
     * lv_linux_drm_mode_get_vertical_resolution
     * lv_linux_drm_mode_get_refresh_rate
     * lv_linux_drm_mode_is_preferred
     * lv_linux_drm_mode_get_raw
     * lv_evdev_create
     * lv_evdev_create_fd
     * lv_evdev_discovery_start
     * lv_evdev_discovery_stop
     * lv_evdev_set_swap_axes
     * lv_evdev_set_calibration
     * lv_evdev_is_raw_key
     * lv_evdev_get_raw_key
     * lv_evdev_delete
     * lv_binfont_create
     * lv_binfont_destroy
     * lv_font_get_bitmap_fmt_txt
     * lv_font_get_glyph_dsc_fmt_txt
     * lv_imgfont_create
     * lv_imgfont_destroy
     * lv_tiny_ttf_create_data
     * lv_tiny_ttf_create_data_ex
     * lv_tiny_ttf_set_size
     * lv_tiny_ttf_destroy
     * lv_bin_decoder_info
     * lv_bin_decoder_get_area
     * lv_bin_decoder_open
     * lv_bin_decoder_close
     * lv_async_call
     * lv_async_call_cancel
     * lv_tree_node_create
     * lv_tree_node_delete
     * lv_tree_walk
     * lv_lock_isr
     * lv_theme_create
     * lv_theme_copy
     * lv_theme_get_from_obj
     * lv_theme_set_parent
     * lv_theme_set_apply_cb
     * lv_theme_get_font_small
     * lv_theme_get_font_normal
     * lv_theme_get_font_large
     * lv_theme_get_color_primary
     * lv_theme_get_color_secondary
     * lv_theme_delete
     * lv_theme_default_init
     * lv_theme_default_is_inited
     * lv_theme_default_get
     * lv_theme_mono_init
     * lv_theme_mono_is_inited
     * lv_theme_mono_get
     * lv_theme_simple_init
     * lv_theme_simple_is_inited
     * lv_theme_simple_get
     * lv_tick_get
     * lv_tick_elaps
     * lv_tick_diff
     * lv_delay_set_cb
     * lv_tick_set_cb
     * lv_tick_get_cb
     * lv_image_set_bitmap_map_src
     * lv_image_get_src
     * lv_image_get_offset_x
     * lv_image_get_offset_y
     * lv_image_get_rotation
     * lv_image_get_pivot
     * lv_image_get_scale
     * lv_image_get_scale_x
     * lv_image_get_scale_y
     * lv_image_get_src_width
     * lv_image_get_src_height
     * lv_image_get_transformed_width
     * lv_image_get_transformed_height
     * lv_image_get_blend_mode
     * lv_image_get_antialias
     * lv_image_get_inner_align
     * lv_image_get_bitmap_map_src
     * lv_image_bind_src
     * lv_animimg_set_src
     * lv_animimg_set_src_reverse
     * lv_animimg_delete
     * lv_animimg_set_start_cb
     * lv_animimg_set_completed_cb
     * lv_animimg_get_src
     * lv_animimg_get_src_count
     * lv_animimg_get_duration
     * lv_animimg_get_repeat_count
     * lv_animimg_get_anim
     * lv_arc_get_angle_start
     * lv_arc_get_angle_end
     * lv_arc_get_bg_angle_start
     * lv_arc_get_bg_angle_end
     * lv_arc_get_value
     * lv_arc_get_min_value
     * lv_arc_get_max_value
     * lv_arc_get_mode
     * lv_arc_get_rotation
     * lv_arc_get_knob_offset
     * lv_arc_get_change_rate
     * lv_arc_bind_value
     * lv_arclabel_set_text_fmt
     * lv_arclabel_get_angle_start
     * lv_arclabel_get_angle_size
     * lv_arclabel_get_dir
     * lv_arclabel_get_recolor
     * lv_arclabel_get_radius
     * lv_arclabel_get_center_offset_x
     * lv_arclabel_get_center_offset_y
     * lv_arclabel_get_text_vertical_align
     * lv_arclabel_get_text_horizontal_align
     * lv_arclabel_get_overflow
     * lv_arclabel_get_end_overlap
     * lv_arclabel_get_text_angle
     * lv_label_set_text_fmt
     * lv_label_set_text_vfmt
     * lv_label_get_text
     * lv_label_get_long_mode
     * lv_label_get_max_lines
     * lv_label_get_letter_pos
     * lv_label_get_letter_on
     * lv_label_is_char_under_pos
     * lv_label_get_text_selection_start
     * lv_label_get_text_selection_end
     * lv_label_get_recolor
     * lv_label_bind_text
     * lv_bar_get_value
     * lv_bar_get_start_value
     * lv_bar_get_min_value
     * lv_bar_get_max_value
     * lv_bar_get_mode
     * lv_bar_get_orientation
     * lv_bar_is_symmetrical
     * lv_bar_bind_value
     * lv_canvas_set_draw_buf
     * lv_canvas_set_px
     * lv_canvas_set_palette
     * lv_canvas_get_draw_buf
     * lv_canvas_get_px
     * lv_canvas_get_image
     * lv_canvas_get_buf
     * lv_canvas_copy_buf
     * lv_canvas_fill_bg
     * lv_canvas_init_layer
     * lv_canvas_finish_layer
     * lv_canvas_buf_size
     * lv_barcode_set_dark_color
     * lv_barcode_set_light_color
     * lv_barcode_update
     * lv_barcode_get_dark_color
     * lv_barcode_get_light_color
     * lv_barcode_get_scale
     * lv_barcode_get_encoding
     * lv_buttonmatrix_set_map
     * lv_buttonmatrix_set_ctrl_map
     * lv_buttonmatrix_get_map
     * lv_buttonmatrix_get_selected_button
     * lv_buttonmatrix_get_button_text
     * lv_buttonmatrix_has_button_ctrl
     * lv_buttonmatrix_get_one_checked
     * lv_calendar_set_highlighted_dates
     * lv_calendar_set_day_names
     * lv_calendar_get_btnmatrix
     * lv_calendar_get_today_date
     * lv_calendar_get_showed_date
     * lv_calendar_get_highlighted_dates
     * lv_calendar_get_highlighted_dates_num
     * lv_calendar_get_pressed_date
     * lv_calendar_add_header_arrow
     * lv_calendar_add_header_dropdown
     * lv_chart_get_type
     * lv_chart_get_point_count
     * lv_chart_get_update_mode
     * lv_chart_get_hor_div_line_count
     * lv_chart_get_ver_div_line_count
     * lv_chart_get_x_start_point
     * lv_chart_get_point_pos_by_id
     * lv_chart_add_series
     * lv_chart_remove_series
     * lv_chart_hide_series
     * lv_chart_set_series_color
     * lv_chart_get_series_color
     * lv_chart_set_x_start_point
     * lv_chart_get_series_next
     * lv_chart_add_cursor
     * lv_chart_remove_cursor
     * lv_chart_set_cursor_pos
     * lv_chart_set_cursor_pos_x
     * lv_chart_set_cursor_pos_y
     * lv_chart_set_cursor_point
     * lv_chart_get_cursor_point
     * lv_chart_set_all_values
     * lv_chart_set_next_value
     * lv_chart_set_next_value2
     * lv_chart_set_series_values
     * lv_chart_set_series_values2
     * lv_chart_set_series_value_by_id
     * lv_chart_set_series_value_by_id2
     * lv_chart_set_series_ext_y_array
     * lv_chart_set_series_ext_x_array
     * lv_chart_get_series_y_array
     * lv_chart_get_series_x_array
     * lv_chart_get_pressed_point
     * lv_chart_get_first_point_center_offset
     * lv_checkbox_get_text
     * lv_dropdown_get_list
     * lv_dropdown_get_text
     * lv_dropdown_get_options
     * lv_dropdown_get_selected
     * lv_dropdown_get_option_count
     * lv_dropdown_get_option_index
     * lv_dropdown_get_symbol
     * lv_dropdown_get_selected_highlight
     * lv_dropdown_get_dir
     * lv_dropdown_is_open
     * lv_dropdown_bind_value
     * lv_imagebutton_get_src_left
     * lv_imagebutton_get_src_middle
     * lv_imagebutton_get_src_right
     * lv_keyboard_set_map
     * lv_keyboard_get_textarea
     * lv_keyboard_get_mode
     * lv_keyboard_get_popovers
     * lv_keyboard_get_map_array
     * lv_keyboard_get_selected_button
     * lv_keyboard_get_button_text
     * lv_keyboard_def_event_cb
     * lv_led_set_color
     * lv_led_get_brightness
     * lv_led_get_color
     * lv_line_set_points
     * lv_line_set_points_mutable
     * lv_line_get_points
     * lv_line_get_point_count
     * lv_line_is_point_array_mutable
     * lv_line_get_points_mutable
     * lv_line_get_y_invert
     * lv_list_add_text
     * lv_list_add_button
     * lv_list_get_button_text
     * lv_lottie_set_draw_buf
     * lv_lottie_get_anim
     * lv_menu_get_cur_main_page
     * lv_menu_get_cur_sidebar_page
     * lv_menu_get_main_header
     * lv_menu_get_main_header_back_button
     * lv_menu_get_sidebar_header
     * lv_menu_get_sidebar_header_back_button
     * lv_menu_back_button_is_root
     * lv_menu_get_mode_header
     * lv_menu_get_mode_root_back_button
     * lv_msgbox_add_title
     * lv_msgbox_add_header_button
     * lv_msgbox_add_text
     * lv_msgbox_add_text_fmt
     * lv_msgbox_add_footer_button
     * lv_msgbox_add_close_button
     * lv_msgbox_get_header
     * lv_msgbox_get_footer
     * lv_msgbox_get_content
     * lv_msgbox_get_title
     * lv_qrcode_set_dark_color
     * lv_qrcode_set_light_color
     * lv_qrcode_update
     * lv_roller_set_selected_str
     * lv_roller_get_selected
     * lv_roller_get_options
     * lv_roller_get_option_count
     * lv_roller_get_option_str
     * lv_roller_bind_value
     * lv_scale_set_text_src
     * lv_scale_add_section
     * lv_scale_section_set_range
     * lv_scale_set_section_range
     * lv_scale_set_section_min_value
     * lv_scale_set_section_max_value
     * lv_scale_section_set_style
     * lv_scale_set_section_style_main
     * lv_scale_set_section_style_indicator
     * lv_scale_set_section_style_items
     * lv_scale_get_mode
     * lv_scale_get_total_tick_count
     * lv_scale_get_major_tick_every
     * lv_scale_get_rotation
     * lv_scale_get_label_show
     * lv_scale_get_angle_range
     * lv_scale_get_range_min_value
     * lv_scale_get_range_max_value
     * lv_scale_bind_section_min_value
     * lv_scale_bind_section_max_value
     * lv_scale_bind_line_needle_value
     * lv_scale_bind_image_needle_value
     * lv_slider_get_value
     * lv_slider_get_left_value
     * lv_slider_get_min_value
     * lv_slider_get_max_value
     * lv_slider_is_dragged
     * lv_slider_get_mode
     * lv_slider_get_orientation
     * lv_slider_is_symmetrical
     * lv_slider_bind_value
     * lv_spangroup_add_span
     * lv_spangroup_delete_span
     * lv_span_set_text
     * lv_span_set_text_fmt
     * lv_span_set_text_static
     * lv_spangroup_set_span_text
     * lv_spangroup_set_span_text_static
     * lv_spangroup_set_span_text_fmt
     * lv_span_set_text_static
     * lv_spangroup_set_span_style
     * lv_span_get_style
     * lv_span_get_text
     * lv_spangroup_get_child
     * lv_spangroup_get_span_count
     * lv_spangroup_get_align
     * lv_spangroup_get_overflow
     * lv_spangroup_get_indent
     * lv_spangroup_get_mode
     * lv_spangroup_get_max_lines
     * lv_spangroup_get_max_line_height
     * lv_spangroup_get_expand_width
     * lv_spangroup_get_expand_height
     * lv_spangroup_get_span_coords
     * lv_spangroup_get_span_by_point
     * lv_spangroup_bind_span_text
     * lv_textarea_get_text
     * lv_textarea_get_placeholder_text
     * lv_textarea_get_label
     * lv_textarea_get_cursor_pos
     * lv_textarea_get_cursor_click_pos
     * lv_textarea_get_password_mode
     * lv_textarea_get_password_bullet
     * lv_textarea_get_one_line
     * lv_textarea_get_accepted_chars
     * lv_textarea_get_max_length
     * lv_textarea_text_is_selected
     * lv_textarea_get_text_selection
     * lv_textarea_get_password_show_time
     * lv_textarea_get_current_char
     * lv_spinbox_get_rollover
     * lv_spinbox_get_value
     * lv_spinbox_get_step
     * lv_spinbox_get_digit_count
     * lv_spinbox_get_dec_point_pos
     * lv_spinbox_get_min_value
     * lv_spinbox_get_max_value
     * lv_spinbox_get_digit_step_direction
     * lv_spinbox_bind_value
     * lv_spinner_get_anim_duration
     * lv_spinner_get_arc_sweep
     * lv_switch_get_orientation
     * lv_table_set_cell_value_fmt
     * lv_table_get_cell_value
     * lv_table_get_row_count
     * lv_table_get_column_count
     * lv_table_get_column_width
     * lv_table_has_cell_ctrl
     * lv_table_get_cell_user_data
     * lv_tabview_add_tab
     * lv_tabview_get_tab_count
     * lv_tabview_get_tab_active
     * lv_tabview_get_tab_button
     * lv_tabview_get_content
     * lv_tabview_get_tab_bar
     * lv_tabview_get_tab_bar_position
     * lv_tileview_add_tile
     * lv_tileview_get_tile_active
     * lv_win_add_title
     * lv_win_add_button
     * lv_win_get_header
     * lv_win_get_content
     */

    /* Driver backend functions (from lv_port_linux, not auto-generated) */
    m.def("driver_backends_register", []() { driver_backends_register(); },
        "Register all available driver backends (must be called first)");
    m.def("driver_backends_init_backend", [](const std::string &name) -> int {
        return driver_backends_init_backend(const_cast<char*>(name.c_str()));
    }, py::arg("backend_name"),
        "Initialize a driver backend by name (e.g. 'DRM', 'FBDEV', 'EVDEV')");
    m.def("driver_backends_is_supported", [](const std::string &name) -> int {
        return driver_backends_is_supported(const_cast<char*>(name.c_str()));
    }, py::arg("backend_name"),
        "Check if a backend is supported, returns 1 or 0");
    m.def("driver_backends_print_supported", []() -> int {
        return driver_backends_print_supported();
    }, "Print supported backends to stdout");
    m.def("driver_backends_run_loop", []() { driver_backends_run_loop(); },
        "Enter the run loop of the selected backend");

}  // PYBIND11_MODULE