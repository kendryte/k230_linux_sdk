cmake_minimum_required(VERSION 3.5)
project(usrsctp C)

option(sctp_build_programs "Build programs" OFF)
option(sctp_inet6 "Enable IPv6" OFF)

add_definitions(
    -D__Userspace__
    -DSCTP_SIMPLE_ALLOCATOR
    -DSCTP_PROCESS_LEVEL_LOCKS
    -DSCTP_DEBUG
    -DINET
    -D_GNU_SOURCE
    -DHAVE_STDATOMIC_H
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/usrsctplib)

set(USRSCTP_SRCS
    usrsctplib/netinet/sctp_asconf.c
    usrsctplib/netinet/sctp_auth.c
    usrsctplib/netinet/sctp_bsd_addr.c
    usrsctplib/netinet/sctp_callout.c
    usrsctplib/netinet/sctp_cc_functions.c
    usrsctplib/netinet/sctp_crc32.c
    usrsctplib/netinet/sctp_indata.c
    usrsctplib/netinet/sctp_input.c
    usrsctplib/netinet/sctp_output.c
    usrsctplib/netinet/sctp_pcb.c
    usrsctplib/netinet/sctp_peeloff.c
    usrsctplib/netinet/sctp_sha1.c
    usrsctplib/netinet/sctp_ss_functions.c
    usrsctplib/netinet/sctp_sysctl.c
    usrsctplib/netinet/sctp_timer.c
    usrsctplib/netinet/sctp_userspace.c
    usrsctplib/netinet/sctp_usrreq.c
    usrsctplib/netinet/sctputil.c
    usrsctplib/netinet6/sctp6_usrreq.c
    usrsctplib/user_environment.c
    usrsctplib/user_mbuf.c
    usrsctplib/user_recv_thread.c
    usrsctplib/user_socket.c
)

add_library(usrsctp STATIC ${USRSCTP_SRCS})

install(TARGETS usrsctp ARCHIVE DESTINATION lib)

install(FILES usrsctplib/usrsctp.h DESTINATION include)
