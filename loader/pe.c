/**
 * H3OS PE32+ loader
 *
 * Loads x86_64 PE executables built for the H3OS PE ABI (not full Win32).
 * Relocations are applied when the preferred ImageBase is unavailable.
 */
#include "pe.h"
#include "../memory/heap.h"
#include "../memory/pmm.h"
#include <h3os/kernel.h>
#include <h3os/string.h>
#include "../drivers/timer/timer.h"

#pragma pack(push, 1)
typedef struct {
    u16 e_magic;
    u16 e_cblp, e_cp, e_crlc, e_cparhdr, e_minalloc, e_maxalloc, e_ss, e_sp;
    u16 e_csum, e_ip, e_cs, e_lfarlc, e_ovno;
    u16 e_res[4];
    u16 e_oemid, e_oeminfo;
    u16 e_res2[10];
    u32 e_lfanew;
} dos_header_t;

typedef struct {
    u32 signature;
    u16 machine;
    u16 number_of_sections;
    u32 time_date_stamp;
    u32 pointer_to_symbol_table;
    u32 number_of_symbols;
    u16 size_of_optional_header;
    u16 characteristics;
} coff_header_t;

typedef struct {
    u16 magic;
    u8  major_linker, minor_linker;
    u32 size_of_code, size_of_initialized_data, size_of_uninitialized_data;
    u32 address_of_entry_point;
    u32 base_of_code;
    u64 image_base;
    u32 section_alignment;
    u32 file_alignment;
    u16 major_os, minor_os, major_image, minor_image, major_subsys, minor_subsys;
    u32 win32_version;
    u32 size_of_image;
    u32 size_of_headers;
    u32 check_sum;
    u16 subsystem;
    u16 dll_characteristics;
    u64 size_of_stack_reserve, size_of_stack_commit;
    u64 size_of_heap_reserve, size_of_heap_commit;
    u32 loader_flags;
    u32 number_of_rva_and_sizes;
} pe32plus_optional_t;

typedef struct {
    char name[8];
    u32 virtual_size;
    u32 virtual_address;
    u32 size_of_raw_data;
    u32 pointer_to_raw_data;
    u32 pointer_to_relocations;
    u32 pointer_to_linenumbers;
    u16 number_of_relocations;
    u16 number_of_linenumbers;
    u32 characteristics;
} section_header_t;

typedef struct {
    u32 virtual_address;
    u32 size;
} data_dir_t;
#pragma pack(pop)

#define IMAGE_FILE_MACHINE_AMD64 0x8664
#define IMAGE_SCN_MEM_EXECUTE    0x20000000
#define IMAGE_REL_BASED_DIR64    10
#define IMAGE_REL_BASED_HIGHLOW  3
#define IMAGE_REL_BASED_ABSOLUTE 0

static void (*g_write)(const char*) = NULL;
static void (*g_writeln)(const char*) = NULL;
static i32 g_exit_code = 0;
static bool g_exited = false;

static void default_write(const char* s) {
    KLOG_INFO("pe", "%s", s ? s : "");
}

static void api_write(const char* s) {
    if (g_write) g_write(s);
    else default_write(s);
}

static void api_writeln(const char* s) {
    if (g_writeln) g_writeln(s);
    else {
        api_write(s);
        api_write("\n");
    }
}

static void api_exit(i32 code) {
    g_exit_code = code;
    g_exited = true;
}

static u64 api_uptime(void) {
    return timer_uptime_ms();
}

static i32 api_exec(const char* path) {
    H3OS_UNUSED(path);
    return -1; /* filled later via pe_run_path if needed */
}

static h3os_pe_api_t g_api;

void pe_init(void) {
    memset(&g_api, 0, sizeof(g_api));
    g_api.magic = H3OS_PE_MAGIC;
    g_api.write = api_write;
    g_api.writeln = api_writeln;
    g_api.exec = api_exec;
    g_api.uptime_ms = api_uptime;
    g_api.exit = api_exit;
    KLOG_INFO("pe", "PE32+ loader ready (H3OS native EXE ABI)");
}

void pe_set_console(void (*write_fn)(const char*), void (*writeln_fn)(const char*)) {
    g_write = write_fn;
    g_writeln = writeln_fn;
}

i32 pe_last_exit_code(void) { return g_exit_code; }

bool pe_probe(const void* data, size_t size, pe_info_t* out) {
    memset(out, 0, sizeof(*out));
    if (!data || size < sizeof(dos_header_t)) {
        out->error = "too small";
        return false;
    }
    const dos_header_t* dos = (const dos_header_t*)data;
    if (dos->e_magic != 0x5A4D) {
        out->error = "not MZ";
        return false;
    }
    if (dos->e_lfanew == 0 || (size_t)dos->e_lfanew + sizeof(coff_header_t) + 4 > size) {
        out->error = "bad e_lfanew";
        return false;
    }
    const u8* p = (const u8*)data + dos->e_lfanew;
    if (p[0] != 'P' || p[1] != 'E' || p[2] || p[3]) {
        out->error = "no PE signature";
        return false;
    }
    const coff_header_t* coff = (const coff_header_t*)(p);
    /* signature is first 4 bytes then coff fields — our struct includes signature */
    if (coff->signature != 0x00004550) {
        out->error = "bad PE sig";
        return false;
    }
    if (coff->machine != IMAGE_FILE_MACHINE_AMD64) {
        out->error = "not AMD64";
        return false;
    }
    if (coff->size_of_optional_header < sizeof(pe32plus_optional_t)) {
        out->error = "optional header too small";
        return false;
    }
    const pe32plus_optional_t* opt =
        (const pe32plus_optional_t*)((const u8*)coff + sizeof(coff_header_t));
    if (opt->magic != 0x20B) {
        out->error = "not PE32+";
        return false;
    }

    out->valid = true;
    out->is_pe32plus = true;
    out->machine = coff->machine;
    out->entry_rva = opt->address_of_entry_point;
    out->image_base = opt->image_base;
    out->size_of_image = opt->size_of_image;
    out->size_of_headers = opt->size_of_headers;
    out->number_of_sections = coff->number_of_sections;
    out->error = NULL;
    return true;
}

