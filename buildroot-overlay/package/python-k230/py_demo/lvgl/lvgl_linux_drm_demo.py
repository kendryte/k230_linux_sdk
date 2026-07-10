#!/usr/bin/env python3
"""
LVGL Python Demo for K230

Demonstrates various LVGL widgets using pybind11 bindings.
Uses k230_init() for initialization.

Usage:
    python3 lvgl_demo.py
"""

import os
import time
import lvgl as lv

ALIGN = lv.ALIGN
EVENT = lv.EVENT_CODE


def main():
    # ---- Initialization ----
    lv.k230_init()
    print("[lvgl] initialized")

    scr = lv.screen_active()

    # ---- Title ----
    title = lv.Label(scr)
    title.label_set_text("K230 LVGL Widgets Demo")
    title.align(ALIGN.LV_ALIGN_TOP_MID, 0, 10)

    # ================================================================
    # 1. Button
    # ================================================================
    btn = lv.Button(scr)
    btn.set_size(100, 40)
    btn.align(ALIGN.LV_ALIGN_TOP_LEFT, 15, 40)
    btn_label = lv.Label(btn)
    btn_label.label_set_text("Button")
    btn_label.center()

    btn_count_label = lv.Label(scr)
    btn_count_label.label_set_text("Click: 0")
    btn_count_label.align_to(btn, ALIGN.LV_ALIGN_OUT_RIGHT_MID, 10, 0)

    btn_clicks = [0]
    def on_btn_click(event_code):
        if event_code == int(EVENT.LV_EVENT_CLICKED):
            btn_clicks[0] += 1
            btn_count_label.label_set_text("Click: %d" % btn_clicks[0])
    btn.add_event_cb(int(EVENT.LV_EVENT_CLICKED), on_btn_click)

    # ================================================================
    # 2. Checkbox
    # ================================================================
    cb1 = lv.Checkbox(scr)
    cb1.set_text("Option A")
    cb1.align(ALIGN.LV_ALIGN_TOP_LEFT, 15, 90)

    cb2 = lv.Checkbox(scr)
    cb2.set_text("Option B")
    cb2.align_to(cb1, ALIGN.LV_ALIGN_OUT_BOTTOM_LEFT, 0, 5)

    cb_label = lv.Label(scr)
    cb_label.label_set_text("Checkboxes above")
    cb_label.align_to(cb2, ALIGN.LV_ALIGN_OUT_RIGHT_MID, 15, -10)

    # ================================================================
    # 3. Slider
    # ================================================================
    slider = lv.Slider(scr)
    slider.slider_set_range(0, 100)
    slider.slider_set_value(30, False)
    slider.set_width(160)
    slider.align(ALIGN.LV_ALIGN_TOP_LEFT, 15, 180)

    slider_label = lv.Label(scr)
    slider_label.label_set_text("30")
    slider_label.align_to(slider, ALIGN.LV_ALIGN_OUT_RIGHT_MID, 10, 0)

    slider_val = [30]
    def on_slider_change(event_code):
        if event_code == int(EVENT.LV_EVENT_VALUE_CHANGED):
            pass
    slider.add_event_cb(int(EVENT.LV_EVENT_VALUE_CHANGED), on_slider_change)

    # Slider +/- buttons (since touch drag may not work with current EVDEV)
    btn_s_minus = lv.Button(scr)
    btn_s_minus.set_size(35, 30)
    btn_s_minus.align_to(slider, ALIGN.LV_ALIGN_OUT_BOTTOM_LEFT, 0, 5)
    lbl_minus = lv.Label(btn_s_minus)
    lbl_minus.label_set_text("-")
    lbl_minus.center()

    btn_s_plus = lv.Button(scr)
    btn_s_plus.set_size(35, 30)
    btn_s_plus.align_to(slider, ALIGN.LV_ALIGN_OUT_BOTTOM_RIGHT, 0, 5)
    lbl_plus = lv.Label(btn_s_plus)
    lbl_plus.label_set_text("+")
    lbl_plus.center()

    def on_slider_minus(event_code):
        if event_code == int(EVENT.LV_EVENT_CLICKED):
            slider_val[0] = max(0, slider_val[0] - 10)
            slider.slider_set_value(slider_val[0], True)
            slider_label.label_set_text(str(slider_val[0]))
    def on_slider_plus(event_code):
        if event_code == int(EVENT.LV_EVENT_CLICKED):
            slider_val[0] = min(100, slider_val[0] + 10)
            slider.slider_set_value(slider_val[0], True)
            slider_label.label_set_text(str(slider_val[0]))
    btn_s_minus.add_event_cb(int(EVENT.LV_EVENT_CLICKED), on_slider_minus)
    btn_s_plus.add_event_cb(int(EVENT.LV_EVENT_CLICKED), on_slider_plus)

    # ================================================================
    # 4. Bar (non-interactive progress bar)
    # ================================================================
    bar = lv.Bar(scr)
    bar.bar_set_range(0, 100)
    bar.bar_set_value(60, False)
    bar.set_size(160, 12)
    bar.align(ALIGN.LV_ALIGN_TOP_LEFT, 15, 260)

    bar_label = lv.Label(scr)
    bar_label.label_set_text("Bar: 60%")
    bar_label.align_to(bar, ALIGN.LV_ALIGN_OUT_RIGHT_MID, 10, 0)

    # ================================================================
    # 5. Arc (circular indicator)
    # ================================================================
    arc = lv.Arc(scr)
    arc.set_range(0, 100)
    arc.set_value(70)
    arc.set_angles(135, 45)
    arc.set_size(80, 80)
    arc.align(ALIGN.LV_ALIGN_TOP_RIGHT, -100, 50)

    arc_label = lv.Label(scr)
    arc_label.label_set_text("Arc: 70")
    arc_label.align_to(arc, ALIGN.LV_ALIGN_OUT_BOTTOM_MID, 0, 5)

    # ================================================================
    # 6. Switch (toggle)
    # ================================================================
    sw = lv.Switch(scr)
    sw.set_size(50, 28)
    sw.align(ALIGN.LV_ALIGN_TOP_RIGHT, -100, 160)

    sw_label = lv.Label(scr)
    sw_label.label_set_text("OFF")
    sw_label.align_to(sw, ALIGN.LV_ALIGN_OUT_RIGHT_MID, 10, 0)

    def on_switch(event_code):
        if event_code == int(EVENT.LV_EVENT_VALUE_CHANGED):
            sw_label.label_set_text("Toggle!")
    sw.add_event_cb(int(EVENT.LV_EVENT_VALUE_CHANGED), on_switch)

    # ================================================================
    # 7. Spinner (loading indicator)
    # ================================================================
    spinner = lv.Spinner(scr)
    spinner.set_size(50, 50)
    spinner.set_anim_params(1000, 60)
    spinner.align(ALIGN.LV_ALIGN_TOP_RIGHT, -100, 210)

    spin_label = lv.Label(scr)
    spin_label.label_set_text("Spinner")
    spin_label.align_to(spinner, ALIGN.LV_ALIGN_OUT_BOTTOM_MID, 0, 5)

    # ================================================================
    # 8. Dropdown (selector)
    # ================================================================
    dd = lv.Dropdown(scr)
    dd.set_options("Red\nGreen\nBlue\nYellow")
    dd.set_size(120, 35)
    dd.align(ALIGN.LV_ALIGN_TOP_RIGHT, -100, 290)

    dd_label = lv.Label(scr)
    dd_label.label_set_text("Dropdown above")
    dd_label.align_to(dd, ALIGN.LV_ALIGN_OUT_TOP_MID, 0, -5)

    # ================================================================
    # 9. Textarea (text input)
    # ================================================================
    ta = lv.Textarea(scr)
    ta.set_size(160, 35)
    ta.set_one_line(True)
    ta.textarea_set_text("Hello K230")
    ta.set_placeholder_text("Type here...")
    ta.align(ALIGN.LV_ALIGN_BOTTOM_LEFT, 15, -15)

    ta_label = lv.Label(scr)
    ta_label.label_set_text("Textarea")
    ta_label.align_to(ta, ALIGN.LV_ALIGN_OUT_TOP_MID, 0, -5)

    # ================================================================
    # 10. LED indicator
    # ================================================================
    led = lv.Led(scr)
    led.set_size(30, 30)
    led.set_brightness(255)
    led.align(ALIGN.LV_ALIGN_BOTTOM_RIGHT, -100, -60)

    led_label = lv.Label(scr)
    led_label.label_set_text("LED")
    led_label.align_to(led, ALIGN.LV_ALIGN_OUT_BOTTOM_MID, 0, 5)

    # Toggle LED with button
    btn_led = lv.Button(scr)
    btn_led.set_size(60, 30)
    btn_led.align_to(led, ALIGN.LV_ALIGN_OUT_RIGHT_MID, 10, 0)
    lbl_led = lv.Label(btn_led)
    lbl_led.label_set_text("Toggle")
    lbl_led.center()

    led_on = [True]
    def on_toggle_led(event_code):
        if event_code == int(EVENT.LV_EVENT_CLICKED):
            if led_on[0]:
                led.off()
                led.set_brightness(30)
            else:
                led.on()
                led.set_brightness(255)
            led_on[0] = not led_on[0]
    btn_led.add_event_cb(int(EVENT.LV_EVENT_CLICKED), on_toggle_led)

    # ================================================================
    # Main loop
    # ================================================================
    print("LVGL demo running... Press Ctrl+C to exit")
    try:
        while True:
            idle_time = lv.timer_handler()
            time.sleep(idle_time / 1000.0)
    except KeyboardInterrupt:
        print("\nExiting...")
        os._exit(0)

if __name__ == "__main__":
    main()
