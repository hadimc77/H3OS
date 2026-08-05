/**
 * FAT32 — not yet implemented; VFS returns -ENOSYS equivalent
 */
#include "fat32.h"
#include <h3os/kernel.h>

i32 fat32_mount(const char* device, const char* mountpoint) {
    H3OS_UNUSED(device);
    H3OS_UNUSED(mountpoint);
    KLOG_WARN("fat32", "FAT32 driver not linked in v0.1");
    return -1;
}

i32 fat32_unmount(const char* mountpoint) {
    H3OS_UNUSED(mountpoint);
    return -1;
}
