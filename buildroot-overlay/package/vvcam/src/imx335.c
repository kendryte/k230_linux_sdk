#include <vvcam_sensor.h>

static int init(void** ctx) {
    return 0;
}

static void deinit(void** ctx) {

}

struct vvcam_sensor vvcam_imx335 = {
    .name = "imx335",
    .ctrl = {
        .init = init,
        .deinit = deinit,
        .enum_mode = 0,
        .get_mode = 0,
        .set_mode = 0,
        .set_stream = 0,
        .set_analog_gain = 0,
        .set_digital_gain = 0,
        .set_int_time = 0
    }
};
