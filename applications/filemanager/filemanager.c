/**
 * H3OS File Manager — graphical VFS browser
 */
#include "filemanager.h"
#include "../../drivers/framebuffer/framebuffer.h"
#include "../../filesystem/vfs/vfs.h"
#include <h3os/string.h>

typedef struct {
    char path[VFS_PATH_MAX];
} fm_state_t;

static fm_state_t g_fm;

static void fm_draw(window_t* self) {
    fm_state_t* st = (fm_state_t*)self->user;
    i32 x = self->x + 12;
    i32 y = self->y + 40;

    fb_draw_string(x, y, "Path:", 0xFF7FA3AD, 0xFFFFFFFF);
    fb_draw_string(x + 48, y, st->path, 0xFF2EC4B6, 0xFFFFFFFF);
    y += 20;
    fb_draw_string(x, y, "NAME                  TYPE   SIZE", 0xFF7FA3AD, 0xFFFFFFFF);
    y += 14;

    vfs_node_t* dir = vfs_resolve(st->path);
    if (!dir || dir->type != VFS_DIR) {
        fb_draw_string(x, y, "(invalid directory)", 0xFFE85D4C, 0xFFFFFFFF);
        return;
    }

    int row = 0;
    for (vfs_node_t* c = dir->child; c && row < 16; c = c->sibling, row++) {
        char line[96];
        line[0] = '\0';
        strncpy(line, c->name, 20);
        while (strlen(line) < 22) strcat(line, " ");
        strcat(line, c->type == VFS_DIR ? "dir    " : "file   ");
        if (c->type == VFS_FILE) {
            char num[24];
            utoa(c->size, num, 10);
            strcat(line, num);
        }
        fb_draw_string(x, y + row * 12, line, 0xFFE8F4F8, 0xFFFFFFFF);
    }
    if (!dir->child) {
        fb_draw_string(x, y, "(empty)", 0xFF7FA3AD, 0xFFFFFFFF);
    }
}

window_t* filemanager_open(void) {
    strncpy(g_fm.path, vfs_cwd(), VFS_PATH_MAX - 1);
    window_t* w = wm_create("File Manager", 80, 70, 480, 320);
    if (!w) return NULL;
    w->user = &g_fm;
    w->draw = fm_draw;
    return w;
}
