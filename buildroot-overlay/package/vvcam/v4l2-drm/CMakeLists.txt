cmake_minimum_required(VERSION 3.1.0)

set(CMAKE_BUILD_TYPE Debug)

project(v4l2-drm)

include(FindPkgConfig)
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBDRM REQUIRED libdrm)

include_directories(include)

add_library(display src/display.c)
target_link_libraries(display ${LIBDRM_LIBRARIES})
target_include_directories(display PUBLIC ${LIBDRM_INCLUDE_DIRS})

add_executable(${PROJECT_NAME} src/main.c)
target_link_libraries(${PROJECT_NAME} display)

install(TARGETS ${PROJECT_NAME} DESTINATION /usr/bin)
