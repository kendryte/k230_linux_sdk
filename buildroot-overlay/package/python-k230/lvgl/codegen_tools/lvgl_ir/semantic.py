"""
Semantic Classifier and Property Mapper for LVGL API IR.

This module classifies LVGL C functions into semantic categories
(setter, getter, factory, callback, etc.) and maps setter/getter
pairs to Python properties.
"""

from __future__ import annotations
import re
from typing import Dict, List, Optional, Tuple
from .schema import FunctionIR, FuncCategory, ParamIR, WidgetIR


# ---------------------------------------------------------------------------
# Type classification helpers
# ---------------------------------------------------------------------------

# LVGL object pointer types (first param that makes a function a "method")
OBJ_PTR_PATTERN = re.compile(r'^lv_obj_t\s*\*$')

# Known widget names extracted from LVGL function naming conventions
WIDGET_NAMES = [
    "obj", "button", "label", "image", "slider", "bar", "switch",
    "textarea", "canvas", "checkbox", "dropdown", "roller", "arc",
    "span", "spinbox", "spinner", "table", "tabview", "tileview",
    "win", "msgbox", "list", "chart", "led", "line", "scale",
    "animimg", "keyboard", "meter",
    # Sub-objects
    "display", "screen", "group", "indev", "timer",
    # Style/font/draw subsystems
    "style", "font", "draw", "theme", "color",
    # Container types
    "menu", "panel",
]

# Map widget name to its create function
WIDGET_CREATE_MAP = {name: f"lv_{name}_create" for name in WIDGET_NAMES}

# C type to Python wrapper type mapping
WRAPPER_TYPE_MAP = {
    "lv_obj_t*": "LvObjWrapper",
    "lv_obj_t *": "LvObjWrapper",
    "lv_display_t*": "LvDisplayWrapper",
    "lv_display_t *": "LvDisplayWrapper",
    "lv_group_t*": "LvGroupWrapper",
    "lv_group_t *": "LvGroupWrapper",
    "lv_indev_t*": "LvIndevWrapper",
    "lv_indev_t *": "LvIndevWrapper",
    "lv_timer_t*": "LvTimerWrapper",
    "lv_timer_t *": "LvTimerWrapper",
    "lv_style_t*": "LvStyleWrapper",
    "lv_style_t *": "LvStyleWrapper",
    "lv_font_t*": "LvFontWrapper",
    "lv_font_t *": "LvFontWrapper",
    "lv_anim_t*": "LvAnimWrapper",
    "lv_anim_t *": "LvAnimWrapper",
    "lv_img_dsc_t*": "LvImgDscWrapper",
    "lv_img_dsc_t *": "LvImgDscWrapper",
}

# C types that are supported as return types for pybind11
SUPPORTED_RETURN_TYPES = {
    "void", "int", "int32_t", "int16_t", "int8_t",
    "uint32_t", "uint16_t", "uint8_t",
    "bool", "char",
    "float", "double",
    "lv_coord_t", "lv_opa_t",
    "lv_align_t", "lv_dir_t", "lv_base_dir_t",
    "lv_text_align_t", "lv_text_decor_t",
    "lv_border_side_t", "lv_grad_dir_t",
    "lv_scrollbar_mode_t", "lv_scroll_snap_t",
    "lv_part_t", "lv_state_t",
    "lv_obj_flag_t", "lv_obj_class_t *",
    "lv_event_code_t",
    "lv_color_t", "lv_point_t", "lv_area_t",
    "lv_font_fmt_txt_cmap_t",
    "const char *", "const char*",
}

# C types that are supported as parameter types for pybind11
SUPPORTED_PARAM_TYPES = SUPPORTED_RETURN_TYPES | {
    "va_list",
    # Object pointer types are handled via wrappers
    "lv_obj_t*", "lv_obj_t *",
    "lv_display_t*", "lv_display_t *",
    "lv_group_t*", "lv_group_t *",
    "lv_indev_t*", "lv_indev_t *",
    "lv_timer_t*", "lv_timer_t *",
    "lv_style_t*", "lv_style_t *",
    "lv_font_t*", "lv_font_t *",
    "lv_anim_t*", "lv_anim_t *",
    "lv_img_dsc_t*", "lv_img_dsc_t *",
    "lv_event_t*", "lv_event_t *",
}


