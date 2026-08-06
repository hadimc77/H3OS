/**
 * Install sample .exe files into the RAMFS /bin directory
 */
#include "pe_seed.h"
#include "pe.h"
#include "../filesystem/vfs/vfs.h"
#include "../sdk/samples/hello_exe.h"
#include <h3os/kernel.h>

void pe_seed_binaries(void) {
    vfs_mkdir("/bin", 0755);
    vfs_node_t* hello = vfs_create("/bin/hello.exe", 0755);
    if (hello) {
        vfs_write(hello, hello_exe, hello_exe_len);
        KLOG_INFO("pe", "Installed /bin/hello.exe (%u bytes)", hello_exe_len);
    }
}
