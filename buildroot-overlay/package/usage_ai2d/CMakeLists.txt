cmake_minimum_required(VERSION 3.5)
set(CMAKE_CXX_STANDARD 17)
project(usage_ai2d)

message("CMAKE_TOOLCHAIN_FILE:" ${CMAKE_TOOLCHAIN_FILE})

# ========== 设置交叉编译工具链路径 ==========
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}   -mcpu=c908v -mabi=lp64d  -mtune=c908 -mrvv-v0p10-compatible  -mrvv-auto-vectorize")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}   -mcpu=c908v -mabi=lp64d  -mtune=c908 -mrvv-v0p10-compatible  -mrvv-auto-vectorize")

if(CMAKE_TOOLCHAIN_FILE MATCHES "buildroot/toolchainfile.cmake")
    # buildroot环境编译，默认编译器为buildroot的编译器
else()
    # shell脚本编译，设置编译器路径
    set(CMAKE_C_COMPILER $ENV{GCC_PATH}/riscv64-unknown-linux-gnu-gcc)
    set(CMAKE_CXX_COMPILER $ENV{GCC_PATH}/riscv64-unknown-linux-gnu-g++)
    set(CMAKE_SYSROOT $ENV{sysroot})
endif()

message("CMAKE_C_COMPILER:" ${CMAKE_C_COMPILER})
message("CMAKE_CXX_COMPILER:" ${CMAKE_CXX_COMPILER})
message("CMAKE_SYSROOT:" ${CMAKE_SYSROOT})

# ========== 设置 SDK 路径和 sysroot ==========
set(usr_root ${CMAKE_SYSROOT}/usr)


add_subdirectory(test_affine)
add_subdirectory(test_crop)
add_subdirectory(test_pad)
add_subdirectory(test_resize)
add_subdirectory(test_shift)


# ========== 安装规则 ==========

if(CMAKE_TOOLCHAIN_FILE MATCHES "buildroot/toolchainfile.cmake")
    # Buildroot 环境
    message(STATUS "使用 Buildroot 编译，安装到 /root/app/usage_ai2d/")
    install(DIRECTORY utils/ DESTINATION /root/app/usage_ai2d)
else()
    # 非 Buildroot，自己本地 build_app.sh 编译
    message(STATUS "本地编译，安装到 ./k230_bin/")
    install(TARGETS ${bin} DESTINATION bin)
endif()