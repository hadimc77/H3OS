/**
 * H3OS — Cooperative round-robin scheduler (preemption via timer tick counter)
 *
 * Full context-switch assembly comes in a later milestone; this layer tracks
 * tasks, accounting, and cooperative yield for kernel threads.
 */
#include <h3os/sched.h>
#include <h3os/kernel.h>
#include <h3os/string.h>
#include <h3os/sync.h>

static task_t tasks[MAX_TASKS];
static task_t* current = NULL;
static task_t* ready_q = NULL;
static u32 next_id = 1;
static u32 task_count = 0;
static spinlock_t sched_lock;

void sched_init(void) {
    memset(tasks, 0, sizeof(tasks));
    spinlock_init(&sched_lock);
    current = NULL;
    ready_q = NULL;
    next_id = 1;
    task_count = 0;

    /* Idle / kernel task */
    task_t* idle = sched_create("kernel-idle", NULL, NULL);
    if (idle) {
        idle->state = TASK_RUNNING;
        current = idle;
    }
    KLOG_INFO("sched", "Scheduler ready (cooperative RR)");
}

task_t* sched_create(const char* name, void (*entry)(void*), void* arg) {
    spin_lock(&sched_lock);
    task_t* t = NULL;
    for (int i = 0; i < MAX_TASKS; i++) {
        if (tasks[i].state == TASK_DEAD || tasks[i].id == 0) {
            t = &tasks[i];
            break;
        }
    }
    if (!t) { spin_unlock(&sched_lock); return NULL; }

    memset(t, 0, sizeof(*t));
    t->id = next_id++;
    strncpy(t->name, name ? name : "task", TASK_NAME - 1);
    t->state = TASK_READY;
    t->entry = entry;
    t->arg = arg;
    t->next = ready_q;
    ready_q = t;
    task_count++;
    spin_unlock(&sched_lock);
    return t;
}

void sched_yield(void) {
    /* Cooperative: rotate ready queue metadata */
    spin_lock(&sched_lock);
    if (current) current->state = TASK_READY;
    if (ready_q) {
        task_t* next = ready_q;
        ready_q = ready_q->next;
        if (current && current->state != TASK_DEAD) {
            current->next = NULL;
            task_t** tail = &ready_q;
            while (*tail) tail = &(*tail)->next;
            *tail = current;
        }
        current = next;
        current->state = TASK_RUNNING;
        current->next = NULL;
    }
    spin_unlock(&sched_lock);
}

void sched_tick(void) {
    if (current) current->ticks++;
}

task_t* sched_current(void) { return current; }
u32 sched_task_count(void) { return task_count; }
