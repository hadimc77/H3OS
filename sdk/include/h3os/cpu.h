/**
 * H3OS — CPU detection (vendor, features, topology hints)
 */
#ifndef H3OS_CPU_H
#define H3OS_CPU_H

#include <h3os/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    char vendor[13];
    char brand[49];
    u32  family;
    u32  model;
    u32  stepping;
    u32  max_basic;
    u32  max_ext;

    bool has_fpu;
    bool has_sse;
    bool has_sse2;
    bool has_sse3;
    bool has_ssse3;
    bool has_sse41;
    bool has_sse42;
    bool has_avx;
    bool has_avx2;
    bool has_aes;
    bool has_rdrand;
    bool has_nx;
    bool has_huge_pages;
    bool has_tsc;
    bool has_apic;
    bool has_x2apic;
    bool has_vmx;       /* Intel VT-x */
    bool has_svm;       /* AMD-V */

    u32  logical_cpus;
    u32  core_count;    /* best-effort */
    u32  l1_cache_kb;
    u32  l2_cache_kb;
    u32  l3_cache_kb;
} cpu_info_t;

void cpu_detect(cpu_info_t* out);
void cpu_init(void);
const cpu_info_t* cpu_get_info(void);

#ifdef __cplusplus
}
#endif

#endif /* H3OS_CPU_H */
