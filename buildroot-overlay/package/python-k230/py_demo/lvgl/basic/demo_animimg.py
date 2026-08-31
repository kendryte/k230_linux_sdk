#!/usr/bin/env python3
"""Animimg demo - animated image cycling through 3 PNG frames

Uses the built-in LVGL example images (animimg001/002/003.png)
from the assets/ directory next to this script.

LVGL file paths need the 'A:' prefix (FS_STDIO drive letter).
"""
import os
import lvgl as lv

# 内置图片资源 (LVGL 自带示例)
_SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
IMAGE_PATHS = [
    f"A:{_SCRIPT_DIR}/../assets/animimg001.png",
    f"A:{_SCRIPT_DIR}/../assets/animimg002.png",
    f"A:{_SCRIPT_DIR}/../assets/animimg003.png",
]

# Optional rotation before lv.init(): _0 / _90 / _180 / _270
lv.settings.rotation = lv.DISPLAY_ROTATION._0
lv.init()
scr = lv.screen_active()

anim = lv.animimg(scr)
anim.center()
anim.set_src(IMAGE_PATHS)
anim.set_duration(1000)          # 一个完整循环 1000ms
anim.set_repeat_count(255)       # 255 = 无限循环
anim.start()

info = lv.label(scr)
info.set_text("Animimg: 3 frames, 1000ms cycle")
info.align_to(anim, lv.ALIGN.OUT_BOTTOM_MID, 0, 10)

lv.run()
