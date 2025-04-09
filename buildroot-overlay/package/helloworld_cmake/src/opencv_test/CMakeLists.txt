# Find OpenCV, you may need to set OpenCV_DIR variable
# to the absolute path to the directory containing OpenCVConfig.cmake file
# via the command line or GUI
find_package(OpenCV REQUIRED)

# If the package has been found, several variables will
# be set, you can find the full list with descriptions
# in the OpenCVConfig.cmake file.
# Print some message showing some of them
message(STATUS "OpenCV library status:")
message(STATUS "    config: ${OpenCV_DIR}")
message(STATUS "    version: ${OpenCV_VERSION}")
message(STATUS "    libraries: ${OpenCV_LIBS}")
message(STATUS "    include path: ${OpenCV_INCLUDE_DIRS}")




add_executable(angle_main angle_main.cc)
add_executable(hist hist.cc)
add_executable(inter inter.cc)
add_executable(media_blur media_blur.cc)
add_executable(warpaffine warpaffine.cc)

# Link your application with OpenCV libraries
target_link_libraries(angle_main PRIVATE ${OpenCV_LIBS})
target_link_libraries(hist PRIVATE ${OpenCV_LIBS})
target_link_libraries(inter PRIVATE ${OpenCV_LIBS})
target_link_libraries(media_blur PRIVATE ${OpenCV_LIBS})
target_link_libraries(warpaffine PRIVATE ${OpenCV_LIBS})


install(TARGETS angle_main DESTINATION  bin/helloworld/opencv_test)
install(TARGETS hist DESTINATION        bin/helloworld/opencv_test)
install(TARGETS inter DESTINATION       bin/helloworld/opencv_test)
install(TARGETS media_blur DESTINATION  bin/helloworld/opencv_test)
install(TARGETS warpaffine DESTINATION  bin/helloworld/opencv_test)
install(FILES  1.jpg DESTINATION        bin/helloworld/opencv_test)
install(FILES  2.jpg DESTINATION        bin/helloworld/opencv_test)
install(PROGRAMS   run.sh DESTINATION        bin/helloworld/opencv_test)
install(PROGRAMS   opencv_benchmark.py  DESTINATION        bin/helloworld/opencv_test)
