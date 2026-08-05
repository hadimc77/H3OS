;==============================================================================
; H3OS Boot Entry — Multiboot2 header + 64-bit long-mode trampoline
; Architecture: x86_64 | Assembler: NASM
;==============================================================================

BITS 32

SECTION .multiboot
align 8

; Multiboot2 header
multiboot_header_start:
    dd 0xE85250D6                ; magic
    dd 0                         ; architecture (i386)
    dd multiboot_header_end - multiboot_header_start
    dd -(0xE85250D6 + 0 + (multiboot_header_end - multiboot_header_start))

    ; Framebuffer tag — request graphics mode
    align 8
    dw 5                         ; type = framebuffer
    dw 1                         ; optional
    dd 20                        ; size
    dd 1280                      ; width
    dd 720                       ; height
    dd 32                        ; depth

    ; End tag
    align 8
    dw 0
    dw 0
    dd 8
multiboot_header_end:

SECTION .bss
align 16
stack_bottom:
    resb 16384                   ; 16 KiB bootstrap stack
stack_top:

align 4096
boot_pml4:
    resb 4096
boot_pdpt:
    resb 4096
boot_pd:
    resb 4096

SECTION .text
global _start
extern kernel_main

_start:
    cli
    mov esp, stack_top

    ; Save Multiboot2 magic + info pointer
    mov edi, eax                 ; magic
    mov esi, ebx                 ; info

    ; Verify Multiboot2 magic
    cmp eax, 0x36D76289
    jne .hang

    ; ---- Enable PAE + long mode ----
    ; Identity-map first 1 GiB using 2 MiB huge pages (512 entries)
    ; PML4[0] -> PDPT, PDPT[0] -> PD, PD[i] = (i << 21) | PRESENT | RW | HUGE

    mov eax, boot_pdpt
    or  eax, 0x03                ; present | writable
    mov [boot_pml4], eax

    mov eax, boot_pd
    or  eax, 0x03
    mov [boot_pdpt], eax

    mov ecx, 0
.map_loop:
    mov eax, ecx
    shl eax, 21                  ; 2 MiB * index
    or  eax, 0x83                ; present | writable | page size (2MiB)
    mov [boot_pd + ecx * 8], eax
    inc ecx
    cmp ecx, 512
    jb  .map_loop

    ; Load CR3
    mov eax, boot_pml4
    mov cr3, eax

    ; Enable PAE
    mov eax, cr4
    or  eax, (1 << 5)
    mov cr4, eax

    ; Enable Long Mode (EFER.LME)
    mov ecx, 0xC0000080
    rdmsr
    or  eax, (1 << 8)
    wrmsr

    ; Enable paging
    mov eax, cr0
    or  eax, (1 << 31)
    mov cr0, eax

    ; Load 64-bit GDT and far jump into long mode
    lgdt [gdt64.pointer]
    jmp gdt64.code:long_mode_entry

.hang:
    hlt
    jmp .hang

;------------------------------------------------------------------------------
BITS 64
long_mode_entry:
    mov ax, gdt64.data
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    mov rsp, stack_top

    ; edi = magic, esi = multiboot info (still valid in lower 32 bits)
    mov edi, edi
    mov esi, esi
    xor rbp, rbp

    ; Pass multiboot info address to kernel_main (RDI on System V ABI)
    mov rdi, rsi
    call kernel_main

.halt:
    cli
    hlt
    jmp .halt

;------------------------------------------------------------------------------
SECTION .rodata
align 16
gdt64:
    dq 0                         ; null
.code equ $ - gdt64
    dq (1 << 43) | (1 << 44) | (1 << 47) | (1 << 41) | (1 << 53) ; code
.data equ $ - gdt64
    dq (1 << 44) | (1 << 47) | (1 << 41)                         ; data
.pointer:
    dw $ - gdt64 - 1
    dq gdt64
