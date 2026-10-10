#include "display.h"
#include "v4l2-drm.h"
#include "common.h"
#include <bits/types/struct_timeval.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>


static void help(const char* argv0) {
    printf("Usage: %s -d 1 -w 480 -h 320\n", argv0);
    printf(
        "Options:\n"
        "\t-d Video device number\n"
        "\t-w Width\n"
        "\t-h Height\n"
        "\t-n Buffer number\n"
        "\t-f Format, NV12/NV16\n"
        "\t-s Disable display\n"
        "\t-x | --crop-x Crop offset X\n"
        "\t-y | --crop-y Crop offset Y\n"
        "\t--crop-width  Crop width\n"
        "\t--crop-height Crop height\n"
        "\t--hflip 0/1 Sensor horizontal mirror\n"
        "\t--vflip 0/1 Sensor vertical flip\n"
        "\t--sw Sensor output width (with --sh and --sfps)\n"
        "\t--sh Sensor output height\n"
        "\t--sfps Sensor target fps\n"
        "\t--rotation N Rotation: 0=0°, 1=90°, 2=180°, 3=270°\n"
        "\t--ae 0/1 Auto exposure before stream on: 0=off (manual), 1=on (auto)\n"
        "\t--exp US Exposure time in microseconds, not the V4L2 100 us unit (manual mode)\n"
        "\t--again N Analogue gain in 1/1000 x, 1000=1.0x (manual mode)\n"
        "Keys (type the key, then Enter):\n"
        "\tq quit, d dump\n"
        "\t+ / - double / halve exposure, ] / [ double / halve analogue gain\n"
    );
}

static uint32_t to_v4l2_fourcc(const char* fourcc) {
    return v4l2_fourcc(fourcc[0], fourcc[1], fourcc[2], fourcc[3]);
}

/* Positive integer only. 0, a negative value and trailing junk are rejected. */
static int parse_positive(const char* text, int* out) {
    char* end = NULL;
    long value;

    if (!text || !*text)
        return -1;
    errno = 0;
    value = strtol(text, &end, 10);
    if (errno || end == text || *end || value <= 0 || value > 0x7fffffffL)
        return -1;
    *out = (int)value;
    return 0;
}

static int parse_cmd(int argc, char* argv[], struct v4l2_drm_context* context) {
    int ch;
    int option_index = 0;
    int context_idx = -1;

    struct option longopt[] = {
        {
            "crop-x",
            required_argument,
            NULL,
            'x'
        },
        {
            "crop-y",
            required_argument,
            NULL,
            'y'
        },
        {
            "crop-width",
            required_argument,
            NULL,
            'c',
        },
        {
            "crop-height",
            required_argument,
            NULL,
            'g'
        },
        {
            "hflip",
            required_argument,
            NULL,
            'm'
        },
        {
            "vflip",
            required_argument,
            NULL,
            'v'
        },
        {
            "rotation",
            required_argument,
            NULL,
            'r'
        },
        {
            "sw",
            required_argument,
            NULL,
            256
        },
        {
            "sh",
            required_argument,
            NULL,
            257
        },
        {
            "sfps",
            required_argument,
            NULL,
            258
        },
        {
            "ae",
            required_argument,
            NULL,
            259
        },
        {
            "exp",
            required_argument,
            NULL,
            260
        },
        {
            "again",
            required_argument,
            NULL,
            261
        },
        {0, 0, 0, 0}
    };

    while((ch = getopt_long_only(argc, argv, "w:h:d:n:f:sx:y:m:v:r:", longopt, &option_index)) != -1) {
        if ((context_idx < 0) && (ch != 'd')) {
            help(argv[0]);
            return -1;
        }
        switch (ch) {
            case 'w':
                context[context_idx].width = atoi(optarg);
                break;
            case 'h':
                context[context_idx].height = atoi(optarg);
                break;
            case 'd':
                context_idx += 1;
                v4l2_drm_default_context(&context[context_idx]);
                context[context_idx].device = atoi(optarg);
                break;
            case 'n':
                context[context_idx].buffer_num = atoi(optarg);
                break;
            case 'f':
                context[context_idx].video_format = to_v4l2_fourcc(optarg);
                if (context[context_idx].display_format == 0) {
                    context[context_idx].display = false;
                }
                break;
            case 'r':
                context[context_idx].drm_rotation = atoi(optarg);
                break;
            case 's':
                // disable display
                context[context_idx].display = false;
                break;
            case 'x':
                context[context_idx].crop_size.offset_x = atoi(optarg);
                break;
            case 'y':
                context[context_idx].crop_size.offset_y = atoi(optarg);
                break;
            case 'g':
                context[context_idx].crop_size.height = atoi(optarg);
                break;
            case 'c':
                context[context_idx].crop_size.width = atoi(optarg);
                break;
            case 'm':
                context[context_idx].hflip = (atoi(optarg) != 0) ? 1 : 0;
                break;
            case 'v':
                context[context_idx].vflip = (atoi(optarg) != 0) ? 1 : 0;
                break;
            case 256:
                context[context_idx].sensor_width = (uint32_t)atoi(optarg);
                break;
            case 257:
                context[context_idx].sensor_height = (uint32_t)atoi(optarg);
                break;
            case 258:
                context[context_idx].sensor_fps = (uint32_t)atoi(optarg);
                break;
            case 259:
                context[context_idx].ae_mode = (atoi(optarg) != 0) ?
                    V4L2_EXPOSURE_AUTO : V4L2_EXPOSURE_MANUAL;
                break;
            case 260: {
                int value;
                if (parse_positive(optarg, &value) < 0) {
                    fprintf(stderr, "invalid --exp '%s': want a positive number of microseconds\n",
                        optarg ? optarg : "");
                    return -1;
                }
                context[context_idx].ae_exposure_us = value;
                break;
            }
            case 261: {
                int value;
                if (parse_positive(optarg, &value) < 0) {
                    fprintf(stderr, "invalid --again '%s': want a positive gain in 1/1000 x\n",
                        optarg ? optarg : "");
                    return -1;
                }
                context[context_idx].ae_again_milli = value;
                break;
            }
            default:
                help(argv[0]);
                return -1;
        }
    }

    bool sw_sensor_target = false;

    for (int i = 0; i <= context_idx; i++) {
        if (context[i].sensor_width && context[i].sensor_height &&
            context[i].sensor_fps) {
            context[i].sensor_target_valid = true;
            sw_sensor_target = true;
        }
    }

    if (sw_sensor_target && context_idx > 0) {
        fprintf(stderr, "sensor target SW mode only supports one camera\n");
        return -1;
    }

    return context_idx + 1;
}

