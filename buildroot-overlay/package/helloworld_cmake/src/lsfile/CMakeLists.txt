# cmake_minimum_required(VERSION 3.10)
# project(helloWorld_cmake)

# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0 -funwind-tables")
# set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -rdynamic")
add_executable(helloWorld_lsfile list_file.cpp)

install(TARGETS helloWorld_lsfile DESTINATION /root/helloworld/lsfile)

# add_subdirectory(opencv_test)
