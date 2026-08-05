#include "settings.h"
#include "../../drivers/framebuffer/framebuffer.h"
#include <h3os/adaptive.h>
#include <h3os/version.h>

static void settings_draw(window_t* self) {
    fb_draw_string(self->x + 16, self->y + 48, "H3OS Settings", 0xFF2EC4B6, 0xFFFFFFFF);
    fb_draw_string(self->x + 16, self->y + 72, "Appearance  |  System  |  Network", 0xFFE8F4F8, 0xFFFFFFFF);
    fb_draw_string(self->x + 16, self->y + 100, "Performance profile:", 0xFF7FA3AD, 0xFFFFFFFF);
    fb_draw_string(self->x + 16, self->y + 116, adaptive_profile_name(), 0xFFE8F4F8, 0xFFFFFFFF);
    fb_draw_string(self->x + 16, self->y + 148, "Version " H3OS_VERSION_STRING " (" H3OS_CODENAME ")", 0xFF7FA3AD, 0xFFFFFFFF);
}

window_t* settings_open(void) {
    window_t* w = wm_create("Settings", 160, 100, 420, 260);
    if (!w) return NULL;
    w->draw = settings_draw;
    return w;
}
