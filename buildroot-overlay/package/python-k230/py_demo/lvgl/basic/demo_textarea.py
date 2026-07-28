#!/usr/bin/env python3
"""Textarea demo - single line, password, numbers only"""

import lvgl as lv

lv.init()
scr = lv.screen_active()

ta = lv.textarea(scr)
ta.set_size(200, 35)
ta.set_one_line(True)
ta.set_text("Hello K230")
ta.set_placeholder_text("Type here...")
ta.align(lv.ALIGN.TOP_MID, 0, 30)

ta_pwd = lv.textarea(scr)
ta_pwd.set_size(200, 35)
ta_pwd.set_one_line(True)
ta_pwd.set_password_mode(True)
ta_pwd.set_placeholder_text("Password...")
ta_pwd.align_to(ta, lv.ALIGN.OUT_BOTTOM_MID, 0, 15)

ta_num = lv.textarea(scr)
ta_num.set_size(200, 35)
ta_num.set_one_line(True)
ta_num.set_accepted_chars("0123456789")
ta_num.set_placeholder_text("Numbers only...")
ta_num.set_text("123")
ta_num.align_to(ta_pwd, lv.ALIGN.OUT_BOTTOM_MID, 0, 15)

lv.run()
