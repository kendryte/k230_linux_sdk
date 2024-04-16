#include <vvcam_sensor.h>

static int init(void** ctx) {
    return 0;
}

static void deinit(void* ctx) {

}

static int enum_mode(void* ctx, uint32_t index, struct vvcam_sensor_mode* mode) {
    return 0;
}

static int get_mode(void* ctx, struct vvcam_sensor_mode* mode) {
    return 0;
}

static int set_mode(void* ctx, struct vvcam_sensor_mode* mode) {
    return 0;
}

static int set_stream(void* ctx, bool on) {
    return 0;
}

static int set_analog_gain(void* ctx, float gain) {
    return 0;
}

static int set_digital_gain(void* ctx, float gain) {
    return 0;
}

static int set_int_time(void* ctx, float gain) {
    return 0;
}

struct vvcam_sensor vvcam_imx335 = {
    .name = "imx335",
    .ctrl = {
        .init = init,
        .deinit = deinit,
        .enum_mode = enum_mode,
        .get_mode = get_mode,
        .set_mode = set_mode,
        .set_stream = set_stream,
        .set_analog_gain = set_analog_gain,
        .set_digital_gain = set_digital_gain,
        .set_int_time = set_int_time
    }
};
