/**
 * H3OS — IPC primitives (message ports)
 */
#ifndef H3OS_IPC_H
#define H3OS_IPC_H

#include <h3os/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define IPC_MAX_PORTS 64
#define IPC_MSG_SIZE  256

typedef struct {
    u32 from;
    u32 type;
    u32 size;
    u8  data[IPC_MSG_SIZE];
} ipc_message_t;

typedef struct {
    u32 id;
    bool active;
    char name[32];
} ipc_port_t;

void ipc_init(void);
i32  ipc_create_port(const char* name);
i32  ipc_send(u32 port, const ipc_message_t* msg);
i32  ipc_recv(u32 port, ipc_message_t* msg);

#ifdef __cplusplus
}
#endif

#endif
