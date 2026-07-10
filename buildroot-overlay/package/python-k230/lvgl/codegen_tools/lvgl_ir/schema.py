"""
IR Schema - Data structures for LVGL API Intermediate Representation.

These dataclasses represent the LVGL API in a normalized form that is
independent of the parsing backend (libclang or pycparser) and suitable
for code generation.
"""

from __future__ import annotations
from dataclasses import dataclass, field
from typing import Optional, Dict, List, Any
from enum import Enum


# ---------------------------------------------------------------------------
# Enumerations for classification
# ---------------------------------------------------------------------------

class FuncCategory(Enum):
    """Classification of a C function's role."""
    CONSTRUCTOR = "constructor"      # lv_xxx_create
    SETTER = "setter"                # lv_xxx_set_yyy
    GETTER = "getter"                # lv_xxx_get_yyy
    METHOD = "method"                # lv_xxx_yyy (other method on object)
    FACTORY = "factory"              # module-level factory
    CALLBACK_TYPE = "callback_type"  # typedef for callback function pointer
    EVENT_CALLBACK = "event_callback"  # add_event_cb style
    UTILITY = "utility"              # module-level utility function
    INTERNAL = "internal"            # internal/private function (skip)
    UNSUPPORTED = "unsupported"      # cannot be bound (skip with reason)


class ParamDirection(Enum):
    """Parameter data flow direction."""
    IN = "in"
    OUT = "out"
    INOUT = "inout"


# ---------------------------------------------------------------------------
# Parameter IR
# ---------------------------------------------------------------------------

@dataclass
class ParamIR:
    """Represents a single function parameter."""
    name: str                          # C parameter name
    c_type: str                        # C type string (e.g., "lv_obj_t *")
    cpp_type: Optional[str] = None     # pybind11-compatible C++ type (None = unsupported)
    direction: ParamDirection = ParamDirection.IN
    default_value: Optional[str] = None
    is_self: bool = False              # True if this is the object's 'self' param (first obj ptr)
    is_output: bool = False            # True if this is an output parameter (pointer to result)

    # For obj pointer params, track the wrapper type
    wrapper_type: Optional[str] = None  # e.g., "LvObjWrapper", "LvDisplayWrapper"


# ---------------------------------------------------------------------------
# Function IR
# ---------------------------------------------------------------------------

@dataclass
class FunctionIR:
    """Represents a single C function in the IR."""
    # Identity
    name: str                          # C function name (e.g., "lv_obj_set_x")
    python_name: str                   # Python method name (e.g., "set_x")
    category: FuncCategory             # Classification

    # Type info
    target_type: Optional[str] = None  # Object type this method belongs to (e.g., "lv_obj_t*")
    widget_name: Optional[str] = None  # Widget name (e.g., "obj", "button", "label")
    params: List[ParamIR] = field(default_factory=list)
    return_type: str = "void"
    return_cpp_type: Optional[str] = None
    is_variadic: bool = False
    is_static: bool = False            # True if module-level (not a method on an object)
    is_inline: bool = False

    # Semantic mapping
    property_mapping: Optional[Dict[str, str]] = None
    # e.g., {"property": "x", "access": "write"} for setter
    # e.g., {"property": "x", "access": "read"} for getter

    # Skip tracking
    skip: bool = False
    skip_reason: Optional[str] = None  # Why this function was skipped

    # Source location
    source_file: Optional[str] = None
    source_line: Optional[int] = None


# ---------------------------------------------------------------------------
# Enum IR
# ---------------------------------------------------------------------------

@dataclass
class EnumMemberIR:
    """A single enum member."""
    c_name: str                        # C name (e.g., "LV_ALIGN_CENTER")
    python_name: str                   # Python name (e.g., "CENTER")
    value: Optional[int] = None       # Integer value if known


@dataclass
class EnumIR:
    """Represents a C enum."""
    c_type: str                        # C typedef name (e.g., "lv_align_t")
    python_name: str                   # Python enum name (e.g., "ALIGN")
    members: List[EnumMemberIR] = field(default_factory=list)
    is_anonymous: bool = False         # True if no C typedef found
    source_file: Optional[str] = None


