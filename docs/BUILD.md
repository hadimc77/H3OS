# Building & Running H3OS

## Quick start (Docker)

```bash
chmod +x tools/*.sh
./tools/docker-build.sh
./tools/run-qemu.sh
```

## Native (Debian/Ubuntu/WSL)

```bash
sudo apt update
sudo apt install build-essential nasm grub-pc-bin grub-common xorriso qemu-system-x86
make clean all iso
make run
```

### Cross toolchain (optional)

```bash
export CROSS=x86_64-elf-
make
```

## CMake

```bash
cmake -B build-cmake -DCMAKE_BUILD_TYPE=Release
cmake --build build-cmake
```

The Makefile remains the supported path for ISO generation.

## QEMU tips

```bash
qemu-system-x86_64 -cdrom build/h3os.iso -m 1024M -smp 2 -serial stdio
```

Serial output shows the kernel log (`[INFO][boot] …`).

## VirtualBox

1. New VM → Other/Unknown 64-bit  
2. 512 MB+ RAM  
3. Attach `build/h3os.iso`  
4. Enable PAE/NX  

## Real hardware

Write ISO to USB ( balenaEtcher / `dd` ) and boot with Secure Boot disabled until signed boot is implemented.
