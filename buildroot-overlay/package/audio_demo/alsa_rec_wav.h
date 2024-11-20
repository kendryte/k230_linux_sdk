#ifndef ALSA_REC_WAV_H
#define ALSA_REC_WAV_H

/**
 * @brief Records audio and saves it as a WAV file.
 *
 * This function records audio from the default ALSA device and writes it to the specified WAV file.
 *
 * @param filename     The name of the output WAV file.
 * @param duration     Recording duration in seconds.
 * @param sample_rate  Sampling rate in Hz.
 * @param num_channels Number of audio channels, e.g., 1 for mono, 2 for stereo.
 *
 * @return Returns 0 on success, or a negative error code on failure.
 */
int alsa_record_wav(const char* filename, unsigned int duration, unsigned int sample_rate, int num_channels);

#endif // ALSA_REC_WAV_H