#!/usr/bin/env python3
"""
LVGL Python Demo for K230

Demonstrates various LVGL widgets using pybind11 bindings.
Uses init() for initialization.

Usage:
    python3 lvgl_linux_drm_demo.py
"""

import os
import lvgl as lv

ALIGN = lv.ALIGN
EVENT = lv.EVENT


def main():
    # ---- Rotation (optional) ----
    # Must be set before lv.init() — the DRM backend applies it to the
    # display plane during init: _0 / _90 / _180 / _270
    lv.settings.rotation = lv.DISPLAY_ROTATION._0

    # ---- Initialization ----
    lv.init()
    print("[lvgl] initialized")

    scr = lv.screen_active()

    # ---- Title ----
    title = lv.label(scr)
    title.set_text("K230 LVGL Widgets Demo")
    title.align(ALIGN.TOP_MID, 0, 10)

    # ================================================================
    # 1. Button
    # ================================================================
    btn = lv.button(scr)
    btn.set_size(100, 40)
    btn.align(ALIGN.TOP_LEFT, 15, 40)
    btn_label = lv.label(btn)
    btn_label.set_text("Button")
    btn_label.center()

    btn_count_label = lv.label(scr)
    btn_count_label.set_text("Click: 0")
    btn_count_label.align_to(btn, ALIGN.OUT_RIGHT_MID, 10, 0)

    btn_clicks = [0]
    def on_btn_click(event):
        if event.code == EVENT.CLICKED:
            btn_clicks[0] += 1
            btn_count_label.set_text("Click: %d" % btn_clicks[0])
    btn.add_event_cb(EVENT.CLICKED, on_btn_click)

    # ================================================================
    # 2. Checkbox
    # ================================================================
    cb1 = lv.checkbox(scr)
    cb1.set_text("Option A")
    cb1.align(ALIGN.TOP_LEFT, 15, 90)

    cb2 = lv.checkbox(scr)
    cb2.set_text("Option B")
    cb2.align_to(cb1, ALIGN.OUT_BOTTOM_LEFT, 0, 5)

    cb_label = lv.label(scr)
    cb_label.set_text("Checkboxes above")
    cb_label.align_to(cb2, ALIGN.OUT_RIGHT_MID, 15, -10)

    # ================================================================
    # 3. Slider
    # ================================================================
    slider = lv.slider(scr)
    slider.set_range(0, 100)
    slider.set_value(30, False)
    slider.set_width(160)
    slider.align(ALIGN.TOP_LEFT, 15, 180)

    slider_label = lv.label(scr)
    slider_label.set_text("30")
    slider_label.align_to(slider, ALIGN.OUT_RIGHT_MID, 10, 0)

    slider_val = [30]
    def on_slider_change(event):
        if event.code == EVENT.VALUE_CHANGED:
            pass
    slider.add_event_cb(EVENT.VALUE_CHANGED, on_slider_change)

    # Slider +/- buttons (since touch drag may not work with current EVDEV)
    btn_s_minus = lv.button(scr)
    btn_s_minus.set_size(35, 30)
    btn_s_minus.align_to(slider, ALIGN.OUT_BOTTOM_LEFT, 0, 5)
    lbl_minus = lv.label(btn_s_minus)
    lbl_minus.set_text("-")
    lbl_minus.center()

    btn_s_plus = lv.button(scr)
    btn_s_plus.set_size(35, 30)
    btn_s_plus.align_to(slider, ALIGN.OUT_BOTTOM_RIGHT, 0, 5)
    lbl_plus = lv.label(btn_s_plus)
    lbl_plus.set_text("+")
    lbl_plus.center()

    def on_slider_minus(event):
        if event.code == EVENT.CLICKED:
            slider_val[0] = max(0, slider_val[0] - 10)
            slider.set_value(slider_val[0], True)
            slider_label.set_text(str(slider_val[0]))
    def on_slider_plus(event):
        if event.code == EVENT.CLICKED:
            slider_val[0] = min(100, slider_val[0] + 10)
            slider.set_value(slider_val[0], True)
            slider_label.set_text(str(slider_val[0]))
    btn_s_minus.add_event_cb(EVENT.CLICKED, on_slider_minus)
    btn_s_plus.add_event_cb(EVENT.CLICKED, on_slider_plus)

    # ================================================================
    # 4. Bar (non-interactive progress bar)
    # ================================================================
    bar = lv.bar(scr)
    bar.set_range(0, 100)
    bar.set_value(60, False)
    bar.set_size(160, 12)
    bar.align(ALIGN.TOP_LEFT, 15, 260)

    bar_label = lv.label(scr)
    bar_label.set_text("Bar: 60%")
    bar_label.align_to(bar, ALIGN.OUT_RIGHT_MID, 10, 0)

    # ================================================================
    # 5. Arc (circular indicator)
    # ================================================================
    arc = lv.arc(scr)
    arc.set_range(0, 100)
    arc.set_value(70)
    arc.set_angles(135, 45)
    arc.set_size(80, 80)
    arc.align(ALIGN.TOP_RIGHT, -100, 50)

    arc_label = lv.label(scr)
    arc_label.set_text("Arc: 70")
    arc_label.align_to(arc, ALIGN.OUT_BOTTOM_MID, 0, 5)

    # ================================================================
    # 6. Switch (toggle)
    # ================================================================
    sw = lv.switch(scr)
    sw.set_size(50, 28)
    sw.align(ALIGN.TOP_RIGHT, -100, 160)

    sw_label = lv.label(scr)
    sw_label.set_text("OFF")
    sw_label.align_to(sw, ALIGN.OUT_RIGHT_MID, 10, 0)

    def on_switch(event):
        if event.code == EVENT.VALUE_CHANGED:
            sw_label.set_text("Toggle!")
    sw.add_event_cb(EVENT.VALUE_CHANGED, on_switch)

    # ================================================================
    # 7. Spinner (loading indicator)
    # ================================================================
    spinner = lv.spinner(scr)
    spinner.set_size(50, 50)
    spinner.set_anim_params(1000, 60)
    spinner.align(ALIGN.TOP_RIGHT, -100, 210)

    spin_label = lv.label(scr)
    spin_label.set_text("Spinner")
    spin_label.align_to(spinner, ALIGN.OUT_BOTTOM_MID, 0, 5)

    # ================================================================
    # 8. Dropdown (selector)
    # ================================================================
    dd = lv.dropdown(scr)
    dd.set_options("Red\nGreen\nBlue\nYellow")
    dd.set_size(120, 35)
    dd.align(ALIGN.TOP_RIGHT, -100, 290)

    dd_label = lv.label(scr)
    dd_label.set_text("Dropdown above")
    dd_label.align_to(dd, ALIGN.OUT_TOP_MID, 0, -5)

    # ================================================================
    # 9. Textarea (text input)
    # ================================================================
    ta = lv.textarea(scr)
    ta.set_size(160, 35)
    ta.set_one_line(True)
    ta.set_text("Hello K230")
    ta.set_placeholder_text("Type here...")
    ta.align(ALIGN.BOTTOM_LEFT, 15, -15)

    ta_label = lv.label(scr)
    ta_label.set_text("Textarea")
    ta_label.align_to(ta, ALIGN.OUT_TOP_MID, 0, -5)

    # ================================================================
    # 10. LED indicator
    # ================================================================
    led = lv.led(scr)
    led.set_size(30, 30)
    led.set_brightness(255)
    led.align(ALIGN.BOTTOM_RIGHT, -100, -60)

    led_label = lv.label(scr)
    led_label.set_text("LED")
    led_label.align_to(led, ALIGN.OUT_BOTTOM_MID, 0, 5)

    # Toggle LED with button
    btn_led = lv.button(scr)
    btn_led.set_size(60, 30)
    btn_led.align_to(led, ALIGN.OUT_RIGHT_MID, 10, 0)
    lbl_led = lv.label(btn_led)
    lbl_led.set_text("Toggle")
    lbl_led.center()

    led_on = [True]
    def on_toggle_led(event):
        if event.code == EVENT.CLICKED:
            if led_on[0]:
                led.off()
                led.set_brightness(30)
            else:
                led.on()
                led.set_brightness(255)
            led_on[0] = not led_on[0]
    btn_led.add_event_cb(EVENT.CLICKED, on_toggle_led)

    # ================================================================
    # Main loop
    # ================================================================
    print("LVGL demo running... Press Ctrl+C to exit")
    try:
        lv.run()
    except KeyboardInterrupt:
        print("\nExiting...")
        os._exit(0)

if __name__ == "__main__":
    main()
