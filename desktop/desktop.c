/**
 * H3OS Desktop — Windows-inspired Horizon shell
 *
 * Taskbar + Start menu + Search + Action Center + desktop icons +
 * context menu + snap layouts — branded for H3OS (teal Horizon).
 */
#include "desktop.h"
#include "../drivers/framebuffer/framebuffer.h"
#include "../drivers/mouse/mouse.h"
#include "../window_manager/wm.h"
#include "../applications/terminal/terminal.h"
#include "../applications/calculator/calculator.h"
#include "../applications/settings/settings.h"
#include "../applications/filemanager/filemanager.h"
#include "../applications/taskmanager/taskmanager.h"
#include <h3os/adaptive.h>
#include <h3os/version.h>
#include <h3os/kernel.h>
#include <h3os/string.h>
#include <h3os/power.h>
#include "../drivers/timer/timer.h"
#include "../memory/pmm.h"

#define COL_DESK_TOP   0xFF0A3D62
#define COL_DESK_BOT   0xFF061820
#define COL_TASKBAR    0xE0121A22
#define COL_START_BG   0xF0141E28
#define COL_ACCENT     0xFF2EC4B6
#define COL_TEXT       0xFFF3F7FA
#define COL_MUTED      0xFF8AA4B0
#define COL_HOVER      0xFF1C2E3A
#define COL_LIGHT_TOP  0xFF4AA3C7
#define COL_LIGHT_BOT  0xFFD6EAF2
#define COL_NOTIFY     0xF0162030

#define TASKBAR_H 48
#define START_W   420
#define START_H   520
#define ACTION_W  360

static theme_mode_t g_theme = THEME_DARK;
static u32 g_accent = COL_ACCENT;
static bool g_start = false;
static bool g_search = false;
static bool g_action = false;
static bool g_ctx = false;
static bool g_snap_ui = false;
static bool g_click_consumed = false;
static i32  g_ctx_x = 0, g_ctx_y = 0;
static u32  g_frame = 0;
static char g_search_q[48];
static int  g_search_len = 0;
static u32  g_notify_count = 2;

void desktop_set_theme(theme_mode_t mode) { g_theme = mode; }
void desktop_set_accent(u32 color) { g_accent = color; }
void desktop_toggle_launcher(void) {
    g_start = !g_start;
    if (g_start) { g_action = false; g_search = false; g_ctx = false; }
}
bool desktop_launcher_open(void) { return g_start; }
bool desktop_mouse_click_consumed(void) { return g_click_consumed; }

void desktop_launch(desktop_app_t app) {
    switch (app) {
        case APP_TERMINAL: terminal_open(); break;
        case APP_FILES:    filemanager_open(); break;
        case APP_SETTINGS: settings_open(); break;
        case APP_CALC:     calculator_open(); break;
        case APP_TASKS:    taskmanager_open(); break;
        default: break;
    }
    g_start = false;
    g_search = false;
    g_ctx = false;
}

static void format_clock(char* out) {
    u64 ms = timer_uptime_ms();
    u64 sec = (ms / 1000) % 60;
    u64 min = (ms / 60000) % 60;
    u64 hr  = (ms / 3600000) % 24;
    out[0] = (char)('0' + (hr / 10)); out[1] = (char)('0' + (hr % 10));
    out[2] = ':';
    out[3] = (char)('0' + (min / 10)); out[4] = (char)('0' + (min % 10));
    out[5] = '\0';
    H3OS_UNUSED(sec);
}

static void draw_wallpaper(fb_t* fb) {
    u32 top = (g_theme == THEME_DARK) ? COL_DESK_TOP : COL_LIGHT_TOP;
    u32 bot = (g_theme == THEME_DARK) ? COL_DESK_BOT : COL_LIGHT_BOT;
    for (u32 y = 0; y < fb->height - TASKBAR_H; y++) {
        u32 t = (y * 255) / (fb->height ? fb->height : 1);
        u32 r0 = (top >> 16) & 0xFF, g0 = (top >> 8) & 0xFF, b0 = top & 0xFF;
        u32 r1 = (bot >> 16) & 0xFF, g1 = (bot >> 8) & 0xFF, b1 = bot & 0xFF;
        u32 r = (r0 * (255 - t) + r1 * t) / 255;
        u32 g = (g0 * (255 - t) + g1 * t) / 255;
        u32 b = (b0 * (255 - t) + b1 * t) / 255;
        u32 color = 0xFF000000 | (r << 16) | (g << 8) | b;
        for (u32 x = 0; x < fb->width; x++) {
            /* soft light bloom like Windows wallpaper glow */
            if (((x + g_frame / 3) % 180) < 2 && (y % 3) == 0)
                color = (g_theme == THEME_DARK) ? 0xFF0E4A6A : 0xFFB8DCEC;
            fb_put_pixel((i32)x, (i32)y, color);
        }
    }
}

