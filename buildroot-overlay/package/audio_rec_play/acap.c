#include "acap.h"
#include "capture_pcm.h"
#include "play_pcm.h"
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include "audio_cfg.h"
#include "audio_improvement.h"

#define ENABLE_DEBUG_AUDIO 0
static pthread_t capture_thread, play_thread;
static char pcm_capture_buf[BUFFER_SAMPLES * BITS_PER_SAMPLE / 8];
static char pcm_paly_buf[BUFFER_SAMPLES * BITS_PER_SAMPLE / 8];
static int stop_threads = 0; // 用于控制线程的停止
static char pcm_out_buf[BUFFER_SAMPLES * BITS_PER_SAMPLE / 8];

// 采集音频的线程函数
static void* capture_thread_func(void* arg) {
    FILE* pcm_output_file = fopen(CAPTURE_PCM_FILENAME, "wb");
    if (!pcm_output_file) {
        perror("Failed to open output PCM file");
        return NULL;
    }
#if ENABLE_DEBUG_AUDIO
    FILE* pcm_mic_file = fopen("mic.pcm", "wb");
    if (!pcm_mic_file) {
        perror("Failed to open mic PCM file");
        return NULL;
    }
#endif

    while (!stop_threads) {
        // 采集 PCM 数据
        if (0 != capture_pcm(pcm_capture_buf)) {
            usleep(1000 * 1);
            continue;
        }

        // 这里可以添加处理 PCM 数据的逻辑
#if ENABLE_DEBUG_AUDIO
        fwrite(pcm_capture_buf, 1, sizeof(pcm_capture_buf), pcm_mic_file);
#endif

        enhance_audio_quality(pcm_capture_buf,(short*)pcm_out_buf);
        fwrite(pcm_out_buf, 1, sizeof(pcm_out_buf), pcm_output_file);
    }
    printf("capture_thread_func end\n");

    fclose(pcm_output_file);
#if ENABLE_DEBUG_AUDIO
    fclose(pcm_mic_file);
#endif
    return NULL;
}

// 播放音频的线程函数
static void* play_thread_func(void* arg) {
    FILE* pcm_file = fopen("audio.pcm", "rb");
    if (!pcm_file) {
        // 如果当前目录打开失败，尝试打开 /usr/bin/audio.pcm
        pcm_file = fopen("/usr/bin/audio.pcm", "rb");
        if (!pcm_file)
        {
            perror("Failed to open PCM file");
            return NULL;
        }
    }

#if ENABLE_DEBUG_AUDIO
    FILE* pcm_speaker_file = fopen("speaker.pcm", "wb");
    if (!pcm_speaker_file) {
        perror("Failed to open speaker PCM file");
        return NULL;
    }
#endif

    while (!stop_threads) {
        // 从文件读取 PCM 数据
        size_t read_size = fread(pcm_paly_buf, 1, sizeof(pcm_paly_buf), pcm_file);
        if (read_size < sizeof(pcm_paly_buf)) {
            // 如果文件读取完毕，回到文件开头
            fseek(pcm_file, 0, SEEK_SET);
            printf("play again\n");
            continue;
        }

#if ENABLE_DEBUG_AUDIO
        fwrite(pcm_paly_buf,1,sizeof(pcm_paly_buf),pcm_speaker_file);
#endif

        process_reference_audio(pcm_paly_buf,sizeof(pcm_paly_buf));

        // 播放 PCM 数据
        play_pcm(pcm_paly_buf);

    }
    printf("play_thread_func end\n");

    fclose(pcm_file);

#if ENABLE_DEBUG_AUDIO
    fclose(pcm_speaker_file);
#endif
    return NULL;
}

void acap_init() {
    audio_quality_init(SAMPLE_RATE,BUFFER_SAMPLES,2000,false);
    init_capture_pcm(SAMPLE_RATE, CHANNELS_NUM, BUFFER_SAMPLES, BITS_PER_SAMPLE);
    init_play_pcm(SAMPLE_RATE, CHANNELS_NUM, BUFFER_SAMPLES, BITS_PER_SAMPLE);
}

void acap_deinit() {
    deinit_capture_pcm();
    deinit_play_pcm();
}

void acap_start() {

    // 创建采集音频的线程
    if (pthread_create(&capture_thread, NULL, capture_thread_func, NULL) != 0) {
        perror("Failed to create capture thread");
        return;
    }

    // 创建播放音频的线程
    if (pthread_create(&play_thread, NULL, play_thread_func, NULL) != 0) {
        perror("Failed to create play thread");
        return;
    }

}

void acap_stop() {
    stop_threads = 1; // 设置标志位，停止线程
    // 等待线程结束
    pthread_join(capture_thread, NULL);
    pthread_join(play_thread, NULL);
}

// 播放指定 PCM 文件的函数
void acap_play_pcm_file(const char* file_path) {
    FILE* pcm_file = fopen(file_path, "rb");
    if (!pcm_file) {
        perror("Failed to open PCM file");
        return;
    }

    init_play_pcm(SAMPLE_RATE, CHANNELS_NUM, BUFFER_SAMPLES, BITS_PER_SAMPLE);

    char pcm_play_buf[BUFFER_SAMPLES * BITS_PER_SAMPLE / 8];

    while (true) {
        // 从文件读取 PCM 数据
        size_t read_size = fread(pcm_play_buf, 1, sizeof(pcm_play_buf), pcm_file);
        if (read_size < sizeof(pcm_play_buf)) {
            // 如果文件读取完毕，退出循环
            printf("End of PCM file reached\n");
            break;
        }

        // 播放 PCM 数据
        play_pcm(pcm_play_buf);
    }

    deinit_play_pcm();
    fclose(pcm_file);
}