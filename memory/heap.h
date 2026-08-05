/**
 * H3OS — Kernel heap allocator (first-fit with coalescing)
 */
#ifndef H3OS_HEAP_H
#define H3OS_HEAP_H

#include <h3os/types.h>

#ifdef __cplusplus
extern "C" {
#endif

void  heap_init(void);
void* kmalloc(size_t size);
void* kcalloc(size_t n, size_t size);
void* krealloc(void* ptr, size_t size);
void  kfree(void* ptr);

typedef struct {
    u64 total_bytes;
    u64 used_bytes;
    u64 free_bytes;
    u64 allocations;
} heap_stats_t;

void heap_get_stats(heap_stats_t* out);

#ifdef __cplusplus
}
#endif

#endif /* H3OS_HEAP_H */
