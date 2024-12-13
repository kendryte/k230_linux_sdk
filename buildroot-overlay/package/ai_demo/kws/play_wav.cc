#include <stdio.h>
#include <stdlib.h>
#include "alsa/asoundlib.h"
#include "alsa/pcm.h"
#include "play_wav.h"

static int g_read_size = 0;

static int getWaveData(FILE* fp,char* filename,WAVINFO* waveinfo,char* buffer,int* frames)
{
	int ret = -1;
	int rv;
	int frameBytes = 0;
	int blocknum;
	int frameOnly;

	if(buffer == NULL){
		printf("error %d: %s open failed.\n",__LINE__,filename);
		return -1;
	}

	frameBytes = waveinfo->channels * waveinfo->bitsPerSample / 8;

	if(frameBytes * (*frames) <= waveinfo->dataSize - g_read_size)
	{
		blocknum = fread(buffer,frameBytes,*frames,fp);
		if(blocknum != (*frames)){
			printf("error %d: %s read failed.\n",__LINE__,filename);
			return -1;
		}
		g_read_size += frameBytes * (*frames);
	}
	else{
		return -1;
	}

	return 0;
}

int playerPcm(char* filename,WAVINFO* waveinfo)
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
    snd_pcm_hw_params_set_channels(handle, params, waveinfo->channels);

    /* 44100 bits/second sampling rate (CD quality) */
    val = waveinfo->sampleRate;
    snd_pcm_hw_params_set_rate_near(handle, params, &val, &dir);
	//printf("pcm rate: val:%d dir:%d.\n",val,dir);

    /* set period size t 40ms frames */
	frames = waveinfo->sampleRate/25;
    rc = snd_pcm_hw_params_set_period_size_near(handle, params, &frames, &dir);
	printf("%d rc = %d,pcm frames: frames:%ld dir:%d.\n",__LINE__,rc,frames,dir);

    /* write params to the driver */
    rc = snd_pcm_hw_params(handle, params);
    if (rc < 0) {
        printf("unable to set hw params: %s\n",snd_strerror(rc));
        goto OUT;
    }

    /* use buffer large enough to hold one period */
    rc = snd_pcm_hw_params_get_period_size(params, &frames, &dir);
	printf("%d,rc = %d,frames:%ld dir:%d.\n",__LINE__,rc,frames,dir);

    size = frames * 4; //2 2 bytes/sample, 2 channels
    buffer = (char *)malloc(size);


	fp = fopen(filename,"r");
	if(fp==NULL){
		printf("error %d: %s open failed.\n",__LINE__,filename);
		return ret;
	}

	//skip wav header
	rc = fseek(fp,waveinfo->dataFoft,SEEK_SET);
	if(rc != 0){
		printf("error %d: fseek %d failed.\n",__LINE__,waveinfo->dataFoft);
		fclose(fp);
		return -1;
	}

	for(;;)
	{
		forGetFrames = frames;

		memset(buffer,0,sizeof(buffer));
		rc = getWaveData(fp,filename,waveinfo,buffer,&forGetFrames);
		if(rc != 0){
			//printf("error %d: %s getWaveData failed.\n",__LINE__,filename);
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


		if(forGetFrames != (int)frames){
			printf("%s play over.\n",filename);
			break;
		}
	}
    g_read_size=0;
	ret = 0;
OUT:
	snd_pcm_drain(handle);
    snd_pcm_close(handle);
    free(buffer);
	fclose(fp);

	return ret;
}

int analyseWaveFile(char* filename,WAVINFO* waveinfo)
{
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        fprintf(stderr, "Failed to open file: %s\n", filename);
        return -1;
    }

    char chunk_id[5];
    unsigned int chunk_size;
    char format[5];

    // 读取RIFF chunk
    fread(chunk_id, 1, 4, fp);
    chunk_id[4] = '\0';
    fread(&chunk_size, 4, 1, fp);
    fread(format, 1, 4, fp);
    format[4] = '\0';

    // 读取fmt chunk
    fread(chunk_id, 1, 4, fp);
    fread(&chunk_size, 4, 1, fp);
    unsigned short audio_format;
    unsigned short num_channels;
    unsigned int sample_rate;
    unsigned int byte_rate;
    unsigned short block_align;
    unsigned short bits_per_sample;
    fread(&audio_format, 2, 1, fp);
    fread(&num_channels, 2, 1, fp);
    fread(&sample_rate, 4, 1, fp);
    fread(&byte_rate, 4, 1, fp);
    fread(&block_align, 2, 1, fp);
    fread(&bits_per_sample, 2, 1, fp);

	waveinfo->channels = num_channels;
	waveinfo->sampleRate = sample_rate;
	waveinfo->bitsPerSample =  bits_per_sample;

    // 读取附加信息
    if (chunk_size > 16) {
        fseek(fp, chunk_size - 16, SEEK_CUR);
    }

    // 读取data chunk
    while(1)
    {
        fread(chunk_id, 1, 4, fp);
        if (strcmp(chunk_id,"data") !=0)
        {
            fseek(fp, -2L, SEEK_CUR);
            continue;
        }
        fread(&chunk_size, 4, 1, fp);
       // printf("chunk_id:%s,chunk_size:%d\n",chunk_id,chunk_size);
        break;
    }

	waveinfo->dataFoft = ftell(fp);
	waveinfo->dataBeginFoft = waveinfo->dataFoft;
	waveinfo->dataSize = chunk_size;
	printf("waveinfo->dataSize:%d,dataFoft:%d,dataBeginFoft:%d\n",waveinfo->dataSize,waveinfo->dataFoft,waveinfo->dataBeginFoft);

    fclose(fp);
    return 0;
}