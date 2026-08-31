"""Run a setup.py with the wheel tags forced to a cross-compilation target.

setuptools' bdist_wheel takes the interpreter tag from the *running*
interpreter and then asserts the resulting (impl, abi, plat) triple is in
packaging.tags.sys_tags(). When buildroot's host CPython 3.13 cross-builds
python-k230 for CPython 3.12 that assertion always fails, even though the
extensions themselves are correct -- their SOABI comes from the target
sysconfigdata selected via _PYTHON_SYSCONFIGDATA_NAME, not from sys.version.

Overriding get_tag is the smallest intervention that keeps the rest of the
build (extension suffixes, wheel layout, RECORD) untouched.

Usage: cross_bdist_wheel.py <impl_tag> <abi_tag> <plat_tag> -- <setup.py args...>
"""

import runpy
import sys

try:
    from setuptools.command.bdist_wheel import bdist_wheel
except ImportError:  # setuptools < 70.1 delegates to the standalone wheel package
    from wheel.bdist_wheel import bdist_wheel

if len(sys.argv) < 5 or sys.argv[4] != "--":
    sys.exit(__doc__.strip().splitlines()[-1])

impl_tag, abi_tag, plat_tag = sys.argv[1:4]
bdist_wheel.get_tag = lambda self: (impl_tag, abi_tag, plat_tag)

sys.argv = ["setup.py"] + sys.argv[5:]
runpy.run_path("setup.py", run_name="__main__")
