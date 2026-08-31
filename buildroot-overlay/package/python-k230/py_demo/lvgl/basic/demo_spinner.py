#!/usr/bin/env python3
"""Spinner demo - loading animation"""

import lvgl as lv

# Optional rotation before lv.init(): _0 / _90 / _180 / _270
lv.settings.rotation = lv.DISPLAY_ROTATION._0
lv.init()
scr = lv.screen_active()

sp = lv.spinner(scr)
sp.set_size(60, 60)
sp.center()
sp.set_anim_params(1000, 60)

label = lv.label(scr)
label.set_text("Loading...")
label.align_to(sp, lv.ALIGN.OUT_BOTTOM_MID, 0, 10)

lv.run()
