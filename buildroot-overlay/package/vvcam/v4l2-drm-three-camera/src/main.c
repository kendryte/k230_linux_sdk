#include "three_camera.h"
#include <display.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <linux/videodev2.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>

#define pr(fmt, ...) fprintf(stderr, "[v4l2-drm-three-camera] " fmt "\n", ##__VA_ARGS__)

static void help(const char* argv0)
{
    printf("Usage: %s -w 480 -h 320\n", argv0);
    printf("       %s -d 0 -w 480 -h 320 -x 0   -y 0   -l 3 \\\n", argv0);
    printf("          -d 1 -w 480 -h 320 -x 480 -y 0   -l 2 \\\n");
    printf("          -d 2 -w 480 -h 320 -x 960 -y 0   -l 1\n");
    printf("\nExample (3 cameras, MCM):\n");
    printf("       %s -d 3 -w 240 -h 240 -l 1 \\\n", argv0);
    printf("          -d 2 -w 240 -h 400 -x 240 -l 2 \\\n");
    printf("          -d 1 -w 480 -h 800 -x 480 -l 3\n");
    printf(
        "\nOptions (per -d block, up to %d cameras):\n"
        "\t-d Video device (/dev/videoN)\n"
        "\t-w Width\n"
        "\t-h Height\n"
        "\t-n Buffer count\n"
        "\t-f Format NV12/NV16\n"
        "\t-s Disable display\n"
        "\t-x Display start X (CRTC_X)\n"
        "\t-y Display start Y (CRTC_Y)\n"
        "\t-l Video layer 1=video_1, 2=video_2, 3=video_3\n"
        "\t--rotation N 0/1/2/3\n"
        "\n"
        "Default: /dev/video0..2 (csi0 MP/SP1/SP2), layers 3/2/1, horizontal layout.\n"
        "MCM pad map: video0-2=csi0 MP/SP1/SP2, video3-5=csi1, video6-8=csi2 "
        "(see vvcam_pipeline_link.h).\n",
        THREE_CAMERA_MAX
    );
}

static uint32_t to_v4l2_fourcc(const char* fourcc)
{
    return v4l2_fourcc(fourcc[0], fourcc[1], fourcc[2], fourcc[3]);
}

static int parse_cmd(int argc, char* argv[], struct three_camera_context* context)
{
    int ch;
    int option_index = 0;
    int context_idx = -1;
    bool width_set = false;
    bool height_set = false;
    bool layer_set[THREE_CAMERA_MAX] = {false};

    struct option longopt[] = {
        {"rotation", required_argument, NULL, 'r'},
        {0, 0, 0, 0}
    };

    while ((ch = getopt_long_only(argc, argv, "w:h:d:n:f:sx:y:l:r:", longopt, &option_index)) != -1) {
        if ((context_idx < 0) && (ch != 'd') && (ch != 'w') && (ch != 'h')) {
            help(argv[0]);
            return -1;
        }
        switch (ch) {
            case 'w':
                width_set = true;
                if (context_idx < 0) {
                    for (int i = 0; i < THREE_CAMERA_MAX; i++) {
                        context[i].v4l2.width = (unsigned)atoi(optarg);
                    }
                } else {
                    context[context_idx].v4l2.width = (unsigned)atoi(optarg);
                }
                break;
            case 'h':
                height_set = true;
                if (context_idx < 0) {
                    for (int i = 0; i < THREE_CAMERA_MAX; i++) {
                        context[i].v4l2.height = (unsigned)atoi(optarg);
                    }
                } else {
                    context[context_idx].v4l2.height = (unsigned)atoi(optarg);
                }
                break;
            case 'd':
                context_idx += 1;
                if (context_idx >= THREE_CAMERA_MAX) {
                    pr("at most %d cameras", THREE_CAMERA_MAX);
                    return -1;
                }
                three_camera_default_context(&context[context_idx]);
                context[context_idx].v4l2.device = (unsigned)atoi(optarg);
                if (width_set) {
                    context[context_idx].v4l2.width = context[0].v4l2.width;
                }
                if (height_set) {
                    context[context_idx].v4l2.height = context[0].v4l2.height;
                }
                break;
            case 'n':
                context[context_idx].v4l2.buffer_num = (unsigned)atoi(optarg);
                break;
            case 'f':
                context[context_idx].v4l2.video_format = to_v4l2_fourcc(optarg);
                break;
            case 'r':
                context[context_idx].v4l2.drm_rotation = (enum drm_rotation)atoi(optarg);
                break;
            case 's':
                context[context_idx].v4l2.display = false;
                break;
            case 'x':
                context[context_idx].v4l2.offset_x = (unsigned)atoi(optarg);
                break;
            case 'y':
                context[context_idx].v4l2.offset_y = (unsigned)atoi(optarg);
                break;
            case 'l':
                context[context_idx].video_layer = (unsigned)atoi(optarg);
                layer_set[context_idx] = true;
                if (context[context_idx].video_layer < 1 ||
                    context[context_idx].video_layer > 3) {
                    pr("-l must be 1..3 (video_1..video_3)");
                    return -1;
                }
                break;
            default:
                help(argv[0]);
                return -1;
        }
    }

    if (context_idx < 0) {
        if (!width_set || !height_set) {
            help(argv[0]);
            return -1;
        }
        for (int i = 0; i < THREE_CAMERA_MAX; i++) {
            three_camera_default_context(&context[i]);
            context[i].v4l2.device = (unsigned)i;
            context[i].v4l2.width = context[0].v4l2.width;
            context[i].v4l2.height = context[0].v4l2.height;
        }
        context_idx = THREE_CAMERA_MAX - 1;
    }

    for (int i = 0; i <= context_idx; i++) {
        if (!layer_set[i]) {
            context[i].video_layer = (unsigned)(3 - i);
        }
    }

    return context_idx + 1;
}

