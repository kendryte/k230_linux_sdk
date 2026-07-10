#!/usr/bin/env python3
"""Tileview demo - swipe between two pages"""
import os, time
import lvgl as lv

lv.k230_init()
scr = lv.screen_active()

tv = lv.Tileview(scr)
tv.set_size(260, 200)
tv.center()

tile1 = tv.add_tile(0, 0, lv.DIR.LV_DIR_RIGHT)
tile2 = tv.add_tile(1, 0, lv.DIR.LV_DIR_LEFT)

lbl1 = lv.Label(tile1)
lbl1.label_set_text("Page 1 (swipe right)")
lbl1.center()

lbl2 = lv.Label(tile2)
lbl2.label_set_text("Page 2 (swipe left)")
lbl2.center()

try:
    while True:
        idle = lv.timer_handler()
        time.sleep(idle / 1000.0)
except KeyboardInterrupt:
    os._exit(0)
