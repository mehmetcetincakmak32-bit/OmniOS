/* OmniOS — kernel/bsp/sm8250/drivers/audio.c */
/* Audio subsystem — ADSP + WCD938x codec (I2S) */
/* SPDX-License-Identifier: MIT */

#include "../sm8250.h"
#include <stdio.h>
#include <string.h>

#define ADSP_BASE         SM8250_ADSP_BASE
#define ADSP_CTRL         0x00
#define ADSP_STATUS       0x04
#define ADSP_I2S_TX       0x100
#define ADSP_I2S_RX       0x200
#define ADSP_DMA_BASE     0x1000

#define I2S_SR_48KHZ      48000
#define I2S_BITS_16       16
#define I2S_CHANNELS      2

static int audio_initialized = 0;
static int audio_volume = 75;
static int mic_muted = 0;

int audio_init(void) {
    printf("[AUDIO] ADSP + WCD938x init\n");

    /* Reset ADSP */
    write_reg(ADSP_BASE, ADSP_CTRL, 0x02);
    for (int i = 0; i < 10000; i++) {
        if (!(read_reg(ADSP_BASE, ADSP_STATUS) & 0x02)) break;
    }

    /* Configure I2S: 48kHz, 16-bit, stereo */
    write_reg(ADSP_BASE, ADSP_I2S_TX + 0x00, I2S_SR_48KHZ);
    write_reg(ADSP_BASE, ADSP_I2S_TX + 0x04, (I2S_BITS_16 << 16) | I2S_CHANNELS);

    /* Start ADSP */
    write_reg(ADSP_BASE, ADSP_CTRL, 0x01);

    audio_initialized = 1;
    printf("[AUDIO] Ready: 48kHz/16bit/stereo\n");
    return 0;
}

int audio_play(const int16_t *pcm, uint32_t frames) {
    if (!audio_initialized) return -1;
    uint32_t bytes = frames * I2S_CHANNELS * sizeof(int16_t);
    if (bytes > 0x1000) bytes = 0x1000;

    memcpy((void *)(ADSP_BASE + ADSP_DMA_BASE), pcm, bytes);
    write_reg(ADSP_BASE, ADSP_I2S_TX + 0x10, bytes);
    return (int)frames;
}

int audio_record(int16_t *pcm, uint32_t max_frames) {
    if (!audio_initialized) return -1;
    uint32_t avail = read_reg(ADSP_BASE, ADSP_I2S_RX + 0x10);
    if (avail == 0) return 0;
    uint32_t frames = avail / (I2S_CHANNELS * sizeof(int16_t));
    if (frames > max_frames) frames = max_frames;
    memcpy(pcm, (const void *)(ADSP_BASE + ADSP_I2S_RX), frames * I2S_CHANNELS * sizeof(int16_t));
    return (int)frames;
}

void audio_set_volume(int percent) {
    if (percent < 0) percent = 0;
    if (percent > 100) percent = 100;
    audio_volume = percent;
    write_reg(ADSP_BASE, ADSP_I2S_TX + 0x20, percent);
}

int audio_get_volume(void) { return audio_volume; }

void audio_set_mic_mute(int mute) {
    mic_muted = mute;
    write_reg(ADSP_BASE, ADSP_I2S_TX + 0x24, mute ? 1 : 0);
}

int audio_is_mic_muted(void) { return mic_muted; }

/* Tone generation for ringtone / notifications */
void audio_play_tone(uint32_t freq_hz, uint32_t duration_ms) {
    if (!audio_initialized) return;
    printf("[AUDIO] Tone: %u Hz, %u ms\n", freq_hz, duration_ms);
    write_reg(ADSP_BASE, ADSP_I2S_TX + 0x30, freq_hz);
    write_reg(ADSP_BASE, ADSP_I2S_TX + 0x34, duration_ms);
}
