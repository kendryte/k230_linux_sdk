#!/usr/bin/env python3
"""Textarea demo - single line, password, numbers only"""
import os, time
import lvgl as lv

lv.k230_init()
scr = lv.screen_active()

ta = lv.Textarea(scr)
ta.set_size(200, 35)
ta.set_one_line(True)
ta.textarea_set_text("Hello K230")
ta.set_placeholder_text("Type here...")
ta.align(lv.ALIGN.LV_ALIGN_TOP_MID, 0, 30)

ta_pwd = lv.Textarea(scr)
ta_pwd.set_size(200, 35)
ta_pwd.set_one_line(True)
ta_pwd.set_password_mode(True)
ta_pwd.set_placeholder_text("Password...")
ta_pwd.align_to(ta, lv.ALIGN.LV_ALIGN_OUT_BOTTOM_MID, 0, 15)

ta_num = lv.Textarea(scr)
ta_num.set_size(200, 35)
ta_num.set_one_line(True)
ta_num.set_accepted_chars("0123456789")
ta_num.set_placeholder_text("Numbers only...")
ta_num.textarea_set_text("123")
ta_num.align_to(ta_pwd, lv.ALIGN.LV_ALIGN_OUT_BOTTOM_MID, 0, 15)

try:
    while True:
        idle = lv.timer_handler()
        time.sleep(idle / 1000.0)
except KeyboardInterrupt:
    os._exit(0)
