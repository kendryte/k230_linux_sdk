#ifndef __AUDIO_ENHANCE_H__
#define __AUDIO_ENHANCE_H__
#ifdef __cplusplus
extern "C" {
#endif

typedef void* AUDIO3A_AEC_HANDLE;


typedef struct {
    void * echo_state;
    void * preprocess_state;
    int  sample_rate;// Sample rate of the audio in Hz
    int  frame_size;//Number of samples to process at one time (should correspond to 10-20 ms)
    int  filter_length;//Number of samples of echo to cancel (should generally correspond to 100-500 ms)
} AUDIO3A_AEC_CONFIG;


/**
 * @brief Initializes the AEC module with the given configuration.
 *
 * @param config Pointer to the AEC configuration structure.
 * @return int Status code (0 for success, non-zero for failure).
 */
int audio3a_aec_init(AUDIO3A_AEC_CONFIG* config);

/**
 * @brief Processes the audio signal for Acoustic Echo Cancellation (AEC).
 *
 * This function takes in microphone and speaker buffers, processes them
 * according to the provided AEC configuration, and outputs the processed
 * audio signal.
 *
 * @param config Pointer to the AEC configuration structure.
 * @param mic_buf Pointer to the buffer containing the microphone input signal.
 * @param spk_buf Pointer to the buffer containing the speaker input signal.
 * @param out_buf Pointer to the buffer where the processed output signal will be stored.
 *
 * @return An integer indicating the success or failure of the processing.
 */
int audio3a_aec_process(AUDIO3A_AEC_CONFIG* config,short *mic_buf, short *spk_buf, short *out_buf);

/**
 * @brief Destroys the AEC module and releases any allocated resources.
 *
 * @param config Pointer to the AEC configuration structure.
 * @return int Status code (0 for success, non-zero for failure).
 */
int audio3a_aec_destroy(AUDIO3A_AEC_CONFIG* config);

#ifdef __cplusplus
}
#endif

#endif // __AUDIO3A_AEC_H__