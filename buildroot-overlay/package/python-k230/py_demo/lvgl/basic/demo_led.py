#!/usr/bin/env python3
"""Led demo - toggle LED on/off"""

import lvgl as lv

# Optional rotation before lv.init(): _0 / _90 / _180 / _270
lv.settings.rotation = lv.DISPLAY_ROTATION._0
lv.init()
scr = lv.screen_active()

led = lv.led(scr)
led.set_size(30, 30)
led.set_color(lv.color(0, 255, 0))
led.set_brightness(255)
led.center()

led_label = lv.label(scr)
led_label.set_text("ON")
led_label.align_to(led, lv.ALIGN.OUT_RIGHT_MID, 10, 0)

led_on = [True]
def on_toggle(event):
    if event.code == lv.EVENT.CLICKED:
        if led_on[0]:
            led.off()
            led.set_brightness(30)
            led_label.set_text("OFF")
        else:
            led.on()
            led.set_brightness(255)
            led_label.set_text("ON")
        led_on[0] = not led_on[0]

btn = lv.button(scr)
btn.set_size(70, 30)
btn.align_to(led, lv.ALIGN.OUT_BOTTOM_MID, 0, 15)
lbl = lv.label(btn)
lbl.set_text("Toggle")
lbl.center()
btn.add_event_cb(lv.EVENT.CLICKED, on_toggle)

lv.run()
