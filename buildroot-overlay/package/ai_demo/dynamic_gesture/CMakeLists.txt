set(src main.cc 
    ${CMAKE_SOURCE_DIR}/common/utils.cc
    ${CMAKE_SOURCE_DIR}/common/ai_base.cc
    ${CMAKE_SOURCE_DIR}/common/dma_buf_manager.cc
    hand_detection.cc hand_keypoint.cc dynamic_gesture.cc)
set(bin dynamic_gesture.elf)

add_executable(${bin} ${src})

target_link_libraries(${bin} -Wl,--start-group Nncase.Runtime.Native nncase.rt_modules.k230 functional_k230 opencv_imgcodecs opencv_imgproc opencv_core sharpyuv libjpeg.so.9 webp png z v4l2-drm drm display mmz pthread -Wl,--end-group)

install(TARGETS ${bin} DESTINATION bin)