#!/usr/bin/env python3
"""Bar demo - progress bar with +10% button"""

import lvgl as lv

lv.init()
scr = lv.screen_active()

bar = lv.bar(scr)
bar.set_size(180, 15)
bar.center()
bar.set_range(0, 100)
bar.set_value(0, False)

val_label = lv.label(scr)
val_label.set_text("0%")
val_label.align_to(bar, lv.ALIGN.OUT_RIGHT_MID, 10, 0)

progress = [0]
def on_click(event):
    if event.code == lv.EVENT.CLICKED:
        progress[0] = (progress[0] + 10) % 110
        bar.set_value(progress[0], True)
        val_label.set_text("%d%%" % progress[0])

btn = lv.button(scr)
btn.set_size(100, 35)
btn.align_to(bar, lv.ALIGN.OUT_BOTTOM_MID, 0, 15)
lbl = lv.label(btn)
lbl.set_text("+10%")
lbl.center()
btn.add_event_cb(lv.EVENT.CLICKED, on_click)

lv.run()
