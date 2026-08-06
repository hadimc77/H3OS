/**
 * H3OS — Compositing window manager (Windows-inspired chrome)
 */
#include "wm.h"
#include "../drivers/framebuffer/framebuffer.h"
#include "../drivers/mouse/mouse.h"
#include <h3os/adaptive.h>
#include <h3os/kernel.h>
#include <h3os/string.h>

#define COL_TITLE_BG   0xFF101B24
#define COL_TITLE_FG   0xFFF3F7FA
#define COL_BORDER     0xFF2A4552
#define COL_SHADOW     0x55000000
#define COL_CLOSE      0xFFE81123
#define COL_MAX        0xFF2EC4B6
#define COL_MIN        0xFF7FA3AD
#define TASKBAR_H      48

static window_t windows[WM_MAX_WINDOWS];
static window_t* focused = NULL;
static u32 win_count = 0;

i32 wm_taskbar_height(void) { return TASKBAR_H; }

void wm_init(void) {
    memset(windows, 0, sizeof(windows));
    focused = NULL;
    win_count = 0;
    KLOG_INFO("wm", "Window manager online (Horizon shell)");
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
            win->bg = 0xFF0E1A22;
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
        if (win->state == WIN_MINIMIZED) win->state = WIN_NORMAL;
    }
}

void wm_restore_or_focus(window_t* win) {
    if (!win) return;
    if (win->focused && win->state != WIN_MINIMIZED) {
        win->state = WIN_MINIMIZED;
        focused = NULL;
        return;
    }
    wm_focus(win);
}

void wm_move(window_t* win, i32 x, i32 y) {
    if (!win) return;
    win->x = x; win->y = y;
}

void wm_resize(window_t* win, i32 w, i32 h) {
    if (!win) return;
    if (w < 160) w = 160;
    if (h < 100) h = 100;
    win->w = w; win->h = h;
}

void wm_set_state(window_t* win, win_state_t state) {
    if (!win) return;
    fb_t* fb = fb_get();
    i32 tb = TASKBAR_H;
    if (state == WIN_MAXIMIZED && win->state != WIN_MAXIMIZED) {
        win->restore_x = win->x; win->restore_y = win->y;
        win->restore_w = win->w; win->restore_h = win->h;
        win->x = 0; win->y = 0;
        win->w = (i32)fb->width;
        win->h = (i32)fb->height - tb;
    } else if (state == WIN_NORMAL && win->state == WIN_MAXIMIZED) {
        win->x = win->restore_x; win->y = win->restore_y;
        win->w = win->restore_w; win->h = win->restore_h;
    }
    win->state = state;
}

void wm_snap(window_t* win, int layout) {
    if (!win) return;
    fb_t* fb = fb_get();
    i32 W = (i32)fb->width;
    i32 H = (i32)fb->height - TASKBAR_H;
    i32 hw = W / 2;
    i32 hh = H / 2;

    if (win->state != WIN_MAXIMIZED && layout != 6) {
        win->restore_x = win->x; win->restore_y = win->y;
        win->restore_w = win->w; win->restore_h = win->h;
    }

    switch (layout) {
        case 0: win->x = 0; win->y = 0; win->w = hw; win->h = H; break;
        case 1: win->x = hw; win->y = 0; win->w = W - hw; win->h = H; break;
        case 2: win->x = 0; win->y = 0; win->w = hw; win->h = hh; break;
        case 3: win->x = hw; win->y = 0; win->w = W - hw; win->h = hh; break;
        case 4: win->x = 0; win->y = hh; win->w = hw; win->h = H - hh; break;
        case 5: win->x = hw; win->y = hh; win->w = W - hw; win->h = H - hh; break;
        case 6: wm_set_state(win, WIN_MAXIMIZED); return;
        default: break;
    }
    win->state = WIN_NORMAL;
}

static void draw_caption_btn(i32 x, i32 y, u32 fill, const char* glyph) {
    fb_fill_rect(x, y, 46, 32, fill);
    fb_draw_string(x + 18, y + 12, glyph, 0xFFFFFFFF, 0xFFFFFFFF);
}

