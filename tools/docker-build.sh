#!/usr/bin/env bash
# Build H3OS inside Docker (works on Windows/macOS/Linux hosts)
set -euo pipefail
cd "$(dirname "$0")/.."
docker build -t h3os-builder .
docker run --rm -v "$(pwd):/src" -w /src h3os-builder make clean all iso
echo "ISO: build/h3os.iso"
