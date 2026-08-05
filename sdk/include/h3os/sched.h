/**
 * H3OS — Process / thread / scheduler interfaces
 */
#ifndef H3OS_SCHED_H
#define H3OS_SCHED_H

#include <h3os/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_TASKS 64
#define TASK_NAME 32

typedef enum {
    TASK_READY = 0,
    TASK_RUNNING,
    TASK_BLOCKED,
    TASK_DEAD
} task_state_t;

typedef struct task {
    u32 id;
    char name[TASK_NAME];
    task_state_t state;
    u64 ticks;
    void (*entry)(void*);
    void* arg;
    struct task* next;
} task_t;

void    sched_init(void);
task_t* sched_create(const char* name, void (*entry)(void*), void* arg);
void    sched_yield(void);
void    sched_tick(void);
task_t* sched_current(void);
u32     sched_task_count(void);

#ifdef __cplusplus
}
#endif

#endif /* H3OS_SCHED_H */
