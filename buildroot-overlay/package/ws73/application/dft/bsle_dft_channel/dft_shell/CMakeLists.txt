set(
    sle_comm_src_list
    ${CMAKE_CURRENT_SOURCE_DIR}/msg_queue.c
    ${CMAKE_CURRENT_SOURCE_DIR}/bsle_dft_at_server.c
)
set(BTH_SLE_RAM_LIST "${BTH_SLE_RAM_LIST}" "${sle_comm_src_list}" CACHE INTERNAL "sle ram obj list" FORCE)
