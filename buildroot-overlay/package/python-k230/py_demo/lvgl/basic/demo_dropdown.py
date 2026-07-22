#!/usr/bin/env python3
"""Dropdown demo - color selector"""

import lvgl as lv

lv.init()
scr = lv.screen_active()

dd = lv.Dropdown(scr)
dd.set_options("Red\nGreen\nBlue\nYellow")
dd.set_size(140, 35)
dd.center()

color_label = lv.Label(scr)
color_label.set_text("Selected: Red (0)")
color_label.align_to(dd, lv.ALIGN.OUT_BOTTOM_MID, 0, 10)

colors = ["Red", "Green", "Blue", "Yellow"]
def on_change(event_code):
    if event_code == lv.EVENT.VALUE_CHANGED:
        sel = dd.get_selected()
        color_label.set_text("Selected: %s (%d)" % (colors[sel], sel))

dd.add_event_cb(lv.EVENT.VALUE_CHANGED, on_change)

lv.run()
