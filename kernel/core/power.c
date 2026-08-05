#include <h3os/power.h>
#include <h3os/kernel.h>

void power_init(void) {
    KLOG_INFO("power", "ACPI/power management stubs armed");
}

void power_shutdown(void) {
    KLOG_INFO("power", "Shutdown requested");
    outw(0x604, 0x2000);  /* QEMU */
    outw(0xB004, 0x2000); /* Bochs/older QEMU */
    for (;;) cpu_halt();
}

void power_reboot(void) {
    KLOG_INFO("power", "Reboot requested");
    outb(0x64, 0xFE);
    for (;;) cpu_halt();
}

void power_sleep(void) {
    KLOG_INFO("power", "Sleep not yet supported");
}
