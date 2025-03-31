/*
 * Copyright (c) CompanyNameMagicTag. 2023-2023. All rights reserved.
 * 文 件 名   : type.h
 * 作    者   : CompanyName
 * 生成日期   : 2023年1月11日
 * 功能描述   : 基本类型定义
 */


#ifndef __TYPES_H__
#define __TYPES_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


/*
 * 1 Other Header File Including
 */


/*
 * 2 Macro Definition
 */
#define WAPI_SUCCESS    0
#define WAPI_FAILED     -1
#define WAIFAILED      -2

#ifdef WAPI_STUB
#ifndef STATIC
#define STATIC
#endif
#else /* WAPI_STUB */
#ifndef STATIC
#define STATIC static
#endif
#endif /* WAPI_STUB */

typedef unsigned char    uint8;
typedef unsigned short   uint16;
typedef unsigned int     uint32;
typedef char             int8;
typedef short            int16;
typedef int              int32;





/*
 * 3 Enum Type Definition
 */


/*
 * 4 Global Variable Declaring
 */


/*
 * 5 Message Header Definition
 */


/*
 * 6 Message Definition
 */


/*
 * 7 STRUCT Type Definition
 */


/*
 * 8 UNION Type Definition
 */


/*
 * 9 OTHERS Definition
 */


/*
 * 10 Function Declare
 */


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of types.h */
