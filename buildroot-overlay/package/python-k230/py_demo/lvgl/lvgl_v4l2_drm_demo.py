#!/usr/bin/env python3
"""
LVGL + V4L2-DRM Demo for K230

Demonstrates how to combine LVGL (OSD layer) with camera video (v4l2-drm)
on the K230 platform.

- v4l2-drm handles the camera video layer
- LVGL renders the OSD overlay on top (transparent background)

Usage:
    python3 lvgl_v4l2_drm_demo.py
"""

import os
import time
import lvgl as lv
from k230_v4l2_drm import V4l2Drm


def main():
    # ================================================================
    # 1. Setup v4l2-drm (camera input + DRM display)
    # ================================================================
    v4l2drm = V4l2Drm(context_num=1, osd=True)
    width, height = v4l2drm.drm_init(drm_id=0)
    if width < 0:
        print("Failed to initialize DRM display")
        return
    print("[v4l2-drm] Display: %dx%d" % (width, height))

    v4l2drm.set_context(0, 1, width, height, "NV12", True)
    v4l2drm.setup()

    # ================================================================
    # 2. Initialize LVGL + K230 driver
    #    k230_driver_init() internally:
    #      - calls lv_init()
    #      - registers driver backends
    #      - inits DRM_V4L2_K230 display backend (OSD plane)
    #      - inits EVDEV input backend
    # ================================================================
    lv.init(v4l2drm, v4l2_drm_run_flag=1)
    print("[lvgl] initialized")

    # ================================================================
    # 3. Build OSD UI
    # ================================================================
    scr = lv.screen_active()
    scr.set_style_bg_opa(lv.OPA_TRANSP, lv.PART.MAIN)

    # -- Title --
    title = lv.Label(scr)
    title.set_text("K230 LVGL + Camera")
    title.set_style_bg_opa(lv.OPA_50, lv.PART.MAIN)
    title.set_style_bg_color(lv.color_black(), lv.PART.MAIN)
    title.set_style_text_color(lv.color_white(), lv.PART.MAIN)
    title.align(lv.ALIGN.TOP_MID, 0, 10)

    # -- Toggle button --
    btn = lv.Button(scr)
    btn.set_size(120, 40)
    btn.align(lv.ALIGN.BOTTOM_MID, 0, -20)

    btn_text = lv.Label(btn)
    btn_text.set_text("Hide OSD")
    btn_text.center()

    # -- Toggle callback --
    osd_visible = [True]

    def on_btn_click(event_code):
        if event_code == lv.EVENT.CLICKED:
            if osd_visible[0]:
                title.add_flag(lv.OBJ_FLAG.HIDDEN)
                btn_text.set_text("Show OSD")
                osd_visible[0] = False
            else:
                title.remove_flag(lv.OBJ_FLAG.HIDDEN)
                btn_text.set_text("Hide OSD")
                osd_visible[0] = True

    btn.add_event_cb(lv.EVENT.CLICKED, on_btn_click)

    # ================================================================
    # 4. Run: start v4l2-drm display thread + LVGL main loop
    # ================================================================
    v4l2drm.display_start()
    print("Running... Ctrl+C to exit")

    try:
        lv.run()
    except KeyboardInterrupt:
        print("\nExiting...")
        v4l2drm.display_stop()
        os._exit(0)


if __name__ == "__main__":
    main()
