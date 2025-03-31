/*
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "msg_queue.h"
#include "securec.h"
#include "endian.h"
#include "bsle_dft_chan_user.h"
#include "bsle_dft_at_server.h"

#define TEST_SUITE_FUNCTIONS_MAX_ARGS 20
static char *g_test_suite_functions_argv[TEST_SUITE_FUNCTIONS_MAX_ARGS];
static int g_test_suite_functions_argc;
#define TEST_SUITE_FUNCTIONS_MAX_FUNCS 150
static unsigned int g_function_count = 0;
#ifndef WSCFG_ANDROID
static unsigned g_msgqueue_id = -1;
#endif
uint8_t g_bp_dev_enable_state = -1;
#define DFT_STRING_LEN 10

typedef struct {
    char *func_name;
    char *func_description;
    int (*func)(int argc, char *argv[]);
} functions;

static functions g_test_suite_functions_known_funcs[TEST_SUITE_FUNCTIONS_MAX_FUNCS];

typedef struct {
    uint16_t len;
    uint8_t data[0];
} sle_hci_dft_data_buff;

static uint8_t bt_sdk_at_char_to_num_10(uint8_t ch, uint8_t *num)
{
    if (ch >= '0' && ch <= '9') {
        *num = ch - '0';
        return 0;
    }
    return -1;
}

static uint8_t bt_sdk_at_char_to_num_16(uint8_t ch, uint8_t *num)
{
    if (ch >= '0' && ch <= '9') {
        *num = ch - '0';
        return 0;
    } else if (ch >= 'a' && ch <= 'f') {
        *num = ch - 'a' + 10;  // to hex add 10
        return 0;
    } else if (ch >= 'A' && ch <= 'F') {
        *num = ch - 'A' + 10;  // to hex add 10
        return 0;
    }
    return -1;
}

static uint8_t bt_sdk_at_str_to_num16(uint8_t *str, uint16_t *num)
{
    uint8_t *str_cpy = str;
    uint8_t len = (uint8_t)strlen((char *)str_cpy);
    uint8_t tmp_num;
    uint8_t step = 10;
    uint32_t sum = 0;
    uint8_t ret;
    if (len > 2) {  // len 2
        if (str_cpy[0] == '0' && (str_cpy[1] == 'x' || str_cpy[1] == 'X')) {
            str_cpy += 2;  // len 2
            step = 16;     // step 16
            len -= 2;      // len 2
        }
    }
    for (uint8_t i = 0; i < len; i++) {
        if (step == 16) {  // step 16
            ret = bt_sdk_at_char_to_num_16(str_cpy[i], &tmp_num);
        } else {
            ret = bt_sdk_at_char_to_num_10(str_cpy[i], &tmp_num);
        }
        if (ret == -1) {
            return -1;
        }

        sum = sum * step + tmp_num;
        if (sum > 0xFFFF) {
            return -1;
        }
    }
    *num = (uint16_t)sum;
    return 0;
}

static uint8_t bt_sdk_at_str_to_signed_num16(uint8_t *str, uint16_t *num)
{
    uint8_t *str_cpy = str;
    uint8_t len = (uint8_t)strlen((char *)str_cpy);
    uint8_t tmp_num;
    uint8_t step = 10;
    uint8_t flag = 0;
    uint32_t sum = 0;

    uint8_t ret;
    if (len > 1) {
        if (str_cpy[0] == '-') {
            flag = 1;
            len -= 1;
            str_cpy++;
        }
    }

    for (uint8_t i = 0; i < len; i++) {
        ret = bt_sdk_at_char_to_num_10(str_cpy[i], &tmp_num);
        if (ret == -1) {
            return -1;
        }

        sum = sum * step + tmp_num;
        if (sum > 0x7fff) {
            return -1;
        }
    }
    if (flag == 1) {
        sum = 0xFFFF - sum + 1;
    }
    *num = (uint16_t)sum;
    return 0;
}

uint8_t bt_sdk_at_str_to_num8(uint8_t *str, uint8_t *num)
{
    uint8_t ret;
    uint16_t num16 = 0;
    ret = bt_sdk_at_str_to_num16(str, &num16);
    if (num16 > 0xFF) {
        return -1;
    }
    *num = (uint8_t)num16;
    return ret;
}

static uint8_t bt_sdk_at_str_to_data(uint8_t *str, uint16_t data_len, uint8_t *data)
{
    uint8_t *str_cpy = str;
    uint8_t len = (uint8_t)strlen((char *)str_cpy);
    uint8_t num_h;
    uint8_t num_l;

    if (len != data_len * 2) {          // len * 2
        if (len != data_len * 2 + 2) {  // len * 2 + 2
            return -1;
        }
        if (!(str_cpy[0] == '0' && (str_cpy[1] == 'x' || str_cpy[1] == 'X'))) {
            return -1;
        }
        str_cpy += 2;  // len 2
        len -= 2;      // len 2
    }
    for (uint8_t i = 0; i < len - 1; i += 2) {  // 偶数位 2
        if ((bt_sdk_at_char_to_num_16(str_cpy[i], &num_h) == -1) ||
            (bt_sdk_at_char_to_num_16(str_cpy[i + 1], &num_l) == -1)) {
            return -1;
        }
        data[i / 2] = (num_h << 4) | num_l;  // 偶数位 2 move 4
    }
    return 0;
}

static void bt_sdk_int2hex_str(uint32_t num, char *hex_str, int size)
{
    uint32_t num_tmp = num;
    for (int i = size - 1; i >= 0; i--) {
        uint32_t digit = num_tmp & 0xF;  // 0xF获取低4位的值
        hex_str[i] = (digit < DFT_STRING_LEN) ? ('0' + digit) : ('A' + (digit - DFT_STRING_LEN));
        num_tmp >>= 4;  // 将数值右移4位，继续处理高4位
    }
}

int sle_at_cmd_send_dft_data(int argc, char *argv[])
{
    uint16_t data_len = 0;
    uint8_t index = 0;
    uint8_t result = 0;

    if ((argv == NULL) || (argc != 2)) {  // argc num 2
        printf("sle_at_cmd_send_dft_data parameter invaild.\r\n");
        return -1;
    }
    result |= bt_sdk_at_str_to_num16((uint8_t *)argv[index++], &data_len);
    if (data_len <= 0) {
        printf("[Error] sle_at_cmd_send_dft_data str to num failed!\r\n");
        return -1;
    }
    uint8_t *data_buff = malloc(sizeof(uint8_t) * data_len);
    if (data_buff == NULL) {
        printf("[Error] sle_at_cmd_send_dft_data malloc failed!\r\n");
        return -1;
    }
    result |= bt_sdk_at_str_to_data((uint8_t *)(argv[index++]), data_len, data_buff);
    if (result != 0) {
        printf("[Error] sle_at_cmd_send_dft_data: param failed!\r\n");
        free(data_buff);
        return -1;
    }
    result = sle_hci_chan_send_data(data_buff, data_len);
    free(data_buff);
    return result;
}

int ble_at_cmd_send_dft_data(int argc, char *argv[])
{
    uint16_t data_len = 0;
    uint8_t index = 0;
    uint8_t result = 0;

    if ((argv == NULL) || (argc != 2)) {  // argc num 2
        printf("ble_at_cmd_send_dft_data parameter invaild.\r\n");
        return -1;
    }
    result |= bt_sdk_at_str_to_num16((uint8_t *)argv[index++], &data_len);
    if (data_len <= 0) {
        printf("[Error] ble_at_cmd_send_dft_data str to num failed!\r\n");
        return -1;
    }
    uint8_t *data_buff = malloc(sizeof(uint8_t) * data_len);
    if (data_buff == NULL) {
        printf("[Error] malloc failed!\r\n");
    }
    result |= bt_sdk_at_str_to_data((uint8_t *)(argv[index++]), data_len, data_buff);
    if (result != 0) {
        printf("[Error] ble_at_cmd_send_dft_data: param failed!\r\n");
        free(data_buff);
        return -1;
    }
    result = ble_hci_chan_send_data(data_buff, data_len);
    free(data_buff);
    return result;
}

// 加载ko和命令准备
static int bp_at_cmd_dev_enable(void)
{
    g_bp_dev_enable_state = 1;
    char *msg_argv[DFT_AGV_NUM_TWO] = {NULL};
    msg_argv[0] = "4";         // argc len 4
    msg_argv[1] = "00000000";  // bt device open flag
    return ble_at_cmd_send_dft_data(DFT_AGV_NUM_TWO, msg_argv);
}

static int bp_at_cmd_dev_disable(void)
{
    g_bp_dev_enable_state = 0;
    char *msg_argv[DFT_AGV_NUM_TWO] = {NULL};
    msg_argv[0] = "4";         // argc len 4
    msg_argv[1] = "01010101";  // bt device close flag
    return ble_at_cmd_send_dft_data(DFT_AGV_NUM_TWO, msg_argv);
}

static int ble_at_cmd_bp_init(void)
{
    if (g_bp_dev_enable_state != 1) {
        printf("Please enable bt device factory test \n");
        return -1;
    }
    char *msg_argv[DFT_AGV_NUM_TWO] = {NULL};
    msg_argv[0] = "6";             // argc len 6
    msg_argv[1] = "01BDFD020101";  // 产测初始化
    return ble_at_cmd_send_dft_data(DFT_AGV_NUM_TWO, msg_argv);
}

static int ble_at_cmd_bp_deinit(void)
{
    if (g_bp_dev_enable_state != 1) {
        printf("Please enable bt device factory test \n");
        return -1;
    }
    char *msg_argv[DFT_AGV_NUM_TWO] = {NULL};
    msg_argv[0] = "6";             // argc len 6
    msg_argv[1] = "01BDFD020100";  // 产测去初始化
    return ble_at_cmd_send_dft_data(DFT_AGV_NUM_TWO, msg_argv);
}

static int ble_at_cmd_bp_xotrim_corse(int argc, char *argv[])
{
    if (g_bp_dev_enable_state != 1) {
        printf("Please enable bt device factory test \n");
        return -1;
    }
    if (argc != DFT_AGV_INDEX_ONE || argv == NULL) {
        printf("xotrim_corse data parameter invaild.\r\n");
        return -1;
    }
    int32_t ret = 0;
    char *msg_argv[DFT_AGV_NUM_TWO] = {NULL};
    msg_argv[0] = "6";                            // argc len 6
    char *data_buff = malloc(sizeof(char) * 13);  // malloc size 13
    if (data_buff == NULL) {
        printf("malloc fail\n");
        return -1;
    }
    char temp_str[DFT_STRING_LEN] = "01BDFD0202";  // 粗调HCI命令
    ret = memcpy_s(data_buff, DFT_AGV_INDEX_TRIRTEEN, temp_str, sizeof(temp_str));
    if (ret != 0) {
        free(data_buff);
        printf("xotrim_corse memcpy_s fail\n");
        return -1;
    }
    data_buff[DFT_STRING_LEN] = '\0';
    uint8_t value = 0;
    ret = bt_sdk_at_str_to_num8((uint8_t *)argv[0], &value); /* 0 arg idx */
    if (ret == -1) {
        free(data_buff);
        printf("xotrim_corse bt_sdk_at_str_to_num8 fail\n");
        return -1;
    }
    char tmp_data_buff[DFT_AGV_NUM_TWO + 1];
    bt_sdk_int2hex_str(value, tmp_data_buff, sizeof(tmp_data_buff) - 1);
    tmp_data_buff[DFT_AGV_NUM_TWO] = '\0';
    ret = strcat_s(data_buff, DFT_AGV_INDEX_TRIRTEEN, tmp_data_buff);  // malloc size 13
    if (ret != 0) {
        free(data_buff);
        printf("xotrim_corse strcat_s fail\n");
        return -1;
    }
    data_buff[DFT_AGV_INDEX_TRIRTEEN - 1] = '\0';
    msg_argv[1] = data_buff;
    ret = ble_at_cmd_send_dft_data(DFT_AGV_NUM_TWO, msg_argv);
    free(data_buff);
    return ret;
}

