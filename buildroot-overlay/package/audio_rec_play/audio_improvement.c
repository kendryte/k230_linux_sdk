#include "audio_improvement.h"
#include "audio_enhance.h"
#include "audio_cfg.h"
#include <pthread.h>
#include <list>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define MAX_REF_BUF_NUM 3
#define PREPROCESS_TIME_SEC 60
// 计算 PREPROCESS_TIME_SEC 秒的数据帧数，采样率为 8000Hz，每帧 320 个样本
#define FIXED_PROCESS_FRAMES (PREPROCESS_TIME_SEC * SAMPLE_RATE / BUFFER_SAMPLES)
typedef struct tag_pcm_buf_info
{
  short pcm_buffer[BUFFER_SAMPLES];
  int   pcm_index;
  bool  valid_buf;
}pcm_buf_info;
typedef std::list<pcm_buf_info> REF_BUF_LIST;

static pthread_mutex_t pcm_ref_mutex = PTHREAD_MUTEX_INITIALIZER;
static AUDIO3A_AEC_CONFIG aec_config;
static REF_BUF_LIST pcm_ref_list;
static pcm_buf_info pcm_ref_buf;


static int process_audio3a_aec_fixed_files(AUDIO3A_AEC_CONFIG* aec_cfg)
{
  printf("process_audio3a_aec_fixed_files begin\n");
  short mic_buf[BUFFER_SAMPLES], speaker_buf[BUFFER_SAMPLES], out_buf[BUFFER_SAMPLES];
  // 先处理固定的 PREPROCESS_TIME_SEC 秒文件
  FILE *fixed_mic_fd, *fixed_speaker_fd;
  fixed_mic_fd = fopen("mic_fix.pcm", "rb");
  fixed_speaker_fd = fopen("speaker_fix.pcm", "rb");
  if (fixed_mic_fd == NULL || fixed_speaker_fd == NULL) {
    fprintf(stderr, "Failed to open fixed files\n");
    return -1;
  }

  // 处理固定文件 PREPROCESS_TIME_SEC 秒的数据
  int frame_count = 0;
  while (!feof(fixed_mic_fd) && !feof(fixed_speaker_fd) && frame_count < FIXED_PROCESS_FRAMES) {
    fread(mic_buf, sizeof(short), BUFFER_SAMPLES, fixed_mic_fd);
    fread(speaker_buf, sizeof(short), BUFFER_SAMPLES, fixed_speaker_fd);
    audio3a_aec_process(aec_cfg,(short *)mic_buf,(short *)speaker_buf,out_buf);
    frame_count++;
  }

  printf("process_audio3a_aec_fixed_files end\n");
  return 0;

}

int audio_quality_init(int sample_rate,int frame_size,int filter_length,bool bpreprocess)
{
    memset(&pcm_ref_buf,0,sizeof(pcm_ref_buf));
    aec_config.sample_rate = sample_rate;
    aec_config.frame_size = frame_size;
    aec_config.filter_length = filter_length;

    if (0 != audio3a_aec_init(&aec_config))
    {
        printf("audio3a_aec_init failed\n");
        return -1;
    }

    if (bpreprocess)
    {
      if (0 != process_audio3a_aec_fixed_files(&aec_config))
      {
          printf("process_audio3a_aec_fixed_files failed\n");
          return -1;
      }
    }

    return 0;
}

static int ncount = 0;
int process_reference_audio(char* ref_buf,int size)
{
    memcpy((char*)pcm_ref_buf.pcm_buffer+pcm_ref_buf.pcm_index, ref_buf, size);
    pcm_ref_buf.pcm_index += size;
    if (pcm_ref_buf.pcm_index >= BUFFER_SAMPLES*2) {
      pcm_ref_buf.pcm_index = 0;
      pcm_ref_buf.valid_buf = true;
      pthread_mutex_lock(&pcm_ref_mutex);
      pcm_ref_list.push_back(pcm_ref_buf);
      if (pcm_ref_list.size() > MAX_REF_BUF_NUM) {
        pcm_ref_list.pop_front();
        printf("[%d]play:PCM data list is full\n",ncount++);
      }
      pthread_mutex_unlock(&pcm_ref_mutex);

    }
    return 0;
}

int enhance_audio_quality(char* mic_buf,short *out_buf)
{
    pcm_buf_info pcm_ref_tmp_buf;
    while(1)
    {
      pthread_mutex_lock(&pcm_ref_mutex);
      if (!pcm_ref_list.empty()) {
        pcm_ref_tmp_buf = pcm_ref_list.front();
        pcm_ref_list.pop_front();
        pthread_mutex_unlock(&pcm_ref_mutex);
        break;
      }
      pthread_mutex_unlock(&pcm_ref_mutex);
      usleep(1000*1);
    }

    return audio3a_aec_process(&aec_config,(short *)mic_buf,(short *)pcm_ref_tmp_buf.pcm_buffer,out_buf);

}