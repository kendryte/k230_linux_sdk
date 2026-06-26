#!/usr/bin/env python3
"""
gen_pybind11.py - Generate pybind11 bindings for LVGL from header files.

Based on gen_mpy.py from lv_binding_micropython, but outputs pybind11 C++ code
instead of MicroPython C API code.

Usage:
    python3 gen_pybind11.py \\
        -M lvgl -MP lv \\
        -I /path/to/lvgl/include \\
        -I /path/to/lv_conf.h/dir \\
        -I /path/to/pycparser/fake_libc_include \\
        -o lvgl_pybind_generated.cpp \\
        lvgl.h

Or with preprocessed file:
    python3 gen_pybind11.py \\
        -M lvgl -MP lv \\
        -E preprocessed.i \\
        -o lvgl_pybind_generated.cpp \\
        lvgl.h
"""

from __future__ import print_function
import collections
import sys
import copy
from functools import lru_cache
import os

def memoize(func):
    @lru_cache(maxsize=1000000)
    def memoized(*args, **kwargs):
        return func(*args, **kwargs)
    return memoized

def eprint(*args, **kwargs):
    print(*args, file=sys.stderr, **kwargs)

from sys import argv
from argparse import ArgumentParser
import subprocess
import re
from os.path import dirname, abspath
from os.path import commonprefix

from pycparser import c_parser, c_ast, c_generator

# Auto-detect fake_libc_include path from pycparser installation
def find_fake_libc_include():
    """Find fake_libc_include directory from installed pycparser."""
    try:
        pycparser_dir = os.path.dirname(pycparser.__file__ if hasattr(pycparser, '__file__') else '')
        candidate = os.path.join(pycparser_dir, 'utils', 'fake_libc_include')
        if os.path.isdir(candidate):
            return candidate
    except Exception:
        pass
    # Try common system paths
    for p in [
        os.path.expanduser('~/.local/lib/python3.8/site-packages/pycparser/utils/fake_libc_include'),
        os.path.expanduser('~/.local/lib/python3.13/site-packages/pycparser/utils/fake_libc_include'),
        '/usr/lib/python3/dist-packages/pycparser/utils/fake_libc_include',
        '/usr/local/lib/python3.8/dist-packages/pycparser/utils/fake_libc_include',
    ]:
        if os.path.isdir(p):
            return p
    return None

FAKE_LIBC_INCLUDE = find_fake_libc_include()

#
# Argument parsing
#

argParser = ArgumentParser(description='Generate pybind11 bindings for LVGL')
argParser.add_argument("-I", "--include", dest="include",
    help="Preprocessor include path", metavar="<Include Path>", action="append")
argParser.add_argument("-D", "--define", dest="define",
    help="Define preprocessor macro", metavar="<Macro Name>", action="append")
argParser.add_argument("-E", "--external-preprocessing", dest="ep",
    help="Prevent preprocessing. Assume input file is already preprocessed",
    metavar="<Preprocessed File>", action="store")
argParser.add_argument("-M", "--module_name", dest="module_name",
    help="Module name", metavar="<Module name string>", action="store")
argParser.add_argument("-MP", "--module_prefix", dest="module_prefix",
    help="Module prefix that starts every function name",
    metavar="<Prefix string>", action="store")
argParser.add_argument("-o", "--output", dest="output",
    help="Output file name", metavar="<Output File>", action="store")
argParser.add_argument("input", nargs="+")
argParser.set_defaults(include=[], define=[], ep=None, output=None, input=[])
args = argParser.parse_args()

module_name = args.module_name
module_prefix = args.module_prefix if args.module_prefix else args.module_name
output_file = args.output

#
# C preprocessing
#

if not args.ep:
    all_includes = list(args.include)
    if FAKE_LIBC_INCLUDE:
        all_includes.append(FAKE_LIBC_INCLUDE)
    pp_cmd = (
        "gcc -E -std=c99 -DPYCPARSER {macros} {include} {input} {first_input}".format(
            input=" ".join("-include %s" % inp for inp in args.input),
            first_input="%s" % args.input[0],
            macros=" ".join("-D%s" % define for define in args.define),
            include=" ".join("-I %s" % inc for inc in all_includes),
        )
    )
    s = subprocess.check_output(pp_cmd.split()).decode()
else:
    pp_cmd = "Preprocessing was disabled."
    s = ""
    with open(args.ep, "r") as f:
        s += f.read()

#
# AST parsing helper functions (from gen_mpy.py)
#

@memoize
def remove_declname(ast):
    if hasattr(ast, "declname"):
        ast.declname = None
    if isinstance(ast, tuple):
        remove_declname(ast[1])
        return
    for i, c1 in enumerate(ast.children()):
        child = ast.children()[i]
        remove_declname(child)

@memoize
def add_default_declname(ast, name):
    if hasattr(ast, "declname"):
        if ast.declname == None:
            ast.declname = name
    if isinstance(ast, tuple):
        add_default_declname(ast[1], name)
        return
    for i, c1 in enumerate(ast.children()):
        child = ast.children()[i]
        add_default_declname(child, name)

@memoize
def convert_array_to_ptr(ast):
    if hasattr(ast, "type") and isinstance(ast.type, c_ast.ArrayDecl):
        ast.type = c_ast.PtrDecl(
            ast.type.quals if hasattr(ast.type, "quals") else [], ast.type.type)
    if isinstance(ast, tuple):
        return convert_array_to_ptr(ast[1])
    for i, c1 in enumerate(ast.children()):
        child = ast.children()[i]
        convert_array_to_ptr(child)

@memoize
def remove_quals(ast):
    if hasattr(ast, "quals"):
        ast.quals = []
    if hasattr(ast, "dim_quals"):
        ast.dim_quals = []
    if isinstance(ast, tuple):
        return remove_quals(ast[1])
    for i, c1 in enumerate(ast.children()):
        child = ast.children()[i]
        if not isinstance(child, c_ast.FuncDecl):
            remove_quals(child)

@memoize
def remove_explicit_struct(ast):
    if isinstance(ast, c_ast.TypeDecl) and isinstance(ast.type, c_ast.Struct):
        explicit_struct_name = ast.type.name
        if explicit_struct_name:
            if explicit_struct_name in explicit_structs:
                ast.type = c_ast.IdentifierType([explicit_structs[explicit_struct_name]])
            elif explicit_struct_name in structs:
                ast.type = c_ast.IdentifierType([explicit_struct_name])
    if isinstance(ast, tuple):
        return remove_explicit_struct(ast[1])
    for i, c1 in enumerate(ast.children()):
        child = ast.children()[i]
        remove_explicit_struct(child)

@memoize
def get_type(arg, **kwargs):
    if isinstance(arg, str):
        return arg
    remove_quals_arg = "remove_quals" in kwargs and kwargs["remove_quals"]
    arg_ast = copy.deepcopy(arg)
    remove_explicit_struct(arg_ast)
    if remove_quals_arg:
        remove_quals(arg_ast)
    return gen.visit(arg_ast)

