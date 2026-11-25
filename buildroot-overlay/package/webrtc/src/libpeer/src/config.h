#ifndef CONFIG_H_
#define CONFIG_H_

#define SCTP_MTU (1200)
#define CONFIG_MTU (1300)

#ifndef CONFIG_USE_LWIP
#define CONFIG_USE_LWIP 0
#endif

#ifndef CONFIG_USE_HWRNG
#define CONFIG_USE_HWRNG 0
#endif

#ifndef CONFIG_USE_GETIFADDRS
#define CONFIG_USE_GETIFADDRS 1
#endif

#ifndef CONFIG_MBEDTLS_DEBUG
#define CONFIG_MBEDTLS_DEBUG 0
#endif

#ifndef CONFIG_MBEDTLS_2_X
#define CONFIG_MBEDTLS_2_X 0
#endif

#if CONFIG_MBEDTLS_2_X
#define RSA_KEY_LENGTH 512
#else
#define RSA_KEY_LENGTH 1024
#endif

#ifndef CONFIG_DTLS_USE_ECDSA
#define CONFIG_DTLS_USE_ECDSA 1
#endif

#ifndef CONFIG_USE_USRSCTP
#define CONFIG_USE_USRSCTP 1
#endif

#ifndef CONFIG_SDP_BUFFER_SIZE
#define CONFIG_SDP_BUFFER_SIZE 8096
#endif

#ifndef CONFIG_MQTT_BUFFER_SIZE
#define CONFIG_MQTT_BUFFER_SIZE 4096
#endif

#ifndef CONFIG_HTTP_BUFFER_SIZE
#define CONFIG_HTTP_BUFFER_SIZE 4096
#endif

/* Bounds how long a single mbedtls_ssl_read() may block waiting for data
 * (see ssl_transport_mbedtls_recv_timeout()). peer_signaling_loop() calls
 * into this on every iteration of a 10ms-sleep poll loop while holding
 * g_mqtt_mutex, and signaling traffic is bursty (idle most of the time) —
 * so a large timeout here does not make the connection more reliable, it
 * just means an idle iteration blocks everyone waiting on that mutex
 * (heartbeat/ICE-candidate publish) for up to this long: at the original
 * 3000ms, the idle select() dominated almost the entire poll cycle, so a
 * publish landed behind an in-flight wait almost every time. 20ms caps
 * that worst case at a tiny fraction of the original while still leaving
 * "no data yet" returning fast relative to the 10ms sleep that governs the
 * actual poll cadence.
 *
 * This value only applies once MQTT CONNACK and SUBACK are both confirmed —
 * peer_signaling_connect() calls ssl_transport_set_read_timeout() to switch
 * over at that point. See CONFIG_TLS_HANDSHAKE_READ_TIMEOUT for why the
 * handshake and the CONNACK/SUBACK wait use a separate, longer timeout:
 * MQTT_ProcessLoop() only does a single, non-retrying transport read, so
 * switching to this short timeout any earlier starves it before the
 * SUBACK can arrive. */
#ifndef CONFIG_TLS_READ_TIMEOUT
#define CONFIG_TLS_READ_TIMEOUT 20
#endif

/* mbedtls_ssl_conf_read_timeout() is a single config-wide value used by
 * every blocking read on the connection, including the reads inside
 * mbedtls_ssl_handshake() itself. The handshake's round trips cross a real
 * network (cellular RTT can run well past 20ms), so reusing the short
 * steady-state CONFIG_TLS_READ_TIMEOUT here makes mbedtls_ssl_handshake()
 * time out and silently re-enter itself over and over before the server's
 * response actually lands — harmless (it is resumable), but it spams
 * "ssl handshake error: -0x6800" (MBEDTLS_ERR_SSL_TIMEOUT) and busy-polls
 * instead of blocking once for the real RTT. ssl_transport_connect() uses
 * this generous value for the handshake; peer_signaling_connect() keeps
 * using it through MQTT_Connect() and MQTT_ProcessLoop()'s SUBACK wait too,
 * only switching down to CONFIG_TLS_READ_TIMEOUT once both succeed. */
#ifndef CONFIG_TLS_HANDSHAKE_READ_TIMEOUT
#define CONFIG_TLS_HANDSHAKE_READ_TIMEOUT 3000
#endif

#ifndef CONFIG_KEEPALIVE_TIMEOUT
#define CONFIG_KEEPALIVE_TIMEOUT 10000
#endif

#ifndef CONFIG_AUDIO_DURATION
#define CONFIG_AUDIO_DURATION 20
#endif

#ifndef CONFIG_MAX_NALU_SIZE
#define CONFIG_MAX_NALU_SIZE (10 * 1024)  // 10KB
#endif

#define CONFIG_IPV6 0
// empty will use first active interface
#define CONFIG_IFACE_PREFIX ""

// #define LOG_LEVEL LEVEL_DEBUG
#ifndef LOG_REDIRECT
#define LOG_REDIRECT 0
#endif

// Disable MQTT and HTTP signaling
// #define DISABLE_PEER_SIGNALING 1

#endif  // CONFIG_H_
