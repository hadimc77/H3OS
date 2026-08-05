#!/usr/bin/env bash
# Build H3OS from Windows via WSL (no Docker/sudo required if NASM is in ~/.local)
set -euo pipefail
export PATH="$HOME/.local/bin:$PATH"
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"

if ! command -v nasm >/dev/null; then
  echo "NASM not found. Building into ~/.local ..."
  cd /tmp
  curl -fsSL -o nasm.tar.gz "https://www.nasm.us/pub/nasm/releasebuilds/2.16.03/nasm-2.16.03.tar.gz"
  tar xzf nasm.tar.gz
  cd nasm-2.16.03
  ./configure --prefix="$HOME/.local"
  make -j"$(nproc)"
  make install
  export PATH="$HOME/.local/bin:$PATH"
  cd "$ROOT"
fi

make clean all
echo "Kernel: $ROOT/build/h3os.elf"

if command -v grub-mkrescue >/dev/null && command -v xorriso >/dev/null; then
  make iso
else
  echo "NOTE: grub-mkrescue not installed — ELF only. Install grub-pc-bin xorriso for ISO."
fi

if command -v qemu-system-x86_64 >/dev/null; then
  echo "Launching QEMU (serial log on stdio)..."
  if [[ -f build/h3os.iso ]]; then
    qemu-system-x86_64 -cdrom build/h3os.iso -m 512M -serial stdio -no-reboot -no-shutdown
  else
    qemu-system-x86_64 -kernel build/h3os.elf -m 512M -serial stdio \
      -display none -no-reboot -no-shutdown || true
  fi
fi