@memoize
def get_name(type):
    if isinstance(type, c_ast.Decl):
        return type.name
    if isinstance(type, c_ast.Struct) and type.name and type.name in explicit_structs:
        return explicit_structs[type.name]
    if isinstance(type, c_ast.Struct):
        return type.name
    if isinstance(type, c_ast.TypeDecl):
        return type.declname
    if isinstance(type, c_ast.IdentifierType):
        return type.names[0]
    if isinstance(type, c_ast.FuncDecl):
        return type.type.declname
    if isinstance(type, (c_ast.PtrDecl, c_ast.ArrayDecl)):
        return get_type(type, remove_quals=True)
    else:
        return gen.visit(type)

@memoize
def remove_arg_names(ast):
    if isinstance(ast, c_ast.TypeDecl):
        ast.declname = None
        remove_arg_names(ast.type)
    elif isinstance(ast, c_ast.Decl):
        remove_arg_names(ast.type)
    elif isinstance(ast, c_ast.FuncDecl):
        remove_arg_names(ast.args)
    elif isinstance(ast, c_ast.ParamList):
        for param in ast.params:
            remove_arg_names(param)

@memoize
def function_prototype(func):
    bare_func = copy.deepcopy(func)
    remove_declname(bare_func)
    ptr_decl = c_ast.PtrDecl(quals=[], type=bare_func.type)
    func_proto = c_ast.Typename(name=None, quals=[], align=[], type=ptr_decl)
    return func_proto

#
# Module specific text patterns
#

base_obj_name = "obj"
base_obj_type = "%s_%s_t" % (module_prefix, base_obj_name)
lv_ext_pattern = re.compile("^{prefix}_([^_]+)_ext_t".format(prefix=module_prefix))
lv_obj_pattern = re.compile("^{prefix}_([^_]+)".format(prefix=module_prefix), re.IGNORECASE)
lv_func_pattern = re.compile("^{prefix}_(.+)".format(prefix=module_prefix), re.IGNORECASE)
create_obj_pattern = re.compile("^{prefix}_(.+)_create$".format(prefix=module_prefix))
lv_method_pattern = re.compile("^{prefix}_[^_]+_(.+)".format(prefix=module_prefix), re.IGNORECASE)
lv_base_obj_pattern = re.compile(
    "^(struct _){{0,1}}{prefix}_{base_name}_t( *\\*?)$".format(
        prefix=module_prefix, base_name=base_obj_name))
lv_str_enum_pattern = re.compile("^_?{prefix}_STR_(.+)".format(prefix=module_prefix.upper()))
lv_callback_type_pattern = re.compile("({prefix}_){{0,1}}(.+)_cb(_t){{0,1}}".format(prefix=module_prefix))
lv_global_callback_pattern = re.compile(".*g_cb_t")
lv_func_returns_array = re.compile(".*_array$")
lv_enum_name_pattern = re.compile(
    "^(ENUM_){{0,1}}({prefix}_){{0,1}}(.*)".format(prefix=module_prefix.upper()))

def sanitize(id, kwlist=[
        "False", "None", "True", "and", "as", "assert", "break", "class",
        "continue", "def", "del", "elif", "else", "except", "finally",
        "for", "from", "global", "if", "import", "in", "is", "lambda",
        "nonlocal", "not", "or", "pass", "raise", "return", "try",
        "while", "with", "yield"]):
    if id in kwlist:
        result = "_%s" % id
    else:
        result = id
    result = result.strip()
    result = result.replace(" ", "_")
    result = result.replace("*", "_ptr")
    return result

@memoize
def simplify_identifier(id):
    match_result = lv_func_pattern.match(id)
    return match_result.group(1) if match_result else id

def obj_name_from_ext_name(ext_name):
    return lv_ext_pattern.match(ext_name).group(1)

def obj_name_from_func_name(func_name):
    return lv_obj_pattern.match(func_name).group(1)

def ctor_name_from_obj_name(obj_name):
    return "{prefix}_{obj}_create".format(prefix=module_prefix, obj=obj_name)

def is_method_of(func_name, obj_name):
    return func_name.lower().startswith(
        "{prefix}_{obj}_".format(prefix=module_prefix, obj=obj_name).lower())

def method_name_from_func_name(func_name):
    res = lv_method_pattern.match(func_name).group(1)
    return res if res != "del" else "delete"

def get_enum_name(enum):
    match_result = lv_enum_name_pattern.match(enum)
    return match_result.group(3) if match_result else enum

def str_enum_to_str(str_enum):
    res = lv_str_enum_pattern.match(str_enum).group(1)
    return ("%s_" % module_prefix.upper()) + res

def is_obj_ctor(func):
    if not create_obj_pattern.match(func.name):
        return False
    if not func.type.args:
        return False
    if not lv_base_obj_pattern.match(get_type(func.type.type, remove_quals=True)):
        return False
    args = func.type.args.params
    if len(args) < 1:
        return False
    if not lv_base_obj_pattern.match(get_type(args[0].type, remove_quals=True)):
        return False
    return True

def is_global_callback(arg_type):
    arg_type_str = get_name(arg_type.type)
    result = lv_global_callback_pattern.match(arg_type_str)
    return result

def is_struct(type):
    return isinstance(type, c_ast.Struct) or isinstance(type, c_ast.Union)

#
# Initialization, data structures, helper functions
#

obj_metadata = collections.OrderedDict()
func_metadata = collections.OrderedDict()
callback_metadata = collections.OrderedDict()
func_prototypes = {}

parser = c_parser.CParser()
gen = c_generator.CGenerator()
ast = parser.parse(s, filename="<none>")

# Fix forward declarations
forward_struct_decls = {}
for item in ast.ext[:]:
    if isinstance(item, c_ast.Decl) and item.name is None and \
       isinstance(item.type, c_ast.Struct) and item.type.name is not None:
        if item.type.decls is None:
            forward_struct_decls[item.type.name] = [item]
        else:
            if item.type.name in forward_struct_decls:
                decs = forward_struct_decls[item.type.name]
                if len(decs) == 2:
                    decl, td = decs
                    td.type.type.decls = item.type.decls[:]
                    ast.ext.remove(decl)
                    ast.ext.remove(item)
    elif isinstance(item, c_ast.Typedef) and isinstance(item.type, c_ast.TypeDecl) and \
         item.name and item.type.declname and item.name == item.type.declname and \
         isinstance(item.type.type, c_ast.Struct) and item.type.type.decls is None:
        if item.type.type.name in forward_struct_decls:
            forward_struct_decls[item.type.type.name].append(item)

# Types and structs
typedefs = [x.type for x in ast.ext if isinstance(x, c_ast.Typedef)]
synonym = {}
for t in typedefs:
    if isinstance(t, c_ast.TypeDecl) and isinstance(t.type, c_ast.IdentifierType):
        if t.declname != t.type.names[0]:
            synonym[t.declname] = t.type.names[0]
    if isinstance(t, c_ast.TypeDecl) and isinstance(t.type, c_ast.Struct):
        if t.declname != t.type.name:
            synonym[t.declname] = t.type.name

struct_typedefs = [typedef for typedef in typedefs if is_struct(typedef.type)]
structs_without_typedef = collections.OrderedDict(
    (decl.type.name, decl.type)
    for decl in ast.ext
    if hasattr(decl, "type") and is_struct(decl.type))

for typedef in struct_typedefs:
    if typedef.type.decls is None:
        struct_name = typedef.type.name
        if typedef.type.name in structs_without_typedef:
            typedef.type = structs_without_typedef[struct_name]

