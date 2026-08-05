# H3OS Test Plan (v0.1)

## Boot

- [ ] GRUB shows H3OS menu entry
- [ ] Kernel serial log prints banner
- [ ] No panic before desktop loop

## Desktop

- [ ] Wallpaper + brand wordmark visible
- [ ] Top bar clock advances
- [ ] Dock visible
- [ ] Ctrl+L opens launcher
- [ ] Ctrl+D toggles theme

## Terminal

- [ ] Ctrl+T / auto-open terminal works
- [ ] `help`, `ls`, `pwd`, `systeminfo` succeed
- [ ] `cat /etc/motd` prints welcome text
- [ ] `mkdir` / `cd` / `rm` manipulate RAMFS

## Hardware paths

- [ ] PCI enumeration logs devices under QEMU
- [ ] Adaptive profile matches RAM size
