cmake_minimum_required(VERSION 3.10)
if(CMAKE_TOOLCHAIN_FILE MATCHES "buildroot/toolchainfile.cmake")
    get_filename_component(CURRENT_DIR_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
    project(${CURRENT_DIR_NAME})
    set(src main.cc  kws.cc
            feature_pipeline.cc
            fft.cc
            get_pcm.cc
            play_wav.cc)
    include(../common/ai_demo_cml_common)
    target_link_libraries(${bin} PRIVATE alure2 asound libopenal.so)
    install(FILES ${KMODE_DIR}/kws.kmodel  DESTINATION  /root/app/${PROJECT_NAME}/)
    install(DIRECTORY ${KMODE_DIR}/../utils/reply_wav/  DESTINATION  /root/app/${PROJECT_NAME}/reply_wav)
    install(PROGRAMS ${SHELL_DIR}/kws.sh  DESTINATION /root/app/${PROJECT_NAME}/)
else()

    set(src main.cc
            ${CMAKE_SOURCE_DIR}/common/ai_base.cc
            kws.cc
            feature_pipeline.cc
            fft.cc
            get_pcm.cc
            play_wav.cc)
    set(bin kws.elf)

    add_executable(${bin} ${src})
    target_link_libraries(${bin} -Wl,--start-group Nncase.Runtime.Native nncase.rt_modules.k230 functional_k230 opencv_imgcodecs opencv_imgproc opencv_core sharpyuv libjpeg.so.9 webp png z v4l2-drm drm display mmz pthread alure2 asound libopenal.so -Wl,--end-group)

    install(TARGETS ${bin} DESTINATION bin)
endif()
