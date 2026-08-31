# python-k230 wheel 的 Python ABI 问题

## 背景

`buildroot-overlay/package/python-k230` 由 buildroot 用它自带的 CPython 3.13 交叉编译，
产出 `k230_python-1.0.0-cp313-cp313-linux_riscv64.whl`。

`distribution.sh` 会把这个 wheel 解压进目标 rootfs 的 `dist-packages`：

| 发行版 | Python | 结果 |
| --- | --- | --- |
| debian13 | 3.13 | 可用 |
| ubuntu24 | 3.12 | **不可用** |

不可用的原因有两层：

1. 扩展文件名带 ABI 后缀 `*.cpython-313-riscv64-linux-gnu.so`，python3.12 的
   `FileFinder` 根本不会去找它。
2. 即使改名，pybind11 大量使用 CPython 内部结构，3.12 与 3.13 之间不是 ABI 兼容的，
   加载后会崩。

`Py_LIMITED_API` / abi3 这条路走不通：buildroot 自带的 pybind11 是 2.13.6，不支持
Limited API；而且代码里用到了 `py::dynamic_attr` 和 buffer protocol。

## 解决办法：额外交叉编译一份 cp312 wheel

`buildroot-overlay/board/canaan/k230-soc/distribution/build_k230_wheel_py312.sh` 用
**同一套玄铁工具链**和**同一份 staging 库**重新编译扩展，只把 CPython 头文件换成
Ubuntu 24.04 riscv64 的 python3.12。

```bash
make buildroot
buildroot-overlay/board/canaan/k230-soc/distribution/build_k230_wheel_py312.sh output/k230_canmv_01studio_defconfig
# -> output/.../build/python-k230-1.0.0/dist/k230_python-1.0.0-cp312-cp312-linux_riscv64.whl
```

`distribution.sh` 会读取目标 rootfs 里的 Python 版本自动挑选对应的 wheel；
做 ubuntu 镜像时若 cp312 wheel 不存在，会自动调用上面的脚本。所以正常情况下
直接 `sudo make ubuntu` 即可，不需要手工执行。

### 为什么可以直接换头文件

产出的 `.so` 最高只需要 `GLIBC_2.32` / `GLIBCXX_3.4.29`，Ubuntu 24.04 是
glibc 2.39 + GCC 13（GLIBCXX_3.4.32），完全覆盖。四个扩展引用的全部 CPython
符号也都在 Ubuntu 的 libpython3.12 里有定义。

### 为什么不能在设备上直接编译

Ubuntu 自带的 gcc-13 不认识 `-mcpu=c908v`、`-mrvv-v0p10-compatible` 这些玄铁扩展
选项（见 `setup.py` 的 `extra_compile_args`），必须继续用交叉工具链。

## 实现要点

脚本里几处不太直观的地方：

- **`_PYTHON_SYSCONFIGDATA_NAME`**：指向从 `libpython3.12-stdlib` 里取出的
  `_sysconfigdata__linux_riscv64-linux-gnu.py`。这样宿主的 3.13 解释器会报告目标的
  `SOABI`，`build_ext` 才会生成 `.cpython-312-riscv64-linux-gnu.so`。
  它被单独放在一个目录里，避免 3.12 stdlib 的其它模块混进 `sys.path`。

- **改写 `LIBDIR`**：distutils 会把 sysconfigdata 里的 `LIBDIR` 原样加进
  `library_dirs`，Ubuntu 的 `/usr/lib/riscv64-linux-gnu` 会被 buildroot 的编译器
  wrapper 判定为 unsafe cross-compilation path 而报错，所以改指向 staging。

- **`buildroot-overlay/board/canaan/k230-soc/distribution/cross_bdist_wheel.py`**：setuptools 的
  `bdist_wheel` 从**运行中**的解释器取 interpreter tag，然后断言结果在
  `packaging.tags.sys_tags()` 里。宿主是 3.13 时这个断言必然失败（哪怕扩展本身完全正确），
  所以覆盖掉 `get_tag`。

- **`--build-base build-py312`**：setuptools 的 `build/` 子目录名来自运行中的解释器
  (`cpython-313`)，两个 ABI 会共用同一个目录。分开放，避免覆盖 buildroot 已经编好的
  cp313 目标文件。

- 用的是 `output/<conf>/host/bin/riscv64-*-gcc`（buildroot 的 wrapper），不是
  `/opt/toolchain` 下的原始编译器 —— wrapper 会补上 `--sysroot` 和正确的
  `-march`/`-mabi`。

### 目录布局

脚本会创建或使用以下目录（以 `output/k230_canmv_01studio_defconfig` 为例）：

| 目录 | 说明 | 持久化 |
| --- | --- | --- |
| `dl/python3.12-riscv64/` | 从 Ubuntu ports 下载的 python3.12 riscv64 deb 包缓存 | ✅ |
| `output/.../build/python312-riscv64-sysroot/` | `dpkg -x` 解压后的 python3.12 头文件和库 | ✅ |
| `output/.../build/python312-riscv64-sysroot/_cross_sysconfig/` | 改写 `LIBDIR` 后的 sysconfigdata 模块 | ✅ |
| `output/.../build/python-k230-1.0.0/build-py312/` | cp312 专用的构建临时目录（与 buildroot 的 `build/` 隔离） | ✅ |
| `output/.../build/python-k230-1.0.0/dist/` | 最终 wheel 输出目录（cp312 与 cp313 共存） | ✅ |

所有构建产物都放在 `output/` 下方，不需要额外临时目录（调试时用的 `/tmp/py312*` 等不是脚本的一部分）。

## 升级 Ubuntu 版本时

改 `buildroot-overlay/board/canaan/k230-soc/distribution/build_k230_wheel_py312.sh`
顶部的 `PY312_DEB_VERSION`（或用环境变量覆盖）。
如果新的发行版换成了 python3.13，`distribution.sh` 会自动选 buildroot 产出的 cp313
wheel，这个脚本就不再需要了。
