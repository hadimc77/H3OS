/**
 * H3OS — VFS + RAMFS bootstrap (H3FS-compatible in-memory tree)
 */
#include "vfs.h"
#include "../../memory/heap.h"
#include <h3os/kernel.h>
#include <h3os/string.h>

static vfs_node_t nodes[VFS_MAX_NODES];
static u32 node_count = 0;
static vfs_node_t* root = NULL;
static char cwd[VFS_PATH_MAX] = "/";

static vfs_node_t* alloc_node(const char* name, vfs_type_t type, u32 mode) {
    if (node_count >= VFS_MAX_NODES) return NULL;
    vfs_node_t* n = &nodes[node_count++];
    memset(n, 0, sizeof(*n));
    strncpy(n->name, name, VFS_NAME_MAX - 1);
    n->type = type;
    n->mode = mode;
    return n;
}

static void link_child(vfs_node_t* parent, vfs_node_t* child) {
    child->parent = parent;
    child->sibling = parent->child;
    parent->child = child;
}

static vfs_node_t* find_child(vfs_node_t* dir, const char* name) {
    for (vfs_node_t* c = dir->child; c; c = c->sibling) {
        if (strcmp(c->name, name) == 0) return c;
    }
    return NULL;
}

void vfs_init(void) {
    memset(nodes, 0, sizeof(nodes));
    node_count = 0;
    root = alloc_node("/", VFS_DIR, 0755);
    strcpy(cwd, "/");

    /* Seed a familiar UNIX-like hierarchy on RAMFS */
    vfs_mkdir("/bin", 0755);
    vfs_mkdir("/etc", 0755);
    vfs_mkdir("/home", 0755);
    vfs_mkdir("/home/user", 0755);
    vfs_mkdir("/tmp", 0777);
    vfs_mkdir("/var", 0755);
    vfs_mkdir("/usr", 0755);
    vfs_mkdir("/usr/share", 0755);
    vfs_mkdir("/dev", 0755);
    vfs_mkdir("/proc", 0755);

    vfs_node_t* motd = vfs_create("/etc/motd", 0644);
    if (motd) {
        const char* msg = "Welcome to H3OS — The Future Starts Here.\n";
        vfs_write(motd, msg, strlen(msg));
    }
    vfs_node_t* ver = vfs_create("/etc/h3os-release", 0644);
    if (ver) {
        const char* msg = "NAME=\"H3OS\"\nVERSION=\"0.1.0\"\nCODENAME=\"Horizon\"\n";
        vfs_write(ver, msg, strlen(msg));
    }

    KLOG_INFO("vfs", "RAMFS/H3FS root mounted with %u nodes", node_count);
}

vfs_node_t* vfs_root(void) { return root; }
const char* vfs_cwd(void) { return cwd; }

static void normalize_path(const char* path, char* out) {
    if (path[0] == '/') {
        strncpy(out, path, VFS_PATH_MAX - 1);
    } else {
        strcpy(out, cwd);
        if (out[strlen(out) - 1] != '/') strcat(out, "/");
        strcat(out, path);
    }
    out[VFS_PATH_MAX - 1] = '\0';
}

vfs_node_t* vfs_resolve(const char* path) {
    char full[VFS_PATH_MAX];
    normalize_path(path, full);

    if (strcmp(full, "/") == 0) return root;

    vfs_node_t* cur = root;
    char* p = full + 1;
    char part[VFS_NAME_MAX];

    while (*p) {
        size_t i = 0;
        while (*p && *p != '/' && i + 1 < VFS_NAME_MAX) part[i++] = *p++;
        part[i] = '\0';
        while (*p == '/') p++;
        if (part[0] == '\0') continue;
        if (strcmp(part, ".") == 0) continue;
        if (strcmp(part, "..") == 0) {
            if (cur->parent) cur = cur->parent;
            continue;
        }
        cur = find_child(cur, part);
        if (!cur) return NULL;
    }
    return cur;
}

