/*
 * Copyright (c) @CompanyNameMagicTag 2014-2018. All rights reserved.
 * Description: By defining data type for ANSI string and including "input.inl",
 *              this file generates real underlying function used by scanf family API.
 * Author: lishunda
 * Create: 2014-02-25
 */

#define SECUREC_FORMAT_OUTPUT_INPUT 1
#ifdef SECUREC_FOR_WCHAR
#undef SECUREC_FOR_WCHAR
#endif

#include "secinput.h"

#include "input.inl"

SECUREC_INLINE int SecIsDigit(SecInt ch)
{
    /* SecInt to unsigned char clear  571, use bit mask to clear negative return of ch */
    return isdigit((int)((unsigned int)(unsigned char)(ch) & 0xffU));
}
SECUREC_INLINE int SecIsXdigit(SecInt ch)
{
    return isxdigit((int)((unsigned int)(unsigned char)(ch) & 0xffU));
}
SECUREC_INLINE int SecIsSpace(SecInt ch)
{
    return isspace((int)((unsigned int)(unsigned char)(ch) & 0xffU));
}

