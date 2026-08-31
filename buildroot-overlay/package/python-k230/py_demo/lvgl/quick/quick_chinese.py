#!/usr/bin/env python3
"""FreeType 中文字体 — 加载 TTF 字体显示中文"""

import lvgl as lv

# 屏幕旋转，须在 lv.init() 前设置: _0 / _90 / _180 / _270
lv.settings.rotation = lv.DISPLAY_ROTATION._0
lv.init()
scr = lv.screen_active()

# 加载 24px 中文字体 (思源黑体精简版)
font_cn = lv.freetype_font_create(
    "/usr/lib/fonts/SourceHanSansSC-Normal-Min.ttf",
    lv.FREETYPE_RENDER.BITMAP,   # 位图渲染 (推荐)
    24,                                # 字号 (px)
    lv.FREETYPE_STYLE.NORMAL,    # 正常样式
)

# 使用字体显示中文
label = lv.label(scr)
label.set_style_text_font(font_cn, lv.SELECTOR.DEFAULT)   # 设置字体
label.set_text("你好，世界！")
label.center()

lv.run()
