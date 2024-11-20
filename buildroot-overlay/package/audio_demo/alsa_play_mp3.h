/**
 * @file alsa_play_mp3.h
 * @brief Header file for ALSA MP3 player functions.
 *
 * This file contains the declarations of functions related to playing MP3 files using ALSA.
 */

#ifndef __ALSA_PLAY_MP3_H__
#define __ALSA_PLAY_MP3_H__

/**
 * @struct k_mp3_info_t
 * @brief Structure to hold information about an MP3 file.
 *
 * This structure contains information such as frame size, number of channels, sample rate, layer, and bitrate of an MP3 file.
 */
typedef struct
{
    int frame_bytes;    /**< Size of each frame in bytes */
    int channels;       /**< Number of audio channels */
    int rate_sample;    /**< Sample rate in Hz */
    int layer;          /**< MPEG audio layer */
    int bitrate_kbps;   /**< Bitrate in kilobits per second */
} k_mp3_info_t;

/**
 * @brief Analyzes an MP3 file and retrieves its information.
 *
 * This function analyzes the specified MP3 file and populates the provided k_mp3_info_t structure with information about the file.
 *
 * @param filename The path to the MP3 file.
 * @param mp3info Pointer to a k_mp3_info_t structure to store the retrieved information.
 * @return Returns 0 on success, or a negative value on failure.
 */
int k_analyse_mp3_file(char* filename, k_mp3_info_t* mp3info);

/**
 * @brief play an MP3 file using the provided information.
 *
 * This function play the specified MP3 file using the provided k_mp3_info_t structure, which contains information about the file.
 *
 * @param filename The path to the MP3 file.
 * @param mp3info Pointer to a k_mp3_info_t structure containing information about the file.
 * @return Returns 0 on success, or a negative value on failure.
 */
int k_play_mp3(char* filename, const k_mp3_info_t* mp3info);

#endif // __ALSA_PLAY_MP3_H__