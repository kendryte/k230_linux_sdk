#!/usr/bin/env python3
"""Tabview demo - three tab pages"""
import os, time
import lvgl as lv

lv.k230_init()
scr = lv.screen_active()

tv = lv.Tabview(scr)
tv.set_size(260, 200)
tv.center()

tab1 = tv.add_tab("Page 1")
tab2 = tv.add_tab("Page 2")
tab3 = tv.add_tab("Page 3")

lbl1 = lv.Label(tab1)
lbl1.label_set_text("Content of Page 1")
lbl1.center()

lbl2 = lv.Label(tab2)
lbl2.label_set_text("Content of Page 2")
lbl2.center()

lbl3 = lv.Label(tab3)
lbl3.label_set_text("Content of Page 3")
lbl3.center()

try:
    while True:
        idle = lv.timer_handler()
        time.sleep(idle / 1000.0)
except KeyboardInterrupt:
    os._exit(0)
