#ifndef GET_PCM_H
#define GET_PCM_H

#include<stdio.h>
#include<string.h>
#include <vector>

using namespace std;

int initPcm(unsigned int sample_rate, int num_channels);

void getPcm(std::vector<float> &wav);

int deinitPcm();

#endif // GET_PCM_H