structs = collections.OrderedDict(
    (typedef.declname, typedef.type)
    for typedef in struct_typedefs
    if typedef.declname and typedef.type.decls)
structs.update(structs_without_typedef)
explicit_structs = collections.OrderedDict(
    (typedef.type.name, typedef.declname)
    for typedef in struct_typedefs
    if typedef.type.name)
opaque_structs = collections.OrderedDict(
    (typedef.declname, c_ast.Struct(name=typedef.declname, decls=[]))
    for typedef in typedefs
    if isinstance(typedef.type, c_ast.Struct) and typedef.type.decls == None)
structs.update({k: v for k, v in opaque_structs.items() if k not in structs})

# Functions and objects
func_defs = [x.decl for x in ast.ext if isinstance(x, c_ast.FuncDef)]
func_decls = [x for x in ast.ext if isinstance(x, c_ast.Decl) and isinstance(x.type, c_ast.FuncDecl)]
all_funcs = func_defs + func_decls
funcs = [f for f in all_funcs if not f.name.startswith("_")]
obj_ctors = [func for func in funcs if is_obj_ctor(func)]
for obj_ctor in obj_ctors:
    funcs.remove(obj_ctor)
obj_names = [create_obj_pattern.match(ctor.name).group(1) for ctor in obj_ctors]

def has_ctor(obj_name):
    return ctor_name_from_obj_name(obj_name) in [ctor.name for ctor in obj_ctors]

def get_ctor(obj_name):
    global obj_ctors
    return next(ctor for ctor in obj_ctors if ctor.name == ctor_name_from_obj_name(obj_name))

def get_methods(obj_name):
    global funcs
    return [func for func in funcs
            if is_method_of(func.name, obj_name)
            and (not func.name == ctor_name_from_obj_name(obj_name))]

@memoize
def noncommon_part(member_name, stem_name):
    common_part = commonprefix([member_name, stem_name])
    n = len(common_part) - 1
    while n > 0 and member_name[n] != "_":
        n -= 1
    return member_name[n + 1:]

@memoize
def get_first_arg(func):
    if not func.type.args:
        return None
    if not len(func.type.args.params) >= 1:
        return None
    if not func.type.args.params[0].type:
        return None
    return func.type.args.params[0].type

@memoize
def get_first_arg_type(func):
    first_arg = get_first_arg(func)
    if not first_arg:
        return None
    if not first_arg.type:
        return None
    return get_type(first_arg.type, remove_quals=True)

def get_base_struct_name(struct_name):
    return struct_name[:-2] if struct_name.endswith("_t") else struct_name

@memoize
def get_struct_functions(struct_name):
    global funcs
    if not struct_name:
        return []
    base_struct_name = get_base_struct_name(struct_name)
    reverse_aliases = [alias for alias in struct_aliases if struct_aliases[alias] == struct_name]
    return (
        [func for func in funcs
         if noncommon_part(simplify_identifier(func.name), simplify_identifier(struct_name))
            != simplify_identifier(func.name)
         and get_first_arg_type(func) == struct_name]
        if (struct_name in structs or len(reverse_aliases) > 0) else []
    ) + (
        get_struct_functions(struct_aliases[struct_name])
        if struct_name in struct_aliases else []
    )

@memoize
def is_struct_function(func):
    return func in get_struct_functions(get_first_arg_type(func))

@memoize
def is_static_member(func, obj_type=base_obj_type):
    first_arg = get_first_arg(func)
    if first_arg:
        if isinstance(first_arg, c_ast.ArrayDecl):
            return True
    if is_struct_function(func):
        return False
    first_arg_type = get_first_arg_type(func)
    return (first_arg_type == None) or (first_arg_type != obj_type)

parent_obj_names = {
    child_name: base_obj_name for child_name in obj_names if child_name != base_obj_name
}
parent_obj_names[base_obj_name] = None

# Parse Enums
enum_defs = [x for x in ast.ext if hasattr(x, "type") and isinstance(x.type, c_ast.Enum)]
enum_defs += [x.type for x in ast.ext
    if hasattr(x, "type") and hasattr(x.type, "type")
    and isinstance(x.type, c_ast.TypeDecl) and isinstance(x.type.type, c_ast.Enum)]

def get_enum_members(obj_name):
    global enums
    if obj_name not in enums:
        return []
    return [enum_member_name for enum_member_name, value in enums[obj_name].items()]

def get_enum_member_name(enum_member):
    if enum_member[0].isdigit():
        enum_member = "_" + enum_member
    return enum_member

def get_enum_value(obj_name, enum_member):
    return enums[obj_name][enum_member]

# Parse function pointers
func_typedefs = collections.OrderedDict(
    (t.name, t) for t in ast.ext
    if isinstance(t, c_ast.Typedef) and isinstance(t.type, c_ast.PtrDecl)
    and isinstance(t.type.type, c_ast.FuncDecl))

# Global blobs
blobs = collections.OrderedDict(
    (decl.name, decl.type.type)
    for decl in ast.ext
    if isinstance(decl, c_ast.Decl) and "extern" in decl.storage
    and hasattr(decl, "type") and isinstance(decl.type, c_ast.TypeDecl)
    and not decl.name.startswith("_"))

int_constants = []

#
# Type convertors for pybind11
#

class MissingConversionException(ValueError):
    pass

# pybind11 handles most basic types automatically
# We only need special handling for LVGL-specific types

# Map C type -> pybind11 C++ cast/expression for converting Python arg to C
py_to_c = {
    "bool": "py_to_basic",
    "char": "py_to_basic",
    "unsigned char": "py_to_basic",
    "short": "py_to_basic",
    "unsigned short": "py_to_basic",
    "int": "py_to_basic",
    "unsigned int": "py_to_basic",
    "long": "py_to_basic",
    "unsigned long": "py_to_basic",
    "long long": "py_to_basic",
    "unsigned long long": "py_to_basic",
    "int8_t": "py_to_basic",
    "uint8_t": "py_to_basic",
    "int16_t": "py_to_basic",
    "uint16_t": "py_to_basic",
    "int32_t": "py_to_basic",
    "uint32_t": "py_to_basic",
    "int64_t": "py_to_basic",
    "uint64_t": "py_to_basic",
    "size_t": "py_to_basic",
    "float": "py_to_basic",
    "double": "py_to_basic",
    "const char *": "py_to_str",
    "char *": "py_to_str",
    "void *": "py_to_void_ptr",
    "const void *": "py_to_void_ptr",
    "const uint8_t *": "py_to_void_ptr",
}

# Map C type -> pybind11 C++ expression for converting C return to Python
c_to_py = {
    "bool": "c_to_basic",
    "char": "c_to_basic",
    "unsigned char": "c_to_basic",
    "short": "c_to_basic",
    "unsigned short": "c_to_basic",
    "int": "c_to_basic",
    "unsigned int": "c_to_basic",
    "long": "c_to_basic",
    "unsigned long": "c_to_basic",
    "long long": "c_to_basic",
    "unsigned long long": "c_to_basic",
    "int8_t": "c_to_basic",
    "uint8_t": "c_to_basic",
    "int16_t": "c_to_basic",
    "uint16_t": "c_to_basic",
    "int32_t": "c_to_basic",
    "uint32_t": "c_to_basic",
    "int64_t": "c_to_basic",
    "uint64_t": "c_to_basic",
    "size_t": "c_to_basic",
    "float": "c_to_basic",
    "double": "c_to_basic",
    "const char *": "c_to_str",
    "char *": "c_to_str",
}

