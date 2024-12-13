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