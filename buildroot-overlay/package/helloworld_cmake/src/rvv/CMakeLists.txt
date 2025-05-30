# cmake_minimum_required(VERSION 3.10)
# project(helloWorld_cmake)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}   -mcpu=c908v -mabi=lp64d  -mtune=c908 -mrvv-v0p10-compatible  -mrvv-auto-vectorize")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}   -mcpu=c908v -mabi=lp64d  -mtune=c908 -mrvv-v0p10-compatible  -mrvv-auto-vectorize")


set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -rdynamic")
add_executable(rvv_test  rvv_test.cpp)
target_link_libraries(rvv_test PRIVATE m)
install(TARGETS rvv_test  DESTINATION /root/app/helloworld/rvv)




add_executable(vsetvl_e32m2  vsetvl_e32m2.cpp)
install(TARGETS vsetvl_e32m2  DESTINATION /root/app/helloworld/rvv)