# Map C type -> Python type name for metadata
c_to_py_type = {
    "void": "None",
    "bool": "bool",
    "char": "int",
    "unsigned char": "int",
    "short": "int",
    "unsigned short": "int",
    "int": "int",
    "unsigned int": "int",
    "long": "int",
    "unsigned long": "int",
    "int8_t": "int",
    "uint8_t": "int",
    "int16_t": "int",
    "uint16_t": "int",
    "int32_t": "int",
    "uint32_t": "int",
    "int64_t": "int",
    "uint64_t": "int",
    "size_t": "int",
    "float": "float",
    "double": "float",
    "const char *": "str",
    "char *": "str",
    "void *": "object",
}

struct_aliases = collections.OrderedDict()

#
# Enum parsing
#

enums = collections.OrderedDict()
for enum_def in enum_defs:
    if isinstance(enum_def, c_ast.TypeDecl) and enum_def.declname == "memory_order":
        continue
    while hasattr(enum_def.type, "name") and not enum_def.type.values:
        enum_def = next(
            e for e in enum_defs
            if hasattr(e.type, "name") and e.type.name == enum_def.type.name and e.type.values)
    member_names = [
        member.name for member in enum_def.type.values.enumerators
        if not member.name.startswith("_")]
    enum_name = commonprefix(member_names)
    enum_name = "_".join(enum_name.split("_")[:-1])
    enum = collections.OrderedDict()
    for member in enum_def.type.values.enumerators:
        if member.name.startswith("_"):
            continue
        member_name = member.name[len(enum_name) + 1:] if len(enum_name) > 0 else member.name
        if member_name[0].isdigit():
            member_name = "_" + member_name
        if len(enum_name) > 0 and get_enum_name(enum_name) != "ENUM":
            enum[member_name] = member.name
        else:
            int_constants.append(member.name)
    if len(enum) > 0:
        if len(get_enum_name(enum_name)) > 0:
            prev_enum = enums.get(enum_name)
            if prev_enum:
                prev_enum.update(enum)
            else:
                enums[enum_name] = enum

for enum in [enum for enum in enums if len(enums[enum]) == 1 and enum.startswith("ENUM")]:
    int_constants.append("%s_%s" % (enum, next(iter(enums[enum]))))
    del enums[enum]

# String enums
# LV_STR_SYMBOL enum: its members (LV_STR_SYMBOL_BULLET etc.) are integer
# enum values of unregistered type _lv_str_symbol_id_t that pybind11 cannot
# convert.  They duplicate the LV_SYMBOL_* string macros which are already
# emitted as module attributes elsewhere, so omitting them avoids both the
# conversion error and the duplicate-attribute conflict.
for enum_def in enum_defs:
    if not enum_def.type.values:
        continue
    member_names = [
        str_enum_to_str(member.name) for member in enum_def.type.values.enumerators
        if lv_str_enum_pattern.match(member.name)]
    enum_name = commonprefix(member_names)
    enum_name = "_".join(enum_name.split("_")[:-1])
    if enum_name == module_prefix.upper() + "_SYMBOL":
        # LV_STR_SYMBOL_* are integer IDs that duplicate the LV_SYMBOL_* string
        # macros — skip them entirely.
        continue
    if enum_name:
        enum = collections.OrderedDict()
        for member in enum_def.type.values.enumerators:
            full_name = str_enum_to_str(member.name)
            member_name = full_name[len(enum_name) + 1:]
            enum[member_name] = full_name
        if len(enum) > 0:
            if enum_name in enums:
                enums[enum_name].update(enum)
            else:
                enums[enum_name] = enum

eprint("--> Found %d objects, %d enums, %d functions, %d structs" %
       (len(obj_names), len(enums), len(funcs), len(structs)))

# =====================================================================
# PYBIND11 CODE GENERATION
# =====================================================================

# Output buffer
output_lines = []

def emit(line=""):
    output_lines.append(line)

# ------------------------------------------------------------------
# Emit file header
# ------------------------------------------------------------------

emit("/*")
emit(" * Auto-Generated file by gen_pybind11.py, DO NOT EDIT!")
emit(" *")
emit(" * Command line:")
emit(" * %s" % " ".join(argv))
emit(" *")
emit(" * Generating Objects: %s" % ", ".join(
    ["%s(%s)" % (objname, parent_obj_names[objname]) for objname in obj_names]))
emit(" */")
emit("")
emit("#include <pybind11/pybind11.h>")
emit("#include <pybind11/stl.h>")
emit("#include <pybind11/functional.h>")
emit("#include <pybind11/numpy.h>")
emit('#include "lvgl/lvgl.h"')
emit('#include "lvgl/driver_backends.h"')
emit('#include "lvgl_pybind_helpers.h"')
emit("")
emit("namespace py = pybind11;")
emit("")

# ------------------------------------------------------------------
# Helper: determine if a type is a basic type (handled automatically by pybind11)
# ------------------------------------------------------------------

BASIC_TYPES = {
    "bool", "_Bool", "char", "unsigned char", "short", "unsigned short",
    "int", "unsigned int", "long", "unsigned long",
    "long long", "unsigned long long",
    "int8_t", "uint8_t", "int16_t", "uint16_t",
    "int32_t", "uint32_t", "int64_t", "uint64_t",
    "size_t", "float", "double",
    "void", "const char *", "char *",
    "void *", "const void *", "const uint8_t *",
}

# Types that should be skipped entirely (can't be bound to Python)
SKIP_TYPES = {"va_list", "__va_list", "__builtin_va_list"}

def is_return_type_supported(return_type):
    """Check if a return type can be safely used with pybind11.
    Only basic types and lv_obj_t* are safe. Struct pointers cause
    incomplete type errors because pybind11 needs full type definition."""
    if return_type == "void":
        return True
    if is_basic_type(return_type):
        return True
    if is_obj_ptr_type(return_type):
        return True
    # Enum return types are fine
    if return_type in enums or return_type.rstrip(" *") in enums:
        return True
    # All other types (struct pointers, etc.) - not supported
    return False

def is_obj_ptr_type(type_str):
    """Check if a type is lv_obj_t* or a widget pointer."""
    return type_str == "%s_obj_t *" % module_prefix or \
           lv_base_obj_pattern.match(type_str) is not None

def is_enum_type(type_str):
    """Check if a type matches a known enum."""
    return type_str in enums or type_str.rstrip(" *") in enums

def resolve_typedef(type_str):
    """Resolve typedef chains: e.g. lv_anim_enable_t -> bool, lv_dir_t -> ..."""
    seen = set()
    current = type_str
    while current in synonym and current not in seen:
        seen.add(current)
        current = synonym[current]
    # Normalize C99 _Bool to C++ bool for pybind11 compatibility
    if current == "_Bool":
        current = "bool"
    return current

