#!/usr/bin/env python3
"""Style demo - gradient, rounded corners, shadow"""
import os, time
import lvgl as lv

lv.k230_init()
scr = lv.screen_active()

scr.set_style_bg_color(lv.color(30, 30, 60), 0)
scr.set_style_bg_grad_color(lv.color(60, 30, 90), 0)
scr.set_style_bg_grad_dir(lv.GRAD_DIR.LV_GRAD_DIR_VER, 0)

btn = lv.Button(scr)
btn.set_size(150, 50)
btn.center()
btn.set_style_radius(15, 0)
btn.set_style_bg_color(lv.color(0, 120, 215), 0)
btn.set_style_bg_grad_color(lv.color(0, 80, 180), 0)
btn.set_style_bg_grad_dir(lv.GRAD_DIR.LV_GRAD_DIR_VER, 0)
btn.set_style_shadow_width(15, 0)
btn.set_style_shadow_color(lv.color(0, 0, 0), 0)
btn.set_style_shadow_opa(lv.LV_OPA_50, 0)
btn.set_style_shadow_offset_y(5, 0)

label = lv.Label(btn)
label.label_set_text("Styled Button")
label.set_style_text_color(lv.color_white(), 0)
label.center()

card = lv.Obj.create(scr)
card.set_size(200, 100)
card.align(lv.ALIGN.LV_ALIGN_CENTER, 0, 80)
card.set_style_radius(10, 0)
card.set_style_border_width(2, 0)
card.set_style_border_color(lv.color(100, 100, 200), 0)
card.set_style_bg_opa(lv.LV_OPA_70, 0)
card.set_style_bg_color(lv.color(40, 40, 80), 0)
card.set_style_pad_top(10, 0)
card.set_style_pad_bottom(10, 0)
card.set_style_pad_left(10, 0)
card.set_style_pad_right(10, 0)

card_text = lv.Label(card)
card_text.label_set_text("Card Content")
card_text.set_style_text_color(lv.color_white(), 0)
card_text.center()

try:
    while True:
        idle = lv.timer_handler()
        time.sleep(idle / 1000.0)
except KeyboardInterrupt:
    os._exit(0)
