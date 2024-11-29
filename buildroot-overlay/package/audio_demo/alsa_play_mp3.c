#include "alsa_play_mp3.h"
#define MINIMP3_IMPLEMENTATION 1
#include <stdio.h>
#include "minimp3.h"
#include<stdio.h>

#ifdef __cplusplus
extern "C"
{
#endif
#include "alsa/asoundlib.h"
#include "alsa/pcm.h"
#ifdef __cplusplus
};
#endif

#include <string.h>
#define MP3_BUF_SIZE 4096
#define MAX_CACHE_BUFFER_SIZE MINIMP3_MAX_SAMPLES_PER_FRAME*8
static char* sMaxbuf = NULL;
static int bufIndex = 0;
static int bufSize = 0;

int k_analyse_mp3_file(char *filename, k_mp3_info_t *mp3info)
{
    mp3dec_t mp3d;
    // 定义mp3dec_frame_info_t
    mp3dec_frame_info_t frame_info;
    int mp3len = 0;
    short file_data[MINIMP3_MAX_SAMPLES_PER_FRAME];

    FILE *f = fopen(filename, "rb");
    if (f == NULL)
    {
        printf("open file failed\n");
        return -1;
    }

    mp3dec_init(&mp3d);
    while (1)
    {
        mp3len = fread((char *)file_data, 1, sizeof(file_data), f);
        if (mp3len <= 0)
        {
            return -1;
        }

        int samples_to_get = mp3dec_decode_frame(&mp3d, file_data, sizeof(file_data), NULL, &frame_info);
        if (samples_to_get > 0)
        {
            printf("channels:%d,sample rate:%d,frame_byte:%d,bitrate_kbps:%d,layer:%d\n",
                   frame_info.channels, frame_info.hz, frame_info.frame_bytes, frame_info.bitrate_kbps, frame_info.layer);

            break;
        }
    }

    fclose(f);
    mp3info->frame_bytes = frame_info.frame_bytes;
    mp3info->channels = frame_info.channels;
    mp3info->rate_sample = frame_info.hz;
    mp3info->layer = frame_info.layer;
    mp3info->bitrate_kbps = frame_info.bitrate_kbps;

    return 0;
}

static int cache_init(const k_mp3_info_t* mp3info) {
    bufIndex = 0;
    bufSize = 0;
    sMaxbuf = (char *)malloc(MAX_CACHE_BUFFER_SIZE);
    return 0;
}

static int cache_data(const char *data, int dataSize) {
    if (bufIndex + dataSize <= MAX_CACHE_BUFFER_SIZE) {
        memcpy(sMaxbuf + bufIndex, data, dataSize);
        bufIndex += dataSize;
        bufSize += dataSize;
        return 0;
    }
    return -1;
}

static int get_fixed_data(char *output, int outputSize) {
    if (bufSize >= outputSize) {
        memcpy(output, sMaxbuf, outputSize);
        memmove(sMaxbuf, sMaxbuf + outputSize, bufSize - outputSize);
        bufIndex -= outputSize;
        bufSize -= outputSize;
        return 0;
    }
    return -1;
}

static int cache_deinit() {
    if (NULL != sMaxbuf)
    {
        free(sMaxbuf);
        sMaxbuf = NULL;
        bufIndex = 0;
        bufSize = 0;
    }
}

