#!/usr/bin/env python3
"""Spinbox demo - increment/decrement with buttons"""
import os, time
import lvgl as lv

lv.k230_init()
scr = lv.screen_active()

sb = lv.Spinbox(scr)
sb.set_width(120)
sb.center()
sb.set_digit_format(4, 0)
sb.set_step(100)
sb.spinbox_set_range(0, 9999)
sb.spinbox_set_value(0)

val_label = lv.Label(scr)
val_label.label_set_text("0")
val_label.align_to(sb, lv.ALIGN.LV_ALIGN_OUT_BOTTOM_MID, 0, 10)

def on_inc(event_code):
    if event_code == int(lv.EVENT_CODE.LV_EVENT_CLICKED):
        sb.increment()
        val_label.label_set_text(str(sb.spinbox_get_value()))

def on_dec(event_code):
    if event_code == int(lv.EVENT_CODE.LV_EVENT_CLICKED):
        sb.decrement()
        val_label.label_set_text(str(sb.spinbox_get_value()))

btn_inc = lv.Button(scr)
btn_inc.set_size(50, 30)
btn_inc.align_to(sb, lv.ALIGN.LV_ALIGN_OUT_RIGHT_MID, 15, 0)
lbl_i = lv.Label(btn_inc)
lbl_i.label_set_text("+")
lbl_i.center()
btn_inc.add_event_cb(int(lv.EVENT_CODE.LV_EVENT_CLICKED), on_inc)

btn_dec = lv.Button(scr)
btn_dec.set_size(50, 30)
btn_dec.align_to(sb, lv.ALIGN.LV_ALIGN_OUT_LEFT_MID, -15, 0)
lbl_d = lv.Label(btn_dec)
lbl_d.label_set_text("-")
lbl_d.center()
btn_dec.add_event_cb(int(lv.EVENT_CODE.LV_EVENT_CLICKED), on_dec)

try:
    while True:
        idle = lv.timer_handler()
        time.sleep(idle / 1000.0)
except KeyboardInterrupt:
    os._exit(0)
