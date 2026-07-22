#!/usr/bin/env python3
"""Roller demo - day selector"""

import lvgl as lv

lv.init()
scr = lv.screen_active()

roller = lv.Roller(scr)
roller.set_options("Monday\nTuesday\nWednesday\nThursday\nFriday", lv.ROLLER.NORMAL)
roller.set_visible_row_count(3)
roller.set_size(120, 100)
roller.center()

day_label = lv.Label(scr)
day_label.set_text("Day: Monday")
day_label.align_to(roller, lv.ALIGN.OUT_BOTTOM_MID, 0, 10)

def on_change(event_code):
    if event_code == lv.EVENT.VALUE_CHANGED:
        opts = roller.get_options()
        sel = roller.get_selected()
        name = opts.split("\n")[sel]
        day_label.set_text("Day: %s" % name)

roller.add_event_cb(lv.EVENT.VALUE_CHANGED, on_change)

lv.run()
