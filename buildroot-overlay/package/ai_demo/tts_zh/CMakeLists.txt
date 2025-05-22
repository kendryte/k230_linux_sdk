cmake_minimum_required(VERSION 3.10)
if(CMAKE_TOOLCHAIN_FILE MATCHES "buildroot/toolchainfile.cmake")
    get_filename_component(CURRENT_DIR_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
    project(${CURRENT_DIR_NAME})
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}   -mcpu=c908v -mabi=lp64d  -mtune=c908 -mrvv-v0p10-compatible  -mrvv-auto-vectorize")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}   -mcpu=c908v -mabi=lp64d  -mtune=c908 -mrvv-v0p10-compatible  -mrvv-auto-vectorize")

    #set(src main.cc  anomaly_det.cc)
    include_directories(include)

    # add_subdirectory(paddlespeech_zhfrontend)
    add_subdirectory(src)
    add_subdirectory(tts_src)


else()

    # include_directories(paddlespeech_zhfrontend)
    # include_directories(paddlespeech_zhfrontend/pypinyin)
    # include_directories(paddlespeech_zhfrontend/zh_normalization)
    # include_directories(paddlespeech_zhfrontend/cppjieba/include)
    # include_directories(paddlespeech_zhfrontend/cppjieba/deps)
    # include_directories(paddlespeech_zhfrontend/cppjieba/deps/limonp/include)
    include_directories(include)

    # add_subdirectory(paddlespeech_zhfrontend)
    add_subdirectory(src)
    add_subdirectory(tts_src)
endif()
