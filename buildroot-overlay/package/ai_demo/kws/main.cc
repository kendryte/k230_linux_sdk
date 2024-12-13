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


#include <iostream>
#include <thread>
#include <unistd.h>
#include "mmz.h"
#include "kws.h"
#include "play_wav.h"
#include "get_pcm.h"

using std::cerr;
using std::cout;
using std::endl;

std::atomic<bool> audio_stop(false);
void print_usage(const char *name)
{
    cout << "Usage: " << name << "<kmodel_kws> <num_keyword> <task_name> <debug_mode>" << endl
         << "Options:" << endl
         << "  kmodel_kws       语音唤醒 kmodel路径\n"
         << "  task_name        任务名称\n"
         << "  num_keyword      关键词数量\n"
         << "  spot_thresh      唤醒词激活阈值\n"
         << "  debug_mode       是否需要调试, 0、1、2分别表示不调试、简单调试、详细调试\n"
         << "\n"
         << endl;
}

void video_proc(char *argv[])
{
    // 唤醒后的回复音频
    // 需要提前准备好音频并存放在reply_wav文件夹下
    std::vector<char *>reply_wav_list = {
        "./reply_wav/jiaowoganma.wav",
        "./reply_wav/wozai.wav",
        "./reply_wav/zaide.wav",
        "./reply_wav/laile.wav",
        "./reply_wav/wolaile.wav",
    };
    unsigned int sample_rate=16000;
    int num_channels=0;
    char* pcm_buffer;
    initPcm(sample_rate,num_channels);
    // 初始化KWS模型
    KWS kws(argv[1], argv[2], atoi(argv[3]), atof(argv[4]), atoi(argv[5]));

    // 声明用于储存检测结果的变量
    std::string results;
    std::vector<float> wav;
    while (!audio_stop)
    {
        ScopedTiming st("total time", 1);
        wav.clear();
        getPcm(wav);

        // 将音频流PCM数据处理为音频Fbank特征
        bool ok = kws.pre_process(wav);

        // 模型推理
        kws.inference();

        // 获得检测结果
        std::string results = kws.post_process();

        if (results == "Deactivated!"){
            std::cout << "Deactivated!" << std::endl;
        }
        else{
            std::cout << results << std::endl;
            std::srand(static_cast<unsigned int>(std::time(nullptr)));
            std::default_random_engine randomEngine(std::rand());
            std::uniform_int_distribution<int> distribution(0, reply_wav_list.size() - 1);
            int index = distribution(randomEngine);
            char * reply_wav_file = reply_wav_list[index];
            WAVINFO waveinfo;
            analyseWaveFile(reply_wav_file,&waveinfo);
            playerPcm(reply_wav_file,&waveinfo);
        }
    }

    deinitPcm();
}

void __attribute__((destructor)) cleanup() {
    std::cout << "Cleaning up memory..." << std::endl;
    shrink_memory_pool();
    kd_mpi_mmz_deinit();
}

int main(int argc, char *argv[])
{
    std::cout << "case " << argv[0] << " built at " << __DATE__ << " " << __TIME__ << std::endl;
    if (argc != 6)
    {
        print_usage(argv[0]);
        return -1;
    }

    else
    {
        std::thread thread_isp(video_proc, argv);
        while (getchar() != 'q')
        {
            usleep(10000);
        }
        audio_stop = true;
        thread_isp.join();
    }
    return 0;
}
