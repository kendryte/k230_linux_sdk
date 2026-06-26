#ifndef CONFIG_H_
#define CONFIG_H_

#define PACKAGE_VERSION "2.4.2"
#define PACKAGE_STRING "libsrtp2 2.4.2"

/* #define ENABLE_DEBUG_LOGGING 1 */

#define MBEDTLS 1
#define GCM 1

#define CPU_CISC 1
/* #undef CPU_RISC */
/* #undef HAVE_X86 */

/* #undef WORDS_BIGENDIAN */

#define HAVE_ARPA_INET_H 1
#define HAVE_BYTESWAP_H 1
#define HAVE_INTTYPES_H 1
/* #undef HAVE_MACHINE_TYPES_H */
#define HAVE_NETINET_IN_H 1
#define HAVE_STDINT_H 1
#define HAVE_STDLIB_H 1
/* #undef HAVE_SYS_INT_TYPES_H */
#define HAVE_SYS_SOCKET_H 1
#define HAVE_SYS_TYPES_H 1
#define HAVE_UNISTD_H 1
/* #undef HAVE_WINDOWS_H */
/* #undef HAVE_WINSOCK2_H */

#define HAVE_SIGACTION 1
#define HAVE_INET_ATON 1
#define HAVE_USLEEP 1

#define HAVE_UINT8_T 1
#define HAVE_UINT16_T 1
#define HAVE_UINT32_T 1
#define HAVE_UINT64_T 1
#define HAVE_INT32_T 1

#define SIZEOF_UNSIGNED_LONG 8
#define SIZEOF_UNSIGNED_LONG_LONG 8

#define HAVE_INLINE 1

#endif /* CONFIG_H_ */
