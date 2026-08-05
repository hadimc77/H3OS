/**
 * H3OS — Programmable Interval Timer (8253/8254) driver
 */
#include "timer.h"
#include <h3os/interrupts.h>
#include <h3os/kernel.h>

static volatile u64 g_ticks = 0;
static u32 g_freq = 100;

static void timer_irq(interrupt_frame_t* frame) {
    H3OS_UNUSED(frame);
    g_ticks++;
}

void timer_init(u32 frequency_hz) {
    if (frequency_hz < 18) frequency_hz = 18;
    if (frequency_hz > 1000) frequency_hz = 1000;
    g_freq = frequency_hz;
    g_ticks = 0;

    u32 divisor = 1193182 / frequency_hz;
    outb(0x43, 0x36);
    outb(0x40, (u8)(divisor & 0xFF));
    outb(0x40, (u8)((divisor >> 8) & 0xFF));

    irq_install(0, timer_irq);
    KLOG_INFO("timer", "PIT armed at %u Hz", frequency_hz);
}

u64 timer_ticks(void) { return g_ticks; }

u64 timer_uptime_ms(void) {
    return (g_ticks * 1000ULL) / g_freq;
}

void timer_sleep_ms(u64 ms) {
    u64 target = timer_uptime_ms() + ms;
    while (timer_uptime_ms() < target) {
        cpu_pause();
    }
}
