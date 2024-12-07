cmake_minimum_required(VERSION 3.5)
set(CMAKE_CXX_STANDARD 17)
project(ai_demo)

add_definitions(-DWITH_OPENCV)
if (NOT DEFINED BR2_PRIMARY_SITE)
    set(BR2_PRIMARY_SITE "https://ai.b-bug.org/~/wangjianxin/dl/")
endif()
# 设置编译器路径
set(CMAKE_C_COMPILER /opt/toolchain/Xuantie-900-gcc-linux-6.6.0-glibc-x86_64-V2.10.2/bin/riscv64-unknown-linux-gnu-gcc)

set(sdk_root "${PROJECT_SOURCE_DIR}/../../../")
set(usr_root "${sdk_root}/output/k230_canmv_defconfig/host/riscv64-buildroot-linux-gnu/sysroot/usr")

include_directories(${usr_root}/buildroot-overlay/package/libmmz/)
include_directories(${usr_root}/include/nncase/include/)
include_directories(${usr_root}/include)
include_directories(${usr_root}/include/libdrm)
include_directories(${usr_root}/include/opencv4)
include_directories(${sdk_root}/output/k230_canmv_defconfig/build/freetype-2.13.2/include)
include_directories(./common)

link_directories(${usr_root}/lib/)

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -L/opt/toolchain/Xuantie-900-gcc-linux-6.6.0-glibc-x86_64-V2.10.2/sysroot/usr/lib64/lp64d -L/opt/toolchain/Xuantie-900-gcc-linux-6.6.0-glibc-x86_64-V2.10.2/sysroot/")

# 定义要排除的目录列表
set(exclude_dirs "cmake" "k230_bin" "shell" "out" "llamac" ".git" "common" "kmodel" "face_detect")

file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/*")
foreach(file ${files})
    # 判断当前文件是否是一个目录
    if(IS_DIRECTORY ${file})
        # 获取目录名称
        get_filename_component(dir_name ${file} NAME)
        # 检查目录是否在排除列表中
        list(FIND exclude_dirs ${dir_name} is_exclude_dir)
        # 如果目录不在排除列表中，则处理该目录
        if(${is_exclude_dir} EQUAL -1)
            # 处理该目录，例如添加到项目中
            if(${dir_name})
                add_subdirectory(${dir_name}) 
            endif()
        endif()
    endif()
endforeach()