typedef struct { const char* label; desktop_app_t app; i32 x, y; } desk_icon_t;

static desk_icon_t g_icons[] = {
    {"This PC",     APP_FILES,    28,  40},
    {"Files",       APP_FILES,    28, 120},
    {"Terminal",    APP_TERMINAL, 28, 200},
    {"Settings",    APP_SETTINGS, 28, 280},
    {"Recycle Bin", APP_FILES,    28, 360},
};

static void draw_desktop_icons(void) {
    for (u32 i = 0; i < H3OS_ARRAY_SIZE(g_icons); i++) {
        i32 x = g_icons[i].x, y = g_icons[i].y;
        fb_fill_rounded_rect(x, y, 64, 52, 6, 0x55101820);
        fb_fill_rounded_rect(x + 14, y + 6, 36, 28, 4, g_accent);
        fb_draw_string(x + 4, y + 40, g_icons[i].label, COL_TEXT, 0xFFFFFFFF);
    }
}

static void draw_taskbar(fb_t* fb) {
    i32 y = (i32)fb->height - TASKBAR_H;
    fb_fill_rect(0, y, (i32)fb->width, TASKBAR_H, COL_TASKBAR);
    fb_fill_rect(0, y, (i32)fb->width, 1, 0xFF2A3A45);

    /* Start button */
    fb_fill_rounded_rect(8, y + 6, 40, 36, 6, g_start ? g_accent : 0xFF1A2832);
    fb_draw_string(16, y + 18, "H3", g_start ? 0xFF061018 : g_accent, 0xFFFFFFFF);

    /* Search pill */
    fb_fill_rounded_rect(56, y + 10, 180, 28, 8, 0xFF1A2832);
    fb_draw_string(68, y + 18, g_search ? g_search_q : "Search H3OS", COL_MUTED, 0xFFFFFFFF);

    /* Pinned apps */
    const char* pins[] = {"Term", "Files", "Set", "Calc", "Task"};
    for (int i = 0; i < 5; i++) {
        i32 px = 250 + i * 52;
        fb_fill_rounded_rect(px, y + 8, 44, 32, 6, 0xFF1A2832);
        fb_draw_string(px + 6, y + 18, pins[i], COL_TEXT, 0xFFFFFFFF);
    }

    /* Open window buttons (task list) */
    i32 tx = 520;
    u32 n = wm_count();
    for (u32 i = 0; i < n && i < 6; i++) {
        window_t* w = wm_get(i);
        if (!w) continue;
        u32 bg = w->focused ? g_accent : 0xFF1A2832;
        fb_fill_rounded_rect(tx, y + 8, 100, 32, 6, bg);
        char title[12];
        strncpy(title, w->title, 11);
        title[11] = '\0';
        fb_draw_string(tx + 8, y + 18, title, w->focused ? 0xFF061018 : COL_TEXT, 0xFFFFFFFF);
        if (w->state != WIN_MINIMIZED)
            fb_fill_rect(tx + 10, y + 36, 80, 2, g_accent);
        tx += 108;
    }

    /* System tray */
    i32 tray_x = (i32)fb->width - 210;
    fb_draw_string(tray_x, y + 18, "WiFi", COL_MUTED, 0xFFFFFFFF);
    fb_draw_string(tray_x + 40, y + 18, "Vol", COL_MUTED, 0xFFFFFFFF);

    /* Notification bell */
    fb_fill_rounded_rect(tray_x + 78, y + 10, 28, 28, 6, g_action ? g_accent : 0xFF1A2832);
    fb_draw_string(tray_x + 86, y + 18, "N", g_action ? 0xFF061018 : COL_TEXT, 0xFFFFFFFF);
    if (g_notify_count) {
        fb_fill_rect(tray_x + 96, y + 10, 8, 8, 0xFFE85D4C);
    }

    char clock[8];
    format_clock(clock);
    fb_draw_string((i32)fb->width - 70, y + 12, clock, COL_TEXT, 0xFFFFFFFF);
    fb_draw_string((i32)fb->width - 78, y + 26, "Aug 6", COL_MUTED, 0xFFFFFFFF);
}

