# H3OS Source Tree

```
H3OS/
├── boot/                  # Multiboot2, linker, GRUB
│   ├── boot.asm
│   ├── linker.ld
│   ├── grub.cfg
│   └── multiboot2.h
├── kernel/
│   ├── arch/x86_64/       # CPU, GDT, IDT, ISR
│   ├── core/              # main, log, string, sched, adaptive, ipc, power
│   ├── sync/              # spinlock, mutex, semaphore
│   └── syscall/           # syscall dispatcher
├── memory/                # PMM, VMM, heap
├── drivers/               # timer, keyboard, mouse, framebuffer, rtc, pci
├── filesystem/            # vfs, h3fs, fat32, ext2
├── graphics/              # theme tokens / future GPU
├── window_manager/        # compositing WM
├── desktop/               # Horizon DE
├── network/               # stack scaffold
├── security/              # users / permissions
├── applications/          # terminal, calculator, settings
├── sdk/include/h3os/      # public headers
├── tools/                 # docker/wsl/qemu/h3pkg helpers
├── docs/                  # architecture, build, roadmap
├── tests/                 # test plan
├── Makefile
├── CMakeLists.txt
├── Dockerfile
└── README.md
```
