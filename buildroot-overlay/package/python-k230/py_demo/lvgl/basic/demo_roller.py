#!/usr/bin/env python3
"""Roller demo - day selector"""
import os, time
import lvgl as lv

lv.k230_init()
scr = lv.screen_active()

roller = lv.Roller(scr)
roller.roller_set_options("Monday\nTuesday\nWednesday\nThursday\nFriday", lv.ROLLER_MODE.LV_ROLLER_MODE_NORMAL)
roller.set_visible_row_count(3)
roller.set_size(120, 100)
roller.center()

day_label = lv.Label(scr)
day_label.label_set_text("Day: Monday")
day_label.align_to(roller, lv.ALIGN.LV_ALIGN_OUT_BOTTOM_MID, 0, 10)

def on_change(event_code):
    if event_code == int(lv.EVENT_CODE.LV_EVENT_VALUE_CHANGED):
        opts = roller.roller_get_options()
        sel = roller.roller_get_selected()
        name = opts.split("\n")[sel]
        day_label.label_set_text("Day: %s" % name)

roller.add_event_cb(int(lv.EVENT_CODE.LV_EVENT_VALUE_CHANGED), on_change)

try:
    while True:
        idle = lv.timer_handler()
        time.sleep(idle / 1000.0)
except KeyboardInterrupt:
    os._exit(0)
