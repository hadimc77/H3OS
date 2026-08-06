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
#include "../drivers/mouse/mouse.h"
#include "../drivers/framebuffer/framebuffer.h"
#include "../drivers/rtc/rtc.h"
#include "../drivers/pci/pci.h"
#include "../filesystem/vfs/vfs.h"
#include "../window_manager/wm.h"
#include "../desktop/desktop.h"
#include "../applications/terminal/terminal.h"
#include "../network/net.h"
#include "../security/security.h"

void klog_init(void);
void cpu_init(void);

static volatile bool g_running = true;

static void banner(void) {
    KLOG_INFO("boot", "======================================================");
    KLOG_INFO("boot", "  %s  v%s  (%s)", H3OS_NAME, H3OS_VERSION_STRING, H3OS_CODENAME);
    KLOG_INFO("boot", "  %s", H3OS_TAGLINE);
    KLOG_INFO("boot", "  Hybrid Kernel  |  x86_64  |  Multiboot2");
    KLOG_INFO("boot", "======================================================");
}

void kernel_idle(void) {
    while (g_running) {
        char c;
        while (keyboard_try_read(&c)) {
            if (c == 20) { /* Ctrl+T */
                desktop_launch(APP_TERMINAL);
                continue;
            }
            if (c == 12) { /* Ctrl+L / Win-like Start */
                desktop_toggle_launcher();
                continue;
            }
            if (c == 14) { /* Ctrl+N notification center */
                /* reuse action via toggle launcher path — open snap */
                desktop_toggle_snap();
                continue;
            }
            if (c == 6) { /* Ctrl+F files */
                desktop_launch(APP_FILES);
                continue;
            }
            if (c == 19) { /* Ctrl+S settings */
                desktop_launch(APP_SETTINGS);
                continue;
            }
            if (c == 4) { /* Ctrl+D theme */
                static int th = 0;
                th = !th;
                desktop_set_theme(th ? THEME_LIGHT : THEME_DARK);
                continue;
            }
            wm_handle_key(c);
        }

        /* Mouse: desktop chrome first, then window manager */
        desktop_handle_mouse();
        if (!desktop_mouse_click_consumed()) {
            wm_handle_mouse();
        }

        desktop_tick();
        sched_tick();
        desktop_render();
        mouse_poll_frame();

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

    gdt_init();
    cpu_init();

    pmm_init(mb2);
    heap_init();
    vmm_init();

    idt_init();
    timer_init(100);
    rtc_init();

    keyboard_init();
    mouse_init();
    pci_init();
    fb_init(mb2);

    adaptive_init();

    security_init();
    vfs_init();
    net_init();
    ipc_init();
    power_init();
    sched_init();
    syscall_init();

    wm_init();
    desktop_init();
    terminal_open();

    KLOG_INFO("boot", "H3OS is ready. Entering desktop event loop.");
    cpu_sti();
    kernel_idle();
}
