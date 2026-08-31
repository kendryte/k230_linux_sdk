#!/usr/bin/env python3
"""Button demo - click counter"""

import lvgl as lv

# Optional rotation before lv.init(): _0 / _90 / _180 / _270
lv.settings.rotation = lv.DISPLAY_ROTATION._0
lv.init()
scr = lv.screen_active()

btn = lv.button(scr)
btn.set_size(120, 40)
btn.center()

btn_label = lv.label(btn)
btn_label.set_text("Click Me")
btn_label.center()

count_label = lv.label(scr)
count_label.set_text("Clicks: 0")
count_label.align_to(btn, lv.ALIGN.OUT_BOTTOM_MID, 0, 10)

clicks = [0]
def on_click(event):
    if event.code == lv.EVENT.CLICKED:
        clicks[0] += 1
        count_label.set_text("Clicks: %d" % clicks[0])

btn.add_event_cb(lv.EVENT.CLICKED, on_click)

lv.run()
