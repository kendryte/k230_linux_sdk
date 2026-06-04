set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -rdynamic")

# Find libdrm package
find_package(PkgConfig REQUIRED)
pkg_check_modules(DRM REQUIRED libdrm)

# Find libpng
pkg_check_modules(PNG REQUIRED libpng)

# Find zlib
pkg_check_modules(ZLIB REQUIRED zlib)

add_executable(drm_simple  drm_simple.c)
target_include_directories(drm_simple PRIVATE ${DRM_INCLUDE_DIRS})
target_link_libraries(drm_simple PRIVATE ${DRM_LIBRARIES} m)
install(TARGETS drm_simple  DESTINATION /root/app/helloworld/drm/)

add_executable(drm_k230_lib  drm_k230_lib.c)
target_include_directories(drm_k230_lib PRIVATE ${DRM_INCLUDE_DIRS})
target_link_libraries(drm_k230_lib PRIVATE m pthread display ${DRM_LIBRARIES})
install(TARGETS drm_k230_lib  DESTINATION /root/app/helloworld/drm/)

add_executable(drm_k230_lib_show_img  drm_k230_lib_show_img.c)
target_include_directories(drm_k230_lib_show_img PRIVATE ${DRM_INCLUDE_DIRS} ${PNG_INCLUDE_DIRS})
target_link_libraries(drm_k230_lib_show_img PRIVATE display ${DRM_LIBRARIES} ${PNG_LIBRARIES})
install(TARGETS drm_k230_lib_show_img  DESTINATION /root/app/helloworld/drm/)
