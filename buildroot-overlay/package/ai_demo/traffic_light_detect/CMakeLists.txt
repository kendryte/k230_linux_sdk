cmake_minimum_required(VERSION 3.10)
if(CMAKE_TOOLCHAIN_FILE MATCHES "buildroot/toolchainfile.cmake")
    get_filename_component(CURRENT_DIR_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
    project(${CURRENT_DIR_NAME})
    set(src main.cc   traffic_light_detect.cc)
    include(../common/ai_demo_cml_common)
    install(FILES ${AIDEMO_KMODEL_DIR}/traffic.kmodel DESTINATION  /root/app/${PROJECT_NAME}/)
    install(FILES ${AIDEMO_IMAGES_DIR}/traffic.jpg DESTINATION  /root/app/${PROJECT_NAME}/)
    install(PROGRAMS ${AIDEMO_SHELL_DIR}/traffic_light_detect_isp.sh DESTINATION  /root/app/${PROJECT_NAME}/)
    install(PROGRAMS ${AIDEMO_SHELL_DIR}/traffic_light_detect_image.sh DESTINATION  /root/app/${PROJECT_NAME}/)
else()

    set(src main.cc
        ${CMAKE_SOURCE_DIR}/common/utils.cc
        ${CMAKE_SOURCE_DIR}/common/ai_base.cc
        ${CMAKE_SOURCE_DIR}/common/sensor_buf_manager.cc
        traffic_light_detect.cc)
    set(bin traffic_light_detect.elf)

    add_executable(${bin} ${src})

    target_link_libraries(${bin} -Wl,--start-group Nncase.Runtime.Native nncase.rt_modules.k230 functional_k230 opencv_imgcodecs opencv_imgproc opencv_core sharpyuv libjpeg.so.9 webp png z v4l2-drm drm display mmz pthread -Wl,--end-group)

    install(TARGETS ${bin} DESTINATION bin)
endif()
