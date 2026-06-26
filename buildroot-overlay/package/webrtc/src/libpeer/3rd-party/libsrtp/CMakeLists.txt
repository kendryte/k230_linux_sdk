cmake_minimum_required(VERSION 3.5)
project(srtp2 C)

option(TEST_APPS "Build test apps." OFF)

add_definitions(
    -DHAVE_CONFIG_H
    -D_GNU_SOURCE
)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/crypto/include
)

# Find mbedtls headers (sibling directory)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../mbedtls/include)

set(SRTP_SRCS
    srtp/srtp.c
    crypto/cipher/cipher.c
    crypto/cipher/cipher_test_cases.c
    crypto/cipher/null_cipher.c
    crypto/cipher/aes_icm_mbedtls.c
    crypto/cipher/aes_gcm_mbedtls.c
    crypto/hash/auth.c
    crypto/hash/auth_test_cases.c
    crypto/hash/null_auth.c
    crypto/hash/hmac_mbedtls.c
    crypto/kernel/alloc.c
    crypto/kernel/crypto_kernel.c
    crypto/kernel/err.c
    crypto/kernel/key.c
    crypto/math/datatypes.c
    crypto/replay/rdb.c
    crypto/replay/rdbx.c
)

add_library(srtp2 STATIC ${SRTP_SRCS})

install(TARGETS srtp2 ARCHIVE DESTINATION lib)

install(DIRECTORY include/ DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)
install(DIRECTORY crypto/include/ DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)
