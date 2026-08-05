/**
 * H3OS Terminal application — built-in command shell
 */
#ifndef H3OS_TERMINAL_H
#define H3OS_TERMINAL_H

#include <h3os/types.h>
#include "../../window_manager/wm.h"

#ifdef __cplusplus
extern "C" {
#endif

window_t* terminal_open(void);
void      terminal_init_commands(void);

#ifdef __cplusplus
}
#endif

#endif /* H3OS_TERMINAL_H */
