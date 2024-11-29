/* Copyright (c) 2023, Canaan Bright Sight Co., Ltd
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>

#include "alsa_play_wav.h"
#include "alsa_play_mp3.h"
#include "alsa_rec_wav.h"

static int      g_sample_rate = 44100;
static int     g_channels = 2;
static char g_file_name[256];
static int       g_type = -1;


static  void _help()
{
    printf("Please input:\n");
    printf("-type: test mapi audio function[0-2]\n");
    printf("  type 0:play wav by alsa\n");
    printf("  type 1:play mp3 by alsa\n");
    printf("  type 2:record wav by alsa\n");

    printf("-samplerate: set audio sample(8000 ~ 192000)\n");
    printf("-filename: load or save file name\n");
    printf("-channels: audio channels(1,2)\n");
}

int main(int argc, char *argv[])
{
    if (argc <= 1)
    {
        _help();
        return -1;
    }

    for (int i = 1; i < argc; i += 2)
    {
        if (strcmp(argv[i], "-help") == 0)
        {
            _help();
            return 0;
        }
        else if (strcmp(argv[i], "-samplerate") == 0)
        {
            g_sample_rate = atoi(argv[i + 1]);
        }
        else if (strcmp(argv[i], "-channels") == 0)
        {
            g_channels = atoi(argv[i + 1]);
        }
        else if (strcmp(argv[i], "-type") == 0)
        {
            g_type = atoi(argv[i + 1]);
        }
        else if (strcmp(argv[i], "-filename") == 0)
        {
            memcpy(g_file_name,argv[i + 1],strlen(argv[i + 1])+1);
        }
        else
        {
            printf("Error :Invalid arguments %s\n", argv[i]);
            _help();
            return -1;
        }
    }

    printf("audio type:%d,sample rate:%d,channels:%d,filename:%s\n", g_type, g_sample_rate,g_channels,g_file_name);

    if (0 == g_type)
    {
        int ret;
        WAVINFO waveinfo;

        ret = analyseWaveFile(g_file_name,&waveinfo);
        if(ret == 0){
            printf("channels:%d sampleRate:%d bitsPerSample:%d dataSize:%d dataFoft:%d\n",waveinfo.channels,
                waveinfo.sampleRate,waveinfo.bitsPerSample,waveinfo.dataSize,waveinfo.dataFoft);
        }
        else{
            printf("ERR %d: analyse WaveFile %s failed.\n",__LINE__,g_file_name);
            return -1;
        }

        playerPcm(g_file_name,&waveinfo);
    }
    else if (1 == g_type)
    {
        k_mp3_info_t mp3info;
        k_analyse_mp3_file(g_file_name,&mp3info);
        k_play_mp3(g_file_name,&mp3info);
    }
    else if (2 == g_type)
    {
        alsa_record_wav(g_file_name,15,g_sample_rate,g_channels);
    }
    else
    {
        printf("ERR %d: audio type %d is not supported.\n",__LINE__,g_type);
        return -1;
    }

    printf("sample done\n");
    return 0;
}
