/**
 * H3OS — Virtual File System
 */
#ifndef H3OS_VFS_H
#define H3OS_VFS_H

#include <h3os/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define VFS_NAME_MAX 128
#define VFS_PATH_MAX 256
#define VFS_MAX_NODES 256

typedef enum {
    VFS_FILE = 1,
    VFS_DIR  = 2
} vfs_type_t;

typedef struct vfs_node {
    char name[VFS_NAME_MAX];
    vfs_type_t type;
    u32 mode;          /* unix-like permissions */
    u32 uid, gid;
    u64 size;
    u8* data;
    struct vfs_node* parent;
    struct vfs_node* child;
    struct vfs_node* sibling;
} vfs_node_t;

void        vfs_init(void);
vfs_node_t* vfs_root(void);
vfs_node_t* vfs_resolve(const char* path);
vfs_node_t* vfs_mkdir(const char* path, u32 mode);
vfs_node_t* vfs_create(const char* path, u32 mode);
i32         vfs_write(vfs_node_t* node, const void* buf, u64 size);
i32         vfs_read(vfs_node_t* node, void* buf, u64 max, u64* out_size);
i32         vfs_unlink(const char* path);
i32         vfs_copy(const char* src, const char* dst);
i32         vfs_rename(const char* src, const char* dst);
void        vfs_list(vfs_node_t* dir, void (*cb)(vfs_node_t*, void*), void* ctx);
const char* vfs_cwd(void);
i32         vfs_chdir(const char* path);

#ifdef __cplusplus
}
#endif

#endif /* H3OS_VFS_H */
