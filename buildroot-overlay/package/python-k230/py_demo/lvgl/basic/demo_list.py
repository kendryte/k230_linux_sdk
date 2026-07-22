#!/usr/bin/env python3
"""List demo - icons and text items"""

import lvgl as lv

lv.init()
scr = lv.screen_active()

list_obj = lv.List(scr)
list_obj.set_size(180, 200)
list_obj.center()

list_obj.add_text("Fruits")
list_obj.add_button(lv.SYMBOL_FILE, "Apple")
list_obj.add_button(lv.SYMBOL_FILE, "Banana")
list_obj.add_button(lv.SYMBOL_FILE, "Orange")

list_obj.add_text("Settings")
list_obj.add_button(lv.SYMBOL_SETTINGS, "General")
list_obj.add_button(lv.SYMBOL_WIFI, "Network")

lv.run()
