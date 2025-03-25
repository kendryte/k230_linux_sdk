#ifndef __ALSA_CAPTURE_PCM_H
#define __ALSA_CAPTURE_PCM_H

#include<stdio.h>
#include<string.h>

int init_capture_pcm(int sample_rate, int num_channels,int frame_samples,int bits_per_sample);

int capture_pcm(char* pcm_out_buffer);

int deinit_capture_pcm();

#endif // __ALSA_CAPTURE_PCM_H