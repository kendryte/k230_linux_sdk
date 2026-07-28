#!/usr/bin/env python3
"""Tileview demo - swipe between two pages"""

import lvgl as lv

lv.init()
scr = lv.screen_active()

tv = lv.tileview(scr)
tv.set_size(260, 200)
tv.center()

tile1 = tv.add_tile(0, 0, lv.DIR.RIGHT)
tile2 = tv.add_tile(1, 0, lv.DIR.LEFT)

lbl1 = lv.label(tile1)
lbl1.set_text("Page 1 (swipe right)")
lbl1.center()

lbl2 = lv.label(tile2)
lbl2.set_text("Page 2 (swipe left)")
lbl2.center()

lv.run()