# ---------------------------------------------------------------------------
# Struct IR
# ---------------------------------------------------------------------------

@dataclass
class StructFieldIR:
    """A single struct field."""
    name: str
    c_type: str
    is_bitfield: bool = False
    bit_width: Optional[int] = None


@dataclass
class StructIR:
    """Represents a C struct/union."""
    name: str                          # C struct name (e.g., "lv_obj_t")
    is_opaque: bool = False            # True if forward-declared only (no fields visible)
    is_union: bool = False
    fields: List[StructFieldIR] = field(default_factory=list)
    has_bitfield: bool = False
    has_anonymous_member: bool = False
    source_file: Optional[str] = None


# ---------------------------------------------------------------------------
# Callback IR
# ---------------------------------------------------------------------------

@dataclass
class CallbackIR:
    """Represents a callback function pointer typedef."""
    name: str                          # typedef name (e.g., "lv_event_cb_t")
    param_types: List[str] = field(default_factory=list)
    return_type: str = "void"
    source_file: Optional[str] = None


# ---------------------------------------------------------------------------
# Widget IR
# ---------------------------------------------------------------------------

@dataclass
class WidgetIR:
    """Represents an LVGL widget (object class)."""
    name: str                          # Widget name (e.g., "button")
    parent: Optional[str] = None       # Parent widget name (usually "obj")
    create_func: Optional[str] = None  # Constructor function name (e.g., "lv_button_create")
    methods: List[FunctionIR] = field(default_factory=list)
    properties: Dict[str, Dict] = field(default_factory=dict)
    # properties: {"x": {"getter": func_ir, "setter": func_ir}, ...}


# ---------------------------------------------------------------------------
# Module IR (top-level)
# ---------------------------------------------------------------------------

@dataclass
class ModuleIR:
    """Top-level IR containing all LVGL API information."""
    # All functions
    functions: List[FunctionIR] = field(default_factory=list)

    # Functions indexed by name for fast lookup
    _func_index: Dict[str, FunctionIR] = field(default_factory=dict, repr=False)

    # Enums
    enums: List[EnumIR] = field(default_factory=list)

    # Structs
    structs: List[StructIR] = field(default_factory=list)

    # Callback types
    callbacks: List[CallbackIR] = field(default_factory=list)

    # Widgets (organized by object type)
    widgets: Dict[str, WidgetIR] = field(default_factory=dict)

    # Module metadata
    module_name: str = "lvgl"
    module_prefix: str = "lv"
    lvgl_version: Optional[str] = None

    # Statistics
    total_c_functions: int = 0
    total_bound_functions: int = 0
    total_skipped_functions: int = 0

    def build_index(self):
        """Build function name index for fast lookup."""
        self._func_index = {f.name: f for f in self.functions}

    def get_function(self, name: str) -> Optional[FunctionIR]:
        """Look up a function by C name."""
        if not self._func_index:
            self.build_index()
        return self._func_index.get(name)

    def get_bound_functions(self) -> List[FunctionIR]:
        """Get all functions that will be bound (not skipped)."""
        return [f for f in self.functions if not f.skip]

    def get_skipped_functions(self) -> List[FunctionIR]:
        """Get all skipped functions with reasons."""
        return [f for f in self.functions if f.skip]

    def get_by_category(self, category: FuncCategory) -> List[FunctionIR]:
        """Get all functions of a given category."""
        return [f for f in self.functions if f.category == category]

    def get_widget_methods(self, widget_name: str) -> List[FunctionIR]:
        """Get all methods for a specific widget."""
        return [f for f in self.functions
                if f.widget_name == widget_name and not f.skip]

    def get_module_functions(self) -> List[FunctionIR]:
        """Get all module-level (non-method) functions."""
        return [f for f in self.functions
                if f.is_static and not f.skip]

    def compute_stats(self):
        """Compute statistics."""
        self.total_c_functions = len(self.functions)
        self.total_bound_functions = len(self.get_bound_functions())
        self.total_skipped_functions = len(self.get_skipped_functions())
