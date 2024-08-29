cmake_minimum_required(VERSION 3.5)

set(CMAKE_CXX_STANDARD 17)

project(face_detect)

add_definitions(-mcpu=c908)
#add_definitions(-fsanitize=address)
#add_link_options(-fsanitize=address -static-libasan)

find_package(nncaseruntime REQUIRED)
#find_package(nncase_rt_modules_k230 REQUIRED)
#find_package(gsl-lite REQUIRED)
find_package(OpenCV)

#message(nncaseruntime_LIBS=${nncaseruntime_LIBS} ${nncaseruntime_INCLUDE_DIRS})

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

if(${OpenCV_FOUND})

include(FindPkgConfig)
find_package(PkgConfig REQUIRED)
pkg_check_modules(DISPLAY REQUIRED display)
pkg_check_modules(V4L2DRM REQUIRED v4l2-drm)

include_directories(${OpenCV_INCLUDE_DIRS} ${DISPLAY_INCLUDE_DIRS} ${V4L2DRM_INCLUDE_DIRS})
add_executable(${PROJECT_NAME}.elf main_cv.cpp model.cc mobile_retinaface.cc anchors_320.cc util.cc)
target_link_libraries(${PROJECT_NAME}.elf ${OpenCV_LIBS} ${DISPLAY_LIBRARIES} ${V4L2DRM_LIBRARIES})
install(PROGRAMS run_cv.sh DESTINATION /app/face_detect)

else(${OpenCV_FOUND})

add_executable(${PROJECT_NAME}.elf main.cc model.cc mobile_retinaface.cc anchors_320.cc util.cc)
install(PROGRAMS run.sh DESTINATION /app/face_detect)

endif(${OpenCV_FOUND})

target_link_libraries(${PROJECT_NAME}.elf nncase.rt_modules.k230 nncaseruntime functional_k230 mmz pthread)
#target_link_libraries(${PROJECT_NAME}.elf nncaseruntime nncase_rt_modules_k230 mmz)

install(TARGETS ${PROJECT_NAME}.elf DESTINATION /app/face_detect)
install(FILES face_detection_320.kmodel ai2d_input.bin DESTINATION /app/face_detect)
