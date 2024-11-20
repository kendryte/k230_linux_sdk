#include <stdio.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>

#define PCM_DEVICE "default"
#define WAV_HEADER_SIZE 44

static void write_wav_header(FILE *file, int sample_rate, int num_channels, int bits_per_sample, int num_samples) {
    int byte_rate = sample_rate * num_channels * bits_per_sample / 8;
    int block_align = num_channels * bits_per_sample / 8;
    int data_size = num_samples * num_channels * bits_per_sample / 8;

    fwrite("RIFF", 1, 4, file);
    int chunk_size = 36 + data_size;
    fwrite(&chunk_size, 4, 1, file);
    fwrite("WAVE", 1, 4, file);
    fwrite("fmt ", 1, 4, file);
    int subchunk1_size = 16;
    fwrite(&subchunk1_size, 4, 1, file);
    short audio_format = 1;
    fwrite(&audio_format, 2, 1, file);
    fwrite(&num_channels, 2, 1, file);
    fwrite(&sample_rate, 4, 1, file);
    fwrite(&byte_rate, 4, 1, file);
    fwrite(&block_align, 2, 1, file);
    fwrite(&bits_per_sample, 2, 1, file);
    fwrite("data", 1, 4, file);
    fwrite(&data_size, 4, 1, file);
}
int alsa_record_wav(const char* filename, unsigned int duration, unsigned int sample_rate, int num_channels) {
    int rc;
    int size;
    snd_pcm_t *pcm_handle;
    snd_pcm_hw_params_t *params;
    int bits_per_sample = 16;
    int dir = 0;
    snd_pcm_uframes_t frames;
    char *buffer;
    FILE *file;

    // Open PCM device for recording (capture)
    rc = snd_pcm_open(&pcm_handle, PCM_DEVICE, SND_PCM_STREAM_CAPTURE, 0);
    if (rc < 0) {
        fprintf(stderr, "unable to open pcm device: %s\n", snd_strerror(rc));
        return -1;
    }

    // Allocate a hardware parameters object
    snd_pcm_hw_params_alloca(&params);

    // Fill it in with default values
    snd_pcm_hw_params_any(pcm_handle, params);

    // Set the desired hardware parameters
    snd_pcm_hw_params_set_access(pcm_handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
    snd_pcm_hw_params_set_format(pcm_handle, params, SND_PCM_FORMAT_S16_LE);
    snd_pcm_hw_params_set_channels(pcm_handle, params, num_channels);
    snd_pcm_hw_params_set_rate_near(pcm_handle, params, &sample_rate, &dir);

    // Write the parameters to the driver
    rc = snd_pcm_hw_params(pcm_handle, params);
    if (rc < 0) {
        fprintf(stderr, "unable to set hw parameters: %s\n", snd_strerror(rc));
        snd_pcm_close(pcm_handle);
        return -1;
    }

    // Use a buffer large enough to hold one period
    snd_pcm_hw_params_get_period_size(params, &frames, &dir);
    size = frames * num_channels * bits_per_sample / 8; // 2 bytes/sample, 2 channels
    buffer = (char *) malloc(size);

    // Open the output file
    file = fopen(filename, "wb");
    if (!file) {
        fprintf(stderr, "unable to open output file\n");
        free(buffer);
        snd_pcm_close(pcm_handle);
        return -1;
    }

    // Write the WAV header
    write_wav_header(file, sample_rate, num_channels, bits_per_sample, 0);

    // Capture audio data
    int num_samples = 0;
    unsigned int total_frames = duration * sample_rate;
    while (num_samples < total_frames) {
        rc = snd_pcm_readi(pcm_handle, buffer, frames);
        if (rc == -EPIPE) {
            // EPIPE means overrun
            fprintf(stderr, "overrun occurred\n");
            snd_pcm_prepare(pcm_handle);
        } else if (rc < 0) {
            fprintf(stderr, "error from read: %s\n", snd_strerror(rc));
        } else if (rc != (int)frames) {
            fprintf(stderr, "short read, read %d frames\n", rc);
        }

        fwrite(buffer, size, 1, file);
        num_samples += frames;
    }

    // Update the WAV header with the correct data size
    fseek(file, 0, SEEK_SET);
    write_wav_header(file, sample_rate, num_channels, bits_per_sample, num_samples);

    // Clean up
    fclose(file);
    free(buffer);
    snd_pcm_drain(pcm_handle);
    snd_pcm_close(pcm_handle);

    return 0;
}

int test_rec()
{
    int rc;
    int size;
    snd_pcm_t *pcm_handle;
    snd_pcm_hw_params_t *params;
    unsigned int sample_rate = 44100;
    int num_channels = 2;
    int bits_per_sample = 16;
    int dir;
    snd_pcm_uframes_t frames;
    char *buffer;
    FILE *file;

    // Open PCM device for recording (capture)
    rc = snd_pcm_open(&pcm_handle, PCM_DEVICE, SND_PCM_STREAM_CAPTURE, 0);
    if (rc < 0) {
        fprintf(stderr, "unable to open pcm device: %s\n", snd_strerror(rc));
        exit(1);
    }

    // Allocate a hardware parameters object
    snd_pcm_hw_params_alloca(&params);

    // Fill it in with default values
    snd_pcm_hw_params_any(pcm_handle, params);

    // Set the desired hardware parameters
    snd_pcm_hw_params_set_access(pcm_handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
    snd_pcm_hw_params_set_format(pcm_handle, params, SND_PCM_FORMAT_S16_LE);
    snd_pcm_hw_params_set_channels(pcm_handle, params, num_channels);
    snd_pcm_hw_params_set_rate_near(pcm_handle, params, &sample_rate, &dir);

    // Write the parameters to the driver
    rc = snd_pcm_hw_params(pcm_handle, params);
    if (rc < 0) {
        fprintf(stderr, "unable to set hw parameters: %s\n", snd_strerror(rc));
        exit(1);
    }

    // Use a buffer large enough to hold one period
    snd_pcm_hw_params_get_period_size(params, &frames, &dir);
    size = frames * num_channels * bits_per_sample / 8; // 2 bytes/sample, 2 channels
    buffer = (char *) malloc(size);

    // Open the output file
    file = fopen("output.wav", "wb");
    if (!file) {
        fprintf(stderr, "unable to open output file\n");
        exit(1);
    }

    // Write the WAV header
    write_wav_header(file, sample_rate, num_channels, bits_per_sample, 0);

    // Capture audio data
    int num_samples = 0;
    while (1) {
        rc = snd_pcm_readi(pcm_handle, buffer, frames);
        if (rc == -EPIPE) {
            // EPIPE means overrun
            fprintf(stderr, "overrun occurred\n");
            snd_pcm_prepare(pcm_handle);
        } else if (rc < 0) {
            fprintf(stderr, "error from read: %s\n", snd_strerror(rc));
        } else if (rc != (int)frames) {
            fprintf(stderr, "short read, read %d frames\n", rc);
        }

        fwrite(buffer, size, 1, file);
        num_samples += frames;

        // For demonstration, we will capture only a few seconds of audio
        if (num_samples >= sample_rate * 5) {
            break;
        }
    }

    // Update the WAV header with the correct data size
    fseek(file, 0, SEEK_SET);
    write_wav_header(file, sample_rate, num_channels, bits_per_sample, num_samples);

    // Clean up
    fclose(file);
    free(buffer);
    snd_pcm_drain(pcm_handle);
    snd_pcm_close(pcm_handle);

    return 0;
}
