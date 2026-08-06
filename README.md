# H3OS

**The Future Starts Here.**

H3OS is a modern hybrid-kernel operating system for x86_64 (ARM64 / RISC-V planned).  
Inspired by the best ideas from Windows, Linux, and macOS — original in architecture, branding, and implementation.

---

## മലയാളം / Malayalam

H3OS ഒരു പുതിയ hybrid-kernel ഓപ്പറേറ്റിംഗ് സിസ്റ്റമാണ്.  
ഇപ്പോൾ **bootable foundation** റെഡി ആണ്: kernel, memory manager, drivers, desktop (Horizon), terminal, VFS/RAMFS, adaptive performance.

### ബിൽഡ് ചെയ്യാൻ

**Docker (Windows-ൽ ഏറ്റവും എളുപ്പം):**
```bash
./tools/docker-build.sh
qemu-system-x86_64 -cdrom build/h3os.iso -m 512M -serial stdio
```

**Linux / WSL:**
```bash
sudo apt install build-essential nasm grub-pc-bin xorriso qemu-system-x86
make iso
make run
```

### Desktop (Windows-inspired Horizon shell)

- Bottom **taskbar**: Start (H3), Search, pinned apps, open-window buttons, tray, clock
- **Start menu** with pinned apps, recommended, power off
- **Notification / Action Center** with quick settings tiles
- **Desktop icons** (This PC, Files, Terminal, Settings, Recycle Bin)
- **Right-click** context menu (Personalize, Display settings, …)
- **Snap layouts** (Ctrl+N) + drag-to-edge snap
- Window chrome: min / max / close like Windows

| Shortcut | Action |
|----------|--------|
| Ctrl+T | Terminal |
| Ctrl+L | Start menu |
| Ctrl+F | File Manager |
| Ctrl+S | Settings |
| Ctrl+N | Snap layouts |
| Ctrl+D | Dark / Light theme |

### `.exe` run (PE32+)

Terminal-ൽ:
```text
run /bin/hello.exe
hello.exe
```
H3OS native PE executables work now. Full Windows Win32 apps (Notepad.exe, games) need a future compatibility layer — see `docs/EXE.md`.

---

## English

### What works in v0.1 (Horizon)

- Multiboot2 boot → 64-bit long mode  
- Hybrid kernel core (logging, panic, CPUID)  
- PMM + heap + VMM helpers  
- GDT / IDT / PIC / PIT / PS/2 keyboard / RTC / PCI scan  
- Linear framebuffer + double buffering + software compositor  
- Horizon desktop (wallpaper, top bar, dock, launcher, themes)  
- Window manager (move/resize/focus chrome)  
- Built-in Terminal with UNIX-like commands  
- VFS + RAMFS (H3FS on-disk format defined)  
- Adaptive Low / Balanced / High performance profiles  
- Security users + permission checks  
- Network loopback scaffold + `h3pkg` tooling stub  

### Project layout

```
boot/              Multiboot2 entry + linker + GRUB
kernel/            Hybrid kernel (arch, core, sync, syscall)
memory/            PMM, VMM, heap
drivers/           timer, keyboard, framebuffer, rtc, pci, …
filesystem/        vfs, ramfs/h3fs, fat32/ext stubs
graphics/          theme tokens / future GPU path
window_manager/    compositing WM
desktop/           Horizon DE
network/           stack scaffold
security/          users / permissions
applications/      terminal (+ more apps)
sdk/include/h3os/  public headers
tools/             build/run/h3pkg helpers
docs/              architecture & guides
tests/             reserved
```

### Build requirements

- NASM  
- GCC or Clang capable of freestanding x86_64 (`-ffreestanding -mno-red-zone`)  
- GNU ld  
- GRUB (`grub-mkrescue`) + xorriso for ISO  
- QEMU (or VirtualBox / VMware — boot the ISO)  

Optional: Docker (see `Dockerfile`).

### Run

```bash
make            # build/h3os.elf
make iso        # build/h3os.iso
make run        # QEMU + serial log
```

VirtualBox / VMware: create a machine, attach `build/h3os.iso`, enable EFI optional (BIOS+GRUB is enough).

### Implementation notes

- Early boot identity-maps the first 1 GiB with 2 MiB pages.  
- Desktop renders through a software back buffer for tear-free presents.  
- Adaptive layer dials back blur/animations on low RAM / single-core hosts.  
- Syscall table and scheduler are live; full ring-3 + context switch is next.  

See [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md) and [docs/BUILD.md](docs/BUILD.md).

---

**H3OS 0.1.0 — Horizon**  
Codename fits the brand: a clear horizon line where the future begins.