static int ble_at_cmd_bp_xotrim_fine(int argc, char *argv[])
{
    if (g_bp_dev_enable_state != 1) {
        printf("Please enable bt device factory test \n");
        return -1;
    }
    if (argc != DFT_AGV_INDEX_ONE || argv == NULL) {
        printf("xotrim fine data parameter invaild.\r\n");
        return -1;
    }
    int32_t ret = 0;
    char *msg_argv[DFT_AGV_NUM_TWO] = {NULL};
    msg_argv[0] = "6";                            // argc len 6
    char *data_buff = malloc(sizeof(char) * 13);  // malloc size 13
    if (data_buff == NULL) {
        printf("malloc fail\n");
        return -1;
    }
    char temp_str[DFT_STRING_LEN] = "01BDFD0203";  // 细调HCI命令
    ret = memcpy_s(data_buff, DFT_AGV_INDEX_TRIRTEEN, temp_str, sizeof(temp_str));
    if (ret != 0) {
        free(data_buff);
        printf("xotrim fine memcpy_s fail\n");
        return -1;
    }
    data_buff[DFT_STRING_LEN] = '\0';
    uint8_t value = 0;
    ret = bt_sdk_at_str_to_num8((uint8_t *)argv[0], &value); /* 0 arg idx */
    if (ret == -1) {
        free(data_buff);
        printf("xotrim_corse bt_sdk_at_str_to_num8 fail\n");
        return -1;
    }
    char tmp_data_buff[DFT_AGV_NUM_TWO + 1] = {0};
    bt_sdk_int2hex_str(value, tmp_data_buff, sizeof(tmp_data_buff) - 1);
    tmp_data_buff[DFT_AGV_NUM_TWO] = '\0';
    ret = strcat_s(data_buff, DFT_AGV_INDEX_TRIRTEEN, tmp_data_buff);  // malloc size 13
    if (ret != 0) {
        free(data_buff);
        printf("xotrim fine strcat_s fail\n");
        return -1;
    }
    data_buff[DFT_AGV_INDEX_TRIRTEEN - 1] = '\0';
    msg_argv[1] = data_buff;
    ret = ble_at_cmd_send_dft_data(DFT_AGV_NUM_TWO, msg_argv);
    free(data_buff);
    return ret;
}

