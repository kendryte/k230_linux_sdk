"""
LVGL Binding IR (Intermediate Representation) Layer

This module defines the data structures used to represent LVGL API
in a format suitable for code generation (pybind11, Python wrapper, etc.).

The IR decouples parsing (libclang/pycparser) from code generation,
enabling semantic transformations like property mapping.
"""

from .schema import (
    ParamIR,
    FunctionIR,
    EnumMemberIR,
    EnumIR,
    StructFieldIR,
    StructIR,
    CallbackIR,
    WidgetIR,
    ModuleIR,
)

from .semantic import SemanticClassifier, PropertyMapper

__all__ = [
    "ParamIR",
    "FunctionIR",
    "EnumMemberIR",
    "EnumIR",
    "StructFieldIR",
    "StructIR",
    "CallbackIR",
    "WidgetIR",
    "ModuleIR",
    "SemanticClassifier",
    "PropertyMapper",
]
