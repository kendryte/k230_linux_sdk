#!/usr/bin/env python3
"""Label demo - wrap, scroll, dots modes"""
import os, time
import lvgl as lv

lv.k230_init()
scr = lv.screen_active()

# 换行模式
label1 = lv.Label(scr)
label1.label_set_text("Label with\nline wrap")
label1.set_width(120)
label1.align(lv.ALIGN.LV_ALIGN_TOP_MID, 0, 20)

# 滚动模式
label2 = lv.Label(scr)
label2.label_set_text("This is a long scrolling label text")
label2.set_long_mode(lv.LABEL_LONG_MODE.LV_LABEL_LONG_MODE_SCROLL_CIRCULAR)
label2.set_width(150)
label2.align(lv.ALIGN.LV_ALIGN_CENTER, 0, 0)

# 省略号模式
label3 = lv.Label(scr)
label3.label_set_text("This text is too long and will be truncated with dots")
label3.set_long_mode(lv.LABEL_LONG_MODE.LV_LABEL_LONG_MODE_DOTS)
label3.set_width(150)
label3.align(lv.ALIGN.LV_ALIGN_BOTTOM_MID, 0, -20)

try:
    while True:
        idle = lv.timer_handler()
        time.sleep(idle / 1000.0)
except KeyboardInterrupt:
    os._exit(0)
