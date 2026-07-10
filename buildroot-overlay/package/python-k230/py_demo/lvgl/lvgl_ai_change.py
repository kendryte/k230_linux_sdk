#!/usr/bin/env python3
"""
K230 AI Demo Launcher - LVGL 界面

点击按钮切换不同的 AI Demo。
自适应屏幕分辨率 (800x480 / 1024x600 / 1280x720 等)。

Usage:
    python3 lvgl_ai_change.py
"""

import os
import sys
import time
import subprocess
import lvgl as lv

# Short aliases for enum values (generated bindings use LV_ prefixed names)
ALIGN = lv.ALIGN
EVENT = lv.EVENT_CODE
SCROLLBAR_MODE = lv.SCROLLBAR_MODE
OBJ_FLAG = lv.OBJ_FLAG
STATE = lv.STATE

# ============================================================================
# AI Demo 配置
# ============================================================================
AI_DEMOS = [
    {
        "name": "Face Detect",
        "desc": "Face Detection",
        "script": "/root/py_demo/ai/face_detect.py",
        "args": "/root/py_demo/ai/face_detection_320.kmodel 0.5 0.2",
        "color": (0x00, 0x96, 0x88),   # teal
    },
    {
        "name": "Face Pose",
        "desc": "Face Pose Estimation",
        "script": "",
        "args": "",
        "color": (0x1E, 0x88, 0xE5),   # blue
    },
    {
        "name": "License Plate",
        "desc": "License Plate Detection",
        "script": "",
        "args": "",
        "color": (0x43, 0xA0, 0x47),   # green
    },
    {
        "name": "Object Detect",
        "desc": "Object Detection",
        "script": "",
        "args": "",
        "color": (0xFB, 0x8C, 0x00),   # orange
    },
    {
        "name": "Helmet Detect",
        "desc": "Helmet Detection",
        "script": "",
        "args": "",
        "color": (0xE5, 0x39, 0x35),   # red
    },
    {
        "name": "Person Detect",
        "desc": "Person Detection",
        "script": "",
        "args": "",
        "color": (0x8E, 0x24, 0xAA),   # purple
    },
]

# ============================================================================
# 全局状态
# ============================================================================
selected_idx = [0]       # 当前选中的 demo 索引
demo_buttons = []        # 按钮对象列表
status_label = None      # 底部状态标签
btn_launch = None        # 启动按钮

# ============================================================================
# 屏幕尺寸 (在 main() 中初始化)
# ============================================================================
SCR_W = 1280
SCR_H = 720


def c(r, g, b):
    """创建 LVGL 颜色 (RGB888)"""
    return lv.color_make(r, g, b)


# 常用颜色
C_WHITE = c(0xFF, 0xFF, 0xFF)
C_DARK_BG = c(0x18, 0x18, 0x18)
C_TITLE_BG = c(0x1A, 0x23, 0x7E)
C_LIGHT_BLUE = c(0x90, 0xCA, 0xF9)
C_CARD_BG = c(0x2D, 0x2D, 0x2D)
C_CARD_BG_SEL = c(0x3D, 0x3D, 0x3D)
C_BORDER = c(0x42, 0x42, 0x42)
C_GREY = c(0x75, 0x75, 0x75)
C_LIGHT_GREY = c(0x9E, 0x9E, 0x9E)
C_GREEN = c(0x4C, 0xAF, 0x50)
C_RED = c(0xEF, 0x53, 0x50)
C_BAR_BG = c(0x1A, 0x1A, 0x1A)
C_BTN_START = c(0x00, 0x96, 0x88)
C_BTN_EXIT = c(0x42, 0x42, 0x42)
C_BLACK = c(0x00, 0x00, 0x00)


def create_title_bar(parent):
    """创建顶部标题栏"""
    bar_h = max(40, int(SCR_H * 0.075))

    title_cont = lv.Obj.create(parent)
    title_cont.set_size(SCR_W, bar_h)
    title_cont.align(ALIGN.LV_ALIGN_TOP_MID, 0, 0)
    title_cont.set_style_bg_color(C_TITLE_BG, 0)
    title_cont.set_style_bg_opa(255, 0)
    title_cont.set_style_border_width(0, 0)
    title_cont.set_style_radius(0, 0)
    title_cont.set_scrollbar_mode(SCROLLBAR_MODE.LV_SCROLLBAR_MODE_OFF)

    title = lv.Label(title_cont)
    title.label_set_text("K230 AI Demo Launcher")
    title.set_style_text_color(C_WHITE, 0)
    title.align(ALIGN.LV_ALIGN_CENTER, 0, 0)

    # 小屏隐藏副标题
    if SCR_W >= 1024:
        subtitle = lv.Label(title_cont)
        subtitle.label_set_text("CanMV K230 | Select a demo and press Start")
        subtitle.set_style_text_color(C_LIGHT_BLUE, 0)
        subtitle.align(ALIGN.LV_ALIGN_RIGHT_MID, -20, 0)


