/*
 * Copyright (c) @CompanyNameMagicTag 2024-2024. All rights reserved.
 * Description: ble client app.
 */

#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <signal.h>
#include "ble_gatt_client.h"

#define BLE_CLIENT_SLEEP_S 5

uint8_t g_client_running = 1;

void ble_gatt_client_signal_handler(int signum)
{
    printf("recv signal: %d, exit ble_client_sample.\n", signum);
    g_client_running = 0;
}

void ble_gatt_client_register_signal(void)
{
    // 注册 SIGINT (Ctrl+C) 信号的处理函数
    if (signal(SIGINT, ble_gatt_client_signal_handler) == SIG_ERR) {
        printf("register signal SIGINT failed.\n");
    }
    // 注册 SIGTERM 信号的处理函数
    if (signal(SIGTERM, ble_gatt_client_signal_handler) == SIG_ERR) {
        printf("register signal SIGTERM failed.\n");
    }
    // 注册 SIGTERM 信号的处理函数
    if (signal(SIGSEGV, ble_gatt_client_signal_handler) == SIG_ERR) {
        printf("register signal SIGSEGV failed.\n");
    }
    // 注册 SIGTERM 信号的处理函数
    if (signal(SIGKILL, ble_gatt_client_signal_handler) == SIG_ERR) {
        printf("register signal SIGKILL failed.\n");
    }
    // 注册 SIGTERM 信号的处理函数
    if (signal(SIGILL, ble_gatt_client_signal_handler) == SIG_ERR) {
        printf("register signal SIGILL failed.\n");
    }
}

int main()
{
    printf("ble_client_sample main start.\n\n");
    ble_gatt_client_register_signal();
    ble_gatt_client_init();
    while (g_client_running) {
        sleep(BLE_CLIENT_SLEEP_S);
    }
    ble_gatt_client_deinit();
    return 0;
}