static int ble_at_cmd_bp_read_temp(void)
{
    if (g_bp_dev_enable_state != 1) {
        printf("Please enable bt device factory test \n");
        return -1;
    }
    char *msg_argv[DFT_AGV_NUM_TWO] = {NULL};
    msg_argv[0] = "5";           // argc len 5
    msg_argv[1] = "01BDFD0104";  // 获取温度
    return ble_at_cmd_send_dft_data(DFT_AGV_NUM_TWO, msg_argv);
}

static int ble_at_cmd_bp_xotrim_wr_efuse(void)
{
    if (g_bp_dev_enable_state != 1) {
        printf("Please enable bt device factory test \n");
        return -1;
    }
    char *msg_argv[DFT_AGV_NUM_TWO] = {NULL};
    msg_argv[0] = "5";           // argc len 5
    msg_argv[1] = "01BDFD0105";  // 写频偏校准efuse
    return ble_at_cmd_send_dft_data(DFT_AGV_NUM_TWO, msg_argv);
}

static int ble_at_cmd_bp_xotrim_rd_efuse(void)
{
    if (g_bp_dev_enable_state != 1) {
        printf("Please enable bt device factory test \n");
        return -1;
    }
    char *msg_argv[DFT_AGV_NUM_TWO] = {NULL};
    msg_argv[0] = "5";           // argc len 5
    msg_argv[1] = "01BDFD0106";  // 读频偏校准efuse
    return ble_at_cmd_send_dft_data(DFT_AGV_NUM_TWO, msg_argv);
}

static int ble_at_cmd_bp_temp_wr_efuse(int argc, char *argv[])
{
    if (g_bp_dev_enable_state != 1) {
        printf("Please enable bt device factory test \n");
        return -1;
    }
    if (argc != DFT_AGV_INDEX_ONE || argv == NULL) {
        printf("temp write efuse data parameter invaild.\r\n");
        return -1;
    }
    int32_t ret = 0;
    char *msg_argv[DFT_AGV_NUM_TWO] = {NULL};
    msg_argv[0] = "7";                            // argc len 7
    char *data_buff = malloc(sizeof(char) * 15);  // malloc size 15
    if (data_buff == NULL) {
        printf("malloc fail\n");
        return -1;
    }
    char temp_str[DFT_STRING_LEN] = "01BDFD0307";  // 写温度进efuse
    ret = memcpy_s(data_buff, DFT_AGV_INDEX_FIFTEEN, temp_str, sizeof(temp_str));
    if (ret != 0) {
        free(data_buff);
        printf("temp write efuse memcpy_s fail\n");
        return -1;
    }
    data_buff[DFT_STRING_LEN] = '\0';
    uint16_t value = 0;
    ret = bt_sdk_at_str_to_num16((uint8_t *)argv[0], &value); /* 0 arg idx */
    if (ret == -1) {
        free(data_buff);
        printf("xotrim_corse bt_sdk_at_str_to_num8 fail\n");
        return -1;
    }
    char tmp_data_buff[DFT_AGV_INDEX_FOUR + 1];
    bt_sdk_int2hex_str(htobe16(value), tmp_data_buff, sizeof(tmp_data_buff) - 1);
    tmp_data_buff[DFT_AGV_INDEX_FOUR] = '\0';
    ret = strcat_s(data_buff, DFT_AGV_INDEX_FIFTEEN, tmp_data_buff);  // malloc size 13
    if (ret != 0) {
        free(data_buff);
        printf("temp write efuse strcat_s fail\n");
        return -1;
    }
    data_buff[DFT_AGV_INDEX_FIFTEEN - 1] = '\0';
    msg_argv[1] = data_buff;
    ret = ble_at_cmd_send_dft_data(DFT_AGV_NUM_TWO, msg_argv);
    free(data_buff);
    return ret;
}

static int ble_at_cmd_bp_temp_rd_efuse(void)
{
    if (g_bp_dev_enable_state != 1) {
        printf("Please enable bt device factory test \n");
        return -1;
    }
    char *msg_argv[DFT_AGV_NUM_TWO] = {NULL};
    msg_argv[0] = "5";           // argc len 5
    msg_argv[1] = "01BDFD0108";  // 从efuse读温度
    return ble_at_cmd_send_dft_data(DFT_AGV_NUM_TWO, msg_argv);
}

