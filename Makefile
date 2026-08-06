#==============================================================================
# H3OS Makefile — cross-compile x86_64 freestanding hybrid kernel
#==============================================================================

ARCH        ?= x86_64
TARGET      := h3os
BUILD       := build
ISO         := $(BUILD)/h3os.iso
KERNEL_ELF  := $(BUILD)/h3os.elf

# Toolchain — override with CROSS=x86_64-elf- or use host clang
CROSS       ?=
CC          := $(CROSS)gcc
CXX         := $(CROSS)g++
AS          := nasm
LD          := $(CROSS)ld
OBJCOPY     := $(CROSS)objcopy

CFLAGS := -std=c11 -ffreestanding -fno-builtin -fno-stack-protector \
          -fno-pic -fno-pie -mno-red-zone -m64 -Wall -Wextra -O2 \
          -Isdk/include -I. -Iboot -MMD -MP

ASFLAGS := -f elf64
LDFLAGS := -nostdlib -static -z max-page-size=0x1000 -T boot/linker.ld

ASM_SRCS := \
	boot/boot.asm \
	kernel/arch/x86_64/isr.asm

C_SRCS := \
	kernel/core/main.c \
	kernel/core/string.c \
	kernel/core/log.c \
	kernel/core/adaptive.c \
	kernel/core/sched.c \
	kernel/core/ipc.c \
	kernel/core/power.c \
	kernel/arch/x86_64/cpu.c \
	kernel/arch/x86_64/gdt.c \
	kernel/arch/x86_64/idt.c \
	kernel/sync/sync.c \
	kernel/syscall/syscall.c \
	memory/pmm.c \
	memory/heap.c \
	memory/vmm.c \
	drivers/timer/timer.c \
	drivers/keyboard/keyboard.c \
	drivers/mouse/mouse.c \
	drivers/framebuffer/framebuffer.c \
	drivers/rtc/rtc.c \
	drivers/pci/pci.c \
	filesystem/vfs/vfs.c \
	window_manager/wm.c \
	desktop/desktop.c \
	applications/terminal/terminal.c \
	applications/calculator/calculator.c \
	applications/settings/settings.c \
	applications/filemanager/filemanager.c \
	applications/taskmanager/taskmanager.c \
	network/net.c \
	security/security.c

ASM_OBJS := $(patsubst %.asm,$(BUILD)/%.o,$(ASM_SRCS))
C_OBJS   := $(patsubst %.c,$(BUILD)/%.o,$(C_SRCS))
OBJS     := $(ASM_OBJS) $(C_OBJS)
DEPS     := $(C_OBJS:.o=.d)

.PHONY: all clean iso run run-serial dirs info

all: dirs $(KERNEL_ELF)

dirs:
	@mkdir -p $(BUILD)/boot \
		$(BUILD)/kernel/core $(BUILD)/kernel/arch/x86_64 \
		$(BUILD)/kernel/sync $(BUILD)/kernel/syscall \
		$(BUILD)/memory \
		$(BUILD)/drivers/timer $(BUILD)/drivers/keyboard $(BUILD)/drivers/mouse \
		$(BUILD)/drivers/framebuffer $(BUILD)/drivers/rtc $(BUILD)/drivers/pci \
		$(BUILD)/filesystem/vfs \
		$(BUILD)/window_manager $(BUILD)/desktop \
		$(BUILD)/applications/terminal $(BUILD)/applications/calculator \
		$(BUILD)/applications/settings $(BUILD)/applications/filemanager \
		$(BUILD)/applications/taskmanager \
		$(BUILD)/network $(BUILD)/security \
		$(BUILD)/iso/boot/grub

$(BUILD)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/%.o: %.asm
	$(AS) $(ASFLAGS) $< -o $@

$(KERNEL_ELF): $(OBJS) boot/linker.ld
	$(LD) $(LDFLAGS) -o $@ $(OBJS)
	@echo "[H3OS] Kernel linked: $@"

iso: $(KERNEL_ELF)
	cp $(KERNEL_ELF) $(BUILD)/iso/boot/h3os.elf
	cp boot/grub.cfg $(BUILD)/iso/boot/grub/grub.cfg
	grub-mkrescue -o $(ISO) $(BUILD)/iso
	@echo "[H3OS] ISO ready: $(ISO)"

run: iso
	qemu-system-x86_64 -cdrom $(ISO) -m 512M -serial stdio -no-reboot -no-shutdown

run-serial: $(KERNEL_ELF)
	qemu-system-x86_64 -kernel $(KERNEL_ELF) -m 512M -serial stdio \
		-display sdl -no-reboot -no-shutdown || \
	qemu-system-x86_64 -kernel $(KERNEL_ELF) -m 512M -serial stdio \
		-no-reboot -no-shutdown

clean:
	rm -rf $(BUILD)

info:
	@echo "H3OS build"
	@echo "  CC=$(CC)"
	@echo "  Kernel=$(KERNEL_ELF)"
	@echo "  ISO=$(ISO)"

-include $(DEPS)
