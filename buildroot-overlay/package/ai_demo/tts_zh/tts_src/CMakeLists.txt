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