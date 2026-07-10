#!/usr/bin/env python3
"""Msgbox demo - message box with title, text and close button"""
import os, time
import lvgl as lv

lv.k230_init()
scr = lv.screen_active()

msgbox = lv.Msgbox(scr)
msgbox.add_title("Welcome")
msgbox.add_text("This is a message box demo.")
msgbox.add_close_button()
msgbox.center()

try:
    while True:
        idle = lv.timer_handler()
        time.sleep(idle / 1000.0)
except KeyboardInterrupt:
    os._exit(0)
