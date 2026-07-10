#!/usr/bin/env python3
"""
gen_pybind11.py - Generate pybind11 bindings for LVGL from header files.

Architecture: libclang → IR → pybind11 C++ → Python wrapper

This script serves as the pipeline entry point. It:
1. Parses LVGL headers using libclang
2. Builds an Intermediate Representation (IR) of the API
3. Generates pybind11 C++ bindings from the IR
4. Generates Python wrapper classes from the IR

Usage:
    python3 gen_pybind11.py \
        -I /path/to/lvgl/include \
        --compile-commands /path/to/compile_commands.json \
        -o lvgl_pybind_generated.cpp \
        lvgl.h

    Or with manual arguments:
    python3 gen_pybind11.py \
        -I /path/to/lvgl/include \
        -I /path/to/lv_conf.h/dir \
        -D LV_CONF_PATH=lv_conf.h \
        -o lvgl_pybind_generated.cpp \
        /path/to/lvgl/lvgl.h
"""

from __future__ import print_function
import sys
import os
from argparse import ArgumentParser

# Ensure local lvgl_ir/ and lvgl_codegen/ are importable
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

def eprint(*args, **kwargs):
    print(*args, file=sys.stderr, **kwargs)

#
# Argument parsing
#

argParser = ArgumentParser(description='Generate pybind11 bindings for LVGL (libclang → IR pipeline)')
argParser.add_argument("-I", "--include", dest="include",
    help="Preprocessor include path", metavar="<Include Path>", action="append")
argParser.add_argument("-D", "--define", dest="define",
    help="Define preprocessor macro", metavar="<Macro Name>", action="append")
argParser.add_argument("-M", "--module_name", dest="module_name",
    help="Module name", metavar="<Module name string>", action="store")
argParser.add_argument("-MP", "--module_prefix", dest="module_prefix",
    help="Module prefix that starts every function name",
    metavar="<Prefix string>", action="store")
argParser.add_argument("--compile-commands", dest="compile_commands",
    help="Path to compile_commands.json from CMake build", metavar="<Path>", action="store")
argParser.add_argument("-o", "--output", dest="output",
    help="Output C++ file name", metavar="<Output File>", action="store")
argParser.add_argument("--wrapper-output", dest="wrapper_output",
    help="Output Python wrapper file name", metavar="<Output File>", action="store")
argParser.add_argument("input", nargs="+")
argParser.set_defaults(include=[], define=[], output=None, compile_commands=None,
                       wrapper_output=None, module_name=None, module_prefix=None, input=[])

args = argParser.parse_args()

module_name = args.module_name or "lvgl"
module_prefix = args.module_prefix or module_name
output_file = args.output
wrapper_output = args.wrapper_output

# Verify libclang is available
try:
    import clang.cindex
except ImportError:
    eprint("ERROR: libclang Python binding not available.")
    eprint("Install with: pip install libclang==18.1.1")
    sys.exit(1)

#
# Pipeline
#

def run_pipeline():
    """Run the libclang → IR → pybind11 → wrapper pipeline."""
    from lvgl_ir.clang_to_ir import ClangToIR
    from lvgl_codegen.pybind11_gen import Pybind11Generator
    from lvgl_codegen.python_wrapper import PythonWrapperGenerator

    # Step 1: Parse headers and build IR
    eprint("==> Step 1: Parsing LVGL headers with libclang...")
    converter = ClangToIR(module_name=module_name, module_prefix=module_prefix)

    # Build clang arguments
    extra_args = []
    for inc in args.include:
        extra_args.append("-I" + inc)
    for defn in args.define:
        extra_args.append("-D" + defn)

    # Find compile_commands.json automatically if not specified
    cc_path = args.compile_commands
    if not cc_path:
        # Try to find it relative to the include paths
        for inc in args.include:
            candidate = os.path.join(inc, '..', 'buildroot-build', 'compile_commands.json')
            if os.path.exists(candidate):
                cc_path = candidate
                eprint("    Found compile_commands.json: %s" % cc_path)
                break

    header_path = args.input[0]
    ir = converter.parse(
        header_path=header_path,
        compile_commands_path=cc_path,
        extra_args=extra_args,
    )

    eprint("==> IR built:")
    eprint("    Total functions: %d" % ir.total_c_functions)
    eprint("    Bound functions: %d (%.1f%%)" % (ir.total_bound_functions,
        ir.total_bound_functions / ir.total_c_functions * 100 if ir.total_c_functions > 0 else 0))
    eprint("    Skipped functions: %d" % ir.total_skipped_functions)
    eprint("    Enums: %d" % len(ir.enums))
    eprint("    Widgets: %d" % len(ir.widgets))
    eprint("    Properties: %d" % sum(len(w.properties) for w in ir.widgets.values()))

    # Step 2: Generate pybind11 C++ code
    eprint("==> Step 2: Generating pybind11 C++ code...")

    gen = Pybind11Generator(ir)
    cpp_code = gen.generate()
    eprint("    Generated %d lines of C++ code" % len(cpp_code.split('\n')))
    eprint("    Bound %d functions" % len(gen.generated_funcs))

    # Step 3: Generate Python wrapper
    eprint("==> Step 3: Generating Python wrapper...")
    wrapper_gen = PythonWrapperGenerator(ir)
    py_code = wrapper_gen.generate()
    eprint("    Generated %d lines of Python wrapper code" % len(py_code.split('\n')))

    # Step 4: Write output files
    if output_file:
        with open(output_file, "w") as f:
            f.write(cpp_code)
        eprint("==> Written C++ output to: %s" % output_file)
    else:
        print(cpp_code)

    if wrapper_output:
        with open(wrapper_output, "w") as f:
            f.write(py_code)
        eprint("==> Written Python wrapper to: %s" % wrapper_output)

    # Print summary
    eprint("\n=== Summary ===")
    eprint("  Objects: %d" % len(ir.widgets))
    eprint("  Enums: %d" % len(ir.enums))
    eprint("  Functions generated: %d / %d" % (len(gen.generated_funcs), ir.total_c_functions))
    eprint("  Functions not generated: %d" % ir.total_skipped_functions)

    # List not-generated functions
    skipped = ir.get_skipped_functions()
    if skipped:
        eprint("\n  Skipped functions (by reason):")
        reasons = {}
        for f in skipped:
            r = f.skip_reason or "unknown"
            reasons.setdefault(r, []).append(f.name)
        for reason, funcs in sorted(reasons.items()):
            eprint("    %s: %d functions" % (reason, len(funcs)))
            for fn in funcs[:5]:
                eprint("      - %s" % fn)
            if len(funcs) > 5:
                eprint("      ... and %d more" % (len(funcs) - 5))


#
# Main
#

run_pipeline()
