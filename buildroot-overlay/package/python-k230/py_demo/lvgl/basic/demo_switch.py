#!/usr/bin/env python3
"""Switch demo - toggle on/off"""
import os, time
import lvgl as lv

lv.k230_init()
scr = lv.screen_active()

sw = lv.Switch(scr)
sw.set_size(50, 28)
sw.center()

sw_label = lv.Label(scr)
sw_label.label_set_text("OFF")
sw_label.align_to(sw, lv.ALIGN.LV_ALIGN_OUT_RIGHT_MID, 10, 0)

def on_switch(event_code):
    if event_code == int(lv.EVENT_CODE.LV_EVENT_VALUE_CHANGED):
        sw_label.label_set_text("Toggle!")

sw.add_event_cb(int(lv.EVENT_CODE.LV_EVENT_VALUE_CHANGED), on_switch)

try:
    while True:
        idle = lv.timer_handler()
        time.sleep(idle / 1000.0)
except KeyboardInterrupt:
    os._exit(0)
