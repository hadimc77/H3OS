/**
 * H3OS — Kernel logging (serial + ring buffer)
 */
#include <h3os/kernel.h>
#include <h3os/string.h>
#include <stdarg.h>

#define LOG_RING_SIZE 8192

static char log_ring[LOG_RING_SIZE];
static size_t log_pos = 0;
static log_level_t min_level = LOG_DEBUG;

/* COM1 serial — always available under QEMU for early diagnostics */
#define COM1 0x3F8

static void serial_init(void) {
    outb(COM1 + 1, 0x00);
    outb(COM1 + 3, 0x80);
    outb(COM1 + 0, 0x03); /* 38400 baud */
    outb(COM1 + 1, 0x00);
    outb(COM1 + 3, 0x03);
    outb(COM1 + 2, 0xC7);
    outb(COM1 + 4, 0x0B);
}

static void serial_write(char c) {
    while ((inb(COM1 + 5) & 0x20) == 0) { /* wait THR empty */ }
    outb(COM1, (u8)c);
}

static void ring_putc(char c) {
    log_ring[log_pos % LOG_RING_SIZE] = c;
    log_pos++;
    serial_write(c);
}

static void ring_puts(const char* s) {
    while (*s) ring_putc(*s++);
}

static void kvprintf(const char* fmt, va_list ap) {
    char num[72];
    while (*fmt) {
        if (*fmt != '%') {
            ring_putc(*fmt++);
            continue;
        }
        fmt++;
        switch (*fmt++) {
            case 's': {
                const char* s = va_arg(ap, const char*);
                ring_puts(s ? s : "(null)");
                break;
            }
            case 'c':
                ring_putc((char)va_arg(ap, int));
                break;
            case 'd':
            case 'i':
                itoa(va_arg(ap, int), num, 10);
                ring_puts(num);
                break;
            case 'u':
                utoa(va_arg(ap, unsigned int), num, 10);
                ring_puts(num);
                break;
            case 'x':
                utoa(va_arg(ap, unsigned int), num, 16);
                ring_puts(num);
                break;
            case 'p': {
                ring_puts("0x");
                utoa((u64)(uintptr_t)va_arg(ap, void*), num, 16);
                ring_puts(num);
                break;
            }
            case 'l': {
                if (*fmt == 'l') {
                    fmt++;
                    if (*fmt == 'u') {
                        fmt++;
                        utoa(va_arg(ap, unsigned long long), num, 10);
                        ring_puts(num);
                    } else if (*fmt == 'x') {
                        fmt++;
                        utoa(va_arg(ap, unsigned long long), num, 16);
                        ring_puts(num);
                    } else if (*fmt == 'd') {
                        fmt++;
                        itoa(va_arg(ap, long long), num, 10);
                        ring_puts(num);
                    }
                }
                break;
            }
            case '%':
                ring_putc('%');
                break;
            default:
                ring_putc('%');
                ring_putc(*(fmt - 1));
                break;
        }
    }
}

void klog_init(void) {
    serial_init();
    log_pos = 0;
    memset(log_ring, 0, sizeof(log_ring));
}

void klog(log_level_t level, const char* subsystem, const char* fmt, ...) {
    static const char* names[] = {
        "TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"
    };
    if (level < min_level) return;

    ring_putc('[');
    ring_puts(names[level]);
    ring_puts("][");
    ring_puts(subsystem);
    ring_puts("] ");

    va_list ap;
    va_start(ap, fmt);
    kvprintf(fmt, ap);
    va_end(ap);
    ring_putc('\n');
}

void kpanic(const char* fmt, ...) {
    cpu_cli();
    ring_puts("\n!!!! H3OS KERNEL PANIC !!!!\n");
    va_list ap;
    va_start(ap, fmt);
    kvprintf(fmt, ap);
    va_end(ap);
    ring_putc('\n');
    for (;;) {
        cpu_halt();
    }
}
