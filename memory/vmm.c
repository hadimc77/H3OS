/**
 * H3OS — Virtual Memory Manager
 *
 * Operates on the identity-mapped 1 GiB established by the bootloader.
 * New mappings allocate page tables from PMM as needed.
 */
#include "vmm.h"
#include "pmm.h"
#include <h3os/kernel.h>
#include <h3os/string.h>

static u64* pml4 = NULL;

static u64* table_walk_alloc(u64* table, u64 index) {
    if (!(table[index] & PTE_PRESENT)) {
        phys_addr_t page = pmm_alloc_page();
        if (!page) return NULL;
        memset((void*)(uintptr_t)page, 0, PAGE_SIZE);
        table[index] = page | PTE_PRESENT | PTE_WRITE;
    }
    return (u64*)(uintptr_t)(table[index] & ~0xFFFULL);
}

void vmm_init(void) {
    pml4 = (u64*)(uintptr_t)read_cr3();
    KLOG_INFO("vmm", "Using boot PML4 at %p (identity map active)", (void*)pml4);
}

bool vmm_map_page(virt_addr_t virt, phys_addr_t phys, u64 flags) {
    u64 i4 = (virt >> 39) & 0x1FF;
    u64 i3 = (virt >> 30) & 0x1FF;
    u64 i2 = (virt >> 21) & 0x1FF;
    u64 i1 = (virt >> 12) & 0x1FF;

    u64* pdpt = table_walk_alloc(pml4, i4);
    if (!pdpt) return false;
    u64* pd = table_walk_alloc(pdpt, i3);
    if (!pd) return false;

    /* If a huge page is present, refuse split for now */
    if (pd[i2] & PTE_HUGE) return false;

    u64* pt = table_walk_alloc(pd, i2);
    if (!pt) return false;

    pt[i1] = (phys & ~0xFFFULL) | (flags & 0xFFFULL) | PTE_PRESENT;
    __asm__ volatile("invlpg (%0)" : : "r"(virt) : "memory");
    return true;
}

bool vmm_unmap_page(virt_addr_t virt) {
    u64 i4 = (virt >> 39) & 0x1FF;
    u64 i3 = (virt >> 30) & 0x1FF;
    u64 i2 = (virt >> 21) & 0x1FF;
    u64 i1 = (virt >> 12) & 0x1FF;

    if (!(pml4[i4] & PTE_PRESENT)) return false;
    u64* pdpt = (u64*)(uintptr_t)(pml4[i4] & ~0xFFFULL);
    if (!(pdpt[i3] & PTE_PRESENT)) return false;
    u64* pd = (u64*)(uintptr_t)(pdpt[i3] & ~0xFFFULL);
    if (pd[i2] & PTE_HUGE) return false;
    if (!(pd[i2] & PTE_PRESENT)) return false;
    u64* pt = (u64*)(uintptr_t)(pd[i2] & ~0xFFFULL);
    pt[i1] = 0;
    __asm__ volatile("invlpg (%0)" : : "r"(virt) : "memory");
    return true;
}

phys_addr_t vmm_resolve(virt_addr_t virt) {
    u64 i4 = (virt >> 39) & 0x1FF;
    u64 i3 = (virt >> 30) & 0x1FF;
    u64 i2 = (virt >> 21) & 0x1FF;
    u64 i1 = (virt >> 12) & 0x1FF;

    if (!(pml4[i4] & PTE_PRESENT)) return 0;
    u64* pdpt = (u64*)(uintptr_t)(pml4[i4] & ~0xFFFULL);
    if (!(pdpt[i3] & PTE_PRESENT)) return 0;
    u64* pd = (u64*)(uintptr_t)(pdpt[i3] & ~0xFFFULL);
    if (pd[i2] & PTE_HUGE) {
        return (pd[i2] & ~0x1FFFFFULL) + (virt & 0x1FFFFF);
    }
    if (!(pd[i2] & PTE_PRESENT)) return 0;
    u64* pt = (u64*)(uintptr_t)(pd[i2] & ~0xFFFULL);
    if (!(pt[i1] & PTE_PRESENT)) return 0;
    return (pt[i1] & ~0xFFFULL) + (virt & 0xFFF);
}
