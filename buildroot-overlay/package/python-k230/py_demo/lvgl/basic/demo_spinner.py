#!/usr/bin/env python3
"""Spinner demo - loading animation"""
import os, time
import lvgl as lv

lv.k230_init()
scr = lv.screen_active()

sp = lv.Spinner(scr)
sp.set_size(60, 60)
sp.center()
sp.set_anim_params(1000, 60)

label = lv.Label(scr)
label.label_set_text("Loading...")
label.align_to(sp, lv.ALIGN.LV_ALIGN_OUT_BOTTOM_MID, 0, 10)

try:
    while True:
        idle = lv.timer_handler()
        time.sleep(idle / 1000.0)
except KeyboardInterrupt:
    os._exit(0)
