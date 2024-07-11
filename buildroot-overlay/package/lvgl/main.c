/* Copyright (c) 2023, Canaan Bright Sight Co., Ltd
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include "lvgl/lvgl.h"
#include "lvgl/demos/lv_demos.h"
#include "lvgl/demos/music/lv_demo_music_main.h"
#include "lv_drivers/display/drm.h"
#include "lv_drivers/indev/evdev.h"
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>
// #include <vg_lite.h>

lv_coord_t lcd_w;
lv_coord_t lcd_h;
uint32_t lcd_dpi;
bool flag_running = true;

static void sighandler(int sig) {
    flag_running = false;
}

static void dump_maps(void) {
    int fd = open("/proc/self/maps", O_RDONLY);
    char buffer[4096];
    ssize_t size;

    read:
    size = read(fd, buffer, sizeof(buffer));
    if (size > 0) {
        write(STDOUT_FILENO, buffer, size);
        goto read;
    }
    close(fd);
}

static void test_mem_speed(void* buffer, size_t size) {
    struct timeval tv, tv2;
    uint64_t dut;
    void* tmp = malloc(size);
    gettimeofday(&tv, NULL);
    for (int i = 0; i < 256; i++) {
        memset(buffer, i, size);
    }
    gettimeofday(&tv2, NULL);
    dut = 1000000 * (tv2.tv_sec - tv.tv_sec) + tv2.tv_usec - tv.tv_usec;
    printf("memset %p: %.3f MB/s, elapsed %lu us\n", buffer, 1000000. * size * 256 / dut / 1024 / 1024, dut);

    gettimeofday(&tv, NULL);
    for (int i = 0; i < 256; i++) {
        memcpy(tmp, buffer, size);
    }
    gettimeofday(&tv2, NULL);
    dut = 1000000 * (tv2.tv_sec - tv.tv_sec) + tv2.tv_usec - tv.tv_usec;
    printf("memcpy %p: %.3f MB/s, elapsed %lu us\n", buffer, 1000000. * size * 256 / dut / 1024 / 1024, dut);
    free(tmp);
}

int main(void)
{
    #if 0
    vg_lite_init(480, 800);
    vg_lite_buffer_t buffer = {
        .width = 480,
        .height = 800,
        .format = VG_LITE_ARGB8888
    };
    vg_lite_allocate(&buffer);
    dump_maps();
    test_mem_speed(buffer.memory, buffer.stride * buffer.height);
    vg_lite_free(&buffer);
    vg_lite_close();
    return 0;
    #endif
    /*LittlevGL init*/
    lv_init();

    /*Linux frame buffer device init*/
    drm_init();
    drm_get_sizes(&lcd_w, &lcd_h, &lcd_dpi);
    printf("lcd w,h,dpi:%d,%d,%d \n", lcd_w, lcd_h, lcd_dpi);

    uint32_t draw_buf_size = lcd_w * lcd_h * sizeof(lv_color_t);
    static lv_disp_draw_buf_t disp_buf;
    #if DRM_DIRECT_BUFFER
    lv_color_t *buf_2_1 = drm_get_map(0);
    lv_color_t *buf_2_2 = drm_get_map(1);
    #else
    lv_color_t *buf_2_1 = malloc(draw_buf_size);
    lv_color_t *buf_2_2 = malloc(draw_buf_size);
    #endif
    lv_disp_draw_buf_init(&disp_buf, buf_2_1, buf_2_2, draw_buf_size);
    printf("buffers: %p %p\n", buf_2_1, buf_2_2);
    dump_maps();
    test_mem_speed(buf_2_1, draw_buf_size);
    test_mem_speed(buf_2_2, draw_buf_size);

    /*Initialize and register a display driver*/
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.draw_buf   = &disp_buf;
    disp_drv.flush_cb   = drm_flush;
    disp_drv.hor_res    = lcd_w;
    disp_drv.ver_res    = lcd_h;
    disp_drv.screen_transp = 1;
    disp_drv.full_refresh = 1;
    #if DRM_DIRECT_BUFFER
    disp_drv.direct_mode = 1;
    #endif
    lv_disp_drv_register(&disp_drv);

    #if USE_EVDEV
    evdev_init();
    static lv_indev_drv_t indev_drv_1;
    lv_indev_drv_init(&indev_drv_1); /*Basic initialization*/
    indev_drv_1.type = LV_INDEV_TYPE_POINTER;

    /*This function will be called periodically (by the library) to get the mouse position and state*/
    indev_drv_1.read_cb = evdev_read;
    /*lv_indev_t *mouse_indev = */lv_indev_drv_register(&indev_drv_1);


    /*Set a cursor for the mouse*/
    // LV_IMG_DECLARE(mouse_cursor_icon)
    // lv_obj_t * cursor_obj = lv_img_create(lv_scr_act()); /*Create an image object for the cursor */
    // lv_img_set_src(cursor_obj, &mouse_cursor_icon);           /*Set the image source*/
    // lv_indev_set_cursor(mouse_indev, cursor_obj);             /*Connect the image  object to the driver*/
    #endif


    /*Create a Demo*/
    // lv_demo_widgets();
    lv_demo_music();
    _lv_demo_music_play(0);
    //printf("lvgl version:%d.%d.%d \n", lv_version_major(), lv_version_minor(), lv_version_patch());

    /*Handle LitlevGL tasks (tickless mode)*/
    signal(SIGINT, sighandler);
    signal(SIGTERM, sighandler);
    while(flag_running) {
        usleep(1000 * lv_timer_handler());
    }

    return 0;
}

/*Set in lv_conf.h as `LV_TICK_CUSTOM_SYS_TIME_EXPR`*/
uint32_t custom_tick_get(void)
{
    static uint64_t start_ms = 0;
    if(start_ms == 0) {
        struct timeval tv_start;
        gettimeofday(&tv_start, NULL);
        start_ms = (tv_start.tv_sec * 1000000 + tv_start.tv_usec) / 1000;
    }

    struct timeval tv_now;
    gettimeofday(&tv_now, NULL);
    uint64_t now_ms;
    now_ms = (tv_now.tv_sec * 1000000 + tv_now.tv_usec) / 1000;

    uint32_t time_ms = now_ms - start_ms;
    return time_ms;
}
