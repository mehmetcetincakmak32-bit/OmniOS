/* OmniOS — kernel/bsp/sm8250/drivers/audio.h */
/* Audio subsystem interface */
/* SPDX-License-Identifier: MIT */

#ifndef OMNIOS_AUDIO_H
#define OMNIOS_AUDIO_H

#include <stdint.h>

int  audio_init(void);
int  audio_play(const int16_t *pcm, uint32_t frames);
int  audio_record(int16_t *pcm, uint32_t max_frames);
void audio_set_volume(int percent);
int  audio_get_volume(void);
void audio_set_mic_mute(int mute);
int  audio_is_mic_muted(void);
void audio_play_tone(uint32_t freq_hz, uint32_t duration_ms);

#endif
