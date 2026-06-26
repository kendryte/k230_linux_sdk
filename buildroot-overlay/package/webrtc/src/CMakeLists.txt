cmake_minimum_required(VERSION 3.5)
project(webrtc C)

# libpeer source root (from rtos reference)
set(LIBPEER_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/libpeer)




option(BUILD_SHARED_LIBS "Build shared libraries" OFF)

# Compile definitions matching rtos Makefile
add_definitions(
    -D_GNU_SOURCE
    -DCONFIG_IPV6=0
    -DCONFIG_USE_LWIP=0
    -DCONFIG_USE_USRSCTP=1
    -DHTTP_DO_NOT_USE_CUSTOM_CONFIG
    -DMQTT_DO_NOT_USE_CUSTOM_CONFIG
)

# Include directories for libpeer (rtos: headers live in src/)
include_directories(${LIBPEER_ROOT}/src)

# coreHTTP includes
include_directories(${LIBPEER_ROOT}/3rd-party/coreHTTP/source/include)
include_directories(${LIBPEER_ROOT}/3rd-party/coreHTTP/source/interface)
include_directories(${LIBPEER_ROOT}/3rd-party/coreHTTP/source/dependency/3rdparty/llhttp/include)

# coreMQTT includes
include_directories(${LIBPEER_ROOT}/3rd-party/coreMQTT/source/include)
include_directories(${LIBPEER_ROOT}/3rd-party/coreMQTT/source/interface)

# Private 3rd-party dependency includes
include_directories(${LIBPEER_ROOT}/3rd-party/mbedtls/include)
include_directories(${LIBPEER_ROOT}/3rd-party/libsrtp/include)
include_directories(${LIBPEER_ROOT}/3rd-party/libsrtp/crypto/include)
include_directories(${LIBPEER_ROOT}/3rd-party/usrsctp/usrsctplib)

# cJSON (from buildroot cjson package in staging sysroot)
include_directories(${WEBRTC_STAGING_DIR}/usr/include/cjson)

include(ExternalProject)

# Build and install directories for dependencies
set(DEP_INSTALL_DIR ${CMAKE_BINARY_DIR}/dep_install)

# ---------- mbedtls ----------
# Enable MBEDTLS_SSL_DTLS_SRTP (DTLS-SRTP extension required by WebRTC)
file(READ ${LIBPEER_ROOT}/3rd-party/mbedtls/include/mbedtls/mbedtls_config.h MBEDTLS_CONFIG_CONTENT)
string(REPLACE "//#define MBEDTLS_SSL_DTLS_SRTP" "#define MBEDTLS_SSL_DTLS_SRTP" MBEDTLS_CONFIG_MODIFIED "${MBEDTLS_CONFIG_CONTENT}")
file(WRITE ${LIBPEER_ROOT}/3rd-party/mbedtls/include/mbedtls/mbedtls_config.h "${MBEDTLS_CONFIG_MODIFIED}")

ExternalProject_Add(mbedtls
    SOURCE_DIR ${LIBPEER_ROOT}/3rd-party/mbedtls
    CMAKE_ARGS
        -DCMAKE_C_FLAGS=-fPIC
        -DENABLE_TESTING=off
        -DENABLE_PROGRAMS=off
        -DCMAKE_INSTALL_PREFIX=${DEP_INSTALL_DIR}
        -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
    BUILD_BYPRODUCTS
        ${DEP_INSTALL_DIR}/lib/libmbedtls.a
        ${DEP_INSTALL_DIR}/lib/libmbedcrypto.a
        ${DEP_INSTALL_DIR}/lib/libmbedx509.a
)

# ---------- libsrtp2 ----------
# rtos libsrtp uses mbedtls backend (aes_icm_mbedtls, aes_gcm_mbedtls, hmac_mbedtls)
ExternalProject_Add(srtp2
    SOURCE_DIR ${LIBPEER_ROOT}/3rd-party/libsrtp
    CMAKE_ARGS
        -DCMAKE_C_FLAGS=-fPIC
        -DTEST_APPS=off
        -DCMAKE_INSTALL_PREFIX=${DEP_INSTALL_DIR}
        -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
    BUILD_BYPRODUCTS ${DEP_INSTALL_DIR}/lib/libsrtp2.a
)