# ---------------------------------------------------------------------------
# SemanticClassifier
# ---------------------------------------------------------------------------

class SemanticClassifier:
    """Classifies LVGL C functions into semantic categories."""

    # Patterns for classification
    _PATTERNS = [
        # Constructor: lv_xxx_create
        (re.compile(r'^lv_(\w+)_create$'), FuncCategory.CONSTRUCTOR),
        # Event callback registration: lv_xxx_add_event_cb
        (re.compile(r'_add_event_cb$'), FuncCategory.EVENT_CALLBACK),
        # Setter: lv_xxx_set_yyy
        (re.compile(r'_set_'), FuncCategory.SETTER),
        # Getter: lv_xxx_get_yyy
        (re.compile(r'_get_'), FuncCategory.GETTER),
        # Callback type: lv_xxx_cb_t (shouldn't be a function, but just in case)
        (re.compile(r'_cb_t$'), FuncCategory.CALLBACK_TYPE),
        # Internal/private: lv_xxx_internal_yyy
        (re.compile(r'_internal_'), FuncCategory.INTERNAL),
        # Draw subsystem internals
        (re.compile(r'^lv_draw_'), FuncCategory.UTILITY),
    ]

    def classify(self, func_name: str, first_param_type: Optional[str] = None) -> FuncCategory:
        """Classify a function by its name and first parameter type.

        Args:
            func_name: C function name (e.g., "lv_obj_set_x")
            first_param_type: Type string of the first parameter (e.g., "lv_obj_t *")

        Returns:
            The semantic category of the function.
        """
        # Check name patterns first (most specific)
        for pattern, category in self._PATTERNS:
            if pattern.search(func_name):
                return category

        # If first param is an object pointer, it's a method
        if first_param_type and self._is_obj_ptr(first_param_type):
            return FuncCategory.METHOD

        # Module-level function
        return FuncCategory.UTILITY

    def extract_widget_name(self, func_name: str) -> Optional[str]:
        """Extract the widget name from a function name.

        Examples:
            lv_obj_set_x → "obj"
            lv_button_create → "button"
            lv_label_set_text → "label"
        """
        # Match lv_WIDGETNAME_xxx patterns
        for widget in sorted(WIDGET_NAMES, key=len, reverse=True):
            if func_name.startswith(f"lv_{widget}_"):
                return widget
        # Generic obj fallback
        if func_name.startswith("lv_obj_"):
            return "obj"
        return None

    def extract_python_name(self, func_name: str, widget_name: Optional[str] = None) -> str:
        """Convert C function name to Python method name.

        Examples:
            lv_obj_set_x → "set_x"
            lv_button_create → "create" (on Button class)
            lv_label_set_text → "set_text" (on Label class)
        """
        if widget_name:
            prefix = f"lv_{widget_name}_"
            if func_name.startswith(prefix):
                return func_name[len(prefix):]

        # Fallback: strip lv_ prefix
        if func_name.startswith("lv_"):
            return func_name[3:]
        return func_name

    @staticmethod
    def _is_obj_ptr(type_str: str) -> bool:
        """Check if a type string is an lv_obj_t* pointer (the base widget type).

        Only lv_obj_t* is the 'self' parameter for widget methods.
        Other types like lv_display_t*, lv_anim_t*, lv_group_t* etc.
        are NOT self parameters — they are separate subsystem objects.
        """
        type_str = type_str.strip()
        # Match lv_obj_t * or const lv_obj_t * only
        return bool(re.match(r'^(const\s+)?lv_obj_t\s*\*$', type_str))


# ---------------------------------------------------------------------------
# PropertyMapper
# ---------------------------------------------------------------------------

