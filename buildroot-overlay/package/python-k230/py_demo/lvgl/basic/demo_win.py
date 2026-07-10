#!/usr/bin/env python3
"""Win demo - window with title and content"""
import os, time
import lvgl as lv

lv.k230_init()
scr = lv.screen_active()

win = lv.Win(scr)
win.set_size(260, 200)
win.center()

win.win_add_title("My Window")

content = win.win_get_content()
label = lv.Label(content)
label.label_set_text("Window content area")
label.center()

try:
    while True:
        idle = lv.timer_handler()
        time.sleep(idle / 1000.0)
except KeyboardInterrupt:
    os._exit(0)
