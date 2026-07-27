cmake_minimum_required(VERSION 3.5)

set(CMAKE_BUILD_TYPE Debug)

project(v4l2-drm-three-camera)

include(FindPkgConfig)
find_package(PkgConfig REQUIRED)
pkg_check_modules(DISPLAY REQUIRED display)
pkg_check_modules(LIBDRM REQUIRED libdrm)

include_directories(
    include
    ${DISPLAY_INCLUDE_DIRS}
    ${LIBDRM_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/../v4l2-drm/include
)

add_executable(${PROJECT_NAME}
    src/main.c
    src/setup.c
    src/display_video_plane.c
)
target_link_libraries(${PROJECT_NAME} v4l2-drm ${DISPLAY_LIBRARIES} ${LIBDRM_LIBRARIES})

install(TARGETS ${PROJECT_NAME} DESTINATION /usr/bin)
