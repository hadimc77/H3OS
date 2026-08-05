# H3OS Architecture Overview

## Hybrid Kernel

H3OS uses a **hybrid kernel**: performance-critical services (memory, scheduling,
IPC, drivers) run in kernel space, while higher-level subsystems (desktop,
apps, package management) are modular and replaceable.

```
Bootloader (Multiboot2 / GRUB)
        │
        ▼
   CPU + GDT/IDT
        │
        ▼
 Memory (PMM → VMM → Heap)
        │
        ▼
 Drivers (PIT, KBD, PCI, FB, RTC)
        │
        ▼
 Scheduler + Syscalls + Security
        │
        ▼
 VFS (RAMFS / H3FS API)
        │
        ▼
 Graphics → Window Manager → Desktop (Horizon)
        │
        ▼
 Applications (Terminal, …)
```

## Design Decisions

1. **Multiboot2** — portable boot across QEMU, VirtualBox, VMware, real hardware via GRUB.
2. **Identity-mapped first GiB** — fast bring-up; higher-half kernel is a planned upgrade.
3. **Software compositor** — works without GPU drivers; GPU paths plug into `graphics/`.
4. **Adaptive performance** — detects RAM/CPU and selects Low / Balanced / High profiles.
5. **H3FS** — on-disk format defined now; RAMFS implements the VFS API for day-one usability.
6. **Brand palette** — deep teal “Horizon” theme; intentionally not generic purple/cream AI aesthetics.

## Target Roadmap

| Milestone | Focus |
|-----------|--------|
| 0.1 Horizon | Boot, memory, desktop, terminal (this tree) |
| 0.2 | User-mode, real context switch, ELF loader |
| 0.3 | Persistent H3FS, AHCI/NVMe |
| 0.4 | Networking (TCP/IP), h3pkg repos |
| 0.5 | GPU accel, audio, SMP scheduling |
| 1.0 | Stable APIs, Secure Boot, installer |
