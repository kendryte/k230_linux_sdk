# Build with original makefile
set(CSI_CV_BUILD_DIR ${OpenCV_SOURCE_DIR}/3rdparty/csi-cv)
set(CSI_CV_INTERFACE_DIR ${OpenCV_SOURCE_DIR}/3rdparty/csi-cv/interface/opencv)
set(CSI_CV_INTERFACE_INCLUDE ${OpenCV_SOURCE_DIR}/3rdparty/csi-cv/interface/opencv/include)
set(CSI_CV_INCLUDE ${OpenCV_SOURCE_DIR}/3rdparty/csi-cv/include)


# Configure the target cpu information
if(CORE STREQUAL "C908V")
add_custom_target(thead ALL
    COMMAND ${CMAKE_COMMAND} -E copy ${CSI_CV_INTERFACE_INCLUDE}/csi_cv_hal.hpp ${CMAKE_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy ${CSI_CV_BUILD_DIR}/libcsi_cv_c908v.a ${3P_LIBRARY_OUTPUT_PATH}/libcsi_cv.a
    WORKING_DIRECTORY ./
    )
elseif(CORE STREQUAL "C907FDVM-RV64")
add_custom_target(thead ALL
    COMMAND ${CMAKE_COMMAND} -E copy ${CSI_CV_INTERFACE_INCLUDE}/csi_cv_hal.hpp ${CMAKE_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy ${CSI_CV_BUILD_DIR}/libcsi_cv_c907fdvm-rv64.a ${3P_LIBRARY_OUTPUT_PATH}/libcsi_cv.a
    WORKING_DIRECTORY ./
    )
elseif(CORE STREQUAL "C907FDVM-RV32")
add_custom_target(thead ALL
    COMMAND ${CMAKE_COMMAND} -E copy ${CSI_CV_INTERFACE_INCLUDE}/csi_cv_hal.hpp ${CMAKE_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy ${CSI_CV_BUILD_DIR}/libcsi_cv_c907fdvm-rv32.a ${3P_LIBRARY_OUTPUT_PATH}/libcsi_cv.a
    WORKING_DIRECTORY ./
    )
elseif(CORE STREQUAL "C906FDV")
add_custom_target(thead ALL
    COMMAND ${CMAKE_COMMAND} -E copy ${CSI_CV_INTERFACE_INCLUDE}/csi_cv_hal.hpp ${CMAKE_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy ${CSI_CV_BUILD_DIR}/libcsi_cv_c906fdv.a ${3P_LIBRARY_OUTPUT_PATH}/libcsi_cv.a
    WORKING_DIRECTORY ./
    )
elseif(CORE STREQUAL "C920")
add_custom_target(thead ALL
    COMMAND ${CMAKE_COMMAND} -E copy ${CSI_CV_INTERFACE_INCLUDE}/csi_cv_hal_c920.hpp ${CMAKE_BINARY_DIR}/csi_cv_hal.hpp
    COMMAND ${CMAKE_COMMAND} -E copy ${CSI_CV_BUILD_DIR}/libcsi_cv_c920.a ${3P_LIBRARY_OUTPUT_PATH}/libcsi_cv.a
    WORKING_DIRECTORY ./
    )
elseif(CORE STREQUAL "C920V2")
add_custom_target(thead ALL
    COMMAND ${CMAKE_COMMAND} -E copy ${CSI_CV_INTERFACE_INCLUDE}/csi_cv_hal.hpp ${CMAKE_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy ${CSI_CV_BUILD_DIR}/libcsi_cv_c920v2.a ${3P_LIBRARY_OUTPUT_PATH}/libcsi_cv.a
    WORKING_DIRECTORY ./
    )
else()
add_custom_target(thead ALL
    COMMAND ${CMAKE_COMMAND} -E copy ${CSI_CV_INTERFACE_INCLUDE}/csi_cv_hal.hpp ${CMAKE_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy ${CSI_CV_BUILD_DIR}/libcsi_cv_c908v.a ${3P_LIBRARY_OUTPUT_PATH}/libcsi_cv.a
    WORKING_DIRECTORY ./
    )
endif()
#COMMENT "Build the csi-cv library.")

add_library(csi_cv STATIC IMPORTED IMPORTED_LOCATION ${3P_LIBRARY_OUTPUT_PATH})

set_target_properties(csi_cv PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${3P_LIBRARY_OUTPUT_PATH})
# config the hal information
set(CSI_CV_HAL_VERSION "0.0.1" PARENT_SCOPE)
set(CSI_CV_HAL_LIBRARIES "${3P_LIBRARY_OUTPUT_PATH}/libcsi_cv.a" PARENT_SCOPE)
set(CSI_CV_HAL_HEADERS "csi_cv_hal.hpp" PARENT_SCOPE)
set(CSI_CV_HAL_INCLUDE_DIRS "${CMAKE_BINARY_DIR}" PARENT_SCOPE)


include_directories("${CMAKE_BINARY_DIR}")
