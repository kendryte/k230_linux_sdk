# cmake_minimum_required(VERSION 3.10)
# project(helloWorld_cmake)

# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}   -mcpu=c908v -mabi=lp64d  -mtune=c908 -mrvv-v0p10-compatible  -mrvv-auto-vectorize")
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}   -mcpu=c908v -mabi=lp64d  -mtune=c908 -mrvv-v0p10-compatible  -mrvv-auto-vectorize")


set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -rdynamic")
add_executable(pwm  pwm.c)
target_link_libraries(pwm PRIVATE m)
install(TARGETS pwm  DESTINATION /root/app/helloworld/pwm/)




# add_executable(pwm1  pwm1.c)
# target_link_libraries(pwm1 PRIVATE m pthread)
# install(TARGETS pwm1  DESTINATION /root/app/helloworld/pwm/)
