#!/usr/bin/env python3
"""基础控件演示 — 按钮/复选框/滑块/进度条/弧形/开关/LED"""

import lvgl as lv

lv.init()
scr = lv.screen_active()

# 标题
title = lv.Label(scr)
title.set_text("K230 LVGL Demo")
title.align(lv.ALIGN.TOP_MID, 0, 10)

# 按钮 + 点击计数
btn = lv.Button(scr)
btn.set_size(100, 40)
btn.align(lv.ALIGN.TOP_LEFT, 15, 40)
btn_label = lv.Label(btn)
btn_label.set_text("Button")
btn_label.center()

count_label = lv.Label(scr)
count_label.set_text("Click: 0")
count_label.align_to(btn, lv.ALIGN.OUT_RIGHT_MID, 10, 0)

clicks = [0]
def on_click(event_code):
    if event_code == lv.EVENT.CLICKED:
        clicks[0] += 1
        count_label.set_text("Click: %d" % clicks[0])
btn.add_event_cb(lv.EVENT.CLICKED, on_click)

# 复选框
cb = lv.Checkbox(scr)
cb.set_text("Enable")
cb.align(lv.ALIGN.TOP_LEFT, 15, 90)

# 滑块
slider = lv.Slider(scr)
slider.set_range(0, 100)
slider.set_value(30, False)
slider.set_width(160)
slider.align(lv.ALIGN.TOP_LEFT, 15, 140)

# 进度条
bar = lv.Bar(scr)
bar.set_range(0, 100)
bar.set_value(60, False)
bar.set_size(160, 12)
bar.align(lv.ALIGN.TOP_LEFT, 15, 190)

# 弧形指示器
arc = lv.Arc(scr)
arc.set_range(0, 100)
arc.set_value(70)
arc.set_angles(135, 45)    # 前景弧: 135° → 45°
arc.set_size(80, 80)
arc.align(lv.ALIGN.TOP_RIGHT, -100, 50)

# 开关
sw = lv.Switch(scr)
sw.set_size(50, 28)
sw.align(lv.ALIGN.TOP_RIGHT, -100, 150)

# LED 指示灯
led = lv.Led(scr)
led.set_size(30, 30)
led.set_brightness(255)
led.set_color(lv.color(0, 255, 0))   # 绿色
led.align(lv.ALIGN.BOTTOM_RIGHT, -100, -60)

lv.run()
