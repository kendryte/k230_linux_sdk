#ifndef __ALSA_PLAY_WAV_H__
#define __ALSA_PLAY_WAV_H__

#include<stdio.h>
#include<string.h>

typedef unsigned int DWORD;
typedef unsigned short WORD;

typedef struct __RIFF_HEADER
{
	char szRiffID[4];  // 'R','I','F','F'
	DWORD dwRiffSize;
	char szRiffFormat[4]; // 'W','A','V','E'
}RIFF_HEADER;

typedef struct __WAVE_FORMAT
{
	WORD wFormatTag;		//一般为0x0001
	WORD wChannels;			//通道数
	DWORD dwSamplesPerSec;	//采样频率
	DWORD dwAvgBytesPerSec;	//每秒需要的字节数
	WORD wBlockAlign;		//数据块对齐单位，每个采样点需要的字节数
	WORD wBitsPerSample;	//每个采样点需要的bit数
}WAVE_FORMAT;

typedef struct __FMT_BLOCK
{
	char szFmtID[4]; // 'f','m','t',' '
	DWORD dwFmtSize;
	WAVE_FORMAT wavFormat;
}FMT_BLOCK;


typedef struct __FACT_BLOCK
{
	char szFactID[4]; // 'f','a','c','t'
	DWORD dwFactSize;
}FACT_BLOCK;

typedef struct DATA_BLOCK
{
	char szDataID[4]; // 'd','a','t','a'
	DWORD dwDataSize;
}DATA_BLOCK;

typedef union __FACT_DATA_BLOCK{
	FACT_BLOCK fact_block;
	DATA_BLOCK data_block;
}FACT_DATA_BLOCK;

typedef struct wavinfo{
	int channels;
	int sampleRate;
	int bitsPerSample;
	int dataSize;
	int dataFoft;
	int dataBeginFoft;
}WAVINFO;

int playerPcm(int channels,unsigned int &sample_rate,char *mydata,unsigned int &data_len);

#endif