static int ble_at_cmd_bp_pwrcali_set_curve(int argc, char *argv[])
{
    if (g_bp_dev_enable_state != 1) {
        printf("Please enable bt device factory test \n");
        return -1;
    }
    if (argc != DFT_AGV_INDEX_TWO || argv == NULL) {
        printf("pwrcali_set_curve data parameter invaild.\r\n");
        return -1;
    }
    int32_t ret = 0;
    char *msg_argv[DFT_AGV_NUM_TWO] = {NULL};
    msg_argv[0] = "7";                            // argc len 7
    char *data_buff = malloc(sizeof(char) * 15);  // malloc string size 15
    if (data_buff == NULL) {
        printf("malloc fail\n");
        return -1;
    }
    char temp_str[DFT_STRING_LEN] = "01BDFD0309";  // 设置功率拟合曲线常数项放大系数
    ret = memcpy_s(data_buff, DFT_AGV_INDEX_FIFTEEN, temp_str, sizeof(temp_str));
    if (ret != 0) {
        free(data_buff);
        printf("pwrcali_set_curve memcpy_s fail\n");
        return -1;
    }
    data_buff[DFT_STRING_LEN] = '\0';
    uint8_t value = 0;
    ret = bt_sdk_at_str_to_num8((uint8_t *)argv[0], &value); /* 0 arg idx */
    if (ret == -1) {
        free(data_buff);
        printf("xotrim_corse argv[0] bt_sdk_at_str_to_num8 fail\n");
        return -1;
    }
    char flag_data_buff[DFT_AGV_NUM_TWO + 1];
    bt_sdk_int2hex_str(value, flag_data_buff, sizeof(flag_data_buff) - 1);
    flag_data_buff[DFT_AGV_NUM_TWO] = '\0';
    ret = strcat_s(data_buff, DFT_AGV_INDEX_FIFTEEN, flag_data_buff);  // malloc size 13
    value = 0;
    ret = bt_sdk_at_str_to_num8((uint8_t *)argv[1], &value); /* 1 arg idx */
    if (ret == -1) {
        free(data_buff);
        printf("xotrim_corse argv[1] bt_sdk_at_str_to_num8 fail\n");
        return -1;
    }
    char value_data_buff[DFT_AGV_NUM_TWO + 1];
    bt_sdk_int2hex_str(value, value_data_buff, sizeof(value_data_buff) - 1);
    value_data_buff[DFT_AGV_NUM_TWO] = '\0';
    ret = strcat_s(data_buff, DFT_AGV_INDEX_FIFTEEN, value_data_buff);  // malloc size 13
    if (ret != 0) {
        free(data_buff);
        printf("pwrcali_set_curve strcat_s argv[1] fail\n");
        return -1;
    }
    data_buff[DFT_AGV_INDEX_FIFTEEN - 1] = '\0';
    msg_argv[1] = data_buff;
    ret = ble_at_cmd_send_dft_data(DFT_AGV_NUM_TWO, msg_argv);
    free(data_buff);
    return ret;
}

static int ble_at_cmd_bp_pwrcali_set_pwr(int argc, char *argv[])
{
    if (g_bp_dev_enable_state != 1) {
        printf("Please enable bt device factory test \n");
        return -1;
    }
    if (argc != DFT_AGV_INDEX_TWO || argv == NULL) {
        printf("pwrcali_set_pwr data parameter invaild.\r\n");
        return -1;
    }
    int32_t ret = 0;
    char *msg_argv[DFT_AGV_NUM_TWO] = {NULL};
    msg_argv[0] = "9";                            // argc len 9
    char *data_buff = malloc(sizeof(char) * 19);  // malloc string size 19
    if (data_buff == NULL) {
        printf("malloc fail\n");
        return -1;
    }
    char temp_str[DFT_STRING_LEN] = "01BDFD050A";  // 下发功率，计算得到补偿值
    ret = memcpy_s(data_buff, DFT_AGV_INDEX_NINETEEN, temp_str, sizeof(temp_str));
    if (ret != 0) {
        free(data_buff);
        printf("pwrcali_set_pwr memcpy_s fail\n");
        return -1;
    }
    data_buff[DFT_STRING_LEN] = '\0';
    uint16_t value = 0;
    ret = bt_sdk_at_str_to_num16((uint8_t *)argv[0], &value); /* 0 arg idx */
    if (ret == -1) {
        free(data_buff);
        printf("pwrcali_set_pwr argv[0] bt_sdk_at_str_to_num16 fail\n");
        return -1;
    }
    char target_data_buff[DFT_AGV_INDEX_FOUR + 1];
    bt_sdk_int2hex_str(htobe16(value), target_data_buff, sizeof(target_data_buff) - 1);
    target_data_buff[DFT_AGV_INDEX_FOUR] = '\0';
    ret = strcat_s(data_buff, DFT_AGV_INDEX_NINETEEN, target_data_buff);
    if (ret != 0) {
        free(data_buff);
        printf("pwrcali_set_pwr strcat_s fail\n");
        return -1;
    }
    value = 0;
    char msr_data_buff[DFT_AGV_INDEX_FOUR + 1];
    ret = bt_sdk_at_str_to_num16((uint8_t *)argv[1], &value); /* 1 arg idx */
    if (ret == -1) {
        free(data_buff);
        printf("pwrcali_set_pwr argv[1] bt_sdk_at_str_to_num8 fail\n");
        return -1;
    }
    bt_sdk_int2hex_str(htobe16(value), msr_data_buff, sizeof(msr_data_buff) - 1);
    msr_data_buff[DFT_AGV_INDEX_FOUR] = '\0';
    ret = strcat_s(data_buff, DFT_AGV_INDEX_NINETEEN, msr_data_buff);  // malloc size 19
    if (ret != 0) {
        free(data_buff);
        printf("pwrcali_set_pwr strcat_s argv[1] fail\n");
        return -1;
    }
    data_buff[DFT_AGV_INDEX_NINETEEN - 1] = '\0';
    msg_argv[1] = data_buff;
    ret = ble_at_cmd_send_dft_data(DFT_AGV_NUM_TWO, msg_argv);
    free(data_buff);
    return ret;
}

static int ble_at_cmd_bp_pwrcali_rd_comp(void)
{
    if (g_bp_dev_enable_state != 1) {
        printf("Please enable bt device factory test \n");
        return -1;
    }
    char *msg_argv[DFT_AGV_NUM_TWO] = {NULL};
    msg_argv[0] = "5";           // argc len 5
    msg_argv[1] = "01BDFD010B";  // 读取功率校准补偿值和放大系数
    return ble_at_cmd_send_dft_data(DFT_AGV_NUM_TWO, msg_argv);
}

static int ble_at_cmd_bp_pwrcali_set_comp(void)
{
    if (g_bp_dev_enable_state != 1) {
        printf("Please enable bt device factory test \n");
        return -1;
    }
    char *msg_argv[DFT_AGV_NUM_TWO] = {NULL};
    msg_argv[0] = "5";           // argc len 5
    msg_argv[1] = "01BDFD010C";  // 下发补偿值
    return ble_at_cmd_send_dft_data(DFT_AGV_NUM_TWO, msg_argv);
}

static int ble_at_cmd_bp_pwrcali_wr_efuse(void)
{
    if (g_bp_dev_enable_state != 1) {
        printf("Please enable bt device factory test \n");
        return -1;
    }
    char *msg_argv[DFT_AGV_NUM_TWO] = {NULL};
    msg_argv[0] = "5";           // argc len 5
    msg_argv[1] = "01BDFD010D";  // 写efuse
    return ble_at_cmd_send_dft_data(DFT_AGV_NUM_TWO, msg_argv);
}

