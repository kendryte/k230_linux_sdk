# cmake_minimum_required(VERSION 3.10)
# project(helloWorld_cmake)

# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}   -mcpu=c908v -mabi=lp64d  -mtune=c908 -mrvv-v0p10-compatible  -mrvv-auto-vectorize")
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}   -mcpu=c908v -mabi=lp64d  -mtune=c908 -mrvv-v0p10-compatible  -mrvv-auto-vectorize")


set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -rdynamic")
add_executable(spi  spi.c)
target_link_libraries(spi PRIVATE m)
install(TARGETS spi  DESTINATION /root/app/helloworld/spi/)




# add_executable(spi1  spi1.c)
# target_link_libraries(spi1 PRIVATE m pthread)
# install(TARGETS spi1  DESTINATION /root/app/helloworld/spi/)
