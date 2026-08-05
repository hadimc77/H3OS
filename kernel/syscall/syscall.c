/**
 * H3OS — System call dispatcher (kernel side)
 */
#include <h3os/syscall.h>
#include <h3os/sched.h>
#include <h3os/kernel.h>
#include <h3os/string.h>
#include "../../drivers/timer/timer.h"

void syscall_init(void) {
    KLOG_INFO("syscall", "Syscall table online (%d entries)", SYS_MAX);
}

i64 syscall_dispatch(u64 id, u64 a0, u64 a1, u64 a2, u64 a3) {
    H3OS_UNUSED(a2);
    H3OS_UNUSED(a3);
    switch ((syscall_id_t)id) {
        case SYS_EXIT:
            KLOG_INFO("syscall", "exit(%llu)", (unsigned long long)a0);
            return 0;
        case SYS_WRITE: {
            const char* s = (const char*)(uintptr_t)a0;
            size_t n = (size_t)a1;
            for (size_t i = 0; i < n && s; i++) {
                /* route to kernel log serial for now */
                char tmp[2] = { s[i], 0 };
                if (s[i] == '\n') klog(LOG_INFO, "user", "%s", "");
                else { H3OS_UNUSED(tmp); }
            }
            return (i64)n;
        }
        case SYS_YIELD:
            sched_yield();
            return 0;
        case SYS_GETPID: {
            task_t* t = sched_current();
            return t ? (i64)t->id : 0;
        }
        case SYS_UPTIME:
            return (i64)timer_uptime_ms();
        case SYS_LOG:
            klog(LOG_INFO, "user", "%s", (const char*)(uintptr_t)a0);
            return 0;
        default:
            KLOG_WARN("syscall", "unknown syscall %llu", (unsigned long long)id);
            return -1;
    }
}
