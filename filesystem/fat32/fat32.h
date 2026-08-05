/**
 * FAT32 filesystem driver — stub (read/write planned)
 */
#ifndef H3OS_FAT32_H
#define H3OS_FAT32_H

#include <h3os/types.h>

#ifdef __cplusplus
extern "C" {
#endif

i32 fat32_mount(const char* device, const char* mountpoint);
i32 fat32_unmount(const char* mountpoint);

#ifdef __cplusplus
}
#endif

#endif
