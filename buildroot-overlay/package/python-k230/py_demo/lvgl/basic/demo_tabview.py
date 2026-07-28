#!/usr/bin/env python3
"""Tabview demo - three tab pages"""

import lvgl as lv

lv.init()
scr = lv.screen_active()

tv = lv.tabview(scr)
tv.set_size(260, 200)
tv.center()

tab1 = tv.add_tab("Page 1")
tab2 = tv.add_tab("Page 2")
tab3 = tv.add_tab("Page 3")

lbl1 = lv.label(tab1)
lbl1.set_text("Content of Page 1")
lbl1.center()

lbl2 = lv.label(tab2)
lbl2.set_text("Content of Page 2")
lbl2.center()

lbl3 = lv.label(tab3)
lbl3.set_text("Content of Page 3")
lbl3.center()

lv.run()
