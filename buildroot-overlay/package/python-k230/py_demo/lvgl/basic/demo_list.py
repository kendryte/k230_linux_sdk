#!/usr/bin/env python3
"""List demo - icons and text items"""
import os, time
import lvgl as lv

lv.k230_init()
scr = lv.screen_active()

list_obj = lv.List(scr)
list_obj.set_size(180, 200)
list_obj.center()

list_obj.add_text("Fruits")
list_obj.add_button(lv.LV_STR_SYMBOL_FILE, "Apple")
list_obj.add_button(lv.LV_STR_SYMBOL_FILE, "Banana")
list_obj.add_button(lv.LV_STR_SYMBOL_FILE, "Orange")

list_obj.add_text("Settings")
list_obj.add_button(lv.LV_STR_SYMBOL_SETTINGS, "General")
list_obj.add_button(lv.LV_STR_SYMBOL_WIFI, "Network")

try:
    while True:
        idle = lv.timer_handler()
        time.sleep(idle / 1000.0)
except KeyboardInterrupt:
    os._exit(0)
