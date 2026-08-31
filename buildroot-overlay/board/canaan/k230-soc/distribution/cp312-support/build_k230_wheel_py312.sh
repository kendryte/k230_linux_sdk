#!/bin/bash
# Cross-build the python-k230 wheel for CPython 3.12.
#
# Buildroot builds python-k230 against its own CPython 3.13, producing
# k230_python-*-cp313-cp313-linux_riscv64.whl. That wheel works on the debian13
# rootfs (python3.13) but not on ubuntu24 (python3.12): the extension .so files
# carry a cpython-313 suffix and pybind11 relies on CPython internals that are
# not ABI-compatible across 3.12/3.13.
#
# This script produces a second, cp312-tagged wheel from the same sources, using
# the same Xuantie cross toolchain and the same staging libraries. Only the
# CPython headers differ: they come from Ubuntu 24.04's riscv64 python3.12-dev
# packages. The resulting .so files need at most GLIBC_2.32 / GLIBCXX_3.4.29,
# which Ubuntu 24.04 (glibc 2.39, GCC 13) satisfies.
#
# Usage: buildroot-overlay/board/canaan/k230-soc/distribution/cp312-support/build_k230_wheel_py312.sh <BRW_BUILD_DIR>
#        e.g. buildroot-overlay/board/canaan/k230-soc/distribution/cp312-support/build_k230_wheel_py312.sh output/k230_canmv_01studio_defconfig
#
# Output: <BRW_BUILD_DIR>/build/python-k230-1.0.0/dist/
#             k230_python-1.0.0-cp312-cp312-linux_riscv64.whl

set -e

BRW_BUILD_DIR="$1"
if [ -z "${BRW_BUILD_DIR}" ]; then
    echo "Usage: $0 <BRW_BUILD_DIR>" >&2
    exit 1
fi
BRW_BUILD_DIR="$(cd "${BRW_BUILD_DIR}" && pwd)"
K230_SDK_ROOT="$(cd "$(dirname "$0")/../../../../../../" && pwd)"
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"


BLUE="\033[1;34;40m"
RED="\033[1;31;40m"
GREEN="\033[1;32;40m"
NONE="\033[0m"
print_blue()  { echo -e "${BLUE}$*${NONE}"; }
print_red()   { echo -e "${RED}$*${NONE}"; }
print_green() { echo -e "${GREEN}$*${NONE}"; }

# Ubuntu 24.04 (noble) riscv64 python3.12 packages.
PY312_DEB_VERSION="${PY312_DEB_VERSION:-3.12.3-1ubuntu0.16}"
PY312_DEB_BASE="${PY312_DEB_BASE:-https://ports.ubuntu.com/ubuntu-ports/pool/main/p/python3.12}"
PY312_DEBS="libpython3.12-dev python3.12-dev libpython3.12-stdlib libpython3.12-minimal"

STAGING_DIR="${BRW_BUILD_DIR}/staging"
HOST_DIR="${BRW_BUILD_DIR}/host"
PKG_BUILD_DIR="${BRW_BUILD_DIR}/build/python-k230-1.0.0"
PY312_SYSROOT="${BRW_BUILD_DIR}/build/python312-riscv64-sysroot"
DL_DIR="${K230_SDK_ROOT}/dl/python3.12-riscv64"

for d in "${STAGING_DIR}" "${HOST_DIR}" "${PKG_BUILD_DIR}"; do
    if [ ! -d "${d}" ]; then
        print_red "Error: ${d} not found. Build buildroot first: make buildroot"
        exit 1
    fi
done

HOSTPY="${HOST_DIR}/bin/python3"
# Buildroot's toolchain wrapper (adds --sysroot and the correct -march/-mabi).
# The raw Xuantie binaries would link against the host's /usr/lib.
CROSS_GCC="$(ls "${HOST_DIR}"/bin/*-linux-gnu-gcc 2>/dev/null | head -1)"
CROSS_GXX="$(ls "${HOST_DIR}"/bin/*-linux-gnu-g++ 2>/dev/null | head -1)"
CROSS_AR="$(ls "${HOST_DIR}"/bin/*-linux-gnu-ar 2>/dev/null | head -1)"
if [ -z "${CROSS_GCC}" ] || [ -z "${CROSS_GXX}" ]; then
    print_red "Error: cross toolchain wrapper not found under ${HOST_DIR}/bin"
    exit 1
fi

# ---------------------------------------------------------------- fetch headers
mkdir -p "${DL_DIR}"
for p in ${PY312_DEBS}; do
    deb="${p}_${PY312_DEB_VERSION}_riscv64.deb"
    if [ ! -f "${DL_DIR}/${deb}" ]; then
        print_blue "Downloading ${deb}..."
        wget -q --show-progress -O "${DL_DIR}/${deb}.tmp" "${PY312_DEB_BASE}/${deb}"
        mv "${DL_DIR}/${deb}.tmp" "${DL_DIR}/${deb}"
    fi
done

