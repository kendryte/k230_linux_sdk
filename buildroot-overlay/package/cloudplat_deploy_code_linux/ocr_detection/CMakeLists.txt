
set(common_files "../common_files")
include_directories(${common_files})

set(common_files_src 
    ${common_files}/ai_base.cc
    ${common_files}/ai_utils.cc
    ${common_files}/sensor_buf_manager.cc
    ${common_files}/json.cpp
    ${common_files}/parse_args.cc
)

set(src main.cc ocr_box.cc clipper.cpp)
set(all_src ${src} ${common_files_src})

set(bin ocr_detection.elf)

add_executable(${bin} ${all_src})

# ========== 添加头文件搜索路径 ==========
include_directories(
    ${usr_root}/buildroot-overlay/package/libmmz/
    ${usr_root}/include/nncase/include/
    ${usr_root}/include
    ${usr_root}/include/libdrm
    ${usr_root}/include/opencv4
    ${sdk_root}/output/${CONF_VALUE}/build/freetype-2.13.2/include
)

# ========== 添加链接库目录 ==========
target_link_directories(${bin} PRIVATE
    ${usr_root}/lib
    /opt/toolchain/Xuantie-900-gcc-linux-6.6.0-glibc-x86_64-V3.0.2/sysroot/usr/lib64/lp64d
    /opt/toolchain/Xuantie-900-gcc-linux-6.6.0-glibc-x86_64-V3.0.2/sysroot/
)

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

install(TARGETS ${bin} DESTINATION bin)
