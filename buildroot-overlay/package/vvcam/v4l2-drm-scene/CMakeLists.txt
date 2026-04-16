cmake_minimum_required(VERSION 3.5)

set(CMAKE_BUILD_TYPE Debug)

project(v4l2-drm-scene)

include(FindPkgConfig)
find_package(PkgConfig REQUIRED)
pkg_check_modules(DISPLAY REQUIRED display)

include_directories(${DISPLAY_INCLUDE_DIRS})

# 不依赖 v4l2-drm 库，完全独立实现
add_executable(${PROJECT_NAME} main.c)
target_link_libraries(${PROJECT_NAME} ${DISPLAY_LIBRARIES})

install(TARGETS ${PROJECT_NAME} DESTINATION /usr/bin)
