/**
 * H3OS IPC — port registry (queueing wired with scheduler sleep later)
 */
#include <h3os/ipc.h>
#include <h3os/kernel.h>
#include <h3os/string.h>

static ipc_port_t ports[IPC_MAX_PORTS];
static u32 port_count = 0;

void ipc_init(void) {
    memset(ports, 0, sizeof(ports));
    port_count = 0;
    KLOG_INFO("ipc", "IPC ports ready");
}

i32 ipc_create_port(const char* name) {
    if (port_count >= IPC_MAX_PORTS) return -1;
    ipc_port_t* p = &ports[port_count];
    p->id = port_count;
    p->active = true;
    strncpy(p->name, name ? name : "port", 31);
    port_count++;
    return (i32)p->id;
}

i32 ipc_send(u32 port, const ipc_message_t* msg) {
    if (port >= port_count || !ports[port].active || !msg) return -1;
    H3OS_UNUSED(msg);
    return 0; /* queued in future */
}

i32 ipc_recv(u32 port, ipc_message_t* msg) {
    if (port >= port_count || !ports[port].active || !msg) return -1;
    return -1; /* would block */
}
