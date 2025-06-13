#ifndef SMOLRTSP_SERVER_H
#define SMOLRTSP_SERVER_H
#include <stdint.h>
#include <stddef.h>
int smolrtsp_server_start();
int smolrtsp_server_stop();
int smolrtsp_send_audio_stream(const uint8_t *data, size_t size, uint64_t timestamp);
int smolrtsp_send_video_stream(const uint8_t *data, size_t size, uint64_t timestamp);
#endif // SMOLRTSP_SERVER_H