static int ble_at_cmd_bp_pwrcali_rd_efuse(void)
{
    if (g_bp_dev_enable_state != 1) {
        printf("Please enable bt device factory test \n");
        return -1;
    }
    char *msg_argv[DFT_AGV_NUM_TWO] = {NULL};
    msg_argv[0] = "5";           // argc len 5
    msg_argv[1] = "01BDFD010E";  // 回读efuse
    return ble_at_cmd_send_dft_data(DFT_AGV_NUM_TWO, msg_argv);
}

static int ble_at_cmd_bp_xotrim_rd(void)
{
    if (g_bp_dev_enable_state != 1) {
        printf("Please enable bt device factory test \n");
        return -1;
    }
    char *msg_argv[DFT_AGV_NUM_TWO] = {NULL};
    msg_argv[0] = "5";           // argc len 5
    msg_argv[1] = "01BDFD010F";  // 读频偏校准efuse
    return ble_at_cmd_send_dft_data(DFT_AGV_NUM_TWO, msg_argv);
}

static int ble_at_cmd_reset_xotrim_and_pwrcali(void)
{
    if (g_bp_dev_enable_state != 1) {
        printf("Please enable bt device factory test \n");
        return -1;
    }
    char *msg_argv[DFT_AGV_NUM_TWO] = {NULL};
    msg_argv[0] = "5";           // argc len 5
    msg_argv[1] = "01BDFD0110";  // 恢复功率初始值
    return ble_at_cmd_send_dft_data(DFT_AGV_NUM_TWO, msg_argv);
}

static int ble_at_cmd_bp_ble_rst(void)
{
    if (g_bp_dev_enable_state != 1) {
        printf("Please enable bt device factory test \n");
        return -1;
    }
    char *msg_argv[DFT_AGV_NUM_TWO] = {NULL};
    msg_argv[0] = "4";         // argc len 4
    msg_argv[1] = "01030C00";  // BLE软复位
    return ble_at_cmd_send_dft_data(DFT_AGV_NUM_TWO, msg_argv);
}

static int ble_at_cmd_bp_ble_tx(int argc, char *argv[])
{
    if (g_bp_dev_enable_state != 1) {
        printf("Please enable bt device factory test \n");
        return -1;
    }
    if (argc != DFT_AGV_INDEX_FOUR || argv == NULL) {
        printf("ble_tx data parameter invaild.\r\n");
        return -1;
    }
    int32_t ret = 0;
    char *msg_argv[DFT_AGV_NUM_TWO] = {NULL};
    msg_argv[0] = "8";                            // argc len 8
    char *data_buff = malloc(sizeof(char) * 17);  // malloc string size 17
    if (data_buff == NULL) {
        printf("malloc fail\n");
        return -1;
    }
    char temp_str[DFT_STRING_LEN] = "01342004";  // BLE TX调制信号
    ret = memcpy_s(data_buff, DFT_AGV_INDEX_SEVENTEEN, temp_str, sizeof(temp_str));
    if (ret != 0) {
        free(data_buff);
        printf("ble_tx memcpy_s fail\n");
        return -1;
    }
    data_buff[DFT_STRING_LEN - DFT_AGV_NUM_TWO] = '\0';
    for (int i = 0; i < 4; i++) {                                        // index :4(0-3)
        ret = strcat_s(data_buff, DFT_AGV_INDEX_SEVENTEEN, argv[i]);  // malloc string size 23
        if (ret != 0) {
            free(data_buff);
            printf("sle_rx strcat_s argv[%d] fail\n", i);
            return -1;
        }
    }
    data_buff[DFT_AGV_INDEX_SEVENTEEN - 1] = '\0';
    msg_argv[1] = data_buff;
    ret = ble_at_cmd_send_dft_data(DFT_AGV_NUM_TWO, msg_argv);
    free(data_buff);
    return ret;
}

static int ble_at_cmd_bp_ble_rx(int argc, char *argv[])
{
    if (g_bp_dev_enable_state != 1) {
        printf("Please enable bt device factory test \n");
        return -1;
    }
    if (argc != DFT_AGV_INDEX_THREE || argv == NULL) {
        printf("ble_rx data parameter invaild.\r\n");
        return -1;
    }
    int32_t ret = 0;
    char *msg_argv[DFT_AGV_NUM_TWO] = {NULL};
    msg_argv[0] = "7";                            // argc len 8
    char *data_buff = malloc(sizeof(char) * 15);  // malloc string size 15
    if (data_buff == NULL) {
        printf("malloc fail\n");
        return -1;
    }
    char temp_str[DFT_STRING_LEN] = "01332003";  // BLE RX调制信号
    ret = memcpy_s(data_buff, DFT_AGV_INDEX_FIFTEEN, temp_str, sizeof(temp_str));
    if (ret != 0) {
        free(data_buff);
        printf("ble_rx memcpy_s fail\n");
        return -1;
    }
    data_buff[DFT_STRING_LEN - DFT_AGV_NUM_TWO] = '\0';
    for (int i = 0; i < 3; i++) {                                        // index :3(0-2)
        ret = strcat_s(data_buff, DFT_AGV_INDEX_FIFTEEN, argv[i]);  // malloc string size 23
        if (ret != 0) {
            free(data_buff);
            printf("sle_rx strcat_s argv[%d] fail\n", i);
            return -1;
        }
    }
    data_buff[DFT_AGV_INDEX_FIFTEEN - 1] = '\0';
    msg_argv[1] = data_buff;
    ret = ble_at_cmd_send_dft_data(DFT_AGV_NUM_TWO, msg_argv);
    free(data_buff);
    return ret;
}

static int ble_at_cmd_bp_ble_trx_end(void)
{
    if (g_bp_dev_enable_state != 1) {
        printf("Please enable bt device factory test \n");
        return -1;
    }
    char *msg_argv[DFT_AGV_NUM_TWO] = {NULL};
    msg_argv[0] = "4";         // argc len 4
    msg_argv[1] = "011F2000";  // 结束BLE TRX
    return ble_at_cmd_send_dft_data(DFT_AGV_NUM_TWO, msg_argv);
}

static int ble_at_cmd_bp_sle_rst(void)
{
    if (g_bp_dev_enable_state != 1) {
        printf("Please enable bt device factory test \n");
        return -1;
    }
    char *msg_argv[DFT_AGV_NUM_TWO] = {NULL};
    msg_argv[0] = "5";           // argc len 4
    msg_argv[1] = "A108040000";  // SLE软复位
    return sle_at_cmd_send_dft_data(DFT_AGV_NUM_TWO, msg_argv);
}

