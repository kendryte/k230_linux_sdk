cmake_minimum_required(VERSION 3.10)
if(CMAKE_TOOLCHAIN_FILE MATCHES "buildroot/toolchainfile.cmake")
    get_filename_component(CURRENT_DIR_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
    project(${CURRENT_DIR_NAME})
    set(src main.cc  face_detection.cc anchors_320.cc anchors_640.cc face_mesh_post.cc face_mesh.cc)
    include(../common/ai_demo_cml_common)
    install(FILES ${KMODE_DIR}/face_detection_320.kmodel DESTINATION  /root/app/${PROJECT_NAME}/)
    install(FILES ${KMODE_DIR}/face_alignment.kmodel DESTINATION  /root/app/${PROJECT_NAME}/)
    install(FILES ${KMODE_DIR}/face_alignment_post.kmodel DESTINATION  /root/app/${PROJECT_NAME}/)
    install(FILES ${KM_IMG}/1024x768.jpg DESTINATION  /root/app/${PROJECT_NAME}/)
    install(FILES ${UTITLS}/bfm_tri.bin DESTINATION  /root/app/${PROJECT_NAME}/)
    install(FILES ${UTITLS}/ncc_code.bin DESTINATION  /root/app/${PROJECT_NAME}/)
    install(PROGRAMS ${SHELL_DIR}/face_mesh_image.sh DESTINATION  /root/app/${PROJECT_NAME}/)
    install(PROGRAMS ${SHELL_DIR}/face_mesh_isp.sh DESTINATION  /root/app/${PROJECT_NAME}/)
else()

    set(src main.cc
        ${CMAKE_SOURCE_DIR}/common/utils.cc
        ${CMAKE_SOURCE_DIR}/common/ai_base.cc
        ${CMAKE_SOURCE_DIR}/common/sensor_buf_manager.cc
        face_detection.cc anchors_320.cc anchors_640.cc face_mesh_post.cc face_mesh.cc)

    set(bin face_mesh.elf)

    add_executable(${bin} ${src})

    target_link_libraries(${bin} -Wl,--start-group Nncase.Runtime.Native nncase.rt_modules.k230 functional_k230 opencv_imgcodecs opencv_imgproc opencv_core sharpyuv libjpeg.so.9 webp png z v4l2-drm drm display mmz pthread -Wl,--end-group)

    install(TARGETS ${bin} DESTINATION bin)
endif()
