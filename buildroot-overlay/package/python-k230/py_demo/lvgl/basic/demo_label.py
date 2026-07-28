#!/usr/bin/env python3
"""Label demo - wrap, scroll, dots modes"""

import lvgl as lv

lv.init()
scr = lv.screen_active()

# 换行模式
label1 = lv.label(scr)
label1.set_text("Label with\nline wrap")
label1.set_width(120)
label1.align(lv.ALIGN.TOP_MID, 0, 20)

# 滚动模式
label2 = lv.label(scr)
label2.set_text("This is a long scrolling label text")
label2.set_long_mode(lv.LABEL_LONG.SCROLL_CIRCULAR)
label2.set_width(150)
label2.align(lv.ALIGN.CENTER, 0, 0)

# 省略号模式
label3 = lv.label(scr)
label3.set_text("This text is too long and will be truncated with dots")
label3.set_long_mode(lv.LABEL_LONG.DOTS)
label3.set_width(150)
label3.align(lv.ALIGN.BOTTOM_MID, 0, -20)

lv.run()