class PropertyMapper:
    """Maps setter/getter function pairs to Python @property definitions.

    For example:
        lv_obj_set_x + lv_obj_get_x → property "x"
        lv_obj_set_size → method set_size (NOT a property, because 2 params)
    """

    def __init__(self, classifier: Optional[SemanticClassifier] = None):
        self.classifier = classifier or SemanticClassifier()

    def build_property_map(self, functions: List[FunctionIR]) -> Dict[str, Dict]:
        """Build a mapping of property names to their getter/setter functions.

        Returns:
            Dict mapping widget_name → {property_name → {"getter": func, "setter": func}}
        """
        # Group setter/getter by widget
        setters: Dict[str, Dict[str, FunctionIR]] = {}  # widget → prop_name → setter
        getters: Dict[str, Dict[str, FunctionIR]] = {}  # widget → prop_name → getter

        for func in functions:
            if func.skip:
                continue
            if func.category == FuncCategory.SETTER:
                prop_name = self._extract_property_name(func.name)
                widget = func.widget_name or "global"
                setters.setdefault(widget, {})[prop_name] = func
            elif func.category == FuncCategory.GETTER:
                prop_name = self._extract_property_name(func.name)
                widget = func.widget_name or "global"
                getters.setdefault(widget, {})[prop_name] = func

        # Match setter/getter pairs
        property_map: Dict[str, Dict] = {}
        all_widgets = set(setters.keys()) | set(getters.keys())

        for widget in all_widgets:
            widget_setters = setters.get(widget, {})
            widget_getters = getters.get(widget, {})
            widget_props: Dict[str, Dict] = {}

            # Properties with both getter and setter
            for prop_name in set(widget_setters.keys()) & set(widget_getters.keys()):
                setter = widget_setters[prop_name]
                getter = widget_getters[prop_name]

                # Only create property if setter has exactly 2 params (self + value)
                # Multi-param setters stay as methods (e.g., set_size(w, h))
                setter_value_params = [p for p in setter.params if not p.is_self]
                if len(setter_value_params) == 1:
                    widget_props[prop_name] = {
                        "getter": getter,
                        "setter": setter,
                        "type": setter_value_params[0].c_type,
                    }
                    # Update function IRs with property mapping
                    setter.property_mapping = {"property": prop_name, "access": "write"}
                    getter.property_mapping = {"property": prop_name, "access": "read"}

            # Properties with only getter (read-only)
            for prop_name in set(widget_getters.keys()) - set(widget_setters.keys()):
                getter = widget_getters[prop_name]
                widget_props[prop_name] = {
                    "getter": getter,
                    "setter": None,
                    "type": "read_only",
                }
                getter.property_mapping = {"property": prop_name, "access": "read"}

            if widget_props:
                property_map[widget] = widget_props

        return property_map

    def build_widgets(self, functions: List[FunctionIR],
                      property_map: Dict[str, Dict]) -> Dict[str, WidgetIR]:
        """Build WidgetIR objects from classified functions and property map.

        Returns:
            Dict mapping widget_name → WidgetIR
        """
        widgets: Dict[str, WidgetIR] = {}

        for func in functions:
            if func.skip or not func.widget_name:
                continue

            widget_name = func.widget_name
            if widget_name not in widgets:
                widgets[widget_name] = WidgetIR(
                    name=widget_name,
                    parent="obj" if widget_name != "obj" else None,
                    create_func=WIDGET_CREATE_MAP.get(widget_name),
                )

            widget = widgets[widget_name]
            widget.methods.append(func)

        # Attach properties
        for widget_name, props in property_map.items():
            if widget_name in widgets:
                widgets[widget_name].properties = props

        return widgets

    @staticmethod
    def _extract_property_name(func_name: str) -> str:
        """Extract the property name from a setter/getter function name.

        Examples:
            lv_obj_set_x → "x"
            lv_obj_get_width → "width"
            lv_label_set_text → "text"
            lv_obj_set_scrollbar_mode → "scrollbar_mode"
        """
        # Find _set_ or _get_ and take the suffix
        for sep in ("_set_", "_get_"):
            idx = func_name.find(sep)
            if idx >= 0:
                prop = func_name[idx + len(sep):]
                return prop
        return func_name
