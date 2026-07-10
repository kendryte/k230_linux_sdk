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
        self._emit_event_callbacks()
        self._emit_color_type()
        self._emit_display_class()
        self._emit_indev_class()
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

        for enum_ir in self.ir.enums:
            if not enum_ir.members:
                continue

            # Derive Python name from C type
            c_type = enum_ir.c_type
            py_name = self._enum_python_name(c_type)

            if c_type and c_type.startswith("lv_"):
                self.emit('    py::enum_<%s>(m, "%s")' % (c_type, py_name))
                for member in enum_ir.members:
                    safe_name = self._sanitize(member.python_name or member.c_name)
                    self.emit('        .value("%s", %s)' % (safe_name, member.c_name))
                self.emit("        .export_values();")
                self.emit("")
            else:
                # Emit as integer constants
                self.emit("    /* Enum %s (no C type found, emitting as constants) */" % c_type)
                for member in enum_ir.members:
                    safe_name = self._sanitize(member.c_name)
                    self.emit('    m.attr("%s") = (int)%s;' % (safe_name, member.c_name))
                self.emit("")

    @staticmethod
    def _enum_python_name(c_type: str) -> str:
        """Convert lv_align_t → ALIGN, lv_dir_t → DIR, etc."""
        if c_type.startswith("lv_") and c_type.endswith("_t"):
            name = c_type[3:-2]  # strip lv_ and _t
            return name.upper()
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
        self.emit("    auto obj_cls = py::class_<LvObjWrapper>(m, \"Obj\");")
        self.emit("    obj_cls.def(py::init<>());")
        self.emit('    obj_cls.def_static("create", [](LvObjWrapper *parent) -> LvObjWrapper* {')
        self.emit("        lv_obj_t *p = parent ? parent->get() : nullptr;")
        self.emit("        return new LvObjWrapper(lv_obj_create(p));")
        self.emit('    }, py::arg("parent") = py::none());')

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
                lambda_params.append("LvObjWrapper *parent")
                py_args.append('py::arg("parent") = py::none()')
                continue
            if param.c_type == "void":
                continue

            arg_name = param.name
            cpp_type = param.cpp_type
            if cpp_type is None:
                return  # Can't generate this factory

            # For wrapper types, need .get() when calling C function
            if cpp_type.endswith("Wrapper &"):
                call_args.append("%s.get()" % arg_name)
            else:
                call_args.append(arg_name)
            lambda_params.append("%s %s" % (cpp_type, arg_name))
            py_args.append('py::arg("%s")' % arg_name)

        if has_parent:
            parent_arg = "parent ? parent->get() : lv_screen_active()"
            all_call_args = ", ".join([parent_arg] + call_args)
        else:
            all_call_args = ", ".join(call_args)
        lambda_params_str = ", ".join(lambda_params)
        py_args_str = ", ".join(py_args)

        self.emit('    m.def("%s", [](%s) -> LvObjWrapper* {' % (py_name, lambda_params_str))
        self.emit("        return new LvObjWrapper(%s(%s));" % (ctor_func.name, all_call_args))
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
                self._emit_method(obj_cls_var="obj_cls", func=func, is_obj_method=True)

        self.emit("")

    # -----------------------------------------------------------------------
    # Method generation
    # -----------------------------------------------------------------------

    def _emit_method(self, obj_cls_var: str, func: FunctionIR, is_obj_method: bool = False):
        """Generate a single method binding."""
        if func.skip or func.name in self.generated_funcs:
            return

        method_name = self._sanitize(func.python_name)
        if method_name == "delete":
            method_name = "delete_obj"

        # Handle method name collisions between widgets
        if method_name in self.bound_method_names:
            first_widget = self.bound_method_names[method_name]
            if first_widget != (func.widget_name or "global"):
                method_name = "%s_%s" % (func.widget_name, method_name)
                if method_name in self.bound_method_names:
                    return  # Still colliding, skip
        self.bound_method_names[method_name] = func.widget_name or "global"

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
            if cpp_type.endswith("Wrapper &"):
                call_args.append("%s.get()" % arg_name)
            else:
                call_args.append(arg_name)
            lambda_params.append("%s %s" % (cpp_type, arg_name))
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
            self.emit('    %s.def("%s", [](%s) -> LvObjWrapper* { return new LvObjWrapper(%s(%s)); }' % (
                obj_cls_var, method_name, lambda_params_str, func.name, call_args_str))
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
        return name
