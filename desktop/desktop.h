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

void desktop_init(void);
void desktop_render(void);
void desktop_tick(void);
void desktop_set_theme(theme_mode_t mode);
void desktop_set_accent(u32 color);
void desktop_toggle_launcher(void);
bool desktop_launcher_open(void);

#ifdef __cplusplus
}
#endif

#endif /* H3OS_DESKTOP_H */
