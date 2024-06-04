#include <stdio.h>
#include <vvcam_sensor.h>

extern struct vvcam_sensor vvcam_ov5647;
extern struct vvcam_sensor vvcam_imx335;

void vvcam_sensor_init(void) {
    printf("sensor_init, vvcam_sensor_add: %p\n", vvcam_sensor_add);
    vvcam_sensor_add(&vvcam_ov5647);
    vvcam_sensor_add(&vvcam_imx335);
}
