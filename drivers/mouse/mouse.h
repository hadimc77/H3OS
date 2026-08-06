/**
 * H3OS — PS/2 mouse driver (IRQ12, 3-byte packets)
 */
#ifndef H3OS_MOUSE_H
#define H3OS_MOUSE_H

#include <h3os/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    i32 x, y;
    i32 dx, dy;
    bool left, right, middle;
    bool left_pressed;
    bool left_released;
    bool right_pressed;
    bool right_released;
} mouse_state_t;

void mouse_init(void);
void mouse_get(mouse_state_t* out);
void mouse_poll_frame(void);
void mouse_draw_cursor(void);

#ifdef __cplusplus
}
#endif

#endif
