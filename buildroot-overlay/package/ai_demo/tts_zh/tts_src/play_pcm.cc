#include "play_pcm.h"

#ifdef __cplusplus
extern "C"
{
#endif
#include "alsa/asoundlib.h"
#include "alsa/pcm.h"
#ifdef __cplusplus
};
#endif

int playerPcm(int channels,unsigned int &sample_rate,char *mydata,unsigned int &data_len)
{
	int rc;
    snd_pcm_t *handle;
    snd_pcm_hw_params_t *params;
    unsigned int val;
    int dir = 0;
    snd_pcm_uframes_t frames;
    int size;
    char *buffer;
	FILE* fp;
	int forGetFrames;
	int ret = -1;

    rc = snd_pcm_open(&handle, "default", SND_PCM_STREAM_PLAYBACK, 0);
    if (rc < 0) {
        printf("unable to open PCM device: %s\n",snd_strerror(rc));
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
    snd_pcm_hw_params_set_channels(handle, params, channels);

    /* 44100 bits/second sampling rate (CD quality) */
    snd_pcm_hw_params_set_rate_near(handle, params,&sample_rate, &dir);
	//printf("pcm rate: val:%d dir:%d.\n",val,dir);

    /* set period size t 40ms frames */
	frames = sample_rate/25;
    rc = snd_pcm_hw_params_set_period_size_near(handle, params, &frames, &dir);
	printf("%d rc = %d,pcm frames: frames:%ld dir:%d.\n",__LINE__,rc,frames,dir);

    /* write params to the driver */
    rc = snd_pcm_hw_params(handle, params);
    if (rc < 0) {
        printf("unable to set hw params: %s\n",snd_strerror(rc));
    }

    /* use buffer large enough to hold one period */
    rc = snd_pcm_hw_params_get_period_size(params, &frames, &dir);
	printf("%d,rc = %d,frames:%ld dir:%d.\n",__LINE__,rc,frames,dir);

    size = frames * 4; //2 2 bytes/sample, 2 channels
    buffer = (char *)malloc(size);
    size_t remainingBytes=data_len;

    int copy_size=frames*2;

	for(;;)
	{
		forGetFrames = frames;
		memset(buffer,0,sizeof(buffer));
		if(remainingBytes > 0) {
            size_t bytesToCopy = remainingBytes < copy_size? remainingBytes : copy_size;
            memcpy(buffer, mydata, bytesToCopy);
            mydata += bytesToCopy;  // 更新数据源指针位置
            remainingBytes -= bytesToCopy;  // 更新剩余字节数
        }else{
            break;
        }

		while(1)
		{
			rc = snd_pcm_writei(handle, buffer, forGetFrames);

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
			break;
		}
	}
	ret = 0;
	snd_pcm_drain(handle);
    snd_pcm_close(handle);
    free(buffer);
	return ret;
}
