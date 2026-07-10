#!/usr/bin/env python3
"""Scale demo - horizontal tick marks"""
import os, time
import lvgl as lv

lv.k230_init()
scr = lv.screen_active()

scale = lv.Scale(scr)
scale.set_size(200, 50)
scale.center()
scale.scale_set_mode(lv.SCALE_MODE.LV_SCALE_MODE_HORIZONTAL_TOP)
scale.set_total_tick_count(11)
scale.set_major_tick_every(2)
scale.scale_set_range(0, 100)
scale.set_label_show(True)

try:
    while True:
        idle = lv.timer_handler()
        time.sleep(idle / 1000.0)
except KeyboardInterrupt:
    os._exit(0)
