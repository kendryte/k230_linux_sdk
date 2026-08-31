#!/usr/bin/env python3
"""Keyboard demo - text input with virtual keyboard"""

import lvgl as lv

# Optional rotation before lv.init(): _0 / _90 / _180 / _270
lv.settings.rotation = lv.DISPLAY_ROTATION._0
lv.init()
scr = lv.screen_active()

ta = lv.textarea(scr)
ta.set_size(200, 35)
ta.set_one_line(True)
ta.set_placeholder_text("Type here...")
ta.align(lv.ALIGN.TOP_MID, 0, 10)

kb = lv.keyboard(scr)
kb.set_size(260, 120)
kb.align(lv.ALIGN.BOTTOM_MID, 0, -10)
kb.set_textarea(ta)

lv.run()
