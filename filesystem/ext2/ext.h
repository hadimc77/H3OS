/**
 * EXT2/EXT4 stubs — future persistent UNIX filesystems
 */
#ifndef H3OS_EXT_H
#define H3OS_EXT_H

#include <h3os/types.h>

i32 ext2_mount(const char* device, const char* mountpoint);
i32 ext4_mount(const char* device, const char* mountpoint);

#endif