static vfs_node_t* ensure_parent(const char* path, char* leaf_out) {
    char full[VFS_PATH_MAX];
    normalize_path(path, full);

    char* last = full;
    for (char* s = full; *s; s++) if (*s == '/') last = s;
    if (last == full) {
        strcpy(leaf_out, full + 1);
        return root;
    }
    *last = '\0';
    strcpy(leaf_out, last + 1);
    return vfs_resolve(full[0] ? full : "/");
}

vfs_node_t* vfs_mkdir(const char* path, u32 mode) {
    char leaf[VFS_NAME_MAX];
    vfs_node_t* parent = ensure_parent(path, leaf);
    if (!parent || parent->type != VFS_DIR || leaf[0] == '\0') return NULL;
    if (find_child(parent, leaf)) return find_child(parent, leaf);
    vfs_node_t* n = alloc_node(leaf, VFS_DIR, mode);
    if (!n) return NULL;
    link_child(parent, n);
    return n;
}

vfs_node_t* vfs_create(const char* path, u32 mode) {
    char leaf[VFS_NAME_MAX];
    vfs_node_t* parent = ensure_parent(path, leaf);
    if (!parent || parent->type != VFS_DIR || leaf[0] == '\0') return NULL;
    if (find_child(parent, leaf)) return find_child(parent, leaf);
    vfs_node_t* n = alloc_node(leaf, VFS_FILE, mode);
    if (!n) return NULL;
    link_child(parent, n);
    return n;
}

i32 vfs_write(vfs_node_t* node, const void* buf, u64 size) {
    if (!node || node->type != VFS_FILE) return -1;
    u8* data = (u8*)kmalloc((size_t)size + 1);
    if (!data) return -1;
    memcpy(data, buf, (size_t)size);
    data[size] = 0;
    if (node->data) kfree(node->data);
    node->data = data;
    node->size = size;
    return (i32)size;
}

i32 vfs_read(vfs_node_t* node, void* buf, u64 max, u64* out_size) {
    if (!node || node->type != VFS_FILE) return -1;
    u64 n = node->size < max ? node->size : max;
    if (n && node->data) memcpy(buf, node->data, (size_t)n);
    if (out_size) *out_size = n;
    return (i32)n;
}

i32 vfs_unlink(const char* path) {
    vfs_node_t* n = vfs_resolve(path);
    if (!n || n == root) return -1;
    vfs_node_t* parent = n->parent;
    if (!parent) return -1;
    if (parent->child == n) {
        parent->child = n->sibling;
    } else {
        for (vfs_node_t* c = parent->child; c; c = c->sibling) {
            if (c->sibling == n) { c->sibling = n->sibling; break; }
        }
    }
    if (n->data) { kfree(n->data); n->data = NULL; }
    n->size = 0;
    n->name[0] = '\0';
    return 0;
}

void vfs_list(vfs_node_t* dir, void (*cb)(vfs_node_t*, void*), void* ctx) {
    if (!dir || dir->type != VFS_DIR) return;
    for (vfs_node_t* c = dir->child; c; c = c->sibling) cb(c, ctx);
}

i32 vfs_chdir(const char* path) {
    vfs_node_t* n = vfs_resolve(path);
    if (!n || n->type != VFS_DIR) return -1;

    /* Rebuild absolute path */
    char parts[32][VFS_NAME_MAX];
    int depth = 0;
    for (vfs_node_t* cur = n; cur && cur != root; cur = cur->parent) {
        if (depth < 32) strncpy(parts[depth++], cur->name, VFS_NAME_MAX - 1);
    }
    strcpy(cwd, "/");
    for (int i = depth - 1; i >= 0; i--) {
        if (strlen(cwd) > 1) strcat(cwd, "/");
        strcat(cwd, parts[i]);
    }
    if (depth == 0) strcpy(cwd, "/");
    return 0;
}
