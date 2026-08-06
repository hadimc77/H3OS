/**
 * H3OS — Desktop Environment shell
 */
#ifndef H3OS_DESKTOP_H
#define H3OS_DESKTOP_H

#include <h3os/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    THEME_DARK = 0,
    THEME_LIGHT
} theme_mode_t;

typedef enum {
    APP_TERMINAL = 0,
    APP_FILES,
    APP_SETTINGS,
    APP_CALC,
    APP_TASKS,
    APP_COUNT
} desktop_app_t;

void desktop_init(void);
void desktop_render(void);
void desktop_tick(void);
void desktop_handle_mouse(void);
bool desktop_mouse_click_consumed(void); /* true if dock/launcher ate the click */
void desktop_set_theme(theme_mode_t mode);
void desktop_set_accent(u32 color);
void desktop_toggle_launcher(void);
bool desktop_launcher_open(void);
void desktop_launch(desktop_app_t app);

#ifdef __cplusplus
}
#endif

#endif /* H3OS_DESKTOP_H */
