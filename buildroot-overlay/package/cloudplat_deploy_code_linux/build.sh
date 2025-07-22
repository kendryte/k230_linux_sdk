#!/bin/bash
SDK_ROOT=$(cd "$(dirname "${BASH_SOURCE[0]}")/../../../" &> /dev/null && pwd)
GCC_PATH="$(cat ${SDK_ROOT}/output/$(cat ${SDK_ROOT}/.last_conf | cut -d= -f2)/.config | grep BR2_TOOLCHAIN_EXTERNAL_PATH | cut -d\" -f2)"/bin
echo ${GCC_PATH}
echo "========================="
sysroot="$(realpath ${SDK_ROOT}/output/$(cat ${SDK_ROOT}/.last_conf | cut -d= -f2)/staging)"
export GCC_PATH
export sysroot

# 获取传入的目标目录（如果有）
TARGET_DIR=$1

# 清理构建目录
rm -rf out
mkdir out

k230_bin=`pwd`/k230_bin
rm -rf ${k230_bin}
mkdir -p ${k230_bin}

pushd out

# 根据是否传入参数设置构建目标
if [ -n "${TARGET_DIR}" ]; then
    echo "构建指定目录: ${TARGET_DIR}"
    cmake -DCMAKE_BUILD_TYPE=Release \
          -DCMAKE_INSTALL_PREFIX=`pwd` \
          -DCMAKE_C_COMPILER=${GCC_PATH}/riscv64-unknown-linux-gnu-gcc \
          -DCMAKE_CXX_COMPILER=${GCC_PATH}/riscv64-unknown-linux-gnu-g++ \
          -DTARGET_DIR=${TARGET_DIR} \
          ..
else
    echo "构建所有模块"
    cmake -DCMAKE_BUILD_TYPE=Release \
          -DCMAKE_INSTALL_PREFIX=`pwd` \
          -DCMAKE_C_COMPILER=${GCC_PATH}/riscv64-unknown-linux-gnu-gcc \
          -DCMAKE_CXX_COMPILER=${GCC_PATH}/riscv64-unknown-linux-gnu-g++ \
          ..
fi

make -j && make install
popd

# 拷贝结果
cp out/bin/*.elf ${k230_bin} 2>/dev/null
cp utils/* ${k230_bin} 2>/dev/null

rm -rf out