# H3OS build environment
FROM debian:bookworm-slim

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential nasm gcc-multilib \
    xorriso grub-pc-bin grub-common \
    qemu-system-x86 \
    python3 \
    && rm -rf /var/lib/apt/lists/*

# Native gcc on amd64 can target freestanding x86_64 with -m64 -ffreestanding
WORKDIR /src
COPY . /src

CMD ["make", "iso"]
