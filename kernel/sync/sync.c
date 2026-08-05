/**
 * H3OS — Spinlocks, mutexes, semaphores
 *
 * Spinlocks are IRQ-safe ticket-free atomic locks for short critical sections.
 * Mutex/semaphore currently spin (no sleep yet) — scheduler integration later.
 */
#include <h3os/sync.h>
#include <h3os/kernel.h>

void spinlock_init(spinlock_t* lock) {
    lock->locked = 0;
}

void spin_lock(spinlock_t* lock) {
    while (__sync_lock_test_and_set(&lock->locked, 1)) {
        while (lock->locked) cpu_pause();
    }
}

bool spin_trylock(spinlock_t* lock) {
    return __sync_lock_test_and_set(&lock->locked, 1) == 0;
}

void spin_unlock(spinlock_t* lock) {
    __sync_lock_release(&lock->locked);
}

void semaphore_init(semaphore_t* sem, i32 value) {
    sem->count = value;
    spinlock_init(&sem->lock);
}

void semaphore_wait(semaphore_t* sem) {
    for (;;) {
        spin_lock(&sem->lock);
        if (sem->count > 0) {
            sem->count--;
            spin_unlock(&sem->lock);
            return;
        }
        spin_unlock(&sem->lock);
        cpu_pause();
    }
}

void semaphore_post(semaphore_t* sem) {
    spin_lock(&sem->lock);
    sem->count++;
    spin_unlock(&sem->lock);
}

void mutex_init(mutex_t* mtx) {
    mtx->state = 0;
    spinlock_init(&mtx->lock);
}

void mutex_lock(mutex_t* mtx) {
    for (;;) {
        spin_lock(&mtx->lock);
        if (mtx->state == 0) {
            mtx->state = 1;
            spin_unlock(&mtx->lock);
            return;
        }
        spin_unlock(&mtx->lock);
        cpu_pause();
    }
}

void mutex_unlock(mutex_t* mtx) {
    spin_lock(&mtx->lock);
    mtx->state = 0;
    spin_unlock(&mtx->lock);
}