print_blue "Extracting python3.12 riscv64 sysroot..."
rm -rf "${PY312_SYSROOT}"
mkdir -p "${PY312_SYSROOT}"
for p in ${PY312_DEBS}; do
    dpkg -x "${DL_DIR}/${p}_${PY312_DEB_VERSION}_riscv64.deb" "${PY312_SYSROOT}"
done

PY312_INC="${PY312_SYSROOT}/usr/include/python3.12"
if [ ! -f "${PY312_INC}/Python.h" ]; then
    print_red "Error: ${PY312_INC}/Python.h missing after extraction"
    exit 1
fi

# ------------------------------------------------------- cross sysconfig module
# Setting _PYTHON_SYSCONFIGDATA_NAME makes the host interpreter report the
# target's SOABI, so build_ext names the extensions .cpython-312-riscv64-*.so
# and bdist_wheel tags the wheel cp312. It is kept in its own directory so that
# nothing else from the 3.12 stdlib ends up on sys.path.
#
# LIBDIR is rewritten because distutils appends it to library_dirs verbatim;
# Ubuntu's /usr/lib/riscv64-linux-gnu is rejected by buildroot's compiler
# wrapper as an unsafe cross-compilation path.
SYSCFG_NAME="_sysconfigdata__linux_riscv64-linux-gnu"
SYSCFG_DIR="${PY312_SYSROOT}/_cross_sysconfig"
mkdir -p "${SYSCFG_DIR}"
sed "s|'LIBDIR': '/usr/lib/riscv64-linux-gnu'|'LIBDIR': '${STAGING_DIR}/usr/lib'|" \
    "${PY312_SYSROOT}/usr/lib/python3.12/${SYSCFG_NAME}.py" > "${SYSCFG_DIR}/${SYSCFG_NAME}.py"
grep -q "'LIBDIR': '${STAGING_DIR}/usr/lib'" "${SYSCFG_DIR}/${SYSCFG_NAME}.py" || {
    print_red "Error: failed to patch LIBDIR in ${SYSCFG_NAME}.py"
    exit 1
}

# ---------------------------------------------------------------------- flags
# Keep in sync with buildroot-overlay/package/python-k230/python-k230.mk
PYBIND11_INC="$("${HOSTPY}" -c 'import pybind11; print(pybind11.get_include())')"
TARGET_SITE_PACKAGES="$(ls -d "${STAGING_DIR}"/usr/lib/python3.*/site-packages | head -1)"

INCLUDES="-I${PY312_INC} -I${PY312_SYSROOT}/usr/include"
INCLUDES="${INCLUDES} -I${STAGING_DIR}/usr/include"
INCLUDES="${INCLUDES} -I${STAGING_DIR}/usr/include/drm"
INCLUDES="${INCLUDES} -I${STAGING_DIR}/usr/include/lvgl"
INCLUDES="${INCLUDES} -I${STAGING_DIR}/usr/include/lvgl_private"
INCLUDES="${INCLUDES} -I${TARGET_SITE_PACKAGES}/numpy/core/include"
INCLUDES="${INCLUDES} -I${PYBIND11_INC}"
INCLUDES="${INCLUDES} -I${STAGING_DIR}/usr/include/opencv4"
INCLUDES="${INCLUDES} -I${STAGING_DIR}/usr/include/libv4l2-drm"

# --------------------------------------------------------------------- build
# A separate --build-base keeps the cp313 objects produced by buildroot intact:
# the build/ subdirectory names are derived from the *running* interpreter
# (3.13) and would otherwise be shared between the two ABIs.
print_blue "Cross-building cp312 wheel..."
cd "${PKG_BUILD_DIR}"
env \
    _PYTHON_SYSCONFIGDATA_NAME="${SYSCFG_NAME}" \
    PYTHONPATH="${SYSCFG_DIR}" \
    _PYTHON_HOST_PLATFORM="linux-riscv64" \
    STAGING_DIR="${STAGING_DIR}" \
    CC="${CROSS_GCC}" \
    CXX="${CROSS_GXX}" \
    LDSHARED="${CROSS_GCC} -shared" \
    LDCXXSHARED="${CROSS_GXX} -shared" \
    AR="${CROSS_AR}" \
    CFLAGS="${INCLUDES}" \
    CXXFLAGS="${INCLUDES}" \
    LDFLAGS="-L${STAGING_DIR}/usr/lib" \
    "${HOSTPY}" "${SCRIPT_DIR}/cross_bdist_wheel.py" cp312 cp312 linux_riscv64 -- \
        build --build-base build-py312 \
        bdist_wheel --plat-name linux_riscv64

WHEEL="${PKG_BUILD_DIR}/dist/k230_python-1.0.0-cp312-cp312-linux_riscv64.whl"
if [ ! -f "${WHEEL}" ]; then
    print_red "Error: expected wheel not produced: ${WHEEL}"
    ls -la "${PKG_BUILD_DIR}/dist/" || true
    exit 1
fi
print_green "Build successful: ${WHEEL}"
