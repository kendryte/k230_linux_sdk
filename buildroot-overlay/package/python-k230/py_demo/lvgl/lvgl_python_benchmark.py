#!/usr/bin/env python3
"""
LVGL Python Benchmark Demo for K230

A Python port of the LVGL widgets demo (lv_demo_widgets.c),
showcasing a wide variety of LVGL widgets to benchmark rendering
performance on the K230 platform.

Tabs:
  - Profile:  avatar, labels, buttons, textarea, dropdown, slider, switch
  - Analytics: charts, scales, arcs with animations
  - Shop:      stacked chart, product list, checkboxes

Usage:
    python3 lvgl_python_benchmark.py
"""

import os
import random
import lvgl as lv
from k230_v4l2_drm import V4l2Drm, ROTATION_0, ROTATION_90, ROTATION_270

# ---------------------------------------------------------------------------
# Shortcuts
# ---------------------------------------------------------------------------
ALIGN    = lv.ALIGN
EVENT    = lv.EVENT
OBJ_FLAG = lv.OBJ_FLAG

# Display size category (mirrors the C demo's DISP_SMALL / DISP_MEDIUM / DISP_LARGE)
# NOTE: These are read at module load time; display may not exist yet.
# For runtime use, read from lv.display_get_default() inside functions.
_disp = lv.display_get_default()
HOR_RES = _disp.get_horizontal_resolution() if _disp else 800
VER_RES = _disp.get_vertical_resolution() if _disp else 480

if HOR_RES <= 320:
    DISP_SIZE = 0  # SMALL
elif HOR_RES < 720:
    DISP_SIZE = 1  # MEDIUM
else:
    DISP_SIZE = 2  # LARGE

# Primary theme color (blue by default)
PRIMARY_COLOR = lv.palette_main(lv.PALETTE.BLUE)