static int ble_at_cmd_bp_sle_tx(int argc, char *argv[])
{
    if (g_bp_dev_enable_state != 1) {
        printf("Please enable bt device factory test \n");
        return -1;
    }
    if (argc != DFT_AGV_INDEX_TEN || argv == NULL) {
        printf("sle_tx data parameter invaild.\r\n");
        return -1;
    }
    int32_t ret = 0;
    size_t i;
    char *msg_argv[DFT_AGV_NUM_TWO] = {NULL};
    msg_argv[0] = "17";                           // argc len 17
    char *data_buff = malloc(sizeof(char) * 35);  // malloc string size 35
    if (data_buff == NULL) {
        printf("malloc fail\n");
        return -1;
    }
    char temp_str[DFT_STRING_LEN] = "A102FC0C00";  // SLE TX调制信号
    ret = memcpy_s(data_buff, DFT_AGV_INDEX_THIRTY_FIVE, temp_str, sizeof(temp_str));
    if (ret != 0) {
        free(data_buff);
        printf("sle_tx memcpy_s fail\n");
        return -1;
    }
    data_buff[DFT_STRING_LEN] = '\0';
    for (i = 0; i < 10; i++) {                                          // index :10(0-9)
        ret = strcat_s(data_buff, DFT_AGV_INDEX_THIRTY_FIVE, argv[i]);  // malloc size 35
        if (ret != 0) {
            free(data_buff);
            printf("sle_tx strcat_s argv[%u] fail\n", i);
            return -1;
        }
    }
    data_buff[DFT_AGV_INDEX_THIRTY_FIVE - 1] = '\0';
    msg_argv[1] = data_buff;
    ret = sle_at_cmd_send_dft_data(DFT_AGV_NUM_TWO, msg_argv);
    free(data_buff);
    return ret;
}

static int ble_at_cmd_bp_sle_rx(int argc, char *argv[])
{
    if (g_bp_dev_enable_state != 1) {
        printf("Please enable bt device factory test \n");
        return -1;
    }
    if (argc != DFT_AGV_INDEX_FIVE || argv == NULL) {
        printf("sle_rx data parameter invaild.\r\n");
        return -1;
    }
    int32_t ret = 0;
    char *msg_argv[DFT_AGV_NUM_TWO] = {NULL};
    msg_argv[0] = "11";                           // argc len 11
    char *data_buff = malloc(sizeof(char) * 23);  // malloc string size 23
    if (data_buff == NULL) {
        printf("malloc fail\n");
        return -1;
    }
    char temp_str[DFT_STRING_LEN] = "A101FC0600";  // SLE RX调制信号
    ret = memcpy_s(data_buff, DFT_AGV_INDEX_TWENTY_THREE, temp_str, sizeof(temp_str));
    if (ret != 0) {
        free(data_buff);
        printf("sle_rx memcpy_s fail\n");
        return -1;
    }
    data_buff[DFT_STRING_LEN] = '\0';
    for (int i = 0; i < 5; i++) {                                        // index :5(0-4)
        ret = strcat_s(data_buff, DFT_AGV_INDEX_TWENTY_THREE, argv[i]);  // malloc string size 23
        if (ret != 0) {
            free(data_buff);
            printf("sle_rx strcat_s argv[%d] fail\n", i);
            return -1;
        }
    }
    data_buff[DFT_AGV_INDEX_TWENTY_THREE - 1] = '\0';
    msg_argv[1] = data_buff;
    ret = sle_at_cmd_send_dft_data(DFT_AGV_NUM_TWO, msg_argv);
    free(data_buff);
    return ret;
}

static int ble_at_cmd_bp_sle_trx_end(void)
{
    if (g_bp_dev_enable_state != 1) {
        printf("Please enable bt device factory test \n");
        return -1;
    }
    char *msg_argv[DFT_AGV_NUM_TWO] = {NULL};
    msg_argv[0] = "5";           // argc len 5
    msg_argv[1] = "A103FC0000";  // 结束SLE TRX
    return sle_at_cmd_send_dft_data(DFT_AGV_NUM_TWO, msg_argv);
}

static int ble_at_cmd_bt_lo_param_check(int argc, char *argv[], int argc_exp)
{
    if (g_bp_dev_enable_state != 1) {
        printf("Please enable bt device factory test \n");
        return -1;
    }
    if (argc != argc_exp || argv == NULL) {
        printf("bt lo data parameter invaild.\r\n");
        return -1;
    }
    return 0;
}

static int ble_at_cmd_bt_lo(int argc, char *argv[])
{
    int32_t ret = ble_at_cmd_bt_lo_param_check(argc, argv, DFT_AGV_INDEX_TWO);
    if (ret != 0) {
        return ret;
    }
    char *msg_argv[DFT_AGV_NUM_TWO] = {NULL};
    msg_argv[0] = "10";                           // argc len 10
    char *data_buff = malloc(sizeof(char) * 21);  // malloc size 21
    if (data_buff == NULL) {
        printf("malloc fail\n");
        return -1;
    }
    char temp_str[DFT_STRING_LEN] = "01B7FD06";  // 单音调测HCI命令
    ret = memcpy_s(data_buff, DFT_AGV_INDEX_TWENTY_ONE, temp_str, sizeof(temp_str));
    if (ret != 0) {
        printf("bt lo memcpy_s fail\n");
        goto cmdend;
    }
    data_buff[DFT_STRING_LEN - DFT_AGV_NUM_TWO] = '\0';
    for (int i = 0; i < 2; i++) {  // index :2(0-1)
        uint8_t value = 0;
        ret = bt_sdk_at_str_to_num8((uint8_t *)argv[i], &value);
        if (ret == -1) {
            printf("bt lo argv[%d] bt_sdk_at_str_to_num8 fail\n", i);
            goto cmdend;
        }
        char flag_data_buff[DFT_AGV_NUM_TWO + 1] = {0};
        bt_sdk_int2hex_str(value, flag_data_buff, sizeof(flag_data_buff) - 1);
        flag_data_buff[DFT_AGV_NUM_TWO] = '\0';
        ret = strcat_s(data_buff, DFT_AGV_INDEX_TWENTY_ONE, flag_data_buff);
        if (ret != 0) {
            printf("bt lo strcat_s argv[%d] fail\n", i);
            goto cmdend;
        }
        ret = strcat_s(data_buff, DFT_AGV_INDEX_TWENTY_ONE, "0000");
        if (ret != 0) {
            printf("bt lo strcat_s argv[0000] fail\n");
            goto cmdend;
        }
    }
    data_buff[DFT_AGV_INDEX_TWENTY_ONE - 1] = '\0';
    msg_argv[1] = data_buff;
    ret = ble_at_cmd_send_dft_data(DFT_AGV_NUM_TWO, msg_argv);
cmdend:
    free(data_buff);
    return ret;
}

