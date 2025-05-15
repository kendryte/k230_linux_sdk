cmake_minimum_required(VERSION 3.10)
project(helloWorld_cmake)

#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0 -funwind-tables -fsanitize=address")
#need copy output/k230_canmv_01studio_defconfig/host/riscv64-buildroot-linux-gnu/sysroot/lib/libasan.so.8;
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0 -funwind-tables")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -rdynamic")
add_executable(helloWorld_cmake main.c )

install(TARGETS helloWorld_cmake DESTINATION /root/helloworld/)

add_subdirectory(opencv_test)
add_subdirectory(lsfile)
add_subdirectory(rvv)
