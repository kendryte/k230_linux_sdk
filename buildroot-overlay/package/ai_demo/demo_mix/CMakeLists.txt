cmake_minimum_required(VERSION 3.10)
if(CMAKE_TOOLCHAIN_FILE MATCHES "buildroot/toolchainfile.cmake")
    get_filename_component(CURRENT_DIR_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
    project(${CURRENT_DIR_NAME})
    set(src hand_detection.cc hand_keypoint.cc anchors_320.cc anchors_640.cc crop.cc cv2_utils.cc dynamic_gesture.cc face_detection.cc face_pose.cc src.cc nanotracker.cc main.cc)
    include(../common/ai_demo_cml_common)

    install(FILES ${KMODE_DIR}/hand_det.kmodel DESTINATION  /root/app/${PROJECT_NAME}/)
    install(FILES ${KMODE_DIR}/handkp_det.kmodel DESTINATION  /root/app/${PROJECT_NAME}/)
    install(FILES ${KMODE_DIR}/gesture.kmodel DESTINATION  /root/app/${PROJECT_NAME}/)
    install(FILES ${KMODE_DIR}/face_detection_320.kmodel DESTINATION  /root/app/${PROJECT_NAME}/)
    install(FILES ${KMODE_DIR}/face_pose.kmodel DESTINATION  /root/app/${PROJECT_NAME}/)
    install(FILES ${KMODE_DIR}/cropped_test127.kmodel DESTINATION  /root/app/${PROJECT_NAME}/)
    install(FILES ${KMODE_DIR}/nanotrack_backbone_sim.kmodel DESTINATION  /root/app/${PROJECT_NAME}/)
    install(FILES ${KMODE_DIR}/nanotracker_head_calib_k230.kmodel DESTINATION  /root/app/${PROJECT_NAME}/)
    install(FILES ${UTITLS}/shang.bin DESTINATION  /root/app/${PROJECT_NAME}/)
    install(FILES ${UTITLS}/xia.bin DESTINATION  /root/app/${PROJECT_NAME}/)
    install(FILES ${UTITLS}/zuo.bin DESTINATION  /root/app/${PROJECT_NAME}/)
    install(FILES ${UTITLS}/you.bin DESTINATION  /root/app/${PROJECT_NAME}/)
    install(PROGRAMS ${SHELL_DIR}/demo_mix.sh DESTINATION  /root/app/${PROJECT_NAME}/)
else()
    set(src main.cc
        ${CMAKE_SOURCE_DIR}/common/utils.cc
        ${CMAKE_SOURCE_DIR}/common/ai_base.cc
        ${CMAKE_SOURCE_DIR}/common/sensor_buf_manager.cc
        hand_detection.cc hand_keypoint.cc anchors_320.cc anchors_640.cc crop.cc cv2_utils.cc dynamic_gesture.cc face_detection.cc face_pose.cc src.cc nanotracker.cc main.cc)
    set(bin demo_mix.elf)

    add_executable(${bin} ${src})

    target_link_libraries(${bin} -Wl,--start-group Nncase.Runtime.Native nncase.rt_modules.k230 functional_k230 opencv_imgcodecs opencv_imgproc opencv_core sharpyuv libjpeg.so.9 webp png z v4l2-drm drm display mmz pthread -Wl,--end-group)

    install(TARGETS ${bin} DESTINATION bin)
endif()