static int ble_at_cmd_bt_lo_gain_convert(char *argv[], char *data_buff)
{
    uint16_t gain = 0;
    int32_t ret = 0;
    ret = bt_sdk_at_str_to_signed_num16((uint8_t *)argv[2], &gain); // 第2+1个参数
    if (ret != 0) {
        printf("bt lo arg 3 err\n");
        return ret;
    }
    uint16_t gain_l = (gain & 0xFF) << 8;
    uint16_t gain_h = (gain & 0xFF00) >> 8;
    gain = gain_l | gain_h;
    char flag_data_buff_gain[DFT_AGV_INDEX_FOUR + 1] = {0};
    bt_sdk_int2hex_str(gain, &flag_data_buff_gain, sizeof(flag_data_buff_gain) - 1);
    flag_data_buff_gain[DFT_AGV_INDEX_FOUR] = '\0';
    ret = strcat_s(data_buff, DFT_AGV_INDEX_TWENTY_ONE, flag_data_buff_gain);
    if (ret != 0) {
        printf("bt lo strcat_s argv[0000] fail\n");
        return ret;
    }
    return 0;
}
static int ble_at_cmd_bt_lo_cmd_analy(char *argv[], char *data_buff)
{
    int32_t ret = 0;
    for (int i = 0; i < 2; i++) {  // index :2(0-1)
        uint8_t value = 0;
        ret = bt_sdk_at_str_to_num8((uint8_t *)argv[i], &value);
        if (ret == -1) {
            printf("bt lo argv[%d] bt_sdk_at_str_to_num8 fail\n", i);
            return ret;
        }
        char flag_data_buff[DFT_AGV_NUM_TWO + 1] = {0};
        bt_sdk_int2hex_str(value, &flag_data_buff, sizeof(flag_data_buff) - 1);
        flag_data_buff[DFT_AGV_NUM_TWO] = '\0';
        ret = strcat_s(data_buff, DFT_AGV_INDEX_TWENTY_ONE, flag_data_buff);
        if (ret != 0) {
            printf("bt lo strcat_s argv[%d] fail\n", i);
            return ret;
        }
        if (i == 0) {
            ret = strcat_s(data_buff, DFT_AGV_INDEX_TWENTY_ONE, "0000");
            if (ret != 0) {
                printf("bt lo strcat_s argv[0000] fail\n");
                return ret;
            }
        } else {
            ret = ble_at_cmd_bt_lo_gain_convert(&argv[0], data_buff);
            if (ret != 0) {
                return ret;
            }
        }
    }
    return 0;
}
static int ble_at_cmd_bt_lo_with_gain(int argc, char *argv[])
{
    int32_t ret = ble_at_cmd_bt_lo_param_check(argc, argv, DFT_AGV_INDEX_THREE);
    if (ret != 0) {
        return ret;
    }
    char *msg_argv[DFT_AGV_NUM_TWO] = {NULL};
    msg_argv[0] = "10";                           // argc len 10
    char *data_buff = malloc(sizeof(char) * 21);  // malloc size 21
    if (data_buff == NULL) {
        printf("malloc fail\n");
        return -1;
    }
    char temp_str[DFT_STRING_LEN] = "01B7FD06";  // 单音调测HCI命令
    ret = memcpy_s(data_buff, DFT_AGV_INDEX_TWENTY_ONE, temp_str, sizeof(temp_str));
    if (ret != 0) {
        printf("bt lo memcpy_s fail\n");
        goto cmdend;
    }
    data_buff[DFT_STRING_LEN - DFT_AGV_NUM_TWO] = '\0';
    ret = ble_at_cmd_bt_lo_cmd_analy(&argv[0], data_buff);
    if (ret != 0) {
        goto cmdend;
    }
    data_buff[DFT_AGV_INDEX_TWENTY_ONE - 1] = '\0';
    msg_argv[1] = data_buff;
    ret = ble_at_cmd_send_dft_data(DFT_AGV_NUM_TWO, msg_argv);
cmdend:
    free(data_buff);
    return ret;
}

static int ble_at_cmd_get_sle_rssi(void)
{
    if (g_bp_dev_enable_state != 1) {
        printf("Please enable bt device factory test \n");
        return -1;
    }
    char *msg_argv[DFT_AGV_NUM_TWO] = {NULL};
    msg_argv[0] = "5";           // argc len 5
    msg_argv[1] = "01BDFD0111";  // 读取SLE RSSI
    return ble_at_cmd_send_dft_data(DFT_AGV_NUM_TWO, msg_argv);
}

