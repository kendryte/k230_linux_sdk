"""
libclang → IR Converter

Parses LVGL header files using libclang and produces a ModuleIR.
Uses compile_commands.json (from CMake) to get correct compilation flags.
"""

from __future__ import annotations
import json
import os
import sys
from typing import Dict, List, Optional, Set, Tuple

try:
    import clang.cindex
    from clang.cindex import (
        Index, TranslationUnit, Cursor, CursorKind,
        Type, TypeKind, SourceLocation, Diagnostic,
    )
    HAS_CLANG = True
except ImportError:
    HAS_CLANG = False

from .schema import (
    ParamIR, FunctionIR, EnumMemberIR, EnumIR,
    StructFieldIR, StructIR, CallbackIR, WidgetIR,
    ModuleIR, FuncCategory, ParamDirection,
)
from .semantic import SemanticClassifier, PropertyMapper
from .rules import (
    SKIP_FUNCTIONS, FORCE_MODULE_LEVEL, WIDGET_NAME_OVERRIDES,
    TYPE_ALIASES, BASIC_CPP_TYPES, BASIC_PTR_TYPES,
    OPAQUE_STRUCT_TYPES, WRAPPED_STRUCT_TYPES,
)


# ---------------------------------------------------------------------------
# Type resolution helpers
# ---------------------------------------------------------------------------

def _is_opaque_param(c_type: str) -> bool:
    """Check if a parameter type involves an opaque/incomplete struct pointer.

    Handles both 'lv_foo_t *' and 'const lv_foo_t *' forms.
    """
    base = c_type.strip().lstrip("const ").rstrip(" *")
    return base in OPAQUE_STRUCT_TYPES

def resolve_type(c_type: str) -> str:
    """Resolve typedef chains to a canonical type."""
    seen = set()
    current = c_type.strip()
    while current in TYPE_ALIASES and current not in seen:
        seen.add(current)
        current = TYPE_ALIASES[current]
    # C99 _Bool → C++ bool
    if current == "_Bool":
        current = "bool"
    return current


def normalize_type_str(clang_type: Type) -> str:
    """Convert a libclang Type to a normalized C type string."""
    # Handle pointer types
    if clang_type.kind == TypeKind.POINTER:
        pointee = clang_type.get_pointee()
        base = normalize_type_str(pointee)
        # const qualifier
        is_const = clang_type.is_const_qualified()
        prefix = "const " if is_const else ""
        return f"{prefix}{base} *"

    # Handle array types
    if clang_type.kind == TypeKind.CONSTANTARRAY:
        elem = clang_type.get_array_element_type()
        base = normalize_type_str(elem)
        size = clang_type.get_array_size()
        return f"{base}[{size}]"

    # Handle typedef
    if clang_type.kind == TypeKind.TYPEDEF:
        # Use the typedef name directly (e.g., "lv_coord_t")
        return clang_type.get_typedef_decl().spelling if clang_type.get_typedef_decl() else clang_type.spelling

    # Handle enum
    if clang_type.kind == TypeKind.ENUM:
        decl = clang_type.get_declaration()
        return decl.spelling if decl.spelling else clang_type.spelling

    # Handle record (struct/union)
    if clang_type.kind == TypeKind.RECORD:
        decl = clang_type.get_declaration()
        return decl.spelling if decl.spelling else clang_type.spelling

    # Handle function proto (for callback params)
    if clang_type.kind == TypeKind.FUNCTIONPROTO:
        return "func_ptr"

    # Default: use spelling
    return clang_type.spelling