def is_enum_c_type(type_str):
    """Check if a type string is a known enum C type (e.g., lv_dir_t, lv_align_t).
    The enums dict keys are prefix names (LV_DIR), but function args use C type names (lv_dir_t).
    We use enum_c_types to map between them."""
    if type_str in enum_c_types.values():
        return True
    # Also check if it's a typedef that resolves to an enum
    resolved = resolve_typedef(type_str)
    if resolved in enum_c_types.values():
        return True
    return False

def cpp_type_for(type_str):
    """Get the C++ type string for pybind11 lambda parameters.
    Returns None for types pybind11 can't handle automatically.
    lv_obj_t* is mapped to LvObjWrapper since we have a wrapper class."""
    if type_str == "void":
        return "void"
    if type_str == "const char *":
        return "const char *"
    if type_str == "char *":
        return "char *"
    if type_str == "void *":
        return "void *"
    if type_str == "bool":
        return "bool"
    # Resolve typedefs to underlying types (e.g. lv_anim_enable_t -> bool)
    resolved = resolve_typedef(type_str)
    if resolved != type_str:
        # Recursively call cpp_type_for on the resolved type
        result = cpp_type_for(resolved)
        if result is not None:
            return result
    # Skip array pointer types like "char *[]" which pybind11 can't handle
    if type_str.endswith("[]"):
        return None
    # Skip double pointer types like "char **" which pybind11 can't handle
    if type_str.endswith(" **"):
        return None
    if type_str.endswith(" *"):
        base = type_str[:-2]
        if is_obj_ptr_type(type_str):
            # Use LvObjWrapper for lv_obj_t* parameters
            return "LvObjWrapper &"
        # Only allow basic type pointers (e.g. int32_t *, uint8_t *)
        # and enum pointers. Struct pointers cause incomplete type errors.
        if base in BASIC_TYPES or base in enums or base.rstrip(" *") in enums or is_enum_c_type(base):
            return base + " *"
        # All other struct pointers - skip (opaque/incomplete types)
        return None
    # Non-pointer types
    if type_str in BASIC_TYPES:
        return type_str
    if type_str in enums or type_str.rstrip(" *") in enums or is_enum_c_type(type_str):
        return type_str
    # Unknown non-basic type
    return None

def get_arg_name(arg, index):
    """Get argument name from AST node."""
    if hasattr(arg, "name") and arg.name:
        return arg.name
    return "arg%d" % index

# ------------------------------------------------------------------
# Track what has been generated
# ------------------------------------------------------------------

generated_funcs = collections.OrderedDict()
generated_enums = set()
generated_struct_classes = set()

# ------------------------------------------------------------------
# Build enum C type mapping (used later for code generation)
# ------------------------------------------------------------------

# Build a mapping: enum C type name -> enum name
# LVGL enums in headers are typedef enum { ... } lv_xxx_t;
# We need to find the C type for each enum
enum_c_types = {}  # enum_name -> C type name (e.g., "lv_align_t")

# Find enum typedefs in the AST
for item in ast.ext:
    if isinstance(item, c_ast.Typedef) and isinstance(item.type, c_ast.TypeDecl):
        if isinstance(item.type.type, c_ast.Enum):
            type_name = item.name  # e.g., "lv_align_t"
            # Find which internal enum name this maps to
            if item.type.type.values:
                member_names = [
                    m.name for m in item.type.type.values.enumerators
                    if not m.name.startswith("_")]
                if member_names:
                    prefix = commonprefix(member_names)
                    prefix = "_".join(prefix.split("_")[:-1])
                    if prefix in enums:
                        enum_c_types[prefix] = type_name

# Also try reverse: enum_name -> guess C type from naming convention
for enum_name in enums:
    if enum_name not in enum_c_types:
        # Try common patterns: lv_ALIGN_t, lv_align_t
        for suffix in ["_t"]:
            candidate = enum_name + suffix
            # Check if this typedef exists
            for item in ast.ext:
                if isinstance(item, c_ast.Typedef) and item.name == candidate:
                    enum_c_types[enum_name] = candidate
                    break

# Note: enum emit code is deferred to inside PYBIND11_MODULE block below

# ------------------------------------------------------------------
# Generate PYBIND11_MODULE block
# ------------------------------------------------------------------

emit("PYBIND11_MODULE(_lvgl, m) {")
emit("    m.doc() = \"LVGL Python bindings (auto-generated)\";")
emit("")

# ------------------------------------------------------------------
# Emit enum definitions (must be inside PYBIND11_MODULE)
# ------------------------------------------------------------------

emit("    /*")
emit("     * Enum definitions")
emit("     */")
emit("")

# Collect all enum type Python names first to detect collisions.
# When .export_values() is used, enum values are exported to the module scope.
# If a value name collides with another enum type name, pybind11 fails with:
#   "generic_type: cannot initialize type X: an object with that name is already defined"
# Solution: prefix colliding value names with the parent enum type name.
enum_type_py_names = set()
for enum_name, members in enums.items():
    c_type = enum_c_types.get(enum_name, None)
    if c_type:
        py_name = sanitize(get_enum_name(enum_name))
        enum_type_py_names.add(py_name)

for enum_name, members in enums.items():
    c_type = enum_c_types.get(enum_name, None)
    py_name = sanitize(get_enum_name(enum_name))

    if c_type:
        emit("    py::enum_<%s>(m, \"%s\")" % (c_type, py_name))
    else:
        # No C type found, emit as module attribute constants.
        # Cast to (int) so pybind11 can always convert the value even if
        # the C enum type is not registered with py::enum_<>.
        # Prefix member names with the enum group name to avoid collisions
        # with py::enum_<> type names (e.g. LV_STYLE_ALIGN vs lv_align_t "ALIGN").
        emit("    /* Enum %s (no C type found, emitting as constants) */" % enum_name)
        # Determine prefix: e.g. LV_STYLE -> STYLE_, LV_OPA -> OPA_
        enum_prefix = get_enum_name(enum_name) + "_"
        for member_name, c_name in members.items():
            safe_name = sanitize(enum_prefix + member_name)
            emit("    m.attr(\"%s\") = (int)%s;" % (safe_name, c_name))
        emit("")
        continue

    for member_name, c_name in members.items():
        safe_name = sanitize(member_name)
        # If this value name collides with another enum type name, prefix it
        # with the parent enum name to avoid the pybind11 conflict.
        # e.g. EVENT.KEY -> EVENT_KEY, INDEV_MODE.EVENT -> INDEV_MODE_EVENT
        if safe_name in enum_type_py_names and safe_name != py_name:
            safe_name = py_name + "_" + safe_name
        emit("        .value(\"%s\", %s)" % (safe_name, c_name))
    emit("        .export_values();")
    emit("")
    generated_enums.add(enum_name)

# Emit integer constants
if int_constants:
    emit("    /* Integer constants */")
    for const_name in int_constants:
        safe_name = sanitize(get_enum_name(const_name) if "_" in const_name else const_name)
        emit("    m.attr(\"%s\") = %s;" % (safe_name, const_name))
    emit("")

# ------------------------------------------------------------------
# Module-level functions (init, timer_handler, etc.)
# ------------------------------------------------------------------

