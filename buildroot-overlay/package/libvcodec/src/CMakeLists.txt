cmake_minimum_required(VERSION 3.10)

project(vcodec VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(${PROJECT_NAME} SHARED
    kd_common.c
    kd_bufgroup.c
    kd_enc.c
    kd_dec.c
    kd_jpeg.c)

# include/ is public, the source directory is private
# (kd_common.h / kd_mvx.h are not installed)
target_include_directories(${PROJECT_NAME}
    PUBLIC  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:include>
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra)

set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION   ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR})

configure_file(vcodec.pc.in
               ${CMAKE_CURRENT_BINARY_DIR}/vcodec.pc @ONLY)

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION lib)
install(FILES include/vcodec.h DESTINATION include)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/vcodec.pc
        DESTINATION lib/pkgconfig)
