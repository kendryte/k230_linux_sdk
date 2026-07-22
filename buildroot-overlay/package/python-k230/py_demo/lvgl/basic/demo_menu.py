#!/usr/bin/env python3
"""Menu demo - sidebar and page navigation"""
import lvgl as lv

lv.init()
scr = lv.screen_active()

menu = lv.Menu(scr)
menu.set_size(260, 240)
menu.center()

# 创建主页面 (MenuPage 是 menu 的子页面容器)
main_page = lv.MenuPage(menu, "Main")
cont = lv.MenuCont(main_page)
label = lv.Label(cont)
label.set_text("Menu Main Page")

# 创建子页面
sub_page = lv.MenuPage(menu, "Sub Page")
sub_cont = lv.MenuCont(sub_page)
sub_label = lv.Label(sub_cont)
sub_label.set_text("This is a sub page")

# 在主页面添加一个可点击的条目，点击后跳转子页面
btn_cont = lv.MenuCont(main_page)
btn_label = lv.Label(btn_cont)
btn_label.set_text("Go to Sub Page >")
menu.set_load_page_event(btn_cont, sub_page)

menu.set_page(main_page)

lv.run()