static void draw_start_menu(fb_t* fb) {
    if (!g_start) return;
    i32 x = 8;
    i32 y = (i32)fb->height - TASKBAR_H - START_H - 8;
    fb_fill_rounded_rect(x, y, START_W, START_H, 12, COL_START_BG);
    fb_draw_rect(x, y, START_W, START_H, g_accent);

    fb_draw_string(x + 24, y + 20, "H3OS", g_accent, 0xFFFFFFFF);
    fb_draw_string(x + 80, y + 20, H3OS_TAGLINE, COL_MUTED, 0xFFFFFFFF);
    fb_draw_string(x + 24, y + 44, "Pinned", COL_MUTED, 0xFFFFFFFF);

    const char* pinned[] = {
        "Terminal", "File Manager", "Settings", "Calculator",
        "Task Manager", "Notepad", "Store", "Power"
    };
    for (int i = 0; i < 8; i++) {
        i32 ax = x + 24 + (i % 4) * 94;
        i32 ay = y + 70 + (i / 4) * 90;
        fb_fill_rounded_rect(ax, ay, 84, 76, 10, COL_HOVER);
        fb_fill_rounded_rect(ax + 22, ay + 12, 40, 32, 6, g_accent);
        fb_draw_string(ax + 8, ay + 52, pinned[i], COL_TEXT, 0xFFFFFFFF);
    }

    fb_draw_string(x + 24, y + 270, "Recommended", COL_MUTED, 0xFFFFFFFF);
    fb_fill_rounded_rect(x + 24, y + 290, 372, 40, 8, COL_HOVER);
    fb_draw_string(x + 40, y + 304, "Welcome to H3OS Horizon", COL_TEXT, 0xFFFFFFFF);
    fb_fill_rounded_rect(x + 24, y + 340, 372, 40, 8, COL_HOVER);
    fb_draw_string(x + 40, y + 354, "Explore Settings & Themes", COL_TEXT, 0xFFFFFFFF);

    /* User + power row */
    fb_fill_rect(x, y + START_H - 56, START_W, 56, 0xFF0C141C);
    fb_draw_string(x + 24, y + START_H - 34, "user", COL_TEXT, 0xFFFFFFFF);
    fb_fill_rounded_rect(x + START_W - 120, y + START_H - 42, 44, 28, 6, 0xFF1A2832);
    fb_draw_string(x + START_W - 108, y + START_H - 34, "Sleep", COL_MUTED, 0xFFFFFFFF);
    fb_fill_rounded_rect(x + START_W - 68, y + START_H - 42, 52, 28, 6, 0xFFE85D4C);
    fb_draw_string(x + START_W - 56, y + START_H - 34, "Off", COL_TEXT, 0xFFFFFFFF);
}

static void draw_action_center(fb_t* fb) {
    if (!g_action) return;
    i32 w = ACTION_W;
    i32 h = 420;
    i32 x = (i32)fb->width - w - 12;
    i32 y = (i32)fb->height - TASKBAR_H - h - 8;

    fb_fill_rounded_rect(x, y, w, h, 12, COL_NOTIFY);
    fb_draw_rect(x, y, w, h, g_accent);
    fb_draw_string(x + 20, y + 20, "Notification Center", g_accent, 0xFFFFFFFF);

    fb_fill_rounded_rect(x + 20, y + 52, w - 40, 56, 8, COL_HOVER);
    fb_draw_string(x + 36, y + 64, "Welcome to H3OS", COL_TEXT, 0xFFFFFFFF);
    fb_draw_string(x + 36, y + 80, "The Future Starts Here.", COL_MUTED, 0xFFFFFFFF);

    fb_fill_rounded_rect(x + 20, y + 118, w - 40, 56, 8, COL_HOVER);
    fb_draw_string(x + 36, y + 130, "System update ready", COL_TEXT, 0xFFFFFFFF);
    fb_draw_string(x + 36, y + 146, "Horizon desktop improvements", COL_MUTED, 0xFFFFFFFF);

    fb_draw_string(x + 20, y + 200, "Quick Settings", COL_MUTED, 0xFFFFFFFF);
    const char* tiles[] = {"Wi-Fi", "BT", "Air", "Night", "Focus", "Cast", "Proj", "Hotspot"};
    for (int i = 0; i < 8; i++) {
        i32 tx = x + 20 + (i % 4) * 80;
        i32 ty = y + 224 + (i / 4) * 70;
        fb_fill_rounded_rect(tx, ty, 72, 58, 8, (i == 0) ? g_accent : COL_HOVER);
        fb_draw_string(tx + 12, ty + 24, tiles[i],
                       (i == 0) ? 0xFF061018 : COL_TEXT, 0xFFFFFFFF);
    }

    fb_draw_string(x + 20, y + 380, adaptive_profile_name(), COL_MUTED, 0xFFFFFFFF);
}

