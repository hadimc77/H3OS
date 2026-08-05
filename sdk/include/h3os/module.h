/**
 * H3OS — Kernel module loader interface
 */
#ifndef H3OS_MODULE_H
#define H3OS_MODULE_H

#include <h3os/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    const char* name;
    const char* version;
    i32 (*init)(void);
    void (*exit)(void);
} h3_module_t;

#define H3_MODULE_EXPORT(n, ver, init_fn, exit_fn) \
    h3_module_t h3_module_##n = { #n, ver, init_fn, exit_fn }

i32  module_register(h3_module_t* mod);
i32  module_unregister(const char* name);

#ifdef __cplusplus
}
#endif

#endif
