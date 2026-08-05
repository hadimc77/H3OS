/**
 * H3OS — CPUID-based hardware feature detection
 */
#include <h3os/cpu.h>
#include <h3os/string.h>
#include <h3os/kernel.h>

static cpu_info_t g_cpu;

static void cpuid(u32 leaf, u32* a, u32* b, u32* c, u32* d) {
    __asm__ volatile("cpuid"
                     : "=a"(*a), "=b"(*b), "=c"(*c), "=d"(*d)
                     : "a"(leaf), "c"(0));
}

static void cpuid_ex(u32 leaf, u32 sub, u32* a, u32* b, u32* c, u32* d) {
    __asm__ volatile("cpuid"
                     : "=a"(*a), "=b"(*b), "=c"(*c), "=d"(*d)
                     : "a"(leaf), "c"(sub));
}

void cpu_detect(cpu_info_t* out) {
    u32 a, b, c, d;
    memset(out, 0, sizeof(*out));

    cpuid(0, &a, &b, &c, &d);
    out->max_basic = a;
    *(u32*)&out->vendor[0] = b;
    *(u32*)&out->vendor[4] = d;
    *(u32*)&out->vendor[8] = c;
    out->vendor[12] = '\0';

    if (out->max_basic >= 1) {
        cpuid(1, &a, &b, &c, &d);
        out->stepping = a & 0xF;
        out->model    = (a >> 4) & 0xF;
        out->family   = (a >> 8) & 0xF;
        if (out->family == 0xF) {
            out->family += (a >> 20) & 0xFF;
            out->model  += ((a >> 16) & 0xF) << 4;
        }

        out->has_fpu   = (d & (1u << 0)) != 0;
        out->has_tsc   = (d & (1u << 4)) != 0;
        out->has_apic  = (d & (1u << 9)) != 0;
        out->has_sse   = (d & (1u << 25)) != 0;
        out->has_sse2  = (d & (1u << 26)) != 0;
        out->has_sse3  = (c & (1u << 0)) != 0;
        out->has_ssse3 = (c & (1u << 9)) != 0;
        out->has_sse41 = (c & (1u << 19)) != 0;
        out->has_sse42 = (c & (1u << 20)) != 0;
        out->has_aes   = (c & (1u << 25)) != 0;
        out->has_avx   = (c & (1u << 28)) != 0;
        out->has_rdrand= (c & (1u << 30)) != 0;
        out->has_x2apic= (c & (1u << 21)) != 0;
        out->has_vmx   = (c & (1u << 5)) != 0;

        out->logical_cpus = (b >> 16) & 0xFF;
        if (out->logical_cpus == 0) out->logical_cpus = 1;
        out->core_count = out->logical_cpus; /* refined below if possible */
    }

    if (out->max_basic >= 7) {
        cpuid_ex(7, 0, &a, &b, &c, &d);
        out->has_avx2 = (b & (1u << 5)) != 0;
    }

    cpuid(0x80000000u, &a, &b, &c, &d);
    out->max_ext = a;

    if (out->max_ext >= 0x80000001u) {
        cpuid(0x80000001u, &a, &b, &c, &d);
        out->has_nx         = (d & (1u << 20)) != 0;
        out->has_huge_pages = (d & (1u << 26)) != 0;
        out->has_svm        = (c & (1u << 2)) != 0;
    }

    if (out->max_ext >= 0x80000004u) {
        u32* brand = (u32*)out->brand;
        cpuid(0x80000002u, &brand[0], &brand[1], &brand[2], &brand[3]);
        cpuid(0x80000003u, &brand[4], &brand[5], &brand[6], &brand[7]);
        cpuid(0x80000004u, &brand[8], &brand[9], &brand[10], &brand[11]);
        out->brand[48] = '\0';
    } else {
        strcpy(out->brand, "Unknown CPU");
    }

    /* Cache sizes — Intel leaf 0x4 / AMD 0x80000006 (best effort) */
    if (out->max_ext >= 0x80000006u) {
        cpuid(0x80000006u, &a, &b, &c, &d);
        out->l2_cache_kb = (c >> 16) & 0xFFFF;
        out->l3_cache_kb = ((d >> 18) & 0x3FFF) * 512; /* AMD encoding */
    }
}

const cpu_info_t* cpu_get_info(void) {
    return &g_cpu;
}

void cpu_init(void) {
    cpu_detect(&g_cpu);
    KLOG_INFO("cpu", "Vendor: %s", g_cpu.vendor);
    KLOG_INFO("cpu", "Brand:  %s", g_cpu.brand);
    KLOG_INFO("cpu", "Family %u Model %u Stepping %u | Logical CPUs: %u",
              g_cpu.family, g_cpu.model, g_cpu.stepping, g_cpu.logical_cpus);
    KLOG_INFO("cpu", "Features: SSE2=%d AVX=%d NX=%d HugePages=%d VT=%d",
              g_cpu.has_sse2, g_cpu.has_avx, g_cpu.has_nx,
              g_cpu.has_huge_pages, g_cpu.has_vmx || g_cpu.has_svm);
}
