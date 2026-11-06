cmake_minimum_required(VERSION 3.5)

set(CMAKE_BUILD_TYPE Debug)

project(vvcam)

file(GLOB SOURCES CONFIGURE_DEPENDS "src/*.c")
add_library(${PROJECT_NAME} SHARED ${SOURCES})
target_include_directories(${PROJECT_NAME} PUBLIC include)
target_link_libraries(${PROJECT_NAME} PUBLIC pthread)

add_subdirectory(v4l2-drm)

add_subdirectory(v4l2-drm-scene)

add_subdirectory(v4l2-drm-three-camera)

find_package(PkgConfig)
if(PkgConfig_FOUND)
	pkg_check_modules(LIBCAMERA libcamera)
endif()
if(LIBCAMERA_FOUND)
	pkg_check_modules(DISPLAY display)
	pkg_check_modules(LIBDRM libdrm)
	if(DISPLAY_FOUND AND LIBDRM_FOUND)
		add_subdirectory(libcamera-drm)
	else()
		message(STATUS "libcamera-drm skipped: display or libdrm not found")
	endif()
endif()

install(TARGETS ${PROJECT_NAME} DESTINATION /usr/lib)
install(DIRECTORY include/ DESTINATION /usr/include)
install(DIRECTORY configs/ DESTINATION /etc/vvcam)
