#!/usr/bin/env bash
# Run H3OS in QEMU
set -euo pipefail
cd "$(dirname "$0")/.."
if [[ ! -f build/h3os.iso ]]; then
  echo "ISO missing — run: make iso   or   ./tools/docker-build.sh"
  exit 1
fi
exec qemu-system-x86_64 -cdrom build/h3os.iso -m 512M -serial stdio -no-reboot -no-shutdown "$@"
