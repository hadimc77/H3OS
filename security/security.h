/**
 * H3OS — Security foundations (users, permissions, isolation hooks)
 */
#ifndef H3OS_SECURITY_H
#define H3OS_SECURITY_H

#include <h3os/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SEC_NAME_MAX 32
#define SEC_MAX_USERS 16

typedef struct {
    u32  uid;
    u32  gid;
    char name[SEC_NAME_MAX];
    bool active;
} user_t;

void   security_init(void);
user_t* security_current_user(void);
bool   security_check_perm(u32 mode, u32 uid, u32 gid, bool write);
void   security_enable_smep_smap_hints(void);

#ifdef __cplusplus
}
#endif

#endif /* H3OS_SECURITY_H */
