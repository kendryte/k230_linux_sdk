
if(CMAKE_TOOLCHAIN_FILE MATCHES "buildroot/toolchainfile.cmake")

    set(src main.cc  hifigan.cc
            fastspeech1.cc
            fastspeech2.cc
            length_regulator.cpp
            play_pcm.cc)
    include(../../common/ai_demo_cml_common)
    target_link_libraries(${bin} PRIVATE tts_zh_lib alure2 asound libopenal.so )
    install(FILES ${AIDEMO_KMODEL_DIR}/zh_fastspeech_1.kmodel  DESTINATION  /root/app/${PROJECT_NAME}/)
    install(FILES ${AIDEMO_KMODEL_DIR}/zh_fastspeech_2.kmodel  DESTINATION  /root/app/${PROJECT_NAME}/)
    install(FILES ${AIDEMO_KMODEL_DIR}/hifigan.kmodel  DESTINATION  /root/app/${PROJECT_NAME}/)
    install(FILES ${AIDEMO_UTILS_DIR}/wav_play.elf  DESTINATION  /root/app/${PROJECT_NAME}/)
    install(DIRECTORY ${AIDEMO_UTILS_DIR}/file  DESTINATION  /root/app/${PROJECT_NAME}/file)
    install(PROGRAMS ${AIDEMO_SHELL_DIR}/tts_zh.sh  DESTINATION /root/app/${PROJECT_NAME}/)
else()

    set(src main.cc
            ${CMAKE_SOURCE_DIR}/common/ai_base.cc
            hifigan.cc
            fastspeech1.cc
            fastspeech2.cc
            length_regulator.cpp
            play_pcm.cc)
    set(bin tts_zh.elf)

    add_executable(${bin} ${src})
    target_link_libraries(${bin} -Wl,--start-group Nncase.Runtime.Native nncase.rt_modules.k230 functional_k230 opencv_imgcodecs opencv_imgproc opencv_core sharpyuv libjpeg.so.9 webp png z v4l2-drm drm display mmz pthread alure2 asound libopenal.so -Wl,--end-group)

    target_link_libraries(${bin} tts_zh_lib)
    install(TARGETS ${bin} DESTINATION bin)

endif()
