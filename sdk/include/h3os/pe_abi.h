/**
 * H3OS PE ABI — native .exe programs receive this table in the first argument.
 * Calling convention: System V AMD64 (RDI = api*).
 */
#ifndef H3OS_PE_ABI_H
#define H3OS_PE_ABI_H

#include <h3os/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define H3OS_PE_MAGIC 0x48335045ULL /* 'H3PE' */

typedef struct h3os_pe_api {
    u64 magic;
    void (*write)(const char* s);           /* print to active console */
    void (*writeln)(const char* s);
    i32  (*exec)(const char* path);         /* run another pe (nested) */
    u64  (*uptime_ms)(void);
    void (*exit)(i32 code);
} h3os_pe_api_t;

typedef i32 (*h3os_pe_entry_t)(h3os_pe_api_t* api);

#ifdef __cplusplus
}
#endif

#endif
