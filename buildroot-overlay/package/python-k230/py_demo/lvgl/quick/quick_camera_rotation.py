#!/usr/bin/env python3
"""摄像头 + OSD + 旋转 — 竖屏 (高>宽) 适配"""
import os, sys
import lvgl as lv
from k230_v4l2_drm import V4l2Drm, ROTATION_0, ROTATION_90, ROTATION_270

# 1. 初始化 v4l2-drm
v4l2drm = V4l2Drm(context_num=1, osd=False)
display_w, display_h = v4l2drm.drm_init(drm_id=0)
if display_h < 0:
    print("Failed to initialize DRM display")
    sys.exit(1)

# 2. 设置摄像头上下文，宽高取 display 的较大值/较小值
#    保证旋转后画面填满屏幕
v4l2drm.set_context(
    0, 1,
    width=max(display_w, display_h),
    height=min(display_w, display_h),
    format="NV12", display=True
)

# 3. 根据屏幕方向设置旋转角度
#    横屏 (w > h) → ROTATION_0, 竖屏 (h > w) → ROTATION_90
rotation = ROTATION_0 if display_w > display_h else ROTATION_90
v4l2drm.set_rotation(0, rotation)
v4l2drm.setup()

# 4. 初始化 LVGL，共享 DRM 显示
lv.init(v4l2drm, v4l2_drm_run_flag=1)

# 5. 透明背景 + 半透明标题栏
scr = lv.screen_active()
scr.set_style_bg_opa(lv.OPA_TRANSP, lv.SELECTOR.DEFAULT)

title = lv.label(scr)
title.set_text("K230 Camera + Rotation")
title.set_style_bg_opa(lv.OPA_50, lv.SELECTOR.DEFAULT)
title.set_style_bg_color(lv.color_black(), lv.SELECTOR.DEFAULT)
title.set_style_text_color(lv.color_white(), lv.SELECTOR.DEFAULT)
title.align(lv.ALIGN.TOP_MID, 0, 10)

# 6. 启动摄像头显示
v4l2drm.display_start()
try:
    lv.run()
except KeyboardInterrupt:
    v4l2drm.display_stop()
    os._exit(0)
