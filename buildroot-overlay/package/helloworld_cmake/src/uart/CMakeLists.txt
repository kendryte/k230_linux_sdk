# cmake_minimum_required(VERSION 3.10)
# project(helloWorld_cmake)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}   -mcpu=c908v -mabi=lp64d  -mtune=c908 -mrvv-v0p10-compatible  -mrvv-auto-vectorize")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}   -mcpu=c908v -mabi=lp64d  -mtune=c908 -mrvv-v0p10-compatible  -mrvv-auto-vectorize")


set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -rdynamic")
add_executable(uart  uart.c)
target_link_libraries(uart PRIVATE m)
install(TARGETS uart  DESTINATION /root/app/helloworld/uart/)




add_executable(uart1  uart1.c)
target_link_libraries(uart1 PRIVATE m pthread)
install(TARGETS uart1  DESTINATION /root/app/helloworld/uart/)
