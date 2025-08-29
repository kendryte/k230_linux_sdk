cmake_minimum_required(VERSION 3.10)
if(CMAKE_TOOLCHAIN_FILE MATCHES "buildroot/toolchainfile.cmake")
    get_filename_component(CURRENT_DIR_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
    project(${CURRENT_DIR_NAME})
    find_package(Freetype REQUIRED)
    include_directories(${FREETYPE_INCLUDE_DIRS})
    include_directories(${CMAKE_SOURCE_DIR}/../common)
    set(src main.cc  ocr_box.cc ocr_reco.cc clipper.cpp text_paint.cc)
    link_libraries(freetype)
    include(../common/ai_demo_cml_common)
    install(FILES ${AIDEMO_KMODEL_DIR}/ocr_det_int16.kmodel DESTINATION  /root/app/${PROJECT_NAME}/)
    install(FILES ${AIDEMO_KMODEL_DIR}/ocr_rec_int16.kmodel DESTINATION  /root/app/${PROJECT_NAME}/)
    install(FILES ${AIDEMO_IMAGES_DIR}/333.jpg DESTINATION  /root/app/${PROJECT_NAME}/)
    install(FILES ${AIDEMO_UTILS_DIR}/dict_ocr.txt  DESTINATION  /root/app/${PROJECT_NAME}/)
    install(FILES ${AIDEMO_UTILS_DIR}/SourceHanSansSC-Normal-Min.ttf  DESTINATION  /root/app/${PROJECT_NAME}/)
    install(PROGRAMS ${AIDEMO_SHELL_DIR}/ocr_img.sh DESTINATION  /root/app/${PROJECT_NAME}/)
    install(PROGRAMS ${AIDEMO_SHELL_DIR}/ocr_isp.sh DESTINATION  /root/app/${PROJECT_NAME}/)

else()

    set(src main.cc
        ${CMAKE_SOURCE_DIR}/common/utils.cc
        ${CMAKE_SOURCE_DIR}/common/ai_base.cc
        ${CMAKE_SOURCE_DIR}/common/sensor_buf_manager.cc
        ocr_box.cc ocr_reco.cc clipper.cpp text_paint.cc)

    set(bin ocr_reco.elf)

    add_executable(${bin} ${src})

    target_link_libraries(${bin} -Wl,--start-group Nncase.Runtime.Native nncase.rt_modules.k230 functional_k230 opencv_imgcodecs opencv_imgproc opencv_core libbz2.so.1.0 freetype sharpyuv libjpeg.so.9 webp png z v4l2-drm drm display mmz pthread -Wl,--end-group)

    install(TARGETS ${bin} DESTINATION bin)
endif()
