/**
 * H3OS — Compositing window manager (software)
 */
#include "wm.h"
#include "../drivers/framebuffer/framebuffer.h"
#include <h3os/adaptive.h>
#include <h3os/kernel.h>
#include <h3os/string.h>
#include "../memory/heap.h"

/* Brand palette — deep ocean teal, not purple/cream clichés */
#define COL_TITLE_BG   0xFF0B1F2A
#define COL_TITLE_FG   0xFFE8F4F8
#define COL_BORDER     0xFF1A4A5C
#define COL_SHADOW     0x66000000
#define COL_CLOSE      0xFFE85D4C
#define COL_MAX        0xFF3DDC97
#define COL_MIN        0xFFF0C75E

static window_t windows[WM_MAX_WINDOWS];
static window_t* focused = NULL;
static u32 win_count = 0;

void wm_init(void) {
    memset(windows, 0, sizeof(windows));
    focused = NULL;
    win_count = 0;
    KLOG_INFO("wm", "Window manager online");
}

window_t* wm_create(const char* title, i32 x, i32 y, i32 w, i32 h) {
    const perf_settings_t* p = adaptive_settings();
    if (win_count >= p->max_windows) return NULL;

    for (int i = 0; i < WM_MAX_WINDOWS; i++) {
        if (!windows[i].active) {
            window_t* win = &windows[i];
            memset(win, 0, sizeof(*win));
            win->active = true;
            strncpy(win->title, title ? title : "Untitled", WM_TITLE_LEN - 1);
            win->x = x; win->y = y; win->w = w; win->h = h;
            win->restore_x = x; win->restore_y = y;
            win->restore_w = w; win->restore_h = h;
            win->bg = 0xFF0E2430;
            win->accent = 0xFF2EC4B6;
            win->state = WIN_NORMAL;
            win_count++;
            wm_focus(win);
            return win;
        }
    }
    return NULL;
}

void wm_destroy(window_t* win) {
    if (!win || !win->active) return;
    win->active = false;
    if (focused == win) focused = NULL;
    win_count--;
}

void wm_focus(window_t* win) {
    for (int i = 0; i < WM_MAX_WINDOWS; i++)
        windows[i].focused = false;
    if (win) {
        win->focused = true;
        focused = win;
    }
}

void wm_move(window_t* win, i32 x, i32 y) {
    if (!win) return;
    win->x = x; win->y = y;
}

void wm_resize(window_t* win, i32 w, i32 h) {
    if (!win) return;
    if (w < 120) w = 120;
    if (h < 80) h = 80;
    win->w = w; win->h = h;
}

void wm_set_state(window_t* win, win_state_t state) {
    if (!win) return;
    fb_t* fb = fb_get();
    if (state == WIN_MAXIMIZED && win->state != WIN_MAXIMIZED) {
        win->restore_x = win->x; win->restore_y = win->y;
        win->restore_w = win->w; win->restore_h = win->h;
        win->x = 0; win->y = 40;
        win->w = (i32)fb->width;
        win->h = (i32)fb->height - 40 - 56;
    } else if (state == WIN_NORMAL && win->state == WIN_MAXIMIZED) {
        win->x = win->restore_x; win->y = win->restore_y;
        win->w = win->restore_w; win->h = win->restore_h;
    }
    win->state = state;
}

static void draw_window_chrome(window_t* win) {
    const perf_settings_t* p = adaptive_settings();
    i32 x = win->x, y = win->y, w = win->w, h = win->h;

    if (p->shadows) {
        fb_fill_rect(x + 4, y + 4, w, h, COL_SHADOW);
    }

    fb_fill_rounded_rect(x, y, w, h, 8, win->bg);
    fb_fill_rect(x, y, w, 28, COL_TITLE_BG);
    fb_draw_string(x + 12, y + 10, win->title, COL_TITLE_FG, 0xFFFFFFFF);

    /* Traffic lights — H3OS style (right side, teal accent circle + controls) */
    fb_fill_rect(x + w - 54, y + 8, 12, 12, COL_MIN);
    fb_fill_rect(x + w - 38, y + 8, 12, 12, COL_MAX);
    fb_fill_rect(x + w - 22, y + 8, 12, 12, COL_CLOSE);

    if (win->focused) {
        fb_draw_rect(x, y, w, h, win->accent);
    } else {
        fb_draw_rect(x, y, w, h, COL_BORDER);
    }

    if (win->draw) win->draw(win);
}

void wm_render(void) {
    /* Non-focused first, focused last */
    for (int i = 0; i < WM_MAX_WINDOWS; i++) {
        if (windows[i].active && !windows[i].focused &&
            windows[i].state != WIN_MINIMIZED)
            draw_window_chrome(&windows[i]);
    }
    if (focused && focused->active && focused->state != WIN_MINIMIZED)
        draw_window_chrome(focused);
}

void wm_handle_key(char c) {
    if (focused && focused->on_key) focused->on_key(focused, c);
}

window_t* wm_focused(void) { return focused; }
u32 wm_count(void) { return win_count; }
