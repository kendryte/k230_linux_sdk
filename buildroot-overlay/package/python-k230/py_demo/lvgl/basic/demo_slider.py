#!/usr/bin/env python3
"""Slider demo - +/- buttons to adjust value"""

import lvgl as lv

lv.init()
scr = lv.screen_active()

slider = lv.Slider(scr)
slider.set_width(180)
slider.center()
slider.set_range(0, 100)
slider.set_value(50, False)

val_label = lv.Label(scr)
val_label.set_text("50")
val_label.align_to(slider, lv.ALIGN.OUT_RIGHT_MID, 10, 0)

val = [50]
def on_plus(event_code):
    if event_code == lv.EVENT.CLICKED:
        val[0] = min(100, val[0] + 10)
        slider.set_value(val[0], True)
        val_label.set_text(str(val[0]))

def on_minus(event_code):
    if event_code == lv.EVENT.CLICKED:
        val[0] = max(0, val[0] - 10)
        slider.set_value(val[0], True)
        val_label.set_text(str(val[0]))

btn_m = lv.Button(scr)
btn_m.set_size(35, 30)
btn_m.align_to(slider, lv.ALIGN.OUT_BOTTOM_LEFT, 0, 10)
lbl_m = lv.Label(btn_m)
lbl_m.set_text("-")
lbl_m.center()
btn_m.add_event_cb(lv.EVENT.CLICKED, on_minus)

btn_p = lv.Button(scr)
btn_p.set_size(35, 30)
btn_p.align_to(slider, lv.ALIGN.OUT_BOTTOM_RIGHT, 0, 10)
lbl_p = lv.Label(btn_p)
lbl_p.set_text("+")
lbl_p.center()
btn_p.add_event_cb(lv.EVENT.CLICKED, on_plus)

lv.run()
