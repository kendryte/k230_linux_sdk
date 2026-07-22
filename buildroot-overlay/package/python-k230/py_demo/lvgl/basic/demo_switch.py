#!/usr/bin/env python3
"""Switch demo - toggle on/off"""

import lvgl as lv

lv.init()
scr = lv.screen_active()

sw = lv.Switch(scr)
sw.set_size(50, 28)
sw.center()

sw_label = lv.Label(scr)
sw_label.set_text("OFF")
sw_label.align_to(sw, lv.ALIGN.OUT_RIGHT_MID, 10, 0)

def on_switch(event_code):
    if event_code == lv.EVENT.VALUE_CHANGED:
        sw_label.set_text("Toggle!")

sw.add_event_cb(lv.EVENT.VALUE_CHANGED, on_switch)

lv.run()
