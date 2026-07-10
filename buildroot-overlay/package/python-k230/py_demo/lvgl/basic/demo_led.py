#!/usr/bin/env python3
"""Led demo - toggle LED on/off"""
import os, time
import lvgl as lv

lv.k230_init()
scr = lv.screen_active()

led = lv.Led(scr)
led.set_size(30, 30)
led.set_color(lv.color(0, 255, 0))
led.set_brightness(255)
led.center()

led_label = lv.Label(scr)
led_label.label_set_text("ON")
led_label.align_to(led, lv.ALIGN.LV_ALIGN_OUT_RIGHT_MID, 10, 0)

led_on = [True]
def on_toggle(event_code):
    if event_code == int(lv.EVENT_CODE.LV_EVENT_CLICKED):
        if led_on[0]:
            led.off()
            led.set_brightness(30)
            led_label.label_set_text("OFF")
        else:
            led.on()
            led.set_brightness(255)
            led_label.label_set_text("ON")
        led_on[0] = not led_on[0]

btn = lv.Button(scr)
btn.set_size(70, 30)
btn.align_to(led, lv.ALIGN.LV_ALIGN_OUT_BOTTOM_MID, 0, 15)
lbl = lv.Label(btn)
lbl.label_set_text("Toggle")
lbl.center()
btn.add_event_cb(int(lv.EVENT_CODE.LV_EVENT_CLICKED), on_toggle)

try:
    while True:
        idle = lv.timer_handler()
        time.sleep(idle / 1000.0)
except KeyboardInterrupt:
    os._exit(0)
