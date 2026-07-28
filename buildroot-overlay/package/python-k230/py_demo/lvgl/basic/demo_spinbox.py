#!/usr/bin/env python3
"""Spinbox demo - increment/decrement with buttons"""

import lvgl as lv

lv.init()
scr = lv.screen_active()

sb = lv.spinbox(scr)
sb.set_width(120)
sb.center()
sb.set_digit_format(4, 0)
sb.set_step(100)
sb.set_range(0, 9999)
sb.set_value(0)

val_label = lv.label(scr)
val_label.set_text("0")
val_label.align_to(sb, lv.ALIGN.OUT_BOTTOM_MID, 0, 10)

def on_inc(event):
    if event.code == lv.EVENT.CLICKED:
        sb.increment()
        val_label.set_text(str(sb.get_value()))

def on_dec(event):
    if event.code == lv.EVENT.CLICKED:
        sb.decrement()
        val_label.set_text(str(sb.get_value()))

btn_inc = lv.button(scr)
btn_inc.set_size(50, 30)
btn_inc.align_to(sb, lv.ALIGN.OUT_RIGHT_MID, 15, 0)
lbl_i = lv.label(btn_inc)
lbl_i.set_text("+")
lbl_i.center()
btn_inc.add_event_cb(lv.EVENT.CLICKED, on_inc)

btn_dec = lv.button(scr)
btn_dec.set_size(50, 30)
btn_dec.align_to(sb, lv.ALIGN.OUT_LEFT_MID, -15, 0)
lbl_d = lv.label(btn_dec)
lbl_d.set_text("-")
lbl_d.center()
btn_dec.add_event_cb(lv.EVENT.CLICKED, on_dec)

lv.run()
