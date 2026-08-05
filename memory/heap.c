/**
 * H3OS — Kernel heap
 *
 * Early heap backed by a static arena expanded via PMM when needed.
 * Block header stores size + free flag; adjacent free blocks coalesce.
 */
#include "heap.h"
#include "pmm.h"
#include <h3os/kernel.h>
#include <h3os/string.h>

#define HEAP_MAGIC   0x4833414C4C4F43ULL  /* H3ALLOC */

#define HEAP_ALIGN   16
#define HEAP_MIN     32
#define EARLY_HEAP   (512 * 1024)

typedef struct block {
    u64 magic;
    u64 size;          /* payload size */
    bool free;
    struct block* next;
    struct block* prev;
} block_t;

static u8 early_arena[EARLY_HEAP] __attribute__((aligned(16)));
static block_t* heap_head = NULL;
static u64 heap_used = 0;
static u64 heap_total = 0;
static u64 heap_allocs = 0;

static void heap_add_region(void* addr, size_t size) {
    if (size < sizeof(block_t) + HEAP_MIN) return;
    block_t* b = (block_t*)addr;
    b->magic = HEAP_MAGIC;
    b->size  = size - sizeof(block_t);
    b->free  = true;
    b->next  = heap_head;
    b->prev  = NULL;
    if (heap_head) heap_head->prev = b;
    heap_head = b;
    heap_total += size;
}

void heap_init(void) {
    heap_head = NULL;
    heap_used = 0;
    heap_total = 0;
    heap_allocs = 0;
    heap_add_region(early_arena, EARLY_HEAP);
    KLOG_INFO("heap", "Early heap ready (%u KiB)", EARLY_HEAP / 1024);
}

static void coalesce(block_t* b) {
    if (b->next && b->next->free) {
        b->size += sizeof(block_t) + b->next->size;
        b->next = b->next->next;
        if (b->next) b->next->prev = b;
    }
    if (b->prev && b->prev->free) {
        b->prev->size += sizeof(block_t) + b->size;
        b->prev->next = b->next;
        if (b->next) b->next->prev = b->prev;
    }
}

static bool expand_heap(size_t need) {
    size_t pages = H3OS_ALIGN_UP(need + sizeof(block_t) + PAGE_SIZE, PAGE_SIZE) / PAGE_SIZE;
    phys_addr_t phys = pmm_alloc_pages(pages);
    if (!phys) return false;
    /* Identity-mapped in early boot */
    heap_add_region((void*)(uintptr_t)phys, pages * PAGE_SIZE);
    return true;
}

void* kmalloc(size_t size) {
    if (size == 0) return NULL;
    size = H3OS_ALIGN_UP(size, HEAP_ALIGN);
    if (size < HEAP_MIN) size = HEAP_MIN;

    for (int attempt = 0; attempt < 2; attempt++) {
        for (block_t* b = heap_head; b; b = b->next) {
            if (b->magic != HEAP_MAGIC) {
                kpanic("Heap corruption detected");
            }
            if (b->free && b->size >= size) {
                /* Split if leftover is useful */
                if (b->size >= size + sizeof(block_t) + HEAP_MIN) {
                    block_t* n = (block_t*)((u8*)b + sizeof(block_t) + size);
                    n->magic = HEAP_MAGIC;
                    n->size  = b->size - size - sizeof(block_t);
                    n->free  = true;
                    n->next  = b->next;
                    n->prev  = b;
                    if (b->next) b->next->prev = n;
                    b->next  = n;
                    b->size  = size;
                }
                b->free = false;
                heap_used += b->size;
                heap_allocs++;
                return (void*)(b + 1);
            }
        }
        if (!expand_heap(size)) break;
    }
    KLOG_ERROR("heap", "Out of memory requesting %llu bytes", (unsigned long long)size);
    return NULL;
}

void* kcalloc(size_t n, size_t size) {
    size_t total = n * size;
    void* p = kmalloc(total);
    if (p) memset(p, 0, total);
    return p;
}

void kfree(void* ptr) {
    if (!ptr) return;
    block_t* b = ((block_t*)ptr) - 1;
    if (b->magic != HEAP_MAGIC || b->free) {
        KLOG_ERROR("heap", "Invalid free at %p", ptr);
        return;
    }
    b->free = true;
    heap_used -= b->size;
    coalesce(b);
}

void* krealloc(void* ptr, size_t size) {
    if (!ptr) return kmalloc(size);
    if (size == 0) { kfree(ptr); return NULL; }
    block_t* b = ((block_t*)ptr) - 1;
    if (b->size >= size) return ptr;
    void* n = kmalloc(size);
    if (!n) return NULL;
    memcpy(n, ptr, b->size);
    kfree(ptr);
    return n;
}

void heap_get_stats(heap_stats_t* out) {
    out->total_bytes = heap_total;
    out->used_bytes  = heap_used;
    out->free_bytes  = heap_total > heap_used ? heap_total - heap_used : 0;
    out->allocations = heap_allocs;
}
