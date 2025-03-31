/*
 * Copyright (c) @CompanyNameMagicTag 2024-2024. All rights reserved.
 * Description: ble server app.
 */

#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <signal.h>
#include "ble_uuid_server.h"

#define BLE_SERVER_SLEEP_S 5

uint8_t g_server_running = 1;

void ble_uuid_server_signal_handler(int signum)
{
    printf("recv signal: %d, exit ble_server_sample.\n", signum);
    g_server_running = 0;
}

void ble_uuid_server_register_signal(void)
{
    // 注册 SIGINT (Ctrl+C) 信号的处理函数
    if (signal(SIGINT, ble_uuid_server_signal_handler) == SIG_ERR) {
        printf("register signal SIGINT failed.\n");
    }
    // 注册 SIGTERM 信号的处理函数
    if (signal(SIGTERM, ble_uuid_server_signal_handler) == SIG_ERR) {
        printf("register signal SIGTERM failed.\n");
    }
    // 注册 SIGTERM 信号的处理函数
    if (signal(SIGSEGV, ble_uuid_server_signal_handler) == SIG_ERR) {
        printf("register signal SIGSEGV failed.\n");
    }
    // 注册 SIGTERM 信号的处理函数
    if (signal(SIGKILL, ble_uuid_server_signal_handler) == SIG_ERR) {
        printf("register signal SIGKILL failed.\n");
    }
    // 注册 SIGTERM 信号的处理函数
    if (signal(SIGILL, ble_uuid_server_signal_handler) == SIG_ERR) {
        printf("register signal SIGILL failed.\n");
    }
}

int main()
{
    printf("ble_server_sample main start.\n\n");
    ble_uuid_server_register_signal();
    ble_uuid_server_init();
    while (g_server_running) {
        sleep(BLE_SERVER_SLEEP_S);
    }
    ble_uuid_server_deinit();
    return 0;
}