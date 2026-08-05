/**
 * H3OS — Virtual memory / paging helpers
 */
#ifndef H3OS_VMM_H
#define H3OS_VMM_H

#include <h3os/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PTE_PRESENT   (1ULL << 0)
#define PTE_WRITE     (1ULL << 1)
#define PTE_USER      (1ULL << 2)
#define PTE_PWT       (1ULL << 3)
#define PTE_PCD       (1ULL << 4)
#define PTE_ACCESSED  (1ULL << 5)
#define PTE_DIRTY     (1ULL << 6)
#define PTE_HUGE      (1ULL << 7)
#define PTE_GLOBAL    (1ULL << 8)
#define PTE_NX        (1ULL << 63)

void vmm_init(void);
bool vmm_map_page(virt_addr_t virt, phys_addr_t phys, u64 flags);
bool vmm_unmap_page(virt_addr_t virt);
phys_addr_t vmm_resolve(virt_addr_t virt);

#ifdef __cplusplus
}
#endif

#endif /* H3OS_VMM_H */
