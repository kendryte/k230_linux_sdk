from setuptools import setup, Extension, find_packages
import pybind11

ext = Extension(
    "k230_ext",
    sources=["k230_c_ext/_ext.c"],
)

k230_display = Extension(
    "k230_display",
    sources=["k230_display/k230_display_pybind.cpp","k230_display/k230_display_pybind_display_lib.cpp"],
    include_dirs=[pybind11.get_include()],
    libraries=["display", "drm", "opencv_core", "opencv_imgproc"],
    extra_compile_args=["-O3", "-mcpu=c908v", "-mabi=lp64d", "-mtune=c908", "-mrvv-v0p10-compatible", "-mrvv-auto-vectorize", "-std=c++17"],
    language='c++',
)

setup(
    name="k230-python",
    version="1.0.0",
    author="wangjianxin",
    description="Python for k230",
    packages=find_packages(),
    ext_modules=[ext, k230_display],
    zip_safe=False,
    entry_points={
        "console_scripts": [
            "k230-iomux=k230.iomux:main",
        ],
    },
)
