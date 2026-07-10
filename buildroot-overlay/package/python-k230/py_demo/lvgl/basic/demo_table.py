#!/usr/bin/env python3
"""Table demo - simple data table"""
import os, time
import lvgl as lv

lv.k230_init()
scr = lv.screen_active()

table = lv.Table(scr)
table.set_row_count(4)
table.set_column_count(2)
table.set_column_width(0, 80)
table.set_column_width(1, 80)
table.center()

table.set_cell_value(0, 0, "Name")
table.set_cell_value(0, 1, "Value")
table.set_cell_value(1, 0, "CPU")
table.set_cell_value(1, 1, "800MHz")
table.set_cell_value(2, 0, "RAM")
table.set_cell_value(2, 1, "256MB")
table.set_cell_value(3, 0, "Display")
table.set_cell_value(3, 1, "800x480")

try:
    while True:
        idle = lv.timer_handler()
        time.sleep(idle / 1000.0)
except KeyboardInterrupt:
    os._exit(0)
