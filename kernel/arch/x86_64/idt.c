/**
 * H3OS — IDT + PIC8259 interrupt controller
 */
#include <h3os/interrupts.h>
#include <h3os/kernel.h>
#include <h3os/string.h>

typedef struct {
    u16 offset_low;
    u16 selector;
    u8  ist;
    u8  type_attr;
    u16 offset_mid;
    u32 offset_high;
    u32 zero;
} __attribute__((packed)) idt_entry_t;

typedef struct {
    u16 limit;
    u64 base;
} __attribute__((packed)) idt_ptr_t;

static idt_entry_t idt[256];
static idt_ptr_t   idt_ptr;
static irq_handler_t irq_handlers[16];

extern void* isr_stub_table[];

static void idt_set(u8 vec, void* handler, u8 type_attr) {
    u64 addr = (u64)handler;
    idt[vec].offset_low  = (u16)(addr & 0xFFFF);
    idt[vec].selector    = 0x08; /* kernel code */
    idt[vec].ist         = 0;
    idt[vec].type_attr   = type_attr;
    idt[vec].offset_mid  = (u16)((addr >> 16) & 0xFFFF);
    idt[vec].offset_high = (u32)((addr >> 32) & 0xFFFFFFFF);
    idt[vec].zero       = 0;
}

void pic_init(void) {
    /* Remap PIC: IRQs 0-7 -> 32-39, IRQs 8-15 -> 40-47 */
    outb(0x20, 0x11);
    outb(0xA0, 0x11);
    outb(0x21, 0x20);
    outb(0xA1, 0x28);
    outb(0x21, 0x04);
    outb(0xA1, 0x02);
    outb(0x21, 0x01);
    outb(0xA1, 0x01);

    /* Mask all except timer (IRQ0) and keyboard (IRQ1) initially */
    outb(0x21, 0xFC);
    outb(0xA1, 0xFF);

    KLOG_INFO("pic", "PIC8259 remapped to vectors 32-47");
}

void irq_ack(u8 irq) {
    if (irq >= 8) outb(0xA0, 0x20);
    outb(0x20, 0x20);
}

void irq_install(u8 irq, irq_handler_t handler) {
    if (irq < 16) {
        irq_handlers[irq] = handler;
        /* Unmask */
        if (irq < 8) {
            outb(0x21, inb(0x21) & (u8)~(1 << irq));
        } else {
            outb(0xA1, inb(0xA1) & (u8)~(1 << (irq - 8)));
        }
    }
}

void idt_init(void) {
    memset(idt, 0, sizeof(idt));
    memset(irq_handlers, 0, sizeof(irq_handlers));

    for (int i = 0; i < 256; i++) {
        idt_set((u8)i, isr_stub_table[i], 0x8E); /* present, DPL0, interrupt gate */
    }

    idt_ptr.limit = sizeof(idt) - 1;
    idt_ptr.base  = (u64)&idt;
    __asm__ volatile("lidt %0" : : "m"(idt_ptr));

    pic_init();
    KLOG_INFO("idt", "IDT loaded (256 vectors)");
}

/* Called from assembly ISR stubs */
void isr_dispatch(interrupt_frame_t* frame) {
    if (frame->int_no < 32) {
        static const char* exceptions[] = {
            "Divide Error", "Debug", "NMI", "Breakpoint",
            "Overflow", "Bound Range", "Invalid Opcode", "Device Not Available",
            "Double Fault", "Coprocessor Segment", "Invalid TSS", "Segment Not Present",
            "Stack Fault", "General Protection", "Page Fault", "Reserved",
            "x87 FP", "Alignment Check", "Machine Check", "SIMD FP",
            "Virtualization", "Control Protection", "Reserved", "Reserved",
            "Reserved", "Reserved", "Reserved", "Reserved",
            "Hypervisor Injection", "VMM Communication", "Security", "Reserved"
        };
        const char* name = frame->int_no < 32 ? exceptions[frame->int_no] : "Unknown";

        if (frame->int_no == 14) {
            kpanic("Page Fault at rip=%p cr2=%p err=%llu (%s)",
                   (void*)frame->rip, (void*)read_cr2(),
                   (unsigned long long)frame->err_code, name);
        }
        kpanic("Exception %llu (%s) at rip=%p err=%llu",
               (unsigned long long)frame->int_no, name,
               (void*)frame->rip, (unsigned long long)frame->err_code);
    }

    if (frame->int_no >= 32 && frame->int_no < 48) {
        u8 irq = (u8)(frame->int_no - 32);
        if (irq_handlers[irq]) {
            irq_handlers[irq](frame);
        }
        irq_ack(irq);
    }
}
