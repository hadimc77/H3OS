#include "calculator.h"
#include "../../drivers/framebuffer/framebuffer.h"
#include <h3os/string.h>

static void calc_draw(window_t* self) {
    fb_draw_string(self->x + 16, self->y + 48, "H3OS Calculator", 0xFFE8F4F8, 0xFFFFFFFF);
    fb_draw_string(self->x + 16, self->y + 68, "0", 0xFF2EC4B6, 0xFFFFFFFF);
    fb_draw_string(self->x + 16, self->y + 96, "[ 7 ][ 8 ][ 9 ][ / ]", 0xFF7FA3AD, 0xFFFFFFFF);
    fb_draw_string(self->x + 16, self->y + 112, "[ 4 ][ 5 ][ 6 ][ * ]", 0xFF7FA3AD, 0xFFFFFFFF);
    fb_draw_string(self->x + 16, self->y + 128, "[ 1 ][ 2 ][ 3 ][ - ]", 0xFF7FA3AD, 0xFFFFFFFF);
    fb_draw_string(self->x + 16, self->y + 144, "[ 0 ][ . ][ = ][ + ]", 0xFF7FA3AD, 0xFFFFFFFF);
}

window_t* calculator_open(void) {
    window_t* w = wm_create("Calculator", 100, 120, 280, 220);
    if (!w) return NULL;
    w->draw = calc_draw;
    return w;
}
