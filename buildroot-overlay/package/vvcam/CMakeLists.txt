cmake_minimum_required(VERSION 3.1.0)

set(CMAKE_BUILD_TYPE Debug)

project(vvcam)

file(GLOB SOURCES "src/*.c")
add_library(${PROJECT_NAME} SHARED ${SOURCES})
target_include_directories(${PROJECT_NAME} PUBLIC include)

add_subdirectory(v4l2-drm)

install(TARGETS ${PROJECT_NAME} DESTINATION /usr/lib)
install(PROGRAMS ${CMAKE_CURRENT_SOURCE_DIR}/isp_media_server DESTINATION /usr/bin)
install(DIRECTORY include/ DESTINATION /usr/include)
install(DIRECTORY configs/ DESTINATION /etc/vvcam)
