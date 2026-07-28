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
    scr = lv.screen_active()
    btn = lv.button(scr)
    btn.set_size(120, 50)
    btn.center()

    label = lv.label(btn)
    label.set_text("Hello!")
    label.center()

    # Event handling with on() alias
    def on_click(e):
        print("Button clicked!")

    btn.on(lv.EVENT.CLICKED, on_click)

    # Style with named selector (no magic number)
    btn.set_style_bg_color(lv.color.red(), lv.SELECTOR.DEFAULT)

    # Main loop
    lv.run()  # or: while True: idle = lv.timer_handler(); time.sleep(idle / 1000.0)
"""

from ._wrapper import *
