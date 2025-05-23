cmake_minimum_required(VERSION 3.10)
if(CMAKE_TOOLCHAIN_FILE MATCHES "buildroot/toolchainfile.cmake")
    get_filename_component(CURRENT_DIR_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
    project(${CURRENT_DIR_NAME})
    set(src main.cc BYTETracker.cc kalmanFilter.cc lapjv.cc STrack.cc person_detect.cc)
    include(../common/ai_demo_cml_common)
    include_directories("eigen3")
    install(FILES ${KMODE_DIR}/bytetrack_yolov5n.kmodel DESTINATION  /root/app/${PROJECT_NAME}/)
    install(DIRECTORY ${KM_IMG}/bytetrack_data  DESTINATION  /root/app/${PROJECT_NAME}/bytetrack_data)
    install(PROGRAMS ${SHELL_DIR}/bytetrack_isp.sh DESTINATION  /root/app/${PROJECT_NAME}/)
    install(PROGRAMS ${SHELL_DIR}/bytetrack_img.sh DESTINATION  /root/app/${PROJECT_NAME}/)
    install(PROGRAMS ${SHELL_DIR}/bytetrack_img.sh DESTINATION  /root/app/${PROJECT_NAME}/bytetrack_data/output)


else()
    set(src main.cc
        ${CMAKE_SOURCE_DIR}/common/utils.cc
        ${CMAKE_SOURCE_DIR}/common/ai_base.cc
        ${CMAKE_SOURCE_DIR}/common/sensor_buf_manager.cc
        BYTETracker.cc kalmanFilter.cc lapjv.cc STrack.cc person_detect.cc)
    set(bin bytetrack.elf)

    include_directories("eigen3")

    add_executable(${bin} ${src})

    target_link_libraries(${bin} -Wl,--start-group Nncase.Runtime.Native nncase.rt_modules.k230 functional_k230 opencv_imgcodecs opencv_imgproc opencv_core sharpyuv libjpeg.so.9 webp png z v4l2-drm drm display mmz pthread -Wl,--end-group)

    install(TARGETS ${bin} DESTINATION bin)
endif()
