#!/usr/bin/env python3
"""摄像头 + OSD 叠加 — LVGL 界面覆盖在摄像头画面上"""
import os
import lvgl as lv
from k230_v4l2_drm import V4l2Drm

# 1. 初始化 v4l2-drm (摄像头 + DRM 显示)
v4l2drm = V4l2Drm(context_num=1, osd=True)
width, height = v4l2drm.drm_init(drm_id=0)
v4l2drm.set_context(0, 1, width, height, "NV12", True)
v4l2drm.setup()

# 2. 初始化 LVGL，共享 DRM 显示
lv.init(v4l2drm, v4l2_drm_run_flag=1)

# 3. 设置透明背景，让摄像头画面透出来
scr = lv.screen_active()
scr.set_style_bg_opa(lv.OPA_TRANSP, lv.SELECTOR.DEFAULT)

# 4. 添加 OSD 文字
title = lv.label(scr)
title.set_text("Camera + OSD")
title.align(lv.ALIGN.TOP_MID, 0, 10)

# 5. 启动摄像头显示，进入主循环
v4l2drm.display_start()
try:
    lv.run()
except KeyboardInterrupt:
    os._exit(0)
