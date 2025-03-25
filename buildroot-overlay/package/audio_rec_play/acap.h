#ifndef ALSA_ACAP_H
#define ALSA_ACAP_H
#define CAPTURE_PCM_FILENAME "captured_audio.pcm"
void acap_init();
void acap_deinit();
void acap_start();
void acap_stop();
void acap_play_pcm_file(const char* file_path);
#endif
