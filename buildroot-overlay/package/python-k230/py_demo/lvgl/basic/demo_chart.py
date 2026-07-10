#!/usr/bin/env python3
"""Chart demo - line chart with live data"""
import os, time
import lvgl as lv

lv.k230_init()
scr = lv.screen_active()

# ---- 折线图 + 实时数据 ----
chart = lv.Chart(scr)
chart.set_size(240, 160)
chart.center()
chart.set_type(lv.CHART_TYPE.LV_CHART_TYPE_LINE)
chart.set_div_line_count(5, 5)
chart.chart_set_point_count(20)

# 添加两条数据线
ser1 = chart.chart_add_series(lv.color(0, 180, 0), lv.CHART_AXIS.LV_CHART_AXIS_PRIMARY_Y)   # 绿色
ser2 = chart.chart_add_series(lv.color(255, 80, 0), lv.CHART_AXIS.LV_CHART_AXIS_PRIMARY_Y)   # 橙色

# 初始化数据
data1 = [30, 45, 35, 60, 50, 70, 55, 65, 80, 75,
         60, 50, 65, 70, 55, 45, 60, 75, 65, 55]
data2 = [20, 25, 30, 35, 40, 30, 35, 45, 40, 50,
         45, 55, 50, 40, 35, 45, 50, 40, 35, 30]

for v in data1:
    chart.chart_set_next_value(ser1, v)
for v in data2:
    chart.chart_set_next_value(ser2, v)
chart.chart_refresh()

# 样式
chart.set_style_bg_color(lv.color(20, 20, 40), lv.PART.LV_PART_MAIN)
chart.set_style_bg_opa(lv.LV_OPA_100, lv.PART.LV_PART_MAIN)
chart.set_style_radius(8, lv.PART.LV_PART_MAIN)
chart.set_style_line_color(lv.color(60, 60, 80), lv.PART.LV_PART_MAIN)
chart.set_style_line_width(1, lv.PART.LV_PART_MAIN)

# 图例
legend1 = lv.Label(scr)
legend1.label_set_text("Series 1 (green)")
legend1.set_style_text_color(lv.color(0, 180, 0), 0)
legend1.align_to(chart, lv.ALIGN.LV_ALIGN_OUT_BOTTOM_LEFT, 5, 5)

legend2 = lv.Label(scr)
legend2.label_set_text("Series 2 (orange)")
legend2.set_style_text_color(lv.color(255, 80, 0), 0)
legend2.align_to(legend1, lv.ALIGN.LV_ALIGN_OUT_RIGHT_MID, 15, 0)

# 动态追加数据的按钮
btn = lv.Button(scr)
btn.set_size(100, 30)
btn.align(lv.ALIGN.LV_ALIGN_BOTTOM_MID, 0, -10)
btn_label = lv.Label(btn)
btn_label.label_set_text("Add Data")
btn_label.center()

idx = [0]
def on_add(event_code):
    if event_code == int(lv.EVENT_CODE.LV_EVENT_CLICKED):
        idx[0] += 1
        import math
        v1 = int(50 + 30 * math.sin(idx[0] * 0.5))
        v2 = int(35 + 20 * math.cos(idx[0] * 0.3))
        chart.chart_set_next_value(ser1, v1)
        chart.chart_set_next_value(ser2, v2)
        chart.chart_refresh()

btn.add_event_cb(int(lv.EVENT_CODE.LV_EVENT_CLICKED), on_add)

try:
    while True:
        idle = lv.timer_handler()
        time.sleep(idle / 1000.0)
except KeyboardInterrupt:
    os._exit(0)
