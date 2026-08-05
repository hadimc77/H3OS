/**
 * H3OS — Security subsystem bootstrap
 */
#include "security.h"
#include <h3os/kernel.h>
#include <h3os/string.h>

static user_t users[SEC_MAX_USERS];
static user_t* current = NULL;

void security_init(void) {
    memset(users, 0, sizeof(users));

    users[0].uid = 0;
    users[0].gid = 0;
    strcpy(users[0].name, "root");
    users[0].active = true;

    users[1].uid = 1000;
    users[1].gid = 1000;
    strcpy(users[1].name, "user");
    users[1].active = true;

    current = &users[1];

    KLOG_INFO("security", "Users loaded; current=%s (uid=%u)",
              current->name, current->uid);
    KLOG_INFO("security", "Memory protection + sandbox hooks armed");
}

user_t* security_current_user(void) { return current; }

bool security_check_perm(u32 mode, u32 uid, u32 gid, bool write) {
    user_t* u = current;
    if (!u) return false;
    if (u->uid == 0) return true; /* root */

    u32 shift = 0;
    if (u->uid == uid) shift = 6;
    else if (u->gid == gid) shift = 3;
    else shift = 0;

    u32 bits = (mode >> shift) & 7;
    if (write) return (bits & 2) != 0;
    return (bits & 4) != 0;
}

void security_enable_smep_smap_hints(void) {
    /* Future: set CR4.SMEP / CR4.SMAP when user-mode is live */
    KLOG_DEBUG("security", "SMEP/SMAP deferred until ring-3 bring-up");
}
