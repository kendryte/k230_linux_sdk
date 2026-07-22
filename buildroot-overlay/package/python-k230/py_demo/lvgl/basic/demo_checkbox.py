#!/usr/bin/env python3
"""Checkbox demo - multiple options with status"""

import lvgl as lv

lv.init()
scr = lv.screen_active()

cb1 = lv.Checkbox(scr)
cb1.set_text("Option A")
cb1.align(lv.ALIGN.TOP_LEFT, 20, 30)

cb2 = lv.Checkbox(scr)
cb2.set_text("Option B")
cb2.align_to(cb1, lv.ALIGN.OUT_BOTTOM_LEFT, 0, 10)

cb3 = lv.Checkbox(scr)
cb3.set_text("Option C")
cb3.align_to(cb2, lv.ALIGN.OUT_BOTTOM_LEFT, 0, 10)

status = lv.Label(scr)
status.set_text("Check an option")
status.align_to(cb3, lv.ALIGN.OUT_BOTTOM_LEFT, 0, 20)

def on_check(event_code):
    if event_code == lv.EVENT.VALUE_CHANGED:
        checked = []
        for cb, name in [(cb1, "A"), (cb2, "B"), (cb3, "C")]:
            if int(cb.get_state()) & int(lv.STATE.CHECKED):
                checked.append(name)
        status.set_text("Checked: %s" % (", ".join(checked) if checked else "None"))

cb1.add_event_cb(lv.EVENT.VALUE_CHANGED, on_check)
cb2.add_event_cb(lv.EVENT.VALUE_CHANGED, on_check)
cb3.add_event_cb(lv.EVENT.VALUE_CHANGED, on_check)

lv.run()