def get_cpp_type(c_type: str) -> Optional[str]:
    """Determine the pybind11-compatible C++ type for a given C type.

    Returns None if the type cannot be used in pybind11.
    """
    c_type = c_type.strip()

    # Resolve typedefs first
    resolved = resolve_type(c_type)
    if resolved in BASIC_CPP_TYPES:
        return resolved

    # Void
    if c_type == "void":
        return "void"

    # Basic types
    if c_type in BASIC_CPP_TYPES:
        return c_type

    # String types
    if c_type in ("const char *", "char *", "const char*"):
        return "const char *"

    # Void pointer - preserve const qualifier
    if c_type == "void *":
        return "void *"
    if c_type == "const void *":
        return "const void *"

    # Basic pointer types
    if c_type in BASIC_PTR_TYPES:
        return c_type

    # LVGL enum types (pass by value) - check if it's a known LVGL enum typedef
    if c_type.startswith("lv_") and c_type.endswith("_t") and "*" not in c_type:
        # This is likely an LVGL enum or small struct typedef
        # If it resolves to a basic type, use that
        if resolved in BASIC_CPP_TYPES:
            return resolved
        # Otherwise, try to pass it by value (pybind11 can handle small enums)
        return c_type

    # LVGL struct types pass-by-value (small structs like lv_point_t, lv_area_t, lv_color32_t)
    PASS_BY_VALUE_STRUCTS = {
        "lv_point_t", "lv_fpoint_t", "lv_area_t",
        "lv_color32_t", "lv_color_t",
        "lv_font_glyph_dsc_t",
    }
    if c_type in PASS_BY_VALUE_STRUCTS:
        return c_type

    # LVGL enum/struct pointer types
    if c_type.endswith(" *") or c_type.endswith("*"):
        base = c_type.rstrip(" *").replace("const ", "").replace("const", "").strip()
        is_const = c_type.strip().startswith("const ")

        # Pointer to wrapped struct type (for parameters: use wrapper reference)
        for struct_type, wrapper in WRAPPED_STRUCT_TYPES.items():
            if base == struct_type:
                return f"{wrapper} &"

        # Pointer to pass-by-value struct (e.g., lv_point_t *, const lv_area_t *)
        if base in PASS_BY_VALUE_STRUCTS:
            return f"{'const ' if is_const else ''}{base} *"

        # Pointer to basic type (e.g., int32_t *, char *)
        if base in BASIC_CPP_TYPES or resolved.rstrip(" *") in BASIC_CPP_TYPES:
            return f"{'const ' if is_const else ''}{base} *"

        # Pointer to LVGL enum type
        if base.startswith("lv_") and base.endswith("_t"):
            # Could be an enum pointer or opaque struct pointer
            if base in OPAQUE_STRUCT_TYPES:
                return None  # Opaque, no wrapper yet
            # Try as enum pointer - preserve const
            return f"{'const ' if is_const else ''}{base} *"

    # Enum pointer types (e.g., lv_align_t *)
    resolved_base = resolved.rstrip(" *")
    if c_type.endswith(" *") and resolved_base in BASIC_CPP_TYPES:
        return f"{resolved_base} *"

    # Double pointer (e.g., char **) → skip
    if c_type.endswith("**") or c_type.endswith(" **"):
        return None

    # Array type (e.g., char[]) → skip
    if c_type.endswith("[]"):
        return None

    # LVGL enum/typedef types that resolve to basic types
    if resolved in BASIC_CPP_TYPES:
        return resolved

    # Unknown type
    return None


# ---------------------------------------------------------------------------
# CompileCommands loader
# ---------------------------------------------------------------------------

def load_compile_commands(path: str) -> Dict[str, List[str]]:
    """Load compileCommands.json and return a dict mapping file → args."""
    with open(path) as f:
        data = json.load(f)

    result: Dict[str, List[str]] = {}
    for entry in data:
        filepath = entry.get("file", "")
        command = entry.get("command", "")
        if not command:
            continue
        import shlex
        tokens = shlex.split(command)
        # Extract -I, -D, -std flags
        args = []
        i = 1  # skip compiler name
        while i < len(tokens):
            tok = tokens[i]
            if tok.startswith("-I"):
                if len(tok) > 2:
                    args.append(tok)
                elif i + 1 < len(tokens):
                    args.append(tok)
                    args.append(tokens[i + 1])
                    i += 1
            elif tok == "-isystem" and i + 1 < len(tokens):
                args.append("-I" + tokens[i + 1])
                i += 1
            elif tok.startswith("-D"):
                args.append(tok)
            elif tok.startswith("--sysroot"):
                args.append(tok)
            elif tok.startswith("-std="):
                args.append(tok)
            elif tok.startswith("-f"):
                # Skip -fPIC etc (not relevant for parsing)
                pass
            i += 1
        result[filepath] = args
    return result


