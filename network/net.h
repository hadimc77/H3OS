/**
 * H3OS — Networking stack scaffold
 */
#ifndef H3OS_NET_H
#define H3OS_NET_H

#include <h3os/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    bool ipv4;
    bool ipv6;
    bool link_up;
    u8   mac[6];
    u32  ipv4_addr;
    u32  ipv4_mask;
    u32  gateway;
} net_iface_t;

void net_init(void);
net_iface_t* net_loopback(void);
i32  net_send(const void* data, size_t len);
i32  net_poll(void* buf, size_t max);

#ifdef __cplusplus
}
#endif

#endif /* H3OS_NET_H */
