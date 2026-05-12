from setuptools import setup, Extension, find_packages

ext = Extension(
    "k230_ext",
    sources=["k230_c_ext/_ext.c"],
)

setup(
    name="k230-python-demo",
    version="1.0.0",
    author="wangjianxin",
    description="Python test for k230",
    packages=find_packages(),
    ext_modules=[ext],
    zip_safe=False,
    entry_points={
        "console_scripts": [
            "k230-iomux=k230.iomux:main",
        ],
    },
)
