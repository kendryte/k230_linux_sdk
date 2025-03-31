/*
 * Copyright (c) @CompanyNameMagicTag. 2023. All rights reserved.
 * Description: Main function of sle client sample.
 */
#include <unistd.h>
#include <stddef.h>
#include <stdio.h>
#include <signal.h>
#include "stdint.h"
#include "sle_common.h"
#include "sle_connection_manager.h"
#include "sle_ssap_client.h"
#include "sle_uuid_client.h"

#define SLE_CLIENT_SLEEP_TIME  5

uint8_t g_running = 1;

void signal_handler(int signum)
{
    printf("接收到信号 %d\n", signum);
    g_running = 0;
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

void main(void)
{
    printf("main start.\n\n");
    register_signal();  // 注册信号处理

    sle_client_init();  // SLE client初始化

    while (g_running) {  // 收到信号g_running状态改变
        sleep(SLE_CLIENT_SLEEP_TIME);  // 5
    }

    sle_client_deinit();  // SLE client去初始化

    printf("main end.\n\n");
}