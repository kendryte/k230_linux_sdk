#include <display.h>
#include <drm_fourcc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xf86drmMode.h>

#define pr(fmt, ...) fprintf(stderr, "[three-camera] " fmt "\n", ##__VA_ARGS__)

static bool plane_in_use(const struct display* display, uint32_t plane_id)
{
    const struct display_plane* p = display->planes;

    while (p != NULL) {
        if (p->plane_id == plane_id) {
            return true;
        }
        p = p->next;
    }
    return false;
}

static bool plane_supports_fourcc(struct display* display, drmModePlanePtr plane,
                                  unsigned int fourcc)
{
    for (unsigned j = 0; j < plane->count_formats; j++) {
        if (plane->formats[j] != fourcc) {
            continue;
        }
        if (fourcc == DRM_FORMAT_NV12) {
            if ((display->drm_rotation == rotation_90) ||
                (display->drm_rotation == rotation_270)) {
                drmModeObjectPropertiesPtr props_ptr =
                    drmModeObjectGetProperties(display->fd, plane->plane_id,
                                               DRM_MODE_OBJECT_PLANE);
                if (props_ptr == NULL) {
                    return false;
                }
                for (unsigned k = 0; k < props_ptr->count_props; k++) {
                    drmModePropertyPtr prop =
                        drmModeGetProperty(display->fd, props_ptr->props[k]);
                    if (prop && strcmp(prop->name, "rotation") == 0) {
                        drmModeFreeObjectProperties(props_ptr);
                        return true;
                    }
                }
                drmModeFreeObjectProperties(props_ptr);
                continue;
            }
        }
        return true;
    }
    return false;
}

static struct display_plane* attach_drm_plane(struct display* display, drmModePlanePtr plane,
                                              unsigned int fourcc)
{
    drmModeObjectPropertiesPtr props =
        drmModeObjectGetProperties(display->fd, plane->plane_id, DRM_MODE_OBJECT_PLANE);
    struct display_plane* display_plane;

    if (props == NULL) {
        pr("get plane props failed");
        return NULL;
    }

    display_plane = malloc(sizeof(struct display_plane));
    if (!display_plane) {
        drmModeFreeObjectProperties(props);
        return NULL;
    }
    display_plane->next = NULL;
    for (unsigned i = 0; i < props->count_props; i++) {
        display_plane->props[i] = drmModeGetProperty(display->fd, props->props[i]);
    }
    display_plane->props_count = props->count_props;
    drmModeFreeObjectProperties(props);

    display_plane->display = display;
    display_plane->fourcc = fourcc;
    display_plane->plane_id = plane->plane_id;
    display_plane->plane = plane;
    display_plane->buffers = NULL;
    display_plane->next = display->planes;
    display->planes = display_plane;
    display_plane->first = true;
    return display_plane;
}

struct display_plane* three_camera_get_video_plane(struct display* display, unsigned int fourcc,
                                                 unsigned video_layer)
{
    drmModePlaneResPtr planes;
    drmModePlanePtr plane;
    uint32_t match_ids[3];
    unsigned match_count = 0;
    unsigned pick;

    if (video_layer < 1 || video_layer > 3) {
        pr("video_layer must be 1..3");
        return NULL;
    }

    planes = drmModeGetPlaneResources(display->fd);
    if (!planes) {
        pr("drmModeGetPlaneResources error");
        return NULL;
    }

    for (unsigned i = 0; i < planes->count_planes; i++) {
        plane = drmModeGetPlane(display->fd, planes->planes[i]);
        if (!plane) {
            pr("drmModeGetPlane error");
            break;
        }
        if (!(plane->possible_crtcs & (1 << display->crtc_idx)) ||
            !plane_supports_fourcc(display, plane, fourcc)) {
            drmModeFreePlane(plane);
            continue;
        }
        if (match_count < 3) {
            match_ids[match_count++] = plane->plane_id;
        }
        drmModeFreePlane(plane);
    }
    drmModeFreePlaneResources(planes);

    pick = 3 - video_layer;
    if (match_count <= pick) {
        pr("video layer %u not available (found %u YUV planes)", video_layer, match_count);
        return NULL;
    }
    if (plane_in_use(display, match_ids[pick])) {
        pr("video layer %u plane_id %u already in use", video_layer, match_ids[pick]);
        return NULL;
    }

    plane = drmModeGetPlane(display->fd, match_ids[pick]);
    if (!plane) {
        pr("drmModeGetPlane error for layer %u", video_layer);
        return NULL;
    }

    return attach_drm_plane(display, plane, fourcc);
}
