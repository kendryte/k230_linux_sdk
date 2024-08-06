cmake_minimum_required(VERSION 3.1.0)

set(CMAKE_BUILD_TYPE Debug)

project(v4l2-drm)

include(FindPkgConfig)
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBDRM REQUIRED libdrm)

include_directories(include)

file(GLOB SOURCES "src/*.c")
add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} ${LIBDRM_LIBRARIES})
target_include_directories(${PROJECT_NAME} PUBLIC ${LIBDRM_INCLUDE_DIRS})

install(TARGETS ${PROJECT_NAME} DESTINATION /usr/bin)
