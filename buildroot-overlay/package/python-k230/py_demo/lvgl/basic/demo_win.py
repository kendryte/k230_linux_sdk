#!/usr/bin/env python3
"""Win demo - window with title and content"""

import lvgl as lv

# Optional rotation before lv.init(): _0 / _90 / _180 / _270
lv.settings.rotation = lv.DISPLAY_ROTATION._0
lv.init()
scr = lv.screen_active()

win = lv.win(scr)
win.set_size(260, 200)
win.center()

win.add_title("My Window")

content = win.get_content()
label = lv.label(content)
label.set_text("Window content area")
label.center()

lv.run()