emit("    /* Module-level functions */")
emit("    m.def(\"init\", &lv_init, \"Initialize LVGL library\");")
emit("    m.def(\"deinit\", &lv_deinit, \"Deinitialize LVGL library\");")
emit("    m.def(\"is_initialized\", &lv_is_initialized, \"Check if LVGL is initialized\");")
emit("    m.def(\"timer_handler\", &py_timer_handler, \"Call LVGL timer handler\");")
emit("    m.def(\"task_handler\", &py_timer_handler, \"Alias for timer_handler\");")
emit("    m.def(\"tick_get\", &lv_tick_get, \"Get elapsed milliseconds\");")
emit("    m.def(\"tick_inc\", &lv_tick_inc, \"Update tick value\", py::arg(\"ms\"));")
emit("")

# ------------------------------------------------------------------
# Version info
# ------------------------------------------------------------------

emit("    /* Version info */")
emit("    m.def(\"version_major\", []() { return lv_version_major(); });")
emit("    m.def(\"version_minor\", []() { return lv_version_minor(); });")
emit("    m.def(\"version_patch\", []() { return lv_version_patch(); });")
emit("    m.def(\"version_info\", []() { return std::string(lv_version_info()); });")
emit("")

# ------------------------------------------------------------------
# Display wrapper class
# ------------------------------------------------------------------

emit("    /* Display */")
emit("    py::class_<LvDisplayWrapper>(m, \"Display\")")
emit("        .def(py::init<>())")
emit("        .def(\"set_resolution\", [](LvDisplayWrapper &self, int32_t h, int32_t v) {")
emit("            lv_display_set_resolution(self.get(), h, v);")
emit("        }, py::arg(\"hor_res\"), py::arg(\"ver_res\"))")
emit("        .def(\"set_rotation\", [](LvDisplayWrapper &self, lv_display_rotation_t r) {")
emit("            lv_display_set_rotation(self.get(), r);")
emit("        }, py::arg(\"rotation\"))")
emit("        .def(\"get_horizontal_resolution\", [](LvDisplayWrapper &self) -> int32_t {")
emit("            return lv_display_get_horizontal_resolution(self.get());")
emit("        })")
emit("        .def(\"get_vertical_resolution\", [](LvDisplayWrapper &self) -> int32_t {")
emit("            return lv_display_get_vertical_resolution(self.get());")
emit("        })")
emit("        .def(\"flush_ready\", [](LvDisplayWrapper &self) {")
emit("            lv_display_flush_ready(self.get());")
emit("        });")
emit("")
emit("    m.def(\"display_create\", [](int32_t h, int32_t v) -> LvDisplayWrapper* {")
emit("        return new LvDisplayWrapper(lv_display_create(h, v));")
emit("    }, py::arg(\"hor_res\"), py::arg(\"ver_res\"));")
emit("    m.def(\"display_get_default\", []() -> LvDisplayWrapper* {")
emit("        return new LvDisplayWrapper(lv_display_get_default());")
emit("    });")
emit("")

# ------------------------------------------------------------------
# Indev wrapper class
# ------------------------------------------------------------------

emit("    /* Input device */")
emit("    py::class_<LvIndevWrapper>(m, \"Indev\")")
emit("        .def(py::init<>());")
emit("")

# ------------------------------------------------------------------
# Base Object class
# ------------------------------------------------------------------

emit("    /* Base Object class */")
emit("    auto obj_cls = py::class_<LvObjWrapper>(m, \"Obj\")")
emit("        .def(py::init<>())")
emit("        .def_static(\"create\", [](LvObjWrapper *parent) -> LvObjWrapper* {")
emit("            lv_obj_t *p = parent ? parent->get() : nullptr;")
emit("            return new LvObjWrapper(lv_obj_create(p));")
emit("        }, py::arg(\"parent\") = py::none())")

# Generate methods for base obj
obj_methods = get_methods(base_obj_name)
for method in obj_methods:
    try:
        # Skip variadic functions (va_list args that pycparser can't handle)
        if "vfmt" in method.name or "vprintf" in method.name:
            continue

        args = method.type.args.params if method.type.args else []
        # Skip methods with complex argument types we can't handle
        skip = False
        for a in args:
            if isinstance(a, c_ast.EllipsisParam):
                skip = True
                break
        if skip:
            continue

        method_name = sanitize(method_name_from_func_name(method.name))
        if method_name in ["delete"]:  # reserved
            method_name = "delete_obj"

        # Get return type
        return_type = get_type(method.type.type, remove_quals=True)
        return_qualified = gen.visit(method.type.type)

        # Skip non-methods: first arg must be lv_obj_t*
        if args:
            first_type = get_type(args[0].type, remove_quals=True)
            if not is_obj_ptr_type(first_type):
                continue

        # Build lambda body
        # For obj methods, first arg is always self (lv_obj_t*)
        call_args = []
        lambda_params = []
        py_args = []

        for i, arg in enumerate(args):
            if i == 0:
                continue  # skip self
            arg_name = get_arg_name(arg, i)
            arg_type = get_type(arg.type, remove_quals=True)
            cpp_type = cpp_type_for(arg_type)
            if cpp_type is None:
                skip = True
                break
            # For LvObjWrapper& params, need to call .get() when passing to C function
            if cpp_type == "LvObjWrapper &":
                call_args.append("%s.get()" % arg_name)
            else:
                call_args.append(arg_name)
            lambda_params.append("%s %s" % (cpp_type, arg_name))
            py_args.append("py::arg(\"%s\")" % arg_name)

        if skip:
            continue

        call_args_str = ", ".join(["self.get()"] + call_args)
        lambda_params_str = ", ".join(["LvObjWrapper &self"] + lambda_params)
        py_args_str = ", ".join(py_args)

        # Verify arg count matches
        c_arg_count = sum(1 for a in args if not (isinstance(a, c_ast.EllipsisParam) or (hasattr(a, 'type') and get_type(a.type, remove_quals=True) == 'void')))
        expected_call_args = c_arg_count
        actual_call_args = 1 + len(call_args)
        if actual_call_args < expected_call_args:
            continue

        if not is_return_type_supported(return_type):
            continue

        if return_type == "void":
            emit("        .def(\"%s\", [](%s) { %s(%s); }" % (
                method_name, lambda_params_str, method.name, call_args_str))
        elif is_obj_ptr_type(return_type):
            emit("        .def(\"%s\", [](%s) -> LvObjWrapper* { return new LvObjWrapper(%s(%s)); }" % (
                method_name, lambda_params_str, method.name, call_args_str))
        elif is_basic_type(return_type):
            emit("        .def(\"%s\", [](%s) -> %s { return %s(%s); }" % (
                method_name, lambda_params_str, return_qualified, method.name, call_args_str))
        else:
            continue

        if py_args_str:
            emit("            , %s)" % py_args_str)
        else:
            emit("            )")

        generated_funcs[method.name] = True
    except Exception as e:
        pass  # Skip methods we can't handle

emit("        ;")  # end obj_cls
emit("")

# ------------------------------------------------------------------
# Screen management helpers
# ------------------------------------------------------------------

emit("    /* Screen management */")
emit("    m.def(\"screen_active\", []() -> LvObjWrapper* {")
emit("        return new LvObjWrapper(lv_screen_active(), false);")
emit("    });")
emit("    m.def(\"screen_load\", [](LvObjWrapper &screen) {")
emit("        lv_screen_load(screen.get());")
emit("    }, py::arg(\"screen\"));")
emit("")

