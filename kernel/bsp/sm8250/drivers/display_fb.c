/* OmniOS — kernel/bsp/sm8250/drivers/display_fb.c */
/* MDSS framebuffer driver — simple FB for wlroots/compositor */
/* SPDX-License-Identifier: MIT */

#include "../sm8250.h"
#include <stdio.h>
#include <string.h>

#define FB_WIDTH          1080
#define FB_HEIGHT         2340
#define FB_BPP            32
#define FB_STRIDE         (FB_WIDTH * (FB_BPP / 8))
#define FB_SIZE           (FB_STRIDE * FB_HEIGHT)
#define FB_ADDR           0x9D000000  /* Framebuffer physical address */

static int fb_ready = 0;

int display_fb_init(void) {
    printf("[DISPLAY_FB] Framebuffer: %dx%d %dbpp\n", FB_WIDTH, FB_HEIGHT, FB_BPP);

    /* Configure MDSS for framebuffer mode */
    write_reg(SM8250_DISPLAY_BASE, MDSS_MDP_CTRL, 1);
    write_reg(SM8250_DISPLAY_BASE, MDSS_INTF_CTRL, (FB_WIDTH << 16) | FB_HEIGHT);
    write_reg(SM8250_DISPLAY_BASE, MDSS_DSI0_CTRL, 1);

    /* Set framebuffer address in MDSS */
    write_reg(SM8250_DISPLAY_BASE, 0x2000, FB_ADDR);
    write_reg(SM8250_DISPLAY_BASE, 0x2004, FB_STRIDE);
    write_reg(SM8250_DISPLAY_BASE, 0x2008, (FB_BPP / 8));

    /* Clear framebuffer */
    memset((void *)FB_ADDR, 0, FB_SIZE);

    fb_ready = 1;
    printf("[DISPLAY_FB] FB at 0x%x, size %u\n", FB_ADDR, FB_SIZE);
    return 0;
}

int display_fb_get_info(uint32_t *width, uint32_t *height, uint32_t *bpp, uintptr_t *addr) {
    if (!fb_ready) return -1;
    *width  = FB_WIDTH;
    *height = FB_HEIGHT;
    *bpp    = FB_BPP;
    *addr   = FB_ADDR;
    return 0;
}

void display_fb_flush(void) {
    /* Trigger display update */
    write_reg(SM8250_DISPLAY_BASE, 0x2010, 1);
}

void display_fb_set_pixel(uint32_t x, uint32_t y, uint32_t color) {
    if (x >= FB_WIDTH || y >= FB_HEIGHT) return;
    *(volatile uint32_t *)(FB_ADDR + y * FB_STRIDE + x * 4) = color;
}

void display_fb_fill_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color) {
    for (uint32_t row = y; row < y + h && row < FB_HEIGHT; row++) {
        for (uint32_t col = x; col < x + w && col < FB_WIDTH; col++) {
            display_fb_set_pixel(col, row, color);
        }
    }
}
