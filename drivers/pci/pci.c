/**
 * H3OS — PCI configuration space scanner
 */
#include "pci.h"
#include <h3os/kernel.h>

static pci_device_t devices[PCI_MAX_DEVICES];
static u32 device_count = 0;

static u32 pci_cfg_read(u8 bus, u8 slot, u8 func, u8 offset) {
    u32 address = (u32)((1u << 31) | ((u32)bus << 16) | ((u32)slot << 11) |
                        ((u32)func << 8) | (offset & 0xFC));
    outl(0xCF8, address);
    return inl(0xCFC);
}

void pci_init(void) {
    device_count = 0;
    for (u16 bus = 0; bus < 256; bus++) {
        for (u8 slot = 0; slot < 32; slot++) {
            for (u8 func = 0; func < 8; func++) {
                u32 id = pci_cfg_read((u8)bus, slot, func, 0);
                u16 vendor = (u16)(id & 0xFFFF);
                if (vendor == 0xFFFF) {
                    if (func == 0) break;
                    continue;
                }
                if (device_count >= PCI_MAX_DEVICES) goto done;

                u32 reg2 = pci_cfg_read((u8)bus, slot, func, 0x08);
                pci_device_t* d = &devices[device_count++];
                d->bus = (u8)bus;
                d->slot = slot;
                d->func = func;
                d->vendor = vendor;
                d->device = (u16)(id >> 16);
                d->class_code = (u8)(reg2 >> 24);
                d->subclass = (u8)(reg2 >> 16);
                d->prog_if = (u8)(reg2 >> 8);

                if (func == 0) {
                    u32 ht = pci_cfg_read((u8)bus, slot, 0, 0x0C);
                    if (((ht >> 16) & 0x80) == 0) break; /* not multi-function */
                }
            }
        }
    }
done:
    KLOG_INFO("pci", "Enumerated %u PCI device(s)", device_count);
    for (u32 i = 0; i < device_count && i < 8; i++) {
        KLOG_DEBUG("pci", "  %02x:%02x.%u  %04x:%04x  class %02x:%02x",
                   devices[i].bus, devices[i].slot, devices[i].func,
                   devices[i].vendor, devices[i].device,
                   devices[i].class_code, devices[i].subclass);
    }
}

u32 pci_device_count(void) { return device_count; }
const pci_device_t* pci_get(u32 index) {
    return index < device_count ? &devices[index] : NULL;
}
