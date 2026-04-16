/*
 * v4l2-drm-scene - V4L2 DRM demo with multi-scene switching support
 * 
 * Features:
 * - Single video device support
 * - DMABUF zero-copy buffer sharing
 * - Multi-scene switching via JSON config file
 * - Hotkey switching (1, 2, 3... for different scenes)
 * - FPS display and keyboard control
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <getopt.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <poll.h>

#include <linux/videodev2.h>
#include <drm_fourcc.h>
#include "display.h"

#define MAX_SCENES 10

// Scene 配置结构
struct scene_config {
    char name[32];
    char scene_path[128];
    char sensor[32];
    char xml_file[64];
    char manu_json_file[64];
    char auto_json_file[32];
    uint32_t mode;
};

// 正确的控件 ID 定义 (V4L2_CID_PRIVATE_BASE = 0x00980900)
#define MY_CID_BASE 0x00980900  // V4L2_CID_PRIVATE_BASE

struct scene_context {
    unsigned width;
    unsigned height;
    unsigned device;
    int video_fd;
    unsigned frame_count;
    uint32_t video_format;
    uint32_t display_format;
    unsigned buffer_num;
    struct display* display;
    struct display_plane* plane;
    struct display_buffer** display_buffers;
    struct v4l2_buffer vbuffer;
    bool running;
    bool flag_display;
    unsigned offset_x;
    unsigned offset_y;
    int buffer_hold[3];
    unsigned int wp;
    bool flag_dqbuf;
    
    // Multi-scene 相关
    char* scene_config_file;
    struct scene_config scenes[MAX_SCENES];
    int num_scenes;
    int current_scene_index;
    bool scene_change_requested;
    int target_scene_index;
};

static struct timeval tv, tv2;

// 函数前向声明
static int get_scene_config(struct scene_context* ctx, struct scene_config* cfg);
static void print_current_scene(struct scene_config* cfg);
static void cleanup(struct scene_context* ctx);

static void help(const char* argv0) {
    printf("Usage: %s -d <device> -w <width> -h <height> -s <scene_config>\n", argv0);
    printf("Options:\n");
    printf("\t-d Video device number\n");
    printf("\t-w Width\n");
    printf("\t-h Height\n");
    printf("\t-s Scene config JSON file (with multiple scenes)\n");
    printf("\nHotkeys (when scene config loaded):\n");
    printf("\t1-%d  Switch to scene 1-%d\n", MAX_SCENES, MAX_SCENES);
    printf("\tg     Get current scene config\n");
    printf("\tq     Quit\n");
}

// JSON 解析
static int parse_scene_config(const char* filename, struct scene_context* ctx) {
    FILE* f = fopen(filename, "r");
    if (!f) {
        perror("fopen scene config");
        return -1;
    }
    
    ctx->num_scenes = 0;
    int current_scene = -1;
    char line[256];
    
    while (fgets(line, sizeof(line), f)) {
        if (strstr(line, "\"name\"")) {
            char* colon = strchr(line, ':');
            if (colon) {
                char* value = colon + 1;
                while (*value == ' ' || *value == '\t') value++;
                if (*value == '"') {
                    value++;
                    char* end = strchr(value, '"');
                    if (end) {
                        *end = '\0';
                        current_scene = ctx->num_scenes;
                        if (current_scene < MAX_SCENES) {
                            memset(&ctx->scenes[current_scene], 0, sizeof(struct scene_config));
                            strncpy(ctx->scenes[current_scene].name, value, 
                                   sizeof(ctx->scenes[current_scene].name) - 1);
                            ctx->num_scenes++;
                        }
                    }
                }
            }
        }
        
        if (current_scene >= 0 && current_scene < MAX_SCENES) {
            char* colon = strchr(line, ':');
            if (!colon) continue;
            
            *colon = '\0';
            char* key = line;
            char* value = colon + 1;
            
            while (*value == ' ' || *value == '\t') value++;
            if (*value == '"') value++;
            char* end = value + strlen(value) - 1;
            while (end > value && (*end == '"' || *end == '\n' || *end == '\r' || *end == ',')) {
                *end = '\0';
                end--;
            }
            
            if (strstr(key, "scene_path")) {
                strncpy(ctx->scenes[current_scene].scene_path, value, 
                       sizeof(ctx->scenes[current_scene].scene_path) - 1);
            } else if (strstr(key, "sensor") && !strstr(key, "name")) {
                strncpy(ctx->scenes[current_scene].sensor, value, 
                       sizeof(ctx->scenes[current_scene].sensor) - 1);
            } else if (strstr(key, "xml_file")) {
                strncpy(ctx->scenes[current_scene].xml_file, value, 
                       sizeof(ctx->scenes[current_scene].xml_file) - 1);
            } else if (strstr(key, "manu_json_file")) {
                strncpy(ctx->scenes[current_scene].manu_json_file, value, 
                       sizeof(ctx->scenes[current_scene].manu_json_file) - 1);
            } else if (strstr(key, "auto_json_file")) {
                strncpy(ctx->scenes[current_scene].auto_json_file, value, 
                       sizeof(ctx->scenes[current_scene].auto_json_file) - 1);
            } else if (strstr(key, "mode") && !strstr(key, "name")) {
                ctx->scenes[current_scene].mode = atoi(value);
            }
        }
    }
    
    fclose(f);
    return ctx->num_scenes > 0 ? 0 : -1;
}

static void print_available_scenes(struct scene_context* ctx) {
    printf("\nAvailable scenes (%d):\n", ctx->num_scenes);
    for (int i = 0; i < ctx->num_scenes && i < MAX_SCENES; i++) {
        printf("  [%d] %s: sensor=%s, xml=%s\n", 
               i + 1, 
               ctx->scenes[i].name[0] ? ctx->scenes[i].name : "unnamed",
               ctx->scenes[i].sensor,
               ctx->scenes[i].xml_file);
    }
    printf("\n");
}

static int xioctl(int fd, int request, void* arg) {
    int r;
    do {
        r = ioctl(fd, request, arg);
    } while (-1 == r && EINTR == errno);
    return r;
}

static int setup_device(struct scene_context* ctx) {
    char device_name[32];
    snprintf(device_name, sizeof(device_name), "/dev/video%u", ctx->device);
    
    ctx->video_fd = open(device_name, O_RDWR | O_NONBLOCK);
    if (ctx->video_fd < 0) {
        perror(device_name);
        return -1;
    }
    
    struct v4l2_capability cap;
    if (xioctl(ctx->video_fd, VIDIOC_QUERYCAP, &cap) < 0) {
        perror("VIDIOC_QUERYCAP");
        return -1;
    }
    
    struct v4l2_format fmt = {0};
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (xioctl(ctx->video_fd, VIDIOC_G_FMT, &fmt) < 0) {
        perror("VIDIOC_G_FMT");
        return -1;
    }
    
    fmt.fmt.pix.pixelformat = ctx->video_format;
    fmt.fmt.pix.width = ctx->width;
    fmt.fmt.pix.height = ctx->height;
    fmt.fmt.pix.field = V4L2_FIELD_NONE;
    
    if (xioctl(ctx->video_fd, VIDIOC_S_FMT, &fmt) < 0) {
        perror("VIDIOC_S_FMT");
        return -1;
    }
    
    return 0;
}

static int setup_display(struct scene_context* ctx) {
    if (!ctx->flag_display) {
        return 0;
    }
    
    ctx->display = display_init(0);
    if (!ctx->display) {
        fprintf(stderr, "Failed to init display\n");
        return -1;
    }
    
    ctx->display_format = DRM_FORMAT_NV12;
    
    ctx->plane = display_get_plane(ctx->display, ctx->display_format);
    if (!ctx->plane) {
        fprintf(stderr, "Failed to get display plane\n");
        return -1;
    }
    
    ctx->display_buffers = calloc(ctx->buffer_num, sizeof(struct display_buffer*));
    
    struct display_buffer* prev_buf = NULL;
    for (unsigned int i = 0; i < ctx->buffer_num; i++) {
        struct display_buffer* buf = display_allocate_buffer(ctx->plane, ctx->width, ctx->height);
        if (!buf) {
            fprintf(stderr, "Failed to alloc display buffer %u\n", i);
            return -1;
        }
        ctx->display_buffers[i] = buf;
        
        if (prev_buf) {
            prev_buf->next = buf;
        } else {
            ctx->plane->buffers = buf;
        }
        buf->next = NULL;
        prev_buf = buf;
    }
    
    return 0;
}

static int setup_buffers(struct scene_context* ctx) {
    struct v4l2_requestbuffers req = {0};
    req.count = ctx->buffer_num;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    
    if (ctx->flag_display) {
        req.memory = V4L2_MEMORY_DMABUF;
    } else {
        req.memory = V4L2_MEMORY_MMAP;
    }
    
    if (xioctl(ctx->video_fd, VIDIOC_REQBUFS, &req) < 0) {
        perror("VIDIOC_REQBUFS");
        return -1;
    }
    
    if (req.count < 2) {
        fprintf(stderr, "Insufficient buffer memory\n");
        return -1;
    }
    
    ctx->buffer_num = req.count;
    
    if (ctx->flag_display) {
        for (unsigned int i = 0; i < ctx->buffer_num; i++) {
            if (!ctx->display_buffers[i]) {
                fprintf(stderr, "display_buffers[%u] is NULL\n", i);
                return -1;
            }
            
            memset(&ctx->vbuffer, 0, sizeof(ctx->vbuffer));
            ctx->vbuffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            ctx->vbuffer.memory = V4L2_MEMORY_DMABUF;
            ctx->vbuffer.index = i;
            ctx->vbuffer.m.fd = ctx->display_buffers[i]->dmabuf_fd;
            ctx->vbuffer.length = ctx->display_buffers[i]->size;
            
            if (xioctl(ctx->video_fd, VIDIOC_QBUF, &ctx->vbuffer) < 0) {
                perror("VIDIOC_QBUF");
                return -1;
            }
        }
    } else {
        for (unsigned int i = 0; i < ctx->buffer_num; i++) {
            memset(&ctx->vbuffer, 0, sizeof(ctx->vbuffer));
            ctx->vbuffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            ctx->vbuffer.memory = V4L2_MEMORY_MMAP;
            ctx->vbuffer.index = i;
            
            if (xioctl(ctx->video_fd, VIDIOC_QUERYBUF, &ctx->vbuffer) < 0) {
                perror("VIDIOC_QUERYBUF");
                return -1;
            }
            
            if (xioctl(ctx->video_fd, VIDIOC_QBUF, &ctx->vbuffer) < 0) {
                perror("VIDIOC_QBUF");
                return -1;
            }
        }
    }
    
    for (unsigned i = 0; i < 3; i++) {
        ctx->buffer_hold[i] = -1;
    }
    ctx->wp = 0;
    ctx->flag_dqbuf = false;
    
    return 0;
}

static int start_capturing(struct scene_context* ctx) {
    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    
    if (xioctl(ctx->video_fd, VIDIOC_STREAMON, &type) < 0) {
        perror("VIDIOC_STREAMON");
        return -1;
    }
    
    if (ctx->flag_display && ctx->display_buffers) {
        display_commit_buffer(ctx->display_buffers[0], ctx->offset_x, ctx->offset_y);
    }
    
    return 0;
}

static int stop_capturing(struct scene_context* ctx) {
    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    
    if (xioctl(ctx->video_fd, VIDIOC_STREAMOFF, &type) < 0) {
        perror("VIDIOC_STREAMOFF");
        return -1;
    }
    
    return 0;
}

// 应用 scene 配置
static int apply_scene_config_ctrls(struct scene_context* ctx, struct scene_config* cfg) {
    struct v4l2_ext_controls ctrls = {0};
    struct v4l2_ext_control ctrl[6] = {0};
    int ret;
    
    ctrl[0].id = MY_CID_BASE + 0;
    ctrl[0].size = strlen(cfg->scene_path) + 1;
    ctrl[0].string = cfg->scene_path;
    
    ctrl[1].id = MY_CID_BASE + 1;
    ctrl[1].size = strlen(cfg->sensor) + 1;
    ctrl[1].string = cfg->sensor;
    
    ctrl[2].id = MY_CID_BASE + 2;
    ctrl[2].size = strlen(cfg->xml_file) + 1;
    ctrl[2].string = cfg->xml_file;
    
    ctrl[3].id = MY_CID_BASE + 3;
    ctrl[3].size = strlen(cfg->manu_json_file) + 1;
    ctrl[3].string = cfg->manu_json_file;
    
    ctrl[4].id = MY_CID_BASE + 4;
    ctrl[4].size = strlen(cfg->auto_json_file) + 1;
    ctrl[4].string = cfg->auto_json_file;
    
    ctrl[5].id = MY_CID_BASE + 5;
    ctrl[5].value = cfg->mode;
    
    ctrls.ctrl_class = V4L2_CTRL_CLASS_USER;
    ctrls.count = 6;
    ctrls.controls = ctrl;
    
    ret = xioctl(ctx->video_fd, VIDIOC_S_EXT_CTRLS, &ctrls);
    if (ret < 0) {
        printf("DEBUG: VIDIOC_S_EXT_CTRLS failed with errno=%d (%s)\n", errno, strerror(errno));
        return -1;
    }
    
    return 0;
}

// 获取当前 scene 配置
static int get_scene_config(struct scene_context* ctx, struct scene_config* cfg) {
    struct v4l2_ext_controls ctrls = {0};
    struct v4l2_ext_control ctrl[6] = {0};
    int i;
    
    ctrl[0].id = MY_CID_BASE + 0;
    ctrl[0].size = sizeof(cfg->scene_path);
    ctrl[0].string = cfg->scene_path;
    
    ctrl[1].id = MY_CID_BASE + 1;
    ctrl[1].size = sizeof(cfg->sensor);
    ctrl[1].string = cfg->sensor;
    
    ctrl[2].id = MY_CID_BASE + 2;
    ctrl[2].size = sizeof(cfg->xml_file);
    ctrl[2].string = cfg->xml_file;
    
    ctrl[3].id = MY_CID_BASE + 3;
    ctrl[3].size = sizeof(cfg->manu_json_file);
    ctrl[3].string = cfg->manu_json_file;
    
    ctrl[4].id = MY_CID_BASE + 4;
    ctrl[4].size = sizeof(cfg->auto_json_file);
    ctrl[4].string = cfg->auto_json_file;
    
    ctrl[5].id = MY_CID_BASE + 5;
    ctrl[5].value = 0;
    
    ctrls.ctrl_class = V4L2_CTRL_CLASS_USER;
    ctrls.count = 6;
    ctrls.controls = ctrl;
    
    if (xioctl(ctx->video_fd, VIDIOC_G_EXT_CTRLS, &ctrls) < 0) {
        return -1;
    }
    
    for (i = 0; i < 6; i++) {
        switch (ctrl[i].id) {
            case MY_CID_BASE + 5:
                cfg->mode = ctrl[i].value;
                break;
        }
    }
    
    return 0;
}

// 打印当前 scene 配置
static void print_current_scene(struct scene_config* cfg) {
    printf("\n=== Current Scene Configuration ===\n");
    printf("  Scene Path:      %s\n", cfg->scene_path);
    printf("  Sensor:          %s\n", cfg->sensor);
    printf("  XML File:        %s\n", cfg->xml_file);
    printf("  Manual JSON:     %s\n", cfg->manu_json_file);
    printf("  Auto JSON:       %s\n", cfg->auto_json_file);
    printf("  Mode:            %u\n", cfg->mode);
    printf("====================================\n");
}

// 完全清理资源（用于 scene 切换）
static void full_cleanup(struct scene_context* ctx) {
    printf("Cleaning up all resources...\n");
    
    // 1. 停止捕获
    if (ctx->video_fd >= 0) {
        stop_capturing(ctx);
        printf("  - Stopped capturing\n");
    }
    sleep(1);
    // 2. 关闭 video fd
    if (ctx->video_fd >= 0) {
        close(ctx->video_fd);
        ctx->video_fd = -1;
        printf("  - Closed video fd\n");
    }
    
    //3. 释放 display buffers
    if (ctx->display_buffers) {
        for (unsigned int i = 0; i < ctx->buffer_num; i++) {
            if (ctx->display_buffers[i]) {
                display_free_buffer(ctx->display_buffers[i]);
            }
        }
        free(ctx->display_buffers);
        ctx->display_buffers = NULL;
        printf("  - Freed display buffers\n");
    }
    
    // 4. 退出 display
    if (ctx->display) {
        display_exit(ctx->display);
        ctx->display = NULL;
        printf("  - Exited display\n");
    }
    
    //5. 重置状态
    ctx->plane = NULL;
    for (unsigned i = 0; i < 3; i++) {
        ctx->buffer_hold[i] = -1;
    }
    ctx->wp = 0;
    ctx->flag_dqbuf = false;
    
    printf("Cleanup complete\n");
}

static int restart_video_device(struct scene_context* ctx) {
    printf("Restarting video device (full: close -> reopen)...\n");

    full_cleanup(ctx);

    if (setup_device(ctx) < 0) {
        fprintf(stderr, "Failed to setup device on restart\n");
        return -1;
    }
    printf("  - Opened video device\n");
    printf("  - Set format\n");

    if (setup_display(ctx) < 0) {
        fprintf(stderr, "Failed to setup display on restart\n");
        return -1;
    }
    printf("  - Setup display\n");

    if (setup_buffers(ctx) < 0) {
        fprintf(stderr, "Failed to setup buffers on restart\n");
        return -1;
    }
    printf("  - Setup buffers\n");

    if (start_capturing(ctx) < 0) {
        fprintf(stderr, "Failed to start capturing on restart\n");
        return -1;
    }
    printf("  - Started capturing\n");

    printf("Video device restarted successfully\n");
    return 0;
}

static int process_frame(struct scene_context* ctx) {
    struct pollfd fds = {0};
    fds.fd = ctx->video_fd;
    fds.events = POLLIN | POLLPRI;
    
    int ret = poll(&fds, 1, 1000);
    if (ret < 0) {
        if (errno == EINTR) return 0;
        perror("poll");
        return -1;
    }
    if (ret == 0) {
        return 0;
    }
    
    if (!fds.revents) {
        return 0;
    }
    
    ctx->vbuffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (ctx->flag_display) {
        ctx->vbuffer.memory = V4L2_MEMORY_DMABUF;
    } else {
        ctx->vbuffer.memory = V4L2_MEMORY_MMAP;
    }
    
    if (ctx->flag_dqbuf) {
        if (xioctl(ctx->video_fd, VIDIOC_DQBUF, &ctx->vbuffer) < 0) {
            return 0;
        }
        ctx->frame_count++;
        
        if (xioctl(ctx->video_fd, VIDIOC_QBUF, &ctx->vbuffer) < 0) {
            perror("VIDIOC_QBUF");
            return -1;
        }
        return 0;
    }
    
    ctx->wp = (ctx->wp + 1) % 3;
    if (ctx->buffer_hold[ctx->wp] >= 0) {
        ctx->vbuffer.index = ctx->buffer_hold[ctx->wp];
        if (ctx->flag_display) {
            ctx->vbuffer.m.fd = ctx->display_buffers[ctx->vbuffer.index]->dmabuf_fd;
        }
        if (xioctl(ctx->video_fd, VIDIOC_QBUF, &ctx->vbuffer) < 0) {
            perror("VIDIOC_QBUF");
            return -1;
        }
    }
    
    if (xioctl(ctx->video_fd, VIDIOC_DQBUF, &ctx->vbuffer) < 0) {
        return 0;
    }
    
    ctx->frame_count++;
    ctx->buffer_hold[ctx->wp] = ctx->vbuffer.index;
    ctx->flag_dqbuf = true;
    
    return 0;
}

static int update_display(struct scene_context* ctx) {
    if (!ctx->flag_display || !ctx->plane) {
        return 0;
    }
    
    bool has_buffer = false;
    for (unsigned i = 0; i < 3; i++) {
        if (ctx->buffer_hold[i] >= 0) {
            has_buffer = true;
            break;
        }
    }
    
    if (!has_buffer) {
        return 0;
    }
    
    display_handle_vsync(ctx->display);
    
    for (unsigned i = 0; i < 3; i++) {
        if (ctx->buffer_hold[i] < 0) {
            continue;
        }
        
        if (!ctx->display_buffers[ctx->buffer_hold[i]]) {
            fprintf(stderr, "display_buffers[%d] is NULL\n", ctx->buffer_hold[i]);
            return -1;
        }
        
        int ret = display_update_buffer(ctx->display_buffers[ctx->buffer_hold[i]], 
                                        ctx->offset_x, ctx->offset_y);
        if (ret < 0) {
            fprintf(stderr, "display_update_buffer failed\n");
            return -1;
        }
        
        ctx->flag_dqbuf = false;
    }
    
    if (display_commit(ctx->display) < 0) {
        fprintf(stderr, "display_commit failed\n");
        return -1;
    }
    
    return 0;
}

static int scene_handler(struct scene_context* ctx) {
    static unsigned int response = 0;
    response += 1;
    
    gettimeofday(&tv2, NULL);
    uint64_t duration = 1000000 * (tv2.tv_sec - tv.tv_sec) + tv2.tv_usec - tv.tv_usec;
    
    if (duration >= 1000000) {
        fprintf(stderr, "FPS: %.2f          \r", response * 1000000. / duration);
        fflush(stderr);
        response = 0;
        gettimeofday(&tv, NULL);
    }
    
    char c;
    ssize_t n = read(STDIN_FILENO, &c, 1);
    if ((n > 0) && (c != '\n')) {
        // 'g' 键获取当前 scene 配置
        if (c == 'g') {
            struct scene_config current_cfg = {0};
            printf("\nGetting current scene config...\n");
            if (get_scene_config(ctx, &current_cfg) == 0) {
                print_current_scene(&current_cfg);
            } else {
                fprintf(stderr, "Failed to get scene config\n");
            }
            return c;
        }
        
        // 数字键 1-9 切换对应 scene
        if (c >= '1' && c <= '9') {
            int scene_idx = c - '1';
            
            if (ctx->num_scenes == 0) {
                printf("\n⚠️  No scene config loaded\n");
                return c;
            }
            
            if (scene_idx >= ctx->num_scenes) {
                printf("\n⚠️  Scene %d not available (max: %d scenes loaded)\n", 
                       scene_idx + 1, ctx->num_scenes);
                printf("   Available scenes: 1-%d\n", ctx->num_scenes);
                return c;
            }
            
            if (scene_idx == ctx->current_scene_index) {
                printf("\nℹ️  Already on scene %d (%s)\n", 
                       scene_idx + 1, ctx->scenes[scene_idx].name);
                return c;
            }
            
            printf("\nSwitch to scene %d (%s) requested\n", 
                   scene_idx + 1, ctx->scenes[scene_idx].name);
            ctx->target_scene_index = scene_idx;
            ctx->scene_change_requested = true;
        } else {
            if (c != 'q') {
                printf("\n⚠️  Invalid key '%c'. Press 1-%d for scenes, 'g' to get config, or 'q' to quit\n", 
                       c, ctx->num_scenes);
            }
        }
        return c;
    }
    if ((n < 0) && (errno != EAGAIN)) {
        return -1;
    }
    
    return 0;
}

static void cleanup(struct scene_context* ctx) {
    if (ctx->video_fd >= 0) {
        stop_capturing(ctx);
    }
    
    if (ctx->display_buffers) {
        for (unsigned int i = 0; i < ctx->buffer_num; i++) {
            if (ctx->display_buffers[i]) {
                display_free_buffer(ctx->display_buffers[i]);
            }
        }
        free(ctx->display_buffers);
    }
    
    if (ctx->display) {
        display_exit(ctx->display);
    }
    
    if (ctx->video_fd >= 0) {
        close(ctx->video_fd);
    }
    
    if (ctx->scene_config_file) {
        free(ctx->scene_config_file);
    }
}

int main(int argc, char* argv[]) {
    struct scene_context ctx = {0};
    int ch;
    
    ctx.device = 0;
    ctx.width = 640;
    ctx.height = 480;
    ctx.buffer_num = 5;
    ctx.video_format = V4L2_PIX_FMT_NV12;
    ctx.video_fd = -1;
    ctx.flag_display = true;
    ctx.offset_x = 0;
    ctx.offset_y = 0;
    ctx.scene_config_file = NULL;
    ctx.num_scenes = 0;
    ctx.current_scene_index = -1;
    ctx.scene_change_requested = false;
    
    struct option longopt[] = {
        {"scene_config", required_argument, NULL, 's'},
        {0, 0, 0, 0}
    };
    
    while((ch = getopt_long_only(argc, argv, "d:w:h:s:", longopt, NULL)) != -1) {
        switch (ch) {
            case 'd':
                ctx.device = atoi(optarg);
                break;
            case 'w':
                ctx.width = atoi(optarg);
                break;
            case 'h':
                ctx.height = atoi(optarg);
                break;
            case 's':
                ctx.scene_config_file = strdup(optarg);
                break;
            default:
                help(argv[0]);
                return -1;
        }
    }
    
    if (ctx.width == 0 || ctx.height == 0) {
        fprintf(stderr, "Error: width and height must be specified\n");
        help(argv[0]);
        return -1;
    }
    
    printf("v4l2-drm-scene: device=%u, width=%u, height=%u\n", 
           ctx.device, ctx.width, ctx.height);
    
    if (ctx.scene_config_file) {
        printf("Loading scene config: %s\n", ctx.scene_config_file);
        
        if (parse_scene_config(ctx.scene_config_file, &ctx) < 0) {
            fprintf(stderr, "Error: failed to parse scene config\n");
            return -1;
        }
        
        print_available_scenes(&ctx);
        printf("Press 1-%d to switch scenes, 'g' to get config, 'q' to quit\n", ctx.num_scenes);
    } else {
        printf("No scene config loaded. Run with -s <config.json> to enable scene switching.\n");
    }
    
    if (setup_device(&ctx) < 0) {
        fprintf(stderr, "Error: setup_device failed\n");
        return -1;
    }
    
    if (setup_display(&ctx) < 0) {
        fprintf(stderr, "Error: setup_display failed\n");
        cleanup(&ctx);
        return -1;
    }
    
    if (setup_buffers(&ctx) < 0) {
        fprintf(stderr, "Error: setup_buffers failed\n");
        cleanup(&ctx);
        return -1;
    }
    
    int flag = fcntl(STDIN_FILENO, F_GETFL);
    flag |= O_NONBLOCK;
    if (fcntl(STDIN_FILENO, F_SETFL, flag)) {
        fprintf(stderr, "Error: can't set stdin non-block\n");
        cleanup(&ctx);
        return -1;
    }
    
    if (start_capturing(&ctx) < 0) {
        fprintf(stderr, "Error: start_capturing failed\n");
        cleanup(&ctx);
        return -1;
    }
    
    gettimeofday(&tv, NULL);
    ctx.running = true;
    
    printf("Running...\n");
    
    while (ctx.running) {
        int ret = process_frame(&ctx);
        if (ret < 0) {
            break;
        }
        
        ret = update_display(&ctx);
        if (ret < 0) {
            break;
        }
        
        ret = scene_handler(&ctx);
        if (ret == 'q') {
            printf("\nQuit requested\n");
            break;
        }
        if (ret < 0) {
            break;
        }
        
        if (ctx.scene_change_requested) {
            printf("Applying scene %d config...\n", ctx.target_scene_index + 1);
            
            if (apply_scene_config_ctrls(&ctx, &ctx.scenes[ctx.target_scene_index]) < 0) {
                fprintf(stderr, "Error: failed to apply scene config\n");
                ctx.scene_change_requested = false;
                continue;
            }
            sleep(1);
            if (restart_video_device(&ctx) < 0) {
                fprintf(stderr, "Error: failed to restart video device\n");
                ctx.scene_change_requested = false;
                continue;
            }
            
            ctx.current_scene_index = ctx.target_scene_index;
            printf("✓ Switched to scene %d: %s\n", 
                   ctx.current_scene_index + 1, 
                   ctx.scenes[ctx.current_scene_index].name);
            ctx.scene_change_requested = false;
        }
    }
    
    cleanup(&ctx);
    
    return 0;
}
