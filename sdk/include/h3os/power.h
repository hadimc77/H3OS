/**
 * H3OS — Power management hooks
 */
#ifndef H3OS_POWER_H
#define H3OS_POWER_H

#include <h3os/types.h>

void power_init(void);
void power_shutdown(void);
void power_reboot(void);
void power_sleep(void);

#endif
