cmake_minimum_required(VERSION 3.5)

set(CMAKE_CXX_STANDARD 17)

project(display
    VERSION 1.0
    DESCRIPTION "abstruct to libdrm"
)

include(FindPkgConfig)
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBDRM REQUIRED libdrm)

file(GLOB SOURCES "src/*.c")

add_library(${PROJECT_NAME} src/display.c)
target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${LIBDRM_INCLUDE_DIRS}>
    #$<INSTALL_INTERFACE:${LIBDRM_INCLUDE_DIRS}>
)
target_link_libraries(${PROJECT_NAME} ${LIBDRM_LIBRARIES})

# add_library(pipeline src/pipeline.c)
# target_include_directories(pipeline PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
# target_link_libraries(pipeline ev)
# add_executable(test-pipeline src/test-pipeline.c)
# target_link_libraries(test-pipeline pipeline)

if(${BUILD_TEST})
    add_executable(test-${PROJECT_NAME} src/test.cpp)
    target_link_libraries(test-${PROJECT_NAME} display)
endif(${BUILD_TEST})


install(DIRECTORY include/ DESTINATION include)
install(TARGETS ${PROJECT_NAME} DESTINATION lib)

set(DRM_INCLUDES "")
foreach(DRM_INCLUDE ${LIBDRM_INCLUDE_DIRS})
    set(DRM_INCLUDES "${DRM_INCLUDES} -I${DRM_INCLUDE}")
endforeach(DRM_INCLUDE ${LIBDRM_INCLUDE_DIRS})

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.pc
    @ONLY
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.pc DESTINATION lib/pkgconfig)