static void draw_snap_flyout(fb_t* fb) {
    if (!g_snap_ui) return;
    i32 w = 280, h = 120;
    i32 x = ((i32)fb->width - w) / 2;
    i32 y = 60;
    fb_fill_rounded_rect(x, y, w, h, 10, COL_START_BG);
    fb_draw_string(x + 16, y + 14, "Snap layouts", g_accent, 0xFFFFFFFF);
    /* 6 layout previews */
    for (int i = 0; i < 6; i++) {
        i32 px = x + 16 + i * 42;
        i32 py = y + 44;
        fb_draw_rect(px, py, 36, 48, COL_MUTED);
        if (i == 0) fb_fill_rect(px + 2, py + 2, 15, 44, g_accent);
        if (i == 1) fb_fill_rect(px + 19, py + 2, 15, 44, g_accent);
        if (i == 2) { fb_fill_rect(px + 2, py + 2, 15, 20, g_accent); }
        if (i == 3) { fb_fill_rect(px + 19, py + 2, 15, 20, g_accent); }
        if (i == 4) { fb_fill_rect(px + 2, py + 26, 15, 20, g_accent); }
        if (i == 5) { fb_fill_rect(px + 19, py + 26, 15, 20, g_accent); }
    }
}

static void draw_context_menu(void) {
    if (!g_ctx) return;
    i32 x = g_ctx_x, y = g_ctx_y;
    i32 w = 200, h = 168;
    fb_fill_rounded_rect(x, y, w, h, 8, COL_START_BG);
    fb_draw_rect(x, y, w, h, 0xFF2A3A45);
    const char* items[] = {
        "View", "Sort by", "Refresh",
        "New folder", "Display settings", "Personalize"
    };
    for (int i = 0; i < 6; i++) {
        fb_draw_string(x + 16, y + 14 + i * 24, items[i], COL_TEXT, 0xFFFFFFFF);
    }
}

static void draw_brand_watermark(fb_t* fb) {
    i32 x = (i32)fb->width - 220;
    i32 y = (i32)fb->height - TASKBAR_H - 70;
    fb_draw_string(x, y, "H3OS", g_accent, 0xFFFFFFFF);
    fb_draw_string(x, y + 16, H3OS_VERSION_STRING "  " H3OS_CODENAME, COL_MUTED, 0xFFFFFFFF);
}

void desktop_init(void) {
    g_theme = THEME_DARK;
    g_accent = COL_ACCENT;
    g_start = g_search = g_action = g_ctx = g_snap_ui = false;
    g_frame = 0;
    g_search_q[0] = '\0';
    g_search_len = 0;
    KLOG_INFO("desktop", "Windows-style Horizon shell ready");
}

void desktop_render(void) {
    fb_t* fb = fb_get();
    draw_wallpaper(fb);
    draw_desktop_icons();
    draw_brand_watermark(fb);
    wm_render();
    draw_start_menu(fb);
    draw_action_center(fb);
    draw_snap_flyout(fb);
    draw_context_menu();
    draw_taskbar(fb);
    mouse_draw_cursor();
    fb_swap();
}

void desktop_tick(void) {
    const perf_settings_t* p = adaptive_settings();
    if (p->animations) g_frame++;
}

