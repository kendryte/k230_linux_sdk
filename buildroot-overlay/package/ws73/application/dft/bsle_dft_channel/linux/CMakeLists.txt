cmake_minimum_required(VERSION 3.16)
project("bp_channel")
get_filename_component(CURRENT_SOURCE_DIR "" ABSOLUTE)

if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/../../../../driver/platform)
   set(CMAKE_SEC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../../driver/platform/libc_sec)
   set(CMAKE_OSAL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../../driver/platform/osal)
   set(CMAKE_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../../)
elseif (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../utils/securec)
   set(CMAKE_SEC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../utils/securec)
   set(CMAKE_OSAL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../utils/osal)
   set(CMAKE_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../)
endif()

add_definitions(${defines} -D BTH_TEST -D CONFIG_GLE -D BT_MANAGER_DEPLOYED=2 -D BT_CODEC_TID=2 -D __BTC_INLINE=)

set(headers
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../dft_channel
    ${CMAKE_CURRENT_SOURCE_DIR}/../dft_shell
    ${CMAKE_SEC_DIR}/include
    ${CMAKE_OSAL_DIR}/linux/userspace/include
    ${CMAKE_ROOT_DIR}/driver/platform/drv/include
    ${CMAKE_ROOT_DIR}/utils/osal/linux/userspace/include/memory
    ${CMAKE_ROOT_DIR}/include/native
    ${CMAKE_CURRENT_SOURCE_DIR}/include/platform
    ${CMAKE_CURRENT_SOURCE_DIR}/include/product
)

if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/../../../../driver/platform)
    set(headers
        ${headers}
        ${CMAKE_OSAL_DIR}/include
    )
endif()

set(sources
    ${CMAKE_CURRENT_SOURCE_DIR}/../main.c
    ${CMAKE_SEC_DIR}/src/memcpy_s.c
    ${CMAKE_SEC_DIR}/src/memset_s.c
    ${CMAKE_SEC_DIR}/src/memmove_s.c
    ${CMAKE_SEC_DIR}/src/sprintf_s.c
    ${CMAKE_SEC_DIR}/src/vsprintf_s.c
    ${CMAKE_SEC_DIR}/src/strcpy_s.c
    ${CMAKE_SEC_DIR}/src/strncpy_s.c
    ${CMAKE_SEC_DIR}/src/strcat_s.c
    ${CMAKE_SEC_DIR}/src/secureprintoutput_a.c
)

add_subdirectory(../dft_channel ../build/channel_build)
add_subdirectory(../dft_shell ../build/shell_build)

# add_library生成.a和add_executable可执行文件二选一放开
# add_library(sparklinkd STATIC ${sources})
list(APPEND CFLAGS -s -fPIE -pie -W -Wall -Wl,-z,now -Wl,-z,relro -fstack-protector-all -Os)

add_executable(bp_channel ${sources} ${BTH_SLE_RAM_LIST})
target_include_directories(bp_channel PRIVATE ${headers})
target_compile_options(bp_channel PRIVATE ${CFLAGS})
target_link_libraries(bp_channel pthread ${CFLAGS})

#上板需要设置如下交叉编译链，本地可以通过删除交叉编译链配置进行验证
set(CMAKE_C_COMPILER ${WSCFG_CROSS_COMPILE}gcc)
set(CMAKE_CXX_COMPILER ${WSCFG_CROSS_COMPILE}g++)

message("CMAKE_CURRENT_SOURCE_DIR:" ${CMAKE_CURRENT_SOURCE_DIR})
