# ========== 源文件自动收集 ==========
file(GLOB src CONFIGURE_DEPENDS *.cc)
set(bin test_shift.elf)

# ========== 创建可执行文件 ==========
add_executable(${bin} ${src})

# ========== 添加头文件搜索路径 ==========
include_directories(
    ${usr_root}/buildroot-overlay/package/libmmz/
    ${usr_root}/include/nncase/include/
    ${usr_root}/include
    ${usr_root}/include/libdrm
    ${usr_root}/include/opencv4
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
    -Wl,--end-group
)

# ========== 安装规则 ==========

if(CMAKE_TOOLCHAIN_FILE MATCHES "buildroot/toolchainfile.cmake")
    # Buildroot 环境
    message(STATUS "使用 Buildroot 编译，安装到 /root/app/usage_ai2d/")
    install(TARGETS ${bin} DESTINATION /root/app/usage_ai2d/)
else()
    # 非 Buildroot，自己本地 build_app.sh 编译
    message(STATUS "本地编译，安装到 ./k230_bin/")
    install(TARGETS ${bin} DESTINATION bin)
endif()