static struct timeval tv, tv2;
static struct display* display = NULL;
static int num = 0;

static int handler(struct v4l2_drm_context* context, bool displayed)
{
    static unsigned response = 0, display_frame_count = 0;

    response += 1;
    if (displayed) {
        display_frame_count += 1;
    }
    gettimeofday(&tv2, NULL);
    uint64_t duration = 1000000ULL * (uint64_t)(tv2.tv_sec - tv.tv_sec)
        + (uint64_t)(tv2.tv_usec - tv.tv_usec);
    if (duration >= 1000000) {
        fprintf(stderr, " poll: %.2f, ", response * 1000000. / duration);
        response = 0;
        if (display) {
            fprintf(stderr, "display: %.2f, ", display_frame_count * 1000000. / duration);
            display_frame_count = 0;
        }
        for (int i = 0; i < num; i++) {
            fprintf(stderr, "[%d]: %.2f, ", i, context[i].frame_count * 1000000. / duration);
            context[i].frame_count = 0;
        }
        fprintf(stderr, "          \r");
        fflush(stderr);
        gettimeofday(&tv, NULL);
    }

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

int main(int argc, char* argv[])
{
    struct three_camera_context context[THREE_CAMERA_MAX] = {0};
    struct v4l2_drm_context v4l2_ctx[THREE_CAMERA_MAX];
    int ret;
    int i;

    if (argc <= 1) {
        help(argv[0]);
        return -1;
    }

    ret = parse_cmd(argc, argv, context);
    if (ret < 0) {
        return -1;
    }
    num = ret;

    ret = three_camera_setup(context, (unsigned)num, &display);
    if (ret < 0) {
        return -1;
    }

    for (i = 0; i < num; i++) {
        v4l2_ctx[i] = context[i].v4l2;
    }

    int flag = fcntl(STDIN_FILENO, F_GETFL);
    flag |= O_NONBLOCK;
    if (fcntl(STDIN_FILENO, F_SETFL, flag)) {
        pr("can't set stdin non-block");
        goto out;
    }
    gettimeofday(&tv, NULL);

    ret = v4l2_drm_run(v4l2_ctx, (unsigned)num, handler);

out:
    if (display) {
        display_exit(display);
    }
    return ret < 0 ? -1 : 0;
}
