#include <stdio.h>
#include <vvcam_sensor.h>

extern struct vvcam_sensor vvcam_ov5647;
extern struct vvcam_sensor vvcam_imx335;

void vvcam_sensor_init(void) {
    // get /dev/media0
    printf("k230 builtin sensor driver, version %s %s\n", __DATE__, __TIME__);
    vvcam_sensor_add(&vvcam_ov5647);
    vvcam_sensor_add(&vvcam_imx335);
}
