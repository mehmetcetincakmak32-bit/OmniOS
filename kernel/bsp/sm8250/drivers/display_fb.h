/* OmniOS — kernel/bsp/sm8250/drivers/display_fb.h */
/* Framebuffer display interface */
/* SPDX-License-Identifier: MIT */

#ifndef OMNIOS_DISPLAY_FB_H
#define OMNIOS_DISPLAY_FB_H

#include <stdint.h>

int  display_fb_init(void);
int  display_fb_get_info(uint32_t *width, uint32_t *height, uint32_t *bpp, uintptr_t *addr);
void display_fb_flush(void);
void display_fb_set_pixel(uint32_t x, uint32_t y, uint32_t color);
void display_fb_fill_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color);

#endif
