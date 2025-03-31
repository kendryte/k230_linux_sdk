#===============================================================================
# @brief    cmake file
# Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2023-2023. All rights reserved.
#===============================================================================
set(SOURCES "${SOURCES}"
    "${CMAKE_CURRENT_SOURCE_DIR}/ble_server_adv.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/ble_uuid_server.c"
    PARENT_SCOPE)