static void draw_window_chrome(window_t* win) {
    const perf_settings_t* p = adaptive_settings();
    i32 x = win->x, y = win->y, w = win->w, h = win->h;

    if (p->shadows && win->state != WIN_MAXIMIZED) {
        fb_fill_rect(x + 6, y + 6, w, h, COL_SHADOW);
    }

    fb_fill_rect(x, y, w, h, win->bg);
    /* Windows-style title bar */
    fb_fill_rect(x, y, w, 32, win->focused ? 0xFF15232E : COL_TITLE_BG);
    fb_draw_string(x + 14, y + 12, win->title, COL_TITLE_FG, 0xFFFFFFFF);

    /* Caption buttons: min / max / close (Windows order, right side) */
    draw_caption_btn(x + w - 138, y, 0xFF1A2A35, "_");
    draw_caption_btn(x + w - 92, y, 0xFF1A2A35, "O");
    draw_caption_btn(x + w - 46, y, win->focused ? COL_CLOSE : 0xFF3A2030, "X");

    if (win->focused) {
        fb_fill_rect(x, y + 31, w, 2, win->accent);
    } else {
        fb_draw_rect(x, y, w, h, COL_BORDER);
    }

    if (win->draw) win->draw(win);
}

void wm_render(void) {
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

window_t* wm_get(u32 index) {
    u32 n = 0;
    for (int i = 0; i < WM_MAX_WINDOWS; i++) {
        if (!windows[i].active) continue;
        if (n == index) return &windows[i];
        n++;
    }
    return NULL;
}

window_t* wm_at(i32 x, i32 y) {
    if (focused && focused->active && focused->state != WIN_MINIMIZED) {
        if (x >= focused->x && x < focused->x + focused->w &&
            y >= focused->y && y < focused->y + focused->h)
            return focused;
    }
    for (int i = WM_MAX_WINDOWS - 1; i >= 0; i--) {
        window_t* w = &windows[i];
        if (!w->active || w->state == WIN_MINIMIZED) continue;
        if (x >= w->x && x < w->x + w->w && y >= w->y && y < w->y + w->h)
            return w;
    }
    return NULL;
}

static window_t* drag_win = NULL;
static i32 drag_off_x = 0, drag_off_y = 0;
static bool dragging = false;

void wm_handle_mouse(void) {
    mouse_state_t m;
    mouse_get(&m);

    if (dragging && drag_win && drag_win->active) {
        if (m.left) {
            if (drag_win->state == WIN_MAXIMIZED) {
                wm_set_state(drag_win, WIN_NORMAL);
                drag_off_x = drag_win->w / 2;
                drag_off_y = 16;
            }
            wm_move(drag_win, m.x - drag_off_x, m.y - drag_off_y);
        } else {
            /* Aero-style snap on release near edges */
            fb_t* fb = fb_get();
            if (m.x <= 12) wm_snap(drag_win, 0);
            else if (m.x >= (i32)fb->width - 12) wm_snap(drag_win, 1);
            else if (m.y <= 8) wm_snap(drag_win, 6);
            dragging = false;
            drag_win = NULL;
        }
        return;
    }

    if (!m.left_pressed) return;

    window_t* hit = wm_at(m.x, m.y);
    if (!hit) return;
    wm_focus(hit);

    i32 tx = hit->x, ty = hit->y, tw = hit->w;
    if (m.y >= ty && m.y < ty + 32) {
        if (m.x >= tx + tw - 46 && m.x < tx + tw) {
            wm_destroy(hit);
            return;
        }
        if (m.x >= tx + tw - 92 && m.x < tx + tw - 46) {
            if (hit->state == WIN_MAXIMIZED) wm_set_state(hit, WIN_NORMAL);
            else wm_set_state(hit, WIN_MAXIMIZED);
            return;
        }
        if (m.x >= tx + tw - 138 && m.x < tx + tw - 92) {
            hit->state = WIN_MINIMIZED;
            if (focused == hit) focused = NULL;
            return;
        }
        dragging = true;
        drag_win = hit;
        drag_off_x = m.x - hit->x;
        drag_off_y = m.y - hit->y;
    }
}
