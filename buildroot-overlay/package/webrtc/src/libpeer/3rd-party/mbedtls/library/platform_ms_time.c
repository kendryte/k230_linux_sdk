#include "mbedtls/platform_time.h"
#include <sys/time.h>
#include <time.h>

mbedtls_ms_time_t mbedtls_ms_time(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (mbedtls_ms_time_t)tv.tv_sec * 1000 + (mbedtls_ms_time_t)tv.tv_usec / 1000;
}
