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
    );
}

static uint32_t to_v4l2_fourcc(const char* fourcc) {
    return v4l2_fourcc(fourcc[0], fourcc[1], fourcc[2], fourcc[3]);
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
            257,
        },
        {
            "crop-height",
            required_argument,
            NULL,
            258
        },
        {0, 0, 0, 0}
    };

    while((ch = getopt_long_only(argc, argv, "w:h:d:n:f:sx:y:", longopt, &option_index)) != -1) {
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
            case 's':
                // disable display
                context[context_idx].display = false;
                break;
            default:
                help(argv[0]);
                return -1;
        }
    }

    return context_idx + 1;
}

static struct timeval tv, tv2;
static struct display* display = NULL;
static int num = 0;

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
    if ((n > 0) && (c != '\n')) {
        return c;
    }
    if ((n < 0) && (errno != EAGAIN)) {
        return -1;
    }
    return 0;
}

int main(int argc, char* argv[]) {
    struct v4l2_drm_context context[9];
    int flag_display = 0;

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
