#!/usr/bin/env python3
"""
LVGL Demo for K230 (default DRM backend)

Uses the standard Linux DRM driver without v4l2-drm camera overlay.
This is the simplest way to get LVGL running on K230.

Usage:
    python3 lvgl_drm_demo.py
"""

import os
import time
import lvgl as lv


def main():
    # ---- Initialize LVGL + default DRM driver ----
    # k230_init() with no arguments uses the standard DRM backend
    lv.k230_init()
    print("[lvgl] initialized with default DRM backend")

    # ---- Create UI ----
    scr = lv.screen_active()

    # Title
    title = lv.Label(scr)
    title.label_set_text("K230 LVGL Demo")
    title.align(lv.ALIGN.LV_ALIGN_TOP_MID, 0, 10)

    # Button
    btn = lv.Button(scr)
    btn.set_size(120, 40)
    btn.align(lv.ALIGN.LV_ALIGN_CENTER, 0, 0)

    btn_text = lv.Label(btn)
    btn_text.label_set_text("Click me")
    btn_text.center()

    # Click counter
    count_label = lv.Label(scr)
    count_label.label_set_text("Clicks: 0")
    count_label.align_to(btn, lv.ALIGN.LV_ALIGN_OUT_BOTTOM_MID, 0, 10)

    clicks = [0]

    def on_click(event_code):
        if event_code == int(lv.EVENT_CODE.LV_EVENT_CLICKED):
            clicks[0] += 1
            count_label.label_set_text("Clicks: %d" % clicks[0])

    btn.add_event_cb(int(lv.EVENT_CODE.LV_EVENT_CLICKED), on_click)

    # Slider
    slider = lv.Slider(scr)
    slider.set_width(160)
    slider.align(lv.ALIGN.LV_ALIGN_CENTER, 0, 60)

    slider_label = lv.Label(scr)
    slider_label.label_set_text("50")
    slider_label.align_to(slider, lv.ALIGN.LV_ALIGN_OUT_RIGHT_MID, 10, 0)

    def on_slider(event_code):
        if event_code == int(lv.EVENT_CODE.LV_EVENT_VALUE_CHANGED):
            val = slider.get_value()
            slider_label.label_set_text(str(val))

    slider.add_event_cb(int(lv.EVENT_CODE.LV_EVENT_VALUE_CHANGED), on_slider)

    # ---- Main loop ----
    print("Running... Ctrl+C to exit")

    try:
        while True:
            idle = lv.timer_handler()
            time.sleep(idle / 1000.0)
    except KeyboardInterrupt:
        print("\nExiting...")
        os._exit(0)


if __name__ == "__main__":
    main()