static void apply_relocations(u8* image, u64 load_base, u64 preferred,
                              const u8* file, size_t file_size,
                              const pe32plus_optional_t* opt, const coff_header_t* coff) {
    if (load_base == preferred) return;
    if (opt->number_of_rva_and_sizes < 6) return;

    const data_dir_t* dirs = (const data_dir_t*)((const u8*)opt + 112);
    H3OS_UNUSED(file);
    H3OS_UNUSED(file_size);
    H3OS_UNUSED(coff);

    const data_dir_t* reloc_dir = &dirs[5];
    if (reloc_dir->virtual_address == 0 || reloc_dir->size == 0) return;

    i64 delta = (i64)load_base - (i64)preferred;
    u32 pos = 0;
    while (pos + 8 <= reloc_dir->size) {
        u32 page_rva = *(u32*)(image + reloc_dir->virtual_address + pos);
        u32 block_size = *(u32*)(image + reloc_dir->virtual_address + pos + 4);
        if (block_size < 8) break;
        u32 count = (block_size - 8) / 2;
        u16* entries = (u16*)(image + reloc_dir->virtual_address + pos + 8);
        for (u32 i = 0; i < count; i++) {
            u16 e = entries[i];
            u16 type = e >> 12;
            u16 off = e & 0xFFF;
            u8* target = image + page_rva + off;
            if (type == IMAGE_REL_BASED_DIR64) {
                u64 val = *(u64*)target;
                *(u64*)target = (u64)((i64)val + delta);
            } else if (type == IMAGE_REL_BASED_HIGHLOW) {
                u32 val = *(u32*)target;
                *(u32*)target = (u32)((i32)val + (i32)delta);
            } else if (type == IMAGE_REL_BASED_ABSOLUTE) {
                /* skip */
            }
        }
        pos += block_size;
    }
}

i32 pe_run(const void* data, size_t size, const char* name) {
    pe_info_t info;
    if (!pe_probe(data, size, &info)) {
        KLOG_ERROR("pe", "reject %s: %s", name ? name : "exe", info.error ? info.error : "?");
        return -1;
    }

    const dos_header_t* dos = (const dos_header_t*)data;
    const coff_header_t* coff = (const coff_header_t*)((const u8*)data + dos->e_lfanew);
    const pe32plus_optional_t* opt =
        (const pe32plus_optional_t*)((const u8*)coff + sizeof(coff_header_t));
    const section_header_t* secs =
        (const section_header_t*)((const u8*)opt + coff->size_of_optional_header);

    size_t img_pages = H3OS_ALIGN_UP(info.size_of_image, PAGE_SIZE) / PAGE_SIZE;
    if (img_pages == 0) return -1;
    phys_addr_t phys = pmm_alloc_pages(img_pages);
    if (!phys) {
        /* fallback heap */
        u8* heap_img = (u8*)kmalloc(info.size_of_image);
        if (!heap_img) return -1;
        phys = (phys_addr_t)(uintptr_t)heap_img;
    }
    u8* image = (u8*)(uintptr_t)phys;
    memset(image, 0, info.size_of_image);

    size_t hdr_copy = info.size_of_headers;
    if (hdr_copy > size) hdr_copy = size;
    if (hdr_copy > info.size_of_image) hdr_copy = info.size_of_image;
    memcpy(image, data, hdr_copy);

    for (u16 i = 0; i < info.number_of_sections; i++) {
        const section_header_t* s = &secs[i];
        if (s->size_of_raw_data == 0 || s->pointer_to_raw_data == 0) continue;
        if ((size_t)s->pointer_to_raw_data + s->size_of_raw_data > size) continue;
        if ((size_t)s->virtual_address + s->size_of_raw_data > info.size_of_image) continue;
        memcpy(image + s->virtual_address,
               (const u8*)data + s->pointer_to_raw_data,
               s->size_of_raw_data);
    }

    apply_relocations(image, (u64)(uintptr_t)image, info.image_base,
                      (const u8*)data, size, opt, coff);

    if (info.entry_rva >= info.size_of_image) {
        KLOG_ERROR("pe", "bad entry RVA");
        return -1;
    }

    h3os_pe_entry_t entry = (h3os_pe_entry_t)(void*)(image + info.entry_rva);
    g_exited = false;
    g_exit_code = 0;

    KLOG_INFO("pe", "Running %s entry=%p size=%u",
              name ? name : "exe", (void*)entry, info.size_of_image);

    i32 rc = entry(&g_api);
    if (g_exited) rc = g_exit_code;

    KLOG_INFO("pe", "%s exited with %d", name ? name : "exe", rc);
    return rc;
}
