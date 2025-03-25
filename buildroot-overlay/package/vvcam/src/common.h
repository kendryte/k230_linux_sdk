#pragma once

#ifndef __COMMON_H__
#define __COMMON_H__

#define COLOR_NONE "\033[0m"
#define RED "\033[1;31;40m"
#define BLUE "\033[1;34;40m"
#define GREEN "\033[1;32;40m"
#define YELLOW "\033[1;33;40m"

#define LOG_LEVEL 4
#define pr_verb(fmt, ...)
#define pr_info(fmt, ...)
#define pr_warn(fmt, ...)
#define pr_err(fmt, ...)

#if LOG_LEVEL >= 4
#undef pr_verb
#define pr_verb(fmt, ...) fprintf(stderr,BLUE "[dw] " fmt "\n" COLOR_NONE, ##__VA_ARGS__)
#endif

#if LOG_LEVEL >= 3
#undef pr_info
#define pr_info(fmt, ...) fprintf(stderr,GREEN "[dw] " fmt "\n" COLOR_NONE, ##__VA_ARGS__)
#endif

#if LOG_LEVEL >= 2
#undef pr_warn
#define pr_warn(fmt, ...) fprintf(stderr,YELLOW "[dw] " fmt "\n" COLOR_NONE, ##__VA_ARGS__)
#endif

#if LOG_LEVEL >= 1
#undef pr_err
#define pr_err(fmt, ...) fprintf(stderr,RED "[dw] " fmt "\n" COLOR_NONE, ##__VA_ARGS__)
#endif

#define CKE(x,e) do{pr_verb("enter %s", #x);int v=x;if((v)!=0){pr_err("error %d(%s) at %s:%d",v,strerror(errno),__FILE_NAME__,__LINE__);ret=-1;goto e;}}while(0)

#endif