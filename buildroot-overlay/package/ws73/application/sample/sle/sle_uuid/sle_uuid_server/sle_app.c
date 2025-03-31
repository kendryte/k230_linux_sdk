/*
 * Copyright (c) @CompanyNameMagicTag 2024-2024. All rights reserved.
 * Description: sle uuid server app.
 */

#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <signal.h>
#include "sle_uuid_server.h"

#define SLE_SERVER_SLEEP_S 5

uint8_t g_server_running = 1;

void signal_handler(int signum)
{
    printf("recv signal: %d, exit sle_server_sample.\n", signum);
    g_server_running = 0;
}

void register_signal(void)
{
    // 注册 SIGINT (Ctrl+C) 信号的处理函数
    if (signal(SIGINT, signal_handler) == SIG_ERR) {
        printf("register signal SIGINT failed.\n");
    }
    // 注册 SIGTERM 信号的处理函数
    if (signal(SIGTERM, signal_handler) == SIG_ERR) {
        printf("register signal SIGTERM failed.\n");
    }
    // 注册 SIGTERM 信号的处理函数
    if (signal(SIGSEGV, signal_handler) == SIG_ERR) {
        printf("register signal SIGSEGV failed.\n");
    }
    // 注册 SIGTERM 信号的处理函数
    if (signal(SIGKILL, signal_handler) == SIG_ERR) {
        printf("register signal SIGKILL failed.\n");
    }
    // 注册 SIGTERM 信号的处理函数
    if (signal(SIGILL, signal_handler) == SIG_ERR) {
        printf("register signal SIGILL failed.\n");
    }
}

int main()
{
    printf("sle_server_sample main start.\n\n");
    register_signal();
    sle_uuid_server_init();
    while (g_server_running) {
        sleep(SLE_SERVER_SLEEP_S);
    }
    sle_uuid_server_deinit();
    return 0;
}