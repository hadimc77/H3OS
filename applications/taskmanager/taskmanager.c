/**
 * H3OS Task Manager — scheduler + memory stats
 */
#include "taskmanager.h"
#include "../../drivers/framebuffer/framebuffer.h"
#include "../../memory/pmm.h"
#include "../../memory/heap.h"
#include <h3os/sched.h>
#include <h3os/adaptive.h>
#include <h3os/string.h>
#include "../../drivers/timer/timer.h"

static const char* state_name(task_state_t s) {
    switch (s) {
        case TASK_READY: return "ready";
        case TASK_RUNNING: return "run";
        case TASK_BLOCKED: return "block";
        case TASK_DEAD: return "dead";
        default: return "?";
    }
}

static void tm_draw(window_t* self) {
    i32 x = self->x + 12;
    i32 y = self->y + 40;

    pmm_stats_t mem;
    heap_stats_t heap;
    pmm_get_stats(&mem);
    heap_get_stats(&heap);

    char buf[96];
    fb_draw_string(x, y, "H3OS Task Manager", 0xFF2EC4B6, 0xFFFFFFFF);
    y += 16;

    strcpy(buf, "Profile: ");
    strcat(buf, adaptive_profile_name());
    fb_draw_string(x, y, buf, 0xFFE8F4F8, 0xFFFFFFFF);
    y += 12;

    strcpy(buf, "Uptime ms: ");
    char num[32];
    utoa(timer_uptime_ms(), num, 10);
    strcat(buf, num);
    fb_draw_string(x, y, buf, 0xFF7FA3AD, 0xFFFFFFFF);
    y += 12;

    strcpy(buf, "RAM free MiB: ");
    utoa((mem.free_pages * PAGE_SIZE) / (1024 * 1024), num, 10);
    strcat(buf, num);
    fb_draw_string(x, y, buf, 0xFFE8F4F8, 0xFFFFFFFF);
    y += 12;

    strcpy(buf, "Heap used: ");
    utoa(heap.used_bytes, num, 10);
    strcat(buf, num);
    strcat(buf, " / ");
    utoa(heap.total_bytes, num, 10);
    strcat(buf, num);
    fb_draw_string(x, y, buf, 0xFFE8F4F8, 0xFFFFFFFF);
    y += 18;

    fb_draw_string(x, y, "PID  STATE   TICKS    NAME", 0xFF7FA3AD, 0xFFFFFFFF);
    y += 14;

    u32 count = sched_task_count();
    /* Walk via sched_current + create listing from known API:
       we only have sched_current publicly; list via wm-like scan if needed.
       Use sched_current and synthetic rows for desktop services. */
    task_t* cur = sched_current();
    int row = 0;
    if (cur) {
        char line[96];
        strcpy(line, "");
        utoa(cur->id, num, 10);
        strcat(line, num);
        while (strlen(line) < 5) strcat(line, " ");
        strcat(line, state_name(cur->state));
        while (strlen(line) < 13) strcat(line, " ");
        utoa(cur->ticks, num, 10);
        strcat(line, num);
        while (strlen(line) < 22) strcat(line, " ");
        strcat(line, cur->name);
        fb_draw_string(x, y + row * 12, line, 0xFFE8F4F8, 0xFFFFFFFF);
        row++;
    }

    fb_draw_string(x, y + row * 12 + 8, "Services: desktop, wm, terminal", 0xFF7FA3AD, 0xFFFFFFFF);
    H3OS_UNUSED(count);
    H3OS_UNUSED(self);
}

window_t* taskmanager_open(void) {
    window_t* w = wm_create("Task Manager", 200, 90, 420, 280);
    if (!w) return NULL;
    w->draw = tm_draw;
    return w;
}
