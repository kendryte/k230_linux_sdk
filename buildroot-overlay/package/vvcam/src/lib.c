#include <stdio.h>
#include <vvcam_sensor.h>

extern struct vvcam_sensor vvcam_ov5647;
extern struct vvcam_sensor vvcam_imx335;
extern struct vvcam_sensor vvcam_gc2093;
extern struct vvcam_sensor vvcam_gc2053;
extern struct vvcam_sensor vvcam_bf3238;
extern struct vvcam_sensor vvcam_ov13850;


void vvcam_sensor_init(void) {
    // get /dev/media0
    printf("k230 builtin sensor driver, built %s %s, API version %lu\n", __DATE__, __TIME__, VVCAM_API_VERSION);
    vvcam_sensor_add(&vvcam_ov5647);
    vvcam_sensor_add(&vvcam_imx335);
    vvcam_sensor_add(&vvcam_gc2093);
    vvcam_sensor_add(&vvcam_gc2053);
    vvcam_sensor_add(&vvcam_bf3238);
    vvcam_sensor_add(&vvcam_ov13850);
}

int vvcam_sensor_find_mode_by_size_fps(struct vvcam_sensor *sensor, void *ctx,
    uint16_t width, uint16_t height, uint32_t fps)
{
    int fallback = -1;
    int exact = -1;

    if (!sensor || !sensor->ctrl.enum_mode) {
        return -1;
    }

    for (uint32_t i = 0; ; i++) {
        struct vvcam_sensor_mode mode;

        if (sensor->ctrl.enum_mode(ctx, i, &mode)) {
            break;
        }

        if (mode.width != width || mode.height != height) {
            continue;
        }

        if (fallback < 0) {
            fallback = (int)i;
        }

        if (mode.ae_info.cur_fps == fps) {
            exact = (int)i;
            break;
        }
    }

    return (exact >= 0) ? exact : fallback;
}
