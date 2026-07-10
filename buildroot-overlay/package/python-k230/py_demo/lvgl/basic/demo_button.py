#!/usr/bin/env python3
"""Button demo - click counter"""
import os, time
import lvgl as lv

lv.k230_init()
scr = lv.screen_active()

btn = lv.Button(scr)
btn.set_size(120, 40)
btn.center()

btn_label = lv.Label(btn)
btn_label.label_set_text("Click Me")
btn_label.center()

count_label = lv.Label(scr)
count_label.label_set_text("Clicks: 0")
count_label.align_to(btn, lv.ALIGN.LV_ALIGN_OUT_BOTTOM_MID, 0, 10)

clicks = [0]
def on_click(event_code):
    if event_code == int(lv.EVENT_CODE.LV_EVENT_CLICKED):
        clicks[0] += 1
        count_label.label_set_text("Clicks: %d" % clicks[0])

btn.add_event_cb(int(lv.EVENT_CODE.LV_EVENT_CLICKED), on_click)

try:
    while True:
        idle = lv.timer_handler()
        time.sleep(idle / 1000.0)
except KeyboardInterrupt:
    os._exit(0)
