/**
 * H3OS — Synchronization primitives
 */
#ifndef H3OS_SYNC_H
#define H3OS_SYNC_H

#include <h3os/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    volatile u32 locked;
} spinlock_t;

typedef struct {
    volatile i32 count;
    spinlock_t   lock;
} semaphore_t;

typedef struct {
    volatile i32 state; /* 0 unlocked, 1 locked */
    spinlock_t   lock;
} mutex_t;

void spinlock_init(spinlock_t* lock);
void spin_lock(spinlock_t* lock);
void spin_unlock(spinlock_t* lock);
bool spin_trylock(spinlock_t* lock);

void semaphore_init(semaphore_t* sem, i32 value);
void semaphore_wait(semaphore_t* sem);
void semaphore_post(semaphore_t* sem);

void mutex_init(mutex_t* mtx);
void mutex_lock(mutex_t* mtx);
void mutex_unlock(mutex_t* mtx);

#ifdef __cplusplus
}
#endif

#endif /* H3OS_SYNC_H */
