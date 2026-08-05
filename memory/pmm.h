/**
 * H3OS — Physical Memory Manager (bitmap page allocator)
 */
#ifndef H3OS_PMM_H
#define H3OS_PMM_H

#include <h3os/types.h>
#include "../boot/multiboot2.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    u64 total_pages;
    u64 used_pages;
    u64 free_pages;
    u64 total_bytes;
} pmm_stats_t;

void        pmm_init(mb2_info_t* mb2);
phys_addr_t pmm_alloc_page(void);
phys_addr_t pmm_alloc_pages(size_t count);
void        pmm_free_page(phys_addr_t addr);
void        pmm_free_pages(phys_addr_t addr, size_t count);
void        pmm_get_stats(pmm_stats_t* out);

#ifdef __cplusplus
}
#endif

#endif /* H3OS_PMM_H */
