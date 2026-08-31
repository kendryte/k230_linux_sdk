#!/usr/bin/env python3
"""Chinese text demo - display CJK characters using FreeType font"""

import lvgl as lv

# Optional rotation before lv.init(): _0 / _90 / _180 / _270
lv.settings.rotation = lv.DISPLAY_ROTATION._0
lv.init()
scr = lv.screen_active()

# Load a Chinese font via FreeType
# The font file is shipped on the K230 device at /usr/lib/fonts/
font_cn = lv.freetype_font_create(
    "/usr/lib/fonts/SourceHanSansSC-Normal-Min.ttf",
    lv.FREETYPE_RENDER.BITMAP,
    24,
    lv.FREETYPE_STYLE.NORMAL,
)

# Title
title = lv.label(scr)
title.set_style_text_font(font_cn, lv.SELECTOR.DEFAULT)
title.set_text("你好，世界！")
title.align(lv.ALIGN.TOP_MID, 0, 30)

# Multi-line Chinese text
body = lv.label(scr)
body.set_style_text_font(font_cn, lv.SELECTOR.DEFAULT)
body.set_text("这是LVGL中文显示示例。\n思源黑体FreeTyp。")
body.set_width(300)
body.align(lv.ALIGN.CENTER, 0, -20)

# Smaller font size
font_sm = lv.freetype_font_create(
    "/usr/lib/fonts/SourceHanSansSC-Normal-Min.ttf",
    lv.FREETYPE_RENDER.BITMAP,
    16,
    lv.FREETYPE_STYLE.NORMAL,
)
footer = lv.label(scr)
footer.set_style_text_font(font_sm, lv.SELECTOR.DEFAULT)
footer.set_text("小字体测试 — 16px")
footer.align(lv.ALIGN.BOTTOM_MID, 0, -30)

# Mixed Chinese + English
mixed = lv.label(scr)
mixed.set_style_text_font(font_cn, lv.SELECTOR.DEFAULT)
mixed.set_text("当前温度：25度 天气晴")
mixed.align(lv.ALIGN.CENTER, 0, 50)

lv.run()
