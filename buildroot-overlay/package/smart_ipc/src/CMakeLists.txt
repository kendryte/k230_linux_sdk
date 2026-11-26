cmake_minimum_required(VERSION 3.10)

# CXX is for the two shims: rtsp_out.cpp over librtsp_server, and
# ai_face.cpp over the nncase runtime
project(smart_ipc VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
# 17, not 11: the nncase headers want it
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)
include(FindPkgConfig)
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBDRM REQUIRED libdrm)

# vcodec.h and libvcodec.so are installed into staging by the libvcodec
# package; the DRM side comes from the display package.
#
# cam_capture.c is also in the vcodec_demo package, which has its own copy:
# the two encoder demos there need it and nothing else is shared with them.
#
# osd2d.c drives the 2D engine over V4L2 for the burnt-in boxes.
#
# Everything is listed here, including the AI and WebRTC sources. They used
# to be added only when their libraries turned up, with the C code behind
# #ifdefs. That is gone: Config.in selects every one of those packages, so
# the "without them" build was never actually produced - it only cost
# eighteen #ifdefs in smart_ipc.c and one way for a long option to quietly
# stop working. A missing library is now a build error, below.
add_executable(smart_ipc
    smart_ipc.c
    cam_capture.c
    osd2d.c
    rtsp_out.cpp
    webrtc_out.c
    webrtc/http_server.c
    ai_face.cpp
    ai/ai_base.cc
    ai/face_detection.cc
    ai/anchors_320.cc
    ai/anchors_640.cc
    ai/utils.cc)

target_compile_options(smart_ipc PRIVATE -Wall -Wextra)
target_include_directories(smart_ipc PRIVATE ${LIBDRM_INCLUDE_DIRS})

# librtsp_server leaves live555 undefined, so the four libraries it was
# built against have to be named here as well
target_link_libraries(smart_ipc PRIVATE vcodec display ${LIBDRM_LIBRARIES}
    rtsp_server liveMedia groupsock BasicUsageEnvironment UsageEnvironment)

# --------------------------------------------------------------------
# WebRTC output: the same encoder packets, into a browser.
# --------------------------------------------------------------------
find_path(PEER_INCLUDE_DIR peer.h PATH_SUFFIXES peer)
# peer_signaling.h includes <cJSON.h> unqualified, so cjson's own directory
# has to be on the include path even though we never include it ourselves
find_path(CJSON_INCLUDE_DIR cJSON.h PATH_SUFFIXES cjson)
find_library(PEER_LIB peer)
find_library(SRTP2_LIB srtp2)
find_library(USRSCTP_LIB usrsctp)
find_library(MBEDTLS_LIB mbedtls)
find_library(MBEDX509_LIB mbedx509)
find_library(MBEDCRYPTO_LIB mbedcrypto)
find_library(CJSON_LIB cjson)

if(NOT PEER_INCLUDE_DIR OR NOT CJSON_INCLUDE_DIR OR NOT PEER_LIB
   OR NOT SRTP2_LIB OR NOT USRSCTP_LIB OR NOT MBEDTLS_LIB
   OR NOT MBEDX509_LIB OR NOT MBEDCRYPTO_LIB OR NOT CJSON_LIB)
    message(FATAL_ERROR
        "smart_ipc needs the webrtc package and cJSON. Config.in selects "
        "BR2_PACKAGE_WEBRTC and BR2_PACKAGE_CJSON, so getting here means "
        "they were turned off by hand or failed to build.")
endif()

# webrtc/ is a byte-for-byte copy of the rt-smart sample so it can still be
# diffed against it, see webrtc/README.md. Its warnings are upstream's
# business; webrtc_out.c is ours and keeps -Wall -Wextra.
set_source_files_properties(webrtc/http_server.c
    PROPERTIES COMPILE_OPTIONS "-w")

target_include_directories(smart_ipc PRIVATE
    ${PEER_INCLUDE_DIR} ${CJSON_INCLUDE_DIR})

# Static libraries that refer back to each other, so order matters:
# whatever is needed must come after whoever needs it.
target_link_libraries(smart_ipc PRIVATE
    ${PEER_LIB}
    ${SRTP2_LIB}
    ${USRSCTP_LIB}
    ${MBEDTLS_LIB}
    ${MBEDX509_LIB}
    ${MBEDCRYPTO_LIB}
    ${CJSON_LIB}
    Threads::Threads)

# --------------------------------------------------------------------
# Face detection on a third camera channel.
# --------------------------------------------------------------------
find_path(NNCASE_INCLUDE_DIR nncase/runtime/interpreter.h)
find_library(NNCASE_RUNTIME_LIB Nncase.Runtime.Native)
pkg_check_modules(OPENCV4 REQUIRED opencv4)

if(NOT NNCASE_INCLUDE_DIR OR NOT NNCASE_RUNTIME_LIB)
    message(FATAL_ERROR
        "smart_ipc needs the nncase runtime. Config.in selects "
        "BR2_PACKAGE_LIBNNCASE, so getting here means it was turned off "
        "by hand or failed to build.")
endif()

# The nncase kernels are built for this core and vector extension; the
# static libraries will not link against anything else. Only the C++ files
# are built that way: cam_capture.c and osd2d.c are plain C with no reason
# to be tied to one core.
set(AI_ARCH_FLAGS -mcpu=c908v -mabi=lp64d -mtune=c908
    -mrvv-v0p10-compatible -mrvv-auto-vectorize)

set_source_files_properties(ai_face.cpp PROPERTIES
    COMPILE_OPTIONS "${AI_ARCH_FLAGS}")

# The files under ai/ are kept as close to the face_detect package's copies
# as this program allows, so they can still be diffed against it when it
# moves. Eight of them and the kmodel are byte-identical; face_detection.cc
# and .h lost draw_result(), draw_result_video() and the sensor_set.h
# include, which pull in a display stack this program does not use. ai/
# README.md spells those two out.
# They are noisy about narrowing conversions, and that is the vendor's
# business, not ours, so they are built without warnings and ai_face.cpp
# keeps -Wall -Wextra.
set_source_files_properties(
    ai/ai_base.cc ai/face_detection.cc
    ai/anchors_320.cc ai/anchors_640.cc ai/utils.cc
    PROPERTIES COMPILE_OPTIONS "${AI_ARCH_FLAGS};-w")

target_include_directories(smart_ipc PRIVATE
    ${NNCASE_INCLUDE_DIR} ${OPENCV4_INCLUDE_DIRS})

target_link_directories(smart_ipc PRIVATE ${OPENCV4_LIBRARY_DIRS})

# The nncase libraries are static and refer back to each other, so the
# linker has to be allowed more than one pass over them. Only those three
# are named: opencv is shared and needs no group.
#
# imgcodecs is in there for Utils::dump_*_image(), which the debug modes
# use to write a frame out as a picture.
target_link_libraries(smart_ipc PRIVATE
    -Wl,--start-group
    ${NNCASE_RUNTIME_LIB}
    nncase.rt_modules.k230
    functional_k230
    mmz
    -Wl,--end-group
    opencv_imgcodecs
    opencv_imgproc
    opencv_core
    Threads::Threads)

# This package ships its own copy of the model rather than loading the
# face_detect demo's: that demo can be deconfigured or move its files and
# smart_ipc would start failing for no reason of its own. The file under
# models/ is byte-identical to the one face_detect installs.
install(FILES models/face_detection_320.kmodel
    DESTINATION share/smart_ipc)

install(TARGETS smart_ipc RUNTIME DESTINATION bin)