int add_function(char *name, char *description, int (*func)(void))
{
    if (g_function_count + 1 >= TEST_SUITE_FUNCTIONS_MAX_FUNCS) {
        return 1;
    }
    g_test_suite_functions_known_funcs[g_function_count].func_name = name;
    g_test_suite_functions_known_funcs[g_function_count].func_description = description;
    g_test_suite_functions_known_funcs[g_function_count].func = func;
    g_function_count++;
    return 0;
}
void bgh_dft_command_register(void)
{
    // 加载ko和命令准备
    add_function("bpcmd^dev_enable", "bpcmd dev_enable", bp_at_cmd_dev_enable);
    add_function("bpcmd^dev_disable", "bpcmd dev_disable", bp_at_cmd_dev_disable);
    add_function("bpcmd^bp_init", "bpcmd bp_init", ble_at_cmd_bp_init);
    add_function("bpcmd^bp_deinit", "bpcmd bp_deinit", ble_at_cmd_bp_deinit);
    // 频偏校准
    add_function("bpcmd^xotrim_corse", "bpcmd xotrim_corse <reg_val>", ble_at_cmd_bp_xotrim_corse);
    add_function("bpcmd^xotrim_fine", "bpcmd xotrim_fine <reg_val>", ble_at_cmd_bp_xotrim_fine);
    add_function("bpcmd^read_temp", "bpcmd read_temp", ble_at_cmd_bp_read_temp);
    add_function("bpcmd^xotrim_wr_efuse", "bpcmd xotrim_wr_efuse", ble_at_cmd_bp_xotrim_wr_efuse);
    add_function("bpcmd^xotrim_rd_efuse", "bpcmd xotrim_rd_efuse", ble_at_cmd_bp_xotrim_rd_efuse);
    add_function("bpcmd^temp_wr_efuse", "bpcmd temp_wr_efuse <temp>", ble_at_cmd_bp_temp_wr_efuse);
    add_function("bpcmd^temp_rd_efuse", "bpcmd temp_rd_efuse", ble_at_cmd_bp_temp_rd_efuse);
    // 功率校准
    add_function("bpcmd^pwrcali_set_curve", "bpcmd pwrcali_set_curve <flag> <value>", ble_at_cmd_bp_pwrcali_set_curve);
    add_function(
        "bpcmd^pwrcali_set_pwr", "bpcmd pwrcali_set_pwr  <target_per> <msr_pwr>", ble_at_cmd_bp_pwrcali_set_pwr);
    add_function("bpcmd^pwrcali_rd_comp", "bpcmd pwrcali_rd_comp", ble_at_cmd_bp_pwrcali_rd_comp);
    add_function("bpcmd^pwrcali_set_comp ", "bpcmd pwrcali_set_comp ", ble_at_cmd_bp_pwrcali_set_comp);
    add_function("bpcmd^pwrcali_wr_efuse", "bpcmd pwrcali_wr_efuse", ble_at_cmd_bp_pwrcali_wr_efuse);
    add_function("bpcmd^pwrcali_rd_efuse", "bpcmd pwrcali_rd_efuse ", ble_at_cmd_bp_pwrcali_rd_efuse);

    // RF测试TRX
    add_function("bpcmd^ble_rst", "bpcmd ble_rst", ble_at_cmd_bp_ble_rst);
    add_function("bpcmd^ble_tx", "bpcmd ble_tx <chnl> <data_len> <payload_type> <phy>", ble_at_cmd_bp_ble_tx);
    add_function("bpcmd^ble_rx", "bpcmd ble_rx <chnl> <phy> <modulation>", ble_at_cmd_bp_ble_rx);
    add_function("bpcmd^ble_trx_end ", "bpcmd ble_trx_end ", ble_at_cmd_bp_ble_trx_end);
    add_function("bpcmd^sle_rst", "bpcmd sle_rst", ble_at_cmd_bp_sle_rst);
    add_function("bpcmd^sle_tx",
        "bpcmd sle_tx <channel> <tx_power> <data_len> <payload_type> <phy> <format> <rate> <pilot_ratio> <polar> "
        "<interval>",
        ble_at_cmd_bp_sle_tx);
    add_function(
        "bpcmd^sle_rx", "bpcmd sle_rx <channel> <phy> <format> <pilot_ratio> <interval>", ble_at_cmd_bp_sle_rx);
    add_function("bpcmd^sle_trx_end", "bpcmd sle_trx_end", ble_at_cmd_bp_sle_trx_end);
    add_function("bpcmd^bt_lo", "bpcmd bt_lo <freq> <mode>", ble_at_cmd_bt_lo);
    add_function("bpcmd^bt_lo_gain", "bpcmd bt_lo_gain <freq> <mode> <gain>", ble_at_cmd_bt_lo_with_gain);
    add_function("bpcmd^get_sle_rssi", "bpcmd get_sle_rssi", ble_at_cmd_get_sle_rssi);

    add_function("sle_send_dft_data", "sle_send_dft_data  < len >  [ data ]", sle_at_cmd_send_dft_data);
    add_function("ble_send_dft_data", "ble_send_dft_data  < len >  [ data ]", ble_at_cmd_send_dft_data);
    add_function("bpcmd^xotrim_rd_reg", "bpcmd xotrim_rd_reg", ble_at_cmd_bp_xotrim_rd);
    add_function("bpcmd^cali_reset", "bpcmd cali_reset", ble_at_cmd_reset_xotrim_and_pwrcali);
}

static char test_suite_cmd_helper_split(char *result[], char res_len, char *str, char split_char)
{
    /* replace split_char with '\0' and return the positions of the start of the strings */
    int i = 0;
    char *string = str;
    bool new = true;
    while ((*string) != 0) {
        if (new) {
            if ((*string) == split_char) {
                string++;
                continue;
            }
            result[i] = string;
            if (++i >= res_len) {
                return (char)(i - 1);
            }
            new = false;
        }
        if (*string == split_char) {
            *string = '\0';
            new = true;
        }
        string++;
    }
    return (char)i;
}

static void test_suite_functions_split_command(char *command)
{
    /* initialise the command array to zeros. */
    for (int i = 0; i < TEST_SUITE_FUNCTIONS_MAX_ARGS; i++) {
        g_test_suite_functions_argv[i] = 0;
    }
    g_test_suite_functions_argc =
        test_suite_cmd_helper_split(g_test_suite_functions_argv, TEST_SUITE_FUNCTIONS_MAX_ARGS, command, ' ');
}

static errno_t handle_msg(char *buf, int buf_size)
{
    int ret = 0;
#ifdef WSCFG_ANDROID
    if (buf_size <= 1) {
        return EINVAL;
    }
    (void)fgets(buf, buf_size-1, stdin);
#else
    ret = test_suite_recv_msg(g_msgqueue_id, CLIENT_TYPE, buf, buf_size); /* server删除从client发送来的消息 */
#endif
    if (strlen((char*)buf) <= 0) {
        return EINVAL;
    }
    buf[strlen((char *)buf) - 1] = '\0'; /* ！！！将结尾的换行替换成结束 */
    test_suite_functions_split_command(buf);
    if (ret == 0 && g_test_suite_functions_argc != 0) {
        if (strncasecmp(buf, "Quit", 4) == 0) {  // quit len 4
            printf("client Quit!\n");
            return -EINTR;
        }
#ifndef WSCFG_ANDROID
        if (strncasecmp(buf, START_MSG, 5) == 0) {                       // start len 5
            test_suite_send_msg(g_msgqueue_id, SERVER_TYPE, START_MSG);  // 发送一条启动消息
            return EOK;
        }
#endif
        for (unsigned int i = 0; i < g_function_count; i++) {
            if (strncasecmp(g_test_suite_functions_argv[0], g_test_suite_functions_known_funcs[i].func_name,
                strlen(g_test_suite_functions_argv[0])) == 0) {
                g_test_suite_functions_known_funcs[i].func(
                    g_test_suite_functions_argc - 1, g_test_suite_functions_argv + 1);
                break;
            }
        }
    }
    return EOK;
}

int cmd_main_add_functions(void)
{
#ifndef WSCFG_ANDROID
    g_msgqueue_id = (unsigned)test_suite_new_queue();
    if (g_msgqueue_id < 0) {
        printf("create queue error");
        return -1;
    }
#endif
    char buf[SIZE];
    bgh_dft_command_register();
    while (1) {
        if (memset_s(buf, sizeof(buf), '\0', sizeof(buf)) != EOK) { /* buf保存消费掉的消息 */
            perror("memset_s error");
        }
        if (handle_msg(buf, SIZE)) {
            break;
        }
    }
#ifdef WSCFG_ANDROID
    return 0;
#else
    return test_suite_delete_queue(g_msgqueue_id); /* 最后由server删除消息队列 */
#endif
}