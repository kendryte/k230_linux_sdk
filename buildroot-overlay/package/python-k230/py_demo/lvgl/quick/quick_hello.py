#!/usr/bin/env python3
"""最小 LVGL 示例 — 一个按钮 + Hello 文字"""

import lvgl as lv

# 初始化 LVGL + K230 DRM 驱动
lv.init()

scr = lv.screen_active()

# 创建按钮
btn = lv.button(scr)
btn.set_size(120, 40)
btn.center()

# 按钮上的文字
label = lv.label(btn)
label.set_text("Hello K230!")
label.center()

# 主循环
lv.run()
