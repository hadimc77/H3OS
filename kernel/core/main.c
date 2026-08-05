/**
 * H3OS Kernel Entry — hybrid kernel bring-up sequence
 *
 * Boot → CPU → Memory → Interrupts → Drivers → Scheduler →
 * Graphics → Desktop → Applications
 */
#include <h3os/kernel.h>
#include <h3os/version.h>
#include <h3os/cpu.h>
#include <h3os/interrupts.h>
#include <h3os/adaptive.h>
#include <h3os/sched.h>
#include <h3os/syscall.h>
#include <h3os/string.h>
#include <h3os/ipc.h>
#include <h3os/power.h>

#include "../boot/multiboot2.h"
#include "../memory/pmm.h"
#include "../memory/heap.h"
#include "../memory/vmm.h"
#include "../drivers/timer/timer.h"
#include "../drivers/keyboard/keyboard.h"
#include "../drivers/framebuffer/framebuffer.h"
#include "../drivers/rtc/rtc.h"
#include "../drivers/pci/pci.h"
#include "../filesystem/vfs/vfs.h"
#include "../window_manager/wm.h"
#include "../desktop/desktop.h"
#include "../applications/terminal/terminal.h"
#include "../network/net.h"
#include "../security/security.h"

/* Declared in other TUs */
void klog_init(void);
void cpu_init(void);

static volatile bool g_running = true;
static bool ctrl_down = false;

static void banner(void) {
    KLOG_INFO("boot", "======================================================");
    KLOG_INFO("boot", "  %s  v%s  (%s)", H3OS_NAME, H3OS_VERSION_STRING, H3OS_CODENAME);
    KLOG_INFO("boot", "  %s", H3OS_TAGLINE);
    KLOG_INFO("boot", "  Hybrid Kernel  |  x86_64  |  Multiboot2");
    KLOG_INFO("boot", "======================================================");
}

void kernel_idle(void) {
    while (g_running) {
        /* Drain keyboard into desktop / terminal */
        char c;
        while (keyboard_try_read(&c)) {
            if (c == 0x11 || c == ('t' & 0x1F)) { /* Ctrl+T-ish via raw — handle below */
            }

            /* Simple control sequences using ASCII */
            if (c == 20) { /* Ctrl+T */
                terminal_open();
                continue;
            }
            if (c == 12) { /* Ctrl+L */
                desktop_toggle_launcher();
                continue;
            }
            if (c == 4) { /* Ctrl+D theme toggle */
                static int th = 0;
                th = !th;
                desktop_set_theme(th ? THEME_LIGHT : THEME_DARK);
                continue;
            }

            wm_handle_key(c);
            H3OS_UNUSED(ctrl_down);
        }

        desktop_tick();
        sched_tick();
        desktop_render();

        /* Pace to adaptive target FPS using busy wait on PIT */
        const perf_settings_t* p = adaptive_settings();
        u64 frame_ms = p->target_fps ? (1000 / p->target_fps) : 16;
        u64 start = timer_uptime_ms();
        while (timer_uptime_ms() - start < frame_ms) {
            cpu_pause();
        }
    }

    for (;;) cpu_halt();
}

void kernel_main(u64 multiboot_info_addr) {
    mb2_info_t* mb2 = (mb2_info_t*)(uintptr_t)multiboot_info_addr;

    klog_init();
    banner();

    if (!mb2) {
        kpanic("Missing Multiboot2 information structure");
    }
    KLOG_INFO("boot", "Multiboot2 info at %p (%u bytes)",
              (void*)mb2, mb2->total_size);

    /* CPU */
    gdt_init();
    cpu_init();

    /* Memory */
    pmm_init(mb2);
    heap_init();
    vmm_init();

    /* Interrupts + time */
    idt_init();
    timer_init(100);
    rtc_init();

    /* Drivers */
    keyboard_init();
    pci_init();
    fb_init(mb2);

    /* Adaptive profile based on detected hardware */
    adaptive_init();

    /* Core services */
    security_init();
    vfs_init();
    net_init();
    ipc_init();
    power_init();
    sched_init();
    syscall_init();

    /* UI stack */
    wm_init();
    desktop_init();
    terminal_open();

    KLOG_INFO("boot", "H3OS is ready. Entering desktop event loop.");
    cpu_sti();
    kernel_idle();
}
