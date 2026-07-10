#!/usr/bin/env python3
"""Checkbox demo - multiple options with status"""
import os, time
import lvgl as lv

lv.k230_init()
scr = lv.screen_active()

cb1 = lv.Checkbox(scr)
cb1.set_text("Option A")
cb1.align(lv.ALIGN.LV_ALIGN_TOP_LEFT, 20, 30)

cb2 = lv.Checkbox(scr)
cb2.set_text("Option B")
cb2.align_to(cb1, lv.ALIGN.LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10)

cb3 = lv.Checkbox(scr)
cb3.set_text("Option C")
cb3.align_to(cb2, lv.ALIGN.LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10)

status = lv.Label(scr)
status.label_set_text("Check an option")
status.align_to(cb3, lv.ALIGN.LV_ALIGN_OUT_BOTTOM_LEFT, 0, 20)

def on_check(event_code):
    if event_code == int(lv.EVENT_CODE.LV_EVENT_VALUE_CHANGED):
        checked = []
        for cb, name in [(cb1, "A"), (cb2, "B"), (cb3, "C")]:
            if int(cb.get_state()) & int(lv.STATE.LV_STATE_CHECKED):
                checked.append(name)
        status.label_set_text("Checked: %s" % (", ".join(checked) if checked else "None"))

cb1.add_event_cb(int(lv.EVENT_CODE.LV_EVENT_VALUE_CHANGED), on_check)
cb2.add_event_cb(int(lv.EVENT_CODE.LV_EVENT_VALUE_CHANGED), on_check)
cb3.add_event_cb(int(lv.EVENT_CODE.LV_EVENT_VALUE_CHANGED), on_check)

try:
    while True:
        idle = lv.timer_handler()
        time.sleep(idle / 1000.0)
except KeyboardInterrupt:
    os._exit(0)
