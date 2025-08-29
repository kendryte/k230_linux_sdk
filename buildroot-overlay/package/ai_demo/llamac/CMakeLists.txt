cmake_minimum_required(VERSION 3.10)
if(CMAKE_TOOLCHAIN_FILE MATCHES "buildroot/toolchainfile.cmake")
    get_filename_component(CURRENT_DIR_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
    project(${CURRENT_DIR_NAME})
    set(src llama_run.cc)
    include(../common/ai_demo_cml_common)
    install(FILES ${AIDEMO_UTILS_DIR}/llama.bin DESTINATION  /root/app/${PROJECT_NAME}/)
    install(FILES ${AIDEMO_UTILS_DIR}/tokenizer.bin DESTINATION  /root/app/${PROJECT_NAME}/)
    install(PROGRAMS ${AIDEMO_SHELL_DIR}/llama_build.sh DESTINATION  /root/app/${PROJECT_NAME}/)
else()
    set(src llama_run.cc)

    set(bin anomaly_det.elf)

    include_directories(.)
    add_executable(${bin} ${src})

    target_link_libraries(${bin} -Wl,--start-group Nncase.Runtime.Native nncase.rt_modules.k230 functional_k230 opencv_imgcodecs opencv_imgproc opencv_core sharpyuv libjpeg.so.9 webp png z v4l2-drm drm display mmz pthread -Wl,--end-group)

    install(TARGETS ${bin} DESTINATION bin)
endif()
