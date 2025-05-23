cmake_minimum_required(VERSION 3.5)
set(CMAKE_CXX_STANDARD 17)
project(ai_demo_common VERSION 1.0.0)
# find_package(nncaseruntime REQUIRED)
#find_package(nncase_rt_modules_k230 REQUIRED)
#find_package(gsl-lite REQUIRED)
find_package(OpenCV)


#message(STATUS "xtttt ${OpenCV_INCLUDE_DIRS}")

file(GLOB src *.cc)
file(GLOB heads_instll *.h *.hpp)

# 生成静态库
add_library(ai_demo_common STATIC   ${src} )


# 指定库的包含目录
target_include_directories(ai_demo_common
    PUBLIC
        $<BUILD_INTERFACE:${OpenCV_INCLUDE_DIRS}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>  # 当前源目录
        $<INSTALL_INTERFACE:include/ai_demo_common>
)



# 安装静态库和头文件
install(TARGETS ai_demo_common
    EXPORT ai_demo_commonTargets
    ARCHIVE DESTINATION lib
    # LIBRARY DESTINATION lib
    # RUNTIME DESTINATION bin
)

# install(DIRECTORY include/ DESTINATION include)
# 安装多个头文件到目标位置
install(FILES
        ai_base.h  dma_buf_manager.h  sensor_buf_manager.h  utils.h  vi_vo.h  scoped_timing.hpp
        DESTINATION include/ai_demo_common
)



# 5. 包含用于生成配置文件的 CMake 模块
include(CMakePackageConfigHelpers)
include(GNUInstallDirs) # 提供标准的安装目录变量
#message(STATUS "xtttt ${CMAKE_INSTALL_LIBDIR}, ${CMAKE_CURRENT_BINARY_DIR},${CMAKE_INSTALL_FULL_LIBDIR}")
# 6. 生成版本文件 (ai_demo_commonConfigVersion.cmake)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/ai_demo_commonConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

# 7. 配置主配置文件模板 (ai_demo_commonConfig.cmake.in -> ai_demo_commonConfig.cmake)
configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/ai_demo_commonConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/ai_demo_commonConfig.cmake"
    INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}"
    NO_SET_AND_CHECK_MACRO
)

# 8. 安装生成的配置文件和版本文件
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/ai_demo_commonConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/ai_demo_commonConfigVersion.cmake"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}"
)

# 9. 安装前面用 EXPORT 标记的目标信息文件
install(EXPORT ai_demo_commonTargets
    FILE ai_demo_commonTargets.cmake # 生成的文件名
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}"
)
