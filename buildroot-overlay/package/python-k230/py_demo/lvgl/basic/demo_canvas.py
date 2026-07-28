#!/usr/bin/env python3
"""Canvas demo - ARGB8888 buffer with numpy, draw a red rectangle"""
import numpy as np
import lvgl as lv

lv.init()
scr = lv.screen_active()

# 创建 ARGB8888 格式缓冲区
W, H = 100, 100
buf = np.zeros((H, W, 4), dtype=np.uint8)
buf[:, :] = [30, 30, 60, 255]  # 填充深蓝背景 (BGRA in memory)

canvas = lv.canvas(scr)
canvas.set_buffer(buf.ctypes.data, W, H, lv.COLOR_FORMAT.ARGB8888)
canvas.center()

# 绘制一个红色矩形区域
buf[20:80, 20:80] = [0, 0, 255, 255]  # 红色 (BGRA)

lv.run()
