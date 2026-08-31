#!/usr/bin/env python3
"""Arc demo - +/- buttons to adjust value"""

import lvgl as lv

# Optional rotation before lv.init(): _0 / _90 / _180 / _270
lv.settings.rotation = lv.DISPLAY_ROTATION._0
lv.init()
scr = lv.screen_active()

arc = lv.arc(scr)
arc.set_size(120, 120)
arc.center()
arc.set_range(0, 100)
arc.set_value(0)
arc.set_bg_angles(135, 45)
arc.set_mode(lv.ARC_MODE.NORMAL)

val_label = lv.label(scr)
val_label.set_text("0")
val_label.align_to(arc, lv.ALIGN.OUT_BOTTOM_MID, 0, 10)

val = [0]
def on_plus(event):
    if event.code == lv.EVENT.CLICKED:
        val[0] = min(100, val[0] + 10)
        arc.set_value(val[0])
        val_label.set_text(str(val[0]))

def on_minus(event):
    if event.code == lv.EVENT.CLICKED:
        val[0] = max(0, val[0] - 10)
        arc.set_value(val[0])
        val_label.set_text(str(val[0]))

btn_plus = lv.button(scr)
btn_plus.set_size(40, 35)
btn_plus.align_to(arc, lv.ALIGN.OUT_RIGHT_MID, 20, 0)
lbl = lv.label(btn_plus)
lbl.set_text("+")
lbl.center()
btn_plus.add_event_cb(lv.EVENT.CLICKED, on_plus)

btn_minus = lv.button(scr)
btn_minus.set_size(40, 35)
btn_minus.align_to(arc, lv.ALIGN.OUT_LEFT_MID, -20, 0)
lbl2 = lv.label(btn_minus)
lbl2.set_text("-")
lbl2.center()
btn_minus.add_event_cb(lv.EVENT.CLICKED, on_minus)

lv.run()
