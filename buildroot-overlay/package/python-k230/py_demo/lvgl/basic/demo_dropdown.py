#!/usr/bin/env python3
"""Dropdown demo - color selector"""
import os, time
import lvgl as lv

lv.k230_init()
scr = lv.screen_active()

dd = lv.Dropdown(scr)
dd.set_options("Red\nGreen\nBlue\nYellow")
dd.set_size(140, 35)
dd.center()

color_label = lv.Label(scr)
color_label.label_set_text("Selected: Red (0)")
color_label.align_to(dd, lv.ALIGN.LV_ALIGN_OUT_BOTTOM_MID, 0, 10)

colors = ["Red", "Green", "Blue", "Yellow"]
def on_change(event_code):
    if event_code == int(lv.EVENT_CODE.LV_EVENT_VALUE_CHANGED):
        sel = dd.get_selected()
        color_label.label_set_text("Selected: %s (%d)" % (colors[sel], sel))

dd.add_event_cb(int(lv.EVENT_CODE.LV_EVENT_VALUE_CHANGED), on_change)

try:
    while True:
        idle = lv.timer_handler()
        time.sleep(idle / 1000.0)
except KeyboardInterrupt:
    os._exit(0)
