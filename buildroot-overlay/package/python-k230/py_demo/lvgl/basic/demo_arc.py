#!/usr/bin/env python3
"""Arc demo - +/- buttons to adjust value"""
import os, time
import lvgl as lv

lv.k230_init()
scr = lv.screen_active()

arc = lv.Arc(scr)
arc.set_size(120, 120)
arc.center()
arc.set_range(0, 100)
arc.set_value(0)
arc.set_bg_angles(135, 45)
arc.set_mode(lv.ARC_MODE.LV_ARC_MODE_NORMAL)

val_label = lv.Label(scr)
val_label.label_set_text("0")
val_label.align_to(arc, lv.ALIGN.LV_ALIGN_OUT_BOTTOM_MID, 0, 10)

val = [0]
def on_plus(event_code):
    if event_code == int(lv.EVENT_CODE.LV_EVENT_CLICKED):
        val[0] = min(100, val[0] + 10)
        arc.set_value(val[0])
        val_label.label_set_text(str(val[0]))

def on_minus(event_code):
    if event_code == int(lv.EVENT_CODE.LV_EVENT_CLICKED):
        val[0] = max(0, val[0] - 10)
        arc.set_value(val[0])
        val_label.label_set_text(str(val[0]))

btn_plus = lv.Button(scr)
btn_plus.set_size(40, 35)
btn_plus.align_to(arc, lv.ALIGN.LV_ALIGN_OUT_RIGHT_MID, 20, 0)
lbl = lv.Label(btn_plus)
lbl.label_set_text("+")
lbl.center()
btn_plus.add_event_cb(int(lv.EVENT_CODE.LV_EVENT_CLICKED), on_plus)

btn_minus = lv.Button(scr)
btn_minus.set_size(40, 35)
btn_minus.align_to(arc, lv.ALIGN.LV_ALIGN_OUT_LEFT_MID, -20, 0)
lbl2 = lv.Label(btn_minus)
lbl2.label_set_text("-")
lbl2.center()
btn_minus.add_event_cb(int(lv.EVENT_CODE.LV_EVENT_CLICKED), on_minus)

try:
    while True:
        idle = lv.timer_handler()
        time.sleep(idle / 1000.0)
except KeyboardInterrupt:
    os._exit(0)