# ------------------------------------------------------------------
# Widget factory functions + widget methods on obj_cls
# ------------------------------------------------------------------
# pybind11 requires a 1:1 mapping between a C++ type and its Python name.
# We cannot register py::class_<LvObjWrapper> multiple times with different names.
# Instead, each widget gets a module-level factory function (e.g. lv.Image(parent))
# that returns an Obj instance, and widget-specific methods are added to obj_cls.

# First pass: emit factory functions for all widgets
emit("    /* Widget factory functions */")
for obj_name in obj_names:
    if obj_name == base_obj_name:
        continue  # Already generated above

    py_name = sanitize(obj_name)
    class_name = py_name[0].upper() + py_name[1:]

    if has_ctor(obj_name):
        ctor = get_ctor(obj_name)
        ctor_args = ctor.type.args.params if ctor.type.args else []

        # Build ctor lambda
        call_args = []
        lambda_params = []
        py_args = []

        skip = False
        for i, arg in enumerate(ctor_args):
            if i == 0:
                # First arg is parent
                lambda_params.append("LvObjWrapper *parent")
                py_args.append("py::arg(\"parent\") = py::none()")
                continue
            arg_name = get_arg_name(arg, i)
            arg_type = get_type(arg.type, remove_quals=True)
            if arg_type == "void":
                continue
            call_args.append(arg_name)
            cpp_type = cpp_type_for(arg_type)
            if cpp_type is None:
                skip = True
                break
            lambda_params.append("%s %s" % (cpp_type, arg_name))
            py_args.append("py::arg(\"%s\")" % arg_name)

        if skip:
            continue

        parent_arg = "parent ? parent->get() : lv_screen_active()"
        all_call_args = ", ".join([parent_arg] + call_args)
        lambda_params_str = ", ".join(lambda_params)
        py_args_str = ", ".join(py_args)

        emit("    m.def(\"%s\", [](%s) -> LvObjWrapper* {" % (class_name, lambda_params_str))
        emit("        return new LvObjWrapper(%s(%s));" % (ctor.name, all_call_args))
        if py_args_str:
            emit("    }, %s);" % py_args_str)
        else:
            emit("    });")

        generated_funcs[ctor.name] = True

emit("")

# Second pass: add widget-specific methods to obj_cls
emit("    /* Widget-specific methods on Obj */")
# Track bound Python method names to detect collisions.
# When multiple widgets define the same method name (e.g. set_value, set_text),
# pybind11 would overwrite the earlier binding with the later one.
# We prefix colliding names with the widget name (e.g. slider_set_value).
bound_method_names = {}  # python_method_name -> obj_name that first defined it
for obj_name in obj_names:
    if obj_name == base_obj_name:
        continue  # Already generated above

    methods = get_methods(obj_name)
    for method in methods:
        try:
            if method.name in generated_funcs:
                continue

            # Skip variadic functions (va_list args that pycparser can't handle)
            if "vfmt" in method.name or "vprintf" in method.name:
                continue

            args = method.type.args.params if method.type.args else []
            skip = False
            for a in args:
                if isinstance(a, c_ast.EllipsisParam):
                    skip = True
                    break
            if skip:
                continue

            method_name = sanitize(method_name_from_func_name(method.name))
            if method_name in ["delete"]:
                method_name = "delete_obj"

            # Handle method name collisions between widgets
            if method_name in bound_method_names:
                first_obj = bound_method_names[method_name]
                if first_obj != obj_name:
                    # Collision: prefix both the current and the original with widget name
                    # (The original was already emitted without prefix, so we emit a
                    # prefixed alias for the current widget. The original keeps its name.)
                    py_name = sanitize(obj_name)
                    method_name = py_name + "_" + method_name
                    # If the prefixed name also collides, skip
                    if method_name in bound_method_names:
                        continue
            bound_method_names[method_name] = obj_name

            return_type = get_type(method.type.type, remove_quals=True)
            return_qualified = gen.visit(method.type.type)

            call_args = []
            lambda_params = []
            py_args = []

            # Skip non-methods: first arg must be lv_obj_t*
            if args:
                first_type = get_type(args[0].type, remove_quals=True)
                if not is_obj_ptr_type(first_type):
                    continue

            for i, arg in enumerate(args):
                if i == 0:
                    continue  # skip self
                arg_name = get_arg_name(arg, i)
                arg_type = get_type(arg.type, remove_quals=True)
                if arg_type == "void":
                    continue
                # lv_obj_t* args are handled as LvObjWrapper&, not as opaque structs
                if is_obj_ptr_type(arg_type):
                    call_args.append("%s.get()" % arg_name)
                    lambda_params.append("LvObjWrapper & %s" % arg_name)
                    py_args.append("py::arg(\"%s\")" % arg_name)
                    continue
                # Skip if arg is an opaque struct pointer (pybind11 can't handle incomplete types)
                arg_base = arg_type.rstrip(" *")
                if arg_type.endswith(" *") and arg_base in opaque_structs:
                    skip = True
                    break
                cpp_type = cpp_type_for(arg_type)
                if cpp_type is None:
                    skip = True
                    break
                # For LvObjWrapper& params, need to call .get() when passing to C function
                if cpp_type == "LvObjWrapper &":
                    call_args.append("%s.get()" % arg_name)
                else:
                    call_args.append(arg_name)
                lambda_params.append("%s %s" % (cpp_type, arg_name))
                py_args.append("py::arg(\"%s\")" % arg_name)

            call_args_str = ", ".join(["self.get()"] + call_args)
            lambda_params_str = ", ".join(["LvObjWrapper &self"] + lambda_params)
            py_args_str = ", ".join(py_args)

            # Verify arg count matches: self + call_args must cover all non-void C params
            # Count expected C args (excluding void args)
            c_arg_count = sum(1 for a in args if not (isinstance(a, c_ast.EllipsisParam) or (hasattr(a, 'type') and get_type(a.type, remove_quals=True) == 'void')))
            expected_call_args = c_arg_count  # self.get() + call_args
            actual_call_args = 1 + len(call_args)  # 1 for self.get()
            if actual_call_args < expected_call_args:
                # Some args were silently dropped, skip this function
                continue

            if not is_return_type_supported(return_type):
                continue

            if return_type == "void":
                emit("    obj_cls.def(\"%s\", [](%s) { %s(%s); }" % (
                    method_name, lambda_params_str, method.name, call_args_str))
            elif is_obj_ptr_type(return_type):
                emit("    obj_cls.def(\"%s\", [](%s) -> LvObjWrapper* { return new LvObjWrapper(%s(%s)); }" % (
                    method_name, lambda_params_str, method.name, call_args_str))
            elif is_basic_type(return_type):
                emit("    obj_cls.def(\"%s\", [](%s) -> %s { return %s(%s); }" % (
                    method_name, lambda_params_str, return_qualified, method.name, call_args_str))
            else:
                continue

            if py_args_str:
                emit("        , %s);" % py_args_str)
            else:
                emit("        );")

            generated_funcs[method.name] = True
        except Exception as e:
            pass

emit("")

# ------------------------------------------------------------------
# Event callback support
# ------------------------------------------------------------------