# ---------- usrsctp ----------
ExternalProject_Add(usrsctp
    SOURCE_DIR ${LIBPEER_ROOT}/3rd-party/usrsctp
    CMAKE_ARGS
        -DCMAKE_C_FLAGS=-fPIC
        -Dsctp_build_programs=off
        -Dsctp_inet6=off
        -DCMAKE_INSTALL_PREFIX=${DEP_INSTALL_DIR}
        -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
    BUILD_BYPRODUCTS ${DEP_INSTALL_DIR}/lib/libusrsctp.a
)

# ---------- libpeer (main library) ----------
# rtos source files (from rtos Makefile CFILES)
set(LIBPEER_CORE_SRCS
    ${LIBPEER_ROOT}/src/agent.c
    ${LIBPEER_ROOT}/src/base64.c
    ${LIBPEER_ROOT}/src/address.c
    ${LIBPEER_ROOT}/src/dtls_srtp.c
    ${LIBPEER_ROOT}/src/ice.c
    ${LIBPEER_ROOT}/src/mdns.c
    ${LIBPEER_ROOT}/src/peer.c
    ${LIBPEER_ROOT}/src/peer_connection.c
    ${LIBPEER_ROOT}/src/peer_signaling.c
    ${LIBPEER_ROOT}/src/ports.c
    ${LIBPEER_ROOT}/src/rtcp.c
    ${LIBPEER_ROOT}/src/rtp.c
    ${LIBPEER_ROOT}/src/sctp.c
    ${LIBPEER_ROOT}/src/sdp.c
    ${LIBPEER_ROOT}/src/socket.c
    ${LIBPEER_ROOT}/src/ssl_transport.c
    ${LIBPEER_ROOT}/src/stun.c
    ${LIBPEER_ROOT}/src/utils.c
)

# coreHTTP source files (from rtos Makefile)
set(HTTP_SRCS
    ${LIBPEER_ROOT}/3rd-party/coreHTTP/source/core_http_client.c
    ${LIBPEER_ROOT}/3rd-party/coreHTTP/source/dependency/3rdparty/llhttp/src/api.c
    ${LIBPEER_ROOT}/3rd-party/coreHTTP/source/dependency/3rdparty/llhttp/src/llhttp.c
    ${LIBPEER_ROOT}/3rd-party/coreHTTP/source/dependency/3rdparty/llhttp/src/http.c
)

# coreMQTT source files (from rtos Makefile)
set(MQTT_SRCS
    ${LIBPEER_ROOT}/3rd-party/coreMQTT/source/core_mqtt.c
    ${LIBPEER_ROOT}/3rd-party/coreMQTT/source/core_mqtt_state.c
    ${LIBPEER_ROOT}/3rd-party/coreMQTT/source/core_mqtt_serializer.c
)

add_library(peer STATIC
    ${LIBPEER_CORE_SRCS}
    ${HTTP_SRCS}
    ${MQTT_SRCS}
)

target_include_directories(peer PUBLIC
    ${LIBPEER_ROOT}/src
    ${DEP_INSTALL_DIR}/include
)

add_dependencies(peer mbedtls usrsctp srtp2)

# cjson is a shared library from buildroot's cjson package
link_directories(${WEBRTC_STAGING_DIR}/usr/lib)

target_link_libraries(peer
    ${DEP_INSTALL_DIR}/lib/libsrtp2.a
    ${DEP_INSTALL_DIR}/lib/libusrsctp.a
    ${DEP_INSTALL_DIR}/lib/libmbedtls.a
    ${DEP_INSTALL_DIR}/lib/libmbedcrypto.a
    ${DEP_INSTALL_DIR}/lib/libmbedx509.a
    cjson
    pthread
)

# Install libpeer static library
install(TARGETS peer ARCHIVE DESTINATION lib)

# Install dependency static libraries
install(FILES
    ${DEP_INSTALL_DIR}/lib/libsrtp2.a
    ${DEP_INSTALL_DIR}/lib/libusrsctp.a
    ${DEP_INSTALL_DIR}/lib/libmbedtls.a
    ${DEP_INSTALL_DIR}/lib/libmbedcrypto.a
    ${DEP_INSTALL_DIR}/lib/libmbedx509.a
    DESTINATION lib
)

# Install libpeer public headers (rtos: peer.h, peer_connection.h, peer_signaling.h)
install(FILES
    ${LIBPEER_ROOT}/src/peer.h
    ${LIBPEER_ROOT}/src/peer_connection.h
    ${LIBPEER_ROOT}/src/peer_signaling.h
    DESTINATION include/peer
)

# Install dependency headers
install(DIRECTORY ${DEP_INSTALL_DIR}/include/
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)
