/**
 * H3OS — Public kernel API surface for modules and drivers
 */
#ifndef H3OS_KERNEL_H
#define H3OS_KERNEL_H

#include <h3os/types.h>
#include <h3os/version.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ---- Logging ---- */
typedef enum {
    LOG_TRACE = 0,
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR,
    LOG_FATAL
} log_level_t;

void klog(log_level_t level, const char* subsystem, const char* fmt, ...);
void kpanic(const char* fmt, ...) __attribute__((noreturn));

#define KLOG_INFO(sub, ...)  klog(LOG_INFO,  sub, __VA_ARGS__)
#define KLOG_WARN(sub, ...)  klog(LOG_WARN,  sub, __VA_ARGS__)
#define KLOG_ERROR(sub, ...) klog(LOG_ERROR, sub, __VA_ARGS__)
#define KLOG_DEBUG(sub, ...) klog(LOG_DEBUG, sub, __VA_ARGS__)

/* ---- I/O ports ---- */
static inline void outb(u16 port, u8 val) {
    __asm__ volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}
static inline u8 inb(u16 port) {
    u8 ret;
    __asm__ volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}
static inline void outw(u16 port, u16 val) {
    __asm__ volatile("outw %0, %1" : : "a"(val), "Nd"(port));
}
static inline u16 inw(u16 port) {
    u16 ret;
    __asm__ volatile("inw %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}
static inline void outl(u16 port, u32 val) {
    __asm__ volatile("outl %0, %1" : : "a"(val), "Nd"(port));
}
static inline u32 inl(u16 port) {
    u32 ret;
    __asm__ volatile("inl %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}
static inline void io_wait(void) {
    outb(0x80, 0);
}

/* ---- CPU helpers ---- */
static inline void cpu_halt(void) {
    __asm__ volatile("hlt");
}
static inline void cpu_cli(void) {
    __asm__ volatile("cli");
}
static inline void cpu_sti(void) {
    __asm__ volatile("sti");
}
static inline void cpu_pause(void) {
    __asm__ volatile("pause");
}
static inline u64 read_cr2(void) {
    u64 val;
    __asm__ volatile("mov %%cr2, %0" : "=r"(val));
    return val;
}
static inline u64 read_cr3(void) {
    u64 val;
    __asm__ volatile("mov %%cr3, %0" : "=r"(val));
    return val;
}
static inline void write_cr3(u64 val) {
    __asm__ volatile("mov %0, %%cr3" : : "r"(val) : "memory");
}
static inline u64 rdtsc(void) {
    u32 lo, hi;
    __asm__ volatile("rdtsc" : "=a"(lo), "=d"(hi));
    return ((u64)hi << 32) | lo;
}

/* ---- Kernel lifecycle ---- */
void kernel_main(u64 multiboot_info_addr);
void kernel_idle(void) __attribute__((noreturn));

#ifdef __cplusplus
}
#endif

#endif /* H3OS_KERNEL_H */
