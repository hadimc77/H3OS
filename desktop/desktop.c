/**
 * H3OS Desktop — Horizon shell
 *
 * Brand-first: H3OS wordmark dominates the first composition.
 * Palette: deep teal ocean night — original, not generic AI purple/cream.
 */
#include "desktop.h"
#include "../drivers/framebuffer/framebuffer.h"
#include "../window_manager/wm.h"
#include <h3os/adaptive.h>
#include <h3os/version.h>
#include <h3os/kernel.h>
#include <h3os/string.h>
#include "../drivers/timer/timer.h"
#include "../memory/pmm.h"

#define COL_BG0        0xFF061018
#define COL_BG1        0xFF0A1E28
#define COL_ACCENT     0xFF2EC4B6
#define COL_ACCENT_DIM 0xFF1A6B62
#define COL_TEXT       0xFFE8F4F8
#define COL_MUTED      0xFF7FA3AD
#define COL_PANEL      0xEE0B1F2A
#define COL_TASKBAR    0xF00B1F2A
#define COL_DOCK       0xE00E2A36
#define COL_LIGHT_BG   0xFFF2F7F9
#define COL_LIGHT_TEXT 0xFF0B1F2A

static theme_mode_t g_theme = THEME_DARK;
static u32 g_accent = COL_ACCENT;
static bool g_launcher = false;
static u32 g_frame = 0;

void desktop_set_theme(theme_mode_t mode) { g_theme = mode; }
void desktop_set_accent(u32 color) { g_accent = color; }
void desktop_toggle_launcher(void) { g_launcher = !g_launcher; }
bool desktop_launcher_open(void) { return g_launcher; }

static void draw_wallpaper(fb_t* fb) {
    u32 top = (g_theme == THEME_DARK) ? COL_BG0 : COL_LIGHT_BG;
    u32 bot = (g_theme == THEME_DARK) ? COL_BG1 : 0xFFD5E8EE;

    for (u32 y = 0; y < fb->height; y++) {
        /* Vertical gradient */
        u32 t = (y * 255) / (fb->height ? fb->height : 1);
        u32 r0 = (top >> 16) & 0xFF, g0 = (top >> 8) & 0xFF, b0 = top & 0xFF;
        u32 r1 = (bot >> 16) & 0xFF, g1 = (bot >> 8) & 0xFF, b1 = bot & 0xFF;
        u32 r = (r0 * (255 - t) + r1 * t) / 255;
        u32 g = (g0 * (255 - t) + g1 * t) / 255;
        u32 b = (b0 * (255 - t) + b1 * t) / 255;
        u32 color = 0xFF000000 | (r << 16) | (g << 8) | b;

        /* Soft diagonal atmosphere band */
        for (u32 x = 0; x < fb->width; x++) {
            u32 c = color;
            if ((x + y / 2 + (g_frame / 2)) % 97 < 2) {
                c = (g_theme == THEME_DARK) ? 0xFF0D2A36 : 0xFFE3F0F4;
            }
            fb_put_pixel((i32)x, (i32)y, c);
        }
    }

    /* Brand hero — first viewport composition: brand > headline > tagline */
    u32 brand = (g_theme == THEME_DARK) ? g_accent : COL_ACCENT_DIM;
    u32 text  = (g_theme == THEME_DARK) ? COL_TEXT : COL_LIGHT_TEXT;
    u32 muted = COL_MUTED;

    i32 cx = (i32)fb->width / 2 - 80;
    i32 cy = (i32)fb->height / 2 - 60;

    /* Large brand mark */
    fb_draw_string(cx, cy, "H3OS", brand, 0xFFFFFFFF);
    fb_draw_string(cx, cy + 18, H3OS_TAGLINE, text, 0xFFFFFFFF);
    fb_draw_string(cx, cy + 34, "Press Ctrl+T for Terminal  |  Ctrl+L Launcher",
                   muted, 0xFFFFFFFF);
}

