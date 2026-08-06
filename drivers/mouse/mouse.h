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
    bool left_pressed;   /* rising edge */
    bool left_released;  /* falling edge */
} mouse_state_t;

void mouse_init(void);
void mouse_get(mouse_state_t* out);
void mouse_poll_frame(void); /* clear edge flags after UI consumes them */
void mouse_draw_cursor(void);

#ifdef __cplusplus
}
#endif

#endif