static struct timeval tv, tv2;
static struct display* display = NULL;
static int num = 0;

/* Only meaningful in manual mode; in auto mode AE overrides the value. */
static void adjust_ae(struct v4l2_drm_context* context, char key) {
    bool exposure = (key == '+' || key == '-');
    uint32_t id = exposure ? V4L2_CID_EXPOSURE : V4L2_CID_ANALOGUE_GAIN;
    int min = exposure ? 1 : 1000;
    int cur, val, ret;

    for (int i = 0; i < num; i++) {
        ret = v4l2_drm_get_ae_ctrl(&context[i], id, &cur);
        if (ret < 0) {
            fprintf(stderr, "\n[%d] get %s failed: %s\n", i,
                exposure ? "exposure" : "analogue_gain", strerror(-ret));
            continue;
        }
        if (cur <= 0) {
            fprintf(stderr, "\n[%d] get %s read back %d\n", i,
                exposure ? "exposure" : "analogue_gain", cur);
            continue;
        }
        val = (key == '+' || key == ']') ? cur * 2 : cur / 2;
        if (val < min)
            val = min;
        ret = v4l2_drm_set_ae_ctrl(&context[i], id, val);
        if (ret == 0)
            ret = v4l2_drm_get_ae_ctrl(&context[i], id, &val);
        if (ret < 0)
            fprintf(stderr, "\n[%d] set %s failed: %s\n", i,
                exposure ? "exposure" : "analogue_gain", strerror(-ret));
        else
            fprintf(stderr, "\n[%d] %s: %d -> %d %s\n", i,
                exposure ? "exposure" : "analogue_gain", cur, val,
                exposure ? "us" : "/1000x");
    }
}

int handler(struct v4l2_drm_context* context, bool displayed) {
    // FPS
    static unsigned response = 0, display_frame_count = 0;
    response += 1;
    if (displayed) {
        display_frame_count += 1;
    }
    gettimeofday(&tv2, NULL);
    uint64_t duration = 1000000 * (tv2.tv_sec - tv.tv_sec) + tv2.tv_usec - tv.tv_usec;
    if (duration >= 1000000) {
        fprintf(stderr, " poll: %.2f, ", response * 1000000. / duration);
        response = 0;
        if (display) {
            fprintf(stderr, "display: %.2f, ", display_frame_count * 1000000. / duration);
            display_frame_count = 0;
        }
        for (unsigned i = 0; i < num; i++) {
            fprintf(stderr, "[%u]: %.2f, ", i, context[i].frame_count * 1000000. / duration);
            context[i].frame_count = 0;
        }
        fprintf(stderr, "          \r");
        fflush(stderr);
        gettimeofday(&tv, NULL);
    }
    // key
    char c;
    ssize_t n = read(STDIN_FILENO, &c, 1);
    if ((n > 0) && (c == '+' || c == '-' || c == ']' || c == '[')) {
        adjust_ae(context, c);
        return 0;
    }
    if ((n > 0) && (c != '\n')) {
        return c;
    }
    if ((n < 0) && (errno != EAGAIN)) {
        return -1;
    }
    return 0;
}

int main(int argc, char* argv[]) {
    struct v4l2_drm_context context[9] = {0};
    int flag_display = 0;

    if(argc <= 1){
        help(argv[0]);
        return -1;
    }


    int ret = parse_cmd(argc, argv, context);
    if (ret < 0) {
        return -1;
    }
    num = ret;
    ret = v4l2_drm_setup(context, num, &display);
    if (ret < 0) {
        return -1;
    }
    if (display) {
        flag_display = 1;
    }
    int flag = fcntl(STDIN_FILENO, F_GETFL);
    flag |= O_NONBLOCK;
    if (fcntl(STDIN_FILENO, F_SETFL, flag)) {
        pr("can't set stdin non-block");
        goto streamoff;
    }
    gettimeofday(&tv, NULL);

    

    ret = v4l2_drm_run(context, num, handler);

    streamoff:
    if (display) {
        display_exit(display);
    }
    return 0;
}
