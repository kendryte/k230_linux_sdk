#!/usr/bin/env python3
"""Chinese text demo - display CJK characters using FreeType font"""

import lvgl as lv

lv.init()
scr = lv.screen_active()

# Load a Chinese font via FreeType
# The font file is shipped on the K230 device at /usr/lib/fonts/
font_cn = lv.freetype_font_create(
    "/usr/lib/fonts/SourceHanSansSC-Normal-Min.ttf",
    lv.FREETYPE_FONT_RENDER.BITMAP,
    24,
    lv.FREETYPE_FONT_STYLE.NORMAL,
)

# Title
title = lv.Label(scr)
title.set_style_text_font(font_cn, 0)
title.set_text("你好，世界！")
title.align(lv.ALIGN.TOP_MID, 0, 30)

# Multi-line Chinese text
body = lv.Label(scr)
body.set_style_text_font(font_cn, 0)
body.set_text("这是LVGL中文显示示例。\n思源黑体FreeTyp。")
body.set_width(300)
body.align(lv.ALIGN.CENTER, 0, -20)

# Smaller font size
font_sm = lv.freetype_font_create(
    "/usr/lib/fonts/SourceHanSansSC-Normal-Min.ttf",
    lv.FREETYPE_FONT_RENDER.BITMAP,
    16,
    lv.FREETYPE_FONT_STYLE.NORMAL,
)
footer = lv.Label(scr)
footer.set_style_text_font(font_sm, 0)
footer.set_text("小字体测试 — 16px")
footer.align(lv.ALIGN.BOTTOM_MID, 0, -30)

# Mixed Chinese + English
mixed = lv.Label(scr)
mixed.set_style_text_font(font_cn, 0)
mixed.set_text("当前温度：25度 天气晴")
mixed.align(lv.ALIGN.CENTER, 0, 50)

lv.run()
