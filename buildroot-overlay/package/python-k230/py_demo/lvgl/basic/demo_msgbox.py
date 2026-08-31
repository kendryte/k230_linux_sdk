#!/usr/bin/env python3
"""Msgbox demo - modal message box with close button and footer buttons"""

import lvgl as lv

# Optional rotation before lv.init(): _0 / _90 / _180 / _270
lv.settings.rotation = lv.DISPLAY_ROTATION._0
lv.init()

# Create a modal message box (parent=None → uses top layer)
mbox = lv.msgbox()
mbox.add_title("Hello")
mbox.add_text("This is a message box with two buttons.")
mbox.add_close_button()

# Add footer buttons
btn_apply = mbox.add_footer_button("Apply")
btn_cancel = mbox.add_footer_button("Cancel")

def on_footer_btn(event):
    if event.code == lv.EVENT.CLICKED:
        mbox.close_async()

btn_apply.add_event_cb(lv.EVENT.CLICKED, on_footer_btn)
btn_cancel.add_event_cb(lv.EVENT.CLICKED, on_footer_btn)

lv.run()
