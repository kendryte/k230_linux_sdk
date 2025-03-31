/*
 * Copyright (c) @CompanyNameMagicTag 2014-2018. All rights reserved.
 * Description: Provides internal functions used by this library, such as memory
 *              copy and memory move. Besides, include some helper function for
 *              printf family API, such as SecVsnprintfImpl
 * Author: lishunda
 * Create: 2014-02-25
 */

/* Avoid duplicate header files,not include securecutil.h */
#include "securecutil.h"

#if defined(ANDROID) && (SECUREC_HAVE_WCTOMB || SECUREC_HAVE_MBTOWC)
#include <wchar.h>
#if SECUREC_HAVE_WCTOMB
/*
 * Convert wide characters to narrow multi-bytes
 */
int wctomb(char *s, wchar_t wc)
{
    return wcrtomb(s, wc, NULL);
}
#endif

#if SECUREC_HAVE_MBTOWC
/*
 * Converting narrow multi-byte characters to wide characters
 */
int mbtowc(wchar_t *pwc, const char *s, size_t n)
{
    return mbrtowc(pwc, s, n, NULL);
}
#endif
#endif

/* The version num is 0x5 (High 8 bits) */
#define SECUREC_C_VERSION     0x500U
#define SECUREC_SPC_VERSION   0xaU
#define SECUREC_VERSION_STR   "Secure C"

/*
 * Get version string and version number.
 * The rules for version number are as follows:
 * 1) SPC verNumber<->verStr like:
 * 0x201<->C01
 * 0x202<->C01SPC001   Redefine numbers after this version
 * 0x502<->C01SPC002
 * 0x503<->C01SPC003
 * ...
 * 0X50a<->SPC010
 * 0X50b<->SPC011
 * ...
 * 0x700<->C02
 * 0x701<->C01SPC001
 * 0x702<->C02SPC002
 * ...
 * 2) CP verNumber<->verStr like:
 * 0X601<->CP0001
 * 0X602<->CP0002
 * ...
 */
const char *GetHwSecureCVersion(unsigned short *verNumber)
{
    if (verNumber != NULL) {
        *verNumber = (unsigned short)(SECUREC_C_VERSION | SECUREC_SPC_VERSION);
    }
    return SECUREC_VERSION_STR;
}
#if SECUREC_IN_KERNEL
EXPORT_SYMBOL(GetHwSecureCVersion);
#endif

