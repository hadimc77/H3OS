/**
 * H3OS — PCI bus enumerator (Type-1 config space)
 */
#ifndef H3OS_PCI_H
#define H3OS_PCI_H

#include <h3os/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    u8  bus, slot, func;
    u16 vendor, device;
    u8  class_code, subclass, prog_if;
} pci_device_t;

#define PCI_MAX_DEVICES 64

void pci_init(void);
u32  pci_device_count(void);
const pci_device_t* pci_get(u32 index);

#ifdef __cplusplus
}
#endif

#endif /* H3OS_PCI_H */