void desktop_handle_mouse(void) {
    mouse_state_t m;
    mouse_get(&m);
    g_click_consumed = false;
    fb_t* fb = fb_get();
    i32 tb_y = (i32)fb->height - TASKBAR_H;

    /* Right-click desktop context menu */
    if (m.right_pressed) {
        if (m.y < tb_y && !wm_at(m.x, m.y)) {
            g_ctx = true;
            g_ctx_x = m.x;
            g_ctx_y = m.y;
            g_start = g_action = g_search = false;
            g_click_consumed = true;
            return;
        }
    }

    if (!m.left_pressed) return;

    /* Close popups when clicking elsewhere */
    bool in_taskbar = (m.y >= tb_y);

    /* Taskbar interactions */
    if (in_taskbar) {
        g_ctx = false;
        /* Start */
        if (m.x >= 8 && m.x < 48) {
            desktop_toggle_launcher();
            g_click_consumed = true;
            return;
        }
        /* Search */
        if (m.x >= 56 && m.x < 236) {
            g_search = true;
            g_start = false;
            g_action = false;
            g_click_consumed = true;
            return;
        }
        /* Pinned */
        if (m.x >= 250 && m.x < 250 + 5 * 52) {
            int idx = (m.x - 250) / 52;
            if (idx >= 0 && idx < 5) desktop_launch((desktop_app_t)idx);
            g_click_consumed = true;
            return;
        }
        /* Task buttons */
        i32 tx = 520;
        u32 n = wm_count();
        for (u32 i = 0; i < n && i < 6; i++) {
            if (m.x >= tx && m.x < tx + 100) {
                window_t* w = wm_get(i);
                wm_restore_or_focus(w);
                g_click_consumed = true;
                return;
            }
            tx += 108;
        }
        /* Action center */
        i32 tray_x = (i32)fb->width - 210;
        if (m.x >= tray_x + 78 && m.x < tray_x + 106) {
            g_action = !g_action;
            g_start = false;
            g_click_consumed = true;
            return;
        }
        /* Clock / date → action center */
        if (m.x >= (i32)fb->width - 90) {
            g_action = !g_action;
            g_click_consumed = true;
            return;
        }
        g_click_consumed = true;
        return;
    }

    /* Start menu clicks */
    if (g_start) {
        i32 sx = 8;
        i32 sy = tb_y - START_H - 8;
        if (m.x >= sx && m.x < sx + START_W && m.y >= sy && m.y < sy + START_H) {
            /* pinned grid */
            for (int i = 0; i < 8; i++) {
                i32 ax = sx + 24 + (i % 4) * 94;
                i32 ay = sy + 70 + (i / 4) * 90;
                if (m.x >= ax && m.x < ax + 84 && m.y >= ay && m.y < ay + 76) {
                    if (i < APP_COUNT) desktop_launch((desktop_app_t)i);
                    else if (i == 5) terminal_open(); /* Notepad → terminal for now */
                    else if (i == 6) terminal_open();
                    else if (i == 7) power_shutdown();
                    g_click_consumed = true;
                    return;
                }
            }
            /* Power off button */
            if (m.x >= sx + START_W - 68 && m.y >= sy + START_H - 42) {
                power_shutdown();
                g_click_consumed = true;
                return;
            }
            g_click_consumed = true;
            return;
        }
        g_start = false;
    }

    /* Action center clicks */
    if (g_action) {
        i32 ax = (i32)fb->width - ACTION_W - 12;
        i32 ay = tb_y - 420 - 8;
        if (m.x >= ax && m.x < ax + ACTION_W && m.y >= ay && m.y < ay + 420) {
            g_notify_count = 0;
            g_click_consumed = true;
            return;
        }
        g_action = false;
    }

    /* Snap flyout */
    if (g_snap_ui) {
        i32 w = 280, h = 120;
        i32 x = ((i32)fb->width - w) / 2;
        i32 y = 60;
        if (m.x >= x && m.x < x + w && m.y >= y && m.y < y + h) {
            for (int i = 0; i < 6; i++) {
                i32 px = x + 16 + i * 42;
                if (m.x >= px && m.x < px + 36) {
                    window_t* foc = wm_focused();
                    if (foc) wm_snap(foc, i);
                    g_snap_ui = false;
                    g_click_consumed = true;
                    return;
                }
            }
        }
        g_snap_ui = false;
    }

    /* Context menu */
    if (g_ctx) {
        if (m.x >= g_ctx_x && m.x < g_ctx_x + 200 &&
            m.y >= g_ctx_y && m.y < g_ctx_y + 168) {
            int item = (m.y - g_ctx_y - 10) / 24;
            if (item == 4) desktop_launch(APP_SETTINGS);
            if (item == 5) {
                static int th = 0; th = !th;
                desktop_set_theme(th ? THEME_LIGHT : THEME_DARK);
            }
            g_ctx = false;
            g_click_consumed = true;
            return;
        }
        g_ctx = false;
    }

    /* Desktop icons */
    for (u32 i = 0; i < H3OS_ARRAY_SIZE(g_icons); i++) {
        i32 x = g_icons[i].x, y = g_icons[i].y;
        if (m.x >= x && m.x < x + 64 && m.y >= y && m.y < y + 64) {
            desktop_launch(g_icons[i].app);
            g_click_consumed = true;
            return;
        }
    }
}

void desktop_toggle_snap(void) { g_snap_ui = !g_snap_ui; }
