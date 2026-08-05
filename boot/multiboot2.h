/**
 * H3OS — Multiboot2 structures (subset used by the kernel)
 * Spec: https://www.gnu.org/software/grub/manual/multiboot2/
 */
#ifndef H3OS_MULTIBOOT2_H
#define H3OS_MULTIBOOT2_H

#include <h3os/types.h>

#define MULTIBOOT2_BOOTLOADER_MAGIC 0x36D76289

#define MB2_TAG_END              0
#define MB2_TAG_CMDLINE          1
#define MB2_TAG_BOOT_LOADER_NAME 2
#define MB2_TAG_MODULE           3
#define MB2_TAG_BASIC_MEMINFO    4
#define MB2_TAG_BOOTDEV          5
#define MB2_TAG_MMAP             6
#define MB2_TAG_FRAMEBUFFER      8
#define MB2_TAG_ELF_SECTIONS     9
#define MB2_TAG_APM              10
#define MB2_TAG_EFI32            11
#define MB2_TAG_EFI64            12
#define MB2_TAG_SMBIOS           13
#define MB2_TAG_ACPI_OLD         14
#define MB2_TAG_ACPI_NEW         15

#define MB2_MEMORY_AVAILABLE        1
#define MB2_MEMORY_RESERVED         2
#define MB2_MEMORY_ACPI_RECLAIMABLE 3
#define MB2_MEMORY_NVS              4
#define MB2_MEMORY_BADRAM           5

typedef struct {
    u32 type;
    u32 size;
} mb2_tag_t;

typedef struct {
    u32 type;
    u32 size;
    u32 mem_lower;
    u32 mem_upper;
} mb2_tag_basic_meminfo_t;

typedef struct {
    u64 addr;
    u64 len;
    u32 type;
    u32 zero;
} mb2_mmap_entry_t;

typedef struct {
    u32 type;
    u32 size;
    u32 entry_size;
    u32 entry_version;
    mb2_mmap_entry_t entries[];
} mb2_tag_mmap_t;

typedef struct {
    u32 type;
    u32 size;
    u64 addr;
    u32 pitch;
    u32 width;
    u32 height;
    u8  bpp;
    u8  framebuffer_type;
    u16 reserved;
} mb2_tag_framebuffer_t;

typedef struct {
    u32 total_size;
    u32 reserved;
} mb2_info_t;

#endif /* H3OS_MULTIBOOT2_H */
