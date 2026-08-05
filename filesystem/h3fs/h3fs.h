/**
 * H3FS — H3OS native filesystem (on-disk format definition)
 *
 * v0.1 ships RAMFS that speaks the same VFS API. Persistent H3FS
 * journaled volumes land in a subsequent milestone.
 */
#ifndef H3OS_H3FS_H
#define H3OS_H3FS_H

#include <h3os/types.h>

#define H3FS_MAGIC       0x48334653u  /* 'H3FS' */
#define H3FS_VERSION     1
#define H3FS_BLOCK_SIZE  4096
#define H3FS_NAME_MAX    255

typedef struct {
    u32 magic;
    u32 version;
    u64 block_count;
    u64 inode_count;
    u64 free_blocks;
    u64 free_inodes;
    u64 journal_start;
    u64 journal_blocks;
    u8  uuid[16];
    char label[64];
} __attribute__((packed)) h3fs_superblock_t;

typedef struct {
    u32 mode;
    u32 uid, gid;
    u64 size;
    u64 atime, mtime, ctime;
    u64 direct[12];
    u64 indirect;
    u64 double_indirect;
    u32 flags;       /* journaled, compressed, encrypted */
    u32 nlink;
} __attribute__((packed)) h3fs_inode_t;

#endif /* H3OS_H3FS_H */
