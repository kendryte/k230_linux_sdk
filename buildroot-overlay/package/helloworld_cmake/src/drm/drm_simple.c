#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>
#include <xf86drm.h>
#include <xf86drmMode.h>
#include <drm_fourcc.h>

int main() {
    int fd = open("/dev/dri/card0", O_RDWR | O_CLOEXEC);
    if (fd < 0) return -1;

    // 启用 Universal Planes
    drmSetClientCap(fd, DRM_CLIENT_CAP_UNIVERSAL_PLANES, 1);

    // 获取资源
    drmModeRes *res = drmModeGetResources(fd);
    uint32_t conn_id = res->connectors[0];
    uint32_t crtc_id = res->crtcs[0];

    drmModeConnector *conn = drmModeGetConnector(fd, conn_id);
    int width = conn->modes[0].hdisplay;
    int height = conn->modes[0].vdisplay;
    drmModeFreeConnector(conn);

    // 动态查找 Primary Plane (支持 ARGB8888 格式)
    uint32_t plane_id = 0;
    drmModePlaneRes *pres = drmModeGetPlaneResources(fd);
    if (pres) {
        for (uint32_t i = 0; i < pres->count_planes; i++) {
            drmModePlane *p = drmModeGetPlane(fd, pres->planes[i]);
            if (!p) continue;

            // 检查是否支持 ARGB8888 (AR24) 格式
            for (uint32_t j = 0; j < p->count_formats; j++) {
                if (p->formats[j] == DRM_FORMAT_ARGB8888) {
                    plane_id = p->plane_id;
                    printf("[Found] Primary Plane: %u (supports ARGB8888)\n", plane_id);
                    goto found;
                }
            }
            drmModeFreePlane(p);
        }
    found:
        drmModeFreePlaneResources(pres);
    }

    if (!plane_id) {
        fprintf(stderr, "No Primary Plane with ARGB8888 found\n");
        drmModeFreeResources(res);
        close(fd);
        return -1;
    }

    drmModeFreeResources(res);

    // 创建 Dumb Buffer
    struct drm_mode_create_dumb creq = { .width = width, .height = height, .bpp = 32 };
    if (drmIoctl(fd, DRM_IOCTL_MODE_CREATE_DUMB, &creq) < 0) {
        perror("CREATE_DUMB failed");
        close(fd);
        return -1;
    }

    // 创建 Framebuffer
    uint32_t fb_id;
    uint32_t handles[4] = { creq.handle }, pitches[4] = { creq.pitch }, offsets[4] = { 0 };
    if (drmModeAddFB2(fd, width, height, DRM_FORMAT_ARGB8888, handles, pitches, offsets, &fb_id, 0) < 0) {
        perror("AddFB2 failed");
        close(fd);
        return -1;
    }

    // 映射显存
    struct drm_mode_map_dumb mreq = { .handle = creq.handle };
    if (drmIoctl(fd, DRM_IOCTL_MODE_MAP_DUMB, &mreq) < 0) {
        perror("MAP_DUMB failed");
        close(fd);
        return -1;
    }
    uint32_t *map = mmap(0, creq.size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, mreq.offset);

    // 显示纯红色
    for (int i = 0; i < width * height; i++) map[i] = 0xFFFF0000;

    // 设置 Plane
    if (drmModeSetPlane(fd, plane_id, crtc_id, fb_id, 0,
                        0, 0, width, height,
                        0, 0, width << 16, height << 16) < 0) {
        perror("SetPlane failed");
        close(fd);
        return -1;
    }

    printf("[+] Screen is RED. Sleeping 5s...\n");
    sleep(5);

    // 清理
    munmap(map, creq.size);
    drmModeRmFB(fd, fb_id);
    struct drm_mode_destroy_dumb dreq = { .handle = creq.handle };
    drmIoctl(fd, DRM_IOCTL_MODE_DESTROY_DUMB, &dreq);
    close(fd);

    return 0;
}
