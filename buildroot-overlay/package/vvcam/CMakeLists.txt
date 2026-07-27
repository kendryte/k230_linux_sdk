cmake_minimum_required(VERSION 3.5)

set(CMAKE_BUILD_TYPE Debug)

project(vvcam)

file(GLOB SOURCES CONFIGURE_DEPENDS "src/*.c")
add_library(${PROJECT_NAME} SHARED ${SOURCES})
target_include_directories(${PROJECT_NAME} PUBLIC include)

add_subdirectory(v4l2-drm)

add_subdirectory(v4l2-drm-scene)

add_subdirectory(v4l2-drm-three-camera)

install(TARGETS ${PROJECT_NAME} DESTINATION /usr/lib)
install(PROGRAMS ${CMAKE_CURRENT_SOURCE_DIR}/isp_media_server DESTINATION /usr/bin)
install(DIRECTORY include/ DESTINATION /usr/include)
install(DIRECTORY configs/ DESTINATION /etc/vvcam)
