cmake_minimum_required(VERSION 3.10)
if(CMAKE_TOOLCHAIN_FILE MATCHES "buildroot/toolchainfile.cmake")
    get_filename_component(CURRENT_DIR_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
    project(${CURRENT_DIR_NAME})
    set(src main.cc  face_detection.cc anchors_320.cc anchors_640.cc face_emotion.cc)
    include(../common/ai_demo_cml_common)
    install(FILES ${AIDEMO_KMODEL_DIR}/face_detection_320.kmodel DESTINATION  /root/app/${PROJECT_NAME}/)
    install(FILES ${AIDEMO_KMODEL_DIR}/face_emotion.kmodel DESTINATION  /root/app/${PROJECT_NAME}/)
    install(FILES ${AIDEMO_IMAGES_DIR}/1024x768.jpg DESTINATION  /root/app/${PROJECT_NAME}/)
    install(PROGRAMS ${AIDEMO_SHELL_DIR}/face_emotion_image.sh DESTINATION  /root/app/${PROJECT_NAME}/)
    install(PROGRAMS ${AIDEMO_SHELL_DIR}/face_emotion_isp.sh DESTINATION  /root/app/${PROJECT_NAME}/)

else()

    set(src main.cc
        ${CMAKE_SOURCE_DIR}/common/utils.cc
        ${CMAKE_SOURCE_DIR}/common/ai_base.cc
        ${CMAKE_SOURCE_DIR}/common/sensor_buf_manager.cc
        face_detection.cc anchors_320.cc anchors_640.cc face_emotion.cc)

    set(bin face_emotion.elf)
    add_executable(${bin} ${src})

    # 添加链接器选项
    target_link_libraries(${bin} -Wl,--start-group Nncase.Runtime.Native nncase.rt_modules.k230 functional_k230 opencv_imgcodecs opencv_imgproc opencv_core sharpyuv libjpeg.so.9 webp png z v4l2-drm drm display mmz pthread -Wl,--end-group)

    install(TARGETS ${bin} DESTINATION bin)
endif()