int k_play_mp3(char* filename, const k_mp3_info_t* mp3info)
{
	int rc;
    snd_pcm_t *handle;
    snd_pcm_hw_params_t *params;
    unsigned int val;
    int dir = 0;
    snd_pcm_uframes_t frames;
    int size;
	int forGetFrames;
	int ret = -1;

    mp3dec_t mp3d;
    // 定义mp3dec_frame_info_t
    mp3dec_frame_info_t frame_info;
    int mp3len = 0;
    int ilen = 0;

    int fixed_len =  (mp3info->rate_sample / 25) * 2 * mp3info->channels;
    char* fixed_data = (char *)malloc(fixed_len);
    mp3d_sample_t sample_data[MINIMP3_MAX_SAMPLES_PER_FRAME];//每次解码最大的样本数

    FILE *f = fopen(filename, "rb");
    if (f == NULL)
    {
        printf("open file failed\n");
        free (fixed_data);
        return -1;
    }

    mp3dec_init(&mp3d);
    cache_init(mp3info);

    rc = snd_pcm_open(&handle, "default", SND_PCM_STREAM_PLAYBACK, 0);
    if (rc < 0) {
        printf("unable to open PCM device: %s\n",snd_strerror(rc));
        free (fixed_data);
        return -1;
    }

    /* alloc hardware params object */
    snd_pcm_hw_params_alloca(&params);

    /* fill it with default values */
    snd_pcm_hw_params_any(handle, params);

    /* interleaved mode */
    snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);

	/* signed 16 bit little ending format */
    snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S16_LE);

    /* two channels (stereo) */
    snd_pcm_hw_params_set_channels(handle, params, mp3info->channels);

    /* 44100 bits/second sampling rate (CD quality) */
    val = mp3info->rate_sample;
    snd_pcm_hw_params_set_rate_near(handle, params, &val, &dir);
	//printf("pcm rate: val:%d dir:%d.\n",val,dir);

    /* set period size t 40ms frames */
	frames = mp3info->rate_sample/25;
    rc = snd_pcm_hw_params_set_period_size_near(handle, params, &frames, &dir);
	//printf("%d rc = %d,pcm frames: frames:%ld dir:%d.\n",__LINE__,rc,frames,dir);

    /* write params to the driver */
    rc = snd_pcm_hw_params(handle, params);
    if (rc < 0) {
        printf("unable to set hw params: %s\n",snd_strerror(rc));
        goto OUT;
    }

    /* use buffer large enough to hold one period */
    rc = snd_pcm_hw_params_get_period_size(params, &frames, &dir);
    printf("%d,rc = %d,frames:%ld dir:%d.\n",__LINE__,rc,frames,dir);

    char sBuf[MP3_BUF_SIZE];
    fread(sBuf,1,sizeof(sBuf),f);

    int samples_to_get = mp3dec_decode_frame(&mp3d, sBuf, sizeof(sBuf), sample_data, &frame_info);

    //while (samples_to_get > 0)
    while (1)
    {
        //printf("============samples_to_get:%d\n",samples_to_get* 2 * mp3info->channels);
        cache_data((char *)sample_data, samples_to_get * 2 * mp3info->channels);
        if (0 == get_fixed_data(fixed_data, fixed_len))
        {
            rc = snd_pcm_writei(handle, fixed_data, frames);
            if (rc == -EPIPE) {
                /* -EPIPE means underrun */
                fprintf(stderr, "underrun occured\n");
                snd_pcm_prepare(handle);
            } else if (rc < 0) {
                fprintf(stderr, "error from writei: %s\n", snd_strerror(rc));
                break;  /* skip one period */
            }
            else if (rc == 0 || rc == -EAGAIN)
			{
                usleep(1000);
				continue;
			}
        }
        ilen += frame_info.frame_bytes;
        if (samples_to_get <= 0)
            printf("@@@@@decode frame success,sample size:%d,decode frame_bytes:%d,frame_info.frame_bytes:%d,free size:%d\n",\
            samples_to_get,ilen,frame_info.frame_bytes,MP3_BUF_SIZE-ilen);

        if (MP3_BUF_SIZE-ilen < 1152)
        {
            if (samples_to_get <= 0)
                printf("mp3dec_decode_frame failed,again:%d\n",samples_to_get);
            memmove(sBuf, sBuf + ilen, MP3_BUF_SIZE - ilen);
            if (fread(sBuf+ MP3_BUF_SIZE - ilen,1,ilen,f) < ilen)
            {
                //printf("fread failed\n");
                break;
            }
            ilen = 0;
            samples_to_get = mp3dec_decode_frame(&mp3d, sBuf, sizeof(sBuf), sample_data, &frame_info);
        }
        else
        {
            samples_to_get = mp3dec_decode_frame(&mp3d, sBuf + ilen, MP3_BUF_SIZE-ilen, sample_data, &frame_info);
        }
    }

OUT:
    fclose(f);
	snd_pcm_drain(handle);
    snd_pcm_close(handle);
    free (fixed_data);
    cache_deinit();

	return 0;
}