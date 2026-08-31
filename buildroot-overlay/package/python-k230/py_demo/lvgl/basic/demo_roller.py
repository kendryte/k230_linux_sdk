#!/usr/bin/env python3
"""Roller demo - day selector"""

import lvgl as lv

# Optional rotation before lv.init(): _0 / _90 / _180 / _270
lv.settings.rotation = lv.DISPLAY_ROTATION._0
lv.init()
scr = lv.screen_active()

roller = lv.roller(scr)
roller.set_options("Monday\nTuesday\nWednesday\nThursday\nFriday", lv.ROLLER_MODE.NORMAL)
roller.set_visible_row_count(3)
roller.set_size(120, 100)
roller.center()

day_label = lv.label(scr)
day_label.set_text("Day: Monday")
day_label.align_to(roller, lv.ALIGN.OUT_BOTTOM_MID, 0, 10)

def on_change(event):
    if event.code == lv.EVENT.VALUE_CHANGED:
        opts = roller.get_options()
        sel = roller.get_selected()
        name = opts.split("\n")[sel]
        day_label.set_text("Day: %s" % name)

roller.add_event_cb(lv.EVENT.VALUE_CHANGED, on_change)

lv.run()
