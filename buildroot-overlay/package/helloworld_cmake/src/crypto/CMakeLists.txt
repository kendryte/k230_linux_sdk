# cmake_minimum_required(VERSION 3.10)
# project(helloWorld_cmake)

# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}   -mcpu=c908v -mabi=lp64d  -mtune=c908 -mrvv-v0p10-compatible  -mrvv-auto-vectorize")
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}   -mcpu=c908v -mabi=lp64d  -mtune=c908 -mrvv-v0p10-compatible  -mrvv-auto-vectorize")


set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -rdynamic")

add_executable(kendryte_sha256_test  kendryte_sha256_test.c)
target_link_libraries(kendryte_sha256_test PRIVATE pthread)
install(TARGETS kendryte_sha256_test  DESTINATION /root/app/helloworld/crypto/)

add_executable(kendryte_sha256_bench  kendryte_sha256_bench.c)
install(TARGETS kendryte_sha256_bench  DESTINATION /root/app/helloworld/crypto/)

add_executable(sha256-concurrent  sha256-concurrent.c)
target_link_libraries(sha256-concurrent PRIVATE pthread)
install(TARGETS sha256-concurrent  DESTINATION /root/app/helloworld/crypto/)
