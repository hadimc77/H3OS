/**
 * H3OS — Window Manager
 */
#ifndef H3OS_WM_H
#define H3OS_WM_H

#include <h3os/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define WM_MAX_WINDOWS 32
#define WM_TITLE_LEN   64

typedef enum {
    WIN_NORMAL = 0,
    WIN_MAXIMIZED,
    WIN_MINIMIZED,
    WIN_FULLSCREEN
} win_state_t;

typedef struct window window_t;

struct window {
    bool active;
    char title[WM_TITLE_LEN];
    i32  x, y, w, h;
    i32  restore_x, restore_y, restore_w, restore_h;
    u32  bg;
    u32  accent;
    win_state_t state;
    bool always_on_top;
    bool focused;
    void (*draw)(window_t* self);
    void (*on_key)(window_t* self, char c);
    void* user;
};

void      wm_init(void);
window_t* wm_create(const char* title, i32 x, i32 y, i32 w, i32 h);
void      wm_destroy(window_t* win);
void      wm_focus(window_t* win);
void      wm_move(window_t* win, i32 x, i32 y);
void      wm_resize(window_t* win, i32 w, i32 h);
void      wm_set_state(window_t* win, win_state_t state);
void      wm_snap(window_t* win, int layout); /* 0=left 1=right 2=top-left 3=top-right 4=bot-left 5=bot-right 6=max */
void      wm_render(void);
void      wm_handle_key(char c);
void      wm_handle_mouse(void);
window_t* wm_focused(void);
window_t* wm_at(i32 x, i32 y);
u32       wm_count(void);
window_t* wm_get(u32 index);
void      wm_restore_or_focus(window_t* win);
i32       wm_taskbar_height(void);

#ifdef __cplusplus
}
#endif

#endif /* H3OS_WM_H */
