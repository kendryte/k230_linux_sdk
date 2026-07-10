#!/usr/bin/env python3
"""Bar demo - progress bar with +10% button"""
import os, time
import lvgl as lv

lv.k230_init()
scr = lv.screen_active()

bar = lv.Bar(scr)
bar.set_size(180, 15)
bar.center()
bar.bar_set_range(0, 100)
bar.bar_set_value(0, False)

val_label = lv.Label(scr)
val_label.label_set_text("0%")
val_label.align_to(bar, lv.ALIGN.LV_ALIGN_OUT_RIGHT_MID, 10, 0)

progress = [0]
def on_click(event_code):
    if event_code == int(lv.EVENT_CODE.LV_EVENT_CLICKED):
        progress[0] = (progress[0] + 10) % 110
        bar.bar_set_value(progress[0], True)
        val_label.label_set_text("%d%%" % progress[0])

btn = lv.Button(scr)
btn.set_size(100, 35)
btn.align_to(bar, lv.ALIGN.LV_ALIGN_OUT_BOTTOM_MID, 0, 15)
lbl = lv.Label(btn)
lbl.label_set_text("+10%")
lbl.center()
btn.add_event_cb(int(lv.EVENT_CODE.LV_EVENT_CLICKED), on_click)

try:
    while True:
        idle = lv.timer_handler()
        time.sleep(idle / 1000.0)
except KeyboardInterrupt:
    os._exit(0)
