#include <errno.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "config.h"

#if CONFIG_USE_LWIP
#include "lwip/ip_addr.h"
#include "lwip/netdb.h"
#include "lwip/netif.h"
#include "lwip/sys.h"
#else
#include <ifaddrs.h>
#include <net/if.h>
#include <netdb.h>
#include <sys/ioctl.h>
#endif

#include "ports.h"
#include "utils.h"

int ports_get_host_addr(Address* addr, const char* iface_prefix) {
  int ret = 0;

#if CONFIG_USE_LWIP
  struct netif* netif;
  int i;
  for (netif = netif_list; netif != NULL; netif = netif->next) {
    switch (addr->family) {
      case AF_INET6:
        for (i = 0; i < LWIP_IPV6_NUM_ADDRESSES; i++) {
          if (!ip6_addr_isany(netif_ip6_addr(netif, i))) {
            memcpy(&addr->sin6.sin6_addr, netif_ip6_addr(netif, i), 16);
            ret = 1;
            break;
          }
        }
        break;
      case AF_INET:
      default:
        if (!ip_addr_isany(&netif->ip_addr)) {
          memcpy(&addr->sin.sin_addr, &netif->ip_addr.u_addr.ip4, 4);
          ret = 1;
        }
        break;
    }

    if (ret) {
      break;
    }
  }
#else
  /* If iface_prefix is specified, use ioctl as before (for K230 etc.).
   * Otherwise, use getifaddrs to enumerate all interfaces and pick
   * the first UP non-loopback IPv4 address — works on any Linux. */
  if (iface_prefix && strlen(iface_prefix) > 0) {
    int sock;
    struct ifreq ifr;

    if (addr->family != AF_INET) {
      return 0;
    }

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
      LOGE("Failed to create socket for get_host_addr: %s", strerror(errno));
      return -1;
    }

    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, iface_prefix, sizeof(ifr.ifr_name) - 1);

    if (ioctl(sock, SIOCGIFADDR, &ifr) == 0) {
      memcpy(&addr->sin, &ifr.ifr_addr, sizeof(struct sockaddr_in));
      ret = 1;
    } else {
      LOGD("SIOCGIFADDR failed for %s: %s", ifr.ifr_name, strerror(errno));
    }

    close(sock);
  } else {
    static const char* ifnames[] = {"u0", "e0", "eth0", "en0", "w0", "wlan0", NULL};
    struct ifaddrs* ifaddr;
    struct ifaddrs* ifa;

    if (addr->family != AF_INET) {
      return 0;
    }

    if (getifaddrs(&ifaddr) == 0) {
      for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL) continue;
        if (ifa->ifa_flags & IFF_LOOPBACK) continue;
        if (ifa->ifa_addr->sa_family != AF_INET) continue;
        if (!(ifa->ifa_flags & IFF_UP)) continue;

        memcpy(&addr->sin, ifa->ifa_addr, sizeof(struct sockaddr_in));
        ret = 1;
        break;
      }

      freeifaddrs(ifaddr);
      if (ret) {
        return ret;
      }
    }

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
      LOGE("Failed to create socket for get_host_addr: %s", strerror(errno));
      return -1;
    }

    struct ifreq ifr;
    for (int i = 0; ifnames[i] != NULL; i++) {
      memset(&ifr, 0, sizeof(ifr));
      strncpy(ifr.ifr_name, ifnames[i], sizeof(ifr.ifr_name) - 1);

      if (ioctl(sock, SIOCGIFADDR, &ifr) == 0) {
        struct sockaddr_in* sin = (struct sockaddr_in*)&ifr.ifr_addr;
        if (sin->sin_addr.s_addr != htonl(INADDR_ANY)) {
          memcpy(&addr->sin, sin, sizeof(struct sockaddr_in));
          ret = 1;
          LOGI("Got host IP from %s", ifnames[i]);
          break;
        }
      }
    }

    close(sock);
  }
#endif
  return ret;
}

int ports_resolve_addr(const char* host, Address* addr) {
  char addr_string[ADDRSTRLEN];
  int ret = -1;

  addr_set_family(addr, AF_INET);

  struct in_addr ipv4;
  LOGI("ports_resolve_addr: host='%s'", host);
  if (inet_pton(AF_INET, host, &ipv4) == 1) {
    addr->sin.sin_family = AF_INET;
    addr->sin.sin_addr = ipv4;
    addr_to_string(addr, addr_string, sizeof(addr_string));
    LOGI("Resolved %s -> %s (inet_pton)", host, addr_string);
    return 0;
  }

  struct addrinfo hints, *res, *p;
  int status;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  if ((status = getaddrinfo(host, NULL, &hints, &res)) != 0) {
    LOGE("getaddrinfo error: %d\n", status);
    return ret;
  }

  for (p = res; p != NULL; p = p->ai_next) {
    if (p->ai_family == addr->family) {
      switch (addr->family) {
        case AF_INET6:
          memcpy(&addr->sin6, p->ai_addr, sizeof(struct sockaddr_in6));
          break;
        case AF_INET:
        default:
          memcpy(&addr->sin, p->ai_addr, sizeof(struct sockaddr_in));
          break;
      }
      ret = 0;
    }
  }

  addr_to_string(addr, addr_string, sizeof(addr_string));
  LOGI("Resolved %s -> %s (getaddrinfo)", host, addr_string);
  freeaddrinfo(res);
  return ret;
}

uint32_t ports_get_epoch_time() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return (uint32_t)tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

void ports_sleep_ms(int ms) {
#if CONFIG_USE_LWIP
  sys_msleep(ms);
#else
  usleep(ms * 1000);
#endif
}
