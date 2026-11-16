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
# Usage: buildroot-overlay/board/canaan/k230-soc/distribution/cp312-support/build_k230_wheel_py312_autover.sh <BRW_BUILD_DIR>
#        e.g. buildroot-overlay/board/canaan/k230-soc/distribution/cp312-support/build_k230_wheel_py312_autover.sh output/k230_canmv_01studio_defconfig
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
#
# Ubuntu removes superseded package versions from the pool, so a hard-coded
# version eventually 404s (wget exit 8). This script therefore resolves the
# version at run time:
#   - PY312_DEB_VERSION set by the caller -> used as-is (strict pin, no fallback)
#   - otherwise PY312_DEB_PREFERRED is tried first, then the newest version of
#     the PY312_DEB_SERIES series that is available for all packages (online
#     pool listing, or the local download cache when offline).
PY312_DEB_VERSION_PINNED="${PY312_DEB_VERSION:-}"
PY312_DEB_PREFERRED="${PY312_DEB_PREFERRED:-3.12.3-1ubuntu0.17}"
# Stay on noble's 3.12.3 build; other 3.12.x uploads in the pool belong to
# other Ubuntu releases.
PY312_DEB_SERIES="${PY312_DEB_SERIES:-3.12.3-1}"
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

# ------------------------------------------------------------ resolve version
mkdir -p "${DL_DIR}"
# Drop leftovers of earlier failed downloads (e.g. empty .tmp files from a 404).
rm -f "${DL_DIR}"/*.deb.tmp

# True if every package of version $1 is already in the download cache.
cached_all() {
    local p
    for p in ${PY312_DEBS}; do
        [ -s "${DL_DIR}/${p}_${1}_riscv64.deb" ] || return 1
    done
}

# True if every package of version $1 appears in the pool listing $2.
listed_all() {
    local p
    for p in ${PY312_DEBS}; do
        grep -qxF "${p}_${1}_riscv64.deb" <<<"$2" || return 1
    done
}

# Newest version of the series for which check function $1 (with extra arg $2)
# succeeds, picked from the candidate list on stdin.
newest_matching() {
    local v
    sort -Vru | while read -r v; do
        if [ -n "${v}" ] && "$1" "${v}" "$2"; then
            echo "${v}"
            break
        fi
    done
    return 0
}

if [ -n "${PY312_DEB_VERSION_PINNED}" ]; then
    PY312_DEB_VERSION="${PY312_DEB_VERSION_PINNED}"
    print_blue "Using pinned python3.12 version ${PY312_DEB_VERSION}"
elif cached_all "${PY312_DEB_PREFERRED}"; then
    PY312_DEB_VERSION="${PY312_DEB_PREFERRED}"
else
    print_blue "Querying ${PY312_DEB_BASE}/ for available python3.12 versions..."
    POOL_LIST="$(wget -q -O - "${PY312_DEB_BASE}/" 2>/dev/null \
        | grep -o '[A-Za-z0-9.+_-]*_riscv64\.deb' | sort -u || true)"
    PY312_DEB_VERSION=""
    if [ -n "${POOL_LIST}" ]; then
        if listed_all "${PY312_DEB_PREFERRED}" "${POOL_LIST}"; then
            PY312_DEB_VERSION="${PY312_DEB_PREFERRED}"
        else
            PY312_DEB_VERSION="$(sed -n "s/^python3\.12-dev_\(${PY312_DEB_SERIES//./\\.}[^_]*\)_riscv64\.deb$/\1/p" <<<"${POOL_LIST}" \
                | newest_matching listed_all "${POOL_LIST}")"
        fi
    else
        # Offline: fall back to the newest complete set in the local cache.
        print_blue "Pool listing unavailable, looking in ${DL_DIR}..."
        PY312_DEB_VERSION="$(ls "${DL_DIR}" 2>/dev/null \
            | sed -n "s/^python3\.12-dev_\(${PY312_DEB_SERIES//./\\.}[^_]*\)_riscv64\.deb$/\1/p" \
            | newest_matching cached_all)"
    fi
    if [ -z "${PY312_DEB_VERSION}" ]; then
        print_red "Error: no python3.12 ${PY312_DEB_SERIES}* riscv64 version with all of"
        print_red "       [${PY312_DEBS}] found at ${PY312_DEB_BASE}/ or in ${DL_DIR}"
        print_red "       Set PY312_DEB_VERSION=<version> to pin one manually."
        exit 1
    fi
    if [ "${PY312_DEB_VERSION}" != "${PY312_DEB_PREFERRED}" ]; then
        print_blue "Preferred ${PY312_DEB_PREFERRED} not available, using ${PY312_DEB_VERSION}"
    fi
fi
print_blue "python3.12 riscv64 package version: ${PY312_DEB_VERSION}"

# ---------------------------------------------------------------- fetch headers
for p in ${PY312_DEBS}; do
    deb="${p}_${PY312_DEB_VERSION}_riscv64.deb"
    if [ ! -s "${DL_DIR}/${deb}" ]; then
        print_blue "Downloading ${deb}..."
        wget -q --show-progress -O "${DL_DIR}/${deb}.tmp" "${PY312_DEB_BASE}/${deb}" || {
            rc=$?
            rm -f "${DL_DIR}/${deb}.tmp"
            print_red "Error: download failed (wget exit ${rc}): ${PY312_DEB_BASE}/${deb}"
            exit "${rc}"
        }
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
