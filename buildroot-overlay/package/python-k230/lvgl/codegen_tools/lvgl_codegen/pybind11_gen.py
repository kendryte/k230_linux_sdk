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
        self._emit_event_callbacks()
        self._emit_color_type()
        self._emit_display_class()
        self._emit_indev_class()
        self._emit_freetype_font()
        self._emit_driver_backends()
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
        self.emit('#include "lvgl/demos/benchmark/lv_demo_benchmark.h"')
        self.emit('#include "lvgl/font/lv_freetype.h"')
        self.emit('#include "lvgl_pybind_helpers.h"')
        self.emit("")
        self.emit("namespace py = pybind11;")
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

        # Second pass: manually inject non-conflicting enum values into module namespace
        # Values that clash with an enum type name or appear in multiple enums are
        # only accessible via their enum type (e.g., lv.EVENT.KEY, lv.ALIGN.CENTER)
        self.emit("    /*")
        self.emit("     * Export non-conflicting enum values into module namespace")
        self.emit("     * (conflicting values are only accessible via their enum type)")
        self.emit("     */")
        for py_name, safe_name, member_c_name in all_value_names:
            if safe_name not in conflicting_values:
                self.emit('    m.attr("%s") = (int)%s;' % (safe_name, member_c_name))
        self.emit("")

    @staticmethod
    def _strip_enum_prefix(c_name: str, enum_py_name: str = "") -> str:
        """Strip LV_ prefix and enum class name prefix from a C enum member name.

        Examples:
            LV_ALIGN_TOP_LEFT + ALIGN → TOP_LEFT
            LV_EVENT_CLICKED + EVENT → CLICKED
            LV_OBJ_FLAG_CLICKABLE + OBJ_FLAG → CLICKABLE
            LV_ARC_MODE_NORMAL + ARC → NORMAL  (strips ARC_ then MODE_ via suffix)
            LV_OPA_50 + OPA → 50
            LV_STR_SYMBOL_AUDIO + SYMBOL → AUDIO
        """
        name = c_name
        # Strip LV_ prefix
        if name.startswith("LV_"):
            name = name[3:]
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
    # IMPORTANT: Do NOT strip suffixes that would cause name collisions with
    # other enums (e.g., _ORIENTATION on BAR_ORIENTATION → BAR collides with
    # BAR_MODE → BAR; _EDITABLE/_GROUP_DEF on OBJ_CLASS_* → OBJ_CLASS collides).
    ENUM_NAME_SUFFIX_STRIP = {
        "_CODE": "",       # EVENT_CODE → EVENT
        "_TYPE": "",       # CHART_TYPE → CHART, INDEV_TYPE → INDEV
        "_MODE": "",       # ARC_MODE → ARC, BAR_MODE → BAR, SCROLLBAR_MODE → SCROLLBAR
        "_RES": "",        # COVER_RES → COVER, FS_RES → FS
        "_CMP": "",        # STYLE_STATE_CMP → STYLE_STATE
        "_WALK_RES": "",   # OBJ_TREE_WALK_RES → OBJ_TREE_WALK
        "_TRANSFORM_FLAG": "", # OBJ_POINT_TRANSFORM_FLAG → OBJ_POINT
        "_REFOCUS_POLICY": "", # GROUP_REFOCUS_POLICY → GROUP
        "_GESTURE_TYPE": "", # INDEV_GESTURE_TYPE → INDEV_GESTURE
        "_LONG_MODE": "",  # LABEL_LONG_MODE → LABEL_LONG
        "_ENCODING": "",   # BARCODE_ENCODING → BARCODE
        # "_ORIENTATION": "",  # REMOVED: BAR_ORIENTATION → BAR collides with BAR_MODE → BAR
        "_RENDER_MODE": "", # DISPLAY_RENDER_MODE → DISPLAY_RENDER
        "_LOAD_ANIM": "",  # SCREEN_LOAD_ANIM → SCREEN_LOAD
        "_COMPRESS": "",   # IMAGE_COMPRESS → IMAGE
        "_SRC": "",        # IMAGE_SRC → IMAGE
        "_INHERITABLE": "", # OBJ_CLASS_THEME_INHERITABLE → OBJ_CLASS_THEME
        # "_EDITABLE": "",  # REMOVED: OBJ_CLASS_EDITABLE → OBJ_CLASS collides with OBJ_CLASS_GROUP_DEF
        # "_GROUP_DEF": "", # REMOVED: OBJ_CLASS_GROUP_DEF → OBJ_CLASS collides with OBJ_CLASS_EDITABLE
    }

    @staticmethod
    def _enum_python_name(c_type: str) -> str:
        """Convert lv_align_t → ALIGN, lv_event_code_t → EVENT, etc."""
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
        self.emit('    m.def("task_handler", &py_timer_handler, "Alias for timer_handler");')
        self.emit('    m.def("tick_get", &lv_tick_get, "Get elapsed milliseconds");')
        self.emit('    m.def("tick_inc", &lv_tick_inc, "Update tick value", py::arg("ms"));')
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
        self.emit("    auto obj_cls = py::class_<LvObjWrapper>(m, \"Obj\", py::dynamic_attr());")
        self.emit("    obj_cls.def(py::init<>());")
        # Expose keep_parent for lifecycle management (called internally by factory functions)
        self.emit('    obj_cls.def("_keep_parent", [](LvObjWrapper &self, py::object parent) {')
        self.emit("        self.keep_parent(parent);")
        self.emit("    }, py::arg(\"parent\"));")

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
            py_name = widget_name[0].upper() + widget_name[1:]
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

        # If this python method name collides across widgets, prefix with widget name
        # e.g. set_rotation (owned by arc, image, scale) → arc_set_rotation, image_set_rotation, scale_set_rotation
        if colliding_names and method_name in colliding_names and widget_name:
            method_name = "%s_%s" % (widget_name, method_name)

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
            if cpp_type.endswith("Wrapper &"):
                call_args.append("%s.get()" % arg_name)
            elif param.c_type == "const void *" and cpp_type == "const char *":
                call_args.append("(const void *)%s" % arg_name)
            else:
                call_args.append(arg_name)
            lambda_params.append("%s %s" % (cpp_type, arg_name))

            # Add default value for style selector parameters (uint32_t selector → 0)
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

        self.emit('    obj_cls.def("refresh_chart", [](LvObjWrapper &self) {')
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
    # Event callbacks
    # -----------------------------------------------------------------------

    def _emit_event_callbacks(self):
        self.emit("    /* Event callback support */")
        self.emit('    obj_cls.def("add_event_cb", [](LvObjWrapper &self, int filter, py::function callback) {')
        self.emit("        register_event_callback(self.get(), (lv_event_code_t)filter, callback);")
        self.emit('    }, py::arg("filter"), py::arg("callback"));')
        self.emit("")

    # -----------------------------------------------------------------------
    # Color type
    # -----------------------------------------------------------------------

    def _emit_color_type(self):
        self.emit("    /* Color type */")
        self.emit("    py::class_<lv_color_t>(m, \"Color\")")
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
        self.emit("    py::class_<LvDisplayWrapper>(m, \"Display\")")
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
        self.emit("    py::class_<LvIndevWrapper>(m, \"Indev\")")
        self.emit("        .def(py::init<>());")
        self.emit("")
        self.emit('    m.def("indev_create", []() -> LvIndevWrapper* {')
        self.emit("        return new LvIndevWrapper(lv_indev_create());")
        self.emit("    });")
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
        self.emit('    auto font_cls = py::class_<LvFontWrapper>(m, "Font", py::dynamic_attr());')
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