emit("    /* Event callback support */")
emit("    obj_cls.def(\"add_event_cb\", [](LvObjWrapper &self, int filter, py::function callback) {")
emit("        register_event_callback(self.get(), (lv_event_code_t)filter, callback);")
emit("    }, py::arg(\"filter\"), py::arg(\"callback\"));")
emit("")

# ------------------------------------------------------------------
# Color utilities
# ------------------------------------------------------------------

emit("    /* Color utilities */")
emit("    m.def(\"color_make\", [](uint8_t r, uint8_t g, uint8_t b) { return lv_color_make(r, g, b); },")
emit("        py::arg(\"r\"), py::arg(\"g\"), py::arg(\"b\"));")
emit("    m.def(\"color_hex\", [](uint32_t c) { return lv_color_hex(c); }, py::arg(\"c\"));")
emit("    m.def(\"color_black\", []() { return lv_color_black(); });")
emit("    m.def(\"color_white\", []() { return lv_color_white(); });")
emit("")

# ------------------------------------------------------------------
# OPA (Opacity) constants — now emitted by the enum block above,
# no need to duplicate them here.
# ------------------------------------------------------------------

emit("")

# ------------------------------------------------------------------
# Font default
# ------------------------------------------------------------------

emit("    /* Font default */")
emit("    m.def(\"font_get_default\", []() -> const lv_font_t * { return lv_font_get_default(); });")
emit("")

# ------------------------------------------------------------------
# Global functions (not methods of any object)
# ------------------------------------------------------------------

emit("    /* Global module functions */")
module_funcs = [func for func in funcs if func.name not in generated_funcs]
not_generated = []

for func in module_funcs:
    try:
        if func.name in generated_funcs:
            continue

        # Skip variadic functions (va_list args that pycparser can't handle)
        if "vfmt" in func.name or "vprintf" in func.name:
            not_generated.append(func.name)
            continue

        args = func.type.args.params if func.type.args else []
        # Handle single void arg
        if len(args) == 1 and get_type(args[0].type, remove_quals=True) == "void":
            args = []

        # Skip functions with ellipsis or complex types
        skip = False
        for a in args:
            if isinstance(a, c_ast.EllipsisParam):
                skip = True
                break
        if skip:
            not_generated.append(func.name)
            continue

        return_type = get_type(func.type.type, remove_quals=True)
        return_qualified = gen.visit(func.type.type)

        func_name = sanitize(simplify_identifier(func.name))

        # Build lambda
        call_args = []
        lambda_params = []
        py_args = []

        for i, arg in enumerate(args):
            arg_name = get_arg_name(arg, i)
            arg_type = get_type(arg.type, remove_quals=True)
            if arg_type == "void":
                continue
            # lv_obj_t* args are handled as LvObjWrapper&, not as opaque structs
            if is_obj_ptr_type(arg_type):
                call_args.append("%s.get()" % arg_name)
                lambda_params.append("LvObjWrapper & %s" % arg_name)
                py_args.append("py::arg(\"%s\")" % arg_name)
                continue
            # Skip if arg is an opaque struct pointer (pybind11 can't handle incomplete types)
            arg_base = arg_type.rstrip(" *")
            if arg_type.endswith(" *") and arg_base in opaque_structs:
                skip = True
                break
            cpp_type = cpp_type_for(arg_type)
            if cpp_type is None:
                skip = True
                break
            # For LvObjWrapper& params, need to call .get() when passing to C function
            if cpp_type == "LvObjWrapper &":
                call_args.append("%s.get()" % arg_name)
            else:
                call_args.append(arg_name)
            lambda_params.append("%s %s" % (cpp_type, arg_name))
            py_args.append("py::arg(\"%s\")" % arg_name)

        if skip:
            not_generated.append(func.name)
            continue

        call_args_str = ", ".join(call_args)
        lambda_params_str = ", ".join(lambda_params)
        py_args_str = ", ".join(py_args)

        # Verify arg count matches for global functions
        c_arg_count = sum(1 for a in args if not (isinstance(a, c_ast.EllipsisParam) or (hasattr(a, 'type') and get_type(a.type, remove_quals=True) == 'void')))
        if len(call_args) < c_arg_count:
            not_generated.append(func.name)
            continue

        if not is_return_type_supported(return_type):
            not_generated.append(func.name)
            continue

        if return_type == "void":
            emit("    m.def(\"%s\", [](%s) { %s(%s); }" % (
                func_name, lambda_params_str, func.name, call_args_str))
        elif is_obj_ptr_type(return_type):
            emit("    m.def(\"%s\", [](%s) -> LvObjWrapper* { return new LvObjWrapper(%s(%s)); }" % (
                func_name, lambda_params_str, func.name, call_args_str))
        elif is_basic_type(return_type):
            emit("    m.def(\"%s\", [](%s) -> %s { return %s(%s); }" % (
                func_name, lambda_params_str, return_qualified, func.name, call_args_str))
        else:
            not_generated.append(func.name)
            continue

        if py_args_str:
            emit("        , %s);" % py_args_str)
        else:
            emit("        );")

        generated_funcs[func.name] = True
    except Exception as e:
        not_generated.append(func.name)

if not_generated:
    emit("")
    emit("    /* Functions not generated:")
    for name in not_generated:
        emit("     * %s" % name)
    emit("     */")

# ------------------------------------------------------------------
# Manually bound platform-specific functions (not from lvgl.h)
# These are from lv_port_linux / driver_backends.h
# ------------------------------------------------------------------

emit("")
emit("    /* Driver backend functions (from lv_port_linux, not auto-generated) */")
emit("    m.def(\"driver_backends_register\", []() { driver_backends_register(); },")
emit("        \"Register all available driver backends (must be called first)\");")
emit("    m.def(\"driver_backends_init_backend\", [](const std::string &name) -> int {")
emit("        return driver_backends_init_backend(const_cast<char*>(name.c_str()));")
emit("    }, py::arg(\"backend_name\"),")
emit("        \"Initialize a driver backend by name (e.g. 'DRM', 'FBDEV', 'EVDEV')\");")
emit("    m.def(\"driver_backends_is_supported\", [](const std::string &name) -> int {")
emit("        return driver_backends_is_supported(const_cast<char*>(name.c_str()));")
emit("    }, py::arg(\"backend_name\"),")
emit("        \"Check if a backend is supported, returns 1 or 0\");")
emit("    m.def(\"driver_backends_print_supported\", []() -> int {")
emit("        return driver_backends_print_supported();")
emit("    }, \"Print supported backends to stdout\");")
emit("    m.def(\"driver_backends_run_loop\", []() { driver_backends_run_loop(); },")
emit("        \"Enter the run loop of the selected backend\");")

emit("")
emit("}  // PYBIND11_MODULE")

# ------------------------------------------------------------------
# Write output
# ------------------------------------------------------------------

output_text = "\n".join(output_lines)

if output_file:
    with open(output_file, "w") as f:
        f.write(output_text)
    eprint("Generated %s (%d lines)" % (output_file, len(output_lines)))
else:
    print(output_text)

eprint("Summary:")
eprint("  Objects: %d" % len(obj_names))
eprint("  Enums: %d" % len(enums))
eprint("  Functions generated: %d / %d" % (len(generated_funcs), len(funcs) + len(obj_ctors)))
eprint("  Functions not generated: %d" % len(not_generated))
