cmake_minimum_required(VERSION 3.5)
set(CMAKE_CXX_STANDARD 17)
project(yolo)

message("CMAKE_TOOLCHAIN_FILE:" ${CMAKE_TOOLCHAIN_FILE})

# ========== 源文件自动收集 ==========
file(GLOB src CONFIGURE_DEPENDS src/*.cc)
set(bin yolo.elf)

# ========== 创建可执行文件 ==========
add_executable(${bin} ${src})

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

# ========== 添加头文件搜索路径 ==========
include_directories(
    ${usr_root}/buildroot-overlay/package/libmmz/
    ${usr_root}/include/nncase/include/
    ${usr_root}/include
    ${usr_root}/include/libdrm
    ${usr_root}/include/opencv4
    ${usr_root}/include/freetype2
    ${usr_root}/../../../../build/freetype-2.13.2/include
)

# ========== 添加链接库目录 ==========
target_link_directories(${bin} PRIVATE
    ${usr_root}/lib
    ${CMAKE_SYSROOT}
    ${CMAKE_SYSROOT}/usr/lib64/lp64d
)

# ========== 链接所需的库 ==========
# 使用 --start-group 和 --end-group 包裹，解决静态库相互依赖的链接问题
target_link_libraries(${bin}
    -Wl,--start-group
    Nncase.Runtime.Native
    nncase.rt_modules.k230
    functional_k230
    opencv_imgcodecs
    opencv_imgproc
    opencv_core
    sharpyuv
    libjpeg.so.9
    webp
    png
    z
    v4l2-drm
    drm
    display
    mmz
    pthread
    freetype
    -Wl,--end-group
)

# ========== 安装规则 ==========

if(CMAKE_TOOLCHAIN_FILE MATCHES "buildroot/toolchainfile.cmake")
    # Buildroot 环境
    message(STATUS "使用 Buildroot 编译，安装到 /root/app/yolo/")
    install(TARGETS ${bin} DESTINATION /root/app/yolo)
    install(DIRECTORY utils/ DESTINATION /root/app/yolo)
else()
    # 非 Buildroot，自己本地 build_app.sh 编译
    message(STATUS "本地编译，安装到 ./k230_bin/")
    install(TARGETS ${bin} DESTINATION bin)
endif()