# ===========================================================================
# Profile tab
# ===========================================================================
def profile_create(parent, tv=None):
    """Create the Profile tab content – mirrors lv_demo_widgets_profile_create()."""

    # ---- Panel 1: Avatar / Name / Contact / Buttons ----
    panel1 = lv.obj(parent)
    panel1.set_height(lv.SIZE_CONTENT)
    panel1.set_flex_flow(lv.FLEX_FLOW.ROW_WRAP)
    panel1.set_flex_align(lv.FLEX_ALIGN.SPACE_EVENLY, lv.FLEX_ALIGN.CENTER, lv.FLEX_ALIGN.CENTER)
    panel1.set_style_pad_all(10, lv.SELECTOR.DEFAULT)

    # Avatar placeholder (colored circle with initials)
    avatar = lv.obj(panel1)
    avatar.set_size(60, 60)
    avatar.set_style_radius(30, lv.SELECTOR.DEFAULT)
    avatar.set_style_bg_color(lv.palette_main(lv.PALETTE.BLUE), lv.SELECTOR.DEFAULT)
    avatar.set_style_bg_opa(lv.OPA_COVER, lv.SELECTOR.DEFAULT)
    avatar.set_style_border_width(0, lv.SELECTOR.DEFAULT)
    avatar_label = lv.label(avatar)
    avatar_label.set_text("ES")
    avatar_label.set_style_text_color(lv.color_white(), lv.SELECTOR.DEFAULT)
    avatar_label.center()

    # Name and description
    name = lv.label(panel1)
    name.set_text("Elena Smith")

    dsc = lv.label(panel1)
    dsc.set_text("This is a short description of me. Take a look at my profile!")
    dsc.set_long_mode(lv.LABEL_LONG.WRAP)
    dsc.set_style_text_opa(lv.OPA_50, lv.SELECTOR.DEFAULT)

    # Contact info
    email_icn = lv.label(panel1)
    email_icn.set_text(lv.SYMBOL_ENVELOPE)
    email_icn.set_style_text_color(PRIMARY_COLOR, lv.SELECTOR.DEFAULT)

    email_label = lv.label(panel1)
    email_label.set_text("elena@smith.com")

    call_icn = lv.label(panel1)
    call_icn.set_text(lv.SYMBOL_CALL)
    call_icn.set_style_text_color(PRIMARY_COLOR, lv.SELECTOR.DEFAULT)

    call_label = lv.label(panel1)
    call_label.set_text("+79 246 123 4567")

    # Buttons
    log_out_btn = lv.button(panel1)
    log_out_btn.set_height(lv.SIZE_CONTENT)
    log_out_lbl = lv.label(log_out_btn)
    log_out_lbl.set_text("Log out")
    log_out_lbl.center()

    invite_btn = lv.button(panel1)
    invite_btn.add_state(lv.STATE.DISABLED)
    invite_btn.set_height(lv.SIZE_CONTENT)
    invite_lbl = lv.label(invite_btn)
    invite_lbl.set_text("Invite")
    invite_lbl.center()

    # ---- Panel 2: Form fields ----
    panel2 = lv.obj(parent)
    panel2.set_height(lv.SIZE_CONTENT)
    panel2.set_flex_flow(lv.FLEX_FLOW.COLUMN)
    panel2.set_flex_align(lv.FLEX_ALIGN.START, lv.FLEX_ALIGN.START, lv.FLEX_ALIGN.START)
    panel2.set_style_pad_all(10, lv.SELECTOR.DEFAULT)

    panel2_title = lv.label(panel2)
    panel2_title.set_text("Your profile")

    user_name_label = lv.label(panel2)
    user_name_label.set_text("User name")
    user_name_label.set_style_text_opa(lv.OPA_50, lv.SELECTOR.DEFAULT)

    user_name = lv.textarea(panel2)
    user_name.set_one_line(True)
    user_name.set_placeholder_text("Your name")

    password_label = lv.label(panel2)
    password_label.set_text("Password")
    password_label.set_style_text_opa(lv.OPA_50, lv.SELECTOR.DEFAULT)

    password = lv.textarea(panel2)
    password.set_one_line(True)
    password.set_password_mode(True)
    password.set_placeholder_text("Min. 8 chars.")

    gender_label = lv.label(panel2)
    gender_label.set_text("Gender")
    gender_label.set_style_text_opa(lv.OPA_50, lv.SELECTOR.DEFAULT)

    gender = lv.dropdown(panel2)
    gender.set_options("Male\nFemale\nOther")

    birthday_label = lv.label(panel2)
    birthday_label.set_text("Birthday")
    birthday_label.set_style_text_opa(lv.OPA_50, lv.SELECTOR.DEFAULT)

    birthdate = lv.textarea(panel2)
    birthdate.set_one_line(True)
    birthdate.set_text("01.01.1990")

    # ---- Virtual keyboard for textareas (mirrors C demo ta_event_cb) ----
    kb = lv.keyboard(lv.screen_active())
    kb.add_flag(OBJ_FLAG.HIDDEN)

    # ---- Date picker for birthdate (replaces C demo's lv_calendar) ----
    # lv_calendar and lv_layer_top are not exposed in the Python binding,
    # so we use a dropdown-based date picker on screen_active() instead.
    date_picker_obj = [None]  # mutable container for closure

    def show_date_picker(event):
        if event.code != EVENT.FOCUSED:
            return
        # Hide keyboard if visible
        kb.add_flag(OBJ_FLAG.HIDDEN)
        disp = lv.display_get_default()
        ver = disp.get_vertical_resolution() if disp else 480
        if tv is not None:
            tv.set_height(ver)

        if date_picker_obj[0] is not None:
            return  # already showing

        scr = lv.screen_active()

        # Dimmed background overlay
        bg = lv.obj(scr)
        bg.set_size(lv.pct(100), lv.pct(100))
        bg.set_style_bg_opa(lv.OPA_50, lv.SELECTOR.DEFAULT)
        bg.set_style_bg_color(lv.palette_main(lv.PALETTE.GREY), lv.SELECTOR.DEFAULT)
        bg.set_style_border_width(0, lv.SELECTOR.DEFAULT)

        cont = lv.obj(bg)
        cont.set_size(280, 200)
        cont.align(ALIGN.CENTER, 0, 0)
        cont.set_flex_flow(lv.FLEX_FLOW.COLUMN)
        cont.set_flex_align(lv.FLEX_ALIGN.CENTER, lv.FLEX_ALIGN.CENTER, lv.FLEX_ALIGN.CENTER)
        cont.set_style_pad_all(15, lv.SELECTOR.DEFAULT)

        title = lv.label(cont)
        title.set_text("Select date")
        title.set_style_text_opa(lv.OPA_50, lv.SELECTOR.DEFAULT)

        row = lv.obj(cont)
        row.remove_style_all()
        row.set_flex_flow(lv.FLEX_FLOW.ROW)
        row.set_flex_align(lv.FLEX_ALIGN.CENTER, lv.FLEX_ALIGN.CENTER, lv.FLEX_ALIGN.CENTER)
        row.set_style_pad_column(8, lv.SELECTOR.DEFAULT)
        row.set_width(lv.pct(100))

        year_dd = lv.dropdown(row)
        year_dd.set_width(80)
        years = "\n".join(str(y) for y in range(1970, 2031))
        year_dd.set_options(years)
        year_dd.set_selected(20)  # 1990

        month_dd = lv.dropdown(row)
        month_dd.set_width(60)
        month_dd.set_options("01\n02\n03\n04\n05\n06\n07\n08\n09\n10\n11\n12")
        month_dd.set_selected(0)  # 01

        day_dd = lv.dropdown(row)
        day_dd.set_width(60)
        day_dd.set_options("\n".join("%02d" % d for d in range(1, 32)))
        day_dd.set_selected(0)  # 01

        def on_ok(event):
            y = year_dd.get_selected() + 1970
            m = month_dd.get_selected() + 1
            d = day_dd.get_selected() + 1
            birthdate.set_text("%02d.%02d.%d" % (d, m, y))
            close_date_picker()

        ok_btn = lv.button(cont)
        ok_btn.set_height(lv.SIZE_CONTENT)
        ok_lbl = lv.label(ok_btn)
        ok_lbl.set_text("OK")
        ok_lbl.center()
        ok_btn.add_event_cb(EVENT.CLICKED, on_ok)

        def on_bg_click(event):
            if event.code == EVENT.PRESSED:
                target = event.get_target()
                if target == bg:
                    close_date_picker()

        def close_date_picker():
            bg.delete_obj()
            date_picker_obj[0] = None

        bg.add_event_cb(EVENT.ALL, on_bg_click)
        date_picker_obj[0] = cont

    birthdate.add_event_cb(EVENT.ALL, show_date_picker)

    def ta_event_cb(event, ta=None):
        code = event.code
        if code == EVENT.FOCUSED:
            disp = lv.display_get_default()
            hor = disp.get_horizontal_resolution() if disp else 800
            ver = disp.get_vertical_resolution() if disp else 480

            kb.set_textarea(ta)
            kb.set_style_max_height(hor * 2 // 3, lv.SELECTOR.DEFAULT)
            kb.remove_flag(OBJ_FLAG.HIDDEN)
            if tv is not None:
                tv.update_layout()
                tv.set_height(ver - kb.get_height())
            ta.scroll_to_view_recursive(False)
        elif code == EVENT.DEFOCUSED:
            kb.add_flag(OBJ_FLAG.HIDDEN)
            if tv is not None:
                disp = lv.display_get_default()
                ver = disp.get_vertical_resolution() if disp else 480
                tv.set_height(ver)
        elif code == EVENT.READY or code == EVENT.CANCEL:
            kb.add_flag(OBJ_FLAG.HIDDEN)
            if tv is not None:
                disp = lv.display_get_default()
                ver = disp.get_vertical_resolution() if disp else 480
                tv.set_height(ver)

    user_name.add_event_cb(EVENT.ALL, lambda e: ta_event_cb(e, user_name))
    password.add_event_cb(EVENT.ALL, lambda e: ta_event_cb(e, password))
    # birthdate uses date picker, not keyboard

    # ---- Panel 3: Skills ----
    panel3 = lv.obj(parent)
    panel3.set_height(lv.SIZE_CONTENT)
    panel3.set_flex_flow(lv.FLEX_FLOW.COLUMN)
    panel3.set_flex_align(lv.FLEX_ALIGN.START, lv.FLEX_ALIGN.START, lv.FLEX_ALIGN.START)
    panel3.set_style_pad_all(10, lv.SELECTOR.DEFAULT)

    panel3_title = lv.label(panel3)
    panel3_title.set_text("Your skills")

    experience_label = lv.label(panel3)
    experience_label.set_text("Experience")
    experience_label.set_style_text_opa(lv.OPA_50, lv.SELECTOR.DEFAULT)

    slider1 = lv.slider(panel3)
    slider1.set_width(lv.pct(95))
    slider1.set_range(0, 100)
    slider1.set_value(30, False)

    slider_val_label = lv.label(panel3)
    slider_val_label.set_text("30 %")

    def on_slider(event):
        if event.code == EVENT.VALUE_CHANGED:
            v = slider1.get_value()
            slider_val_label.set_text("%d %%" % v)
    slider1.add_event_cb(EVENT.VALUE_CHANGED, on_slider)

    team_player_label = lv.label(panel3)
    team_player_label.set_text("Team player")
    team_player_label.set_style_text_opa(lv.OPA_50, lv.SELECTOR.DEFAULT)

    sw1 = lv.switch(panel3)

    hard_working_label = lv.label(panel3)
    hard_working_label.set_text("Hard-working")
    hard_working_label.set_style_text_opa(lv.OPA_50, lv.SELECTOR.DEFAULT)

    sw2 = lv.switch(panel3)

    # ---- Layout (flex-based, responsive) ----
    # Use COLUMN layout so all three panels stack vertically and are
    # reachable by scrolling. (ROW layout hid panel2/panel3 off-screen.)
    parent.set_flex_flow(lv.FLEX_FLOW.COLUMN)
    parent.set_flex_align(lv.FLEX_ALIGN.START, lv.FLEX_ALIGN.START, lv.FLEX_ALIGN.START)
    panel1.set_width(lv.pct(100))
    panel2.set_width(lv.pct(100))
    panel3.set_width(lv.pct(100))


# ===========================================================================
# Analytics tab
# ===========================================================================
# Globals for cross-callback access
chart1 = None
chart2 = None
ser1_handle = 0
ser3_handle = 0

session_desktop = [1000]
session_tablet  = [1000]
session_mobile  = [1000]
down1 = [False]
down2 = [False]
down3 = [False]


def _create_scale_box(parent, title_text, text1, text2, text3):
    """Helper: create a scale card with 3 colored bullet labels."""
    cont = lv.obj(parent)
    cont.set_height(lv.SIZE_CONTENT)
    cont.set_flex_grow(1)
    cont.set_flex_flow(lv.FLEX_FLOW.COLUMN)
    cont.set_flex_align(lv.FLEX_ALIGN.START, lv.FLEX_ALIGN.START, lv.FLEX_ALIGN.START)
    cont.set_style_pad_all(10, lv.SELECTOR.DEFAULT)

    title_lbl = lv.label(cont)
    title_lbl.set_text(title_text)

    scale = lv.scale(cont)
    scale.set_mode(lv.SCALE_MODE.ROUND_INNER)
    scale.set_width(lv.pct(100))

    # Bullet 1 (red)
    b1 = lv.obj(cont)
    b1.set_size(13, 13)
    b1.set_style_radius(lv.RADIUS_CIRCLE, lv.SELECTOR.DEFAULT)
    b1.set_style_bg_color(lv.palette_main(lv.PALETTE.RED), lv.SELECTOR.DEFAULT)
    b1.set_style_bg_opa(lv.OPA_COVER, lv.SELECTOR.DEFAULT)
    b1.set_style_border_width(0, lv.SELECTOR.DEFAULT)
    l1 = lv.label(cont)
    l1.set_text(text1)

    # Bullet 2 (blue)
    b2 = lv.obj(cont)
    b2.set_size(13, 13)
    b2.set_style_radius(lv.RADIUS_CIRCLE, lv.SELECTOR.DEFAULT)
    b2.set_style_bg_color(lv.palette_main(lv.PALETTE.BLUE), lv.SELECTOR.DEFAULT)
    b2.set_style_bg_opa(lv.OPA_COVER, lv.SELECTOR.DEFAULT)
    b2.set_style_border_width(0, lv.SELECTOR.DEFAULT)
    l2 = lv.label(cont)
    l2.set_text(text2)

    # Bullet 3 (green)
    b3 = lv.obj(cont)
    b3.set_size(13, 13)
    b3.set_style_radius(lv.RADIUS_CIRCLE, lv.SELECTOR.DEFAULT)
    b3.set_style_bg_color(lv.palette_main(lv.PALETTE.GREEN), lv.SELECTOR.DEFAULT)
    b3.set_style_bg_opa(lv.OPA_COVER, lv.SELECTOR.DEFAULT)
    b3.set_style_border_width(0, lv.SELECTOR.DEFAULT)
    l3 = lv.label(cont)
    l3.set_text(text3)

    return scale


def analytics_create(parent):
    """Create the Analytics tab content – mirrors lv_demo_widgets_analytics_create()."""
    global chart1, chart2, ser1_handle, ser3_handle

    parent.set_flex_flow(lv.FLEX_FLOW.ROW_WRAP)
    parent.set_flex_align(lv.FLEX_ALIGN.SPACE_EVENLY, lv.FLEX_ALIGN.START, lv.FLEX_ALIGN.START)

    # ---- Chart 1: Unique visitors (line) ----
    chart1_cont = lv.obj(parent)
    chart1_cont.set_height(lv.pct(100))
    chart1_cont.set_style_max_height(300, lv.SELECTOR.DEFAULT)
    chart1_cont.set_flex_grow(1)
    chart1_cont.set_flex_flow(lv.FLEX_FLOW.COLUMN)
    chart1_cont.set_style_pad_all(10, lv.SELECTOR.DEFAULT)

    chart1_title = lv.label(chart1_cont)
    chart1_title.set_text("Unique visitors")

    chart1 = lv.chart(chart1_cont)
    chart1.set_type(lv.CHART_TYPE.LINE)
    chart1.set_div_line_count(5, 12)
    chart1.set_point_count(12)
    chart1.set_width(lv.pct(100))
    chart1.set_height(180)

    ser1_handle = chart1.add_series(PRIMARY_COLOR, lv.CHART_AXIS.PRIMARY_Y)
    for _ in range(12):
        chart1.set_next_value(ser1_handle, random.randint(10, 80))
    chart1.refresh()

    # ---- Chart 2: Monthly revenue (bar) ----
    chart2_cont = lv.obj(parent)
    chart2_cont.set_height(lv.pct(100))
    chart2_cont.set_style_max_height(300, lv.SELECTOR.DEFAULT)
    chart2_cont.set_flex_grow(1)
    chart2_cont.set_flex_flow(lv.FLEX_FLOW.COLUMN)
    chart2_cont.set_style_pad_all(10, lv.SELECTOR.DEFAULT)

    chart2_title = lv.label(chart2_cont)
    chart2_title.set_text("Monthly revenue")

    chart2 = lv.chart(chart2_cont)
    chart2.set_type(lv.CHART_TYPE.BAR)
    chart2.set_div_line_count(5, 12)
    chart2.set_point_count(12)
    chart2.set_width(lv.pct(100))
    chart2.set_height(180)

    ser2_handle = chart2.add_series(lv.palette_main(lv.PALETTE.GREY), lv.CHART_AXIS.PRIMARY_Y)
    ser3_handle = chart2.add_series(PRIMARY_COLOR, lv.CHART_AXIS.PRIMARY_Y)
    for _ in range(12):
        chart2.set_next_value(ser2_handle, random.randint(10, 80))
        chart2.set_next_value(ser3_handle, random.randint(10, 80))
    chart2.refresh()

    # ---- Scale 1: Monthly Target (animated arcs using lv.anim_t) ----
    scale1 = _create_scale_box(parent, "Monthly Target", "Revenue: -", "Sales: -", "Costs: -")

    scale1.set_mode(lv.SCALE_MODE.ROUND_OUTER)
    scale1.set_style_pad_all(30, lv.SELECTOR.DEFAULT)

    arc1 = lv.arc(scale1)
    arc1.set_style_bg_opa(0, lv.PART.KNOB)
    arc1.set_style_arc_opa(0, lv.PART.MAIN)
    arc1.set_size(lv.pct(100), lv.pct(100))
    arc1.set_style_arc_width(15, lv.PART.INDICATOR)
    arc1.set_style_arc_color(lv.palette_main(lv.PALETTE.BLUE), lv.PART.INDICATOR)
    arc1.remove_flag(OBJ_FLAG.CLICKABLE)
    arc1.set_range(0, 100)
    arc1.set_value(20)

    arc2 = lv.arc(scale1)
    arc2.set_style_bg_opa(0, lv.PART.KNOB)
    arc2.set_size(lv.pct(100), lv.pct(100))
    arc2.set_style_margin_all(20, lv.SELECTOR.DEFAULT)
    arc2.set_style_arc_opa(0, lv.PART.MAIN)
    arc2.set_style_arc_width(15, lv.PART.INDICATOR)
    arc2.set_style_arc_color(lv.palette_main(lv.PALETTE.RED), lv.PART.INDICATOR)
    arc2.remove_flag(OBJ_FLAG.CLICKABLE)
    arc2.center()
    arc2.set_range(0, 100)
    arc2.set_value(20)

    arc3 = lv.arc(scale1)
    arc3.set_style_bg_opa(0, lv.PART.KNOB)
    arc3.set_size(lv.pct(100), lv.pct(100))
    arc3.set_style_margin_all(40, lv.SELECTOR.DEFAULT)
    arc3.set_style_arc_opa(0, lv.PART.MAIN)
    arc3.set_style_arc_width(15, lv.PART.INDICATOR)
    arc3.set_style_arc_color(lv.palette_main(lv.PALETTE.GREEN), lv.PART.INDICATOR)
    arc3.remove_flag(OBJ_FLAG.CLICKABLE)
    arc3.center()
    arc3.set_range(0, 100)
    arc3.set_value(20)

    # Animate arc1: 20→100→20, duration 4100ms / reverse 2700ms
    a1 = lv.anim_t()
    a1.set_var(arc1)
    a1.set_exec_cb(arc1, lambda var, v: var.set_value(v))
    a1.set_values(20, 100)
    a1.set_duration(4100)
    a1.set_reverse_duration(2700)
    a1.set_repeat_count(lv.ANIM_REPEAT_INFINITE)
    a1.start()

    # Animate arc2: 20→100→20, duration 2600ms / reverse 3200ms
    a2 = lv.anim_t()
    a2.set_var(arc2)
    a2.set_exec_cb(arc2, lambda var, v: var.set_value(v))
    a2.set_values(20, 100)
    a2.set_duration(2600)
    a2.set_reverse_duration(3200)
    a2.set_repeat_count(lv.ANIM_REPEAT_INFINITE)
    a2.start()

    # Animate arc3: 20→100→20, duration 2800ms / reverse 1800ms
    a3 = lv.anim_t()
    a3.set_var(arc3)
    a3.set_exec_cb(arc3, lambda var, v: var.set_value(v))
    a3.set_values(20, 100)
    a3.set_duration(2800)
    a3.set_reverse_duration(1800)
    a3.set_repeat_count(lv.ANIM_REPEAT_INFINITE)
    a3.start()

    # ---- Scale 2: Sessions (timer-driven arcs) ----
    scale2 = _create_scale_box(parent, "Sessions", "Desktop: -", "Tablet: -", "Mobile: -")

    scale2.set_angle_range(360)
    scale2.set_total_tick_count(11)
    scale2.set_style_length(30, lv.PART.INDICATOR)
    scale2.set_major_tick_every(1)

    s2_arc1 = lv.arc(scale2)
    s2_arc1.set_size(lv.pct(100), lv.pct(100))
    s2_arc1.set_style_margin_all(10, lv.SELECTOR.DEFAULT)
    s2_arc1.set_style_bg_opa(0, lv.PART.KNOB)
    s2_arc1.set_style_arc_opa(0, lv.PART.MAIN)
    s2_arc1.set_style_arc_width(10, lv.PART.INDICATOR)
    s2_arc1.set_style_arc_rounded(False, lv.PART.INDICATOR)
    s2_arc1.set_style_arc_color(lv.palette_main(lv.PALETTE.BLUE), lv.PART.INDICATOR)
    s2_arc1.remove_flag(OBJ_FLAG.CLICKABLE)
    s2_arc1.center()
    s2_arc1.set_range(0, 360)
    s2_arc1.set_value(120)

    s2_arc2 = lv.arc(scale2)
    s2_arc2.set_size(lv.pct(100), lv.pct(100))
    s2_arc2.set_style_margin_all(5, lv.SELECTOR.DEFAULT)
    s2_arc2.set_style_bg_opa(0, lv.PART.KNOB)
    s2_arc2.set_style_arc_opa(0, lv.PART.MAIN)
    s2_arc2.set_style_arc_width(20, lv.PART.INDICATOR)
    s2_arc2.set_style_arc_rounded(False, lv.PART.INDICATOR)
    s2_arc2.set_style_arc_color(lv.palette_main(lv.PALETTE.RED), lv.PART.INDICATOR)
    s2_arc2.remove_flag(OBJ_FLAG.CLICKABLE)
    s2_arc2.center()
    s2_arc2.set_range(0, 360)
    s2_arc2.set_value(120)

    s2_arc3 = lv.arc(scale2)
    s2_arc3.set_size(lv.pct(100), lv.pct(100))
    s2_arc3.set_style_bg_opa(0, lv.PART.KNOB)
    s2_arc3.set_style_arc_opa(0, lv.PART.MAIN)
    s2_arc3.set_style_arc_width(30, lv.PART.INDICATOR)
    s2_arc3.set_style_arc_rounded(False, lv.PART.INDICATOR)
    s2_arc3.set_style_arc_color(lv.palette_main(lv.PALETTE.GREEN), lv.PART.INDICATOR)
    s2_arc3.remove_flag(OBJ_FLAG.CLICKABLE)
    s2_arc3.center()
    s2_arc3.set_range(0, 360)
    s2_arc3.set_value(120)

    # Timer callback for scale2 arcs using lv.timer_create
    def scale2_timer_cb(timer):
        if down1[0]:
            session_desktop[0] -= 137
            if session_desktop[0] < 1400:
                down1[0] = False
        else:
            session_desktop[0] += 116
            if session_desktop[0] > 4500:
                down1[0] = True

        if down2[0]:
            session_tablet[0] -= 3
            if session_tablet[0] < 1400:
                down2[0] = False
        else:
            session_tablet[0] += 9
            if session_tablet[0] > 4500:
                down2[0] = True

        if down3[0]:
            session_mobile[0] -= 57
            if session_mobile[0] < 1400:
                down3[0] = False
        else:
            session_mobile[0] += 76
            if session_mobile[0] > 4500:
                down3[0] = True

        all_val = session_desktop[0] + session_tablet[0] + session_mobile[0]
        angle1 = (session_desktop[0] * 354) // all_val
        angle2 = (session_tablet[0] * 354) // all_val

        s2_arc1.set_angles(0, angle1)
        s2_arc2.set_angles(angle1 + 2, angle1 + 2 + angle2)
        s2_arc3.set_angles(angle1 + 2 + angle2 + 2, 358)

    lv.timer_create(scale2_timer_cb, 100)

    # ---- Scale 3: Network Speed (gauge-style with anim) ----
    scale3 = _create_scale_box(parent, "Network Speed", "Low speed", "Normal Speed", "High Speed")

    scale3.set_range(10, 60)
    scale3.set_total_tick_count(21)
    scale3.set_major_tick_every(4)
    scale3.set_style_length(10, lv.PART.ITEMS)
    scale3.set_style_length(20, lv.PART.INDICATOR)
    scale3.set_angle_range(225)
    scale3.set_rotation(135)

    # Needle indicator (line)
    needle_line = lv.line(scale3)
    needle_line.set_style_line_width(3, lv.SELECTOR.DEFAULT)
    needle_line.set_style_line_color(lv.palette_main(lv.PALETTE.RED), lv.SELECTOR.DEFAULT)
    needle_line.align(ALIGN.CENTER, 0, 0)

    mbps_label = lv.label(scale3)
    mbps_label.set_text("-")

    mbps_unit = lv.label(scale3)
    mbps_unit.set_text("Mbps")

    # Animate scale3 needle using lv.anim_t
    def scale3_anim_cb(var, v):
        try:
            scale3.set_line_needle_value(needle_line, 50, v)
        except Exception:
            pass
        mbps_label.set_text(str(v))

    a_s3 = lv.anim_t()
    a_s3.set_var(scale3)
    a_s3.set_exec_cb(scale3, scale3_anim_cb)
    a_s3.set_values(10, 60)
    a_s3.set_duration(4100)
    a_s3.set_reverse_duration(800)
    a_s3.set_repeat_count(lv.ANIM_REPEAT_INFINITE)
    a_s3.start()

    mbps_label.align(ALIGN.TOP_MID, 10, lv.pct(55))
    mbps_unit.align_to(mbps_label, ALIGN.OUT_RIGHT_BOTTOM, 10, 0)


# ===========================================================================
# Shop tab
# ===========================================================================
def _create_shop_item(parent, name, category, price):
    """Create a single shop item row – mirrors create_shop_item()."""
    cont = lv.obj(parent)
    cont.remove_style_all()
    cont.set_size(lv.pct(100), lv.SIZE_CONTENT)
    cont.set_flex_flow(lv.FLEX_FLOW.ROW)
    cont.set_flex_align(lv.FLEX_ALIGN.START, lv.FLEX_ALIGN.CENTER, lv.FLEX_ALIGN.CENTER)
    cont.set_style_pad_column(10, lv.SELECTOR.DEFAULT)
    cont.set_style_pad_row(5, lv.SELECTOR.DEFAULT)

    # Placeholder image (colored square)
    img = lv.obj(cont)
    img.set_size(40, 40)
    img.set_style_radius(5, lv.SELECTOR.DEFAULT)
    img.set_style_bg_color(lv.palette_main(lv.PALETTE.BLUE_GREY), lv.SELECTOR.DEFAULT)
    img.set_style_bg_opa(lv.OPA_COVER, lv.SELECTOR.DEFAULT)
    img.set_style_border_width(0, lv.SELECTOR.DEFAULT)

    # Text column
    text_col = lv.obj(cont)
    text_col.remove_style_all()
    text_col.set_flex_flow(lv.FLEX_FLOW.COLUMN)
    text_col.set_flex_grow(1)

    name_lbl = lv.label(text_col)
    name_lbl.set_text(name)

    cat_lbl = lv.label(text_col)
    cat_lbl.set_text(category)
    cat_lbl.set_style_text_opa(lv.OPA_50, lv.SELECTOR.DEFAULT)

    price_lbl = lv.label(cont)
    price_lbl.set_text(price)

    return cont


def shop_create(parent):
    """Create the Shop tab content – mirrors lv_demo_widgets_shop_create()."""
    parent.set_flex_flow(lv.FLEX_FLOW.ROW_WRAP)
    parent.set_flex_align(lv.FLEX_ALIGN.SPACE_EVENLY, lv.FLEX_ALIGN.START, lv.FLEX_ALIGN.START)

    # ---- Panel 1: Monthly Summary ----
    panel1 = lv.obj(parent)
    panel1.set_size(lv.pct(100), lv.SIZE_CONTENT)
    panel1.set_style_pad_bottom(30, lv.SELECTOR.DEFAULT)
    panel1.set_flex_flow(lv.FLEX_FLOW.COLUMN)
    panel1.set_style_pad_all(10, lv.SELECTOR.DEFAULT)

    title = lv.label(panel1)
    title.set_text("Monthly Summary")

    date = lv.label(panel1)
    date.set_text("8-15 July, 2021")
    date.set_style_text_opa(lv.OPA_50, lv.SELECTOR.DEFAULT)

    amount = lv.label(panel1)
    amount.set_text("$27,123.25")

    hint = lv.label(panel1)
    hint.set_text(lv.SYMBOL_UP + " 17% growth this week")
    hint.set_style_text_color(lv.palette_main(lv.PALETTE.GREEN), lv.SELECTOR.DEFAULT)

    chart3 = lv.chart(panel1)
    chart3.set_type(lv.CHART_TYPE.STACKED)
    chart3.set_div_line_count(6, 0)
    chart3.set_point_count(7)
    chart3.set_width(lv.pct(100))
    chart3.set_height(150)

    ser4 = chart3.add_series(lv.palette_main(lv.PALETTE.GREEN), lv.CHART_AXIS.PRIMARY_Y)
    ser5 = chart3.add_series(lv.palette_main(lv.PALETTE.BLUE), lv.CHART_AXIS.PRIMARY_Y)
    ser6 = chart3.add_series(lv.palette_main(lv.PALETTE.RED), lv.CHART_AXIS.PRIMARY_Y)

    for _ in range(8):
        chart3.set_next_value(ser4, random.randint(20, 40))
        chart3.set_next_value(ser5, random.randint(15, 30))
        chart3.set_next_value(ser6, random.randint(15, 30))
    chart3.refresh()

    # ---- Product list ----
    list_cont = lv.obj(parent)
    list_cont.set_height(lv.pct(100))
    list_cont.set_style_max_height(300, lv.SELECTOR.DEFAULT)
    list_cont.set_flex_flow(lv.FLEX_FLOW.COLUMN)
    list_cont.set_flex_grow(1)
    list_cont.add_flag(OBJ_FLAG.FLEX_IN_NEW_TRACK)
    list_cont.set_style_pad_all(10, lv.SELECTOR.DEFAULT)

    list_title = lv.label(list_cont)
    list_title.set_text("Top products")

    _create_shop_item(list_cont, "Blue T-shirt", "Clothes", "$722")
    _create_shop_item(list_cont, "Blue T-shirt", "Clothes", "$411")
    _create_shop_item(list_cont, "Blue T-shirt", "Clothes", "$917")
    _create_shop_item(list_cont, "Blue T-shirt", "Clothes", "$64")
    _create_shop_item(list_cont, "Blue T-shirt", "Clothes", "$805")

    # ---- Notifications ----
    notif_cont = lv.obj(parent)
    notif_cont.set_height(lv.pct(100))
    notif_cont.set_style_max_height(300, lv.SELECTOR.DEFAULT)
    notif_cont.set_flex_flow(lv.FLEX_FLOW.COLUMN)
    notif_cont.set_flex_grow(1)
    notif_cont.set_style_pad_all(10, lv.SELECTOR.DEFAULT)

    notif_title = lv.label(notif_cont)
    notif_title.set_text("Notification")

    cb1 = lv.checkbox(notif_cont)
    cb1.set_text("Item purchased")

    cb2 = lv.checkbox(notif_cont)
    cb2.set_text("New connection")

    cb3 = lv.checkbox(notif_cont)
    cb3.set_text("New subscriber")
    cb3.add_state(lv.STATE.CHECKED)

    cb4 = lv.checkbox(notif_cont)
    cb4.set_text("New message")
    cb4.add_state(lv.STATE.DISABLED)

    cb5 = lv.checkbox(notif_cont)
    cb5.set_text("Milestone reached")
    cb5.add_state(lv.STATE.CHECKED)
    cb5.add_state(lv.STATE.DISABLED)

    cb6 = lv.checkbox(notif_cont)
    cb6.set_text("Out of stock")


# ===========================================================================
# Color changer (floating palette switcher)
# ===========================================================================
PALETTE_LIST = [
    lv.PALETTE.BLUE, lv.PALETTE.GREEN, lv.PALETTE.BLUE_GREY,
    lv.PALETTE.ORANGE, lv.PALETTE.RED, lv.PALETTE.PURPLE, lv.PALETTE.TEAL,
]


def color_changer_create(parent):
    """Create the floating color palette switcher – mirrors color_changer_create()."""
    color_cont = lv.obj(parent)
    color_cont.remove_style_all()
    color_cont.set_flex_flow(lv.FLEX_FLOW.ROW)
    color_cont.set_flex_align(lv.FLEX_ALIGN.SPACE_EVENLY, lv.FLEX_ALIGN.CENTER, lv.FLEX_ALIGN.CENTER)
    color_cont.add_flag(OBJ_FLAG.FLOATING)

    color_cont.set_style_bg_color(lv.color_white(), lv.SELECTOR.DEFAULT)
    color_cont.set_style_bg_opa(lv.OPA_COVER, lv.SELECTOR.DEFAULT)
    color_cont.set_style_radius(lv.RADIUS_CIRCLE, lv.SELECTOR.DEFAULT)

    btn_size = 42 if DISP_SIZE == 0 else 50
    color_cont.set_size(btn_size, btn_size)
    color_cont.align(ALIGN.BOTTOM_RIGHT, -10, -10)

    for pal in PALETTE_LIST:
        c = lv.button(color_cont)
        c.set_style_bg_color(lv.palette_main(pal), lv.SELECTOR.DEFAULT)
        c.set_style_radius(lv.RADIUS_CIRCLE, lv.SELECTOR.DEFAULT)
        c.set_style_opa(lv.OPA_TRANSP, lv.SELECTOR.DEFAULT)
        c.set_size(20, 20)
        c.remove_flag(OBJ_FLAG.SCROLL_ON_FOCUS)

        # Capture palette value in closure
        def make_color_cb(palette_val):
            def color_cb(event):
                if event.code == EVENT.CLICKED:
                    # Update chart series colors
                    global ser1_handle, ser3_handle
                    new_color = lv.palette_main(palette_val)
                    try:
                        if chart1 and ser1_handle:
                            chart1.set_series_color(ser1_handle, new_color)
                        if chart2 and ser3_handle:
                            chart2.set_series_color(ser3_handle, new_color)
                    except Exception:
                        pass
            return color_cb
        c.add_event_cb(EVENT.ALL, make_color_cb(pal))

    # Tint button (toggle)
    tint_btn = lv.button(parent)
    tint_btn.add_flag(OBJ_FLAG.FLOATING | OBJ_FLAG.CLICKABLE)
    tint_btn.set_style_bg_color(lv.color_white(), lv.SELECTOR.DEFAULT)
    tint_btn.set_style_pad_all(10, lv.SELECTOR.DEFAULT)
    tint_btn.set_style_radius(lv.RADIUS_CIRCLE, lv.SELECTOR.DEFAULT)
    tint_btn.set_style_shadow_width(0, lv.SELECTOR.DEFAULT)

    tint_icon = lv.label(tint_btn)
    tint_icon.set_text(lv.SYMBOL_TINT)
    tint_icon.center()

    tint_size = 42 if DISP_SIZE == 0 else 50
    tint_btn.set_size(tint_size, tint_size)
    tint_btn.align(ALIGN.BOTTOM_RIGHT, -15, -15)

    expanded = [False]

    def tint_cb(event):
        if event.code == EVENT.CLICKED:
            if not expanded[0]:
                # Expand
                max_w = color_cont.get_parent().get_width() - 20
                color_cont.set_width(max_w)
                color_cont.align(ALIGN.BOTTOM_RIGHT, -10, -10)
                for i in range(color_cont.get_child_count()):
                    child = color_cont.get_child(i)
                    child.set_style_opa(lv.OPA_COVER, lv.SELECTOR.DEFAULT)
                expanded[0] = True
            else:
                # Collapse
                btn_size_inner = 42 if DISP_SIZE == 0 else 50
                color_cont.set_width(btn_size_inner)
                color_cont.align(ALIGN.BOTTOM_RIGHT, -10, -10)
                for i in range(color_cont.get_child_count()):
                    child = color_cont.get_child(i)
                    child.set_style_opa(lv.OPA_TRANSP, lv.SELECTOR.DEFAULT)
                expanded[0] = False

    tint_btn.add_event_cb(EVENT.ALL, tint_cb)


# ===========================================================================
# Main entry point
# ===========================================================================
def main():
    global HOR_RES, VER_RES, DISP_SIZE

    # ================================================================
    # 1. Setup v4l2-drm (camera input + DRM display)
    # ================================================================
    v4l2drm = V4l2Drm(context_num=1, osd=False)
    display_w, display_h = v4l2drm.drm_init(drm_id=0)
    if display_h < 0:
        print("[v4l2-drm] Failed to initialize DRM display")
        return
    print("[v4l2-drm] Display: %dx%d" % (display_w, display_h))

    v4l2drm.set_context(
        0, 1,
        width=max(display_w, display_h),
        height=min(display_w, display_h),
        format="NV12", display=True,
    )
    # 竖屏旋转90°，横屏不旋转
    rotation = ROTATION_90 if display_w < display_h else ROTATION_0
    print("[v4l2-drm] rotation=%s" % ("90°" if rotation == ROTATION_90 else "0°"))
    v4l2drm.set_rotation(0, rotation)
    v4l2drm.setup()

    # ================================================================
    # 2. Initialize LVGL + K230 driver
    # ================================================================
    lv.init(v4l2drm, v4l2_drm_run_flag=1)
    print("[lvgl] initialized")

    # Re-read display resolution after lv.init() (module-level values may be 0)
    _disp = lv.display_get_default()
    if _disp:
        HOR_RES = _disp.get_horizontal_resolution()
        VER_RES = _disp.get_vertical_resolution()
    if HOR_RES <= 320:
        DISP_SIZE = 0
    elif HOR_RES < 720:
        DISP_SIZE = 1
    else:
        DISP_SIZE = 2
    print("[lvgl] display: %dx%d DISP_SIZE=%d" % (HOR_RES, VER_RES, DISP_SIZE))

    # ---- Transparent background so camera video shows through ----
    # Screen + tabview + tab content all need transparent backgrounds,
    # otherwise the opaque theme default covers the video layer.
    scr = lv.screen_active()
    scr.set_style_bg_opa(lv.OPA_TRANSP, lv.PART.MAIN)

    # ---- Create Tabview ----
    tv = lv.tabview(scr)
    tab_bar_size = 75 if DISP_SIZE == 2 else 45
    tv.set_tab_bar_size(tab_bar_size)
    # Make tabview + its content area transparent so video shows through
    tv.set_style_bg_opa(lv.OPA_TRANSP, lv.PART.MAIN)
    tv.get_content().set_style_bg_opa(lv.OPA_TRANSP, lv.PART.MAIN)
    tv.get_tab_bar().set_style_bg_opa(lv.OPA_70, lv.PART.MAIN)  # tab bar keeps slight tint for readability

    t1 = tv.add_tab("Profile")
    t2 = tv.add_tab("Analytics")
    t3 = tv.add_tab("Shop")

    # ---- Populate tabs ----
    profile_create(t1, tv)
    analytics_create(t2)
    shop_create(t3)

    # ---- Color changer ----
    color_changer_create(tv)

    # ---- Main loop ----
    v4l2drm.display_start()

    print("LVGL benchmark demo running... Press Ctrl+C to exit")
    try:
        lv.run()
    except KeyboardInterrupt:
        print("\nExiting...")
        v4l2drm.display_stop()
        os._exit(0)


if __name__ == "__main__":
    main()
