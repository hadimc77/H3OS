/**
 * H3OS — System call ABI
 */
#ifndef H3OS_SYSCALL_H
#define H3OS_SYSCALL_H

#include <h3os/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    SYS_EXIT     = 0,
    SYS_WRITE    = 1,
    SYS_READ     = 2,
    SYS_OPEN     = 3,
    SYS_CLOSE    = 4,
    SYS_YIELD    = 5,
    SYS_GETPID   = 6,
    SYS_UPTIME   = 7,
    SYS_LOG      = 8,
    SYS_MAX
} syscall_id_t;

void  syscall_init(void);
i64   syscall_dispatch(u64 id, u64 a0, u64 a1, u64 a2, u64 a3);

#ifdef __cplusplus
}
#endif

#endif /* H3OS_SYSCALL_H */
