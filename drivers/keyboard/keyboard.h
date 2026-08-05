/**
 * H3OS — PS/2 keyboard driver
 */
#ifndef H3OS_KEYBOARD_H
#define H3OS_KEYBOARD_H

#include <h3os/types.h>

#ifdef __cplusplus
extern "C" {
#endif

void keyboard_init(void);
bool keyboard_has_char(void);
char keyboard_read_char(void); /* blocking if empty returns 0 */
bool keyboard_try_read(char* out);

#ifdef __cplusplus
}
#endif

#endif /* H3OS_KEYBOARD_H */
