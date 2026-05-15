# cmake_minimum_required(VERSION 3.10)
# project(helloWorld_cmake)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}   -mcpu=c908v -mabi=lp64d  -mtune=c908 -mrvv-v0p10-compatible  -mrvv-auto-vectorize")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}   -mcpu=c908v -mabi=lp64d  -mtune=c908 -mrvv-v0p10-compatible  -mrvv-auto-vectorize")


set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -rdynamic")
add_executable(i2c  i2c.c)
target_link_libraries(i2c PRIVATE m)
install(TARGETS i2c  DESTINATION /root/app/helloworld/i2c/)




# add_executable(i2c1  i2c1.c)
# target_link_libraries(i2c1 PRIVATE m pthread)
# install(TARGETS i2c1  DESTINATION /root/app/helloworld/i2c/)
