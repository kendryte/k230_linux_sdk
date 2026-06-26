"""
LVGL Python bindings for K230

This module provides Python access to LVGL (Light and Versatile Graphics Library)
for creating graphical user interfaces on the K230 platform.

The bindings are auto-generated from LVGL header files by gen_pybind11.py.

Example usage:
    import lvgl as lv

    # Initialize LVGL
    lv.init()

    # Create a display (using DRM or framebuffer)
    disp = lv.display_create(800, 480)

    # Create a screen with a button
    scr = lv.Obj.screen_active()
    btn = lv.Button(scr)
    btn.set_size(120, 50)
    btn.center()

    label = lv.Label(btn)
    label.set_text("Hello!")
    label.center()

    # Main loop
    while True:
        lv.timer_handler()
        time.sleep(0.01)
"""

from ._lvgl import *
