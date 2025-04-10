# cmake_minimum_required(VERSION 3.10)
# project(helloWorld_cmake)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}  -march=rv64gcvxthead")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -rdynamic")
add_executable(rvv_test  rvv_test.cpp)
target_link_libraries(rvv_test PRIVATE m)
install(TARGETS rvv_test  DESTINATION /root/helloworld/rvv)




add_executable(vsetvl_e32m2  vsetvl_e32m2.cpp)
install(TARGETS vsetvl_e32m2  DESTINATION /root/helloworld/rvv)
