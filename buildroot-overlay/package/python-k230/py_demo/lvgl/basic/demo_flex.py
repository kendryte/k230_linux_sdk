#!/usr/bin/env python3
"""Flex layout demo - row and column"""

import lvgl as lv

lv.init()
scr = lv.screen_active()

row = scr.obj()
row.set_size(300, 50)
row.align(lv.ALIGN.CENTER, 0, -40)
row.set_flex_flow(lv.FLEX_FLOW.ROW)
row.set_flex_align(
    lv.FLEX_ALIGN.SPACE_EVENLY,
    lv.FLEX_ALIGN.CENTER,
    lv.FLEX_ALIGN.CENTER
)

for i in range(5):
    btn = lv.button(row)
    btn.set_size(45, 35)
    lbl = lv.label(btn)
    lbl.set_text("B%d" % i)
    lbl.center()

col = scr.obj()
col.set_size(100, 200)
col.align(lv.ALIGN.CENTER, 0, 80)
col.set_flex_flow(lv.FLEX_FLOW.COLUMN)
col.set_flex_align(
    lv.FLEX_ALIGN.CENTER,
    lv.FLEX_ALIGN.CENTER,
    lv.FLEX_ALIGN.CENTER
)

for i in range(4):
    lbl = lv.label(col)
    lbl.set_text("Item %d" % i)

lv.run()
