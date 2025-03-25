cmake_minimum_required(VERSION 3.5)

set(CMAKE_BUILD_TYPE Debug)

project(v4l2-drm)

include(FindPkgConfig)
find_package(PkgConfig REQUIRED)
pkg_check_modules(DISPLAY REQUIRED display)

include_directories(include ${DISPLAY_INCLUDE_DIRS})

add_library(${PROJECT_NAME} src/lib.c)
target_link_libraries(${PROJECT_NAME} ${DISPLAY_LIBRARIES})

add_executable(${PROJECT_NAME}-test src/main.c)
target_link_libraries(${PROJECT_NAME}-test ${PROJECT_NAME})
set_target_properties(${PROJECT_NAME}-test PROPERTIES OUTPUT_NAME ${PROJECT_NAME})

install(TARGETS ${PROJECT_NAME} DESTINATION /usr/lib)
install(TARGETS ${PROJECT_NAME}-test DESTINATION /usr/bin)
install(DIRECTORY include/ DESTINATION include)
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.pc
    @ONLY
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.pc DESTINATION lib/pkgconfig)
