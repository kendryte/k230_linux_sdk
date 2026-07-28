#!/usr/bin/env python3
"""Line demo - draw a zigzag line via set_points + style"""
import lvgl as lv

lv.init()
scr = lv.screen_active()

# 创建折线控件
line = lv.line(scr)
line.center()

# 设置折线坐标点 (相对于控件自身)
points = [
    (0, 0),
    (40, 30),
    (80, 0),
    (120, 30),
    (160, 0),
    (200, 30),
]
line.set_points(points)

# 样式：线宽、颜色、圆角端点
line.set_style_line_width(3, lv.SELECTOR.DEFAULT)
line.set_style_line_color(lv.color_hex(0x0080FF), lv.SELECTOR.DEFAULT)
line.set_style_line_rounded(True, lv.SELECTOR.DEFAULT)

lv.run()
