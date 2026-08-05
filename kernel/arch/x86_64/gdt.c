/**
 * H3OS — Runtime GDT (kernel + user segments) for long mode
 */
#include <h3os/interrupts.h>
#include <h3os/kernel.h>
#include <h3os/string.h>

typedef struct {
    u16 limit_low;
    u16 base_low;
    u8  base_mid;
    u8  access;
    u8  granularity;
    u8  base_high;
} __attribute__((packed)) gdt_entry_t;

typedef struct {
    u16 limit;
    u64 base;
} __attribute__((packed)) gdt_ptr_t;

static gdt_entry_t gdt[5];
static gdt_ptr_t   gdt_ptr;

static void gdt_set(int i, u32 base, u32 limit, u8 access, u8 gran) {
    gdt[i].base_low    = (u16)(base & 0xFFFF);
    gdt[i].base_mid    = (u8)((base >> 16) & 0xFF);
    gdt[i].base_high   = (u8)((base >> 24) & 0xFF);
    gdt[i].limit_low   = (u16)(limit & 0xFFFF);
    gdt[i].granularity = (u8)((limit >> 16) & 0x0F);
    gdt[i].granularity |= gran & 0xF0;
    gdt[i].access      = access;
}

extern void gdt_flush(u64 ptr); /* asm */

void gdt_init(void) {
    memset(gdt, 0, sizeof(gdt));

    gdt_set(0, 0, 0, 0, 0);                /* null */
    gdt_set(1, 0, 0, 0x9A, 0x20);          /* kernel code 64-bit */
    gdt_set(2, 0, 0, 0x92, 0x00);          /* kernel data */
    gdt_set(3, 0, 0, 0xFA, 0x20);          /* user code 64-bit */
    gdt_set(4, 0, 0, 0xF2, 0x00);          /* user data */

    gdt_ptr.limit = sizeof(gdt) - 1;
    gdt_ptr.base  = (u64)&gdt;

    __asm__ volatile("lgdt %0" : : "m"(gdt_ptr));

    /* Reload data segments; code segment via far return */
    __asm__ volatile(
        "mov $0x10, %%ax\n"
        "mov %%ax, %%ds\n"
        "mov %%ax, %%es\n"
        "mov %%ax, %%fs\n"
        "mov %%ax, %%gs\n"
        "mov %%ax, %%ss\n"
        "pushq $0x08\n"
        "leaq 1f(%%rip), %%rax\n"
        "pushq %%rax\n"
        "lretq\n"
        "1:\n"
        ::: "rax", "memory"
    );

    KLOG_INFO("gdt", "GDT loaded (5 entries)");
}
