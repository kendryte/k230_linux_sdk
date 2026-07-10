#!/usr/bin/env python3
"""Flex layout demo - row and column"""
import os, time
import lvgl as lv

lv.k230_init()
scr = lv.screen_active()

row = lv.Obj.create(scr)
row.set_size(300, 50)
row.align(lv.ALIGN.LV_ALIGN_CENTER, 0, -40)
row.set_flex_flow(lv.FLEX_FLOW.LV_FLEX_FLOW_ROW)
row.set_flex_align(
    lv.FLEX_ALIGN.LV_FLEX_ALIGN_SPACE_EVENLY,
    lv.FLEX_ALIGN.LV_FLEX_ALIGN_CENTER,
    lv.FLEX_ALIGN.LV_FLEX_ALIGN_CENTER
)

for i in range(5):
    btn = lv.Button(row)
    btn.set_size(45, 35)
    lbl = lv.Label(btn)
    lbl.label_set_text("B%d" % i)
    lbl.center()

col = lv.Obj.create(scr)
col.set_size(100, 200)
col.align(lv.ALIGN.LV_ALIGN_CENTER, 0, 80)
col.set_flex_flow(lv.FLEX_FLOW.LV_FLEX_FLOW_COLUMN)
col.set_flex_align(
    lv.FLEX_ALIGN.LV_FLEX_ALIGN_CENTER,
    lv.FLEX_ALIGN.LV_FLEX_ALIGN_CENTER,
    lv.FLEX_ALIGN.LV_FLEX_ALIGN_CENTER
)

for i in range(4):
    lbl = lv.Label(col)
    lbl.label_set_text("Item %d" % i)

try:
    while True:
        idle = lv.timer_handler()
        time.sleep(idle / 1000.0)
except KeyboardInterrupt:
    os._exit(0)
