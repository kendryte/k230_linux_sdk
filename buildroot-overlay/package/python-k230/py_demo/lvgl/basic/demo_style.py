#!/usr/bin/env python3
"""Style demo - gradient, rounded corners, shadow"""

import lvgl as lv

lv.init()
scr = lv.screen_active()

scr.set_style_bg_color(lv.color(30, 30, 60), lv.PART.MAIN)
scr.set_style_bg_grad_color(lv.color(60, 30, 90), lv.PART.MAIN)
scr.set_style_bg_grad_dir(lv.GRAD_DIR.VER, lv.PART.MAIN)

btn = lv.Button(scr)
btn.set_size(150, 50)
btn.center()
btn.set_style_radius(15, lv.PART.MAIN)
btn.set_style_bg_color(lv.color(0, 120, 215), lv.PART.MAIN)
btn.set_style_bg_grad_color(lv.color(0, 80, 180), lv.PART.MAIN)
btn.set_style_bg_grad_dir(lv.GRAD_DIR.VER, lv.PART.MAIN)
btn.set_style_shadow_width(15, lv.PART.MAIN)
btn.set_style_shadow_color(lv.color(0, 0, 0), lv.PART.MAIN)
btn.set_style_shadow_opa(lv.OPA_50, lv.PART.MAIN)
btn.set_style_shadow_offset_y(5, lv.PART.MAIN)

label = lv.Label(btn)
label.set_text("Styled Button")
label.set_style_text_color(lv.color_white(), lv.PART.MAIN)
label.center()

card = scr.obj()
card.set_size(200, 100)
card.align(lv.ALIGN.CENTER, 0, 80)
card.set_style_radius(10, lv.PART.MAIN)
card.set_style_border_width(2, lv.PART.MAIN)
card.set_style_border_color(lv.color(100, 100, 200), lv.PART.MAIN)
card.set_style_bg_opa(lv.OPA_70, lv.PART.MAIN)
card.set_style_bg_color(lv.color(40, 40, 80), lv.PART.MAIN)
card.set_style_pad_top(10, lv.PART.MAIN)
card.set_style_pad_bottom(10, lv.PART.MAIN)
card.set_style_pad_left(10, lv.PART.MAIN)
card.set_style_pad_right(10, lv.PART.MAIN)

card_text = lv.Label(card)
card_text.set_text("Card Content")
card_text.set_style_text_color(lv.color_white(), lv.PART.MAIN)
card_text.center()

lv.run()
