#ifndef __AUDIO_IMPROVEMENT_H__
#define __AUDIO_IMPROVEMENT_H__
#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif

// 声音增加初始化
int audio_quality_init(int sample_rate, int frame_size, int filter_length, bool bpreprocess);

// 处理参考声音
int process_reference_audio(char* ref_buf, int size);

// 增强音质
int enhance_audio_quality(char* mic_buf, short *out_buf);

#ifdef __cplusplus
}
#endif

#endif // __AUDIO_IMPROVEMENT_H__