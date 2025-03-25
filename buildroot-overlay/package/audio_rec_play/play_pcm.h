#ifndef __ALSA_PLAY_PCM_H__
#define __ALSA_PLAY_PCM_H__

#include <stdio.h>
#include <string.h>

int init_play_pcm(int sample_rate,int channels,int frame_samples,int bits_per_sample);

int play_pcm(char *pcm_buf);

int deinit_play_pcm();

#endif