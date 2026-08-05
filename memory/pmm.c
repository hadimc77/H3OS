/**
 * H3OS — Physical Memory Manager
 *
 * Bitmap allocator over Multiboot2 memory map. Pages below 1 MiB and the
 * kernel image itself are reserved. Designed for x86_64 with 4 KiB pages.
 */
#include "pmm.h"
#include <h3os/kernel.h>
#include <h3os/string.h>

extern u8 _kernel_end[];

#define PMM_MAX_PAGES (512ULL * 1024) /* track up to 2 GiB with 4K pages */

static u8  page_bitmap[PMM_MAX_PAGES / 8];
static u64 total_pages = 0;
static u64 used_pages  = 0;
static u64 highest_page = 0;

static inline bool bitmap_test(u64 page) {
    return (page_bitmap[page / 8] & (1u << (page % 8))) != 0;
}

static inline void bitmap_set(u64 page) {
    page_bitmap[page / 8] |= (u8)(1u << (page % 8));
}

static inline void bitmap_clear(u64 page) {
    page_bitmap[page / 8] &= (u8)~(1u << (page % 8));
}

static void reserve_region(u64 addr, u64 len) {
    u64 start = addr / PAGE_SIZE;
    u64 end   = H3OS_ALIGN_UP(addr + len, PAGE_SIZE) / PAGE_SIZE;
    for (u64 p = start; p < end && p < PMM_MAX_PAGES; p++) {
        if (!bitmap_test(p)) {
            bitmap_set(p);
            used_pages++;
        }
    }
}

static mb2_tag_t* mb2_find(mb2_info_t* info, u32 type) {
    if (!info) return NULL;
    u8* ptr = (u8*)info + 8;
    u8* end = (u8*)info + info->total_size;
    while (ptr < end) {
        mb2_tag_t* tag = (mb2_tag_t*)ptr;
        if (tag->type == MB2_TAG_END) break;
        if (tag->type == type) return tag;
        ptr += (tag->size + 7) & ~7u;
    }
    return NULL;
}

void pmm_init(mb2_info_t* mb2) {
    memset(page_bitmap, 0xFF, sizeof(page_bitmap)); /* all reserved initially */
    used_pages = 0;
    total_pages = 0;
    highest_page = 0;

    mb2_tag_mmap_t* mmap = (mb2_tag_mmap_t*)mb2_find(mb2, MB2_TAG_MMAP);
    if (!mmap) {
        /* Fallback: assume 128 MiB available from 1 MiB */
        KLOG_WARN("pmm", "No multiboot mmap — using 128 MiB fallback");
        for (u64 p = 256; p < 32768 && p < PMM_MAX_PAGES; p++) {
            bitmap_clear(p);
        }
        total_pages = 32768;
        highest_page = 32768;
        used_pages = 256;
    } else {
        u32 entries = (mmap->size - 16) / mmap->entry_size;
        for (u32 i = 0; i < entries; i++) {
            mb2_mmap_entry_t* e = (mb2_mmap_entry_t*)((u8*)mmap->entries + i * mmap->entry_size);
            u64 end_page = H3OS_ALIGN_UP(e->addr + e->len, PAGE_SIZE) / PAGE_SIZE;
            if (end_page > highest_page) highest_page = end_page;
            if (end_page > total_pages) total_pages = end_page;

            if (e->type == MB2_MEMORY_AVAILABLE) {
                u64 start = H3OS_ALIGN_UP(e->addr, PAGE_SIZE) / PAGE_SIZE;
                u64 end   = H3OS_ALIGN_DOWN(e->addr + e->len, PAGE_SIZE) / PAGE_SIZE;
                for (u64 p = start; p < end && p < PMM_MAX_PAGES; p++) {
                    bitmap_clear(p);
                }
            }
        }
        if (total_pages > PMM_MAX_PAGES) total_pages = PMM_MAX_PAGES;

        /* Count free then mark used by recounting set bits after reserves */
        used_pages = 0;
        for (u64 p = 0; p < total_pages; p++) {
            if (bitmap_test(p)) used_pages++;
        }
    }

    /* Reserve low memory + kernel */
    reserve_region(0, 0x100000);
    reserve_region(0x100000, (u64)_kernel_end - 0x100000);
    /* Reserve bitmap itself if it sits in available RAM (it's in .bss = kernel) */

    pmm_stats_t st;
    pmm_get_stats(&st);
    KLOG_INFO("pmm", "Ready — total %llu MiB, free %llu MiB (%llu pages free)",
              (unsigned long long)(st.total_bytes / (1024 * 1024)),
              (unsigned long long)((st.free_pages * PAGE_SIZE) / (1024 * 1024)),
              (unsigned long long)st.free_pages);
}

phys_addr_t pmm_alloc_page(void) {
    for (u64 p = 0; p < total_pages; p++) {
        if (!bitmap_test(p)) {
            bitmap_set(p);
            used_pages++;
            return p * PAGE_SIZE;
        }
    }
    return 0;
}

phys_addr_t pmm_alloc_pages(size_t count) {
    if (count == 0) return 0;
    if (count == 1) return pmm_alloc_page();

    for (u64 p = 0; p + count <= total_pages; p++) {
        bool ok = true;
        for (size_t i = 0; i < count; i++) {
            if (bitmap_test(p + i)) { ok = false; break; }
        }
        if (ok) {
            for (size_t i = 0; i < count; i++) {
                bitmap_set(p + i);
                used_pages++;
            }
            return p * PAGE_SIZE;
        }
    }
    return 0;
}

void pmm_free_page(phys_addr_t addr) {
    u64 p = addr / PAGE_SIZE;
    if (p >= total_pages) return;
    if (bitmap_test(p)) {
        bitmap_clear(p);
        used_pages--;
    }
}

void pmm_free_pages(phys_addr_t addr, size_t count) {
    for (size_t i = 0; i < count; i++) {
        pmm_free_page(addr + i * PAGE_SIZE);
    }
}

void pmm_get_stats(pmm_stats_t* out) {
    out->total_pages = total_pages;
    out->used_pages  = used_pages;
    out->free_pages  = total_pages > used_pages ? total_pages - used_pages : 0;
    out->total_bytes = total_pages * PAGE_SIZE;
}
