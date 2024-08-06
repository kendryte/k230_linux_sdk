#pragma once
#ifndef __COMMON_H__
#define __COMMON_H__

#define pr(fmt,...) fprintf(stderr,"[v4l2_drm] "fmt"\n", ##__VA_ARGS__)
#define CKE(x,go) do{int e=(x);if(e){pr(""#x" error %d(%s) at line %d",errno,strerror(errno),__LINE__);goto go;}}while(0)

#endif