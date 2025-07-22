cmake_minimum_required(VERSION 3.5)
set(CMAKE_CXX_STANDARD 17)
project(cloudplat_deploy_code_linux)

# ========== 获取 Buildroot 当前配置名 ==========
# 读取上层目录中 .last_conf 文件，提取 CONF=xxx 的配置名
file(READ "../../../.last_conf" LAST_CONF_CONTENT)
string(REGEX MATCH "CONF=([^\n]+)" _ "${LAST_CONF_CONTENT}")
if (CMAKE_MATCH_1)
    set(CONF_VALUE "${CMAKE_MATCH_1}")
    message(STATUS "当前构建配置: ${CONF_VALUE}")
else()
    message(FATAL_ERROR "未能在 .last_conf 中找到 CONF 配置")
endif()

# ========== 设置交叉编译工具链路径 ==========
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}   -mcpu=c908v -mabi=lp64d  -mtune=c908 -mrvv-v0p10-compatible  -mrvv-auto-vectorize")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}   -mcpu=c908v -mabi=lp64d  -mtune=c908 -mrvv-v0p10-compatible  -mrvv-auto-vectorize")
# 设置编译器路径
set(CMAKE_C_COMPILER $ENV{GCC_PATH}/riscv64-unknown-linux-gnu-gcc)
set(CMAKE_SYSROOT $ENV{sysroot})

# ========== 设置 SDK 路径和 sysroot ==========
set(sdk_root "${PROJECT_SOURCE_DIR}/../../../")
set(usr_root "${sdk_root}/output/${CONF_VALUE}/host/riscv64-buildroot-linux-gnu/sysroot/usr")

# ========== 条件添加子目录 ==========
# 如果没有传入 TARGET_DIR，就添加所有模块
if(NOT DEFINED TARGET_DIR OR TARGET_DIR STREQUAL "")
    message(STATUS "未指定 TARGET_DIR，构建全部模块")
    add_subdirectory(classification)
    add_subdirectory(detection)
    add_subdirectory(segmentation)
    add_subdirectory(ocr_detection)
    add_subdirectory(ocr_recognition)
    add_subdirectory(ocr)
    add_subdirectory(metric_learning)
    add_subdirectory(multilabel_cls)
else()
    message(STATUS "仅构建指定模块: ${TARGET_DIR}")
    add_subdirectory(${TARGET_DIR})
endif()
