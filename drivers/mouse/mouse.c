#include "mouse.h"
#include <h3os/kernel.h>
#include <h3os/string.h>

static mouse_state_t g_mouse;

void mouse_init(void) {
    memset(&g_mouse, 0, sizeof(g_mouse));
    KLOG_INFO("mouse", "PS/2 mouse stub — IRQ12 path planned");
}

void mouse_get(mouse_state_t* out) {
    if (out) *out = g_mouse;
}
