#!/usr/bin/env python3
"""Image demo - load image file and rotate with button

Note: LVGL uses FS_STDIO backend with drive letter 'A:', so file paths
must be prefixed with 'A:' (e.g. "A:/path/to/image.png").

Supported formats:
  - PNG  (recommended, most reliable)
  - BMP
  - JPEG (baseline only; progressive/optimized JPEGs may fail with
    "jd_mcu_load error" — re-save as baseline JPEG or convert to PNG)

Usage: Change the IMAGE_PATH below to point to an actual image file.
"""

import lvgl as lv

# >>> 修改为你的图片路径 (需要 'A:' 前缀) <<<
IMAGE_PATH = "A:/root/py_demo/lvgl/assets/animimg001.png"

lv.init()
scr = lv.screen_active()

img = lv.Image(scr)
img.center()
img.set_src(IMAGE_PATH)

# 缩放: 256=1x, 512=2x
img.set_scale(256)

# 旋转: 0.1度单位, 900=90.0°
img.set_pivot(30, 30)
img.set_rotation(0)

info = lv.Label(scr)
info.set_text("Image: %s" % IMAGE_PATH)
info.set_style_text_color(lv.color(150, 150, 150), lv.PART.MAIN)
info.align_to(img, lv.ALIGN.OUT_BOTTOM_MID, 0, 10)

angle = [0]
def rotate_step(event_code):
    if event_code == lv.EVENT.CLICKED:
        angle[0] = (angle[0] + 300) % 3600  # 每次旋转30度
        img.set_rotation(angle[0])

btn = lv.Button(scr)
btn.set_size(100, 35)
btn.align(lv.ALIGN.BOTTOM_MID, 0, -20)
btn_label = lv.Label(btn)
btn_label.set_text("Rotate")
btn_label.center()
btn.add_event_cb(lv.EVENT.CLICKED, rotate_step)

lv.run()
