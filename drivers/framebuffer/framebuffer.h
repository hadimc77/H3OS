/**
 * H3OS — Linear framebuffer interface
 */
#ifndef H3OS_FRAMEBUFFER_H
#define H3OS_FRAMEBUFFER_H

#include <h3os/types.h>
#include "../../boot/multiboot2.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    u32* pixels;
    u32  width;
    u32  height;
    u32  pitch;   /* bytes per row */
    u8   bpp;
    bool double_buffered;
    u32* back;    /* software back buffer (optional) */
} fb_t;

void  fb_init(mb2_info_t* mb2);
fb_t* fb_get(void);
void  fb_clear(u32 color);
void  fb_put_pixel(i32 x, i32 y, u32 color);
u32   fb_get_pixel(i32 x, i32 y);
void  fb_fill_rect(i32 x, i32 y, i32 w, i32 h, u32 color);
void  fb_draw_rect(i32 x, i32 y, i32 w, i32 h, u32 color);
void  fb_draw_line(i32 x0, i32 y0, i32 x1, i32 y1, u32 color);
void  fb_draw_rounded_rect(i32 x, i32 y, i32 w, i32 h, i32 r, u32 color);
void  fb_fill_rounded_rect(i32 x, i32 y, i32 w, i32 h, i32 r, u32 color);
void  fb_blit(i32 x, i32 y, i32 w, i32 h, const u32* src, i32 src_stride);
void  fb_swap(void); /* present back buffer if enabled */

/* 8x16 bitmap font */
void  fb_draw_char(i32 x, i32 y, char c, u32 fg, u32 bg);
void  fb_draw_string(i32 x, i32 y, const char* s, u32 fg, u32 bg);

#ifdef __cplusplus
}
#endif

#endif /* H3OS_FRAMEBUFFER_H */
