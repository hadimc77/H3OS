/**
 * H3OS — PIT timer + system clock
 */
#ifndef H3OS_TIMER_H
#define H3OS_TIMER_H

#include <h3os/types.h>

#ifdef __cplusplus
extern "C" {
#endif

void timer_init(u32 frequency_hz);
u64  timer_ticks(void);
u64  timer_uptime_ms(void);
void timer_sleep_ms(u64 ms);

#ifdef __cplusplus
}
#endif

#endif /* H3OS_TIMER_H */