static void draw_top_bar(fb_t* fb) {
    fb_fill_rect(0, 0, (i32)fb->width, 36, COL_PANEL);
    fb_draw_string(16, 14, "H3OS", g_accent, 0xFFFFFFFF);
    fb_draw_string(56, 14, H3OS_VERSION_STRING, COL_MUTED, 0xFFFFFFFF);

    /* Clock (uptime-based placeholder HH:MM:SS) */
    u64 ms = timer_uptime_ms();
    u64 sec = (ms / 1000) % 60;
    u64 min = (ms / 60000) % 60;
    u64 hr  = (ms / 3600000) % 24;
    char clock[16];
    clock[0] = (char)('0' + (hr / 10)); clock[1] = (char)('0' + (hr % 10));
    clock[2] = ':';
    clock[3] = (char)('0' + (min / 10)); clock[4] = (char)('0' + (min % 10));
    clock[5] = ':';
    clock[6] = (char)('0' + (sec / 10)); clock[7] = (char)('0' + (sec % 10));
    clock[8] = '\0';
    fb_draw_string((i32)fb->width - 80, 14, clock, COL_TEXT, 0xFFFFFFFF);

    const char* prof = adaptive_profile_name();
    fb_draw_string((i32)fb->width - 280, 14, prof, COL_MUTED, 0xFFFFFFFF);
}

static void draw_dock(fb_t* fb) {
    i32 dock_w = 280;
    i32 dock_h = 48;
    i32 x = ((i32)fb->width - dock_w) / 2;
    i32 y = (i32)fb->height - dock_h - 12;

    fb_fill_rounded_rect(x, y, dock_w, dock_h, 12, COL_DOCK);

    const char* icons[] = {"Term", "Files", "Set", "Calc", "Task"};
    for (int i = 0; i < 5; i++) {
        i32 ix = x + 20 + i * 52;
        fb_fill_rounded_rect(ix, y + 8, 40, 32, 6, 0xFF143642);
        fb_draw_string(ix + 4, y + 18, icons[i], COL_TEXT, 0xFFFFFFFF);
    }
}

static void draw_launcher(fb_t* fb) {
    if (!g_launcher) return;
    i32 w = 420, h = 320;
    i32 x = ((i32)fb->width - w) / 2;
    i32 y = ((i32)fb->height - h) / 2 - 20;

    fb_fill_rounded_rect(x, y, w, h, 14, COL_PANEL);
    fb_draw_rect(x, y, w, h, g_accent);
    fb_draw_string(x + 24, y + 24, "H3OS Launcher", g_accent, 0xFFFFFFFF);
    fb_draw_string(x + 24, y + 48, "Search apps, settings, files...", COL_MUTED, 0xFFFFFFFF);

    const char* apps[] = {
        "Terminal", "File Manager", "Settings", "Calculator",
        "Task Manager", "Text Editor", "Image Viewer", "Package Manager"
    };
    for (int i = 0; i < 8; i++) {
        i32 ax = x + 24 + (i % 2) * 190;
        i32 ay = y + 80 + (i / 2) * 48;
        fb_fill_rounded_rect(ax, ay, 170, 36, 8, 0xFF143642);
        fb_draw_string(ax + 12, ay + 14, apps[i], COL_TEXT, 0xFFFFFFFF);
    }
}

void desktop_init(void) {
    g_theme = THEME_DARK;
    g_accent = COL_ACCENT;
    g_launcher = false;
    g_frame = 0;
    KLOG_INFO("desktop", "Horizon desktop environment started");
}

void desktop_render(void) {
    fb_t* fb = fb_get();
    draw_wallpaper(fb);
    draw_top_bar(fb);
    wm_render();
    draw_dock(fb);
    draw_launcher(fb);

    /* Status strip */
    pmm_stats_t st;
    pmm_get_stats(&st);
    char mem[64];
    /* simple status */
    fb_draw_string(16, (i32)fb->height - 14,
                   "H3OS Horizon  |  hybrid kernel  |  The Future Starts Here.",
                   COL_MUTED, 0xFFFFFFFF);
    H3OS_UNUSED(mem);
    H3OS_UNUSED(st);

    fb_swap();
}

void desktop_tick(void) {
    const perf_settings_t* p = adaptive_settings();
    if (p->animations) g_frame++;
}