def create_demo_button(parent, demo, idx, col, row):
    """创建单个 AI Demo 按钮 (卡片样式，自适应屏幕)"""
    COLS = 3
    margin = max(8, int(SCR_W * 0.02))
    gap = max(6, int(SCR_W * 0.015))

    title_h = max(40, int(SCR_H * 0.075))
    bottom_h = max(40, int(SCR_H * 0.09))

    # 卡片尺寸：填满可用空间
    card_w = (SCR_W - margin * 2 - gap * (COLS - 1)) // COLS
    card_h = (SCR_H - title_h - bottom_h - margin * 2 - gap) // 2

    x = margin + col * (card_w + gap)
    y = title_h + margin + row * (card_h + gap)

    # 卡片容器
    card = lv.Obj.create(parent)
    card.set_size(card_w, card_h)
    card.set_pos(x, y)
    card.set_style_bg_color(C_CARD_BG, 0)
    card.set_style_bg_opa(255, 0)
    card.set_style_radius(max(6, int(min(SCR_W, SCR_H) * 0.01)), 0)
    card.set_style_border_width(2, 0)
    card.set_style_border_color(C_BORDER, 0)
    card.set_style_shadow_width(max(4, int(SCR_H * 0.01)), 0)
    card.set_style_shadow_color(C_BLACK, 0)
    card.set_scrollbar_mode(SCROLLBAR_MODE.LV_SCROLLBAR_MODE_OFF)
    card.add_flag(OBJ_FLAG.LV_OBJ_FLAG_CLICKABLE)
    card.remove_flag(OBJ_FLAG.LV_OBJ_FLAG_SCROLLABLE)

    # 左侧色条
    bar_w = max(3, int(SCR_W * 0.004))
    color_bar = lv.Obj.create(card)
    color_bar.set_size(bar_w, card_h - int(card_h * 0.15))
    color_bar.align(ALIGN.LV_ALIGN_LEFT_MID, max(5, int(SCR_W * 0.006)), 0)
    color_bar.set_style_bg_color(c(*demo["color"]), 0)
    color_bar.set_style_bg_opa(255, 0)
    color_bar.set_style_radius(max(1, bar_w // 2), 0)
    color_bar.set_style_border_width(0, 0)
    color_bar.set_scrollbar_mode(SCROLLBAR_MODE.LV_SCROLLBAR_MODE_OFF)

    # Demo 名称
    name_label = lv.Label(card)
    name_label.label_set_text(demo["name"])
    name_label.set_style_text_color(C_WHITE, 0)
    name_label.align(ALIGN.LV_ALIGN_TOP_LEFT, int(card_w * 0.08), int(card_h * 0.15))

    # Demo 英文描述 (小屏隐藏)
    if SCR_W >= 800:
        desc_label = lv.Label(card)
        desc_label.label_set_text(demo["desc"])
        desc_label.set_style_text_color(C_LIGHT_GREY, 0)
        desc_label.align(ALIGN.LV_ALIGN_TOP_LEFT, int(card_w * 0.08), int(card_h * 0.4))

    # 状态标签 (可用/未实现)
    status = lv.Label(card)
    if demo["script"]:
        status.label_set_text("READY")
        status.set_style_text_color(C_GREEN, 0)
    else:
        status.label_set_text("TODO")
        status.set_style_text_color(C_GREY, 0)
    status.align(ALIGN.LV_ALIGN_BOTTOM_LEFT, int(card_w * 0.08), -int(card_h * 0.1))

    # 右侧箭头指示
    arrow = lv.Label(card)
    arrow.label_set_text(">")
    arrow.set_style_text_color(C_GREY, 0)
    arrow.align(ALIGN.LV_ALIGN_RIGHT_MID, -int(card_w * 0.04), 0)

    # 点击事件
    def on_card_click(event_code):
        if event_code == int(EVENT.LV_EVENT_CLICKED):
            select_demo(idx)

    card.add_event_cb(int(EVENT.LV_EVENT_CLICKED), on_card_click)

    demo_buttons.append(card)
    return card


def select_demo(idx):
    """选中某个 demo，更新 UI 状态"""
    selected_idx[0] = idx
    demo = AI_DEMOS[idx]

    # 更新所有按钮的选中/未选中样式
    for i, btn in enumerate(demo_buttons):
        if i == idx:
            btn.set_style_border_color(c(*demo["color"]), 0)
            btn.set_style_border_width(3, 0)
            btn.set_style_bg_color(C_CARD_BG_SEL, 0)
        else:
            btn.set_style_border_color(C_BORDER, 0)
            btn.set_style_border_width(2, 0)
            btn.set_style_bg_color(C_CARD_BG, 0)

    # 更新底部状态栏
    if demo["script"]:
        status_label.label_set_text("Selected: %s  (%s)" % (demo["name"], demo["desc"]))
        status_label.set_style_text_color(C_WHITE, 0)
        btn_launch.remove_state(STATE.LV_STATE_DISABLED)
    else:
        status_label.label_set_text("Selected: %s  (Not implemented yet)" % demo["name"])
        status_label.set_style_text_color(C_RED, 0)
        btn_launch.add_state(STATE.LV_STATE_DISABLED)


def create_bottom_bar(parent):
    """创建底部操作栏"""
    global status_label, btn_launch

    bar_h = max(40, int(SCR_H * 0.09))
    btn_w = max(80, int(SCR_W * 0.1))
    btn_h = max(30, int(SCR_H * 0.06))
    btn_gap = max(10, int(SCR_W * 0.015))

    bar = lv.Obj.create(parent)
    bar.set_size(SCR_W, bar_h)
    bar.align(ALIGN.LV_ALIGN_BOTTOM_MID, 0, 0)
    bar.set_style_bg_color(C_BAR_BG, 0)
    bar.set_style_bg_opa(255, 0)
    bar.set_style_border_width(0, 0)
    bar.set_style_radius(0, 0)
    bar.set_scrollbar_mode(SCROLLBAR_MODE.LV_SCROLLBAR_MODE_OFF)

    # 状态标签
    status_label = lv.Label(bar)
    status_label.label_set_text("Selected: %s" % AI_DEMOS[0]["name"])
    status_label.set_style_text_color(C_WHITE, 0)
    status_label.align(ALIGN.LV_ALIGN_LEFT_MID, max(10, int(SCR_W * 0.02)), 0)

    # 启动按钮
    btn_launch = lv.Button(bar)
    btn_launch.set_size(btn_w, btn_h)
    btn_launch.align(ALIGN.LV_ALIGN_RIGHT_MID, -(btn_w + btn_gap * 2), 0)
    btn_launch.set_style_bg_color(C_BTN_START, 0)
    btn_launch.set_style_bg_opa(255, 0)
    btn_launch.set_style_radius(max(4, int(btn_h * 0.18)), 0)
    btn_launch.set_style_text_color(C_WHITE, 0)

    lbl_launch = lv.Label(btn_launch)
    lbl_launch.label_set_text("Start")
    lbl_launch.center()

    def on_launch(event_code):
        if event_code == int(EVENT.LV_EVENT_CLICKED):
            launch_demo()

    btn_launch.add_event_cb(int(EVENT.LV_EVENT_CLICKED), on_launch)

    # 退出按钮
    btn_exit = lv.Button(bar)
    btn_exit.set_size(btn_w, btn_h)
    btn_exit.align(ALIGN.LV_ALIGN_RIGHT_MID, -btn_gap, 0)
    btn_exit.set_style_bg_color(C_BTN_EXIT, 0)
    btn_exit.set_style_bg_opa(255, 0)
    btn_exit.set_style_radius(max(4, int(btn_h * 0.18)), 0)
    btn_exit.set_style_text_color(C_WHITE, 0)

    lbl_exit = lv.Label(btn_exit)
    lbl_exit.label_set_text("Exit")
    lbl_exit.center()

    def on_exit(event_code):
        if event_code == int(EVENT.LV_EVENT_CLICKED):
            print("Exit pressed, quitting...")
            os._exit(0)

    btn_exit.add_event_cb(int(EVENT.LV_EVENT_CLICKED), on_exit)


def launch_demo():
    """启动选中的 AI Demo"""
    idx = selected_idx[0]
    demo = AI_DEMOS[idx]
    script = demo["script"]

    if not script:
        print("Demo '%s' is not implemented yet" % demo["name"])
        return

    if not os.path.exists(script):
        print("Script not found: %s" % script)
        status_label.label_set_text("Error: script not found!")
        status_label.set_style_text_color(C_RED, 0)
        return

    # 组装启动命令
    cmd = [sys.executable, script]
    if demo["args"]:
        cmd += demo["args"].split()

    print("Launching: %s (%s)" % (demo["name"], " ".join(cmd)))

    # 释放 LVGL 显示资源，避免 DRM 冲突
    lv.deinit()

    # 后台启动 AI demo 脚本
    subprocess.Popen(cmd)
    print("AI demo started, exiting launcher...")
    os._exit(0)


def main():
    global SCR_W, SCR_H

    # ---- LVGL 初始化 ----
    lv.k230_init()

    # ---- 获取屏幕分辨率 ----
    disp = lv.display_get_default()
    SCR_W = disp.get_horizontal_resolution()
    SCR_H = disp.get_vertical_resolution()
    print("Screen resolution: %dx%d" % (SCR_W, SCR_H))

    scr = lv.screen_active()
    scr.set_style_bg_color(C_DARK_BG, 0)
    scr.set_style_bg_opa(255, 0)

    # ---- 顶部标题栏 ----
    create_title_bar(scr)

    # ---- AI Demo 按钮网格 (2行 x 3列) ----
    for idx, demo in enumerate(AI_DEMOS):
        col = idx % 3
        row = idx // 3
        create_demo_button(scr, demo, idx, col, row)

    # ---- 底部操作栏 ----
    create_bottom_bar(scr)

    # ---- 默认选中第一个 ----
    select_demo(0)

    # ---- 主循环 ----
    print("AI Demo Launcher running... Press Ctrl+C to exit")
    try:
        while True:
            idle_time = lv.timer_handler()
            time.sleep(idle_time / 1000.0)
    except KeyboardInterrupt:
        print("\nExiting...")
        os._exit(0)


if __name__ == "__main__":
    main()
