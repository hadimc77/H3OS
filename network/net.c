/**
 * H3OS — Network stack (loopback first; Ethernet/Wi-Fi drivers next)
 */
#include "net.h"
#include <h3os/kernel.h>
#include <h3os/string.h>

static net_iface_t lo;

void net_init(void) {
    memset(&lo, 0, sizeof(lo));
    lo.ipv4 = true;
    lo.ipv6 = true;
    lo.link_up = true;
    lo.ipv4_addr = 0x7F000001; /* 127.0.0.1 */
    lo.ipv4_mask = 0xFF000000;
    lo.mac[0] = 0x02; /* locally administered */

    KLOG_INFO("net", "Loopback up 127.0.0.1/8 (IPv6 ::1 planned)");
    KLOG_INFO("net", "Protocols staged: TCP UDP ICMP DNS DHCP HTTP HTTPS");
}

net_iface_t* net_loopback(void) { return &lo; }

i32 net_send(const void* data, size_t len) {
    H3OS_UNUSED(data);
    return (i32)len; /* loopback discard */
}

i32 net_poll(void* buf, size_t max) {
    H3OS_UNUSED(buf);
    H3OS_UNUSED(max);
    return 0;
}
