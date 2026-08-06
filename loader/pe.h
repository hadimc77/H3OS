/**
 * H3OS PE32+ loader — run native H3OS .exe (x86_64) images
 */
#ifndef H3OS_PE_H
#define H3OS_PE_H

#include <h3os/types.h>
#include <h3os/pe_abi.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    bool valid;
    bool is_pe32plus;
    u16  machine;
    u32  entry_rva;
    u64  image_base;
    u32  size_of_image;
    u32  size_of_headers;
    u16  number_of_sections;
    const char* error;
} pe_info_t;

void pe_init(void);
bool pe_probe(const void* data, size_t size, pe_info_t* out);
i32  pe_run(const void* data, size_t size, const char* name);
void pe_set_console(void (*write_fn)(const char*), void (*writeln_fn)(const char*));
i32  pe_last_exit_code(void);

#ifdef __cplusplus
}
#endif

#endif
