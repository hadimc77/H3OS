/**
 * PS/2 mouse driver stub
 */
#ifndef H3OS_MOUSE_H
#define H3OS_MOUSE_H

#include <h3os/types.h>

typedef struct {
    i32 x, y;
    bool left, right, middle;
} mouse_state_t;

void mouse_init(void);
void mouse_get(mouse_state_t* out);

#endif