def find_best_compile_args(compile_commands: Dict[str, List[str]],
                           target_header: str) -> List[str]:
    """Find the best compilation arguments for a target header."""
    # Try exact match first
    if target_header in compile_commands:
        return compile_commands[target_header]

    # Try matching by basename
    target_base = os.path.basename(target_header)
    for filepath, args in compile_commands.items():
        if os.path.basename(filepath) == target_base:
            return args

    # Use the first LVGL .c file's args as a proxy
    for filepath, args in sorted(compile_commands.items()):
        if "lvgl" in filepath.lower() and filepath.endswith(".c"):
            return args

    return ["-std=gnu99"]


# ---------------------------------------------------------------------------
# ClangToIR Converter
# ---------------------------------------------------------------------------

class ClangToIR:
    """Converts libclang AST to ModuleIR."""

    def __init__(self, module_name: str = "lvgl", module_prefix: str = "lv"):
        self.module_name = module_name
        self.module_prefix = module_prefix
        self.classifier = SemanticClassifier()
        self.property_mapper = PropertyMapper(self.classifier)

        # Accumulated IR data
        self.functions: List[FunctionIR] = []
        self.enums: List[EnumIR] = []
        self.structs: List[StructIR] = []
        self.callbacks: List[CallbackIR] = []

        # Tracking
        self._seen_funcs: Set[str] = set()
        self._seen_enums: Set[str] = set()
        self._seen_structs: Set[str] = set()
        self._seen_typedefs: Set[str] = set()

    def parse(self, header_path: str,
              compile_commands_path: Optional[str] = None,
              extra_args: Optional[List[str]] = None) -> ModuleIR:
        """Parse LVGL headers and produce ModuleIR.

        Args:
            header_path: Path to lvgl.h or specific header file.
            compile_commands_path: Path to compile_commands.json from CMake build.
            extra_args: Additional clang arguments (e.g., -I paths, -D macros).
        """
        if not HAS_CLANG:
            raise ImportError(
                "libclang Python binding not available. "
                "Install with: pip install libclang"
            )

        # Get compilation args
        args: List[str] = []
        if compile_commands_path and os.path.exists(compile_commands_path):
            cc = load_compile_commands(compile_commands_path)
            args = find_best_compile_args(cc, header_path)

        if extra_args:
            args.extend(extra_args)

        # Detect sysroot from args (set by compile_commands.json)
        sysroot = None
        for a in args:
            if a.startswith("--sysroot="):
                sysroot = a[len("--sysroot="):]
                break

        # Set target triple based on sysroot
        if sysroot and "riscv64" in sysroot:
            args.insert(0, "-target")
            args.insert(1, "riscv64-unknown-linux-gnu")
            # Add GCC builtin headers for riscv64 target (stddef.h etc.)
            _gcc_builtin = "/opt/toolchain/Xuantie-900-gcc-linux-6.6.0-glibc-x86_64-V3.0.2/lib/gcc/riscv64-unknown-linux-gnu/14.1.1/include"
            if os.path.isdir(_gcc_builtin):
                args.append("-I" + _gcc_builtin)

        # Ensure system includes are present
        if not any("-I/usr/include" in a for a in args):
            if sysroot:
                # Use sysroot's system headers instead of host headers
                args.extend([
                    "-I" + sysroot + "/usr/include",
                ])
            else:
                args.extend([
                    "-I/usr/lib/gcc/x86_64-linux-gnu/9/include",
                    "-I/usr/local/include",
                    "-I/usr/include/x86_64-linux-gnu",
                    "-I/usr/include",
                ])

        # Parse
        idx = Index.create()
        tu = idx.parse(
            header_path,
            args=args,
            options=(
                clang.cindex.TranslationUnit.PARSE_DETAILED_PROCESSING_RECORD
                | clang.cindex.TranslationUnit.PARSE_SKIP_FUNCTION_BODIES
            ),
        )

        # Report diagnostics
        errors = 0
        warnings = 0
        for d in tu.diagnostics:
            if d.severity >= clang.cindex.Diagnostic.Error:
                errors += 1
                eprint(f"ERROR: {d.spelling} at {d.location}")
            elif d.severity >= clang.cindex.Diagnostic.Warning:
                warnings += 1
        if errors > 0:
            eprint(f"libclang: {errors} errors, {warnings} warnings (proceeding anyway)")

        # Walk AST and build IR
        self._walk_cursor(tu.cursor, header_path)

        # Classify and organize
        self._classify_all()

        # Build property maps and widgets
        property_map = self.property_mapper.build_property_map(self.functions)
        widgets = self.property_mapper.build_widgets(self.functions, property_map)

        # Build ModuleIR
        module_ir = ModuleIR(
            functions=self.functions,
            enums=self.enums,
            structs=self.structs,
            callbacks=self.callbacks,
            widgets=widgets,
            module_name=self.module_name,
            module_prefix=self.module_prefix,
        )
        module_ir.compute_stats()
        return module_ir

    # -----------------------------------------------------------------------
    # AST Walking
    # -----------------------------------------------------------------------

    def _walk_cursor(self, cursor: Cursor, root_file: str):
        """Walk the AST and extract relevant declarations."""
        for child in cursor.get_children():
            # Only process declarations from LVGL headers
            if child.location and child.location.file:
                fname = str(child.location.file)
                if "lvgl" not in fname.lower() and "lv_" not in fname.lower():
                    continue
            else:
                continue

            kind = child.kind

            if kind == CursorKind.FUNCTION_DECL:
                self._process_function(child)
            elif kind == CursorKind.ENUM_DECL:
                self._process_enum(child)
            elif kind == CursorKind.STRUCT_DECL:
                self._process_struct(child)
            elif kind == CursorKind.UNION_DECL:
                self._process_struct(child)
            elif kind == CursorKind.TYPEDEF_DECL:
                self._process_typedef(child)

    def _process_function(self, cursor: Cursor):
        """Extract function information from a FUNCTION_DECL cursor."""
        name = cursor.spelling
        if not name or not name.startswith("lv_"):
            return
        if name in self._seen_funcs:
            return
        if name.startswith("_"):
            return  # Skip internal functions

        self._seen_funcs.add(name)

        # Get parameter info
        params = []
        for i, arg in enumerate(cursor.get_arguments()):
            arg_name = arg.spelling or f"arg{i}"
            arg_type_str = normalize_type_str(arg.type)

            # Determine if this is the "self" parameter (first obj ptr)
            # Both lv_obj_t * and const lv_obj_t * are valid self parameters
            is_self = (i == 0 and self.classifier._is_obj_ptr(arg_type_str))

            # Determine wrapper type (check both const and non-const)
            wrapper_type = None
            for struct_type, wrapper in WRAPPED_STRUCT_TYPES.items():
                for pattern in [f"{struct_type} *", f"{struct_type}*",
                                f"const {struct_type} *", f"const {struct_type}*"]:
                    if arg_type_str == pattern:
                        wrapper_type = wrapper
                        break
                if wrapper_type:
                    break

            cpp_type = get_cpp_type(arg_type_str)
            if cpp_type == "LvObjWrapper &" and is_self:
                cpp_type = "LvObjWrapper &"  # self is handled specially

            param = ParamIR(
                name=arg_name,
                c_type=arg_type_str,
                cpp_type=cpp_type,
                is_self=is_self,
                wrapper_type=wrapper_type,
            )
            params.append(param)

        # Get return type
        ret_type_str = normalize_type_str(cursor.result_type)
        ret_cpp = get_cpp_type(ret_type_str)

        # Check if variadic (libclang Python binding doesn't have is_variadic on Type,
        # so we check by looking at the last argument type)
        is_variadic = False
        try:
            func_type = cursor.type
            if func_type.kind == TypeKind.FUNCTIONPROTO:
                # Check if the function has an ellipsis parameter
                num_args = func_type.get_num_arg_types()
                # Check for ellipsis by seeing if get_arguments() returns a
                # cursor with kind EllipsisParam (unusual but possible)
                args_list = list(cursor.get_arguments())
                for a in args_list:
                    if a.kind == CursorKind.UNEXPOSED_EXPR and a.spelling == "...":
                        is_variadic = True
                        break
                # Alternative: check the function spelling for "..." pattern
                # This is a heuristic but works for LVGL
                if not is_variadic and num_args > 0:
                    last_arg_type = func_type.get_arg_type(num_args - 1)
                    if last_arg_type.spelling == "...":
                        is_variadic = True
        except Exception:
            pass

        # Determine if should skip
        skip = False
        skip_reason = None

        if name in SKIP_FUNCTIONS:
            skip = True
            skip_reason = "manual skip rule"
        elif is_variadic and "fmt" not in name:
            skip = True
            skip_reason = "variadic (non-fmt)"
        elif any(p.cpp_type is None and not p.is_self for p in params):
            skip = True
            unsupported = [p for p in params if p.cpp_type is None and not p.is_self]
            skip_reason = f"unsupported param type: {unsupported[0].c_type}"
        elif ret_cpp is None and ret_type_str != "void":
            skip = True
            skip_reason = f"unsupported return type: {ret_type_str}"
        elif any(p.wrapper_type and p.wrapper_type != "LvObjWrapper" and not p.is_self for p in params):
            # Skip functions with non-obj wrapper params (display, indev, etc.)
            # These need separate wrapper class methods, not obj methods
            skip = True
            non_obj = [p for p in params if p.wrapper_type and p.wrapper_type != "LvObjWrapper" and not p.is_self]
            skip_reason = f"non-obj wrapper param: {non_obj[0].c_type}"
        elif any(_is_opaque_param(p.c_type) for p in params):
            skip = True
            opaque = [p for p in params if _is_opaque_param(p.c_type)]
            skip_reason = f"incomplete struct param: {opaque[0].c_type}"

        func_ir = FunctionIR(
            name=name,
            python_name="",  # Will be filled by _classify_all
            category=FuncCategory.UTILITY,  # Will be filled by _classify_all
            target_type=None,
            widget_name=None,
            params=params,
            return_type=ret_type_str,
            return_cpp_type=ret_cpp,
            is_variadic=is_variadic,
            skip=skip,
            skip_reason=skip_reason,
            source_file=str(cursor.location.file) if cursor.location.file else None,
            source_line=cursor.location.line if cursor.location else None,
        )
        self.functions.append(func_ir)

    def _process_enum(self, cursor: Cursor):
        """Extract enum information from an ENUM_DECL cursor."""
        name = cursor.spelling
        if not name:
            return
        if name in self._seen_enums:
            return

        self._seen_enums.add(name)

        # Get enum members
        members = []
        for child in cursor.get_children():
            if child.kind == CursorKind.ENUM_CONSTANT_DECL:
                members.append(EnumMemberIR(
                    c_name=child.spelling,
                    python_name="",  # Will be derived later
                    value=child.enum_value,
                ))

        # Determine C typedef name
        c_type = name
        typedef_cursor = cursor.semantic_parent
        if typedef_cursor and typedef_cursor.kind == CursorKind.TYPEDEF_DECL:
            c_type = typedef_cursor.spelling

        enum_ir = EnumIR(
            c_type=c_type,
            python_name="",  # Will be derived later
            members=members,
            is_anonymous=(not name.startswith("lv_")),
            source_file=str(cursor.location.file) if cursor.location.file else None,
        )
        self.enums.append(enum_ir)

    def _process_struct(self, cursor: Cursor):
        """Extract struct information from a STRUCT_DECL cursor."""
        name = cursor.spelling
        if not name:
            return
        if name in self._seen_structs:
            return

        self._seen_structs.add(name)

        fields = []
        has_bitfield = False
        has_anonymous = False

        for child in cursor.get_children():
            if child.kind == CursorKind.FIELD_DECL:
                field_name = child.spelling or ""
                field_type = normalize_type_str(child.type)
                is_bitfield = child.is_bitfield()
                bit_width = child.get_bitfield_width() if is_bitfield else None

                if is_bitfield:
                    has_bitfield = True
                if not field_name:
                    has_anonymous = True

                fields.append(StructFieldIR(
                    name=field_name,
                    c_type=field_type,
                    is_bitfield=is_bitfield,
                    bit_width=bit_width,
                ))

        is_opaque = len(fields) == 0
        is_union = (cursor.kind == CursorKind.UNION_DECL)

        struct_ir = StructIR(
            name=name,
            is_opaque=is_opaque,
            is_union=is_union,
            fields=fields,
            has_bitfield=has_bitfield,
            has_anonymous_member=has_anonymous,
            source_file=str(cursor.location.file) if cursor.location.file else None,
        )
        self.structs.append(struct_ir)

    def _process_typedef(self, cursor: Cursor):
        """Process typedef declarations, especially for callbacks and enums."""
        name = cursor.spelling
        if not name or name in self._seen_typedefs:
            return

        self._seen_typedefs.add(name)

        # Check for typedef of function pointer (callback type)
        underlying = cursor.underlying_typedef_type
        if underlying.kind == TypeKind.POINTER:
            pointee = underlying.get_pointee()
            if pointee.kind == TypeKind.FUNCTIONPROTO:
                # This is a callback typedef
                param_types = [normalize_type_str(arg.type) for arg in
                               cursor.get_children() if arg.kind == CursorKind.PARM_DECL]
                cb_ir = CallbackIR(
                    name=name,
                    param_types=param_types,
                    return_type=normalize_type_str(pointee.get_result()),
                    source_file=str(cursor.location.file) if cursor.location.file else None,
                )
                self.callbacks.append(cb_ir)

        # Check for typedef of enum
        if underlying.kind == TypeKind.ENUM:
            # The enum was already processed in _process_enum
            # But we can record the C type name here
            enum_decl = underlying.get_declaration()
            if enum_decl:
                # Update existing enum's c_type if needed
                for enum_ir in self.enums:
                    if enum_ir.c_type == enum_decl.spelling:
                        enum_ir.c_type = name
                        break

    # -----------------------------------------------------------------------
    # Post-processing
    # -----------------------------------------------------------------------

    def _classify_all(self):
        """Classify all functions and set python_name, widget_name, etc."""
        for func in self.functions:
            # Get first param type for classification
            first_type = func.params[0].c_type if func.params else None

            # Classify
            func.category = self.classifier.classify(func.name, first_type)

            # Override: some functions should be module-level despite looking like methods
            if func.name in FORCE_MODULE_LEVEL:
                func.is_static = True
                func.widget_name = None

            # Extract widget name
            func.widget_name = self.classifier.extract_widget_name(func.name)
            if func.name in WIDGET_NAME_OVERRIDES:
                func.widget_name = WIDGET_NAME_OVERRIDES[func.name]

            # Determine target_type
            if func.widget_name and first_type:
                func.target_type = first_type

            # Determine if static (module-level)
            if not func.is_static:
                if func.category in (FuncCategory.UTILITY, FuncCategory.FACTORY):
                    func.is_static = True
                elif func.category in (FuncCategory.SETTER, FuncCategory.GETTER, FuncCategory.METHOD):
                    func.is_static = not (first_type and self.classifier._is_obj_ptr(first_type))

            # Set python name
            func.python_name = self.classifier.extract_python_name(
                func.name, func.widget_name
            )

            # Mark constructors as static
            if func.category == FuncCategory.CONSTRUCTOR:
                func.is_static = True
                func.python_name = func.widget_name or "create"


def eprint(*args, **kwargs):
    """Print to stderr."""
    print(*args, file=sys.stderr, **kwargs)
