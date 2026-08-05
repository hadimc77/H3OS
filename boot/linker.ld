/*==============================================================================
 * H3OS Linker Script — x86_64 Multiboot2 kernel
 * Places Multiboot header in the first 8 KiB as required by the spec.
 *============================================================================*/

ENTRY(_start)

SECTIONS
{
    . = 1M;

    .multiboot ALIGN(8) :
    {
        KEEP(*(.multiboot))
    }

    .text ALIGN(4K) :
    {
        *(.text .text.*)
    }

    .rodata ALIGN(4K) :
    {
        *(.rodata .rodata.*)
    }

    .data ALIGN(4K) :
    {
        *(.data .data.*)
    }

    .bss ALIGN(4K) :
    {
        *(COMMON)
        *(.bss .bss.*)
    }

    /* Kernel end marker for the physical memory manager */
    _kernel_start = 1M;
    _kernel_end   = .;
}
