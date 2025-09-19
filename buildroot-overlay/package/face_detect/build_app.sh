#!/bin/bash
SDK_ROOT=$(cd "$(dirname "${BASH_SOURCE[0]}")/../../../" &> /dev/null && pwd)
GCC_PATH="$(cat ${SDK_ROOT}/output/$(cat ${SDK_ROOT}/.last_conf | cut -d= -f2)/.config | grep BR2_TOOLCHAIN_EXTERNAL_PATH | cut -d\" -f2)"/bin
echo ${GCC_PATH}
echo "========================="
sysroot="$(realpath ${SDK_ROOT}/output/$(cat ${SDK_ROOT}/.last_conf | cut -d= -f2)/staging)"
export GCC_PATH
export sysroot

# clear
rm -rf out
mkdir out

k230_bin=`pwd`/k230_bin
rm -rf ${k230_bin}
mkdir -p ${k230_bin}

pushd out
cmake -DCMAKE_BUILD_TYPE=Release                \
      -DCMAKE_INSTALL_PREFIX=`pwd`               \
      -DCMAKE_C_COMPILER=${GCC_PATH}/riscv64-unknown-linux-gnu-gcc \
      -DCMAKE_CXX_COMPILER=${GCC_PATH}/riscv64-unknown-linux-gnu-g++ \
      ..
make -j && make install
popd

if [ -f out/bin/face_detect.elf ]; then
      cp out/bin/face_detect.elf ${k230_bin}
fi

cp -r utils/* ${k230_bin}

rm -rf out
