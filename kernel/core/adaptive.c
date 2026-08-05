/**
 * H3OS — Hardware-aware visual/performance profile selection
 */
#include <h3os/adaptive.h>
#include <h3os/cpu.h>
#include "../memory/pmm.h"
#include <h3os/kernel.h>

static perf_settings_t g_perf;

void adaptive_init(void) {
    pmm_stats_t mem;
    pmm_get_stats(&mem);
    const cpu_info_t* cpu = cpu_get_info();
    u64 mem_mb = mem.total_bytes / (1024 * 1024);

    if (mem_mb < 512 || cpu->logical_cpus <= 1 || !cpu->has_sse2) {
        g_perf.profile = PERF_LOW;
        g_perf.animations = false;
        g_perf.blur = false;
        g_perf.shadows = false;
        g_perf.transparency = false;
        g_perf.target_fps = 30;
        g_perf.max_windows = 4;
    } else if (mem_mb < 2048 || cpu->logical_cpus < 4) {
        g_perf.profile = PERF_BALANCED;
        g_perf.animations = true;
        g_perf.blur = false;
        g_perf.shadows = true;
        g_perf.transparency = true;
        g_perf.target_fps = 60;
        g_perf.max_windows = 12;
    } else {
        g_perf.profile = PERF_HIGH;
        g_perf.animations = true;
        g_perf.blur = true;
        g_perf.shadows = true;
        g_perf.transparency = true;
        g_perf.target_fps = 60;
        g_perf.max_windows = 32;
    }

    KLOG_INFO("adaptive", "Profile: %s (RAM=%lluMiB CPUs=%u)",
              adaptive_profile_name(),
              (unsigned long long)mem_mb, cpu->logical_cpus);
}

const perf_settings_t* adaptive_settings(void) { return &g_perf; }

const char* adaptive_profile_name(void) {
    switch (g_perf.profile) {
        case PERF_LOW: return "Low-End / Lightweight";
        case PERF_BALANCED: return "Balanced";
        case PERF_HIGH: return "High-End / Enhanced";
        default: return "Unknown";
    }
}
