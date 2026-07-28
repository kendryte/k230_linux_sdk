#!/usr/bin/env python3
"""Scale demo - horizontal tick marks"""

import lvgl as lv

lv.init()
scr = lv.screen_active()

scale = lv.scale(scr)
scale.set_size(200, 50)
scale.center()
scale.set_mode(lv.SCALE_MODE.HORIZONTAL_TOP)
scale.set_total_tick_count(11)
scale.set_major_tick_every(2)
scale.set_range(0, 100)
scale.set_label_show(True)

lv.run()
