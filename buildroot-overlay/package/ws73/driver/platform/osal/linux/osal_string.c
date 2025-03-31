/*
 * Copyright (c) CompanyNameMagicTag 2012-2021. All rights reserved.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/printk.h>
#include <linux/string.h>
#include <linux/version.h>
#include "soc_osal.h"

int osal_memncmp(const void *buf1, unsigned long size1, const void *buf2, unsigned long size2)
{
    if (size1 != size2) {
        return -1;
    }
    return memcmp(buf1, buf2, size1);
}
EXPORT_SYMBOL(osal_memncmp);
int osal_memncmp_tmp_new(const void *buf1, const void *buf2, unsigned long size)
{
    return memcmp(buf1, buf2, size);
}
EXPORT_SYMBOL(osal_memncmp_tmp_new);
int osal_strcmp(const char *cs, const char *ct)
{
    return strcmp(cs, ct);
}
EXPORT_SYMBOL(osal_strcmp);
int osal_strncmp(const char *s1, unsigned long size1, const char *s2, unsigned long size2)
{
    if (size1 != size2 || s1 == NULL || s2 == NULL) {
        return -1;
    }
    return strncmp(s1, s2, size1);
}
EXPORT_SYMBOL(osal_strncmp);
int osal_strncmp_tmp_new(const char *str1, const char *str2, unsigned long size)
{
    return strncmp(str1, str2, size);
}
EXPORT_SYMBOL(osal_strncmp_tmp_new);
int osal_strcasecmp(const char *s1, const char *s2)
{
    return strcasecmp(s1, s2);
}
EXPORT_SYMBOL(osal_strcasecmp);
int osal_strncasecmp(const char *s1, unsigned long size1, const char *s2, unsigned long size2)
{
    if (size1 != size2) {
        return -1;
    }
    return strncasecmp(s1, s2, size1);
}
EXPORT_SYMBOL(osal_strncasecmp);
int osal_strncasecmp_tmp_new(const char *str1, const char *str2, unsigned long size)
{
    return strncasecmp(str1, str2, size);
}
EXPORT_SYMBOL(osal_strncasecmp_tmp_new);
char *osal_strchr(const char *s, int c)
{
    return strchr(s, c);
}
EXPORT_SYMBOL(osal_strchr);
char *osal_strnchr(const char *s, int count, int c)
{
    return strnchr(s, count, c);
}
EXPORT_SYMBOL(osal_strnchr);
char *osal_strrchr(const char *s, int c)
{
    return strrchr(s, c);
}
EXPORT_SYMBOL(osal_strrchr);
char *osal_strstr(const char *s1, const char *s2)
{
    return strstr(s1, s2);
}
EXPORT_SYMBOL(osal_strstr);
char *osal_strnstr(const char *s1, const char *s2, int len)
{
    return strnstr(s1, s2, len);
}
EXPORT_SYMBOL(osal_strnstr);
int osal_strlen(const char *s)
{
    return strlen(s);
}
EXPORT_SYMBOL(osal_strlen);
int osal_strnlen(const char *s, int count)
{
    return strnlen(s, count);
}
EXPORT_SYMBOL(osal_strnlen);
char *osal_strpbrk(const char *cs, const char *ct)
{
    return strpbrk(cs, ct);
}
EXPORT_SYMBOL(osal_strpbrk);
char *osal_strsep(char **s, const char *ct)
{
    return strsep(s, ct);
}
EXPORT_SYMBOL(osal_strsep);
int osal_strspn(const char *s, const char *accept)
{
    return strspn(s, accept);
}
EXPORT_SYMBOL(osal_strspn);
int osal_strcspn(const char *s, const char *reject)
{
    return strcspn(s, reject);
}
EXPORT_SYMBOL(osal_strcspn);
void *osal_memscan(void *addr, int c, int size)
{
    return memscan(addr, c, size);
}
EXPORT_SYMBOL(osal_memscan);
int osal_memcmp(const void *cs, const void *ct, int count)
{
    return memcmp(cs, ct, count);
}
EXPORT_SYMBOL(osal_memcmp);
void *osal_memchr(const void *s, int c, int n)
{
    return memchr(s, c, n);
}
EXPORT_SYMBOL(osal_memchr);
void *osal_memchr_inv(const void *s, int c, int n)
{
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3,2,0))
    return memchr_inv(s, c, n);
#else
    return NULL;
#endif
}
EXPORT_SYMBOL(osal_memchr_inv);
unsigned long long osal_strtoull(const char *cp, char **endp, unsigned int base)
{
    return simple_strtoull(cp, endp, base);
}
EXPORT_SYMBOL(osal_strtoull);
unsigned long osal_strtoul(const char *cp, char **endp, unsigned int base)
{
    return simple_strtoul(cp, endp, base);
}
EXPORT_SYMBOL(osal_strtoul);
long osal_strtol(const char *cp, char **endp, unsigned int base)
{
    return simple_strtol(cp, endp, base);
}
EXPORT_SYMBOL(osal_strtol);
long long osal_strtoll(const char *cp, char **endp, unsigned int base)
{
    return simple_strtoll(cp, endp, base);
}
EXPORT_SYMBOL(osal_strtoll);

