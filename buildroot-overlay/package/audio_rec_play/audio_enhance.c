#include "audio_enhance.h"
#include <stdlib.h>
#include "speex/speex_echo.h"
#include "speex/speex_preprocess.h"

int audio3a_aec_init(AUDIO3A_AEC_CONFIG* config)
{
    SpeexEchoState *st;
    SpeexPreprocessState *den;

    st = speex_echo_state_init(config->frame_size, config->filter_length);
    den = speex_preprocess_state_init(config->frame_size, config->sample_rate);
    speex_echo_ctl(st, SPEEX_ECHO_SET_SAMPLING_RATE, &config->sample_rate);
    speex_preprocess_ctl(den, SPEEX_PREPROCESS_SET_ECHO_STATE, st);

    // 启用自动增益控制（AGC）
    int agc = 1;
    speex_preprocess_ctl(den, SPEEX_PREPROCESS_SET_AGC, &agc);

    // 启用噪声抑制（ANS）
    int noiseSuppress = 1;
    speex_preprocess_ctl(den, SPEEX_PREPROCESS_SET_DENOISE, &noiseSuppress);

    config->echo_state = st;
    config->preprocess_state = den;
    return 0;
}

int audio3a_aec_process(AUDIO3A_AEC_CONFIG* config, short *mic_buf, short *spk_buf, short *out_buf)
{
    speex_echo_cancellation((SpeexEchoState*)config->echo_state, mic_buf, spk_buf, out_buf);
    speex_preprocess_run((SpeexPreprocessState*)config->preprocess_state, out_buf);
    return 0;
}

int audio3a_aec_destroy(AUDIO3A_AEC_CONFIG* config)
{
    if (config->echo_state) {
        speex_echo_state_destroy((SpeexEchoState*)config->echo_state);
        config->echo_state = NULL;
    }
    if (config->preprocess_state) {
        speex_preprocess_state_destroy((SpeexPreprocessState*)config->preprocess_state);
        config->preprocess_state = NULL;
    }
    return 0;
}