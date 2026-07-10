#!/usr/bin/env python3
"""Keyboard demo - text input with virtual keyboard"""
import os, time
import lvgl as lv

lv.k230_init()
scr = lv.screen_active()

ta = lv.Textarea(scr)
ta.set_size(200, 35)
ta.set_one_line(True)
ta.set_placeholder_text("Type here...")
ta.align(lv.ALIGN.LV_ALIGN_TOP_MID, 0, 10)

kb = lv.Keyboard(scr)
kb.set_size(260, 120)
kb.align(lv.ALIGN.LV_ALIGN_BOTTOM_MID, 0, -10)
kb.set_textarea(ta)

try:
    while True:
        idle = lv.timer_handler()
        time.sleep(idle / 1000.0)
except KeyboardInterrupt:
    os._exit(0)
