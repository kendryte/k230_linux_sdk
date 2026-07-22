#!/usr/bin/env python3
"""Win demo - window with title and content"""

import lvgl as lv

lv.init()
scr = lv.screen_active()

win = lv.Win(scr)
win.set_size(260, 200)
win.center()

win.add_title("My Window")

content = win.get_content()
label = lv.Label(content)
label.set_text("Window content area")
label.center()

lv.run()
