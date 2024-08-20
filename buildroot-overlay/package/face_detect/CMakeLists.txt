cmake_minimum_required(VERSION 3.5)

set(CMAKE_CXX_STANDARD 17)

project(face_detect)

find_package(OpenCV)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

if(${OpenCV_FOUND})

include_directories(${OpenCV_INCLUDE_DIRS})
add_executable(${PROJECT_NAME}.elf main_cv.cpp model.cc mobile_retinaface.cc anchors_320.cc util.cc)
target_link_libraries(${PROJECT_NAME}.elf ${OpenCV_LIBS})
install(PROGRAMS run_cv.sh DESTINATION /app/face_detect)

else(${OpenCV_FOUND})

add_executable(${PROJECT_NAME}.elf main.cc model.cc mobile_retinaface.cc anchors_320.cc util.cc)
install(PROGRAMS run.sh DESTINATION /app/face_detect)

endif(${OpenCV_FOUND})

target_link_libraries(${PROJECT_NAME}.elf nncase.rt_modules.k230 Nncase.Runtime.Native functional_k230 mmz)

install(TARGETS ${PROJECT_NAME}.elf DESTINATION /app/face_detect)
install(FILES face_detection_320.kmodel ai2d_input.bin DESTINATION /app/face_detect)
