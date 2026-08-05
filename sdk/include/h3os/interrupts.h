/**
 * H3OS — Interrupt / GDT / IDT interfaces
 */
#ifndef H3OS_INTERRUPTS_H
#define H3OS_INTERRUPTS_H

#include <h3os/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    u64 r15, r14, r13, r12, r11, r10, r9, r8;
    u64 rbp, rdi, rsi, rdx, rcx, rbx, rax;
    u64 int_no, err_code;
    u64 rip, cs, rflags, rsp, ss;
} interrupt_frame_t;

typedef void (*irq_handler_t)(interrupt_frame_t* frame);

void gdt_init(void);
void idt_init(void);
void pic_init(void);
void irq_install(u8 irq, irq_handler_t handler);
void irq_ack(u8 irq);

/* Implemented in isr.asm — table of 256 stub pointers */
extern void* isr_stub_table[];

#ifdef __cplusplus
}
#endif

#endif /* H3OS_INTERRUPTS_H */
