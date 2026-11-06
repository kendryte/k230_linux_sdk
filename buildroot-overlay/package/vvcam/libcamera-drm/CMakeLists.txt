cmake_minimum_required(VERSION 3.5)

project(libcamera-drm)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBCAMERA libcamera)
pkg_check_modules(DISPLAY display)
pkg_check_modules(LIBDRM libdrm)
if(NOT LIBCAMERA_FOUND OR NOT DISPLAY_FOUND OR NOT LIBDRM_FOUND)
	message(STATUS "libcamera-drm skipped: libcamera, display or libdrm not found")
	return()
endif()

function(vvcam_libcamera_tool name)
	add_executable(${name} ${name}.cpp)
	target_compile_options(${name} PRIVATE -std=c++20 -Wall -Wextra -O2)
	target_include_directories(${name} PRIVATE
		${LIBCAMERA_INCLUDE_DIRS}
		${CMAKE_CURRENT_SOURCE_DIR}/../include)
	target_link_libraries(${name} ${LIBCAMERA_LIBRARIES} pthread)
endfunction()

vvcam_libcamera_tool(vvcam-libcamera-drm)
target_include_directories(vvcam-libcamera-drm PRIVATE ${DISPLAY_INCLUDE_DIRS} ${LIBDRM_INCLUDE_DIRS})
target_link_libraries(vvcam-libcamera-drm ${DISPLAY_LIBRARIES} ${LIBDRM_LIBRARIES})

vvcam_libcamera_tool(vvcam-libcamera-hold)
vvcam_libcamera_tool(vvcam-libcamera-grab)

install(TARGETS vvcam-libcamera-drm vvcam-libcamera-hold vvcam-libcamera-grab
	DESTINATION /usr/bin)
