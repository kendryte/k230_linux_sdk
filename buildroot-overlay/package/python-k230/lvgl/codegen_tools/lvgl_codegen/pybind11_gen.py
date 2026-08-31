"""
IR → pybind11 C++ Code Generator

Generates a pybind11 C++ source file from ModuleIR.
This replaces the direct AST → code approach in the old gen_pybind11.py.
"""

from __future__ import print_function
import sys
import os
from typing import Dict, List, Optional, Set

from lvgl_ir.schema import (
    ModuleIR, FunctionIR, EnumIR, EnumMemberIR, StructIR,
    FuncCategory, ParamIR,
)
from lvgl_ir.rules import WRAPPED_STRUCT_TYPES


def eprint(*args, **kwargs):
    """Print to stderr."""
    print(*args, file=sys.stderr, **kwargs)


class Pybind11Generator:
    """Generates pybind11 C++ code from ModuleIR."""

    def __init__(self, module_ir: ModuleIR):
        self.ir = module_ir
        self.output_lines: List[str] = []
        self.generated_funcs: Set[str] = set()
        self.bound_method_names: Dict[str, str] = {}  # python_name → widget_name

    def generate(self) -> str:
        """Generate the complete pybind11 C++ source file."""
        self._emit_header()
        self._emit_module_start()
        self._emit_enums()
        self._emit_module_functions()
        self._emit_obj_class()
        self._emit_widget_factories()
        self._emit_widget_methods()
        self._emit_string_overloads()
        self._emit_chart_manual_bindings()
        self._emit_canvas_manual_bindings()
        self._emit_line_manual_bindings()
        self._emit_menu_manual_bindings()
        self._emit_animimg_manual_bindings()
        self._emit_event_callbacks()
        self._emit_color_type()
        self._emit_display_class()
        self._emit_indev_class()
        self._emit_timer_class()
        self._emit_anim_class()
        self._emit_theme_bindings()
        self._emit_obj_remove_style_binding()
        self._emit_freetype_font()
        self._emit_driver_backends()
        self._emit_keyboard_group_helpers()
        self._emit_not_generated_list()
        self._emit_module_end()

        return "\n".join(self.output_lines)

    def emit(self, line: str = ""):
        """Append a line to the output."""
        self.output_lines.append(line)

    # -----------------------------------------------------------------------
    # File Header
    # -----------------------------------------------------------------------

    def _emit_header(self):
        self.emit("/*")
        self.emit(" * Auto-Generated file by gen_pybind11.py (IR-based), DO NOT EDIT!")
        self.emit(" *")
        self.emit(" * LVGL Python bindings generated via libclang → IR → pybind11 pipeline.")
        self.emit(" *")
        self.emit(" * Module: %s" % self.ir.module_name)
        self.emit(" * Total functions: %d" % self.ir.total_c_functions)
        self.emit(" * Bound functions: %d" % self.ir.total_bound_functions)
        self.emit(" * Skipped functions: %d" % self.ir.total_skipped_functions)
        self.emit(" * Enums: %d" % len(self.ir.enums))
        self.emit(" * Widgets: %d" % len(self.ir.widgets))
        self.emit(" */")
        self.emit("")
        self.emit("#include <pybind11/pybind11.h>")
        self.emit("#include <pybind11/stl.h>")
        self.emit("#include <pybind11/functional.h>")
        self.emit("#include <pybind11/numpy.h>")
        self.emit('#include "lvgl/lvgl.h"')
        self.emit('#include "lvgl/driver_backends.h"')
        self.emit('#include "lvgl/simulator_settings.h"')
        self.emit('#include "lvgl/demos/benchmark/lv_demo_benchmark.h"')
        self.emit('#include "lvgl/font/lv_freetype.h"')
        self.emit('#include "lvgl_pybind_helpers.h"')
        self.emit("")
        self.emit("namespace py = pybind11;")
        self.emit("")
        self.emit("/* Global simulator settings, defined in driver_backends.c (liblvgl_linux) */")
        self.emit('extern "C" simulator_settings_t settings;')
        self.emit("")

    # -----------------------------------------------------------------------
    # PYBIND11_MODULE block
    # -----------------------------------------------------------------------

    def _emit_module_start(self):
        self.emit("PYBIND11_MODULE(_lvgl, m) {")
        self.emit('    m.doc() = "LVGL Python bindings (auto-generated via IR)";')
        self.emit("")

    def _emit_module_end(self):
        self.emit("")
        self.emit("}  // PYBIND11_MODULE")

    # -----------------------------------------------------------------------
    # Enums
    # -----------------------------------------------------------------------

    def _emit_enums(self):
        self.emit("    /*")
        self.emit("     * Enum definitions")
        self.emit("     */")
        self.emit("")

        # Pre-compute all enum Python names and all value names to detect collisions.
        # pybind11's export_values() injects every value into the module namespace,
        # which causes two kinds of collisions:
        #   1. A value name matches an enum type name → "cannot initialize type X: already defined"
        #   2. The same value name appears in multiple enums → the last one wins silently
        #   3. Two different C enums map to the same Python name (e.g. lv_fs_res_t and
        #      lv_fs_mode_t both → FS) → "cannot initialize type FS: already defined"
        # Solution: don't use export_values(). For enum name collisions, keep the suffix.
        all_enum_py_names_raw = []  # [(c_type, py_name_before_dedup), ...]
        for enum_ir in self.ir.enums:
            if not enum_ir.members:
                continue
            c_type = enum_ir.c_type
            if c_type and c_type.startswith("lv_"):
                py_name = self._enum_python_name(c_type)
                all_enum_py_names_raw.append((c_type, py_name))

        # Detect enum name collisions and deduplicate by keeping the suffix
        from collections import Counter
        enum_name_counts = Counter(py_name for _, py_name in all_enum_py_names_raw)
        colliding_enum_names = {name for name, count in enum_name_counts.items() if count > 1}

        # Build a mapping from c_type to final py_name (deduplicated)
        enum_py_name_map: Dict[str, str] = {}
        for c_type, py_name in all_enum_py_names_raw:
            if py_name in colliding_enum_names:
                # Keep the suffix: FS_RES instead of FS, FS_MODE instead of FS
                if c_type.startswith("lv_") and c_type.endswith("_t"):
                    enum_py_name_map[c_type] = c_type[3:-2].upper()
                else:
                    enum_py_name_map[c_type] = py_name
            else:
                enum_py_name_map[c_type] = py_name

        all_enum_py_names = set(enum_py_name_map.values())
        all_value_names = []  # (py_name, safe_name, member_c_name, enum_py_name)
        for enum_ir in self.ir.enums:
            if not enum_ir.members:
                continue
            c_type = enum_ir.c_type
            if c_type and c_type.startswith("lv_"):
                py_name = enum_py_name_map.get(c_type, self._enum_python_name(c_type))
                for member in enum_ir.members:
                    stripped = self._strip_enum_prefix(member.c_name, py_name)
                    safe_name = self._sanitize(stripped)
                    all_value_names.append((py_name, safe_name, member.c_name))

        # Count value name occurrences to detect duplicates
        from collections import Counter
        value_name_counts = Counter(sn for _, sn, _ in all_value_names)
        conflicting_values = {name for name, count in value_name_counts.items()
                              if count > 1 or name in all_enum_py_names}

        # First pass: emit enum types WITHOUT export_values()
        for enum_ir in self.ir.enums:
            if not enum_ir.members:
                continue

            c_type = enum_ir.c_type
            py_name = enum_py_name_map.get(c_type, self._enum_python_name(c_type))

            if c_type and c_type.startswith("lv_"):
                # Bitmask-style enums need |, &, ^, ~ operators via py::arithmetic()
                BITMASK_ENUMS = {
                    "lv_obj_flag_t", "lv_obj_state_t", "lv_part_t",
                    "lv_style_selector_t", "lv_text_decor_t",
                }
                if c_type in BITMASK_ENUMS:
                    self.emit('    py::enum_<%s>(m, "%s", py::arithmetic())' % (c_type, py_name))
                else:
                    self.emit('    py::enum_<%s>(m, "%s")' % (c_type, py_name))
                for member in enum_ir.members:
                    stripped = self._strip_enum_prefix(member.c_name, py_name)
                    safe_name = self._sanitize(stripped)
                    self.emit('        .value("%s", %s)' % (safe_name, member.c_name))
                # Do NOT call export_values() — we manually inject below
                self.emit(";")
                self.emit("")
            else:
                # Check if this is the LVGL symbol ID enum
                is_symbol_enum = any(
                    m.c_name.startswith("LV_STR_SYMBOL_") for m in enum_ir.members
                )
                if is_symbol_enum:
                    self.emit("    /* Enum %s - exported as UTF-8 symbol strings for use with icon APIs */" % c_type)
                    for member in enum_ir.members:
                        stripped = self._strip_enum_prefix(member.c_name, "STR")
                        safe_name = self._sanitize(stripped)
                        symbol_macro = member.c_name.replace("LV_STR_SYMBOL_", "LV_SYMBOL_")
                        # Only export if no collision
                        if safe_name not in conflicting_values:
                            self.emit('    m.attr("%s") = %s;' % (safe_name, symbol_macro))
                        else:
                            self.emit('    /* %s skipped (name collision) — use SYMBOL.%s instead */' % (safe_name, safe_name))
                    self.emit("")
                else:
                    # Emit as integer constants
                    self.emit("    /* Enum %s (no C type found, emitting as constants) */" % c_type)
                    for member in enum_ir.members:
                        stripped = self._strip_enum_prefix(member.c_name, py_name)
                        safe_name = self._sanitize(stripped)
                        if safe_name not in conflicting_values:
                            self.emit('    m.attr("%s") = (int)%s;' % (safe_name, member.c_name))
                        else:
                            self.emit('    /* %s skipped (name collision) */' % safe_name)
                    self.emit("")

    @staticmethod
    def _strip_enum_prefix(c_name: str, enum_py_name: str = "") -> str:
        """Strip LV_ prefix and enum class name prefix from a C enum member name.

        Examples:
            LV_ALIGN_TOP_LEFT + ALIGN → TOP_LEFT
            LV_EVENT_CLICKED + EVENT → CLICKED
            LV_OBJ_FLAG_CLICKABLE + OBJ_FLAG → CLICKABLE
            LV_ARC_MODE_NORMAL + ARC_MODE → NORMAL
            LV_OPA_50 + OPA → 50
            LV_STR_SYMBOL_AUDIO + SYMBOL → AUDIO
        """
        name = c_name
        # Strip LV_ prefix
        if name.startswith("LV_"):
            name = name[3:]

        # For overridden enum names, use the C prefix that corresponds to the
        # original C type name (before the override) so value stripping works correctly.
        # e.g. LV_FREETYPE_FONT_RENDER_BITMAP + FREETYPE_RENDER
        #      → strip FREETYPE_FONT_RENDER_ prefix → BITMAP
        override_c_prefix = Pybind11Generator.ENUM_VALUE_PREFIX_OVERRIDES.get(enum_py_name)
        if override_c_prefix and name.startswith(override_c_prefix + "_"):
            name = name[len(override_c_prefix) + 1:]
            # Handle SCREEN_LOAD: after stripping SCREEN_LOAD_, leftover has ANIM_ prefix
            for suffix in Pybind11Generator.ENUM_NAME_SUFFIX_STRIP:
                suffix_prefix = suffix + "_"
                if suffix_prefix.startswith("_"):
                    suffix_prefix = suffix_prefix[1:]
                if name.startswith(suffix_prefix):
                    name = name[len(suffix_prefix):]
                    break
            return name

        # Strip enum class name prefix (e.g., ALIGN_ from ALIGN_TOP_LEFT)
        if enum_py_name and name.startswith(enum_py_name + "_"):
            name = name[len(enum_py_name) + 1:]
        # If the py_name was derived by stripping a suffix (e.g., ARC_MODE → ARC),
        # the remaining name still has the suffix part as prefix (e.g., MODE_NORMAL).
        # Strip that suffix-derived prefix (MODE_) to get the clean value name (NORMAL).
        if enum_py_name:
            for suffix in Pybind11Generator.ENUM_NAME_SUFFIX_STRIP:
                suffix_prefix = suffix + "_"  # e.g., _MODE_ → MODE_ (after prior strip)
                # The suffix prefix is the part after the py_name: _MODE_ becomes MODE_
                if suffix_prefix.startswith("_"):
                    suffix_prefix = suffix_prefix[1:]  # _MODE_ → MODE_
                if name.startswith(suffix_prefix):
                    name = name[len(suffix_prefix):]
                    break
        return name

    # Suffixes to strip from enum Python names for cleaner API.
    # Only strip truly redundant suffixes that don't carry independent meaning.
    # Keep suffixes like _FLAG, _SIDE, _CTRL that are part of the enum's identity
    # (per API review: OBJ_FLAG.CLICKABLE, BORDER_SIDE.FULL, etc.)
    #
    # IMPORTANT: Naming convention (API review line 342) — keep _MODE/_TYPE/_ENCODING/
    # _COMPRESS/_SRC suffixes so widget-specific enums stay self-describing and
    # consistent (e.g., ARC_MODE.NORMAL, CHART_TYPE.LINE, CHART_UPDATE.SHIFT, BARCODE_ENCODING.EAN_13).
    # Only strip suffixes that are pure boilerplate (EVENT_CODE → EVENT, COVER_RES → COVER).
    ENUM_NAME_SUFFIX_STRIP = {
        "_CODE": "",       # EVENT_CODE → EVENT
        "_RES": "",        # COVER_RES → COVER, FS_RES → FS
        "_CMP": "",        # STYLE_STATE_CMP → STYLE_STATE
        "_WALK_RES": "",   # OBJ_TREE_WALK_RES → OBJ_TREE_WALK
        "_TRANSFORM_FLAG": "", # OBJ_POINT_TRANSFORM_FLAG → OBJ_POINT
        "_REFOCUS_POLICY": "", # GROUP_REFOCUS_POLICY → GROUP
        "_GESTURE_TYPE": "", # INDEV_GESTURE_TYPE → INDEV_GESTURE
        "_LONG_MODE": "_LONG",  # LABEL_LONG_MODE → LABEL_LONG (keeps _LONG identity)
        # "_ORIENTATION": "",  # REMOVED: BAR_ORIENTATION → BAR collides with BAR_MODE → BAR
        "_RENDER_MODE": "_RENDER", # DISPLAY_RENDER_MODE → DISPLAY_RENDER (keeps _RENDER identity for clarity)
        "_LOAD_ANIM": "_LOAD",  # SCREEN_LOAD_ANIM → SCREEN_LOAD (keeps _LOAD identity)
        "_INHERITABLE": "", # OBJ_CLASS_THEME_INHERITABLE → OBJ_CLASS_THEME
        # "_EDITABLE": "",  # REMOVED: OBJ_CLASS_EDITABLE → OBJ_CLASS collides with OBJ_CLASS_GROUP_DEF
        # "_GROUP_DEF": "", # REMOVED: OBJ_CLASS_GROUP_DEF → OBJ_CLASS collides with OBJ_CLASS_EDITABLE
        # Kept (NOT stripped) for naming consistency:
        #   _TYPE  (CHART_TYPE, INDEV_TYPE)
        #   _MODE  (ARC_MODE, BAR_MODE, KEYBOARD_MODE, SCROLLBAR_MODE, ...)
        #   _ENCODING (BARCODE_ENCODING)
        #   _COMPRESS (IMAGE_COMPRESS)
        #   _SRC  (IMAGE_SRC)
    }

    # Per-enum overrides for names that can't be derived by simple suffix stripping.
    # Used to shorten verbose names (FREETYPE_FONT_* → FREETYPE_*, MENU_MODE_* → MENU_*).
    ENUM_NAME_OVERRIDES = {
        "lv_freetype_font_render_mode_t": "FREETYPE_RENDER",
        "lv_freetype_font_style_t": "FREETYPE_STYLE",
        "lv_menu_mode_header_t": "MENU_HEADER",
        "lv_menu_mode_root_back_button_t": "MENU_ROOT_BACK_BTN",
    }

    # For overridden enum names, the C value prefix to strip (derived from the C type).
    # Without this, _strip_enum_prefix can't find the right prefix to strip from values.
    # e.g. LV_FREETYPE_FONT_RENDER_BITMAP + FREETYPE_RENDER → strip FREETYPE_FONT_RENDER_ → BITMAP
    ENUM_VALUE_PREFIX_OVERRIDES = {
        "FREETYPE_RENDER": "FREETYPE_FONT_RENDER_MODE",
        "FREETYPE_STYLE": "FREETYPE_FONT_STYLE",
        "LABEL_LONG": "LABEL_LONG_MODE",
        "MENU_HEADER": "MENU_MODE_HEADER",
        "MENU_ROOT_BACK_BTN": "MENU_MODE_ROOT_BACK_BUTTON",
        "SCREEN_LOAD": "SCREEN_LOAD",  # needs suffix strip for _ANIM_
    }

    @staticmethod
    def _enum_python_name(c_type: str) -> str:
        """Convert lv_align_t → ALIGN, lv_event_code_t → EVENT, etc."""
        # Check explicit overrides first (handles non-suffix renames)
        if c_type in Pybind11Generator.ENUM_NAME_OVERRIDES:
            return Pybind11Generator.ENUM_NAME_OVERRIDES[c_type]
        if c_type.startswith("lv_") and c_type.endswith("_t"):
            name = c_type[3:-2]  # strip lv_ and _t
            name = name.upper()
            # Strip common suffixes for cleaner names
            for suffix, replacement in Pybind11Generator.ENUM_NAME_SUFFIX_STRIP.items():
                if name.endswith(suffix):
                    name = name[:-len(suffix)] + replacement
                    break
            return name
        return c_type

    # -----------------------------------------------------------------------
    # Module-level functions (init, timer_handler, etc.)
    # -----------------------------------------------------------------------

    def _emit_module_functions(self):
        self.emit("    /* Module-level functions */")
        self.emit('    m.def("init", &lv_init, "Initialize LVGL library");')
        self.emit('    m.def("deinit", &lv_deinit, "Deinitialize LVGL library");')
        self.emit('    m.def("is_initialized", &lv_is_initialized, "Check if LVGL is initialized");')
        self.emit('    m.def("timer_handler", &py_timer_handler, "Call LVGL timer handler");')
        self.emit("")
        self.emit("    /* Module constants */")
        self.emit('    m.attr("SIZE_CONTENT") = (int32_t)LV_SIZE_CONTENT;')
        self.emit('    m.attr("RADIUS_CIRCLE") = (int32_t)LV_RADIUS_CIRCLE;')
        self.emit('    m.attr("OPA_TRANSP") = (int)LV_OPA_TRANSP;')
        self.emit('    m.attr("OPA_COVER") = (int)LV_OPA_COVER;')
        self.emit("")
        self.emit("    /* pct() helper — equivalent of lv_pct(x) macro */")
        self.emit('    m.def("pct", [](int32_t x) -> int32_t { return LV_PCT(x); }, py::arg("x"), "Set size as percentage of parent (e.g. lv.pct(50))");')
        self.emit("")
        self.emit("    /* Font default (as uintptr_t for Python — use with set_style_text_font) */")
        self.emit('    m.attr("font_default") = reinterpret_cast<uintptr_t>((const lv_font_t *)lv_font_get_default());')
        self.emit("")
        self.emit("    /* Palette color functions */")
        self.emit('    m.def("palette_main", [](lv_palette_t p) -> lv_color_t { return lv_palette_main(p); }, py::arg("palette"), "Get main color for a palette");')
        self.emit('    m.def("palette_darken", [](lv_palette_t p, uint8_t lvl) -> lv_color_t { return lv_palette_darken(p, lvl); }, py::arg("palette"), py::arg("level"), "Get darkened color for a palette");')
        self.emit('    m.def("palette_lighten", [](lv_palette_t p, uint8_t lvl) -> lv_color_t { return lv_palette_lighten(p, lvl); }, py::arg("palette"), py::arg("level"), "Get lightened color for a palette");')
        self.emit("")
        self.emit("    /* Version info */")
        self.emit("    m.def(\"version_major\", []() { return lv_version_major(); });")
        self.emit("    m.def(\"version_minor\", []() { return lv_version_minor(); });")
        self.emit("    m.def(\"version_patch\", []() { return lv_version_patch(); });")
        self.emit("    m.def(\"version_info\", []() { return std::string(lv_version_info()); });")
        self.emit("")
        self.emit("    /* LVGL Demos (from lvgl_demos library) */")
        self.emit("    m.def(\"demo_benchmark\", []() { lv_demo_benchmark(); }, \"Run LVGL benchmark demo\");")
        self.emit("")

    # -----------------------------------------------------------------------
    # Base Object class
    # -----------------------------------------------------------------------

    def _emit_obj_class(self):
        self.emit("    /* Base Object class */")
        self.emit("    auto obj_cls = py::class_<LvObjWrapper>(m, \"obj\", py::dynamic_attr());")
        # Default constructor: lv.obj() — creates a root object on the active screen
        self.emit("    obj_cls.def(py::init<>());")
        # Parent constructor: lv.obj(parent) — creates a child container under parent.
        # Matches the API pattern used by all other widgets (lv.btn(parent), lv.label(parent)).
        self.emit('    obj_cls.def(py::init([](py::object parent_obj) {')
        self.emit("        LvObjWrapper *_parent = parent_obj.is_none() ? nullptr : parent_obj.cast<LvObjWrapper*>();")
        self.emit("        LvObjWrapper *wrapper = new LvObjWrapper(lv_obj_create(_parent ? _parent->get() : lv_screen_active()));")
        self.emit("        if (_parent) wrapper->keep_parent(parent_obj);")
        self.emit("        return wrapper;")
        self.emit('    }), py::arg("parent") = py::none());')
        # Expose keep_parent for lifecycle management (called internally by factory functions)
        self.emit('    obj_cls.def("_keep_parent", [](LvObjWrapper &self, py::object parent) {')
        self.emit("        self.keep_parent(parent);")
        self.emit("    }, py::arg(\"parent\"));")
        # Expose infer_widget_type for dispatch methods when _widget_type is not set
        self.emit('    obj_cls.def("_infer_widget_type", [](LvObjWrapper &self) -> const char * {')
        self.emit("        return self.infer_widget_type();")
        self.emit("    });")

        # Generate methods for base obj
        obj_methods = self.ir.get_widget_methods("obj")
        for func in obj_methods:
            self._emit_method(obj_cls_var="obj_cls", func=func, is_obj_method=True)

        self.emit("")

        # Screen management helpers
        self.emit("    /* Screen management */")
        self.emit("    m.def(\"screen_active\", []() -> LvObjWrapper* {")
        self.emit("        return new LvObjWrapper(lv_screen_active(), false);")
        self.emit("    });")
        self.emit("    m.def(\"screen_load\", [](LvObjWrapper &screen) {")
        self.emit("        lv_screen_load(screen.get());")
        self.emit('    }, py::arg("screen"));')
        self.emit("")

    # -----------------------------------------------------------------------
    # Widget factory functions
    # -----------------------------------------------------------------------

    def _emit_widget_factories(self):
        self.emit("    /* Widget factory functions */")

        for widget_name, widget in sorted(self.ir.widgets.items()):
            if widget_name == "obj":
                continue  # Already handled above

            # Find constructor function
            ctor_func = None
            for func in self.ir.functions:
                if func.name == widget.create_func and not func.skip:
                    ctor_func = func
                    break

            if not ctor_func:
                continue

            # Generate factory function
            py_name = widget_name
            self._emit_factory_function(py_name, ctor_func)

        self.emit("")

    def _emit_factory_function(self, py_name: str, ctor_func: FunctionIR):
        """Generate a widget factory function."""
        call_args = []
        lambda_params = []
        py_args = []
        has_parent = False

        for i, param in enumerate(ctor_func.params):
            if param.is_self:
                # First param is parent (lv_obj_t*)
                has_parent = True
                lambda_params.append("py::object parent_obj")
                py_args.append('py::arg("parent") = py::none()')
                continue
            if param.c_type == "void":
                continue

            arg_name = param.name
            cpp_type = param.cpp_type
            if cpp_type is None:
                return  # Can't generate this factory

            # For wrapper types, need .get() when calling C function
            # For const void* params (mapped to const char* for pybind11), add cast
            if cpp_type.endswith("Wrapper &"):
                call_args.append("%s.get()" % arg_name)
            elif param.c_type == "const void *" and cpp_type == "const char *":
                call_args.append("(const void *)%s" % arg_name)
            else:
                call_args.append(arg_name)
            lambda_params.append("%s %s" % (cpp_type, arg_name))
            py_args.append('py::arg("%s")' % arg_name)

        if has_parent:
            all_call_args = ", ".join(["_parent_obj.cast<LvObjWrapper*>()->get()"] + call_args)
        else:
            all_call_args = ", ".join(call_args)
        lambda_params_str = ", ".join(lambda_params)
        py_args_str = ", ".join(py_args)

        self.emit('    m.def("%s", [](%s) -> LvObjWrapper* {' % (py_name, lambda_params_str))
        if has_parent:
            # Special case: lv_msgbox_create(NULL) should create a backdrop on
            # lv_layer_top() for modal behavior.  Passing lv_screen_active() instead
            # would skip the backdrop and break the close button visibility.
            if ctor_func.name in self.NULL_PARENT_WIDGETS:
                self.emit("        LvObjWrapper *_parent = parent_obj.is_none() ? nullptr : parent_obj.cast<LvObjWrapper*>();")
                self.emit("        LvObjWrapper *wrapper = new LvObjWrapper(%s(_parent ? _parent->get() : NULL%s));" % (ctor_func.name, ", " + ", ".join(call_args) if call_args else ""))
            else:
                self.emit("        LvObjWrapper *_parent = parent_obj.is_none() ? nullptr : parent_obj.cast<LvObjWrapper*>();")
                self.emit("        LvObjWrapper *wrapper = new LvObjWrapper(%s(_parent ? _parent->get() : lv_screen_active()%s));" % (ctor_func.name, ", " + ", ".join(call_args) if call_args else ""))
        else:
            self.emit("        LvObjWrapper *wrapper = new LvObjWrapper(%s(%s));" % (ctor_func.name, all_call_args))
        if has_parent:
            # Keep parent Python reference to prevent GC from collecting parent
            # while child is still alive (prevents use-after-free crash)
            self.emit("        if (_parent) wrapper->keep_parent(parent_obj);")
        self.emit("        return wrapper;")
        self.generated_funcs.add(ctor_func.name)

        if py_args_str:
            self.emit("    }, %s);" % py_args_str)
        else:
            self.emit("    });")

    # -----------------------------------------------------------------------
    # Widget-specific methods
    # -----------------------------------------------------------------------

    # Only these widget types' methods can be added to obj_cls,
    # because they are LVGL widgets (subclasses of lv_obj_t).
    # Other types like display, indev, group, timer, style, etc.
    # are separate subsystems and their methods must be module-level.
    OBJ_WIDGET_TYPES = {
        "obj", "image", "animimg", "arc", "arclabel", "label", "bar",
        "canvas", "barcode", "button", "buttonmatrix", "calendar",
        "chart", "checkbox", "dropdown", "imagebutton", "keyboard",
        "led", "line", "list", "lottie", "menu", "menu_page",
        "menu_cont", "menu_section", "menu_separator", "msgbox",
        "qrcode", "roller", "scale", "slider", "spangroup", "textarea",
        "spinbox", "spinner", "switch", "table", "tabview", "tileview", "win",
    }

    def _emit_widget_methods(self):
        self.emit("    /* Widget-specific methods on Obj */")

        # First pass: collect all python method names and detect collisions.
        # When multiple widget types have methods that map to the same python
        # name (e.g. lv_arc_set_rotation / lv_image_set_rotation → "set_rotation"),
        # pybind11 will register all of them as overloads on the same Obj class.
        # At call time pybind11 picks the first overload whose parameter types
        # match, which means Image.set_rotation() would dispatch to
        # lv_arc_set_rotation — a completely wrong C function that corrupts
        # memory and crashes.  The fix: prefix colliding names with the widget
        # type (e.g. image_set_rotation, arc_set_rotation).
        method_name_owners: Dict[str, str] = {}  # python_name → widget_name

        for widget_name, widget in sorted(self.ir.widgets.items()):
            if widget_name == "obj":
                continue
            if widget_name not in self.OBJ_WIDGET_TYPES:
                continue
            for func in widget.methods:
                if func.skip or func.name in self.generated_funcs:
                    continue
                if func.category == FuncCategory.CONSTRUCTOR:
                    continue
                py_name = self._sanitize(func.python_name)
                if py_name == "delete":
                    py_name = "delete_obj"
                if py_name in method_name_owners and method_name_owners[py_name] != widget_name:
                    # Collision detected — both owners will need a prefix
                    pass
                method_name_owners.setdefault(py_name, widget_name)

        # Build the set of names that collide (owned by ≥2 different widgets)
        from collections import Counter
        owner_counts = Counter(method_name_owners.values())  # not useful here
        # We need: for each py_name, count how many distinct widget_names claim it
        name_to_widgets: Dict[str, Set[str]] = {}
        for widget_name, widget in sorted(self.ir.widgets.items()):
            if widget_name == "obj":
                continue
            if widget_name not in self.OBJ_WIDGET_TYPES:
                continue
            for func in widget.methods:
                if func.skip or func.name in self.generated_funcs:
                    continue
                if func.category == FuncCategory.CONSTRUCTOR:
                    continue
                py_name = self._sanitize(func.python_name)
                if py_name == "delete":
                    py_name = "delete_obj"
                name_to_widgets.setdefault(py_name, set()).add(widget_name)

        colliding_names = {name for name, widgets in name_to_widgets.items()
                           if len(widgets) > 1}

        # Second pass: emit methods with prefixed names for collisions
        for widget_name, widget in sorted(self.ir.widgets.items()):
            if widget_name == "obj":
                continue  # Already generated above
            if widget_name not in self.OBJ_WIDGET_TYPES:
                continue  # Skip non-widget types (display, indev, group, etc.)

            for func in widget.methods:
                if func.name in self.generated_funcs:
                    continue
                if func.category == FuncCategory.CONSTRUCTOR:
                    continue  # Factories already generated
                self._emit_method(obj_cls_var="obj_cls", func=func, is_obj_method=True,
                                  widget_name=widget_name,
                                  colliding_names=colliding_names)

        self.emit("")

    # -----------------------------------------------------------------------
    # Method generation
    # -----------------------------------------------------------------------

    def _emit_method(self, obj_cls_var: str, func: FunctionIR, is_obj_method: bool = False,
                     widget_name: str = "", colliding_names: Optional[Set[str]] = None):
        """Generate a single method binding."""
        if func.skip or func.name in self.generated_funcs:
            return

        method_name = self._sanitize(func.python_name)
        if method_name == "delete":
            method_name = "delete_obj"

        # If this python method name collides across widgets, prefix with _widget_name
        # (leading underscore makes it a Python private attribute, so only the
        # short-name dispatch methods in _wrapper.py remain as the public API)
        # e.g. set_rotation (owned by arc, image, scale) → _arc_set_rotation, _image_set_rotation, _scale_set_rotation
        if colliding_names and method_name in colliding_names and widget_name:
            method_name = "_%s_%s" % (widget_name, method_name)

        # Build lambda body
        call_args = []
        lambda_params = []
        py_args = []

        for i, param in enumerate(func.params):
            if param.is_self:
                continue  # skip self
            if param.c_type == "void":
                continue

            arg_name = param.name
            cpp_type = param.cpp_type
            if cpp_type is None:
                return  # Can't handle this parameter

            # For wrapper type params, call .get()
            # For const void* params (mapped to const char* for pybind11), add cast
            # For opaque pointer types mapped to uintptr_t (e.g. const lv_font_t*), cast back
            if cpp_type.endswith("Wrapper &"):
                call_args.append("%s.get()" % arg_name)
            elif param.c_type == "const void *" and cpp_type == "const char *":
                call_args.append("(const void *)%s" % arg_name)
            elif cpp_type == "uintptr_t" and param.c_type in ("const lv_font_t *", "lv_font_t *"):
                cast_type = param.c_type.replace(" ", "")  # e.g. "constlv_font_t*"
                # Normalize: "constlv_font_t*" → "const lv_font_t *"
                if cast_type.startswith("const"):
                    call_args.append("reinterpret_cast<const lv_font_t *>(%s)" % arg_name)
                else:
                    call_args.append("reinterpret_cast<lv_font_t *>(%s)" % arg_name)
            else:
                call_args.append(arg_name)
            lambda_params.append("%s %s" % (cpp_type, arg_name))

            # Add default value for style selector parameters (uint32_t selector → 0)
            # Note: The Pythonic name is SELECTOR.DEFAULT (defined in _wrapper.py),
            # but at the C++ pybind11 layer we use the raw value 0.
            if arg_name == "selector" and cpp_type == "uint32_t":
                py_args.append('py::arg("selector") = 0')
            else:
                py_args.append('py::arg("%s")' % arg_name)

        # Build call expression
        if is_obj_method:
            call_args_str = ", ".join(["self.get()"] + call_args)
            lambda_params_str = ", ".join(["LvObjWrapper &self"] + lambda_params)
        else:
            call_args_str = ", ".join(call_args)
            lambda_params_str = ", ".join(lambda_params)

        py_args_str = ", ".join(py_args)

        # Verify arg count
        non_self_params = [p for p in func.params if not p.is_self and p.c_type != "void"]
        if len(call_args) + (1 if is_obj_method else 0) < len(func.params):
            return  # Some args silently dropped

        # Generate binding based on return type
        return_type = func.return_type
        return_cpp = func.return_cpp_type

        if return_type == "void":
            self.emit('    %s.def("%s", [](%s) { %s(%s); return; }' % (
                obj_cls_var, method_name, lambda_params_str, func.name, call_args_str))
        elif self._is_obj_ptr(return_type):
            # Return LvObjWrapper* with null check → Python None for NULL pointers
            self.emit('    %s.def("%s", [](%s) -> LvObjWrapper* {' % (
                obj_cls_var, method_name, lambda_params_str))
            self.emit('        lv_obj_t *result = %s(%s);' % (func.name, call_args_str))
            self.emit('        return result ? new LvObjWrapper(result) : nullptr;')
            self.emit('    }')
        elif return_cpp and return_cpp.endswith("Wrapper &"):
            # Return wrapper reference → skip (can't return reference to temporary)
            # These are typically functions that return lv_display_t*, lv_indev_t* etc.
            # We skip them because we don't have proper wrapper return handling
            return
        elif return_cpp and return_cpp.endswith(" *") and "Wrapper" in return_cpp:
            # Skip unsupported wrapper pointer returns
            return
        elif return_cpp:
            # For opaque pointer returns mapped to uintptr_t, add reinterpret_cast
            if return_cpp == "uintptr_t" and return_type in ("const lv_font_t *", "lv_font_t *"):
                self.emit('    %s.def("%s", [](%s) -> uintptr_t { return reinterpret_cast<uintptr_t>(%s(%s)); }' % (
                    obj_cls_var, method_name, lambda_params_str, func.name, call_args_str))
            else:
                self.emit('    %s.def("%s", [](%s) -> %s { return %s(%s); }' % (
                    obj_cls_var, method_name, lambda_params_str, return_cpp, func.name, call_args_str))
        else:
            return  # Unsupported return type

        if py_args_str:
            self.emit("        , %s);" % py_args_str)
        else:
            self.emit("        );")

        self.generated_funcs.add(func.name)

    # -----------------------------------------------------------------------
    # String overloads for const void* (icon/image source) methods
    # -----------------------------------------------------------------------

    # Methods that take const void* (mapped to const char*) for icon/image sources
    # also need a std::string overload so Python str works seamlessly
    CONST_VOID_PTR_METHODS = {
        "set_style_bg_image_src": ("value", "selector"),
        "set_src": ("src",),
    }

    def _emit_string_overloads(self):
        """Emit std::string overloads for methods that accept const void* (icon/image src).

        The primary binding uses const char* (mapped from const void*), but adding
        a std::string overload ensures Python str objects work without encoding issues.
        """
        self.emit("    /* String overloads for icon/image source methods */")
        for method_name, args in self.CONST_VOID_PTR_METHODS.items():
            if method_name in self.generated_funcs:
                # Build lambda params and call args
                lambda_params = ["LvObjWrapper &self"]
                call_args = ["self.get()"]
                py_args = []
                for arg in args:
                    if arg == "selector":
                        lambda_params.append("uint32_t %s" % arg)
                        call_args.append(arg)
                    else:
                        lambda_params.append("const std::string & %s" % arg)
                        call_args.append("%s.c_str()" % arg)
                    py_args.append('py::arg("%s")' % arg)

                lambda_params_str = ", ".join(lambda_params)
                call_args_str = ", ".join(call_args)
                py_args_str = ", ".join(py_args)

                # Determine the C function name
                c_func = "lv_obj_%s" % method_name
                if method_name == "set_src":
                    c_func = "lv_image_set_src"

                self.emit('    obj_cls.def("%s", [](%s) { %s(%s); return; }' % (
                    method_name, lambda_params_str, c_func, call_args_str))
                self.emit("        , %s);" % py_args_str)
        self.emit("")

    # -----------------------------------------------------------------------
    # Chart manual bindings (series pointer as uintptr_t)
    # -----------------------------------------------------------------------

    def _emit_chart_manual_bindings(self):
        """Emit manual bindings for chart series operations.

        lv_chart_series_t* is an opaque struct that can't be auto-wrapped,
        so we pass it as uintptr_t (Python int) instead.
        """
        self.emit("    /* Chart: manual bindings for series and data (series pointer stored as user_data) */")

        self.emit('    obj_cls.def("set_point_count", [](LvObjWrapper &self, uint32_t cnt) {')
        self.emit("        lv_chart_set_point_count(self.get(), cnt);")
        self.emit('    }, py::arg("cnt"), "Set the number of points on a chart series");')

        self.emit('    obj_cls.def("add_series", [](LvObjWrapper &self, lv_color_t color, lv_chart_axis_t axis) -> uintptr_t {')
        self.emit("        lv_chart_series_t *ser = lv_chart_add_series(self.get(), color, axis);")
        self.emit("        return reinterpret_cast<uintptr_t>(ser);")
        self.emit('    }, py::arg("color"), py::arg("axis"), "Add a data series to chart, returns series handle as int");')

        self.emit('    obj_cls.def("set_next_value", [](LvObjWrapper &self, uintptr_t series, int32_t value) {')
        self.emit("        lv_chart_series_t *ser = reinterpret_cast<lv_chart_series_t*>(series);")
        self.emit("        lv_chart_set_next_value(self.get(), ser, value);")
        self.emit('    }, py::arg("series"), py::arg("value"), "Set next value on a chart series");')

        self.emit('    obj_cls.def("set_next_value2", [](LvObjWrapper &self, uintptr_t series, int32_t x_value, int32_t y_value) {')
        self.emit("        lv_chart_series_t *ser = reinterpret_cast<lv_chart_series_t*>(series);")
        self.emit("        lv_chart_set_next_value2(self.get(), ser, x_value, y_value);")
        self.emit('    }, py::arg("series"), py::arg("x_value"), py::arg("y_value"), "Set next X/Y value on a chart series (scatter)");')

        self.emit('    obj_cls.def("refresh", [](LvObjWrapper &self) {')
        self.emit("        lv_chart_refresh(self.get());")
        self.emit('    }, "Refresh chart after data change");')

        self.emit('    obj_cls.def("set_series_color", [](LvObjWrapper &self, uintptr_t series, lv_color_t color) {')
        self.emit("        lv_chart_series_t *ser = reinterpret_cast<lv_chart_series_t*>(series);")
        self.emit("        lv_chart_set_series_color(self.get(), ser, color);")
        self.emit('    }, py::arg("series"), py::arg("color"), "Set series color");')

        self.emit('    obj_cls.def("set_all_values", [](LvObjWrapper &self, uintptr_t series, int32_t value) {')
        self.emit("        lv_chart_series_t *ser = reinterpret_cast<lv_chart_series_t*>(series);")
        self.emit("        lv_chart_set_all_values(self.get(), ser, value);")
        self.emit('    }, py::arg("series"), py::arg("value"), "Set all values of a series to the same value");')

        self.emit("")

    # -----------------------------------------------------------------------
    # Canvas manual bindings (void* buf as uintptr_t)
    # -----------------------------------------------------------------------

    def _emit_canvas_manual_bindings(self):
        """Emit manual binding for canvas.set_buffer which takes a void* buf.

        The C API lv_canvas_set_buffer(obj, void *buf, int32_t w, int32_t h, lv_color_format_t cf)
        has a void* parameter that pybind11 maps to capsule, which is unusable from Python.
        We accept buf as uintptr_t (Python int) instead, so users can pass buf.ctypes.data.
        """
        self.emit("    /* Canvas: manual binding for set_buffer (void* buf as uintptr_t) */")
        self.emit('    obj_cls.def("set_buffer", [](LvObjWrapper &self, uintptr_t buf, int32_t w, int32_t h, lv_color_format_t cf) {')
        self.emit("        lv_canvas_set_buffer(self.get(), reinterpret_cast<void*>(buf), w, h, cf);")
        self.emit('    }, py::arg("buf"), py::arg("w"), py::arg("h"), py::arg("cf"), "Set canvas buffer from memory address (use buf.ctypes.data)");')
        self.emit("")

    # -----------------------------------------------------------------------
    # Line manual bindings (set_points takes C array, not auto-generatable)
    # -----------------------------------------------------------------------

    def _emit_line_manual_bindings(self):
        """Emit manual binding for line.set_points which takes a Python list of (x, y) tuples.

        The C API lv_line_set_points(obj, const lv_point_precise_t points[], uint32_t point_num)
        takes a C array which cannot be auto-wrapped. We accept py::list and convert.
        """
        self.emit("    /* Line: manual binding for set_points (C array param) */")
        self.emit('    obj_cls.def("set_points", [](LvObjWrapper &self, py::list points) {')
        self.emit("        size_t n = points.size();")
        self.emit("        auto *arr = new lv_point_precise_t[n];")
        self.emit("        for (size_t i = 0; i < n; i++) {")
        self.emit("            py::tuple pt = points[i].cast<py::tuple>();")
        self.emit("            arr[i].x = pt[0].cast<lv_value_precise_t>();")
        self.emit("            arr[i].y = pt[1].cast<lv_value_precise_t>();")
        self.emit("        }")
        self.emit("        lv_line_set_points(self.get(), arr, (uint32_t)n);")
        self.emit("        delete[] arr;")
        self.emit('    }, py::arg("points"), "Set line points from list of (x, y) tuples");')
        self.emit("")

    # -----------------------------------------------------------------------
    # Menu manual bindings (page/cont/section creation)
    # -----------------------------------------------------------------------

    def _emit_menu_manual_bindings(self):
        """Emit manual bindings for menu sub-object creation.

        lv_menu_page_create(menu, title), lv_menu_cont_create(parent),
        and lv_menu_section_create(parent) return lv_obj_t* but are not
        standard widget constructors, so the auto-generator doesn't create
        factory functions for them. We add them manually.
        """
        self.emit("    /* Menu: manual bindings for page/cont/section creation */")
        # MenuPage factory
        self.emit('    m.def("MenuPage", [](py::object menu_obj, const char * title) -> LvObjWrapper* {')
        self.emit("        LvObjWrapper *_menu = menu_obj.is_none() ? nullptr : menu_obj.cast<LvObjWrapper*>();")
        self.emit("        LvObjWrapper *wrapper = new LvObjWrapper(lv_menu_page_create(_menu ? _menu->get() : lv_screen_active(), title));")
        self.emit("        if (_menu) wrapper->keep_parent(menu_obj);")
        self.emit('        return wrapper;')
        self.emit('    }, py::arg("menu"), py::arg("title") = "");')
        # MenuCont factory
        self.emit('    m.def("MenuCont", [](py::object parent_obj) -> LvObjWrapper* {')
        self.emit("        LvObjWrapper *_parent = parent_obj.is_none() ? nullptr : parent_obj.cast<LvObjWrapper*>();")
        self.emit("        LvObjWrapper *wrapper = new LvObjWrapper(lv_menu_cont_create(_parent ? _parent->get() : lv_screen_active()));")
        self.emit("        if (_parent) wrapper->keep_parent(parent_obj);")
        self.emit('        return wrapper;')
        self.emit('    }, py::arg("parent") = py::none());')
        # MenuSection factory
        self.emit('    m.def("MenuSection", [](py::object parent_obj) -> LvObjWrapper* {')
        self.emit("        LvObjWrapper *_parent = parent_obj.is_none() ? nullptr : parent_obj.cast<LvObjWrapper*>();")
        self.emit("        LvObjWrapper *wrapper = new LvObjWrapper(lv_menu_section_create(_parent ? _parent->get() : lv_screen_active()));")
        self.emit("        if (_parent) wrapper->keep_parent(parent_obj);")
        self.emit('        return wrapper;')
        self.emit('    }, py::arg("parent") = py::none());')
        self.emit("")

    # -----------------------------------------------------------------------
    # Animimg manual bindings (set_src takes C array, not auto-generatable)
    # -----------------------------------------------------------------------

    def _emit_animimg_manual_bindings(self):
        """Emit manual binding for animimg.set_src which takes a Python list of paths.

        The C API lv_animimg_set_src(obj, const void *dsc[], size_t num) takes
        a C array which cannot be auto-wrapped. We use a helper that accepts
        py::list and converts to the required C array.
        """
        self.emit("    /* Animimg: manual binding for set_src (C array param) */")
        self.emit('    obj_cls.def("set_src", [](LvObjWrapper &self, py::list sources) { py_animimg_set_src(self.get(), sources); return; }')
        self.emit('        , py::arg("sources"), "Set image source array from list of file paths");')
        self.emit("")

    # -----------------------------------------------------------------------
    # Event callbacks
    # -----------------------------------------------------------------------

    def _emit_event_callbacks(self):
        self.emit("    /* Event object and callback support */")
        self.emit("    py::class_<LvEventWrapper>(m, \"event\")")
        self.emit('        .def_property_readonly("code", [](LvEventWrapper &e) -> int {')
        self.emit("            return static_cast<int>(lv_event_get_code(e.get()));")
        self.emit("        })")
        self.emit('        .def_property_readonly("target", [](LvEventWrapper &e) -> LvObjWrapper* {')
        self.emit("            lv_obj_t *t = (lv_obj_t *)lv_event_get_target(e.get());")
        self.emit("            return t ? new LvObjWrapper(t) : nullptr;")
        self.emit("        })")
        self.emit('        .def_property_readonly("current_target", [](LvEventWrapper &e) -> LvObjWrapper* {')
        self.emit("            lv_obj_t *t = (lv_obj_t *)lv_event_get_current_target(e.get());")
        self.emit("            return t ? new LvObjWrapper(t) : nullptr;")
        self.emit("        })")
        self.emit('        .def("stop_bubbling", [](LvEventWrapper &e) {')
        self.emit("            lv_event_stop_bubbling(e.get());")
        self.emit("        })")
        self.emit('        .def("stop_processing", [](LvEventWrapper &e) {')
        self.emit("            lv_event_stop_processing(e.get());")
        self.emit("        })")
        self.emit('        .def("stop_trickling", [](LvEventWrapper &e) {')
        self.emit("            lv_event_stop_trickling(e.get());")
        self.emit("        })")
        self.emit('        .def("__eq__", [](LvEventWrapper &e, int code) -> bool {')
        self.emit("            return static_cast<int>(lv_event_get_code(e.get())) == code;")
        self.emit("        })")
        self.emit('        .def("__eq__", [](LvEventWrapper &e, LvEventWrapper &other) -> bool {')
        self.emit("            return e.get() == other.get();")
        self.emit("        })")
        self.emit('        .def("__int__", [](LvEventWrapper &e) -> int {')
        self.emit("            return static_cast<int>(lv_event_get_code(e.get()));")
        self.emit("        })")
        self.emit('        .def("__repr__", [](LvEventWrapper &e) -> std::string {')
        self.emit('            return "event(code=" + std::to_string(static_cast<int>(lv_event_get_code(e.get()))) + ")";')
        self.emit("        });")
        self.emit("")
        self.emit("    /* Event callback registration */")
        self.emit('    obj_cls.def("add_event_cb", [](LvObjWrapper &self, int filter, py::function callback) {')
        self.emit("        register_event_callback(self.get(), (lv_event_code_t)filter, callback);")
        self.emit('    }, py::arg("filter"), py::arg("callback"));')
        self.emit("")

    # -----------------------------------------------------------------------
    # Color type
    # -----------------------------------------------------------------------

    def _emit_color_type(self):
        self.emit("    /* Color type */")
        self.emit("    py::class_<lv_color_t>(m, \"color\")")
        self.emit("        .def(py::init<>())")
        self.emit('        .def_static("from_rgb", [](uint8_t r, uint8_t g, uint8_t b) -> lv_color_t {')
        self.emit("            return lv_color_make(r, g, b);")
        self.emit('        }, py::arg("r"), py::arg("g"), py::arg("b"))')
        self.emit('        .def_static("from_hex", [](uint32_t c) -> lv_color_t {')
        self.emit("            return lv_color_hex(c);")
        self.emit('        }, py::arg("c"))')
        self.emit('        .def_readonly("red", &lv_color_t::red)')
        self.emit('        .def_readonly("green", &lv_color_t::green)')
        self.emit('        .def_readonly("blue", &lv_color_t::blue);')
        self.emit("")
        self.emit("    /* Color utilities */")
        self.emit('    m.def("color_make", [](uint8_t r, uint8_t g, uint8_t b) { return lv_color_make(r, g, b); },')
        self.emit('        py::arg("r"), py::arg("g"), py::arg("b"));')
        self.emit('    m.def("color_hex", [](uint32_t c) { return lv_color_hex(c); }, py::arg("c"));')
        self.emit('    m.def("color_black", []() { return lv_color_black(); });')
        self.emit('    m.def("color_white", []() { return lv_color_white(); });')
        self.emit("")

    # -----------------------------------------------------------------------
    # Display wrapper
    # -----------------------------------------------------------------------

    def _emit_display_class(self):
        self.emit("    /* Display */")
        self.emit("    py::class_<LvDisplayWrapper>(m, \"display\")")
        self.emit("        .def(py::init<>())")
        self.emit('        .def("set_resolution", [](LvDisplayWrapper &self, int32_t h, int32_t v) {')
        self.emit("            lv_display_set_resolution(self.get(), h, v);")
        self.emit('        }, py::arg("hor_res"), py::arg("ver_res"))')
        self.emit('        .def("set_rotation", [](LvDisplayWrapper &self, lv_display_rotation_t r) {')
        self.emit("            lv_display_set_rotation(self.get(), r);")
        self.emit('        }, py::arg("rotation"))')
        self.emit('        .def("get_horizontal_resolution", [](LvDisplayWrapper &self) -> int32_t {')
        self.emit("            return lv_display_get_horizontal_resolution(self.get());")
        self.emit("        })")
        self.emit('        .def("get_vertical_resolution", [](LvDisplayWrapper &self) -> int32_t {')
        self.emit("            return lv_display_get_vertical_resolution(self.get());")
        self.emit("        })")
        self.emit('        .def("flush_ready", [](LvDisplayWrapper &self) {')
        self.emit("            lv_display_flush_ready(self.get());")
        self.emit("        });")
        self.emit("")
        self.emit('    m.def("display_create", [](int32_t h, int32_t v) -> LvDisplayWrapper* {')
        self.emit("        return new LvDisplayWrapper(lv_display_create(h, v));")
        self.emit('    }, py::arg("hor_res"), py::arg("ver_res"));')
        self.emit('    m.def("display_get_default", []() -> LvDisplayWrapper* {')
        self.emit("        return new LvDisplayWrapper(lv_display_get_default());")
        self.emit("    });")
        self.emit("")

    # -----------------------------------------------------------------------
    # Indev wrapper
    # -----------------------------------------------------------------------

    def _emit_indev_class(self):
        self.emit("    /* Input device */")
        self.emit("    py::class_<LvIndevWrapper>(m, \"indev\")")
        self.emit("        .def(py::init<>());")
        self.emit("")
        self.emit('    m.def("indev_create", []() -> LvIndevWrapper* {')
        self.emit("        return new LvIndevWrapper(lv_indev_create());")
        self.emit("    });")
        self.emit("")

    # -----------------------------------------------------------------------
    # Timer wrapper
    # -----------------------------------------------------------------------

    def _emit_timer_class(self):
        """Emit LvTimerWrapper class and timer_create module function.

        lv_timer_create(callback, period) creates a timer that calls the Python
        callback periodically. The callback receives an LvTimerWrapper.
        """
        self.emit("    /* Timer */")
        self.emit('    py::class_<LvTimerWrapper>(m, "timer_t")')
        self.emit("        .def(py::init<>())")
        self.emit('        .def("is_valid", &LvTimerWrapper::is_valid)')
        self.emit('        .def("delete", [](LvTimerWrapper &self) { self.del(); }, "Delete the timer")')
        self.emit('        .def("pause", [](LvTimerWrapper &self) { lv_timer_pause(self.get()); })')
        self.emit('        .def("resume", [](LvTimerWrapper &self) { lv_timer_resume(self.get()); })')
        self.emit('        .def("set_period", [](LvTimerWrapper &self, uint32_t period) { lv_timer_set_period(self.get(), period); }, py::arg("period"))')
        self.emit('        .def("ready", [](LvTimerWrapper &self) { lv_timer_ready(self.get()); });')
        self.emit("")
        self.emit('    m.def("timer_create", [](py::function callback, uint32_t period) -> LvTimerWrapper* {')
        self.emit("        uint64_t cb_id = CallbackManager::instance().store(std::move(callback));")
        self.emit("        lv_timer_t *t = lv_timer_create(lvgl_timer_cb_trampoline, period, reinterpret_cast<void*>(static_cast<uintptr_t>(cb_id)));")
        self.emit("        return new LvTimerWrapper(t);")
        self.emit('    }, py::arg("callback"), py::arg("period"), "Create a timer that calls callback every period ms");')
        self.emit("")

    # -----------------------------------------------------------------------
    # Animation wrapper
    # -----------------------------------------------------------------------

    def _emit_anim_class(self):
        """Emit LvAnimWrapper class with animation API.

        Usage:
            a = lv.anim_t()       # constructs and calls lv_anim_init()
            a.set_var(obj)
            a.set_exec_cb(cb)     # Python callback: cb(var, value)
            a.set_values(start, end)
            a.set_duration(ms)
            a.set_repeat_count(lv.ANIM_REPEAT.INFINITE)
            a.start()
        """
        self.emit("    /* Animation */")
        self.emit('    py::class_<LvAnimWrapper>(m, "anim_t")')
        # Constructor calls lv_anim_init internally
        self.emit("        .def(py::init<>())")
        self.emit('        .def("set_var", [](LvAnimWrapper &self, LvObjWrapper &var) { lv_anim_set_var(self.get(), var.get()); }, py::arg("var"))')
        # exec_cb: register Python callback keyed by the var pointer
        self.emit('        .def("set_exec_cb", [](LvAnimWrapper &self, LvObjWrapper &var, py::function callback) {')
        self.emit("            register_anim_exec_cb(var.get(), std::move(callback));")
        self.emit("            lv_anim_set_exec_cb(self.get(), lvgl_anim_exec_trampoline);")
        self.emit('        }, py::arg("var"), py::arg("callback"), "Set exec callback (must also pass the var object)")')
        self.emit('        .def("set_values", [](LvAnimWrapper &self, int32_t start, int32_t end) { lv_anim_set_values(self.get(), start, end); }, py::arg("start"), py::arg("end"))')
        self.emit('        .def("set_duration", [](LvAnimWrapper &self, uint32_t duration) { lv_anim_set_duration(self.get(), duration); }, py::arg("duration"))')
        self.emit('        .def("set_delay", [](LvAnimWrapper &self, uint32_t delay) { lv_anim_set_delay(self.get(), delay); }, py::arg("delay"))')
        self.emit('        .def("set_reverse_duration", [](LvAnimWrapper &self, uint32_t duration) { lv_anim_set_reverse_duration(self.get(), duration); }, py::arg("duration"))')
        self.emit('        .def("set_reverse_delay", [](LvAnimWrapper &self, uint32_t delay) { lv_anim_set_reverse_delay(self.get(), delay); }, py::arg("delay"))')
        self.emit('        .def("set_repeat_count", [](LvAnimWrapper &self, int32_t count) { lv_anim_set_repeat_count(self.get(), count); }, py::arg("count"))')
        self.emit('        .def("set_repeat_delay", [](LvAnimWrapper &self, uint32_t delay) { lv_anim_set_repeat_delay(self.get(), delay); }, py::arg("delay"))')
        # Predefined path functions
        self.emit('        .def("set_path_linear", [](LvAnimWrapper &self) { lv_anim_set_path_cb(self.get(), lv_anim_path_linear); })')
        self.emit('        .def("set_path_ease_in", [](LvAnimWrapper &self) { lv_anim_set_path_cb(self.get(), lv_anim_path_ease_in); })')
        self.emit('        .def("set_path_ease_out", [](LvAnimWrapper &self) { lv_anim_set_path_cb(self.get(), lv_anim_path_ease_out); })')
        self.emit('        .def("set_path_ease_in_out", [](LvAnimWrapper &self) { lv_anim_set_path_cb(self.get(), lv_anim_path_ease_in_out); })')
        self.emit('        .def("set_path_overshoot", [](LvAnimWrapper &self) { lv_anim_set_path_cb(self.get(), lv_anim_path_overshoot); })')
        self.emit('        .def("set_path_bounce", [](LvAnimWrapper &self) { lv_anim_set_path_cb(self.get(), lv_anim_path_bounce); })')
        self.emit('        .def("set_path_step", [](LvAnimWrapper &self) { lv_anim_set_path_cb(self.get(), lv_anim_path_step); })')
        # completed_cb: register Python callback keyed by the var pointer
        self.emit('        .def("set_completed_cb", [](LvAnimWrapper &self, LvObjWrapper &var, py::function callback) {')
        self.emit("            register_anim_completed_cb(var.get(), std::move(callback));")
        self.emit("            lv_anim_set_completed_cb(self.get(), lvgl_anim_completed_trampoline);")
        self.emit('        }, py::arg("var"), py::arg("callback"))')
        self.emit('        .def("set_early_apply", [](LvAnimWrapper &self, bool en) { lv_anim_set_early_apply(self.get(), en); }, py::arg("en"))')
        # start: lv_anim_start copies the struct and returns lv_anim_t* (LVGL-owned, not safe to wrap)
        # We discard the return value since users don't need it.
        self.emit('        .def("start", [](LvAnimWrapper &self) { lv_anim_start(self.get()); }, "Start the animation")')
        self.emit('        .def_static("delete", [](LvObjWrapper &var) { lv_anim_delete(var.get(), NULL); }, py::arg("var"), "Delete animations on var");')
        self.emit("")
        # ANIM_REPEAT constants
        self.emit("    /* Animation repeat count constants */")
        self.emit('    m.attr("ANIM_REPEAT_INFINITE") = (int)LV_ANIM_REPEAT_INFINITE;')
        self.emit("")

    # -----------------------------------------------------------------------
    # Theme bindings
    # -----------------------------------------------------------------------

    def _emit_theme_bindings(self):
        """Emit theme_default_init binding.

        lv_theme_default_init(display, primary_color, secondary_color, dark, font)
        Returns nothing (the theme is applied to the display).
        """
        self.emit("    /* Theme */")
        self.emit('    m.def("theme_default_init", [](LvDisplayWrapper *disp, lv_color_t primary, lv_color_t secondary, bool dark, uintptr_t font_addr) {')
        self.emit("        const lv_font_t *font = reinterpret_cast<const lv_font_t*>(font_addr);")
        self.emit("        lv_theme_default_init(disp ? disp->get() : NULL, primary, secondary, dark, font);")
        self.emit('    }, py::arg("display") = py::none(), py::arg("primary"), py::arg("secondary"), py::arg("dark") = true, py::arg("font") = reinterpret_cast<uintptr_t>((const lv_font_t *)lv_font_get_default()), "Initialize default theme");')
        self.emit('    m.def("theme_get_color_primary", [](LvObjWrapper *obj) -> lv_color_t { return lv_theme_get_color_primary(obj ? obj->get() : NULL); }, py::arg("obj") = py::none(), "Get primary theme color");')
        self.emit("")

    # -----------------------------------------------------------------------
    # obj.remove_style binding (style param as NULL)
    # -----------------------------------------------------------------------

    def _emit_obj_remove_style_binding(self):
        """Emit obj.remove_style(selector) that calls lv_obj_remove_style with NULL style.

        lv_obj_remove_style(obj, style, selector) takes a const lv_style_t* which is
        opaque from Python. The common usage (remove all styles for a given selector)
        passes NULL as the style. We expose a Python-friendly version.
        """
        self.emit("    /* obj.remove_style — remove styles by selector (NULL style) */")
        self.emit('    obj_cls.def("remove_style", [](LvObjWrapper &self, uint32_t selector) {')
        self.emit("        lv_obj_remove_style(self.get(), NULL, selector);")
        self.emit('    }, py::arg("selector") = (uint32_t)LV_PART_ANY, "Remove styles matching selector");')
        self.emit('    obj_cls.def("remove_style", [](LvObjWrapper &self, uint32_t selector, uint32_t state) {')
        self.emit("        lv_obj_remove_style(self.get(), NULL, selector | state);")
        self.emit('    }, py::arg("selector"), py::arg("state"), "Remove styles matching selector|state");')
        self.emit("")

    # -----------------------------------------------------------------------
    # FreeType font support
    # -----------------------------------------------------------------------

    # Widgets whose create function should pass NULL (not lv_screen_active())
    # when parent is None, so that LVGL creates a backdrop on lv_layer_top().
    NULL_PARENT_WIDGETS = {"lv_msgbox_create"}

    def _emit_freetype_font(self):
        """Emit FreeType font bindings and LvFontWrapper overload for set_style_text_font."""
        self.emit("    /* FreeType font support */")
        self.emit("#if LV_USE_FREETYPE")
        self.emit('    auto font_cls = py::class_<LvFontWrapper>(m, "font", py::dynamic_attr());')
        self.emit("    font_cls.def(py::init<>());")
        self.emit('    font_cls.def("is_valid", &LvFontWrapper::is_valid);')
        self.emit("")
        self.emit('    m.def("freetype_font_create", [](const char *pathname,')
        self.emit("                                     lv_freetype_font_render_mode_t render_mode,")
        self.emit("                                     uint32_t size,")
        self.emit("                                     lv_freetype_font_style_t style) -> LvFontWrapper* {")
        self.emit("        lv_font_t *font = lv_freetype_font_create(pathname, render_mode, size, style);")
        self.emit("        if(!font) {")
        self.emit('            PyErr_SetString(PyExc_RuntimeError,')
        self.emit('                            "freetype_font_create failed - check font path");')
        self.emit("            throw py::error_already_set();")
        self.emit("        }")
        self.emit("        return new LvFontWrapper(font, true);")
        self.emit('    }, py::arg("pathname"), py::arg("render_mode"), py::arg("size"), py::arg("style"));')
        self.emit("")
        self.emit('    m.def("freetype_font_delete", [](LvFontWrapper &font) {')
        self.emit("        lv_freetype_font_delete(font.get());")
        self.emit("    });")
        self.emit("")
        self.emit("    /* LvFontWrapper overload for set_style_text_font */")
        self.emit('    obj_cls.def("set_style_text_font", [](LvObjWrapper &self, LvFontWrapper &font, uint32_t selector) {')
        self.emit("        lv_obj_set_style_text_font(self.get(), font.get(), selector); return;")
        self.emit('    }, py::arg("value"), py::arg("selector") = 0);')
        self.emit("#endif /* LV_USE_FREETYPE */")
        self.emit("")

    # -----------------------------------------------------------------------
    # Font helpers
    # -----------------------------------------------------------------------

    def _emit_font_helpers(self):
        """Emit Montserrat font pointer helpers."""
        self.emit("    /* Font default */")
        self.emit('    m.def("font_get_default", []() -> const lv_font_t * { return lv_font_get_default(); });')
        self.emit("")
        self.emit("    /* Font pointer helpers - guarded by lv_conf.h */")
        for size in [8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30, 32, 34, 36, 38, 40, 42, 44, 46, 48]:
            self.emit("#if LV_FONT_MONTSERRAT_%d" % size)
            self.emit('    m.def("font_montserrat_%d", []() -> const lv_font_t * { return &lv_font_montserrat_%d; });' % (size, size))
            self.emit("#endif")
        self.emit("")

    # -----------------------------------------------------------------------
    # Driver backends
    # -----------------------------------------------------------------------

    def _emit_driver_backends(self):
        self.emit("    /* Driver backend functions */")
        self.emit('    m.def("driver_backends_register", []() { driver_backends_register(); },')
        self.emit('        "Register all available driver backends");')
        self.emit('    m.def("driver_backends_init_backend", [](const std::string &name) -> int {')
        self.emit('        return driver_backends_init_backend(const_cast<char*>(name.c_str()));')
        self.emit('    }, py::arg("backend_name"),')
        self.emit('        "Initialize a driver backend by name");')
        self.emit('    m.def("driver_backends_is_supported", [](const std::string &name) -> int {')
        self.emit('        return driver_backends_is_supported(const_cast<char*>(name.c_str()));')
        self.emit('    }, py::arg("backend_name"),')
        self.emit('        "Check if a backend is supported");')
        self.emit('    m.def("driver_backends_print_supported", []() -> int {')
        self.emit('        return driver_backends_print_supported();')
        self.emit('    }, "Print supported backends");')
        self.emit('    m.def("driver_backends_run_loop", []() { driver_backends_run_loop(); },')
        self.emit('        "Enter the run loop");')
        self.emit('    m.def("k230_driver_init", [](uintptr_t display_ptr, int v4l2_drm_run_flag) -> int {')
        self.emit('        return k230_driver_init(reinterpret_cast<void*>(display_ptr), static_cast<char>(v4l2_drm_run_flag));')
        self.emit('    }, py::arg("display_ptr"), py::arg("v4l2_drm_run_flag"),')
        self.emit('        "Initialize K230 display driver with v4l2-drm backend");')
        self.emit("")
        self.emit("    /* Global simulator settings (defined in driver_backends.c): direct")
        self.emit("     * read/write access, mirrors the C simulator's settings global (the")
        self.emit("     * -R option sets settings.rotation). rotation is consumed by the DRM")
        self.emit("     * backend at init (init_drm -> k230_set_drm_rotation) and by the")
        self.emit("     * evdev touch hook; DRM_V4L2_K230 overwrites it from v4l2drm after init. */")
        self.emit('    py::class_<simulator_settings_t>(m, "SimulatorSettings")')
        self.emit('        .def_readwrite("rotation", &simulator_settings_t::rotation);')
        self.emit('    m.attr("settings") = py::cast(&settings, py::return_value_policy::reference);')
        self.emit("")

    # -----------------------------------------------------------------------
    # Keyboard group / indev helpers (for physical keyboard input via evdev)
    # -----------------------------------------------------------------------

    def _emit_keyboard_group_helpers(self):
        """Emit manual bindings for lv_group_*, lv_indev_set_group/set_type/get_type/
        get_next and lv_evdev_create.

        The auto-generator skips all of these because lv_group_t* is an
        unsupported param/return type and the indev/evdev functions take
        non-obj-wrapper pointer params. Without these, physical keyboard
        input (USB/Bluetooth via evdev) is unreachable from Python — LVGL
        groups are how widgets receive focus from keypad/encoder input
        devices. Group handles are passed to/from Python as uintptr_t
        integers (same pattern as chart series handles).
        """
        self.emit("    /* Keyboard group helpers — bridge for physical keyboard")
        self.emit("     * input via evdev. The lv_group_* API is not auto-generated")
        self.emit("     * (lv_group_t* is unsupported), so we provide a minimal set")
        self.emit("     * of helpers here. Group handles are passed as uintptr_t. */")

        # Group lifecycle and membership
        self.emit('    m.def("group_create", []() -> uintptr_t {')
        self.emit("        return reinterpret_cast<uintptr_t>(lv_group_create());")
        self.emit('    }, "Create a new LVGL group and return its pointer as an integer");')
        self.emit('    m.def("group_set_default", [](uintptr_t group_ptr) {')
        self.emit("        lv_group_set_default(reinterpret_cast<lv_group_t*>(group_ptr));")
        self.emit('    }, py::arg("group"), "Set the default group");')
        self.emit('    m.def("group_get_default", []() -> uintptr_t {')
        self.emit("        return reinterpret_cast<uintptr_t>(lv_group_get_default());")
        self.emit('    }, "Get the default group pointer as an integer");')
        self.emit('    m.def("group_add_obj", [](uintptr_t group_ptr, LvObjWrapper &obj) {')
        self.emit("        lv_group_add_obj(reinterpret_cast<lv_group_t*>(group_ptr), obj.get());")
        self.emit('    }, py::arg("group"), py::arg("obj"), "Add an object to a group");')
        self.emit('    m.def("group_remove_obj", [](LvObjWrapper &obj) {')
        self.emit("        lv_group_remove_obj(obj.get());")
        self.emit('    }, py::arg("obj"), "Remove an object from its group");')
        self.emit('    m.def("group_focus_next", [](uintptr_t group_ptr) {')
        self.emit("        lv_group_focus_next(reinterpret_cast<lv_group_t*>(group_ptr));")
        self.emit('    }, py::arg("group"), "Focus the next object in the group");')
        self.emit('    m.def("group_focus_prev", [](uintptr_t group_ptr) {')
        self.emit("        lv_group_focus_prev(reinterpret_cast<lv_group_t*>(group_ptr));")
        self.emit('    }, py::arg("group"), "Focus the previous object in the group");')
        self.emit('    m.def("group_focus_obj", [](LvObjWrapper &obj) {')
        self.emit("        lv_group_focus_obj(obj.get());")
        self.emit('    }, py::arg("obj"), "Focus a specific object in its group");')
        self.emit('    m.def("group_set_wrap", [](uintptr_t group_ptr, bool en) {')
        self.emit("        lv_group_set_wrap(reinterpret_cast<lv_group_t*>(group_ptr), en);")
        self.emit('    }, py::arg("group"), py::arg("wrap"), "Set whether the group wraps on focus navigation");')
        self.emit('    m.def("group_delete", [](uintptr_t group_ptr) {')
        self.emit("        lv_group_delete(reinterpret_cast<lv_group_t*>(group_ptr));")
        self.emit('    }, py::arg("group"), "Delete a group");')
        self.emit("")

        # Indev helpers — connect a keypad device to a group
        self.emit("    /* Indev helpers for physical keyboard */")
        self.emit('    m.def("indev_set_type", [](LvIndevWrapper &indev, lv_indev_type_t type) {')
        self.emit("        lv_indev_set_type(indev.get(), type);")
        self.emit('    }, py::arg("indev"), py::arg("type"), "Set the input device type");')
        self.emit('    m.def("indev_set_group", [](LvIndevWrapper &indev, uintptr_t group_ptr) {')
        self.emit("        lv_indev_set_group(indev.get(), reinterpret_cast<lv_group_t*>(group_ptr));")
        self.emit('    }, py::arg("indev"), py::arg("group"), "Assign a group to an input device");')
        self.emit('    m.def("indev_get_type", [](LvIndevWrapper &indev) -> lv_indev_type_t {')
        self.emit("        return lv_indev_get_type(indev.get());")
        self.emit('    }, py::arg("indev"), "Get the input device type");')
        self.emit('    m.def("indev_get_next", [](LvIndevWrapper *indev) -> LvIndevWrapper* {')
        self.emit("        lv_indev_t *next = lv_indev_get_next(indev ? indev->get() : NULL);")
        self.emit("        return next ? new LvIndevWrapper(next) : nullptr;")
        self.emit('    }, py::arg("indev") = py::none(), "Get the next input device (pass None for first)");')
        self.emit("")

        # evdev — open a specific keyboard device path
        self.emit("#if LV_USE_EVDEV")
        self.emit('    m.def("evdev_create", [](lv_indev_type_t indev_type, const std::string &dev_path) -> LvIndevWrapper* {')
        self.emit("        lv_indev_t *indev = lv_evdev_create(indev_type, dev_path.c_str());")
        self.emit("        return indev ? new LvIndevWrapper(indev) : nullptr;")
        self.emit('    }, py::arg("indev_type"), py::arg("dev_path"),')
        self.emit('        "Create an evdev input device (e.g. LV_INDEV_TYPE_KEYPAD, \'/dev/input/event0\')");')
        self.emit("#endif")
        self.emit("")

    # -----------------------------------------------------------------------
    # Not-generated list
    # -----------------------------------------------------------------------

    def _emit_not_generated_list(self):
        skipped = self.ir.get_skipped_functions()
        if not skipped:
            return

        self.emit("")
        self.emit("    /* Functions not generated:")
        for func in skipped:
            reason = func.skip_reason or "unknown"
            self.emit("     * %s (%s)" % (func.name, reason))
        self.emit("     */")

    # -----------------------------------------------------------------------
    # Helpers
    # -----------------------------------------------------------------------

    @staticmethod
    def _is_obj_ptr(type_str: str) -> bool:
        """Check if a type is lv_obj_t* or a widget pointer."""
        type_str = type_str.strip()
        if type_str in ("lv_obj_t *", "lv_obj_t*"):
            return True
        # Check for const variants
        if type_str in ("const lv_obj_t *", "const lv_obj_t*"):
            return True
        return False

    @staticmethod
    def _sanitize(name: str) -> str:
        """Sanitize a Python identifier."""
        PYTHON_KEYWORDS = [
            "False", "None", "True", "and", "as", "assert", "break", "class",
            "continue", "def", "del", "elif", "else", "except", "finally",
            "for", "from", "global", "if", "import", "in", "is", "lambda",
            "nonlocal", "not", "or", "pass", "raise", "return", "try",
            "while", "with", "yield",
        ]
        if name in PYTHON_KEYWORDS:
            name = "_%s" % name
        name = name.strip().replace(" ", "_").replace("*", "_ptr")
        # Python identifiers cannot start with a digit; prepend enum class prefix
        # This handles cases like LV_OPA_50 → "50" → "OPA_50"
        if name and name[0].isdigit():
            name = "_" + name
        return name
